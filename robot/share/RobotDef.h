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
 * @author Will Uther
 *
 * Last modification background information
 * $Id: RobotDef.h 6660 2005-06-17 01:48:17Z willu $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/


#ifndef RobotDef_h_DEFINED
#define RobotDef_h_DEFINED

static const long   CAMERA_FRAME_MS = 1000/30;
// number of milliseconds between camera frames
static const long   JOINT_FRAME_MS = 8;
// number of milliseconds between joint frames

static const double TURN_CENTRE_TO_NECK = 7.5;     //cm
static const double NECK_LENGTH = 8.00;    //cm ERS7

//Distance from head tilt center to camera PLANE
static const double FACE_LENGTH = 8.106; // cm

static const double PSD_HEADBASE_LENGTH = 2.5;
static const double PSD_HEADBASE_HEIGHT = 1.34;

// Shoulder heights in cm
static const double DESIRED_FRONT_HEIGHT = 10.0;
static const double DESIRED_BACK_HEIGHT = 13.2;

static const double HEAD_TILT_CENTER_TO_SHOULDER = 1.95;
static const double HEAD_TILT_CENTER_TO_SHOULDER_H = 0.25;
static const double NECKBASE_HEIGHT = DESIRED_FRONT_HEIGHT
+ HEAD_TILT_CENTER_TO_SHOULDER; //cm, may change due to stance

//body tilt in rad, may change due to stance
static const double TILT_OFFSET = 0.248709989;

static const double NECK_TO_ANKLE = 3.0;
static const double NECK_TO_CHEST = 1.8;
// paw kick constants
static const double RIGHT_PAW_OFFSET = -4.1;
static const double LEFT_PAW_OFFSET = 4.1;
static const double INTERCEPT_PAWKICK = 10.0;
static const double FAR_BALL_DIST = 30.0;
static const double CLOSE_BALL_DIST = 8.5;
static const double NECK_TO_PAWTIP = 5.0;
static const double BALL_OFFSET = 5.0;
static const double FULL_BODY_LENGTH = 24.0;    //cm

// Same as HEAD_TILT_CENTER_TO_SHOULDER, but in mm, not cm
static const double NECKBASE_SHOULDER_DISTANCE = 19.5;
// (mm) height difference between camera and tilt2 head pivot
static const double CAMERA_OFFSET = 14.6;

// these are the 95% confidence intervals for pan and tilt
// These were a rough estimation made on 27/05/03 - Ross
static const double PAN_95CF = 4.0; // degrees
static const double TILT_95CF = 3.0; // degrees


//ERS7 physical configuration, consult ModelInformation_7_E.pdf for details
//
static const double SHOULDER_TO_KNEE = 69.5; //old =  64;

//arm length
static const double KNEE_TO_PAW_FRONT = 77.36;
static const double KNEE_TO_PAW_REAR = 83.14;

//l3 in inverse-kinematics model
static const double KNEE_OFFSET = 9;

/* Length from shoulder to hip on each side. */
static const double BODY_LENGTH = 130; // old = 119 
static const double BODY_WIDTH = 134.4; // old = 119 

static const double SHOULDER_TO_BASE = 55;
static const double SHOULDER_TO_REAR = 20;
static const double SHOULDER_TO_FRONT = 30;

static const double FRONT_LEG_COWL_RADIUS = 65;	// radius of the bottom of the front legs

#endif // RobotDef_h_DEFINED

