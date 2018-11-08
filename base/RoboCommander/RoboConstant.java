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
 * $Id: RoboConstant.java 2001 2003-09-24 12:02:35Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * contains the constant used in the RoboCommander
 *
**/

import java.io.File;
import java.awt.Color;
import java.awt.event.KeyEvent;

public class RoboConstant {

    // size of java primitive types
    public static final int INT_BYTES    = 4;
    public static final int DOUBLE_BYTES = 8;
    public static final int LONG_BYTES   = 8;
    public static final int C_LONG_BYTES = 4;
    public static final int INT_BITS     = INT_BYTES * 8;

    // size of the cplane
    public static final int CPLANE_WIDTH           = 208;
    public static final int CPLANE_HEIGHT          = 160;
    public static final int CPLANE_SIZE            = CPLANE_WIDTH * CPLANE_HEIGHT;
    public static final int COMPRESSED_CPLANE_SIZE = CPLANE_SIZE * 2; // the maximum size of the compressed cplane using run-length encoding
    public static final int YUVPLANE_SIZE          = CPLANE_SIZE * 6;

    // size of the sensors reading
    public static final int NUM_SENSOR_COORDS  = 3;
    public static final int SENSOR_SIZE        = NUM_SENSOR_COORDS * C_LONG_BYTES;

    // size of the sanity values
    public static final int SANITY_COUNT       = SanityConstant.sanityDesc.length;
    public static final int NUM_SANITY_COORDS  = (SANITY_COUNT-1) / INT_BITS + 1;
    public static final int SANITY_SIZE        = NUM_SANITY_COORDS * INT_BYTES;

    // size of the object
    public static final int NUM_OBJ            = 17;
    public static final int NUM_INT_COORDS     = 6;
    public static final int ONE_OBJ_SIZE       = NUM_INT_COORDS * INT_BYTES;
    //public static final int NUM_ROBOT_DATA     = 7;
    public static final int NUM_ROBOT_DATA     = 2; // Tried to add 5 bytes for hz
                                                    // data but not ok yet.
    public static final int OBJ_SIZE           = NUM_OBJ*ONE_OBJ_SIZE + NUM_ROBOT_DATA*DOUBLE_BYTES;
    //public static final int OBJ_SIZE           = NUM_OBJ*ONE_OBJ_SIZE + NUM_ROBOT_DATA*INT_BYTES;

    public static final String DEFAULT_OPLANE_LOG_FILE = System.getProperty("user.dir", ".") + File.separator + "OPLANE_LOG";
    public static final String DEFAULT_CPLANE_LOG_FILE = System.getProperty("user.dir", ".") + File.separator + "CPLANE_LOG";
    public static final String DEFAULT_SENSOR_LOG_FILE = System.getProperty("user.dir", ".") + File.separator + "SENSOR_LOG";

    public static final int SCALE         = 3;

    public static final int WINDOW_WIDTH  = CPLANE_WIDTH  * SCALE;
    public static final int WINDOW_HEIGHT = CPLANE_HEIGHT * SCALE;

    public static final int NUM_PLANES    = 24;
    public static final int NUM_COL       = 8;

    // the delay in the play back of log file (in ms)
    public static final int OPLANE_NEXT_FRAME_DELAY = 100;
    public static final int CPLANE_NEXT_FRAME_DELAY = 1000;

    // specific colour for the drawing
    public static final Color BACKGROUND_COLOUR     = Color.white;
    public static final Color OBJECT_BORDER_COLOUR  = Color.black;
    public static final Color ROTATED_BORDER_COLOUR = Color.cyan;
    public static final Color NORMAL_BORDER_COLOUR  = Color.black;
    public static final Color HORIZON_COLOUR        = Color.cyan;

    /*
     * To have a smaller command data size
     * It is used in robot/share/SharedMemoryDef.h, robot/share/PackageDef.h
     * The following places will also need to be change
     * base/work/client/SwitchBoard.h
     * base/work/RoboCommander/RoboCommander.java
    **/
    public static final boolean REDUCE_COMMAND_DATA_SIZE = true;

    // maximum length the sending values can be
    // numbers should agree with base/work/client/PackageDef.h, robot/share/PackageDef.h
    // it includes the '\0' character which only client.c and robot needs
    public static final int NAME_SIZE  = REDUCE_COMMAND_DATA_SIZE? 6 : 72;
    public static final int VALUE_SIZE = REDUCE_COMMAND_DATA_SIZE? 20 : 72;

    // to compare with the effective roll
    public static final double HALF_PI_RADIAN   = Math.PI / 2.0;

    // the magniture (not the sign) should agree with robot/actuatorControl/HeadMotion.h
    // use left and down positive here
    public static final int UP_TILT_DEGREE      = 3;
    public static final int DOWN_TILT_DEGREE    = 80;
    public static final int LEFT_PAN_DEGREE     = 93;
    public static final int RIGHT_PAN_DEGREE    = 93;
    public static final double UP_TILT_RADIAN   = Math.toRadians(UP_TILT_DEGREE);
    public static final double DOWN_TILT_RADIAN = Math.toRadians(DOWN_TILT_DEGREE);
    public static final double LEFT_PAN_RADIAN  = Math.toRadians(LEFT_PAN_DEGREE);
    public static final double RIGHT_PAN_RADIAN = Math.toRadians(RIGHT_PAN_DEGREE);

    public static final double CAMERA_PAN  = 50;
    public static final double CAMERA_TILT = 35;

    /* The roll is multiplied by this factor so that only 4 bytes is used in the transmission, instead of 8.
     * has to agree among the following files: robot/vision/Vision.cc, client.h and RoboConstant.java
    **/
    public static final double SEND_SCALE = 1000.0;

	public static final int COLOUR_MASK = 0x0F;
	public static final int MAYBE_BIT = 0x10;

    public static final Color LIGHT_BLUE  = new Color(127,127,255);
    public static final Color DARK_GREEN  = new Color(0,127,0);
    public static final Color DARK_BLUE   = new Color(0,0,200);
    public static final Color GREEN       = new Color(127,255,127);
    public static final Color LIGHT_BLACK = new Color(200, 200, 200);
    public static final Color ERROR_COLOR = Color.cyan;

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
	/*
        new Color(127, 63, 63), // 15
        new Color(63,63,127),   // 12
        new Color(0,0,100),     // 16
        new Color(127, 0, 0),   // 17
        new Color(127,255,127)  // 18
	*/
    };

    public static final byte NO_COLOUR = (byte) (classifyColours.length - 1);

    public static final int BALL                = 0;
    public static final int BLUE_GOAL           = 1;
    public static final int YELLOW_GOAL         = 2;
    public static final int BLUE_LEFT_BEACON    = 3;
    public static final int BLUE_RIGHT_BEACON   = 4;
    public static final int GREEN_LEFT_BEACON   = 5;
    public static final int GREEN_RIGHT_BEACON  = 6;
    public static final int YELLOW_LEFT_BEACON  = 7;
    public static final int YELLOW_RIGHT_BEACON = 8;
    public static final int RED_DOG             = 9;
    public static final int RED_DOG2            = 10;
    public static final int RED_DOG3            = 11;
    public static final int RED_DOG4            = 12;
    public static final int BLUE_DOG            = 13;
    public static final int BLUE_DOG2           = 14;
    public static final int BLUE_DOG3           = 15;
    public static final int BLUE_DOG4           = 16;

    public static final String[] objectNames = {
        new String("Ball"),
        new String("Blue Goal"), new String("Yellow Goal"),
        new String("Blue(top)/Pink"), new String("Pink(top)/Blue"),
        new String("Green(top)/Pink"), new String("Pink(top)/Green"),
        new String("Yellow(top)/Pink"), new String("Pink(top)/Yellow"),
        new String("Red Dog1"), new String("Red Dog2"), new String("Red Dog3"), new String("Red Dog4"),
        new String("Blue Dog"), new String("Blue Dog2"), new String("Blue Dog3"), new String("Blue Dog4")
    };

    public static final Color BALL_COLOR                       = Color.orange;
    public static final Color BALL_KICKABLE_COLOR              = Color.pink;
    public static final Color BLUE_GOAL_COLOR                  = LIGHT_BLUE;
    public static final Color YELLOW_GOAL_COLOR                = Color.yellow;
    public static final Color BLUE_LEFT_BEACON_TOP_COLOR       = LIGHT_BLUE;
    public static final Color BLUE_LEFT_BEACON_BOTTOM_COLOR    = Color.pink;
    public static final Color BLUE_RIGHT_BEACON_TOP_COLOR      = Color.pink;
    public static final Color BLUE_RIGHT_BEACON_BOTTOM_COLOR   = LIGHT_BLUE;
    public static final Color GREEN_LEFT_BEACON_TOP_COLOR      = GREEN;
    public static final Color GREEN_LEFT_BEACON_BOTTOM_COLOR   = Color.pink;
    public static final Color GREEN_RIGHT_BEACON_TOP_COLOR     = Color.pink;
    public static final Color GREEN_RIGHT_BEACON_BOTTOM_COLOR  = GREEN;
    public static final Color YELLOW_LEFT_BEACON_TOP_COLOR     = Color.yellow;
    public static final Color YELLOW_LEFT_BEACON_BOTTOM_COLOR  = Color.pink;
    public static final Color YELLOW_RIGHT_BEACON_TOP_COLOR    = Color.pink;
    public static final Color YELLOW_RIGHT_BEACON_BOTTOM_COLOR = Color.yellow;
    public static final Color RED_DOG_COLOR                    = Color.red;
    public static final Color BLUE_DOG_COLOR                   = DARK_BLUE;

    public static final int MAX_FORWARD_SPEED = 8;
    public static final int MAX_TURN_SPEED    = 50;
    public static final int FORWARD_STEP      = 1;
    public static final int TURN_STEP         = 10;

    public static final int K_FORWARD        = KeyEvent.VK_W,
                            K_BACKWARD       = KeyEvent.VK_S,
                            K_LEFT           = KeyEvent.VK_A,
                            K_RIGHT          = KeyEvent.VK_D,
                            K_TURNCCW        = KeyEvent.VK_Q,
                            K_TURNCW         = KeyEvent.VK_E,
                            K_H_UP           = KeyEvent.VK_I,
                            K_H_DOWN         = KeyEvent.VK_K,
                            K_H_UP2          = KeyEvent.VK_U,
                            K_H_DOWN2        = KeyEvent.VK_O,
                            K_H_LEFT         = KeyEvent.VK_J,
                            K_H_RIGHT        = KeyEvent.VK_L,
                            K_BLOCK          = KeyEvent.VK_B,
                            K_HOLD           = KeyEvent.VK_H,
                            K_NORMAL         = KeyEvent.VK_N,
                            K_CHEST_PUSH     = KeyEvent.VK_F1,
                            K_GOALIE_KICK    = KeyEvent.VK_F2,
                            K_LIGHTNING_KICK = KeyEvent.VK_F3,
                            K_SPIN_KICK      = KeyEvent.VK_F4,
                            K_TK1            = KeyEvent.VK_1,
                            K_TK2            = KeyEvent.VK_2,
                            K_TK3            = KeyEvent.VK_3,
                            K_TK4            = KeyEvent.VK_4,
                            K_TK5            = KeyEvent.VK_5,
                            K_TK6            = KeyEvent.VK_6,
                            K_TK7            = KeyEvent.VK_7,
                            K_TK8            = KeyEvent.VK_8,
                            K_TK9            = KeyEvent.VK_9,
                            K_TK0            = KeyEvent.VK_0;

    // keys that are used in motion
    public static final String C_FORWARD        = "f",
                               C_BACKWARD       = "f",
                               C_LEFT           = "l",
                               C_RIGHT          = "l",
                               C_TURNCCW        = "t",
                               C_TURNCW         = "t",
                               C_H_UP           = "v",
                               C_H_DOWN         = "v",
                               C_H_UP2          = "v2",
                               C_H_DOWN2        = "v2",
                               C_H_LEFT         = "h",
                               C_H_RIGHT        = "h",
                               C_BLOCK          = "B",
                               C_HOLD           = "H",
                               C_NORMAL         = "N",
                               C_CHEST_PUSH     = "K",
                               C_GOALIE_KICK    = "K",
                               C_LIGHTNING_KICK = "K",
                               C_SPIN_KICK      = "K",
                               C_TK1            = "T",
                               C_TK2            = "T",
                               C_TK3            = "T",
                               C_TK4            = "T",
                               C_TK5            = "T",
                               C_TK6            = "T",
                               C_TK7            = "T",
                               C_TK8            = "T",
                               C_TK9            = "T",
                               C_TK0            = "T";

    // keys that are used in walktype
    public static final String WALKTYPE_CHANGE   = "wt";
    public static final String WALKPARAM1_CHANGE = "wp1";
    public static final String WALKPARAM2_CHANGE = "wp2";
    public static final String WALKPARAM3_CHANGE = "wp3";
    public static final String[] WALKTYPES = { "Normal", "Canter", "Zoidal", "Offset", "Ellipse" };

    // keys that are not used in motion
    public static final String MODE_TRIGGER        = "mode",
                               IP_TRIGGER          = "ip",
                               WORLDMODEL_TRIGGER  = "w",
                               CPLANE_TRIGGER      = "d",
                               BURST_TRIGGER       = "burst",
                               YUVPLANE_TRIGGER    = "y",
                               CONTROL_TRIGGER     = "c",
                               CONTROL_START_VALUE = "1",
                               CONTROL_END_VALUE   = "0",
                               OPLANE_TRIGGER      = "z";

    public static final int START_VALUE   = 1,
                            STOP_VALUE    = 0,
                            START_TYPE    = 1,
                            STOP_TYPE     = 2,
                            UNKNOWN_TYPE  = 3;


    public static final float FORWARD_F = 4,
                              FORWARD_S = 2,
                              LEFT_F    = 4,
                              LEFT_S    = 2,
                              TURN_F    = 10,
                              TURN_S    = 5,
                              TILT_F    = 1,
                              TILT_S    = 1,
                              TILT2_F   = 1,
                              TILT2_S   = 1,
                              PAN_F     = 1,
                              PAN_S     = 1;

    public static final int[] KEYS = {
                            K_FORWARD,
                            K_BACKWARD,
                            K_LEFT,
                            K_RIGHT,
                            K_TURNCCW,
                            K_TURNCW,
                            K_H_UP,
                            K_H_DOWN,
                            K_H_UP2,
                            K_H_DOWN2,
                            K_H_LEFT,
                            K_H_RIGHT,
                            K_BLOCK,
                            K_HOLD,
                            K_NORMAL,
                            K_CHEST_PUSH,
                            K_GOALIE_KICK,
                            K_LIGHTNING_KICK,
                            K_SPIN_KICK,
                            K_TK1,
                            K_TK2,
                            K_TK3,
                            K_TK4,
                            K_TK5,
                            K_TK6,
                            K_TK7,
                            K_TK8,
                            K_TK9,
                            K_TK0
    };

    public static final int STANCE_KEYS_INDEX = 12;
    public static final int KICK_KEYS_INDEX   = 15;

    public static final String[] COMMANDS = {
                            C_FORWARD,
                            C_BACKWARD,
                            C_LEFT,
                            C_RIGHT,
                            C_TURNCCW,
                            C_TURNCW,
                            C_H_UP,
                            C_H_DOWN,
                            C_H_UP2,
                            C_H_DOWN2,
                            C_H_LEFT,
                            C_H_RIGHT,
                            C_BLOCK,
                            C_HOLD,
                            C_NORMAL,
                            C_CHEST_PUSH,
                            C_GOALIE_KICK,
                            C_LIGHTNING_KICK,
                            C_SPIN_KICK,
                            C_TK1,
                            C_TK2,
                            C_TK3,
                            C_TK4,
                            C_TK5,
                            C_TK6,
                            C_TK7,
                            C_TK8,
                            C_TK9,
                            C_TK0
    };
    public static final float[] SPEEDS = {
                              FORWARD_F,
                              FORWARD_S,
                              LEFT_F,
                              LEFT_S,
                              TURN_F,
                              TURN_S,
                              TILT_F,
                              TILT_S,
                              TILT2_F,
                              TILT2_S,
                              PAN_F,
                              PAN_S
    };

    public static final int[] KICK_VALUES = {
                              1, 1, 1,                           // block, hold and normal
                              1, 2,  3,  4,                      // kicks
                              1, 2, 3, 4, 5, -1, -2, -3, -4, -5  // turn kicks
    };

    public static final String RINFO_DELIMA_S = "<RINFO>",
                               RINFO_DELIMA_E = "</RINFO>";

}
