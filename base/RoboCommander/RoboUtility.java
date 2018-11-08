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
 * $Id: RoboUtility.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * contains static utility functions for RoboCommander
 *
**/

import java.util.StringTokenizer;

public class RoboUtility {

    private static final boolean objectCoordDebugMsg = false;
    
    // return true iff this object is stationary
    public static boolean isStationary(int i) {
        if (i==RoboConstant.BLUE_GOAL || 
            i==RoboConstant.YELLOW_GOAL || 
            i==RoboConstant.BLUE_LEFT_BEACON || 
            i==RoboConstant.BLUE_RIGHT_BEACON ||
            i==RoboConstant.GREEN_LEFT_BEACON || 
            i==RoboConstant.GREEN_RIGHT_BEACON || 
            i==RoboConstant.YELLOW_LEFT_BEACON || 
            i==RoboConstant.YELLOW_RIGHT_BEACON) {
            return true;
        }
        return false;
    }
    
    // convert an array of byte to an integer
    public static int bytesToInt(byte[] data, int offset) {
        int[] array = {unsignedToInt(data[offset]), unsignedToInt(data[offset+1]), 
                       unsignedToInt(data[offset+2]), unsignedToInt(data[offset+3])};
        if (objectCoordDebugMsg) {
            if (data[offset]<0) {
                System.out.println("RoboUtility.java: data[offset] " + data[offset] + " < 0");
            }
            if (data[offset+1]<0) {
                System.out.println("RoboUtility.java: data[offset+1] " + data[offset+1] + " < 0");
            }
            if (data[offset+2]<0) {
                System.out.println("RoboUtility.java: data[offset+2] " + data[offset+2] + " < 0");
            }
            if (data[offset+3]<0) {
                System.out.println("RoboUtility.java: data[offset+3] " + data[offset+3] + " < 0");
            }
        }
        return (array[3] << 24) | 
               (array[2] << 16) | 
               (array[1] << 8) | 
               (array[0]);
    }
    
    public static long bytesToLong(byte[] data, int offset) {
        int[] array = {unsignedToInt(data[offset]), unsignedToInt(data[offset+1]), 
                       unsignedToInt(data[offset+2]), unsignedToInt(data[offset+3]),
                       unsignedToInt(data[offset+4]), unsignedToInt(data[offset+5]),
                       unsignedToInt(data[offset+6]), unsignedToInt(data[offset+7])};
        return (array[7] << 56) | 
               (array[6] << 48) | 
               (array[5] << 40) | 
               (array[4] << 32) |
               (array[3] << 24) | 
               (array[2] << 16) | 
               (array[1] << 8) | 
               (array[0]);
    }


    // convert a byte to int
    public static int unsignedToInt(byte b) {
        return b & 0xff;
    }

    // return an array of string separated by one or more delimiters in a line
    // used for reading file
    public static String[] split(String line) {
        return split(line, " ");
    }
    public static String[] split(String line, String delimiter) {
        StringTokenizer st;
        int index;
        String[] array;

        index = 0;
        st    = new StringTokenizer(line, delimiter);
        while (st.hasMoreTokens()) {
            st.nextToken();
            index++;
        }
        
        array = new String[index];
        index = 0;
        st    = new StringTokenizer(line, delimiter);
        while (st.hasMoreTokens()) {
            array[index] = st.nextToken();
            index++;
        }
        
        return array;
    }
    
    public static int valueType(String value) {
        try {
            int num = Integer.parseInt(value.trim());
            if (num>=RoboConstant.START_VALUE) {
                return RoboConstant.START_TYPE;
            } else if (num==RoboConstant.STOP_VALUE) {
                return RoboConstant.STOP_TYPE;
            }
        } catch (Exception e) {
            return RoboConstant.UNKNOWN_TYPE;
        }
        return RoboConstant.UNKNOWN_TYPE;
    }
    
    // print the error on screen
    public static void printError(String filename, String msg) {
        System.out.println(filename + ": " + msg);
    }
    public static void printError(Object o, String msg) {
        printError(o.getClass().getName(), msg);
    }
    
    // convert a decimal to binary string
    public static String dec2Bin(int decimal) {
        if (decimal<=0) {
            return "0";
        }
        String binary = "";
        while (decimal>0) {
            binary   = decimal % 2 + binary;
            decimal /= 2;
        }
        return binary;
    }
}
