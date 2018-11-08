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
 * $Id: Timer.h 4363 2004-09-22 06:58:24Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef OFFLINE

class Timer {
	private:
		bool running;
		int startTime;

	public:
		Timer() {
			running = false;
		}

		void start() {
			startTime = 0; //dummy
			running = true;
		}

		double elapsed() {
			int endTime; 
			endTime = 0 ; //dummy
			running = false;
			return endTime - startTime;
		}

		bool isRunning() {
			return running;
		}
};


#else

#include <MCOOP.h>

class Timer {
	private:
		bool running;
		SystemTime startTime;

	public:
		Timer() {
			running = false;
		}

		void start() {
			GetSystemTime(&startTime);
			running = true;
		}

		double elapsed() {
			SystemTime endTime;
			GetSystemTime(&endTime);
			running = false;
			return double(endTime.seconds
					+ endTime.useconds
					/ 1000000.0
					- startTime.seconds
					- startTime.useconds
					/ 1000000.0);
		}

		bool isRunning() {
			return running;
		}
};

#endif //OFFLINE

#endif // _TIMER_H_
