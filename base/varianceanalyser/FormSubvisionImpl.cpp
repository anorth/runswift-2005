/* FormSubvisionImpl.cpp
 * Implementation of FormSubvision
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qfont.h>
#include <qpainter.h>
#include <qsocketdevice.h>

#include "FormSubvisionImpl.h"
#include "../robolink/robolink.h"

static const bool DEBUG = false;

using namespace std;

enum {
    // Dimensions of the pixmap as displayed
    PIXMAP_COLS = 2 * IMAGE_COLS,
    PIXMAP_ROWS = 2 * IMAGE_ROWS,

    // The maximum value possible in a y, u or v channel
    CHANNEL_MAX = 255,

    // Grid line spacing on histogram (pix)
    GRID_SPACE = 10,

    // Offsets on the gradients graph to minimise clobbering.
    // Should be multiple of GRID_SPACE
    DU_OFFSET = 20,
    DV_OFFSET = -20,
};

static const char* SLICE_COLOUR = "red",
                *HORIZON_COLOUR = "cyan",
                *SCANLINE_COLOUR = "blue",
                *GRID_COLOUR = "lightGray";

static const char* VF_COLOURS[] = {
    "black",    // VF_NONE
    "green",    // VF_FIELDLINE
    "yellow",   // VF_BALL
    "red",      // VF_OBSTACLE
    "darkGreen",// VF_FIELD
    "pink",     // VF_PINK
    "lightBlue",// VF_BLUE
    "orange",   // VF_YELLOW
    "purple",   // VF_UNKNOWN
};

static const char* OBJ_COLOUR_BALL = "yellow";
static const char* OBJ_COLOUR_BEACONPINK = "pink";
static const char* OBJ_COLOUR_BEACONBLUE = "blue";
static const char* OBJ_COLOUR_BEACONYELLOW = "yellow";

// Beacon colours corresponding to the vob indicies in SubObject in
// pairs of { top, bottom }
static const char* beaconColours[4][2] = {
    { OBJ_COLOUR_BEACONBLUE, OBJ_COLOUR_BEACONPINK },
    { OBJ_COLOUR_BEACONPINK, OBJ_COLOUR_BEACONBLUE  },
    { OBJ_COLOUR_BEACONYELLOW, OBJ_COLOUR_BEACONPINK },
    { OBJ_COLOUR_BEACONPINK, OBJ_COLOUR_BEACONYELLOW }};
// goal colours correspoinding to vGoals indicies in SubObject
static const char* goalColours[] = {
    OBJ_COLOUR_BEACONBLUE,
    OBJ_COLOUR_BEACONYELLOW,
};

static const char* CPLANE_COLOURS[] = {
	"orange",
	"blue",
	"green",
	"yellow",
	"magenta",
	"darkBlue",
	"red",
	"green",
	"gray",
	"white",
	"black",
	"white",       // marker, not a real colour
	"white",     // ditto
	"gray", // end of colours
};
static const int NUM_CPLANE_COLOURS =
    sizeof(CPLANE_COLOURS)/sizeof(CPLANE_COLOURS[0]);

static Q_LONG readAllBlock(QIODevice* in, char* data, Q_ULONG size);
static QRgb yuv2rgb(int y, int u, int v);
static void drawLine(QPainter& paint, point start, point finish, int scale = 1);

FormSubvisionImpl::FormSubvisionImpl(QApplication* a, QWidget* parent,
                                        const char* name, WFlags fl) 
            : FormSubvision(parent, name, fl),
                app(a),
                subvision(cortex.subvision),
                subobject(cortex.subobject),
                framereader(this),
                input(0), sock(0), pixmap(0),
                currentFrame(-1) {
    this->pt2relative[0] = this->pt2relative[1] = INT_MAX;
    this->yuv = new uchar[IMAGE_COLS * IMAGE_ROWS * 6];
    this->yplane = this->yuv;
    this->uplane = this->yplane + IMAGE_COLS;
    this->vplane = this->uplane + IMAGE_COLS;
    this->image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);

    QPixmap temp(this->pixmapImage->width(), this->pixmapImage->height());
    this->pixmapImage->setPixmap(temp);

    this->setFocusPolicy(QWidget::StrongFocus);
    QFont f("Bitstream Vera Serif", 8, 0);
    this->setFont( f );
}

FormSubvisionImpl::~FormSubvisionImpl() { 
    if (this->pixmap)
        delete this->pixmap;
}

/* Handles opening of a new file via File->Open menu */
void FormSubvisionImpl::slotFileOpen() {
    //cerr << "slotFileOpen" << endl;
    int ret = -1;
    if (this->input) {
        if (this->input->isOpen()) 
            this->input->close();
        delete this->input;
        this->input = 0;
    }
    this->online = false;
    this->alreadyGotHeader = false;

    if (this->image)
        delete this->image;
    this->image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
    this->image->fill(Qt::gray.pixel());
    this->clearData();

    QString filename = QFileDialog::getOpenFileName(
        "../../..",
        "All files (*.*);;Log files (*.RLOG, *.rlog);;BFL files (*.BFL)",
        this, "Open BFL file", "Choose a BFL file to open");
    if (filename.isNull())
        return;

    this->input = new QFile(filename);
    this->input->open(IO_ReadOnly);

    if (filename.right(4) == "RLOG" || filename.right(4) == "rlog") {
        this->frames.clear();
        ret = readFrame();
        this->currentFrame = 0;
        if (this->frames.size() > 0) {
            btnNextFrame->setEnabled(true);
            btnPrevFrame->setEnabled(true);
        } else {
            btnNextFrame->setEnabled(false);
            btnPrevFrame->setEnabled(false);
            lblFrameNo->setText("");
            cout << "RLOG contained no frames" << endl;
            return;
        }
    } else {
        cerr << filename << " - unrecognised extension. Expected .rlog"
            << endl;
        return;
    }
    
    if (ret == -1)
        return;

    if (this->shouldSampleImage)
        this->sampleImage();
    this->slotRedraw();
    
    
    logObjects();
            
}


void FormSubvisionImpl::logObjects() {
    
    QString ext = QString(".txt");
    QString name = (dynamic_cast<QFile*>(this->input))->name();
    
    int index1 = name.findRev("/")+1;
    int index2 = name.findRev(".rlog");
    if (index1 == -1 || index2 == -1) {
        cout << "ERROR - not right format, must end with .rlog" << endl;
        return;
    }
    name = name.mid(index1,index2-index1);
        
    ofstream outBallDist((QString("./logBallDist-")+name+ext).data());
    ofstream outBallHead((QString("./logBallHead-")+name+ext).data()); 
    
    ofstream outBeaconDistPinkBlue((QString("./logBeaconDistPinkBlue-")+name+ext).data());
    ofstream outBeaconHeadPinkBlue((QString("./logBeaconHeadPinkBlue-")+name+ext).data()); 
    ofstream outBeaconDistBluePink((QString("./logBeaconDistBluePink-")+name+ext).data());
    ofstream outBeaconHeadBluePink((QString("./logBeaconHeadBluePink-")+name+ext).data()); 
    ofstream outBeaconDistPinkYellow((QString("./logBeaconDistPinkYellow-")+name+ext).data());
    ofstream outBeaconHeadPinkYellow((QString("./logBeaconHeadPinkYellow-")+name+ext).data()); 
    ofstream outBeaconDistYellowPink((QString("./logBeaconDistYellowPink-")+name+ext).data());
    ofstream outBeaconHeadYellowPink((QString("./logBeaconHeadYellowPink-")+name+ext).data()); 
                
    ofstream outGoalDistYellow((QString("./logGoalDistYellow-")+name+ext).data());
    ofstream outGoalHeadYellow((QString("./logGoalHeadYellow-")+name+ext).data());
    ofstream outGoalDistBlue((QString("./logGoalDistBlue-")+name+ext).data());
    ofstream outGoalHeadBlue((QString("./logGoalHeadBlue-")+name+ext).data());
    
    ofstream outLinesDist((QString("./logLineDist-")+name+ext).data());
    ofstream outLinesHead((QString("./logLineHead-")+name+ext).data()); 
    
    /*
    if (!outBallDist) { 
        cout << "ERROR - ball dist" << endl;
        return;
    }
    if (!outBallHead) {
        cout << "ERROR - ball head" << endl;
        return;
    }
    if (!outBeaconDist) { 
        cout << "ERROR - beacon dist" << endl;
        return;
    }
    if (!outBeaconHead) { 
        cout << "ERROR - beacon head" << endl;
        return;
    }
    if (!outGoalDist) { 
        cout << "ERROR - goal dist" << endl;
        return;
    }
    if (!outGoalHead) { 
        cout << "ERROR - goal head" << endl;
        return;
    }
    if (!outLinesDist) { 
        cout << "ERROR - line dist" << endl;
        return;
    }
    if (!outLinesHead) { 
        cout << "ERROR - line head" << endl;
        return;
    }
      */  
    while (1) {
        slotNextFrame();
        if (!this->btnNextFrame->isEnabled()) {
            break;
        }
        
        if (this->vBall.cf > 0) {
            outBallDist << this->vBall.dist2 << endl;
            outBallHead << this->vBall.head << endl;    
        }
        
        if (this->vBeacons[svPinkOnBlue].cf > 0) {
            outBeaconDistPinkBlue << this->vBeacons[svPinkOnBlue].dist2 << endl;
            outBeaconHeadPinkBlue << this->vBeacons[svPinkOnBlue].head << endl;
        }
        
        if (this->vBeacons[svBlueOnPink].cf > 0) {
            outBeaconDistBluePink << this->vBeacons[svBlueOnPink].dist2 << endl;
            outBeaconHeadBluePink << this->vBeacons[svBlueOnPink].head << endl;
        }
        
        if (this->vBeacons[svPinkOnYellow].cf > 0) {
            outBeaconDistPinkYellow << this->vBeacons[svPinkOnYellow].dist2 << endl;
            outBeaconHeadPinkYellow << this->vBeacons[svPinkOnYellow].head << endl;   
        }
        
        if (this->vBeacons[svYellowOnPink].cf > 0) { 
            outBeaconDistYellowPink << this->vBeacons[svYellowOnPink].dist2 << endl;
            outBeaconHeadYellowPink << this->vBeacons[svYellowOnPink].head << endl;
        }
        
        if (this->vGoals[svBlueGoal].cf > 0) {
            outGoalDistBlue << this->vGoals[svBlueGoal].dist2 << endl;
            outGoalHeadBlue << this->vGoals[svBlueGoal].head << endl;
        }
        
        if (this->vGoals[svYellowGoal].cf > 0) {
            outGoalDistYellow << this->vGoals[svYellowGoal].dist2 << endl;
            outGoalHeadYellow << this->vGoals[svYellowGoal].head << endl;
        }     
                                                  
    } 
    
    outBallDist.close(); 
    outBallHead.close();
    
    outBeaconDistPinkBlue.close();  
    outBeaconHeadPinkBlue.close();
    outBeaconDistBluePink.close();  
    outBeaconHeadBluePink.close();
    outBeaconDistPinkYellow.close();  
    outBeaconHeadPinkYellow.close();
    outBeaconDistYellowPink.close();  
    outBeaconHeadYellowPink.close();
    
    outGoalDistYellow.close();
    outGoalHeadYellow.close();
    outGoalDistBlue.close();
    outGoalHeadBlue.close();    
    
    while (1) {
        slotPrevFrame();
        if (!this->btnPrevFrame->isEnabled()) {
            break;
        }
    }
}


/* Displays a dialog for connection settings and connects to the specified
 * host/port if the dialog is accepted
 */
void FormSubvisionImpl::slotConnectDialog(void) {
    if (this->connect.exec() == QDialog::Accepted) {
        istringstream str(this->connect.textPort->text());
        unsigned short port;
        str >> port;
        this->slotConnect(this->connect.textHost->text(), port);
    }
}

/* Connects to the specified host and port and starts a thread to read and
 * display the data.
 */
void FormSubvisionImpl::slotConnect(QString host, unsigned short port) {
    if (this->input) {
        if (this->input->isOpen()) 
            this->input->close();
        delete this->input;
        this->input = 0;
    }

    if (this->image)
        delete this->image;
    this->image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
    this->image->fill(Qt::gray.pixel());
    this->clearData();

    if (this->sock)
        delete this->sock;
    this->sock = new QSocketDevice();

    if (DEBUG)
        cerr << "Connecting to " << host << ":" << port << "...";
    if (! this->sock->connect(host, port)) {
        cout << "Connection failed. ";
        QSocketDevice::Error reason = sock->error();
        if (reason == QSocketDevice::NoError)
            cout << "No error, try again later" << endl;
        else if (reason == QSocketDevice::AlreadyBound)
            cout << "Socket already  bound" << endl;
        else if (reason == QSocketDevice::Inaccessible)
            cout << "Socket inaccessible" << endl;
        else if (reason == QSocketDevice::InternalError)
            cout << "Internal error" << endl;
        else if (reason == QSocketDevice::Impossible)
            cout << "Socket attempted somthing impossible" << endl;
        else if (reason == QSocketDevice::NoFiles)
            cout << "No file descriptors left";
        else if (reason == QSocketDevice::ConnectionRefused)
            cout << "Connection refused" << endl;
        else if (reason == QSocketDevice::NetworkFailure)
            cout << "Network failure" << endl;
        else
            cout << "Unknown error" << endl;

        delete this->sock;
        this->sock = 0;
        this->input = 0;
        return;
    }
    
    cout << "Connected" << endl;
    this->online = true;
    this->alreadyGotHeader = false;
    this->input = this->sock;

    this->framereader.start();
}

/* Disconnects any connected socket */
void FormSubvisionImpl::slotDisconnect() {
    if (! this->online)
        return;

    if (this->input) {
        if (this->input->isOpen()) 
            this->input->close();
        delete this->input;
        this->input = 0;
    }

    if (this->sock) {
        delete this->sock;
        this->sock = 0;
    }
    this->online = false;
}

/* Handles histogram calculation and redraw when the slice coordinates
 * are changed. The co-ords from the spinboxes have origin in the bottom left
 * corner but the drawing methods all use top left, so we convert here.
 */
void FormSubvisionImpl::slotSliceCoordsChanged() {
    //cerr << "slotSliceCoordsChanged" << endl;
    slotRedraw();
}

/* Handles toggle of the point 2 "lock relative" checkbox, which updates
 * pt 2 coords to stay constant relative to point 1
 */
void FormSubvisionImpl::slotTogglePt2Relative(bool relative) {
    if (relative) {    // toggle true
        this->pt2relative[0] = spinPt2X->value()-spinPt1X->value();
        this->pt2relative[1] = spinPt2Y->value()-spinPt1Y->value();
        spinPt2X->setEnabled(false);
        spinPt2Y->setEnabled(false);
    } else {           // toggle false
        spinPt2X->setValue(spinPt1X->value()+this->pt2relative[0]);
        spinPt2Y->setValue(spinPt1Y->value()+this->pt2relative[1]);
        this->pt2relative[0] = this->pt2relative[1] = INT_MAX;
        // Out of bounds is handled for us by the spinbox's max value
        spinPt2X->setEnabled(true);
        spinPt2Y->setEnabled(true);
    }
}

/* Directs redrawing of the pixmap and histograms when something changes.
 * The updates are all drawn onto the hidden this->pixmap and then
 * bitBlt'ed once at the end onto pixmapImage. Yay - flicker free animation!
 */
void FormSubvisionImpl::slotRedraw() {

    if (currentFrame == -1)
        return;
    if (rbScanlineVert->isChecked())
        spinBoxScanline->setMaxValue(subvision.scanlines.size() - 1);
    else if (rbScanlineHorz->isChecked())
        spinBoxScanline->setMaxValue(subvision.horzScanlines.size() - 1);
    else
        spinBoxScanline->setMaxValue(0);

    if (tabHistogramSource->currentPage()->name() == QString("pgSlice")) {
        this->histPoint1.first = spinPt1X->value();
        this->histPoint1.second = spinPt2Y->value();

        if (this->pt2relative[0] == INT_MAX) {
            this->histPoint2.first = spinPt2X->value();
            this->histPoint2.second = IMAGE_ROWS - 1 - spinPt2Y->value();
        } else {
           this->histPoint2.first = histPoint1.first + this->pt2relative[0];
           this->histPoint2.second = histPoint1.second - this->pt2relative[1];
        }
    } else if (tabHistogramSource->currentPage()->name()
            == QString("pgScanline")) {
        if (rbScanlineVert->isChecked()) {
            int index = spinBoxScanline->value();
            this->histPoint1.first = subvision.scanlines[index].first.first;
            this->histPoint1.second = subvision.scanlines[index].first.second;
            this->histPoint2.first = subvision.scanlines[index].second.first;
            this->histPoint2.second = subvision.scanlines[index].second.second;
        } else if (rbScanlineHorz->isChecked()) {
            int index = spinBoxScanline->value();
            this->histPoint1.first = subvision.horzScanlines[index].first.first;
            this->histPoint1.second = subvision.horzScanlines[index].first.second;
            this->histPoint2.first = subvision.horzScanlines[index].second.first;
            this->histPoint2.second = subvision.horzScanlines[index].second.second;
        } else {
            histPoint1 = point(0, 0);
            histPoint2 = point(1,1);
        }
    }


    if (DEBUG)
        cerr << "redrawing" << endl;

    // revert to unaltered image for display
    this->displayImage();
    this->displayScanlines();
    this->drawSlice();
    this->displayFeatures();
    this->displayObjects();
    this->calculateHistogram();
    bitBlt(this->pixmapImage, 0, 0, this->pixmap);

    if (DEBUG)
        cout << "Features: " << this->features.size() << endl;
}

/* Handles advancing to the next frame of cam_orient/yuv data */
void FormSubvisionImpl::slotNextFrame(void) {
    this->clearData();
    int frame = readFrame();
    if (frame > 0)
        this->currentFrame++;
    else {
        btnNextFrame->setEnabled(false);
        return;
    }
    btnPrevFrame->setEnabled(true);

    if (this->shouldSampleImage)
        this->sampleImage();
    this->slotRedraw();
}

/* Handles regressing to the previous frame of cam_orient/yuv data */
void FormSubvisionImpl::slotPrevFrame(void) {
//    cerr << this->frames.size() << " frames." << endl;
    if (this->currentFrame == 0) {
        btnPrevFrame->setEnabled(false);
        return;
    }
    btnNextFrame->setEnabled(true);

    this->currentFrame--;
    this->input->at(this->frames[this->currentFrame]);
    this->clearData();
    int frame = readFrame();

    if (frame < 0) {
        btnNextFrame->setEnabled(false);
        return;
    }

    if (this->shouldSampleImage)
        this->sampleImage();
    this->slotRedraw();
}


void FormSubvisionImpl::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Left)
        this->slotPrevFrame();
    else if (e->key() == Qt::Key_Right)
        this->slotNextFrame();
    else
        e->ignore();
}


/***** Class Helpers *****/

/* Reads chunks from the current stream position and parses the data, stopping
 * when a chunk from a frame different to the first chunk is encountered.
 * The frame offset of this frame is appended to frames if it is larger than
 * the last offset already there. The frame number is returned, or -1 is
 * returned if there was an error or no more frames. If the data contained
 * RLNK_SUBVISION or RLNK_SUBOBJECT data then shouldSampleImage is set false,
 * else if there is YUV it is set true. Online should indicate whether 
 * the input stream is a file (and can be rewound) or a socket stream
 * (which shouldn't).
 */
int FormSubvisionImpl::readFrame(void) {
    static robolink_header header;
    int offset;

    int frameid = -1;
    bool foundYUV = false, foundSubvision = false;

    if (! this->alreadyGotHeader)
        offset = this->input->at();
    else
        offset = this->input->at() - sizeof(header);

    if (DEBUG)
        cerr << "readFrame at " << offset << endl;
    
    while (true) {
        if (! (input->status() == IO_Ok)) { // probably EOF
            this->slotDisconnect();
            break;
        }

        if (! this->alreadyGotHeader) {
            if (DEBUG)
                cerr << "chunk header at " << this->input->at() << endl;
            readAllBlock(this->input, (char *)&header, sizeof(header));
            if (! (input->status() == IO_Ok)) { // probably EOF
                this->slotDisconnect();
                break;
            }
        }
        if (memcmp(header.magic, ROBOLINK_MAGIC, sizeof(header.magic)) != 0) {
            cout << "Error: robolink magic didn't match at "
                << this->input->at() - sizeof(header)
                << ". I got " << (int)header.magic[0] << ", "
                << (int)header.magic[1] << ", " << (int)header.magic[2]
                << ", " << (int)header.magic[3] << endl;
            frameid = -1;
            break;

#if 0
            while (true) { // keep looking to reaquire header
                readAllBlock(this->input, (char*)&header.magic,
                        sizeof(header.magic));
                cout << ".";
                if (! (input->status() == IO_Ok)) {
                    break;
                }
                if (memcmp(header.magic, ROBOLINK_MAGIC, sizeof(header.magic))
                        == 0) {
                    readAllBlock(this->input,
                                (char*)(&header + sizeof(header.magic)),
                                sizeof(header) - sizeof(header.magic));
                }
            }   
#endif
        }
        if (! (input->status() == IO_Ok)) {
            cout << "File not good" << endl;
            this->slotDisconnect();
            break;
        }
        if (frameid == -1) {
            frameid = header.frame_num;
            this->lblFrameNo->setText(QString("%1").arg(header.frame_num));
        }
        if (header.frame_num != frameid) { // found data from a new frame
            if (this->online) {
                this->alreadyGotHeader = true;
            } else {
                this->input->at(this->input->at() - sizeof(header)); // rewind
            }
            if (DEBUG)
                cerr << "Next frame is " << header.frame_num << " at "
                    << this->input->at() << endl;
            break;
        } else {
            this->alreadyGotHeader = false; // so we read header on next chunk
        }
        if (DEBUG)
            cerr << "Found data type " << header.data_type << " length "
                << header.data_len << " for frame " << frameid << endl;

        if (header.data_type == RLNK_YUVPLANE) {
            foundYUV = true;
            parseYUV(false);
        } else if (header.data_type == RLNK_PWALK) {
            parsePWalkInfo(false);
        } else if (header.data_type == RLNK_CAM_ORIENT) {
            parseCamOrient(false);
        } else if (header.data_type == RLNK_SUBVISION) {
            foundSubvision = true;
            parseFeatures(false, header.data_len);
        } else if (header.data_type == RLNK_SUBOBJECT) {
            foundSubvision = true;
            parseObjects(false);
        } else if (header.data_type == RLNK_INSANITY) {
            foundSubvision = true;
            parseInsanities(false);
        } else { // skip other data
            cout << "Skipping unrecognised data" << endl;
            this->input->at(this->input->at() + header.data_len);
        }
        if (! (input->status() == IO_Ok)) {
            this->slotDisconnect();
            cout << "File not good" << endl;
            frameid = -1;
           break;
        }

    }
    this->input->resetStatus(); // clear EOF flag

    if (this->frames.empty() ||
            (frameid != -1 && offset > this->frames.back())) {
        this->frames.push_back(offset);
    }
    
    // The image is sampled offline if we got a YUV image and didn't get
    // the processed subvision data
    this->shouldSampleImage = (frameid != -1 && foundYUV && ! foundSubvision);

    if (DEBUG)
        cerr << "finished frame id " << frameid << endl;
    return frameid;
}

/* Parses the YUV stream at this->input and loads the image data. If hdr is
 * true a robolink header should be parsed first. Returns a new QImage of the
 * image data.
 */
void FormSubvisionImpl::parseYUV(bool hdr) {
    if (DEBUG)
        cerr << "parseYUV (" << hdr << ") at " << this->input->at() 
            << ". image = " << (void*)this->image << endl;
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseYUV: input bad or not open at " << this->input->at()
            << endl;
        return;
    }

    if (hdr) {
        robolink_header header;
        readAllBlock(this->input, (char*)&header, sizeof(robolink_header));
        if (! (input->status() == IO_Ok)) {
            cout << "ParseYUV: Input error " << input->status() << endl;
            return;
        }
    }

    char* ptr = reinterpret_cast<char*>(this->yuv);
    for (int row = 0; row < IMAGE_ROWS; ++row) {
        readAllBlock(this->input, ptr, IMAGE_COLS); // y1
        readAllBlock(this->input, ptr + IMAGE_COLS, IMAGE_COLS); // u
        readAllBlock(this->input, ptr + 2*IMAGE_COLS, IMAGE_COLS); // v
        readAllBlock(this->input, ptr + 3*IMAGE_COLS, IMAGE_COLS); // y2
        readAllBlock(this->input, ptr + 4*IMAGE_COLS, IMAGE_COLS); // y3
        readAllBlock(this->input, ptr + 5*IMAGE_COLS, IMAGE_COLS); // y4
        if (! (input->status() == IO_Ok)) {
            cout << "ParseYUV: Input error " << input->status() << endl;
            this->slotDisconnect();
            return;
        }
//        for (int col = 0; col < IMAGE_COLS; ++col) {
//            QRgb pixel = yuv2rgb(yplane[row * ROW_SKIP + col], 
//                                uplane[row * ROW_SKIP + col],
//                                vplane[row * ROW_SKIP + col]);
//            image->setPixel(col, row, pixel);
//        }
        ptr += 6 * IMAGE_COLS;
    }
    if (DEBUG)
        cerr << "Done parsing YUV" << endl;
    return;
}

/* Reads and stores PWalkInfo from this->input, which must be positioned at or
 * after the header to the rlnk_pwalkinfo chunk. When finished the
 * stream will point to the header of the next chunk. If header is true, the
 * header should be parsed first, else it has already been read.
 */
void FormSubvisionImpl::parsePWalkInfo(bool hdr) {
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parsePWalkInfo: input bad or not open at " << this->input->at()
            << endl;
        return;
    }
    if (DEBUG)
        cerr << "parsing rlnk_pwalkinfo from " << input->at() << endl;

    // skip header
    if (hdr)
        this->input->at(this->input->at() + sizeof(robolink_header));

    readAllBlock(this->input, (char *) &this->pWalkInfo,
                    sizeof(this->pWalkInfo));
    if (! (input->status() == IO_Ok)) {
        cout << "parsePWalkInfo: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    if (DEBUG) {
        cerr << "PWalkInfo parsed, " << sizeof(this->pWalkInfo) << " bytes."
            << " frontShoulderHeight: " << this->pWalkInfo.frontShoulderHeight
            << " bodyTilt: " << this->pWalkInfo.bodyTilt
            << " desiredPan: " << this->pWalkInfo.desiredPan
            << " desiredTilt: " << this->pWalkInfo.desiredTilt
            << " desiredCrane: " << this->pWalkInfo.desiredCrane << endl;
    }

}

/* Reads and stores camera orientation data from this->input, which must be
 * positioned at or after the header to the cam_orient chunk. When finished the
 * stream will point to the header of the next chunk. If header is true, the
 * header should be parsed first, else it has already been read.
 */
void FormSubvisionImpl::parseCamOrient(bool hdr) {
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseCamOrient: input bad or not open at " << this->input->at()
            << endl;
        return;
    }
    if (DEBUG)
        cerr << "parsing cam_orient from " << input->at() << endl;

    // skip header
    if (hdr)
        this->input->at(this->input->at() + sizeof(robolink_header));

    readAllBlock(this->input, (char *) &this->camorient,
                    sizeof(this->camorient));
    if (! (input->status() == IO_Ok)) {
        cout << "ParseCamOrient: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    if (DEBUG) {
        cerr << "camorient " << sizeof(this->camorient) << " bytes "
            << "hz_exists: " << this->camorient.hz_exists
            << ", hz_up_is_sky: " << this->camorient.hz_up_is_sky << endl;
        cerr << "hz_intercept: " << this->camorient.hz_intercept
            << ", hz_gradient: " << this->camorient.hz_gradient << endl;
    }
}

/* Reads and stores a list of VisualFeatures from this->input. If hdr is
 * true then a robolink header should be parsed first, else size contains
 * the length of data to parse.
 */
void FormSubvisionImpl::parseFeatures(bool hdr, int size) {
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseFeatures: input bad or not open at " << this->input->at()
            << endl;
    }

    if (hdr) {
        robolink_header header;
        readAllBlock(this->input, (char*)&header, sizeof(robolink_header));
        if (! (input->status() == IO_Ok)) {
            cout << "ParseFeatures: Input error " << input->status() << endl;
            this->slotDisconnect();
            return;
        }
        size = header.data_len;
    }
    int numFeatures = size/sizeof(VisualFeature);
    if (DEBUG)
        cerr << "parseFeatures " << size << " bytes = "
            << numFeatures << " features at "
            << sizeof(VisualFeature) << " bytes each" << endl;

    VisualFeature f;
    int i;
    for (i = 0; i < numFeatures; ++i) {
        readAllBlock(this->input, (byte*)&f, sizeof(f));
        this->features.push_back(f);
    }
    if (! (input->status() == IO_Ok)) {
        cout << "ParseFeatures: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    if (DEBUG)
        cerr << "parsed " << i << " features" << endl;
}

/* Reads and stores a list of VisualObjects from this->input. If hdr is
 * true then a robolink header should be parsed first.
 */
void FormSubvisionImpl::parseObjects(bool hdr) {
    if (DEBUG)
        cerr << "parseObjects " << 7*sizeof(VisualObject) << " bytes at "
            << sizeof(VisualObject) << " bytes each of 7 vobs" << endl;
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseObjects: input bad or not open at " << this->input->at()
            << endl;
    }

    if (hdr)
        this->input->at(this->input->at() + sizeof(robolink_header));
   
    readAllBlock(this->input, (byte*)&this->vBall, sizeof(VisualObject));
    readAllBlock(this->input, (byte*)&this->vBeacons[svBlueOnPink],
            sizeof(VisualObject));
    readAllBlock(this->input, (byte*)&this->vBeacons[svPinkOnBlue],
            sizeof(VisualObject));
    readAllBlock(this->input, (byte*)&this->vBeacons[svYellowOnPink],
            sizeof(VisualObject));
    readAllBlock(this->input, (byte*)&this->vBeacons[svPinkOnYellow],
            sizeof(VisualObject));
    readAllBlock(this->input, (byte*)&this->vGoals[svBlueGoal],
            sizeof(VisualObject));
    readAllBlock(this->input, (byte*)&this->vGoals[svYellowGoal],
            sizeof(VisualObject));
    if (! (input->status() == IO_Ok)) {
        cout << "ParseObjects: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    if (DEBUG)
        cerr << "Done parsing objects" << endl;
}

/* Parses insanity data and stores it */
void FormSubvisionImpl::parseInsanities(bool hdr) {
    if (DEBUG)
        cerr << "parseInsanities " <<  (INSANITY_COUNT/8 + 1)
            << " bytes" << endl;
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseInsanities: input bad or not open at " << this->input->at()
            << endl;
    }

    if (hdr)
        this->input->at(this->input->at() + sizeof(robolink_header));
   
    readAllBlock(this->input, (byte*)&this->insanities, INSANITY_COUNT/8 + 1);

    if (! (input->status() == IO_Ok)) {
        cout << "ParseInsanities: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    if (DEBUG)
        cerr << "Done parsing insanities" << endl;
    return;
}

/* Resamples the image with the current settings and copies the features and
 * objects into this forms features and objects */
void FormSubvisionImpl::sampleImage(void)
{
    if (this->pixmap)
        delete this->pixmap;
    this->pixmap = new QPixmap(this->image->scale(this->pixmapImage->width(),
                                                this->pixmapImage->height()));

    if (DEBUG)
        cout << "Sampling image..." << endl;
//    this->subvision.sampleImage(this->yuv, this->camorient.hz_intercept,
//                    this->camorient.hz_gradient, !this->camorient.hz_up_is_sky,
//                    this->camorient.hz_exists, this->camorient.hz_below_image);
//    this->subobject.findVisualObjects();
    struct PWalkInfoStruct pwalkinfo = {
            &this->pWalkInfo.leading_leg,
            &this->pWalkInfo.currentWalkType,
            &this->pWalkInfo.currentStep,
            &this->pWalkInfo.currentPG,
            &this->pWalkInfo.frontShoulderHeight,
            &this->pWalkInfo.bodyTilt,
            &this->pWalkInfo.desiredPan,
            &this->pWalkInfo.desiredTilt,
            &this->pWalkInfo.desiredCrane,
            0,   // isHighGain not used in VisualCortex
    };
    this->cortex.processImage(this->yuv, this->pWalkInfo.utilt,
                                this->pWalkInfo.upan, this->pWalkInfo.ucrane,
                                this->pWalkInfo.range2Obstacle, &pwalkinfo);

    // Copy information to this object for display. We copy rather than access
    // it directly from the display code so that we can also display data that
    // we read from log and haven't calculated.
    this->camorient.hz_intercept = cortex.hz_c;
    this->camorient.hz_gradient = cortex.hz_m;
    this->camorient.hz_exists = cortex.hzExists;
    this->camorient.hz_up_is_sky = cortex.hzUpIsSky;
    this->camorient.hz_below_image = cortex.hzAllAboveHorizon;
    this->features = this->subvision.features;
    this->vBall = this->cortex.vob[vobBall];
    this->vBeacons[svPinkOnBlue] = this->cortex.vob[vobPinkBlueBeacon];
    this->vBeacons[svBlueOnPink] = this->cortex.vob[vobBluePinkBeacon];
    this->vBeacons[svPinkOnYellow] = this->cortex.vob[vobPinkYellowBeacon];
    this->vBeacons[svYellowOnPink] = this->cortex.vob[vobYellowPinkBeacon];
    this->vGoals[svBlueGoal] = this->cortex.vob[vobBlueGoal];
    this->vGoals[svYellowGoal] = this->cortex.vob[vobYellowGoal];
    memcpy(this->insanities, SanityChecks::fired, INSANITY_COUNT/8 + 1);
}

/* Clears feature data read from file/network or created through sampled image
 */
void FormSubvisionImpl::clearData() {
    this->features.clear();
    this->vBall.cf = 0;
    this->vBeacons[svPinkOnBlue].cf = 0;
    this->vBeacons[svBlueOnPink].cf = 0;
    this->vBeacons[svPinkOnYellow].cf = 0;
    this->vBeacons[svYellowOnPink].cf = 0;
    this->vGoals[svBlueGoal].cf = 0;
    this->vGoals[svYellowGoal].cf = 0;
    this->camorient.hz_intercept = 0;
    this->camorient.hz_gradient = 0;
    this->camorient.hz_exists = true;
    this->camorient.hz_up_is_sky = true;
    this->camorient.hz_below_image = false;
    memset(this->insanities, 0, INSANITY_COUNT/8 + 1);
    this->image->fill(Qt::darkGray.pixel());
}


/* Displays the background RGB image, CPlane or plain background */
void FormSubvisionImpl::displayImage(void) {
    if (this->rbImageYUV->isChecked()) {
        if (this->pixmap)
            delete this->pixmap;
        for (int row = 0; row < IMAGE_ROWS; ++row) {
            for (int col = 0; col < IMAGE_COLS; ++col) {
                QRgb pixel = yuv2rgb(yplane[row * ROW_SKIP + col], 
                                    uplane[row * ROW_SKIP + col],
                                    vplane[row * ROW_SKIP + col]);
                image->setPixel(col, row, pixel);
            }
        }
        this->pixmap = new QPixmap(this->image->scale(
                                            this->pixmapImage->width(),
                                            this->pixmapImage->height()));
    } else if (this->rbImageCPlane->isChecked()) { // FIXME: BROKEN
        if (DEBUG)
            cerr << "displaying CPlane...";
        uchar* cplane = this->subvision.gcd.cplane;
        cerr << "cplane = " <<  (void*) cplane << endl;
        QImage* image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
        for (int y = 0; y < IMAGE_ROWS; ++y) {
            for (int x = 0; x < IMAGE_COLS; ++x) {
                cerr << (int)cplane[y*IMAGE_COLS + x] << " ";
                image->setPixel(x, y,
                        QColor(CPLANE_COLOURS[cplane[y*IMAGE_COLS + x]]).rgb());
            }
        }
        if (this->pixmap)
            delete this->pixmap;
        this->pixmap = new QPixmap(image->scale(this->pixmapImage->width(),
                                                this->pixmapImage->height()));
        delete image;
    } else {
        if (! this->pixmap)
            this->pixmap = new QPixmap(image->scale(this->pixmapImage->width(),
                                                this->pixmapImage->height()));
        this->pixmap->fill(Qt::darkGray);
    }
}

/* Draws the slice line over the image pixmap. Origin is top left corner. */
void FormSubvisionImpl::drawSlice() {
    if (DEBUG)
        cerr << "drawslice" << endl;
    // convert coords to display resolution
    if (! this->checkBoxDisplaySlice->isChecked())
        return;
    if (this->pixmap == 0)
        return;
    if (histPoint1 == histPoint2)
        return;

    QPainter p(this->pixmap);
    p.setPen(QPen(QColor(SLICE_COLOUR), 1));
    drawLine(p, histPoint1, histPoint2, 2);
}


/* Grabs the scan lines and horizon from Subvision (after sampling the image)
 * and displays them on the pixmap
 */
void FormSubvisionImpl::displayScanlines(void) {
    if (! checkBoxDisplayScanlines->isChecked())
        return;
    if (DEBUG)
        cerr << "displayScanLines" << endl;
    // draw scan lines
    QPainter p(this->pixmap);
    p.setPen(QPen(QColor(SCANLINE_COLOUR), 1));
    vector<pair<point, point> >::iterator itr;
    for (   itr=subvision.scanlines.begin();
            itr != subvision.scanlines.end();
            ++itr) {
//        if (DEBUG)
//            cerr << "scanline: " << itr->first.first << ", "
//                << itr->first.second << " - " << itr->second.first << ", "
//                << itr->second.second << endl;
        // NOTE: rather than use QT line drawing we use our own to make
        // sure the same lines get drawn as processed
        drawLine(p, itr->first, itr->second, 2);
    }
    for (   itr=subvision.horzScanlines.begin();
            itr != subvision.horzScanlines.end();
            ++itr) {
//        if (DEBUG)
//            cerr << "horzscanline: " << itr->first.first << ", "
//                << itr->first.second << " - " << itr->second.first << ", "
//                << itr->second.second << endl;
        drawLine(p, itr->first, itr->second, 2);
    }
    // draw horizon
    pair<point, point> horizon = make_pair(subvision.horizonPoint,
                                            subvision.otherHorizonPoint);
    if (horizon.first != horizon.second) {
        if (DEBUG)
            cerr << "drawing horizon from " << horizon.first.first << ","
                << horizon.first.second << " to " << horizon.second.first << ","
                << horizon.second.second << endl;
        p.setPen(QPen(QColor(HORIZON_COLOUR), 1));
        drawLine(p, horizon.first, horizon.second, 2);
    }

}

/* Grab feature points from Subvision and display them on the pixmap */
void FormSubvisionImpl::displayFeatures() {
    if (DEBUG)
        cerr << "displayFeatures" << endl;
    if (! checkBoxDisplayFeatures->isChecked())
        return;
    QPainter p(this->pixmap);

    vector<VisualFeature>::iterator itr = this->features.begin();
    for (; itr != this->features.end(); ++itr) {
//        if (DEBUG)
//            cerr << "Feature at " << itr->x << ", " << itr->y << endl;
        if (itr->type == VF_UNKNOWN)
            continue;
        else if (itr->type == VF_PINK || itr->type == VF_BLUE
                || itr->type == VF_YELLOW) {
            p.setPen(QPen(VF_COLOURS[itr->type], 3));
            drawLine(p, make_pair((int)itr->x, (int)itr->y),
                        make_pair((int)itr->endx, (int)itr->endy), 2);
        } else {
            p.setPen(QPen(VF_COLOURS[itr->type], 1));
            p.drawRect((int)itr->x * 2, (int)itr->y * 2, 2, 2);
        }
    }
}

/* Display recognised object information from SubObject */
void FormSubvisionImpl::displayObjects() {
    if (DEBUG)
        cerr << "displayObjects" << endl;
    if (! checkBoxDisplayObjects->isChecked())
        return;
    QPainter p(this->pixmap);

    // ball
    p.setPen(QPen(OBJ_COLOUR_BALL, 1));
    if (this->vBall.cf > 0) {
        if (DEBUG)
            cerr << "ball at " << this->vBall.cx << ", "
                << this->vBall.cy << " radius "
                << this->vBall.radius << endl;
        int radius = (int)this->vBall.radius;
        int x = (int)this->vBall.cx - radius;
        int y = (int)this->vBall.cy - radius;
        p.drawEllipse(x * 2, y * 2, radius * 4, radius * 4);
    }


    // beacons
    double horizonAngle = atan(this->camorient.hz_gradient);
//    int flip = subvision.getHorizonUpsideDown() ? -1 : 1;
    point pt;
    QPointArray rect(4);
    for (int i = 0; i < NUM_BEACONS; ++i) {
        VisualObject& b = vBeacons[i];
        if (b.cf > 0) {
            if (DEBUG)
                cerr << "beacon at " << b.cx << ","
                    << b.cy << endl;
            int cx = (int)b.cx;
            int cy = (int)b.cy;
            int x = (int)b.x;
            int y = (int)b.y;
            int width = (int)b.width;
            int height = (int)b.height;
            int size = height/4;
            // draw centroid (between colours)
            p.setPen(QPen(OBJ_COLOUR_BEACONPINK, 1));
            p.drawRect(cx * 2, cy * 2, 2, 2); // pink centre
            // draw top box
            pt.first = 2*x, pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(0, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(1, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*(y+size);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(2, pt.first, pt.second);
            pt.first = 2*x, pt.second = 2*(y+size);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(3, pt.first, pt.second);
            p.setPen(QPen(beaconColours[i][0], 1));
            p.drawPolygon(rect);
            // bottom box
            y += size;
            pt.first = 2*x, pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(0, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(1, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*(y+size);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(2, pt.first, pt.second);
            pt.first = 2*x, pt.second = 2*(y+size);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(3, pt.first, pt.second);
            p.setPen(QPen(beaconColours[i][1], 1));
            p.drawPolygon(rect);
            // base
            p.setPen(QPen("white", 1));
            y += size;
            pt.first = 2*x, pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(0, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(1, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*(y+size+size);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(2, pt.first, pt.second);
            pt.first = 2*x, pt.second = 2*(y+size+size);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(3, pt.first, pt.second);
            p.drawPolygon(rect);
        }
    }

    // goals
    for (int i = 0; i < NUM_GOALS; ++i) {
        VisualObject& g = vGoals[i];
        if (g.cf > 0) {
            if (DEBUG)
                cerr << "goal at " << g.cx << ","
                    << g.cy << endl;
            int cx = (int)g.cx;
            int cy = (int)g.cy;
            int x = (int)g.x;
            int y = (int)g.y;
            int width = (int)g.width;
            int height = (int)g.height;
            // draw centroid (between colours)
            p.setPen(QPen(OBJ_COLOUR_BEACONPINK, 1));
            p.drawRect(cx * 2, cy * 2, 2, 2); // pink centre
            // draw box
            p.setPen(QPen(goalColours[i], 1));
            pt.first = 2*x, pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(0, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*y;
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(1, pt.first, pt.second);
            pt.first = 2*(x+width), pt.second = 2*(y+height);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(2, pt.first, pt.second);
            pt.first = 2*x, pt.second = 2*(y+height);
            pt = rotatePoint(pt.first, pt.second, 2*cx, 2*cy, -horizonAngle);
            rect.setPoint(3, pt.first, pt.second);
            p.drawPolygon(rect);
        }
    }

    //if (this->tabDisplayInfo->currentPage()->name() != QString("tabObjects"));
    //    return;
    // Also fill in the info in the object information panel
    // Ball
    ostringstream str;
    str.precision(3);
    str << this->vBall.cx << ", " << this->vBall.cy;
    this->lblBallCentroid->setText(str.str());
    str.str("");
    str << this->vBall.x << ", " << this->vBall.y << "; " << this->vBall.width
        << ", " << this->vBall.height;
    this->lblBallRect->setText(str.str());
    str.str("");
    str << this->vBall.radius;
    this->lblBallRadius->setText(str.str());
    str.str("");
    str << this->vBall.imgHead;
    this->lblBallImgHead->setText(str.str());
    str.str("");
    str << this->vBall.imgElev;
    this->lblBallImgElev->setText(str.str());
    str.str("");
    str << this->vBall.dist2;
    this->lblBallDist2->setText(str.str());
    str.str("");
    str << this->vBall.dist3;
    this->lblBallDist3->setText(str.str());
    str.str("");
    str << this->vBall.head;
    this->lblBallHead->setText(str.str());
    str.str("");
    str << this->vBall.elev;
    this->lblBallElev->setText(str.str());
    str.str("");
    str << this->vBall.cf;
    this->lblBallCf->setText(str.str());
    str.str("");
    str << this->vBall.var;
    this->lblBallVar->setText(str.str());
    str.str("");
    str << this->vBall.angleVar;
    this->lblBallAngleVar->setText(str.str());
    str.str("");
    str << this->vBall.objectPosition[0] << ", "
        << this->vBall.objectPosition[2] << ", "
        << this->vBall.objectPosition[1];
    this->lblBallPosition->setText(str.str());
    str.str("");

    // beacon info
    QLabel* const bCentroid[NUM_BEACONS] =
        {this->lblBluePinkCentroid, this->lblPinkBlueCentroid,
        this->lblYellowPinkCentroid, this->lblPinkYellowCentroid};
    QLabel* const bDist[NUM_BEACONS] =
        {this->lblBluePinkDist, this->lblPinkBlueDist,
        this->lblYellowPinkDist, this->lblPinkYellowDist};
    QLabel* const bHead[NUM_BEACONS] =
        {this->lblBluePinkHead, this->lblPinkBlueHead,
        this->lblYellowPinkHead, this->lblPinkYellowHead};
    QLabel* const bElev[NUM_BEACONS] =
        {this->lblBluePinkElev, this->lblPinkBlueElev,
        this->lblYellowPinkElev, this->lblPinkYellowElev};
    for (int i = 0; i < NUM_BEACONS; ++i) {
        str << this->vBeacons[i].cx << ", "
            << this->vBeacons[i].cy;
        bCentroid[i]->setText(str.str());
        str.str("");
        str << this->vBeacons[i].dist2;
        bDist[i]->setText(str.str());
        str.str("");
        str << this->vBeacons[i].head;
        bHead[i]->setText(str.str());
        str.str("");
        str << this->vBeacons[i].elev;
        bElev[i]->setText(str.str());
        str.str("");
    }
    
    // goal info
    QLabel* const gCentroid[NUM_BEACONS] =
        {this->lblBlueGoalCentroid, this->lblYellowGoalCentroid};
    QLabel* const gDist[NUM_BEACONS] =
        {this->lblBlueGoalDist, this->lblYellowGoalDist};
    QLabel* const gHead[NUM_BEACONS] =
        {this->lblBlueGoalHead, this->lblYellowGoalHead};
    QLabel* const gElev[NUM_BEACONS] =
        {this->lblBlueGoalElev, this->lblYellowGoalElev};
    for (int i = 0; i < NUM_GOALS; ++i) {
        str << this->vGoals[i].cx << ", "
            << this->vGoals[i].cy;
        gCentroid[i]->setText(str.str());
        str.str("");
        str << this->vGoals[i].dist2;
        gDist[i]->setText(str.str());
        str.str("");
        str << this->vGoals[i].head;
        gHead[i]->setText(str.str());
        str.str("");
        str << this->vGoals[i].elev;
        gElev[i]->setText(str.str());
        str.str("");
    }
    
    // and insanities
    for (int i = 0; i < INSANITY_COUNT; ++i) {
        if (insanities[i/8] & (1 << (i%8))) {
            str << insanityNames[i] << endl;
        }
    }
    this->textInsanities->setText(str.str());
}

/* Calculates the Histogram values for the given slice.
 * Origin is top left corner
 */
void FormSubvisionImpl::calculateHistogram() {
//    cerr << "calculateHistogram" << endl;
    if (! checkBoxDisplayHistogram->isChecked())
        return;

    int histMax = pixmapLevels->height() - 1;

    // Pixmap for YUV levels
    QPixmap levelPixmap(pixmapLevels->width(),
                        pixmapLevels->height());
    levelPixmap.fill(); // white
    // Pixmap for YUV gradients (change per pixel)
    QPixmap gradPixmap(pixmapGradients->width(),
                        pixmapGradients->height());
    gradPixmap.fill(); // white
    // Pixmap for gradient sum (sum of absolute value of gradient)
    QPixmap sumPixmap(pixmapGradientSum->width(),
                        pixmapGradientSum->height());
    sumPixmap.fill(); // white

    // Paint grid lines on gradient graphs
    QPainter pGrid;
    pGrid.begin(&levelPixmap);
    pGrid.setPen(QPen(QColor(GRID_COLOUR), 1));
    for (int y = GRID_SPACE; y < levelPixmap.height(); y += GRID_SPACE) {
        pGrid.drawLine(0, levelPixmap.height() - y,
                        levelPixmap.width(), levelPixmap.height() - y);
    }
    pGrid.end();
    pGrid.begin(&gradPixmap);
    pGrid.setPen(QPen(QColor(GRID_COLOUR), 1));
    pGrid.drawLine(0, histMax/2, gradPixmap.width(), histMax/2);
    for (int y = GRID_SPACE; y < gradPixmap.height()/2; y += GRID_SPACE) {
        pGrid.drawLine(0, histMax/2 + y, gradPixmap.width(), histMax/2 + y);
        pGrid.drawLine(0, histMax/2 - y, gradPixmap.width(), histMax/2 - y);
    }
    pGrid.end();
    pGrid.begin(&sumPixmap);
    pGrid.setPen(QPen(QColor(GRID_COLOUR), 1));
    for (int y = GRID_SPACE; y < sumPixmap.height(); y += GRID_SPACE) {
        pGrid.drawLine(0, sumPixmap.height() - y,
                        sumPixmap.width(), sumPixmap.height() - y);
    }
    pGrid.end();

    QPainter py; // Painter for Y level
    py.begin(&levelPixmap);
    py.setPen(QPen(QColor("black"), 1));
    QPainter pu; // Painter for U level
    pu.begin(&levelPixmap);
    pu.setPen(QPen(QColor("magenta"), 1));
    QPainter pv; // Painter for V level
    pv.begin(&levelPixmap);
    pv.setPen(QPen(QColor("cyan"), 1));
    QPainter pdy; // Painter for Y gradient
    pdy.begin(&gradPixmap);
    pdy.setPen(QPen(QColor("black"), 1));
    QPainter pdu; // Painter for U gradient
    pdu.begin(&gradPixmap);
    pdu.setPen(QPen(QColor("magenta"), 1));
    QPainter pdv; // Painter for V gradient
    pdv.begin(&gradPixmap);
    pdv.setPen(QPen(QColor("cyan"), 1));
    QPainter psum; // Painter for sum of gradients
    psum.begin(&sumPixmap);
    psum.setPen(QPen(QColor("black"), 1));

    point pt = histPoint1;
    
    // The line drawing algorithm chooses which pixels to process
    // by "drawing" the slice
    double histScale = histMax / double(CHANNEL_MAX);    // axis scale factors
    int prevPoint[] = {0,
        int(histMax - this->yplane[pt.second * ROW_SKIP + pt.first]*histScale),
        int(histMax - this->uplane[pt.second * ROW_SKIP + pt.first]*histScale),
        int(histMax - this->vplane[pt.second * ROW_SKIP + pt.first]*histScale),
        histMax/2, histMax/2, histMax/2,
        histMax};
    py.drawPoint(prevPoint[0], prevPoint[1]);
    pu.drawPoint(prevPoint[0], prevPoint[2]);
    pv.drawPoint(prevPoint[0], prevPoint[3]);
    pdy.drawPoint(prevPoint[0], prevPoint[4]);
    pdu.drawPoint(prevPoint[0], prevPoint[5] + DU_OFFSET);
    pdv.drawPoint(prevPoint[0], prevPoint[6] + DV_OFFSET);
    psum.drawPoint(prevPoint[0], prevPoint[7]);

    LineDraw::startLine(histPoint1, histPoint2);
    while ((pt = LineDraw::nextPoint()) != histPoint2) {
        int newX = prevPoint[0] + 2;
        int newY = int(histMax - this->yplane[pt.second * ROW_SKIP + pt.first]
                        * histScale);
        int newU = int(histMax - this->uplane[pt.second * ROW_SKIP + pt.first]
                        * histScale);
        int newV = int(histMax - this->vplane[pt.second * ROW_SKIP + pt.first]
                        * histScale);
        int newdY = newY - prevPoint[1] + (histMax/2);
        int newdU = newU - prevPoint[2] + (histMax/2);
        int newdV = newV - prevPoint[3] + (histMax/2);
        int newSum = histMax - (abs(newY - prevPoint[1])
                + abs(newU - prevPoint[2]) + abs(newV - prevPoint[3]));
        py.drawLine(prevPoint[0], prevPoint[1], newX, newY);
        pu.drawPoint(newX, newU);
        pv.drawPoint(newX, newV);
        pdy.drawLine(prevPoint[0], prevPoint[4], newX, newdY);
        pdu.drawLine(prevPoint[0], prevPoint[5] + DU_OFFSET,
                        newX, newdU + DU_OFFSET);
        pdv.drawLine(prevPoint[0], prevPoint[6] + DV_OFFSET,
                        newX, newdV + DV_OFFSET);
        psum.drawLine(prevPoint[0], prevPoint[7], newX, newSum);
        prevPoint[0] = newX;
        prevPoint[1] = newY;
        prevPoint[2] = newU;
        prevPoint[3] = newV;
        prevPoint[4] = newdY;
        prevPoint[5] = newdU;
        prevPoint[6] = newdV;
        prevPoint[7] = newSum;
    }

    if (DEBUG)
        cerr << "Finishing drawing histograms" << endl;
    py.end();
    pu.end();
    pv.end();
    pdy.end();
    pdu.end();
    pdv.end();
    psum.end();
    pixmapLevels->setPixmap(levelPixmap);
    pixmapGradients->setPixmap(gradPixmap);
    pixmapGradientSum->setPixmap(sumPixmap);
}

/***** Helpers *****/

/* Reads exactly size bytes from the input stream into the data buffer. This
 * loops over QIODevice::readBlock which may read less that the requested
 * amount of data.
 */
static Q_LONG readAllBlock(QIODevice* in, char* data, Q_ULONG size) {
    Q_LONG ret;
    Q_ULONG count = 0;
    while (count < size) {
        ret = in->readBlock(data + count, size - count);
        if (ret < 0) {
            cerr << "Error in readBlock. State: " << in->state() << endl;
            return -1;
        } else if (ret == 0) {
            cerr << "EOF in readBlock" << endl;
            return count;
        }
        count += ret;
    }
//    if (DEBUG)
//        cerr << "readAllBlock got " << count << " bytes" << endl;
    return count;
}

/* Rather than use QT drawLine we do it manually to ensure that the line we 
 * draw is the same as the line that was processed
 */
void drawLine(QPainter& paint, point start, point finish, int scale) {
    if (start == finish) {
        cerr << "drawLine " << start.first << "," << start.second
            << " to " << finish.first << "," << finish.second << endl;
        return;
    }
    point a, b;
    a.first = start.first * scale;
    a.second = start.second * scale;
    b.first = finish.first * scale;
    b.second = finish.second * scale;

    LineDraw::startLine(a, b);
    point p;
    while ((p = LineDraw::nextPoint()) != b) {
        paint.drawPoint(p.first, p.second);
    }
}

/* Converts a YUV pixel spec to RGB */
QRgb yuv2rgb(int y, int u, int v) {
    u -= 128; v -= 128;
    int r = (int)((1.164*y) + (1.596*u));
    int g = (int)((1.164*y) - (0.813*u) - (0.391*v));
    int b = (int)((1.164*y) + (1.596*v));
    
    if (r < 0) r = 0;       if (g < 0) g = 0;       if (b < 0) b = 0;
    if (r > 255) r = 255;   if (g > 255) g = 255;   if (b > 255) b = 255;
    
    return qRgb(r, g, b);
}
