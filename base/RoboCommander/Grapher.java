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
 * UNSW 2002 Robocup (Andres Olave)
 *
 * Last modification background information
 * $Id: Grapher.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.*;

public class Grapher extends JFrame {
    
    public static final String RINFO_DELIMA_S = "<RINFO>",
                               RINFO_DELIMA_E = "</RINFO>",
                               STRING_DELIMA_S= "<String>",
                               STRING_DELIMA_E= "</String>";
  
    public static int PORT = 5014;
    public static int PORT1 = 5005;
    //    public static int NUM_FIELDS = 14;
    GraphDisplay d;    
    private boolean acceptMore;

    OutputStream out;
    RoboCommander rc;
 
    public Grapher(int port, RoboCommander rc) {
        super("Grapher");
        try {
            //  jbInit();
            //  show();
            this.rc=rc;
            (new Acceptor(port)).start();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    class SimServer extends Thread {

        SimServer() {
            d=new GraphDisplay("Distance Measures", rc);
        }

        public void run() {
            while(acceptMore) {
                d.processData();
                try {
                    sleep(100);
                } catch(InterruptedException ie) {}
            }
            closeDisplays();
        }

        private void closeDisplays() {
            d.dispose();
            dispose();
            System.exit(0);
        }
    }    

    
    class Acceptor extends Thread {
        ServerSocket serverSock;
        int port;
        public Acceptor(int port) {
            this.port=port;
        }
        public void run() {

            try {
                // create server socket
                serverSock = new ServerSocket(port);
                System.out.println("Grapher (receive, " + port + "): ready");
                Socket sock = serverSock.accept();
                (new Reader(sock)).start();
            } catch (IOException e) {
                System.err.println("RoboCommander.Acceptor: socket error ");
                e.printStackTrace();
            }
        }
    } // class Acceptor
 
    class Reader extends Thread {
        Socket sock;
        BufferedReader in;
        double[] data;
        int[] idata;
        String[] sdata;
        int     loc; // current loc in data array
        boolean inData=false;
        boolean inString=false;
        
        Reader(Socket s) {
            sock = s;
            //        System.err.print("opening graph ");
            //            displays = new Hashtable();
            d=new GraphDisplay("Distance Measures",rc);
            //System.err.println("finished");
            //        d.show();
        }

        public void run() {
            try {
                data   = new double[100]; // temporary for now
                idata  = new int[100];
                sdata  = new String[100];
                loc    = 0;
                inData = false;
                boolean target = false;
                boolean firstSend = true;
//        d.show();
//        System.err.print("Opening Socket ... ");

                in = new BufferedReader(new InputStreamReader(sock.getInputStream()));
                
//        System.err.println("Complete");

                for (String line = in.readLine();line != null;line = in.readLine()) {
//            System.err.println(line);
//System.out.println(line);
                    if(line.equals(STRING_DELIMA_S) && !inString && !inData) {
                        inString=true;
                    } else if (line.equals(STRING_DELIMA_E) && inString && !inData) {
                        inString=false;
                        if (firstSend) {
                            d.reset();
                            d.setLabels(sdata,loc);
                            firstSend=false;
                        }
                        loc=0;
                    } else if (inString && firstSend) {
                        sdata[loc++]=line;
                    } else if (!inData && line.equals(RINFO_DELIMA_S)) { // start of data
                        //System.out.println("Start of info");
                        inData = true;
                    } else if (inData && line.equals(RINFO_DELIMA_E)) { // end of data
                        //System.out.println("end of info");
                        inData = false;
                        if (!target) {
                            processData(data,loc);
                        } else {
                            processTarget(idata,loc);
                        }
                        loc = 0;
                        target=false;
                    }  else if (inData && line.equals("target")) {
                        target=true;
                    } else if (inData) { // robot data
                        try {
                            if(target) {
                                idata[loc++] = Integer.parseInt(line);
                            } else {
                                data[loc++] = Double.parseDouble(line);
                            }
                        } catch (NumberFormatException e) {
                            System.err.println("GraphDisplay.Reader.run: failed for parse info: " + line);
                        }
                    }
                }

                // ignore the rest
                System.out.println("Connection Closed");
                in.close();
                sock.close();
            } catch (IOException e) {
                System.err.println("GraphDisplay.Reader.run: IOException");
                e.printStackTrace();
            }
            
            closeDisplays();
        }
        
        public void processTarget(int[] data, int size) {
            d.processTarget(data,size);
        }
        
        public void processData(double[] data, int size) {
            d.processData(data,0,size);
        }
        
        private void closeDisplays() {
            d.dispose();
            dispose();
            System.exit(0);
        }
    } // end class Reader

}
