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
 * $Id$
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef CPLANEDISPLAY_H
#define CPLANEDISPLAY_H
#include "FormCPlaneDisplay.h"
#include "CPlane.h"
#include "SimpleServer.h"

#include <qpixmap.h>
#include <qobject.h>
#include <qtimer.h>
#include <qpaintdevice.h>
#include <qimage.h>
#include <qlabel.h>
#include <qsocket.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qfile.h>

#include <string>
#include <vector>
#include <fstream>

using namespace std;

struct OPlane{
    int objNum;
    int x;
    int y;
    int w;
    int h;
    int d;
};

class CPlaneDisplay : public FormCPlaneDisplay
{
    Q_OBJECT

public:
    CPlaneDisplay( const char * logFileName, const char* saveFileName,
                   QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    bool loadFromFile(const string &fileName);
    //void paintEvent(QPaintEvent *paintEvent);
    int playBackLength();
    ~CPlaneDisplay();

public slots:
    void slotBack();
    void slotStop();
    void slotForward();
    void slotAnimate();
    void slotConnect();
    void slotListen();
    void slotToggleLogging();
    bool close(bool);
    void slotChangeCurrentObject(int item);
    
    void slotChangeUpdatingCPlane( bool state );

//TODO do i need to do this?
private slots:
    void playNext();
    void readRemoteCPlane();
    void newConnection(ClientSocket *);

private:
    void displayCPlane(char *cplaneComressed);
    void displayObjects(int *objectData, int pan, int tilt);
    void showFrame();
    void parseSensorsSanityObjects(char *data, int size);
    void parseCPlaneBuffer(char *buf);
    int* displayObjectFieldValues(int *p);
    void displaySeenObjectOnGUI(vector<OPlane> &seenObject, int pan, int tilt);
    void drawBoundingBox(const OPlane &object);
    void drawRotatedBoundingBox(const OPlane &object,double roll);
    void showObjectLabels(vector<OPlane> &seenObject);
	void convertXYCPlane2Image(int &x, int &y);

    string version;
    vector<CPlane *> frames;
    QPixmap *pixmap;
    int cplaneWidth, cplaneHeight;
    int currentFrame;
    QSocket *clientSocket;
    SimpleServer *serverSocket;
    QTimer *timer;
    bool bLogToFile;
    QFile fLogFile;
    string logFileName;
    int numObject;
    int currentObject;

    vector<QCheckBox*> objectCheckBoxes ;
};

#endif // CPLANEDISPLAY_H
