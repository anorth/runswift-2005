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



#include <math.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qiodevice.h>
#include <qsocket.h>
#include <qlayout.h>
#include <qwaitcondition.h>

#include <qstring.h>
#include <qgl.h>
#include <fstream>
#include <sstream>

#include "FormSubvision.h"
#include "dlgConnect.h"
#include "DPixmap.h"
#include "FrameReader.h"

#include "../robolink/robolink.h"
#include "../../robot/vision/SubVision.h"
#include "../../robot/vision/SubObject.h"
#include "../../robot/vision/VisualCortex.h"
#include "../../robot/vision/SanityChecks.h"
#include "../../robot/vision/Obstacle.h"
#include "../../robot/vision/gps.h"

//MAXGAUSSIANS same with maxGaussians in gps
#define MAXGAUSSIANS 16 


enum {
    // Dimensions of the image from the BFL files
    IMAGE_COLS = 208,
    IMAGE_ROWS = 160,
};

struct DogEdgeTable {
    vector< vector<int> > dogsEdgePointsQuantity;
    vector< vector<double> > dogsEdgePointsWeight;
    double x,y,h;
};

struct FlagObject {
    //int frameNum;
    int flags[VOB_COUNT];
};

class FormSubvisionImpl : public FormSubvision
{
    Q_OBJECT

    public:
        FormSubvisionImpl(QApplication* app, QWidget* parent = 0,
                            const char* name = 0, WFlags fl = 0 );
        ~FormSubvisionImpl();

    public slots:
        void slotFileOpen(void);
        void slotConnectDialog(void);
        void slotConnect(QString host, unsigned short port);
        void slotDisconnect();
        void slotSetMacAddress();
        void slotSliceCoordsChanged(void);
        void slotTogglePt2Relative(bool rel);
        void slotRedraw(void);
        void slotNextFrame(void);
        void slotPrevFrame(void);
        void slotPlay(void);
        void slotPlayFast(void);
        void slotAnalyseVariance(void);
        void slotDisplayPointsDebug(void);
        void slotDisplayPoint(void);
        void slotGPSDirectUpdate(void);
        void slotGaussianDisplay(void);
        void slotGPS_CP_Gaussians(void);
        void slotRefreshImage(void);
        void slotPopup(void);
        void slotDPixmapRedraw(void);
        void slot_EC_TEW_Default(void);
        void slotGPSGradientAscent(void);
        void slotModifyFlag(void);
        void slotSavingFlag(void);
        void slotLoadingFlag(void);
        void slotDisplayFlag(void);
        void slotCopyFlagFromObj(void);
        void slotDisplayHough(void);
        void slotInitPink(void);
        
    protected:
        void keyPressEvent(QKeyEvent* e);
        QGLWidget *glwidget;

    private:
        int readFrame(void);
        void parsePWalkInfo(bool hdr);
        void parseCamOrient(bool hdr);
        void parseYUV(bool hdr);
        void parseCPlane(bool hdr, int size);
        void parseSubCPlane(bool hdr, int size);
        void parseFeatures(bool hdr, int size);
        void parseObjects(bool hdr);
        void parseInsanities(bool hdr);
        void parseAtomicAction(void);
        void parseGPSInfo(void);
        void parsePinkObject(void);
        void parseGaussians(int size);
        
        void sampleImage();
        void clearData(void);
        void drawCPlane();
        void displayImage(void);
        void drawSlice(void);
        void displayFeatures();
        void displayObjects();
        void displayScanlines(void);
        void displayObstacles(void);
        void displayGrabTurnKickInfo(void);
        void displayGPSInfo(void);
        void calculateHistogram(void);      
        void initRobotCondition(void);
        void calibrateEdge();
        void calculateEdgePointAllFrame(void);
        bool frontToTopView(double inPoints[2], int outPoints[2],
                            int widthDisplay);
        void copyVobToCortex(void);
        
        //print out to a ofstream
        void GPSOutputSelfData(ofstream &file);
        
        /* return the size after filtering */
        int projectAndFilterPoints(double *inPoints, int *afterFiltering, int no_of_points, int clip_X = INT_MAX, int clip_Y = INT_MAX);
        int projectAndFilterPoints(double *inPoints, double *afterFiltering, int no_of_points, int clip_X = INT_MAX, int clip_Y = INT_MAX);
        
        QApplication* app;
        dlgConnect connect;
        DPixmap dpixmap;
        
        VisualCortex cortex;
        SubVision& subvision;
        SubObject& subobject;
        GPS gps; //for gps info update
        friend class FrameReader;
        FrameReader framereader;

        bool shouldSampleImage;         // true if YUV should be sampled
        bool online;                    // true if input is realtime
        QIODevice* input;               // input stream to read
        QSocketDevice* sock;
        bool alreadyGotHeader;          // read the header for next frame?
        vector<int> frames;             // file offsets of the first
                                        // data for each frame
        bool gotCPlane, gotSubCPlane, gotYUV, gotCamOrient,
             gotSubVision, gotSubObject, gotInsanities, 
             gotPWalk, gotGPSInfo, gotPinkObject,
             gotAtomicAction, gotGaussians;

        QImage* image;                  // image as parsed
        QPixmap* pixmap;                // QPixmap displayed (scaled)
        int currentFrame;               // index into frames

	int lastMotionFrameNo;          // index of the last frame
	int lastStep;                   // last PWalk Step No.

        struct rlnk_pwalksensors pWalkInfo; // PWalk position info
	struct rlnk_atomicaction atomicAction; 	// Atomic Action Info
        struct rlnk_gpsinfo gpsinfo;//gps info
        struct rlnk_gaussians allgaussians[MAXGAUSSIANS];
        int dogGaussiansSize;
        
        //Localisation Challenge
        VisualObject pinkObjects[MAX_NO_OF_PINK_OBJECTS];
        int num_of_pink;
        
        // YUV values for analysis. This is the same format as captured
        // by the robot camera: rows of IMAGE_COLS pixels, the rows
        // ordered: Y1, U, V, Y2, Y3, Y4. See the wiki for possible information
        // on the position of the Y values.
        uchar *yuv;
        uchar *yplane, *uplane, *vplane; // convenience pointers into yuv
        uchar cplane[CPLANE_WIDTH * CPLANE_HEIGHT];

        // the members below are for when we are only displaying - rather than 
        // calculating - the vision information
        struct cam_orient camorient;
        vector<VisualFeature> features;
        vector<VisualFeature> obstacleFeatures;
        VisualObject vBall;
        VisualObject vBeacons[NUM_BEACONS];
        VisualObject vGoals[NUM_GOALS];
        unsigned char insanities[INSANITY_COUNT/8 + 1];


        point histPoint1, histPoint2; // histogram slice line
        int pt2relative[2]; // Relative position of point 2 to point 1
        
        static const bool DEBUG = false;        
        
        /* Edge calibration */
        //Array size for the field top view
        static const int ARRAY_SIZE_X = FIELD_WIDTH + 2 * VisualCortex::offsetX;
        static const int ARRAY_SIZE_Y = FIELD_LENGTH + 2 * VisualCortex::offsetY;
        
        int lineCounter[ARRAY_SIZE_X][ARRAY_SIZE_Y];
        int lineOutsideCounter;
        vector< DogEdgeTable > tablePerFile;
        void resetLineCounter(void);
        void DPixmapDrawField(
            QPainter &p, 
            double convert, 
            double offsetDisplayX, 
            double offsetDisplayY);
        void DPixmapDrawPointProjection(
            QPainter &p, 
            double convert, 
            double offsetDisplayX, 
            double offsetDisplayY,
            double xpos,
            double ypos,
            double hpos);
        void DPixmapDrawProbability(
            QPainter &p, 
            double convert, 
            double offsetDisplayX, 
            double offsetDisplayY);
        void DPixmapDrawDog(
            QPainter &p, 
            double convert, 
            double offsetDisplayX, 
            double offsetDisplayY,
            double xpos,
            double ypos,
            double hpos);
      std::map <int, FlagObject> flagobjects;
      bool checkFlag(FlagObject &errorMap);
      void WritingAFlag(ostream &fp_out, const FlagObject& fo);
};


