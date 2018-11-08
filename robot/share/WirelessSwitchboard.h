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

#ifndef WirelessSwitchboard_h_DEFINED
#define WirelessSwitchboard_h_DEFINED

#define USE_TCP
//#undef USE_TCP

#define USE_UDP
//#undef USE_UDP

//#define IR_LATENCY_TEST
#undef IR_LATENCY_TEST

#define SENDING_DEBUG
//#undef SENDING_DEBUG

// WARNING - don't leave sending line debug on and sending of c plane off
// WARNING - don't leave LOG_YUV on

//#define SENDING_WORLD_MODEL_DEBUG--> Kim has removed all #define of this
//#undef SENDING_WORLD_MODEL_DEBUG
//#define SENDING_CPLANE   --> Kim has removed all #define of this
//#undef SENDING_CPLANE
//#define SENDING_OPLANE --> Kim has removed all #define of this
//#undef SENDING_OPLANE
//#define SENDING_YUVPLANE --> Kim has removed all #define of this
//#undef SENDING_YUVPLANE
#define SENDING_LINE_DEBUG
//#undef SENDING_LINE_DEBUG
//#define LOG_YUV
#undef LOG_YUV

#endif // WirelessSwitchboard_h_DEFINED
