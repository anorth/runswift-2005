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
 * UNSW 2002 Robocup (Tim Tam)
 *
 * Last modification background information
 * $Id: CPlaneDisplay.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * displays the C-plane that the robot classifies
 *
**/

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import javax.swing.*;
import javax.swing.border.*;
import java.net.*;
import java.util.*;

public class CPlaneDisplay extends JFrame {

    private static final boolean debugMsg                   = false;
    private static final boolean optimizeDebugMsg           = false;
    private static final boolean goalTiltTesting            = false;
    private static final boolean newOplaneDebugMsg          = false;
    private static final boolean oplaneMissingDebugMsg      = false;
    private static final boolean compressedCPlaneDebugMsg   = false;
    private static final boolean objectCoordDebugMsg        = false;
    private static final boolean sensorValuesDebugMsg       = false;
    private static final boolean sanityValuesDebugMsg       = false;
    private static final boolean toBinaryDebugMsg           = false;
    private static final boolean printSanityDebugMsg        = false;
    private static final boolean unsignedByteDebugMsg       = false;
    private static final boolean loadAgainDebugMsg          = false;

    // to measure the number of cplane received
    private static long cplaneCounter                  = 0;
    private static final boolean cplaneCounterDebugMsg = false;

    /* To send a counter from vision to wireless to client.c to CPlaneDisplay.java
     * places needed to be change:
     * trunk/robot/share/SwitchBoard.h (DELAY_TESTING)
     * trunk/base/work/client/client.c (DELAY_TESTING)
     * trunk/base/work/RoboCommander/CPlaneDisplay.java (isDelayTesting)
    **/
    private static final boolean isDelayTesting = false;
    private static final int MAX_SCALE          = 5;
    private static final int MIN_SCALE          = 1;
    
    // version for the cplane, so that warning will pop up when log method is changed
    private static String CPLANE_VERSION        = "(empty)";
    private static String CPLANE_VERSION_FILE   = "CPLANE_VERSION.txt";
    
    // constants used for the drawing and window
    private static final int scale                    = 3;
    private static final int halfScale                = RoboConstant.SCALE / 2;
    private static final int window_width             = RoboConstant.CPLANE_WIDTH * scale;
    private static final int window_height            = RoboConstant.CPLANE_HEIGHT * scale;
    
    // the desire size gap to the top edge so that object names can be displayed correct
    public final static int TOP_GAP = 15;
    
    // to test whether the coordinate has very large number, ie when a unsigned number is read as signed, overfitting
    private static final int INVALID_COORD      = 100000000;
    
    // constants to indicate the loading of log file has finished
    private static final int FILE_LOAD_FINISH   = -1;
    

    
    private static final String ROBOT_FOLDER    = File.separator + "tmp";
    private static final String ROBOT_FILE_NAME = "image";
    private static final String ROBOT_FILE_EXT  = ".img";

    // GUI for the left and right panel
    JPanel leftPanel                = new JPanel();
    JPanel rightPanel               = new JPanel();

    // GUI for the cplane
    JPanel paintArea                = new PaintArea();

    // GUI for the control
    TitledBorder controlBorder      = new TitledBorder("Control Panel");
    JPanel controlPanel             = new JPanel();
    ButtonGroup buttonGroup         = new ButtonGroup();
    JButton saveButton              = new JButton("Save");
    
    // GUI for the sensor panel
    TitledBorder sensorBorder       = new TitledBorder("Sensor Panel");
    JPanel sensorPanel              = new JPanel();
    JLabel[] sensorLabel            = new JLabel[RoboConstant.NUM_SENSOR_COORDS];
    JTextField[] sensorTextField    = new JTextField[RoboConstant.NUM_SENSOR_COORDS];

    // GUI for the log
    JPanel logPanel                 = new JPanel();
    TitledBorder logBorder          = new TitledBorder("Play Back Panel");
    JProgressBar progressBar        = new JProgressBar();
    
    // GUI for the play back file
    JPanel playBackFilePanel        = new JPanel();
    JLabel playBackFileLabel        = new JLabel("Log:");
    JTextField playBackFileField    = new JTextField(25);
    JButton chooseButton            = new JButton("Choose File...");
    JButton loadButton              = new JButton("Load");

    // GUI for the play back panel
    JPanel playBackPanel            = new JPanel();
    JButton gotoStartButton         = new JButton("<<");
    JButton backwardButton          = new JButton("<");
    JButton playButton              = new JButton("Play");
    JButton stopButton              = new JButton("Stop");
    JButton forwardButton           = new JButton(">");
    JButton gotoLastButton          = new JButton(">>");
    JTextField currentFrame         = new JTextField(4);
    JLabel totalFrame               = new JLabel();
    
    // GUI for the sanity panel
    JPanel sanityPanel              = new JPanel();
    JLabel sanityLabel              = new JLabel("Sanity Checks");
    JTextArea sanityTextArea        = new JTextArea();
    JScrollPane sanityScrollPane    = new JScrollPane(sanityTextArea);
    

    Object parent;
    BufferedImage pic  = new BufferedImage(window_width, window_height, BufferedImage.TYPE_INT_ARGB);
    Graphics offscreen = pic.getGraphics();
    int currentPlane   = RoboConstant.NUM_PLANES;
    int port;
    
    byte[] buffer1 = new byte[RoboConstant.COMPRESSED_CPLANE_SIZE + RoboConstant.SENSOR_SIZE + RoboConstant.SANITY_SIZE + RoboConstant.OBJ_SIZE];
    byte[] buffer2 = new byte[RoboConstant.COMPRESSED_CPLANE_SIZE + RoboConstant.SENSOR_SIZE + RoboConstant.SANITY_SIZE + RoboConstant.OBJ_SIZE];
    int dataBufferSize1 = 0;
    int dataBufferSize2 = 0;
    int compressedCPlaneSize1 = 0;
    int compressedCPlaneSize2 = 0;
    
    boolean readInBuffer1, readInBuffer2, canDrawBuffer1, canDrawBuffer2;
    
    boolean saveToFile     = false;
    int     savedFileCount = 0;
    
    Drawer drawer = new Drawer();

    javax.swing.Timer animationTimer;
        
    CPlane[] cplaneArray = null;
    int cplaneIndex      = 0;
    
    public CPlaneDisplay(Object parent, int port) {
        super("CPlaneDisplay");
        try {
            this.parent = parent;
            this.port   = port;
            init(RoboConstant.DEFAULT_CPLANE_LOG_FILE, false);
            (new Receiver()).start();
        } catch(Exception e) {
            e.printStackTrace();
        }

    }

    public CPlaneDisplay(String filename) {
        super("CPlaneDisplay");
        try {
            int slashPos    = RoboConstant.DEFAULT_CPLANE_LOG_FILE.lastIndexOf(File.separator);
            if (slashPos>=0) {
                filename = RoboConstant.DEFAULT_CPLANE_LOG_FILE.substring(0, slashPos) + File.separator + filename;
            }
            initCPlaneLogVersion();
            init(filename, true);
            show();
            loadFile();
        } catch(Exception e) {
            e.printStackTrace();
        }

    }

    /* sets up display */
    private void init(String filename, boolean loadFile) throws Exception {
        this.readInBuffer1  = true;
        this.readInBuffer2  = true;
        this.canDrawBuffer1 = false;
        this.canDrawBuffer2 = false;
        
        paintArea.setPreferredSize(new Dimension(window_width, window_height));

        if (!loadFile) {
            // add buttons
            controlPanel.setLayout(new GridLayout(RoboConstant.NUM_PLANES/RoboConstant.NUM_COL, RoboConstant.NUM_COL));
            for (int i = 0 ; i <= RoboConstant.NUM_PLANES ; i++) {
                String command = i + "";
                JRadioButton rb;
                if (i == RoboConstant.NUM_PLANES) {
                    rb = new JRadioButton("All");
                    rb.setSelected(true);
                } else {
                    rb = new JRadioButton(command);
                }
                controlPanel.add(rb);
                buttonGroup.add(rb);
                rb.setActionCommand(command);

                rb.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        currentPlane = Integer.parseInt(e.getActionCommand());
                    }
                });
            }

            // add save button
            saveButton.addActionListener( new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    saveToFile = true;
                }
            });
            controlBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
            controlPanel.setBorder(controlBorder);
            controlPanel.add(saveButton);

        } else {
            sensorBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
            sensorPanel.setBorder(sensorBorder);
            sensorPanel.setLayout(new FlowLayout());
            sensorPanel.setPreferredSize(new Dimension(window_width, 70));
            for (int i=0; i<RoboConstant.NUM_SENSOR_COORDS; i++) {
                sensorLabel[i]     = new JLabel("Sensor" + i);
                sensorTextField[i] = new JTextField(9);
                sensorTextField[i].setEditable(false);
                sensorPanel.add(sensorLabel[i]);
                sensorPanel.add(sensorTextField[i]);
                sensorPanel.add(new JPanel());
            }
        }

        // set the default to the current directory
        playBackFileField.setText(filename);
        playBackFileField.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                loadFile();
            }
        });
        playBackFileField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                playBackFileField.selectAll();
            }
        });

        chooseButton.setToolTipText("Choose a file to play back");
        chooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser chooser;
                String currentDir = "";
                int slashPos;
                
                // start the file chooser in the directory of the specified file
                currentDir = playBackFileField.getText().trim();
                slashPos   = currentDir.lastIndexOf(File.separator);
                if (slashPos==-1) {
                    chooser = new JFileChooser();
                } else {
                    chooser = new JFileChooser(currentDir.substring(0, slashPos));
                }
                int returnVal = chooser.showOpenDialog(null);
                if(returnVal == JFileChooser.APPROVE_OPTION) {
                    playBackFileField.setText(chooser.getSelectedFile().getPath());
                }
            }
        });
        
        loadButton.setToolTipText("Load the log file to play back");
        loadButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                loadFile();
            }
        });
        
        playBackFilePanel.add(playBackFileLabel);
        playBackFilePanel.add(playBackFileField);
        playBackFilePanel.add(chooseButton);
        playBackFilePanel.add(loadButton);
        
        // GUI for the play back panel
        gotoStartButton.setToolTipText("Go to the first frame");
        gotoStartButton.setEnabled(false);
        gotoStartButton.setMnemonic(KeyEvent.VK_HOME);
        gotoStartButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                cplaneIndex = 0;
                paintScreen();
                
            }
        });

        backwardButton.setToolTipText("Go to the previous frame");
        backwardButton.setEnabled(false);
        backwardButton.setMnemonic(KeyEvent.VK_LEFT);
        backwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                cplaneIndex--;
                paintScreen();
            }
        });

        // animation of the cplane, implemented in the javax.swing.Timer class
        animationTimer = new javax.swing.Timer(RoboConstant.CPLANE_NEXT_FRAME_DELAY, new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                animationTimerAction();
            }
        });
        
        playButton.setToolTipText("Play animation");
        playButton.setEnabled(false);
        playButton.setMnemonic(KeyEvent.VK_P);
        playButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chooseButton.setEnabled(false);
                loadButton.setEnabled(false);
                gotoStartButton.setEnabled(false);
                backwardButton.setEnabled(false);
                forwardButton.setEnabled(false);
                gotoLastButton.setEnabled(false);
                playButton.setEnabled(false);
                currentFrame.setEditable(false);
                stopButton.setEnabled(true);
                animationTimer.start();
            }
        });

        stopButton.setToolTipText("Stop animation");
        stopButton.setEnabled(false);
        stopButton.setMnemonic(KeyEvent.VK_S);
        stopButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chooseButton.setEnabled(true);
                loadButton.setEnabled(true);
                gotoStartButton.setEnabled(true);
                backwardButton.setEnabled(true);
                forwardButton.setEnabled(true);
                gotoLastButton.setEnabled(true);
                playButton.setEnabled(true);
                currentFrame.setEditable(true);
                stopButton.setEnabled(false);
                animationTimer.stop();
            }
        });

        forwardButton.setToolTipText("Go to the next frame");
        forwardButton.setEnabled(false);
        forwardButton.setMnemonic(KeyEvent.VK_RIGHT);
        forwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                cplaneIndex++;
                paintScreen();
            }
        });
        
        gotoLastButton.setToolTipText("Go to the last frame");
        gotoLastButton.setEnabled(false);
        gotoLastButton.setMnemonic(KeyEvent.VK_END);
        gotoLastButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                cplaneIndex = cplaneArray.length-1;
                paintScreen();
            }
        });
        
        currentFrame.setEditable(false);
        currentFrame.setHorizontalAlignment(JTextField.RIGHT);
        currentFrame.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    cplaneIndex = Integer.parseInt(currentFrame.getText());
                    paintScreen();
                } catch (NumberFormatException e2) {
                    JOptionPane.showMessageDialog(null, "Error: Input must be integer", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        
        playBackPanel.add(gotoStartButton);
        playBackPanel.add(backwardButton);
        playBackPanel.add(playButton);
        playBackPanel.add(stopButton);
        playBackPanel.add(forwardButton);
        playBackPanel.add(gotoLastButton);
        playBackPanel.add(currentFrame);
        playBackPanel.add(totalFrame);
        
        if (loadFile) {
            progressBar.setStringPainted(true);
        }
        logBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        logPanel.setBorder(logBorder);
        logPanel.setLayout(new BorderLayout());
        logPanel.add(playBackFilePanel, BorderLayout.NORTH);
        logPanel.add(playBackPanel, BorderLayout.CENTER);
        if (loadFile) {
            logPanel.add(progressBar, BorderLayout.SOUTH);
        }

        if (loadFile) {
            Insets textAreaMargin = new Insets(2, 5, 2, 5);
            sanityLabel.setFont(new Font("Helvetica", Font.BOLD, 14));
            sanityTextArea.setMargin(textAreaMargin);
            sanityTextArea.setLineWrap(true);      // so that the text wil wrap
            sanityTextArea.setWrapStyleWord(true); // so that the wrapping will be based on each word
            sanityScrollPane.setPreferredSize(new Dimension(400, 600));
            sanityPanel.setLayout(new BorderLayout());
            sanityPanel.add(sanityLabel, BorderLayout.NORTH);
            sanityPanel.add(sanityScrollPane, BorderLayout.CENTER);

            leftPanel.setLayout(new BorderLayout());
            leftPanel.add(paintArea, BorderLayout.NORTH);
            leftPanel.add(sensorPanel, BorderLayout.CENTER);
            leftPanel.add(logPanel, BorderLayout.SOUTH);
            rightPanel.add(sanityPanel);
            
            this.getContentPane().setLayout(new BorderLayout());
            this.getContentPane().add(leftPanel, BorderLayout.CENTER);
            this.getContentPane().add(rightPanel, BorderLayout.EAST);
        } else {
            this.getContentPane().setLayout(new BorderLayout());
            this.getContentPane().add(paintArea, BorderLayout.NORTH);
            this.getContentPane().add(controlPanel, BorderLayout.CENTER);
            this.getContentPane().add(logPanel, BorderLayout.SOUTH);
        }

        pack();
        setResizable(true);
    }
    
    void initCPlaneLogVersion() {
        try {
            BufferedReader versionFile = new BufferedReader(new FileReader(CPLANE_VERSION_FILE));
            CPLANE_VERSION = versionFile.readLine();
        } catch (FileNotFoundException fnfe) {
            try {
                PrintWriter versionFile = new PrintWriter(new BufferedWriter(new FileWriter(CPLANE_VERSION_FILE)));
                versionFile.println(CPLANE_VERSION);
                versionFile.close();
                RoboUtility.printError(this, "File " + CPLANE_VERSION_FILE + " does not exists. Created.");
            } catch (IOException ioe2) {
                RoboUtility.printError(this, "Error in writing CPlane Version File");
                ioe2.printStackTrace();
            }
        } catch (IOException e) {
            RoboUtility.printError(this, "Error in reading CPlane Version File");
            e.printStackTrace();
        }
    }

    // draw a pixel given the colour in integer in the col and row specified for a specified width
    private void drawPixel(int colourInInt, int col, int row, int colWidth) {
        // if color is not defined
        boolean maybeCol = false;
        if ((colourInInt & RoboConstant.MAYBE_BIT) != 0) {
        	maybeCol = true;
        	colourInInt = colourInInt & ~RoboConstant.MAYBE_BIT;
        }
        if (colourInInt >= RoboConstant.classifyColours.length - 1) {
            colourInInt = RoboConstant.NO_COLOUR;
        }

        if (colourInInt == currentPlane || currentPlane == RoboConstant.NUM_PLANES) {
        	Color pixelCol = RoboConstant.classifyColours[colourInInt];
            try {
                offscreen.setColor(pixelCol);
                offscreen.fillRect(col*scale,row*scale,colWidth*scale,scale);
				if (maybeCol) {
					offscreen.setColor(Color.black);
					offscreen.drawRect(col*scale,row*scale,colWidth*scale-1,scale-1);
				}
            } catch (ArrayIndexOutOfBoundsException e) {
                System.out.println("colourInInt: " + colourInInt);
                e.printStackTrace();
                offscreen.setColor(RoboConstant.ERROR_COLOR);
                offscreen.fillRect(col*scale,row*scale,colWidth*scale,scale);
            }
        } else {
            offscreen.setColor(RoboConstant.ERROR_COLOR);
            offscreen.fillRect(col*scale,row*scale,colWidth*scale,scale);
        }
    }

    // draw the cplane given the data from the dog
    public void drawCPlaneOPlane(byte[] data, int compressedSize, int numObj) {
        if (cplaneCounterDebugMsg) {
            RoboUtility.printError(this, "received " + cplaneCounter + " cplanes.");
            cplaneCounter++;
        }
        if (!this.isVisible()) {
            setVisible(true);
        }
        int offset = 0;

        offset = drawCPlaneOnly(data, compressedSize, offset);
        offset = readSensors(data, offset);
        offset = readSanityValues(data, offset);
        readObjects(data, numObj, offset);

        paintArea.repaint();

        // Write to file the red and blue robots
        if (saveToFile) {
            try {
            
                PrintWriter writer = new PrintWriter(new FileWriter(ROBOT_FOLDER + 
                    File.separator + ROBOT_FILE_NAME + savedFileCount + ROBOT_FILE_EXT));
                
                int index = 0;
                for (int row = 0; row < RoboConstant.CPLANE_HEIGHT; row++) {
                    for (int col = 0; col < RoboConstant.CPLANE_WIDTH; col++) {
                        if (RoboConstant.classifyColours[RoboUtility.unsignedToInt(data[index])].equals(RoboConstant.RED_DOG_COLOR))
                        writer.print("r");
                    else if (RoboConstant.classifyColours[RoboUtility.unsignedToInt(data[index])].equals(RoboConstant.BLUE_DOG_COLOR))
                        writer.print("b");
                    else
                        writer.print(" ");
                
                    index++;
                }
                writer.println();
            }
	        
	    	writer.flush();
            writer.close();
	        savedFileCount++;
		
	    } catch (Exception ex) {
	        ex.printStackTrace();
	    }
	    saveToFile = false;
	}
    }
    // draw the cplane given the data, used in the playback
    public int drawCPlaneNoObjects(byte[] data, int compressedSize) {
        if (!isVisible()) {
            setVisible(true);
        }
        return drawCPlaneOnly(data, compressedSize, 0);
    }
    
    // draw the cplane without the log, 
    // this is separated from the log to avoid checking for every pixel
    private int drawCPlaneOnly(byte[] data, int compressedSize, int offset) {
        if (compressedSize>data.length) {
            RoboUtility.printError(this, "compressedSize: " + compressedSize + " greater than array length data.length: " + data.length);
            return compressedSize;
        }
        // go through all the pixels and draw them
        int col = 0;
        int row = 0;
        int pixelCount, pixelColour, difference;
        while (offset<compressedSize) {
            pixelCount  = RoboUtility.unsignedToInt(data[offset]);
            pixelColour = RoboUtility.unsignedToInt(data[offset+1]);
            if (compressedCPlaneDebugMsg) {
                RoboUtility.printError(this, "col: " + col + "; pixelCount: " + pixelCount);
            }
            // if a run is longer than the remainder of this row, fill this row
            // and jump to the next
            // keep doing it until (col+pixelCount)<RoboConstant.CPLANE_WIDTH
            while (offset<compressedSize && pixelCount>0 && (col+pixelCount)>=RoboConstant.CPLANE_WIDTH) {
                difference = RoboConstant.CPLANE_WIDTH - col;
                drawPixel(pixelColour, col, row, difference);
                col = 0;
                row++;
                pixelCount -= difference;
            }
            // draw the bar of pixel and increment 'col'
            if (offset<compressedSize && pixelCount>0) {
                drawPixel(pixelColour, col, row, pixelCount);
                col += pixelCount;
                if (col>=RoboConstant.CPLANE_WIDTH) {
                    RoboUtility.printError(this, "Abnormal. Should be filtered in the while loop");
                }
            } else if (offset>=compressedSize) {
                RoboUtility.printError(this, "offset greater than data array size: " + offset + "; compressedSize: " + compressedSize);
            } else if (pixelCount<0) {
                RoboUtility.printError(this, "negative pixelCount: " + pixelCount);
            }
            offset += 2;
        }
        return compressedSize;
    }
    
    // read the sensor readings
    private int readSensors(byte[] data, int offset) {
        return (offset + RoboConstant.SENSOR_SIZE);
    }
    
    private void displaySensorValues(long[] sensorData) {
        for (int i=0; i<sensorData.length; i++) {
            sensorTextField[i].setText(""+sensorData[i]);
        }
    }

    private void displaySanityValues(int[] sanityData) {
        String binaryString;
        int stringLength, valueOffset;
        boolean firedSanity = false;
        
        // clear the test area
        sanityTextArea.setText("");

        for (int i=0; i<sanityData.length; i++) {
            // convert to a string of binary numbers
            binaryString = RoboUtility.dec2Bin(sanityData[i]);
            stringLength = binaryString.length();

            if (toBinaryDebugMsg) {
                RoboUtility.printError(this, "sanityData[i]: " + sanityData[i] + "; binaryString: " + binaryString);
            }
            
            // add sanity to the sanity TextArea from the end of string
            valueOffset = i * RoboConstant.INT_BITS;
            for (int j=(stringLength-1); j>=0; j--) {
                if (binaryString.charAt(j)=='1') {
                    if (printSanityDebugMsg) {
                        RoboUtility.printError(this, "should update textarea with statement: " + SanityConstant.sanityDesc[stringLength-1-j+valueOffset]);
                    }
                    sanityTextArea.append(SanityConstant.sanityDesc[stringLength-1-j+valueOffset] + "\n\n");
                    firedSanity = true;
                }
            }
        }
        sanityTextArea.setCaretPosition(0); // set the textarea to display always the first line
        if (!firedSanity) {
            sanityTextArea.append("(No sanity checks get fired)");
        }
    }

    // read the sanity values
    private int readSanityValues(byte[] data, int offset) {
        return (offset + RoboConstant.SANITY_SIZE);
    }
    
    // read and draw the objects
    private int readObjects(byte[] data, int numObj, int offset) {
        int objectNum, x, y, w, h, d;
        double pan, tilt, asinAngle, roll, cosRoll, sinRoll;

        // read the pan and tilt first
        pan = RoboUtility.bytesToInt(data, offset) / RoboConstant.SEND_SCALE;
        offset += RoboConstant.INT_BYTES;
        tilt = RoboUtility.bytesToInt(data, offset) / RoboConstant.SEND_SCALE;
        offset += RoboConstant.INT_BYTES;
        
        // calculate the corresponding roll
        roll    = Math.asin(Math.sin(pan) * Math.sin(-tilt));
        cosRoll = Math.cos(roll);
        sinRoll = Math.sin(roll);

        // fix the angle when tilt is more than 90 degrees
        if (tilt>RoboConstant.HALF_PI_RADIAN) {
            if (roll < 0) {
                roll = - Math.PI - roll;
            } else {
                roll = Math.PI - roll;
            }
        }

        for(int i = 0; i < numObj; i++) {
            objectNum = RoboUtility.bytesToInt(data, offset);
            offset   += RoboConstant.INT_BYTES;
            x         = RoboUtility.bytesToInt(data, offset);
            offset   += RoboConstant.INT_BYTES;
            y         = RoboUtility.bytesToInt(data, offset);
            offset   += RoboConstant.INT_BYTES;
            w         = RoboUtility.bytesToInt(data, offset);
            offset   += RoboConstant.INT_BYTES;
            h         = RoboUtility.bytesToInt(data, offset);
            offset   += RoboConstant.INT_BYTES;
            d         = RoboUtility.bytesToInt(data, offset);
            offset   += RoboConstant.INT_BYTES;
            drawObject(objectNum, x, y, w, h, d, cosRoll, sinRoll);
        }
        return offset;
    }
    
    public void drawObject(OPlane oplane) {
        if (oplane.getRobotData().length!=RoboConstant.NUM_ROBOT_DATA) {
            RoboUtility.printError(this, "Robotdata in invalid length. robotData.length: " + oplane.getRobotData().length + " should be " + RoboConstant.NUM_ROBOT_DATA);
        }
        // calculate the corresponding roll
        double pan, tilt, roll;
        pan  = oplane.getRobotData(0);
        tilt = oplane.getRobotData(1);
        roll = Math.asin(Math.sin(pan) * Math.sin(-tilt));

        /* Get the horizon information. */
        /* Temp disabled because something still wrong with sending from
        ** client.. */
        if (false) {
            
            int hzX1, hzX2, hzY1, hzY2, hzOthers;
            hzX1 = (int) oplane.getRobotData(2);
            hzY1 = (int) oplane.getRobotData(3);
            hzX2 = (int) oplane.getRobotData(4);
            hzY2 = (int) oplane.getRobotData(5);
            hzOthers = (int) oplane.getRobotData(6);

            System.out.println("hz1: " + hzX1 + ", " + hzY1 + " hz2: " + hzX2 + 
                    ", " + hzY2 + " other: " + hzOthers);
        }


        // fix the angle when tilt is more than 90 degrees
        if (tilt>RoboConstant.HALF_PI_RADIAN) {
            if (roll < 0) {
                roll = - Math.PI - roll;
            } else {
                roll = Math.PI - roll;
            }
        }

        for (int i=0; i<oplane.getPlanes().length; i++) {
            if (oplane.getPlane(i).getPlaneData().length!=RoboConstant.NUM_INT_COORDS) {
                RoboUtility.printError(this, "Error in drawing objects. Data is in invalid length. planeData.length: " + oplane.getPlane(i).getPlaneData().length + " should be " + RoboConstant.NUM_INT_COORDS);
                return;
            }
            drawObject(oplane.getPlane(i).getPlaneData(0), oplane.getPlane(i).getPlaneData(1), 
                       oplane.getPlane(i).getPlaneData(2), oplane.getPlane(i).getPlaneData(3), 
                       oplane.getPlane(i).getPlaneData(4), oplane.getPlane(i).getPlaneData(5), 
                       Math.cos(roll), Math.sin(roll));
        }
    }
    public void drawObject(int objectNum, int x, int y, int w, int h, int d, double cosRoll, double sinRoll) {
        if (w == 0 && h == 0 && x == 0 && y == 0) {
            return;
        }
        
        if (objectCoordDebugMsg) {
            RoboUtility.printError(this, "" + objectNum + " " + x + " " + y + " " + w + " " + h + " " + d);
        }

        if (objectNum>INVALID_COORD) {
            RoboUtility.printError(this, "invalid objectNum: " + objectNum + " ");
            return;
        }
        if (x>INVALID_COORD) {
            RoboUtility.printError(this, "invalid x: " + x + " ");
            return;
        }
        if (y>INVALID_COORD) {
            RoboUtility.printError(this, "invalid y: " + y + " ");
            return;
        }
        if (w>INVALID_COORD) {
            RoboUtility.printError(this, "invalid w: " + w + " ");
            return;
        }
        if (h>INVALID_COORD) {
            RoboUtility.printError(this, "invalid h: " + h + " ");
            return;
        }
        if (d>INVALID_COORD) {
            RoboUtility.printError(this, "invalid d: " + d + " ");
            return;
        }

        offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);

        switch (objectNum) {
            case RoboConstant.BALL:
                drawBallBorder(objectNum, x, y, w, h, d);
                break;
            case RoboConstant.BLUE_GOAL:
            case RoboConstant.YELLOW_GOAL:
                drawGoalBorder(objectNum, x, y, w, h, d, cosRoll, sinRoll);
                break;
            case RoboConstant.BLUE_LEFT_BEACON:
            case RoboConstant.BLUE_RIGHT_BEACON:
            case RoboConstant.GREEN_LEFT_BEACON:
            case RoboConstant.GREEN_RIGHT_BEACON:
            case RoboConstant.YELLOW_LEFT_BEACON:
            case RoboConstant.YELLOW_RIGHT_BEACON:
                drawBeaconBorder(objectNum, x, y, w, h, d, cosRoll, sinRoll);
                break;
            case RoboConstant.RED_DOG:            
            case RoboConstant.RED_DOG2:
            case RoboConstant.RED_DOG3:
            case RoboConstant.RED_DOG4:
            case RoboConstant.BLUE_DOG:
            case RoboConstant.BLUE_DOG2:
            case RoboConstant.BLUE_DOG3:
            case RoboConstant.BLUE_DOG4:
                drawRobotBorder(objectNum, x, y, w, h, d, cosRoll, sinRoll);
                break;
        }
    }

    // draws the ball
    public void drawBallBorder(int objectNum, int x, int y, int w, int h, int d) {
        // draw object border
        offscreen.drawRect(x*scale, y*scale, w*scale, h*scale);

         // draw big cross to show center of obj
        offscreen.drawLine(x*scale, y*scale + h*halfScale, x*scale + w*scale, y*scale + h*halfScale);
        offscreen.drawLine(x*scale+w*halfScale , y*scale, x*scale + w*halfScale, y*scale + h*scale);

        // draw the object number at the top left and bottom left corner
        if (y*scale<TOP_GAP) {
            offscreen.drawString(RoboConstant.objectNames[objectNum] + "", x*scale<0? 0:x*scale, TOP_GAP);
        } else {
            offscreen.drawString(RoboConstant.objectNames[objectNum] + "", x*scale<0? 0:x*scale, y*scale);
        }

        // draw distance at center of object
        offscreen.drawString(d + "", x*scale+w*halfScale, y*scale + h*halfScale);
    }
    
    // draws the goal
    public void drawGoalBorder(int objectNum, int x, int y, int w, int h, int d, double cosRoll, double sinRoll) {
        int[] xArray = {x, x+w, x+w, x};
        int[] yArray = {y, y, y+h, y+h};
        double dx, dy;
        double xCenter, yCenter; // center coordinate of shape
        double newX, newY;

        dx      = RoboConstant.CPLANE_WIDTH / 2.0;
        dy      = RoboConstant.CPLANE_HEIGHT / 2.0;
        xCenter = 0;
        yCenter = 0;

        for (int j=0; j<xArray.length; j++) {
            newX = xArray[j] * cosRoll - yArray[j] * sinRoll - dx*cosRoll + dy*sinRoll + dx;
            newY = xArray[j] * sinRoll + yArray[j] * cosRoll - dx*sinRoll - dy*cosRoll + dy;

            xArray[j] = (int) (newX * scale);
            yArray[j] = (int) (newY * scale);

            xCenter += xArray[j];
            yCenter += yArray[j];
        }

        // draw object border
        offscreen.drawPolygon(xArray, yArray, xArray.length);

        // draw big cross to show center of obj
        offscreen.drawLine((xArray[0]+xArray[1])/2, (yArray[0]+yArray[1])/2, (xArray[2] + xArray[3])/2, (yArray[2]+yArray[3])/2);
        offscreen.drawLine((xArray[0]+xArray[3])/2, (yArray[0]+yArray[3])/2, (xArray[1]+xArray[2])/2, (yArray[1]+yArray[2])/2);

        // draw the object number at each of the corner
        if (yArray[0]<TOP_GAP) {
            offscreen.drawString(RoboConstant.objectNames[objectNum] + "", xArray[0]<0? 0:xArray[0], TOP_GAP);
        } else {
            offscreen.drawString(RoboConstant.objectNames[objectNum] + "", xArray[0]<0? 0:xArray[0], yArray[0]);
        }

        // draw distance at center of object
        offscreen.drawString(d + "", (int)(xCenter/(double)(xArray.length)), (int)(yCenter/(double)(yArray.length)));
    }

    public void drawBeaconBorder(int objectNum, int x, int y, int w, int h, int d, double cosRoll, double sinRoll) {
        drawBallBorder(objectNum, x, y, w, h, d);
    }

    // draws in the bottom and top half colours of a beacon
    public void drawRobotBorder(int objectNum, int x, int y, int w, int h, int d, double cosRoll, double sinRoll) {

        // x,y is now original values without rotation, so don't need to
        // unrotate them. I don't think robot needs to be displayed with
        // rotation+unrotation like goals.

        //drawGoalBorder(objectNum, x, y, w, h, d, cosRoll, sinRoll);

        drawBallBorder(objectNum, x, y, w, h, d);
    }
    
    private void loadFile() {
        String line = "";
        try {
            BufferedReader file = new BufferedReader(new FileReader(playBackFileField.getText()));
            String[] array;
            int i, j, compressedSize, sensorDataCounter, sanityDataCounter, robotDataCounter;
            int[] planeData, sanityData;
            long[] sensorData;
            byte[] cplane;
            Vector cplaneVector = new Vector(); // to store the number of cplanes, will copy into the variable cplaneArray
            Vector oplaneVector = new Vector(); // to store the unknown number of oplanes
            double[] robotData;
            boolean completeLine = true; // true when the line is complete. Usually incomplete line occurs at the last line

            int lineCounter = 1;

            // read the version
            line = file.readLine();
            if (line!=null && !line.equals(CPLANE_VERSION)) {
                RoboUtility.printError(this, "Incorrect Version Error. Log file version: " + line + ". Needed: " + CPLANE_VERSION);
            }
            
            loadButton.setEnabled(false);

            if (loadAgainDebugMsg) {
                System.out.println("Loading file " + playBackFileField.getText());
            }
            while (completeLine) {
                line = file.readLine();
                if (line==null) {
                    break;
                }
                loadingFile(lineCounter++);
                if (loadAgainDebugMsg) {
                    if (lineCounter%5==4) {
                        System.out.print(lineCounter + " ");
                        if (lineCounter%100==99) {
                            System.out.println();
                        }
                    }
                }
                array = line.split(" ");

                // read the size of the compressed cplane
                compressedSize = Integer.parseInt(array[0]);
                cplane         = new byte[compressedSize];

                if (compressedSize>RoboConstant.COMPRESSED_CPLANE_SIZE) {
                    RoboUtility.printError(this, "compressedSize (" + compressedSize + ") greater than RoboConstant.COMPRESSED_CPLANE_SIZE (" + RoboConstant.COMPRESSED_CPLANE_SIZE + ")");
                }

                // read the cplane
                for (i=1; i<=compressedSize; i++) {
                    if (i>=array.length) {
                        completeLine = false;
                        break;
                    }
                    try {
                        cplane[i-1] = Byte.parseByte(array[i]);
                    } catch (NumberFormatException nfe) {
                        cplane[i-1] = (byte) (Integer.parseInt(array[i]) + (Byte.MIN_VALUE << 1));
                        if (unsignedByteDebugMsg) {
                            RoboUtility.printError(this, array[i] + " changed to " + cplane[i-1]);
                        }
                    }
                }
                
                if (!completeLine) {
                    break;
                }
                
                // read the sensor values
                sensorDataCounter = 0;
                sensorData        = new long[RoboConstant.NUM_SENSOR_COORDS];
                while (completeLine && sensorDataCounter<sensorData.length) {
                    if ((i+sensorDataCounter)>=array.length) {
                        completeLine = false;
                        break;
                    }
                    sensorData[sensorDataCounter] = Long.parseLong(array[i+sensorDataCounter]);
                    sensorDataCounter++;
                }
                i += sensorData.length;
                
                if (!completeLine) {
                    break;
                }

                // read the sanity values
                sanityDataCounter = 0;
                sanityData        = new int[RoboConstant.NUM_SANITY_COORDS];
                while (sanityDataCounter<sanityData.length) {
                    if ((i+sanityDataCounter)>=array.length) {
                        completeLine = false;
                        break;
                    }
                    sanityData[sanityDataCounter] = Integer.parseInt(array[i+sanityDataCounter]);
                    sanityDataCounter++;
                }
                i += sanityData.length;
                
                if (!completeLine) {
                    break;
                }

                // read the robotData (ie. pan+tilt)
                robotDataCounter = 0;
                robotData        = new double[RoboConstant.NUM_ROBOT_DATA];
                while (robotDataCounter<robotData.length) {
                    if ((i+robotDataCounter)>=array.length) {
                        completeLine = false;
                        break;
                    }
                    robotData[robotDataCounter] = Double.parseDouble(array[i+robotDataCounter]);
                    robotDataCounter++;
                }
                i += robotData.length;

                if (!completeLine) {
                    break;
                }

                // read the objects
                boolean GPS = false;
                while (i<array.length && completeLine && !GPS) {
                    planeData = new int[RoboConstant.NUM_INT_COORDS];
                    for (j=0; j<planeData.length; j++) {
                        if ((i+j)>=array.length) {
                            completeLine = false;
                            break;
                        }
                        if (array[i+j].equals(new String("GPS"))) {
                            GPS = true;
                            break;
                        }
                        planeData[j] = Integer.parseInt(array[i+j]);
                    }
                    if (!completeLine) {
                        break;
                    }
                    oplaneVector.addElement(new Plane(planeData));
                    i += j;
                }
                if (!completeLine) {
                    break;
                }

                cplaneVector.addElement(new CPlane(cplane, compressedSize, sensorData, sanityData, oplaneVector, robotData));

                // reset variables
                oplaneVector.clear();
            }

            if (loadAgainDebugMsg) {
                System.out.println("Finishing loading");
            }
            cplaneArray = new CPlane[cplaneVector.size()];
            cplaneVector.copyInto(cplaneArray);

            cplaneIndex = 0; // always start with the first frame
            paintScreen();

            totalFrame.setText(" / " + cplaneArray.length);
            if (cplaneArray.length>0) {
                playButton.setEnabled(true);
            }
            
            if (oplaneMissingDebugMsg) {
                System.out.println("planes in 0: " + cplaneArray[0].oplane.planes.length);
                for (i=0; i<cplaneArray[0].oplane.planes.length; i++) {
                    System.out.print(cplaneArray[0].oplane);
                }
            }

            // finish loading file
            loadingFile(FILE_LOAD_FINISH);
        } catch (IOException e2) {
            JOptionPane.showMessageDialog(this, "Error: IOException caught.\nSpecified file cannot be opened", "Error", JOptionPane.ERROR_MESSAGE);
            gotoStartButton.setEnabled(false);
            backwardButton.setEnabled(false);
            forwardButton.setEnabled(false);
            gotoLastButton.setEnabled(false);
            playButton.setEnabled(false);
            stopButton.setEnabled(false);
            currentFrame.setEditable(false);
            playBackFileField.requestFocus();
            e2.printStackTrace();
            return;
        } catch (Exception e3) {
            JOptionPane.showMessageDialog(this, "Error: Specified file is not in the correct format", "Error", JOptionPane.ERROR_MESSAGE);
            gotoStartButton.setEnabled(false);
            backwardButton.setEnabled(false);
            forwardButton.setEnabled(false);
            gotoLastButton.setEnabled(false);
            playButton.setEnabled(false);
            stopButton.setEnabled(false);
            currentFrame.setEditable(false);
            playBackFileField.requestFocus();
            System.out.println("Line Error: " + line);
            e3.printStackTrace();
            return;
        }
    }

    private void animationTimerAction() {
        cplaneIndex++;
        paintScreen();
        if (cplaneIndex==(cplaneArray.length-1)) {
            animationTimer.stop();
            JOptionPane.showMessageDialog(this, "End of animation", "Information", JOptionPane.INFORMATION_MESSAGE);
            chooseButton.setEnabled(true);
            loadButton.setEnabled(true);
            gotoStartButton.setEnabled(true);
            backwardButton.setEnabled(true);
            currentFrame.setEditable(true);
        }
    }

    // paint the frame specified with cplaneIndex on the screen
    private void paintScreen() {
        if (cplaneArray!=null && cplaneIndex>=0 && cplaneIndex<cplaneArray.length) {
            // display the cplane
            drawCPlaneNoObjects(cplaneArray[cplaneIndex].pixels, cplaneArray[cplaneIndex].compressedSize);
            
            // display the sensor values
            displaySensorValues(cplaneArray[cplaneIndex].sensorData);
            
            // display the sanity values
            displaySanityValues(cplaneArray[cplaneIndex].sanityData);
            
            // display the objects
            drawObject(cplaneArray[cplaneIndex].oplane);
            currentFrame.setText("" + (cplaneIndex+1)); // counter starts with 1

            if (cplaneIndex==0) {
                gotoStartButton.setEnabled(false);
                backwardButton.setEnabled(false);
            } else if (!animationTimer.isRunning()) {
                gotoStartButton.setEnabled(true);
                backwardButton.setEnabled(true);
                currentFrame.setEditable(true);
            }

            if (cplaneIndex==(cplaneArray.length-1)) {
                forwardButton.setEnabled(false);
                gotoLastButton.setEnabled(false);
                playButton.setEnabled(false);
                stopButton.setEnabled(false);
            } else if (!animationTimer.isRunning()) {
                forwardButton.setEnabled(true);
                gotoLastButton.setEnabled(true);
                playButton.setEnabled(true); // don't set the button active if it is running
                currentFrame.setEditable(true);
            }
            
            paintArea.repaint();
        } else {
            JOptionPane.showMessageDialog(this, 
            "Error in displaying the cplaneArray.\nEither the file loading process is not completed successfully\nor an invalid frame is specified", 
            "Error", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    private void loadingFile(int n) {
        if (loadAgainDebugMsg) {
            System.out.print("n: " + n + "; FILE_LOAD_FINISH: " + FILE_LOAD_FINISH + " n==FILE_LOAD_FINISH: " + (n==FILE_LOAD_FINISH));
        }
        if (n==FILE_LOAD_FINISH) {
            loadButton.setEnabled(true);
            if (progressBar.isIndeterminate()) {
                progressBar.setIndeterminate(false);
            }
            progressBar.setString("File loaded successfully");
            if (loadAgainDebugMsg) {
                System.out.println(" Error ");
            }
        } else {
            progressBar.setIndeterminate(true);
            progressBar.setString("Loading file... " + n + " frames");
            if (loadAgainDebugMsg) {
                System.out.println(" OK ");
            }
        }
    }
    
    public JTextField getPlayBackFileField() {
        return playBackFileField;
    }
    public JButton getLoadButton() {
        return loadButton;
    }
    
    /**
     * read in the cplane from client.c
    **/
    private class Receiver extends Thread {
        // read cplane info from socket
        public void run() {
            try {
                
            	
            	while(true) {
            		listenForConnection();
            	
            		System.out.println("listenForConnection returned");
            	}
            	
            	
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    
        void listenForConnection() throws IOException, InterruptedException {
        	
        	int read;    // total read
            int numRead; // read this time
            int offset, dataTotalSize, backupCPlaneSize, backupObjectSize;

            // accept a client
            ServerSocket ss = new ServerSocket(port);
            System.out.println("CPlaneDisplay (receive," + port + "): ready");
            Socket sock = ss.accept();
            InputStream in = sock.getInputStream();
            ss.close();

            byte delayTestingCounterArray[] = new byte[RoboConstant.INT_BYTES];
            byte[] sizeBuffer = new byte[RoboConstant.INT_BYTES];
            byte[] localBuff = new byte[RoboConstant.COMPRESSED_CPLANE_SIZE + RoboConstant.SENSOR_SIZE + RoboConstant.SANITY_SIZE + RoboConstant.OBJ_SIZE];

            byte temp;    // for swapping byte

            boolean dropPacket;
            
            if (debugMsg) {
                System.out.println("Receiver started()");
            }
            if(! drawer.isAlive()) {
            	drawer.start();
            }
            
            while(true) {
                dropPacket = false;
                
                if (!canDrawBuffer1) {
                    readInBuffer1 = true;
                    localBuff     = buffer1;
                } else if (!canDrawBuffer2) {
                    readInBuffer2 = true;
                    localBuff     = buffer2;
                } else {
                    dropPacket = true;
                }
                
                if (debugMsg) {
                    if (!dropPacket) {
                        if (readInBuffer1) {
                            System.out.println("reading in buffer 1");
                        } else if (readInBuffer2) {
                            System.out.println("reading in buffer 2");
                        }
                    }
                }
                
                offset = 0;

                // read the size of compressed cplane
                read = 0;
                while (read != sizeBuffer.length) {
                    try {
                        numRead = in.read(sizeBuffer, read, sizeBuffer.length-read);
                        if (numRead <= 0) {
                            
                        	System.out.println("read no bytes");
                        	sock.close();
                        	drawer.sleep(1000);
                        	ss.close();
                            return;
                        }
                        read += numRead;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        RoboUtility.printError(this, "error reading size of compressed cplane.");
                        e.printStackTrace();
                        System.exit(1);
                    }
                }
                if (readInBuffer1) {
                    compressedCPlaneSize1 = RoboUtility.bytesToInt(sizeBuffer, 0);
                    backupCPlaneSize      = compressedCPlaneSize1;
                } else if (readInBuffer2) {
                    compressedCPlaneSize2 = RoboUtility.bytesToInt(sizeBuffer, 0);
                    backupCPlaneSize      = compressedCPlaneSize2;
                } else {
                    // just to read it, have no effect to the cplane
                    backupCPlaneSize = RoboUtility.bytesToInt(sizeBuffer, 0);

                    // initialise localBuff
                    localBuff  = new byte[backupCPlaneSize + RoboConstant.SENSOR_SIZE + RoboConstant.SANITY_SIZE + RoboConstant.OBJ_SIZE];
                }
                
                if (compressedCPlaneDebugMsg) {
                    RoboUtility.printError(this, "compressed CPlane size " + backupCPlaneSize);
                }
                
                // read the cplane
                read = 0;
                while (read != backupCPlaneSize) {
                    try {
                        /* Read from the socket input stream into the
                        ** localBuff. */
                        numRead = in.read(localBuff, read, backupCPlaneSize-read);
                        if (numRead <= 0) {
                            
                        	System.out.println("read no bytes (print statement 2)");
                        	sock.close();
                        	drawer.sleep(1000);
                        	ss.close();
                            return;
                        }
                        read += numRead;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        RoboUtility.printError(this, "error reading cplane.");
                        e.printStackTrace();
                        System.exit(1);
                    }
                }
                offset += backupCPlaneSize;
                
                // read the sensor readings
                read = 0;
                while (read != RoboConstant.SENSOR_SIZE) {
                    try {
                        numRead = in.read(localBuff, offset, RoboConstant.SENSOR_SIZE-read);
                        if (numRead <= 0) {
                        	System.out.println("read no bytes (print statement 3)");
                        	sock.close();
                        	drawer.sleep(1000);
                        	ss.close();
                            return;
                        }
                        read += numRead;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        RoboUtility.printError(this, "error reading sensor readings.");
                        e.printStackTrace();
                        System.exit(1);
                    }
                }
                offset += RoboConstant.SENSOR_SIZE;

                // read the sanity values
                read = 0;
                while (read != RoboConstant.SANITY_SIZE) {
                    try {
                        numRead = in.read(localBuff, offset, RoboConstant.SANITY_SIZE-read);
                        if (numRead <= 0) {
                        	System.out.println("read no bytes (print statement 4)");
                        	sock.close();
                        	drawer.sleep(1000);
                        	ss.close();
                            return;
                        }
                        read += numRead;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        RoboUtility.printError(this, "error reading sanity values.");
                        e.printStackTrace();
                        System.exit(1);
                    }
                }
                offset += RoboConstant.SANITY_SIZE;

                // read the number of object
                read = 0;
                while (read != sizeBuffer.length) {
                    try {
                        numRead = in.read(sizeBuffer, read, sizeBuffer.length-read);

                        if (optimizeDebugMsg) {
                            System.out.println("Counter read: " + read);
                        }
                        if (numRead <= 0) {
                        	System.out.println("read no bytes (print statement 5)");
                        	sock.close();
                        	drawer.sleep(1000);
                        	ss.close();
                            return;
                        }
                        read += numRead;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        RoboUtility.printError(this, "error reading number of object.");
                        e.printStackTrace();
                        System.exit(1);
                    }
                }
                if (readInBuffer1) {
                    dataBufferSize1  = RoboUtility.bytesToInt(sizeBuffer, 0);
                    backupObjectSize = dataBufferSize1;
                    if (optimizeDebugMsg || oplaneMissingDebugMsg) {
                        System.out.println("dataBufferSize1: " + dataBufferSize1 + "; dataTotalSize: " + dataTotalSize);
                    }
                } else if (readInBuffer2) {
                    dataBufferSize2  = RoboUtility.bytesToInt(sizeBuffer, 0);
                    backupObjectSize = dataBufferSize2;
                    if (optimizeDebugMsg || oplaneMissingDebugMsg) {
                        System.out.println("dataBufferSize2: " + dataBufferSize2 + "; dataTotalSize: " + dataTotalSize);
                    }
                } else {
                    // use temporary variable to store the data object size but this won't get drawn
                    backupObjectSize = RoboUtility.bytesToInt(sizeBuffer, 0);
                }
                dataTotalSize = RoboConstant.ONE_OBJ_SIZE * backupObjectSize;
                // add spaces for the pan and tilt (the doubles are stored as INT_BYTES)
                dataTotalSize += RoboConstant.INT_BYTES * RoboConstant.NUM_ROBOT_DATA;

                // read the object data
                read = 0;
                while (read != dataTotalSize) {
                    try {
                        numRead = in.read(localBuff, offset, dataTotalSize-read);

                        if (optimizeDebugMsg) {
                            System.out.println("Object read: " + read);
                        }
                        if (numRead <= 0) {
                        	System.out.println("read no bytes (print statement 6)");
                        	sock.close();
                        	drawer.sleep(1000);
                        	ss.close();
                            return;
                        }
                        read += numRead;
                    } catch (ArrayIndexOutOfBoundsException e) {
                        RoboUtility.printError(this, "error reading the object data.");
                        e.printStackTrace();
                        System.exit(1);
                    }
                }
                offset += dataTotalSize;

                if (debugMsg) {
                    if (!dropPacket) {
                        if (readInBuffer1) {
                            System.out.println("reading in buffer 1 **");
                        } else if (readInBuffer2) {
                            System.out.println("reading in buffer 2 **");
                        }
                    }
                }

                if (dropPacket && debugMsg) {
                    System.out.println("Packet dropped");
                } else if (readInBuffer1) {
                    readInBuffer1  = false;
                    canDrawBuffer1 = true;
                } else if (readInBuffer2) {
                    readInBuffer2  = false;
                    canDrawBuffer2 = true;
                }
                
                if (isDelayTesting) {
                    // read the delayTestingCounter
                    read = 0;
                    while (read != RoboConstant.INT_BYTES) {
                        numRead = in.read(delayTestingCounterArray, read, delayTestingCounterArray.length-read);
                        read += numRead;
                    }
                    
                    if (!dropPacket) {
                        // reverse the array since C stores the most significant number first while Java is the other way around
                        for (int i=0; i<(RoboConstant.INT_BYTES/2); i++) {
                            temp                        = delayTestingCounterArray[i];
                            delayTestingCounterArray[i] = delayTestingCounterArray[RoboConstant.INT_BYTES-i-1];
                            delayTestingCounterArray[RoboConstant.INT_BYTES-i-1] = temp;
                        }
                        RoboUtility.printError(this, "---------- delayTestingCounter: " + RoboUtility.bytesToInt(delayTestingCounterArray, 0) + " ----------");
                    }
                }
                
            }
        }
    }

    
    
    // draw the cplane from buffer
    private class Drawer extends Thread {
        public void run() {
            if (debugMsg) {
                System.out.println("Drawer started()");
            }
            while (true) {
                if (!readInBuffer1 && canDrawBuffer1) {
                    if (debugMsg) {
                        System.out.println("----- drawing buffer 1");
                    }
                    drawCPlaneOPlane(buffer1, compressedCPlaneSize1, dataBufferSize1);
                    canDrawBuffer1 = false;
                    if (debugMsg) {
                        System.out.println("----- drawing buffer 1 **");
                    }
                } else if (!readInBuffer2 && canDrawBuffer2) {
                    if (debugMsg) {
                        System.out.println("----- drawing buffer 2");
                    }
                    drawCPlaneOPlane(buffer2, compressedCPlaneSize2, dataBufferSize2);
                    canDrawBuffer2 = false;
                    if (debugMsg) {
                        System.out.println("----- drawing buffer 2 **");
                    }
                }
            }
        }
    }

    // panel to handle the drawing
    private class PaintArea extends JPanel {
        PaintArea() {
            super(false);
        }

        public void repaint(Graphics g) {
            paint(g);
        }

        public void paint(Graphics g) {
            // draw onto the online image
            g.drawImage(pic, 0, 0, this);
        }
    }


    // represent the cplane (colour plane + objects bounding box)
    private class CPlane {
        int compressedSize;
        byte[] pixels;
        long[] sensorData;
        int[] sanityData;
        OPlane oplane;
        
        CPlane(byte[] pixels, int compressedSize, long[] sensorData, int[] sanityData, Vector v, double[] robotData) {
            this.pixels         = pixels;
            this.compressedSize = compressedSize;
            this.sensorData     = sensorData;
            this.sanityData     = sanityData;
            this.oplane         = new OPlane(v, robotData);
        }
        
    }

    // represent the oplane (an array of planes)
    private class OPlane {
        Plane[] planes;

        // the data of the robot (pan, tilt)
        double[] robotData = new double[RoboConstant.NUM_ROBOT_DATA]; 

        OPlane(Vector v, double[] robotData) {
            this.planes = new Plane[v.size()];
            v.copyInto(this.planes);
            this.robotData = robotData;
        }

        public String toString() {
            String s = "";
            int i;
            for (i=0; i<robotData.length; i++) {
                s += robotData[i] + " ";
            }
            s += "\n";
            for (i=0; i<planes.length; i++) {
                s += i + ": " + planes[i].toString() + "\n";
            }
            return s;
        }
        
        public Plane[] getPlanes() {
            return planes;
        }
        public Plane getPlane(int i) {
            return planes[i];
        }
        public double[] getRobotData() {
            return robotData;
        }
        public double getRobotData(int i) {
            return robotData[i];
        }
    }

    private class Plane {
        int[] planeData;

        Plane(int[] planeData) {
            this.planeData = new int[RoboConstant.NUM_INT_COORDS];
            for (int i=0; i<planeData.length; i++) {
                this.planeData[i] = planeData[i];
            }
        }

        public String toString() {
            String s = "";
            for (int i=0; i<this.planeData.length; i++) {
                s += this.planeData[i] + " ";
            }
            return s;
        }
        
        public int[] getPlaneData() {
            return planeData;
        }
        public int getPlaneData(int i) {
            return planeData[i];
        }
    }

    public static void main(String[] args) {
        CPlaneDisplay c = null;
        if (args.length == 0)
            c = new CPlaneDisplay(null, 5010);
        else if (args.length == 1)
            c = new CPlaneDisplay(args[0]);
        else {
            RoboUtility.printError("CPlaneDisplay.java", "Usage java CPlaneDisplay logfile");
            System.exit(1);
        }

        c.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
    }
}
