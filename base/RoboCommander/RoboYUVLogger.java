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
 * $Id:
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


public class RoboYUVLogger {
    public static final int WIDTH = RoboConstant.CPLANE_WIDTH;
    public static final int HEIGHT = RoboConstant.CPLANE_HEIGHT;

    public static final int SIZE = RoboConstant.YUVPLANE_SIZE;

    public static int PORT = 5011;

    int scale = 3;
    int counter = 0;

    byte[] yuvInfo = new byte[RoboConstant.YUVPLANE_SIZE]; // stores the yuv bytes received
    byte[] cPlaneInfo = new byte[RoboConstant.CPLANE_SIZE];
    byte[] dummyCPlane = new byte[RoboConstant.CPLANE_WIDTH];

    private YUVReceiver receiver = new YUVReceiver();

    public RoboYUVLogger() {
        (new Thread(receiver)).start();

    }

    public static void main(String[] args) {
        RoboYUVLogger logger = new RoboYUVLogger();
    }

    // saves the image in a bfl file
    private void save() {
        String fileName = counter + ".bfl";
        counter++;
        try {
            File file = new File(fileName);
            FileOutputStream out = new FileOutputStream(file);
            for (int i = 0; i < SIZE; i += WIDTH*6) {
                out.write(yuvInfo,i,WIDTH*6);
                out.write(dummyCPlane);
            }
            out.close();
            System.out.println("RoboYUVLogger: file saved <" + fileName + ">");
        } catch (IOException e) {
            System.err.println("RoboYUVLogger: problem in saving file <" + fileName + ">");
        }
    }

    // reads data until data array full
    private int read(InputStream in, byte[] data) throws IOException {
        int read = 0;
        while (read != data.length) {
            int newRead = in.read(data, read, data.length - read);
            if (newRead <= 0)
                return read;
            read += newRead;
        }
        System.out.println("YUVLogger:received size: " + read);
        return read;
    }


    class YUVReceiver implements Runnable {

        public void run() {
            byte[] type = new byte[1];
            InputStream in;
            try {
                ServerSocket ss = new ServerSocket(PORT);
                Socket s = ss.accept();
                in = s.getInputStream();
                ss.close();
                while (true) {
                    // removes the problem where tcp sends a little bit off
                    // info at a time
                    read(in, type);
                    if (type[0] == 10) {
                        int received;
                        if ((received=read(in,cPlaneInfo)) != cPlaneInfo.length) {
                            System.out.println("cPlaneInfo cut short: " + received);
                            return;
                        }
                    } else if (type[0] == 11)  {
                        // yuv
                        if (read(in,yuvInfo) != yuvInfo.length) {
                            System.out.println("yuvInfo cut short");
                            return;
                        } else {
                            save();
                        }
                    }
                }
            } catch(Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }
    } // end class YUVDisplay
}
