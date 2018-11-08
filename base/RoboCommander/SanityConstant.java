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
 * UNSW 2003 Robocup (Alex Tang)
 *
 * Last modification background information
 * $Id: SanityConstant.java 1953 2003-08-21 03:51:39Z eileenm $
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

public class SanityConstant {
    // the order of this array should be the same as the contstans in robot/vision/VisualCortex.h
    public static final String[] sanityDesc = {
        "See Yellow Goal. Ignore Pink(top)/Blue Beacon - YELLOW_GOAL_NOT_BLUE_RIGHT_BEACON",
        "See Yellow Goal. Ignore Blue(top)/Pink Beacon - YELLOW_GOAL_NOT_BLUE_LEFT_BEACON",
        "See Blue Goal. Ignore Pink(top)/Yellow Beacon - BLUE_GOAL_NOT_YELLOW_RIGHT_BEACON",
        "See Blue Goal. Ignore Yellow(top)/Pink Beacon - BLUE_GOAL_NOT_YELLOW_LEFT_BEACON",
        "See Pink(top)/Blue Beacon. Ignore Yellow(top)/Pink Beacon. - BLUE_RIGHT_BEACON_NOT_YELLOW_LEFT_BEACON",
        "See Yellow(top)/Pink Beacon. Ignore Pink(top)/Blue Beacon. - YELLOW_LEFT_BEACON_NOT_BLUE_RIGHT_BEACON",
        "See Blue(top)/Pink Beacon. Ignore Yellow(top)/Pink Beacon. - BLUE_LEFT_BEACON_NOT_YELLOW_LEFT_BEACON",
        "See Yellow(top)/Pink Beacon. Ignore Blue(top)/Pink Beacon. - YELLOW_LEFT_BEACON_NOT_BLUE_LEFT_BEACON",
        "See Blue(top)/Pink Beacon. Ignore Pink(top)/Yellow Beacon. - BLUE_LEFT_BEACON_NOT_YELLOW_RIGHT_BEACON",
        "See Pink(top)/Yellow Beacon. Ignore Blue(top)/Pink Beacon. - YELLOW_RIGHT_BEACON_NOT_BLUE_LEFT_BEACON",
        "See Pink(top)/Blue Beacon. Ignore Pink(top)/Yellow Beacon. - BLUE_RIGHT_BEACON_NOT_YELLOW_RIGHT_BEACON",
        "See Pink(top)/Yellow Beacon. Ignore Pink(top)/Blue Beacon. - YELLOW_RIGHT_BEACON_NOT_BLUE_RIGHT_BEACON",
        "See Green(top)/Pink Beacon. Ignore Pink(top)/Green Beacon. - GREEN_LEFT_BEACON_NOT_GREEN_RIGHT_BEACON",
        "See Pink(top)/Green Beacon. Ignore Green(top)/Pink Beacon. - GREEN_RIGHT_BEACON_NOT_GREEN_LEFT_BEACON",
        "Blue(top)/Pink Beacon outside edge - BLUE_LEFT_BEACON_OUTSIDE_EDGE",
        "Pink(top)/Blue Beacon outside edge - BLUE_RIGHT_BEACON_OUTSIDE_EDGE",
        "Yellow(top)/Pink Beacon outside edge - YELLOW_LEFT_BEACON_OUTSIDE_EDGE",
        "Pink(top)/Yellow Beacon outside edge - YELLOW_RIGHT_BEACON_OUTSIDE_EDGE",
        "Blue(top)/Pink Beacon too high - BEACON_HEIGHT_TOO_HIGH",
        "Pink(top)/Blue Beacon too high - BEACON_HEIGHT_TOO_HIGH(2)",
        "Green(top)/Pink Beacon too high - BEACON_HEIGHT_TOO_HIGH(3)",
        "Pink(top)/Green Beacon too high - BEACON_HEIGHT_TOO_HIGH(4)",
        "Yellow(top)/Pink Beacon too high - BEACON_HEIGHT_TOO_HIGH(5)",
        "Pink(top)/Yellow Beacon too high - BEACON_HEIGHT_TOO_HIGH(6)",
        "Negative Blue(top)/Pink Beacon Elevation - NEGATIVE_BEACON_ELEVATION",
        "Negative Pink(top)/Blue Beacon Elevation - NEGATIVE_BEACON_ELEVATION(2)",
        "Negative Green(top)/Pink Beacon Elevation - NEGATIVE_BEACON_ELEVATION(3)",
        "Negative Pink(top)/Green Beacon Elevation - NEGATIVE_BEACON_ELEVATION(4)",
        "Negative Yellow(top)/Pink Beacon Elevation - NEGATIVE_BEACON_ELEVATION(5)",
        "Negative Pink(top)/Yellow Beacon Elevation - NEGATIVE_BEACON_ELEVATION(6)",
        "Yellow Goal Too Low and Area less than 1584. Ignore Goal. - YELLOW_GOAL_TOO_LOW_AREA_1584",
        "Blue Goal Too Low and Area less than 1584. Ignore Goal. - BLUE_GOAL_TOO_LOW_AREA_1584",
        "Yellow Goal Too High. Ignore Goal - YELLOW_GOAL_TOO_HIGH",
        "Blue Goal Too High. Ignore Goal - BLUE_GOAL_TOO_HIGH",
        "Yellow Goal too Low - YELLOW_GOAL_TOO_LOW",
        "Blue Goal too low - BLUE_GOAL_TOO_LOW",
        "See Yellow Beacons. Ignore Blue Goal - YELLOW_BEACON_NOT_BLUE_GOAL",
        "See Blue Beacons. Ignore Yellow Goal - BLUE_BEACON_NOT_YELLOW_GOAL",
        "Blue Goal too Far - BLUE_GOAL_TOO_FAR",
        "Yellow Goal too Far - YELLOW_GOAL_TOO_FAR",
        "Blue Goal more confident than Yellow Goal. Ignore Yellow Goal - BLUE_GOAL_CONFIDENT_THAN_YELLOW_GOAL",
        "Yellow Goal more confident than Blue Goal. Ignore Blue Goal - YELLOW_GOAL_CONFIDENT_THAN_BLUE_GOAL",
        "Blue Goal variance less than Yellow Goal. Ignore Yellow Goal - BLUE_GOAL_VARIANCE_LESS_THAN_YELLOW_GOAL",
        "Yellow Goal variance less than Blue Goal. Ignore Blue Goal - YELLOW_GOAL_VAIRANCE_LESS_THAN_BLUE_GOAL",
        "Invalid Blue(top)/Pink Beacon - INVALID_BEACON",
        "Invalid Pink(top)/Blue Beacon - INVALID_BEACON(2)",
        "Invalid Green(top)/Pink Beacon - INVALID_BEACON(3)",
        "Invalid Pink(top)/Green Beacon - INVALID_BEACON(4)",
        "Invalid Yellow(top)/Pink Beacon - INVALID_BEACON(5)",
        "Invalid Pink(top)/Yellow Beacon - INVALID_BEACON(6)",
        "Blue(top)/Pink Beacon expected area too small - BEACON_EXPECTED_AREA_TOO_SMALL",
        "Pink(top)/Blue Beacon expected area too small - BEACON_EXPECTED_AREA_TOO_SMALL(2)",
        "Green(top)/Pink Beacon expected area too small - BEACON_EXPECTED_AREA_TOO_SMALL(3)",
        "Pink(top)/Green Beacon expected area too small - BEACON_EXPECTED_AREA_TOO_SMALL(4)",
        "Yellow(top)/Pink Beacon expected area too small - BEACON_EXPECTED_AREA_TOO_SMALL(5)",
        "Pink(top)/Yellow Beacon expected area too small - BEACON_EXPECTED_AREA_TOO_SMALL(6)",
        "Ball too thin or too flat - BALL_TOO_THIN_OR_TOO_FLAT",
        "Ball elevation too high - BALL_ELEVATION_TOO_HIGH",
        "Need 3 Points to form circle. Ignore Ball Blob - NEED_3_POINTS_TO_FORM_BALL",
        "3 Points of Ball in straight line. Ignore Ball - POINTS_IN_STRAIGHT_LINE",
        "Ball has 0 width. Ignore Ball - BALL_HAS_0_WIDTH",
        "Ball has 0 radius. Ignore Ball - BALL_HAS_0_RADIUS",
        "Either 0 Half Beacon area or 0 Pink area. Cannot be a Beacon - ZERO_HALF_BEACON_AREA_OR_PINK_AREA",
        "Pink too far apart from colour blob. Cannot be a Beacon - PINK_TOO_FAR_APART_FROM_COLOUR_BLOB",
        "Beacon surroundered by a lot of green. Ignore Beacon - BEACON_SURROUNDERED_BY_GREEN",
        "Unproportional Beacon. Ignore Beacon - UNPROPROTIONAL_BEACON",
        "Beacon sourroundered by a lot of green or unproportion - BEACON_ON_FLOOR_OR_UNPROPROTION",
        "Beacon cannot match Pink blob. Ignore Beacon - CANNOT_MATCH_PINK_BLOB",
        "Incorrect Ball elevation or flat distance. Ignore Ball - INCORRECT_BALL_ELEVATION_OR_FLAT_DISTANCE",
        "Ball is closer than infinity than below mathematical horizon. Ignore Ball - BALL_NOT_BELOW_HORIZON",
        "Ball elevation greater than Blue Goal elevation. Ignore Ball - BALL_ELEVATION_GREATER_THAN_GOAL_ELEVATION",
        "Ball elevation greater than Yellow Goal elevation. Ignore Ball - BALL_ELEVATION_GREATER_THAN_GOAL_ELEVATION(2)",
        "Ball elevation greater than Blue(top)/Pink Beacon. Ignore Ball - BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION",
        "Ball elevation greater than Pink(top)/Blue Beacon. Ignore Ball - BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION(2)",
        "Ball elevation greater than Green(top)/Pink Beacon. Ignore Ball - BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION(3)",
        "Ball elevation greater than Pink(top)/Green Beacon. Ignore Ball - BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION(4)",
        "Ball elevation greater than Yellow(top)/Pink Beacon. Ignore Ball - BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION(5)",
        "Ball elevation greater than Pink(top)/Yellow Beacon. Ignore Ball - BALL_ELEVATION_GREATER_THAN_BEACON_ELEVATION(6)",
        "Ball too close to Blue(top)/Pink Beacon. Ignore Ball - BALL_TOO_CLOSE_TO_BEACON",
        "Ball too close to Pink(top)/Blue Beacon. Ignore Ball - BALL_TOO_CLOSE_TO_BEACON(2)",
        "Ball too close to Green(top)/Pink Beacon. Ignore Ball - BALL_TOO_CLOSE_TO_BEACON(3)",
        "Ball too close to Pink(top)/Green Beacon. Ignore Ball - BALL_TOO_CLOSE_TO_BEACON(4)",
        "Ball too close to Yellow(top)/Pink Beacon. Ignore Ball - BALL_TOO_CLOSE_TO_BEACON(5)",
        "Ball too close to Pink(top)/Yellow Beacon. Ignore Ball - BALL_TOO_CLOSE_TO_BEACON(6)",
        "Ball occluded by Blue(top)/Pink Beacon. Ignore Ball - BALL_OCCLUDED_BY_BEACON",
        "Ball occluded by Pink(top)/Blue Beacon. Ignore Ball - BALL_OCCLUDED_BY_BEACON(2)",
        "Ball occluded by Green(top)/Pink Beacon. Ignore Ball - BALL_OCCLUDED_BY_BEACON(3)",
        "Ball occluded by Pink(top)/Green Beacon. Ignore Ball - BALL_OCCLUDED_BY_BEACON(4)",
        "Ball occluded by Yellow(top)/Pink Beacon. Ignore Ball - BALL_OCCLUDED_BY_BEACON(5)",
        "Ball occluded by Pink(top)/Yellow Beacon. Ignore Ball - BALL_OCCLUDED_BY_BEACON(6)",
        "Too much red in Ball. Ignore Ball - TOO_MUCH_RED_IN_BALL",
        "Too much yellow and red in Ball. Ignore Ball - TOO_MUCH_YELLOW_RED_IN_BALL",
        "Ball Bounding box completely inside Red Robot Bounding box. Ignore Ball - BALL_BOUNDING_BOX_INSIDE_RED_ROBOT_BOUNDING_BOX",
        "Ball touches Red blob. Ignore Ball - BALL_TOUCHES_RED_BLOB",
        "Ball inside Yellow Goal. Ignore Ball - BALL_INSIDE_YELLOW_GOAL",
        "Ball outside field. Ignore Ball - BALL_OUTSIDE_FIELD",
        "Ball occupied area is 0. Ignore Ball - BALL_OCCUPIED_AREA_0",
        "Ball area less than 100 and not occupy more than 40% of area of projected bounding box. Ignore Ball - BALL_AREA_LT100_NOT_OCCUPY_MT40_PROJECTED_BOUNDING_BOX",
        "Ball area less than 150 and not occupy more than 25% of area of projected bounding box. Ignore Ball - BALL_AREA_LT150_NOT_OCCUPY_MT25_PROJECTED_BOUNDING_BOX",
        "Ball area less than 300 and no green near by. Ignore Ball - BALL_AREA_LT300_NO_GREEN_NEARBY",
        "Yellow Goal inside Ball. Ignore Goal - YELLOW_GOAL_INSIDE_BALL",
        "Blue(top)/Pink Beacon either overlap or too close. Ignore Beacon - BEACON_EITHER_OVERLAP_OR_TOO_CLOSE",
        "Pink(top)/Blue Beacon either overlap or too close. Ignore Beacon - BEACON_EITHER_OVERLAP_OR_TOO_CLOSE(2)",
        "Green(top)/Pink Beacon either overlap or too close. Ignore Beacon - BEACON_EITHER_OVERLAP_OR_TOO_CLOSE(3)",
        "Pink(top)/Green Beacon either overlap or too close. Ignore Beacon - BEACON_EITHER_OVERLAP_OR_TOO_CLOSE(4)",
        "Yellow(top)/Pink Beacon either overlap or too close. Ignore Beacon - BEACON_EITHER_OVERLAP_OR_TOO_CLOSE(5)",
        "Pink(top)/Yellow Beacon either overlap or too close. Ignore Beacon - BEACON_EITHER_OVERLAP_OR_TOO_CLOSE(6)",
        "Blob already matched. Cannot be a Goal - BLOB_ALREADY_MATCHED",
        "Blob area less than 30. Cannot be a Goal - BLOB_AREA_LT30",
        "Blob too high. Cannot be a Goal - BLOB_TOO_HIGH",
        "Blob elevation greater than Blue(top)/Pink Beacon elevation. Cannot be a Goal - BLOB_ELEVATION_GT_BEACON_ELEVATION",
        "Blob elevation greater than Pink(top)/Blue Beacon elevation. Cannot be a Goal - BLOB_ELEVATION_GT_BEACON_ELEVATION(2)",
        "Blob elevation greater than Green(top)/Pink Beacon elevation. Cannot be a Goal - BLOB_ELEVATION_GT_BEACON_ELEVATION(3)",
        "Blob elevation greater than Pink(top)/Green Beacon elevation. Cannot be a Goal - BLOB_ELEVATION_GT_BEACON_ELEVATION(4)",
        "Blob elevation greater than Yellow(top)/Pink Beacon elevation. Cannot be a Goal - BLOB_ELEVATION_GT_BEACON_ELEVATION(5)",
        "Blob elevation greater than Pink(top)/Yellow Beacon elevation. Cannot be a Goal - BLOB_ELEVATION_GT_BEACON_ELEVATION(6)",
        "Blob too small. Cannot be a Goal - BLOB_TOO_SMALL",
        "Blob on the left of the left Beacon. Cannot be a Goal - BLOB_LEFT_OF_LEFT_BEACON",
        "Blob on the right of the right Beacon. Cannot be a Goal - BLOB_RIGHT_OF_RIGHT_BEACON",
        "Too much background colours between Blobs. Cannot be a Goal - TOO_MUCH_BACKGROUND_COLOUR_BETWEEN_BLOBS",
        "Bottom of blob too high from current Goal Blob. Cannot be a Goal - BLOB_BOTTOM_TOO_HIGH_FROM_CURRENT_GOAL_BLOB",
        "Blob area less than minimum goal size. Cannot be a Goal - BLOB_AREA_LESS_THAN_MINIMUM_GOAL_SIZE",
        "Blob density too low. Cannot be a Goal - BLOB_DENSITY_TOO_LOW",
        "No Green near blob. Cannot be a Goal - NO_GREEN_NEAR_BLOB",
        "Tiny blob width or height. Cannot be a Goal - TINY_BLOB_WIDTH_OR_HEIGHT",
        "Blob on the left of the left Beacon 2. Cannot be a Goal - BLOB_LEFT_OF_LEFT_BEACON2",
        "Blob on the right of the right Beacon 2. Cannot be a Goal - BLOB_RIGHT_OF_RIGHT_BEACON2",
        "Red Robot distance greater than 240. Ignore Robot - RED_ROBOT_DISTANCE_GT240",
        "Blue Robot distance greater than 165. Ignore Robot - BLUE_ROBOT_DISTANCE_GT165",
        "Blue blob above Blue(top)/Pink Beacon. Cannot be Blue Robot - BLUE_BLOB_ABOVE_BEACON",
        "Blue blob above Pink(top)/Blue Beacon. Cannot be Blue Robot - BLUE_BLOB_ABOVE_BEACON(2)",
        "Blue blob above Green(top)/Pink Beacon. Cannot be Blue Robot - BLUE_BLOB_ABOVE_BEACON(3)",
        "Blue blob above Pink(top)/Green Beacon. Cannot be Blue Robot - BLUE_BLOB_ABOVE_BEACON(4)",
        "Blue blob above Yellow(top)/Pink Beacon. Cannot be Blue Robot - BLUE_BLOB_ABOVE_BEACON(5)",
        "Blue blob above Pink(top)/Yellow Beacon. Cannot be Blue Robot - BLUE_BLOB_ABOVE_BEACON(6)",
        "Red blob above Blue(top)/Pink Beacon. Cannot be Red Robot - RED_BLOB_ABOVE_BEACON",
        "Red blob above Pink(top)/Blue Beacon. Cannot be Red Robot - RED_BLOB_ABOVE_BEACON(2)",
        "Red blob above Green(top)/Pink Beacon. Cannot be Red Robot - RED_BLOB_ABOVE_BEACON(3)",
        "Red blob above Pink(top)/Green Beacon. Cannot be Red Robot - RED_BLOB_ABOVE_BEACON(4)",
        "Red blob above Yellow(top)/Pink Beacon. Cannot be Red Robot - RED_BLOB_ABOVE_BEACON(5)",
        "Red blob above Pink(top)/Yellow Beacon. Cannot be Red Robot - RED_BLOB_ABOVE_BEACON(6)",
        "Blue blob above Blue Goal. Cannot be Blue Robot - BLUE_BLOB_ABOVE_GOAL",
        "Blue blob above Yellow Goal. Cannot be Blue Robot - BLUE_BLOB_ABOVE_GOAL(2)",
        "Red blob above Blue Goal. Cannot be Red Robot - RED_BLOB_ABOVE_GOAL",
        "Red blob above Yellow Goal. Cannot be Red Robot - RED_BLOB_ABOVE_GOAL(2)",
        "Blue blob above Head. Cannot be Blue Robot - BLUE_BLOB_ABOVE_HEAD",
        "Red blob above Head. Cannot be Red Robot - RED_BLOB_ABOVE_HEAD",
        "Red blob area 0. Cannot be Red Robot - RED_BLOB_AREA_0",
        "Blue blob area 0. Cannot be Blue Robot - BLUE_BLOB_AREA_0",
        "Red blob too close to Ball. Cannot be Red Robot - RED_BLOB_NEAR_BALL",
        "Blue blob too close to Ball. Cannot be Blue Robot - BLUE_BLOB_NEAR_BALL",
        "Blue blob around red robot 1. Cannot be a Blue Robot - BLUE_BLOB_AROUND_RED_ROBOT",
        "Blue blob around red robot 2. Cannot be a Blue Robot - BLUE_BLOB_AROUND_RED_ROBOT(2)",
        "Blue blob around red robot 3. Cannot be a Blue Robot - BLUE_BLOB_AROUND_RED_ROBOT(3)",
        "Blue blob around red robot 4. Cannot be a Blue Robot - BLUE_BLOB_AROUND_RED_ROBOT(4)",
        "Identify Red robot 1's legs or area above bandana as blue dog - RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT",
        "Identify Red robot 2's legs or area above bandana as blue dog - RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT(2)",
        "Identify Red robot 3's legs or area above bandana as blue dog - RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT(3)",
        "Identify Red robot 4's legs or area above bandana as blue dog - RED_ROBOT_LEG_AREA_AS_BLUE_ROBOT(4)",
        "Blue(top)/Pink Beacon too close to Blob. Cannot be a Robot - BEACON_TOO_CLOSE_TO_ROBOT_BLOB",
        "Pink(top)/Blue Beacon too close to Blob. Cannot be a Robot - BEACON_TOO_CLOSE_TO_ROBOT_BLOB(2)",
        "Green(top)/Pink Beacon too close to Blob. Cannot be a Robot - BEACON_TOO_CLOSE_TO_ROBOT_BLOB(3)",
        "Pink(top)/Green Beacon too close to Blob. Cannot be a Robot - BEACON_TOO_CLOSE_TO_ROBOT_BLOB(4)",
        "Yellow(top)/Pink Beacon too close to Blob. Cannot be a Robot - BEACON_TOO_CLOSE_TO_ROBOT_BLOB(5)",
        "Pink(top)/Yellow Beacon too close to Blob. Cannot be a Robot - BEACON_TOO_CLOSE_TO_ROBOT_BLOB(6)",
        "Blob too high to Blue(top)/Pink Beacon. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_BEACON",
        "Blob too high to Pink(top)/Blue Beacon. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_BEACON(2)",
        "Blob too high to Green(top)/Pink Beacon. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_BEACON(3)",
        "Blob too high to Pink(top)/Green Beacon. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_BEACON(4)",
        "Blob too high to Yellow(top)/Pink Beacon. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_BEACON(5)",
        "Blob too high to Pink(top)/Yellow Beacon. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_BEACON(6)",
        "Blue Goal too close to Blob. Cannot be a Robot - GOAL_TOO_CLOSE_TO_ROBOT_BLOB",
        "Yellow Goal too close to Blob. Cannot be a Robot - GOAL_TOO_CLOSE_TO_ROBOT_BLOB(2)",
        "Blob too high to Blue Goal. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_GOAL",
        "Blob too high to Yellow Goal. Cannot be a Robot - ROBOT_BLOB_TOO_HIGH_TO_GOAL(2)",
        "Robot Effective tilt too high. Cannot be a Robot - ROBOT_TILT_TOO_HIGH",
        "Robot Effective tilt too low. Cannot be a Robot - ROBOT_TILT_TOO_LOW",
        "Blob too far. Ignore Bounding Box - BLOB_TOO_FAR",
        "Blob too wide. Ignore Bounding Box - BLOB_TOO_WIDE",
        "Blob too tall. Ignore Bounding Box - BLOB_TOO_TALL",
        "Blob too thin. Ignore Bounding Box - BLOB_TOO_THIN",
        "Blob too low. Ignore Bounding Box - BLOB_TOO_LOW",
        "Wrong Blob aspect ratio. Ignore Bounding Box - WRONG_BLOB_ASPECT_RATIO",

        "BEACON_PINK_TOO_HIGH_TO_HORIZON",
        "BEACON_PINK_TOO_LOW_TO_HORIZON",
        "BEACON_TOO_LONG",
        "BEACON_NOT_VERTICAL_TO_HORIZON",
        "BEACON_TOO_HIGH_TO_HORIZON",
        "BEACON_TOO_LOW_TO_HORIZON",
        "BLUE_GOAL_IN_CORNER",
        "TOO_LITTLE_ORANGE_IN_BALL",
        "BEACON_ACTUALLY_PART_OF_BALL",
        "BLUE_ROBOT_IN_CORNER",
        "GOAL_INSIDE_ROBOT",

        "BEACON_NON_PINK_BLOB_TOO_SMALL",
        "BEACON_NON_PINK_BLOB_SMALL_N_SCATTERED",
        "BEACON_NON_PINK_BLOB_SMALL_N_LARGE_ASPECT_RATIO",
        "BEACON_HAS_NEGATIVE_ELEVATION",
        "BEACON_BAD_BOUNDING_BOX_ASPECT_RATIO",
        "BEACON_NOT_ENOUGH_WHITE_BELOW",
        "BEACON_BAD_HEIGHT",

        "BALL_DENSITY_SMALL_AND_IN_EDGE",
        "BALL_ABOVE_HORIZON",
        "BALL_TOO_FAR",
        "BALL_HEIGHT_TOO_LARGE",

        "dummy",
        "dummy",
        "dummy",
        "dummy",
        "dummy"
    };
}





