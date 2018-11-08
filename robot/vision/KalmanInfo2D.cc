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
 * $Id: KalmanInfo2D.cc 6292 2005-05-31 05:15:55Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 **/

#include "KalmanInfo2D.h"
#include "../share/Common.h"
#include "../share/stlCommon.h"
#include "../share/SharedMemoryDef.h"
//#include "gps.h"


static const double KALMAN_INFO_HYBRID_BEST_FIT_WEIGHT = 0.25;
static const double MIN_PROBSUM = 0.000000001;

int debugCounter = 0;

//////////////////////////////////////////////////
// Public KalmanInfo2D methods
//////////////////////////////////////////////////

KalmanInfo2D::KalmanInfo2D(GPS *g, double iniX, double iniY, double iniXVar, double iniYVar) {
	gps       = g;
	val(0, 0) = iniX;
	val(1, 0) = iniY;
	cov(0, 0) = iniXVar;
	cov(0, 1) = 0;
	cov(1, 0) = 0;
	cov(1, 1) = iniYVar;
	normal2Info();
	globalCurrent = false;
	localCurrent = false;
	resetUpdates();
}


const Vector & KalmanInfo2D::getPos(int context) {
	if (!valuesCurrent)
		info2Normal();

	if (context == GLOBAL) {
		if (!globalCurrent)
			normal2Global();
		return globalVal;

	} else if (context == LOCAL) {
		if (!localCurrent)
			normal2Local();
		return localVal;
	}
	cout << "Called from KalmanInfo2D::getPos(int)! I takes local and global only!" << endl;
	return globalVal;
}


const MMatrix2 & KalmanInfo2D::getCov() {
	// make sure normal and information data are in sync
	if (!valuesCurrent)
		info2Normal();

	return cov;
}


double KalmanInfo2D::getProb(double x, double y) {
	// make sure normal and information data are in sync
	// since we need to know how far the vector is from the mean (covariance matrix not
	// really needed
	if (!valuesCurrent)
		info2Normal();

	MVec2 diff;
	diff(0, 0) = x - val(0, 0);
	diff(1, 0) = y - val(1, 0);
	//cout << "diff=" << diff(0,0) << "," << diff(0,1) << endl;
	MRowVec2 diffTrans;
	diffTrans.isTranspose(diff);

	// note - infoCov is the inverse of Cov
	MRowVec2 val1;
	val1.isMult(diffTrans, infoCov);
	MMatrix1 val2;
	val2.isMult(val1, diff);
	double detInv = infoCov(0, 0) * infoCov(1, 1)
		- infoCov(0, 1) * infoCov(1, 0);
	double detInvSqrt;
	
	if (detInv > 0) detInvSqrt = sqrt(detInv);
	else detInvSqrt = sqrt(-detInv);

	return TWO_PI_INV * detInvSqrt * exp(-val2(0, 0) / 2);
}


double KalmanInfo2D::getNumSD(double x, double y) {
	// make sure normal and information data are in sync
	// since we need to know how far the vector is from the mean (covariance matrix not
	// really needed
	if (!valuesCurrent)
		info2Normal();

	MVec2 diff;
	diff(0, 0) = x - val(0, 0);
	diff(1, 0) = y - val(1, 0);
	//cout << "diff=" << diff(0,0) << "," << diff(0,1) << endl;
	MRowVec2 diffTrans;
	diffTrans.isTranspose(diff);

	// note - infoCov is the inverse of Cov
	MRowVec2 val1;
	val1.isMult(diffTrans, infoCov);
	MMatrix1 val2;
	val2.isMult(val1, diff);
	return val2(0, 0);
}


double KalmanInfo2D::getProb(const SlimVisOb &vob) {
	//cout << "getProb1 (" << n << ") .. head=" << gps->self().h;
	double ang = gps->self().h + vob.h;
	//cout << " ang=" << ang;
	double dx = vob.d *cos(DEG2RAD(ang));
	//cout << " dx=" << dx;
	double dy = vob.d *sin(DEG2RAD(ang));
	//cout << " dy=" << dy << endl;
	return getProb(gps->self().pos.x + dx, gps->self().pos.y + dy);
}


double KalmanInfo2D::getNumSD(const SlimVisOb &vob) {
	double ang = gps->self().h + vob.h;
	double dx = vob.d *cos(DEG2RAD(ang));
	double dy = vob.d *sin(DEG2RAD(ang));
	return getNumSD(gps->self().pos.x + dx, gps->self().pos.y + dy);
}

double KalmanInfo2D::getHeadDiff(const SlimVisOb &vob) {
	if (!valuesCurrent)
		info2Normal();

	double dx = val(0, 0) - gps->self().pos.x;
	double dy = val(1, 0) - gps->self().pos.y;
	if (dx == 0 && dy == 0) return 0.0;

	double expHead = RAD2DEG(atan2(dy, dx)) - gps->self().h;
    return NormalizeAngle_180(expHead - vob.h);
}

void KalmanInfo2D::flushUpdate(IFKUpdateData &data) {
	data.iVecx = (float) updateVal(0, 0);
	data.iVecy = (float) updateVal(1, 0);

	data.iMat11 = (float) updateCov(0, 0);
	data.iMat12 = (float) updateCov(0, 1);
	data.iMat21 = (float) updateCov(1, 0);
	data.iMat22 = (float) updateCov(1, 1);

	resetUpdates();
}


const MVec2 & KalmanInfo2D::getUpdatePos() const {
	return updateVal;
}


const MMatrix2 & KalmanInfo2D::getUpdateCov() const {
	return updateCov;
}


void KalmanInfo2D::resetUpdates() {
	updateVal(0, 0) = 0;
	updateVal(1, 0) = 0;
	updateCov(0, 0) = 0;
	updateCov(1, 0) = 0;
	updateCov(0, 1) = 0;
	updateCov(1, 1) = 0;
}


void KalmanInfo2D::addUpdates(const MVec2 &upVal, const MMatrix2 &upCov) {
	infoVal += upVal;
	infoCov += upCov;
	valuesCurrent = false;
	globalCurrent = false;
	localCurrent = false;
}


void KalmanInfo2D::addUpdates(const IFKUpdateData &data) {
	infoVal(0, 0) = infoVal(0, 0) + (double) data.iVecx;
	infoVal(1, 0) = infoVal(1, 0) + (double) data.iVecy;

	infoCov(0, 0) = infoCov(0, 0) + (double) data.iMat11;
	infoCov(0, 1) = infoCov(0, 1) + (double) data.iMat12;
	infoCov(1, 0) = infoCov(1, 0) + (double) data.iMat21;
	infoCov(1, 1) = infoCov(1, 1) + (double) data.iMat22;

	valuesCurrent = false;
	globalCurrent = false;
	localCurrent = false;
}


void KalmanInfo2D::infoKalman(const SlimVisOb &vob, double weight) {
	// make sure normal and information data are in sync
	if (!valuesCurrent)
		info2Normal();

	// formulae for the observation are
	//     - d = sqrt(SQUARE(vobx - myx) + SQUARE(voby - myy))
	//     - h = atan((voby - myy)/(vobx - myx)) - myh    , note degrees form of inverse tan

	MVec2 meas;
	meas(0, 0) = vob.d;
	meas(1, 0) = vob.h;

	const double dx = val(0, 0) - gps->self().pos.x;
	const double dy = val(1, 0) - gps->self().pos.y;
	if (dx == 0 && dy == 0) {
		return;
	} // should really never happen
	const double dxsq = SQUARE(dx);
	const double dysq = SQUARE(dy);
	const double expDist = sqrt(dxsq + dysq);
	const double expHead = NormalizeAngle_180(RAD2DEG(atan2(dy, dx)) - gps->self().h);
	MVec2 expMeas;
	expMeas(0, 0) = expDist;
	expMeas(1, 0) = expHead;

	// jacobian of observation formulae with respect to dog's position
	MMatrix23 jacSelf;
	jacSelf(0, 0) = -dx / expDist;
	jacSelf(0, 1) = -dy / expDist;
	jacSelf(0, 2) = 0;
	jacSelf(1, 0) = (DEG_OVER_RAD * dy) / (dxsq + dysq);
	jacSelf(1, 1) = -(DEG_OVER_RAD * dx) / (dxsq + dysq);
	jacSelf(1, 2) = -1;

	// transpose of jacobian wrt position
	MMatrix32 jacSelfT;
	jacSelfT.isTranspose(jacSelf);

	// jacobian of observation formulae with respect to opponent's position
	MMatrix2 jacOpp;
	jacOpp(0, 0) = dx / expDist;
	jacOpp(0, 1) = dy / expDist;
	jacOpp(1, 0) = -(DEG_OVER_RAD * dy) / (dxsq + dysq);
	jacOpp(1, 1) = (DEG_OVER_RAD * dx) / (dxsq + dysq);

	// transpose of jacobian wrt opponent
	MMatrix2 jacOppT;
	jacOppT.isTranspose(jacOpp);

	// uncertainty of our own position is currently expressed in cartesian coordinates
	// we must convert this to polar coordinates relative to the observation
	const MMatrix3 &rcov = gps->selfCov();
	MMatrix23 m1;
	m1.isMult(jacSelf, rcov);
	MMatrix2 ownCovWRTMeas;
	ownCovWRTMeas.isMult(m1, jacSelfT);

	// add covariance of dog's position to covariance of measurement to calculate true
	// covariance of the observation
	MMatrix2 obsCov(0);
	obsCov(0, 0) = vob.var;
	obsCov(1, 1) = vob.angleVar;
	obsCov += ownCovWRTMeas;

	// calculate information associated with observation
	MMatrix2 obsCovInv;
	obsCovInv.isInverse(obsCov);
	MMatrix2 prefixM;
	prefixM.isMult(jacOppT, obsCovInv);
	prefixM *= weight;
	MVec2 postfixV;
	postfixV.isMult(jacOpp, val);
	postfixV += meas;
	postfixV -= expMeas;
	MVec2 obValInfo;
	obValInfo.isMult(prefixM, postfixV);
	MMatrix2 obCovInfo;
	obCovInfo.isMult(prefixM, jacOpp);

	// add these to information form and update information
	infoVal += obValInfo;
	infoCov += obCovInfo;
	updateVal += obValInfo;
	updateCov += obCovInfo;

	// Clip position values to field
	info2Normal();
	if (FieldClipping(val(0, 0), val(1, 0)))
    	normal2Info();
    
	globalCurrent = false;
	localCurrent = false;
}

/* Reset ball position and variance */
void KalmanInfo2D::setVal(double x, double y, double var) {
    //cerr << "KI2D setting ball to " << x << ", " << y << endl;
    
	val(0, 0) = x;
	val(1, 0) = y;
	cov(0, 0) = var;
	cov(0, 1) = 0;
	cov(1, 0) = 0;
	cov(1, 1) = var;
	normal2Info();
	globalCurrent = false;
	localCurrent = false;
	resetUpdates();
}

/* addx and addy are velocity information */
void KalmanInfo2D::growVariance(double amount, double addx, double addy) {
	// note 1/det of information matrix is generalised variance
	double det = infoCov(0, 0) * infoCov(1, 1) - infoCov(0, 1) * infoCov(1, 0);
	if (det < 1 / (get95CF(FIELD_LENGTH / 2) * get95CF(FIELD_WIDTH / 2))) {
		return;
	}

	/* The following is the basic method of doing what needs to be done.
	   The switching between information form and normal form, while
	   theoretically correct, leads to floating point errors and hence a slight
	   shift in the position of the gaussian. These errors compound to shift
	   large gaussians significantly. The main problem is that growing the
	   variance requires modification of both information quantities,
	   ie Y=P^(-1) & y=P^(-1)x. We may rearrange the equation that is used
	   Y+=(Y^(-1)+D)^(-1) --> Y+ = (I+DY)^(-1)Y, hence premultiplication is
	   used and conversion back to normal form is not required.*/
       
	   if(!valuesCurrent) {
           info2Normal();
	   }
	   cov(0,0) = cov(0,0) + amount;
	   cov(1,1) = cov(1,1) + amount;
	   val(0,0) = val(0,0) + addx;  // add velocity
	   val(1,0) = val(1,0) + addy;
	   FieldClipping(val(0, 0), val(1, 0));
       
	   normal2Info();
	   globalCurrent = false;
	   localCurrent = false;
	// note if global and local values were current they should still be OK
	// as we are only growing the covariance
/*
	MMatrix2 identity(1.0);
	MMatrix2 d(amount);
	MMatrix2 m;
	m.isMult(d, infoCov);
	m += identity;

	MMatrix2 minv;
	minv.isInverse(m);

	infoCov.preMult(minv);
	infoVal.preMult(minv);

	// now have to update the position
	// use fact Y(x+dx) = Yx + Ydx
	if (addx != 0.0 || addy != 0.0) {
		MVec2 diff;
		diff(0, 0) = addx;
		diff(1, 0) = addy;
		diff.preMult(infoCov);
		infoVal += diff;
	}

	// Clip position values to field
	info2Normal();
	if (FieldClipping(val(0, 0), val(1, 0)))
    	normal2Info();*/
        
	globalCurrent = false;
	localCurrent = false;
}


int KalmanInfo2D::serialize(unsigned char *add, int base) {
	int ret = base;
	ret = infoVal.serialize(add, ret);
	ret = infoCov.serialize(add, ret);
	// note ignore update variables since the purpose of this is to
	// use offline and hence update vals won't be used

	return ret;
}


int KalmanInfo2D::unserialize(unsigned char *add, int base) {
	int ret = base;
	ret = infoVal.unserialize(add, ret);
	ret = infoCov.unserialize(add, ret);

	valuesCurrent = false;
	globalCurrent = false;
	localCurrent = false;

	return ret;
}



//////////////////////////////////////////////////
// Private KalmanInfo2D methods
//////////////////////////////////////////////////




//////////////////////////////////////////////////
// Public methods associated to KalmanInfo2D
//////////////////////////////////////////////////

void getGlobalObservation(const MVec3 &pos,
		const MMatrix3 &cov,
		double dist,
		double head,
		double distVar,
		double headVar,
		MVec2 &obsPos,
		MMatrix2 &obsCov) {
	double trueHeadRad = DEG2RAD(pos(2, 0) + head);
	obsPos(0, 0) = pos(0, 0) + dist * cos(trueHeadRad);
	obsPos(1, 0) = pos(1, 0) + dist * sin(trueHeadRad);

	// jacobian of the global observation wrt robot's position
	MMatrix23 jacRobot(0);
	jacRobot(0, 0) = 1.0;
	jacRobot(0, 2) = -RAD_OVER_DEG * sin(trueHeadRad);
	jacRobot(1, 1) = 1.0;
	jacRobot(1, 2) = RAD_OVER_DEG * cos(trueHeadRad);

	// transpose ...
	MMatrix32 jacRobotT;
	jacRobotT.isTranspose(jacRobot);

	// jacobian of the global observation wrt polar observation
	MMatrix2 jacPolar(0);
	jacPolar(0, 0) = cos(trueHeadRad);
	jacPolar(0, 1) = -RAD_OVER_DEG * sin(trueHeadRad);
	jacPolar(1, 0) = sin(trueHeadRad);
	jacPolar(1, 1) = RAD_OVER_DEG * cos(trueHeadRad);

	// transpose ...
	MMatrix2 jacPolarT;
	jacPolarT.isTranspose(jacPolar);

	// variance from uncertainty of robot position
	MMatrix23 m1;
	m1.isMult(jacRobot, cov);
	MMatrix2 varRobot;
	varRobot.isMult(m1, jacRobotT);

	// variance from uncertainty of observation
	MMatrix2 oCov(0);
	oCov(0, 0) = distVar;
	oCov(1, 1) = headVar;
	MMatrix2 m2;
	m2.isMult(jacPolar, oCov);
	MMatrix2 varObs;
	varObs.isMult(m2, jacPolarT);

	// resulting covariance is the sum of these
	obsCov = varRobot;
	obsCov += varObs;
}


void applyKalmanUpdateSimple(KalmanInfo2D **array,
		int num,
		const SlimVisOb &vob) {
	double maxProb = 0;
	int opNum = -1;

	for (int j = 0 ; j < num ; j++) {
		double p = array[j]->getProb(vob);
		if (p > maxProb) {
			maxProb = p;
			opNum = j;
		}
	}

	array[opNum]->infoKalman(vob);
}


void applyKalmanUpdateWA(KalmanInfo2D **array, int num, const SlimVisOb &vob) {
	double probs[num];
	double probSum = 0;

	for (int j = 0 ; j < num ; j++) {
		probs[j] = array[j]->getProb(vob);
		probSum += probs[j];
	}

	if (probSum == 0) // BIG ERROR - should never happen
	{
		return;
	}

	for (int j = 0 ; j < num ; j++) {
		array[j]->infoKalman(vob, probs[j] / probSum);
	}
}


void applyKalmanUpdateHybrid(KalmanInfo2D **array,
		int num,
		const SlimVisOb &vob) {
    static const double lostOpponentVar = get95CF(50);

    double worstOppVar = 0.0, bestHeadDiff = 360.0;
    double headDiff[num], approxVar;
	int best = -1, lost = -1;
    MMatrix2 cov;
   
    for (int j = 0 ; j < num ; j++) {
        headDiff[j] = fabs(array[j]->getHeadDiff(vob));
        
        cov = array[j]->getCov();
        approxVar = sqrt(SQUARE(cov(0, 0)) + SQUARE(cov(1, 1)));
        if (approxVar > lostOpponentVar && approxVar > worstOppVar) {
            worstOppVar = approxVar;
            lost = j;
        }
        
        if (headDiff[j] < bestHeadDiff) {
            bestHeadDiff = headDiff[j];
            best = j;
        }
    }

    if (bestHeadDiff > 15.0 && lost != -1)
        array[lost]->infoKalman(vob);
    else
        array[best]->infoKalman(vob);
}


struct node {
	int array[8];
	// double [numVob][numGaussian]
	//const double *numSDs;
	const double *probs;
	int numVob;
	int numGaussian;

	node & operator=(const node &rhs) {
		for (int i = 0 ; i < 8 ; i++) {
			array[i] = rhs.array[i];
		}
		probs = rhs.probs;
		numVob = rhs.numVob;
		numGaussian = rhs.numGaussian;
		return *this;
	}

	void clearArray() {
		for (int i = 0 ; i < 8 ; i++) {
			array[i] = -1;
		}
	}

	void printArray() {
		cout << "array={";
		for (int i = 0 ; i < numVob - 1 ; i++) {
			cout << array[i] << ",";
		}
		cout << array[numVob - 1] << "}" << endl;
	}
};

// heuristic
double h(const node &n) {
	double ret = 0;
	for (int i = 0 ; i < n.numVob ; i++) {
		//double numSD = 0;
		double prob = 1;
		if (n.array[i] == -1) {
			for (int j = 0 ; j < n.numGaussian ; j++) {
				bool before = false;
				for (int k = 0 ; k < i ; k++) {
					if (n.array[k] == j)
						before = true;
				}
				if (!before) {
					//if(n.numSDs[i*n.numGaussian+j] < numSD)
					//    numSD = n.numSDs[i*n.numGaussian+j];
					if (n.probs[i * n.numGaussian + j] < prob)
						prob = n.probs[i * n.numGaussian + j];
				}
			}
		}
		else
			prob = n.probs[i * n.numGaussian + n.array[i]];
		ret += prob;
	}
	//return ret;
	return 1 - ret;
}


// Goal Test
bool gt(const node &n) {
	for (int i = 0 ; i < n.numVob ; i++) {
		if (n.array[i] == -1) {
			return false;
		}
	}
	return true;
}

// expand node
void en(const node &n, list<node> &list) {
	for (int i = 0 ; i < n.numVob ; i++) {
		if (n.array[i] == -1) {
			for (int j = 0 ; j < n.numGaussian ; j++) {
				bool before = false;
				for (int k = 0 ; k < i ; k++) {
					if (n.array[k] == j)
						before = true;
				}
				if (!before) {
					node next = n;
					next.array[i] = j;
					list.push_front(next);
				}
			}
		}
	}
}


void applyKalmanMultiUpdatesHybrid(KalmanInfo2D **array,
		int num,
		const SlimVisOb vob[],
		int vobMinInd,
		int vobMaxInd) {
	int vobNum = 0;
	for (int i = vobMinInd ; i <= vobMaxInd ; i++) {
		if (vob[i].var < get95CF(150)) {
			vobNum++;
		}
	}
	if (vobNum == 0) {
		return;
	}
	double *probs = new double[vobNum *num];
	//double *numSDs = new double[vobNum*num];
	for (int i = 0 ; i < vobNum ; i++) {
		for (int j = 0 ; j < num ; j++) {
			// this is inefficient - repeated calculations - Ross
			probs[i * num + j] = array[j]->getProb(vob[vobMinInd + i]);
			//numSDs[i*num+j] = array[j]->getNumSD(vob[vobMinInd+i]);
		}
	}

	node start;
	start.clearArray();
	start.probs = probs;
	start.numVob = vobNum;
	start.numGaussian = num;

	start.printArray();
	node sol = aStarSearch<node>(start, gt, h, en);
	sol.printArray();

	for (int i = 0 ; i < vobNum ; i++) {
		double probSum = 0;
		int best = sol.array[i];
		for (int j = 0 ; j < num ; j++)
			probSum += probs[i * num + j];

		// if the summ of the probabilities is to low we just update the closest
		if (probSum < MIN_PROBSUM) {
			array[best]->infoKalman(vob[vobMinInd + i], 1.0);
			cout << probSum << " ";
			continue;
		}
		else
			cout << ".";

		for (int j = 0 ; j < num ; j++) {
			double weight = probs[i *num + j]
				/ probSum * (1 - KALMAN_INFO_HYBRID_BEST_FIT_WEIGHT);
			if (j == best)
				weight += KALMAN_INFO_HYBRID_BEST_FIT_WEIGHT;
			array[j]->infoKalman(vob[vobMinInd + i], weight);
		}
	}
	cout << endl;

	// if there is a gaussian that we should have seen and it did not win an
	// observation then increase it's variance considerably
	/*for(int j=0 ; j<num ; j++) {

	  }*/

	delete(probs);
	//delete(numSDs);
}
