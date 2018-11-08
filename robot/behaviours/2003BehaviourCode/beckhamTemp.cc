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


////////////////////////////////////////////////////////////
// This file is just ment to be a dump of old 
// and misc stuff that we dont really need
////////////////////////////////////////////////////////////


//the old beckham backoff strat
/*
   void Beckham::doBeckhamBackOffForward(double headToTeammate, double teammatex, double teammatey) {

   double towardsTargetGoal = RAD2DEG(atan2(FIELD_LENGTH-ballY, (FIELD_WIDTH/2.0)-ballX));

   if (lastBackOffStrategy == BO_ATTACK) {
   doBeckhamNormalForward();
   lastBackOffStrategy = BO_ATTACK;
   return;
   } else if (lastBackOffStrategy == BO_GETBEHIND) {
   if (abs(gps->self().h - towardsTargetGoal) < 20) {
   doBeckhamSupportForward(ballX, ballY);
   lastBackOffStrategy = BO_BACKOFF;
   return;
   }
   saFarGetBehindBall(prevAttackMode != GetBehindBall, towardsTargetGoal);
   lastBackOffStrategy = BO_GETBEHIND;
   return;
   }

   for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {

   interpBehav info = (interpBehav)gps->tmObj(i-1).behavioursVal;

// for all people in front of you (using gps) and further than the ball
if (i != PLAYER_NUMBER &&
gps->tmObj(i-1).counter > 0 &&
info.amGoalie == 0 &&
gps->teammate(i).posVar <= get95CF(50) &&
abs(gps->teammate(i).pos.head) < 30 &&
gps->teammate(i).pos.d > gps->ball().pos.d + 10 &&
gps->ball().posVar < get95CF(50)
) {

// if you can see the ball and your looking up and they are looking down attack ball
if (vBall.cf>=3 && gps->self().h > 40 && gps->self().h < 140 &&
gps->teammate(i).h > 220 && gps->teammate(i).h < 320) {
doBeckhamAttackBall();
lastBackOffStrategy = BO_ATTACK;
return;
}

// if your looking down and they are looking up back off
if (gps->self().h > 220 && gps->self().h < 320 &&
gps->teammate(i).h > 40 && gps->teammate(i).h < 140 && info.ballDist < 50) {
doBeckhamSupportForward(ballX, ballY);
return;
}

// if they are looking pretty straight up or their heading is similar to yours
if ((gps->teammate(i).h > 55 && gps->teammate(i).h < 125 || 
abs(gps->self().h - gps->teammate(i).h) < 27) && info.ballDist < 50) {
doBeckhamSupportForward(ballX, ballY);
return;
}

// else get behind ball
saFarGetBehindBall(prevAttackMode != GetBehindBall, towardsTargetGoal);
lastBackOffStrategy = BO_GETBEHIND;
return;
}
}

// for all people infront of you and further than ball using vision
for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
// if your facing upish and your downfield from the ball attack ball
if (vTeammate[i].var < get95CF(50) && ABS(vTeammate[i].h) < 30 &&
vBall.cf > 3 && vBall.d < 50 && vTeammate[i].d > vBall.d + 10 &&
gps->self().h > 50 && gps->self().h < 130
) {
lastBackOffStrategy = BO_ATTACK;
doBeckhamAttackBall();
return;
}
}   

//maybe when falling through
//decide on whether or not you need to attack anyway?

lastBackOffStrategy = BO_BACKOFF;
doBeckhamSupportForward(ballX, ballY);
}
*/



///////////NOTES/////////////

/*
   THOUGHTS ON COMPUTING KALMAN FOR BALL USING LOCAL OR GLOBAL COORDS

   STORE LOCAL
   if wireless add their pos variance and then your pos var
   (or send more data)
   if want global add your pos var
   always add your motion prediction update variance to balls predition update variance

   STORE GLOBAL
   if wireless add their pos var
   if want local add your pos var again
   always add your pos var on ball measurement updates

   ARGUMENTS FOR GLOBAL
   -  your pos var, worse than your time update var
   -  but your pos var affects your ball measurement
   (which is worse than affecting your prediction variance)
   ++ in STORE GLOBAL object location better in kidnapped robot scenario
   -  in STORE LOCAL object localisation not effected if your localisation goes out
   but hopefully by the end, your own localisation wont go out
   -+ local adds more variance in wireless updates but less important
   -+ i have a feeling we might be accessing local more but strategy dont care as much for variance
   +  global easier to understand and code
   -  perhaps your self localisation is too jumpy...because most of the time
   your looking at the floor and only getting a glance of beacons..
   lets see what its like after lines are included

   want to keep variance small so kalman filter works properly

   RIGHT NOW BALL IMPLEMENTED IN GLOBAL
   CLOSE THE LOOP FIRST, AND THEN.... PERHAPS RECONSIDER???

   DEFINATE FOR SELF AND TEAMMATES, STORE GLOBAL
   FOR OPPONENTS.... LEANING TOWARDS GLOBAL
   FOR BALL......... STILL UNDECIDED   PERSONAL PREFERANCE.. GLOBAL?
   */
