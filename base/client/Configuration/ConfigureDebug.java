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
 * @author UNSW 2002 Robocup (David Zheng Wang)
 *
 * Last modification background information
 * $Id: ConfigureDebug.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * sets the network information of the robot
 *
**/
 
import java.io.*;
import java.util.*;

public class ConfigureDebug extends ConfigureBase {
    public static final String
        PORT_FILE    = "../snap/port.cfg",
        CONNECT_FILE = "../snap/connect.cfg",
        STUB_FILE    = "../MsgPasser/stub.cfg",
        OBJ_FILE     = "../snap/object.cfg",
        MSG_PASSER_FILE = "../MsgPasser/MsgPasser.h",

        DEFAULT_NUM_ROBOT      = "1",
        DEFAULT_ROBOT_IP       = "10/1/",
	    DEFAULT_NETWORK_IP     = "192.168.0.0",
        DEFAULT_SEND_PORT      = "1200",
        DEFAULT_RECEIVE_PORT   = "1100";

    /////////
    // markers tell where info should be replaced
    ////////
    public static final String
        PORT_MARKER = "# OPEN-R Service Name",
        START_MARKER = "# End Normal Services",
        CONNECT_MARKER = "[ERS-210]",
        STUB_MARKER = "ObjectName : MsgPasser",

        PORT_SEND =    "TCPGateway.LISTENER/1/.CHAR.O                  /2/    /3/",
        PORT_RECEIVE = "TCPGateway.SPEAKER/1/.CHAR.S                   /2/    /3/",

        END_MARKER = "# End Debug Services",

        CONNECT_GATEWAY = "TCPGateway.SPEAKER/1/.CHAR.S MsgPasser.BOT/1/.CHAR.O",
       CONNECT_BOT     = "MsgPasser.BOT/1/.CHAR.S	TCPGateway.LISTENER/2/.CHAR.O",

        STUB_SUB_NUM = "NumOfOSubject   : /1/",
        STUB_OBS_NUM = "NumOfOObserver  : /1/",

        STUB_SERVICE = "Service : \"MsgPasser.BOT/1/.CHAR.S\", null, null\n" + "Service : \"MsgPasser.BOT/1/.CHAR.O\", null, Update()",
//        STUB_SERVICE = "Service : \"MsgPasser.BOT/1/.CHAR.O\", null, Update()",
        DEBUG_STUB   = "Service : \"MsgPasser.RECEIVER.CHAR.S\", null, null\n" +
                       "Service : \"MsgPasser.RECEIVER.CHAR.O\", null, Command()\n",
        DEBUG_OBJ_MARKER = "$OPENR_LINUX/module/System/tcpGateway",
        NO_DEBUG_OBJ     = "../MsgPasser/msgPasser",
        DEBUG_OBJ        = "../MsgReceiver/msgReceiver\n" + NO_DEBUG_OBJ,
        DEBUG_CONNECT_GATWAY = "MsgReceiver.SENDER.CHAR.S MsgPasser.RECEIVER.CHAR.O",
        DEBUG_CONNECT_BOT    = "MsgPasser.RECEIVER.CHAR.S TCPGateway.LISTENER/1/.CHAR.O",
        DEBUG_PASSER_MARKER = "#define DEBUG";

    public ConfigureDebug() {}

    public void run(boolean debug, int numRobots, String[] ip, int sendPort, int receivePort) {
        try {
	    // read the port map file
            Vector pFile = readFile(PORT_FILE);
	    // read connect file
	    Vector cFile = readFile(CONNECT_FILE);
	    // read stub file
            Vector sFile = readFile(STUB_FILE);
	    // read obj file
            Vector oFile = readFile(OBJ_FILE);
	    // read msg passer file
            Vector mFile = readFile(MSG_PASSER_FILE);


	    //////////
            // write port file
            /////////
	    BufferedWriter out = writeFile(pFile,PORT_FILE,START_MARKER);
	    String line;
	    for (int i = 0; i < numRobots; i++) {
		line = replace(PORT_SEND,1,i + 1);
		line = replace(line,2,sendPort);
		line = replace (line,3,ip[i]);
		out.write(line + "\n");
		line = replace(PORT_RECEIVE,1,i + 1);
		line = replace(line,2,receivePort);
		line = replace (line,3,ip[i]);
		out.write(line + "\n");
	    }
        out.write(END_MARKER + "\n");
	    out.close();


	    //////////
            // write connect file
            /////////
	    out = writeFile(cFile,CONNECT_FILE,START_MARKER);
	    for (int i = 0; i < numRobots; i++) {
		line = replace(CONNECT_GATEWAY,1,i + 1);
		out.write(line + "\n");
		for (int j = 0; j < numRobots; j++) {
		    if (i != j) {
			line = replace(CONNECT_BOT,1,i + 1);
			line = replace(line,2,j + 1);
			out.write(line + "\n");
		    }
		}
	    }
            if (debug) {
                out.write(DEBUG_CONNECT_GATWAY + "\n");
                for (int j = 0; j < numRobots; j++) {
                    line = replace(DEBUG_CONNECT_BOT,1,j + 1);
                    out.write(line + "\n");
                }
            }
        out.write(END_MARKER + "\n");
	    out.close();

	    //////////
            // write stub file
            /////////
            int numObs = numRobots;
            if (debug) {
                numObs++; // this is number of objects
            }
	    out = writeFile(sFile,STUB_FILE,STUB_MARKER);
	    line = replace(STUB_SUB_NUM,1,numObs);
	    out.write(line + "\n");
	    line = replace(STUB_OBS_NUM,1,numObs);
	    out.write(line + "\n");
	    for (int i = 0; i < numRobots; i++) {
		line = replace(STUB_SERVICE,1,i + 1);
		out.write(line + "\n");
	}
            if (debug) {
                out.write(DEBUG_STUB + "\n");
            }
	    out.close();

            //////////
            // write obj file
            //////////
	    out = writeFile(oFile,OBJ_FILE,DEBUG_OBJ_MARKER);
            if (debug) {
                out.write(DEBUG_OBJ + "\n");
            } else {
                out.write(NO_DEBUG_OBJ + "\n");
            }
            out.close();

            //////////
            // write msg passer file
            //////////
            for (int i = 0; i < mFile.size(); i++) {
                line = (String) mFile.elementAt(i);
                if (line.indexOf(DEBUG_PASSER_MARKER) >= 0) {
                    if (debug) {
                        mFile.setElementAt(DEBUG_PASSER_MARKER,i);
                    } else {
                        mFile.setElementAt("//" + DEBUG_PASSER_MARKER,i);
                    }
                }
            }
            writeFile(mFile,MSG_PASSER_FILE);

        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }

}

