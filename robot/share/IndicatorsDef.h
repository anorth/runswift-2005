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
 * $Id: IndicatorsDef.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef _IndicatorsDef_h_DEFINED
#define _IndicatorsDef_h_DEFINED

/* Indicator count constants. */
static const int NUM_OF_INDICATORS = 31;
static const int NUM_OF_INDICATOR_JOINTS = 5;
static const int NUM_OF_INDICATOR_LEDS = (NUM_OF_INDICATORS
		- NUM_OF_INDICATOR_JOINTS);
static const int NUM_OF_JOINT_IND_FRAMES = 2;
static const int NUM_OF_EAR_IND_FRAMES = 2;

/* Mouth constants. */
static const int IND_MOUTH_CLOSED = -10000;
static const int IND_MOUTH_OPEN = -950000;

/* Tail constants. */
static const int IND_TAIL_RIGHT = 1000000;
static const int IND_TAIL_LEFT = -1000000;
static const int IND_TAIL_UP = 1000000;
static const int IND_TAIL_DOWN = 17000;

static const int IND_TAIL_H_CENTRED = (IND_TAIL_LEFT + IND_TAIL_RIGHT) / 2;
static const int IND_TAIL_V_CENTRED = (IND_TAIL_UP + IND_TAIL_DOWN) / 2;

/* Ear state constants. */
static const int IND_EAR_UP = 1;
static const int IND_EAR_DOWN = 2;

/* Face and back lights constants. */
static const int IND_LED3_INTENSITY_MIN = 0x00000000;
static const int IND_LED3_INTENSITY_MAX = 0x000000FF;

static const int IND_LED3_MODE_A = 0x00000000;
static const int IND_LED3_MODE_B = 0x00000F00;

static const int IND_LED2_ON = 1;
static const int IND_LED2_OFF = 2;

static const int IND_UNCHANGED  = -1;
static const int IND_LED_ON     = 1;
static const int IND_LED_OFF    = 0;

/* Indices for different sets of indicators. */
static const int IND_MOUTH_START = 0;
static const int IND_TAIL_START = 1;
static const int IND_EAR_START = 3;
static const int IND_FACE_LED3_START = 5;
static const int IND_LED2_START = 19;
static const int IND_BACK_LED3_START = 25;

#endif //_IndicatorsDef_h_DEFINED
