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


/* Robolink.h
 * Base <-> Robot communications
 */

#ifndef _BASE_ROBOLINK_H_
#define _BASE_ROBOLINK_H_

enum {
    // Port to connect to on the robot
    ROBOLINK_DOG_PORT = 9000,
    // Maximum size of a data chunk. Must be large enough for, e.g. a YUV frame
    ROBOLINK_BUF_LEN = 300000 // ~300 K
};


// data type identifiers as sent by the robot. Insert new types at the end
// of the enum only, or old log files will no longer be readable
enum data_type {
    RLNK_ALL = 0,
    RLNK_DEBUG,
    RLNK_CPLANE,
    RLNK_YUVPLANE,
    RLNK_CAM_ORIENT,        // camera orientation, horizon etc. Obsolete
    RLNK_SENSOR, //5
    RLNK_SUBVISION,         // subvision visual features
    RLNK_SUBOBJECT,         // subobject recognised vobs
    RLNK_INSANITY,
    RLNK_PWALK,             // PWalk info
    RLNK_ATOMIC_ACTION,//10 // atomic actions sent to actuator control
    RLNK_SUBCPLANE,         // subsampled CPlane
    RLNK_DOGID,             // robot identifying information like MAC address
    RLNK_GPSINFO,           // contains gps info about dog
    RLNK_BESTGAP,
    RLNK_PINKOBJECT,        // For localisation challenge
    RLNK_GAUSSIANS,        // For localisation challenge
    
    //TODO: make frame profile data go through robolink
    RLNK_DROPFRAME,         //telnet dropped frame profiles
    RLNK_FRAME,             //telnet normal frame profiles every few seconds
    
    RLNK_NUM_DATA_TYPES,    // must be the last entry
};

// definition of the header information preceeding each data block (both to
// and from the robot)
typedef struct {
    char            magic[4];   // magic header = "RLNK"
    unsigned int    team_num;   // robot team ID
    unsigned int    player_num; // robot player number
    long            frame_num;  // robot frame when data was collected, 4 bytes
    unsigned int    data_type;  // enum data_type
    unsigned int    data_len;   // num bytes in data section
} robolink_header;

const char ROBOLINK_MAGIC[4] = {'R', 'L', 'N', 'K'};

/*** Definitions of struct types for some of the data types above
 * Note that the dog seems to align all struct members on 8-byte double
 * boundaries (64 bit), so
 * 1. don't use chars, shorts, bools to save space.
 * 2. Insert padding at the end to ensure sizeof(struct) is a multiple of 8
 */

/* PWalkInfo and sensor data to be given to VisualCortex */
struct rlnk_pwalksensors {
    long utilt;                     // microradians, from sensors
    long upan;
    long ucrane;
    long range2Obstacle;            // units?
    int leading_leg;                // from PWalkInfo
    int currentWalkType;
    int currentStep;
    int currentPG;
    double frontShoulderHeight;     // mm
    double bodyTilt;                // radians
    double desiredPan;
    double desiredTilt;
    double desiredCrane;
//    bool isHighGain; // we don't need this in vision and it costs us 7 bytes
};

/* Camera orientation and horizon data */
struct cam_orient {
//    double body_tilt;       // body tilt
//    double roll;            // camera roll
    double hz_intercept;    // y axis intercept of horizon
    double hz_gradient;     // horizon gradient in image
    int    hz_exists;       // bool, true if horizon visible
    int    hz_up_is_sky;    // bool, true if horizon right way up
    int    hz_below_image;  // bool, true if when !hz_exists, hz is below image
    int    padding;
//    long   upan;            // microradians pan
//    long   utilt;           // microradians tilt
//    long   ucrane;          // microradians crane
};

/* Atomic action data as sent to actuator control */
struct rlnk_atomicaction {
  int command;
  int walkType;
  int PG;
  int timeElapsed; // for walk calibration
  float forward;
  float left;
  float turn;
  float forwardMaxStep;
  float leftMaxStep;
  float turnCCWMaxStep;
  //int pad;
};


/* Robot identification information */
struct rlnk_dogid {
    long mac;   // last four bytes of MAC address
    int pad;
};

struct rlnk_gpsinfo{
    //These hold the last gps position info
    //The current frame, dog think that he is in..
    float posX;
    float posY;
    float heading;
    
    //position and heading variance
    float posVar;
    float hVar;
    
    // These hold the last motion update values
    // For motion update
    float motionForward;
    float motionLeft;
    float motionTurn;
    float motionPWM;
    
    int motionFCounter;
    //from sensor(commonsense) object, it is used for filtering edge update
    float headSpeed;
    //from visual cortex for visual confidence update
    int visualConfidence;
};

struct rlnk_gaussians{
    float x,y,h,weight;
    float cov[3][3];
    int padding;
};
# endif
