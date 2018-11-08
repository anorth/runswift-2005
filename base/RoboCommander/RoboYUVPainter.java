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
 * $Id: RoboYUVPainter.java 2001 2003-09-24 12:02:35Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 **/

import java.awt.*;
import java.awt.image.*;
import javax.swing.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.*;

public class RoboYUVPainter
    extends JFrame
    implements BasicWindowRepainter {

    public static final int GCD_PLANES = 18;
    public static final int R = 0;
    public static final int Y = 0;
    public static final int G = 1;
    public static final int U = 1;
    public static final int B = 2;
    public static final int V = 2;
    public static final int TRANSPARENCY = 200; // 0-255, 255 being solid and 0 being invisible
    public static final String XYC_COMMAND = "xyc"; // command to update the gcd mapping
    public static final String RECEIVE_COMMAND = "x"; // command to receive new image

    private byte[] yuvInfo = new byte[RoboConstant.YUVPLANE_SIZE]; // stores the yuv bytes received
    private byte[] cPlaneInfo = new byte[RoboConstant.CPLANE_SIZE]; // stores the yuv bytes received
    private byte[] dummyCPlane = new byte[RoboConstant.CPLANE_WIDTH];
    private int[][][] rgbInfo = new int[3][RoboConstant.CPLANE_HEIGHT][
        RoboConstant.CPLANE_WIDTH];

    private double headPan = 0, headTilt = 0, arx = 0, ary = 0, arh = 0;

    private JFrame positionFrame = new JFrame("Robot position");
    private BasicWindow positionWindow;

    private int scale = 3;

    public static final Color[] classifyColours = {
        new Color(255, 127, 0, TRANSPARENCY),
        new Color(127, 127, 255, TRANSPARENCY),
        new Color(0, 127, 0, TRANSPARENCY),
        new Color(255, 255, 0, TRANSPARENCY),
        new Color(255, 127, 127, TRANSPARENCY),
        new Color(0, 0, 200, TRANSPARENCY),
        new Color(255, 0, 0, TRANSPARENCY),
        new Color(127, 255, 127, TRANSPARENCY),
        new Color(255, 255, 255, TRANSPARENCY),
        new Color(0, 0, 0, TRANSPARENCY)
    };

    // which plane to display
    private int currentPlane = 15;

    // port to receive info
    private int port;

    // where the mouse clicked on (units in relation to yuv array)
    private int realX = -1, realY = -1, clickX = -1, clickY = -1;
    private RoboCommander roboCommander;

    private JButton bSave = new JButton("Save");
    private JButton bReceive = new JButton("Receive");
    private JLabel lFileName = new JLabel("File Name");
    private JTextField fFileName = new JTextField(20);
    private JLabel lScale = new JLabel("View Scale");
    private JTextField fScale = new JTextField(scale + "", 4);
    private YUVDisplay pDrawingBoard = new YUVDisplay();
    private Container content;
    private JScrollPane sDrawingView = new JScrollPane();

    private JPanel pControl = new JPanel();
    private JPanel pBFLControl = new JPanel();
    private JPanel pCPlaneControl = new JPanel();

    public RoboYUVPainter(int port, RoboCommander rc) {
        super("BFL Image");
        for (int i = 0; i < RoboConstant.CPLANE_SIZE; i++) {
            cPlaneInfo[i] = 0;
        }
        for (int i = 0; i < RoboConstant.CPLANE_SIZE * 3; i++) {
            yuvInfo[i] = 0;
        }
        try {
            jbInit();
            this.port = port;
            this.roboCommander = rc;
            (new Thread(pDrawingBoard)).start();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

    }

    private boolean isNumber(String s) {
        try {
            int t = Integer.parseInt(s);
            return true;
        }
        catch (NumberFormatException ex) {
            return false;
        }
    }

    /*
     * look for number in the file name and increment it
     */
    private String getIncrementedFileName(String fileName) {
        String newName;

        for (int i = 0; i < fileName.length(); i++) {
            for (int j = fileName.length() - 1; j >= i; j--) {
                if (isNumber(fileName.substring(i, j + 1))) {
                    int num = Integer.parseInt(fileName.substring(i, j + 1));
                    newName = fileName.substring(0, i) + String.valueOf(num + 1) +
                        fileName.substring(j + 1);
                    return newName;
                }
            }
        }
        newName = new String(fileName);
        //if no number found, insert it before the dot
        if (fileName.indexOf(".") != -1) {
            newName.replaceFirst(".", "0.");
        }
        else
            newName = newName + "0";
        return newName;
    }

    // saves the image in a bfl file
    private void save() {
        String fileName = fFileName.getText();
        // no file name
        if (fileName == null || fileName.equals("")) {
            JOptionPane.showMessageDialog(null, "Error: invalid filename field",
                                          "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        try {
            File file = new File(fileName);
            FileOutputStream out = new FileOutputStream(file);
            for (int i = 0; i < RoboConstant.CPLANE_HEIGHT; i++) {
                out.write(yuvInfo, i * RoboConstant.CPLANE_WIDTH * 6,
                          RoboConstant.CPLANE_WIDTH * 6);
                out.write(cPlaneInfo, i * RoboConstant.CPLANE_WIDTH,
                          RoboConstant.CPLANE_WIDTH);
            }
            out.close();
            System.out.println("RoboYUVPainter.java: File saved as <" +
                               fileName + ">");
            fFileName.setText(getIncrementedFileName(fileName));
        }
        catch (IOException e) {
            JOptionPane.showMessageDialog(null,
                "Error: IOException caught. \nCannot write to file " + fileName,
                                          "Error", JOptionPane.ERROR_MESSAGE);
            System.err.println("RoboYUVPainter.java: Poblem in saving file <" +
                               fileName + ">");
        }
    }

    // reads data until data array full
    private int read(InputStream in, byte[] data) throws IOException {
        int read = 0;
        while (read != data.length) {
            int newRead = in.read(data, read, data.length - read);
            if (newRead == 0) {
                return read;
            }
            else if (newRead == -1) {
                return -1;
            }

            read += newRead;
        }
        System.out.println("YUVPainter:received size: " + read);
        return read;
    }

    private void YUVtoRGB(byte[] yuvInfo, int[][][] rgbInfo) {
        int y, u, v;
        for (int row = 0; row < RoboConstant.CPLANE_HEIGHT; row++) {
            for (int col = 0; col < RoboConstant.CPLANE_WIDTH; col++) {
                y = yuvInfo[row * RoboConstant.CPLANE_WIDTH * 6 + col] & 0xff;
                u = yuvInfo[row * RoboConstant.CPLANE_WIDTH * 6 +
                    RoboConstant.CPLANE_WIDTH + col] & 0xff;
                v = yuvInfo[row * RoboConstant.CPLANE_WIDTH * 6 +
                    RoboConstant.CPLANE_WIDTH * 2 + col] & 0xff;

                int[] rgb = RoboYUV.YUV2RGB(y, u, v);
                rgbInfo[R][row][col] = rgb[0];
                rgbInfo[G][row][col] = rgb[1];
                rgbInfo[B][row][col] = rgb[2];
            }
        }
    }

    // convert byte to int
    public int unsignedToInt(byte b) {
        return b & 0xff;
    }

    private void processMouse(MouseEvent e) {
        clickX = e.getX();
        clickY = e.getY();
        realX = clickX / scale;
        realY = clickY / scale;

        //System.out.println("click: " + clickX + " " + clickY);
        //System.out.println("real: " + realX + " " + realY);
        if (realX < 0 || realX > RoboConstant.CPLANE_WIDTH || realY < 0 ||
            realY > RoboConstant.CPLANE_HEIGHT) {
            System.err.println(
                "initMainWindow: error in real numbers, click outside window");
        }

        pDrawingBoard.repaint();
    }

    private void sendToRoboCommander(String name, String value) {
        if (roboCommander == null) {
            System.out.println("Trying to send , name='" + name + "' , value='" +
                               value + "'");
        }
        else {
            roboCommander.send(name, value);
        }
    }

    private void sendMouse(MouseEvent e) {
        processMouse(e);

        // check slection values
        if (realX < 0 || realX > RoboConstant.CPLANE_WIDTH || realY < 0 ||
            realY > RoboConstant.CPLANE_HEIGHT) {
            System.err.println(
                "mouseReleased: error in real numbers, click outside window");
            return;
        }
        // check plane
        if (currentPlane >= RoboConstant.NUM_PLANES) {
            System.err.println("RoboYUVPainter: plane slection invalid");
            return;
        }

        // get the yuv and cplane info
        int y = mapY(unsignedToInt(yuvInfo[realY * 6 *
                                   RoboConstant.CPLANE_WIDTH + realX]));
        int u = mapU(unsignedToInt(yuvInfo[realY * 6 *
                                   RoboConstant.CPLANE_WIDTH + realX +
                                   RoboConstant.CPLANE_WIDTH]));
        int v = mapV(unsignedToInt(yuvInfo[realY * 6 *
                                   RoboConstant.CPLANE_WIDTH + realX +
                                   RoboConstant.CPLANE_WIDTH * 2]));
        int cPlaneColour = currentPlane;

        ////////////////////
        // send co-ordinates
        /////////////////////
        System.out.println("Sending Co-ordinates to Dog: " +
                           y + " " + u + " " + v + " " + currentPlane);

        sendToRoboCommander(XYC_COMMAND,
                            y + " " + u + " " + v + " " + currentPlane);

        // modify the cplane info
        int tempY, tempU, tempV;
        for (int row = 0; row < RoboConstant.CPLANE_HEIGHT; row++) {
            for (int col = 0; col < RoboConstant.CPLANE_WIDTH; col++) {
                tempY = mapY(unsignedToInt(yuvInfo[row * 6 *
                                           RoboConstant.CPLANE_WIDTH + col]));
                tempU = mapU(unsignedToInt(yuvInfo[row * 6 *
                                           RoboConstant.CPLANE_WIDTH + col +
                                           RoboConstant.CPLANE_WIDTH]));
                tempV = mapV(unsignedToInt(yuvInfo[row * 6 *
                                           RoboConstant.CPLANE_WIDTH + col +
                                           RoboConstant.CPLANE_WIDTH * 2]));
                //System.out.println("checking value");
                if (y == tempY && u == tempU && v == tempV) {
                    //System.out.println("same value");
                    cPlaneInfo[row * RoboConstant.CPLANE_WIDTH +
                        col] = (byte) cPlaneColour;
                }
            }
        }

        clickX = -1;
        clickY = -1;
        realX = -1;
        realY = -1;
        pDrawingBoard.drawCPlane = true;
        pDrawingBoard.repaint();
    }

    // maps the yuv value to the right GCD look up value
    private int mapY(int val) {
        int planeValue;

        if (val < 40)
            planeValue = 0;
        else if (val < 45)
            planeValue = 1;
        else if (val < 50)
            planeValue = 2;
        else if (val < 55)
            planeValue = 3;
        else if (val < 60)
            planeValue = 4;
        else if (val < 65)
            planeValue = 5;
        else if (val < 70)
            planeValue = 6;
        else if (val < 75)
            planeValue = 7;
        else if (val < 80)
            planeValue = 8;
        else if (val < 85)
            planeValue = 9;
        else if (val < 90)
            planeValue = 10;
        else if (val < 100)
            planeValue = 11;
        else if (val < 110)
            planeValue = 12;
        else if (val < 120)
            planeValue = 13;
        else if (val < 140)
            planeValue = 14;
        else if (val < 160)
            planeValue = 15;
        else if (val < 190)
            planeValue = 16;
        else
            planeValue = 17;

        return planeValue;
    }

    private int mapU(int u) {
        return u;
    }

    private int mapV(int v) {
        return v;
    }

    private void jbInit() throws Exception {
        content = this.getContentPane();
        content.setLayout(new BorderLayout());

        sDrawingView.getViewport().add(pDrawingBoard);
        pDrawingBoard.setPreferredSize(new Dimension(RoboConstant.CPLANE_WIDTH *
            scale, RoboConstant.CPLANE_HEIGHT * scale));
        content.add(sDrawingView, BorderLayout.CENTER);

        // setup control panel
        pControl.setLayout(new BorderLayout());
        // bfl control
        pBFLControl.setLayout(new FlowLayout());
        pBFLControl.add(lScale);
        pBFLControl.add(fScale);
        pBFLControl.add(lFileName);
        pBFLControl.add(fFileName);
        pBFLControl.add(bSave);
        pBFLControl.add(bReceive);
        pControl.add(pBFLControl, BorderLayout.SOUTH);

        // add cplane buttons
        pCPlaneControl.setLayout(new GridLayout(RoboConstant.NUM_PLANES /
                                                RoboConstant.NUM_COL,
                                                RoboConstant.NUM_COL));
        ButtonGroup buttonGroup = new ButtonGroup();
        for (int i = 0; i <= RoboConstant.NUM_PLANES + 1; i++) {
            String command = i + "";
            JRadioButton rb;
            // set text
            if (i == RoboConstant.NUM_PLANES) {
                rb = new JRadioButton("All");
            }
            else if (i == RoboConstant.NUM_PLANES + 1) {
                rb = new JRadioButton("None");
            }
            else {
                rb = new JRadioButton(command);
            }
            // set selection
            if (i == currentPlane) {
                rb.setSelected(true);
            }
            pCPlaneControl.add(rb);
            buttonGroup.add(rb);
            rb.setActionCommand(command);

            rb.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    currentPlane = Integer.parseInt(e.getActionCommand());
                    pDrawingBoard.drawCPlane = true;
                    pDrawingBoard.repaint();
                    //System.out.println(e.getActionCommand());
                }
            });
        }
        pControl.add(pCPlaneControl, BorderLayout.CENTER);

        content.add(pControl, BorderLayout.SOUTH);

        ////////////
        // add listeners
        ////////////
        fScale.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    int v = Integer.parseInt(fScale.getText());
                    scale = v;
                    pDrawingBoard.setPreferredSize(new Dimension(RoboConstant.
                        CPLANE_WIDTH * scale,
                        RoboConstant.CPLANE_HEIGHT * scale));
                    pDrawingBoard.changed = true;
                    pDrawingBoard.repaint();
                }
                catch (NumberFormatException ex) {}
            }
        });

        ActionListener aSave = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    save();
                }
                catch (NumberFormatException ex) {}
            }
        };
        fFileName.addActionListener(aSave);
        bSave.addActionListener(aSave);

        // recieve image data from dog
        bReceive.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                roboCommander.send(RECEIVE_COMMAND, 1 + "");
            }
        });

        // track mouse motions
        sDrawingView.addMouseListener(new MouseAdapter() {
            public void mousePressed(MouseEvent e) {
                processMouse(e);
            }

            public void mouseReleased(MouseEvent e) {
                sendMouse(e);
            }
        });
        sDrawingView.addMouseMotionListener(new MouseMotionAdapter() {
            public void mouseDragged(MouseEvent e) {
                processMouse(e);
            }
        });
        pack();

        //Prepare position frame.
        //300x500 for field, 100x500 for head position.
        positionWindow = new BasicWindow(300 + 100, 500, this);
        positionFrame.getContentPane().add(positionWindow);
        positionFrame.pack();
//        positionFrame.show();

        //show();
    }

    public void repaintBasicWindow(Graphics g, BasicWindow w) {
        if (w == positionWindow) {
            //This routine is called whenever repaint is called
            //on the basic window positionWindow.
            //Note that the origin of the field is
            //at (0, 300) and OWN goal is on top.
            int width = w.getPreferredSize().width;
            int height = w.getPreferredSize().height;
            g.setColor(Color.WHITE);
            g.fillRect(0, 0, width, height);
            g.setColor(Color.GREEN);
            // FIXME: does this need updating for new field?
            g.fillRect(9, 0, 291, 440);
            g.setColor(Color.BLACK);
            g.drawArc(300, 0, 100, 100, 0, 180);
            g.drawLine(300 + 50, 50,
                       300 + 50 - (int) ( (double) 50 * Math.sin(headPan)),
                       50 - (int) ( (double) 50 * Math.cos(headPan)));
            g.drawArc(300, 100, 100, 100, 90, 180);
            g.drawLine(300 + 50, 150,
                       300 + 50 - (int) ( (double) 50 * Math.cos(headTilt)),
                       150 + (int) ( (double) 50 * Math.sin(headTilt)));

            g.fillOval(300 - (int) arx - 5, (int) ary - 5, 10, 10);
            g.drawLine(300 - (int) arx, (int) ary,
                       300 -
                       ( (int) arx + (int) (50.0 * Math.cos(Math.toRadians(arh)))),
                       (int) ary + (int) (50.0 * Math.sin(Math.toRadians(arh))));

        }
    }

    class YUVDisplay
        extends JPanel
        implements Runnable {
        BufferedImage img = new BufferedImage(RoboConstant.CPLANE_WIDTH * scale,
                                              RoboConstant.CPLANE_HEIGHT *
                                              scale,
                                              BufferedImage.TYPE_INT_ARGB);
        BufferedImage imgYUV = new BufferedImage(RoboConstant.CPLANE_WIDTH *
                                                 scale,
                                                 RoboConstant.CPLANE_HEIGHT *
                                                 scale,
                                                 BufferedImage.TYPE_INT_ARGB);
        Graphics2D g2d, g2dYUV;
        boolean changed = true;
        boolean drawCPlane = true;

        YUVDisplay() {
            super(false);
            g2d = img.createGraphics();
            g2dYUV = imgYUV.createGraphics();
        }

        public void setChanged(boolean c) {
            changed = c;
        }

        public void readYUVPlane(InputStream in) throws IOException{
            byte[] type = new byte[1];
            while (true){
                if (read(in, type) == -1)
                    return;

                // cplane
                int received = 0;
                if (type[0] == 10) {
                    System.out.println("Waiting cPlane");

                    if ( (received = read(in, cPlaneInfo)) !=
                        cPlaneInfo.length) {
                        System.out.println("cPlaneInfo cut short" +
                                           received + " # " + cPlaneInfo.length);
                        return;
                    }
                }
                else if (type[0] == 11) {
                    System.out.println("Waiting yuvPlane");
                    // yuv
                    if ( (received = read(in, yuvInfo)) !=
                        yuvInfo.length) {
                        System.out.println("yuvInfo cut short" +
                                           received + " # " + yuvInfo.length);
                        return;
                    }
                    else {
                        //Print out head position and expected position.
//                double pan, tilt, arx, ary, arh;
                        int offset = yuvInfo.length -
                            RoboConstant.CPLANE_WIDTH;
                        headPan = Double.longBitsToDouble(
                            ( ( (long) (yuvInfo[offset + 7] & 0xff) <<
                               56) |
                             ( (long) (yuvInfo[offset + 6] & 0xff) <<
                              48) |
                             ( (long) (yuvInfo[offset + 5] & 0xff) <<
                              40) |
                             ( (long) (yuvInfo[offset + 4] & 0xff) <<
                              32) |
                             ( (long) (yuvInfo[offset + 3] & 0xff) <<
                              24) |
                             ( (long) (yuvInfo[offset + 2] & 0xff) <<
                              16) |
                             ( (long) (yuvInfo[offset + 1] & 0xff) <<
                              8) |
                             ( (long) (yuvInfo[offset + 0] & 0xff))));
                        headTilt = Double.longBitsToDouble(
                            ( ( (long) (yuvInfo[offset + 15] & 0xff) <<
                               56) |
                             ( (long) (yuvInfo[offset + 14] & 0xff) <<
                              48) |
                             ( (long) (yuvInfo[offset + 13] & 0xff) <<
                              40) |
                             ( (long) (yuvInfo[offset + 12] & 0xff) <<
                              32) |
                             ( (long) (yuvInfo[offset + 11] & 0xff) <<
                              24) |
                             ( (long) (yuvInfo[offset + 10] & 0xff) <<
                              16) |
                             ( (long) (yuvInfo[offset + 9] & 0xff) <<
                              8) |
                             ( (long) (yuvInfo[offset + 8] & 0xff))));
                        arx = Double.longBitsToDouble(
                            ( ( (long) (yuvInfo[offset + 23] & 0xff) <<
                               56) |
                             ( (long) (yuvInfo[offset + 22] & 0xff) <<
                              48) |
                             ( (long) (yuvInfo[offset + 21] & 0xff) <<
                              40) |
                             ( (long) (yuvInfo[offset + 20] & 0xff) <<
                              32) |
                             ( (long) (yuvInfo[offset + 19] & 0xff) <<
                              24) |
                             ( (long) (yuvInfo[offset + 18] & 0xff) <<
                              16) |
                             ( (long) (yuvInfo[offset + 17] & 0xff) <<
                              8) |
                             ( (long) (yuvInfo[offset + 16] & 0xff))));
                        ary = Double.longBitsToDouble(
                            ( ( (long) (yuvInfo[offset + 31] & 0xff) <<
                               56) |
                             ( (long) (yuvInfo[offset + 30] & 0xff) <<
                              48) |
                             ( (long) (yuvInfo[offset + 29] & 0xff) <<
                              40) |
                             ( (long) (yuvInfo[offset + 28] & 0xff) <<
                              32) |
                             ( (long) (yuvInfo[offset + 27] & 0xff) <<
                              24) |
                             ( (long) (yuvInfo[offset + 26] & 0xff) <<
                              16) |
                             ( (long) (yuvInfo[offset + 25] & 0xff) <<
                              8) |
                             ( (long) (yuvInfo[offset + 24] & 0xff))));
                        arh = Double.longBitsToDouble(
                            ( ( (long) (yuvInfo[offset + 39] & 0xff) <<
                               56) |
                             ( (long) (yuvInfo[offset + 38] & 0xff) <<
                              48) |
                             ( (long) (yuvInfo[offset + 37] & 0xff) <<
                              40) |
                             ( (long) (yuvInfo[offset + 36] & 0xff) <<
                              32) |
                             ( (long) (yuvInfo[offset + 35] & 0xff) <<
                              24) |
                             ( (long) (yuvInfo[offset + 34] & 0xff) <<
                              16) |
                             ( (long) (yuvInfo[offset + 33] & 0xff) <<
                              8) |
                             ( (long) (yuvInfo[offset + 32] & 0xff))));
                        System.out.println("\npan = " + headPan +
                                           "\ntilt= " + headTilt + "\narx = " +
                                           arx + "\nary = " + ary + "\narh = " +
                                           arh);
//                        positionWindow.repaint(); //Doesn't display anything useful??
//                        break;
                    }
                }
                // show the display if hidden
                RoboYUVPainter.this.setVisible(true);
//                positionFrame.show(); //Useless??

                // convert colour
                YUVtoRGB(yuvInfo, rgbInfo);
                changed = true;
                repaint();
            }
        }

        public void run() {

            InputStream in;
            try {
                ServerSocket ss = new ServerSocket(port);
                System.out.println();
                System.out.println("RoboYUVPainter (receive," + port +
                                   "): ready");
                while (true) {
                    Socket s = ss.accept();
                    System.out.println("RoboYUVPainter : Got new connection.");
                    in = s.getInputStream();

                    // read cPlane then yuv in equence

                    readYUVPlane(in);

                    System.out.println("RoboYUVPainter : Connection closed.");
                }
//                ss.close(); //FIXME:enable the thread to stop, and close serversocket
            }
            catch (Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }

        // draw yuv and cplane
        public void paint(Graphics g) {
            // image has increased in size
            if (img.getWidth() < getWidth() || img.getHeight() < getHeight()) {
                img = new BufferedImage(getWidth(), getHeight(),
                                        BufferedImage.TYPE_INT_ARGB);
                imgYUV = new BufferedImage(getWidth(), getHeight(),
                                           BufferedImage.TYPE_INT_ARGB);
                g2d = img.createGraphics();
                g2dYUV = imgYUV.createGraphics();
                changed = true;
            }

            if (changed) {
                //System.out.println("repainting");
                // clear screen
                g2dYUV.setColor(Color.white);
                g2dYUV.fillRect(0, 0, getWidth(), getHeight());

                // draw yuv in rgb colors
                for (int row = 0; row < RoboConstant.CPLANE_HEIGHT; row++) {
                    for (int col = 0; col < RoboConstant.CPLANE_WIDTH; col++) {
                        //System.out.prrowntln(rROWnfo[row][j] + " " + gROWnfo[row][j] + " " + bROWnfo[i][j]);
                        g2dYUV.setColor(new Color(rgbInfo[R][row][col],
                                                  rgbInfo[G][row][col],
                                                  rgbInfo[B][row][col]));
                        g2dYUV.fillRect(col * scale, row * scale, scale, scale);
                    }
                }
            }

            if (changed || drawCPlane) {
                g2d.drawImage(imgYUV, 0, 0, this);

                // draw cplane data
                int index = 0;
                for (int row = 0; row < RoboConstant.CPLANE_HEIGHT; row++) {
                    //System.out.println("" + cPlaneInfo[index]);
                    for (int col = 0; col < RoboConstant.CPLANE_WIDTH; col++) {
                        // draw the color of only wanted pixels
                        if (currentPlane == unsignedToInt(cPlaneInfo[index]) ||
                            currentPlane == RoboConstant.NUM_PLANES) {
//                            System.out.println("cPlaneInfo index = " + index + " --  unsignedtoInt = " +
//                            unsignedToInt(cPlaneInfo[index]) + " cPlaneInfo[index] = " + cPlaneInfo[index]);
                            g2d.setColor(classifyColours[unsignedToInt(
                                cPlaneInfo[index])]);

                            g2d.fillRect(col * scale, row * scale, scale, scale);
                        }
                        index++;
                    }
                }
            }
            g.drawImage(img, 0, 0, this);

            // draw mouse track
            g.setColor(Color.black);
            if (0 <= clickX && clickX < RoboConstant.CPLANE_WIDTH * scale &&
                0 <= clickY && clickY < RoboConstant.CPLANE_HEIGHT * scale) {
                // rectangle is drawn from x,y to x+width,y+width hence the -1 here
                g.drawRect(clickX - clickX % scale, clickY - clickY % scale,
                           scale - 1, scale - 1);
            }

            changed = false;
            drawCPlane = false;
        }
    } // end class YUVDisplay

    static final int DEFAULT_PORT = 5011;
    //main entry
    static public void main(String[] args) {
        RoboYUVPainter yuvPainter = new RoboYUVPainter(DEFAULT_PORT, null);
    }
}
