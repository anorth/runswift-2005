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
 * $Id: SanityConstant.h 6484 2005-06-08 11:06:57Z nmor250 $
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

// Remember to update sanityNames in SanityChecks.cc to match this
enum Insanity {
    SANE = 0,
    RDR_INSANITY,                   // machine learnt sanity
    
    BALL_ABOVE_HORIZON,             // ball insanities
    BALL_INFRARED_OBSTACLE,
    BALL_VERY_LARGE_RADIUS,
    BALL_NOT_MUCH_ORANGE,           
    BALL_TOO_RED,           
    BALL_TOO_PINK,     
    BALL_TOO_YELLOW,   
    BALL_NOT_MUCH_GREEN_AROUND,   
    
    GOAL_TOO_WIDE,                  // goal insanities
    GOAL_NOT_WIDE_ENOUGH,
    GOAL_FLOATING,
    GOAL_ABOVE_HORIZON,
    GOAL_IT_MIGHT_BE_BEACON,

    BEACON_BELOW_HORIZON,           // beacon insanities

    TWO_GOALS_CHOSE_HIGHEST_CF,     // world insanities
    TWO_GOALS_CHOSE_LOWEST_ELEV,
    GOAL_IN_BEACON,
    GOAL_ABOVE_BEACON,
    BEACON_IN_GOAL,
    BALL_ABOVE_GOAL,
    BALL_ABOVE_BEACON,
    DIAGONALLY_OPPOSITE_BEACONS,
    END_BEACONS_TOO_CLOSE,
    
    INSANITY_COUNT,                 // must be last entry
};

// see SanityChecks.cc
extern const char* sanityNames[];

#endif // SanityConstant_h_DEFINED
