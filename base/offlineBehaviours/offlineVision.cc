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


#include <string>
#include <utility>
#include <vector>
#include <list>

#include "../../robot/share/Common.h"
#include "../../robot/share/SwitchBoard.h"
#include "../../robot/share/WirelessSwitchboard.h"
#include "../../robot/share/VisionDef.h"
#include "../../robot/share/WirelessTypeDef.h"
#include "../../robot/share/OdometerDef.h"
#include "../../robot/vision/gps.h"
#include "../../robot/vision/CommonSense.h"
#include "../../robot/vision/PlayerName.h"
#include "../../robot/vision/def.h"
#include "../../robot/vision/Robolink.h"
#include "../../robot/vision/Obstacle.h"
#include "../../robot/vision/Odometer.h"
#include "../../robot/vision/PyEmbed.h"
#include "../../robot/vision/InfoManager.h"
#include "../../robot/vision/Profile.h"
#include "../../robot/behaviours/Behaviours.h"
//#include "../../robot/share/SharedMemoryDef.h"
//#include "../../robot/share/WalkLearningParams.h"
//#include "../../robot/../share/SharedMemoryDef.h"

#include "../../robot/vision/offlineVision.h"

using namespace std;

//#define WORLD_MODEL_DEBUG
//#define FRAMECOUNT

#if 0
    void ResultCamera(const ONotifyEvent &);
    void GPSMotionUpdateAssert(const ONotifyEvent &event);
    void OdometerAssert(const ONotifyEvent &event);
    void ExternalCommand(const ONotifyEvent &event);
    void TeammateWM(const ONotifyEvent &event);
    void TeamTalk(const ONotifyEvent &event);
    //void LatencyTesting(const ONotifyEvent& event);
    void ResultSensor(const ONotifyEvent &event);
    void SetPAgent(const ONotifyEvent &event);
    void GameData(const ONotifyEvent &event);
    void sendWirelessMessage(int msgType,
                             int dataType,
                             int dataLength,
                             void *sharedMem,
                             const void *msg);
    void sendWirelessMessage(const string &str);
#endif

static int gotTMWM;

static CommonSense sensors;
static VisualCortex vision;
static GPS gps;
static Obstacle obstacle;

double dzero = 0;
int izero = 0;
bool zero = 0;
static struct PWalkInfoStruct PWalkInfo = {
    &izero, &izero, &izero, &izero, &izero,
    &dzero, &dzero, &dzero, &dzero, &dzero,
    &zero
};

static int lastWalkType; // The walk type in the previous frame

//static long currentFrameID;
//static long lastFrameID;
//static long lastKickFrameID;
    
// timestamp of the start of this vision frame
//SystemTime frameStartTime;

// to slow the transferate
//static int cplaneCounter;
//static int oplaneCounter;
//static int displayEnvironmentCounter;
//static int shareEnvironmentCounter;

// store information of cplane
//unsigned char compressedCPlane[COMPRESSED_CPLANE_SIZE + SENSOR_SIZE + SANITY_SIZE + OBJ_SIZE];

// this counter will be sent across to the base to record the delay occured
//int delayTestingCounter;

//char dataResultCamera[100];

static int frameCounter;
static int wait4settle;        // for debugging
static bool powerIsOn;
static bool sharedMemIsSetup;

static bool camNotSet;

//static bool forceCplane;

//static unsigned long ethernetMACaddress;	// holds last 4 bytes

// Called by VisPolicy through PyEmbed to initialise offline vision
void initOfflineVision(void) {

    Behaviours::InitBehaviours();

	powerIsOn = false;
	sharedMemIsSetup = false;
	camNotSet = true;
	wait4settle = 30;
	//sharedrc = NULL;
	//sharedDebugMemory = NULL;
	frameCounter = 0;
	//LastframePt = 0;
	//Last_WM_Pt = 10 * 125;
    lastWalkType = -1;

    PyBehaviours::sensors = &sensors;
    PyBehaviours::vision = &vision;
    PyBehaviours::gps = &gps;
    PyBehaviours::obstacle = &obstacle;
    //PyBehaviours::transmission = this;
    PyBehaviours::PWalkInfo = &PWalkInfo;
    
    Behaviours::sensors = &sensors;
    Behaviours::vision = &vision;
    Behaviours::gps = &gps;
    //Behaviours::transmission = this;
    
    gInfoManager.setSensors(&sensors);
    gInfoManager.setGPS(&gps);
    gInfoManager.setVisualCortex(&vision);

	//cplaneCounter  = 0;
	//oplaneCounter  = 0;
	//displayEnvironmentCounter = 0;
	//shareEnvironmentCounter = 0;

	//delayTestingCounter = 0;

	gotTMWM = 0;
    

}

#if 0
// Called by VisPolicy through PyEmbed to process a frame. This emulates
// resultCamera in Vision.cc
void processFrame() {
	// Setting goal
	if (gps.goalNotSet) {
		gps.SetGoals(true); // be Red
		//sendPlayerNumber(Behaviours::PLAYER_NUMBER, goalDir);
        //vision.invalidateEdgeMap();
	}


}
#endif
