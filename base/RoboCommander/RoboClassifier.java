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
 * $Id: RoboClassifier.java 1953 2003-08-21 03:51:39Z eileenm $
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


public class RoboClassifier extends JFrame {

    public static final int PIC_WIDTH = RoboConstant.CPLANE_WIDTH;
    public static final int PIC_HEIGHT = RoboConstant.CPLANE_HEIGHT;
    public static final int SIZE = PIC_WIDTH*PIC_HEIGHT*3;
    public static int PORT = 5012;

    int scale = 3;

    byte[] yuvInfo = new byte[SIZE]; // stores the yuv bytes received
    byte[] cPlane = new byte[PIC_WIDTH*PIC_HEIGHT];

    int[][] rInfo = new int[PIC_HEIGHT][PIC_WIDTH];
    int[][] gInfo = new int[PIC_HEIGHT][PIC_WIDTH];
    int[][] bInfo = new int[PIC_HEIGHT][PIC_WIDTH];
    Choice colours = new Choice();


/*    private JButton bSave = new JButton("Save");
    private JLabel     lFileName = new JLabel("File Name");
    private JTextField fFileName = new JTextField(20);*/
    private JLabel     lScale = new JLabel("View Scale");
    private JTextField fScale = new JTextField(scale+"",4);
    private ClassifyDisplay  pDrawingBoard = new ClassifyDisplay();


    public RoboClassifier() {

        Container content = getContentPane();
        content.setLayout(new BorderLayout());

        pDrawingBoard.setSize(500,500);
        content.add(pDrawingBoard,BorderLayout.CENTER);

        JPanel pControl = new JPanel();
        pControl.setLayout(new FlowLayout());
        pControl.add(lScale);
        pControl.add(fScale);

        colours.add("0");
        colours.add("1");
        colours.add("2");
        colours.add("3");
        colours.add("4");
        colours.add("5");
        colours.add("6");
        colours.add("7");
        colours.add("8");
        pControl.add(colours);
/*        pControl.add(lFileName);
        pControl.add(fFileName);
        pControl.add(bSave);*/
        content.add(pControl,BorderLayout.SOUTH);

        ////////////
        // add listeners
        ////////////
        fScale.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    try {
                        scale = Integer.parseInt(fScale.getText());
                        pDrawingBoard.repaint();
                    } catch (NumberFormatException ex) {}
                }});
        colours.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                       try {
                        pDrawingBoard.currentColour = Integer.parseInt(colours.getSelectedItem());
                        pDrawingBoard.repaint();
                    } catch (NumberFormatException ex) {}
                }});


/*        ActionListener aSave = new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    try {
                        save();
                    } catch (NumberFormatException ex) {}
                }};
        fFileName.addActionListener(aSave);
        bSave.addActionListener(aSave);*/




        setSize(550,500);
        //setVisible(true);
        (new Thread(pDrawingBoard)).start();
    }

    public static void main(String[] args) {
        RoboClassifier rc = new RoboClassifier();
    }

    class ClassifyDisplay extends JPanel implements Runnable {

        public final Color[] classifyColours =  {
                new Color(255,127,0),
                new Color(127,127,255),
                new Color(0,127,0),
                Color.yellow,
                new Color(255,127,127),
                Color.red,
                new Color(0,0,127),
                new Color(127,255,127)
        };
        public static final int ORANGE = 0;
        public static final int BLUE = 1;
        public static final int GREEN = 2;
        public static final int YELLOW = 3;
        public static final int PINK = 4;
        public static final int BLUE_DOG = 5;
        public static final int RED_DOG = 6;
        public static final int GREEN_FIELD = 7;
        public static final int BACKGROUND = 8;

        int currentColour = PINK;

        ClassifyDisplay() {
            super(true);
        }

        public void run() {
            InputStream in;
            int read = 0;
            try {
                System.out.println("waiting");
                ServerSocket ss = new ServerSocket(PORT);
                Socket s = ss.accept();
                System.out.println("RoboClassifier: Connection Established");
                in = s.getInputStream();
                ss.close();
                while(true) {
                    // removes the problem where tcp sends a little bit off
                    // info at a time
                    while (read != SIZE) {
                        int newRead = in.read(yuvInfo,read,SIZE-read);
                        if (newRead <= 0)
                            return;
                        read += newRead;
                    }
                    while (read != SIZE+PIC_WIDTH*PIC_HEIGHT) {
                        int newRead = in.read(cPlane,read-SIZE,SIZE+PIC_HEIGHT*PIC_WIDTH-read);
                        if (newRead <= 0)
                            return;
                        read += newRead;
                    }

                    RoboClassifier.this.setVisible(true);

                    // convert colour from YUV in RGB
                    int y, u, v;
                    for (int row = 0; row < PIC_HEIGHT; row++) {
                        for (int col = 0; col < PIC_WIDTH; col++) {
                            y = yuvInfo[row*PIC_WIDTH*6 + col] & 0xff;
                            u = yuvInfo[row*PIC_WIDTH*6 + PIC_WIDTH + col] & 0xff;
                            v = yuvInfo[row*PIC_WIDTH*6 + PIC_WIDTH*2 + col] & 0xff;
                            if ((col == 0 & row == 0) || (col == PIC_WIDTH & row == PIC_HEIGHT)) {
                                System.out.println("yuv = " + y + " " + u + " " + v);
                                System.out.println("byte yuv = " + Byte.toString(yuvInfo[row*PIC_WIDTH*6 + col]) + " " +
                                                   Byte.toString(yuvInfo[row*PIC_WIDTH*6 + PIC_WIDTH + col]) + " " +
                                                   Byte.toString(yuvInfo[row*PIC_WIDTH*6 + PIC_WIDTH*2 + col]));
                            }

                            //change range of u and v for rgb conversion
                            u-=128;
                            v-=128;

                            //System.out.println("yuv = " + y + " " + u + " " + v);
                            rInfo[row][col] = (int)((1.164*y) + (1.596*u));
                            gInfo[row][col] = (int)((1.164*y) - (0.813*u) - (0.391*v));
                            bInfo[row][col] = (int)((1.164*y) + (1.596*v));

                            if (rInfo[row][col]<0) rInfo[row][col]=0; else if (rInfo[row][col]>255) rInfo[row][col]=255;
                            if (gInfo[row][col]<0) gInfo[row][col]=0; else if (gInfo[row][col]>255) gInfo[row][col]=255;
                            if (bInfo[row][col]<0) bInfo[row][col]=0; else if (bInfo[row][col]>255) bInfo[row][col]=255;
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
            for (int row = 0; row < PIC_HEIGHT; row++) {
                for (int col = 0; col < PIC_WIDTH; col++) {
                    //System.out.prrowntln(rROWnfo[row][j] + " " + gROWnfo[row][j] + " " + bROWnfo[i][j]);
                    g.setColor( new Color(rInfo[row][col],gInfo[row][col],bInfo[row][col]) );
                    g.fillRect(col*scale,row*scale,scale,scale);

                    if (cPlane[row * PIC_WIDTH + col] == currentColour) {
                        g.setColor(Color.black);
                        g.fillRect(col*scale,row*scale,scale,scale);
                    }
                }
            }
            g.setColor(Color.black);
        }
    }
}
