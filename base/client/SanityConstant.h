/*

Copyright 2003 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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
 * $Id: SanityConstant.h 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Constants for the firing of sanity checks
 * The orders for these files should be the same
 * share/SanityConstant.h
 * client/SanityConstant.h
 * RoboCommander/SanityConstant.java
 * 
**/


#ifndef SanityConstant_h_DEFINED
#define SanityConstant_h_DEFINED

enum {
    YELLOW_GOAL_NOT_BLUE_RIGHT_BEACON = 0, 
    YELLOW_GOAL_NOT_BLUE_LEFT_BEACON,
    BLUE_GOAL_NOT_YELLOW_RIGHT_BEACON,
    BLUE_GOAL_NOT_YELLOW_LEFT_BEACON,
    BLUE_RIGHT_BEACON_NOT_YELLOW_LEFT_BEACON,
    YELLOW_LEFT_BEACON_NOT_BLUE_RIGHT_BEACON,
    BLUE_LEFT_BEACON_NOT_YELLOW_LEFT_BEACON,
    YELLOW_LEFT_BEACON_NOT_BLUE_LEFT_BEACON,
    BLUE_LEFT_BEACON_NOT_YELLOW_RIGHT_BEACON,
    YELLOW_RIGHT_BEACON_NOT_BLUE_LEFT_BEACON,
    BLUE_RIGHT_BEACON_NOT_YELLOW_RIGHT_BEACON,
    YELLOW_RIGHT_BEACON_NOT_BLUE_RIGHT_BEACON,
    GREEN_LEFT_BEACON_NOT_GREEN_RIGHT_BEACON,
    GREEN_RIGHT_BEACON_NOT_GREEN_LEFT_BEACON,
    BLUE_LEFT_BEACON_OUTSIDE_EDGE,
    BLUE_RIGHT_BEACON_OUTSIDE_EDGE,
    YELLOW_LEFT_BEACON_OUTSIDE_EDGE,
    YELLOW_RIGHT_BEACON_OUTSIDE_EDGE,
    BEACON_HEIGHT_TOO_HIGH,
    BEACON_HEIGHT_TOO_HIGH2,
    BEACON_HEIGHT_TOO_HIGH3,
    BEACON_HEIGHT_TOO_HIGH4,
    BEACON_HEIGHT_TOO_HIGH5,
    BEACON_HEIGHT_TOO_HIGH6,
    NEGATIVE_BEACON_ELEVATION,
    NEGATIVE_BEACON_ELEVATION2,
    NEGATIVE_BEACON_ELEVATION3,
    NEGATIVE_BEACON_ELEVATION4,
    NEGATIVE_BEACON_ELEVATION5,
    NEGATIVE_BEACON_ELEVATION6,
    YELLOW_GOAL_TOO_LOW_AREA_1584,
    BLUE_GOAL_TOO_LOW_AREA_1584,
    YELLOW_GOAL_TOO_HIGH,
    BLUE_GOAL_TOO_HIGH,
    YELLOW_GOAL_TOO_LOW,
    BLUE_GOAL_TOO_LOW,
    YELLOW_BEACON_NOT_BLUE_GOAL,
    BLUE_BEACON_NOT_YELLOW_GOAL,
    BLUE_GOAL_TOO_FAR,
    YELLOW_GOAL_TOO_FAR,
    BLUE_GOAL_CONFIDENT_THAN_YELLOW_GOAL,
    YELLOW_GOAL_CONFIDENT_THAN_BLUE_GOAL,
    BLUE_GOAL_VARIANCE_LESS_THAN_YELLOW_GOAL,
    YELLOW_GOAL_VAIRANCE_LESS_THAN_BLUE_GOAL,
    INVALID_BEACON,
    INVALID_BEACON2,
    INVALID_BEACON3,
    INVALID_BEACON4,
    INVALID_BEACON5,
    INVALID_BEACON6,
    BEACON_EXPECTED_AREA_TOO_SMALL,
    BEACON_EXPECTED_AREA_TOO_SMALL2,
    BEACON_EXPECTED_AREA_TOO_SMALL3,
    BEACON_EXPECTED_AREA_TOO_SMALL4,
    BEACON_EXPECTED_AREA_TOO_SMALL5,
    BEACON_EXPECTED_AREA_TOO_SMALL6,
    BALL_TOO_THIN_OR_TOO_FLAT,
    BALL_ELEVATION_TOO_HIGH,
    NEED_3_POINTS_TO_FORM_BALL,
    POINTS_IN_STRAIGHT_LINE,
    BALL_HAS_0_WIDTH, 
    BALL_HAS_0_RADIUS,
    ZERO_HALF_BEACON_AREA_OR_PINK_AREA,
    PINK_TOO_FAR_APART_FROM_COLOUR_BLOB,
    BEACON_SURROUNDERED_BY_GREEN,
    UNPROPROTIONAL_BEACON,
    BEACON_ON_FLOOR_OR_UNPROPROTION,
    CANNOT_MATCH_PINK_BLOB,
    INCORRECT_BALL_ELEVATION_OR_FLAT_DISTANCE,
    BALL_NOT_BELOW_HORIZON,
    BALL_ELEVATION_GREATER_THAN_GOAL_ELEVATION,
    BALL_ELEVATION_GREATER_THAN_GOAL_ELEVATION2,
    BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION,
    BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION2,
    BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION3,
    BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION4,
    BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION5,
    BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION6,
    BALL_TOO_CLOSE_TO_BEACON,
    BALL_TOO_CLOSE_TO_BEACON2,
    BALL_TOO_CLOSE_TO_BEACON3,
    BALL_TOO_CLOSE_TO_BEACON4,
    BALL_TOO_CLOSE_TO_BEACON5,
    BALL_TOO_CLOSE_TO_BEACON6,
    BALL_OCCLUDED_BY_BEACON,
    BALL_OCCLUDED_BY_BEACON2,
    BALL_OCCLUDED_BY_BEACON3,
    BALL_OCCLUDED_BY_BEACON4,
    BALL_OCCLUDED_BY_BEACON5,
    BALL_OCCLUDED_BY_BEACON6,
    TOO_MUCH_RED_IN_BALL,
    TOO_MUCH_YELLOW_RED_IN_BALL,
    BALL_BOUNDING_BOX_INSIDE_RED_ROBOT_BOUNDING_BOX,
    BALL_TOUCHES_RED_BLOB,
    BALL_INSIDE_YELLOW_GOAL,
    BALL_OUTSIDE_FIELD,
    BALL_OCCUPIED_AREA_0,
    BALL_AREA_LT100_NOT_OCCUPY_MT40_PROJECTED_BOUNDING_BOX,
    BALL_AREA_LT150_NOT_OCCUPY_MT25_PROJECTED_BOUNDING_BOX,
    BALL_AREA_LT300_NO_GREEN_NEARBY,
    YELLOW_GOAL_INSIDE_BALL,
    BEACON_EITHER_OVERLAP_OR_TOO_CLOSE,
    BEACON_EITHER_OVERLAP_OR_TOO_CLOSE2,
    BEACON_EITHER_OVERLAP_OR_TOO_CLOSE3,
    BEACON_EITHER_OVERLAP_OR_TOO_CLOSE4,
    BEACON_EITHER_OVERLAP_OR_TOO_CLOSE5,
    BEACON_EITHER_OVERLAP_OR_TOO_CLOSE6,
    BLOB_ALREADY_MATCHED,
    BLOB_AREA_LT30,
    BLOB_TOO_HIGH,
    BLOB_ELEVATION_GT_BEACON_ELEVATION,
    BLOB_ELEVATION_GT_BEACON_ELEVATION2,
    BLOB_ELEVATION_GT_BEACON_ELEVATION3,
    BLOB_ELEVATION_GT_BEACON_ELEVATION4,
    BLOB_ELEVATION_GT_BEACON_ELEVATION5,
    BLOB_ELEVATION_GT_BEACON_ELEVATION6,
    BLOB_TOO_SMALL,
    BLOB_LEFT_OF_LEFT_BEACON,
    BLOB_RIGHT_OF_RIGHT_BEACON,
    TOO_MUCH_BACKGROUND_COLOUR_BETWEEN_BLOBS,
    BLOB_BOTTOM_TOO_HIGH_FROM_CURRENT_GOAL_BLOB,
    BLOB_AREA_LESS_THAN_MINIMUM_GOAL_SIZE,
    BLOB_DENSITY_TOO_LOW,
    NO_GREEN_NEAR_BLOB,
    TINY_BLOB_WIDTH_OR_HEIGHT,
    BLOB_LEFT_OF_LEFT_BEACON2,
    BLOB_RIGHT_OF_RIGHT_BEACON2,
    RED_ROBOT_DISTANCE_GT240,
    BLUE_ROBOT_DISTANCE_GT165,
    BLUE_BLOB_ABOVE_BEACON,
    BLUE_BLOB_ABOVE_BEACON2,
    BLUE_BLOB_ABOVE_BEACON3,
    BLUE_BLOB_ABOVE_BEACON4,
    BLUE_BLOB_ABOVE_BEACON5,
    BLUE_BLOB_ABOVE_BEACON6,
    RED_BLOB_ABOVE_BEACON,
    RED_BLOB_ABOVE_BEACON2,
    RED_BLOB_ABOVE_BEACON3,
    RED_BLOB_ABOVE_BEACON4,
    RED_BLOB_ABOVE_BEACON5,
    RED_BLOB_ABOVE_BEACON6,
    BLUE_BLOB_ABOVE_GOAL,
    BLUE_BLOB_ABOVE_GOAL2,
    RED_BLOB_ABOVE_GOAL,
    RED_BLOB_ABOVE_GOAL2,
    BLUE_BLOB_ABOVE_HEAD,
    RED_BLOB_ABOVE_HEAD,
    RED_BLOB_AREA_0,
    BLUE_BLOB_AREA_0,
    RED_BLOB_NEAR_BALL,
    BLUE_BLOB_NEAR_BALL,
    BLUE_BLOB_AROUND_RED_ROBOT,
    BLUE_BLOB_AROUND_RED_ROBOT2,
    BLUE_BLOB_AROUND_RED_ROBOT3,
    BLUE_BLOB_AROUND_RED_ROBOT4,
    RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT,
    RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT2,
    RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT3,
    RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT4,
    BEACON_TOO_CLOSE_TO_ROBOT_BLOB,
    BEACON_TOO_CLOSE_TO_ROBOT_BLOB2,
    BEACON_TOO_CLOSE_TO_ROBOT_BLOB3,
    BEACON_TOO_CLOSE_TO_ROBOT_BLOB4,
    BEACON_TOO_CLOSE_TO_ROBOT_BLOB5,
    BEACON_TOO_CLOSE_TO_ROBOT_BLOB6,
    ROBOT_BLOB_TOO_HIGH_TO_BEACON,
    ROBOT_BLOB_TOO_HIGH_TO_BEACON2,
    ROBOT_BLOB_TOO_HIGH_TO_BEACON3,
    ROBOT_BLOB_TOO_HIGH_TO_BEACON4,
    ROBOT_BLOB_TOO_HIGH_TO_BEACON5,
    ROBOT_BLOB_TOO_HIGH_TO_BEACON6,
    GOAL_TOO_CLOSE_TO_ROBOT_BLOB,
    GOAL_TOO_CLOSE_TO_ROBOT_BLOB2,
    ROBOT_BLOB_TOO_HIGH_TO_GOAL,
    ROBOT_BLOB_TOO_HIGH_TO_GOAL2,
    ROBOT_TILT_TOO_HIGH,
    ROBOT_TILT_TOO_LOW,
    BLOB_TOO_FAR,
    BLOB_TOO_WIDE,
    BLOB_TOO_TALL,
    BLOB_TOO_THIN,
    BLOB_TOO_LOW,
    WRONG_BLOB_ASPECT_RATIO,
    SANITY_COUNT // the number of sanity checks. This should always be the last one
};

#endif // SanityConstant_h_DEFINED


