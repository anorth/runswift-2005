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
 * @author UNSW 2002 Robocup (Tim Tam)
 *
 * Last modification background information
 * $Id: SendBFL.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Sends a bfl image on disk to the input port.
 * Format is YUV coorindates (WIDTH*HEIGHT*6 bytes) then the Cplane (WIDTH*HEIGHT bytes)
 *
**/

import java.net.*;
import java.io.*;


class SendBFL {

    public static final int HEIGHT = RoboConstant.CPLANE_HEIGHT;
    public static final int WIDTH = RoboConstant.CPLANE_WIDTH;

    public static void main(String[] args) throws Exception {
        if (args.length != 2) {
            System.err.println("Usage: java SendBFL <bfl image> <port>");
            System.exit(-1);
        }

        Socket s = new Socket("localhost", Integer.parseInt(args[1]));
        DataOutputStream out = new DataOutputStream(s.getOutputStream());

        // Open the file and store it in a byte array.
        File f = new File(args[0]);
        FileInputStream fis = new FileInputStream(f);
        int size = (int)f.length();
        byte[] buf = new byte[size];

        int index = 0;
        int read = 0;
        while(index != size-HEIGHT*WIDTH) {
            read = fis.read(buf, index, WIDTH*6);
            index += read;
            fis.skip(WIDTH);
        }

        System.out.println("hi");

        f = new File(args[0]);
        fis = new FileInputStream(f);
        while(index != size) {
            fis.skip(WIDTH*6);
            read = fis.read(buf, index, WIDTH);
            index += read;
        }
        System.out.println(buf[buf.length-1]);

        // Send the bfl image to the
        out.write(buf, 0, buf.length);

        System.out.println("Bytes sent!");

    }
}

