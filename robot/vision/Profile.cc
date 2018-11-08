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


/* Implementation of the static members in Profile.h, online profiling. */

#include "Profile.h"
#include <sstream>

using namespace std;

// this must match enum ProfId in Profile.h
static const char* profNames[] = {
    "***Frame***",
    "ProcessImage",
    "SubVision",
    "SV: verts",
    "SV: horizs",
    "SV: extra ball",
    "SubObject",
    "SO: wall",
    "SO: goals",
    "SO: beacons",
    "SO: pruning",
    "SO: insanities",
    "Ball Detection",
    "Line mapping",
    "GPS",
    "GPS vis",
    "GPS motion",
    "GPS edge",
    "GPS obstacle",
    "Behaviours",
    "BE: vislink: misc",
    "BE: vislink: obs",
    "BE: vislink: box",
    "BE: vislink: head",
    "BE: vislink: gap",
    "BE: vislink: between",
    "BE: vislink: nearest", 
    "BE: Shared Obstacles",   
    "PY: Behaviou",
    "PY: pForward",
    "PY: rAttacker",
    "PY: hFrameResetPreset",
    "PY: hFrameResetReset",
    "PY: hFrameResetOfflineReset",
    "PY: hFrameResetPostset",            
    "PY: Global.frameReset",
    "PY: other",
    "All profiles",
};

vector<SystemTime> Profile::profiles(NUM_PROFID);
vector<SystemTime> Profile::savedprofiles(NUM_PROFID);
vector<SystemTime> Profile::resumeprofiles(NUM_PROFID);
int Profile::debugId;


void Profile::print(ProfId pid, int d_id) {
    // print the specified
    stringstream debugStr;
    debugStr << "";
    if (d_id != -1) debugStr << " (r#" << d_id << ")";
    
    if (pid != NUM_PROFID) {
#ifdef OFFLINE
        cerr << "Pr# " << profNames[pid] << " : " 
            << (profiles[pid].seconds * 1000 
                    + (double)profiles[pid].useconds / 1000)
            << "ms" << debugStr << endl;
#else
        if (d_id != -1)
        {
            OSYSDEBUG(("Pr# %s : %.2fms (r#%d)\n", profNames[pid],
                (profiles[pid].seconds * 1000 + 
                    (double)profiles[pid].useconds / 1000), d_id));
        }
        else
        {
            OSYSDEBUG(("Pr# %s : %.2fms\n", profNames[pid],
                (profiles[pid].seconds * 1000 + 
                    (double)profiles[pid].useconds / 1000)));
        }
#endif
        return;
    }
    // else print them all
    for (int i = 0; i < NUM_PROFID; ++i) {
#ifdef OFFLINE
        cerr << "Pr# " << profNames[i] << " : " 
            << (profiles[i].seconds * 1000
                    + (double)profiles[i].useconds / 1000)
            << "ms" << debugStr << endl;
#else
        if (d_id != -1)
        {
            OSYSDEBUG(("Pr# %s : %.2fms (r#%d)\n", profNames[i],
                (profiles[i].seconds * 1000 + 
                    (double)profiles[i].useconds / 1000), d_id));
        }
        else
        {
            OSYSDEBUG(("Pr# %s : %.2fms\n", profNames[i],
                (profiles[i].seconds * 1000 + 
                    (double)profiles[i].useconds / 1000)));
        }
#endif
    }
}

void Profile::save(){
    for (int i = 0; i < NUM_PROFID; ++i) {
        savedprofiles[i] = profiles[i];
    }
}

void Profile::printsaved(ProfId pid) {
    // print the specified
    if (pid != NUM_PROFID) {
#ifdef OFFLINE
        cerr << "Pr# " << profNames[pid] << " : " 
            << (savedprofiles[pid].seconds * 1000 
                    + (double)savedprofiles[pid].useconds / 1000)
            << "ms" << endl;
#else
        OSYSDEBUG(("Pr# %s : %.2fms\n", profNames[pid],
                    (savedprofiles[pid].seconds * 1000 + 
                     (double)savedprofiles[pid].useconds / 1000)));
#endif
        return;
    }
    // else print them all
    for (int i = 0; i < NUM_PROFID; ++i) {
#ifdef OFFLINE
        cerr << "Pr# " << profNames[i] << " : " 
            << (savedprofiles[i].seconds * 1000
                    + (double)savedprofiles[i].useconds / 1000)
            << "ms" << endl;
#else
        OSYSDEBUG(("Pr# %s : %.2fms\n", profNames[i],
                    (savedprofiles[i].seconds * 1000 + 
                     (double)savedprofiles[i].useconds / 1000)));
#endif
    }
}

