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


package RoboComm;

import java.io.*;
import java.net.*;

import RC.*;
import RoboShare.*;

/**
 *
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
*  CHANGE LOG:
*  + version 1.1:
*        - Log YUV stream to YUVWorkingFolder, create unique name for each yuv
*  plane (in the form of yyyy_mm_dd_$$.bfl )
 * + version 1.2:
 *       - Refactor: make this generic, by removing dependency from RDRApplication
 *         (sothat either RDRApplication or RobotController can listen to YUV frames)
 */

import java.util.*;

public class YUVStreamClient extends YUVReceiver{
    public static final int WIDTH = RobotDefinition.CPLANE_WIDTH;
    public static final int HEIGHT = RobotDefinition.CPLANE_HEIGHT;


    public static int PORT = 5011;

    int scale = 3;

    int YUVStreamSize = StreamingConstant.YUVPLANE_SIZE / 2;
    byte[] yuvStream = new byte[YUVStreamSize];
    byte[] yuvInfo = new byte[StreamingConstant.YUVPLANE_SIZE]; // stores the yuv bytes received
    byte[] cPlaneInfo = new byte[StreamingConstant.CPLANE_SIZE];
    byte[] dummyCPlane = new byte[RobotDefinition.CPLANE_WIDTH];

    private StreamReceiver receiver = new StreamReceiver();

    private boolean bIsRunning = false;

    public YUVStreamClient() {
    }


    public void start() {
        bIsRunning = true;
        if (receiver.isRunning) {
            receiver.flagStop();
        }
        else {
            (new Thread(receiver)).start();
        }
    }

    public void stop() {
        bIsRunning = false;
        receiver.flagStop();
    }

    public boolean isRunning(){
        return bIsRunning;
    }


    private void notifyListener() {
        BFL streamedBFL = new BFL(true);
        streamedBFL.readStream(yuvInfo,cPlaneInfo);
        try {
            streamedBFL.saveFile(RCUtils.generateUniqueName());
            System.out.println("RoboYUVLogger: file saved <" +
                               streamedBFL.fileName + ">");
        }
        catch (Exception ex) {
            System.err.println("YUVListener : Error saving bfl file ");
        }
        notifyYUVListeners(streamedBFL);
    }



    // reads data until data array full
    private int read(InputStream in, byte[] data) throws IOException {
        int read = 0;
        while (read != data.length) {
            int newRead = in.read(data, read, data.length - read);
            if (newRead <= 0) {
                if (newRead < 0) {
                    return -1;
                }
                else {
                    return read;
                }
            }
            read += newRead;
        }
        System.out.println("YUVLogger:received size: " + read);
        return read;
    }

    class StreamReceiver
        implements Runnable {
        private boolean isRunning = false;
        ServerSocket ss = null;

        public void flagStop() {
            try {
                if (ss != null) {
                    ss.close();
                    isRunning = false;

                }
            }
            catch (IOException ex) {
                System.out.println("ServerSocket exception.");
            }
        }

        private void unshortenedYUV(byte [] yuvStream, byte [] yuvInfo){
            int LONG_ROWLENGTH = 6 * WIDTH;
            int SHORT_ROWLENGTH = 3 * WIDTH;
            for (int i=HEIGHT-1;i>=0;i--){
                for (int j=0;j<SHORT_ROWLENGTH;j++){
                    yuvInfo[ i * LONG_ROWLENGTH + j] = yuvStream[ i * SHORT_ROWLENGTH + j];

                }
            }
        }

        public void run() {
            isRunning = true;
            byte[] type = new byte[1];
            InputStream in;
            try {
                ss = new ServerSocket(PORT);
                System.out.println("YUVListener (receive," + PORT + "): ready");
                while (true) {
                    Socket s = ss.accept();
                    System.out.println("YUVListener : New connection.");
                    in = s.getInputStream();

                    while (true) {
                        // removes the problem where tcp sends a little bit off
                        // info at a time
                        int nread;
                        if ( (nread = read(in, type)) == -1) {
                            break;
                        }
                        if (type[0] == 10) {
                            int received;
                            if ( (received = read(in, cPlaneInfo)) !=
                                cPlaneInfo.length) {
                                System.out.println("cPlaneInfo cut short: " +
                                    received);
                                break;
                            }
                        }
                        else if (type[0] == 11) {
                            // yuv
                            int t = read(in, yuvStream);
                            if (t != YUVStreamSize) {
                                System.out.println("yuvInfo cut short " + t + " != " + YUVStreamSize);
                                break;
                            }
                            else {
                                //HACKED here, make the shortened one longer
                                unshortenedYUV(yuvStream,yuvInfo);
                                notifyListener();
                            }
                        }
                        else {
                            System.out.println("Unexpected byte:" + type[0] +
                                               " nread = " + nread);
                            break;
                        }
                    }
                }
            }
            catch (SocketException e){
                e.printStackTrace();
                System.out.println("YUVListener : Connection closed.");
            }
            catch (Exception e) {
                e.printStackTrace();
            }
            System.out.println("YUVListener stopped.");

        }
    } // end class YUVDisplay
}
