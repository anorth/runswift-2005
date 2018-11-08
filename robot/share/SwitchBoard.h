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
 * UNSW 2003 Robocup (Eric Chung)
 *
 * Last modification background information
 * $Id: SwitchBoard.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * This file is for you to put all your hash defines that switch on and off 
 * certain sections of the code in the same place.
 *
 * This way, when you go to compile, you just need to check this file and turn
 * one and off whatever you want/need
 *
 * Make sure to include this file wherever you need the #defines defined
 * Try to keep this organised.. eg if the hash defines are for
 * the VisualCortex.cc file, then put them together under a heading.
 *
 **/

#ifndef _SwitchBoard_h_DEFINED
#define _SwitchBoard_h_DEFINED 

/////////////////////
// VisualCortex.cc //
/////////////////////

//only one of these 3 is to be defined at any 1 time
//#define OLD_C2C
//#define NEWCENTROID_C2C
#define NEWCENTROID_BEACONEXT_C2C

//Define EDGE_DETECT to enable use of the edge detect support 
//functions (projectPoints, etc.)
#define EDGE_DETECT
//#undef EDGE_DETECT

//Define ENABLE_EDGE_DETECT to actually run the edge detection 
//code every frame (this will slow things down!). 
#define ENABLE_EDGE_DETECT
//#undef ENABLE_EDGE_DETECT

// define FIREBALL to enable furball i mean fireball
#define FIREBALL
//#undef FIREBALL

#define SILENT

//////////////////
// ActionPro.cc //
//////////////////

//only one of these 2 is to be defined at anytime
#define PSTEP_PULL_INTERFACE   //if you dont tell the legs what to do, they keep doing the last thing you told it.. note stepComplete is always true
//#define PSTEP_PUSH_INTERFACE //old method, ie legs dont do anything once they have finished doing what you told them to do

///////////////////////
// SharedMemoryDef.h //
///////////////////////
/* To send a counter from vision to wireless to client.c to CPlaneDisplay.java
 * places needed to be change:
 * trunk/robot/share/SwitchBoard.h (DELAY_TESTING)
 * trunk/base/work/client/client.c (DELAY_TESTING)
 * trunk/base/work/RoboCommander/CPlaneDisplay.java (isDelayTesting)
 **/
//#define DELAY_TESTING

#endif // _SwitchBoard_h_DEFINED
