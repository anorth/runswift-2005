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
 * UNSW 2003 Robocup (Alex Tang)
 *
 * Last modification background information
 * $Id: ClientSwitchBoard.h 1964 2003-08-26 15:26:27Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * same as the switch board file in robot/share/
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

#ifndef _ClientSwitchBoard_h_DEFINED
#define _ClientSwitchBoard_h_DEFINED

/*
 * whether CPlane will be log in client
**/
#define LOG_CPLANE
#ifdef JOYSTICK_CONTROL_D
#undef LOG_CPLANE
#endif

#endif // _ClientSwitchBoard_h_DEFINED
