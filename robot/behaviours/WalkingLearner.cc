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
 * $Id: WalkingLearner.cc 5072 2005-03-10 22:59:09Z alexn $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



//TOBECONTINUE: 
//head position when reading IR sensor : NormalWalk, head 0 0 20
//use aaForceStepComplete when stopping
//run optimization with fixed walk
#include "WalkingLearner.h"
#include "../vision/InfoManager.h"
#include "Behaviours.h"
#include "../share/SharedMemoryDef.h"
#include "../share/WirelessTypeDef.h"
#include "../share/WalkLearningParams.h"
#include <string>
#include <sstream>
#include "kimUtil.h"
#include "../vision/Blob.h"

#ifndef OFFLINE
#include "../vision/Vision.h"
#endif //OFFLINE

using namespace std;
using namespace Behaviours;

static const bool bDebugSeenBeacon = false;
static const bool bDebugParameters = false;
static const bool bDebugState = false;


const double FORWARD_FASTEST = 9;
const double FORWARD_WHENTURN = 7;
const double MINIMUM_PINK_AREA = 1;

const int LEARNING_WALK = EllipseWalkWT ; // SkellipticalWalkWT;
const int PG_FIRST = 60;
const int HF_FIRST = 90;
const int HB_FIRST = 120;
const int FFO_FIRST = 55;
const int FSO_FIRST = 15;
const int BFO_FIRST = -55;
const int BSO_FIRST = 5;
const int ELP_WIDTH_FRONT_FIRST = 30;
const int ELP_HEIGHT_FRONT_FIRST = 20;
const int ELP_WIDTH_BACK_FIRST = 50;
const int ELP_HEIGHT_BACK_FIRST = 30;

static const int STAYING_STILL_PERIOD = 30; //30 frames = 1 second

static const int FINISH = 0;
static const int CONTINUE = 1;

int WalkingLearner::actionCounter = 0;
string debugBuffer;
ostringstream debugSS;
//ostringstream debugSS_1;


static double forwardSpeed;
static bool useTurn;
static bool learnUsingHighGain = true;

int xColor = cBEACON_YELLOW;
int numXColor;
HalfBeacon **xColorArray;

VisualObject *lowBeacon ;
VisualObject *highBeacon;
VisualObject *theBeacon; // is one of the above 2

double stopDistance1, stopDistance2;

/////////////////////////////////////////////////////////////////
//real definitions of variables
//

namespace WalkingLearner {
    const char *stateToStr[] = {
        "lsIDLE ",
        "lsMOVING_TO_READY_STATE",
        "lsREADY",
		"lsSTART_LEARNING",
		"lsPREPARATION1",
		"lsWALKING_FORTH",
		"lsPREPARATION2",
        "lsTURNING180",
		"lsPREPARATION3",
		"lsWALKING_BACK",
		"lsPREPARATION4",
		"lsDONE",
		"lsGETTING_LOST",
		"lsLOW_BATTERY",
		"lsWALK_CALIBRATING",
		"lsLEARNING_PINK_DISTANCE_LOW",
		"lsLEARNING_PINK_DISTANCE_HIGH",
		"lsTURN_AWAY_FROM_PINK",
		"lsPRE_PREPARATION1",
		"lsNOSTAT"
	};

	LearningState nextState[lsNOSTATE];

	LearningState learningState;
	LearningState lastLearningState;
	Timer timer;
	int lockFrames;
	double lastWalkingForthTime, lastWalkingBackTime;
	double irSensorValue;
	double irSensorValueFar;
}

static int TURNING_FRAMES = 55;
static int TURNING_MIN_FRAMES = 25;
static int TURNING_STEP = 10;
static int BACKING_OFF = -3;

WalkLearningParams WalkingLearner::wlParams;

/////////////////// Local function headers ////////////////////////////

void setTurningTowardBeacon(HalfBeacon &hb, double &turnCCW);
void adjustTurning(KimCommand &command);
void initWLParams(WalkLearningParams &wlParams);
bool isVerticallyAligned(HalfBeacon *bc1, HalfBeacon *bc2);
int aligningToBeacon();

/////////////////// Function definitions ///////////////////////

void printVob() {
	static int count = 0;
    if (vision->numPink > 0){
        for (int p = 0; p < vision->numPink; p++) {
            /*
            HalfBeacon pink = *(vision->hbPink[p]);
            cout << "Pink areas = " << pink.area << " " << pink.cx << " " << pink.cy << " min= " << pink.getXMin() << " max " << pink.getXMax() << "  " << numXColor << endl;
            for (int i=0;i<numXColor;i++){
                cout << "Dist = " << DISTANCE( xColorArray[i]->cx , xColorArray[i]->cy, pink.cx, pink.cy) << "  area = " << xColorArray[i]->area << " " << isVerticallyAligned(&pink, xColorArray[i]) ;
                HalfBeacon *beacon = xColorArray[i];
                double area = beacon->area;
                double area2 = SQUARE( beacon->getXMin() - beacon->getXMax() ) ;
                cout << "  xarea = " << area << " xarea2 = " << area2 ;
                cout << "  xmin = " << beacon->getXMin() << " xmax = " << beacon->getXMax() << endl;
            }
            */
        }
    }


#ifndef OFFLINE
    OStatus result;
    OPowerStatus currentStatus;

    result = OPENR::GetPowerStatus(&currentStatus);
    cout << "Battery capacity: " << currentStatus.remainingCapacity << endl;
    cout << "Battery current : " << currentStatus.current << endl;
#endif
}

double getLearningBodyTilt(){
    using namespace WalkingLearner;
    switch (LEARNING_WALK) {
    	case EllipseWalkWT:
		    return asin((wlParams.ep.hb - wlParams.ep.hb) / BODY_LENGTH);
		case SkellipticalWalkWT:
			return asin((wlParams.sp.backH - wlParams.sp.frontH) / BODY_LENGTH);
		case SkiWalkWT:
			return asin((wlParams.skp.backH - wlParams.skp.frontH) / BODY_LENGTH);
		default:
			cout << "Unknown walktype in getLearningBodyTilt" << endl;
			break;
	}
	return 0;
}

bool canSeeAPink(HalfBeacon &pink) {
	if (vision->numPink > 0) {
		pink = *(vision->hbPink[0]);
		/*
		   pink.Print();
		   cout << "Areas = " ;
		   for (int i=0;i<vision->numPink;i++)
		   cout << vision->hbPink[i]->area << " " ;
		   cout << endl;
		   */
		return pink.area > MINIMUM_PINK_AREA ;
	}
	else {
		return false;
	}
}

double getImageHead(const HalfBeacon &hb) {
	return PointToHeading(hb.cx);
}

void printSomething(int INTERVAL) {
	static int count = 0;
	if (count++ % INTERVAL != 0) {
		return;
	}

	HalfBeacon pink;
	if (canSeeAPink(pink)) {
		cout << "imhHead = " << getImageHead(pink) << endl;
	}
}

// the state cycle is defined here , only defined nessesary next States
void WalkingLearner::initNextState(){
    nextState[lsIDLE]                   = lsMOVING_TO_READY_STATE;
    nextState[lsMOVING_TO_READY_STATE]  = lsREADY;
    nextState[lsREADY]                  = lsSTART_LEARNING;
    nextState[lsSTART_LEARNING]         = lsPRE_PREPARATION1;
    nextState[lsPRE_PREPARATION1]         = lsPREPARATION1;
    nextState[lsPREPARATION1]           = lsWALKING_FORTH;
    nextState[lsWALKING_FORTH]          = lsPREPARATION2;
    nextState[lsPREPARATION2]           = lsTURNING180;
    nextState[lsTURNING180]             = lsPREPARATION3;
    nextState[lsPREPARATION3]           = lsWALKING_BACK;
    nextState[lsWALKING_BACK]           = lsPREPARATION4;
    nextState[lsPREPARATION4]           = lsDONE;
}

//Parameters to start off with
void initWLParams(WalkLearningParams &wlParams){
	bzero((char *)&wlParams, sizeof(wlParams));	// make sure everything we haven't set is zero
	wlParams.walkType = LEARNING_WALK;
	switch (LEARNING_WALK) {
		case EllipseWalkWT:
			useTurn = false;
			forwardSpeed = FORWARD_FASTEST;
			wlParams.ep.PG = PG_FIRST;
			wlParams.ep.hf = HF_FIRST;
			wlParams.ep.hb = HB_FIRST;
			wlParams.ep.ffo = FFO_FIRST;
			wlParams.ep.fso = FSO_FIRST;
			wlParams.ep.bfo = BFO_FIRST;
			wlParams.ep.bso = BSO_FIRST;
			wlParams.ep.elpWidthFront = ELP_WIDTH_FRONT_FIRST;
			wlParams.ep.elpHeightFront = ELP_HEIGHT_FRONT_FIRST;
			wlParams.ep.elpWidthBack = ELP_WIDTH_BACK_FIRST;
			wlParams.ep.elpHeightBack = ELP_HEIGHT_BACK_FIRST;
		break;
		case SkellipticalWalkWT:
			useTurn = true;
			forwardSpeed = 6;
			wlParams.sp.init();
		break;
		case SkiWalkWT:
			useTurn = true;
			forwardSpeed = 6;
			wlParams.skp.init();
		break;
    }
}

void WalkingLearner::initPlayer() {
	cout << "Initiating WalkingLearner" << endl;
    initNextState();
    initWLParams(wlParams);
	learningState = lsIDLE;
	lastWalkingBackTime = -1;
	lastWalkingForthTime = -1;
	lockFrames = -1;

    if (Behaviours::PLAYER_NUMBER == 1){
        xColor = cBEACON_YELLOW;
    }
    else if (Behaviours::PLAYER_NUMBER == 2){
        xColor = cBEACON_BLUE;
    }
    else if (Behaviours::PLAYER_NUMBER == 3){
        xColor = cBEACON_GREEN;
    }

    
}

void WalkingLearner::setLearningWalkParams(){
    setWalkParams();
	Behaviours::walkType = wlParams.walkType;
    switch (wlParams.walkType) {
    	case EllipseWalkWT:
			Behaviours::PG = wlParams.ep.PG;
			Behaviours::hF = wlParams.ep.hf;
			Behaviours::hB = wlParams.ep.hb;
			Behaviours::ffO = wlParams.ep.ffo;
			Behaviours::fsO = wlParams.ep.fso;
			Behaviours::bfO = wlParams.ep.bfo;
			Behaviours::bsO = wlParams.ep.bso;
    	break;
    	case SkellipticalWalkWT:
			Behaviours::PG = wlParams.sp.halfStepTime;
			Behaviours::hF = wlParams.sp.frontH;
			Behaviours::hB = wlParams.sp.backH;
			Behaviours::ffO = wlParams.sp.frontF;
			Behaviours::fsO = wlParams.sp.frontS;
			Behaviours::bfO = wlParams.sp.backF;
			Behaviours::bsO = wlParams.sp.backS;
			Behaviours::hdF = wlParams.sp.frontFwdHeight;
			Behaviours::hdB = wlParams.sp.backFwdHeight;
    	break;
    	case SkiWalkWT:
			Behaviours::PG = wlParams.sp.halfStepTime;
			Behaviours::hF = wlParams.sp.frontH;
			Behaviours::hB = wlParams.sp.backH;
			Behaviours::ffO = wlParams.sp.frontF;
			Behaviours::fsO = wlParams.sp.frontS;
			Behaviours::bfO = wlParams.sp.backF;
			Behaviours::bsO = wlParams.sp.backS;
			Behaviours::hdF = 20;
			Behaviours::hdB = 40;
    	break;
    	default:
    		cout << "Unknown walk type in WalkingLearner::setLearningWalkParams" << endl;
    	break;
    }
}

void WalkingLearner::initializeNewAction() {
	irSensorValue = sensors->sensorVal[ssINFRARED_NEAR];
	irSensorValueFar = sensors->sensorVal[ssINFRARED_FAR];
    actionCounter += 1;

	//cout <<  "IR = " << sensors->sensorVal[ssINFRARED_NEAR] << endl;
    //setLearningWalkParams();
    setStandParams();
	panx = 0;
	tilty = 0;
	cranez = RAD2DEG(getLearningBodyTilt());
	headtype = ABS_H;
    

    //init xColor (color of the beacon that the dog is running to)
    switch (xColor ){
        case cBEACON_YELLOW:
            numXColor = vision->numYellow;
            xColorArray = vision->hbYellow;
            lowBeacon = &vision->vob[vobYellowLeftBeacon];
            highBeacon = &vision->vob[vobYellowRightBeacon];
            break;
        case cBEACON_GREEN:
            numXColor = vision->numGreen;
            xColorArray = vision->hbGreen;
            lowBeacon = &vision->vob[vobGreenLeftBeacon];
            highBeacon = &vision->vob[vobGreenRightBeacon];
            break;
        case cBEACON_BLUE:
            numXColor = vision->numBlue;
            xColorArray = vision->hbBlue;
            lowBeacon = &vision->vob[vobBlueLeftBeacon];
            highBeacon = &vision->vob[vobBlueRightBeacon];
            break;
        default:
            break;
    }
    debugSS << "Low beacon : " << lowBeacon->x << " " << lowBeacon->y << " " << lowBeacon->cf << endl;
    debugSS << "High beacon : " << highBeacon->x << " " << highBeacon->y << " " << highBeacon->cf << endl;
}

bool WalkingLearner::DecideNextAction() {
	//printVob();
	//printSomething(10);
	initializeNewAction();
	switch (learningState) {
		case lsIDLE: 
            doIdleState();
            break;
        case lsMOVING_TO_READY_STATE:
            moveToReadyState();
            break;
        case lsREADY:
            stayReady();
            break;

		case lsSTART_LEARNING: 
            startLearning();
            break;
		case lsPRE_PREPARATION1: 
            doPrePreparation1();
            break;
		case lsPREPARATION1: 
            preparation1();
            break;
		case lsWALKING_FORTH: 
            walkingForth();
            break;
		case lsPREPARATION2: 
            preparation2();
            break;
		case lsTURNING180: 
            turnAround180AtLowPinkBeacon();
            break;
        case lsPREPARATION3: 
            preparation3();
            break;
		case lsWALKING_BACK: 
            walkingBack();
            break;
		case lsPREPARATION4: 
            preparation4();
            break;
        case lsDONE: 
            doneLearning();
            break;
        case lsNOSTATE: 
            cout << " WARNING : in no state" << endl;
            break;
        case lsWALK_CALIBRATING:
            setWalkCalibration();
            break;
        default: break;
	}
    //cout << "walk : " << Behaviours::forward << " " << Behaviours::left << " " << Behaviours::turnCCW << endl; 
    storeDebuggingInformation();
    /*
#ifndef OFFLINE
    OStatus result;
    OPowerStatus currentStatus;

    result = OPENR::GetPowerStatus(&currentStatus);
    cout << "Battery current : " << currentStatus.current << endl;
#endif
    */
	return true;
}

/*
void setLearningBeaconState(string rawValue){
    using namespace WalkingLearner;
    lastLearningState = learningState;
    if (rawValue.find("high") != string::npos){
        learningState = lsLEARNING_PINK_DISTANCE_HIGH;
    }
    else if (rawValue.find("low") != string::npos) {
        learningState = lsLEARNING_PINK_DISTANCE_LOW;
    }
    else {
        xColor = atoi(rawValue.c_str());
        cout << " set beacon to " << xColor << endl;
    }
}
*/

void WalkingLearner::gotWirelessCommand(const char *value) {
	KimCommand command(value); //already handled the elimination of packet sent to other robots
	cout << "Got command value " << value << "-> " << command.toString() << endl;
    switch (command.type) {
        case kcmStart: 
            learningState = lsSTART_LEARNING;
            break;
        case kcmQuery: 
            replyQuery(command);
            break;
        case kcmAdjustTurning: 
            adjustTurning(command);
            break;
        case kcmStop: 
            learningState = lsIDLE;
            stopEverything();
            break;
        case kcmSetState:
            setLearningState(command.rawValue);
            break;
        case kcmSetLearnParams:
            setLearningParameters(command.rawValue);
            break;
        case kcmBeReady:
            learningState = lsMOVING_TO_READY_STATE;
            break;
        case kcmWalkStraight:
            learningState = lsWALK_CALIBRATING;
            break;

        default: break;
    }
}

void WalkingLearner::stayReady() {
    setStandParams();
}

void WalkingLearner::trackPink(){
    HalfBeacon pink;
    if (canSeeAPink(pink)){
        setHeadTowardBeacon(pink);
    }
}

void WalkingLearner::doIdleState() {
	//cout << "in idle state" << RAD2DEG(getLearningBodyTilt()) << endl;
    setStandParams();
    headtype = ABS_H;
	cranez = RAD2DEG(getLearningBodyTilt());
    //walkType = LEARNING_WALK;
    
}

void WalkingLearner::moveToReadyState(){
    turnAround180AtHighPinkBeacon();
}

void WalkingLearner::startLearning() {
	VisualObject &beaconLeft = vision->vob[vobGreenLeftBeacon];
	VisualObject &beaconRight = vision->vob[vobGreenRightBeacon];

    /*
	walkType = wlParams.walkType;
	forward = forwardSpeed;
	Behaviours::left = 0;
	turnCCW = 0;
    */

	goNextState(); //lsPRE_PREPARATION1
}

void WalkingLearner::replyQuery(KimCommand &command) {
	VisualObject &beaconLeft = vision->vob[vobGreenLeftBeacon];
	VisualObject &beaconRight = vision->vob[vobGreenRightBeacon];
	command = command;
	ostringstream reply;
    cout << Behaviours::PLAYER_NUMBER << " " 
		<< stateToStr[learningState]
		<< " "
	    << lastWalkingForthTime << " " << lastWalkingBackTime
        << " " 
        << stopDistance1 << " " << stopDistance2 
		<< endl;
	reply << "&&&&" 
        << Behaviours::PLAYER_NUMBER << " " 
		<< stateToStr[learningState]
		<< " "
	    << lastWalkingForthTime << " " << lastWalkingBackTime
        << " " 
        << stopDistance1 << " " << stopDistance2 
		<< endl
		<< " x = " << gps->self().pos.x
		<< " y = " << gps->self().pos.y
		<< " heading = " << gps->self().h
	    << " d = " << beaconLeft.dist2 << " cf = " << beaconLeft.cf 
        << " - "
        << " d = " << beaconRight.dist2 << " cf = " << beaconRight.cf 
		<< endl;

#ifndef OFFLINE
    transmission->sendWirelessMessage(reply.str());
#endif
	printVob();
    //dodgy, uncommenting this line make the dog crash -> it's ok now
    cout << "Debug Buffer : " << endl << debugBuffer << endl << " ----- " << endl;
}

static int CLOSE_DISTANCE_FORTH = 400000;
static int CLOSE_DISTANCE_BACK = 350000;


void setTurningTowardBeacon(HalfBeacon &hb, double &turnCCW) {
    double currentPanx = gInfoManager.getJointSensor(headPan);
    turnCCW = currentPanx + PointToHeading(hb.cx);

    turnCCW = CLIP(turnCCW,20.0);
	if (abs(turnCCW) > 5) {
        turnCCW *= 1; //adjust for "inaccurate turning" walk
	}
	else if (!useTurn) {
		turnCCW = 0;
	}
}

const double TRACK_DELTA_CRANEZ = 1; //degree
const double TRACK_THRESHOLD = 15; //degree

//Relatively tracking abitrary object, note that absolute tracking 
//using cranez and panx
void WalkingLearner::setHeadTowardBeacon(HalfBeacon &pink){
    //
    headtype = REL_Z;
    tilty = 0; 
    //calculate cranez
    double t = PointToElevation(pink.cy);
    if (t < 0){
        cranez = -TRACK_DELTA_CRANEZ;
    }
    else{
        if (t >= TRACK_THRESHOLD){
            cranez = TRACK_DELTA_CRANEZ;
        }
        else cranez = 0;
    }
    //calculate panx
    double currentPanx = gInfoManager.getJointSensor(headPan);
    panx = currentPanx + PointToHeading(pink.cx);

    debugSS << " cy = " << pink.cy 
        << " cx = " << pink.cx 
        << " PointToElevation = " << PointToElevation(pink.cy)
        << " PointToHeading = " << PointToHeading(pink.cx)
        << "cranez = " << cranez 
        << "panx = " << panx << endl;
}


void WalkingLearner::walkingForth() {
	HalfBeacon pink;
	if (canSeeAPink(pink)) {
		setWalkingStraightForward();
		setTurningTowardBeacon(pink, turnCCW);
        setHeadTowardBeacon(pink);
	}
	else {
		setWalkingStraightForward();
        tilty = RAD2DEG(getLearningBodyTilt());

	}

	if (irSensorValueFar < CLOSE_DISTANCE_FORTH) {
		goNextState(); //->lsPREPARATION2
	}
}

static const int PINK_IN_BEACON_THRESHOLD = 40;
static const int FAR_DISTANCE_THRESHOLD = 40;

bool pinkInsideTheBeacon(HalfBeacon &pink, VisualObject *visualBeacon){
    if (bDebugSeenBeacon)
        cout << "pink : " << pink.cx << " " << pink.cy << endl
            << "beacon : " << visualBeacon->x << " " << visualBeacon->y << " " << visualBeacon->cf << endl;
    return (visualBeacon->cf > 0 && DISTANCE(pink.cx, pink.cy, visualBeacon->x, visualBeacon->y) < PINK_IN_BEACON_THRESHOLD);
}

bool canSeeABeacon(HalfBeacon &pink){
    for (int p=0;p<vision->numPink;p++){
        pink = *vision->hbPink[p];
        if (pinkInsideTheBeacon(pink,theBeacon) && theBeacon->dist2 > FAR_DISTANCE_THRESHOLD) 
            return true;
    }
    return false;
}

static const int NUM_OF_FRAMES_TO_BE_SEEN = 5;

int canSeeANumberOfBeacon(const int PERIOD){
    using WalkingLearner::actionCounter;
    static int counter = 0;
    static int lastActionCounter = 0;
    static int numBeaconSeen = 0;
    if (lastActionCounter != actionCounter -1){ //reset
        counter = 0;
        numBeaconSeen = 0;
    }
    lastActionCounter = actionCounter;

    cout << "canSeeANumberOfBeacon counter = " << counter << endl;

    if (counter ++ == PERIOD){
        counter = 0;
        bool seenAllTheTime = numBeaconSeen == PERIOD;
        numBeaconSeen = 0;
        return (seenAllTheTime)?0:1;
    }
    static HalfBeacon pink;
    if (canSeeABeacon(pink)) numBeaconSeen ++;
    return 1;
}

void WalkingLearner::turnAround180AtLowPinkBeacon() {
	cranez = RAD2DEG(getBodyTilt()) + 5 ; //since high pink beacon is abit higher 

	HalfBeacon pink;
    theBeacon = highBeacon;
	if (canSeeABeacon(pink)) {
        switch (canSeeANumberOfBeacon(NUM_OF_FRAMES_TO_BE_SEEN) ){
            case 0: 
                cout << "Done first leg" << irSensorValueFar << " " << irSensorValue<< endl;
                goNextState(); // -> lsPREPARATION3
                break;
            default:
                break;
        }
	}
    else{
        walkType = NormalWalkWT;
        forward = 0;
        Behaviours::left = 0;
        turnCCW = TURNING_STEP;
    }
}

void WalkingLearner::turnAround180AtHighPinkBeacon() {
	tilty = RAD2DEG(getBodyTilt());

	HalfBeacon pink;
    setStandParams();
    theBeacon = lowBeacon;
    if (canSeeABeacon(pink)) {
        switch (canSeeANumberOfBeacon(NUM_OF_FRAMES_TO_BE_SEEN) ){
            case 0: 
                goNextState(); // -> lsREADY
                break;
            default:
                break;
        }
	}
    else{
        walkType = NormalWalkWT;
        forward = 0;
        Behaviours::left = 0;
        turnCCW = TURNING_STEP;
    }
}

void WalkingLearner::walkingBack() {
	//walkingToBeacon(vision->vob[vobGreenRightBeacon]);

	HalfBeacon pink;
	if (canSeeAPink(pink)) {
		setWalkingStraightForward();
		setTurningTowardBeacon(pink, turnCCW);
        setHeadTowardBeacon(pink);
	}
	else {
		setWalkingStraightForward();
        tilty = RAD2DEG(getLearningBodyTilt());
	}

	if (irSensorValueFar < CLOSE_DISTANCE_BACK) {
		goNextState(); // -> lsPREPARATION4
	}
}


static const int ENOUGH_BACKING_OFF = 30 * 3; // 3 seconds
int backingOff(){
    using namespace WalkingLearner;
    static int counter = 0;
    static int lastActionCounter = 0;
    if (lastActionCounter != actionCounter -1){ //reset
        counter = 0;
    }
    counter += 1;
    lastActionCounter = actionCounter;

    if (counter >= ENOUGH_BACKING_OFF)
        return FINISH;
    forward = BACKING_OFF;
    return CONTINUE;
}

int stayingStill(){
    using namespace WalkingLearner;
    static int counter = 0;
    static int lastActionCounter = 0;
    if (lastActionCounter != actionCounter -1){ //reset
        counter = 0;
    }
    counter += 1;
    lastActionCounter = actionCounter;

    if (bDebugState)
        cout << "stay still " << counter << endl;
    if (counter >= STAYING_STILL_PERIOD)
        return FINISH;
    setStandParams();
    return CONTINUE;
}

void WalkingLearner::doPrePreparation1(){
    if (bDebugState)
        cout << "pre-pre" << endl;
    theBeacon = lowBeacon;
    HalfBeacon pink;
    if (canSeeABeacon(pink)){
        setHeadTowardBeacon(pink);
    }

    int result = aligningToBeacon();
    forward = -2;
    if (result == FINISH){
        result = backingOff();
        if (result == FINISH){
            result = stayingStill();
            if (result == FINISH)
                goNextState(); //->lsPREPARATION1
        }
    }
}

void WalkingLearner::preparation1() {
	if (learnUsingHighGain)
		setHighGain();
    else
        setLowGain();
	timer.start();
	goNextState(); //-> lsWALKING_FORTH;
}

void WalkingLearner::preparation2() {
    static int lastActionCounter = 0;
    static int counter = 0;
    if (lastActionCounter != actionCounter -1 ){
        counter = 0;
    }
    lastActionCounter = actionCounter;
    counter += 1;

    //cout << irSensorValueFar << " " << stopDistance1 << endl;
    if (counter == STAYING_STILL_PERIOD){
        lastWalkingForthTime = timer.elapsed();
        stopDistance1 = irSensorValueFar;
        //setLowGain();
        goNextState(); //-> lsTURNING180;
    }
}

static const int OFF_TO_THE_LEFT = 10; //pixels off the cplane's center
static const int OFF_TO_THE_RIGHT = -10;
static const int ALIGNING_MAXINUM_PERIOD = 30*4; //4 sec

//get the heading of a point in the image, relative to the neck. 
//Note that this doesn't take into account the head rotation.
//in degree
double getPointHeading(double x){
    double currentPanx = gInfoManager.getJointSensor(headPan);
    return currentPanx + PointToHeading(x);
}

int aligningToBeacon(){
    using namespace WalkingLearner;

    static int lastActionCounter = 0;
    static int counter = 0;
    if (lastActionCounter != actionCounter -1){ //reset
        counter = 0;
    }
    lastActionCounter = actionCounter;
    counter += 1;
    if (counter >= ALIGNING_MAXINUM_PERIOD)
        return FINISH;

    HalfBeacon pink;
    if (canSeeABeacon(pink)){
        setHeadTowardBeacon(pink);
        double off = getPointHeading(pink.cx);
        if (bDebugState)
            cout << "Aligning " << pink.cx << " " << off << endl;
        if (off < OFF_TO_THE_RIGHT)
            turnCCW = -TURNING_STEP;
        else if ( off > OFF_TO_THE_LEFT)
            turnCCW = TURNING_STEP;
        else 
            return FINISH;
        if (bDebugState)
            cout << turnCCW << endl;
    }
    else{
        if (bDebugState)
            cout << "Aligning to beacon : lost track of the beacon????" << endl;
    }
    return CONTINUE;
}

void WalkingLearner::preparation3() {
    theBeacon = highBeacon;
    int result = aligningToBeacon();
    if (result == FINISH){
        result = stayingStill();
        if (result == FINISH){
            if (learnUsingHighGain)
                setHighGain();
            else
                setLowGain();
            timer.start();
            goNextState(); //-> lsWALKING_BACK;
        }
    }
}

void WalkingLearner::preparation4() {
    static int lastActionCounter = 0;
    static int counter = 0;
    if (lastActionCounter != actionCounter -1 ){
        counter = 0;
    }
    lastActionCounter = actionCounter;
    counter += 1;

    //cout << irSensorValueFar << " " << stopDistance2 << endl;
    if (counter == STAYING_STILL_PERIOD){
        lastWalkingBackTime = timer.elapsed();
        stopDistance2 = irSensorValueFar;
        //setLowGain();
        goNextState(); //-> lsDONE;
    }
}

void WalkingLearner::doneLearning() {
	//learningState = lsIDLE;
    setStandParams();
	stopEverything();
	//cout << "DONE " << irSensorValueFar << " " << irSensorValue << endl;
	//	<< endl;
}

void adjustTurning(KimCommand &command) {
	istringstream ss(command.rawValue);
	ss >> TURNING_FRAMES;
	cout << "Turning frames = " << command.rawValue << endl;
}

void WalkingLearner::setWalkCalibration(){
    turnCCW = 30;
    walkType = wlParams.walkType;
}

void WalkingLearner::setWalkingStraightForward() {
    Behaviours::forward = forwardSpeed;	// FORWARD_FASTEST;
	Behaviours::left = 0;
	walkType = wlParams.walkType;
}

void WalkingLearner::goNextState() {
	cout << "From state " << stateToStr[learningState];
	learningState = nextState[learningState];
	cout << " to state " << stateToStr[learningState] << endl;
}

void WalkingLearner::stopEverything() {
	forward = 0;
	Behaviours::left = 0;
	turnCCW = 0;

	panx = 0;
	tilty = 0;
}

void WalkingLearner::setLearningState(const char *str){
    istringstream ss(str);
    int ls;
    ss >> ls;
    learningState = (LearningState) ls; //WARNING: error prone
    cout << "LearningState is set to " << stateToStr[learningState] << endl;
}

void WalkingLearner::setLearningParameters(const char *str){
    istringstream ss(str);
    if (bDebugParameters)
        cout << "Got SLP " << str << endl;
    switch (wlParams.walkType) {
    	case EllipseWalkWT:
    	forwardSpeed = FORWARD_FASTEST;
    	useTurn = false;
        wlParams.ep.read(ss);
    	cout << "Set SLP : " << endl;
        wlParams.ep.print();
		break;
		case SkellipticalWalkWT:
		useTurn = true;
		ss >> forwardSpeed;
		wlParams.sp.read(ss);
    		cout << "Set SLP: " << forwardSpeed << ", ";
		wlParams.sp.print(cout);
		cout << endl;
		break;
		case SkiWalkWT:
		useTurn = true;
		ss >> forwardSpeed;
		wlParams.skp.read(ss);
    		cout << "Set SLP: " << forwardSpeed << ", ";
		wlParams.skp.print(cout);
		cout << endl;
		break;
		default:
			cout << "Unknown walk type in WalkingLearner::setLearningParameters" << endl;
		break;
	}


    #ifndef OFFLINE
        transmission->sendLearningParams(wlParams);
    #endif //OFFLINE
}

//same function, different version, use in python
void WalkingLearner::setWalkLearningParameters(const char *paraStr){
    WalkLearningParams  wlParams;
    double forwardSpeed;
    istringstream ss(paraStr);
    cout << "Got SLP " << paraStr << endl;
    string type;
    ss >> type;
    if (type == "ELI"){
        wlParams.walkType = EllipseWalkWT;
    }
    else if (type == "SKE"){
        wlParams.walkType = SkellipticalWalkWT;
    } if (type == "SKI"){
        wlParams.walkType = SkiWalkWT;
    }
   
    switch (wlParams.walkType) {
        case EllipseWalkWT:
            wlParams.ep.read(ss);
            break;
        case SkellipticalWalkWT:
            ss >> forwardSpeed;
            wlParams.sp.read(ss);
            //wlParams.sp.print(cout);
            cout << "Ske forwardSpeed = " << forwardSpeed << endl;
            break;
        case SkiWalkWT:
            ss >> forwardSpeed;
            wlParams.skp.read(ss);
            //wlParams.skp.print(cout);
            cout << "Ski forwardSpeed = " << forwardSpeed << endl;
            break;
        default:
            cout << "Unknown walk type in WalkingLearner::setLearningParameters" << endl;
            break;
    }

#ifndef OFFLINE
    transmission->sendLearningParams(wlParams);
#endif //OFFLINE
}


//a decent way to get debugging information
void WalkingLearner::storeDebuggingInformation(){
	debugSS << " state: " << stateToStr[learningState] << endl;
    debugSS << " panx " << gInfoManager.getJointSensor(headPan) << endl;
    debugSS << " IR-near = " << irSensorValue << endl;
    debugSS << " IR-far = " << irSensorValueFar << endl;
    debugSS << " Walking : [" << forward << " " << Behaviours::left << " " << Behaviours::turnCCW << "]" << endl
        << " Head : [" << panx << " " << tilty << " " << cranez << "] " << endl;
    switch (wlParams.walkType) {
    	case EllipseWalkWT:
    		debugSS << " WalkParams : Ellipse [" << wlParams.ep.PG << " " << wlParams.ep.hf << " " << wlParams.ep.hb << " " << wlParams.ep.ffo << " " << wlParams.ep.fso << " " << wlParams.ep.bfo << " " << wlParams.ep.bso << " " << wlParams.ep.elpWidthFront << " " << wlParams.ep.elpHeightFront << " " << wlParams.ep.elpWidthBack << " " << wlParams.ep.elpHeightBack << "]" << endl;
		break;
		case SkellipticalWalkWT:
   			debugSS << " WalkParams : SkellipticalWalk [" << forwardSpeed << ", ";
			wlParams.sp.print(debugSS);
			debugSS << "]" << endl;
		break;
		case SkiWalkWT:
   			debugSS << " WalkParams : SkiWalk [" << forwardSpeed << ", ";
			wlParams.skp.print(debugSS);
			debugSS << "]" << endl;
		break;
		default:
			debugSS << "Unknown WalkType!!" << endl;
		break;
	}
    //debugSS << debugSS_1.str() << endl;
    //debugSS_1.str("");

    debugBuffer = debugSS.str(); //store in a buffer, not print out yet
    debugSS.str(""); //clear
}

