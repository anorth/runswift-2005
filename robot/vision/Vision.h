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
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 * The interface of the Vision object
 *
 **/

#ifndef _Vision_h_DEFINED
#define _Vision_h_DEFINED

#ifndef OFFLINE

#include <OPENR/OObject.h>
#include <OPENR/ObjcommMessages.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>
#include "TCPConnection.h"
#endif //OFFLINE

#include <string>
#include <utility>
#include <vector>
#include <list>

#include "../share/Common.h"
#include "gps.h"
#include "CommonSense.h"
#include "PlayerName.h"
#include "def.h"
#include "Robolink.h"
#include "../share/SwitchBoard.h"
#include "../share/WirelessSwitchboard.h"
#include "../share/SharedMemoryDef.h"
#include "../share/WalkLearningParams.h"
#include "Obstacle.h"

using namespace std;

#define WORLD_MODEL_DEBUG
#define FRAMECOUNT


// array of flags of debug information to send
// indexed by enum data_type in share/robolink.h
extern bool send_data[];

// indexes into send_data
#if 0
enum SEND_DATA {
    SEND_ENABLED = 0,
    SEND_PWALK,
    SEND_FEATURES,
    SEND_OBJECTS,
    SEND_INSANITIES,
    SEND_CPLANE,
    SEND_SUBCPLANE,
    SEND_YUV,
    SEND_ATOMIC_ACTION,
    SEND_GPSINFO,
    SEND_PINKOBJECT,
    SEND_GAUSSIANS,
    NUM_SEND_DATA   // must be last
};
#endif

extern int SUBVISION_SEND_INTERVAL;
extern int CPLANE_SEND_INTERVAL;
extern int DISPLAY_ENVIRONMENT_INTERVAL;
extern int SHARE_ENVIRONMENT_INTERVAL;

static const int P_RED_DOG = 1;
static const int P_BLUE_DOG = -1;

static const double FRAME_SKIP = CAMERA_FRAME_MS*1.2 / JOINT_FRAME_MS + 2;

/* name overlap between enumeration here and
 * in Prolog.h but overlapping values are the same 
 **/
enum CommTrunkState {
    IDLE,
    START,
    RUNNING
};

/* Max # of TCP connections */
static const int NUM_TCP_CONNECTIONS = 1;

enum {
    // Must be at least ROBOLINK_BUF_LEN and big enough to handle all the data
    // you want to send in one frame
    TCP_SEND_BUF_SIZE = 300000,
    TCP_RECV_BUF_SIZE = 1024,
    TCP_INTERNAL_BUF_SIZE = TCP_SEND_BUF_SIZE + 10000,
};

class Vision : public OObject {
public:
    Vision();
    ~Vision();

    //  OPENR Methods
    //
    virtual OStatus DoInit(const OSystemEvent &event);
    virtual OStatus DoStart(const OSystemEvent &event);
    virtual OStatus DoStop(const OSystemEvent &event);
    virtual OStatus DoDestroy(const OSystemEvent &event);

    OSubject *subject[numOfSubject];
    OObserverVector *observer[numOfObserver];

    //  Inter-Object Communication Methods
    //
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

    //void Ready(const OReadyEvent& event);
    //void NotifyClientCommand(const ONotifyEvent& event);
    void try1(const OReadyEvent &event);

    /**
     * for sharing and displaying environment with other dogs
     **/
    void shareEnvironment();
    void sendPlayerNumber(int playerNumber, bool isRedRobot);
    void sendPlayerNumber(int playerNumber, int team);

    /* For sending behaviour coordination messages */
    void sendTeamTalk(const char* data, int length);

    /* transmit World Model */
    void displayEnvironment();

    /* store all the values of the sanity (1 for fired, 0 for unfired) */
    int storeSanityValues(unsigned char *planeArray, int arrayLength);

    /* store the sensors reading */
    int storeSensors(unsigned char *planeArray, int arrayLength);
    
    /* display what the dog sees */
    void sendYUVPlane();
    void sendYUVPlaneLowRes();

    void sendPythonAction(int); // send the atomic action from python behaviour to robolink
    void sendGPSInfo(); 
   
    /* Send dog MAC address to robolink */
    void sendDogId();

    void sendAA(AtomicAction &nextAA);
    void sendOP(OffsetParams &nextOP);
    void sendLearningParams(WalkLearningParams &params);

    void sendIndicators(int *inds, size_t sz);

    int gotTMWM;

    long getFrameID() { return this->currentFrameID; }

    // TCP connections are being moved here from Wireless
#ifdef USE_TCP
    OStatus TCPListen(int port, int* id);
    OStatus TCPQueue(int id, byte* buf, int size);
    OStatus TCPSend(int id/*, int bufId byte* buf, int size*/);
    OStatus TCPReceive(int id, byte* buf, int size);
    OStatus TCPClose(int id);
    // callbacks
    void TCPListenCont(ANTENVMSG msg);
    void TCPSendCont(ANTENVMSG msg);
    void TCPReceiveCont(ANTENVMSG msg);
    void TCPCloseCont(ANTENVMSG msg);
#endif // USE_TCP
        
private:

#ifdef USE_TCP
    OStatus InitTCPConnection(int id);
    OStatus TCPListen(int id);
#endif

    // keep track of whether Indicators.cc should have LED control
    bool indicatorState;

    // these OObservers and OSubjects appear to be just shortcuts to save
    // an array reference into subject[] and observer[] later on.
    OObserver *objectIn;
    OObserver *dogCommIn;
    //OObserver *dogCommLTIn;

    // OObserverVector *oSensor, *oGSensor;

    OSubject *objectOut;
    OSubject *dogCommOut;
    OSubject *dogPhotoOut;
    OSubject *playerNumberOut;

    OSubject *sSetIndication;
    OSubject *sExecuteAA;
    OSubject *sGaitOptimiser;
    OSubject *sWalkingLearner;

#ifdef SENDING_DEBUG
    RCRegion *sharedrc;
    RC_DEBUG_DATA *sharedDebugMemory;
#endif // SENDING_DEBUG


    CommonSense sensors;
    VisualCortex vision;
    GPS gps;
    Obstacle obstacle;
    struct PWalkInfoStruct PWalkInfo;

    //shared region of memory that is going to be used to extract
    //the stepComplete variable and the pAgent variable (instance of a PWalk)
    //from ActuatorControl, specifically ActionPro
    RCRegion *shmPWalk;

    int lastWalkType; // The walk type in the previous frame

    long currentFrameID;
    long lastFrameID;
    long lastKickFrameID;    
    
    // timestamp of the start of this vision frame
    SystemTime frameStartTime;

    // to slow the transferate
    int cplaneCounter;
    int oplaneCounter;
    int displayEnvironmentCounter;
    int shareEnvironmentCounter;

    // store information of cplane
    unsigned char compressedCPlane[COMPRESSED_CPLANE_SIZE + SENSOR_SIZE + SANITY_SIZE + OBJ_SIZE];

    // this counter will be sent across to the base to record the delay occured
    int delayTestingCounter;

    char dataResultCamera[100];

    int wait4settle;        // for debugging
    bool powerIsOn;
    bool sharedMemIsSetup;

    void SetCamera();
    void SetCamera(longword wb_val, longword gain_val, longword shutter_val);
    bool camNotSet;

    bool forceCplane;

#ifdef FRAMECOUNT
    SystemTime lastTime;
    SystemTime curTime;

    // for timing ProcessImage
    SystemTime imgProcessTime;

    // for timing gps updates
    SystemTime gpsVisTime;

    // for timing  behaviours
    SystemTime behavioursTime;

    int frameCounter;
    long LastframePt;
    int droppedFrames;
    int framesSinceLastDrop;
#endif
#ifdef WORLD_MODEL_DEBUG
    long Last_WM_Pt;
#endif

    unsigned long ethernetMACaddress;	// hardware dog ID - holds last 4 bytes

#ifdef USE_TCP
    antStackRef ipstackRef;
    TCPConnection tcpconnections[NUM_TCP_CONNECTIONS];
#endif
    // check the correctness of the primitive size
    void checkPrimitiveSize() {
        int zero = 0;
        if (INT_BYTES != sizeof(int)) {
            cout << "Vision.cc: Invalid INT_SIZE " << INT_BYTES << ". Should be "
            << sizeof(int) << endl;
            0 / zero; // causes a crash
        }
        if (LONG_BYTES != sizeof(long)) {
            cout << "Vision.cc: Invalid LONG_SIZE " << LONG_BYTES
            << ". Should be " << sizeof(long) << endl;
            0 / zero; // causes a crash
        }
    }
};

#endif // _Vision_h_DEFINED
