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
 * @author Will Uther
 *
 * Last modification background information
 * modified by weiming 8/Dec/04
 * $Id: FieldDef.h 
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 * 
 **/



#ifndef OSAKA
#define OSAKA

static const int LEFT = 1;
static const int RIGHT = 2;

// actual dimension of the field
static const int FIELD_WIDTH = 363;//360; //was 290;
static const int FIELD_LENGTH = 542;//540 //was 440;
static const int FIELD_DIAGONAL = 652;//649; //was 527;
static const int WALL_THICKNESS = 0; //0 //was 10;
static const int GOAL_WIDTH = 80; // 80// was 60
static const int GOAL_LENGTH = 30;
static const int GOAL_HEIGHT = 30;
static const int CORNER_WIDTH = 0; //was 50; // this is the width of the corner triangle 
// + 2*wall thickness
static const int GOALBOX_WIDTH = 128;//130; //was 100;
static const int GOALBOX_DEPTH = 65; //65 ;//was 50;
static const int CIRCLE_DIAMETER = 36;// 36; //was 30;
static const double LINE_WIDTH = 2.4;// 2.5;// was 2.4; // width of tape


// new constants for beacons of 2005 field
static const int BEACON_EDGE_LINE_OFFSET_X = 13;//15;
static const int BEACON_BOTTOM_LINE_OFFSET_Y = 136;//135;
static const int BEACON_RADIUS = 5;

static const int BEACON_LEFT_X = -BEACON_EDGE_LINE_OFFSET_X;
static const int BEACON_RIGHT_X = FIELD_WIDTH + BEACON_EDGE_LINE_OFFSET_X;
static const int BEACON_CLOSE_Y = BEACON_BOTTOM_LINE_OFFSET_Y;
static const int BEACON_FAR_Y = FIELD_LENGTH - BEACON_BOTTOM_LINE_OFFSET_Y;

// the dropin points
static const int DROPIN_OFFSET_X = 40;
static const int DROPIN_OFFSET_Y = 100;

// the edge of the carpet beyond the field lines. Note that at the moment it is
// undefined as to what is beyond this point, it could even be more carpets :)
static const int SIDE_FRINGE_WIDTH = 19;//20
static const int GOAL_FRINGE_WIDTH = 29;//30
static const int SIDE_WALL_FRINGE_WIDTH = 88;
static const int GOAL_WALL_FRINGE_WIDTH = 89;
static const int SIDE_WALL_FRINGE_CENTER_Y = (FIELD_LENGTH + 2 * GOAL_WALL_FRINGE_WIDTH)/4;
static const int GOAL_WALL_FRINGE_CENTER_X = (FIELD_WIDTH + 2 * SIDE_WALL_FRINGE_WIDTH)/4;

static const double MAX_BEACON_DIST = sqrt((double)(BEACON_FAR_Y*BEACON_FAR_Y) 
    + ((FIELD_WIDTH + SIDE_FRINGE_WIDTH)*(FIELD_WIDTH + SIDE_FRINGE_WIDTH)));

#endif // OSAKA
