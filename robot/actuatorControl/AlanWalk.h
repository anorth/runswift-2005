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
 * $Id: AlanWalk.h 4364 2004-09-22 07:04:12Z kcph007 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _AlanWalk_h
#define _AlanWalk_h

#include "NormalWalk.h"
#include "SpecialAction.h"
#include <vector>

using namespace std;

class AlanWalk : public SpecialAction, public NormalWalk{
	public:

	AlanWalk() {
	}
	
	virtual ~AlanWalk() {
	}
    void setParameters(const AtomicAction &command);
    void setCurrentJointCommand(JointCommand &jointCommand);
    bool usingHead(){ return false; }
	
	protected:

	virtual void calibrate(double f, double l, double t);

    private:
    void composeTheMove();
    void reverseAndRotateLocusPoints(vector<JointCommand> &locusPoints, int PG);
    void linearInterpolatePoint(JointCommand &corner1, JointCommand &corner2, JointCommand &output, int i, int total);
    void makeLocusPoint(JointCommand *cornerJoints, int PGR, int PGH, vector<JointCommand> &locusPoints);


	
};

#endif // _AlanWalk_h
