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
 * $Id:
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "Behaviours.h"
#include "UNSW2004.h"
#ifdef COMPILE_ALL_CPP
static int spinDribbleTimer = 0;

static double spinDribbleLastHeadpanx = 0;
static double spinDribbleLastHeadtilty = 0;
static double spinDribbleLastHeadcranez = 0;
static int    spinDribbleLastHeadtype = ABS_XYZ;

void Behaviours::aaSpinDribble() {


  KEV_DEBUG(cout << "***********************\n";);
  KEV_DEBUG(cout << "     SPIN DRIBBLE\n";);
  KEV_DEBUG(cout << "***********************\n";);

	
  UNSW2004::doUNSW2004TrackVisualBall();

  spinDribbleTimer++;
  if (spinDribbleTimer ==1 ) {
    smartSetBeacon();
    setTurnKickParams(0);
    return; // newly added
    /*spinDribbleLastHeadtype = headtype;
    spinDribbleLastHeadpanx = panx;
    spinDribbleLastHeadtilty = tilty;
    spinDribbleLastHeadcranez = cranez;*/
  }    
  if (spinDribbleTimer < 14) {
    setTurnKickParams(0);
    /*headtype = spinDribbleLastHeadtype;            
    panx = spinDribbleLastHeadpanx;
    tilty = spinDribbleLastHeadtilty;
    cranez = spinDribbleLastHeadcranez;*/
    //Beckham::doBeckhamTrackVisualBall();
    return;
  }
  else if (spinDribbleTimer < 24) {
    setTurnKickParams(0);
    smartActiveGpsLocalise(); 
    return;
  }
  
  if (!gps->canSee(vobBall)) {
    headtype = spinDribbleLastHeadtype;            
    panx = spinDribbleLastHeadpanx;
    tilty = spinDribbleLastHeadtilty;
    cranez = spinDribbleLastHeadcranez;
  }

  double reqAccuracy = requiredAccuracy(20);
  reqAccuracy = MAX(reqAccuracy, 5);

  double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
  double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
  
  //double min = MIN(abs(minGoalHead), abs(maxGoalHead));
  //double max = MAX(abs(minGoalHead), abs(maxGoalHead));
  //KEV_DEBUG (cout << "*****************\n";); 
  //KEV_DEBUG (cout << "minGoalHead: " << minGoalHead << " maxGoalHead: " << maxGoalHead << endl;);
  //KEV_DEBUG (cout << "min:         " << min         << " max:         " << max << endl;);

  // if ball aligned correctly, dribble ball towards target goal
  /*if (ABS (gps->tGoal().pos.getHead()) <= accuracyRequirement &&
      !vision->facingFieldEdge) {
  */    
  if (maxGoalHead >= 0 && minGoalHead <= 0 && !vision->facingFieldEdge) {
    lockMode = Dribble;
    resetDribbleTimer();
    aaDribble();
  }
  else {
    // spin with ball under snout
 
   // might be a buggy check
      if ( vBall.cf >= 3 && !UNSW2004::isUNSW2004BallUnderChin() ) {
	lockMode = NoMode;
	return;
      }
    setTurnKickParams ( CLIP(gps->tGoal().pos.getHead(), 25.0)) ;

  }
}

static int dribbleTimer = 0;

void Behaviours::resetDribbleTimer() {
  dribbleTimer = 0;
  spinDribbleTimer = 0;
}

void Behaviours::aaDribble() {

  dribbleTimer++;

  if ((dribbleTimer < 25)
      && ((!gps->canSee(vobBall)) || (gps->canSee(vobBall) // 20.0
				   && vBall.d < 25.0  && abs(vBall.d < 15)))) {

    headtype = ABS_H;
    panx = 0;
    tilty = 0;
    cranez = 0;

    if (vtGoal.cf > 0) {
      UNSW2004::hoverToSpot(vtGoal.d, vtGoal.h);
      //KEV_DEBUG (cout << "can see the goal in aaDribble\n" ;);
      leds(2, 2, 2, 2, 2, 2);
    }
    else {
      double targetx = FIELD_WIDTH / 2;
      double targety;
      //double targety = FIELD_LENGTH + WALL_THICKNESS;
      if (gps->self().pos.x <= FIELD_WIDTH / 2) {
        targety = (4 * WALL_THICKNESS / FIELD_WIDTH) * gps->self().pos.x
                + (FIELD_LENGTH - WALL_THICKNESS);
      }
      else {
        targety = (-4 * WALL_THICKNESS / FIELD_WIDTH) * (gps->self().pos.x
                   - FIELD_WIDTH / 2) + (FIELD_LENGTH + WALL_THICKNESS);
      }

      Vector localTarget;
      double dx = targetx - gps->self().pos.x;
      double dy = targety - gps->self().pos.y;
      localTarget.setVector(vCART, dx, dy);
      localTarget.rotate(90 - gps->self().h);

      double targetd = localTarget.d;
      double targeth = localTarget.head;

      UNSW2004::hoverToSpot(targetd, targeth);
      //KEV_DEBUG (cout << "cant see the goal in aaDribble\n" ;);
    }
  }
  else {
    //KEV_DEBUG (cout << "Break out the mode\n" ;);
    lockMode = NoMode;
    chooseStrategy();
  }
}
#endif //COMPILE_ALL_CPP
