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

#include "Behaviours.h"

void Behaviours::aaUPkickRight() {
  //saBallTracking();

  //if (vBall.d < 10 && tilty < -12) {
  /*  if ( vBall.d < 11.76 ) {
    //grabTime = 80;
		if (*currentWalkType != UpennRightWT)
			walkType = UpennRightWT;
		else {
		  //lockMode = NoMode; // so that the kick will not have ghost.
			grabInterval = 50;
		}
	}*/
  //setHighGain ();
  if (*currentWalkType == UpennRightWT) {
    lockMode = NoMode;
  }
  else {
    walkType = UpennRightWT;
  }
}

void Behaviours::aaUPkickLeft() {
  //saBallTracking();

//if (vBall.d < 14 && tilty < -12) {
/*  if ( vBall.d < 11.76 ) { 
    //grabTime = 80;
    if (*currentWalkType != UpennLeftWT)
			walkType = UpennLeftWT;
		else {
		  //lockMode = NoMode; // so that the kick will not have ghost.
			grabInterval = 50;
		}
		}*/
  //setHighGain ();
  if (*currentWalkType == UpennLeftWT) {
    lockMode = NoMode;
  }
  else {
    walkType = UpennLeftWT;
  }
}

/*
 * Strategy:
 *  aaGoalieKick
 *
 * Description:
 *  performs the UNSW front kick
 *
 * Strategy member variables used:
 *  grabInterval - 
 *      after executing kick, ensure
 *      there is no ball grabbing for
 *      2 seconds.
 *  lockMode -
 *      gets set to resetMode to
 *      prevent ghost kicks
 */

void Behaviours::aaGoalieKick() {

  //Beckham::doBeckhamTrackVisualBall();
  if (*currentWalkType == FwdKickWT) {
    lockMode = NoMode;
  }
  else {
    walkType = FwdKickWT;
  }
	//grabInterval = 50;
}


/*
 * Strategy:
 *  aaLightningKick
 *
 * Description:
 *  fast kick
 *
 * Strategy member variables used:
 *  grabInterval -
 *      gets ret to 50, i.e.
 *      50 frames must elapse before
 *      another attempt to grab the
 *      ball is made
 */

void Behaviours::aaLightningKick() {

	//saBallTracking();
	//Beckham::doBeckhamTrackVisualBall();
  if (*currentWalkType == FastKickWT) {
    lockMode = NoMode;
  }
  else {
    walkType = FastKickWT; 
  }
}
/*
 * Description:
 *  Performs a chest-push on the ball
 */

void Behaviours::aaChestPush() {

  //saBallTracking();

  if (*currentWalkType == ChestPush) {
    lockMode = NoMode;
  }
  else {
    walkType = ChestPush; 
  }
}
