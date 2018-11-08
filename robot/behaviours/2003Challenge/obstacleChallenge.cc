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
 * $Id: obstacleChallenge.cc 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/

#include "obstacleChallenge.h"


namespace obstacleChallenge {
	//--- action states -----------------------
	static const int STATE_FIND_TARGET_GOAL = 0;
	static const int STATE_ESCAPE_TRAP = 1;
	static const int STATE_TESTING = 2;


	//--- sub action states -------------------
	static const int ACTION_ROTATE_FACE_TARGET = 0;
	static const int ACTION_LOOK_FOR_GAP = 1;
	static const int ACTION_ROTATE_FACE_GAP = 2;
	static const int ACTION_GAP_WALK = 3;


	//--- gap walk modes ----------------------
	static const int MODE_DIRECT_TO_GOAL = 0;
	static const int MODE_FREQ_STOP = 1;
	static const int MODE_NO_GAP = 2;
	static const int MODE_NEAR_GOAL = 3;


	//--- current action state ----------------
	static int currentState;
	static int currentSubState;
	static int currentGapWalkMode;
	static int sideWalkLock;   //0=nolock, 1=left, -1=right
	static int sideStepLock;   //0=nolock, 1=left, -1=right used when front block


	//--- internal flags ----------------------
	static bool sideWalkInit;
	static bool lockHead;
	static bool blockLeft;
	static bool blockRight;
	static bool escapeStable;
	static bool fixHead;
	static bool foundRobot;
	static bool infraRedTracking;


	//--- internal counters -------------------
	static double lastPanX;
	static double lastTiltY;
	static double old_tilt;
	static double old_pan;
	static int panDirection;
	static int sideWalkLockCount;
	static int sideStepLockCount;
	static int escapeCount;
	static int gapWalkCount;
	static int stateTimeout;
	static int infraRedTrackingCount;
	static double infraRedTrackingHead;


	//--- local variables ---------------------
	static int goalX;
	static int goalY;
	static double goalH;

	static int targetX;
	static int targetY;
	static double targetH;
	static double startX;
	static double startY;
	static double scale;

	static int escapeX;
	static int escapeY;
	static double escapeH;


	//--- scan gap data -----------------------
	static const  int NUM_GAPS = 13;
	static const  int gapDir[] = {
		60, 50, 40, 30, 20, 10, 0, -10, -20, -30, -40, -50, -60
	};
	static int gapScanCount;
	static double gapWalkDist;
	static double gapScanTotal[NUM_GAPS];
	static int gapScanTotalCount[NUM_GAPS];
	static double gapScanInfraRed[NUM_GAPS];
	static double gapScanAverage[NUM_GAPS];


	//--- world grid --------------------------
	static const int GRID_SIZE = 30;
	static const int NUM_ROWS = FIELD_LENGTH / GRID_SIZE;
	static const int NUM_COLS = FIELD_WIDTH / GRID_SIZE;
	static bool grid[NUM_ROWS][NUM_COLS];
}



//--- init variables ----------------------
void obstacleChallenge::initObsChallenge() {
	lastPanX = 0;
	lastTiltY = 0;
	old_tilt = 0;
	old_pan = 0;     
	panDirection = 1; 

	sideWalkLock = 0; 
	sideWalkInit = false;
	sideWalkLockCount = 0;
	sideStepLockCount = 0;
	escapeCount = 0;

	gapWalkCount = 0;
	gapScanCount = 0;
	gapWalkDist = 100;

	lockHead = false;
	blockLeft = false;
	blockRight = false;
	escapeStable = false;
	fixHead = false;

	foundRobot = false;
	infraRedTracking = false;
	infraRedTrackingCount = 0;
	infraRedTrackingHead = 0;

	goalX = FIELD_WIDTH / 2;  
	goalY = FIELD_LENGTH;
	targetX = FIELD_WIDTH / 2;  
	targetY = FIELD_LENGTH;

	lockMode = NoMode;
	stateTimeout = 0;
	currentState = STATE_FIND_TARGET_GOAL;
	//currentState          = STATE_TESTING;
	currentSubState = ACTION_ROTATE_FACE_TARGET;
	currentGapWalkMode = MODE_FREQ_STOP;
}



//--- entry for each frame ----------------
void obstacleChallenge::doObsChallenge() {
	setDefaultParams();
	setWalkParams();
	setObsChallengeValues();
	decideObsChallengeNextAction();
}



//--- change variables --------------------
void obstacleChallenge::setObsChallengeValues() {
	obsChallengeLookAroundHeadParams();
}



//--- choose an action to take ------------
void obstacleChallenge::decideObsChallengeNextAction() {
	if (lockMode != NoMode && lockMode != ResetMode) {
		switch (lockMode) {
			case ChestPush: aaChestPush();
					break;
			case SpinKick: aaSpinKick();
				       break;
			case SpinChestPush: aaSpinChestPush();
					    break;
			case GoalieKick: aaGoalieKick();
					 break;
			case LightningKick: aaLightningKick();
					    break;
		}
	}
	else {
		switch (currentState) {
			case STATE_FIND_TARGET_GOAL: obsChallengeFindTargetGoal();
						     break;
			case STATE_ESCAPE_TRAP: obsChallengeEscapeTrap();
						break;
			case STATE_TESTING: obsChallengeTesting();
					    break;
			default: obsChallengeFindTargetGoal();
				 break;
		}
	}
}





/////////////////////////////////////////////////////////////
// Actions Start here
/////////////////////////////////////////////////////////////


//--- temp testing --------------------------------------
void obstacleChallenge::obsChallengeTesting() {
	setStandParams();
	obsChallengeRobotTracking();
	escapeCount++;

	if (escapeCount == 25) {
		escapeCount = 0;
		for (int i = vobRedDog; i <= vobBlueDog4; i++) {
			if (vision->vob[i].cf > 0 && vision->vob[i].var < get95CF(150)) {
				if (i <= vobRedDog4)
					cout << "Red:  " << vision->vob[i].d << endl;
				else
					cout << "Blue: " << vision->vob[i].d << endl;
			}
		}
		cout << "\n" << endl;
		cout << "Infra Red: " << sensors->sensorVal[ssINFRARED_FAR] / 10000.0
			<< endl;
	}
}



//--- start challenge, find target goal -----------------
void obstacleChallenge::obsChallengeFindTargetGoal() {
	//-- rotate on the spot to face goal -------------
	if (currentSubState == ACTION_ROTATE_FACE_TARGET) {
		leds(2, 1, 1, 2, 1, 1);
		stateTimeout++;

		double diffx = goalX - gps->self().pos.x; 
		double diffy = goalY - gps->self().pos.y;
		goalH = RAD2DEG(atan2(diffy, diffx));
		double maxTurn = 15.0;
		double relh = NormalizeAngle_180(goalH - (gps->self().h));

		turnCCW = CLIP(relh, maxTurn);
		forward = 0;
		left = 0;

		if (ABS(NormalizeAngle_180(gps->self().h - goalH)) < 10
				|| stateTimeout > 200) {
			// 8 sec
			currentSubState = ACTION_LOOK_FOR_GAP;
			stateTimeout = 0;
			gapScanCount = 0;
			obsChallengeClearWorldGrid();
			for (int i = 0; i < NUM_GAPS; i++) {
				gapScanTotal[i] = 0;
				gapScanTotalCount[i] = 0;
				gapScanInfraRed[i] = 0;
				gapScanAverage[i] = 0;
			}
		}
		return;


		//-- stand and look for gap ----------------------
	}
	else if (currentSubState == ACTION_LOOK_FOR_GAP) {
		leds(1, 2, 1, 1, 2, 1);
		stateTimeout++;

		//- stand ----------------------------
		setStandParams();
		forward = 0;
		left = 0;
		turnCCW = 0;


		//- analyse scan results -------------
		if (gapScanCount > 1 || stateTimeout > 125) {
			// 5 sec

			// average distance   
			for (int i = 0; i < NUM_GAPS; i++) {
				if (gapScanTotalCount[i] > 0)
					gapScanAverage[i] = gapScanTotal[i] / gapScanTotalCount[i];
				cout << "Gap " << gapDir[i] << ": " << gapScanAverage[i] << endl;
				cout << "IR  " << gapDir[i] << ": " << gapScanInfraRed[i] << endl;
			}

			// target goal distance
			double goalDist = DISTANCE(gps->self().pos.x,
					gps->self().pos.y,
					gps->tGoal().pos.x,
					gps->tGoal().pos.y);


			// world grid density
			int leftDense = 0;
			int rightDense = 0;
			int totalGrid = 0;
			int selfX = (int) (gps->self().pos.x / GRID_SIZE);
			int selfY = (int) (gps->self().pos.y / GRID_SIZE);

			for (int r = selfY + 1; r < NUM_ROWS; r++) {
				for (int c = 0; c < NUM_COLS; c++) {
					totalGrid++;
					if (grid[r][c]) {
						if (NUM_COLS % 2 == 1) {
							// odd cols
							if (c <= NUM_COLS / 2)
								leftDense++;
							if (c >= NUM_COLS / 2)
								rightDense++;
						}
						else {
							// even cols
							if (c < NUM_COLS / 2)
								leftDense++;
							else
								rightDense++;
						}
					}
				}
			}
			cout << "world grid" << endl;            
			obsChallengePrintWorldGrid();


			startX = gps->self().pos.x;
			startY = gps->self().pos.y;

			// clear path to goal
			if (gapScanAverage[6] == 0
					&& gapScanAverage[5] == 0
					&& gapScanAverage[7] == 0) {
				cout << "clear path to goal 10,0,-10" << endl;            
				targetX = goalX;
				targetY = goalY;
				currentGapWalkMode = MODE_DIRECT_TO_GOAL;
			}
			else if ((gapScanAverage[6] == 0 || gapScanAverage[6] > 150)
					&& (gapScanAverage[5] == 0 || gapScanAverage[5] > 150)
					&& (gapScanAverage[7] == 0 || gapScanAverage[7] > 150)) {
				cout << "clear path to goal 0" << endl;                
				targetX = goalX;
				targetY = goalY;
				currentGapWalkMode = MODE_DIRECT_TO_GOAL;
				//TO FIX
				/*
				   } else if ((gapScanAverage[6]==0 || gapScanAverage[6]>goalDist-40) && 
				   (gapScanAverage[5]==0 || gapScanAverage[5]>goalDist-40) && 
				   (gapScanAverage[7]==0 || gapScanAverage[7]>goalDist-40)) {
				   cout << "clear path to goal with robot in goal" << endl;                
				   targetX = goalX;
				   targetY = goalY;
				   currentGapWalkMode = MODE_DIRECT_TO_GOAL;
				   */
				//END TO FIX


				// near goal            
			}
			else if (ABS(gps->self().pos.x - goalX) <= 50 && ABS(gps->self().pos.y
						- goalY) <= 50) {
				cout << "near goal" << endl;            
				targetX = goalX;
				targetY = goalY;
				currentGapWalkMode = MODE_NEAR_GOAL;


				// stop and look mode
			}
			else {
				// see if robot near by
				double closestDogDist = 0;
				for (int i = 0; i < NUM_GAPS; i++) {
					if (closestDogDist == 0 || gapScanAverage[i] < closestDogDist)
						closestDogDist = gapScanAverage[i];
				}


				bool found = false;
				int gapStart = 0;
				int gapEnd = 0;

				// with robot within 60 cm
				if (closestDogDist < 60) {
					cout << "robot within 60cm" << endl;                
					int p = 0;
					int endP = NUM_GAPS;

					while (true) {
						int s = -1;
						for (; p < endP; p++) {
							if (gapScanAverage[p] == 0) {
								s = p;
								break;
							}
						}
						if (p == endP)
							break;

						int e = s;
						for (; p < endP; p++) {
							if (gapScanAverage[p] != 0) {
								break;
							}
							else {
								e = p;
							}
						}


						if (!found || e - s > gapEnd - gapStart) {
							cout << "gap start " << gapDir[gapStart] << " gap end "
								<< gapDir[gapEnd] << endl;                        
							found = true;
							gapStart = s;
							gapEnd = e;
						}
						else if (e - s == gapEnd - gapStart) {
							cout << "same size largest gap" << endl;

							//bias towards non-dense side
							if (leftDense > rightDense
									&& (e - s) / 2 > (gapEnd - gapStart) / 2) {
								cout << "bias towards non-dense side" << endl;                            
								found = true;
								gapStart = s;
								gapEnd = e;

								//bias towards edge
							}
							else if ((gps->self().pos.x < 50 && s == 0)
									|| (gps->self().pos.x > FIELD_WIDTH - 50
										&& e == NUM_GAPS - 1)) {
								cout << "bias towards edge" << endl;
								cout << "gap start " << gapDir[gapStart] << " gap end "
									<< gapDir[gapEnd] << endl;                        

								found = true;
								gapStart = s;
								gapEnd = e;

								//bias towards centre
							}
							else {
								if (ABS(gapDir[(e - s) / 2])
										< ABS(gapDir[(gapEnd - gapStart) / 2])) {
									cout << "bias towards centre" << endl;
									cout << "gap start " << gapDir[gapStart] << " gap end "
										<< gapDir[gapEnd] << endl;                        

									found = true;
									gapStart = s;
									gapEnd = e;
								}
							}
						}
					}

					// no robot within 60 cm
				}
				else {
					cout << "no robot within 60cm" << endl;                

					int p = 3;  //choose within 30 degrees
					while (true) {
						int s = -1;
						for (; p < NUM_GAPS - 3; p++) {
							if (gapScanAverage[p] == 0) {
								s = p;
								break;
							}
						}
						if (p == NUM_GAPS - 3)
							break;

						int e = s;
						for (; p < NUM_GAPS - 3; p++) {
							if (gapScanAverage[p] != 0) {
								break;
							}
							else {
								e = p;
							}
						}


						if (!found || e - s > gapEnd - gapStart) {
							cout << "gap start " << gapDir[gapStart] << " gap end "
								<< gapDir[gapEnd] << endl;                        

							found = true;
							gapStart = s;
							gapEnd = e;
						}
						else if (e - s == gapEnd - gapStart) {
							cout << "same size largest gap" << endl;   

							//bias towards non-dense side
							if (leftDense > rightDense
									&& (e - s) / 2 > (gapEnd - gapStart) / 2) {
								cout << "bias towards non-dense side" << endl;                            
								found = true;
								gapStart = s;
								gapEnd = e;                 

								//bias towards edge
							}
							else if ((gps->self().pos.x < 50 && s == 0)
									|| (gps->self().pos.x > FIELD_WIDTH - 50
										&& e == NUM_GAPS - 1)) {
								cout << "bias towards edge" << endl;
								cout << "gap start " << gapDir[gapStart] << " gap end "
									<< gapDir[gapEnd] << endl;                        
								found = true;
								gapStart = s;
								gapEnd = e;

								//bias towards centre
							}
							else {
								if (ABS(gapDir[(e - s) / 2])
										< ABS(gapDir[(gapEnd - gapStart) / 2])) {
									cout << "bias towards centre" << endl;
									cout << "gap start " << gapDir[gapStart] << " gap end "
										<< gapDir[gapEnd] << endl;                        
									found = true;
									gapStart = s;
									gapEnd = e;
								}
							}
						}
					}
				}


				// no gap, start reduce range
				if (!found) {
					cout << "no gap found" << endl;                
					for (int d = 300; d >= 150 && !found; d -= 50) {
						cout << "using distance " << d << endl;                    
						int p = 0;
						int endP = NUM_GAPS;

						while (true) {
							int s = -1;
							for (; p < endP; p++) {
								if (gapScanAverage[p] > d) {
									s = p;
									break;
								}
							}
							if (p == endP)
								break;

							int e = s;
							for (; p < endP; p++) {
								if (gapScanAverage[p] <= d) {
									break;
								}
								else {
									e = p;
								}
							}


							if (!found || e - s > gapEnd - gapStart) {
								cout << "gap start " << gapDir[gapStart] << " gap end "
									<< gapDir[gapEnd] << endl;                        
								found = true;
								gapStart = s;
								gapEnd = e;
							}
							else if (e - s == gapEnd - gapStart) {
								cout << "same size largest gap" << endl; 

								//bias towards non-dense side
								if (leftDense > rightDense
										&& (e - s) / 2 > (gapEnd - gapStart) / 2) {
									cout << "bias towards non-dense side" << endl;                            
									found = true;
									gapStart = s;
									gapEnd = e;      

									//bias towards edge
								}
								else if ((gps->self().pos.x < 50 && s == 0)
										|| (gps->self().pos.x > FIELD_WIDTH - 50
											&& e == NUM_GAPS - 1)) {
									cout << "bias towards edge" << endl;
									cout << "gap start " << gapDir[gapStart] << " gap end "
										<< gapDir[gapEnd] << endl;                        
									found = true;
									gapStart = s;
									gapEnd = e;

									//bias towards centre
								}
								else {
									if (ABS(gapDir[(e - s) / 2])
											< ABS(gapDir[(gapEnd - gapStart) / 2])) {
										cout << "bias towards centre" << endl;
										cout << "gap start " << gapDir[gapStart] << " gap end "
											<< gapDir[gapEnd] << endl;                        
										found = true;
										gapStart = s;
										gapEnd = e;
									}
								}
							}
						}
					}
				}


				//still no gap
				if (!found) {
					cout << "found no gap, doggie walk to goal" << endl;                
					targetX = goalX;
					targetY = goalY;
					currentGapWalkMode = MODE_NO_GAP;

					//target through gap
				}
				else {
					double gapStartDist = 0;
					double gapEndDist = 0;
					if (gapStart != 0)
						gapStartDist = gapScanAverage[gapStart - 1];
					if (gapEnd != NUM_GAPS - 1)
						gapEndDist = gapScanAverage[gapEnd + 1];

					if (gapStartDist != 0 && gapEndDist != 0) {
						if (ABS(gapStartDist - gapEndDist) >= 80) {
							gapWalkDist = (gapStartDist + gapEndDist) / 2;
						}
						else {
							if (gapStartDist > gapEndDist)
								gapWalkDist = gapStartDist + 50;
							else
								gapWalkDist = gapEndDist + 50;
						}
					}
					else if (gapStartDist == 0 && gapEndDist == 0) {
						gapWalkDist = FIELD_LENGTH - gps->self().pos.y;
					}
					else if (gapStartDist != 0) {
						gapWalkDist = gapStartDist + 50;
					}
					else {
						gapWalkDist = gapEndDist + 50;
					}

					int gap = (gapStart + gapEnd) / 2;
					targetH = gapDir[gap];
					cout << "walk direction " << gapDir[gap] << endl;
					cout << "walk distance " << gapWalkDist << endl;                    

					double deltax = gapWalkDist *cos(DEG2RAD(targetH + gps->self().h));
					double deltay = gapWalkDist *sin(DEG2RAD(targetH + gps->self().h));

					targetX = CLIP((int) (gps->self().pos.x + deltax),
							FIELD_WIDTH - (WALL_THICKNESS + 5));
					targetY = CLIP((int) (gps->self().pos.y + deltay),
							FIELD_LENGTH - (WALL_THICKNESS + 5));

					if (targetX < WALL_THICKNESS + 5)
						targetX = WALL_THICKNESS + 5;

					// clip top left corner
					if (targetX<CORNER_WIDTH && targetY>FIELD_LENGTH - CORNER_WIDTH) {
						targetX = CORNER_WIDTH;
						targetY = FIELD_LENGTH - CORNER_WIDTH;
						// clip top right corner
					}
					else if (targetX > FIELD_WIDTH - CORNER_WIDTH
							&& targetY > FIELD_LENGTH - CORNER_WIDTH) {
						targetX = FIELD_WIDTH - CORNER_WIDTH;
						targetY = FIELD_LENGTH - CORNER_WIDTH;
					} 

					currentGapWalkMode = MODE_FREQ_STOP;

					cout << "targetX = " << targetX << " targetY = " << targetY << endl;
					cout << "frequent stop mode" << endl;
				}
			}
			currentSubState = ACTION_ROTATE_FACE_GAP;
			stateTimeout = 0;
			return;
		}


		//- collect scan data ----------------
		obsChallengeUpdateWorldGrid();

		if (infraRedTracking) {
			double dist = obsChallengeInfraRedRobotDist(infraRedTrackingHead, 10.0);
			if (dist != -1 || infraRedTrackingCount > 15) {
				for (int j = 0; j < NUM_GAPS; j++) {
					if (ABS(infraRedTrackingHead - gapDir[j]) <= 5) {
						gapScanInfraRed[j] = dist;
					}
				}

				foundRobot = false;
				infraRedTracking = false;
				infraRedTrackingCount = 0;
				infraRedTrackingHead = 0;
			}
		}

		for (int i = vobRedDog; i <= vobBlueDog4; i++) {
			if (vision->vob[i].cf > 0 && vision->vob[i].var < get95CF(150)) {
				//estimate robot position
				double deltax = vision->vob[i].d *cos(DEG2RAD(vision->vob[i].head
							+ gps->self().h));
				double deltay = vision->vob[i].d *sin(DEG2RAD(vision->vob[i].head
							+ gps->self().h));
				double robotx = gps->self().pos.x + deltax;
				double roboty = gps->self().pos.y + deltay;


				for (int j = 0; j < NUM_GAPS; j++) {
					//close robot
					if (vision->vob[i].d < 60) {
						//left edge
						if (robotx < 35) {
							if (gapDir[j] > vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 10) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}

							//right edge
						}
						else if (robotx > FIELD_WIDTH - 35) {
							if (gapDir[j] < vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 10) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}

							//target edge left and self left
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx < FIELD_WIDTH / 2
								&&                //robot on left
								gps->self().pos.x < FIELD_WIDTH / 2) {
							//self on left

							if (gapDir[j] > vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 10) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}       

							//target edge right and self left
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx > FIELD_WIDTH / 2
								&&                //robot on right
								gps->self().pos.x < FIELD_WIDTH / 2) {
							//self on left

							if (gapDir[j] < vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 10) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}       

							//target edge right and self right
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx > FIELD_WIDTH / 2
								&&                //robot on right
								gps->self().pos.x > FIELD_WIDTH / 2) {
							//self on right 

							if (gapDir[j] < vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 10) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}                           

							//target edge left and self right
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx <FIELD_WIDTH / 2
								&&                //robot on left
								gps->self().pos.x> FIELD_WIDTH / 2) {
							//self on right

							if (gapDir[j] > vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 10) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}                                                          

							//not near edge
						}
						else if (ABS(vision->vob[i].head - gapDir[j]) <= 10) {
							gapScanTotal[j] += vision->vob[i].d;
							gapScanTotalCount[j]++;

							if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								if (!infraRedTracking) {
									foundRobot = false;
									infraRedTracking = true;
									infraRedTrackingCount = 0;
									infraRedTrackingHead = vision->vob[i].head;
								}
							}
						}


						//far robot

					}
					else {
						//left edge
						if (robotx < 35) {
							if (gapDir[j] > vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;
							}

							//right edge
						}
						else if (robotx > FIELD_WIDTH - 35) {
							if (gapDir[j] < vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;
							}

							//target edge left and self left
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx < FIELD_WIDTH / 2
								&&                //robot on left
								gps->self().pos.x < FIELD_WIDTH / 2) {
							//self on left

							if (gapDir[j] > vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}       

							//target edge right and self left
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx > FIELD_WIDTH / 2
								&&                //robot on right
								gps->self().pos.x < FIELD_WIDTH / 2) {
							//self on left

							if (gapDir[j] < vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}       

							//target edge right and self right
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx > FIELD_WIDTH / 2
								&&                //robot on right
								gps->self().pos.x > FIELD_WIDTH / 2) {
							//self on right

							if (gapDir[j] < vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}                           

							//target edge left and self right
						}
						else if (roboty > FIELD_LENGTH - 35
								&&              //robot on edge
								gps->self().pos.y > FIELD_LENGTH - 100
								&&  //self on edge
								robotx <FIELD_WIDTH / 2
								&&                //robot on left
								gps->self().pos.x> FIELD_WIDTH / 2) {
							//self on right

							if (gapDir[j] > vision->vob[i].head || ABS(vision->vob[i].head
										- gapDir[j]) <= 5) {
								gapScanTotal[j] += vision->vob[i].d;
								gapScanTotalCount[j]++;

								if (gapScanInfraRed[j] == 0 && ABS(vision->vob[i].head
											- gapDir[j]) <= 5) {
									if (!infraRedTracking) {
										foundRobot = false;
										infraRedTracking = true;
										infraRedTrackingCount = 0;
										infraRedTrackingHead = vision->vob[i].head;
									}
								}
							}                         

							//not near edge
						}
						else if (ABS(vision->vob[i].head - gapDir[j]) <= 5) {
							gapScanTotal[j] += vision->vob[i].d;
							gapScanTotalCount[j]++;
						}
					}
				}
			}
		}
		return;


		//-- rotate on the spot to face gap --------------
	}
	else if (currentSubState == ACTION_ROTATE_FACE_GAP) {
		leds(1, 1, 2, 1, 1, 2);
		stateTimeout++;

		double diffx = targetX - gps->self().pos.x; 
		double diffy = targetY - gps->self().pos.y;
		targetH = RAD2DEG(atan2(diffy, diffx));
		double maxTurn = 15.0;
		double relh = NormalizeAngle_180(targetH - (gps->self().h));

		turnCCW = CLIP(relh, maxTurn);
		forward = 0;
		left = 0;

		if (ABS(NormalizeAngle_180(gps->self().h - targetH)) < 10
				|| stateTimeout > 200) {
			//8 sec
			if (stateTimeout > 200) {
				cout << "ROTATE_FACE_GAP timed out selfH = " << gps->self().h
					<< " targetH = " << targetH << endl;
			}        
			currentSubState = ACTION_GAP_WALK;
			gapWalkCount = 0;
			stateTimeout = 0;
		}
		return;


		//-- walk through gap ----------------------------
	}
	else if (currentSubState == ACTION_GAP_WALK) {
		gapWalkCount++;

		// Direct to goal use visual goal
		if (currentGapWalkMode == MODE_DIRECT_TO_GOAL) {
			//calculate walk
			obsChallengeGoDirectToTarget();

			bool found70 = false;
			bool obsInGoal = false;
			double closestObs = 100;
			for (int i = vobRedDog; i <= vobBlueDog4; i++) {
				if (vision->vob[i].cf > 0 && vision->vob[i].var < get95CF(150)) {
					if (ABS(vision->vob[i].head) < 20 && vision->vob[i].d < 70) {
						found70 = true;

						if (vision->vob[i].d < closestObs) {
							closestObs = vision->vob[i].d;

							double deltax = vision->vob[i].d *cos(DEG2RAD(vision->vob[i].head
										+ gps->self().h));
							double deltay = vision->vob[i].d *sin(DEG2RAD(vision->vob[i].head
										+ gps->self().h));
							double obsx = gps->self().pos.x + deltax;
							double obsy = gps->self().pos.y + deltay;

							if (ABS(obsx - goalX) <= 40 && obsy > FIELD_LENGTH - 40)
								obsInGoal = true;
							else
								obsInGoal = false;
						}
					}
				}
			}

			bool inGoalBox = false;
			if (ABS(gps->self().pos.x - goalX) <= 40 && ABS(gps->self().pos.y
						- goalY) <= 45) {
				inGoalBox = true;
			}

			//break if caught in own goal
			if (obsChallengeLockInOwnGoal()) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}

			//if obstacle in goal, keep walking
			if (obsInGoal) {
				return;
			}

			//break if front is obstructed
			if (found70 && !inGoalBox) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}
		}


		// Mode frequent stopping
		if (currentGapWalkMode == MODE_FREQ_STOP) {
			//calculate walk
			obsChallengeGoDirectToTarget();

			//break if caught in own goal
			if (obsChallengeLockInOwnGoal()) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}


			bool breakWall = false;
			if ((gps->self().pos.x < 15 && targetX < 0)
					|| (gps->self().pos.x > FIELD_WIDTH - 15 && targetX > FIELD_WIDTH)) {
				breakWall = true;
			}
			if (gps->self().pos.y > FIELD_LENGTH - 15 && targetY > FIELD_LENGTH) {
				breakWall = true;
			}

			//break if target is reached
			//or is the intended distance away from original position
			//or have walked for 20secs
			//or is on the wall and target is beyond the wall
			//or is in front of goal
			double walkedDist = DISTANCE(startX,
					startY,
					gps->self().pos.x,
					gps->self().pos.y);
			if ((ABS(gps->self().pos.x - targetX) < 15 && ABS(gps->self().pos.y
							- targetY) < 15)
					|| walkedDist
					> gapWalkDist
					|| gapWalkCount
					> 500 //|| breakWall
					|| (ABS(gps->self().pos.x - goalX) <= 40 && ABS(gps->self().pos.y
							- goalY) <= 45)) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}
		}


		// Mode no gap
		if (currentGapWalkMode == MODE_NO_GAP) {
			//local avoidance
			obsChallengeGotoTarget();

			//break if caught in own goal
			if (obsChallengeLockInOwnGoal()) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}

			//break after 10 sec
			if (gapWalkCount > 250) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}
		}


		// Mode near goal
		if (currentGapWalkMode == MODE_NEAR_GOAL) {
			//local avoidance
			obsChallengeGotoTarget();

			//break if location is far off
			if (gps->self().pos.y<FIELD_LENGTH - 80
					&& ABS(gps->self().pos.x - goalX)>80) {
				setStandParams();
				forward = 0;
				left = 0;
				turnCCW = 0;
				stateTimeout = 0;
				currentSubState = ACTION_ROTATE_FACE_TARGET;
				return;
			}

			//walk into goal if reached target
			if (ABS(gps->self().pos.y - goalY) < 10
					&& ABS(gps->self().pos.x - goalX) < 10) {
				currentGapWalkMode = MODE_DIRECT_TO_GOAL;
			}
		}


		// default
	}
	else {
		setStandParams();
		forward = 0;
		left = 0;
		turnCCW = 0;
		stateTimeout = 0;
		currentSubState = ACTION_ROTATE_FACE_TARGET;
	}
}




//--- front avoidance -----------------------------------
void obstacleChallenge::obsChallengeGoDirectToTarget() {
	//--- heading to target -------------------------
	double diffx = targetX - gps->self().pos.x; 
	double diffy = targetY - gps->self().pos.y;
	targetH = RAD2DEG(atan2(diffy, diffx));


	//--- turn and walk to target -------------------
	double maxSpeed = 6.0;
	double maxTurn = 15.0;
	double relx = targetX - (gps->self().pos.x);
	double rely = targetY - (gps->self().pos.y);
	double reld = sqrt(SQUARE(relx) + SQUARE(rely));
	double relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
			- (gps->self().h));
	double relh = NormalizeAngle_180(targetH - (gps->self().h));

	forward = CLIP(reld, maxSpeed) * cos(DEG2RAD(relTheta));
	left = CLIP(reld, maxSpeed) * sin(DEG2RAD(relTheta));
	turnCCW = CLIP(relh, maxTurn);


	//--- use visual goal ---------------------------
	fixHead = false;
	if (currentGapWalkMode
			== MODE_DIRECT_TO_GOAL
			&& ABS(gps->self().pos.x
				- goalX)
			< 15
			&& ABS(gps->self().pos.y
				- goalY)
			< 15) {
		if (gps->canSee(gps->targetGoal)) {
			fixHead = true;
			//obsChallengeGoalTracking();
			relx = vision->vob[gps->targetGoal].d * cos(DEG2RAD(vision->vob[gps->targetGoal].head
						+ gps->self().h));
			rely = vision->vob[gps->targetGoal].d * sin(DEG2RAD(vision->vob[gps->targetGoal].head
						+ gps->self().h));
			reld = sqrt(SQUARE(relx) + SQUARE(rely));
			relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
					- (gps->self().h));

			forward = CLIP(reld, maxSpeed) * cos(DEG2RAD(relTheta));
			left = CLIP(reld, maxSpeed) * sin(DEG2RAD(relTheta));
			turnCCW = CLIP(vision->vob[gps->targetGoal].head, maxTurn);
		}
	} 



	//--- side way check ----------------------------
	if (lockHead) {
		//-- 1st look to dir of walk --
		if (sideStepLockCount > 60) {
			for (int i = vobRedDog; i <= vobBlueDog4; i++) {
				if (vision->vob[i].cf > 0 && vision->vob[i].d < 40) {
					if (vision->vob[i].head > 45)
						blockLeft = true;
					if (vision->vob[i].head < -45)
						blockRight = true;
				}
			}
			setStandParams();
			forward = 0;
			left = 0;
			turnCCW = 0;
			sideStepLockCount--;
			return;

			//-- 2nd look to opp dir of walk --
		}
		else if (sideStepLockCount > 45) {
			if (sideStepLock == 1 && !blockLeft) {
				// if our current
				sideStepLockCount = 45;                       // way is already
				return;                                       // clear, no need
			}
			else if (sideStepLock == -1 && !blockRight) {
				// to look opp dir
				sideStepLockCount = 45;
				return;
			}

			for (int i = vobRedDog; i <= vobBlueDog4; i++) {
				if (vision->vob[i].cf > 0 && vision->vob[i].d < 40) {
					if (vision->vob[i].head > 45)
						blockLeft = true;
					if (vision->vob[i].head < -45)
						blockRight = true;
				}
			}

			setStandParams();
			forward = 0;
			left = 0;
			turnCCW = 0;
			sideStepLockCount--;
			return;

			//-- look ahead and stablise --
		}
		else if (sideStepLockCount > 40) {
			if (sideStepLock == 1 && blockLeft && !blockRight) {
				sideStepLock = -1;
			}
			else if (sideStepLock == -1 && blockRight && !blockLeft) {
				sideStepLock = 1;
			}
			else if (blockLeft
					&& blockRight
					&& escapeCount
					< 400
					&&  //no run away from goal
					!(ABS(gps->self().pos.x - goalX) <= 65 && ABS(gps->self().pos.y
							- goalY) <= 100)
					&& !(gps->self().pos.y < 100)) {
				sideStepLockCount = 0;                       // no re-trigger
				sideStepLock = 0;                       // within 4 sec
				lockHead = false;
				currentState = STATE_ESCAPE_TRAP;
				escapeCount = 500;                     // approx 20 sec
				escapeStable = false;

				if (gps->self().pos.y - 60 > 0)                // 0.6m backward
					escapeY = (int) gps->self().pos.y - 60;
				else
					escapeY = 0;

				if (gps->self().pos.x > targetX) {
					// target on left
					if (gps->self().pos.x - 90 > 0)            // 0.9m left
						escapeX = (int) gps->self().pos.x - 90;
					else
						escapeX = 0;
				}
				else {
					// target on right
					if (gps->self().pos.x + 90 < FIELD_WIDTH)  // 0.9m right
						escapeX = (int) gps->self().pos.x + 90;
					else
						escapeX = FIELD_WIDTH;
				}

				obsChallengeEscapeTrap();
				return;
			}

			setStandParams();
			forward = 0;
			left = 0;
			turnCCW = 0;
			sideStepLockCount--;
			return;
		}
		else if (sideStepLockCount == 0) {
			sideStepLockCount = 70;
			blockLeft = false;
			blockRight = false;
		}
		else {
			sideStepLockCount--;
		}
	}


	//--- avoid front robot (vision) ----------------
	double closestDogDist = 100;
	lockHead = false;
	for (int i = vobRedDog; i <= vobBlueDog4; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].d < 80) {
			if (vision->vob[i].d < closestDogDist) {
				closestDogDist = vision->vob[i].d;

				if (vision->vob[i].d < 45) {
					// robot in front
					if (vision->vob[i].head > -20 && vision->vob[i].head < 20) {
						bool inGoalBox = false;
						if (ABS(gps->self().pos.x - goalX)
								< GOAL_WIDTH
								/ 2
								&& ABS(gps->self().pos.y
									> FIELD_LENGTH
									- GOALBOX_DEPTH))
							inGoalBox = true;

						bool inOwnGoal = false;
						if (ABS(gps->self().pos.x - goalX) < GOAL_WIDTH / 2
								&& gps->self().pos.y < WALL_THICKNESS)
							inOwnGoal = true;

						if (gps->self().pos.x <10
								|| gps->self().pos.x> FIELD_WIDTH - 10
								|| gps->self().pos.y > FIELD_LENGTH - 10
								|| inGoalBox
								|| inOwnGoal)
							sideStepLock = 0;

						if (sideStepLock == 0) {
							if (gps->self().pos.y > FIELD_LENGTH - 10) {
								bool faceUp = false;
								if (gps->self().h > 55 && gps->self().h < 125)
									faceUp = true;

								if (gps->self().pos.x < FIELD_WIDTH / 2 - GOAL_WIDTH / 2
										&& !faceUp) {
									sideStepLock = -1;
								}
								else if (gps->self().pos.x
										> FIELD_WIDTH
										- (FIELD_WIDTH / 2 + GOAL_WIDTH / 2)
										&& !faceUp) {
									sideStepLock = 1;
								}
								else {
									if (gps->self().pos.x < targetX) {
										sideStepLock = -1;
									}
									else {
										sideStepLock = 1;
									}
								}
							}
							else {
								if (gps->self().pos.x < targetX) {
									if (gps->self().pos.x > FIELD_WIDTH - 10)
										sideStepLock = 1;
									else
										sideStepLock = -1;
								}
								else {
									if (gps->self().pos.x < 10)
										sideStepLock = -1;
									else
										sideStepLock = 1;
								}
							}
						}

						forward = 0;
						turnCCW = 0;
						left = sideStepLock * 3;
						lockHead = true;
					}
				}

				leds(2, 2, 2, 2, 2, 2);                        // panic light
			}
		}
	}


	//--- avoid front robot (infra red) -------------
	bool nearGoal = (gps->self().pos.y > FIELD_LENGTH - 30
			&& ABS(gps->self().pos.x
				- goalX) < 35);
	bool nearEdge = (gps->self().pos.x <40
			|| gps->self().pos.x> FIELD_WIDTH - 40
			|| gps->self().pos.y > FIELD_LENGTH - 30);

	if (ABS(panx) < 15
			&& sensors->sensorVal[ssINFRARED_FAR] / 10000.0 < 40
			&& !nearGoal
			&& !nearEdge) {
		if (gps->self().pos.x <10
				|| gps->self().pos.x> FIELD_WIDTH - 10
				|| gps->self().pos.y > FIELD_LENGTH - 10)
			sideStepLock = 0;

		if (sideStepLock == 0) {
			if (gps->self().pos.y > FIELD_LENGTH - 10) {
				if (gps->self().pos.x < FIELD_WIDTH / 2 - GOAL_WIDTH / 2) {
					sideStepLock = -1;
				}
				else if (gps->self().pos.x
						> FIELD_WIDTH
						- (FIELD_WIDTH / 2 + GOAL_WIDTH / 2)) {
					sideStepLock = 1;
				}
				else {
					if (gps->self().pos.x < targetX) {
						sideStepLock = -1;
					}
					else {
						sideStepLock = 1;
					}
				}
			}
			else {
				if (gps->self().pos.x < targetX) {
					if (gps->self().pos.x > FIELD_WIDTH - 10)
						sideStepLock = 1;
					else
						sideStepLock = -1;
				}
				else {
					if (gps->self().pos.x < 10)
						sideStepLock = -1;
					else
						sideStepLock = 1;
				}
			}
		}

		forward = 0;
		turnCCW = 0;
		left = sideStepLock * 3;
		lockHead = true;

		leds(2, 2, 2, 2, 2, 2);                        // panic light
	}
}




//--- local avoidance -----------------------------------
void obstacleChallenge::obsChallengeGotoTarget() {
	//--- heading to target -------------------------
	double diffx = targetX - gps->self().pos.x; 
	double diffy = targetY - gps->self().pos.y;
	targetH = RAD2DEG(atan2(diffy, diffx));


	//--- turn and walk to target -------------------
	double maxSpeed = 6.0;
	double maxTurn = 15.0;
	double relx = targetX - (gps->self().pos.x);
	double rely = targetY - (gps->self().pos.y);
	double reld = sqrt(SQUARE(relx) + SQUARE(rely));
	double relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
			- (gps->self().h));
	double relh = NormalizeAngle_180(targetH - (gps->self().h));

	forward = CLIP(reld, maxSpeed) * cos(DEG2RAD(relTheta));
	left = CLIP(reld, maxSpeed) * sin(DEG2RAD(relTheta));
	turnCCW = CLIP(relh, maxTurn);


	//--- use visual goal ---------------------------
	fixHead = false;
	if (ABS(gps->self().pos.x - goalX) < 15
			&& ABS(gps->self().pos.y - goalY) < 15) {
		if (gps->canSee(gps->targetGoal)) {
			fixHead = true;
			//obsChallengeGoalTracking();
			relx = vision->vob[gps->targetGoal].d * cos(DEG2RAD(vision->vob[gps->targetGoal].head
						+ gps->self().h));
			rely = vision->vob[gps->targetGoal].d * sin(DEG2RAD(vision->vob[gps->targetGoal].head
						+ gps->self().h));
			reld = sqrt(SQUARE(relx) + SQUARE(rely));
			relTheta = NormalizeAngle_180(RAD2DEG(atan2(rely, relx))
					- (gps->self().h));

			forward = CLIP(reld, maxSpeed) * cos(DEG2RAD(relTheta));
			left = CLIP(reld, maxSpeed) * sin(DEG2RAD(relTheta));
			turnCCW = CLIP(vision->vob[gps->targetGoal].head, maxTurn);
		}
	} 


	//--- side walk lock ----------------------------
	if (sideWalkLock != 0 && sideWalkLockCount > 0) {
		forward = scale * forward;
		left = sideWalkLock;
		sideWalkLockCount--;
	}


	//--- side way check ----------------------------
	if (lockHead) {
		//-- 1st look to dir of walk --
		if (sideStepLockCount > 60) {
			for (int i = vobRedDog; i <= vobBlueDog4; i++) {
				if (vision->vob[i].cf > 0 && vision->vob[i].d < 40) {
					if (vision->vob[i].head > 45)
						blockLeft = true;
					if (vision->vob[i].head < -45)
						blockRight = true;
				}
			}
			setStandParams();
			forward = 0;
			left = 0;
			turnCCW = 0;
			sideStepLockCount--;
			return;

			//-- 2nd look to opp dir of walk --
		}
		else if (sideStepLockCount > 45) {
			if (sideStepLock == 1 && !blockLeft) {
				// if our current
				sideStepLockCount = 45;                       // way is already
				return;                                       // clear, no need
			}
			else if (sideStepLock == -1 && !blockRight) {
				// to look opp dir
				sideStepLockCount = 45;
				return;
			}

			for (int i = vobRedDog; i <= vobBlueDog4; i++) {
				if (vision->vob[i].cf > 0 && vision->vob[i].d < 40) {
					if (vision->vob[i].head > 45)
						blockLeft = true;
					if (vision->vob[i].head < -45)
						blockRight = true;
				}
			}

			setStandParams();
			forward = 0;
			left = 0;
			turnCCW = 0;
			sideStepLockCount--;
			return;

			//-- look ahead and stablise --
		}
		else if (sideStepLockCount > 40) {
			if (sideStepLock == 1 && blockLeft && !blockRight) {
				sideStepLock = -1;
			}
			else if (sideStepLock == -1 && blockRight && !blockLeft) {
				sideStepLock = 1;
			}
			else if (blockLeft
					&& blockRight
					&& escapeCount
					< 400
					&&  //no run away from goal
					!(ABS(gps->self().pos.x - goalX) <= 65 && ABS(gps->self().pos.y
							- goalY) <= 65)
					&& !(gps->self().pos.y < 100)) {
				sideStepLockCount = 0;                       // no re-trigger
				sideStepLock = 0;                       // within 4 sec
				lockHead = false;
				currentState = STATE_ESCAPE_TRAP;
				escapeCount = 500;                     // approx 20 sec
				escapeStable = false;

				if (gps->self().pos.y - 60 > 0)                // 0.6m backward
					escapeY = (int) gps->self().pos.y - 60;
				else
					escapeY = 0;

				if (gps->self().pos.x > targetX) {
					// target on left
					if (gps->self().pos.x - 90 > 0)            // 0.9m left
						escapeX = (int) gps->self().pos.x - 90;
					else
						escapeX = 0;
				}
				else {
					// target on right
					if (gps->self().pos.x + 90 < FIELD_WIDTH)  // 0.9m right
						escapeX = (int) gps->self().pos.x + 90;
					else
						escapeX = FIELD_WIDTH;
				}

				obsChallengeEscapeTrap();
				return;
			}

			setStandParams();
			forward = 0;
			left = 0;
			turnCCW = 0;
			sideStepLockCount--;
			return;
		}
		else if (sideStepLockCount == 0) {
			sideStepLockCount = 70;
			blockLeft = false;
			blockRight = false;
		}
		else {
			sideStepLockCount--;
		}
	}


	//--- avoid robot within 80cm (vision) ----------
	double closestDogDist = 100;
	lockHead = false;
	for (int i = vobRedDog; i <= vobBlueDog4; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].d < 80) {
			if (vision->vob[i].head > 50 || vision->vob[i].head < -50)
				continue;

			if (vision->vob[i].d < closestDogDist) {
				closestDogDist = vision->vob[i].d;

				if (vision->vob[i].d < 30)
					scale = 0;
				else if (vision->vob[i].d < 40)
					scale = 0.3;
				else if (vision->vob[i].d < 50)
					scale = 0.4;
				else if (vision->vob[i].d <80
						&& vision->vob[i].head> - 25
						&& vision->vob[i].head < 25)
					scale = 0.5;

				if (panx > 45) {
					// robot on left
					sideWalkLock = -1;
					sideWalkLockCount = 5;
				}
				else if (panx < -45) {
					// robot on right
					sideWalkLock = 1;
					sideWalkLockCount = 5;
				}
				else if (sideWalkInit) {
					sideWalkLockCount = 5;              // robot roughly center
					// use last direction
				}
				else if (!sideWalkInit) {
					sideWalkInit = true;
					if (panx > 10) {
						sideWalkLock = -1;
						sideWalkLockCount = 5;
					}
					else if (panx < -10) {
						sideWalkLock = 1;
						sideWalkLockCount = 5;
					}
				}


				if (vision->vob[i].d < 45) {
					// robot in front
					if (vision->vob[i].head > -20 && vision->vob[i].head < 20) {
						bool inGoalBox = false;
						if (ABS(gps->self().pos.x - goalX)
								< GOAL_WIDTH
								/ 2
								&& ABS(gps->self().pos.y
									> FIELD_LENGTH
									- GOALBOX_DEPTH))
							inGoalBox = true;

						bool inOwnGoal = false;
						if (ABS(gps->self().pos.x - goalX) < GOAL_WIDTH / 2
								&& gps->self().pos.y < WALL_THICKNESS)
							inOwnGoal = true;

						if (gps->self().pos.x <20
								|| gps->self().pos.x> FIELD_WIDTH - 20
								|| gps->self().pos.y > FIELD_LENGTH - 20
								|| inGoalBox
								|| inOwnGoal)
							sideStepLock = 0;

						if (sideStepLock == 0) {
							if (gps->self().pos.y > FIELD_LENGTH - 20) {
								bool faceUp = false;
								if (gps->self().h > 55 && gps->self().h < 125)
									faceUp = true;

								if (gps->self().pos.x < FIELD_WIDTH / 2 - GOAL_WIDTH / 2
										&& !faceUp) {
									sideStepLock = -1;
								}
								else if (gps->self().pos.x
										> FIELD_WIDTH
										- (FIELD_WIDTH / 2 + GOAL_WIDTH / 2)
										&& !faceUp) {
									sideStepLock = 1;
								}
								else {
									if (gps->self().pos.x < targetX) {
										sideStepLock = -1;
									}
									else {
										sideStepLock = 1;
									}
								}
							}
							else {
								if (gps->self().pos.x < targetX) {
									if (gps->self().pos.x > FIELD_WIDTH - 20)
										sideStepLock = 1;
									else
										sideStepLock = -1;
								}
								else {
									if (gps->self().pos.x < 20)
										sideStepLock = -1;
									else
										sideStepLock = 1;
								}
							}
						}

						//if (ABS(gps->self().pos.x-goalX)<50 && ABS(gps->self().pos.y-goalY)<50)
						//    turnCCW  = 0;

						forward = 0;
						left = sideStepLock * 3;
						lockHead = true;
					}
				}

				leds(2, 2, 2, 2, 2, 2);                        // panic light
			}
		}
	}
}



//--- trapped, get out ----------------------------------
void obstacleChallenge::obsChallengeEscapeTrap() {
	//--- got out, turn to face target again --------
	if (escapeStable) {
		double diffx = targetX - gps->self().pos.x; 
		double diffy = targetY - gps->self().pos.y;
		escapeH = RAD2DEG(atan2(diffy, diffx));
		double maxTurn = 10.0;
		double relh = NormalizeAngle_180(escapeH - (gps->self().h));

		turnCCW = CLIP(relh, maxTurn);
		forward = 0;
		left = 0;

		if (ABS(NormalizeAngle_180(gps->self().h - escapeH) < 2)) {
			escapeCount = 0;
			escapeStable = false;
			stateTimeout = 0;
			currentState = STATE_FIND_TARGET_GOAL;
			currentSubState = ACTION_ROTATE_FACE_TARGET;
			escapeX = targetX;
			escapeY = targetY;
		}
		return;
	}

	//--- reaching escape point ---------------------
	if ((ABS(gps->self().pos.x - escapeX) < 15 && ABS(gps->self().pos.y
					- escapeY) < 15)
			|| escapeCount
			<= 0) {
		escapeStable = true;
		return;
	}

	//--- escaping ----------------------------------
	int tempX = targetX;
	int tempY = targetY;
	targetX = escapeX;
	targetY = escapeY;
	obsChallengeGotoTarget();
	targetX = tempX;
	targetY = tempY;
	escapeCount--;
}




//--- look around to localise ----------------------------
void obstacleChallenge::obsChallengeLookAroundHeadParams() {
	headtype = ABS_H;
	tilty = 20;

	//-- tracking robot --------------------------
	if (infraRedTracking) {
		return;
	}


	//-- looking for gap -------------------------
	if (currentSubState == ACTION_LOOK_FOR_GAP) {
		int lastPanDirection = panDirection;

		if (lastPanX > 55.0) {
			panDirection = -1;
		}

		if (lastPanX < -55.0) {
			panDirection = +1;
		}

		lastPanX += (panDirection * 4.0);
		panx = lastPanX;

		if (panDirection != lastPanDirection) {
			gapScanCount++;
		}
		return;
	}


	//-- three way look --------------------------
	if (lockHead) {
		// doing side walk
		if (sideStepLockCount > 60) {
			// 1st look to dir of walk
			if (sideStepLock == 1) {
				panx = 90;
				lastPanX = 0;
			}
			else if (sideStepLock == -1) {
				panx = -90;
				lastPanX = 0;
			}
		}
		else if (sideStepLockCount > 45) {
			// 2nd look to opp dir of walk
			if (sideStepLock == 1) {
				panx = -90;
				lastPanX = 0;
			}
			else if (sideStepLock == -1) {
				panx = 90;
				lastPanX = 0;
			}
		}
		else {
			// look straight in front
			panx = 0;
			lastPanX = 0;
		}
		return;
	}


	//-- look straight ---------------------------
	if (fixHead) {
		panx = 0;
		return;
	}


	//-- normal pan head -------------------------
	if (lastPanX > 55.0) {
		// normal walking, pan head
		panDirection = -1;
	}

	if (lastPanX < -55.0) {
		panDirection = +1;
	}

	lastPanX += (panDirection * 8.0);
	panx = lastPanX;
}





/////////////////////////////////////////////////////////////
// Others
/////////////////////////////////////////////////////////////


//--- update world grid -----------------------------------
void obstacleChallenge::obsChallengeUpdateWorldGrid() {
	for (int i = vobRedDog; i <= vobBlueDog4; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].var < get95CF(150)) {
			//--- cal global coord of robot ---------
			double deltax = vision->vob[i].d *cos(DEG2RAD(vision->vob[i].head
						+ gps->self().h));
			double deltay = vision->vob[i].d *sin(DEG2RAD(vision->vob[i].head
						+ gps->self().h));
			double robotx = gps->self().pos.x + deltax;
			double roboty = gps->self().pos.y + deltay;

			//--- cal map coord of robot ------------
			int row = (int) (roboty / GRID_SIZE);
			if (row >= NUM_ROWS)
				row = NUM_ROWS - 1;
			else if (row < 0)
				row = 0;

			int col = (int) (robotx / GRID_SIZE);
			if (col >= NUM_COLS)
				col = NUM_COLS - 1;
			else if (col < 0)
				col = 0;

			//--- update map ------------------------
			grid[row][col] = true;
		}
	}
}



//--- clear world grid ------------------------------------
void obstacleChallenge::obsChallengeClearWorldGrid() {
	for (int r = 0; r < NUM_ROWS; r++) {
		for (int c = 0; c < NUM_COLS; c++) {
			grid[r][c] = false;
		}
	}
}



//--- print world grid ------------------------------------
void obstacleChallenge::obsChallengePrintWorldGrid() {
	for (int r = NUM_ROWS - 1; r >= 0; r--) {
		for (int c = 0; c < NUM_COLS; c++) {
			if (grid[r][c])
				cout << "X";
			else
				cout << " ";
		}
		cout << endl;
	}
	cout << "\n" << endl;
}



//--- track closest robot -------------------------------
#define FOV 10
void obstacleChallenge::obsChallengeClosestRobotTracking() {
	double closestDogDist = FIELD_LENGTH;
	int closestDog = -1;
	for (int i = vobRedDog; i <= vobBlueDog4; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].var < get95CF(150)) {
			if (vision->vob[i].d < closestDogDist) {
				closestDogDist = vision->vob[i].d;
				closestDog = i;
			}
		}
	}

	//modify head param only if robot is seen
	if (closestDog != -1) {
		tilty = vision->vob[closestDog].elev;
		panx = vision->vob[closestDog].head;

		if (abs(panx - hPan) > FOV) {
			double factor = 1.0 + ABS(panx - hPan) / 60;
			panx = hPan + (panx - hPan) * factor;
		}

		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;

		x1 = vision->vob[closestDog].d * tan(radians(vision->vob[closestDog].imgHead));
		y1 = vision->vob[closestDog].d * tan(radians(vision->vob[closestDog].imgElev));
		z1 = vision->vob[closestDog].d;

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
	}
}



//--- track robot given heading -------------------------
bool obstacleChallenge::obsChallengeRobotTracking(double head, double range) {
	double closestDogDist = FIELD_LENGTH;
	int closestDog = -1;
	for (int i = vobRedDog; i <= vobBlueDog4; i++) {
		if (vision->vob[i].cf > 0 && vision->vob[i].var < get95CF(150)) {
			if (ABS(vision->vob[i].head - head) <= range / 2
					&& vision->vob[i].d < closestDogDist) {
				closestDogDist = vision->vob[i].d;
				closestDog = i;
			}
		}
	}

	if (closestDog != -1) {
		tilty = vision->vob[closestDog].elev;
		panx = vision->vob[closestDog].head;

		if (abs(panx - hPan) > FOV) {
			double factor = 1.0 + ABS(panx - hPan) / 60;
			panx = hPan + (panx - hPan) * factor;
		}

		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;

		x1 = vision->vob[closestDog].d * tan(radians(vision->vob[closestDog].imgHead));
		y1 = vision->vob[closestDog].d * tan(radians(vision->vob[closestDog].imgElev));
		z1 = vision->vob[closestDog].d;

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

		return true;
	}
	else {
		return false;
	}
}



//--- measure robot distance given heading --------------
double obstacleChallenge::obsChallengeInfraRedRobotDist(double head,
		double range) {
	// scan for robot within range
	if (!foundRobot) {
		infraRedTrackingCount++;

		// pan head
		if (lastPanX > head + (range / 2)) {
			panDirection = -1;
		}

		if (lastPanX < head - (range / 2)) {
			panDirection = +1;
		}

		if (ABS(lastPanX - head) > (range / 2))
			lastPanX += (panDirection * 4.0);
		else
			lastPanX += (panDirection * 2.0);
		panx = lastPanX;

		// track robot
		if (obsChallengeRobotTracking(head, range)) {
			old_tilt = tilty;
			old_pan = panx;
			foundRobot = true;
			infraRedTrackingCount = 0;
		} 
		return -1;
	}
	else {
		tilty = old_tilt;
		panx = old_pan;
		infraRedTrackingCount++;

		if (infraRedTrackingCount > 5)
			return (sensors->sensorVal[ssINFRARED_FAR] / 10000.0);
		else
			return -1;
	}
}



//--- track goal ----------------------------------------
void obstacleChallenge::obsChallengeGoalTracking() {
	//modify head param only if goal is seen
	if (vision->vob[gps->targetGoal].cf > 0
			&& vision->vob[gps->targetGoal].var < get95CF(100)) {
		tilty = vision->vob[gps->targetGoal].elev;
		panx = vision->vob[gps->targetGoal].head;

		if (abs(panx - hPan) > FOV) {
			double factor = 1.0 + ABS(panx - hPan) / 60;
			panx = hPan + (panx - hPan) * factor;
		}

		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, z6;

		x1 = vision->vob[gps->targetGoal].d * tan(radians(vision->vob[gps->targetGoal].imgHead));
		y1 = vision->vob[gps->targetGoal].d * tan(radians(vision->vob[gps->targetGoal].imgElev));
		z1 = vision->vob[gps->targetGoal].d;

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
	}
}



//--- own goal ------------------------------------------
bool obstacleChallenge::obsChallengeLockInOwnGoal() {
	if (ABS(gps->self().pos.x - goalX) <= GOAL_WIDTH / 2) {
		if (gps->self().pos.y < 5
				&& ABS(NormalizeAngle_0_360(gps->self().h) - 270) <= 80)
			return true;
		if (gps->self().pos.y < 0
				&& ABS(NormalizeAngle_0_360(gps->self().h) - 270) <= 90)
			return true;
	}
	return false;
}
