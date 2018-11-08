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
 * UNSW 2003 Robocup (Nicodemus Sutanto)
 *
 * Last modification background information
 * $Id: Configure.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * sets configuration file of the robot
 *
**/

import java.io.*;
import java.util.*;

public class Configure {

    private static final boolean debugMsg = true;

    public static final String CONF_FILE = ".." + File.separator + "conf.cfg";

    public static final char DOT = '.';
    
    public static final String DEFAULT_NUM_ROBOT       = "1";
    public static final int    DEFAULT_ROBOT_ID        = 100;
    public static final String DEFAULT_NETWORK_IP_HEAD = "192" + DOT + "168" + DOT + "0";
    public static final String DEFAULT_SEND_PORT       = "54321";

    // markers tell where info should be replaced
    public static final String HEADER_MARKER = "# Configuration File   PlayerNumber            IP              Port";

    public static BufferedReader input = new BufferedReader(new InputStreamReader(System.in));

    public static void main(String[] args) {
        new Configure(args);
    }

    public Configure(String[] args) {
        if (args.length>0) {
            String[] ip = processRobotIPs(DEFAULT_NETWORK_IP_HEAD, args);
            writePortFile(ip, DEFAULT_SEND_PORT);
        } else {
            writePortFile();
        }
    }
    
    private void writePortFile() {
        int numRobots;
        int[] robotID;

        String sendPort;
        String networkIPHead;
        String[] ip;

        try {
            numRobots = getNumberOfRobots();

            robotID = getRobotIDs(numRobots);
            
            networkIPHead = getNetworkIPHead();

            ip = processRobotIPs(networkIPHead, robotID);

            // get port
            sendPort = getInfo("Enter port", "Error: Illegal port", DEFAULT_SEND_PORT);

            writePortFile(ip, sendPort);
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }

    private void writePortFile(String[] ip, String sendPort) {
        try {
            PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(CONF_FILE)));
            out.println(HEADER_MARKER);
            out.flush();
            out.println(ip.length);
            out.flush();
            for (int i=0; i<ip.length; i++) {
                out.println(ip[i]);
                out.flush();
            }
            out.println(sendPort);
            out.flush();
            out.close();
        } catch (IOException e) {
            System.out.println(e.getMessage());
        }
    }

    // gets a line of info from user
    protected String getInfo(String msg, String errMsg, String defaultValue) throws IOException {
        String info;

        System.out.print(msg + " [" + defaultValue + "]: ");
        
        info = input.readLine();
        if (info == null) {
            throw new IOException(errMsg);
        }

        if (info.trim().equals("")) {
            info = defaultValue;
        }
        
        return info;
    }

    // get number of robots
    private int getNumberOfRobots() {
        while(true) {
            try {
                return Integer.parseInt(getInfo("Enter number of robots", 
                                                "Error: Illegal Number", DEFAULT_NUM_ROBOT));
            } catch (NumberFormatException e) {
                System.out.println("Illegal number");
            } catch (IOException e) {
                System.err.println(e.getMessage());
            }
        }
    }
    
    // get ID of robots, ie, the last decimal of robots IP
    private int[] getRobotIDs(int numRobots) {
        // get ID of robots
        int[] robotID = new int[numRobots];
        String input;
        for (int i=0; i<numRobots; i++) {
            try {
                if (i>0) {
                    input = getInfo("Enter robot " + (i+1) + "'s ip postfix",
                                    "Error: Illegal Number", (robotID[i-1]+1)+"");
                } else {
                    input = getInfo("Enter robot " + (i+1) + "'s ip postfix",
                                    "Error: Illegal Number", (DEFAULT_ROBOT_ID+i)+"");
                }
                robotID[i] = Integer.parseInt(input);
                for (int j=(i-1); j>=0; j--) {
                    if (robotID[i]==robotID[j]) {
                        System.out.println("Error: ID already used in Robot " + (j+1));
                        i--;
                        break;
                    }
                }
            } catch (IOException e) {
                System.err.println(e.getMessage());
            }
        }
        return robotID;
    }
    
    // get network IP Head
    private String getNetworkIPHead() {
        String networkIPHead;
        int dotCount;
        while(true) {
            try {
                dotCount = 0;
                networkIPHead = getInfo("Enter Network ip prefix", "Error: Illegal IP", DEFAULT_NETWORK_IP_HEAD);
                for (int i=0; i<networkIPHead.length(); i++) {
                    if (networkIPHead.charAt(i)==DOT) {
                        dotCount++;
                    }
                }
                if (dotCount==2) {
		    break;
		}
		System.out.println("Error: invalid IP head entered");
            } catch (IOException e) {
                System.err.println(e.getMessage());
            }
        }
        return networkIPHead;
    }
    
    // convert each int IDs to string IDs and process again
    private String[] processRobotIPs(String networkIPHead, int[] intID) {
        String[] stringID = new String[intID.length];
        for (int i=0; i<intID.length; i++) {
            stringID[i] = "" + intID[i];
        }
        return processRobotIPs(networkIPHead, stringID);
    }
    
    // concatenate the IP of robots given the networkIPHead and robotID
    private String[] processRobotIPs(String networkIPHead, String[] robotID) {
        String[] ip = new String[robotID.length];
        for (int i=0; i<robotID.length; i++) {
            ip[i] = networkIPHead + DOT + robotID[i];
        }
        return ip;
    }
    
}

