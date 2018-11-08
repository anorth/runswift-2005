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

import java.awt.*;
import java.awt.image.*;
import javax.swing.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.*;

/**
 * Title:        DebugModule
 * Description:
 * Copyright:    Copyright (c) 2003
 * Company:      UNSW 2003 Robocup
 * @author UNSW 2003 Robocup (Raymond Sheh)
 * @version 1.0
 */

/**
 * IMPORTANT:
 * To change debugging size you need to change at least these parts:
 * 1: value of DEBUG_SIZE below
 * 2: value of memory space REGION_J in ../definitions/SharedMemoryDef.h
 * 3: value of DEBUG_SIZE in ../commTrunk/VisualCortex.cc (NOW CHANGED TO VISION.CC)
 * 4: value of DEBUG_SIZE in client.c
 */

public class DebugModule
    extends JFrame {
    public static int EDGE_PATTERN = 2;
    public static int EDGE_HOUGH = 1;
    public static int EDGE_DETECT_TYPE = EDGE_PATTERN;

    public static final int fieldWidth = GLField.WIDTH;
    public static final int fieldHeight = GLField.HEIGHT;
    public static final int offsetX = 50;
    public static final int offsetY = 50;
    public static final int incX = 1;
    public static final int incY = 1;
    public static final int gradFieldScaling = 2; //each dimension in the incoming gradfield is half real size.

    public static final int PIC_WIDTH = RoboConstant.CPLANE_WIDTH, PIC_HEIGHT = RoboConstant.CPLANE_HEIGHT;
    public static final int MODE_POSITION = 1;
    private int port, mode;
    private RoboCommander roboCommander;
    private DebugDisplay debugDisplay = new DebugDisplay();
    private Container content;
    public static final int DEBUG_PLANES = 3;
    public static final int DEBUG_SIZE = PIC_WIDTH * PIC_HEIGHT * DEBUG_PLANES;
    private byte[] debugInfo = new byte[DEBUG_SIZE];
    public static final int SCALING = 1;
    public static final int DEBUG_TYPE = 20; //message type to key on
    public static final int HOUGH_NUM_LINES = 3; //Number of hough lines. Must match VisualCortex.h.
    public static final boolean HOUGH_ORIGIN_CENTRE = true;
    public static final int HOUGH_THETA = 2;

    int gradFieldMode;

//  public int WIN_WIDTH, WIN_HEIGHT;

//  	public static final int WIN_WIDTH = PIC_WIDTH * SCALING, WIN_HEIGHT = PIC_HEIGHT*3*SCALING; //for hough
    public static final int WIN_WIDTH = (fieldWidth + 2 * offsetX) * SCALING,
        WIN_HEIGHT = (fieldHeight + 2 * offsetY) * SCALING; //for pattern

    public DebugModule(int port, RoboCommander rc, int inMode) {
        super("Debug Module");
        try {
            jbInit();
            /*
                  if (EDGE_DETECT_TYPE == EDGE_HOUGH)
                  {
                    WIN_WIDTH = PIC_WIDTH * SCALING;
                    WIN_HEIGHT = PIC_HEIGHT*3*SCALING; //for hough
                  }
                  else if (EDGE_DETECT_TYPE == EDGE_PATTERN)
                  {
                    WIN_WIDTH = (fieldWidth+2*offsetX)*SCALING;
                    WIN_HEIGHT = (fieldHeight+2*offsetY)*SCALING; //for pattern
                  }
             */

            this.port = port;
            this.roboCommander = rc;
            this.mode = inMode;
            (new Thread(debugDisplay)).start();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        gradFieldMode = 0;
    }

    private void jbInit() throws Exception {
        //Initializes things ...
        content = this.getContentPane();
        content.setLayout(new BorderLayout());
//		debugDisplay.setPreferredSize(new Dimension(WIN_WIDTH, WIN_HEIGHT));
        content.add(debugDisplay, BorderLayout.CENTER);
        this.pack();

    }

    // reads data until data array full
    private int read(InputStream in, byte[] data) throws IOException {
        int read = 0;
        while (read != data.length) {
            int newRead = in.read(data, read, data.length - read);
            if (newRead <= 0) {
                return read;
            }
            read += newRead;
        }
        System.out.println("DebugModule:received size: " + read);
        return read;
    }

//	class DebugDisplay extends JPanel implements Runnable
    class DebugDisplay
        extends Component
        implements Runnable {
        BufferedImage img = new BufferedImage(WIN_WIDTH, WIN_HEIGHT,
                                              BufferedImage.TYPE_INT_ARGB);
        Graphics2D g2d;
        boolean changed = true;
        int edges[][] = new int[HOUGH_NUM_LINES][2];
        int houghLines[][] = new int[HOUGH_NUM_LINES][7];
        int fieldLines[][][] = new int[HOUGH_NUM_LINES][2][2]; //2 points per line, 2 co-ords per point
        int gradField[] = new int[ ( (fieldWidth + 2 * offsetX) *
                                    (fieldHeight + 2 * offsetY) / (incX * incY)) /
            (2 * gradFieldScaling)];

        DebugDisplay() {
//			super(false);
            super();
            g2d = img.createGraphics();
            this.addMouseListener(new MouseAdapter() {
                public void mousePressed(MouseEvent e) {
                    gradFieldMode++;
                    if (gradFieldMode > 8) { //remember, we go from 1 to 8 not 0 to 7
                        gradFieldMode = 0;
                    }
                    System.out.println("gradFieldMode = " + gradFieldMode);
                    changed = true;
                }
            }
            );
        }

        public Dimension getPreferredSize() {
            return new Dimension(WIN_WIDTH, WIN_HEIGHT);
        }

        public Dimension getMinimumSize() {
            return getPreferredSize();
        }

        public void setChanged(boolean c) {
            changed = c;
        }

        public void run() {
            byte[] type = new byte[1];
            InputStream in;
            try {
                ServerSocket ss = new ServerSocket(port);
                System.out.println("DebugModule (receive," + port + "): ready");
                Socket s = ss.accept();
                in = s.getInputStream();
                ss.close();
                while (true) {
                    while (true) {
                        read(in, type);
                        //Check if it's a debug thingy
                        if (type[0] == DEBUG_TYPE) {
                            if (read(in, debugInfo) != debugInfo.length) {
                                System.out.println("debugInfo cut short");
                            }
                            if (EDGE_DETECT_TYPE == EDGE_HOUGH) { //{{{
//!!!!!							//Do something with debugInfo
                                //At this stage, take the HOUGH_NUM_LINES*2 integers embedded in the
                                //first row of the picture as being r/theta values
                                //of the HOUGH_NUM_LINES strongest lines. Output this to edges[][]
                                //in preparation for painting.
                                for (int i = 0; i < HOUGH_NUM_LINES; i++) {

                                    edges[i][0] = (int) (
                                        ( ( (int) ( (byte) debugInfo[i * 8 + 3] &
                                        0xff) << 24) |
                                         ( (int) ( (byte) debugInfo[i * 8 + 2] &
                                                  0xff) << 16) |
                                         ( (int) ( (byte) debugInfo[i * 8 + 1] &
                                                  0xff) << 8) |
                                         ( (int) ( (byte) debugInfo[i * 8 + 0] &
                                                  0xff))));
                                    edges[i][1] = (int) (
                                        ( ( (int) ( (byte) debugInfo[i * 8 + 7] &
                                        0xff) << 24) |
                                         ( (int) ( (byte) debugInfo[i * 8 + 6] &
                                                  0xff) << 16) |
                                         ( (int) ( (byte) debugInfo[i * 8 + 5] &
                                                  0xff) << 8) |
                                         ( (int) ( (byte) debugInfo[i * 8 + 4] &
                                                  0xff))));

                                    System.out.println("r=" + edges[i][0] +
                                        " t=" + edges[i][1]);
                                    setHoughLines();
                                    //								setHoughLinesBasic();
                                }
                                //Now read the field translated endpoint co-ords.
                                for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                                    for (int j = 0; j < 2; j++) {
                                        for (int k = 0; k < 2; k++) {
                                            fieldLines[i][j][k] = (int) (
                                                ( ( (int) ( (byte) debugInfo[
                                                HOUGH_NUM_LINES * 8 +
                                                i * 16 + j * 8 + k * 4 + 3] &
                                                0xff) << 24) |
                                                 ( (int) ( (byte) debugInfo[
                                                HOUGH_NUM_LINES * 8 +
                                                i * 16 + j * 8 + k * 4 + 2] &
                                                0xff) << 16) |
                                                 ( (int) ( (byte) debugInfo[
                                                HOUGH_NUM_LINES * 8 +
                                                i * 16 + j * 8 + k * 4 + 1] &
                                                0xff) << 8) |
                                                 ( (int) ( (byte) debugInfo[
                                                HOUGH_NUM_LINES * 8 +
                                                i * 16 + j * 8 + k * 4 + 0] &
                                                0xff))));
                                            System.out.println("fieldLines[" +
                                                i + "][" + j + "][" +
                                                k + "] = " + fieldLines[i][j][k]);
                                        }
                                    }
                                }
                            } //}}}
                            else if (EDGE_DETECT_TYPE == EDGE_PATTERN) {
                                //store gradField
                                System.out.println("Going up to " +
                                    ( (fieldWidth + 2 * offsetX) *
                                     (fieldHeight + 2 * offsetY) / (incX * incY)) /
                                    (2 * gradFieldScaling));
                                System.out.println("debugInfo length = " +
                                    debugInfo.length);
                                for (int i = 0;
                                     i <
                                     ( (fieldWidth + 2 * offsetX) *
                                      (fieldHeight + 2 * offsetY) /
                                      (incX * incY)) / (2 * gradFieldScaling);
                                     i++) {
                                    if (i < gradField.length &&
                                        i < debugInfo.length) {
                                        gradField[i] = (int) ( (byte) debugInfo[
                                            i] & 0xFF);
                                    }
                                    else {
                                        System.out.println(
                                            "gradField index out of range! " +
                                            i +
                                            " gradField.length=" +
                                            gradField.length +
                                            " debugInfo.length=" +
                                            debugInfo.length);
                                    }
                                }
                            }
                            break;
                        }
                    }
                    DebugModule.this.setVisible(true);
                    changed = true;
                    repaint();
                }
            }
            catch (Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }

        public void paint_old(Graphics g) { //{{{
            Color colorArray[] = new Color[11]; //FIELDWHITE: Note that field white
            //will be sent as 14 in the cplane.
            //By default, this is recognised
            //as 'other' and assigned black.
            //Instead, we'll assign it grey
            //but we'll need to take it as a special case.
            colorArray[0] = Color.ORANGE;
            colorArray[1] = Color.CYAN;
            colorArray[2] = (Color.GREEN).darker();
            colorArray[3] = Color.YELLOW;
            colorArray[4] = Color.PINK;
            colorArray[5] = Color.BLUE;
            colorArray[6] = Color.RED;
            colorArray[7] = (Color.GREEN).brighter();
            colorArray[8] = Color.WHITE;
            colorArray[9] = Color.BLACK;
            colorArray[10] = Color.WHITE.darker().darker();

            Color angleArray[] = new Color[8];
            angleArray[0] = Color.RED;
            angleArray[1] = Color.ORANGE;
            angleArray[2] = Color.YELLOW;
            angleArray[3] = (Color.GREEN).brighter();
            angleArray[4] = Color.GREEN;
            angleArray[5] = Color.CYAN;
            angleArray[6] = Color.BLUE;
            angleArray[7] = Color.MAGENTA;

            if (changed) {
                int houghMax = 1;
                g2d.setColor(Color.white);
                g2d.fillRect(0, 0, getWidth(), getHeight());
                //Draw stuff on the screen here.
                /*Moved down the bottom ...
                    //Plane 1: edges
                    for (int h = 0; h < PIC_HEIGHT; h ++)
                    {
                     for (int w = 0; w < PIC_WIDTH; w ++)
                     {
                      int blah = (int)((byte)debugInfo[h*PIC_WIDTH+w] & 0xFF);
                      //Edge detected picture
                      if (blah == 10)
                      {
                       g2d.setColor(Color.WHITE);
                      }
                      else
                      {
                       if (blah >= 0 && blah < 8)
                       {
                        //Separate out into the different colors.
                        g2d.setColor(angleArray[blah]);
                       }
                       else
                       {
                        g2d.setColor(Color.BLACK);
//								System.out.println("Invalid edge, " + w + "," + h + " = " + blah);
                       }
                      }
                      g2d.fillRect(w*SCALING,h*SCALING,SCALING,SCALING);
                     }
                    }*/

                //Plane 2: Cplane
                for (int h = PIC_HEIGHT; h < PIC_HEIGHT * 2; h++) {
                    for (int w = 0; w < PIC_WIDTH; w++) {
                        int blah = debugInfo[h * PIC_WIDTH + w];
                        if (blah >= 0 && blah <= 9) {
                            g2d.setColor(colorArray[debugInfo[h * PIC_WIDTH + w]]);
                        }
                        else {
                            if (blah != 14) {
                                g2d.setColor(Color.BLACK);
                            }
                            else {
                                g2d.setColor(colorArray[10]);
                            }
                        }
                        g2d.fillRect(w * SCALING, h * SCALING, SCALING, SCALING);
                    }
                }
                //Plane 3: Houghspace

                for (int h = PIC_HEIGHT * 2; h < PIC_HEIGHT * 3; h++) {
                    for (int w = 0; w < PIC_WIDTH; w++) {
                        int blah = (int) ( (byte) debugInfo[h * PIC_WIDTH + w] &
                                          0xFF);
                        if (blah > houghMax) {
                            houghMax = blah;
                        }
                    }
                }

                System.out.println("houghMax = " + houghMax);
                for (int h = PIC_HEIGHT * 2; h < PIC_HEIGHT * 3; h++) {
                    for (int w = 0; w < PIC_WIDTH; w++) {
                        g2d.setColor(Color.PINK);
                        int blah = (int) ( (byte) debugInfo[h * PIC_WIDTH + w] &
                                          0xFF);
                        if ( (int) ( (byte) debugInfo[h * PIC_WIDTH + w] & 0xFF) >
                            255 ||
                            (int) ( (byte) debugInfo[h * PIC_WIDTH + w] & 0xFF) <
                            0) {
                            g2d.setColor(Color.BLUE);
                            System.out.println("debugInfo = " +
                                               (int) ( (byte) debugInfo[h *
                                PIC_WIDTH + w] &
                                0xFF));
                        }
                        else {
                            if (blah > houghMax || blah < 0) {
                                System.out.println("blah = " + blah +
                                    " houghMax = " + houghMax);
                                g2d.setColor(Color.RED);
                            }
                            else {
                                blah = (int) ( (double) 255 * (double) blah /
                                              (double) houghMax);
                                if (blah > 255 || blah < 0) {
//									System.out.println("Blah wrong = " + blah);
                                    g2d.setColor(Color.GREEN);
                                }
                                else {
                                    if (blah == 253) {
                                        g2d.setColor(Color.ORANGE);
                                    }
                                    else {
                                        g2d.setColor(new Color(255 - blah,
                                            255 - blah, 255 - blah));
                                    }
                                }
                            }
                        }
                        g2d.fillRect(w * SCALING, h * SCALING, SCALING, SCALING);
                    }
                }
                //Print angle marks.
//				g2d.setColor(Color.GREEN);
//				g2d.drawLine(45*i*SCALING, PIC_HEIGHT*2*SCALING, 45*i*SCALING, PIC_HEIGHT*3*SCALING);

                /* Moved down the bottom so that it overwrites field lines that have gone too far.
                    //Now draw hough lines and points.
                    for (int i = 0; i < HOUGH_NUM_LINES; i ++)
                    {
                 System.out.println("Drawing hough line " + i + " " + houghLines[i][0] + " " + houghLines[i][1] + " " + houghLines[i][2] + " " + houghLines[i][3]);
                     g2d.setColor(new Color(houghLines[i][4], houghLines[i][5], houghLines[i][6]));
                     //actual line
                     g2d.drawLine(houghLines[i][0] * SCALING, houghLines[i][1] * SCALING, houghLines[i][2] * SCALING, houghLines[i][3] * SCALING);
                     //dot in hough space
                 if (HOUGH_THETA == 1)
                 {
                     g2d.fillRect((edges[i][1]/3) * SCALING, (edges[i][0] + PIC_HEIGHT*2)*SCALING, SCALING, SCALING);
                 }
                 else
                 {
                 g2d.fillRect((edges[i][1])*SCALING, (edges[i][0] + PIC_HEIGHT*2)*SCALING, SCALING, SCALING);
                 }
                     //key bars
                     g2d.fillRect(i*4*SCALING, (PIC_HEIGHT - 1)*SCALING, 4*SCALING, SCALING);
                    }*/

                //Now draw where lines are relative to the robot. We'll draw these
                //on top of Hough space simply because we don't have anywhere
                //else to draw it (but also, note that the mid-lower section of
                //Hough space is pretty empty anyway).
                //Centre dog about 1/4 the way from the bottom (we can't put it right
                //at the bottom because it'll leave out all lines found when the head is
                //looking to the side).
                //To convert from the co-ords of the dog to co-ords in Java:
                //xnew/SCALING = PIC_WIDTH/2 + x
                //ynew/SCALING = PIC_HEIGHT*2 + (PIC_HEIGHT*3/4)-y
                for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                    g2d.setColor(new Color(houghLines[i][4], houghLines[i][5],
                                           houghLines[i][6]));

                    g2d.drawLine(SCALING * (PIC_WIDTH / 2 + fieldLines[i][0][0]),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                  fieldLines[i][0][1]),
                                 SCALING * (PIC_WIDTH / 2 + fieldLines[i][1][0]),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                  fieldLines[i][1][1]));
                    /*
                         g2d.drawLine(	SCALING*(PIC_WIDTH/2+fieldLines[i][0][0]),
                         SCALING*(PIC_HEIGHT+(PIC_HEIGHT/2)-fieldLines[i][0][1]),
                                SCALING*(PIC_WIDTH/2+fieldLines[i][1][0]),
                         SCALING*(PIC_HEIGHT+(PIC_HEIGHT/2)-fieldLines[i][1][1]));
                     */
                }
                g2d.setColor(Color.RED);
                //draw robot body
//				g2d.fillOval(SCALING*(PIC_WIDTH/2-2), SCALING*(PIC_HEIGHT*2+(PIC_HEIGHT*3/4)-2), SCALING*4, SCALING*4);
                g2d.setColor(Color.GRAY.darker());
                //draw scale
                g2d.drawLine(SCALING * (PIC_WIDTH / 2),
                             SCALING * (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4)),
                             SCALING * (PIC_WIDTH / 2),
                             SCALING *
                             (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) - 200));
                g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 100),
                             SCALING * (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4)),
                             SCALING * (PIC_WIDTH / 2 + 100),
                             SCALING * (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4)));

                for (int i = 0; i < 21; i++) {
                    g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 1),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                  10 * i),
                                 SCALING * (PIC_WIDTH / 2 + 1),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                  10 * i));
                    g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) - 1),
                                 SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) + 1));

                }
                //Draw bigger lines every 5 ...
                for (int i = 0; i < 21; i += 5) {
                    g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 3),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                  10 * i),
                                 SCALING * (PIC_WIDTH / 2 + 3),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                  10 * i));
                    g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) - 3),
                                 SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) + 3));

                }

                //Plane 1: edges
                for (int h = 0; h < PIC_HEIGHT; h++) {
                    for (int w = 0; w < PIC_WIDTH; w++) {
                        int blah = (int) ( (byte) debugInfo[h * PIC_WIDTH + w] &
                                          0xFF);
                        //Edge detected picture

                        if (blah == 10) {
                            g2d.setColor(Color.WHITE);
                        }
                        else {
                            if (blah >= 0 && blah < 8) {
                                //Separate out into the different colors.
                                g2d.setColor(angleArray[blah]);
                            }
                            else {
                                g2d.setColor(Color.BLACK);
//								System.out.println("Invalid edge, " + w + "," + h + " = " + blah);
                            }
                        }
                        g2d.fillRect(w * SCALING, h * SCALING, SCALING, SCALING);

                    }
                }

                //Now draw hough lines and points.
                for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                    System.out.println("Drawing hough line " + i + " " +
                                       houghLines[i][0] +
                                       " " + houghLines[i][1] + " " +
                                       houghLines[i][2] +
                                       " " + houghLines[i][3]);
                    g2d.setColor(new Color(houghLines[i][4], houghLines[i][5],
                                           houghLines[i][6]));
                    //actual line
                    g2d.drawLine(houghLines[i][0] * SCALING,
                                 houghLines[i][1] * SCALING,
                                 houghLines[i][2] * SCALING,
                                 houghLines[i][3] * SCALING);
                    //dot in hough space
                    if (HOUGH_THETA == 1) {
                        g2d.fillRect( (edges[i][1] / 3) * SCALING,
                                     (edges[i][0] + PIC_HEIGHT * 2) * SCALING,
                                     SCALING,
                                     SCALING);
                    }
                    else {
                        g2d.fillRect( (edges[i][1]) * SCALING,
                                     (edges[i][0] + PIC_HEIGHT * 2) * SCALING,
                                     SCALING,
                                     SCALING);
                        g2d.fillRect( (edges[i][1] + 90) * SCALING,
                                     (edges[i][0] + PIC_HEIGHT * 2 +
                                      PIC_HEIGHT / 2 + 1) *
                                     SCALING, SCALING, SCALING);
                        g2d.fillRect( (edges[i][1] - 90) * SCALING,
                                     (edges[i][0] + PIC_HEIGHT * 2 +
                                      PIC_HEIGHT / 2 + 1) *
                                     SCALING, SCALING, SCALING);
                    }
                    //key bars
                    g2d.fillRect(i * 4 * SCALING, (PIC_HEIGHT - 1) * SCALING,
                                 4 * SCALING,
                                 SCALING);
                }

            }
            g.drawImage(img, 0, 0, this);
            changed = false;
        } //}}}

        public void paint(Graphics g) {
            if (EDGE_DETECT_TYPE == EDGE_HOUGH) { //{{{
                Color colorArray[] = new Color[11]; //FIELDWHITE: Note that field white
                //will be sent as 14 in the cplane.
                //By default, this is recognised
                //as 'other' and assigned black.
                //Instead, we'll assign it grey
                //but we'll need to take it as a special case.
                colorArray[0] = Color.ORANGE;
                colorArray[1] = Color.CYAN;
                colorArray[2] = (Color.GREEN).darker();
                colorArray[3] = Color.YELLOW;
                colorArray[4] = Color.PINK;
                colorArray[5] = Color.BLUE;
                colorArray[6] = Color.RED;
                colorArray[7] = (Color.GREEN).brighter();
                colorArray[8] = Color.WHITE;
                colorArray[9] = Color.BLACK;
                colorArray[10] = Color.WHITE.darker().darker();

                Color angleArray[] = new Color[8];
                angleArray[0] = Color.RED;
                angleArray[1] = Color.ORANGE;
                angleArray[2] = Color.YELLOW;
                angleArray[3] = (Color.GREEN).brighter();
                angleArray[4] = Color.GREEN;
                angleArray[5] = Color.CYAN;
                angleArray[6] = Color.BLUE;
                angleArray[7] = Color.MAGENTA;

                if (changed) {
                    int houghMax = 1;
                    g2d.setColor(Color.white);
                    g2d.fillRect(0, 0, getWidth(), getHeight());
                    //Draw stuff on the screen here.
                    /*Moved down the bottom ...
                            //Plane 1: edges
                            for (int h = 0; h < PIC_HEIGHT; h ++)
                            {
                              for (int w = 0; w < PIC_WIDTH; w ++)
                              {
                         int blah = (int)((byte)debugInfo[h*PIC_WIDTH+w] & 0xFF);
                                //Edge detected picture
                                if (blah == 10)
                                {
                                  g2d.setColor(Color.WHITE);
                                }
                                else
                                {
                                  if (blah >= 0 && blah < 8)
                                  {
                                    //Separate out into the different colors.
                                    g2d.setColor(angleArray[blah]);
                                  }
                                  else
                                  {
                                    g2d.setColor(Color.BLACK);
                       //								System.out.println("Invalid edge, " + w + "," + h + " = " + blah);
                                  }
                                }
                         g2d.fillRect(w*SCALING,h*SCALING,SCALING,SCALING);
                              }
                            }*/

                    //Plane 2: Cplane
                    for (int h = PIC_HEIGHT; h < PIC_HEIGHT * 2; h++) {
                        for (int w = 0; w < PIC_WIDTH; w++) {
                            int blah = debugInfo[h * PIC_WIDTH + w];
                            if (blah >= 0 && blah <= 9) {
                                g2d.setColor(colorArray[debugInfo[h * PIC_WIDTH +
                                             w]]);
                            }
                            else {
                                if (blah != 14) {
                                    g2d.setColor(Color.BLACK);
                                }
                                else {
                                    g2d.setColor(colorArray[10]);
                                }
                            }
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                        }
                    }
                    //Plane 3: Houghspace

                    for (int h = PIC_HEIGHT * 2; h < PIC_HEIGHT * 3; h++) {
                        for (int w = 0; w < PIC_WIDTH; w++) {
                            int blah = (int) ( (byte) debugInfo[h * PIC_WIDTH +
                                              w] & 0xFF);
                            if (blah > houghMax) {
                                houghMax = blah;
                            }
                        }
                    }

                    System.out.println("houghMax = " + houghMax);
                    for (int h = PIC_HEIGHT * 2; h < PIC_HEIGHT * 3; h++) {
                        for (int w = 0; w < PIC_WIDTH; w++) {
                            g2d.setColor(Color.PINK);
                            int blah = (int) ( (byte) debugInfo[h * PIC_WIDTH +
                                              w] & 0xFF);
                            if ( (int) ( (byte) debugInfo[h * PIC_WIDTH + w] &
                                        0xFF) > 255 ||
                                (int) ( (byte) debugInfo[h * PIC_WIDTH + w] &
                                       0xFF) < 0) {
                                g2d.setColor(Color.BLUE);
                                System.out.println("debugInfo = " +
                                    (int) ( (byte) debugInfo[h * PIC_WIDTH + w] &
                                           0xFF));
                            }
                            else {
                                if (blah > houghMax || blah < 0) {
                                    System.out.println("blah = " + blah +
                                        " houghMax = " +
                                        houghMax);
                                    g2d.setColor(Color.RED);
                                }
                                else {
                                    blah = (int) ( (double) 255 * (double) blah /
                                                  (double) houghMax);
                                    if (blah > 255 || blah < 0) {
                                        //									System.out.println("Blah wrong = " + blah);
                                        g2d.setColor(Color.GREEN);
                                    }
                                    else {
                                        if (blah == 253) {
                                            g2d.setColor(Color.ORANGE);
                                        }
                                        else {
                                            g2d.setColor(new Color(255 - blah,
                                                255 - blah, 255 - blah));
                                        }
                                    }
                                }
                            }
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                        }
                    }
                    //Print angle marks.
                    //				g2d.setColor(Color.GREEN);
                    //				g2d.drawLine(45*i*SCALING, PIC_HEIGHT*2*SCALING, 45*i*SCALING, PIC_HEIGHT*3*SCALING);

                    /* Moved down the bottom so that it overwrites field lines that have gone too far.
                            //Now draw hough lines and points.
                            for (int i = 0; i < HOUGH_NUM_LINES; i ++)
                            {
                       System.out.println("Drawing hough line " + i + " " + houghLines[i][0] + " " + houghLines[i][1] + " " + houghLines[i][2] + " " + houghLines[i][3]);
                         g2d.setColor(new Color(houghLines[i][4], houghLines[i][5], houghLines[i][6]));
                              //actual line
                              g2d.drawLine(houghLines[i][0] * SCALING, houghLines[i][1] * SCALING, houghLines[i][2] * SCALING, houghLines[i][3] * SCALING);
                              //dot in hough space
                       if (HOUGH_THETA == 1)
                       {
                              g2d.fillRect((edges[i][1]/3) * SCALING, (edges[i][0] + PIC_HEIGHT*2)*SCALING, SCALING, SCALING);
                       }
                       else
                       {
                       g2d.fillRect((edges[i][1])*SCALING, (edges[i][0] + PIC_HEIGHT*2)*SCALING, SCALING, SCALING);
                       }
                              //key bars
                         g2d.fillRect(i*4*SCALING, (PIC_HEIGHT - 1)*SCALING, 4*SCALING, SCALING);
                            }*/

                    //Now draw where lines are relative to the robot. We'll draw these
                    //on top of Hough space simply because we don't have anywhere
                    //else to draw it (but also, note that the mid-lower section of
                    //Hough space is pretty empty anyway).
                    //Centre dog about 1/4 the way from the bottom (we can't put it right
                    //at the bottom because it'll leave out all lines found when the head is
                    //looking to the side).
                    //To convert from the co-ords of the dog to co-ords in Java:
                    //xnew/SCALING = PIC_WIDTH/2 + x
                    //ynew/SCALING = PIC_HEIGHT*2 + (PIC_HEIGHT*3/4)-y
                    for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                        g2d.setColor(new Color(houghLines[i][4],
                                               houghLines[i][5],
                                               houghLines[i][6]));

                        g2d.drawLine(SCALING *
                                     (PIC_WIDTH / 2 + fieldLines[i][0][0]),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                      fieldLines[i][0][1]),
                                     SCALING *
                                     (PIC_WIDTH / 2 + fieldLines[i][1][0]),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                      fieldLines[i][1][1]));
                        /*
                             g2d.drawLine(	SCALING*(PIC_WIDTH/2+fieldLines[i][0][0]),
                             SCALING*(PIC_HEIGHT+(PIC_HEIGHT/2)-fieldLines[i][0][1]),
                             SCALING*(PIC_WIDTH/2+fieldLines[i][1][0]),
                             SCALING*(PIC_HEIGHT+(PIC_HEIGHT/2)-fieldLines[i][1][1]));
                         */
                    }
                    g2d.setColor(Color.RED);
                    //draw robot body
                    //				g2d.fillOval(SCALING*(PIC_WIDTH/2-2), SCALING*(PIC_HEIGHT*2+(PIC_HEIGHT*3/4)-2), SCALING*4, SCALING*4);
                    g2d.setColor(Color.GRAY.darker());
                    //draw scale
                    g2d.drawLine(SCALING * (PIC_WIDTH / 2),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4)),
                                 SCALING * (PIC_WIDTH / 2),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) - 200));
                    g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 100),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4)),
                                 SCALING * (PIC_WIDTH / 2 + 100),
                                 SCALING *
                                 (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4)));

                    for (int i = 0; i < 21; i++) {
                        g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 1),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                      10 * i),
                                     SCALING * (PIC_WIDTH / 2 + 1),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                      10 * i));
                        g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) - 1),
                                     SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) + 1));

                    }
                    //Draw bigger lines every 5 ...
                    for (int i = 0; i < 21; i += 5) {
                        g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 3),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                      10 * i),
                                     SCALING * (PIC_WIDTH / 2 + 3),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) -
                                      10 * i));
                        g2d.drawLine(SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) - 3),
                                     SCALING * (PIC_WIDTH / 2 - 100 + 10 * i),
                                     SCALING *
                                     (PIC_HEIGHT * 2 + (PIC_HEIGHT * 3 / 4) + 3));

                    }

                    //Plane 1: edges
                    for (int h = 0; h < PIC_HEIGHT; h++) {
                        for (int w = 0; w < PIC_WIDTH; w++) {
                            int blah = (int) ( (byte) debugInfo[h * PIC_WIDTH +
                                              w] & 0xFF);
                            //Edge detected picture

                            if (blah == 10) {
                                g2d.setColor(Color.WHITE);
                            }
                            else {
                                if (blah >= 0 && blah < 8) {
                                    //Separate out into the different colors.
                                    g2d.setColor(angleArray[blah]);
                                }
                                else {
                                    g2d.setColor(Color.BLACK);
                                    //								System.out.println("Invalid edge, " + w + "," + h + " = " + blah);
                                }
                            }
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);

                        }
                    }

                    //Now draw hough lines and points.
                    for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                        System.out.println("Drawing hough line " + i + " " +
                                           houghLines[i][0] + " " +
                                           houghLines[i][1] + " " +
                                           houghLines[i][2] + " " +
                                           houghLines[i][3]);
                        g2d.setColor(new Color(houghLines[i][4],
                                               houghLines[i][5],
                                               houghLines[i][6]));
                        //actual line
                        g2d.drawLine(houghLines[i][0] * SCALING,
                                     houghLines[i][1] * SCALING,
                                     houghLines[i][2] * SCALING,
                                     houghLines[i][3] * SCALING);
                        //dot in hough space
                        if (HOUGH_THETA == 1) {
                            g2d.fillRect( (edges[i][1] / 3) * SCALING,
                                         (edges[i][0] + PIC_HEIGHT * 2) *
                                         SCALING, SCALING,
                                         SCALING);
                        }
                        else {
                            g2d.fillRect( (edges[i][1]) * SCALING,
                                         (edges[i][0] + PIC_HEIGHT * 2) *
                                         SCALING, SCALING,
                                         SCALING);
                            g2d.fillRect( (edges[i][1] + 90) * SCALING,
                                         (edges[i][0] + PIC_HEIGHT * 2 +
                                          PIC_HEIGHT / 2 + 1) *
                                         SCALING, SCALING, SCALING);
                            g2d.fillRect( (edges[i][1] - 90) * SCALING,
                                         (edges[i][0] + PIC_HEIGHT * 2 +
                                          PIC_HEIGHT / 2 + 1) *
                                         SCALING, SCALING, SCALING);
                        }
                        //key bars
                        g2d.fillRect(i * 4 * SCALING,
                                     (PIC_HEIGHT - 1) * SCALING,
                                     4 * SCALING, SCALING);
                    }

                }
            } //}}}
            else if (EDGE_DETECT_TYPE == EDGE_PATTERN && changed) { //{{{
                //We have a gradient field gradField where each dimension needs to be scaled by gradFieldScaling before being put onto the screen.
                //Note that any value 255 is a sentinel value which we should mark in red.
                g2d.setColor(Color.WHITE);
                g2d.fillRect(0, 0, getWidth(), getHeight());
                System.out.println("Displaying up to " +
                                   ( ( (fieldHeight + 2 * offsetY - 1) *
                                      (fieldWidth + 2 * offsetX) +
                                      (fieldWidth + 2 * offsetX - 1)) /
                                    (2 * gradFieldScaling)));
                for (int h = 0; h < fieldHeight + 2 * offsetY; h++) {
                    for (int w = 0; w < fieldWidth + 2 * offsetX; w++) {
                        int x = w / 2;
                        int y = h / 2;
                        int gradVal = gradField[ (int) ( (y *
                            (fieldWidth + 2 * offsetX) /
                            2 + x))];
                        if (gradFieldMode != 0) {
                            //we do bitwise checks instead. If bit is set to 1, set gradVal to 252 else set to 0.
                            int blah = (int) Math.pow(2.0,
                                (double) gradFieldMode - 1.0);
                            if ( ( ( (char) gradVal & 0xFF) &
                                  ( (char) blah & 0xFF)) != 0) {
                                gradVal = 252;
                            }
                            else {
                                gradVal = 0;
                            }
                        }

                        if (h == offsetY || h == fieldHeight + offsetY ||
                            w == offsetX ||
                            w == fieldWidth + offsetX) {
                            //one of the boundaries, mark in green.
                            g2d.setColor(Color.GREEN);
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                        }
                        else if (gradVal == 255) {
                            //sentinel, mark in red.
                            g2d.setColor(Color.RED);
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                            System.out.println("Marking " + w + "/" + h +
                                               " in red");
                        }
                        else if (gradVal == 254) {
                            //sentinel, mark in blue.
                            g2d.setColor(Color.BLUE);
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                            System.out.println("Marking " + w + "/" + h +
                                               " in blue");
                        }
                        else if (gradVal == 253) {
                            //Sentinel, mark in green.
                            g2d.setColor(Color.GREEN);
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                            System.out.println("Marking " + w + "/" + h +
                                               " in blue");
                        }
                        else {
                            if (gradVal < 0 || gradVal > 255) {
                                System.out.println("gradVal at " + h + "/" + w +
                                    " has value " +
                                    gradVal);
                                gradVal = 0;
                            }
                            g2d.setColor(new Color(gradVal, gradVal, gradVal));
                            g2d.fillRect(w * SCALING, h * SCALING, SCALING,
                                         SCALING);
                        }
                    }
                }
            } //}}}
            g.drawImage(img, 0, 0, this);
            changed = false;
        }

        private void setHoughLinesBasic() {
            //takes lines from edges[][] and converts into start/end points
            //similar to setHoughLines but instead only gives short tick marks.
            float colorSeparation = (float) 1.0 / ( (float) HOUGH_NUM_LINES);
            int lineColors[][] = new int[HOUGH_NUM_LINES][3];
            for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                float currentHue, rgb[] = null;
                currentHue = colorSeparation * (float) i;
                rgb = (new Color(Color.HSBtoRGB(currentHue, (float) 1,
                                                (float) 1))).
                    getRGBComponents(null);
                lineColors[i][0] = (int) (rgb[0] * 255.0);
                lineColors[i][1] = (int) (rgb[1] * 255.0);
                lineColors[i][2] = (int) (rgb[2] * 255.0);
            }
            for (int i = 0; i < HOUGH_NUM_LINES; i++) {
                if (edges[i][0] == -1 || edges[i][1] == -1) {
                    houghLines[i][0] = 0;
                    houghLines[i][1] = 0;
                    houghLines[i][2] = 0;
                    houghLines[i][3] = 0;
                    houghLines[i][4] = 0;
                    houghLines[i][5] = 0;
                    houghLines[i][6] = 0;
                }
                else {
                    //This algorithm is a lot simpler than the old one. The first point is the radial point
                    //from the centre at a distance r and angle t. The second point is that of a line
                    //10 pixels long and pointing clockwise.
                    int r = edges[i][0];
                    int t = edges[i][1] * HOUGH_THETA;
                    //note that t is in degrees.
                    while (t < 0) {
                        t = t + 360;
                    }
                    while (t > 359) {
                        t = t - 360;
                    }
                    //Note that t is from straight down, to convert to java co-ords
                    //we need to subtract 90.
                    t = t - 90;
                    houghLines[i][0] = (int) ( (double) r *
                                              Math.cos(Math.toRadians(t)));
                    houghLines[i][1] = (int) ( (double) r *
                                              Math.sin(Math.toRadians(t)));
                    houghLines[i][2] = (int) ( (double) r *
                                              Math.cos(Math.toRadians(t)) -
                                              50.0 * Math.sin(Math.toRadians(t)));
                    houghLines[i][3] = (int) ( (double) r *
                                              Math.sin(Math.toRadians(t)) +
                                              50.0 * Math.cos(Math.toRadians(t)));
                    for (int j = 0; j < 4; j++) {
                        System.out.println("houghLines[" + i + "][" + j +
                                           "] = " +
                                           houghLines[i][j]);
                    }
                    //Note that all these are positive and in relation to the middle. Translate this to
                    //co-ords relative to the top left corner and flip y-axis.
                    houghLines[i][0] = houghLines[i][0] + PIC_WIDTH / 2;
                    houghLines[i][1] = PIC_HEIGHT / 2 - houghLines[i][1];
                    houghLines[i][2] = houghLines[i][2] + PIC_WIDTH / 2;
                    houghLines[i][3] = PIC_HEIGHT / 2 - houghLines[i][3];
                    houghLines[i][4] = lineColors[i][0];
                    houghLines[i][5] = lineColors[i][1];
                    houghLines[i][6] = lineColors[i][2];
                }
            }
        }

        private void setHoughLines() {
            //Takes the 4 lines from edges[][] and converts them into start-end points
            //ready for painting. Has the format:
            //
            //houghLines[][0] = x1
            //houghLines[][1] = x2
            //houghLines[][2] = x3
            //houghLines[][3] = x4
            //houghLines[][5] = rgbcolor
            int NUMLINES = HOUGH_NUM_LINES;
            float colorSeparation = (float) 1.0 / ( (float) NUMLINES);
            int lineColors[][] = new int[NUMLINES][3];
            for (int i = 0; i < NUMLINES; i++) {

                float currentHue, rgb[] = null;
                currentHue = colorSeparation * (float) i;
//				rgb = (new Color(Color.HSBtoRGB((float)0, (float)0.5, (float)1-currentHue))).getRGBComponents(null);
                rgb = (new Color(Color.HSBtoRGB(currentHue, (float) 1,
                                                (float) 1))).
                    getRGBComponents(null);
                lineColors[i][0] = (int) (rgb[0] * 255.0);
                lineColors[i][1] = (int) (rgb[1] * 255.0);
                lineColors[i][2] = (int) (rgb[2] * 255.0);

//lineColors[i][0] = lineColors[i][1] = lineColors[i][2] = 255*(int)((float)i*colorSeparation);
            }
            //Now find the line endpoints.
            for (int i = 0; i < NUMLINES; i++) {
                if (edges[i][0] == -1 || edges[i][1] == -1) {
                    //line has been eliminated so shift it off screen.
                    houghLines[i][0] = 0;
                    houghLines[i][1] = 0;
                    houghLines[i][2] = 0;
                    houghLines[i][3] = 0;
                    houghLines[i][4] = 0;
                    houghLines[i][5] = 0;
                    houghLines[i][6] = 0;
                }
                else
                if (HOUGH_ORIGIN_CENTRE) {

                    //Hough transform is taken around centre of image (as in lines
                    //with r=0 pass through the centre)
                    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
                    int r = edges[i][0];
                    int t = edges[i][1] * HOUGH_THETA;
                    //Make no assumptions on where t is
                    while (t < 0) {
                        t = t + 360;
                    }
                    while (t > 359) {
                        t = t - 360;
                    }
                    //now t is between 0 and 360. Make it between 0 and 180 for simplicity
                    //(and make r negative where necessary).
                    if (t > 179) {
                        t = t - 180;
                        r = -r;
                    }
                    //Now we need to know the slope of the line. We need to treat this line
                    //differently depending on if it is closer to vertical or closer to horizontal.
                    //Otherwise this slope may become infinite!
                    if (t < 45 || t > 135) {
                        //ie. line is closer to horizontal. The slope of the line in terms
                        //of horizontal is m=tan(t).
                        double m = Math.tan(Math.toRadians(t)), yInt, leftInt,
                            rightInt,
                            leftX, rightX; //leftX is the x position of the left point, same for rightX.
                        //Now find the vertical axis intercept. Note that this intercept may well be
                        //off the image (but it will never be more than half an image height above
                        //or below the image as we've constrained it to 45 degrees). Also note that
                        //this axis is in the centre of the image (at PIC_WIDTH/2). Note that this will
                        //be different depending on if the line is sloping up or down.
                        if (m > 0) {
                            //line is sloping upwards. Find the vertical intercept.
                            //Note that t is between 0 and 45. The vertical intercept
                            //is the negative of the hypotenuse of a right triangle with r adjacent to the angle t.
                            yInt = - (double) r / Math.cos(Math.toRadians(t));
                        }
                        else if (m == 0) {
                            //line is horizontal. The y intercept is equal to -r.
                            yInt = -r;
                        }
                        else {
                            //line is sloping downwards. The vertical intercept is
                            //the hypotenuse of a right triangle with r adjacent ot the angle 180-t.
                            yInt = (double) r / Math.cos(Math.toRadians(180 - t));
                        }
                        //We now have the y intercept (which may not be on the screen and which is zero'd at
                        //the point PIC_HEIGHT/2) and we have the slope. Find the points where it hits the
                        //left and right edges of the screen. Again note that these points may well be off the screen.
                        leftInt = yInt - m * (double) PIC_WIDTH / 2.0;
                        rightInt = yInt + m * (double) PIC_WIDTH / 2.0;
                        //Now check individually if the left and right intercepts are beyond the picture. If so, correct them.
                        //Also assign top and bottom intercepts. Again remember that these are all zero'd about the centre
                        //of the screen.
                        if (leftInt > (double) PIC_HEIGHT / 2.0) {
                            //Line intercepts left above picture. Adjust leftX. NOTE: This won't work with m = 0
                            //but then again, if the line intercepts above the picture and m = 0 then the line
                            //isn't even on the screen! This should only be called really for m < 0.
                            leftX = - (leftInt - ( (double) PIC_HEIGHT / 2.0)) /
                                m -
                                (double) PIC_WIDTH / 2.0;
                            leftInt = (double) PIC_HEIGHT / 2.0;
                        }
                        else if (leftInt < - (double) PIC_HEIGHT / 2.0) {
                            //Line intercepts left below picture. Adjust leftX. NOTE: Again the thing about m = 0.
                            //This really only applies for m > 0.
                            leftX = - (leftInt + ( (double) PIC_HEIGHT / 2.0)) /
                                m -
                                (double) PIC_WIDTH / 2.0;
                            leftInt = - (double) PIC_HEIGHT / 2.0;
                        }
                        else {
                            //left intercept is somewhere on the picture so set the left X position to
                            //the full left edge.
                            leftX = - (double) PIC_WIDTH / 2.0;
                        }
                        //Now do the same for the right intercept.
                        if (rightInt > (double) PIC_HEIGHT / 2.0) {
                            //remember, m > 0
                            rightX = (double) PIC_WIDTH / 2.0 -
                                (rightInt - (double) PIC_HEIGHT / 2.0) / m;
                            rightInt = (double) PIC_HEIGHT / 2.0;
                        }
                        else if (rightInt < - (double) PIC_HEIGHT / 2.0) {
                            //remember, m < 0
                            rightX = (double) PIC_WIDTH / 2.0 -
                                (rightInt + ( (double) PIC_HEIGHT / 2.0)) / m;
                            rightInt = - (double) PIC_HEIGHT / 2.0;
                        }
                        else {
                            rightX = (double) PIC_WIDTH / 2.0;
                        }
                        //now leftX, leftInt, rightX and rightInt form the pairs we need to pass. Convert them
                        //back to ints that have the origin in the upper left corner.
                        x1 = (int) (leftX + (double) PIC_WIDTH / 2.0);
                        y1 = (int) ( -leftInt + (double) PIC_HEIGHT / 2.0);
                        x2 = (int) (rightX + (double) PIC_WIDTH / 2.0);
                        y2 = (int) ( -rightInt + (double) PIC_HEIGHT / 2.0);
                    }
                    else {
                        //Line is closer to vertical. The slope of the line in terms of
                        //vertical is m=1/tan(t).
                        double m = 1.0 / Math.tan(Math.toRadians(t)), xInt,
                            bottomInt,
                            topInt, bottomY, topY;
                        System.out.println("Line " + i +
                                           " is closer to vertical m = " + m);
                        //Now find the horizontal axis intercept. Note that thsi intercept may well be
                        //off the image (but it will never be more than half an image width to the left
                        //or right of theimage as we've constrained it to 45 degrees). Also note that
                        //this axis is in the centre of the image (at PIC_HEIGHT/2). Note that this will
                        //be different depending on if the line is sloping up or down.
                        if (m > 0) {
                            //line is sloping upwards. Find the horizontal intercept.
                            //Note that t is between 45 and 90. The horizontal intercept
                            //is the hypotenuse of a right triangle with r opposite the angle t.
                            xInt = (double) r / Math.sin(Math.toRadians(t));
                        }
                        else if (m == 0) {
                            //line is vertical. The x intercept is equal to r.
                            xInt = r;
                        }
                        else {
                            //line is sloping downwards. The horizontal intercept is the
                            //hypotenuse of a right triangle with r adjacent to the
                            //angle t-90.
                            xInt = (double) r / Math.cos(Math.toRadians(t - 90));
                        }
//System.out.println("xInt = " + xInt + " r = " + r);
                        //We now have the x intercept (which may not be on the screen and and which is zero'd at
                        //the point PIC_WIDTH/2) and we have the slope. Find the points where it hits the
                        //top and bottom edges of the screen. Again note that these points may well be off the screen.
                        topInt = xInt + m * (double) PIC_HEIGHT / 2.0;
                        bottomInt = xInt - m * (double) PIC_HEIGHT / 2.0;
                        //Now check individually if the top and bottom intercepts are beyond the picture. If so, correct them.
                        //Also assign left and right intercepts. Again remember that these are all zero'd about the centre
                        //of the screen.
                        if (topInt > (double) PIC_WIDTH / 2.0) { //CORRECT ON ROBOT
                            //Line intercepts the top too far to the right. Adjust topY. NOTE: This won't work with
                            //m = 0 but then again, if the line intercepts to the right of the picture and m = 0 then
                            //the line isn't even on the screen! This should only be called really for m > 0.
                            topY = (double) PIC_HEIGHT / 2.0 -
                                (topInt - (double) PIC_WIDTH / 2.0) / m; //CORRECT ON ROBOT
                            topInt = (double) PIC_WIDTH / 2.0; //CORRECT ON ROBOT
                        }
                        else if (topInt < - (double) PIC_WIDTH / 2.0) { //CORRECT ON ROBOT
                            //Line intercepts the top too far to the left. Adjust topY. NOTE: Again the thing about m = 0.
                            //This really only applies for m < 0.
                            topY = (double) PIC_HEIGHT / 2.0 -
                                (topInt + (double) PIC_WIDTH / 2.0) / m; //CORRECT ON ROBOT
                            topInt = - (double) PIC_WIDTH / 2.0; //CORRECT ON ROBOT
                            System.out.println("topY = " + topY + " topInt = " +
                                               topInt);
                        }
                        else {
                            //Top intercept is somewhere on the picture so set the top Y position to
                            //the full top edge.
                            topY = (double) PIC_HEIGHT / 2.0;
                        }
                        //Now do the same for bottom intercept.
                        if (bottomInt > (double) PIC_WIDTH / 2.0) {
                            //Too far to right. m < 0.
                            bottomY = - (bottomInt - (double) PIC_WIDTH / 2.0) /
                                m -
                                (double) PIC_HEIGHT / 2.0;
                            bottomInt = (double) PIC_WIDTH / 2.0;
                        }
                        else if (bottomInt < - (double) PIC_WIDTH / 2.0) {
                            //Too far to left. m > 0.
                            bottomY = - (bottomInt + (double) PIC_WIDTH / 2.0) /
                                m -
                                (double) PIC_HEIGHT / 2.0;
                            bottomInt = - (double) PIC_WIDTH / 2.0;
                        }
                        else {
                            //Somewhere in the middle
                            bottomY = - (double) PIC_HEIGHT / 2.0;
                        }
                        //now topInt, topY, bottomInt and bottomY form the pairs we need to pass. Convert them
                        //back to ints that have the origin in the upper left corner.
                        x1 = (int) (topInt + (double) PIC_WIDTH / 2.0);
                        y1 = (int) ( -topY + (double) PIC_HEIGHT / 2.0);
                        x2 = (int) (bottomInt + (double) PIC_WIDTH / 2.0);
                        y2 = (int) ( -bottomY + (double) PIC_HEIGHT / 2.0);
                    }
                    houghLines[i][0] = (int) x1;
                    houghLines[i][1] = (int) y1;
                    houghLines[i][2] = (int) x2;
                    houghLines[i][3] = (int) y2;
                    houghLines[i][4] = lineColors[i][0];
                    houghLines[i][5] = lineColors[i][1];
                    houghLines[i][6] = lineColors[i][2];

                }
                else {
                    //Hough transform is taken around upper left corner of image (as in lines
                    //with r=0 pass through the upper right corner)
                    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
                    int r = edges[i][0];
                    int t = edges[i][1] * HOUGH_THETA;
                    double xm = Math.sin(rad(t)) * r;
                    double ym = Math.cos(rad(t)) * r;
                    if (Math.abs(t) < 1.0 || Math.abs(t - 360) < 1.0) {
                        //ie. angle is close to zero so line is pretty much horizontal
                        x1 = 0;
                        y1 = r;
                        x2 = PIC_WIDTH;
                        y2 = r;
                    }
                    else if (Math.abs(t - 90) < 1.0 || Math.abs(t - 270) < 1.0) {
                        //ie. angle is close to 90 so line is pretty much vertical
                        x1 = r;
                        y1 = 0;
                        x2 = r;
                        y2 = PIC_HEIGHT;
                    }
                    else {
                        //All other cases
                        double m = -xm / ym;
                        double h = (double) PIC_HEIGHT;
                        double w = (double) PIC_WIDTH;
                        double xA = (h - ym) / m + xm;
                        double yA = -m * xm + ym;
                        double xB = xm - ym / m;
                        double yB = m * (w - xm) + ym;
                        if (m < 0) {
                            if (yA > h) {
                                x1 = xA;
                                y1 = h;
                            }
                            else {
                                x1 = 0;
                                y1 = yA;
                            }
                            if (xB > w) {
                                x2 = w;
                                y2 = yB;
                            }
                            else {
                                x2 = xB;
                                y2 = 0;
                            }
                        }
                        else {
                            if (yA < 0) {
                                x1 = xB;
                                y1 = 0;
                            }
                            else {
                                x1 = 0;
                                y1 = yA;
                            }
                            if (xA > w) {
                                x2 = w;
                                y2 = yB;
                            }
                            else {
                                x2 = xA;
                                y2 = h;
                            }
                        }
                    }
                    houghLines[i][0] = (int) x1;
                    houghLines[i][1] = (int) y1;
                    houghLines[i][2] = (int) x2;
                    houghLines[i][3] = (int) y2;

                    /*
                     houghLines[i][0] = 0;
                     houghLines[i][1] = 0;
                         houghLines[i][2] = (int)((double)edges[i][0]*Math.sin(rad(edges[i][1])));
                         houghLines[i][3] = (int)((double)edges[i][0]*Math.cos(rad(edges[i][1])));
                     */
                    houghLines[i][4] = lineColors[i][0];
                    houghLines[i][5] = lineColors[i][1];
                    houghLines[i][6] = lineColors[i][2];
                }
            }
        }

        private double rad(int t) {
            double angle = (double) t;
            return angle / 180.0 * Math.PI;
        }

    }
}
