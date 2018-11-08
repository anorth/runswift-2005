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
 * $Id: EffectorCommander.h 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 * 
 * The lowest level of the locomotion module that sends values to the effectors.
 **/

#ifndef _EffectorCommander_h_DEFINED
#define _EffectorCommander_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "../share/Common.h"

static const int numberOfJoints = 21;

struct PRMInfo {
	enum {
		UNKNOWN,            // used only by sentry
		JOINT2,
		JOINT3,
		LED2
	}   jointType;

	const char *locator;
	int pgain, igain, dgain;
};

enum PrimitiveIndex {
	HeadTilt,
	HeadPan,
	HeadTilt2,
	Mouth,
	FrontRightLeg1,
	FrontRightLeg2,
	FrontRightLeg3,
	FrontLeft1,
	FrontLeft2,
	FrontLeft3,
	RearRight1,
	RearRight2,
	RearRight3,
	RearLeft1,
	RearLeft2,
	RearLeft3,
	TailTilt,
	TailPan,
	LeftEar,
	RightEar
};

class EffectorCommander : public OObject {
	public:
		EffectorCommander();
		~EffectorCommander();

		//  OSubject
		OSubject *sEffector;

		//  OPENR methods.
		OStatus init(OSubject *subject);
		OStatus start();
		OStatus stop();

		void continueInit(const OReadyEvent &);
		bool isGainSet();
		bool areActive();
		void relaxJoints();
		void reEnableJoints();
		void highGainJoints();
		void setJointGain(int whichJoint, int P , int I , int D , bool highGain);
		OSubject *getSubject();

		OPrimitiveID *getPrimID(PrimitiveIndex i);
		OStatus moveLegs();

	private:
		bool active_;
		RCRegion *region_;
		bool gainSet;

		OStatus InitPrimitives(void);
		OStatus InitCommandVectorData(void);
		bool InitJointGain(void);
		OPrimitiveID primitiveId[numberOfJoints];    // This is not initialized in prims[].
};

/*
 * Returns true if all joint gains are set, false otherwise
 */
inline bool EffectorCommander::isGainSet() {
	return gainSet;
}

/**
 * Returns the effector subject usesd for the initialisation
 */
inline OSubject * EffectorCommander::getSubject() {
	return sEffector;
}

inline OPrimitiveID * EffectorCommander::getPrimID(PrimitiveIndex i) {
	return &primitiveId[i];
}

inline bool EffectorCommander::areActive() {
	return active_;
}

#endif // _EffectorCommander_h_DEFINED

