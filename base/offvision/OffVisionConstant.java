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
 * $Id: OffVisionConstant.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * the constants used in the other java files
 *
**/

import java.awt.*;
import java.awt.event.*;

class OffVisionConstant {

    // directory of where the input files locate
    public static final String FRAME_DIRECTORY     = "frames";
    public static final String OBJECT_DIRECTORY    = "objects";
    public static final String OBJECT_EXTENSION    = "object";
    public static final String BLOB_DIRECTORY      = "blobs";
    public static final String BLOB_EXTENSION      = "blob";

    // directory of where the blob output files locate
    public static final String BLOB_DATA_DIRECTORY = "sanityLearning";
    public static final String BLOB_DATA_EXT       = ".data";

    public static final String[] objectNames = {
        new String("Ball"), new String("BlueGoal"), new String("YellowGoal"),
        new String("BlueLeft"), new String("BlueRight"), new String("GreenLeft"),
        new String("GreenRight"), new String("YellowLeft"), new String("YellowRight"),
        new String("RedDog1"), new String("RedDog2"), new String("RedDog3"), 
        new String("RedDog4"), new String("BlueDog"), new String("BlueDog2"), 
        new String("BlueDog3"), new String("BlueDog4")
    };

    public static final String[] detailObjectNames = {
        new String("Ball"), 
        new String("Blue Goal"), new String("Yellow Goal"),
        new String("Blue(top)/Pink"), new String("Pink(top)/Blue"), 
        new String("Green(top)/Pink"), new String("Pink(top)/Green"), 
        new String("Yellow(top)/Pink"), new String("Pink(top)/Yellow"),
        new String("Red Dog1"), new String("Red Dog2"), new String("Red Dog3"), new String("Red Dog4"), 
        new String("Blue Dog"), new String("Blue Dog2"), new String("Blue Dog3"), new String("Blue Dog4")
    };

    public static final String[] blobFileName = {
        new String("ball"), 
        new String("blueGoal"), new String("yellowGoal"),
        new String("blueTopPink"), new String("pinkTopBlue"), 
        new String("greenTopPink"), new String("pinkTopGreen"), 
        new String("yellowTopPink"), new String("pinkTopYellow"),
        new String("redDog"), new String("blueDog")
    };

    public static final Color LIGHT_BLUE  = new Color(127,127,255);
    public static final Color DARK_GREEN  = new Color(0,127,0);
    public static final Color DARK_BLUE   = new Color(0,0,200);
    public static final Color GREEN       = new Color(127,255,127);
    public static final Color LIGHT_BLACK = new Color(200, 200, 200);
    public static final Color ERROR_COLOR = Color.cyan;

    /* This is same as the classifyColours in RoboConstant.java, except for
     * the very last one for the background. */

    public static final Color[] classifyColours = {
        Color.orange,   // new Color(255, 127, 0)
        LIGHT_BLUE,     // 1
        DARK_GREEN,     // 2
        Color.yellow,   // 3
        Color.pink,     // new Color(255,127,127)
        DARK_BLUE,      // 5
        Color.red,      // 6
        GREEN,          // 7
        LIGHT_BLACK,    // 8
        Color.white,    // 9
        Color.black,    // 10
        Color.lightGray,// 11
        Color.gray,		// 12
        new Color(127, 63, 0),  // 13
        new Color(0,63,0),      // 14
        new Color(127, 127, 0), // 15
        ERROR_COLOR
    };
}
