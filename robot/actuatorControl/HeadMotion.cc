/*

   Copyright 2004 The University of New South Wales(UNSW) and National  
   ICT Australia(NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or(at your option) any later version as  
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
 * $Id: HeadMotion.cc 6483 2005-06-08 10:25:35Z nmor250 $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 * 
 **/

#include "HeadMotion.h"
#include "JointLimit.h"
#include "HeadKinematics.h"

static const bool bDebugHeadMotion = false;

HeadMotion::HeadMotion( double *dPan, double *dTilt, double *dCrane, const double *fsh, const double *bt) {
	desiredPan = dPan;  
	desiredTilt = dTilt;
	desiredCrane = dCrane;
	frontShoulderHeight = fsh;
	bodyTilt = bt;
}

void HeadMotion::SendHeadCommand(int headType, double panx, double tilty, double cranez) {
    if (bDebugHeadMotion)
        cout << "HeadMotion: " << headTypeStr[headType] << " " << panx << " " << tilty << " " << cranez << endl;
	double d, x, y, z;
	switch (headType) {
        case NULL_H:
            break;

        case REL_H: 
            destTilt += tilty;
            destPan += panx;
            destCrane += cranez;
            break;

        case ABS_H: 
            destTilt = tilty;
            destPan = panx;
            destCrane = cranez;
            break;

        /* Make the dog's head point to a remote object, that object has panx,
        ** tilty angle RELATIVE TO THE DOG's head. */
        case ABS_PT: 
            d = 100 * cos(DEG2RAD(tilty));
            x = d * sin(DEG2RAD(panx));
            y = 100 * sin(DEG2RAD(tilty)) + NECKBASE_HEIGHT;
            z = d * cos(DEG2RAD(panx));
            setDestVarsXYZ(x, y, z, destTilt, destPan, destCrane, *frontShoulderHeight, *bodyTilt);
            break;

        /* Given coordinates with respect to the base of the ground, calcualte
         * desired p/t/c of the dog. (panx is the x axis (left/right), tilty 
         * is the up axis and cranez is the forward axis.*/
        case ABS_XYZ: 
            x = panx;
            y = tilty;
            z = cranez;
            setDestVarsXYZ(x, y, z, destTilt, destPan, destCrane, *frontShoulderHeight, *bodyTilt);
            break;
        
        case ABS_XYZ_HACK:
            x = panx;
            y = tilty;
            z = cranez;
            setDestVarsXYZHack(x, y, z, destTilt, destPan, destCrane, *frontShoulderHeight, *bodyTilt);
            break;
            
        /* Made by kim, absolute value for pan/tilt, relative value for
        ** crane.*/
        case REL_Z:
            destPan = panx;
            destTilt = tilty;
            destCrane += cranez;
            break;

        default:
            cout << "WARNING: unknown headType :" << headType << endl;
            break;

    }

    // Max values from spec
    if (destTilt < MIN_TILT) {
        destTilt = MIN_TILT;
	}
	if (destTilt > MAX_TILT) {
		destTilt = MAX_TILT;
	}

	if (destPan < MIN_PAN) {
		destPan = MIN_PAN;
	}
	if (destPan > MAX_PAN) {
		destPan = MAX_PAN;
	}

	if (destCrane < MIN_CRANE) {
		destCrane = MIN_CRANE;
	}
	if (destCrane > MAX_CRANE) {
		destCrane = MAX_CRANE;
	}

    if (bDebugHeadMotion)
        cout << "Head moving :  " << destPan<< " " << destTilt << " " << destCrane << endl;
	*desiredTilt = destTilt;
	*desiredPan = destPan;
	*desiredCrane = destCrane;

}

void HeadMotion::setCurrentJointCommand(JointCommand &jointCommand){
    jointCommand.setJointEnumValue(headPan, DEG2MICRO(destPan) );
    jointCommand.setJointEnumValue(headTilt , DEG2MICRO(destTilt) );
    jointCommand.setJointEnumValue(headCrane , DEG2MICRO(destCrane) );
}

