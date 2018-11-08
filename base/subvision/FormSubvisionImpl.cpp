/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or (at your option) any later version as  
   modified below.  As the original licensors, we add the following  
   conditions to that license:

   In paragraph 2.b), the phrase "distribute or publish" should be  
   interpreted to include entry into a competition, and hence the source  
   of any derived work entered into a competition must be made available  
   to all parties involved in that competition under the terms of this  
   license.

   In addition, if the authors of a derived work publish any conference  
   proceedings, journal articles or other academic papers describing that  
   derived work, then appropriate academic citations to the original work  
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/* FormSubvisionImpl.cpp
 * Implementation of FormSubvision
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qfont.h>
#include <qpainter.h>
#include <qsocketdevice.h>
#include <qgroupbox.h>

#include "../../robot/share/Common.h"
#include "FormSubvisionImpl.h"
#include "../robolink/robolink.h"
#include "../../robot/vision/Profile.h"

#define ANDREWDEBUG 0
#define DEBUG 0

extern const char *walkTypeToStr[]; //is defined in CommonData.cc
extern const char *commandToStr[];

static const char* vfInsanityNames[] = {
    "VF_SANE",
    "VF_BELOW_SUM_THRESH",
    "VF_BELOW_BALL_DU_THRESH",
    "VF_DV_DU_OPPOSITE",
    "VF_ABOVE_SPECULAR_THRESH",
    "VF_NOT_MUCH_BALL_FEATURE",
    "VF_LOOKS_LIKE_RED_ROBOT",
    "VF_LOOKS_LIKE_PINK_BEACON"};


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
                *GRID_COLOUR = "lightGray",
                *BESTGAP_COLOUR = "magenta";

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

//static const char* OBJ_COLOUR_WALL = "purple";
static const char* OBJ_COLOUR_BALL = "yellow";
static const char* OBJ_COLOUR_BEACONPINK = "pink";
static const char* OBJ_COLOUR_BEACONBLUE = "blue";
static const char* OBJ_COLOUR_BEACONYELLOW = "yellow";
static const char* OBJ_COLOUR_GAP = "white";
static const char* OBJ_COLOUR_WALL = "purple";

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

static const char* CPLANE_COLOURS[cNONE + 1] = {
	"orange",
	"lightBlue",
	"green",
	"yellow",
	"magenta",
	"darkBlue",
	"darkRed",
	"green",
	"lightGray",         // robot gray
	"white",
	"black",
	"white",       // marker, not a real colour
	"white",     // ditto
	"gray", // end of colours
    // there is also cNONE = 127 which is set later on
};

static Q_LONG readAllBlock(QIODevice* in, char* data, Q_ULONG size);
static QRgb yuv2rgb(int y, int u, int v);
static void drawLine(QPainter& paint, point start, point finish, int scale = 1);
static double calculateMean(list<double> data);
static double calculateVariance(list<double> data);

FormSubvisionImpl::FormSubvisionImpl(QApplication* a, QWidget* parent,
                                        const char* name, WFlags fl) 
            : FormSubvision(parent, name, fl),
                app(a),
                subvision(cortex.subvision),
                subobject(cortex.subobject),
                framereader(this),
                input(0), sock(0), pixmap(0),
                currentFrame(-1) {
    cortex.setDogParams(0); // default shifts
                    
    this->pt2relative[0] = this->pt2relative[1] = INT_MAX;
    this->yuv = new uchar[IMAGE_COLS * IMAGE_ROWS * 6];
    this->yplane = this->yuv;
    this->uplane = this->yplane + IMAGE_COLS;
    this->vplane = this->uplane + IMAGE_COLS;
    this->image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
    CPLANE_COLOURS[cNONE] = CPLANE_COLOURS[cNONE & ~MAYBE_BIT] = "darkGray";

    QPixmap temp(this->pixmapImage->width(), this->pixmapImage->height());
    this->pixmapImage->setPixmap(temp);
    QPixmap temp2(this->pixmapProjected->width(),
                    this->pixmapProjected->height());
    this->pixmapProjected->setPixmap(temp2);

    this->setFocusPolicy(QWidget::StrongFocus);
    QFont f("Bitstream Vera Serif", 8, 0);
    this->setFont( f );
    
    //set getBestGap functin variables to default
    spinObsMinGap->setValue(MIN_GAP_ANGLE);
    spinObsIntensity->setValue(MIN_OBSTACLE);
    
}

FormSubvisionImpl::~FormSubvisionImpl() { 
    if (this->pixmap != NULL)
        delete this->pixmap;
    if (this->yuv != NULL)
        delete[] this->yuv;
}

/* Handles opening of a new file via File->Open menu */
void FormSubvisionImpl::slotFileOpen() {
    //cerr << "slotFileOpen" << endl;
    int ret = -1;
    if (this->input != NULL) {
        if (this->input->isOpen()) 
            this->input->close();
        delete this->input;
        this->input = 0;
    }
    this->online = false;
    this->alreadyGotHeader = false;

    if (this->image != NULL)
        delete this->image;
    this->image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
    this->image->fill(Qt::gray.pixel());
    this->clearData();

    QString filename = QFileDialog::getOpenFileName(
        "../logs",
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
    else 
        this->initRobotCondition();
    this->slotRedraw();
    
    stringstream s;
    s << "Subvision - " << filename;
    QString cs(s.str());
    setCaption(cs);
    slotLoadingFlag();
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
        
        setCaption("Subvision - Connected to Robolink");
    }
}

/* Connects to the specified host and port and starts a thread to read and
 * display the data.
 */
void FormSubvisionImpl::slotConnect(QString host, unsigned short port) {
    if (this->input != NULL) {
        if (this->input->isOpen()) 
            this->input->close();
        delete this->input;
        this->input = NULL;
    }

    if (this->image != NULL) {
        delete this->image;
        this->image = NULL;
    }
    this->image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
    this->image->fill(Qt::gray.pixel());
    this->clearData();

    if (this->sock != NULL) {
        delete this->sock;
        this->sock = NULL;
    }
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

/* Sets the robot MAC address to use for colour correction */
void FormSubvisionImpl::slotSetMacAddress() {
    bool ok = false;
    unsigned long mac = this->textMAC->text().toULong(&ok, 16);
    if (! ok) {
        cerr << "Error parsing MAC address" << endl;
    }
    this->cortex.setDogParams(mac);
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
    this->displayObstacles();
    this->displayGrabTurnKickInfo();
    this->displayGPSInfo();
    this->calculateHistogram();
    bitBlt(this->pixmapImage, 0, 0, this->pixmap);
    
    
    //show pan in  slider
    this->sliderHeadpan->setValue((int)-RAD2DEG(MICRO2RAD(this->pWalkInfo.upan))); 
    this->calibrateEdge();
    
    if (DEBUG)
        cout << "Features: " << this->features.size() << endl;
}

/* Handles advancing to the next frame of cam_orient/yuv data */
void FormSubvisionImpl::slotNextFrame(void) {
    
    btnNextFrame->parentWidget()->parentWidget()->setFocus();
    
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
    else        
        this->initRobotCondition();
            
    this->slotRedraw();
}

/* Handles regressing to the previous frame of cam_orient/yuv data */
void FormSubvisionImpl::slotPrevFrame(void) {
//    cerr << this->frames.size() << " frames." << endl;

    btnPrevFrame->parentWidget()->parentWidget()->setFocus();
    
    if (this->currentFrame <= 0) {
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
    else 
        this->initRobotCondition();        
    this->slotRedraw();
}

/* Handles clicking the play button to play back all frames in a log
 * with appropriate delays to simulate realtime.
 */
void FormSubvisionImpl::slotPlay(void) {
    // FIXME: use a thread
}

/* Handles clicking the playFast button to process all frames in a log
 * without displaying the results, and as fast as possible
 */
void FormSubvisionImpl::slotPlayFast(void) {
    if (tabDisplayInfo->currentPage()->name() == QString("tabGPSInfo"))
    {
        while (btnNextFrame->isEnabled()) 
        {
            slotNextFrame();
        }
    } 
    else if (tabDisplayInfo->currentPage()->name() == QString("tabEdgeCalibration"))
    {
        calculateEdgePointAllFrame();
    }
    else if (tabDisplayInfo->currentPage()->name() == QString("tabObjects"))
    {
        FlagObject errorMap;
        //clear the map
        for (int x = 0; x < VOB_COUNT; ++x)
        {
            errorMap.flags[x] = 0;
        }        
        while (btnNextFrame->isEnabled())
        {
            int isSame = checkFlag(errorMap);
            //if the stop flag is on then stop if flag not the same
            if (CB_OBJ_Warning_if_false_vision->isChecked() && !isSame)
                break;
            slotNextFrame();
        }
        //Writting the error difference
        for (int x = 0; x < VOB_COUNT; ++x)
        {
            cout << vobName[x] <<"::"<< errorMap.flags[x] << endl;
        }
        return;
    } else {
        int frame;
        while ((frame = readFrame()) != -1) {
            if (this->shouldSampleImage)
                sampleImage();
        }
        this->slotRedraw();
    }

    btnPrevFrame->setEnabled(true);
    btnNextFrame->setEnabled(false);
    currentFrame = this->frames.size() -1;
}


void FormSubvisionImpl::slotAnalyseVariance(void) {    
    int currentFrame = this->currentFrame; 
    
    list<double> ballDists; 
    list<double> bluePinkDists; 
    list<double> pinkBlueDists; 
    list<double> yellowPinkDists; 
    list<double> pinkYellowDists; 
    
    list<double> yellowGoalDists; 
    list<double> blueGoalDists;
    
    // Rewind back to the first frame.
    while (this->btnPrevFrame->isEnabled()) {
        slotPrevFrame();
    }
    
    while (this->btnNextFrame->isEnabled()) {
        slotNextFrame();
        if (this->vBall.cf > 0) {
            ballDists.push_back(this->vBall.dist2);
        }        
        if (this->vBeacons[svPinkOnBlue].cf > 0) {
            pinkBlueDists.push_back(this->vBeacons[svPinkOnBlue].dist2);    
        }        
        if (this->vBeacons[svBlueOnPink].cf > 0) {
            bluePinkDists.push_back(this->vBeacons[svBlueOnPink].dist2); 
        }        
        if (this->vBeacons[svPinkOnYellow].cf > 0) {
            pinkYellowDists.push_back(this->vBeacons[svPinkOnYellow].dist2);
        }        
        if (this->vBeacons[svYellowOnPink].cf > 0) { 
            yellowPinkDists.push_back(this->vBeacons[svYellowOnPink].dist2);
        }        
        if (this->vGoals[svBlueGoal].cf > 0) {
            blueGoalDists.push_back(this->vGoals[svBlueGoal].dist2);
        }        
        if (this->vGoals[svYellowGoal].cf > 0) {
            yellowGoalDists.push_back(this->vGoals[svYellowGoal].dist2);
        }          
    }
    
    // Rewind back to the original position.
    while (this->currentFrame != currentFrame) {
        slotPrevFrame();  
    }
    
    double ballDistMean = calculateMean(ballDists);   
    double ballDistVar = calculateVariance(ballDists); 
    
    double bluePinkDistMean = calculateMean(bluePinkDists);
    double bluePinkDistVar = calculateVariance(bluePinkDists);
    double pinkBlueDistMean = calculateMean(pinkBlueDists); 
    double pinkBlueDistVar = calculateVariance(pinkBlueDists); 
    double yellowPinkDistMean = calculateMean(yellowPinkDists);
    double yellowPinkDistVar = calculateVariance(yellowPinkDists); 
    double pinkYellowDistMean = calculateMean(pinkYellowDists);
    double pinkYellowDistVar = calculateVariance(pinkYellowDists);
    
    double blueGoalDistMean = calculateMean(blueGoalDists);
    double blueGoalDistVar = calculateVariance(blueGoalDists);
    double yellowGoalDistMean = calculateMean(yellowGoalDists);
    double yellowGoalDistVar = calculateVariance(yellowGoalDists);

    
    stringstream s;
    string text;
    
    s << ballDistMean; s >> text; s.clear();
    this->lblVABallDistMean->setText(text);
    s << ballDistVar; s >> text; s.clear();
    this->lblVABallDistVar->setText(text);    
    s << sqrt(ballDistVar); s >> text; s.clear();
    this->lblVABallDistStd->setText(text); 
    
    s << bluePinkDistMean; s >> text; s.clear();  
    this->lblVABluePinkDistMean->setText(text);
    s << bluePinkDistVar; s >> text; s.clear();  
    this->lblVABluePinkDistVar->setText(text);
    s << sqrt(bluePinkDistVar); s >> text; s.clear();  
    this->lblVABluePinkDistStd->setText(text);
    
    s << pinkBlueDistMean; s >> text; s.clear(); 
    this->lblVAPinkBlueDistMean->setText(text);
    s << pinkBlueDistVar; s >> text; s.clear(); 
    this->lblVAPinkBlueDistVar->setText(text);
    s << sqrt(pinkBlueDistVar); s >> text; s.clear(); 
    this->lblVAPinkBlueDistStd->setText(text);
    
    s << yellowPinkDistMean; s >> text; s.clear();  
    this->lblVAYellowPinkDistMean->setText(text);
    s << yellowPinkDistVar; s >> text; s.clear();  
    this->lblVAYellowPinkDistVar->setText(text);
    s << sqrt(yellowPinkDistVar); s >> text; s.clear();  
    this->lblVAYellowPinkDistStd->setText(text);
    
    s << pinkYellowDistMean; s >> text; s.clear(); 
    this->lblVAPinkYellowDistMean->setText(text);
    s << pinkYellowDistVar; s >> text; s.clear(); 
    this->lblVAPinkYellowDistVar->setText(text);
    s << sqrt(pinkYellowDistVar); s >> text; s.clear(); 
    this->lblVAPinkYellowDistStd->setText(text);
    
    s << blueGoalDistMean; s >> text; s.clear();
    this->lblVABlueGoalDistMean->setText(text); 
    s << blueGoalDistVar; s >> text; s.clear();
    this->lblVABlueGoalDistVar->setText(text); 
    s << sqrt(blueGoalDistVar); s >> text; s.clear();
    this->lblVABlueGoalDistStd->setText(text);
    
    s << yellowGoalDistMean; s >> text; s.clear();
    this->lblVAYellowGoalDistMean->setText(text);    
    s << yellowGoalDistVar; s >> text; s.clear();
    this->lblVAYellowGoalDistVar->setText(text);
    s << sqrt(yellowGoalDistVar); s >> text; s.clear();
    this->lblVAYellowGoalDistStd->setText(text);
}

void FormSubvisionImpl::slotGPSDirectUpdate() 
{
    int x = FIELD_WIDTH/2;
    int y = FIELD_LENGTH/2;
    int h = 90;
    if (tabDisplayInfo->currentPage()->name() == QString("tabEdgeCalibration"))
    {
    
        x = this->ECGPSPosXText->text().toInt();
        y = this->ECGPSPosYText->text().toInt();
        h = this->ECGPSHeadingText->text().toInt();
        cout << __func__ << " x:" << x << " y:" << y << " h:" << h << endl;
        
    } 
    else 
    {
        x = this->GPSPosXText->text().toInt();
        y = this->GPSPosYText->text().toInt();
        h = this->GPSHeadingText->text().toInt();
    }
    gps.resetGaussians(x,y,h);
    //gps.GPSDirectPositionUpdate(x,y,h,5,5,5);
    this->slotRedraw();
}

void FormSubvisionImpl::slotDisplayPointsDebug() {  
    this->txtPDOutput->setText("");

    point start(0,0);
    point finish(0,0);
    if (rbScanlineVert->isChecked()) {
        int index = spinBoxScanline->value();
        start.first = subvision.scanlines[index].first.first;
        start.second = subvision.scanlines[index].first.second;
        finish.first = subvision.scanlines[index].second.first;
        finish.second = subvision.scanlines[index].second.second;
    } else if (rbScanlineHorz->isChecked()) {
        int index = spinBoxScanline->value();
        start.first = subvision.horzScanlines[index].first.first;
        start.second = subvision.horzScanlines[index].first.second;
        finish.first = subvision.horzScanlines[index].second.first;
        finish.second = subvision.horzScanlines[index].second.second;
    }
  
     
    int dir = DIR_UP;
    if (rbScanlineHorz->isChecked()) {
        dir = DIR_LEFT;
    }
    if (dir == DIR_UP) { 
        swap(start,finish);
    }
    
    int insanity = VF_SANE;   
    LineDraw::startLine(start,finish);
    //subvision.testPixel(start,dir,false,0,true,insanity);
    point next = start; 
    VisualFeature vf;
    cout << __func__ << " : This feature is broken, talk to nobu" << endl;
    while (next != finish) {
        next = LineDraw::nextPoint();
                
        //vf = subvision.testPixel(next,dir,false,0,false,insanity);
        if (insanity != VF_SANE) {
            char text[256];             
            sprintf(text,"%d,%d : %s",next.first,next.second,vfInsanityNames[insanity]);            
            
            this->txtPDOutput->append(text);      
        }
    } 
      
}

void FormSubvisionImpl::slotDisplayPoint() { 
    bool ok;    
    int x = this->txtPDXCord->text().toInt(&ok,10)*2; 
    int y = this->txtPDYCord->text().toInt(&ok,10)*2;
    
    QPainter p(this->pixmap);
    p.setPen(QPen(QColor("black"),1));   
    p.drawRect(x-1,y-1,2,2);
    bitBlt(this->pixmapImage, 0, 0, this->pixmap);
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
    
    //If file haven't been opened or no connection
    if (this->input == NULL) return -1;
    
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
            gotYUV = true;
            parseYUV(false);
        } else if (header.data_type == RLNK_CPLANE) {
            gotCPlane = true;
            parseCPlane(false, header.data_len);
        } else if (header.data_type == RLNK_SUBCPLANE) {
            gotSubCPlane = true;
            parseSubCPlane(false, header.data_len);
        } else if (header.data_type == RLNK_PWALK) {
            gotPWalk = true;
            parsePWalkInfo(false);
        } else if (header.data_type == RLNK_SUBVISION) {
            gotSubVision = true;
            parseFeatures(false, header.data_len);
        } else if (header.data_type == RLNK_SUBOBJECT) {
            gotSubObject = true;
            parseObjects(false);
        } else if (header.data_type == RLNK_INSANITY) {
            gotInsanities = true;
            parseInsanities(false);
        } else if (header.data_type == RLNK_ATOMIC_ACTION) {
            gotAtomicAction = true;
            parseAtomicAction();
        } else if (header.data_type == RLNK_GPSINFO) {
            gotGPSInfo = true;
            parseGPSInfo();    
        } else if (header.data_type == RLNK_PINKOBJECT) {
            gotPinkObject = true;
            parsePinkObject();            
        } else if (header.data_type == RLNK_GAUSSIANS) {
            gotGaussians = true;
            parseGaussians(header.data_len);
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

	if (gotYUV)
	  this->rbImageYUV->setEnabled(true);
	else
	  this->rbImageYUV->setEnabled(false);
	    
    }
    this->input->resetStatus(); // clear EOF flag

    if (this->frames.empty() ||
            (frameid != -1 && offset > this->frames.back())) {
        this->frames.push_back(offset);
    }
    
    // The image is sampled offline if we got a YUV image and didn't get
    // the processed subvision data
    this->shouldSampleImage = (frameid != -1 && gotYUV && ! gotSubVision
                                && ! gotSubObject && ! gotInsanities);

    if (DEBUG)
        cerr << "finished frame id " << frameid << endl;
    return frameid;
}

// Parse the atomic action
void FormSubvisionImpl::parseAtomicAction() {
  if (! this->input->isOpen() || this->input->status() != IO_Ok) {
    cerr << "parseAtomicAction: input bad or not open at " << this->input->at()
	 << endl;
    return;
  }
  if (DEBUG)
    cerr << "parsing rlnk_atomicaction from " << input->at() << endl;

  readAllBlock(this->input, (char *) &this->atomicAction,
	       sizeof(this->atomicAction));
  if (! (input->status() == IO_Ok)) {
    cout << "parseAtomicAction: Input error " << input->status() << endl;
    this->slotDisconnect();
    return;
  }
}

void FormSubvisionImpl::parseGaussians(int size) {
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << __func__ << ": input bad or not open at " << this->input->at() << endl;
        return;
    }
    int sizeSingleGaussian = sizeof(struct rlnk_gaussians);
    dogGaussiansSize = size / sizeSingleGaussian;//sizeof(SingleGaussian);
  
    if (DEBUG)
    {
    
        cerr << __func__ << " object from " << input->at() << endl;
        /*
        cerr << "roboheader:"<< sizeof(robolink_header)<< " no:"<<dogGaussiansSize<<" size:" << size << " for each:" << sizeof(struct rlnk_gaussians) << endl;
        cerr << "size of MVec3:" <<  sizeof (MMatrix3) << endl;
        cerr << "size of WMObj:" <<  sizeof (WMObj) << endl;
        cerr << "size of double:" <<  sizeof (double) << endl;
        cerr << "size of int:" <<  sizeof (int) << endl;
        cerr << "size total of SingleGaussian:" <<  sizeof (MMatrix3) +sizeof (WMObj) + sizeof (double)  + sizeof (int) << endl;
        */
    }
    
    if (dogGaussiansSize > MAXGAUSSIANS) {
        cout << __func__ << " size too big" <<endl;
        dogGaussiansSize = MAXGAUSSIANS;
        size = MAXGAUSSIANS * sizeSingleGaussian;
    }
    readAllBlock(this->input, (char *) &this->allgaussians,size);

  if (! (input->status() == IO_Ok)) {
    cout << __func__ << ": Input error " << input->status() << endl;
    this->slotDisconnect();
    return;
  }
}

void FormSubvisionImpl::parseGPSInfo() {
  if (! this->input->isOpen() || this->input->status() != IO_Ok) {
    cerr << "parseGPSInfo: input bad or not open at " << this->input->at()
	 << endl;
    return;
  }
  if (DEBUG)
    cerr << "parsing rlnk_gpsinfo from " << input->at() << endl;

  readAllBlock(this->input, (char *) &this->gpsinfo,
	       sizeof(this->gpsinfo));
  if (! (input->status() == IO_Ok)) {
    cout << "parseGPSInfo: Input error " << input->status() << endl;
    this->slotDisconnect();
    return;
  }
}

void FormSubvisionImpl::parsePinkObject() {
  if (! this->input->isOpen() || this->input->status() != IO_Ok) {
    cerr << "parsePinkObject: input bad or not open at " << this->input->at()
	 << endl;
    return;
  }
  if (DEBUG)
    cerr << "parsing rlnk_pinkallobject from " << input->at() << endl;
    int padding;
    readAllBlock(this->input, (char *) &this->pinkObjects,
	       sizeof(this->pinkObjects));
    readAllBlock(this->input, (char *) &this->num_of_pink,
	       sizeof(this->num_of_pink));
    readAllBlock(this->input, (char *) &padding,
	       sizeof(padding));
               
  if (! (input->status() == IO_Ok)) {
    cout << "parsePinkObject: Input error " << input->status() << endl;
    this->slotDisconnect();
    return;
  }
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
    //double averageY = 0.0;
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
        /*Andrew testing for modification yuv
       for (int col = 0; col < IMAGE_COLS; ++col) {
           cout << "[" << *(ptr+col) << " "<< endl;
           averageY +=*(ptr+col);
           // *(ptr+col) += 25;
       }
        */
//        for (int col = 0; col < IMAGE_COLS; ++col) {
//            QRgb pixel = yuv2rgb(yplane[row * ROW_SKIP + col], 
//                                uplane[row * ROW_SKIP + col],
//                                vplane[row * ROW_SKIP + col]);
//            image->setPixel(col, row, pixel);
//        }
        ptr += 6 * IMAGE_COLS;
    }
    //cout << __func__ << " ave Y:" << 1.0*averageY/(IMAGE_ROWS*IMAGE_COLS) << endl;
    if (DEBUG)
        cerr << "Done parsing YUV" << endl;
    return;
}

/* Parses the compressed CPlane stream at this->input and loads the image data.
 * If hdr is true a robolink header should be parsed first, else len contains
 * the number of bytes in the compressed data. Returns a new
 * QImage of the image data.
 */
void FormSubvisionImpl::parseCPlane(bool hdr, int len) {
    if (DEBUG)
        cerr << "parseCPlane (" << hdr << ") at " << this->input->at() 
            << "." << endl ;
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseCPlane: input bad or not open at " << this->input->at()
            << endl;
        return;
    }

    if (hdr) {
        robolink_header header;
        readAllBlock(this->input, (char*)&header, sizeof(robolink_header));
        if (! (input->status() == IO_Ok)) {
            cout << "ParseCPlane: Input error " << input->status() << endl;
            return;
        }
    }

    // Read the whole thing once into a buffer
    uchar buf[COMPRESSED_CPLANE_SIZE];
    readAllBlock(this->input, (char*)buf, len);
    if (! (input->status() == IO_Ok)) {
        cout << "ParseCPlane: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    
    // decode runlength encoding
    int row = 0, col = 0;
    uchar* p = buf;
    uchar length, colour;
    while (row < CPLANE_HEIGHT) {
        length = *(p++);
        colour = *(p++);
        memset(&cplane[row * CPLANE_WIDTH + col], colour, length);
        col += length;
        if (col == CPLANE_WIDTH) {
            ++row;
            col = 0;
        }
    }
    if (DEBUG)
        cerr << "Done parsing CPlane" << endl;
    return;
}

/* Parses the compressed subsampled CPlane stream at this->input and loads the
 * image data.  If hdr is true a robolink header should be parsed first.
 * Returns a new QImage of the image data.
 */
void FormSubvisionImpl::parseSubCPlane(bool hdr, int len) {
    if (DEBUG)
        cerr << "parseSubCPlane (" << hdr << ") at " << this->input->at() 
            << "." << endl ;
    if (! this->input->isOpen() || this->input->status() != IO_Ok) {
        cerr << "parseSubCPlane: input bad or not open at " << this->input->at()
            << endl;
        return;
    }

    if (hdr) {
        robolink_header header;
        readAllBlock(this->input, (char*)&header, sizeof(robolink_header));
        if (! (input->status() == IO_Ok)) {
            cout << "ParseSubCPlane: Input error " << input->status() << endl;
            return;
        }
    }
    
    // Read the whole thing once into a buffer
    uchar buf[COMPRESSED_CPLANE_SIZE];
    readAllBlock(this->input, (char*)buf, len);
    if (! (input->status() == IO_Ok)) {
        cout << "ParseSubCPlane: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    
    
    // decode runlength encoding
    int row = 0, col = 0;
    uchar* p = buf;
    uchar length, colour;
    while (row < CPLANE_HEIGHT) {
        length = *(p++);
        colour = *(p++);
        for (int i = 0; i < SUB_CPLANE_SAMPLE; ++i) {
            memset(&cplane[(row+i) * CPLANE_WIDTH + col], colour,
                    length * SUB_CPLANE_SAMPLE);
        }
        col += length * SUB_CPLANE_SAMPLE;
        if (col >= CPLANE_WIDTH) {
            row += SUB_CPLANE_SAMPLE;
            col = 0;
        }
    }
    if (DEBUG)
        cerr << "Done parsing SubCPlane" << endl;
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
            << " desiredCrane: " << this->pWalkInfo.desiredCrane 
            << " UPan: " << this->pWalkInfo.upan
            << " UTilt: " << this->pWalkInfo.utilt
            << " UCrane: " << this->pWalkInfo.ucrane 
            << endl;
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
        if (f.type == VF_OBSTACLE)
            this->obstacleFeatures.push_back(f);
        else
            this->features.push_back(f);
    }
    if (! (input->status() == IO_Ok)) {
        cout << "ParseFeatures: Input error " << input->status() << endl;
        this->slotDisconnect();
        return;
    }
    if (DEBUG)
        cerr << "parsed " << i << " features" << endl << "obstacle features:" << this->obstacleFeatures.size() <<endl;
}

/* copying vob from objects to this->cortex */
void FormSubvisionImpl::copyVobToCortex(void){
    this->cortex.vob[vobBall] = this->vBall;
    this->cortex.vob[vobPinkBlueBeacon] = this->vBeacons[svPinkOnBlue];
    this->cortex.vob[vobBluePinkBeacon] = this->vBeacons[svBlueOnPink];
    this->cortex.vob[vobPinkYellowBeacon] = this->vBeacons[svPinkOnYellow];
    this->cortex.vob[vobYellowPinkBeacon] = this->vBeacons[svYellowOnPink];
    this->cortex.vob[vobBlueGoal] = this->vGoals[svBlueGoal];
    this->cortex.vob[vobYellowGoal] = this->vGoals[svYellowGoal];
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


/*
 * Initialise robot condition
 * Pan tilt crane, it's useful when doing top view.
 */
void FormSubvisionImpl::initRobotCondition(void)
{
    //Put tilt pan crane to the cortex.pan, tilt and crane
    struct PWalkInfoStruct pwalkinfo = {
            &this->pWalkInfo.leading_leg,
            &this->pWalkInfo.currentWalkType,
            0,
            &this->pWalkInfo.currentStep,
            &this->pWalkInfo.currentPG,
            &this->pWalkInfo.frontShoulderHeight,
            &this->pWalkInfo.bodyTilt,
            &this->pWalkInfo.desiredPan,
            &this->pWalkInfo.desiredTilt,
            &this->pWalkInfo.desiredCrane,
            0,   // isHighGain not used in VisualCortex
            0,
    };
    
    this->cortex.pWalkInfo= &pwalkinfo;
    this->cortex.TrigInit(this->pWalkInfo.utilt,this->pWalkInfo.upan, this->pWalkInfo.ucrane);
    if (gotSubVision)
    {
        this->subvision.features = this->features;
        this->subvision.obstacleFeatures = this->obstacleFeatures;
    }
    
    this->cortex.bodyMoving = true;
    
    if (gotGPSInfo)
        cortex.headMovement = gpsinfo.headSpeed;
    
    
    if (gotAtomicAction)
        this->cortex.bodyMoving = (this->atomicAction.forward != 0 || this->atomicAction.left != 0 || this->atomicAction.turn != 0);
    
    
    this->cortex.bodyMoving = this->CB_Pink_Update_Body_Move->isChecked();    
        
    if (TXT_EC_TEW_Cull_Dist->text() == "" ||
        TXT_EC_TEW_Reduce_Dist->text() == "" ||
        TXT_EC_TEW_Min_Dist->text() == "" ||
        TXT_EC_TEW_Min_Head_Speed->text() == "" || 
        TXT_EC_TEW_Max_Sqr_Dist_Field->text() == "" 
    )
    {
        slot_EC_TEW_Default();
    }
    //cout << __func__ << "body moving" << this->cortex.bodyMoving << endl;
    this->cortex.runEdgeDetect(
        TXT_EC_TEW_Reduce_Dist->text().toDouble(),
        TXT_EC_TEW_Cull_Dist->text().toDouble(),
        TXT_EC_TEW_Min_Dist->text().toInt(),
        TXT_EC_TEW_Min_Head_Speed->text().toDouble(),
        TXT_EC_TEW_Max_Sqr_Dist_Field->text().toDouble()
        );
}

/* Resamples the image with the current settings and copies the features and
 * objects into this forms features and objects */
void FormSubvisionImpl::sampleImage(void)
{
    if (this->pixmap != NULL)
        delete this->pixmap;
    this->pixmap = new QPixmap(this->image->scale(this->pixmapImage->width(),
                                                this->pixmapImage->height()));

    if (DEBUG)
        cout << "Sampling image..." << endl;
    struct PWalkInfoStruct pwalkinfo = {
            &this->pWalkInfo.leading_leg,
            &this->pWalkInfo.currentWalkType,
            0,
            &this->pWalkInfo.currentStep,
            &this->pWalkInfo.currentPG,
            &this->pWalkInfo.frontShoulderHeight,
            &this->pWalkInfo.bodyTilt,
            &this->pWalkInfo.desiredPan,
            &this->pWalkInfo.desiredTilt,
            &this->pWalkInfo.desiredCrane,
            0,   // isHighGain not used in VisualCortex
            0,
    };

    if (checkBoxIsGrabbed->isChecked()) {
        this->cortex.isGrabbed = true;
    }
    if (checkBoxIsGrabbing->isChecked()) {
        this->cortex.isGrabbing = true;
    }
    
    if (gotAtomicAction)
        this->cortex.bodyMoving = (this->atomicAction.forward != 0 || this->atomicAction.left != 0 || this->atomicAction.turn != 0);
    
    
    this->cortex.bodyMoving = this->CB_Pink_Update_Body_Move->isChecked();    
    //cout << " body moving:" << this->cortex.bodyMoving << endl;
    if (DEBUG)
        cout << __func__ << " : VisualCortex->processImage started" << endl;
    this->cortex.processImage(this->yuv, this->pWalkInfo.utilt,
                                this->pWalkInfo.upan, this->pWalkInfo.ucrane,
                                this->pWalkInfo.range2Obstacle, &pwalkinfo);
    if (DEBUG)
        cout << __func__ << " : VisualCortex->processImage end" << endl;
    // Copy information to this object for display. We copy rather than access
    // it directly from the display code so that we can also display data that
    // we read from log and haven't calculated.
    this->camorient.hz_intercept = cortex.hz_c;
    this->camorient.hz_gradient = cortex.hz_m;
    this->camorient.hz_exists = cortex.hzExists;
    this->camorient.hz_up_is_sky = cortex.hzUpIsSky;
    this->camorient.hz_below_image = cortex.hzAllAboveHorizon;
    this->features = this->subvision.features;
    this->obstacleFeatures = this->subvision.obstacleFeatures;
    this->vBall = this->cortex.vob[vobBall];
    this->vBeacons[svPinkOnBlue] = this->cortex.vob[vobPinkBlueBeacon];
    this->vBeacons[svBlueOnPink] = this->cortex.vob[vobBluePinkBeacon];
    this->vBeacons[svPinkOnYellow] = this->cortex.vob[vobPinkYellowBeacon];
    this->vBeacons[svYellowOnPink] = this->cortex.vob[vobYellowPinkBeacon];
    this->vGoals[svBlueGoal] = this->cortex.vob[vobBlueGoal];
    this->vGoals[svYellowGoal] = this->cortex.vob[vobYellowGoal];
    memcpy(this->insanities, SanityChecks::fired, INSANITY_COUNT/8 + 1);

    gotSubVision = gotSubObject = gotCamOrient
        = gotInsanities = gotCPlane = true;
    
    if (DEBUG)
      Profile::print(NUM_PROFID);
}

/* Clears feature data read from file/network or created through sampled image
 */
void FormSubvisionImpl::clearData() {
    this->features.clear();
    this->obstacleFeatures.clear();
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
    gotCPlane = gotSubCPlane = gotYUV = gotCamOrient = gotSubVision
        = gotSubObject = gotInsanities = gotPWalk = gotGPSInfo 
        = gotPinkObject = gotAtomicAction = gotGaussians = false;
    this->image->fill(Qt::darkGray.pixel());
}

//Refresh the image display 
//This just redraw image without calculating any other instruction
void FormSubvisionImpl::slotRefreshImage(void) {
    this->displayImage();
    this->displayScanlines();
    this->drawSlice();
    this->displayFeatures();
    this->displayObjects();   
    bitBlt(this->pixmapImage, 0, 0, this->pixmap);
}

/* Displays the background RGB image, CPlane or plain background */
void FormSubvisionImpl::displayImage(void) {
    if (this->rbImageYUV->isChecked() && this->gotYUV) {
      CorrectedImage::setYUV(this->yuv);
        if (this->pixmap != NULL)
            delete this->pixmap;
        for (int row = 0; row < IMAGE_ROWS; ++row) {
            for (int col = 0; col < IMAGE_COLS; ++col) {
                QRgb pixel;
                if (checkBoxYUVCorrected->isChecked()) {
                    pixel = yuv2rgb(this->cortex.img.y(col, row),
                                    this->cortex.img.u(col, row),
                                    this->cortex.img.v(col, row));
                } else {
                    pixel = yuv2rgb(yplane[row * ROW_SKIP + col], 
                                    uplane[row * ROW_SKIP + col],
                                    vplane[row * ROW_SKIP + col]);
		   
                }
                image->setPixel(col, row, pixel);
            }
        }
        this->pixmap = new QPixmap(this->image->scale(
                                            this->pixmapImage->width(),
                                            this->pixmapImage->height()));
    } else if (this->rbImageCPlane->isChecked()
            && (this->gotCPlane || this->gotSubCPlane)) {
        if (DEBUG)
            cerr << "displaying CPlane" << endl;
        if (shouldSampleImage) {
            CorrectedImage::fillCPlane(this->checkBoxCPlaneTight->isChecked());
            memcpy(this->cplane, CorrectedImage::cplane,
                    CPLANE_WIDTH * CPLANE_HEIGHT);
        }
        QImage* image = new QImage(IMAGE_COLS, IMAGE_ROWS, 32);
        for (int y = 0; y < IMAGE_ROWS; ++y) {
            for (int x = 0; x < IMAGE_COLS; ++x) {
                uchar c = cplane[y*IMAGE_COLS + x] & ~MAYBE_BIT;
                image->setPixel(x, y,
                        QColor(CPLANE_COLOURS[c]).rgb());
            }
        }
        if (this->pixmap != NULL)
            delete this->pixmap;
        this->pixmap = new QPixmap(image->scale(this->pixmapImage->width(),
                                                this->pixmapImage->height()));
        delete image;
    } else {        
        if (this->pixmap != NULL)
            delete this->pixmap;
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
    if (! (checkBoxDisplayScanlines->isChecked() && gotPWalk))
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

    for (   itr=subvision.leftScanlines.begin();
            itr != subvision.leftScanlines.end();
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
        } else if (itr->type == VF_WALL_OBJECT) { 
            int y1 = int(itr->x * 0 + itr->y);
            int y2 = int(itr->x * CPLANE_WIDTH + itr->y); 
            p.setPen(QPen(OBJ_COLOUR_WALL, 2));            
            p.drawLine(0,y1*2,CPLANE_WIDTH*2,y2*2);    
        } else {
            p.setPen(QPen(VF_COLOURS[itr->type], 1));
            p.drawRect((int)itr->x * 2, (int)itr->y * 2, 2, 2);
        }
    }
    for (itr = this->features.begin(); itr != this->features.end(); ++itr) {
        if (itr->type == VF_BALL) {
            p.setPen(QPen(VF_COLOURS[itr->type], 1));
            p.drawRect((int)itr->x * 2, (int)itr->y * 2, 2, 2);
        }
    }     
        
    // obstacle features
    itr = this->obstacleFeatures.begin();
    for (; itr != this->obstacleFeatures.end(); ++itr) {
        p.setPen(QPen(VF_COLOURS[itr->type], 1));
        p.drawRect((int)itr->x * 2, (int)itr->y * 2, 2, 2); 
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
    //double horizonAngle = atan(this->camorient.hz_gradient);
    double horizonAngle = atan(this->cortex.hz_m);
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
            int leftGapMin = (int)g.leftGapMin;
            int leftGapMax = (int)g.leftGapMax;
            int rightGapMin = (int)g.rightGapMin;
            int rightGapMax = (int)g.rightGapMax;
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
            // draw best gap
            if (leftGapMin || leftGapMax) {
                p.setPen(QPen(OBJ_COLOUR_GAP, 1));
                drawLine(p, make_pair(leftGapMin, cy),
                        make_pair(leftGapMax, cy), 2);
                //cerr << "left gap at " << leftGapMin << ", "
                //<< leftGapMax << endl;
            }
            // draw other best gap
            if (rightGapMin || rightGapMax) {
                p.setPen(QPen(OBJ_COLOUR_GAP, 1));
                drawLine(p, make_pair(rightGapMin,cy), make_pair(rightGapMax,cy), 2);
                //cerr << "right gap at " << rightGapMin << ", "
                //<< rightGapMax << endl;
            }
        }
    }

 //   cerr << this->tabDisplayInfo->currentPage()->name() << endl;
//    if (this->tabDisplayInfo->currentPage()->name() != QString("tabObjects"));
//        return;
//    cerr << "Boo!" << endl;
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
    QLabel* const bHeight[NUM_BEACONS] =
        {this->lblBluePinkHeight, this->lblPinkBlueHeight,
        this->lblYellowPinkHeight, this->lblPinkYellowHeight};
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
        str << this->vBeacons[i].height;
        bHeight[i]->setText(str.str());
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
    
    //Flags
    slotDisplayFlag();
}

void FormSubvisionImpl::displayGrabTurnKickInfo() {
  if (!gotPWalk) {
    lblPWalkWalkType->clear();
    lblPWalkStepOnPG->clear();
    lblPWalkStepDiff->clear();
    lblFrameDiff->clear();
    
    lblDesiredPan->clear(); 
    lblDesiredTilt->clear(); 
    lblDesiredCrane->clear(); 
    lblActPan->clear(); 
    lblActTilt->clear(); 
    lblActCrane->clear();
  }
    
    if (DEBUG)
        cerr << "displayGrabTurnKickInfo" << endl;
    if (tabDisplayInfo->currentPage()->name() != QString("tabGrabTurnKickInfo"))
        return;


    ostringstream str;
    str.precision(3);
    if (this->pWalkInfo.currentWalkType == 21) // dive kick
      this->groupBoxPWalkInfo->setPaletteBackgroundColor(QColor("blue"));
    else
      this->groupBoxPWalkInfo->setPaletteBackgroundColor(QColor("grey"));

    str << walkTypeToStr[this->pWalkInfo.currentWalkType];
    this->lblPWalkWalkType->setText(str.str());
    str.str("");

    str << (this->pWalkInfo.currentStep - this->lastStep);
    this->lblPWalkStepDiff->setText(str.str());
    str.str("");
    str << this->pWalkInfo.currentStep << " / " << this->pWalkInfo.currentPG;
    this->lblPWalkStepOnPG->setText(str.str());
    str.str("");
    str << (atoi(this->lblFrameNo->text()) - this->lastMotionFrameNo);
    this->lblFrameDiff->setText(str.str());
    str.str("");
    str << (this->pWalkInfo.currentStep * 100.0 / this->pWalkInfo.currentPG);
    this->lblPWalkStepPercentage->setText(str.str());
    str.str("");
    
    str << (this->pWalkInfo.desiredPan); 
    this->lblDesiredPan->setText(str.str()); 
    str.str("");    
    str << (this->pWalkInfo.desiredTilt); 
    this->lblDesiredTilt->setText(str.str()); 
    str.str("");    
    str << (this->pWalkInfo.desiredCrane); 
    this->lblDesiredCrane->setText(str.str()); 
    str.str("");
    str << (this->pWalkInfo.upan * 180 / 3.14 / 1000000); 
    this->lblActPan->setText(str.str()); 
    str.str("");
    str << (this->pWalkInfo.utilt * 180 / 3.14 / 1000000); 
    this->lblActTilt->setText(str.str()); 
    str.str("");
    str << (this->pWalkInfo.ucrane * 180 / 3.14 / 1000000); 
    this->lblActCrane->setText(str.str()); 
    str.str("");


    // python behaviour
    str << commandToStr[this->atomicAction.command];
    this->lblPyCommand->setText(str.str());
    str.str("");    

    if (this->atomicAction.walkType == 21) // dive kick
      this->groupBoxPyInfo->setPaletteBackgroundColor(QColor("blue"));
    else
      this->groupBoxPyInfo->setPaletteBackgroundColor(QColor("grey"));
    str << walkTypeToStr[this->atomicAction.walkType];
    this->lblPyWalkType->setText(str.str());
    str.str("");

    if (this->atomicAction.PG == 23)
      this->lblGrabbedBall->setText("Grab Turning!!");
    else
      this->lblGrabbedBall->clear();
    str << this->atomicAction.PG;
    this->lblPyPG->setText(str.str());
    str.str("");    
    str << this->atomicAction.forward;
    this->lblPyForward->setText(str.str());
    str.str("");
    str << this->atomicAction.left;
    this->lblPyLeft->setText(str.str());
    str.str(""); 
    str << this->atomicAction.turn;
    this->lblPyTurnCCW->setText(str.str());
    str.str("");
    str << this->atomicAction.forwardMaxStep;
    this->lblPyForwardStep->setText(str.str());
    str.str("");
    str << this->atomicAction.leftMaxStep;
    this->lblPyLeftStep->setText(str.str());
    str.str(""); 
    str << this->atomicAction.turnCCWMaxStep;
    this->lblPyTurnCCWStep->setText(str.str());
    str.str("");
    

    // best gap
    bool hasBestGap = false;
    for (int i = 0; i < NUM_GOALS; ++i) {
      VisualObject& goal = vGoals[i];
      if (goal.cf > 0) {
	str << (int)goal.leftGapMin;
	this->lblBestGapPointLeft->setText(str.str());
	str.str("");
	str << (int)goal.leftGapMax;
	this->lblBestGapPointRight->setText(str.str());
	str.str("");

        double hLeft = pointToHeading(goal.leftGapMin, goal.cy,
				      cortex.sin_eroll, cortex.cos_eroll);
	str << hLeft;
	this->lblBestGapHeadingLeft->setText(str.str());
	str.str("");

        double hRight = pointToHeading(goal.leftGapMax, goal.cy,
				       cortex.sin_eroll, cortex.cos_eroll);
	str << hRight;

	this->lblBestGapHeadingRight->setText(str.str());
	str.str("");

	str << hLeft - hRight;
	this->lblBestGapHeadingRange->setText(str.str());
	str.str("");

	str << (hLeft + hRight) / 2.0;
	this->lblBestGapHeadingMiddle->setText(str.str());
	str.str("");

	double padding = fabs(hLeft - hRight) / 5.0;
        double hMin = MIN(hLeft, hRight);
        double hMax = MAX(hLeft, hRight);	
	  
	if (hMin < -padding && hMax > padding)
	  this->lblOkToShoot->setText("OK TO SHOOT!");
	else
	  this->lblOkToShoot->clear();

	hasBestGap = true;
      }
    }
    if (!hasBestGap) {
      this->lblBestGapPointLeft->clear();
      this->lblBestGapPointRight->clear();
      this->lblBestGapHeadingLeft->clear();
      this->lblBestGapHeadingRight->clear();
      this->lblBestGapHeadingRange->clear();
      this->lblBestGapHeadingMiddle->clear();
    }

    this->lastMotionFrameNo = atoi(this->lblFrameNo->text());
    this->lastStep = this->pWalkInfo.currentStep;
}

/* Displays the projected obstacles in the obstacles panel */
void FormSubvisionImpl::displayObstacles() {
    if (DEBUG)
        cerr << "displayObstacles" << endl;
    if (tabDisplayInfo->currentPage()->name() != QString("tabObstacles") or (!gotYUV and !gotSubVision))
        return;
        
    Obstacle obstacle;
    
    QPixmap projected(this->pixmapProjected->width(),
            this->pixmapProjected->height());
            
    projected.fill(Qt::darkGray);
    QPainter p(&projected);
    point pt1,pt2;
    /**********************************/
    //Drawing lines
    int middle_line = this->pixmapProjected->height()/2/2;
    p.setPen(QPen(QColor(SCANLINE_COLOUR), 1));
    //Draw vertical line
    for (int tmp = 5; tmp <this->pixmapProjected->width() ; tmp+=5)
    {
        if (tmp % 25 == 0)
            p.setPen(QPen(QColor(GRID_COLOUR), 1));
        else 
            p.setPen(QPen(QColor(SCANLINE_COLOUR), 1));
        pt1.first = tmp;
        pt1.second = 0;
        pt2.first = tmp;
        pt2.second = this->pixmapProjected->height();
        drawLine(p, pt1, pt2, 2);
    }
    //Draw horizontal
    
    for (int tmp = 5; tmp < middle_line ; tmp+= 5)
    {
        if (tmp % 25 == 0)
            p.setPen(QPen(QColor(GRID_COLOUR), 1));
        else 
            p.setPen(QPen(QColor(SCANLINE_COLOUR), 1));
        pt1.first = 0;
        pt2.first = this->pixmapProjected->width();        
        pt1.second = pt2.second = middle_line - tmp;
        drawLine(p, pt1, pt2, 2);
        pt1.second = pt2.second = middle_line + tmp;
        drawLine(p, pt1, pt2, 2);        
    }
 
    p.setPen(QPen(QColor(HORIZON_COLOUR), 1));
    //Draw middle line
    pt1.first = 0;
    pt2.first = this->pixmapProjected->width();
    pt1.second = pt2.second = middle_line;
    drawLine(p, pt1, pt2, 2);
    /* End drawing lines */
    /**********************************/
    
    /* Drawing points */
    int maxPoints = 500;
    int counter =0;
    double inPoints[maxPoints*2];
    int sizeInPoints = 0;
    //cout << "obstacleFeatures.size():" <<obstacleFeatures.size() << "this->obstacleFeatures:" <<this->obstacleFeatures.size()<<endl;
    vector<VisualFeature>::iterator itr;
    if (this->CB_DisplayObstacle->isChecked())
    {
        itr= this->obstacleFeatures.begin();
        counter = 0;
        //Getting initial obstacle features points
        for (; itr != this->obstacleFeatures.end() && counter < maxPoints*2 - 1 ; ++itr) {
            if (itr->type == VF_OBSTACLE) {
                inPoints[counter] = itr->x;
                inPoints[counter+1] = itr->y;
                counter +=2;
            }
        }
        sizeInPoints = counter/2;
        int afterFiltering[counter];
        
        /* project it and filter the points */
        int noOfObstaclePoints = 
            projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints, MAX_VIEW_ACCURATE_OBSTACLE, MAX_VIEW_ACCURATE_OBSTACLE);
            
        double dogX, dogY, dogH, posVar, hVar;    
            
        if (gotGPSInfo)
        {
            dogX = (double)this->gpsinfo.posX;
            dogY = (double)this->gpsinfo.posY;
            dogH = (double)this->gpsinfo.heading;    
            posVar = (double)this->gpsinfo.posVar;
            hVar = (double)this->gpsinfo.hVar;        
        }
        else
        {
            dogX = gps.r[0].mean.pos.x;
            dogY = gps.r[0].mean.pos.y;
            dogH = gps.r[0].mean.h;
            posVar = 500;
            hVar = 500;
        }            
       
        //set the obstacle objects with obstacle points            
        obstacle.setObstaclePoints(afterFiltering,noOfObstaclePoints,
        this->currentFrame,dogX,dogY,dogH,posVar,hVar);        
        
        // (SQRT(gps.r[0].cov(0,0))+SQRT(gps.r[0].cov(1,1))) /2 ,SQRT(gps.r[0].cov(2,2)));
        
        /* Top view display*/
        /* after adjust to top view */
        int displayX, displayY;
        for (counter = 0; counter < noOfObstaclePoints; ++counter) 
        {
            /* adjusting to top view */
            displayX = afterFiltering[(counter*2)+1];
            displayY = afterFiltering[(counter*2)] +
                (this->pixmapProjected->height()/2);
            //draw
            p.setPen(QPen(VF_COLOURS[VF_OBSTACLE], 1));
            p.drawRect(displayX,displayY, 2, 2);
        }
        //cout << "sizeInPoints:" << sizeInPoints <<endl;
        //cout << "after filtering:" << noOfObstaclePoints <<endl;      
        
        int gMidX, gMidY;
        double midX, midY;
        int numBoxes = obstacle.getNumGPSBoxes();
        ObstacleBox* gpsobs = obstacle.getGPSBoxes();
        for (counter = 0; counter < numBoxes; ++counter) 
        {
            if (gpsobs[counter].getNoTotalPoints() >= spinObsIntensity->value())
            {
                gpsobs[counter].getMiddlePoint(&gMidX, &gMidY);
                getLocalCoordinate(dogX, dogY, dogH, 
                gMidX, gMidY, &midX, &midY);
                
                /* adjusting to top view */
                displayX = int(midY);
                displayY = int(midX) + (this->pixmapProjected->height()/2);
                
                //draw
                p.setPen(QPen("pink", 1));
                p.drawRect(displayX,displayY, 10, 10);
            }
        }          
    }
    if (this->CB_Display_Fieldline->isChecked())
    {
        itr= this->features.begin();
        counter = 0;
        //Getting initial field line features points
        for (; itr != this->features.end() && counter < maxPoints*2 - 1 ; ++itr) {
            if (itr->type != VF_FIELDLINE)
                continue;
            inPoints[counter] = itr->x;
            inPoints[counter+1] = itr->y;
            counter +=2;
        }
        sizeInPoints = counter/2;
        int afterFiltering[counter];
        
        /* project it and filter the points */
        int no_of_points = 
            projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints);
                    
        /* Top view display*/
        /* after adjust to top view */
        int displayX, displayY;
        for (counter = 0; counter < no_of_points ; ++counter) 
        {
            /* adjusting to top view */
            //cout << "FL:"<<counter<<" " << afterFiltering[(counter*2)] <<":"<< afterFiltering[(counter*2)+1] <<endl;
            displayX = afterFiltering[(counter*2)+1];
            displayY = afterFiltering[(counter*2)] +
                (this->pixmapProjected->height()/2);
            //draw
            p.setPen(QPen(VF_COLOURS[VF_FIELDLINE], 1));
            p.drawRect(displayX,displayY, 2, 2);
        }    
    }
    if (this->CB_Display_Field->isChecked())
    {
        itr= this->features.begin();
        counter = 0;
        //Getting initial field features points
        for (; itr != this->features.end() && counter < maxPoints*2 - 1 ; ++itr) {
            if (itr->type != VF_FIELD)
                continue;
            inPoints[counter] = itr->x;
            inPoints[counter+1] = itr->y;
            counter +=2;
        }
        sizeInPoints = counter/2;
        int afterFiltering[counter];
        
        /* project it and filter the points */
        int no_of_points = 
            projectAndFilterPoints(inPoints,afterFiltering,sizeInPoints);
                    
        /* Top view display*/
        /* after adjust to top view */
        int displayX, displayY;
        for (counter = 0; counter < no_of_points; ++counter) 
        {
            //cout << "F:"<<counter<<" " << afterFiltering[(counter*2)] <<":"<< afterFiltering[(counter*2)+1] <<endl;
            /* adjusting to top view */
            displayX = afterFiltering[(counter*2)+1];
            displayY = afterFiltering[(counter*2)] +
                (this->pixmapProjected->height()/2);
            //draw
            p.setPen(QPen("blue"/*VF_COLOURS[VF_FIELD]*/, 1));
            p.drawRect(displayX,displayY, 2, 2);
        }
        //cout << "FIELD sizeInPoints:" << sizeInPoints <<endl;
        //cout << "FIELD after filtering:" << no_of_points <<endl;        
    }    
    
    if (ANDREWDEBUG) 
    {
        cout << "getNoObstacleInBox(-500,500,500,0):" << obstacle.getNoObstacleInBox(-200,150,200,50) <<endl;
        int posX = 0,posY = 0;
        cout << "NearestObsticle ? " <<obstacle.getPointToNearestObstacleInBox(-200,250,200,50, &posX, &posY, 2);
        cout << ":X:" << posX << ":Y:" << posY << endl;
    }
    
    if (btnObsShowBestGap->isChecked())
    {
        int displayLineLen = 200;
        
        int h = this->pixmapProjected->height();
        //int w = this->pixmapProjected->width();
        
        int gapLeft, gapRight, mybestgap, gapAngle;
        
        ostringstream str;
        str.precision(3);
        
        if (btnObsShowBestGapToBall->isChecked())
        {
            this->spinObsDestX->setValue(-(int)this->vBall.objectPosition[0]);
            this->spinObsDestY->setValue((int)this->vBall.objectPosition[2]);
        }
        
        QBrush brush( yellow ); 
        p.setBrush( brush );
        p.setPen( NoPen );   
        p.drawRect(int(rint(spinObsDestY->value() - 5)),
        int(rint(spinObsDestX->value() - 5) + 
        (this->pixmapProjected->height()/2)), 10, 10);     
        p.setBrush( NoBrush ); 
            
        bool result;
        if(btnObsShowBestGapGPS->isChecked())
        {
            result = obstacle.getBestGap(
            spinObsDestX->value(), spinObsDestY->value(),
            &gapLeft, &gapRight, &mybestgap, &gapAngle,
            GAP_MAX_DIST, MIN_DIST, 
            spinObsMinGap->value(), spinObsIntensity->value(), 
            OBS_USE_GPS /*+ OBS_USE_SHARED*/ /*+ OBS_USE_CONST*/);
        }
        else
        {
            result = obstacle.getBestGap(
            spinObsDestX->value(), spinObsDestY->value(),
            &gapLeft, &gapRight, &mybestgap, &gapAngle,
            GAP_MAX_DIST, MIN_DIST, 
            spinObsMinGap->value(), spinObsIntensity->value(),
            OBS_USE_LOCAL /*+ OBS_USE_SHARED*/ /*+ OBS_USE_CONST*/);
        }
        if (!result){
            mybestgap = 0;

            this->lblBestGapHeading->setText("NOGAP"); 
            
            displayLineLen = 50;    
        } 
        else{
            str << mybestgap;
            this->lblBestGapHeading->setText(str.str());
            str.str("");            
        } 
            
        double mybestgap_RAD = -DEG2RAD(mybestgap);
        
        int MGA = spinObsMinGap->value();
        
        double MGA_RAD = DEG2RAD(MGA);

        int p1x = 0;
        int p1y = (int) (h/2);
        int p2x = (int) (p1x + (cos(mybestgap_RAD) * displayLineLen));
        int p2y = (int) (p1y + (sin(mybestgap_RAD) * displayLineLen));
        
        if (!result) p.setPen(QPen(QColor("red"), 1));
        else p.setPen(QPen(QColor("darkMagenta"), 1));
        drawLine(p, make_pair(p1x,p1y), make_pair(p2x,p2y), 1);
        
        double tmp;
        
        tmp = mybestgap_RAD + (MGA_RAD/2);
        p2x = (int) (p1x + (cos(tmp) * displayLineLen));
        p2y = (int) (p1y + (sin(tmp) * displayLineLen));
        
        if (!result) p.setPen(QPen(QColor("red"), 1));
        else p.setPen(QPen(QColor(BESTGAP_COLOUR), 1));
        drawLine(p, make_pair(p1x,p1y), make_pair(p2x,p2y), 1);     
        
        
        tmp = mybestgap_RAD - (MGA_RAD/2);
        p2x = (int) (p1x + (cos(tmp) * displayLineLen));
        p2y = (int) (p1y + (sin(tmp) * displayLineLen));
        
        if (!result) p.setPen(QPen(QColor("red"), 1));
        else p.setPen(QPen(QColor(BESTGAP_COLOUR), 1));
        drawLine(p, make_pair(p1x,p1y), make_pair(p2x,p2y), 1);     
            
    }
    else
    {
        this->lblBestGapHeading->setText("");
    }
    
    this->pixmapProjected->setPixmap(projected);
}






/* Calculates the Histogram values for the given slice.
 * Origin is top left corner
 */
void FormSubvisionImpl::calculateHistogram() {
//    cerr << "calculateHistogram" << endl;
//    if (! checkBoxDisplayHistogram->isChecked())
//        return;
    if (tabDisplayInfo->currentPage()->name() != QString("tabHistograms"))
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
        //int sat  = int(sqrt(double((newU-128)*(newU-128)+(newV-128)*(newV-128))));
        //int hue  = int(((atan2(double(newU-128),newV-128))/M_PI)*128+128);
        //newU = hue; 
        //newV = sat;
        int newdY = newY - prevPoint[1] + (histMax/2);
        //int newdU = (char(newU - prevPoint[2]))*3 + (histMax/2);
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

/*Project and filter features points */
int FormSubvisionImpl::projectAndFilterPoints(double *inPoints, int *afterFiltering, int no_of_points, int clip_X, int clip_Y)
{
    double afterFilteringDbl[no_of_points*2];
    int counter;
    int no_points = projectAndFilterPoints(inPoints, afterFilteringDbl, no_of_points, clip_X, clip_Y);
    for (counter = 0; counter < no_points; ++counter) 
    {
        afterFiltering[counter*2] = (int)afterFilteringDbl[counter*2];
        afterFiltering[(counter*2)+1] = (int)afterFilteringDbl[(counter*2)+1];
    }
    return counter;
}

/*Project and filter features points */
int FormSubvisionImpl::projectAndFilterPoints(double *inPoints, double *afterFiltering, int no_of_points, int clip_X, int clip_Y)
{
    double afterCentering[no_of_points*2];
    double projectedPoints[no_of_points*2];
    int counter;
    /* after centering the camera coordinate*/
    for (counter = 0; counter < no_of_points; ++counter) 
    {
        afterCentering[counter*2] = TOPLEFT2CENTER_X(inPoints[counter*2]);
        afterCentering[(counter*2)+1] =
            TOPLEFT2CENTER_Y(inPoints[(counter*2)+1]);
    }
    
    /* project it */
    cortex.projectPoints(afterCentering, no_of_points , projectedPoints);
    
    /* filtering for infinite or large int*/
    counter = 0;
    for (int i = 0; i < no_of_points; ++i)
    {
        /* if point are not correct or inaccurate */
        if (projectedPoints[i*2] == INT_MIN || 
            projectedPoints[(i*2)+1] == INT_MIN || 
            ABS(projectedPoints[(i*2)]) > clip_X || 
            ABS(projectedPoints[(i*2)+1]) > clip_Y)
                continue;
        afterFiltering[counter*2] = projectedPoints[(i*2)];
        afterFiltering[(counter*2)+1] = projectedPoints[(i*2)+1];
        //cout << __func__ << "[" << counter << "] x:" << afterFiltering[counter*2] << " :y: " << afterFiltering[(counter*2)+1] <<endl; 
        ++counter;
    }
    return counter;
}

/* Converting front to top view coordinate */
/* Consider that points are using middle image coordinate */
/* TODO: add extra checking if outside the drawing box */
bool FormSubvisionImpl::frontToTopView(double inPoints[2], int outPoints[2], int widthDisplay)
{
    double in[] = {TOPLEFT2CENTER_X(inPoints[0]), TOPLEFT2CENTER_Y(inPoints[1])};
    cortex.projectPoints(in, 1, outPoints);                
    /*
    if (ANDREWDEBUG) 
        cout << "middle coordinate in:"<< inPoints[0] << ":" << inPoints[1]<< " out:" << outPoints[0] << ":" << outPoints[1] <<endl;
    */
    int temp = outPoints[0];
    outPoints[0] = outPoints[1];
    outPoints[1] = temp + widthDisplay/2;
    return true;
}

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


// Helper function to calcuate mean and variance
double calculateMean(list<double> data) { 
    if (data.size() == 0) {  
        return 0;
    }
        
    double sum = 0;    
    list<double>::iterator memberItr;
    for (memberItr = data.begin(); 
            memberItr != data.end();
            ++memberItr) {
        
        sum += *memberItr;                
    }    
    return sum / data.size();
}

double calculateVariance(list<double> data) { 
    if (data.size() == 0) {
        return 0;
    }
    
    double mean = calculateMean(data);
    double sum = 0;
    list<double>::iterator memberItr;
    for (memberItr = data.begin(); 
            memberItr != data.end();
            ++memberItr) {
        
        sum += (*memberItr - mean) * (*memberItr - mean);                
    }
    return sum / data.size();
}
