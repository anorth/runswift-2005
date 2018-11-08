/*

   Copyright 2003 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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
 * UNSW 2003 Robocup (Nicodemus Sutanto)
 *
 * Last modification background information
 * $Id: SharedMemoryDef.h 7128 2005-07-02 15:09:58Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef SharedMemoryDef_h_DEFINED
#define SharedMemoryDef_h_DEFINED

#ifndef OFFLINE
#include <OPENR/OObject.h>
#endif // OFFLINE

#include <ostream>

#include "SwitchBoard.h"
#include "VisionDef.h"
#include "PackageDef.h"
#include "Common.h"
#include "WirelessSwitchboard.h"
#include "hton.h"


// For inter-robot latency testing
static const int NORMAL_WM = 0;
static const int INIT_LAT_WM = -1;

// used to store information form kalman filter updates for sending to teammates
// uses floats to save space, floats are 4 bytes, doubles are 8 bytes
struct IFKUpdateData {
	float iVecx;
	float iVecy;

	float iMat11;
	float iMat12;
	float iMat21;
	float iMat22;
};


static const int NUM_OBSTACLE_SHARE = 24;
// this struct is used to compress obstacle information for sending to teammates
struct ObstacleShareObj { 
    unsigned int x     : 10;
    unsigned int y     : 10; 
    unsigned int count : 12;    
    
    ObstacleShareObj() { 
        x = 0; 
        y = 0; 
        count = 0;
    }    
    
    ObstacleShareObj(int i) { 
        memcpy(this,&i,sizeof(int));
    }
    
    int convert() { 
        int i; 
        memcpy(&i,this,sizeof(ObstacleShareObj)); 
        return i;
    }
};

// The information to be passed when sharing world model
// with other robots
struct WMShareObj {
	//WMObj obj;
	float posx, posy;
	float posh;
	float posVar, hVar;

	unsigned int behavioursVal;
	//team maverick stuff
	/*
	   unsigned int ballDist:29;
	   unsigned int amThirdPlayer:1;
	   unsigned int playerType:2;
	   */

	double ballx;
	double bally;
	double ballvar;

	// information for information form kalman filter tracking opponents
	//IFKUpdateData opponentsUpdate[NUM_OPPONENTS];
        
        // information for obstacles
        unsigned int obstacleVals[NUM_OBSTACLE_SHARE];

#ifdef IR_LATENCY_TEST
	// will be NORMAL_WM, INIT_LAT_WM, or the player number to the robot
	// we are responding to for latency testing
	int latMode;
	SystemTime waited;
#endif // IR_LATENCY_TEST
};

struct WMShareInfo {
	char header[4];
	int playerNum;
	int team;
	WMShareObj obj;
};

#ifdef DELAY_TESTING
static const bool isDelayTesting = true;
#else
static const bool isDelayTesting = false;
#endif

//Define Type of the message that is being sent to Wireless
//shmdt = Share Mem Data Type
enum {
	shmdtMessages,
	shmdtCPlane = 8,  //just to be competible with legacy CPLANE_DATA_TYPE ..., if things work out, remove it
	shmdtOPlane = 9,
	shmdtYUVPlaneCPlane = 10,
	shmdtYUVPlane = 11,
	shmdtActuatorMessage = 15,
	shmdtEdgeDetectDebug = 20,
	shmdtWorldModel = 100
};


// Debug information to be sent to Robocommander
struct RC_DEBUG_DATA {
	char message[100];

	char world_model_debug_data[560];

#ifdef DELAY_TESTING
	char cplane_data[INT_BYTES + COMPRESSED_CPLANE_SIZE + SENSOR_SIZE
        + SANITY_SIZE + OBJ_SIZE + GPS_SIZE + 4]; // for counter
#else
	char cplane_data[INT_BYTES + COMPRESSED_CPLANE_SIZE + SENSOR_SIZE
        + SANITY_SIZE + OBJ_SIZE + GPS_SIZE];
#endif // DELAY_TESTING

	char oplane_data[OBJ_SIZE];

	char yuvplane_data[YUVPLANE_SIZE];
	char yuvCplane_data[CPLANE_SIZE];

	char line_debug_data[CPLANE_SIZE *DEBUG_PLANES];
};




#endif // SharedMemoryDef_h_DEFINED
