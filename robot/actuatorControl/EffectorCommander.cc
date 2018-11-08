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
 * $Id: EffectorCommander.cc 4653 2005-01-18 06:27:35Z weiming $
 *
 * Copyright(c) 2004 UNSW
 * All Rights Reserved.
 *
 **/

#include <OPENR/core_macro.h>
#include <OPENR/ODebug.h>
#include <OPENR/OPENRAPI.h>
#include <iostream>
#include "EffectorCommander.h"

#define _SAFE_INITIALIZATION

// High Gain

static const PRMInfo HGprims[] = {
	// head: tilt, pan, tilt2
	{PRMInfo::JOINT2, "PRM:/r1/c1-Joint2:11", 0x0a, 0x04, 0x02},
	{PRMInfo::JOINT2, "PRM:/r1/c1/c2-Joint2:12", 0x08, 0x02, 0x04},
	{PRMInfo::JOINT2, "PRM:/r1/c1/c2/c3-Joint2:13", 0x08, 0x04, 0x02},
	// mouth
	{PRMInfo::JOINT2, "PRM:/r1/c1/c2/c3/c4-Joint2:14", 0x08, 0x00, 0x04},
	// right front: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r4/c1-Joint2:41", 0x1c, 0x08, 0x01},
	{PRMInfo::JOINT2, "PRM:/r4/c1/c2-Joint2:42", 0x14, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r4/c1/c2/c3-Joint2:43", 0x1c, 0x08, 0x01},
	// left front: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r2/c1-Joint2:21", 0x1c, 0x08, 0x01},
	{PRMInfo::JOINT2, "PRM:/r2/c1/c2-Joint2:22", 0x14, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r2/c1/c2/c3-Joint2:23", 0x1c, 0x08, 0x01},
	// right rear: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r5/c1-Joint2:51", 0x1c, 0x08, 0x01},
	{PRMInfo::JOINT2, "PRM:/r5/c1/c2-Joint2:52", 0x14, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r5/c1/c2/c3-Joint2:53", 0x1c, 0x08, 0x01},
	// left rear: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r3/c1-Joint2:31", 0x1c, 0x08, 0x01},
	{PRMInfo::JOINT2, "PRM:/r3/c1/c2-Joint2:32", 0x14, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r3/c1/c2/c3-Joint2:33", 0x1c, 0x08, 0x01},
	// tail: tilt, pan
	{PRMInfo::JOINT2, "PRM:/r6/c1-Joint2:61", 0x0a, 0x04, 0x04},
	{PRMInfo::JOINT2, "PRM:/r6/c2-Joint2:62", 0x0a, 0x04, 0x04},
	// ear: left, right
	{PRMInfo::JOINT3, "PRM:/r1/c1/c2/c3/e5-Joint4:15", -1, -1, -1},
	{PRMInfo::JOINT3, "PRM:/r1/c1/c2/c3/e6-Joint4:16", -1, -1, -1},
	// SENTRY
	{PRMInfo::UNKNOWN, NULL, -1, -1, -1}
};
// Weak gain - our standard 

static const PRMInfo prims[] = {
	// head: tilt, pan, tilt2
	{PRMInfo::JOINT2, "PRM:/r1/c1-Joint2:11", 0x0a, 0x04, 0x02},
	{PRMInfo::JOINT2, "PRM:/r1/c1/c2-Joint2:12", 0x08, 0x02, 0x04},
	{PRMInfo::JOINT2, "PRM:/r1/c1/c2/c3-Joint2:13", 0x08, 0x04, 0x02},
	// mouth
	{PRMInfo::JOINT2, "PRM:/r1/c1/c2/c3/c4-Joint2:14", 0x08, 0x00, 0x04},
	// right front: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r4/c1-Joint2:41", 0x10, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r4/c1/c2-Joint2:42", 0x0a, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r4/c1/c2/c3-Joint2:43", 0x10, 0x04, 0x01},
	// left front: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r2/c1-Joint2:21", 0x10, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r2/c1/c2-Joint2:22", 0x0a, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r2/c1/c2/c3-Joint2:23", 0x10, 0x04, 0x01},
	// right rear: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r5/c1-Joint2:51", 0x10, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r5/c1/c2-Joint2:52", 0x0a, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r5/c1/c2/c3-Joint2:53", 0x10, 0x04, 0x01},
	// left rear: j1, j2, j3
	{PRMInfo::JOINT2, "PRM:/r3/c1-Joint2:31", 0x10, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r3/c1/c2-Joint2:32", 0x0a, 0x04, 0x01},
	{PRMInfo::JOINT2, "PRM:/r3/c1/c2/c3-Joint2:33", 0x10, 0x04, 0x01},
	// tail: tilt, pan
	{PRMInfo::JOINT2, "PRM:/r6/c1-Joint2:61", 0x0a, 0x04, 0x04},
	{PRMInfo::JOINT2, "PRM:/r6/c2-Joint2:62", 0x0a, 0x04, 0x04},
	// ear: left, right
	{PRMInfo::JOINT3, "PRM:/r1/c1/c2/c3/e5-Joint4:15", -1, -1, -1},
	{PRMInfo::JOINT3, "PRM:/r1/c1/c2/c3/e6-Joint4:16", -1, -1, -1},
	// SENTRY
	{PRMInfo::UNKNOWN, NULL, -1, -1, -1}
};


/*
 * Initiates the variables: sEffector to NULL, active_ to false, 
 * region to NULL and gainSet to false
 */
EffectorCommander::EffectorCommander() {
	sEffector = NULL;
	active_ = false;
	region_ = static_cast<RCRegion*>(NULL);
	gainSet = false;
}

EffectorCommander::~EffectorCommander() {
}

/*
 * Initiates the primitives to use and the Command Vector to use
 */
OStatus EffectorCommander::init(OSubject *subject) {
	sEffector = subject;
	InitPrimitives();
	InitCommandVectorData();
	return oSUCCESS;
}

/*
 * Calls the first iteration of InitJointGain()
 */
OStatus EffectorCommander::start() {
	active_ = true;
	if (sEffector->IsReady()) {
		if (!gainSet) {
			gainSet = InitJointGain();
		}
	}

	return oSUCCESS;
}

/*
 * Removes the memory resources used for this subject and closes all primitives
 */
OStatus EffectorCommander::stop() {
	if (region_) {
		region_->RemoveReference();
		OPENR::DeleteCommandVectorData(region_->MemID());
		region_ = static_cast<RCRegion*>(NULL);
	}

	for (int i = 0; prims[i].locator != NULL; ++i) {
		if (primitiveId[i] != oprimitiveID_UNDEF) {
			OPENR::ClosePrimitive(primitiveId[i]);
		}
	}

	active_ = false;

	return oSUCCESS;
}

/*
 * Called by a Observer Ready method to continue initialisation
 * @param event is the Ready event recieved by the Observer Read method
 */
void EffectorCommander::continueInit(const OReadyEvent &event) {
	if (active_ && sEffector->IsReady(event.SenderID())) {
		if (!gainSet) {
			gainSet = InitJointGain();
		}
	}

	return;
}

/*
 * InitPrimitives opens all joint primitives
 */
OStatus EffectorCommander::InitPrimitives() {
	for (int i = 0; prims[i].locator != NULL; i++) {
		OPENR::OpenPrimitive(prims[i].locator, &primitiveId[i]);
		// if fail, prim[i].id = oprimitiveID_UNDEF
	}

	return oSUCCESS;
}

/*
 * InitCommandVectorData initiatlises the command vector and assigns 
 * the vecotr to a memory region
 */
OStatus EffectorCommander::InitCommandVectorData() {
	MemoryRegionID memID;
	OCommandVectorData *cmdVec;
	if (OPENR::NewCommandVectorData(1, &memID, &cmdVec) == oSUCCESS) {
		cmdVec->SetNumData(1);
		region_ = new RCRegion(cmdVec->vectorInfo.memRegionID,
				cmdVec->vectorInfo.offset,
				(void *) cmdVec,
				cmdVec->vectorInfo.totalSize);
	}
	return oSUCCESS;
}

/**
 * The joint gain initialisation method has two settings.  The only one that
 * can be used with P009 binary set is the 
 * Safe initialisation.  Subsequent binary sets may be able to use the 
 * "unsafe" initialisation
 * Firstly InitJointGain either 
 *     1) Turns the motor on
 *     2) For all subsequent iterations it sets the joint gain for the initiated joint
 * Secondly InitiJointGain either 
 *     1) intiates a joint with signature JOINT2
 *     2) ignores the joint
 */
bool EffectorCommander::InitJointGain(void) {
#ifndef _SAFE_INITIALIZATION

	// Motor power ON
	if (OPENR::SetMotorPower(opowerON) == oSUCCESS) {
		OSYSDEBUG(("Motor ON\n"));
	}
	else {
		OSYSDEBUG(("Motor ON failed...\n"));
	}

	// PID gain
	for (int i = 0; i < numberOfJoints; ++i) {
		if (oprimitiveID_UNDEF != primitiveId[i]) {
			if (prims[i]->jointType == PRMInfo::JOINT2) {
				OPENR::EnableJointGain(primitiveId[i]);
				OPENR::SetJointGain(primitiveId[i],
						prims[i]->pgain,
						prims[i]->igain,
						prims[i]->dgain,
						0x0e,
						0x02,
						0x0f);
			}
		}
	}

	return true;            // everything is done in one step

#else // _SAFE_INITIALIZATION

	// the primitives will be initialized step by step
	static int index;
	static bool motorPowerFlag = false;

	// the first time?
	if (!motorPowerFlag) {
		// Be safe
		for (int i = 0; i < numberOfJoints; ++i) {
			OPENR::DisableJointGain(primitiveId[i]);
		}
		// Motor power ON
		if (OPENR::SetMotorPower(opowerON) == oSUCCESS) {
			OSYSDEBUG(("Motor ON\n"));
		}
		else {
			OSYSDEBUG(("Motor ON failed\n"));
		}

		motorPowerFlag = true;
	}
	else {
		// the command data has been already initialized,
		// it's safe to activate this joint now
		OPENR::EnableJointGain(primitiveId[index]);
		OPENR::SetJointGain(primitiveId[index],
				prims[index].pgain,
				prims[index].igain,
				prims[index].dgain,
				0x0e,
				0x02,
				0x0f);
		// now go on to next primitive
		++index;
	}

	// Initialize this joint
	while (prims[index].locator != NULL) {
		if ((primitiveId[index] != oprimitiveID_UNDEF)
				&& (prims[index].jointType == PRMInfo::JOINT2)) {
			// Only JOINT2 needs explicit initialization
			OJointValue current;
			// This API is a heavy system call,
			// acceptable only for the initialization purpose
			if (OPENR::GetJointValue(primitiveId[index], &current) == oSUCCESS) {
				if (region_->NumberOfReference() == 1) {
					// Only I am using this region
					OCommandVectorData *cmdVec = reinterpret_cast<OCommandVectorData*>(region_->Base());
					OCommandInfo *info0 = cmdVec->GetInfo(0);
					OCommandData *data0 = cmdVec->GetData(0);
					const int nframes = 4;
					info0->Set(odataJOINT_COMMAND2, primitiveId[index], nframes);
					OJointCommandValue2 *cmd = reinterpret_cast<OJointCommandValue2*>(data0->value);
					for (int i = 0; i < nframes; ++i) {
						cmd[i].value = current.value;
					}
					sEffector->SetData(region_);
					sEffector->NotifyObservers();
					break;
				}
				else {
					if (prims < &prims[index]) {
						--index;
					}
					else {
						motorPowerFlag = false;
					}
					return false;
				}
			}
			else {
				// ignore this primitive and go on...
			}
		}
		++index;   // skip this and go on to next primitive
	}

	return (prims[index].locator == NULL) ? true : false;

#endif // _SAFE_INITIALIZATION
}

/*
** Note:
** 1. Joint values are available in the start of the file.
** 2. The setJointGain is used for all joints, but mainly used for the 4 legs.
** 3. The other three values are standard, dont set them, see Sony spec. they
**    are the P,I,D shifts.
*/
void EffectorCommander::setJointGain ( int whichJoint , int P , int I , int D , bool highGain) {

  cout << "EffectorCommander::relaxJoint specific joints: " << whichJoint << endl;

  int i = whichJoint;
  if ( i < 0 || i >= numberOfJoints ) {
      cout << "Incorrect Joint Specified in setting its gain" << endl;
      return;
  }
  if ( highGain ) {
    P = CLIP (P, HGprims[i].pgain);
    I = CLIP (I, HGprims[i].igain);
    D = CLIP (D, HGprims[i].dgain);
  }
  else {
    P = CLIP (P, prims[i].pgain);
    I = CLIP (I, prims[i].igain);
    D = CLIP (D, prims[i].dgain);
  }
  P = MAX (0,P);
  I = MAX (0,I);
  D = MAX (0,D);

  if (oprimitiveID_UNDEF != primitiveId[i]) {
          if (prims[i].jointType == PRMInfo::JOINT2) {
			OPENR::EnableJointGain(primitiveId[i]);
			OPENR::SetJointGain(primitiveId[i],P,I,D,0x0e,0x02,0x0f);
	  }
  }

}


void EffectorCommander::relaxJoints(){
    cout << "EffectorCommander::relaxJoint" << endl;
	// PID gain
	for (int i = 0; i < numberOfJoints; ++i) {
		if (oprimitiveID_UNDEF != primitiveId[i]) {
			if (prims[i].jointType == PRMInfo::JOINT2) {
				OPENR::EnableJointGain(primitiveId[i]);
				OPENR::SetJointGain(primitiveId[i],
						0,
						0,
						0,
						0,
						0,
						0);
			}
		}
	}

}

//WARNING: excerpted from InitJointGain (unsafe)
void EffectorCommander::reEnableJoints(){
	// PID gain

	for (int i = 0; i < numberOfJoints; ++i) {
		if (oprimitiveID_UNDEF != primitiveId[i]) {
			if (prims[i].jointType == PRMInfo::JOINT2) {
				OPENR::EnableJointGain(primitiveId[i]);
				OPENR::SetJointGain(primitiveId[i],
						    prims[i].pgain,
						    prims[i].igain,
						    prims[i].dgain,
						    0x0e,
						    0x02,
						    0x0f);
			}
		}
	}

}

void EffectorCommander::highGainJoints(){
	// i=4 becoz skips the mouth & head.

	for (int i = 0; i < numberOfJoints; ++i) {
		if (oprimitiveID_UNDEF != primitiveId[i]) {
			if (prims[i].jointType == PRMInfo::JOINT2) {
				OPENR::EnableJointGain(primitiveId[i]);
				OPENR::SetJointGain(primitiveId[i],
						    HGprims[i].pgain,
						    HGprims[i].igain,
						    HGprims[i].dgain,
						    0x0e,
						    0x02,
						    0x0f);
			}
		}
	}

}
