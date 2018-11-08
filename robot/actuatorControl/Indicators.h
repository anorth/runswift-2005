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
 * $Id: Indicators.h 4812 2005-02-07 06:37:54Z shnl327 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 * responsible for the movement of tails, eyes, mouth, light, etc
 *
 **/

#ifndef _Indicators_h_DEFINED
#define _Indicators_h_DEFINED

#include <OPENR/OSubject.h>
#include <OPENR/OObserverVector.h>
#include <OPENR/OPENREvent.h>
#include <OPENR/OPENRAPI.h>

#include "EffectorCommander.h"
#include "def.h"

#include "primitives.h"
#include "../share/Common.h"
#include "../share/IndicatorsDef.h"

const char *const INDICATORS[] = {
	"PRM:/r1/c1/c2/c3/c4-Joint2:14",    // Mouth
	"PRM:/r6/c1-Joint2:61",             // Tail tilt
	"PRM:/r6/c2-Joint2:62",             // Tail pan
	"PRM:/r1/c1/c2/c3/e5-Joint4:15",    // Left ear
	"PRM:/r1/c1/c2/c3/e6-Joint4:16",    // Right ear
	"PRM:/r1/c1/c2/c3/la-LED3:la",      // Face light 1
	"PRM:/r1/c1/c2/c3/lb-LED3:lb",      // Face light 2
	"PRM:/r1/c1/c2/c3/lc-LED3:lc",      // Face light 3
	"PRM:/r1/c1/c2/c3/ld-LED3:ld",      // Face light 4
	"PRM:/r1/c1/c2/c3/le-LED3:le",      // Face light 5
	"PRM:/r1/c1/c2/c3/lf-LED3:lf",      // Face light 6
	"PRM:/r1/c1/c2/c3/lg-LED3:lg",      // Face light 7
	"PRM:/r1/c1/c2/c3/lh-LED3:lh",      // Face light 8
	"PRM:/r1/c1/c2/c3/li-LED3:li",      // Face light 9
	"PRM:/r1/c1/c2/c3/lj-LED3:lj",      // Face light 10
	"PRM:/r1/c1/c2/c3/lk-LED3:lk",      // Face light 11
	"PRM:/r1/c1/c2/c3/ll-LED3:ll",      // Face light 12
	"PRM:/r1/c1/c2/c3/lm-LED3:lm",      // Face light 13
	"PRM:/r1/c1/c2/c3/ln-LED3:ln",      // Face light 14
	"PRM:/r1/c1/c2/c3/l1-LED2:l1",      // Head light (color)
	"PRM:/r1/c1/c2/c3/l2-LED2:l2",      // Head light (white)
	"PRM:/r1/c1/c2/c3/l3-LED2:l3",      // Mode indicator (red)
	"PRM:/r1/c1/c2/c3/l4-LED2:l4",      // Mode indicator (green)
	"PRM:/r1/c1/c2/c3/l5-LED2:l5",      // Mode indicator (blue)
	"PRM:/r1/c1/c2/c3/l6-LED2:l6",      // Wireless light
	"PRM:/lu-LED3:lu",                  // Back light (front, color)
	"PRM:/lv-LED3:lv",                  // Back light (front, white)
	"PRM:/lw-LED3:lw",                  // Back light (middle, color)
	"PRM:/lx-LED3:lx",                  // Back light (middle, white)
	"PRM:/ly-LED3:ly",                  // Back light (rear, color)
	"PRM:/lz-LED3:lz"                   // Back light (rear, white)

};

class Indicators {
	private:
		OCommandVectorData *indVec_;
		MemoryRegionID indVecID_;
		RCRegion *indRegion;
		int inds[NUM_OF_INDICATORS];
		void SetUpBuffer(EffectorCommander &eCommander);

	public:
		Indicators(OSubject *subject, EffectorCommander &eCommander);
		~Indicators() {
		}
		OSubject *sIndicators;
		void setIndication(const ONotifyEvent &event);       
        
};

#endif

