

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

#include <iostream>
#include <string.h>
#include "CommandData.h"
#include "debuggingMacros.h"

using namespace std;

static const bool bDebugCorruptedWireless = false;

// nameMatch represents a string representation of a command and it's
// Instruction equivalent
struct nameMatch {
	const char *name;
	Instruction inst;
};


// Add suitable members to this array if you are adding new robocommander commands
static const struct nameMatch matchArray[] = {
	{"yuv", YUVPLANE_STREAM},
	{"rsa", RELOAD_SPECIAL_ACTION},
	{"sfr", SET_CPLANE_SENDING_RATE},
	{"pid", CHANGE_PIDGAIN},
	{"py", PYTHON_COMMAND},
	{"pyc", COMMAND_TO_PYTHON_MODULE},
	{"off", SHUT_DOWN_ROBOT},
	{"jsp", SET_JOINT_SPEED_LIMIT},
	{"jd", SEND_JOINT_DEBUG},
	{"wrl", WALK_LEARNING},
	{"wtcal", WALK_CALIBRATE},
	{"gtr",GENERAL_TESTER_COMMAND},
	{"relax", RELAX_DOG},
	{"debug", WRITE_TO_CONSOLE},
	{"mode", MODE_CHANGE},
	{"ko", UTHER_KICKING_OFF},
	{"p", PLAYER_CHANGE},
	{"wt", WALKTYPE_CHANGE},
	{"wp1", WALKPARAM1_CHANGE},
	{"wp2", WALKPARAM2_CHANGE},
	{"wp3", WALKPARAM3_CHANGE},
	{"f", FORWARD_MOVE},
	{"head", HEAD_MOVE},
	{"l", LEFT_MOVE},
	{"t", TURN_MOVE},
	{"v", SET_TILT},
	{"v2", SET_TILT2},
	{"h", SET_PAN},
	{"B", DO_BLOCK_STANCE},
	{"H", DO_HOLD_STANCE},
	{"N", NORMAL_STANCE_ABSH},
	{"n", NORMAL_STANCE_RELH},
	{"K", SET_STANCE},
	{"T", DO_TURN_KICK},
	{"u", PAN_TILT},
	{"k", TILT_PAN},
	{"q", BALL_TRK},
	{"j", MOVE_COMMAND},
	{"c", HUMAN_CONTROL},
	{"r", SET_COLOUR},
	{"i", SET_INDICATOR},
	{"g", EDGE_DETECT_DEBUG},
	{"d", CPLANE_SEND},
	{"burst", BURST_MODE},
	{"w", WM_SEND},
	{"y", YUVPLANE_SEND},
	{"z", OPLANE_SEND},
	{"e", BEACON_DEBUG},
	{"b", SET_BEACON_DIST_CONST},
	{"E", TEST_TURN_KICK},
	{"Z", SET_CURRENTLY_CARRYING},
	{"lat", DO_WM_LAT_TEST},
	{"ctk", CAL_TURN_KICK},
    {"d1", DANIEL_INDICATOR_TEST1},
    {"d2", DANIEL_INDICATOR_TEST2},    
	{"none", NO_COMMAND} // this must be at the end
};


/* Called by Wireless.cc. */
CommandData::CommandData() {
	type = ROBOCOMMANDER_DATA;
	RC_COMMAND = NO_COMMAND;
	value = 0;
	memset(rawValue, 0, sizeof(rawValue));
}


/* Called by Wireless.cc. */
CommandData::CommandData(const PackageDef &p) {
	type = ROBOCOMMANDER_DATA;
    if (bDebugCorruptedWireless){
        cout << "CommandData: p.name=" << p.name << " p.value=" << p.value << endl;
    }
	bool set = false;
	for (int i = 0 ; strcmp(matchArray[i].name, "none") != 0 ; i++) {
		if (strcmp(matchArray[i].name, p.name) == 0) {
			RC_COMMAND = matchArray[i].inst;
			set = true;
			break;
		}
	}

	if (!set) {
		RC_COMMAND = NO_COMMAND;
	}

	strncpy(rawValue, p.value, sizeof(rawValue));
	cout << "Copying " << p.value << " len = " << strlen(p.value) << endl;

	int n = sscanf(p.value, "%lf %lf %lf", &value, &value2, &value3);    
	//value = atof(p.value);
	if (n < 3) {
		value3 = 0.0;
		if (n < 2) {
			value2 = 0.0;
			if (n < 1) {
				value = 0.0;
			}
		}
	} 
	printRCCommand();
	std::cout << " -> " << p.name << " , " << p.value << " - {" << value << ","
		<< value2 << "," << value3 << "}" << std::endl;
}

/* Called by Wireless.cc. */
CommandData::CommandData(const RoboCupGameControlData &gc) {
	memset(rawValue, 0, sizeof(rawValue));
	type = GAMECONTROLLER_DATA;
	GC_DATA = gc;
}

void CommandData::printRCCommand() const {
	switch (RC_COMMAND) {
		case NO_COMMAND: std::cout << "NO_COMMAND";
				 break;
		case MODE_CHANGE: std::cout << "MODE_CHANGE";
				  break;
		case UTHER_KICKING_OFF: std::cout << "UTHER_KICKING_OFF";
					break;
		case PLAYER_CHANGE: std::cout << "PLAYER_CHANGE";
				    break;
		case WALKTYPE_CHANGE: std::cout << "WALKTYPE_CHANGE";
				      break;
		case WALKPARAM1_CHANGE: std::cout << "WALKPARAM1_CHANGE";
					break;
		case WALKPARAM2_CHANGE: std::cout << "WALKPARAM2_CHANGE";
					break;
		case WALKPARAM3_CHANGE: std::cout << "WALKPARAM3_CHANGE";
					break;
		case FORWARD_MOVE: std::cout << "FORWARD_MOVE";
				   break;
		case LEFT_MOVE: std::cout << "LEFT_MOVE";
				break;
		case TURN_MOVE: std::cout << "TURN_MOVE";
				break;
		case SET_TILT: std::cout << "SET_TILT";
			       break;
		case SET_TILT2: std::cout << "SET_TILT2";
				break;
		case SET_PAN: std::cout << "SET_PAN";
			      break;
		case DO_BLOCK_STANCE: std::cout << "DO_BLOCK_STANCE";
				      break;
		case DO_HOLD_STANCE: std::cout << "DO_HOLD_STANCE";
				     break;
		case NORMAL_STANCE_ABSH: std::cout << "NORMAL_STANCE_ABSH";
					 break;
		case NORMAL_STANCE_RELH: std::cout << "NORMAL_STANCE_RELH";
					 break;
		case SET_STANCE: std::cout << "SET_STANCE";
				 break;
		case DO_TURN_KICK: std::cout << "DO_TURN_KICK";
				   break;
		case PAN_TILT: std::cout << "PAN_TILT";
			       break;
		case TILT_PAN: std::cout << "TILT_PAN";
			       break;
		case BALL_TRK: std::cout << "BALL_TRK";
			       break;
		case MOVE_COMMAND: std::cout << "MOVE_COMMAND";
				   break;
		case HUMAN_CONTROL: std::cout << "HUMAN_CONTROL";
				    break;
		case SET_COLOUR: std::cout << "SET_COLOUR";
				 break;
		case SET_INDICATOR: std::cout << "SET_INDICATOR";
				    break;
		case EDGE_DETECT_DEBUG: std::cout << "EDGE_DETECT_DEBUG";
					break;
		case CPLANE_SEND: std::cout << "CPLANE_SEND";
				  break;
		case BURST_MODE: std::cout << "BURST_MODE";
				 break;
		case WM_SEND: std::cout << "WM_SEND";
			      break;
		case YUVPLANE_SEND: std::cout << "YUVPLANE_SEND";
				    break;
		case OPLANE_SEND: std::cout << "OPLANE_SEND";
				  break;
		case BEACON_DEBUG: std::cout << "BEACON_DEBUG";
				   break;
		case SET_BEACON_DIST_CONST: std::cout << "SET_BEACON_DIST_CONST";
					    break;
		case TEST_TURN_KICK: std::cout << "TEST_TURN_KICK";
				     break;
		case SET_CURRENTLY_CARRYING: std::cout << "SET_CURRENTLY_CARRYING";
					     break;
		case DO_WM_LAT_TEST: std::cout << "DO_WM_LAT_TEST";
				     break;
		case CAL_TURN_KICK: std::cout << "DO_CAL_TURN_KICK";
				    break;      
		case PYTHON_COMMAND: std::cout << "DO_PYTHON_COMMAND";
				    break;      
		case COMMAND_TO_PYTHON_MODULE: std::cout << "COMMAND_TO_PYTHON_MODULE";
				    break;      
		default: 
                            std::cout << "Unknown " << RC_COMMAND <<  endl;
                            break;
	}
}


void CommandData::clear() {
	type = ROBOCOMMANDER_DATA;
	RC_COMMAND = NO_COMMAND;
	memset(rawValue, 0, sizeof(rawValue));
}



