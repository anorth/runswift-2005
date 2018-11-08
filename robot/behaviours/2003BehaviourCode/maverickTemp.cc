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


//////////////////////////////////////////////////////////////////////////////////////////////////////
//this file is just filled up with things that are related to maverick but aint getting used right now
//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// this is to calibrate the defensive get behind ball thing

#ifdef itcantpossiblybe
cout << "opp dist " << (gps->getClosestOppPos(LOCAL)).d << endl;
//cout << "opp dist " << opp.d << endl;

if (ballSource == VISION_BALL && 
		vBall.d < 50 && abs(vBall.h) < 45 &&
		opp.d < 50 /*&& abs(opp.head) < 70 */ /*&&*/
		/*gps->self().pos.y < (FIELD_LENGTH/2.0) && */ /*gps->self().h > 200 && gps->self().h < 340*/) {

	//double awayFromGoal = RAD2DEG(atan2(vBall.x-(FIELD_WIDTH/2.0) , vBall.y-0));
	double awayFromGoal = RAD2DEG(atan2(vBall.y-0, vBall.x-(FIELD_WIDTH/2.0)));
	//cout << "away from goal is " << awayFromGoal << endl;
	saFarGetBehindBall(prevAttackMode != GetBehindBall, awayFromGoal);

	//this is a hack required because the dog gets behind the ball already 
	//and still trys to get to the ball resulting in it walkin backwards


	if (forward <= 0) {
		hoverToBall(vBall.d, vBall.h);
		/*
		   forward = 0;
		   left = 0;
		   turnCCW = 0;
		   */
	}

	//leds(2,0,0,1,0,0);
}  else if (vBall.d > 20) {
	hoverToBall(vBall.d, vBall.h);
}
else {
	forward = 0;
	left = 0;
	turnCCW = 0;
}
return;
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//this is the old style of get behind ball
//its similar to the one dots and spots uses
/*
//if your behind the ball
if (vBall.cf > 0 && gps->self().pos.y < vBall.y + 15) {
//if your teammate is right in your face back off 
if (utilTeammateInFace()) {
forward = -6;
} else {
//if ball towards the left, then walk around to the right
if (vBall.x < 90)
GetNearBall(75,20);
//if ball towards the right, then walk around to the right
else if (vBall.x > 200)
GetNearBall(-75,20);
//else go to the closer of right/left
else
GetNearBall(gps->self().pos.x < vBall.x ? -80 : 80,20);
}
}
//else your infront or aligned with the ball (or cant see the ball)
else {
// if can see ball , you are hence infront or aligned with ball
if (vBall.cf > 0) {
//your infront and ball is towards left edge so get out of the way 
if (vBall.x < WALL_THICKNESS + 35 && gps->self().h > 180) {
left = 12;
} 
//your infront and ball is towards right edge so get out of the way 
else if (vBall.x > FIELD_WIDTH - (WALL_THICKNESS + 35) && gps->self().h > 180) {
left = -12;
} 

else {
left = CLIP(((-gps->tGoal().pos.head / 60) * 12), 12.0);
}
turnCCW = CLIP(vBall.h / 2.0, 5.0);
} 
//cant see the ball
else {
if (gps->ball('g').pos.x < WALL_THICKNESS + 35 && gps->self().h > 180) {
left = 12;
} else if (gps->ball('g').pos.x > FIELD_WIDTH - (WALL_THICKNESS + 35) && gps->self().h > 180) {
left = -12;
} else {
left = CLIP(((-gps->tGoal().pos.head / 60) * 12), 12.0);
}
turnCCW = CLIP(gps->ball('l').pos.head / 2.0, 5.0);
}
}
*/	

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//will turn to face posh asap
//whilst hovering to posx, posy
//posx posy posh are all global
//
//
//
//this function is not properly tested!
//im not sure if it works
void Maverick::hoverToPositionHeading(double posx, double posy, double posh) {

	double maxCanterForwardSpeed = 8;
	double maxCanterLeftSpeed    = 7;


	// variables relative to self localisation
	double relx = posx - (gps->self().pos.x);
	double rely = posy - (gps->self().pos.y);
	double reld = sqrt(SQUARE(relx)+SQUARE(rely));
	double moveDirection = NormalizeAngle_180(RAD2DEG(atan2(rely,relx)) - (gps->self().h));
	double faceDirection = NormalizeAngle_180(posh - (gps->self().h));

	/*
	   if ((posh - gps->self().h) > 180) {
	   posh -= FULL_CIRCLE;
	   } else if ((gps->self().h - posh) > 180) {
	   posh += FULL_CIRCLE;
	   }

	//these are both relative
	double faceDirection = posh - gps->self().h;
	double moveDirection = NormalizeAngle_180(RAD2DEG(atan2(-1*(posx-gps->self().pos.x), posy-gps->self().pos.y)));
	*/

	cout << "******************" << endl;
	cout << "global faceDir " << posh << endl;
	cout << "current head   " << gps->self().h << endl;
	cout << "local  faceDir " << faceDirection << endl;
	cout << "self           " << gps->self().pos.x << ", " << gps->self().pos.y << endl;
	cout << "pos            " << posx << ", " << posy << endl;
	cout << "local moveDir  " << moveDirection << endl;

	//turn to be facing the heading asap
	turnCCW = CLIP (faceDirection/2.0, 40.0);

	// if the ball is at a high angle, walk forward and left appropriately whilst turning
	//if (abs(faceDirection) > 17) {
	setOmniParams();
	//leds(1,1,2,1,1,1);
	walkType = CanterWalkWT;

	double maxF = maxCanterForwardSpeed;
	double maxL = maxCanterLeftSpeed;

	if (turnCCW<10 && turnCCW>=0) { 
		maxF=6;
		maxL=6;
	} 
	//counter clockwise case turning faster
	else if (turnCCW<20 && turnCCW>=0) { 
		maxF=5;
		maxL=5;
	} 
	//turn too fast anticlockwise case
	else if (turnCCW>=0) {
		maxF=3;
		maxL=2;
	}

	//slow clockwise turn case
	else if (turnCCW>-10 && turnCCW<0) { 
		maxF=5;
		maxL=5;
	} 
	//all other clockwise cases
	else {
		maxF=1;
		maxL=3;
	}         

	//calculate how much you should go forward and left whilst turning
	double fComp = cos(DEG2RAD(moveDirection));
	double lComp = sin(DEG2RAD(moveDirection));
	double scale;
	if ( maxF*ABS(fComp)>=maxL*ABS(lComp) ) { scale = 1.0/ABS(fComp); }
	else { scale = 1.0/ABS(lComp); }

	fComp = scale*fComp;
	lComp = scale*lComp;

	forward = maxF*fComp;
	left = maxL*lComp;
	//cout << "forward, left, turn " << forward << "," << left << "," << turnCCW;
	/*
	   }
	//otherwise use zoidal walk to run to the ball
	else {
	setWalkParams();
	walkType = ZoidalWalkWT;
	left = 0;
	forward = MAX_OFFSET_FORWARD;
	}
	*/

	//the rest is guards to make sure your not doing crazy things with the walk
	if (forward < 0 || ABS (turnCCW) > 15) {
		walkType = CanterWalkWT;
	}

	/*
	   if (abs(turnCCW)>25) {
//leds(1,1,2,1,1,2);
forward = CLIP(forward, 4.5);
}
*/

if (walkType == CanterWalkWT) {
	//leds(2,1,1,1,1,1);
	if (forward > maxCanterForwardSpeed) {
		forward = maxCanterForwardSpeed;
	}

	if (turnCCW<10 && turnCCW>=0) { 
		//cout << "clip 10 anti 7.5 , 6" << endl;
		forward = CLIP(forward, 7.5);
		left = CLIP(left,6.0);
		//maxF=6;
		//maxL=6;
	} 
	//counter clockwise case turning faster
	else if (turnCCW<20 && turnCCW>=0) { 
		//cout << "clip 20 anti 5.7 , 5" << endl;
		forward = CLIP(forward, 5.7);
		left = CLIP(left,5.0);
		//maxF=5;
		//maxL=5;
	} 
	//turn too fast anticlockwise case
	else if (turnCCW>=0) {
		//cout << "clip other anti 5 , 5" << endl;        
		forward = CLIP(forward, 5.0);
		left = CLIP(left,5.0);
		//maxF=3;
		//maxL=2;
	}

	//slow clockwise turn case
	else if (turnCCW>-10 && turnCCW<0) { 
		//cout << "clip 10 clockwise 5 , 5" << endl;        
		forward = CLIP(forward, 5.0);
		left = CLIP(left,5.0);
		//maxF=5;
		//maxL=5;
	} 
	//all other clockwise cases
	else {
		//cout << "clip other clockwise 4.3 , 5" << endl;        
		forward = CLIP(forward, 4.3);
		left = CLIP(left,5.0);
		//maxF=1;
		//maxL=3;
	}        

} else {
	//leds(1,1,1,2,1,1);
	if (forward > MAX_OFFSET_FORWARD) {
		forward = MAX_OFFSET_FORWARD;
	}
}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//the old 3rd closest robot method
/*
   bool Maverick::is3rdClosest() {
   int closeRank = 0;

   for (int i = 1; i <= NUM_TEAM_MEMBER; i++) {
   if (i != PLAYER_NUMBER && 
   gps->wmTeammate[i-1].counter > 0 &&
   gps->wmTeammate[i-1].playerType==FORWARD &&
   gps->teammate(i).posVar <= get95CF(50)
   ) {

// increment counter whenever a teammate is 
// closer to the ball (if the distances are fairly similar, 
// if you have the lower player number, still increment the counter)
double myDistMinusTheirs = gps->ball().pos.d - gps->wmTeammate[i-1].ballDist;
if (myDistMinusTheirs > +20)  {
closeRank++;
} else if (abs(myDistMinusTheirs) < 20 && PLAYER_NUMBER > i) {
closeRank++;
}
}
}
return (closeRank == 2);
}
*/

//my old visual back off positioning strategy
/*
   if (gps->self().pos.x <= xMatchingYourY) {
   double x = MIN(tmx - edgeBuffer, xoffset);
   double y = MIN(tmy - edgeBuffer, yoffset);
   double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy-gps->self().pos.y, tmx-gps->self().pos.x)));
   saGoToTargetFacingHeading(tmx-x, tmy+y, h);
   } else {
   double x = MIN(FIELD_WIDTH - edgeBuffer - tmx, xoffset);
   double y = MIN(tmy - edgeBuffer, yoffset);
   double h = NormalizeAngle_0_360(RAD2DEG(atan2(tmy-gps->self().pos.y, tmx-gps->self().pos.x)));
   saGoToTargetFacingHeading(tmx+x, tmy+y, h);
   }
   */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//shoot ball pseudo code
if your very close ie in the penalty area and your fairly aligned to kick the ball
line up for a paw kick
if your not in the clear and your fairly aligned to kick the ball and your heading kinda upfield
line up for a paw kick
else hoverToBall and go for the grab

after grabbing : 
if in target half
if in top corner/edges
spin chest push
if pretty well aligned and pretty clear of opponents
vision spin kick
if very well aligned
lightning kick
if pretty well aligned 
spin front kick
if can hit it with a 90
do 90
if can hit it with a 180
do 180 with smart direction
else decide between 90 and 180
else if own half
if your facing fairly down field
lightning kick
if 90 turn kick gets your further up field
90 turn kick
if 180 turn kick is appropriate 180 turn kick with with smarter direction choice



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//old shoot strategy
/*
   void Maverick::chooseMaverickShootStrategy() {

   double absoluteShootRadius = FIELD_LENGTH/2.0;
   double faceTargetGoalMargin = 30;
   double chestPushRange = FIELD_LENGTH - WALL_THICKNESS - 30;
   double closestOppNearDistance = 25;

   double reqAccuracy = requiredAccuracy(25);

//local coords
double minGoalHead = gps->tGoal().pos.head - reqAccuracy;
double maxGoalHead = gps->tGoal().pos.head + reqAccuracy;


double min = MIN(abs(minGoalHead), abs(maxGoalHead));
double max = MAX(abs(minGoalHead), abs(maxGoalHead));

grabTime = 65;
const Vector& opp = gps->getClosestOppPos(LOCAL);

if (kickingOff) {
lockMode = ProperVariableTurnKick;
setProperVariableTurnKick(90);
aaProperVariableTurnKick();   
kickingOff = false;
return;
}

if (gps->tGoal().pos.d <= absoluteShootRadius) {
//if already lined up with goal lightning
if (maxGoalHead >=0 && minGoalHead <=0) {
//check if your looking straight at the side wall..
//if so your gps is probably wrong
if (vision->facingFieldEdge) {
double dir;
if (gps->self().pos.x < FIELD_WIDTH/2.0) {
dir = -90;
} else {
dir = 90;
}
lockMode = ProperVariableTurnKick;
setProperVariableTurnKick(dir);
aaProperVariableTurnKick();                 
} else {
lockMode = LightningKick;
aaLightningKick();
}
}
else if (gps->self().pos.y >= (FIELD_LENGTH- WALL_THICKNESS - 20)) {
if (vision->facingFieldEdge) {
double dir;
if (gps->self().pos.x < FIELD_WIDTH/2.0) {
dir = -90;
} else {
dir = 90;
}
lockMode = ProperVariableTurnKick;
setProperVariableTurnKick(dir);
aaProperVariableTurnKick();                 
} else {
lockMode = SpinChestPush;
aaSpinChestPush();
}
}
//out of turn kick range do spin front kick
else if (max <= 65) {
//            lockMode = SpinKick;
//            aaSpinKick();
lockMode = SafeSpinKick;
aaSafeSpinKick();
} 
//turn kick go for goal
else {
	double dir;
	if (min<=90 && max>=90) {
		dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
	} else if (min<=180 && max>=180) {
		dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
	} else {
		dir = gps->tGoal().pos.head;
	} 

	if (dir >= 160 && opp.d <= 40 && opp.head >= 0) {
		dir = dir - 360;
	} else if (dir <= -160 && opp.d <= 40 && opp.head <= 0) {
		dir = 360 + dir;
	}

	if (abs(gps->tGoal().pos.head) < 135) {
		dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
	} else {
		dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
	}

	if (vision->facingFieldEdge) {
		double dir;
		if (gps->self().pos.x < FIELD_WIDTH/2.0) {
			dir = -90;
		} else {
			dir = 90;
		}
	}

	lockMode = ProperVariableTurnKick;
	setProperVariableTurnKick(dir);
	aaProperVariableTurnKick();
}
} else {
	//
	if (max <= requiredAccuracy(3.0*FIELD_WIDTH/2.0) && !(vision->facingFieldEdge)) {
		lockMode = LightningKick;
		aaLightningKick();
	} else {

		reqAccuracy = requiredAccuracy(3.0*FIELD_WIDTH/2.0);
		//local coords
		minGoalHead = gps->tGoal().pos.head - reqAccuracy;
		maxGoalHead = gps->tGoal().pos.head + reqAccuracy;
		min = MIN(abs(minGoalHead), abs(maxGoalHead));
		max = MAX(abs(minGoalHead), abs(maxGoalHead));        

		double dir;
		if (min<=90 && max>=90) {
			dir = (gps->tGoal().pos.head > 0 ? 90 : -90);
		} else if (min<=180 && max>=180) {
			dir = (gps->tGoal().pos.head > 0 ? 180 : -180);
		} else {
			dir = gps->tGoal().pos.head;
		} 

		if (dir >= 160 && opp.d <= 40 && opp.head >= 0) {
			dir = dir - 360;
		} else if (dir <= -160 && opp.d <= 40 && opp.head <= 0) {
			dir = 360 + dir;
		}

		if (abs(gps->tGoal().pos.head) < 135) {
			dir = (gps->tGoal().pos.head > 0) ? 90 : -90;
		} else {
			dir = (gps->tGoal().pos.head > 0) ? 180 : -180;
		}

		lockMode = ProperVariableTurnKick;
		setProperVariableTurnKick(dir);
		aaProperVariableTurnKick();    
	}
}
return;
}        
*/



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Behaviours::saGoToTargetFacingHeading(const double& targetx, const double& targety, const double& targeth, double maxSpeed) {

	// variables relative to self localisation
	double relx = targetx - (gps->self().pos.x);
	double rely = targety - (gps->self().pos.y);
	double reld = sqrt(SQUARE(relx)+SQUARE(rely));

	//spot you walk to
	double relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely,relx)) - (gps->self().h));
	//desired heading ie of your body
	double relh = NormalizeAngle_180(targeth - (gps->self().h));



	forward = CLIP (reld, maxSpeed) * cos (DEG2RAD (relTheta));
	left = CLIP (reld, maxSpeed) * sin (DEG2RAD (relTheta));

	//if(reld < 100) {
	double move = sqrt (SQUARE (forward) + SQUARE (left));
	double maxTurn = 15;//(move > 4) ? 5 : (9-move);
	turnCCW = CLIP (relh, maxTurn);
	//}

}

