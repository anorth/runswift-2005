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
 * $Id: JointCommand.h 7107 2005-07-02 05:13:25Z weiming $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _JOINT_COMMAND_H_
#define _JOINT_COMMAND_H_

#include "../share/Common.h"
#include "../share/PWalkDef.h"
#include "MacAddress.h"
#include <iostream>
#include <cstring>

using namespace std;

class JointCommand{
	public:
		JointCommand(){
			reset();
		}

        JointCommand& operator==(JointCommand &copy){
            memcpy(copy.joints,this->joints,sizeof(copy.joints));
            return *this;
        }

		void reset(){
			for (int i=0;i<NUM_OF_EFFECTORS;i++){
				joints[i] = VERY_LARGE_SLONGWORD;
			}
		}

		void setJointValue(int index,slongword value){
			if (index >= 0 || index < NUM_OF_EFFECTORS){
				joints[index] = value;
			}
			else{
				cout << "BUGS: joint index is out of range" << endl;
			}
		}

		slongword getJointValue(int index) const {
			if (index >= 0 || index < NUM_OF_EFFECTORS){
				return joints[index] ;
			}
			else{
				return VERY_LARGE_SLONGWORD;
			}

		}

        //helper functions : index is specified in JointEnum type ( = 1..15)
		void setJointEnumValue(JointEnum index,slongword value){
            setJointValue(index-1,value);
		}

		slongword getJointEnumValue(JointEnum index) const {
            return getJointValue(index-1);
		}

        //not sure why I did this, the value is already in microrad, I probably meant getJointValueInDegree
		slongword getJointValueInMicro(int index) const {
            return getJointValue(index);
		}

		double getJointValueInDegree(int index) const {
            slongword t = getJointValue(index);
            if (t == VERY_LARGE_SLONGWORD)
                return VERY_LARGE_DOUBLE;
            else
                return MICRO2DEG(t);
		}

		double getJointValueInRads(int index) const {
            slongword t = getJointValue(index);
            if (t == VERY_LARGE_SLONGWORD)
                return VERY_LARGE_DOUBLE;
            else
                return MICRO2RAD(t);
		}

	private:
		slongword joints[NUM_OF_EFFECTORS]; // index is in share/PWalkDef.h:JointEnum
};

extern ostream& operator<<(ostream &out , JointCommand &jCommand);

#endif //_JOINT_COMMAND_H_
