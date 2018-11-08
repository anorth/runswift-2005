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


package RoboShare;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */
import java.awt.*;

public class CommonSense {
    public static final byte ORANGE = 0;
    public static final byte BLUE = 1;
    public static final byte GREEN = 2;
    public static final byte YELLOW = 3;
    public static final byte PINK = 4;
    public static final byte BLUE_ROBOT = 5;
    public static final byte RED_ROBOT = 6;
    public static final byte GREEN_FIELD = 7;
    public static final byte GREY = 8;
    public static final byte WHITE = 9;
    public static final byte BLACK = 10;
    //public static final byte FIELD_LINE = 11;
    //public static final byte FIELD_BORDER = 12;
    public static final byte BACKGROUND = 127;

    //public static final int NUMBER_USABLE_COLOR = 13;
    public static final int NUMBER_USABLE_COLOR = 11;

    public static final Color indexToColour[] = {
        Color.ORANGE,
        Color.CYAN, //blue beacon is actually cyan
        Color.GREEN, //no longer exists
        Color.YELLOW,
        Color.PINK,
        Color.BLUE,
        Color.RED,
        Color.GREEN,
        Color.GRAY,
        Color.WHITE, //9
        Color.LIGHT_GRAY,
        Color.DARK_GRAY,	// NUMBER_USABLE_COLOR == default colour when out of range
        //Color.WHITE
    };

    //public static final byte NOCOLOUR = 13;
    //public static final byte ALLCOLOUR = 14;
    public static final byte NOCOLOUR = 11;
    public static final byte ALLCOLOUR = 12;

    public static final int CPLANE_WIDTH = 208;
    public static final int CPLANE_HEIGHT = 160;

    public static final String COLOUR_NAME[] = {
        "Orange", //0
        "Blue",
        "XXXX",
        "Yellow",
        "Pink",
        "Blue robot", //5
        "Red robot",
        "Green field",
        "Grey",
        "White",
        "Black", //10
//        "Field line",
//        "Field border",
        "No Colors",
        "All Colors"
    };

    //blob classification
    public static final int NOT_KNOWN = 0;
    public static final int IS_BLOB = 1;
    public static final int IS_NOT_BLOB = 2;
    public static final String[] CLASS_TO_STRING = {
        "UNKNOWN", "YES", "NO"};

    public static final int CUBE_SIZE = 128;
    public static final int NNMC_FACTOR = 2;

    public static String getColourName(int colour) {
        if (colour >= 0 && colour < COLOUR_NAME.length)
            return COLOUR_NAME[colour];
        else
            return "Unknown";
    }

    public static int getColourFromName(String colourName) {
        for (int colour = 0; colour < COLOUR_NAME.length; colour++)
            if (COLOUR_NAME[colour].equals(colourName))
                return colour;
        return -1;
    }

    public static String getBlobClassification(int C) {
        if (C >= 0 && C < CLASS_TO_STRING.length)
            return CLASS_TO_STRING[C];
        else
            return "$#^%";
    }
}
