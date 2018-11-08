/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
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

#include "ted.h"
#include <iostream>
#include <sstream>
#include <iomanip>

static int panDir = 1;
static int frameID = 0;

void Ted::initTedForward() {}

bool crashDetection() {
    if (panx == 80)
        panDir = -1;
    if (panx == -80)
        panDir = 1;
        
    if (panDir == 1)         
        panx += 10;
    else
        panx -= 10;
        
    if (panx > 80 || panx < -80) {
        panx = 0;   
    }

    cout << "---------------" << endl;
    cout << "Panx: " << panx << endl;
    cout << "Tilty: " << tilty << endl;
    cout << "Cranez: " << cranez << endl;
    cout << "Far distance: "  <<  sensors->sensorVal[ssINFRARED_FAR]  << endl;
    cout << "Near distance: " <<  sensors->sensorVal[ssINFRARED_NEAR] << endl;

    headtype = ABS_H;   
    cranez = -15;
    tilty = -5;

    // 110000 is an experimental value.
    if (sensors->sensorVal[ssINFRARED_NEAR] < 110000)
        return true;
    else
        return false;
}

bool anyTeammateSeen() {
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vTeammate[i].cf > 0)
            return true;
    }
    return false;
}

bool anyTeammateNearSeen() {
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vTeammate[i].cf > 0 && vTeammate[i].d <= 55)
            return true;
    }
    return false;
}

void printAllTeammateDist() {
    for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
        if (vTeammate[i].cf > 0) {
            cout << "Distance: " << vTeammate[i].d << endl;            
        }
    }
}

void printGpsBall() {
    cout << "gps->getBall(GLOBAL).x: " << gps->getBall(GLOBAL).x << endl;
	cout <<	"gps->getBall(GLOBAL).y: " << gps->getBall(GLOBAL).y << endl;
    cout << "gps->getBall(LOCAL).x: " << gps->getBall(LOCAL).x << endl;
	cout <<	"gps->getBall(LOCAL).y: " << gps->getBall(LOCAL).y << endl;
    cout << "gps->getBall(GLOBAL).head: " << gps->getBall(GLOBAL).head << endl;
    cout << "gps->getBall(GLOBAL).d: " << gps->getBall(GLOBAL).d << endl;
    cout << "gps->getBall(LOCAL).head: " << gps->getBall(LOCAL).head << endl;
    cout << "gps->getBall(LOCAL).d: " << gps->getBall(LOCAL).d << endl;
}

void printTeamRobot() {
    cout << "##########################" << endl;
    cout << "vBall.cf: " << vBall.cf << endl;
    if (anyTeammateSeen()) {
        cout << "Seen teammate!" << endl;
        printAllTeammateDist();
    }
    else
        cout << "Not seen teammate!" << endl;
}

using namespace UNSW2004;

static int stuckDetectTimer = 0;
static int stuckDetectAlarm = 0;
static int minOppDistLeft   = 500000;
static int minOppDistRight  = 500000;
static bool StuckKneeRelax  = false;


/*
void lookAround() {
    using Behaviours::left;

    panx  = cranez = tilty = 0;
    int sendist = sensors->sensorVal[ssINFRARED_NEAR];

    if (!stuckDetectAlarm)
        stuckDetectTimer++;
        
    for (int i = 0; i < NUM_TEAM_MEMBER && vTeammate[i].cf > 0; i++) {
        if (hPan < 0 && sendist < minOppDistRight)
            minOppDistRight = sendist;
        if (hPan > 0 && sendist < minOppDistLeft)
            minOppDistLeft  = sendist;    
    }
    
    if (stuckDetectTimer < 20 && vBall.cf > 0 && abs(vBall.h) < 20) {
        panx = -90;
        cranez = -20;
        tilty = -30;
        headtype = ABS_H;    
    }
    if (stuckDetectTimer >= 20 && stuckDetectTimer < 40 && vBall.cf > 0 && abs(vBall.h) < 20) {
        panx = 90;
        cranez = -20;
        tilty = -30;
        headtype = ABS_H;    
    }
    if (stuckDetectTimer == 40) {
        stuckDetectTimer = 0;
        stuckDetectTimer = 0; 
        stuckDetectAlarm = 100;
        minOppDistRight  = 500000;
        minOppDistLeft   = 500000;    
    }
    if ( minOppDistRight < 100000 ) { 
        left += 2;
        // forward = 0;
    }
    if ( minOppDistLeft  < 100000 )  { 
        left -= 2;
        // forward = 0;
    }
    if ( minOppDistRight < 100000 &&  minOppDistLeft  < 100000 ) { forward -= 2; }
   
      return;
}
*/
/*
void lookRight() {
    headtype = ABS_H;
    panx = -90;
    tilty = 0;
    cranez = -20;
    if (anyTeammateNearSeen())
        cout << "Right has some obejcts!" << endl;
}

void lookLeft() {
    headtype = ABS_H;
    panx = 90;
    tilty = 0;
    cranez = -20;
    if (anyTeammateNearSeen())
        cout << "Left has some objects!" << endl;
}

void lookForward() {
    headtype = ABS_H;
    panx = 0;
    tilty = 0;
    cranez = 0;
}

void resetLookAround() {
    timer = 0;
    foundRight   = false;
    foundLeft    = false;
    foundForward = false;
}
*/

int timer = 0;

void testIsBallUnderChin() {
    if (isUNSW2004BallUnderChin())
        cout << " There is a ball under chin!" << endl;
    else
        cout << " No ball under chin!" << endl;
}

// -1 left, 0 forward, 1 right
void tedLookAround() {
    if (lockMode == LookAround)
        UNSW2004::lookAround();
    timer++;
    if (timer == 300) {
        timer = 0;
        resetLookAround();
        UNSW2004::lookAround();        
    }
    return;        

    /*
    timer++;
    
    if (timer < 50) {
        lookRight();
    }
    if (timer >= 50 && timer <= 100) {
        lookLeft();
    }
    if (timer > 100) {
        lookForward();
    }   
   
    if (timer == 150) {
        timer = 0;
    }
    */    
    /*    
    panx  = cranez = tilty = 0;
    int sendist = sensors->sensorVal[ssINFRARED_NEAR];

    if (!stuckDetectAlarm)
        stuckDetectTimer++;
        
    for (int i = 0; i < NUM_TEAM_MEMBER && vTeammate[i].cf > 0; i++) {
        if (hPan < 0 && sendist < minOppDistRight)
            minOppDistRight = sendist;
        if (hPan > 0 && sendist < minOppDistLeft)
            minOppDistLeft  = sendist;    
    }
    
    if (stuckDetectTimer < 20 && vBall.cf > 0 && abs(vBall.h) < 20) {
        panx = -90;
        cranez = -20;
        tilty = -30;
        headtype = ABS_H;    
    }
    if (stuckDetectTimer >= 20 && stuckDetectTimer < 40 && vBall.cf > 0 && abs(vBall.h) < 20) {
        panx = 90;
        cranez = -20;
        tilty = -30;
        headtype = ABS_H;    
    }
    if (stuckDetectTimer == 40) {
        stuckDetectTimer = 0;
        stuckDetectTimer = 0; 
        stuckDetectAlarm = 100;
        minOppDistRight  = 500000;
        minOppDistLeft   = 500000;    
    }
    if ( minOppDistRight < 100000 ) { 
        left += 2;
        // forward = 0;
    }
    if ( minOppDistLeft  < 100000 )  { 
        left -= 2;
        // forward = 0;
    }
    if ( minOppDistRight < 100000 &&  minOppDistLeft  < 100000 ) { forward -= 2; }
   
      return;
   */
}

void resetHead() {
    headtype = ABS_H;
    panx = 0;
    tilty = 0;
    cranez = 0;
}

int lookAroundC = 100;
bool stopForever = false;

int counter = 0;


void Ted::tedDetermineBallSource() {    
    /*
    using namespace Behaviours;
    if (vBall.cf > 0) {
        ballX = vBall.x;
        ballY = vBall.y;
    }
    else {
        ballX = -1;
        ballY = -1;
        cout << "C++: can't seen the ball!" << endl;
    } 
    */   
}

void Ted::printVariables() {
    cout << "vBall.x: " << vBall.x << endl;
    cout << "vBall.y: " << vBall.y << endl;

}

void Ted::doStriker() {
    double x, y;
    UNSW2004::getUNSW2004StrikerPoint(&x, &y);
    cout << "C++: strikeX = " << x << endl;
    cout << "C++: strikeY = " << y << endl;
    UNSW2004::doUNSW2004StrikerForward(x, y);
    cout << "C++: forward = " << forward << endl;
    cout << "C++: left = " << Behaviours::left << endl;
    cout << "C++: turnCCW = " << Behaviours::turnCCW << endl;
    forward = 0;
    Behaviours::left = 0;
    turnCCW = 0;
}


void Ted::doTedForward() {
    if (vBall.cf > 0) {
        UNSW2004::doUNSW2004TrackVisualBall();
    }
    printGpsBall();
    return;
    UNSW2004::determineBallSource();
    if (vBall.cf > 0) {
        UNSW2004::doUNSW2004TrackVisualBall();
        doStriker();
    }
    else
        cout << "No ball seen()" << endl;
    return;

    cout << "doTedForward() lockmode: " << lockMode << endl;
    if (lockMode == UpennRightWT || lockMode == UpennLeftWT)
        cout << "UPenn Walk" << endl;
    if (lockMode == ProperVariableTurnKick)
        cout << "Proper Variable!" << endl;   
    counter++;
    if (lockMode == UpennRightWT) {
        aaUPkickRight();    
        return;
    }    
    if (lockMode == UpennLeftWT) {
        aaUPkickLeft();
        return;    
    }
    if (lockMode == ProperVariableTurnKick) {
        cout << "lockMode == ProperVariableTurnKick!" << endl;
        aaProperVariableTurnKick();
        return;
    }        
    if (counter == 1) {
        lockMode = ProperVariableTurnKick;
        setProperVariableTurnKick(90);
        aaProperVariableTurnKick();
    }
    return;



    cout << "#################################" << endl;
    if (stopForever) {
        turnCCW = 0; Behaviours::left = 0; forward = 0;
        return;
    }

    if (vBall.cf > 0) {
        UNSW2004::hoverToBall(vBall.d, vBall.h);
        UNSW2004::doUNSW2004TrackVisualBall();
        if (UNSW2004::isUNSW2004BallUnderChin()) {
            stopForever = true;    
        }
    }
    else {
        cout << "Stop()!" << endl;
        setStandParams();
        turnCCW = 0; forward = 0; Behaviours::left = 0;
    }



    return;

    testIsBallUnderChin();
    
    return;

    tedLookAround();
/*
    lookAroundC--;
    if (lookAroundC == 0) {
        // lookAroundC = 100;    
        lookAround();
    }
*/
//    else
//        resetHead();
    return;

    
    headFindBall(-55);
    return;


    printTeamRobot();
    return;










    cout << "#################################################" << endl;
    double headingToTm, retX, retY;

//    cout << vision->vob[vobRedDog].d << endl;
//    cout << vision->vob[vobRedDog2].d << endl;
//    cout << vision->vob[vobRedDog3].d << endl;        
//    cout << vision->vob[vobRedDog4].d << endl;            
    cout << vTeammate[0].var << endl;
    cout << vTeammate[1].var << endl;
    cout << vTeammate[2].var << endl;
    cout << vTeammate[3].var << endl;


return;
    bool backOff = true;

    if (backOff) {
        back_front_color = IND_LED3_INTENSITY_MAX;    
        back_front_white = IND_LED3_INTENSITY_MAX;        
        cout << "The backoff robot x: " << retX << endl;
        cout << "Go backoff!" << endl;
    }
    else {
        back_front_color = IND_LED3_INTENSITY_MIN;    
        back_front_white = IND_LED3_INTENSITY_MIN;    
        cout << "NoNoNo backoff!" << endl;
    }

    return;
        
    if (frameID <= 500)
        led1 = IND_LED3_INTENSITY_MAX;
    else
        led1 = IND_LED3_INTENSITY_MIN;

    return;

        if (frameID <= 100)
            frameID++;
        
        if (frameID <= 1) {
            resetActiveLocalise();
            setNewBeacons(LARGE_VAL, 100);
            activeGpsLocalise(true);
        }
}   



