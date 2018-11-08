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
 * @author UNSW 2002 Robocup (David Wang)
 *
 * Last modification background information
 * $Id: RoboYUV.java 1953 2003-08-21 03:51:39Z eileenm $
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


public class RoboYUV extends JFrame {
    public static final int WIDTH = RoboConstant.CPLANE_WIDTH;
    public static final int HEIGHT = RoboConstant.CPLANE_HEIGHT;

    public static final int YUVSCALE = 6;
    public static final int SIZE = WIDTH * HEIGHT * YUVSCALE;
    public static int PORT = 5011;

    int scale = 3;

    byte[] yuvInfo = new byte[SIZE]; // stores the yuv bytes received
    byte[] dummyCPlane = new byte[WIDTH];

    int[][] rInfo = new int[HEIGHT][WIDTH];
    int[][] gInfo = new int[HEIGHT][WIDTH];
    int[][] bInfo = new int[HEIGHT][WIDTH];



    private JButton bSave             = new JButton("Save");
    private JLabel     lFileName      = new JLabel("File Name");
    private JTextField fFileName      = new JTextField(20);
    private JLabel     lScale         = new JLabel("View Scale");
    private JTextField fScale         = new JTextField(scale+"",4);
    private YUVDisplay  pDrawingBoard = new YUVDisplay();


    public RoboYUV() {
        Container content = getContentPane();
        content.setLayout(new BorderLayout());

        pDrawingBoard.setSize(500,500);
        content.add(pDrawingBoard,BorderLayout.CENTER);

        JPanel pControl = new JPanel();
        pControl.setLayout(new FlowLayout());
        pControl.add(lScale);
        pControl.add(fScale);
        pControl.add(lFileName);
        pControl.add(fFileName);
        pControl.add(bSave);
        content.add(pControl,BorderLayout.SOUTH);

        ////////////
        // add listener
        ////////////
        fScale.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    try {
                        int v = Integer.parseInt(fScale.getText());
                        scale = v;
                        pDrawingBoard.repaint();
                    } catch (NumberFormatException ex) {}
                }
        });

        ActionListener aSave = new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    try {
                        save();
                    } catch (NumberFormatException ex) {}
                }
        };
        fFileName.addActionListener(aSave);
        bSave.addActionListener(aSave);


        setSize(550,500);
        //setVisible(true);
        (new Thread(pDrawingBoard)).start();
    }

    public static void main(String[] args) {
        RoboYUV rc = new RoboYUV();
    }

    // saves the image in a bfl file
    private void save() {
        String fileName = fFileName.getText();
        // no file name
        if (fileName == null || fileName.equals("")) {
            return;
        }
        try {
            File file = new File(fileName);
            FileOutputStream out = new FileOutputStream(file);
            for (int i = 0; i < SIZE; i += WIDTH*3) {
                out.write(yuvInfo,i,WIDTH*3);
                out.write(dummyCPlane);
            }
            out.close();
            System.out.println("RoboYUV.save: file saved <" + fileName + ">");
        } catch (IOException e) {
            System.err.println("RoboYUV.save: problem in saving file <" + fileName + ">");
        }
    }

    public static int [] YUV2RGB(int y,int u, int v){
            int [] rgb = new int [3];
            //change range of u and v for rgb conversion
            u -= 128;
            v -= 128;

            rgb[0] = (int) ( (1.164 * y) + (1.596 * u));
            rgb[1] = (int) ( (1.164 * y) - (0.813 * u) - (0.391 * v));
            rgb[2] = (int) ( (1.164 * y) + (1.596 * v));

            if (rgb[0] < 0)
                rgb[0] = 0;
            else if (rgb[0] > 255)
                rgb[0] = 255;
            if (rgb[1] < 0)
                rgb[1] = 0;
            else if (rgb[1] > 255)
                rgb[1] = 255;
            if (rgb[2] < 0)
                rgb[2] = 0;
            else if (rgb[2] > 255)
                rgb[2] = 255;

            return rgb;
        }



    class YUVDisplay extends JPanel implements Runnable {
        YUVDisplay() {
            super(true);
        }

        public void run() {
            InputStream in;
            int read = 0;
            try {
                ServerSocket ss = new ServerSocket(PORT);
                Socket s = ss.accept();
                in = s.getInputStream();
                ss.close();
                while(true) {
                    // removes the problem where tcp sends a little bit off
                    // info at a time
                    while (read != SIZE) {
                        int newRead = in.read(yuvInfo,read,SIZE-read);
                        if (newRead <= 0) {
                            return;
                        }
                        read += newRead;
                    }
                    RoboYUV.this.setVisible(true);

                    System.out.println("read = " + read);

                    System.out.println("received.... " + (int) yuvInfo[0] + (int) yuvInfo[1] +(int) yuvInfo[2]);
                    // convert colour
                    int y, u, v;
                    for (int row = 0; row < HEIGHT; row++) {
                        for (int col = 0; col < WIDTH; col++) {
                            //TODO
                            y = yuvInfo[row*WIDTH*6 + col] & 0xff;
                            u = yuvInfo[row*WIDTH*6 + WIDTH + col] & 0xff;
                            v = yuvInfo[row*WIDTH*6 + WIDTH*2 + col] & 0xff;
                            if ((col == 0 & row == 0) || (col == WIDTH & row == HEIGHT)) {
                                System.out.println("yuv = " + y + " " + u + " " + v);
                                System.out.println("byte yuv = " + Byte.toString(yuvInfo[row*WIDTH*6 + col]) + " " +
                                                   Byte.toString(yuvInfo[row*WIDTH*6 + WIDTH + col]) + " " +
                                                   Byte.toString(yuvInfo[row*WIDTH*6 + WIDTH*2 + col]));
                            }



                            //System.out.println("yuv = " + y + " " + u + " " + v);
                            int [] RGB  = YUV2RGB(y,u,v);
                            rInfo[row][col] = RGB[0];
                            gInfo[row][col] = RGB[1];
                            bInfo[row][col] = RGB[2];


                        }
                    }
                    repaint();
                    read = 0;
                }
            } catch(Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }



        public void paint(Graphics g) {
            g.setColor(Color.white);
            g.fillRect(0,0,getWidth(),getHeight());
            for (int row = 0; row < HEIGHT; row++) {
                for (int col = 0; col < WIDTH; col++) {
                    //System.out.prrowntln(rROWnfo[row][j] + " " + gROWnfo[row][j] + " " + bROWnfo[i][j]);
                    g.setColor( new Color(rInfo[row][col],gInfo[row][col],bInfo[row][col]) );
                    g.fillRect(col*scale,row*scale,scale,scale);
                }
            }
            g.setColor(Color.black);
            //System.out.println("done");
        }

    } // end class YUVDisplay
}
