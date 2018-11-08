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
 * UNSW 2003 Robocup
 *
 * Last modification background information
 * $$
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef CommandData_h_def
#define CommandData_h_def

#include "PackageDef.h"
#include "WirelessSwitchboard.h"
#include "RoboCupGameControlData.h"

// Max size of raw data in CommandData object */
enum {MAX_COMMAND_RAW_SIZE = 200};

typedef enum {
	ROBOCOMMANDER_DATA, // command from UNSW RoboCommander
	GAMECONTROLLER_DATA // command from Sony Game Controller
} CommandType;


// The possible commands to give the dog from robocommander
typedef enum {
	NO_COMMAND,
	MODE_CHANGE,
	UTHER_KICKING_OFF,
	PLAYER_CHANGE,
	WALKTYPE_CHANGE,
	WALKPARAM1_CHANGE,  // 5
	WALKPARAM2_CHANGE,
	WALKPARAM3_CHANGE,
	FORWARD_MOVE,
	HEAD_MOVE,
	LEFT_MOVE,          //10
	TURN_MOVE,          
	SET_TILT,
	SET_TILT2,
	SET_PAN,
	DO_BLOCK_STANCE,    //15
	DO_HOLD_STANCE,
	NORMAL_STANCE_ABSH,
	NORMAL_STANCE_RELH,
	SET_STANCE,
	DO_TURN_KICK,       //20
	PAN_TILT,
	TILT_PAN,
	BALL_TRK,
	MOVE_COMMAND,
	HUMAN_CONTROL,      //25
	SET_COLOUR,
	SET_INDICATOR,
	EDGE_DETECT_DEBUG,
	CPLANE_SEND,
	BURST_MODE,         //30
	WM_SEND,
	YUVPLANE_SEND,
	OPLANE_SEND,
	BEACON_DEBUG,
	SET_BEACON_DIST_CONST, //35
	TEST_TURN_KICK,
	SET_CURRENTLY_CARRYING,
	DO_WM_LAT_TEST,
	CAL_TURN_KICK,
	WRITE_TO_CONSOLE,       //40
	RELAX_DOG,
	WALK_LEARNING,
	WALK_CALIBRATE,
    SEND_JOINT_DEBUG,
	GENERAL_TESTER_COMMAND,   //45
    SET_JOINT_SPEED_LIMIT,
    SHUT_DOWN_ROBOT,
    DANIEL_INDICATOR_TEST1,
    DANIEL_INDICATOR_TEST2,
    PYTHON_COMMAND,            //50
    COMMAND_TO_PYTHON_MODULE,            
    CHANGE_PIDGAIN,
    SET_CPLANE_SENDING_RATE,
    RELOAD_SPECIAL_ACTION,
	YUVPLANE_STREAM				//55
} Instruction;


// structure for commands, will be passed from wireless to vision
struct CommandData {
	CommandType type;

	union {
		struct {
			Instruction RC_COMMAND;
			double value;
			double value2;
			double value3;
			char rawValue[MAX_COMMAND_RAW_SIZE];
		};
		RoboCupGameControlData GC_DATA;
	};

	//default constructor
	CommandData();

	// constructor for robocommander commands
	CommandData(const PackageDef &p);

	// constructor for game controller commands
	CommandData(const RoboCupGameControlData &gc);

	// prints what the robocommander command is to cout
	void printRCCommand() const;

	// clears the contents of this command
	void clear();
};

/* Structure for TeamTalk as received by wireless (the message is variable
 * length).
 */
struct TeamTalk {
    char    header[4];  // always teamTalkHeader == {'N', 'S', 't', 't'}
    int     length;     // number of bytes in message
    void*   message;    // text, may contain nulls
};

#endif // CommandData_h_def
