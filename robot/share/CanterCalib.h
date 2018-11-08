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
 * Last modification background information
 * $Id: CanterCalib.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#ifndef _canterCalib_h
#define _canterCalib_h

namespace Canter {
	static inline double forwardCalib(const double forward,
			const double /*left*/,
			const double /*turnCCW*/) {
		double fc = 0;

		if (forward > 0) {
			fc = 5.02 * forward + 2.5614;
		}
		else if (forward < 0) {
			fc = 4.62 * forward - 6.0006;
		}

		return fc;
	}

	static inline double leftCalib(const double /*forward*/,
			const double left,
			const double /*turnCCW*/) {
		double sc = 0;

		if (left == 0) {
			sc = 0;
		}
		else if (left > 7.2) {
			sc = 52.33768448; 

			/* 11.1632576; 
			   0.0172*pow(left, 3)[6.4198656] - 0.0028*pow(left, 2)[0.145152] +
			   0.6482*left -0.0688[4.59824]; */
		}
		else if (left < -7.2) {
			sc = -52.33768448; 
			/* -11.1632576; */
		}
		else {
			sc = 0.0035 * pow(left, 5)
				+ 0.0016 * pow(left, 4) - 0.1672 * pow(left,
						3)
				- 0.0771 * pow(left,
						2) + 6.4649 * left
				+ 0.1724;

			/* sc = 0.0035*pow(left, 5)[67.72211712] +
			   0.0016*pow(left, 4)[4.29981696] - 0.1672*pow(left, 3)[62.4070656] - 0.0771*pow(left, 2)[3.996864] +
			   6.4649*left + 0.1724[46.71968]; */
		}

		return sc;
	}

	static inline double turnCCWCalib(const double forward,
			const double left,
			const double turnCCW) {
		double tc = 0;
		if (turnCCW > 0) {
			tc = 0.953 * turnCCW; 
			/* + 2.4048; */
		}
		else if (turnCCW < 0) {
			tc = 0.951 * turnCCW;
		}
		/* - 3.6498; */

		if (left < 0 && forward >= 0) {
			tc -= 2.4;
		}

		if (left == 0 && forward == 0 && turnCCW > 0) {
			tc += 2.4048;
		}
		else if (left == 0 && forward == 0 && turnCCW < 0) {
			tc -= 3.6498;
		}

		return tc;
	}
}

#endif // _canterCalib_h
