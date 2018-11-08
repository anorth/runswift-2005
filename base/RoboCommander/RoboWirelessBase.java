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
 * $Id: RoboWirelessBase.java 2090 2003-11-01 16:44:49Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/


public class RoboWirelessBase {

    public static final boolean debugMsg = false;
    public static final boolean worldModelDebug = false;

    public static final int PORT1  = 5005; // Robo commander send
    public static final int PORT2  = 5006; // Robo commander receive
    public static final int PORT3  = 5010; // cplane
    public static final int PORT4  = 5011; // yuv plane
    public static final int PORT5  = 5014;
    public static final int PORT6  = 5015; // debug stream
    public static final int PORT7  = 5102; // oplane

    private static final int NUM_SOCKETS = 7;
    public static boolean socket_humanControl   = true;
    public static boolean socket_roboCommander  = true;
    public static boolean socket_cplaneDisplay  = true;
    public static boolean socket_roboYUVLogger  = false; // only 1 or the other
    public static boolean socket_roboYUVPainter = true;  // of these 2 will be run
    public static boolean socket_grapher        = true;
    public static boolean socket_debugModule    = true;
    public static boolean socket_frameRate      = true;

    private RoboCommander roboCommander = null;
    private CPlaneDisplay cplaneDisplay = null;
    private RoboYUVPainter roboYUVPainter = null;
    private RoboYUVLogger roboYUVLogger = null ;
    private Grapher grapher = null;
    private DebugModule debugModule = null;
    private HumanControl humanControl = null;

    private static boolean logYUV;

    private static final String usage = "default usage command: java RoboWirelessBase\n" +
          "                           runs wirelessBase with all sockets initialised\n" +
          "optional usage command: java RoboWirelessBase (1) (2) (3) (4) (5) (6) (7)\n" +
          "                           runs wirelessBase with the specified sockets enabled/disabled\n" +
          "                           if 0 is specified, socket is disabled\n" +
          "                           if 1 is specified, socket is enabled\n" +
          "                           1 = humanControl\n" +
          "                           2 = roboCommander\n" +
          "                           3 = cplane\n" +
          "                           4 = yuvPlane\n" +
          "                           5 = grapher\n" +
          "                           6 = debug\n" +
          "                           7 = frameRate\n" +
          "                        e.g. java RoboWirelessBase 1 1 0 0 0 0 0\n" +
          "                           runs wirelessBase with only humanControl and roboCommander\n" +
          "                           sockets enabled\n" +
          "                        N.B. If any sockets are disabled, they must also be disabled\n" +
          "                           in client as well";


    public static void main(String[] args) {

        // check command line args for enabling/disabling of sockets
        // 0 = disable, 1 = enable
        if (args.length == 1 && args[0].equals("usage")) {
            System.out.println(usage);
            System.exit(0);
        } else if (args.length == NUM_SOCKETS) {
            for (int i = 0 ; i < args.length ; i++) {
                switch (i) {
                    case 0: if (args[i].equals("0")) {
                                socket_humanControl = false;
                            } else if (args[i].equals("1")) {
                                socket_humanControl = true;
                            }
                            break;
                    case 1: if (args[i].equals("0")) {
                                socket_roboCommander = false;
                            } else if (args[i].equals("1")) {
                                socket_roboCommander = true;
                            }
                            break;
                    case 2: if (args[i].equals("0")) {
                                socket_cplaneDisplay = false;
                            } else if (args[i].equals("1")) {
                                socket_cplaneDisplay = true;
                            }
                            break;
                    case 3: if (args[i].equals("0")) {
                                socket_roboYUVLogger = false;
                                socket_roboYUVPainter = false;
                            } else if (args[i].equals("1")) {
                                socket_roboYUVLogger = false;
                                socket_roboYUVPainter = true;
                            } else if (args[i].equals("2")) {
                                socket_roboYUVLogger = true;
                                socket_roboYUVPainter = false;
                            }

                            break;
                    case 4: if (args[i].equals("0")) {
                                socket_grapher = false;
                            } else if (args[i].equals("1")) {
                                socket_grapher = true;
                            }
                            break;
                    case 5: if (args[i].equals("0")) {
                                socket_debugModule = false;
                            } else if (args[i].equals("1")) {
                                socket_debugModule = true;
                            }
                            break;
                    case 6: if (args[i].equals("0")) {
                                socket_frameRate = false;
                            } else if (args[i].equals("1")) {
                                socket_frameRate = true;
                            }
                            break;
                }
            }
        }

        new RoboWirelessBase();
    }

    RoboWirelessBase() {
        if (socket_humanControl)
            humanControl   = new HumanControl(this, PORT7);
        if (socket_roboCommander)
            roboCommander  = new RoboCommander(this, PORT1,PORT2);
        if (socket_cplaneDisplay)
            cplaneDisplay  = new CPlaneDisplay(this, PORT3);
        if (socket_roboYUVLogger)
   	        roboYUVLogger = new RoboYUVLogger();
        if (socket_roboYUVPainter)
            roboYUVPainter = new RoboYUVPainter(PORT4, roboCommander);
        if (socket_grapher)
            grapher        = new Grapher(PORT5, roboCommander);
        if (socket_debugModule)
            debugModule    = new DebugModule(PORT6, roboCommander, DebugModule.MODE_POSITION);
    }

    public RoboCommander getRoboCommander() {
        return roboCommander;
    }

    public HumanControl getHumanControl() {
        return humanControl;
    }

    public CPlaneDisplay getCPlaneDisplay() {
        return cplaneDisplay;
    }
}
