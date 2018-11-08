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
 **/

#define OPENR_STUBGEN

#include <OPENR/core_macro.h>

#include <MCOOP.h>
#include <OPENR/OPENR.h>
#include <OPENR/OPENRMessages.h>
#include <OPENR/core_macro.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <ERA201D1.h>

#include "Vision.h"
#include "Odometer.h"
#include "PyEmbed.h"
#include "InfoManager.h"
#include "Profile.h"
#include "../share/Common.h"
#include "../share/VisionDef.h"
#include "../share/SharedMemoryDef.h"
#include "../share/WirelessTypeDef.h"
#include "../share/OdometerDef.h"
#include "../behaviours/Behaviours.h"

//#define VISION_DEBUG

static const bool delayDebugMsg = false;
static const bool yuvDebugMsg = false;
static const bool newOplaneDebugMsg = false;
static const bool sensorValueDebugMsg = false;
static const bool sanityValuesDebugMsg = false;
static const bool compressedCPlaneDebugMsg = false;

/* Robolink data send flags and rates. If these are true then that debug stream
 * will be sent at appropriate intervals (below)
 */
bool send_data[RLNK_NUM_DATA_TYPES]; // all default false

int SUBVISION_SEND_INTERVAL = 1; // features, objects, insanities
int CPLANE_SEND_INTERVAL = 1;    // subsampled and full
int YUV_SEND_INTERVAL = 30;
int DISPLAY_ENVIRONMENT_INTERVAL = 60;
int SHARE_ENVIRONMENT_INTERVAL = 5;

static const int MIN_FRAME_AFTER_KICKING = 10;

// the threshold for determining if the dog has booted up as red or blue
static const int YELLOW_GOAL_MOUTH_THRESHOLD = -400000;

// this is sent when the robot recognise any object in the colour plane
static const unsigned char zeroBytes[INT_BYTES] = {
	0, 0, 0, 0
};

static const int objectOrder[NUM_OBJ] = {
  vobBall,
	vobBlueGoal,
	vobYellowGoal,
	vobBluePinkBeacon,
	vobPinkBlueBeacon,
	vobYellowPinkBeacon,
	vobPinkYellowBeacon,
	vobRedDog,
	vobRedDog2,
	vobRedDog3,
	vobRedDog4,
	vobBlueDog,
	vobBlueDog2,
	vobBlueDog3,
	vobBlueDog4
};

Vision::~Vision(){
}

Vision::Vision() {
	OSYSDEBUG(("Vision::Vision()\n"));

	checkPrimitiveSize();

    Behaviours::InitBehaviours();

	powerIsOn = false;
	sharedMemIsSetup = false;
	camNotSet = true;
	wait4settle = 30;
	sharedrc = NULL;
	sharedDebugMemory = NULL;

	GetSystemTime(&lastTime);
	GetSystemTime(&curTime);

	frameCounter = 0;
	LastframePt = 0;
    droppedFrames = 0;
    framesSinceLastDrop = 0;
    send_data[RLNK_DROPFRAME] = true;

	Last_WM_Pt = 10 * 125;
    lastWalkType = -1;

    // if (pythonBehaviours) {
    PyBehaviours::sensors = &sensors;
    PyBehaviours::vision = &vision;
    PyBehaviours::gps = &gps;
    PyBehaviours::obstacle = &obstacle;
    PyBehaviours::transmission = this;
    PyBehaviours::PWalkInfo = &PWalkInfo;
    //} else {
    Behaviours::sensors = &sensors;
    Behaviours::vision = &vision;
    Behaviours::gps = &gps;
    Behaviours::transmission = this;
    //}
    gInfoManager.setSensors(&sensors);
    gInfoManager.setGPS(&gps);
    gInfoManager.setVisualCortex(&vision);

	cplaneCounter  = 0;
	oplaneCounter  = 0;

	displayEnvironmentCounter = 0;
	shareEnvironmentCounter = 0;

	delayTestingCounter = 0;

	gotTMWM = 0;
    
}

OStatus Vision::DoInit(const OSystemEvent &event) {
	OSYSDEBUG(("Vision::DoInit()\n"));

	NEW_ALL_SUBJECT_AND_OBSERVER;
	REGISTER_ALL_ENTRY;
	SET_ALL_READY_AND_NOTIFY_ENTRY;

	// wireless
	objectOut = subject[sbjSPEAKER];
	dogCommOut = subject[sbjDogComm];
	dogPhotoOut = subject[sbjDogPhoto];
	playerNumberOut = subject[sbjPlayerNumber];

	sSetIndication = subject[sbjSetIndication];
	sExecuteAA = subject[sbjExecuteAA];
	sGaitOptimiser = subject[sbjGaitOptimiser];
	sWalkingLearner = subject[sbjWalkingLearner];

	//sensors.oSensor = observer[obsSensor];

	//wireless
	objectIn = observer[obsExternalCommand];
	dogCommIn = observer[obsDogComm];
	//dogCommLTIn = observer[obsDogCommLT];

    // new Vision TCP connections
	ipstackRef = antStackRef(strdup("IPStack"));
#ifdef USE_TCP
    // initialise connections
	for (int i = 0; i < NUM_TCP_CONNECTIONS; ++i) {
		OStatus result = InitTCPConnection(i);
		if (result != oSUCCESS)
			return oFAIL;
	}
#endif	// USE_TCP

	sharedrc = new RCRegion(sizeof(RC_DEBUG_DATA));
	sharedDebugMemory = (RC_DEBUG_DATA *) sharedrc->Base();

	EtherDriverGetMACAddressMsg myMsg;
	EtherStatus result;
	if ((result = ERA201D1_GetMACAddress(&myMsg)) != ETHER_OK) {
		cout << "Vision::DoInit: Bad ERA201D1_GetMACAddress return val: "
            << etherStatusStr(result) << endl;
	} else {
		ethernetMACaddress = ((myMsg.address.octet[2] << 24)
                | (myMsg.address.octet[3] << 16)
                | (myMsg.address.octet[4] << 8)
                | (myMsg.address.octet[5]));
		cout << "Got MAC address for dog: " << myMsg.address
            << " with low bytes: " << ethernetMACaddress << endl;
	}

	vision.setDogParams(ethernetMACaddress);

	return oSUCCESS;
}

void Vision::sendWirelessMessage(int msgType,
		int dataType,
		int dataLength,
		void *sharedMem,
		const void *msg) {
	if (sharedMem == NULL) {
		switch (dataType) {
			case shmdtMessages: sharedMem = sharedDebugMemory->message;
					    break;
			case shmdtWorldModel: sharedMem = sharedDebugMemory->world_model_debug_data;
					      break;
			case shmdtCPlane: sharedMem = sharedDebugMemory->cplane_data;
					  break;
			case shmdtOPlane: sharedMem = sharedDebugMemory->oplane_data;
					  break;
			case shmdtYUVPlaneCPlane: sharedMem = sharedDebugMemory->yuvCplane_data;
						  break;
			case shmdtYUVPlane: sharedMem = sharedDebugMemory->yuvplane_data;
					    break;
			case shmdtEdgeDetectDebug: sharedMem = sharedDebugMemory->line_debug_data;
						   break;
			default: cout
				 << "WARNING Vision::sendWirelessMessage : setting unknown dataType or NULL shared memory"
					 << endl; 
				 return;
		}
	}

	memcpy(sharedMem, msg, dataLength);

	objectOut->ClearBuffer();
	objectOut->SetData(&msgType, sizeof(int));
	objectOut->SetData(&dataType, sizeof(int));
	objectOut->SetData(&dataLength, sizeof(int));
	objectOut->NotifyObservers();
}

void Vision::sendWirelessMessage(const string &str){
    sendWirelessMessage(msgTypeNORMAL,
            shmdtMessages,
            str.length(),
            NULL,
            str.c_str());
}

OStatus Vision::DoStart(const OSystemEvent &event) {
	OSYSDEBUG(("Vision::DoStart()\n"));

	ENABLE_ALL_SUBJECT;
	ASSERT_READY_TO_ALL_OBSERVER;
	// send player number to wireless
	sendPlayerNumber(Behaviours::PLAYER_NUMBER, -1);

	int msgType = msgTypeESTABLISHING;
	MemoryRegionID mem = sharedrc->MemID();
	size_t offset = sharedrc->Offset();
	size_t size = sharedrc->Size();

	objectOut->ClearBuffer();
	objectOut->SetData(&msgType, sizeof(int));
	objectOut->SetData(&mem, sizeof(mem));
	objectOut->SetData(&offset, sizeof(offset));
	objectOut->SetData(&sharedDebugMemory, sizeof(sharedDebugMemory));
	objectOut->SetData(&size, sizeof(size));
	objectOut->NotifyObservers();

#ifdef USE_TCP
    Robolink::initialize(this);
#endif

	return oSUCCESS;
}

OStatus Vision::DoStop(const OSystemEvent &event) {
	//eCommander.stop();

	DISABLE_ALL_SUBJECT;
	DEASSERT_READY_TO_ALL_OBSERVER;

	return oSUCCESS;
}

OStatus Vision::DoDestroy(const OSystemEvent &event) {
	DELETE_ALL_SUBJECT_AND_OBSERVER;

	/* deletes all observers and subjects spawned in DoInit() */
	DeleteComData         

    return oSUCCESS;
}



// send the atomic action from python behaviour to robolink
void Vision::sendPythonAction(int timeElapsed) {
#ifndef OFFLINE
    struct rlnk_atomicaction data;
    data.command  = Behaviours::acCmd;
    data.walkType = Behaviours::walkType;
    data.PG       = Behaviours::PG;
    data.timeElapsed = timeElapsed;
    data.forward  = Behaviours::forward;
    data.left     = Behaviours::left;
    data.turn     = Behaviours::turnCCW;
    data.forwardMaxStep = Behaviours::forwardMaxStep;
    data.leftMaxStep = Behaviours::leftMaxStep;
    data.turnCCWMaxStep = Behaviours::turnCCWMaxStep;
    Robolink::sendToBase((byte *)&data, RLNK_ATOMIC_ACTION, sizeof(data));
#endif // OFFLINE
}

// send the atomic action from python behaviour to robolink
void Vision::sendGPSInfo() {
#ifndef OFFLINE
    struct rlnk_gpsinfo data;
    data.posX           = gps.self().pos.x;
    data.posY           = gps.self().pos.y;
    data.heading        = gps.self().h;
    data.posVar         = gps.self().posVar;
    data.hVar           = gps.self().hVar;
    data.motionForward  = gps.prevMotionForward;
    data.motionLeft     = gps.prevMotionLeft;
    data.motionTurn     = gps.prevMotionTurn;
    data.motionPWM      = gps.prevMotionPWM;
    data.motionFCounter = gps.prevMotionFCounter;
    data.headSpeed      = sensors.getHeadSpeed();
    data.visualConfidence  = vision.getVisualConfidence();
    Robolink::sendToBase((byte *)&data, RLNK_GPSINFO, sizeof(data));
    gps.prevMotionForward = 0;
    gps.prevMotionLeft = 0;
    gps.prevMotionTurn = 0;
    gps.prevMotionPWM = 0;
    gps.prevMotionFCounter = 0;
#endif // OFFLINE
}

/* This is the big one - the driver of our behaviours. This is called 30 times
 * a second when a camera frame is ready and from here we process the frame
 * to recognise objects then do GPS updates and execute a behaviour.
 */
void Vision::ResultCamera(const ONotifyEvent &event) {
    GetSystemTime(&frameStartTime);
    Profile::init();
    Profile::start(PR_FRAME);

	// Setting the camera, not sure if this is mandatory
	if (camNotSet) {
		SetCamera();
		observer[obsYUV]->AssertReady();
		return;
	}

	if (!sharedMemIsSetup) {
		observer[obsYUV]->AssertReady();
		return;
	}

	// Wait for everything to settle
	if (wait4settle > 0) {
		wait4settle--;
		observer[obsYUV]->AssertReady(event.SenderID());
		return;
	}
	// Setting goal
	if (gps.goalNotSet) {
		bool goalDir;
		int mouthPos = sensors.sensorVal[ssMOUTH];
	    if (mouthPos < YELLOW_GOAL_MOUTH_THRESHOLD) {
            // jaws open
            goalDir = false;
            cout << "Go to Yellow goal" << endl;
	    } else {
            // jaws closed
            goalDir = true;
            cout << "Go to Blue goal" << endl;
	    }
#ifdef ONLY_ONE_DOG
        if (Behaviours::PLAYER_NUMBER <= 2){
            goalDir = false;
        }
        else{
            goalDir = true;
        }
#endif//ONLY_ONE_DOG
                
		gps.SetGoals(goalDir);
		sendPlayerNumber(Behaviours::PLAYER_NUMBER, goalDir);

        vision.invalidateEdgeMap();
	}

    if ((*PWalkInfo.currentWalkType) != lastWalkType) {
        vision.invalidateEdgeMap();
        lastWalkType = (*PWalkInfo.currentWalkType);
    }
    
    gps.sendCPlane = false;
    // use accelerometers?
    vision.bodyMoving = (Behaviours::left != 0 ||
                         Behaviours::forward != 0 ||
                         Behaviours::turnCCW != 0);
    vision.headMovement = sensors.getHeadSpeed();

#ifdef FRAMECOUNT
	SystemTime icur;
	GetSystemTime(&icur);
#endif //FRAMECOUNT

    /* Call VisualCortex to process the image */
    long s0, s1, s2;
    s0 = sensors.sensorVal[ssHEAD_TILT];
    s1 = sensors.sensorVal[ssHEAD_PAN];
    s2 = sensors.sensorVal[ssHEAD_CRANE];
    Profile::start(PR_PROCESSIMAGE);
	vision.processImage(event, s0, s1, s2, sensors.range2obstacle, &PWalkInfo);
    Profile::stop(PR_PROCESSIMAGE);

#ifdef FRAMECOUNT
	SystemTime icur2;
	GetSystemTime(&icur2);
	SystemTime idiff = icur2 - icur;
	imgProcessTime = imgProcessTime + idiff;
#endif //FRAMECOUNT

    /* GPS Update */
    Profile::start(PR_GPS);
#ifndef LOCALISATION_CHALLENGE

    #ifdef VISION_DEBUG    
        OSYSDEBUG(("%s : gps.GPSVisionUpdate.\n",__func__));
    #endif //VISION_DEBUG
        Profile::start(PR_GPS_VISION);
        gps.GPSVisionUpdate(vision.vob, sensors.isHeadMoving());
        Profile::stop(PR_GPS_VISION);

    #ifdef VISION_DEBUG    
        OSYSDEBUG(("%s : gps.GPSEdgeUpdate.\n",__func__));
    #endif //VISION_DEBUG
        Profile::start(PR_GPS_EDGE);
        
    #ifndef OFFLINE
        /*Don't run edge update when kicking, 
        since it may not move the head too much but 
        it may move the body agressively which will produce 
        incorrent value of point projection.*/
        if (*(PWalkInfo.currentWalkType) <= AlanWalkWT )
        {   
            if (ABS(currentFrameID - lastKickFrameID) > MIN_FRAME_AFTER_KICKING)
            {
    #endif //OFFLINE           
                // don't run the edge update in time-critical mode
                if (! Behaviours::isTimeCritical()) {
                    gps.GPSEdgeUpdate(&vision, sensors.getHeadSpeed());
                }
    #ifndef OFFLINE
            }
        }
        else
        {
            lastKickFrameID = currentFrameID;
        }
    #endif //OFFLINE       
        Profile::stop(PR_GPS_EDGE);

    #ifdef VISION_DEBUG    
        OSYSDEBUG(("%s : gps.GPSVisualConfidenceUpdate.\n",__func__));
    #endif //VISION_DEBUG
        gps.GPSVisualConfidenceUpdate(vision.getVisualConfidence());

    #ifdef VISION_DEBUG    
        OSYSDEBUG(("%s : obstacle.ObstacleUpdate.\n",__func__));
    #endif //VISION_DEBUG  
        Profile::start(PR_GPS_OBSTACLE);
        obstacle.ObstacleUpdate(&vision,&gps);
        Profile::stop(PR_GPS_OBSTACLE);

#else //LOCALISATION_CHALLENGE
    gps.bodyMoving = (Behaviours::left != 0 ||
                         Behaviours::forward != 0 ||
                         Behaviours::turnCCW != 0);
        
    if (vision.lookForBeaconsAndGoals) {
        gps.GPSVisionUpdate(vision.vob, sensors.isHeadMoving());
        gps.MapLocalisePink(vision.localisePink, vision.numLocalisePink);
    } else {
        /*Just additional for debugging */
        gps.prevMotionForward = gps.motionForward;
        gps.prevMotionLeft = gps.motionLeft;
        gps.prevMotionTurn = gps.motionTurn;
        gps.prevMotionPWM = gps.motionPWM;
        gps.prevMotionFCounter = gps.motionFCounter;
        
        gps.motionPWM = 0;
        gps.motionFCounter = 0;
        gps.motionForward = 0.0;
        gps.motionLeft = 0.0;
        gps.motionTurn = 0.0;
        gps.GPSOffFieldUpdate(&vision);
        gps.LocalisePinkUpdate(&vision);
    }
    gps.GPSEdgeUpdate(&vision, sensors.getHeadSpeed());
    gps.GPSVisualConfidenceUpdate(vision.getVisualConfidence());
    //obstacle.ObstacleUpdate(&vision, &gps);
#endif //LOCALISATION_CHALLENGE
            
    Profile::stop(PR_GPS);
    

#ifdef FRAMECOUNT
	SystemTime gcur;
	GetSystemTime(&gcur);
	SystemTime gdiff = gcur - icur2;
	gpsVisTime = gpsVisTime + gdiff;

	// Counting the number of frames
	frameCounter++;
    
#ifndef OFFLINE
    OPowerStatus currentStatus;
    OPENR::GetPowerStatus(&currentStatus);
    Behaviours::batteryCurrent = currentStatus.current;
#endif

	if (vision.cameraFrame > LastframePt + 5 * 125) {
		LastframePt += 5 * 125;
		GetSystemTime(&curTime);
		SystemTime diff = curTime - lastTime;
		double frames = frameCounter
			/ (diff.seconds + (diff.useconds / 1000000.0));
		int piSpeed = (int)
			((imgProcessTime.seconds*1000000.0 + imgProcessTime.useconds)
			 / frameCounter);
		int gpsSpeed = (int)
			((gpsVisTime.seconds*1000000.0 + gpsVisTime.useconds)
			 / frameCounter);
		int bSpeed = (int)
			((behavioursTime.seconds*1000000.0 + behavioursTime.useconds)
			 / frameCounter);

		sprintf(dataResultCamera,
				"(%d) - %d.%.6d s - %f fps\n Aves : img %dus, gps %dus, behav %dus, batt = %d\n",
				 Behaviours::PLAYER_NUMBER,
				(int)
				diff.seconds,
				(int)
				diff.useconds,
				frames,
				piSpeed,
				gpsSpeed,
				bSpeed,
                Behaviours::batteryCurrent);
		sendWirelessMessage(msgTypeNORMAL,
				shmdtMessages,
				strlen(dataResultCamera),
				sharedDebugMemory->message,
				dataResultCamera);
		frameCounter = 0;
		lastTime = curTime;
		imgProcessTime.seconds = 0;
		imgProcessTime.useconds = 0;
		gpsVisTime.seconds = 0;
		gpsVisTime.useconds = 0;
		behavioursTime.seconds = 0;
		behavioursTime.useconds = 0;
	}

#endif // FRAMECOUNT

	// Debug world model

	if (vision.cameraFrame > Last_WM_Pt + 1 * 125) {
		Last_WM_Pt += 1 * 125;

		/*
		   cout << "Start World Model here ";
		   cout << (long) rx << " " << (long) ry << " " << (long) rh << " " <<
		   (long) rcf;
		   cout << " " << (long) wmobj[0].d << " " << (long) wmobj[0].
		   h << " " << (long) wmobj[0].cf;
		   cout << " " << (long) wmobj[vobRedDog].
		   d << " " << (long) wmobj[vobRedDog].
		   h << " " << (long) wmobj[vobRedDog].cf;
		   cout << " " << (long) wmobj[vobBlueDog].
		   d << " " << (long) wmobj[vobBlueDog].
		   h << " " << (long) wmobj[vobBlueDog].cf;
		   cout << " End World Model here";
		   cout << endl;
		   */
	}

#ifdef FRAMECOUNT  
	SystemTime bcur;
	GetSystemTime(&bcur);
#endif // FRAMECOUNT
    
    // Run the behaviours
    Profile::start(PR_BEHAVIOUR);
    Behaviours::NextAction();
    Profile::stop(PR_BEHAVIOUR);

#ifdef FRAMECOUNT  
    SystemTime bcur2;
    GetSystemTime(&bcur2);
    SystemTime bdiff = bcur2 - bcur;
    behavioursTime = behavioursTime + bdiff;
    
    SystemTime tdiff = bcur2 - icur;
    Behaviours::currentFrameID = vision.cameraFrame;
    
    lastFrameID = currentFrameID;
    currentFrameID = vision.cameraFrame;
    Behaviours::cpuUsage = (tdiff.seconds * 1000000.0 + tdiff.useconds);
#endif // FRAMECOUNT

    // Send data to robolink
    static int sendCnt = 0;
#ifdef VISION_DEBUG    
    OSYSDEBUG(("%s : Sending data to robolink.\n",__func__));
#endif      
    
    if (send_data[RLNK_ALL] && ++sendCnt > 30 * 5) {
        if (sendCnt % SUBVISION_SEND_INTERVAL == 0) {
            if (send_data[RLNK_PWALK])
                vision.sendPWalkSensors();
            if (send_data[RLNK_SUBVISION]) 
                vision.sendFeatures();
            if (send_data[RLNK_SUBOBJECT])
                vision.sendObjects();
            if (send_data[RLNK_INSANITY])
                vision.sendInsanities();
            if (send_data[RLNK_ATOMIC_ACTION])
                sendPythonAction(0); // timeElapsed = 0
            if (send_data[RLNK_GPSINFO])
                sendGPSInfo(); 
            if (send_data[RLNK_PINKOBJECT])
                vision.sendPinkObjects();
            if (send_data[RLNK_GAUSSIANS])
                gps.sendGaussians();
        }
        if (sendCnt % YUV_SEND_INTERVAL == 0) {
            if (send_data[RLNK_YUVPLANE])
                sendYUVPlane();                   
        }   
        if (sendCnt % CPLANE_SEND_INTERVAL == 0) {
            if (send_data[RLNK_CPLANE]) {
                CorrectedImage::fillCPlane();
                CorrectedImage::sendCPlane();
            }
            if (send_data[RLNK_SUBCPLANE]) {
                CorrectedImage::fillSubCPlane();
                CorrectedImage::sendSubCPlane();
            }
        }   
    }
    
    /* Flush the TCP buffers */
    for (int i = 0; i < NUM_TCP_CONNECTIONS; ++i) {
        TCPSend(i);
    }

    Profile::stop(PR_FRAME);

#ifdef FRAMECOUNT    
    if (send_data[RLNK_DROPFRAME])
    {
        if (((currentFrameID - lastFrameID) > FRAME_SKIP) && (lastFrameID > 0))
        {
            droppedFrames++;
            cerr << "Pr# ***PROFILE OF A FRAME DROPPER***" << endl;
            cerr << "Pr# Dropped Frame #" << droppedFrames 
            << ", Frames Since Last Drop: " << framesSinceLastDrop << endl;
            Profile::printsaved(NUM_PROFID);
            
            
            framesSinceLastDrop = 0;
        }    
        else
        {
            framesSinceLastDrop++;
        }
    }
#endif        
    
    if(send_data[RLNK_FRAME])
    {
        // print profile information.
        // FIXME: send this to robolink
        if (vision.vFrame % 120 == 0) {
                            
            //if (Behaviours::isTimeCritical())
            //{
            //    cerr << "Time critical mode" << endl;
            //}
            
            Profile::print(NUM_PROFID);
            
            //cerr << vision.getBallFeatureCount() << " ball features" << endl;
        }
    }

    Profile::save();
    
    observer[obsYUV]->AssertReady(event.SenderID());
}



//set default camera settings
void Vision::SetCamera() {

	//Parameter:
	//  White Balance:
	//      const longword ocamparamWB_INDOOR_MODE  = 1;
	//      const longword ocamparamWB_OUTDOOR_MODE = 2;
	//      const longword ocamparamWB_FL_MODE      = 3; // Fluorescent Lamp
	//
	//  Gain:
	//      const longword ocamparamGAIN_LOW        = 1;
	//      const longword ocamparamGAIN_MID        = 2;
	//      const longword ocamparamGAIN_HIGH       = 3;
	//
	//  Shutter Speed:
	//      const longword ocamparamSHUTTER_SLOW    = 1;
	//      const longword ocamparamSHUTTER_MID     = 2;
	//      const longword ocamparamSHUTTER_FAST    = 3;

	//longword wb_val = ocamparamWB_INDOOR_MODE;
	//longword wb_val = ocamparamWB_OUTDOOR_MODE;
	longword wb_val = ocamparamWB_FL_MODE;

	//longword gain_val = ocamparamGAIN_LOW;
	//longword gain_val = ocamparamGAIN_MID;
	longword gain_val = ocamparamGAIN_HIGH;
        
	//longword shutter_val = ocamparamSHUTTER_SLOW;
	//longword shutter_val = ocamparamSHUTTER_MID;
	longword shutter_val = ocamparamSHUTTER_FAST;

#ifdef LIGHTING_CHALLENGE
        shutter_val = ocamparamSHUTTER_MID;
#endif

	SetCamera(wb_val, gain_val, shutter_val);

}

void Vision::SetCamera(longword wb_val, longword gain_val, longword shutter_val) {

	OPrimitiveID fbkID;
	OPENR::OpenPrimitive("PRM:/r1/c1/c2/c3/i1-FbkImageSensor:F1", &fbkID);

	// White Balance 
	OPrimitiveControl_CameraParam wb(wb_val);
	if (OPENR::ControlPrimitive(fbkID,
				oprmreqCAM_SET_WHITE_BALANCE,
				&wb,
				sizeof(wb),
				0,
				0) == oSUCCESS) {
		OSYSDEBUG(("CAM_SET_WHITE_BALANCE : Success\n"));
	}
	else {
		OSYSDEBUG(("CAM_SET_WHITE_BALANCE : Failed\n"));
	}

	// Gain 
	OPrimitiveControl_CameraParam gain(gain_val);
	if (OPENR::ControlPrimitive(fbkID,
				oprmreqCAM_SET_GAIN,
				&gain,
				sizeof(gain),
				0,
				0) == oSUCCESS) {
		OSYSDEBUG(("CAM_SET_GAIN : Success\n"));
	}
	else {
		OSYSDEBUG(("CAM_SET_GAIN : Failed\n"));
	}

	// Shutter Speed
	OPrimitiveControl_CameraParam shutter(shutter_val);
	if (OPENR::ControlPrimitive(fbkID,
				oprmreqCAM_SET_SHUTTER_SPEED,
				&shutter,
				sizeof(shutter),
				0,
				0) == oSUCCESS) {
		OSYSDEBUG(("CAM_SET_SHUTTER_SPEED : Success\n"));
	}
	else {
		OSYSDEBUG(("CAM_SET_SHUTTER_SPEED : Failed\n"));
	}
        
        /*
	// AWB Speed
	if (OPENR::ControlPrimitive(fbkID,
				oprmreqCAM_AWB_ON,
				0,
				0,
				0,
				0) == oSUCCESS) {
		OSYSDEBUG(("oprmreqCAM_AWB_ON : Success\n"));
	}
	else {
		OSYSDEBUG(("oprmreqCAM_AWB_ON : Failed\n"));
	}

	// AE Speed
	if (OPENR::ControlPrimitive(fbkID,
				oprmreqCAM_AE_ON,
				0,
				0,
				0,
				0) == oSUCCESS) {
		OSYSDEBUG(("oprmreqCAM_AE_ON : Success\n"));
	}
	else {
		OSYSDEBUG(("oprmreqCAM_AE_ON : Failed\n"));
	}
        */
        
        camNotSet = false;
}

//the purpose of this fuction is to call GPSMotionUpdate
//and then assert ready.
//needs to be done here because gps doesn't have access to the observer array
void Vision::GPSMotionUpdateAssert(const ONotifyEvent &event) {
#ifdef FRAMECOUNT  
    SystemTime gcur;
    GetSystemTime(&gcur);
#endif // FRAMECOUNT   

#ifdef VISION_DEBUG    
    OSYSDEBUG(("%s :GPSMotionUpdate .\n",__func__));
#endif      
    const double delta_forward = * (const double *) (event.Data(0));
    const double delta_left = * (const double *) (event.Data(1));
    const double delta_turn = * (const double *) (event.Data(2));
    long PWM = sensors.getMaxPWMValue();
    Profile::start(PR_GPS_MOTION);
    if (sharedMemIsSetup 
	&& *PWalkInfo.currentWalkType == SkellipticalWalkWT
	&& *PWalkInfo.currentMinorWalkType == MWT_SKE_GRAB_DRIBBLE_HOLD) 
      gps.GPSMotionUpdate(delta_forward, delta_left, delta_turn, PWM, false, true);
    else
      gps.GPSMotionUpdate(delta_forward, delta_left, delta_turn, PWM);
    Profile::stop(PR_GPS_MOTION);
    observer[event.ObsIndex()]->AssertReady();
#ifdef FRAMECOUNT  
    SystemTime gcur2;
    GetSystemTime(&gcur2);
    SystemTime gdiff = gcur2 - gcur;
    gpsVisTime = gpsVisTime + gdiff;
#endif // FRAMECOUNT
}

void Vision::OdometerAssert(const ONotifyEvent &event) {
	/* Temporary, if it works, use stepComplete! */
	const OdometerData *data = (const OdometerData *) (event.Data(0));
	Odometer::inverse(data, (sensors.press_sensor[PRESS_BACK_REAR].count > 4));
	observer[event.ObsIndex()]->AssertReady();
}

void Vision::ExternalCommand(const ONotifyEvent &event) {
	const CommandData &cmd = *(const CommandData *) event.Data(0);    
    // calls UNSWSkills->processCommand which handles some types.
    // For other types (including Python command) it puts cmd
    // in Behaviours::command which gets handled each frame by
    // Behaviours::processWControl
    Behaviours::processCommand(cmd);

	objectIn->AssertReady();
}

void Vision::TeammateWM(const ONotifyEvent &event) {
	gotTMWM = 1;
	const WMShareInfo *wm = (const WMShareInfo *) event.Data(0);
    
	gps.processShareInfo(wm->playerNum, wm->obj);
    
    Profile::start(PR_BE_OBS_SHARED);
    obstacle.processShareInfo(wm->obj);
    Profile::stop(PR_BE_OBS_SHARED);
    
	dogCommIn->AssertReady();
}

/* Receive a TeamTalk message from wireless */
void Vision::TeamTalk(const ONotifyEvent &event) {
	const int length = *(const int*) event.Data(0);
    const char* message = static_cast<const char*>(event.Data(1));
    
    //cout << "Vision received teamtalk:" << message << endl;
    //NOTE: the following are only team and player for team talk 
    //handled at c++ code level, for python level these are 
    //normally sender and recipient
    unsigned team = (unsigned)(message[2] - '0');
    unsigned player = (unsigned)(message[3] - '0');

    player %= 4;
    if (player == 0) player = 4;

    if (length == 11 && strncmp(message+4, "debug", 5) == 0) {
        // debug switches can be caught here
        if (team == Behaviours::TEAM_NUMBER
                && player == Behaviours::PLAYER_NUMBER) {
            int stream = message[9];
            send_data[RLNK_ALL] = true;
            send_data[stream] = ! send_data[stream];
            if (send_data[stream]) {
                cout << "Debug stream " << stream << " enabled" << endl;
            } else {
                cout << "Debug stream " << stream << " disabled" << endl;
            }
        }
    } else if (strncmp(message+4, "forma", 5) == 0) {
        //change of formation caught here
        if (team == Behaviours::TEAM_NUMBER) {
            char* head = "fmat";
            int cmd_len = strlen(head) + strlen(message) - 9;
            char cmd[cmd_len];
            strncpy(cmd, head, strlen(head));
            strncpy(cmd + strlen(head), message+9, strlen(message) - 9);
            PyBehaviours::processCommand(cmd, cmd_len);
        }
    } else {
        // Otherwise goes straight to PyEmbed
        PyBehaviours::processCommand(message, length);
    }

	observer[obsTeamTalk]->AssertReady();
}

void Vision::ResultSensor(const ONotifyEvent &event) {
  if (sharedMemIsSetup) {
    sensors.ResultSensor(event, *PWalkInfo.stepID);
    observer[obsSensor]->AssertReady(); // we want to assert ready ASAP
    /* Store new set of joint angles. */
    Odometer::storeData(sensors.sensorVal[ssFL_ROTATOR],
			sensors.sensorVal[ssFL_ABDUCTOR],
			sensors.sensorVal[ssFL_KNEE],
			sensors.sensorVal[ssFR_ROTATOR],
			sensors.sensorVal[ssFR_ABDUCTOR],
			sensors.sensorVal[ssFR_KNEE],
			sensors.sensorVal[ssRL_ROTATOR],
			sensors.sensorVal[ssRL_ABDUCTOR],
			sensors.sensorVal[ssRL_KNEE],
			sensors.sensorVal[ssRR_ROTATOR],
			sensors.sensorVal[ssRR_ABDUCTOR],
			sensors.sensorVal[ssRR_KNEE]);
  } else {
    observer[obsSensor]->AssertReady();
  }
}

void Vision::SetPAgent(const ONotifyEvent &event) {
	const MemoryRegionID mem = *(const MemoryRegionID *) event.Data(0);
	const size_t offset = *(const size_t *) event.Data(1);
	const voidPtr base = *(const voidPtr *) event.Data(2);
	const size_t size = *(const size_t *) event.Data(3);

	if (size != sizeof(ActuatorSharedMemory)) {
		cout << "ERROR: Actuator Shared Memory definition mismatch!" << endl;
	}

	shmPWalk = new RCRegion(mem, offset, base, size);

	ActuatorSharedMemory *ptr = (ActuatorSharedMemory *) shmPWalk->Base();

	if (ptr == NULL) {
		cout << "Bad Actuator Control Shared Memory Base Pointer" << endl;
	} else {
		cout << "Vision has received ActuatorControl shared memory: " << (int) ptr
			<< endl;
		// Behaviours
            Behaviours::leading_leg = &(ptr->leading_leg);
            Behaviours::currentWalkType = &(ptr->currentWalkType);
            Behaviours::shareStep_ = &(ptr->currentStep);
            Behaviours::currentPG = &(ptr->currentPG);
            Behaviours::desiredPan = &(ptr->desiredPan);
            Behaviours::desiredTilt = &(ptr->desiredTilt);
            Behaviours::desiredCrane = &(ptr->desiredCrane);
            //Behaviours::stepComplete = &(ptr->stepComplete);

	    // PWalkInfo
            PWalkInfo.leading_leg = &(ptr->leading_leg);
            PWalkInfo.currentWalkType = &(ptr->currentWalkType);
            PWalkInfo.currentMinorWalkType = &(ptr->currentMinorWalkType);
            PWalkInfo.currentStep = &(ptr->currentStep);
            PWalkInfo.currentPG = &(ptr->currentPG);
            PWalkInfo.frontShoulderHeight = &(ptr->frontShoulderHeight);
            PWalkInfo.bodyTilt = &(ptr->bodyTilt);
            PWalkInfo.desiredPan = &(ptr->desiredPan);
            PWalkInfo.desiredTilt = &(ptr->desiredTilt);
            PWalkInfo.desiredCrane = &(ptr->desiredCrane);
            PWalkInfo.isHighGain = &(ptr->isHighGain);
	    PWalkInfo.stepID = &(ptr->stepID);

	    // vision
            vision.currentWalkType = &(ptr->currentWalkType);

            sharedMemIsSetup = true;
	}
    //I dont assert ready at the end because i dont expect to be getting a
    //message to set up the shared mem region thus activating this function
    //more than once.

}

void Vision::shareEnvironment() {
	if (shareEnvironmentCounter == 0) {
	  
		WMShareObj obj = gps.getShareInfo();
                obstacle.setShareInfo(obj);

		dogCommOut->ClearBuffer();
		dogCommOut->SetData(&obj, sizeof(obj));
		dogCommOut->NotifyObservers();
	}
	shareEnvironmentCounter++;
	shareEnvironmentCounter %= SHARE_ENVIRONMENT_INTERVAL;
}


/* just use the player and team number from the 
 * game controller
 */
void Vision::GameData(const ONotifyEvent &event) {
    
    RoboCupGameControlData gameData = *(const RoboCupGameControlData*)event.Data(2);
    
    /* this is for UNSWSkills/Behaviours */
    Behaviours::PLAYER_NUMBER = *(const int*)event.Data(0);
    Behaviours::TEAM_NUMBER   = *(const int*)event.Data(1);
    Behaviours::prevGameData  = Behaviours::gameData;
    Behaviours::gameData      = gameData;
    Behaviours::processControllerData();    
    
    /* check whether Indicators.cc should have control of LEDs 
       this is applied in sendIndicators */
    if (gameData.state != STATE_INITIAL &&
        gameData.state != STATE_READY &&
        gameData.state != STATE_SET) {
        indicatorState = true;
    } else {
        indicatorState = false;
    }        
    
    observer[event.ObsIndex()]->AssertReady();
}

void Vision::sendPlayerNumber(int playerNumber, bool isRedRobot) {
	int team = isRedRobot?0:1;
	sendPlayerNumber(playerNumber, team);
}

void Vision::sendPlayerNumber(int playerNumber, int team) {
	playerNumberOut->SetData(&playerNumber, sizeof(int));
	playerNumberOut->SetData(&team, sizeof(int));
	playerNumberOut->NotifyObservers();
}

void Vision::sendTeamTalk(const char* data, int length) {
    subject[sbjTeamTalk]->SetData(&length, sizeof(int));
    subject[sbjTeamTalk]->SetData(data, length);
    subject[sbjTeamTalk]->NotifyObservers();
}

void Vision::displayEnvironment() {
	if (displayEnvironmentCounter == 0) {
		static const int TEAMMEMBERS_BEING_SENT = (NUM_TEAM_MEMBER - 1);
		static const int NUM_ATTR_SENT_ROB = 9;
		static const int NUM_ATTR_VIS_PULL = 3;
		static const int NUM_ATTR_SENT_TEAMMATE = 5;
		static const int NUM_ATTR_SENT_BALL = 9;    //for now x y vx vy varx vary varvx varvy
		static const int NUM_ATTR_SENT_WBALL = 3; // source robot num, x & y of wireless ball position
		static const int NUM_ATTR_SENT_OPPONENTS = 5;

		const int arraySize = 2
			+                      // for the player number and the team number

			1 +                      // for the view variable describing things we can see

			3
			+                      // for the last direct update values

			1 +                      // for the size of the remaining vision model data
			NUM_ATTR_SENT_ROB
			+      // the new world model data (x y h varx vary varh varxy varxh varyh)
			NUM_ATTR_VIS_PULL +   // the exponentially decaying visual pull
			NUM_ATTR_SENT_BALL
			+ NUM_ATTR_SENT_WBALL
			+    // for the wireless ball
			TEAMMEMBERS_BEING_SENT *NUM_ATTR_SENT_TEAMMATE + 

			// 1 +                   // for the size of the remaining world model data
			// NUM_ATTR_SENT_ROB +   // the vision data (x y h varx vary varh varxy varxh varyh)
			// NUM_ATTR_SENT_BALL +

			1
			+                      // for the size of the remaining wireless model data
			NUM_ATTR_SENT_ROB +      // the old world model data (x y h varx vary varh varxy varxh varyh)

			1
			+                      // for size of opponents data
			NUM_OPPONENTS *NUM_ATTR_SENT_OPPONENTS;/* +
            
            3 +
            vision.MAX_EDGE_PIXELS * 2; // Derrick: used for debugging edge detection*/
            


		float data[arraySize];
		int ptr = 0;

		//set player number
		data[ptr] = Behaviours::PLAYER_NUMBER;
		ptr++;

		// set direction team color
		if (gps.targetGoal == vobBlueGoal) {
			data[ptr] = P_RED_DOG;
		}
		else {
			data[ptr] = P_BLUE_DOG;
		}
		ptr++;

		// what we can see
		data[ptr++] = (double) gps.view;

		data[ptr++] = (double) gps.directx;
		data[ptr++] = (double) gps.directy;
		data[ptr++] = (double) gps.directh;

		/////////////////////
		// NEW WORLD MODEL
		/////////////////////
		//cout << "*************** got to world model sending *********" << endl;


		data[ptr] = NUM_ATTR_SENT_ROB
			+ NUM_ATTR_VIS_PULL
			+ NUM_ATTR_SENT_BALL
			+ NUM_ATTR_SENT_WBALL
			+ (TEAMMEMBERS_BEING_SENT * NUM_ATTR_SENT_TEAMMATE);
		ptr++;

		const WMObj &self = gps.self();
		const MMatrix3 scov = gps.selfCov();

		data[ptr] = self.pos.x;
		data[ptr + 1] = self.pos.y;
		data[ptr + 2] = self.h;
		data[ptr + 3] = scov(0, 0); //varx
		data[ptr + 4] = scov(1, 1); //vary
		data[ptr + 5] = scov(2, 2); //varh
		data[ptr + 6] = scov(0, 1); //varxy
		data[ptr + 7] = scov(0, 2); // varxh
		data[ptr + 8] = scov(1, 2); // varyh
		ptr += NUM_ATTR_SENT_ROB;

		const MVec3 &vp = gps.getVisualPull();
		data[ptr] = vp(0, 0);
		data[ptr + 1] = vp(1, 0);
		data[ptr + 2] = vp(2, 0);
		ptr += NUM_ATTR_VIS_PULL;

		//WMObj& ball = gps.ball('g');
		const Vector &ball = gps.getBall(GLOBAL);
		//WMObj& vball = gps.vBall();
		const Vector &vball = gps.getVBall(GLOBAL);
		const MMatrix2 &bcov = gps.getBallCov();
		//const MMatrix2 &bvcov = gps.getVelCov();
		//const MMatrix4& bcov = gps.bCov();
		data[ptr] = ball.x; //ball.pos.x;
		data[ptr + 1] = ball.y; //ball.pos.y;
		data[ptr + 2] = vball.x; //vball.pos.x;
		data[ptr + 3] = vball.y; //vball.pos.y;
		data[ptr + 4] = bcov(0, 0); //varx
		data[ptr + 5] = bcov(1, 1); //vary
		data[ptr + 6] = bcov(0, 1); //varxy
		data[ptr + 7] = 0.0; //bvcov(0, 0); //varvx
		data[ptr + 8] = 0.0; //bvcov(1, 1); //varvy
		ptr += NUM_ATTR_SENT_BALL;

		if (gps.shareBallvar < VERY_LARGE_INT) {
			// wireless ball data is valid
			data[ptr] = gps.sbRobotNum;
			data[ptr + 1] = gps.shareBallx;
			data[ptr + 2] = gps.shareBally;
		}
		else {
			// wireless ball data is invalid
			data[ptr] = 0;
			data[ptr + 1] = 0;
			data[ptr + 2] = 0;
		}
		ptr += NUM_ATTR_SENT_WBALL;

		for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
			if ((uint32)i != (Behaviours::PLAYER_NUMBER) - 1) {
				const WMObj &tm = gps.tmObj(i);
				data[ptr] = tm.pos.x;
				data[ptr + 1] = tm.pos.y;
				data[ptr + 2] = tm.h;
				data[ptr + 3] = tm.posVar;
				data[ptr + 4] = tm.hVar;
				ptr += NUM_ATTR_SENT_TEAMMATE;
			}
		}

		/*
		   cout << "**************" << endl;
		   cout << "my player number is " << (Behaviours::PLAYER_NUMBER) << endl;
		   cout << "my location is " << gps.r.pos.x << "," << gps.r.pos.y << endl;
		   for (int i=0; i<NUM_TEAM_MEMBER; i++) {
		   if (i!=((Behaviours::PLAYER_NUMBER)-1)) {
		   cout << "my teammates number " << (i+1) << "'s location is " << gps.wmTeammate[i].pos.x << "," << 
		   gps.wmTeammate[i].pos.y << endl;
		   }
		   }
		   cout << "**************" << endl;
		   */

		/////////////////////
		// VISION MODELS
		/////////////////////

		//data[ptr] = NUM_ATTR_SENT_ROB + NUM_ATTR_SENT_BALL; //number of attributes being sent over
		//ptr++;


		// note - there is no vision state any more as kalman filtering has changed
		// this should be eventually deleted
		/*data[ptr]     = 0;
		  data[ptr + 1] = 0;
		  data[ptr + 2] = 0;
		  data[ptr + 3] = 1; //varx
		  data[ptr + 4] = 1; //vary
		  data[ptr + 5] = 1; //varh
		  data[ptr + 6] = 0; //varxy
		  data[ptr + 7] = 0; // varxh
		  data[ptr + 8] = 0; // varyh
		  ptr += NUM_ATTR_SENT_ROB;*/

		/*data[ptr]     = gps.newBallx;
		  data[ptr + 1] = gps.newBally;
		  data[ptr + 2] = gps.newBallvx;
		  data[ptr + 3] = gps.newBallvy;
		  data[ptr + 4] = (*(gps.ballMeasCov))(0,0); //varx
		  data[ptr + 5] = (*(gps.ballMeasCov))(1,1); //vary
		  data[ptr + 6] = (*(gps.ballMeasCov))(0,1); //varxy
		  data[ptr + 7] = (*(gps.ballMeasCov))(2,2); //varvx
		  data[ptr + 8] = (*(gps.ballMeasCov))(3,3); //varvy
		  ptr += NUM_ATTR_SENT_BALL;*/

		/////////////////////
		// OLD WORLD MODEL
		/////////////////////

		data[ptr] = NUM_ATTR_SENT_ROB; //number of attributes being sent over
		ptr++;

		data[ptr] = gps.oldx;
		data[ptr + 1] = gps.oldy;
		data[ptr + 2] = gps.oldh;
		data[ptr + 3] = gps.oldvx; //varx
		data[ptr + 4] = gps.oldvy; //vary
		data[ptr + 5] = gps.oldvh; //varh
		data[ptr + 6] = gps.oldcxy; //varxy
		data[ptr + 7] = gps.oldcxh; //varxh
		data[ptr + 8] = gps.oldcyh; //varyh
		ptr += NUM_ATTR_SENT_ROB;


		/////////////////////
		// OPPONENTS
		/////////////////////
		data[ptr] = NUM_OPPONENTS * NUM_ATTR_SENT_OPPONENTS;
		ptr++;
		for (int i = 0 ; i < NUM_OPPONENTS ; i++) {
			const Vector v = gps.getOppPos(i, GLOBAL);
			const MMatrix2 &m = gps.getOppCov(i);
			data[ptr] = v.x;
			data[ptr + 1] = v.y;
			data[ptr + 2] = m(0, 0);
			data[ptr + 3] = m(1, 1);
			data[ptr + 4] = m(0, 1);
			ptr += NUM_ATTR_SENT_OPPONENTS;
		}

        // Derrick: for debugging - Sends projected points along with
        // the world model data.
        /*data[ptr++] = (float) vision.numEdgePnts;
        data[ptr++] = (float) vision.MAX_EDGE_PIXELS;
        if (gps.useLineData) data[ptr++] = 1.0;
        else data[ptr++] = 0.0;
        
		if (gps.targetGoal == vobBlueGoal) {
            for (int i = 0; i < (vision.numEdgePnts * 2); i++)
                data[ptr + i] = (float) vision.globalEdgePnts[i];
        } else {
            for (int i = 0; i < (vision.numEdgePnts * 2); i++) {
                data[ptr + i] = (float) vision.globalEdgePnts[i];

                if (i & 1) data[ptr + i] = FIELD_LENGTH - data[ptr + i];
                else data[ptr + i] = FIELD_WIDTH - data[ptr + i];
            }
        }
        for (int i = vision.numEdgePnts * 2; i < vision.MAX_EDGE_PIXELS * 2; i++)
            data[ptr + i] = 0.0;
            
        ptr += (vision.MAX_EDGE_PIXELS * 2);*/

		int dataType = shmdtWorldModel; //i assume this is the number that says "world model stuff"
		int arrayLength = sizeof(data);


		memcpy(sharedDebugMemory->world_model_debug_data, &data, arrayLength);

		int msgType = msgTypeNORMAL;
		objectOut->ClearBuffer();
		objectOut->SetData(&msgType, sizeof(int));
		objectOut->SetData(&dataType, sizeof(int));
		objectOut->SetData(&arrayLength, sizeof(int));
		objectOut->NotifyObservers();
	}        
	displayEnvironmentCounter++;
	displayEnvironmentCounter %= DISPLAY_ENVIRONMENT_INTERVAL;
}


#if 0
int Vision::storeSanityValues(unsigned char *planeArray, int offset) {
	int i, j, total;
	unsigned char * dataPtr;

	for (i = 0; i < NUM_SANITY_COORDS; i++) {
		total = vision.getSanityArray(i);
		dataPtr = (unsigned char *) &total;
		for (j = 0; j < INT_BYTES; j++) {
			planeArray[offset++] = dataPtr[j];
		}

		if (sanityValuesDebugMsg) {
			cout << "Vision.cc: i = " << i << "; NUM_SANITY_COORDS = "
				<< NUM_SANITY_COORDS << "; total = " << total << "; calculated ";
			cout
				<< ((int) ((planeArray[offset - 4] & 0xff)))
				+ ((int) ((planeArray[offset - 3] & 0xff) << 8))
				+ ((int) ((planeArray[offset - 2] & 0xff) << 16))
				+ ((int) ((planeArray[offset - 1] & 0xff) << 24))
				<< endl;
		}
	}
	return offset;
}
#endif

int Vision::storeSensors(unsigned char *planeArray, int offset) {
	int value, i, j;
	unsigned char * dataPtr;

	if (sensorValueDebugMsg) {
		cout << "Sensor: ";
	}

	for (i = 0; i < NUM_SENSOR_COORDS; i++) {
		value = vision.getSensor(i);
		dataPtr = (unsigned char *) &value;
		for (j = 0; j < LONG_BYTES; j++) {
			planeArray[offset++] = dataPtr[j];
		}
		if (sensorValueDebugMsg) {
			cout << value << " ";
		}
	}
	if (sensorValueDebugMsg) {
		cout << value << endl;
	}

	return offset;
}

void Vision::sendYUVPlane() {
#if 0 // to client
    int msgType = msgTypeNORMAL;
    int dataType = shmdtYUVPlaneCPlane ;
    int arrayLength = CPLANE_SIZE;

    memcpy(sharedDebugMemory->yuvCplane_data, vision.cplane, arrayLength);

    objectOut->ClearBuffer();
    objectOut->SetData(&msgType, sizeof(int));
    objectOut->SetData(&dataType, sizeof(int));
    objectOut->SetData(&arrayLength, sizeof(int));
    objectOut->NotifyObservers();

    if (yuvDebugMsg) {
        cout << "Vision.cc: sendYUVPlane - CPlane Size : " << arrayLength << endl;
    }

    dataType = shmdtYUVPlane ;
    arrayLength = YUVPLANE_SIZE;

    memcpy(sharedDebugMemory->yuvplane_data, vision.yplane, arrayLength);

    objectOut->ClearBuffer();
    objectOut->SetData(&msgType, sizeof(int));
    objectOut->SetData(&dataType, sizeof(int));
    objectOut->SetData(&arrayLength, sizeof(int));
    objectOut->NotifyObservers();

    if (yuvDebugMsg) {
        cout << "Vision.cc: sendYUVPlane - YPlane Size : " << arrayLength << endl;
    }
#endif
    // send to robolink. This should replace the above
    if (! (send_data[RLNK_PWALK] && send_data[RLNK_ALL]) )
        vision.sendPWalkSensors();
    Robolink::sendToBase(vision.yplane, RLNK_YUVPLANE, YUVPLANE_SIZE);
}

/* Sends the YUVPlane with only quarter resolution Y (i.e. matching the U/V
 * resolution
 */
void Vision::sendYUVPlaneLowRes() {
    //FIXME
}

/* Send the dog MAC address */
void Vision::sendDogId(void) {
#ifndef OFFLINE
    struct rlnk_dogid data;
    data.mac = ethernetMACaddress;

    Robolink::sendToBase((byte*)&data, RLNK_DOGID, sizeof(data));
#endif
}

void Vision::sendAA(AtomicAction &nextAA) {
    //sExecuteAA->ClearBuffer(obsExecuteAA); //obsExecuteAA is the observer index of the observer reveiving a msg from this transmission.. number is generated automatically by stub.cfg
    sExecuteAA->ClearBuffer(); //obsExecuteAA is the observer index of the observer reveiving a msg from this transmission.. number is generated automatically by stub.cfg
    sExecuteAA->SetData(&nextAA, sizeof(nextAA));
    sExecuteAA->NotifyObservers();
}

void Vision::sendOP(OffsetParams &nextOP) {
    sGaitOptimiser->ClearBuffer();
    sGaitOptimiser->SetData(&nextOP, sizeof(nextOP));
    sGaitOptimiser->NotifyObservers();
}

void Vision::sendLearningParams(WalkLearningParams &params) {
    sWalkingLearner->ClearBuffer();
    sWalkingLearner->SetData(&params, sizeof(params));
    sWalkingLearner->NotifyObservers();
}


void Vision::sendIndicators(int *inds, size_t sz) {
    if (indicatorState) {
        sSetIndication->SetData(inds, sz);
        sSetIndication->NotifyObservers();
    }
}

    
#ifdef USE_TCP
/* Initializes the TCP connection with the given id */
OStatus Vision::InitTCPConnection(int id) {
	OSYSDEBUG(("Vision::InitTCPConnection()\n"));

	tcpconnections[id].state = CONNECTION_CLOSED;

	/* Allocate send buffer */
	antEnvCreateSharedBufferMsg sendBufferMsg(TCP_SEND_BUF_SIZE);

	sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
	if (sendBufferMsg.error != ANT_SUCCESS) {
		OSYSLOG1((osyslogERROR, "Vision::InitTCPConnection() : Can't allocate send buffer[%d] antError %d", id, sendBufferMsg.error));
		return oFAIL;
	}

	sendBufferMsg.buffer.Map();
	tcpconnections[id].sendData = (byte *) (sendBufferMsg.buffer.GetAddress());

	/* Allocate receive buffer */
	antEnvCreateSharedBufferMsg receiveBufferMsg(TCP_RECV_BUF_SIZE);

	receiveBufferMsg.Call(ipstackRef, sizeof(receiveBufferMsg));
	if (receiveBufferMsg.error != ANT_SUCCESS) {
		OSYSLOG1((osyslogERROR,
"Wireless::InitTCPConnection() : Can't allocate receive buffer[%d] antError %d",
                id, receiveBufferMsg.error));
		return oFAIL;
	}

	tcpconnections[id].receiveBuffer = receiveBufferMsg.buffer;
	tcpconnections[id].receiveBuffer.Map();
	tcpconnections[id].receiveData = (byte *)
		(tcpconnections[id].receiveBuffer.GetAddress());

	return oSUCCESS;
}

/* Listen on the specified TCP port. This returns a connection id for the
 * listening socket.
 */
OStatus Vision::TCPListen(int port, int* id) {
    static int last_tcp_id = -1;

    if (last_tcp_id + 1 < NUM_TCP_CONNECTIONS) {
        *id = ++last_tcp_id;
    } else {
		OSYSLOG1((osyslogERROR,
                "Vision::TCPListen: no more TCP connections\n"));
        return oFAIL;
    }

	if (tcpconnections[*id].state != CONNECTION_CLOSED) {
		OSYSLOG1((osyslogERROR,
                "Vision::TCPListen: connection %d not closed\n", *id));
		return oFAIL;
	}

    tcpconnections[*id].port = port;
    return TCPListen(*id);
}


/* Private TCPListen called by the public version and by TCPClose to
 * reconnect. Listens on the specified TCP connection.
 */
OStatus Vision::TCPListen(int id) {
	OSYSDEBUG(("Vision::TCPListen(%d)\n", id));
	// Create endpoint
	antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
                                        TCP_INTERNAL_BUF_SIZE);

	tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
	if (tcpCreateMsg.error != ANT_SUCCESS) {
		OSYSLOG1((osyslogERROR,
                "Vision::TCPListen() : Can't create endpoint[%d] antError %d",
                id, tcpCreateMsg.error));
		return oFAIL;
	}
	tcpconnections[id].endpoint = tcpCreateMsg.moduleRef;

	// Listen
	TCPEndpointListenMsg listenMsg(tcpconnections[id].endpoint,
                                    IP_ADDR_ANY, tcpconnections[id].port);
	listenMsg.continuation = (void *) id;

	listenMsg.Send(ipstackRef, myOID_, Extra_Entry[entryTCPListenCont],
                    sizeof(listenMsg));

	tcpconnections[id].state = CONNECTION_LISTENING;
	return oSUCCESS;
}

/* Callback for TCPListen */
void Vision::TCPListenCont(void *msg) {
	TCPEndpointListenMsg *listenMsg = (TCPEndpointListenMsg *) msg;
	int id = (int) listenMsg->continuation;

	OSYSDEBUG(("Vision::TCPListenCont(%d)\n", id));

	if (listenMsg->error != TCP_SUCCESS) {
		OSYSLOG1((osyslogERROR,
                "Vision::TCPListenCont() : FAILED. listenMsg->error %d",
                listenMsg->error));
		TCPClose(id);
		return;
	}

	tcpconnections[id].state = CONNECTION_CONNECTED;
}

/* Queues data up to be sent when the connection is not busy. You should
 * use this rather than calling TCPSend directly. Returns oSUCCESS if the data
 * is queued correctly (this does not garuntee sending). Returns oFAIL
 * if size == 0, the connection is invalid, or if the queue is full.
 * The data will not be sent until TCPSend is invoked, which should be every
 * frame.
 */
OStatus Vision::TCPQueue(int id, byte* data, int size) {
//    cerr << "TCPQueue " << id << ", " << size << "bytes" << endl;
    if (tcpconnections[id].state != CONNECTION_CONNECTED
            /*&& tcpconnections[id].state != CONNECTION_SENDING*/) {
//		OSYSLOG1((osyslogERROR,
//                "Vision::TCPQueue failed: conn_id = %d, state = %d",
//                id, tcpconnections[id].state));
        return oFAIL;
    }
    if (size <= 0)
        return oFAIL;

    if (size + tcpconnections[id].sendSize > TCP_SEND_BUF_SIZE) {
		OSYSLOG1((osyslogERROR,
                "Vision::TCPQueue failed: conn_id = %d, buffer full", id));
        return oFAIL;
    }
    
    // append the data to the shared buffer
    memcpy(tcpconnections[id].sendData + tcpconnections[id].sendSize,
            data, size);
    tcpconnections[id].sendSize += size;
    
    return oSUCCESS;
}


/* Send data over the specified TCP connection. The data and it's length
 * must be already loaded into the tcpconnections[id] send buffer.
 */
OStatus Vision::TCPSend(int id/*, byte* data int bufId, int size*/) {
//	OSYSDEBUG(("Vision::TCPSend()\n"));
//    cout << "Vision::TCPSend " << id << ", " << tcpconnections[id].sendSize
//        << " bytes" << endl;

    if (tcpconnections[id].sendSize == 0)
        return oSUCCESS;

    // Alexn: despite the OPENR doc you can send multiple TCPSend
    // messages to the stack and they will usually all be processed even
    // before one has had the callback invoked. However since we are sending
    // from the single buffer if we allow CONNECTION_SENDING then it will send
    // the same data twice (TCPQueues this frame will have failed too)
	if (tcpconnections[id].state != CONNECTION_CONNECTED) {
//		OSYSLOG1((osyslogERROR,
//                "Vision::TCPSend failed: conn_id = %d, state = %d, size = %d",
//                id, tcpconnections[id].state, tcpconnections[id].sendSize));
		return oFAIL;
	}

	if (TCP_SEND_BUF_SIZE < tcpconnections[id].sendSize) {
		OSYSLOG1((osyslogERROR,
                "Vision: Error: send buffer size smaller than the actual "
                "data size; send buffer size: %d, send data size %n",
                TCP_SEND_BUF_SIZE, tcpconnections[id].sendSize));
		return oFAIL;
	}

	tcpconnections[id].state = CONNECTION_SENDING;

	TCPEndpointSendMsg sendMsg(tcpconnections[id].endpoint,
			tcpconnections[id].sendData,
			tcpconnections[id].sendSize);

	sendMsg.continuation = (void *) id;

	sendMsg.Send(ipstackRef, myOID_, Extra_Entry[entryTCPSendCont],
            sizeof(sendMsg));

	return oSUCCESS;
}

/* Callback from TCPSend */
void Vision::TCPSendCont(void *msg) {
//	OSYSDEBUG(("Vision::TCPSendCont()\n"));

	TCPEndpointSendMsg *sendMsg = (TCPEndpointSendMsg *) msg;
	int id = (int) (sendMsg->continuation);

	if (sendMsg->error != TCP_SUCCESS) {
		OSYSLOG1((osyslogERROR,
                "Vision::TCPSendCont() : FAILED. sendMsg->error %d",
                sendMsg->error));
        if (sendMsg->error != TCP_CONNECTION_BUSY) {
            TCPClose(id);
        }
        // Truncate the data rather than trying to send it again
        tcpconnections[id].sendSize = 0;
		return;
	}

	tcpconnections[id].sendSize = 0;
	tcpconnections[id].state = CONNECTION_CONNECTED;

}

/* Receive data on the specified TCP channel */
OStatus Vision::TCPReceive(int id, byte* buf, int size) {
//	OSYSDEBUG(("Vision::Receive()\n"));

	if (tcpconnections[id].state != CONNECTION_CONNECTED
			&& tcpconnections[id].state != CONNECTION_SENDING) {
		OSYSLOG1((osyslogERROR, "Vision::TCPReceive: connection idle."));
		return oFAIL;
	}

	if (TCP_RECV_BUF_SIZE < size) {
		OSYSLOG1((osyslogERROR,
                "Vision.cc: Error: receive buffer size smaller than  then "
                "requested data size. Receive buffer size: %d, requested "
                "data size: %d\n", TCP_RECV_BUF_SIZE, size));
		return oFAIL;
	}

    /* Copy data */
    //memcpy(tcpconnections[id].receiveData, buf, size);

	// create a message with the (exact) amount of data to be received
	TCPEndpointReceiveMsg receiveMsg(tcpconnections[id].endpoint,
			tcpconnections[id].receiveData, size, size);
	receiveMsg.continuation = (void *) id;
	receiveMsg.Send(ipstackRef,
			myOID_,
			Extra_Entry[entryTCPReceiveCont],
			sizeof(receiveMsg));
	return oSUCCESS;
}

/* Callback from TCPReceive - has the received data */
void Vision::TCPReceiveCont(void *msg) {
//	OSYSDEBUG(("Vision::TCPReceiveCont()\n"));

	TCPEndpointReceiveMsg *receiveMsg = (TCPEndpointReceiveMsg *) msg;
	int id = (int) (receiveMsg->continuation);

	if (receiveMsg->error != TCP_SUCCESS) {
		OSYSLOG1((osyslogERROR,
                "Wireless::TCPReceiveCont() : FAILED. receiveMsg->error %d",
                receiveMsg->error));
		TCPClose(id);
		return;
	}

    // FIXME: Right now received data is ignored. The options are
    // 1. To call a predefined function to handle recieved data (like
    //    handlepacket in Wireless)
    // 2. Store a callback in the tcpconnection. The callback must be static
    //    or to a predefined objcet (Vision)
	//TCPReceive(id);

}

/* Close the specified TCP connection */
OStatus Vision::TCPClose(int id) {
	OSYSDEBUG(("Vision::TCPClose()\n"));

	if (tcpconnections[id].state == CONNECTION_CLOSED
			|| tcpconnections[id].state == CONNECTION_CLOSING) {
		return oFAIL;
	}

	TCPEndpointCloseMsg closeMsg(tcpconnections[id].endpoint);
	closeMsg.continuation = (void *) id;

	closeMsg.Send(ipstackRef,
			myOID_,
			Extra_Entry[entryTCPCloseCont],
			sizeof(closeMsg));

	tcpconnections[id].state = CONNECTION_CLOSING;
	return oSUCCESS;
}

/* Callback for TCPClose. Attempts to listen on the same port again to 
 * retry the connection
 */
void Vision::TCPCloseCont(void *msg) {
	OSYSDEBUG(("Vision::TCPCloseCont()\n"));

	TCPEndpointCloseMsg *closeMsg = (TCPEndpointCloseMsg *) msg;
	int id = (int) (closeMsg->continuation);

	tcpconnections[id].state = CONNECTION_CLOSED;
	TCPListen(id);
}
#endif // USE_TCP
