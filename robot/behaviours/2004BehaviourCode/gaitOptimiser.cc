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
 * $Id: gaitOptimiser.cc 5072 2005-03-10 22:59:09Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#include "gaitOptimiser.h"
#ifndef OFFLINE
#include "../vision/Vision.h"
#endif //OFFLINE
#include "../share/OffsetDef.h"

#ifdef COMPILE_ALL_CPP
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

static double sqrarg;
#define DSQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static double maxarg1, maxarg2;
#define DMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

namespace GaitOptimiser {
	// penalty constants for when the robot doesn't cover enough ground
	// and stops prematurely
	static const double DIST_PENALISE_THRESHOLD = 550000;
	static const int DIST_PENALISE_PENALTY = 80;

	static const double TOL = 2.0e-4;
	static const double GOLD = 1.618034;
	static const double GLIMIT = 100.0;
	static const double TINY = 1.0e-6;

	static const double CGOLD = 0.3819660;
	static const double ZEPS = 1.0e-10;

	static const int settleLimit = 12;

	static const double xiUnit = 5.0;

	static const double regForward = 6.0;

	static const int reachGoalDist = 60;
	static const int reachGoalDistIR = reachGoalDist * 10000;

	static const double goalHeadingCutoff = 5.0;
	static const double goalCFCutoff = 100.0;
	static const int turnExitTimerCutoff = 15;
	static const int turnMaxPan = 30;

	static const char *logfile = "/MS/powell.log";

	typedef enum {
		initfret,
		initfretgotoown,
		initfretturntotarget,
		initfretgototarget,
		initfretturntoown,
		iterloop,
		dirloop,
		mnevala,
		mnevalagotoown,
		mnevalaturntotarget,
		mnevalagototarget,
		mnevalaturntoown,
		mnevalb,
		mnevalbgotoown,
		mnevalbturntotarget,
		mnevalbgototarget,
		mnevalbturntoown,
		mnevalc,
		mnevalcgotoown,
		mnevalcturntotarget,
		mnevalcgototarget,
		mnevalcturntoown,
		mnloop,
		mnloop1a,
		mnloop1agotoown,
		mnloop1aturntotarget,
		mnloop1agototarget,
		mnloop1aturntoown,
		mnloop1b,
		mnloop1bgotoown,
		mnloop1bturntotarget,
		mnloop1bgototarget,
		mnloop1bturntoown,
		mnloop2a,
		mnloop2agotoown,
		mnloop2aturntotarget,
		mnloop2agototarget,
		mnloop2aturntoown,
		mnloop2b,
		mnloop2bgotoown,
		mnloop2bturntotarget,
		mnloop2bgototarget,
		mnloop2bturntoown,
		mnloop3,
		mnloop3gotoown,
		mnloop3turntotarget,
		mnloop3gototarget,
		mnloop3turntoown,
		mnloop4,
		mnloop4gotoown,
		mnloop4turntotarget,
		mnloop4gototarget,
		mnloop4turntoown,
		mnloopshift,
		initbrent,
		initbrentgotoown,
		initbrentturntotarget,
		initbrentgototarget,
		initbrentturntoown,
		brentloop,
		brentloopgotoown,
		brentloopturntotarget,
		brentloopgototarget,
		brentloopturntoown,
		brentloopupdate,
		dirloopupdate,
		extrapolate,
		extrapolategotoown,
		extrapolateturntotarget,
		extrapolategototarget,
		extrapolateturntoown,
		extrapolateupdate,
		emnevala,
		emnevalagotoown,
		emnevalaturntotarget,
		emnevalagototarget,
		emnevalaturntoown,
		emnevalb,
		emnevalbgotoown,
		emnevalbturntotarget,
		emnevalbgototarget,
		emnevalbturntoown,
		emnevalc,
		emnevalcgotoown,
		emnevalcturntotarget,
		emnevalcgototarget,
		emnevalcturntoown,
		emnloop,
		emnloop1a,
		emnloop1agotoown,
		emnloop1aturntotarget,
		emnloop1agototarget,
		emnloop1aturntoown,
		emnloop1b,
		emnloop1bgotoown,
		emnloop1bturntotarget,
		emnloop1bgototarget,
		emnloop1bturntoown,
		emnloop2a,
		emnloop2agotoown,
		emnloop2aturntotarget,
		emnloop2agototarget,
		emnloop2aturntoown,
		emnloop2b,
		emnloop2bgotoown,
		emnloop2bturntotarget,
		emnloop2bgototarget,
		emnloop2bturntoown,
		emnloop3,
		emnloop3gotoown,
		emnloop3turntotarget,
		emnloop3gototarget,
		emnloop3turntoown,
		emnloop4,
		emnloop4gotoown,
		emnloop4turntotarget,
		emnloop4gototarget,
		emnloop4turntoown,
		emnloopshift,
		einitbrent,
		einitbrentgotoown,
		einitbrentturntotarget,
		einitbrentgototarget,
		einitbrentturntoown,
		ebrentloop,
		ebrentloopgotoown,
		ebrentloopturntotarget,
		ebrentloopgototarget,
		ebrentloopturntoown,
		ebrentloopupdate,
		elinminupdate,
		settleAndVerify
	} LearnerStates;

	static const int OFFSET_LENGTH = 24;
	static const int AVG_RUNS = 4;

	OffsetParams nextOP;

	VisualObject *goalAvob;
	VisualObject *goalBvob;

	double goalADir = 180;
	double goalBDir = 0;

	// powell function variables
	double p[OFFSET_LENGTH];
	double xi[OFFSET_LENGTH][OFFSET_LENGTH];
	double fret = 0;

	int poi = 0, poibig = 0, poj = 0;
	double podel = 0, pofp = 0, pofptt = 0, pot = 0;
	double popt[OFFSET_LENGTH], poptt[OFFSET_LENGTH], poxit[OFFSET_LENGTH];

	// linmin function variables
	int lij = 0, lincom = 0;
	double lixx = 0,
	lixmin = 0,
	lifx = 0,
	lifb = 0,
	lifa = 0,
	libx = 0,
	liax = 0;
	double lipcom[OFFSET_LENGTH], lixicom[OFFSET_LENGTH], lixt[OFFSET_LENGTH];

	// mnbrak function variables
	double mnulim = 0, mnu = 0, mnr = 0, mnq = 0, mnfu = 0, mndum = 0;

	// brent function variables
	int briter = 0;
	double bra = 0,
	brb = 0,
	brd = 0,
	bretemp = 0,
	brfu = 0,
	brfv = 0,
	brfw = 0,
	brfx = 0,
	brp = 0,
	brq = 0,
	brr = 0,
	brtol1 = 0,
	brtol2 = 0,
	bru = 0,
	brv = 0,
	brw = 0,
	brx = 0,
	brxm = 0;
	double bre = 0.0;

	int state = initfret;
	int stateAfterSettle = initfret;
	double *writeAfterSettle;
	int savedTimer;
	int timer = 0;
	int exittimer = 0;

	double current[OFFSET_LENGTH];

	int avgCount = 0;
	int avgLog[AVG_RUNS];

	void headVobTracking(VisualObject *myVob);

	bool nearEndOfRunA();
	bool nearEndOfRunB();
	bool haveTurnedToFaceBallA();
	bool haveTurnedToFaceBallB();
	void turnToFaceGoalA();
	void turnToFaceGoalB();
	void walkToGoalA(double p[OFFSET_LENGTH]);
	void walkToGoalB(double p[OFFSET_LENGTH]);
	void nextState();
	void execute();
	void modifyOffsets();
	void printState();
	void dumpState();
	void undumpState();
	double filterAvg();
}

void GaitOptimiser::initGaitOptimiser() {
	for (int i = 0; i < OFFSET_LENGTH; ++i) {
		p[i] = popt[i] = poptt[i] = poxit[i] = lipcom[i] = lixicom[i] = lixt[i] = current[i] = 0;
	}
	for (int i = 0; i < OFFSET_LENGTH; ++i) {
		for (int j = 0; j < OFFSET_LENGTH; ++j) {
			xi[i][j] = 0;
		}
	}

	for (int i = 0; i < AVG_RUNS; ++i) {
		avgLog[i] = 0;
	}

	timer = 0;
	exittimer = 0;

	setStandParams();
	tilty = 15.0;

	undumpState();
}

void GaitOptimiser::doGaitOptimiser() {
	batteryLEDS();

	nextState();
	execute();

	// Why not make current of type OffsetParams? 
	// Troublesome, need to change a lot.
	memcpy(&nextOP, &current, sizeof(OffsetParams));
#ifndef OFFLINE
	transmission->sendOP(nextOP);
#endif //OFFLINE

	headtype = ABS_H;
}

void GaitOptimiser::headVobTracking(VisualObject *myVob) {
	double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, z5;

	x1 = myVob->d * tan(radians(myVob->imgHead));
	y1 = myVob->d * tan(radians(myVob->imgElev));
	z1 = myVob->d;

	x2 = x1;
	y2 = y1 + NECK_LENGTH2;
	z2 = z1 + FACE_LENGTH2;

	x3 = x2 * cos(radians(-hPan)) - z2 * sin(radians(-hPan));
	y3 = y2;
	z3 = x2 * sin(radians(-hPan)) + z2 * cos(radians(-hPan));

	x4 = x3;
	y4 = z3 * sin(radians(hTilt)) + y3 * cos(radians(hTilt));
	z4 = z3 * cos(radians(hTilt)) - y3 * sin(radians(hTilt));

	if (z4 < NECK_LENGTH) {
		z4 = NECK_LENGTH;
	}

	headtype = ABS_H;
	tilty = degrees(atan(y4 / z4) - asin(NECK_LENGTH / sqrt(z4 * z4 + y4 * y4)));

	x5 = x4;
	z5 = z4 * cos(radians(-tilty)) - y4 * sin(radians(-tilty));

	panx = degrees(atan(x5 / z5));

	// need this bit?
	tilty = hTilt + (tilty - hTilt) * 0.6;
	panx = hPan + (panx - hPan) * 0.8;
}

bool GaitOptimiser::nearEndOfRunA() {
	return (goalAvob->cf > goalCFCutoff && goalAvob->d < reachGoalDist);
}

bool GaitOptimiser::nearEndOfRunB() {
	return (goalBvob->cf > goalCFCutoff && goalBvob->d < reachGoalDist);
}

bool GaitOptimiser::haveTurnedToFaceBallA() {
	return (goalBvob->cf > goalCFCutoff && ABS(goalBvob->h) < goalHeadingCutoff);
}

bool GaitOptimiser::haveTurnedToFaceBallB() {
	return (goalAvob->cf > goalCFCutoff && ABS(goalAvob->h) < goalHeadingCutoff);
}

void GaitOptimiser::turnToFaceGoalA() {
	setCanterParams();
	forward = Behaviours::left = 0.0;

	if (ABS(NormalizeAngle_180(goalBDir - gps->self().h)) < 90) {
		turnCCW = 30.0;
		panx = 0.0;
		tilty = 10.0;
	}
	else if (goalAvob->cf == 0) {
		turnCCW = CLIP(NormalizeAngle_180(goalADir - gps->self().h) / 2, 30.0);
	}
	else {
		if (goalAvob->h < (goalHeadingCutoff + 5)) {
			goalADir = gps->self().h;
		}
		turnCCW = CLIP(goalAvob->h / 2.0, 30.0);
		headVobTracking(goalAvob);
	}
}

void GaitOptimiser::turnToFaceGoalB() {
	setCanterParams();
	forward = Behaviours::left = 0.0;

	if (ABS(NormalizeAngle_180(goalADir - gps->self().h)) < 90) {
		turnCCW = 30.0;
		panx = 0.0;
		tilty = 10.0;
	}
	else if (goalBvob->cf == 0) {
		turnCCW = CLIP(NormalizeAngle_180(goalBDir - gps->self().h) / 2, 30.0);
	}
	else {
		if (goalBvob->h < (goalHeadingCutoff + 5)) {
			goalBDir = gps->self().h;
		}
		turnCCW = CLIP(goalBvob->h / 2.0, 30.0);
		// panx = ABS(vtGoal.h) < turnMaxPan ? vtGoal.h :
		//    (vtGoal.h > turnMaxPan ? turnMaxPan : -turnMaxPan);
		headVobTracking(goalBvob);
	}
}

void GaitOptimiser::walkToGoalA(double p[OFFSET_LENGTH]) {
	for (int i = 0; i < OFFSET_LENGTH; ++i) {
		current[i] = p[i];
	}
	setOffsetParams();
	forward = regForward;
	Behaviours::left = 0.0;
	turnCCW = CLIP(goalAvob->h / 2.0, 10.0);
	headVobTracking(goalAvob);
}

void GaitOptimiser::walkToGoalB(double p[OFFSET_LENGTH]) {
	for (int i = 0; i < OFFSET_LENGTH; ++i) {
		current[i] = p[i];
	}
	setOffsetParams();
	forward = regForward;
	Behaviours::left = 0.0;
	turnCCW = CLIP(goalBvob->h / 2.0, 10.0);
	headVobTracking(goalBvob);
}

void GaitOptimiser::nextState() {
	timer++;

	goalAvob = &(vision->vob[vobGreenLeftBeacon]);
	goalBvob = &(vision->vob[vobGreenRightBeacon]);

	if (state == initfret) {
		if (vtGoal.cf > goalCFCutoff)
			state = initfretgototarget;
		else
			state = initfretgotoown;
		timer = 0;
	}
	else if (state == initfretgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save first point
			writeAfterSettle = &fret;
			stateAfterSettle = initfretturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == initfretturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = iterloop;
				avgCount = 0;
			}
			else {
				state = initfretgototarget;
			}
			timer = 0;
		}
	}
	else if (state == initfretgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save first point
			writeAfterSettle = &fret;
			stateAfterSettle = initfretturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == initfretturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = iterloop;
				avgCount = 0;
			}
			else {
				state = initfretgotoown;
			}
			timer = 0;
		}
	}
	else if (state == iterloop) {
		state = dirloop;
		timer = 0;
	}
	else if (state == dirloop) {
		state = mnevala;
		timer = 0;
	}
	else if (state == mnevala) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnevalagototarget;
		else
			state = mnevalagotoown;
		timer = 0;
	}
	else if (state == mnevalagotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fa
			writeAfterSettle = &lifa;
			stateAfterSettle = mnevalaturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnevalaturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnevalb;
				avgCount = 0;
			}
			else {
				state = mnevalagototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnevalagototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fa
			writeAfterSettle = &lifa;
			stateAfterSettle = mnevalaturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnevalaturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnevalb;
				avgCount = 0;
			}
			else {
				state = mnevalagotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnevalb) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnevalbgototarget;
		else
			state = mnevalbgotoown;
		timer = 0;
	}
	else if (state == mnevalbgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fx
			writeAfterSettle = &lifx;
			stateAfterSettle = mnevalbturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnevalbturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnevalc;
				avgCount = 0;
			}
			else {
				state = mnevalbgototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnevalbgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fx
			writeAfterSettle = &lifx;
			stateAfterSettle = mnevalbturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnevalbturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnevalc;
				avgCount = 0;
			}
			else {
				state = mnevalbgotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnevalc) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnevalcgototarget;
		else
			state = mnevalcgotoown;
		timer = 0;
	}
	else if (state == mnevalcgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fb
			writeAfterSettle = &lifb;
			stateAfterSettle = mnevalcturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnevalcturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloop;
				avgCount = 0;
			}
			else {
				state = mnevalcgototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnevalcgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fb
			writeAfterSettle = &lifb;
			stateAfterSettle = mnevalcturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnevalcturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloop;
				avgCount = 0;
			}
			else {
				state = mnevalcgotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloop) {
		if (lifx > lifb) {
			mnr = (lixx - liax) * (lifx - lifb);
			mnq = (lixx - libx) * (lifx - lifa);
			mnu = (lixx)
				- ((lixx - libx) * mnq - (lixx - liax) * mnr)
				/ (2.0 * SIGN(DMAX(fabs(mnq - mnr), TINY), mnq - mnr));
			mnulim = (lixx) + GLIMIT * (libx - lixx);
			if ((lixx - mnu) * (mnu - libx) > 0.0) {
				state = mnloop1a;
			}
			else if ((libx - mnu) * (mnu - mnulim) > 0.0) {
				state = mnloop2a;
			}
			else if ((mnu - mnulim) * (mnulim - libx) >= 0.0) {
				state = mnloop3;
			}
			else {
				state = mnloop4;
			}
		}
		else {
			state = initbrent;
		}
		timer = 0;
	}
	else if (state == mnloop1a) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnloop1agototarget;
		else
			state = mnloop1agotoown;
		timer = 0;
	}
	else if (state == mnloop1agotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop1aturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop1aturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloop1b;
				avgCount = 0;
			}
			else {
				state = mnloop1agototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnloop1agototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop1aturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop1aturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloop1b;
				avgCount = 0;
			}
			else {
				state = mnloop1agotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloop1b) {
		if (mnfu < lifb) {
			liax = lixx;
			lixx = mnu;
			lifa = lifx;
			lifx = mnfu;
			state = initbrent;
		}
		else if (mnfu > lifx) {
			libx = mnu;
			lifb = mnfu;
			state = initbrent;
		}
		else {
			mnu = (libx) + GOLD * (libx - lixx);
			for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
				lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
			}

			if (vtGoal.cf > goalCFCutoff)
				state = mnloop1bgototarget;
			else
				state = mnloop1bgotoown;
		}
		timer = 0;
	}
	else if (state == mnloop1bgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop1bturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop1bturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop1bgototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnloop1bgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop1bturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop1bturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop1bgotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloop2a) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnloop2agototarget;
		else
			state = mnloop2agotoown;
		timer = 0;
	}
	else if (state == mnloop2agotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop2aturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop2aturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloop2b;
				avgCount = 0;
			}
			else {
				state = mnloop2agototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnloop2agototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop2aturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop2aturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloop2b;
				avgCount = 0;
			}
			else {
				state = mnloop2agotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloop2b) {
		if (mnfu < lifb) {
			SHFT(lixx, libx, mnu, libx + GOLD * (libx - lixx))
				lifx = lifb;
			lifb = mnfu;

			for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
				lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
			}

			if (vtGoal.cf > goalCFCutoff)
				state = mnloop2bgototarget;
			else
				state = mnloop2bgotoown;
		}
		else {
			state = mnloopshift;
		}
		timer = 0;
	}
	else if (state == mnloop2bgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop2bturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop2bturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop2bgototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnloop2bgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop2bturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop2bturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop2bgotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloop3) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnloop3gototarget;
		else
			state = mnloop3gotoown;
		timer = 0;
	}
	else if (state == mnloop3gotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop3turntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop3turntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop3gototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnloop3gototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop3turntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop3turntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop3gotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloop4) {
		if (vtGoal.cf > goalCFCutoff)
			state = mnloop4gototarget;
		else
			state = mnloop4gotoown;
		timer = 0;
	}
	else if (state == mnloop4gotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop4turntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop4turntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop4gototarget;
			}
			timer = 0;
		}
	}
	else if (state == mnloop4gototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = mnloop4turntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == mnloop4turntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = mnloopshift;
				avgCount = 0;
			}
			else {
				state = mnloop4gotoown;
			}
			timer = 0;
		}
	}
	else if (state == mnloopshift) {
		state = mnloop;
		timer = 0;
	}
	else if (state == initbrent) {
		if (vtGoal.cf > goalCFCutoff)
			state = initbrentgototarget;
		else
			state = initbrentgotoown;
		timer = 0;
	}
	else if (state == initbrentgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fw,fv,fx
			writeAfterSettle = &brfw;
			stateAfterSettle = initbrentturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == initbrentturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = brentloop;
				avgCount = 0;
			}
			else {
				state = initbrentgototarget;
			}
			timer = 0;
		}
	}
	else if (state == initbrentgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fw,fv,fx
			writeAfterSettle = &brfw;
			stateAfterSettle = initbrentturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == initbrentturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = brentloop;
				avgCount = 0;
			}
			else {
				state = initbrentgotoown;
			}
			timer = 0;
		}
	}
	else if (state == brentloop) {
		brxm = 0.5 * (bra + brb);
		brtol2 = 2.0 * (brtol1 = TOL * fabs(brx) + ZEPS);
		if (fabs(brx - brxm) <= (brtol2 - 0.5 * (brb - bra))) {
			lixmin = brx;
			fret = brfx;
			state = dirloopupdate;
		}
		else {
			if (fabs(bre) > brtol1) {
				brr = (brx - brw) * (brfx - brfv);
				brq = (brx - brv) * (brfx - brfw);
				brp = (brx - brv) * brq - (brx - brw) * brr;
				brq = 2.0 * (brq - brr);
				if (brq > 0.0) {
					brp = -brp;
				}
				brq = fabs(brq);
				bretemp = bre;
				bre = brd;
				if (fabs(brp) >= fabs(0.5 * brq * bretemp)
						|| brp <= brq * (bra - brx)
						|| brp >= brq * (brb - brx)) {
					brd = CGOLD * (bre = (brx >= brxm ? bra - brx : brb - brx));
				}
				else {
					brd = brp / brq;
					bru = brx + brd;
					if (bru - bra < brtol2 || brb - bru < brtol2) {
						brd = SIGN(brtol1, brxm - brx);
					}
				}
			}
			else {
				brd = CGOLD * (bre = (brx >= brxm ? bra - brx : brb - brx));
			}
			bru = (fabs(brd) >= brtol1 ? brx + brd : brx + SIGN(brtol1, brd));

			for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
				lixt[lij] = lipcom[lij] + bru * lixicom[lij];
			}

			if (vtGoal.cf > goalCFCutoff)
				state = brentloopgototarget;
			else
				state = brentloopgotoown;
		}
		timer = 0;
	}
	else if (state == brentloopgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &brfu;
			stateAfterSettle = brentloopturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == brentloopturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = brentloopupdate;
				avgCount = 0;
			}
			else {
				state = brentloopgototarget;
			}
			timer = 0;
		}
	}
	else if (state == brentloopgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &brfu;
			stateAfterSettle = brentloopturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == brentloopturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = brentloopupdate;
				avgCount = 0;
			}
			else {
				state = brentloopgotoown;
			}
			timer = 0;
		}
	}
	else if (state == brentloopupdate) {
		state = brentloop;
		timer = 0;
	}
	else if (state == dirloopupdate) {
		if (poi < OFFSET_LENGTH) {
			state = dirloop;
		}
		else {
			state = extrapolate;
		}
		timer = 0;
	}
	else if (state == extrapolate) {
		if (vtGoal.cf > goalCFCutoff)
			state = extrapolategototarget;
		else
			state = extrapolategotoown;
		timer = 0;
	}
	else if (state == extrapolategotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save pofptt
			writeAfterSettle = &pofptt;
			stateAfterSettle = extrapolateturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == extrapolateturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = extrapolateupdate;
				avgCount = 0;
			}
			else {
				state = extrapolategototarget;
			}
			timer = 0;
		}
	}
	else if (state == extrapolategototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save pofptt
			writeAfterSettle = &pofptt;
			stateAfterSettle = extrapolateturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == extrapolateturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = extrapolateupdate;
				avgCount = 0;
			}
			else {
				state = extrapolategotoown;
			}
			timer = 0;
		}
	}
	else if (state == extrapolateupdate) {
		if (pofptt < pofp) {
			pot = 2.0 * (pofp - 2.0 * fret + pofptt) * DSQR(pofp - fret - podel)
				- podel * DSQR(pofp
						- pofptt);
			if (pot < 0.0) {
				state = emnevala;
			}
			else {
				state = iterloop;
			}
		}
		else {
			state = iterloop;
		}
		timer = 0;
	}
	else if (state == emnevala) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnevalagototarget;
		else
			state = emnevalagotoown;
		timer = 0;
	}
	else if (state == emnevalagotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fa
			writeAfterSettle = &lifa;
			stateAfterSettle = emnevalaturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnevalaturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnevalb;
				avgCount = 0;
			}
			else {
				state = emnevalagototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnevalagototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fa
			writeAfterSettle = &lifa;
			stateAfterSettle = emnevalaturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnevalaturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnevalb;
				avgCount = 0;
			}
			else {
				state = emnevalagotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnevalb) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnevalbgototarget;
		else
			state = emnevalbgotoown;
		timer = 0;
	}
	else if (state == emnevalbgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fx
			writeAfterSettle = &lifx;
			stateAfterSettle = emnevalbturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnevalbturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnevalc;
				avgCount = 0;
			}
			else {
				state = emnevalbgototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnevalbgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fx
			writeAfterSettle = &lifx;
			stateAfterSettle = emnevalbturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnevalbturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnevalc;
				avgCount = 0;
			}
			else {
				state = emnevalbgotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnevalc) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnevalcgototarget;
		else
			state = emnevalcgotoown;
		timer = 0;
	}
	else if (state == emnevalcgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fb
			writeAfterSettle = &lifb;
			stateAfterSettle = emnevalcturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnevalcturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloop;
				avgCount = 0;
			}
			else {
				state = emnevalcgototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnevalcgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fb
			writeAfterSettle = &lifb;
			stateAfterSettle = emnevalcturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnevalcturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloop;
				avgCount = 0;
			}
			else {
				state = emnevalcgotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloop) {
		if (lifx > lifb) {
			mnr = (lixx - liax) * (lifx - lifb);
			mnq = (lixx - libx) * (lifx - lifa);
			mnu = (lixx)
				- ((lixx - libx) * mnq - (lixx - liax) * mnr)
				/ (2.0 * SIGN(DMAX(fabs(mnq - mnr), TINY), mnq - mnr));
			mnulim = (lixx) + GLIMIT * (libx - lixx);
			if ((lixx - mnu) * (mnu - libx) > 0.0) {
				state = emnloop1a;
			}
			else if ((libx - mnu) * (mnu - mnulim) > 0.0) {
				state = emnloop2a;
			}
			else if ((mnu - mnulim) * (mnulim - libx) >= 0.0) {
				state = emnloop3;
			}
			else {
				state = emnloop4;
			}
		}
		else {
			state = einitbrent;
		}
		timer = 0;
	}
	else if (state == emnloop1a) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnloop1agototarget;
		else
			state = emnloop1agotoown;
		timer = 0;
	}
	else if (state == emnloop1agotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop1aturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop1aturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloop1b;
				avgCount = 0;
			}
			else {
				state = emnloop1agototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnloop1agototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop1aturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop1aturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloop1b;
				avgCount = 0;
			}
			else {
				state = emnloop1agotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloop1b) {
		if (mnfu > lifb) {
			liax = lixx;
			lixx = mnu;
			lifa = lifx;
			lifx = mnfu;
			state = einitbrent;
		}
		else if (mnfu > lifx) {
			libx = mnu;
			lifb = mnfu;
			state = einitbrent;
		}
		else {
			mnu = (libx) + GOLD * (libx - lixx);
			for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
				lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
			}
			if (vtGoal.cf > goalCFCutoff)
				state = emnloop1bgototarget;
			else
				state = emnloop1bgotoown;
		}
		timer = 0;
	}
	else if (state == emnloop1bgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop1bturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop1bturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop1bgototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnloop1bgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop1bturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop1bturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop1bgotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloop2a) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnloop2agototarget;
		else
			state = emnloop2agotoown;
		timer = 0;
	}
	else if (state == emnloop2agotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop2aturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop2aturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloop2b;
				avgCount = 0;
			}
			else {
				state = emnloop2agototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnloop2agototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop2aturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop2aturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloop2b;
				avgCount = 0;
			}
			else {
				state = emnloop2agotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloop2b) {
		if (mnfu < lifb) {
			SHFT(lixx, libx, mnu, libx + GOLD * (libx - lixx))
				lifx = lifb;
			lifb = mnfu;
			for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
				lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
			}
			if (vtGoal.cf > goalCFCutoff)
				state = emnloop2bgototarget;
			else
				state = emnloop2bgotoown;
		}
		else {
			state = emnloopshift;
		}
		timer = 0;
	}
	else if (state == emnloop2bgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop2bturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop2bturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop2bgototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnloop2bgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop2bturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop2bturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop2bgotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloop3) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnloop3gototarget;
		else
			state = emnloop3gotoown;
		timer = 0;
	}
	else if (state == emnloop3gotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop3turntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop3turntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop3gototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnloop3gototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop3turntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop3turntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop3gotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloop4) {
		if (vtGoal.cf > goalCFCutoff)
			state = emnloop4gototarget;
		else
			state = emnloop4gotoown;
		timer = 0;
	}
	else if (state == emnloop4gotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop4turntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop4turntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop4gototarget;
			}
			timer = 0;
		}
	}
	else if (state == emnloop4gototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &mnfu;
			stateAfterSettle = emnloop4turntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == emnloop4turntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = emnloopshift;
				avgCount = 0;
			}
			else {
				state = emnloop4gotoown;
			}
			timer = 0;
		}
	}
	else if (state == emnloopshift) {
		state = emnloop;
		timer = 0;
	}
	else if (state == einitbrent) {
		if (vtGoal.cf > goalCFCutoff)
			state = einitbrentgototarget;
		else
			state = einitbrentgotoown;
		timer = 0;
	}
	else if (state == einitbrentgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fw,fv,fx
			writeAfterSettle = &brfw;
			stateAfterSettle = einitbrentturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == einitbrentturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = ebrentloop;
				avgCount = 0;
			}
			else {
				state = einitbrentgototarget;
			}
			timer = 0;
		}
	}
	else if (state == einitbrentgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fw,fv,fx
			writeAfterSettle = &brfw;
			stateAfterSettle = einitbrentturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == einitbrentturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = ebrentloop;
				avgCount = 0;
			}
			else {
				state = einitbrentgotoown;
			}
			timer = 0;
		}
	}
	else if (state == ebrentloop) {
		brxm = 0.5 * (bra + brb);
		brtol2 = 2.0 * (brtol1 = TOL * fabs(brx) + ZEPS);
		if (fabs(brx - brxm) <= (brtol2 - 0.5 * (brb - bra))) {
			lixmin = brx;
			fret = brfx;
			state = elinminupdate;
		}
		else {
			if (fabs(bre) > brtol1) {
				brr = (brx - brw) * (brfx - brfv);
				brq = (brx - brv) * (brfx - brfw);
				brp = (brx - brv) * brq - (brx - brw) * brr;
				brq = 2.0 * (brq - brr);
				if (brq > 0.0) {
					brp = -brp;
				}
				brq = fabs(brq);
				bretemp = bre;
				bre = brd;
				if (fabs(brp) >= fabs(0.5 * brq * bretemp)
						|| brp <= brq * (bra - brx)
						|| brp >= brq * (brb - brx)) {
					brd = CGOLD * (bre = (brx >= brxm ? bra - brx : brb - brx));
				}
				else {
					brd = brp / brq;
					bru = brx + brd;
					if (bru - bra < brtol2 || brb - bru < brtol2) {
						brd = SIGN(brtol1, brxm - brx);
					}
				}
			}
			else {
				brd = CGOLD * (bre = (brx >= brxm ? bra - brx : brb - brx));
			}
			bru = (fabs(brd) >= brtol1 ? brx + brd : brx + SIGN(brtol1, brd));
			for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
				lixt[lij] = lipcom[lij] + bru * lixicom[lij];
			}

			if (vtGoal.cf > goalCFCutoff)
				state = ebrentloopgototarget;
			else
				state = ebrentloopgotoown;
		}
		timer = 0;
	}
	else if (state == ebrentloopgotoown) {
		if (nearEndOfRunA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &brfu;
			stateAfterSettle = ebrentloopturntotarget;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == ebrentloopturntotarget) {
		if (haveTurnedToFaceBallA()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = ebrentloopupdate;
				avgCount = 0;
			}
			else {
				state = ebrentloopgototarget;
			}
			timer = 0;
		}
	}
	else if (state == ebrentloopgototarget) {
		if (nearEndOfRunB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > 10) {
			// save fu
			writeAfterSettle = &brfu;
			stateAfterSettle = ebrentloopturntoown;
			state = settleAndVerify;
			savedTimer = timer;
			exittimer = 0;

			cout << "Last: ";
			for (int i = 0; i < OFFSET_LENGTH; ++i) {
				cout << current[i] << " ";
			}
			cout << "; time = " << timer << endl;

			if (avgCount == AVG_RUNS) {
				cout << "Filtered value of last 4 runs = " << filterAvg() << endl;
			}

			timer = 0;
		}
	}
	else if (state == ebrentloopturntoown) {
		if (haveTurnedToFaceBallB()) {
			exittimer++;
		}
		else {
			exittimer = 0;
		}

		if (exittimer > turnExitTimerCutoff) {
			if (avgCount == AVG_RUNS) {
				state = ebrentloopupdate;
				avgCount = 0;
			}
			else {
				state = ebrentloopgotoown;
			}
			timer = 0;
		}
	}
	else if (state == ebrentloopupdate) {
		state = ebrentloop;
		timer = 0;
	}
	else if (state == elinminupdate) {
		state = iterloop;
		timer = 0;
	}
	else if (state == settleAndVerify) {
		if (goalBvob->cf > 100)
			headVobTracking(goalBvob);
		else
			headVobTracking(goalAvob);
		if (timer > settleLimit) {
			// check if IR reading ok after settling
			if (sensors->sensorVal[ssINFRARED_FAR] > DIST_PENALISE_THRESHOLD) {
				savedTimer = savedTimer + DIST_PENALISE_PENALTY;
				cout << "Penalised last run for stopping short." << endl;
			}

			// then write the saved timer... if writing to brfw, write
			// to brfv and brfx (it's a particular state case)
			avgLog[avgCount++] = savedTimer;
			if (avgCount == AVG_RUNS) {
				if (writeAfterSettle == &brfw) {
					brfw = brfv = brfx = filterAvg();
				}
				else if (writeAfterSettle == &fret) {
					fret = filterAvg();
					for (poj = 0; poj < OFFSET_LENGTH; ++poj) {
						popt[poj] = p[poj];
					}
				}
				else {
					*writeAfterSettle = filterAvg();
				}
			}

			state = stateAfterSettle;
			timer = 0;
		}
	}
}

void GaitOptimiser::execute() {
	if (state == initfret) {
		setStandParams();
		headtype = ABS_H;
		panx = 0;
		// panx = tilty = 0;
	}
	else if (state == initfretgotoown) {
		walkToGoalA(p);
	}
	else if (state == initfretturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == initfretgototarget) {
		walkToGoalB(p);
	}
	else if (state == initfretturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == iterloop) {
		pofp = fret;
		poibig = 0;
		podel = 0.0;
		poi = 0;
		setStandParams();
	}
	else if (state == dirloop) {
		for (poj = 0; poj < OFFSET_LENGTH; ++poj) {
			poxit[poj] = xi[poj][poi];
		}
		pofptt = fret;
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lipcom[lij] = p[lij];
			lixicom[lij] = poxit[lij];
		}
		liax = 0.0;
		lixx = 1.0;
		setStandParams();
		printState();
		dumpState();
	}
	else if (state == mnevala) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + liax * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == mnevalagotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnevalaturntotarget) {
		turnToFaceGoalB();
		//        tilty = 15.0;
	}
	else if (state == mnevalagototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnevalaturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnevalb) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + lixx * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == mnevalbgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnevalbturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnevalbgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnevalbturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnevalc) {
		if (lifx > lifa) {
			SHFT(mndum, liax, lixx, mndum)
				SHFT(mndum, lifx, lifa, mndum)
		}
		libx = lixx + GOLD * (lixx - liax);
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + libx * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == mnevalcgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnevalcturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnevalcgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnevalcturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloop) {
		// nothing to do; calculations done in nextState
		setStandParams();
	}
	else if (state == mnloop1a) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == mnloop1agotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnloop1aturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnloop1agototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnloop1aturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloop1b) {
		setStandParams();
	}
	else if (state == mnloop1bgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnloop1bturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnloop1bgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnloop1bturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloop2a) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == mnloop2agotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnloop2aturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnloop2agototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnloop2aturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloop2b) {
		setStandParams();
	}
	else if (state == mnloop2bgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnloop2bturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnloop2bgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnloop2bturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloop3) {
		mnu = mnulim;
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == mnloop3gotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnloop3turntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnloop3gototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnloop3turntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloop4) {
		mnu = libx + GOLD * (libx - lixx);
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
	}
	else if (state == mnloop4gotoown) {
		walkToGoalA(lixt);
	}
	else if (state == mnloop4turntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == mnloop4gototarget) {
		walkToGoalB(lixt);
	}
	else if (state == mnloop4turntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == mnloopshift) {
		setStandParams();
		SHFT(liax, lixx, libx, mnu)
			SHFT(lifa, lifx, lifb, mnfu)
	}
	else if (state == initbrent) {
		bra = (liax < libx ? liax : libx);
		brb = (liax > libx ? liax : libx);
		brx = brw = brv = lixx;
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + brx * lixicom[lij];
		}
	}
	else if (state == initbrentgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == initbrentturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == initbrentgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == initbrentturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == brentloop) {
		setStandParams();
	}
	else if (state == brentloopgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == brentloopturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == brentloopgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == brentloopturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == brentloopupdate) {
		if (brfu <= brfx) {
			if (bru >= brx) {
				bra = brx;
			}
			else {
				brb = brx;
			}
			SHFT(brv, brw, brx, bru)
				SHFT(brfv, brfw, brfx, brfu)
		}
		else {
			if (bru < brx) {
				bra = bru;
			}
			else {
				brb = bru;
			}

			if (brfu <= brfw || brw == brx) {
				brv = brw;
				brw = bru;
				brfv = brfw;
				brfw = brfu;
			}
			else if (brfu <= brfv || brv == brx || brv == brw) {
				brv = bru;
				brfv = brfu;
			}
		}
	}
	else if (state == dirloopupdate) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			poxit[lij] *= lixmin;
			p[lij] += poxit[lij];
		}
		if (pofptt - fret > podel) {
			podel = pofptt - fret;
			poibig = poi;
		}
		++poi;
		printState();
		dumpState();
	}
	else if (state == extrapolate) {
		for (poj = 0; poj < OFFSET_LENGTH; ++poj) {
			poptt[poj] = 2.0 * p[poj] - popt[poj];
			poxit[poj] = p[poj] - popt[poj];
			popt[poj] = p[poj];
		}
	}
	else if (state == extrapolategotoown) {
		walkToGoalA(poptt);
	}
	else if (state == extrapolateturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == extrapolategototarget) {
		walkToGoalB(poptt);
	}
	else if (state == extrapolateturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == extrapolateupdate) {
		// nothing to do!
		setStandParams();
	}
	else if (state == emnevala) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lipcom[lij] = p[lij];
			lixicom[lij] = poxit[lij];
		}
		liax = 0.0;
		lixx = 1.0;
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + liax * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == emnevalagotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnevalaturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnevalagototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnevalaturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnevalb) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + lixx * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == emnevalbgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnevalbturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnevalbgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnevalbturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnevalc) {
		if (lifx > lifa) {
			SHFT(mndum, liax, lixx, mndum)
				SHFT(mndum, lifx, lifa, mndum)
		}
		libx = lixx + GOLD * (lixx - liax);
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + libx * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == emnevalcgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnevalcturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnevalcgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnevalcturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloop) {
		// nothing to do; calculations done in nextState
		setStandParams();
	}
	else if (state == emnloop1a) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == emnloop1agotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnloop1aturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnloop1agototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnloop1aturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloop1b) {
		setStandParams();
	}
	else if (state == emnloop1bgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnloop1bturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnloop1bgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnloop1bturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloop2a) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == emnloop2agotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnloop2aturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnloop2agototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnloop2aturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloop2b) {
		setStandParams();
	}
	else if (state == emnloop2bgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnloop2bturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnloop2bgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnloop2bturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloop3) {
		mnu = mnulim;
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
		setStandParams();
	}
	else if (state == emnloop3gotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnloop3turntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnloop3gototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnloop3turntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloop4) {
		mnu = libx + GOLD * (libx - lixx);
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + mnu * lixicom[lij];
		}
	}
	else if (state == emnloop4gotoown) {
		walkToGoalA(lixt);
	}
	else if (state == emnloop4turntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == emnloop4gototarget) {
		walkToGoalB(lixt);
	}
	else if (state == emnloop4turntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == emnloopshift) {
		setStandParams();
		SHFT(liax, lixx, libx, mnu)
			SHFT(lifa, lifx, lifb, mnfu)
	}
	else if (state == einitbrent) {
		bra = (liax < libx ? liax : libx);
		brb = (liax > libx ? liax : libx);
		brx = brw = brv = lixx;
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			lixt[lij] = lipcom[lij] + brx * lixicom[lij];
		}
	}
	else if (state == einitbrentgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == einitbrentturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == einitbrentgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == einitbrentturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == ebrentloop) {
		setStandParams();
	}
	else if (state == ebrentloopgotoown) {
		walkToGoalA(lixt);
	}
	else if (state == ebrentloopturntotarget) {
		turnToFaceGoalB();
		// tilty = 15.0;
	}
	else if (state == ebrentloopgototarget) {
		walkToGoalB(lixt);
	}
	else if (state == ebrentloopturntoown) {
		turnToFaceGoalA();
		// tilty = 15.0;
	}
	else if (state == ebrentloopupdate) {
		if (brfu <= brfx) {
			if (bru >= brx) {
				bra = brx;
			}
			else {
				brb = brx;
			}
			SHFT(brv, brw, brx, bru)
				SHFT(brfv, brfw, brfx, brfu)
		}
		else {
			if (bru < brx) {
				bra = bru;
			}
			else {
				brb = bru;
			}

			if (brfu <= brfw || brw == brx) {
				brv = brw;
				brw = bru;
				brfv = brfw;
				brfw = brfu;
			}
			else if (brfu <= brfv || brv == brx || brv == brw) {
				brv = bru;
				brfv = brfu;
			}
		}
	}
	else if (state == elinminupdate) {
		for (lij = 0; lij < OFFSET_LENGTH; ++lij) {
			poxit[lij] *= lixmin;
			p[lij] += poxit[lij];
		}
		for (poj = 0; poj < OFFSET_LENGTH; ++poj) {
			xi[poj][poibig] = xi[poj][OFFSET_LENGTH - 1];
			xi[poj][OFFSET_LENGTH - 1] = poxit[poj];
		}
		printState();
		dumpState();
	}
	else if (state == settleAndVerify) {
		// just sit and wait a little to settle
		setStandParams();
	}
}

void GaitOptimiser::modifyOffsets() {
	int toMod = lrand48();
	int mod = lrand48();
	if (mod % 2 == 0) {
		mod = 10;
	}
	else {
		mod = -10;
	}

	current[toMod % OFFSET_LENGTH] += mod;
}

void GaitOptimiser::printState() {
	cout << "p = [ ";
	for (int i = 0; i < OFFSET_LENGTH; ++i) {
		cout << p[i] << " ";
	}
	cout << "] = " << fret << endl;

	cout << "xi =" << endl;
	for (int i = 0; i < OFFSET_LENGTH; ++i) {
		cout << "  [";
		for (int j = 0; j < OFFSET_LENGTH; ++j) {
			cout << xi[i][j] << " ";
		}
		cout << "]" << endl;
	}
}

void GaitOptimiser::dumpState() {
	ofstream out(logfile);

	if (out) {
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << p[i] << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i) {
			for (int j = 0; j < OFFSET_LENGTH; ++j)
				out << xi[i][j] << " ";
			out << endl;
		}
		out << fret << " ";
		out << endl;
		out << poi << " " << poibig << " " << poj << " ";
		out << endl;
		out << podel << " " << pofp << " " << pofptt << " " << pot << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << popt[i] << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << poptt[i] << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << poxit[i] << " ";
		out << endl;
		out << lij << " " << lincom << " ";
		out << endl;
		out << lixx << " " << lixmin << " " << lifx << " " << lifb << " " << lifa
			<< " " << libx << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << lipcom[i] << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << lixicom[i] << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << lixt[i] << " ";
		out << endl;
		out << mnulim << " " << mnu << " " << mnr << " " << mnq << " " << mnfu
			<< " " << mndum << " ";
		out << endl;
		out << briter << " " << bra << " " << brb << " " << brd << " " << bretemp
			<< " " << brfu << " " << brfv << " " << brfw << " " << brfx << " "
			<< brp << " " << brq << " " << brr << " " << brtol1 << " " << brtol2
			<< " " << bru << " " << brv << " " << brw << " " << brx << " " << brxm
			<< " ";
		out << endl;
		out << bre << " ";
		out << endl;
		out << state << " " << timer << " " << exittimer << " ";
		out << endl;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			out << current[i] << " ";
		out << endl;
		out << avgCount << " ";
		out << endl;
		for (int i = 0; i < AVG_RUNS; ++i)
			out << avgLog[i] << " ";
		out << endl;
		out.close();
	}
	else {
		cerr << "Error in dumping to logfile: " << logfile << endl;
	}
}

void GaitOptimiser::undumpState() {
	ifstream in(logfile);

	if (in) {
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> p[i];
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			for (int j = 0; j < OFFSET_LENGTH; ++j)
				in >> xi[i][j];
		in >> fret;
		in >> poi >> poibig >> poj;
		in >> podel >> pofp >> pofptt >> pot;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> popt[i];
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> poptt[i];
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> poxit[i];
		in >> lij >> lincom;
		in >> lixx >> lixmin >> lifx >> lifb >> lifa >> libx;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> lipcom[i];
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> lixicom[i];
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> lixt[i];
		in >> mnulim >> mnu >> mnr >> mnq >> mnfu >> mndum;
		in >> briter >> bra >> brb >> brd >> bretemp >> brfu >> brfv >> brfw
			>> brfx >> brp >> brq >> brr >> brtol1 >> brtol2 >> bru >> brv >> brw
			>> brx >> brxm;
		in >> bre;
		in >> state >> timer >> exittimer;
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			in >> current[i];
		in >> avgCount;
		for (int i = 0; i < AVG_RUNS; ++i)
			in >> avgLog[i];

		in.close();
	}
	else {
		cerr << "Starting Powell's method from scratch." << endl;
		state = initfret;

		// set initial point
		p[0] = -3.95598;
		p[1] = -4.06805;
		p[2] = 1.10905;
		p[3] = 2.84048;
		p[4] = -1.53105;
		p[5] = -0.887359;
		p[6] = 0.0525698;
		p[7] = 0.453644;
		p[8] = -3.44603;
		p[9] = 1.20619;
		p[10] = 2.07076;
		p[11] = 3.52325;
		p[12] = -1.37158;
		p[13] = -2.39713;
		p[14] = -2.91902;
		p[15] = -4.77232;
		p[16] = -8.27637;
		p[17] = -6.35036;
		p[18] = 6.63466;
		p[19] = -4.37867;
		p[20] = -6.00226;
		p[21] = -5.66354;
		p[22] = -2.22923;
		p[23] = -1.51026;

		// set directions
		/*
		   for (int i = 0; i < OFFSET_LENGTH; ++i)
		   for (int j = 0; j < OFFSET_LENGTH; ++j)
		   xi[i][j] = i == j ? xiUnit : 0;
		   */
		// reset all to 0
		for (int i = 0; i < OFFSET_LENGTH; ++i)
			for (int j = 0; j < OFFSET_LENGTH; ++j)
				xi[i][j] = 0;

		int vec = 0;

		const int frontLegs = 0;
		const int backLegs = 12;
		const int frontLocus = 6;
		const int backLocus = 0;
		const int topLocus = 0;
		const int bottomLocus = 3;
		const int numVars = 3;
		const int forwardDir = 0;
		const int sideDir = 1;
		const int heightDir = 2;

		// move all fwd pts out
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xi[vec][frontLegs + frontLocus + bottomLocus + forwardDir] = xi[vec][backLegs + frontLocus + topLocus + forwardDir] = xi[vec][backLegs + frontLocus + bottomLocus + forwardDir] = xiUnit;
		xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xi[vec][frontLegs + backLocus + bottomLocus + forwardDir] = xi[vec][backLegs + backLocus + topLocus + forwardDir] = xi[vec][backLegs + backLocus + bottomLocus + forwardDir] =
			-xiUnit;
		vec++;
		// move all pts fwd
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xi[vec][frontLegs + frontLocus + bottomLocus + forwardDir] = xi[vec][backLegs + frontLocus + topLocus + forwardDir] = xi[vec][backLegs + frontLocus + bottomLocus + forwardDir] = xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xi[vec][frontLegs + backLocus + bottomLocus + forwardDir] = xi[vec][backLegs + backLocus + topLocus + forwardDir] = xi[vec][backLegs + backLocus + bottomLocus + forwardDir] = xiUnit;
		vec++;
		// move top fwd pts out
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xi[vec][backLegs + frontLocus + topLocus + forwardDir] = xiUnit;
		xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xi[vec][backLegs + backLocus + topLocus + forwardDir] =
			-xiUnit;
		vec++;
		// move top fwd pts fwd
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xi[vec][backLegs + frontLocus + topLocus + forwardDir] = xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xi[vec][backLegs + backLocus + topLocus + forwardDir] = xiUnit;
		vec++;
		// move front legs fwd out
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xi[vec][frontLegs + frontLocus + bottomLocus + forwardDir] = xiUnit;
		xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xi[vec][frontLegs + backLocus + bottomLocus + forwardDir] =
			-xiUnit;
		vec++;
		// move front legs fwd forward
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xi[vec][frontLegs + frontLocus + bottomLocus + forwardDir] = xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xi[vec][frontLegs + backLocus + bottomLocus + forwardDir] = xiUnit;
		vec++;
		// move front front top pt fwd
		xi[vec][frontLegs + frontLocus + topLocus + forwardDir] = xiUnit;
		vec++;
		// move front back top pt fwd
		xi[vec][frontLegs + backLocus + topLocus + forwardDir] = xiUnit;
		vec++;

		// move all points side
		xi[vec][frontLegs + frontLocus + topLocus + sideDir] = xi[vec][frontLegs + frontLocus + bottomLocus + sideDir] = xi[vec][frontLegs + backLocus + topLocus + sideDir] = xi[vec][frontLegs + backLocus + bottomLocus + sideDir] = xi[vec][backLegs + frontLocus + topLocus + sideDir] = xi[vec][backLegs + frontLocus + bottomLocus + sideDir] = xi[vec][backLegs + backLocus + topLocus + sideDir] = xi[vec][backLegs + backLocus + bottomLocus + sideDir] = xiUnit;
		vec++;
		// move front leg points side
		xi[vec][frontLegs + frontLocus + topLocus + sideDir] = xi[vec][frontLegs + frontLocus + bottomLocus + sideDir] = xi[vec][frontLegs + backLocus + topLocus + sideDir] = xi[vec][frontLegs + backLocus + bottomLocus + sideDir] = xiUnit;
		vec++;
		// move front front leg points side
		xi[vec][frontLegs + frontLocus + topLocus + sideDir] = xi[vec][frontLegs + frontLocus + bottomLocus + sideDir] = xiUnit;
		vec++;
		// move front front top leg point side
		xi[vec][frontLegs + frontLocus + topLocus + sideDir] = xiUnit;
		vec++;
		// move front back top leg point side
		xi[vec][frontLegs + backLocus + topLocus + sideDir] = xiUnit;
		vec++;
		// move back front leg points side
		xi[vec][backLegs + frontLocus + topLocus + sideDir] = xi[vec][backLegs + frontLocus + bottomLocus + sideDir] = xiUnit;
		vec++;
		// move back front top leg point side
		xi[vec][backLegs + frontLocus + topLocus + sideDir] = xiUnit;
		vec++;
		// move back back top leg point side
		xi[vec][backLegs + backLocus + topLocus + sideDir] = xiUnit;
		vec++;

		// move all points up
		xi[vec][frontLegs + frontLocus + topLocus + heightDir] = xi[vec][frontLegs + frontLocus + bottomLocus + heightDir] = xi[vec][frontLegs + backLocus + topLocus + heightDir] = xi[vec][frontLegs + backLocus + bottomLocus + heightDir] = xi[vec][backLegs + frontLocus + topLocus + heightDir] = xi[vec][backLegs + frontLocus + bottomLocus + heightDir] = xi[vec][backLegs + backLocus + topLocus + heightDir] = xi[vec][backLegs + backLocus + bottomLocus + heightDir] = xiUnit;
		vec++;
		// move top points up
		xi[vec][frontLegs + frontLocus + topLocus + heightDir] = xi[vec][frontLegs + backLocus + topLocus + heightDir] = xi[vec][backLegs + frontLocus + topLocus + heightDir] = xi[vec][backLegs + backLocus + topLocus + heightDir] = xiUnit;
		vec++;
		// move front top points up
		xi[vec][frontLegs + frontLocus + topLocus + heightDir] = xi[vec][frontLegs + backLocus + topLocus + heightDir] = xiUnit;
		vec++;
		// move front front top point up
		xi[vec][frontLegs + frontLocus + topLocus + heightDir] = xiUnit;
		vec++;
		// move back front top point up
		xi[vec][backLegs + frontLocus + topLocus + heightDir] = xiUnit;
		vec++;
		// move front bottom points up
		xi[vec][frontLegs + frontLocus + bottomLocus + heightDir] = xi[vec][frontLegs + backLocus + bottomLocus + heightDir] = xiUnit;
		vec++;
		// move front front bottom point up
		xi[vec][frontLegs + frontLocus + bottomLocus + heightDir] = xiUnit;
		vec++;
		// move back front bottom point up
		xi[vec][backLegs + frontLocus + bottomLocus + heightDir] = xiUnit;
		vec++;
	}
}

double GaitOptimiser::filterAvg() {
	list<int> l;
	for (int i = 0; i < AVG_RUNS; ++i) {
		l.push_back(avgLog[i]);
	}
	l.sort();

	list<int>::const_iterator it = l.begin();
	++it;
	int m1 = *it;
	++it;
	int m2 = *it;
	return (m1 + m2) / 2.0;
}
#endif //COMPILE_ALL_CPP
