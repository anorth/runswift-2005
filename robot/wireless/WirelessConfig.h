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
 * @author Nicodemus Sutanto 06-01-02
 *
 * Last modification background information
 * $Id: WirelessConfig.h 4812 2005-02-07 06:37:54Z shnl327 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 * sets wireless configuration of the robot
 *
 **/

#ifndef WirelessConfig_h_DEFINED
#define WirelessConfig_h_DEFINED

#include "../share/PackageDef.h"
#include "../share/WirelessTypeDef.h"
#include "../share/VisionDef.h"

static const int TCP_CONNECTION_MAX = 2;
static const int TCP_BUFFER_SIZE = sizeof(PackageDef) + YUVPLANE_SIZE;
static const int TCP_BUFFER_SIZE_SENDER = sizeof(PackageDef)
	+ YUVPLANE_SIZE;
	static const int TCP_BUFFER_SIZE_RECEIVER = 1024;
	static const int TCP_PORT = 54321;
	static const int TCP_PORT1 = TCP_PORT;
	static const int TCP_PORT2 = TCP_PORT + 1;
	static const int TCP_PORT3 = TCP_PORT + 2;

static const int UDP_CONNECTION_MAX = 4;
static const int UDP_BUFFER_SIZE    = 16384;
static const int UDP_PORT           = 54320;

#endif // WirelessConfig_h_DEFINED
