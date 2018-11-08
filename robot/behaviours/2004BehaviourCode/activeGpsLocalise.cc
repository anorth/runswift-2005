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
 * $Id: activeGpsLocalise.cc 5072 2005-03-10 22:59:09Z alexn $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/
/*
 * Strategy:
 *   aaActiveGpsLocalise
 *
 * Description:
 *  looks to closest beacons and freezes on them for fixed number of frames each
 *
 * Strategy member variables used:
 *  tilt -
 *    used to set tilt to current beacon
 *  pan -
 *    used to set pan to current beacon
 *
 * Local variables used:
 *  activeGpsLocaliseObjects -
 *    array of beacons that may be viewed in increasing pan order
 *  activeGpsLocaliseNumObj -
 *    size of above array
 *  activeGpsLocaliseCurObjInd -
 *    index in above array we are currently trying to look at
 *  activeGpsLocaliseCounter -
 *    counter mainly used for keeping track of how long we have been looking for current beacon
 *  activeGpsLocaliseIndexChange -
 *    amount above index changes by (usually 1 or -1)
 *  activeGpsLocaliseMode -
 *    whether we are looking for specific beacons or scanning horizon
 *  activeGpsLocaliseSeenBeacon -
 *    used to determine if we have seen current beacon or not
 *
 * Restrictions:
 *  setNewBeacons(double) -
 *    call this to set beacons we want to look at (& to reset those beacons)
 *
 * Comments:
 *  
 */

#include "Behaviours.h"

// Added objCranez by Ted Wong. But this is not used for the moment.
struct ViewObject {
	int visIndex;
	double objPan;
	double objTilt;
    double objCranez;
};

static const double BEACON_HEIGHT = 30;
static const int ACTIVE_LOC_MODE = 1;
static const int STATIONARY_LOC_MODE = 2;
static const int NUM_FRAMES_ACT_LOC = 8;
static const int NUM_FRAMES_STAT_LOC = 65;

// Objects we can look at as indexed in visual cortex.
static ViewObject *activeGpsLocaliseObjects;
static int activeGpsLocaliseNumObj = 0;
static int activeGpsLocaliseCurObjInd = 0;

// Number of times called since trying to find current object.
static int activeGpsLocaliseCounter = 0;
static int activeGpsLocaliseIndexChange = 1;

// Mode we are currently in.
static int activeGpsLocaliseMode = ACTIVE_LOC_MODE;
static bool activeGpsLocaliseSeenBeacon = false;

// Debug variable used by Ted Wong.
static bool gpsDebug = false;

// Scan localise variable
static double scanPan;

void Behaviours::activeGpsLocalise(bool useSlowPan) {
	if (activeGpsLocaliseMode == ACTIVE_LOC_MODE) {
        if (gpsDebug)
            cout << "Active_Loc_Mode" << endl;
                    
        // If there is no beacon from gps to look.        
		if (activeGpsLocaliseNumObj == 0) {
            cout << "The robot unable to find a localised beacon at all!" << endl;
			return;
        }   
         
		// Check to see if we can't currently see the desired beacon.
		if (!(gps->canSee(activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].visIndex)) && useSlowPan) {
            if (gpsDebug) {
                cout << "The values couldn't be used!" << endl;
            }
			if (activeGpsLocaliseCounter == NUM_FRAMES_ACT_LOC) {
				if (!activeGpsLocaliseSeenBeacon) {
					// We haven't seen beacon at all - change to scan mode.
					// cout << "can't see object " << activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].visIndex;
					// cout << " at pan=" << activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].objPan;
					// cout << " tilt=" << activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].objTilt << endl;
					// cout << "view = " << (*gps).view << endl;
					activeGpsLocaliseMode = STATIONARY_LOC_MODE;
					activeGpsLocaliseCounter = 0;
					StationaryLocalize();
					return;
				}
			}
		}
		else
			activeGpsLocaliseSeenBeacon = true;

		if (activeGpsLocaliseCounter >= NUM_FRAMES_ACT_LOC) {
			// Time to change beacon we want to look at.
			activeGpsLocaliseCurObjInd += activeGpsLocaliseIndexChange;
			activeGpsLocaliseSeenBeacon = false;
			if (activeGpsLocaliseCurObjInd == activeGpsLocaliseNumObj) {
				// Gone to far to the left.
				activeGpsLocaliseIndexChange = -1;
				activeGpsLocaliseCurObjInd = activeGpsLocaliseNumObj - 1;
			}
			else if (activeGpsLocaliseCurObjInd < 0) {
				// Gone to far to the right.
				activeGpsLocaliseIndexChange = 1;
				activeGpsLocaliseCurObjInd = 0;
			}
			activeGpsLocaliseCounter = 0;
		}
		else
			activeGpsLocaliseCounter++;

        if (gpsDebug) {
		    cout << "Looking at " << activeGpsLocaliseCurObjInd <<  " beacon";
        }

		// Look in desired direction.
        headtype = ABS_H;
		panx = activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].objPan;
        tilty = 0;
        cranez = activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].objTilt + RAD2DEG(offsetValue);
        
        /*      2003 Implementation. Keep here just for reference. Ted Wong.
        
		    headtype = ABS_XYZ;
		    panx = activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].
            tilty = activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].objTilt + RAD2DEG(offsetValue);
        */
	}
	else {
        if (gpsDebug)
            cout << "Stationary_loc_mode" << endl;

		// STAIONARY_LOC_MODE
		// We want to scan the horizon randomly.
		if (activeGpsLocaliseCounter >= NUM_FRAMES_STAT_LOC) {
			// Time to go back to specific beacon tracking.
			tail_h = IND_TAIL_H_CENTRED;
			activeGpsLocaliseMode = ACTIVE_LOC_MODE;
			setNewBeacons(VERY_LARGE_INT);
		}
		else {
			if ((activeGpsLocaliseCounter % 4) / 2 == 0) {
				tail_h = IND_TAIL_LEFT;
			}
			else {
				tail_h = IND_TAIL_RIGHT;
			}
			activeGpsLocaliseCounter++;
			StationaryLocalize();
		}
	}
}

//static int 
static bool debugSetNewBeacons = false;

// Set beacon information. PAN_LIMIT is 100.
void Behaviours::setNewBeacons(double maxDist, double panLimit, bool sortByPan) {
    if (debugSetNewBeacons)
        cout << "setNewBeacons() is called!" << endl;

	// Our position on the field.
	double x = gps->self().pos.x;
	double y = gps->self().pos.y;
	double h = gps->self().h;

	// The visualCortex index of the beacon we are currently looking for.
	int origID = -1;
	if (activeGpsLocaliseNumObj != 0) {
        if (debugSetNewBeacons)
            cout << "Here1" << endl;
		origID = activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].visIndex;
	}

	if (activeGpsLocaliseNumObj > 0) {
        if (debugSetNewBeacons)
            cout << "Here2" << endl;
	    	delete[] activeGpsLocaliseObjects;
	}

    if (debugSetNewBeacons)
        cout << "Here3" << endl;

    if (debugSetNewBeacons && origID == 2)
        cout << "Oh no, orgID == 2 case!" << endl;
        
    if (debugSetNewBeacons && origID == 3)
        cout << "Oh no, orgID == 3 case!" << endl;

	activeGpsLocaliseNumObj = 0;
	// activeGpsLocaliseMode = ACTIVE_LOC_MODE;

	// Represents the pan angle to all the beacons.
	double beaconPans[6] = {
		0, 0, 0, 0, 0, 0
	};
    
	for (int i = 0 ; i < 6 ; i++) {
		// VisualCortex index of beacon.
		int ind = i + 2;
		beaconPans[i] = NormalizeAngle_180(RAD2DEG(atan2(((*(*gps).coord)[ind][1]) - y, ((*(*gps).coord)[ind][0]) - x)) - h);
		double dist = sqrt(SQUARE(((*(*gps).coord)[ind][0]) - x) + SQUARE(((*(*gps).coord)[ind][1]) - y));

		// Is pan direction out of range?
        // Hack added by Ted Wong. 18.3.2004
        // Since the new robots can't see the middle beacons. Set the middle beacon values to VERY_LARGE_INT.
        // Of course it would be better to modify the index of beacon in vision, but too much time would be
        // wasted ..... Anyway the blue-top/pink is 0, green-top/pink is 2, yellow-top/pink is 4,
        // pink-top/blue is 1, pink-top/green is 3 and pink-top/yellow is 5.     
		if (ABS(beaconPans[i]) < panLimit && dist < maxDist && i != 2 && i != 3)
			activeGpsLocaliseNumObj++;
		else
			beaconPans[i] = VERY_LARGE_INT;

        if (debugSetNewBeacons) {
            cout << i << " " << beaconPans[i] << endl;
        }
	}

    if (debugSetNewBeacons)
        cout << "Number of beacons within the pan limit: " << activeGpsLocaliseNumObj << endl;

	// Create array.
	activeGpsLocaliseObjects = new ViewObject[activeGpsLocaliseNumObj];
	int firstInd = -1;

	// Sort into array. The sorted array is activeGpsLocaliseObjects in pan order.
	for (int i = 0; i < activeGpsLocaliseNumObj; i++) {
		int curInd = 0;
        
		for (int j = 1 ; j < 6 ; j++) {
			// if sort by pan
			if (sortByPan) {
				if (beaconPans[j] < beaconPans[curInd])
					curInd = j;
			}
			else {
				// Note:
				// initActiveLocaliseBeaconData needs to be called upon initisaliation
				// updateActiveLocaliseBeaconData needs to be called every frame.
				if (numFramesBeaconSeenLately(j) < numFramesBeaconSeenLately(curInd))
					curInd = j;
			}
		}
        
        // vobMinFixed = 1. visIndex is really equal to curInd.
		activeGpsLocaliseObjects[i].visIndex = curInd + vobMinFixed + 2;
		activeGpsLocaliseObjects[i].objPan = beaconPans[curInd];
		double dist = sqrt(SQUARE(((*(*gps).coord)[curInd + 2][0]) - x) + SQUARE(((*(*gps).coord)[curInd + 2][1]) - y));
		activeGpsLocaliseObjects[i].objTilt = RAD2DEG(atan2(BEACON_HEIGHT, dist));
        // activeGpsLocaliseObjects[i].objCranez = ....
        
        if (debugSetNewBeacons) {
            cout << "curInd is: " << curInd << " and dist is: " << dist << endl;        
        }
        
    	// We have found the beacon we are currently looking for.
	    if (activeGpsLocaliseObjects[i].visIndex == origID) {
		    if (debugSetNewBeacons)
                cout << "The origID is: " << origID << " and i is: " << i << endl;
            firstInd = i;
        }

		beaconPans[curInd] = VERY_LARGE_INT;
	}

	// We didn't find the beacon we are currently looking for - must look for next best beacon to track.
	if (firstInd == -1) {
        if (debugSetNewBeacons)
            cout << "firstInd == -1 has been called!" << endl;
        // Search from negative to positive pan values beacon.
		for (int i = 0 ; i < activeGpsLocaliseNumObj ; i++) {
			if (activeGpsLocaliseIndexChange > 1) {
                if (debugSetNewBeacons)
                    cout << "Here4" << endl;
				double diff = activeGpsLocaliseObjects[i].objPan - vision->pan;
				if (diff > 0 && ((firstInd == -1) || (diff < (activeGpsLocaliseObjects[firstInd].objPan - vision->pan))))
					firstInd = i;
			}
			else {
                if (debugSetNewBeacons)
                    cout << "Here5" << endl;
				double diff = vision->pan - activeGpsLocaliseObjects[i].objPan;
				if (diff > 0 && ((firstInd == -1) || (diff < (vision->pan - activeGpsLocaliseObjects[firstInd].objPan)))) {
                    if (debugSetNewBeacons) {
                        cout << "Here6" << endl;
                        cout << "diff = " << diff << endl;
                        cout << "vision->pan = " << vision->pan << endl;
                    }   
					firstInd = i;
                }
			}
		}
		if (firstInd == -1) {
            if (debugSetNewBeacons)
                cout << "Here7" << endl;
			if (activeGpsLocaliseIndexChange > 1)
				firstInd = activeGpsLocaliseNumObj - 1;
			else
				firstInd = 0;
		}
		activeGpsLocaliseCounter = 0;
		activeGpsLocaliseSeenBeacon = false;
	}
    else {
        if(debugSetNewBeacons)
            cout << "firstInd == -1 didn't been called!" << endl;
    }    

	activeGpsLocaliseCurObjInd = firstInd;

    if (debugSetNewBeacons) {
        cout << vision->pan << endl;
        cout << activeGpsLocaliseCurObjInd << endl;
    }
    
    /*  Commented by Ted Wong. Useless stuff.   
        if (firstInd < 0 || firstInd > 5 || firstInd == 2 || firstInd == 3) {
            cout << "Help! setNewBeacons() in activeGpsLocalise.cc is crashing!" << endl;
            cout << "The stupid firstInd is: " << firstInd << endl;
        }
    */
    
    // Crash if the robot wants to see the two middle beacons.
    if (activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].visIndex == vobMinFixed + 2 + 2 ||
        activeGpsLocaliseObjects[activeGpsLocaliseCurObjInd].visIndex == vobMinFixed + 2 + 3) {
        cout << "Help! setNewBeacons() in activeGpsLocalise.cc is crashing!" << endl;
        cout << "The stupid firstInd is: " << firstInd << endl;        
    }
    else {
        if (debugSetNewBeacons)
            cout << "No crashing!" << endl;
    }
}

///////////////////////////////////////////////////////////;
static const int OPLANE_INTERVAL = 2;

// Start active localise trigger helper functions
///////////////////////////////////////////////////////////

static const int NUM_FRAMES_TO_COUNT = 120;
static const int NUM_OF_BEACONS = 6;

static bool beaconData[NUM_FRAMES_TO_COUNT][NUM_OF_BEACONS];

void Behaviours::initActiveLocaliseBeaconData() {
	for (int i = 0; i < NUM_FRAMES_TO_COUNT; i++) {
		for (int j = 0; j < NUM_OF_BEACONS; j++) {
			beaconData[i][j] = false;
		}
	}
}    

void Behaviours::updateActiveLocaliseBeaconData() {
	for (int i = 1; i < NUM_FRAMES_TO_COUNT; i++) {
		for (int j = 0; j < NUM_OF_BEACONS; j++) {
			beaconData[i - 1][j] = beaconData[i][j];
		}
	}

	for (int j = 0; j < NUM_OF_BEACONS; j++) {
		if (vision->vob[j + vobBlueLeftBeacon].cf > 0)
			beaconData[NUM_OF_BEACONS - 1][j] = true;
		else
			beaconData[NUM_OF_BEACONS - 1][j] = false;;
	}
}

// Precondition: initActiveLocaliseBeaconData is called upon initilisation & 
// updateActiveLocaliseBeaconData is called every frame.
bool Behaviours::hasSeenTwoBeaconsLately() {
	int numOfBeaconsSeen = 0;
	for (int i = 0; i < NUM_OF_BEACONS; i++) {
		for (int j = 0; j < NUM_FRAMES_TO_COUNT; j++) {
			if (beaconData[j][i]) {
				numOfBeaconsSeen++;
				break;
			}
		}
	}    
	cout << "numBeaconsSeen: " << numOfBeaconsSeen << endl;
	if (numOfBeaconsSeen >= 2) {
		return true;
	};
	static const int OPLANE_INTERVAL = 2;

	return false;
}

// Precondition: initActiveLocaliseBeaconData is called upon initilisation & 
// updateActiveLocaliseBeaconData is called every frame.
int Behaviours::numFramesBeaconSeenLately(int beaconIndex) {
	int numOfFramesSeen = 0;
	for (int i = 0; i < NUM_FRAMES_TO_COUNT; i++) {
		if (beaconData[i][beaconIndex]) {
			numOfFramesSeen++;
		}
	}    
	return numOfFramesSeen;
}    

bool Behaviours::canSeeABeacon() {
	for (int i = vobBlueLeftBeacon; i < vobYellowRightBeacon; i++) {
		if (vision->vob[i].cf > 0) {
			return true;
		}
	}    
	return false;
}    

void Behaviours::printBeaconsSeenLately() {
	cout << "#############################" << endl;
	for (int i = 0; i < 6; i++) {
		int num = 0;
		for (int j = 0; j < NUM_FRAMES_TO_COUNT; j++) {
			if (beaconData[j][i])
				num++;
		}    
		cout << "beacon num: " << i << ", numSeen: " << num << endl;
	}
	cout << "#############################" << endl;
}    

///////////////////////////////////////////////////////////
// End active localise trigger helper functions
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// New active localisation, should be smarter
///////////////////////////////////////////////////////////

static int currentVisualIndex = -1;
static bool debugSmart = false;

void Behaviours::smartActiveGpsLocalise() {
    if (debugSmart)
        cout << "smartActiveGpsLocalise()" << endl;
    
	const WMObj &self = gps->self();

	if (currentVisualIndex > 1) {
		double xloc = (*(*gps).coord)[currentVisualIndex][0] - self.pos.x;
		double yloc = (*(*gps).coord)[currentVisualIndex][1] - self.pos.y;

		Vector v;
		v.setVector(vCART, xloc, yloc);
		v.rotate(90 - self.h);
		headtype = ABS_XYZ;
		panx = -v.x;
        tilty = BEACON_HEIGHT;
		cranez = v.y;
	} else {
        scanPan = hPan;
        scanLocalise();
    }
}

void Behaviours::smartSetBeacon(double maxDist, double panLimit) {
    if (debugSmart)
        cout << "smartSetBeacon()" << endl;

	const WMObj &self = gps->self();

	currentVisualIndex = -1;

	const MMatrix3 &cov = gps->selfCov();
	double b = -(cov(0, 0) + cov(1, 1));       //-(varx+vary)
	double c = cov(0, 0) * cov(1, 1) - cov(0, 1) * cov(0, 1); //varx*vary-varxy^2
	double largeEVal;
    
    // If position or heading variance is too large, smart localise
    //  should not be used.
    if (self.hVar < 5E3) {
    
        // look out for very small numbers
        if (b * b - 4 * c < 1E-5) {
            largeEVal = (-b + 0) / 2;
            if (b * b - 4 * c < 0) {
                cout << "WARNING: b*b-4*c < 0 (location activeLocalise): "
                    << (b * b - 4 * c) << endl;
            }
        } else largeEVal = (-b + sqrt(b * b - 4 * c)) / 2;

        double head;
        if (abs(cov(0, 0) - largeEVal) < 0.01 && abs(cov(0, 1)) < 0.01) {
            // first row is zeroes
            if (abs(cov(1, 0)) < 0.01 && abs(cov(1, 1) - largeEVal) < 0.01)
			// second row zeroes - circular variance
                head = self.h;
            else head = RAD2DEG(atan2(-cov(1, 0), (cov(1, 1) - largeEVal)));
        
        } else head = RAD2DEG(atan2(-(cov(0, 0) - largeEVal), cov(0, 1)));
        
        double currentCompHead = VERY_LARGE_INT;
        for (int i = 0 ; i < 6 ; i++) {
            // Skip the middle beacons.
            if (i == 2 || i == 3) continue;

            // gps index of beacon.
            int ind = i + 2;

            double xloc = (*(*gps).coord)[ind][0] - self.pos.x;
            double yloc = (*(*gps).coord)[ind][1] - self.pos.y;

            double locHead = NormalizeAngle_180(RAD2DEG(atan2(yloc, xloc)) - self.h);

            if (abs(locHead) > panLimit) continue;

            double dist = sqrt(SQUARE(xloc) + SQUARE(yloc));
            if (dist > maxDist) continue;

            double globalHead = locHead + self.h;
            double compHead = abs(NormalizeAngle_180(head - globalHead));
        
            if (compHead > 90.0) compHead = 180.0 - compHead;

            DER_DEBUG(
                cout << "Beacon " << i << " has compHead " << compHead << "\n";
            )

            if (compHead < currentCompHead) {
                currentCompHead = compHead;
                currentVisualIndex = ind;
            }
        }
        
        DER_DEBUG(
            if (currentVisualIndex <= 1)
                cout << "smartSetBeacon: I don't know which beacon to look at!\n";
            else
                cout << "smartSetBeacon: I'll look at beacon " << currentVisualIndex << "\n";
        )
    } else scanPan = -scanPan;
}

// Essentially just like stationary localise, except this doesn't
//  have a time limit
void Behaviours::scanLocalise() {
    if (ABS(hPan - scanPan) <= 10) {
        scanPan = (scanPan < 0) ? 90 : -90;
	}

	headtype = REL_H;
	panx = (scanPan > 0) ? 5 : -5;
	tilty =(10 - hTilt) / 3.0;
	cranez = -hCrane / 3.0;
}
