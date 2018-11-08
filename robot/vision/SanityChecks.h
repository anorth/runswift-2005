/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/* SanityChecks.h
 *
 * Visual object recognition tests. This namespace houses both hand-crafted
 * checks and accesses machine-learnt checks.
 *
 * SanityChecks is singleton - everything is static
 */

#ifndef _SANITYCHECKS_H_
#define _SANITYCHECKS_H_

#include <iostream>


class VisualCortex; // forward declarations
class VisualObject;

extern const char* insanityNames[];

class SanityChecks
{
    public:

        static void init(VisualCortex* vc);
        static void clear();

        /* Checks the legitimacy of the given ball/beacon/goal and returns
         * SANE if the object appears valid, else an Insanity code if the
         * object is not valid. These methods must access *only* the 
         * following vob fields (those set by SubObject):
         *   cx, cy, x, y, height, width, cf
         */
        static Insanity checkBallSanity(VisualObject& vBall);
        static Insanity checkBeaconSanity(VisualObject& vBeacon);
        static Insanity checkGoalSanity(VisualObject& vGoal);

        /* Checks the legitimacy of *all* the valid (cf > 0) objects in vobs
         * (which should be a pointer to the VisualObject array in VisualCortex
         * and indexed the same way) existing concurrently, i.e. that they make
         * sense at the same time. If something is wrong the vob with the
         * lowest confidence factor (usually) is invalidated in place. All vob
         * fields may be used.
         */
        static void checkWorldSanity(VisualObject vobs[], int nVobs);

        /* A placeholder function to do something when a sanity check
         * fires, e.g. print it or log it
         */
        inline static Insanity insane(Insanity s) {
//            std::cout << "Insanity fired: " << s << " " << insanityNames[s]
//                << std::endl;
            fired[s/8] |= (1 << (s % 8));
            return s;
        }

        static void sendInsanities();

    private:
        friend class FormSubvisionImpl; // allow offline tools private access
        static VisualCortex* cortex;
        static unsigned char* fired; // bitmap of fired insanities
};

#endif // _SANITYCHECKS_H_
