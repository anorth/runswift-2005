/*
  
   Copyright 2004 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).
   
   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
   must be included in that publication.
   
   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   
 */

/*
 * Last modification background information
 * $Id:$
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "CPlaneDisplay.h"
#include "CPlane.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

#include <qdatetime.h>
#include <qtextstream.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpainter.h>

using namespace std;

static bool bDebugParsing = false;
bool bUpdateCPlane = true;//HACK: should put into class variable instead of global
int lastPan = 0;
int lastTilt = 0;
int *lastObjectData; //to display when not updating

vector<OPlane> seenObject;

enum ReadingStateEnum{
    rsNoReading,
    rsCPlaneSize,
    rsCPlane,
    rsSensors,
    rsSanity,
    rsNumObject,
    rsObjects,
    rsAllState
};

int stateSize[rsAllState] = {
    0,
    4,
    -1, // not detemined yet, depends on other state
    12,
    28,
    4,
    -1
};

// Objet labels indexed by their object id (objNum)
enum { MAX_LABEL = 16 }; // max length of a label
char * objectLabels[] = {
    "Ball", "Blue Goal", "Yellow Goal", "Blue on Pink", "Pink on Blue",
    "Null", "Null", "Yellow on Pink", "Pink on Yellow", "Red Dog 1",
    "Red Dog 2", "Red Dog 3", "Red Dog 4", "Blue Dog 1", "Blue Dog 2",
    "Blue Dog 3", "Blue Dog 4"
};

// The ReadingStateEnum (i.e. index into stateSize) currently being read.
int readingState;

/* 
 *  Constructs a CPlaneDisplay which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
CPlaneDisplay::CPlaneDisplay(const char *saveFile, const char* readFile,
                             QWidget* parent,  const char* name, WFlags fl )
    : FormCPlaneDisplay( parent, name, fl ), fLogFile(saveFile) {
    
    if (readFile) {
        loadFromFile(readFile);
    }
    
    this->logFileName = saveFile;
    ////ostringstream ss;
    ////ss << "Save to log file (" << logFileName << ")?";
    ////chbLogCPlane->setText(ss.str());
    
    pixmap = 0;
    currentFrame = 0;
    timer = new QTimer();
    clientSocket = new QSocket();
    serverSocket = 0; 
    readingState = rsNoReading;
    slotListen();
    objectCheckBoxes.push_back(chbBall);
    objectCheckBoxes.push_back(chbBlueGoal);
    objectCheckBoxes.push_back(chbYellowGoal);
    objectCheckBoxes.push_back(chbBluePink);
    objectCheckBoxes.push_back(chbPinkBlue);
    objectCheckBoxes.push_back(NULL);
    objectCheckBoxes.push_back(NULL);
    objectCheckBoxes.push_back(chbYellowPink);
    objectCheckBoxes.push_back(chbPinkYellow);
    objectCheckBoxes.push_back(chbRed1);
    objectCheckBoxes.push_back(chbRed2);
    objectCheckBoxes.push_back(chbRed3);
    objectCheckBoxes.push_back(chbRed4);
    objectCheckBoxes.push_back(chbBlue1);
    objectCheckBoxes.push_back(chbBlue2);
    objectCheckBoxes.push_back(chbBlue3);
    objectCheckBoxes.push_back(chbBlue4);
    
    cbxObject->clear();
    for (unsigned int i=0;i<objectCheckBoxes.size();i++)
        if (objectCheckBoxes[i] != NULL){
        cbxObject->insertItem(objectCheckBoxes[i]->text());
    }
    else 
        cbxObject->insertItem("NULL");
    
    currentObject = cbxObject->currentItem();
    //playNext();
}

/*
   void CPlaneDisplay::paintEvent(QPaintEvent *paintEvent){
   paintEvent = paintEvent;
   if (pixmap != NULL) 
   bitBlt((QWidget*)lblCPlane,0,0,pixmap);
   }
 */

/*  
 *  Destroys the object and frees any allocated resources
 */
CPlaneDisplay::~CPlaneDisplay(){
    cout << " Destroying CPlaneDisplay" << endl;
    delete (timer);
    for (unsigned int i=0;i<frames.size();i++){
        delete(frames[i]);
    }
    delete (pixmap);
    delete (clientSocket);
    delete (serverSocket);
}

// FIXME: this needs updating to parse the entire CPlane data from the
// file including objects, sensors etc.
bool CPlaneDisplay::loadFromFile(const string &fileName){
    QTime clock;
    int readingTime = 0;
    int imagingTime = 0;
    
    ifstream f(fileName.c_str());
    if (! f.is_open()){
        cout << "Error opening file " << endl;
        return false;
    }
    
    f >> version;
    CPlane *cplane = NULL;
    while (1){
        clock.start();
        cplane = new CPlane();
        if (cplane->parseCPlaneFromStream(f))
            frames.push_back(cplane);
        else
            break;
        
        readingTime += clock.elapsed();
        clock.start();
        cplane->getImage();
        imagingTime += clock.elapsed();
        
    }
    cout << "Reading Time = " << readingTime << endl;
    cout << "Imaging Time = " << imagingTime << endl;
    
    return true;
}

int CPlaneDisplay::playBackLength(){
    return frames.size();
}


/*
 * public slot
 */
void CPlaneDisplay::slotBack() {
    qWarning( "CPlaneDisplay::slotBack() not yet implemented!" );
}

/*
 * public slot
 */
void CPlaneDisplay::slotForward() {
    qWarning( "CPlaneDisplay::slotForward() not yet implemented!" );
}

/*
 * public slot
 */
void CPlaneDisplay::slotAnimate() {
    qWarning( "CPlaneDisplay::slotAnimate() ");
    if (timer)
        delete(timer);
    timer = new QTimer();
    if (currentFrame == playBackLength())
        currentFrame = 0;
    connect(timer,SIGNAL(timeout()),this,SLOT(playNext()));
    timer->start(40, false);
}

void CPlaneDisplay::playNext(){
    if ( (unsigned int) currentFrame >= frames.size() ){
        slotStop();
        return;
    }
    cout << "Current = " << currentFrame << endl;
    int w = lblCPlane->width();
    int h = lblCPlane->height();        
    pixmap = new QPixmap(frames[currentFrame]->getImage()->scale(w,h) );
    //TODO try repaint()
    bitBlt((QWidget*)lblCPlane,0,0,pixmap);
    delete(pixmap);
    currentFrame += 1;
}

void CPlaneDisplay::slotStop(){
    timer->stop() ;     
}

void CPlaneDisplay::slotConnect(){
    
    cout << "slotConnect" << endl;
    QString ip = ledIP->text();
    cout << ip << endl;
    int port = ledPort->text().toInt();
    connect( clientSocket, SIGNAL(readyRead()),
             this, SLOT(readRemoteCPlane()) );
    clientSocket->connectToHost(ip,port);
    cout << "state = " << clientSocket->state() << endl;
    cout << "peerName = " << clientSocket->peerName() << endl;
    cout << "Connection established , local port = " << clientSocket->port() << endl;
}

void CPlaneDisplay::slotListen(){
    cout << "CPlaneDisplay::slotListen" << endl;
    int port = ledPort->text().toInt();
    cout << "port " << port << endl;
    if (serverSocket)
        delete(serverSocket);
    serverSocket = new SimpleServer(port);
    connect( serverSocket, SIGNAL(newConnect(ClientSocket*)),
             this, SLOT(newConnection(ClientSocket*)) );
    cout << "I'm listening on " << port << endl;
    
}

void CPlaneDisplay::newConnection(ClientSocket *socket){
    cout << "New connection " << endl;
    clientSocket = socket;
    connect( socket, SIGNAL(readyRead()),
             this, SLOT(readRemoteCPlane()) );
    
}

/*** CPlane reading and displaying ***/

static const int BUF_SIZE = CPlane::maximumCPlaneSize;

// This uses a simple page flipping display. One buffer is displayed
// while the other is read into memory. Then they swap. Given the current
// CPlane implementation this is probably not even necessary, but is
// flexible if more performance is required in the future.
static char buf1[BUF_SIZE], buf2[BUF_SIZE];
static int bufpos1 = 0, bufpos2 = 0;

// Returns the number of bytes that have been read into the given buffer
int getBufferFilled(const char *buf){
    if (buf == buf1)
        return bufpos1;
    else if (buf == buf2)
        return bufpos2;
    
    cerr << "ERROR: unknown buffer" << endl;
    return 0;
}

// Ensures that at least howmany bytes have been read into the specified buffer
// then returns true. Returns false if they cannot be read now.
bool fillBuffer(QSocket *clientSocket, char *buf, int howmany) {
    int filled = getBufferFilled(buf);
    
    if (filled >= howmany)
        return true;
    
    if ( (filled + (int) clientSocket->bytesAvailable()) >= howmany ){
        int needed = howmany - filled;
        QDataStream ts(clientSocket);
        if (buf == buf1) {
            ts.readRawBytes(buf + bufpos1, needed);
            bufpos1 += needed;
        } else if (buf == buf2) {
            ts.readRawBytes(buf + bufpos2, needed);
            bufpos2 += needed;
        } else {
            cerr << "ERROR: fillBuffer: unknown buffer" << endl;
        }
        return true;
    }
    return false;
}

// Resets the specified buffer
void clearBuffer(const char* buf){
    if (buf == buf1) {
        bufpos1 = 0;
    } else if (buf == buf2) {
        bufpos2 = 0;
    } else {
        cerr << "ERROR: clearBuffer: unknown buffer" << endl;
    }
}

/* Parses that part of the given buffer than has just been read into
 * memory, as indicated by readingState.
 */
void CPlaneDisplay::parseCPlaneBuffer(char* buf){
    //if (bDebugParsing)
    //    cout << "Parsing state " << readingState << endl;
    
    switch (readingState){
    case rsCPlaneSize:
        {
            int cpSize = *(int *) buf;
            if (bDebugParsing)
                cout << " Got CPlane size = " << cpSize << endl;
            stateSize[rsCPlane] = cpSize;
            break;
        }
    case rsCPlane:
        {
            if (bDebugParsing)
                cout << "Display cplane size = " << stateSize[rsCPlane] << endl;
            
            if (bUpdateCPlane){
                // Read and parse the CPlane data into the class image variable
                displayCPlane(buf);
            }
            break;
        }
    case rsSensors:
        {
            // not implemented
            if (bDebugParsing)
                cout << "Reading sensor (" << stateSize[rsSensors] << ")"
                        << endl;
            break;
        }
    case rsSanity:
        {
            // not implemented
            if (bDebugParsing)
                cout << "Reading sanity (" << stateSize[rsSanity] << ")"
                        << endl;
            break;
        }
    case rsNumObject:
        {
            numObject = *(int *) (buf + stateSize[rsCPlaneSize]
                                  + stateSize[rsCPlane] + stateSize[rsSensors]
                                  + stateSize[rsSanity]);
            if (bDebugParsing)
                cout << " Got numobjects = " << numObject << endl;
            stateSize[rsObjects] = numObject * ONE_OBJ_SIZE
                                   + NUM_ROBOT_DATA * INT_BYTES;
            break;
        }
    case rsObjects:
        {
            if (bDebugParsing)
                cout << "Reading objects and robot data ("
                        << stateSize[rsObjects] << ")" << endl;
            if (bUpdateCPlane){
                int *p = (int *) (buf + stateSize[rsCPlaneSize]
                                  + stateSize[rsCPlane] + stateSize[rsSensors]
                                  + stateSize[rsSanity] + stateSize[rsNumObject]);
                // overlay object data onto the image
                int pan = *p++;
                pan = pan;
                int tilt = *p++;
                tilt = tilt;
                displayObjects(p, pan, tilt);
            }
            break;
        }
    default:
        if (bDebugParsing)
            cout << " Not a valid reading state" << readingState << endl;
    }
}

// This procedure is called when data is available at the
// server socket.
void CPlaneDisplay::readRemoteCPlane(){
    QDataStream ts(clientSocket);
    
    if (readingState == rsNoReading){
        readingState = rsCPlaneSize;
    }
    
    static char *buf = buf1;
    static int nBytes = stateSize[rsCPlaneSize];
    while ( fillBuffer(clientSocket, buf, nBytes) ) {
        parseCPlaneBuffer(buf);
        readingState = ReadingStateEnum ( (int) readingState + 1 );
        nBytes += stateSize[readingState];
        if (readingState == rsAllState) {
            // show the frame
            showFrame();
            readingState = rsCPlaneSize;
            nBytes = stateSize[rsCPlaneSize];
            
            // log buffer to file. This is just the data stream as received
            if (bLogToFile){
                QDataStream stream(&fLogFile);
                stream.writeRawBytes(buf, getBufferFilled(buf));
            }
            
            // switch buffers
            clearBuffer(buf);
            if (buf == buf1) {
                buf = buf2;
            } else {
                buf = buf1;
            }
        }
    }
}

/*
void CPlaneDisplay::parseSensorsSanityObjects(char *data, int size){
    if  (size < (NUM_SENSOR_COORDS + NUM_SANITY_COORDS + 2) * INT_BYTES)
        return ; //Not enough data
    cout << "Parsing " << size << endl;
    int t;
    for (int i = 0; i < NUM_SENSOR_COORDS ; i++){
        t = *(int *)data ;
        cout << "Got sensor " << i << " = " << t << endl;
        data += INT_BYTES; // = 4
    }
    
    for (int i = 0; i < NUM_SANITY_COORDS ; i++){
        t = *(int *)data ;
        //cout << "Got sanity " << i << " = " << t << endl;
        data += INT_BYTES; // = 4
    }
    t = *(int *)data ;
    cout << "panx = " << t << endl;
    data += INT_BYTES; // = 4
    
    t = *(int *)data ;
    cout << "tilty = " << t << endl;
    data += INT_BYTES; // = 4
    
    int bytesLeft = size - (NUM_SENSOR_COORDS + NUM_SANITY_COORDS + 2 ) * INT_BYTES;
    int num_obj = 0;
    cout << "Bytes Left " << bytesLeft << endl;
    if (bytesLeft % INT_BYTES == 0)
        num_obj = bytesLeft / INT_BYTES;
        
    cout << "Getting " << num_obj << " objects " << endl;
    
    for (int i = 0; i < num_obj ; i++){
        //cout << "Got object " << i << " : " << endl;
        //cout << "  (obj_index, x, y, width, height, d) = " ;
        for (int j = 0 ; j < NUM_INT_COORDS; j++){
            t = *(int *)data ;
            //cout << t << "  " ;
            data += INT_BYTES; // = 4
        }
        //cout << endl;
    }
    
    
}

*/

void CPlaneDisplay::displayCPlane(char *cplaneCompressed){
    static CPlane cplane;
    static int cc = 0;
    static const int maxEle = 10; // was 50 -- weiming 04/01/10
    static int timeStamp[maxEle];
    static QTime qt;
    
    if (bDebugParsing)
        cout << "Displaying " << endl;
    bool ok = cplane.parseCPlaneFromCompressedPChar(cplaneCompressed);
    if (bDebugParsing)
        cout << "Parsed " << endl;
    if (ok){
        if (cc == 0){
            qt.start();
        }
        else{
            timeStamp[cc%maxEle] = qt.elapsed();
            if (cc > maxEle){
                int timePerMaxEle = timeStamp[cc%maxEle] - timeStamp[(cc+1)%maxEle];
                lcdFrameRate->display(double( maxEle * 1000.0) / timePerMaxEle);
            }
        }
        ++cc;
        
        // These are set here to assist other methods drawing over the pixmap
        // later
        cplaneWidth = cplane.getWidth();
        cplaneHeight = cplane.getHeight();        
        
        if (pixmap)
            delete pixmap;
        pixmap = new QPixmap(cplane.getImage()->scale(
                lblCPlane->width(),lblCPlane->height()));
        
    }
    else{
        cout << "Not valid cplane" << endl;
    }
    
}

// Puts the specified image up on the screen
void CPlaneDisplay::showFrame() {
    bitBlt((QWidget*)lblCPlane,0,0,pixmap);
}

string intToString(int x){
    ostringstream ss;
    ss << x;
    return ss.str();
}

//Displays information about recognized objects on the given image. This
//requires numObject to be set correctly. objectdata is a pointer into
//a buffer containing the object data.
void CPlaneDisplay::displayObjects(int *objectData, int pan, int tilt){
    lastPan = pan;
    lastTilt = tilt;
    lastObjectData = objectData;
    seenObject.clear();
    
    for (int i = 0; i < numObject ; i++){
        OPlane oplane;
        oplane.objNum = *objectData++;
        oplane.x = *objectData++;
        oplane.y = *objectData++;
        oplane.w = *objectData++;
        oplane.h = *objectData++;
        oplane.d = *objectData++;
        seenObject.push_back(oplane);
    }
    
    // draw labels over the image
    showObjectLabels(seenObject);
    
    // Show recognised object indicators
    displaySeenObjectOnGUI(seenObject,pan,tilt);
}

void CPlaneDisplay::convertXYCPlane2Image(int &x,int &y){
	x = x * pixmap->width() / CPLANE_WIDTH;
	y = y * pixmap->height() / CPLANE_HEIGHT;
}

void CPlaneDisplay::drawBoundingBox(const OPlane &object){
	QPainter q(pixmap);
	q.setPen(QColor(0,167,153));
	int x[4] = { object.x , object.x + object.w, object.x + object.w , object.x };
	int y[4] = { object.y , object.y           , object.y + object.h , object.y + object.h };
	for (int i=0;i<4;i++)
		convertXYCPlane2Image(x[i],y[i]);
	for (int i=0;i<4;i++)
		q.drawLine( x[i],y[i],x[ (i+1) % 4 ] ,y[ (i+1) % 4] );
}

void CPlaneDisplay::drawRotatedBoundingBox(const OPlane &object,double roll){
	QPainter q(pixmap);
	q.setPen(QColor(0,167,153));
	int x[4] = { object.x , object.x + object.w, object.x + object.w , object.x };
	int y[4] = { object.y , object.y           , object.y + object.h , object.y + object.h };
    double sinRoll = sin(roll);
    double cosRoll = cos(roll);
    int centerX = CPLANE_WIDTH / 2;
    int centerY = CPLANE_HEIGHT / 2;
	for (int i=0;i<4;i++){
        x[i] -= centerX;
        y[i] -= centerY;
        double newX = x[i] * cosRoll - y[i] * sinRoll;
        double newY = x[i] * sinRoll + y[i] * cosRoll;
        x[i] = (int)newX + centerX;
        y[i] = (int)newY + centerY;
		convertXYCPlane2Image(x[i],y[i]);
    }
	for (int i=0;i<4;i++)
		q.drawLine( x[i],y[i],x[ (i+1) % 4 ] ,y[ (i+1) % 4] );
}

void CPlaneDisplay::displaySeenObjectOnGUI(vector<OPlane> &seenObject, int panMicro,int tiltMicro){
    for (unsigned int i=0;i<objectCheckBoxes.size();i++)
        if (objectCheckBoxes[i] != NULL)
            objectCheckBoxes[i]->setChecked(false);
    bool currentObjectSeen = false;
    double pan = MICRO2RAD(panMicro);
    double tilt = MICRO2RAD(tiltMicro);
    double roll = asin(sin(pan) * sin(-tilt));
    for (unsigned int i = 0;i< seenObject.size();i++){
        int objNum = seenObject[i].objNum;
	if (objNum == 1 || objNum == 2)  //TODO : use proper constants (1 is yellowGoal and 2 is blueGoal)
		drawRotatedBoundingBox(seenObject[i],roll);
	else
		drawBoundingBox(seenObject[i]);
        if (objNum == currentObject){
            currentObjectSeen = true;
            ledValueX->setText(intToString(seenObject[i].x).c_str());
            ledValueY->setText(intToString(seenObject[i].y).c_str());
            ledValueW->setText(intToString(seenObject[i].w).c_str());
            ledValueH->setText(intToString(seenObject[i].h).c_str());
            ledValueD->setText(intToString(seenObject[i].d).c_str());
        }
        //cout << " Got object " << objNum << " y=" << x << " y=" << y << " w=" << w << " h=" << h << " d=" << d << endl;
        if (objNum >= 0 && objNum < (int)objectCheckBoxes.size() && objectCheckBoxes[objNum] != NULL)
            objectCheckBoxes[objNum]->setChecked(true);
        else
            cout << "Invalid object number " << objNum << endl;
    }
    if ( !currentObjectSeen){
        ledValueX->setText("");
        ledValueY->setText("");
        ledValueW->setText("");
        ledValueH->setText("");
        ledValueD->setText("");
    }
}

// Draws object labels over the objects on the image
void CPlaneDisplay::showObjectLabels(vector<OPlane> &seenObject) {
    double scalex = lblCPlane->width() / (double)cplaneWidth;
    double scaley = lblCPlane->height() / (double)cplaneHeight;
    
    QPainter *painter = new QPainter(pixmap, false);
    vector<OPlane>::iterator itr = seenObject.begin();
    while (itr != seenObject.end()) {
        int x = itr->x + (itr->w / 2);
        x = (int)(x * scalex);
        int y = itr->y + (itr->h / 2);
        y = (int)(y * scaley);
        
        // draw label on the image
        ostringstream ss;
        if (x < lblCPlane->width() / 2) {
            ss << "* " << objectLabels[itr->objNum];
            painter->drawText(x, y, ss.str());
        } else {
            // FIXME: shift so right edge is at x,y, trailing *
            //ss << objectLabels[itr->objNum] << " *";
            ss << "* " << objectLabels[itr->objNum];
            painter->drawText(x, y, ss.str());
        }
        
        ++itr;
    }
    
    delete painter;
}

bool CPlaneDisplay::close(bool alsoDelete){
    cout << "CPlaneDisplay closing" << endl;
    return FormCPlaneDisplay::close(alsoDelete);
}

void CPlaneDisplay::slotToggleLogging(){
    if (bLogToFile) {
        bLogToFile = false;
        fLogFile.close();
        cout << "Log file saved" << endl;
    } else {
        if (fLogFile.open(IO_WriteOnly)){
            cout << logFileName << " file opened" << endl;
            QTextStream fs(&fLogFile);
            fs << "v1234" << endl;
            bLogToFile = true;
        }
        else{
            cout << "*** Error openning file " << logFileName << endl;
            bLogToFile = false;
        }
    }
}

void CPlaneDisplay::slotChangeCurrentObject(int item)
{
    cout << "CPlaneDisplay::slotChangeCurrentObject : " << item << endl;
    currentObject = item;
    if (! bUpdateCPlane){
        displaySeenObjectOnGUI(seenObject,lastPan, lastTilt);
    }
}


void CPlaneDisplay::slotChangeUpdatingCPlane( bool state )
{ 
    bUpdateCPlane = !  state;
    cout << "Stop Updating CPlane ? " << bUpdateCPlane << endl;
    lblCPlane->setPixmap(*pixmap);
    
}
//#include "CPlaneDisplay.moc"
//#include "SimpleServer.moc"
