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
 * $Id: PackageDef.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/


#ifndef PackageDef_h_DEFINED
#define PackageDef_h_DEFINED

#include <cstdlib>

#include "WDataInfo.h"
#include "SwitchBoard.h"

//#define REDUCE_COMMAND_DATA_SIZE


// maximum lengths the sending values can be
// numbers should agree with base/work/RoboCommander/RoboConstant.java, 
// Hence, remake RoboCommander, client to pickup the changes

#ifdef REDUCE_COMMAND_DATA_SIZE
static const unsigned int NAME_SIZE = 6;
static const unsigned int VALUE_SIZE = 20;
#else
static const unsigned int NAME_SIZE = 72;
static const unsigned int VALUE_SIZE = 200;
#endif

class PackageDef {
	public:

		char name[NAME_SIZE];
		char value[VALUE_SIZE];
		WDataInfo nameInfo;
		WDataInfo valueInfo;

		PackageDef(char *myName,
				WDataInfo myNameInfo,
				char *myValue,
				WDataInfo myValueInfo) {
			strcpy(name, myName);
			strcpy(value, myValue);
			nameInfo = myNameInfo;
			valueInfo = myValueInfo;
		}

		~PackageDef() {
		}
};



class PackageBackDef {
	public:

		int sizeD;
		WDataInfo info;
		char *data;

		PackageBackDef(char *d, WDataInfo myInfo, int mySizeD) {
			data = (char *) malloc(sizeD);
			strcpy(data, d);
			sizeD = mySizeD;
			info = myInfo;
		}


		~PackageBackDef() {
			free(data);
		}
};

#endif // PackageDef_h_DEFINE
