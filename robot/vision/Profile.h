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


/* Profile.h
 * On-line performance profiling
 */

#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <iostream>
#include <vector>

#ifndef OFFLINE
#include <OPENR/OPENREvent.h>
#else               // hacks to make it work offline
#include <sys/time.h>
#include <time.h>
#include <string.h>
// This struct is exactly the same as struct timeval (<time.h>) but with
// the fields named to match the OPENR SystemTime
typedef struct {
    long seconds;
    long useconds;
} SystemTime;
#define GetSystemTime(x) gettimeofday((struct timeval*)(x), NULL)
#endif

// don't forget to update profileNames in Profile.cc to match
enum ProfId {
    PR_FRAME = 0,
    PR_PROCESSIMAGE,
    PR_SUBVISION,
    PR_SV_VERT,     // vertical scanline processing
    PR_SV_HORZ,     // horizontal scanline processing
    PR_SV_EXTRABALL,// extra ball scanline processing
    PR_SUBOBJECT,
    PR_SO_WALL,
    PR_SO_GOALS,
    PR_SO_BEACONS,
    PR_SO_PRUNE,
    PR_SO_INSANEFEATURES,
    PR_SO_BALL,
    PR_LINEMAPPING,
    PR_GPS,
    PR_GPS_VISION,
    PR_GPS_MOTION,
    PR_GPS_EDGE,
    PR_GPS_OBSTACLE,
    PR_BEHAVIOUR,
    PR_BE_VISLINK,
    PR_BE_VISLINK_OBS,
    PR_BE_OBS_BOX,
    PR_BE_OBS_HEAD,
    PR_BE_OBS_GAP,
    PR_BE_OBS_BETWEEN,
    PR_BE_OBS_NEAREST,
    PR_BE_OBS_SHARED,
    PR_PYTHON_BEHAVIOU,
    PR_PYTHON_PFORWARD,
    PR_PYTHON_RATTACKER,
    PR_PYTHON_HFRAMERESETP,
    PR_PYTHON_HFRAMERESETR,
    PR_PYTHON_HFRAMERESETOR,
    PR_PYTHON_HFRAMERESETPO,
    PR_PYTHON_GLOBALFR,
    PR_PYTHON,
    NUM_PROFID,  
};


/* The Profile class is singleton - everything is static */
class Profile
{
    public:
    
        /* get the profile id for a given python filename 
         * return default python id if no lookup for specific file
         */        
        static inline ProfId lookupPy(char *fname) {
            if (strcmp(fname, "Behaviou.py") == 0) {
                return PR_PYTHON_BEHAVIOU;
            }        
            else if (strcmp(fname, "pForward.py") == 0) {
                return PR_PYTHON_PFORWARD;
            }
            else if (strcmp(fname, "rAttacker.py") == 0) {
                return PR_PYTHON_RATTACKER;
            }            
            else if (strcmp(fname, "hFrameResetP") == 0) {
                return PR_PYTHON_HFRAMERESETP;
            }
            else if (strcmp(fname, "hFrameResetR") == 0) {
                return PR_PYTHON_HFRAMERESETR;
            }
            else if (strcmp(fname, "hFrameResetOR") == 0) {
                return PR_PYTHON_HFRAMERESETOR;
            }
            else if (strcmp(fname, "hFrameResetPO") == 0) {
                return PR_PYTHON_HFRAMERESETPO;
            }                                    
            else if (strcmp(fname, "GlobalFrameReset") == 0) {
                return PR_PYTHON_GLOBALFR;
            }
            
            else {
                return PR_PYTHON;
            }
            
        }    
    
        /* Sets the specified profile to the given time */
        static inline void set(ProfId pid, SystemTime time) {
            profiles[pid] = time;
        }
    
        /* Returns the time recorded in the specified profile (msec).
         * This is only valid after a start-stop cycle
         */
        static inline double get(ProfId pid) {
            return (profiles[pid].seconds * 1000 
                        + (double)profiles[pid].useconds / 1000);
        }

        /* Initialise Profiling */
        static inline void init() {
            for (int i = 0; i < NUM_PROFID; i++)
            {
                profiles[i].seconds = 0;
                profiles[i].useconds = 0;                 
            }
        }  
                            
        /* Starts a timer for the given profile. Call stop(pid) to
         * stop the timer and record the elapsed time.
         */
        static inline void start(ProfId pid, int d_id = -1) {
            GetSystemTime(&resumeprofiles[pid]);
            debugId = d_id;
        }
        

        /* Stops the timer for pid and records the elapsed time in the
         * profile. The result is only valid after a call to start(pid)
         */
        static inline void stop(ProfId pid) {
            SystemTime end;
            GetSystemTime(&end);
        
            if (end.useconds > resumeprofiles[pid].useconds) {
                profiles[pid].seconds = profiles[pid].seconds
                    + (end.seconds - resumeprofiles[pid].seconds);
                profiles[pid].useconds = profiles[pid].useconds
                    + (end.useconds - resumeprofiles[pid].useconds);
            } else {
                profiles[pid].seconds = profiles[pid].seconds 
                    + (end.seconds - 1 - resumeprofiles[pid].seconds);
                profiles[pid].useconds = profiles[pid].useconds
                    + (end.useconds + 1000000 - resumeprofiles[pid].useconds);
            }
                        
        }
        
        
        /* gets you the time taken so far in THIS start/stop iteration,
         * i.e. NOT total time taken so far by profileID this frame
         * only valid between a start/stop pair 
         */
        static inline double timeSoFar(ProfId pid) {
            SystemTime now;
            GetSystemTime(&now);
            
            if (now.useconds > resumeprofiles[pid].useconds) {
                return (now.seconds - resumeprofiles[pid].seconds) * 1000 +
                double(now.useconds - resumeprofiles[pid].useconds) / 1000;
            } else {
                return (now.seconds - 1 - resumeprofiles[pid].seconds) * 1000 +
                double(now.useconds + 1000000 - resumeprofiles[pid].useconds) / 1000;
            }            
            
        } 
        
        static inline void startPy(char *fname) {
            start(lookupPy(fname));
        } 
        
        static inline void stopPy(char *fname) {
            stop(lookupPy(fname));
        }
        
        static inline double timeSoFarPy(char *fname) {
            return timeSoFar(lookupPy(fname));
        }
                
        /* Clears all profiles */
        static inline void clear() {
            profiles.clear();
            profiles.resize((int)NUM_PROFID);
            
            savedprofiles.clear();
            savedprofiles.resize((int)NUM_PROFID);       
            
            resumeprofiles.clear();
            resumeprofiles.resize((int)NUM_PROFID);  
        }

        /* Save a copy of current profile information */
        static void save();
                
        /* Prints the specified profile time to stderr. If pid is NUM_PROFID
         * then all profiles are printed. The profiles are only valid after a
         * start-stop cycle
         */
        static void print(ProfId pid, int debugId = -1);
        
        /* Same as print but for saved values*/
        static void printsaved(ProfId pid);
        
    private:
        // Vector of times indexed by ProfID
        static std::vector<SystemTime> profiles;
        
        static std::vector<SystemTime> savedprofiles;
        
        static std::vector<SystemTime> resumeprofiles;

        static int debugId;
        
};

#endif // _PROFILE_H_
