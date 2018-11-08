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
 * $Id: ballChallenge.cc 427 2003-02-13 11:29:07Z nathanw $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include "ballChallenge.h"

static const int PERIOD14_ = 24;
static const int PERIOD4_ = 18;
static const int PERIOD12_ = 24;

namespace ballChallenge {
	typedef enum {
		c3Init,
		c3Find,
		c3Approach,
		c3Hold,
		c3Carry,
		c3Kick,
		c3Push,
		c3SoftKick,
		c3Edge,
		c3SideWalk,
		c3BackOff
	} States;

	static States state;

	static int timer;

	static int panDirection;

	static int fbPanDir, fbTiltDir;

	static int lostballtimer;
	static int circleDir;
	static double ballMovement;

	static double localisedY;
	static double localisedX;
	static double localisedH;
}

double ballChallenge::calTurn;

void ballChallenge::initBallCollection() {
	timer = 0;
	state = c3Find;
	headtype = ABS_H;
	tilty = panx = 0;
	setStandParams();
	panDirection = 1;

	fbPanDir = fbTiltDir = 0;

	lostballtimer = 0;

	circleDir = NO_CIRCLE;

	ballMovement = 0.0;
	localisedY = 0.0;
	localisedX = 0.0;
	localisedH = 0.0;
}


void ballChallenge::doBallCollection() {
	// Ross
	leds(1, 1, 1, 1, 1, 1);

	// check teammate avoidance first
	if (((vTeammate[0].var < get95CF(50) && vTeammate[0].d < 50)
				|| (vTeammate[1].var < get95CF(50) && vTeammate[1].d < 50)
				|| (vTeammate[2].var < get95CF(50) && vTeammate[2].d < 50)
				|| (vTeammate[3].var < get95CF(50) && vTeammate[3].d < 50))
			&& (state == c3Init || state == c3Find || state == c3Approach)) {
		timer = 0;
		state = c3BackOff;
	}


	if (state == c3Init) {
		// move into pos
#ifdef PlayerA

		Vector target(vCART,
				185.0 + 8.0 * cos(DEG2RAD(gps->self().getHackedH())),
				370.0 + 8.0 * sin(DEG2RAD(gps->self().getHackedH())));

#else

		Vector target(vCART,
				105.0 + 8.0 * cos(DEG2RAD(gps->self().getHackedH())),
				370.0 + 8.0 * sin(DEG2RAD(gps->self().getHackedH())));

#endif

		Vector vcentre(vCART, 145.0, 220.0);
		Vector robot(vCART, (double) gps->self().pos.x, (double) gps->self().pos.y);

		//vector to destination point
		Vector vx(robot);
		vx.sub(target);

		//vcentre- is vector from robot to centre
		vcentre.sub(robot);

#define pointAngle utilNormaliseAngle((vx.getHackedTheta()-gps->self().getHackedH()))
#define centreAngle utilNormaliseAngle((vcentre.getHackedTheta()-gps->self().getHackedH()))

		turnCCW = Cap(centreAngle / 2.0, 20);

		forward = -6.0 * cos(DEG2RAD(pointAngle));
		left = -6.0 * sin(DEG2RAD(pointAngle));

		SlowLookAroundParams();

		if (vx.d < 20) {
			forward /= 2.0;
			left /= 2.0;
			turnCCW /= 2.0;
		}

		if (vx.d < 10) {
			state = c3Find;
			setStandParams();
			timer = 0;
		}
		else {
			setWalkParams();
		}
	}
	else if (state == c3Find) {
		//this is kinda dodgy.. look at the comment in state ==c3Kick
		if (*currentWalkType == FwdKickWT) {
			setStandParams();
		}
		else {
			if (vBall.cf > 0) {
				// ricky: do ball tracking after set param
				setWalkParams();
				setC3GotoBallParams(); 
				BallTracking();
				state = c3Approach;
				fbPanDir = fbTiltDir = 0;
				timer = 0;
			}
			else {
				timer++;
				FindBallHeadParams();
				if (timer > 25) {
					// middle half field for PlayerA, middle field for PlayerB

#ifdef PlayerA

					Vector target(vCART,
							145.0 + 8.0 * cos(DEG2RAD(gps->self().getHackedH())),
							330.0 + 8.0 * sin(DEG2RAD(gps->self().getHackedH())));

#else

					Vector target(vCART,
							145.0 + 8.0 * cos(DEG2RAD(gps->self().getHackedH())),
							220.0 + 8.0 * sin(DEG2RAD(gps->self().getHackedH())));

#endif

					Vector robot(vCART,
							(double) gps->self().pos.x,
							(double) gps->self().pos.y);

					//vector to destination point
					Vector vx(robot);
					vx.sub(target);

#define pointAngle2 utilNormaliseAngle((vx.getHackedTheta()-gps->self().getHackedH()))

#ifdef PlayerA

					turnCCW = 16.0;

#else

					turnCCW = -16.0;

#endif

					forward = -6.0 * cos(DEG2RAD(pointAngle2));
					left = -6.0 * sin(DEG2RAD(pointAngle2));

					if (vx.d < 20) {
						forward /= 4.0;
						left /= 4.0;
					}
					setWalkParams();
				}
				else {
					setStandParams();
				}
			}
		}
	}
	else if (state == c3Approach) {
		if (vBall.cf == 0) {
			state = c3Find;
			timer = 0;
		}
		else {
			// this whole stuck detection thing needs to be reconsidered
			/*
			   if (timer == 0)
			   {
			// set ball movement
			ballMovement = gps->ball().pos.d;
			timer++;
			}
			else if (timer > 75)
			{
			// if stuck somewhere, ball distance not reducing
			timer = 0;
			// guard is a bit suspect, because of multiple balls
			// gps->ball().pos.d + 10.0 > ballMovement without multiple?
			if (ABS(gps->ball().pos.d-ballMovement) < 5.0 && gps->ball().pos.d > 10.0)
			{
			// walk around
			state = c3Init;
			return;
			}
			}
			else
			{
			timer++;
			}
			*/

			// ricky do tracking after walk param set

			// go after the ball until tilt trigger
			// ross ricky: replaced flatdist with camdist
			if (tilty < -60.0
					&& ABS(panx) < 20.0
					&& (vBall.vob->cam_dist < 5) /* || gps->ball().pos.d < 10.0) */) {
						state = c3Hold;
						timer = 0;
						setStandParams();
					}
			else {
				// ROSS
				/*cout << "tilt = " << tilt;
				  cout << "\tABS(pan) = " << ABS(pan);
				  cout << "\tdist = " << gps->ball().pos.d;
				  cout << "\tflat dist = " << vBall.d;
				  cout << "\tvdist = " << vBall.vob->dist;
				  cout << "\tcamDist = " << vBall.vob->cam_dist << endl;*/
				// w00t
				setC3GotoBallParams();
				setWalkParams();
			}
			BallTracking();
		}
	}
	else if (state == c3Hold) {
		leds(0, 2, 0, 0, 2, 0);
		if (vBall.cf == 0 && timer <= 15) {
			lostballtimer++;
			if (lostballtimer > 5) {
				state = c3Find;
				timer = 0;
				lostballtimer = 0;
				return;
			}
		}
		else {
			lostballtimer = 0;
		}

		// l33t
		timer++;
		if (timer > 24) {
			setHoldParams();

			//ERIC
			state = c3Carry;
			gps->currentlyCarrying = true;


			circleDir = NO_CIRCLE;
			timer = 0;
			lostballtimer = 0;
			return;
		}
		else if (timer > 15) {
			headtype = ABS_H;
			tilty = 0;
			panx = 0;
		}
		else {
			BallTracking();
		}
		setHoldParams();
	}
	else if (state == c3Carry) {
		// a bit of dodginess here.. need to assume that hold is 100% reliable,
		// can't just stop because a ball is seen while carrying
		leds(2, 0, 0, 2, 0, 0);
		timer++;

		if (timer < 20) {
			if (gps->self().posVar > get95CF(20)) {
				// have a look-see first
				LookAroundParams();
				setHoldParams();
			}
			else {
				timer = 19;
			}
		} 
		/*
		   else {
		   setCarryParams();
		   headtype = ABS_H;
		   tilt     = 6;
		   pan      = 0;

		   turnCCW = calTurn;
		   left = -turnCCW/6.0;
		   }
		   */

		else {
			if (timer == 20) {
				localisedY = gps->self().pos.y;
				localisedH = gps->self().getHackedH();
				localisedX = gps->self().pos.x;
				// special edge/corner behaviour needed here
				if (gps->self().pos.x <35.0 || gps->self().pos.x> 255.0) {
					//ERIC
					gps->currentlyCarrying = false;
					state = c3Edge;
					timer = 0;
					circleDir = NO_CIRCLE;
					return;
				}
			}

			if (localisedY < 190.0) {
				// somehow strayed to the back half, kick to other goal
				if (circleDir == NO_CIRCLE) {
					// commit to a direction
					// gps needs fixing before this can work
					if (gps->oGoal().pos.getHackedHead() > 0) {
						circleDir = ANTICLOCKWISE;
					}
					else {
						circleDir = CLOCKWISE;
					}
				}

				if (timer > 270) {
					// probably stuck somewhere
					if (localisedY > 50.0) {
						//ERIC
						gps->currentlyCarrying = false;
						state = c3Kick;
						headtype = ABS_H;
					}
					else {
						//ERIC
						gps->currentlyCarrying = false;
						state = c3Push;
					}
					timer = 0;
					circleDir = NO_CIRCLE;
				}
				else if (voGoal.cf > 25) {
					// can see some of goal
					headtype = ABS_H;
					tilty = 5;
					panx = Cap(voGoal.h, 30);

					if ((ABS(voGoal.h) < 5 || voGoal.d < 15) && *stepComplete) {
						if (localisedY > 50.0) {
							//ERIC
							gps->currentlyCarrying = false;
							state = c3Kick;
						}
						else {
							//ERIC
							gps->currentlyCarrying = false;
							state = c3Push;
						}
						timer = 0;
						circleDir = NO_CIRCLE;
					}
					else {
						setCarryParams();
						turnCCW = ABS(voGoal.h)
							> 9.5
							? voGoal.h
							: (voGoal.h > 0 ? 9.5 : -9.5);
						left = -turnCCW / 6.0;
					}
				}
				else {
					// can't see
					headtype = ABS_H;
					tilty = 5;
					panx = 0;
					setCarryParams();
					if (circleDir == ANTICLOCKWISE) {
						turnCCW = 30.0;
						left = -5.0;
					}
					else if (circleDir == CLOCKWISE) {
						turnCCW = -30.0;
						left = 5.0;
					}
				}
			}
			else {
				if (circleDir == NO_CIRCLE) {
					// commit to a direction
					if (gps->tGoal().pos.getHead() > 0) {
						circleDir = ANTICLOCKWISE;
					}
					else {
						circleDir = CLOCKWISE;
					}
				}

				if (timer > 270) {
					// probably stuck somewhere
					if (localisedY < 390.0) {
						//ERIC
						gps->currentlyCarrying = false;
						state = c3Kick;
					}
					else {
						//ERIC
						gps->currentlyCarrying = false;
						state = c3Push;
					}
					timer = 0;
					circleDir = NO_CIRCLE;
				}
				else if (vtGoal.cf > 25) {
					// can see some of goal
					headtype = ABS_H;
					tilty = 5;
					panx = Cap(vtGoal.h, 30);

					if ((ABS(vtGoal.h) < 5 || vtGoal.d < 15) && *stepComplete) {
						//step complete is related to overstepping your mark and turning back
						if (localisedY < 390.0) {
							//ERIC
							gps->currentlyCarrying = false;
							state = c3Kick;
						}
						else {
							//ERIC
							gps->currentlyCarrying = false;
							state = c3Push;
						}
						timer = 0;
						circleDir = NO_CIRCLE;
					}
					else {
						setCarryParams();
						turnCCW = ABS(vtGoal.h)
							> 9.5
							? vtGoal.h
							: (vtGoal.h > 0 ? 9.5 : -9.5);
						left = -turnCCW / 6.0;
					}
				}
				else {
					// can't see
					headtype = ABS_H;
					tilty = 5;
					panx = 0;
					setCarryParams();
					if (circleDir == ANTICLOCKWISE) {
						turnCCW = 30.0;
						left = -5.0;
					}
					else if (circleDir == CLOCKWISE) {
						turnCCW = -30.0;
						left = 5.0;
					}
				}
			}
		}
	}
	else if (state == c3Kick) {
#define kickdelay 4

		timer++;

		/*
		   if(timer == 1)
		   cout << "going to kick the ball!..." << endl;
		   else if(timer == 28) {
		   cout << "finished kicking the ball!!!" << endl << endl;
		   }
		   */

		if (timer > kickdelay) {
			if (*currentWalkType != FwdKickWT) {
				walkType = FwdKickWT;
				// ricky: added params, consistent with the goalie kick skill
				headtype = ABS_H;
				tilty = 0;
				panx = 0;
			}

			//CAUTION : might have a problem here.. dog is still doing kick but you've allowed the dog to change state into
			//the find state....this could be a problem if find does something different when it just starts up
			//look in c3Find to see how i have handled this problem
			else {
				state = c3Find;
				timer = 0;
				setStandParams();
			}
		}
		else {
			setHoldParams();
		}



		/*
		   if (timer >= PERIOD14_ + kickdelay) {
		   state = c3Find;
		   timer = 0;
		   setStandParams();
		   } else if (timer > kickdelay) {
		   walkType = FwdKickWT;
		// ricky: added params, consistent with the goalie kick skill
		headtype = ABS_H;
		tilt = 0;
		pan = 0;
		} else {
		setHoldParams();
		}
		*/
	}
	else if (state == c3Push) {
		timer++;
		if (timer >= PERIOD4_) {
			state = c3Find;
			timer = 0;
			setStandParams();
		}
		else {
			walkType = ChestPushWT;
		}
	}
	else if (state == c3Edge) {
		// edge spec behaviour, kick along the edge

		if (localisedY < 190.0) {
			// kick to other goal
			if (circleDir == NO_CIRCLE) {
				if (gps->oGoal().pos.getHead() > 0.0) {
					circleDir = ANTICLOCKWISE;
				}
				else {
					circleDir = CLOCKWISE;
				}
			}

			localisedH = utilNormaliseAngle(localisedH);

			if (localisedH <-85.0 && localisedH> - 95.0) {
				timer = 0;
				circleDir = NO_CIRCLE;
				state = c3SideWalk;
				/*
				   if (localisedY > 105.0)
				   {
				   state = c3Kick;
				   }
				   else if (localisedY > 65.0)
				   {
				   state = c3SoftKick;
				   }
				   else
				   {
				   state = c3Push;
				   }*/
			}
			else {
				headtype = ABS_H;
				tilty = 20;
				panx = 0;
				setCarryParams();

#define turnrate 2.1

				if (circleDir == ANTICLOCKWISE) {
					turnCCW = 30.0;
					left = -5.0;
					localisedH += turnrate;
				}
				else {
					turnCCW = -30.0;
					left = 5.0;
					localisedH -= turnrate;
				}
			}
		}
		else {
			if (circleDir == NO_CIRCLE) {
				if (gps->tGoal().pos.getHead() > 0.0) {
					circleDir = ANTICLOCKWISE;
				}
				else {
					circleDir = CLOCKWISE;
				}
			}

			localisedH = utilNormaliseAngle(localisedH);

			if (localisedH > 85.0 && localisedH < 95.0) {
				timer = 0;
				circleDir = NO_CIRCLE;
				state = c3SideWalk;
				/*
				   if (localisedY < 335.0)
				//if (gps->self().pos.y < 335.0)
				{
				state = c3Kick;
				}
				else if (localisedY < 385.0)
				//else if (gps->self().pos.y < 390.0)
				{
				state = c3SoftKick;
				}
				else
				{
				state = c3Push;
				}*/
			}
			else {
				headtype = ABS_H;
				tilty = 20;
				panx = 0;
				setCarryParams();

#define turnrate 2.1

				if (circleDir == ANTICLOCKWISE) {
					turnCCW = 30.0;
					left = -5.0;
					localisedH += turnrate;
				}
				else {
					turnCCW = -30.0;
					left = 5.0;
					localisedH -= turnrate;
				}
			}
		}
	}
	else if (state == c3SideWalk) {
		// dave's side walk thingy
		timer++;
		if (timer >= 72 && ABS(panx) < 25.0) {
			state = c3Hold;
			timer = 0;
			return;
		}
		else if (ABS(panx) > 25.0) {
			// lost it
			state = c3Approach;
			timer = 0;
			return;
		}

		if (timer < 18) {
			setWalkParams();
			forward = 2.0;
			turnCCW = vBall.h / 2.0;
			left = 0.0;
		}
		else {
			setWalkParams();
			ffO = 85.0;
			forward = turnCCW = 0.0;

			// use localisedX and localisedY to decide direction
			if (localisedY < 190.0) {
				if (localisedX < 35.0) {
					left = 6.0;
				}
				else {
					left = -6.0;
				}
			}
			else {
				if (localisedX < 35.0) {
					left = -6.0;
				}
				else {
					left = 6.0;
				}
			}
		}

		// ricky ross attempt to reduce stuff
		//        if (timer < 18) {
		if (timer < 36) {
			headtype = ABS_H;
			tilty = -55.0;
			panx = 0.0;
		}
		else {
			BallTracking();
			if (tilty < -55.0)
				tilty = -55.0;
		}
	}
		else if (state == c3BackOff) {
			// passive
			timer++;
			if (timer < 30) {
				setWalkParams();
				LookAroundParams();
				forward = -5.0;
				left = 0.0;

#ifdef PlayerA

				turnCCW = 20.0;

#else

				turnCCW = -20.0;

#endif
			}
			else {
				state = c3Find;
				setStandParams();
				timer = 0;
			}
		}
		else {
			// execution flow doesn't reach here.
			setStandParams();
		}
	}

	static double old_tilt = 0;
	static double old_pan = 0;
#define FOV 10

	void ballChallenge::BallTracking(void) {
		if (vision->vob[vobBall].cf > 0) {
			tilty = vision->vob[vobBall].elev;
			panx = vision->vob[vobBall].head;

			if (abs(panx - hPan) > FOV) {
				double factor = 1.0 + ABS(panx - hPan) / 60;
				panx = hPan + (panx - hPan) * factor;
			}

			double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;

			x1 = vBall.d * tan(radians(vBall.imgHead));
			y1 = vBall.d * tan(radians(vBall.imgElev));
			z1 = vBall.d;

			x2 = x1;
			y2 = y1 + NECK_LENGTH;
			z2 = z1 + FACE_LENGTH;

			x3 = x2 * cos(radians(-hPan)) - z2 * sin(radians(-hPan));
			y3 = y2;
			z3 = x2 * sin(radians(-hPan)) + z2 * cos(radians(-hPan));

			x4 = x3;
			y4 = z3 * sin(radians(hTilt)) + y3 * cos(radians(hTilt));
			z4 = z3 * cos(radians(hTilt)) - y3 * sin(radians(hTilt));

			double turn = radians(turnCCW / 8);
			x5 = x4 * cos(turn) - z4 * sin(turn);
			y5 = y4;
			z5 = x4 * sin(turn) + z4 * cos(turn);

			if (z5 < NECK_LENGTH)
				z5 = NECK_LENGTH;

			headtype = ABS_H;
			double dist = sqrt(z5 *z5 + y5 *y5);
			tilty = degrees(atan(y5 / z5) - asin(NECK_LENGTH / dist));

			old_tilt = tilty;
			old_pan = panx;
		}
		else {
			tilty = old_tilt;
			panx = old_pan;
		}

		/*double faceLength = 7.0;
		  double neckLength = 5.5;
		  double x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,x5,y5,z5,x6,z6;
		// coordinates of ball with respect to robot's camera relative to cameras direction
		// x "horizontal", y "verticle", z "distance"
		x1 = vBall.d * tan(radians(vBall.imgHead));
		y1 = vBall.d * tan(radians(vBall.imgElev));
		z1 = vBall.d;
		// coordinates of ball with respect to base of neck relative to cameras direction
		x2 = x1;
		y2 = y1 + neckLength;
		z2 = z1 + faceLength;
		// coordinates of ball with respect to base of neck relative to body's pan and camera's tilt
		x3 = x2*cos(radians(-hPan))-z2*sin(radians(-hPan));
		y3 = y2;
		z3 = x2*sin(radians(-hPan))+z2*cos(radians(-hPan));
		// coordinates of ball with respect to base of neck relative to body's direction
		x4 = x3;
		y4 = z3*sin(radians(hTilt))+y3*cos(radians(hTilt));
		z4 = z3*cos(radians(hTilt))-y3*sin(radians(hTilt));
		double turn = radians(turnCCW/8);
		x5 = x4*cos(turn) - z4*sin(turn);
		y5 = y4;
		z5 = x4*sin(turn) + z4*cos(turn);
		// ???
		if (z5 < neckLength) z5 = neckLength;
		headtype = ABS_H;
		// tilt = alpha - beta
		// alpha = angle of tilt at base of neck
		// beta = angle of elevation between base of neck and top of neck at centre of ball
		double dist = sqrt(z5*z5 + y5*y5);
		tilt = degrees(atan(y5/z5)-asin(neckLength/dist));
		// these coordinates are in terms of the top of the neck (pan pivot point)
		// and relative to the tilt calculated above
		x6 = x5;
		// i changed this because i thought it was simpler
		// we want z5 to be the coordinate of the ball with repect to the
		// top of the neck in the direction along the nose
		//      - Ross
		//z6 = z4*cos(radians(-tilt))-y4*sin(radians(-tilt));
		z6 = sqrt(SQUARE(dist) - SQUARE(neckLength));
		pan = degrees(atan(x6/z6));
		// need this bit?
		//tilt = hTilt+(tilt-hTilt)*0.6;
		//pan  = hPan+(pan-hPan)*0.8;*/
	}

	/*void ballChallenge::BallTracking(void) {

	  double faceLength = 7.0;
	  double neckLength = 5.5;
	  double x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,x5,z5;

	// coordinates of ball with respect to robot's camera relative to cameras direction
	// x "horizontal", y "verticle", z "distance"
	x1 = vBall.d * tan(radians(vBall.imgHead));
	y1 = vBall.d * tan(radians(vBall.imgElev));
	z1 = vBall.d;

	// coordinates of ball with respect to base of neck relative to cameras direction
	x2 = x1;
	y2 = y1 + neckLength;
	z2 = z1 + faceLength;

	// coordinates of ball with respect to base of neck relative to body's pan and camera's tilt
	x3 = x2*cos(radians(-hPan))-z2*sin(radians(-hPan));
	y3 = y2;
	z3 = x2*sin(radians(-hPan))+z2*cos(radians(-hPan));

	// coordinates of ball with respect to base of neck relative to body's direction
	x4 = x3;
	y4 = z3*sin(radians(hTilt))+y3*cos(radians(hTilt));
	z4 = z3*cos(radians(hTilt))-y3*sin(radians(hTilt));

	// ???
	if (z4 < neckLength) z4 = neckLength;

	headtype = ABS_H;
	// tilt = alpha - beta
	// alpha = angle of tilt at base of neck
	// beta = angle of elevation between base of neck and top of neck at centre of ball
	double dist = sqrt(z4*z4 + y4*y4);
	tilt = degrees(atan(y4/z4)-asin(neckLength/dist));

	// these coordinates are in terms of the top of the neck (pan pivot point)
	// and relative to the tilt calculated above
	x5 = x4;
	// i changed this because i thought it was simpler
	// we want z5 to be the coordinate of the ball with repect to the
	// top of the neck in the direction along the nose
	//      - Ross
	//z6 = z4*cos(radians(-tilt))-y4*sin(radians(-tilt));
	z5 = sqrt(SQUARE(dist) - SQUARE(neckLength));

	pan = degrees(atan(x5/z5));

	// need this bit?
	//tilt = hTilt+(tilt-hTilt)*0.6;
	//pan  = hPan+(pan-hPan)*0.8;
	}*/


	void ballChallenge::GetNearBall(double xoffset, double yoffset) {
		// calculate the vectors
		Vector target(vCART,
				vBall.x
				+ 6.0 * cos(DEG2RAD(gps->self().getHackedH())) + xoffset,
				vBall.y
				+ 6.0 * sin(DEG2RAD(gps->self().getHackedH())) + yoffset);
		Vector robot(vCART, (double) gps->self().pos.x, (double) gps->self().pos.y);

		// vector to destination point
		Vector vx(robot);
		vx.sub(target);

#define pointAngle3 utilNormaliseAngle( (vx.getHackedTheta() - gps->self().getHackedH()) )

		turnCCW = Cap(vBall.h / 2.0, 20);
		forward = -7.0 * cos(DEG2RAD(pointAngle3));
		left = -6.0 * sin(DEG2RAD(pointAngle3));
	}

	void ballChallenge::LookAroundParams() {
		headtype = ABS_H;
		tilty = 20;

		if (panx > 55.0) {
			panDirection = -1;
		}

		if (panx < -55.0) {
			panDirection = +1;
		}

		panx += (panDirection * 8.0);
	}

	void ballChallenge::SlowLookAroundParams() {
		headtype = ABS_H;
		tilty = 20;

		if (panx > 70.0) {
			panDirection = -1;
		}

		if (panx < -70.0) {
			panDirection = +1;
		}

		panx += (panDirection * 5.0);
	}

	void ballChallenge::FindBallHeadParams() {
		headtype = ABS_H;
		if (fbPanDir == 0 && fbTiltDir == 0) {
			if (gps->self().pos.x < 145.0) {
				fbPanDir = -1;
			}
			else {
				fbPanDir = 1;
			}
			tilty = -30.0;
			panx = fbPanDir * 8.0;
		}
		else {
			if (fbPanDir != 0) {
				if (panx <= -70 || panx >= 70) {
					if (tilty <= -30) {
						fbTiltDir = 1;
					}
					else if (tilty >= 0) {
						fbTiltDir = -1;
					}
					fbPanDir = 0;
				}
			}
			else if (fbTiltDir != 0) {
				if (tilty <= -30 || tilty >= 0) {
					if (panx <= -70) {
						fbPanDir = 1;
					}
					else if (panx >= 70) {
						fbPanDir = -1;
					}
					fbTiltDir = 0;
				}
			}
			panx += fbPanDir * 8.0;
			tilty += fbTiltDir * 8.0;
		}
	}

	void ballChallenge::setC3GotoBallParams() {
		if (vBall.x < 30.0) {
			// left edge
			if (gps->self().getHackedH() > 160.0
					|| gps->self().getHackedH() < -160.0
					|| vBall.d < 20.0) {
				setStraightGotoBallParams();
			}
			else {
				GetNearBall(25.0, 0.0);
			}
		}
		else if (vBall.x > 260.0) {
			// right edge
			if ((gps->self().getHackedH() < 20.0 && gps->self().getHackedH() > -20.0)
					|| vBall.d < 20.0) {
				setStraightGotoBallParams();
			}
			else {
				GetNearBall(-25.0, 0.0);
			}
		}
		else {
			setStraightGotoBallParams();
		}
	}        

	void ballChallenge::setStraightGotoBallParams() {
		// need a lot of turn to get align with ball
		if (ABS(vBall.h) > 30) {
			forward = Cap(vBall.d, 2.0) * cos(DEG2RAD(vBall.h));
			left = Cap(vBall.d, 2.0) * sin(DEG2RAD(vBall.h));
			// ricky ross: using cam dist instead
			if (vBall.vob->cam_dist < 50)
				turnCCW = Cap(vBall.h / 2, 30.0);
			else
				turnCCW = Cap(vBall.h, 30.0);
		}

		// can see the ball comfortably
		else {
			if (vBall.d > 30.0) {
				forward = Cap(vBall.d, 8.0) * cos(DEG2RAD(vBall.h));
				left = Cap(vBall.d, 6.0) * sin(DEG2RAD(vBall.h));
			}
			else {
				forward = Cap(vBall.d, 6.0) * cos(DEG2RAD(vBall.h));
				left = Cap(vBall.d, 5.0) * sin(DEG2RAD(vBall.h));
			}

			double move = sqrt(SQUARE(forward) + SQUARE(left));
			double maxTurn = 9 - move;

			turnCCW = Cap(vBall.h / 2.0, maxTurn);
		}
	}
