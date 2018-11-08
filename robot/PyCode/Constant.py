##     Copyright 2005 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     should be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Constant.py
#
# Constants used in behaviours codes. Most player modules will need to import
# this.
#
#===============================================================================

import math

# Directions used in hFWHead.py
dCLOCKWISE      = 0
dANTICLOCKWISE  = 1
dAUTO           = 2
LEFT            = 1 #same goes to side movement positive is left
MIDDLE          = 0
RIGHT           = -1

NUM_OF_EFFECTORS = 15
NUM_SENSORS      = 34


FRAME_RATE = 30 # frames per second


# Indicies for finalWirelessInfo
wiBallDistance = 0
wiIsStriker    = 1
wiIsGoalie     = 2
wiHaveBall     = 3
wiKickedOff    = 4
wiIsDefending  = 5
wiIsBackOff    = 6


# Role constants.
NOROLE           = 0
FACEBACKOFF      = 1
ATTACKER         = 2
WINGER           = 3
ATTACKBACKOFF    = 4
GETBEHINDBACKOFF = 5
SUPPORTER        = 6
SIDEBACKOFF      = 7
SUPPORTBACKOFF   = 8
DEFENDER         = 9
DEFENDERGOALBOX  = 10
STEALTHDOG       = 11
STUCKER          = 12
GOALIE           = 13
GOALIEATTACK     = 14
GOALIEDEFENDSEEB = 15
GOALIEDEFENDNOTS = 16
BACKOFF          = 17
PENALISED        = 18
STRIKER          = 19   

# Face patterns constants.
"""
FP_BACKOFF_RUN_BEHIND   = [1,0,1,0,1]
FP_BACKOFF_SUPPORT      = [1,0,1,0,2]
FP_BACKOFF_ATTACKER     = [1,0,1,0,3]
FP_BACKOFF_IMMEDIATE    = [1,0,1,1,3]

FP_FIRE_PAW_KICK        = [1,1,1,1,1]
FP_HOVER_TO_BALL        = [1,2,1,2,1]
FP_BIRD_OF_PREY         = [2,1,2,1,2]
FP_DIR_KICK             = [3,0,2,0,3]

FP_GET_BEHIND_LE        = [3,0,0,0,0]
FP_GET_BEHIND_RE        = [0,0,0,0,3]
FP_GET_BEHIND_BE        = [0,0,1,0,0]
FP_GET_BEHIND_TLE       = [0,2,0,0,0]
FP_GET_BEHIND_TRE       = [0,0,0,2,0]
FP_GET_BEHIND_TLC       = [1,0,0,0,0]
FP_GET_BEHIND_TRC       = [0,0,0,0,1]

FP_DKD_LEFT             = [0,0,2,2,2]
FP_DKD_RIGHT            = [2,2,2,0,0]
FP_DKD_BOTTOM_LEFT      = [2,0,0,2,2]
FP_DKD_BOTTOM_RIGHT     = [2,2,0,0,2]
"""


# JointEnum enumeration in PWalkDef.h
jNothing                = 0
jHeadTilt               = 1
jHeadPan                = 2
jHeadCrane              = 3
jRightFrontJoint        = 4
jRightFrontShoulder     = 5
jRightFrontKnee         = 6
jLeftFrontJoint         = 7
jLeftFrontShoulder      = 8
jLeftFrontKnee          = 9
jRightRearJoint         = 10
jRightRearShoulder      = 11
jRightRearKnee          = 12
jLeftRearJoint          = 13
jLeftRearShoulder       = 14
jLeftRearKnee           = 15

# Stand constants (cm)
SHOULDER_HEIGHT         = 8.1
HIP_HEIGHT              = 11.0


DOG_BODY_LENGTH         = 13.0 # cm, this does NOT includes the legs, only body
DOG_LENGTH              = 20.0 # cm, this includes the width of the legs
DOG_WIDTH               = 10.0 # cm, this includes the width of two legs as well
DOG_NECK_TO_BODY_CENTER_OFFSET = DOG_BODY_LENGTH / 2  # = 6.5 cm


HANDKICKANGLEOFFSET     = 0


#===============================================================================
# Other constants

LARGE_VAL           = 10000000
BLUE_TEAM           = 0 #compatible with VisionLink.getTeamColor()
RED_TEAM            = 1

# Physical measurements  , from share/BallDef.h
BallRadius            = 4.2
BallDiameter          = 2 * BallRadius

#UNSW FIELD
"""
# Physical dimension of the field, in cm.
FIELD_WIDTH            = 360
FIELD_LENGTH           = 540
FIELD_DIAGONAL         = math.sqrt(FIELD_WIDTH ** 2 + FIELD_LENGTH ** 2)
BEACON_HEIGHT          = 30  # height of the centre of the coloured bit
BEACON_LEFT_X          = -15
BEACON_RIGHT_X         = FIELD_WIDTH - BEACON_LEFT_X
BEACON_NEAR_Y          = 135
BEACON_FAR_Y           = FIELD_LENGTH - BEACON_NEAR_Y
GOAL_WIDTH             = 80
GOAL_LENGTH            = 30
CORNER_WIDTH           = 0
GOALBOX_WIDTH          = 130 #was 100
GOALBOX_DEPTH          = 65 #was 50
CIRCLE_DIAMETER        = 36 #was 30
TARGET_GOAL_X          = FIELD_WIDTH / 2.0
TARGET_GOAL_Y          = FIELD_LENGTH
OWN_GOAL_X             = TARGET_GOAL_X
OWN_GOAL_Y             = 0

MIN_GOALBOX_EDGE_X    = TARGET_GOAL_X - GOALBOX_WIDTH / 2.0
MAX_GOALBOX_EDGE_X    = TARGET_GOAL_X + GOALBOX_WIDTH / 2.0
TOP_GOALBOX_EDGE_Y    = TARGET_GOAL_Y - GOALBOX_DEPTH

LEFT_GOAL_POST         = TARGET_GOAL_X - GOAL_WIDTH/2.0
RIGHT_GOAL_POST        = TARGET_GOAL_X + GOAL_WIDTH/2.0
OWN_GOALBOX_EDGE_Y     = OWN_GOAL_Y + GOALBOX_DEPTH
SIDELINE_FRINGE_WIDTH  = 20
GOAL_FRINGE_WIDTH      = 30
IMAGE_WIDTH            = 208
IMAGE_HEIGHT           = 160
"""

#OSAKA FIELD
# Physical dimension of the field, in cm.
FIELD_WIDTH            = 363#360
FIELD_LENGTH           = 542#540
FIELD_DIAGONAL         = math.sqrt(FIELD_WIDTH ** 2 + FIELD_LENGTH ** 2)
BEACON_HEIGHT          = 30  # height of the centre of the coloured bit
BEACON_LEFT_X          = -13# -15
BEACON_RIGHT_X         = FIELD_WIDTH - BEACON_LEFT_X
BEACON_NEAR_Y          = 136#135 
BEACON_FAR_Y           = FIELD_LENGTH - BEACON_NEAR_Y
GOAL_WIDTH             = 80
GOAL_LENGTH            = 30
CORNER_WIDTH           = 0
GOALBOX_WIDTH          = 128#130 #was 100
GOALBOX_DEPTH          = 65 #was 50
CIRCLE_DIAMETER        = 36 #was 30
TARGET_GOAL_X          = FIELD_WIDTH / 2.0
TARGET_GOAL_Y          = FIELD_LENGTH
OWN_GOAL_X             = TARGET_GOAL_X
OWN_GOAL_Y             = 0

MIN_GOALBOX_EDGE_X    = TARGET_GOAL_X - GOALBOX_WIDTH / 2.0
MAX_GOALBOX_EDGE_X    = TARGET_GOAL_X + GOALBOX_WIDTH / 2.0
TOP_GOALBOX_EDGE_Y    = TARGET_GOAL_Y - GOALBOX_DEPTH

LEFT_GOAL_POST         = TARGET_GOAL_X - GOAL_WIDTH/2.0
RIGHT_GOAL_POST        = TARGET_GOAL_X + GOAL_WIDTH/2.0
OWN_GOALBOX_EDGE_Y     = OWN_GOAL_Y + GOALBOX_DEPTH
SIDELINE_FRINGE_WIDTH  = 19#20
GOAL_FRINGE_WIDTH      = 29#30
IMAGE_WIDTH            = 208
IMAGE_HEIGHT           = 160

# Context  , from Common.h - for gps ball info
CTGlobal                = 0
CTLocal                 = 1

#Visual Object indices , from vision/VisualCortex.h
vobBall = 0
vobBlueGoal = 1
vobYellowGoal = 2
vobBlueOnPinkBeacon = vobBlueLeftBeacon = 3
vobPinkOnBlueBeacon = vobBlueRightBeacon = 4
vobYellowOnPinkBeacon = vobYellowLeftBeacon = 5
vobPinkOnYellowBeacon = vobYellowRightBeacon = 6    
vobRedDog = 7
vobRedDog2 = 8
vobRedDog3 = 9
vobRedDog4 = 10
vobBlueDog = 11
vobBlueDog2 = 12
vobBlueDog3 = 13
vobBlueDog4 = 14

# Ball usage constants
VISION_BALL    = 0
GPS_BALL       = 1
WIRELESS_BALL  = 2
VBALL_LOST     = 12
VBALL_ANYWHERE = 40
MAX_TIME_TO_REACH = 32767
#LOSTBALL_TIME  = 100000 # 100 seconds.. Alex we can't send this big number through wireless

USE_WIRELESS_BALL_VAR = 100

# Max seconds to use a drop-in signal
DROP_IN_REACT_TIME = 2

# Wireless constants
NUM_TEAM_MEMBER = 4

# Basic behaviour constants, also used as an attack mode constants
FIRE_PAWKICK_AUTO      = 0
GET_BEHIND_BALL        = 1
FIELD_EDGE_SPIN        = 2
SPIN_DRIBBLE           = 3
WIRELESS_TEAMMATE_INFO = 4
HOVER_TO_BALL          = 5        # This is like the default case 
FIRE_PAWKICK_LEFT      = 6
FIRE_PAWKICK_RIGHT     = 7



# Supporter constants.
ON_EDGE_OFFSET = 20

# DKD Region constants.
DKDBlank       = 0
DKDRight       = 1
DKDLeft        = 2
DKDBottomRight = 3
DKDBottomLeft  = 4

# GameController related constants.
INITIALSTATE      = 0
READYSTATE        = 1
SETSTATE          = 2
PLAYINGSTATE      = 3
FINISHEDSTATE     = 4

#ROBOCUP_KICKOFF_INVALID  = 0x00
#ROBOCUP_KICKOFF_OWN      = 0x01
#ROBOCUP_KICKOFF_OPPONENT = 0x02

# Striker constants.
FURTHEST_OFFSET = 30

# Stuck detection constants.
STUCK_DIST_TO_BALL_THRESHOLD               = 10
STUCK_NUM_OF_FRAMES_WITHIN_THRESHOLD_LIMIT = 60
STUCK_SOLUTION_ONE_STANDSTILL              = 0
STUCK_SOLUTION_TWO_READYSTATE              = 1
STUCK_THRESHOLD                            = 1000

#Stuck detection related, which leg gets stuck
legFRONT_RIGHT = 1
legFRONT_LEFT  = 2
legBACK_RIGHT  = 3
legBACK_LEFT   = 4

# Ready state signal constants.
NOTDECIDED          = 0
DECIDEPOSITION_A    = 1
DECIDEPOSITION_B    = 2
DECIDEPOSITION_C    = 3
DECIDEPOSITION_D    = 4
POS_OFFSET_IN_KICK  = 7     # Hysterisis for in position
POS_OFFSET_OUT_KICK = 12 
POSITION_OFFSET     = 20
READY_HEADING_OFFSET= 10

# Stealth dog constans.
STEALTHDOG_OFFSET_ANGLE = 80

# Goalie constants.
WIDTH_MOVE_THRESHOLD  = 203
HEIGHT_MOVE_THRESHOLD = 155


CIRCLE_BOP    = 1
NORMAL_BOP    = 0
AVOIDBOX_BOP  = 2
POSTAVOID_BOP = 3


#Sensor values
ssMOUTH                 = 0
ssCHIN                  = 1
ssHEAD_TILT2            = 2
ssHEAD                  = 3
ssINFRARED_NEAR         = 4 
ssINFRARED_FAR          = 5
ssHEAD_PAN              = 6
ssHEAD_TILT1            = 7
ssFL_PALM               = 8
ssFL_KNEE               = 9
ssFL_ABDUCTOR           = 10
ssFL_ROTATOR            = 11
ssRL_PALM               = 12
ssRL_KNEE               = 13
ssRL_ABDUCTOR           = 14
ssRL_ROTATOR            = 15
ssFR_PALM               = 16
ssFR_KNEE               = 17
ssFR_ABDUCTOR           = 18
ssFR_ROTATOR            = 19
ssRR_PALM               = 20
ssRR_KNEE               = 21
ssRR_ABDUCTOR           = 22
ssRR_ROTATOR            = 23
ssTAIL_H                = 24
ssTAIL_V                = 25
ssACCEL_FOR             = 26
ssACCEL_SIDE            = 27
ssACCEL_Z               = 28
ssCHEST_INFRARED        = 29
ssWLAN                  = 30
ssBACK_REAR             = 31
ssBACK_MIDDLE           = 32
ssBACK_FRONT            = 33

# press sensors enum, copied from CommonSense.h
# used in VisionLink.getPressSensorCount(pressSensorID):
PRESS_HEAD = 0
PRESS_BACK_FRONT = 1
PRESS_BACK_MIDDLE = 2
PRESS_BACK_REAR = 3

# switch sensors enum, copied from CommonSense.h
# used in  VisionLink.getSwitchSensorCount(switchSensorID):
SWITCH_CHIN = 0
SWITCH_FL_PALM = 1
SWITCH_RL_PALM = 2
SWITCH_FR_PALM = 3
SWITCH_RR_PALM = 4


#
STATE_EXECUTING = 1
STATE_FAILED    = 2
STATE_SUCCESS   = 3


LOST_BALL_LAST_VISUAL = 4
LOST_BALL_LAST_GPS    = LOST_BALL_LAST_VISUAL + 7
LOST_BALL_HINT        = LOST_BALL_LAST_VISUAL + 7
LOST_BALL_GPS         = LOST_BALL_LAST_VISUAL + 7
LOST_BALL_SCAN        = LOST_BALL_GPS + 55
LOST_BALL_SPIN        = LOST_BALL_SCAN + 100

MIN_VIS_OBSTACLE_BOX = 10
MIN_GPS_OBSTACLE_BOX = 30


GRAB_TIMEOUT = 2500


# Flags for Obstacle Functions
# ----------------------------
# 
# OBS_USE_GPS = False && OBS_USE_LOCAL = False
# use gps obstacles if gps is valid this
# frame, else use local obstacles
# most cases should use this
# OBS_USE_GPS = True && OBS_USE_LOCAL = True
# same as above
# OBS_USE_GPS = True && OBS_USE_LOCAL = False
# use gps obstacles
# OBS_USE_GPS = False && OBS_USE_LOCAL = True
# use local obstacles

OBS_USE_GPS = 1
OBS_USE_LOCAL = 2 

# use constant obstacles (beacons, goals, etc)
# won't work in local

OBS_USE_CONST = 4

# use shared obstacles
# won't work in local

OBS_USE_SHARED = 8

# use no extra obstacles

OBS_USE_NONE = 0

# default flags

OBS_USE_DEFAULT = OBS_USE_CONST + OBS_USE_SHARED

# Debug data constants. These correspond to enum data_type in share/robolink.h
RLNK_ALL = 0
RLNK_DEBUG = 1
RLNK_CPLANE = 2
RLNK_YUVPLANE = 3
RLNK_CAM_ORIENT = 4
RLNK_SENSOR = 5
RLNK_SUBVISION = 6
RLNK_SUBOBJECT = 7
RLNK_INSANITY = 8
RLNK_PWALK = 9
RLNK_ATOMIC_ACTION = 10
RLNK_SUBCPLANE = 11
RLNK_DOGID = 12
RLNK_GPSINFO = 13
RLNK_BESTGAP = 14
RLNK_PINKOBJECT = 15
RLNK_GAUSSIANS = 16
#RLNK_NUM_DATA_TYPES
