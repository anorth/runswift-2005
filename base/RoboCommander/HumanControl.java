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
 * UNSW 2003 Robocup (Alex Tang)
 *
 * Last modification background information
 * $Id: HumanControl.java 2090 2003-11-01 16:44:49Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * An independent java processor showing the oplane, with the ability 
 * to send commands and control the dog
 * to log the oplane and commands
 * to allow the play back of log file
 * (part of the code are borrowed from OPlaneDisplay.java and RoboCommander.java)
 *
**/

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.MouseInputAdapter;
import java.net.*;
import java.util.*;
import java.text.DecimalFormat;
import java.util.StringTokenizer;

public class HumanControl extends JFrame {

    private static final boolean debugMsg           = false;
    private static final boolean bufferDebugMsg     = false;
    private static final boolean mouseDebugMsg      = false;
    private static final boolean joyDebugMsg        = false;
    private static final boolean LATENCY_TEST1      = false;
    private static final boolean newOplaneDebugMsg  = false;
    private static int trial1                       = 0;

    public static final int OPLANE_LOG_INTERVAL     = 1;

    public static final String DEFAULT_PLAYER       = "1";
    public static final String END_OPLANE           = "*";
    public static final String START_PAN_TILT       = "#";

    // number of arguments for the command in the log file
    public static final int COMMAND_LEN             = 3;
    
    // number of arguments for the plane in the log file
    public static final int LOG_FILE_PLANE_LEN      = RoboConstant.NUM_INT_COORDS;

    public static final int SCALE                   = 1;
    public static final int FRAME_WIDTH             = RoboConstant.CPLANE_WIDTH * SCALE;
    public static final int FRAME_HEIGHT            = RoboConstant.CPLANE_HEIGHT * SCALE;
    public static final int IMAGE_WIDTH             = (int)((RoboConstant.LEFT_PAN_DEGREE+RoboConstant.RIGHT_PAN_DEGREE)*FRAME_WIDTH/(double)RoboConstant.CAMERA_PAN);
    public static final int IMAGE_HEIGHT            = (int)((RoboConstant.UP_TILT_DEGREE+RoboConstant.DOWN_TILT_DEGREE)*FRAME_HEIGHT/(double)RoboConstant.CAMERA_TILT);
    public static final int WINDOW_WIDTH            = IMAGE_WIDTH;
    public static final int WINDOW_HEIGHT           = IMAGE_HEIGHT;
    
    // the number of commands skipped for moving the head
    public static final int MOVE_HEAD_INTERVAL      = 3;

    public static final String TILT_PAN_COMMAND        = "k";
    public static final String PAN_TILT_COMMAND        = "u";
    public static final String JOYSTICK_BUTTON_COMMAND = "i";
    public static final String JOYSTICK_MOTION_COMMAND = "j";
    public static final String BALL_TRACK_COMMAND      = "q";

    // receiving of the oplane
    byte[] buffer1 = new byte[RoboConstant.OBJ_SIZE];
    byte[] buffer2 = new byte[RoboConstant.OBJ_SIZE];
    
    int dataBufferSize1 = 0;
    int dataBufferSize2 = 0;

    boolean readInBuffer1, readInBuffer2, canDrawBuffer1, canDrawBuffer2;
    
    OPlaneDrawer oplaneDrawer = new OPlaneDrawer();

    // the format used to display the pan, tilt and roll
    DecimalFormat displayDouble = new DecimalFormat("0.0");

    BufferedImage pic      = new BufferedImage(IMAGE_WIDTH, IMAGE_HEIGHT, BufferedImage.TYPE_INT_ARGB);
    Graphics offscreen     = pic.getGraphics();
    JPanel paintArea       = new PaintArea();
    
    JPanel viewPanel       = new JPanel();
    JPanel userPanel       = new JPanel();
    JPanel userBottomPanel = new JPanel();

    JScrollPane scrollPane = new JScrollPane(paintArea);

    // GUI for the command panel
    JPanel commandPanel             = new JPanel();
    JButton worldModelButton        = new JButton("World Model");
    JLabel nameLabel                = new JLabel("Name:");
    JLabel valueLabel               = new JLabel("Value:");
    JTextField nameField            = new JTextField(RoboConstant.MODE_TRIGGER);
    JTextField valueField           = new JTextField("0");
    JButton sendButton              = new JButton("Send");
    JButton motionButton            = new JButton("Motion");
    TitledBorder commandBorder      = new TitledBorder("Command Panel");


    // GUI for the playback log
    JPanel playlogPanel             = new JPanel();
    TitledBorder playlogBorder      = new TitledBorder("Play Back Panel");
    
    // GUI for the play back file
    JPanel playBackFilePanel        = new JPanel();
    JLabel playBackFileLabel        = new JLabel("Playback Log:");
    JTextField playBackFileField    = new JTextField(RoboConstant.DEFAULT_OPLANE_LOG_FILE, 20);
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
    
    
    // GUI for the settings panel
    JPanel settingsPanel            = new JPanel();

    // GUI for the robot settings
    JPanel robotSettingsPanel       = new JPanel();
    JLabel panLabel                 = new JLabel("Pan");
    JTextField panField             = new JTextField(5);
    JLabel tiltLabel                = new JLabel("Tilt");
    JTextField tiltField            = new JTextField(5);
    JLabel erollLabel               = new JLabel("Effective Roll");
    JTextField erollField           = new JTextField(5);

    // GUI for the oplane display
    JPanel displaySettingsPanel     = new JPanel();
    JCheckBox distanceCheckBox      = new JCheckBox("Show Distance", true);
    JCheckBox horizonCheckBox       = new JCheckBox("Show Horizon", true);
    JCheckBox robotRelativeCheckBox = new JCheckBox("Show object in Robot Relative", true);
    JCheckBox tiltFirstCheckBox     = new JCheckBox("Tilt first", true);
    TitledBorder settingsBorder     = new TitledBorder("Settings Panel");

    // GUI for the log
    PrintWriter oplaneLogFile;
    PrintWriter oplaneLogFile2;
    PrintWriter oplaneLogFile3;
    boolean startOPlaneLog = false;

    TitledBorder logBorder          = new TitledBorder("Log Panel");
    JPanel logPanel                 = new JPanel();
    JPanel oplaneLogPanel           = new JPanel();
    JCheckBox oplaneLogFileCheckBox = new JCheckBox("Log Oplane?", false);
    JCheckBox pauseLogFileCheckBox  = new JCheckBox("Pause Log?", false);
    JLabel oplaneLogFileLabel       = new JLabel("Log:");
    JTextField oplaneLogFileField   = new JTextField(RoboConstant.DEFAULT_OPLANE_LOG_FILE, 20);
    JButton oplaneChooseButton      = new JButton("Browse...");

    // for oplane log playback
    javax.swing.Timer animationTimer;
    OPlane[] oplane = null;
    int oplaneIndex;

    // stores forward, left, turn, tilt, pan values
    float speeds[] = {0,0,0,0,0};

    RoboWirelessBase parent;
    int oplanePort;
    PrintWriter playBackFile;
    
    int oplaneCounter = 0;
    
    // to remember some variables to save computation
    // in radian in drawing oplane
    double lastPan      = 0;
    double lastTilt     = 0;
    double roll         = 0;
    double pos_cos_roll = 0;
    double pos_sin_roll = 0;
    double neg_cos_roll = 0;
    double neg_sin_roll = 0;
    
    // remember the pan and tilt that get sent last to reduce redundancy
    int lastPanSent   = 0;
    int lastTiltSent  = 0;

    // the top left point for drawing the oplane
    int topLeftX      = pan2TopLeft(lastPan);
    int topLeftY      = tilt2TopLeft(lastTilt);
    
    int moveHeadCounter = 0;
    
    boolean mouseDown    = false;
    boolean mouseDragged = false;

    boolean ballTrk = false;

    boolean fileLocked = false;

    // variables to be used for predicting actions
    String object00_num = "?";
    String object00_x = "?";
    String object00_y = "?";
    String object00_w = "?";
    String object00_h = "?";
    String object00_d = "?";
    String object01_num = "?";
    String object01_x = "?";
    String object01_y = "?";
    String object01_w = "?";
    String object01_h = "?";
    String object01_d = "?";
    String object02_num = "?";
    String object02_x = "?";
    String object02_y = "?";
    String object02_w = "?";
    String object02_h = "?";
    String object02_d = "?";
    String object03_num = "?";
    String object03_x = "?";
    String object03_y = "?";
    String object03_w = "?";
    String object03_h = "?";
    String object03_d = "?";
    String object04_num = "?";
    String object04_x = "?";
    String object04_y = "?";
    String object04_w = "?";
    String object04_h = "?";
    String object04_d = "?";
    String object05_num = "?";
    String object05_x = "?";
    String object05_y = "?";
    String object05_w = "?";
    String object05_h = "?";
    String object05_d = "?";
    String object06_num = "?";
    String object06_x = "?";
    String object06_y = "?";
    String object06_w = "?";
    String object06_h = "?";
    String object06_d = "?";
    String object07_num = "?";
    String object07_x = "?";
    String object07_y = "?";
    String object07_w = "?";
    String object07_h = "?";
    String object07_d = "?";
    String object08_num = "?";
    String object08_x = "?";
    String object08_y = "?";
    String object08_w = "?";
    String object08_h = "?";
    String object08_d = "?";
    String object09_num = "?";
    String object09_x = "?";
    String object09_y = "?";
    String object09_w = "?";
    String object09_h = "?";
    String object09_d = "?";
    String object10_num = "?";
    String object10_x = "?";
    String object10_y = "?";
    String object10_w = "?";
    String object10_h = "?";
    String object10_d = "?";
    String object11_num = "?";
    String object11_x = "?";
    String object11_y = "?";
    String object11_w = "?";
    String object11_h = "?";
    String object11_d = "?";
    String object12_num = "?";
    String object12_x = "?";
    String object12_y = "?";
    String object12_w = "?";
    String object12_h = "?";
    String object12_d = "?";
    String object13_num = "?";
    String object13_x = "?";
    String object13_y = "?";
    String object13_w = "?";
    String object13_h = "?";
    String object13_d = "?";
    String object14_num = "?";
    String object14_x = "?";
    String object14_y = "?";
    String object14_w = "?";
    String object14_h = "?";
    String object14_d = "?";
    String object15_num = "?";
    String object15_x = "?";
    String object15_y = "?";
    String object15_w = "?";
    String object15_h = "?";
    String object15_d = "?";
    String object16_num = "?";
    String object16_x = "?";
    String object16_y = "?";
    String object16_w = "?";
    String object16_h = "?";
    String object16_d = "?";

    String object0_1_x = "?";
    String object0_2_x = "?";
    String object0_3_x = "?";
    String object0_4_x = "?";
    String object0_5_x = "?";
    String object0_6_x = "?";
    String object0_7_x = "?";
    String object0_8_x = "?";
    String object0_9_x = "?";
    String object0_10_x = "?";
    String object0_11_x = "?";
    String object0_12_x = "?";
    String object0_13_x = "?";
    String object0_14_x = "?";
    String object0_15_x = "?";
    String object0_16_x = "?";
    String object0_1_y = "?";
    String object0_2_y = "?";
    String object0_3_y = "?";
    String object0_4_y = "?";
    String object0_5_y = "?";
    String object0_6_y = "?";
    String object0_7_y = "?";
    String object0_8_y = "?";
    String object0_9_y = "?";
    String object0_10_y = "?";
    String object0_11_y = "?";
    String object0_12_y = "?";
    String object0_13_y = "?";
    String object0_14_y = "?";
    String object0_15_y = "?";
    String object0_16_y = "?";
    String object0_1_d = "?";
    String object0_2_d = "?";
    String object0_3_d = "?";
    String object0_4_d = "?";
    String object0_5_d = "?";
    String object0_6_d = "?";
    String object0_7_d = "?";
    String object0_8_d = "?";
    String object0_9_d = "?";
    String object0_10_d = "?";
    String object0_11_d = "?";
    String object0_12_d = "?";
    String object0_13_d = "?";
    String object0_14_d = "?";
    String object0_15_d = "?";
    String object0_16_d = "?";

    String tiltStr = "?", panStr = "?";
    String ballTrkStr = "0";
    String head_x = "0";
    String head_y = "0";
    double[] data = null;
    String button = "?";
    String axis_x = "?";
    String axis_y = "?";
    String axis_z = "?";  

    int last_x = 0;
    int last_y = 0;
    int last_z = 0;

    int last_mx = 0;
    int last_my = 0;

    int last_track = 0;

    public HumanControl(RoboWirelessBase p, int oplanePort) {
        super("HumanControl");
        this.parent     = p;
        this.oplanePort = oplanePort;
 
        try {
            (new OPlaneReceiver()).start();
            //(new JoystickControl()).start(); // !! thread currently deactivated, as joystick
                                               // commands currently sent directly via client
            init();
        } catch (IOException e) {
            e.printStackTrace();
            System.exit(1);
        } catch(Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
        
    }

    // disables ball tracking, if it is currently enabled
    private void disableBallTrk() {
        if (ballTrk) {
            ballTrk = false;
            parent.getRoboCommander().send(BALL_TRACK_COMMAND, "0");
        }
    }

    // enables ball tracking, if it is currently disabled
    private void enableBallTrk() {
        if (!ballTrk) {
            ballTrk = true;
            parent.getRoboCommander().send(BALL_TRACK_COMMAND, "1");
        }
    }

    // sets up oplane, label, buttons display
    private void init() throws Exception {

        // button1 = move head to this position
        // button2 = centre head
        // button3 = track ball

        paintArea.addMouseListener(new MouseInputAdapter() {
            public void mousePressed(MouseEvent e) {
                if ((e.getModifiers() & InputEvent.BUTTON1_MASK) == InputEvent.BUTTON1_MASK) { 
                    mouseDown = true;
                    disableBallTrk();
                }
            }

            public void mouseReleased(MouseEvent e) {
                if (mouseDragged) {
                    if ((e.getModifiers() & InputEvent.BUTTON1_MASK) == InputEvent.BUTTON1_MASK) { 
                        // move the head to the release position
                        headMovement(e.getX(), e.getY());
                        mouseDragged = false;
                        mouseDown    = false;
                    }
                }
            }

            public void mouseClicked(MouseEvent e) {
                if ((e.getModifiers() & InputEvent.BUTTON1_MASK) == InputEvent.BUTTON1_MASK) {
                    headMovement(e.getX(), e.getY());
                } else if ((e.getModifiers() & InputEvent.BUTTON2_MASK) == InputEvent.BUTTON2_MASK) {
                    disableBallTrk();
                    headMovement(IMAGE_WIDTH/2, 135); // 135 found by testing, not dynamic as not directly in centre
                } else if ((e.getModifiers() & InputEvent.BUTTON3_MASK) == InputEvent.BUTTON3_MASK) {
                    enableBallTrk();
                } 
            }
        });
        
        paintArea.addMouseMotionListener(new MouseMotionAdapter() {
            public void mouseDragged(MouseEvent e) {
                if ((e.getModifiers() & InputEvent.BUTTON1_MASK) == InputEvent.BUTTON1_MASK) { 
                    mouseDragged = true;
                    if (mouseDown) {
                        if (moveHeadCounter==0) {
                            headMovement(e.getX(), e.getY());
                        }
                        moveHeadCounter = (moveHeadCounter+1) % MOVE_HEAD_INTERVAL;
                    }
                }
            }
        });
    
        worldModelButton.setMnemonic(KeyEvent.VK_W);
        worldModelButton.setToolTipText("(" + RoboConstant.WORLDMODEL_TRIGGER + ", " + DEFAULT_PLAYER + ")");
        worldModelButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                parent.getRoboCommander().send(RoboConstant.WORLDMODEL_TRIGGER, DEFAULT_PLAYER);
            }
        });
        nameField.setMinimumSize(new Dimension(60, 21));
        nameField.setPreferredSize(new Dimension(60, 21));
        nameField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                nameField.selectAll();
            }
        });
        nameField.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                parent.getRoboCommander().send(nameField.getText(), valueField.getText());
            }
        });
        valueField.setPreferredSize(new Dimension(100, 21));
        valueField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                valueField.selectAll();
            }
        });
        valueField.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                parent.getRoboCommander().send(nameField.getText(), valueField.getText());
            }
        });
        sendButton.setToolTipText("Send the name and value to control the robot");
        sendButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                parent.getRoboCommander().send(nameField.getText(), valueField.getText());
            }
        });
        this.setDefaultCloseOperation(3);
        motionButton.setToolTipText("Allow the use of keyboard to control the robot");
        motionButton.addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                parent.getRoboCommander().moveDog(e);
            }
        });
        
        commandBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        commandPanel.setBorder(commandBorder);
        commandPanel.add(worldModelButton);
        commandPanel.add(nameLabel);
        commandPanel.add(nameField);
        commandPanel.add(valueLabel);
        commandPanel.add(valueField);
        commandPanel.add(sendButton);
        commandPanel.add(motionButton);

        oplaneLogFileCheckBox.setToolTipText("If selected, the oplanes will get saved in the oplane log file specified");
        oplaneLogFileCheckBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (oplaneLogFileCheckBox.isSelected()) {
                    startOPlaneLog = true;
                    pauseLogFileCheckBox.setEnabled(true);
                    if (oplaneLogFileField.getText().length()==0) {
                        oplaneLogFileField.setText(RoboConstant.DEFAULT_OPLANE_LOG_FILE);
                    }
                    oplaneLogFileField.requestFocus();
                    try {
                        initialiseLogFile(oplaneLogFileField, oplaneLogFileCheckBox);
                    } catch (IOException ex) {
                        System.out.println("Couldn't initialise log file");
                    }
                } else {
                    startOPlaneLog = false;
                    pauseLogFileCheckBox.setEnabled(false);
                    closeLogFile();
                }
            }
        });
        pauseLogFileCheckBox.setEnabled(false);
        pauseLogFileCheckBox.setToolTipText("If selected, data will not be written to the log file");
        pauseLogFileCheckBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (pauseLogFileCheckBox.isSelected()) {
                    startOPlaneLog = false;
                } else {
                    startOPlaneLog = true;
                }
            }
        });
        oplaneLogFileField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                oplaneLogFileField.selectAll();
            }
        });

        oplaneChooseButton.setToolTipText("Choose a file to log oplane");
        oplaneChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JFileChooser chooser;
                String currentDir = "";
                int slashPos, dotPos;
                
                // start the file chooser in the directory of the specified file
                currentDir = oplaneLogFileField.getText().trim();
                slashPos   = currentDir.lastIndexOf(File.separator);
                if (slashPos==-1) {
                    chooser = new JFileChooser();
                } else {
                    chooser = new JFileChooser(currentDir.substring(0, slashPos));
                }
                int returnVal = chooser.showOpenDialog(null);
                if(returnVal == JFileChooser.APPROVE_OPTION) {
                    oplaneLogFileField.setText(chooser.getSelectedFile().getPath());
                }
            }
        });
        oplaneLogPanel.add(oplaneLogFileCheckBox);
        oplaneLogPanel.add(pauseLogFileCheckBox);
        oplaneLogPanel.add(new JPanel());
        oplaneLogPanel.add(oplaneLogFileLabel);
        oplaneLogPanel.add(oplaneLogFileField);
        oplaneLogPanel.add(oplaneChooseButton);
        
        logBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        logPanel.setBorder(logBorder);
        logPanel.add(oplaneLogPanel);

        // GUI for the settings panel
        tiltField.setEditable(false);
        panField.setEditable(false);
        erollField.setEditable(false);
/*
        robotSettingsPanel.add(panLabel);
        robotSettingsPanel.add(panField);
        robotSettingsPanel.add(tiltLabel);
        robotSettingsPanel.add(tiltField);
        robotSettingsPanel.add(erollLabel);
        robotSettingsPanel.add(erollField);
*/
        distanceCheckBox.setToolTipText("The distance to each other will be shown if selected");

        horizonCheckBox.setToolTipText("The horizon of the robot camera will be shown if selected");
        horizonCheckBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (gotoStartButton.isEnabled() || gotoLastButton.isEnabled()) {
                    // will draw again if it is in loading mode
                    paintScreen();
                }
            }
        });
        robotRelativeCheckBox.setToolTipText("Objects relative to the robot if selected. Play Field relative otherwise.");
        robotRelativeCheckBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (gotoStartButton.isEnabled() || gotoLastButton.isEnabled()) {
                    // will draw again if it is in loading mode
                    paintScreen();
                }
            }
        });
        tiltFirstCheckBox.setToolTipText("Robot will move tilt first then pan if selected. Pan first otherwise.");
        tiltFirstCheckBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
            }
        });
        
        displaySettingsPanel.add(distanceCheckBox);
        displaySettingsPanel.add(horizonCheckBox);
        displaySettingsPanel.add(robotRelativeCheckBox);
        displaySettingsPanel.add(tiltFirstCheckBox);
        
        settingsBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));        
        settingsPanel.setBorder(settingsBorder);
//        settingsPanel.setLayout(new GridLayout(2, 1));
        settingsPanel.setLayout(new GridLayout(1, 1));
//        settingsPanel.add(robotSettingsPanel);
        settingsPanel.add(displaySettingsPanel);
        

        // set the default to the current directory
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
                int slashPos, dotPos;
                
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
        gotoStartButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                oplaneIndex = 0;
                paintScreen();
                
            }
        });

        backwardButton.setToolTipText("Go to the previous frame");
        backwardButton.setEnabled(false);
        backwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                oplaneIndex--;
                paintScreen();
            }
        });

        // animation of the oplane, implemented in the javax.swing.Timer class
        animationTimer = new javax.swing.Timer(RoboConstant.OPLANE_NEXT_FRAME_DELAY, new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                animationTimerAction();
            }
        });
        
        playButton.setToolTipText("Play animation");
        playButton.setEnabled(false);
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
        forwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                oplaneIndex++;
                paintScreen();
            }
        });
        
        gotoLastButton.setToolTipText("Go to the last frame");
        gotoLastButton.setEnabled(false);
        gotoLastButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                oplaneIndex = oplane.length-1;
                paintScreen();
            }
        });
        
        currentFrame.setEditable(false);
        currentFrame.setHorizontalAlignment(JTextField.RIGHT);
        currentFrame.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    oplaneIndex = Integer.parseInt(currentFrame.getText());
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
        
        playlogBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        playlogPanel.setBorder(playlogBorder);
        playlogPanel.setLayout(new GridLayout(2, 1));
        playlogPanel.add(playBackFilePanel);
        playlogPanel.add(playBackPanel);

        userBottomPanel.setLayout(new BorderLayout());
        userBottomPanel.add(logPanel, BorderLayout.NORTH);
        userBottomPanel.add(settingsPanel, BorderLayout.CENTER);
        userBottomPanel.add(playlogPanel, BorderLayout.SOUTH);

        userPanel.setLayout(new BorderLayout());
        userPanel.add(commandPanel, BorderLayout.NORTH);
        userPanel.add(userBottomPanel, BorderLayout.CENTER);
        
        scrollPane.setWheelScrollingEnabled(true);
        scrollPane.setPreferredSize(new Dimension(WINDOW_WIDTH, WINDOW_HEIGHT));
        
        viewPanel.add(scrollPane);
        
        this.getContentPane().setLayout(new BorderLayout());
        this.getContentPane().add(viewPanel, BorderLayout.CENTER);
        this.getContentPane().add(userPanel, BorderLayout.SOUTH);

        // create our images and a graphics contexts
        clearImgBuffer();

        setResizable(false);
        setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
        pack();
    }
    
    // create new oplane log file, where filename is taken from logfilefield only if checkbox selected
    private void initialiseLogFile(JTextField logFileField, JCheckBox logFileCheckBox) throws IOException {
        String filename = logFileField.getText().trim();
        if (startOPlaneLog && !filename.equals("")) {
            oplaneLogFile = new PrintWriter(new BufferedWriter(new FileWriter(filename)));
            oplaneLogFile2 = new PrintWriter(new BufferedWriter(new FileWriter(filename+"m")));
            oplaneLogFile3 = new PrintWriter(new BufferedWriter(new FileWriter(filename+"w")));
        }
    }

    // pauses writing to log file
    private void pauseLogFile() {
        startOPlaneLog = false;
    }

    // resumes writing to log file
    private void resumeLogFile() {
        startOPlaneLog = true;
    }

    // try to close oplane log file
    private void closeLogFile() {
        while (fileLocked) {
            System.out.println("FILE LOCKED... waiting to close");
        }
        startOPlaneLog = false;
        if (oplaneLogFile!=null) {
            oplaneLogFile.close();
        }
        if (oplaneLogFile2!=null) {
            oplaneLogFile2.close();
        }
        if (oplaneLogFile3!=null) {
            oplaneLogFile3.close();
        }
        oplaneLogFileField.setEnabled(true);
        oplaneLogFileField.setText("blank");
    }

    private void switchCommander() {
        parent.getRoboCommander().setVisible(true);
        this.setVisible(false);
    }

    private void headMovement(int x, int y) {
        int currentPan, currentTilt, newTopLeftX, newTopLeftY, newPan, newTilt, sendPan, sendTilt;
        double panDouble, tiltDouble;
        if (mouseDebugMsg) {
            System.out.println("begining: " + x + " " + y + ", ");
        }

        newTopLeftX = x - FRAME_WIDTH/2;
        newTopLeftY = y - FRAME_HEIGHT/2;
        
        if (mouseDebugMsg) {
            System.out.println("topLeft: new " + newTopLeftX + " " + newTopLeftY);
        }

        if (!robotRelativeCheckBox.isSelected()) {
            newTopLeftX = (int) (newTopLeftX * neg_cos_roll - newTopLeftY * neg_sin_roll - x*neg_cos_roll + y*neg_sin_roll + x);
            newTopLeftY = (int) (newTopLeftX * neg_sin_roll + newTopLeftY * neg_cos_roll - x*neg_sin_roll - y*neg_cos_roll + y);
        }

        if (mouseDebugMsg) {
            System.out.println("topLeft: rotated " + newTopLeftX + " " + newTopLeftY);
        }

        if (newTopLeftX<0) {
            newTopLeftX = 0;
        } else if (newTopLeftX>(IMAGE_WIDTH-FRAME_WIDTH)) {
            newTopLeftX = IMAGE_WIDTH-FRAME_WIDTH;
        }
        
        if (newTopLeftY<0) {
            newTopLeftY = 0;
        } else if (newTopLeftY>(IMAGE_HEIGHT-FRAME_HEIGHT)) {
            newTopLeftY = IMAGE_HEIGHT-FRAME_HEIGHT;
        }
        newPan  = topLeft2PanDegree(newTopLeftX);
        newTilt = topLeft2TiltDegree(newTopLeftY);
        
        currentPan  = topLeft2PanDegree(topLeftX);
        currentTilt = topLeft2TiltDegree(topLeftY);
        
        panDouble  = Math.asin(Math.sin(Math.toRadians(newPan)) * Math.cos(Math.toRadians(newTilt)));
        tiltDouble = Math.asin(Math.sin(Math.toRadians(newTilt)) / Math.cos(panDouble));
        
        sendPan  = (int) Math.toDegrees(panDouble);
        sendTilt = (int) Math.toDegrees(tiltDouble);
        
        if (mouseDebugMsg) {
            System.out.println("topLeft: current " + topLeftX   + " " + topLeftY    + "; new " + newTopLeftX + " " + newTopLeftY);
            System.out.println("Pan/tilt picture: " + currentPan + " " + currentTilt + "; last: " + lastPanSent + " " + lastTiltSent + "; new: " + newPan + " " + newTilt);
            System.out.println("Sending pan: " + sendPan + " tilt: " + sendTilt);
        }

        if (lastPanSent!=newPan || lastTiltSent!=newTilt) {
            if (tiltFirstCheckBox.isSelected()) {
                parent.getRoboCommander().send(TILT_PAN_COMMAND, newPan + " " + newTilt);
                lastPanSent  = newPan;
                lastTiltSent = newTilt;
            } else {
                parent.getRoboCommander().send(PAN_TILT_COMMAND, newPan + " " + newTilt);
                lastPanSent  = newPan;
                lastTiltSent = newTilt;
            }
        } else if (mouseDebugMsg) {
            System.out.println("Equal pan and tilt. Don't send");
        }

    }
    
    private void loadFile() {
        try {
            BufferedReader file = new BufferedReader(new FileReader(playBackFileField.getText()));
            // store objects   
            Plane[] planes = new Plane[RoboConstant.NUM_OBJ];
            String line, name, value;
            String[] lineValues;

            Vector allOplanes = new Vector();

            oplane     = null;

            Vector v = new Vector();
            boolean newCommand;
            String[] array;
            Vector oplaneVector = new Vector();
            newCommand = false;
            name       = "";
            value      = "";
            String pan = "", tilt = "";

            while (true) {
                line = file.readLine();
                if (line==null) {
                    break;
                }
                if (v.size()>0 && (line.length()==0 || line.equals(END_OPLANE))) {
                    oplaneVector.addElement(new OPlane(v, name, value));
                    v.clear();
                    // don't reset the name and value, use the previous one
                    newCommand = false;
                    continue;
                }
                array = RoboUtility.split(line,",");
                if (array[0].equals(START_PAN_TILT)) {
                    pan = array[1];
                    tilt = array[2];
                } else if (array.length==COMMAND_LEN) {
                    if (newCommand) {
                        System.out.println("commands " + name + " " + value + " are interleave between planes");
                    }
                    name  = array[0];
                    value = array[1];
                    newCommand = true;
                } else if (array.length==LOG_FILE_PLANE_LEN) {
                    v.addElement(new Plane(array,pan,tilt));
                }
            }

            oplane = new OPlane[oplaneVector.size()];
            oplaneVector.copyInto(oplane);

            oplaneIndex = 0;
            paintScreen();

            totalFrame.setText(" / " + oplane.length);
            if (oplane.length>0) {
                playButton.setEnabled(true);
            }

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
            return;
        }
    }

    private void animationTimerAction() {
        oplaneIndex++;
        paintScreen();
        if (oplaneIndex==(oplane.length-1)) {
            animationTimer.stop();
            JOptionPane.showMessageDialog(this, "End of animation", "Information", JOptionPane.INFORMATION_MESSAGE);
            chooseButton.setEnabled(true);
            loadButton.setEnabled(true);
            gotoStartButton.setEnabled(true);
            backwardButton.setEnabled(true);
            currentFrame.setEditable(true);
        }
    }

    // clear the buffer
    private void clearImgBuffer() {
        offscreen.setColor(RoboConstant.BACKGROUND_COLOUR);
        offscreen.fillRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
    }

    private void clearScreen() {
        clearImgBuffer();
        paintArea.repaint();
    }

    // paint the frame specified with oplaneIndex on the screen
    private void paintScreen() {
        clearImgBuffer();

        if (oplane!=null && oplaneIndex>=0 && oplaneIndex<oplane.length) {
            for (int i=0; i<oplane[oplaneIndex].planes.length; i++) {
                startDrawObj(i==0, oplane[oplaneIndex].planes[i].data);
            }
            nameField.setText(oplane[oplaneIndex].name);
            valueField.setText(oplane[oplaneIndex].value);
            
            currentFrame.setText("" + (oplaneIndex+1)); // counter starts with 1

            if (oplaneIndex==0) {
                gotoStartButton.setEnabled(false);
                backwardButton.setEnabled(false);
            } else if (!animationTimer.isRunning()) {
                gotoStartButton.setEnabled(true);
                backwardButton.setEnabled(true);
                currentFrame.setEditable(true);
            }

            if (oplaneIndex==(oplane.length-1)) {
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
            "Error in displaying the oplane.\nEither the file loading process is not completed successfully\nor an invalid frame is specified", 
            "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    // paint the given o plane
    // oplane data is given in the following form:
    // pan, tilt, objectVector
    // where visible objects are put in objectVector with data:
    // x, y, w, h, d
    // if no objects seen, objectVector is simply:
    // 0, 0, 0, 0, 0, 0
    public void drawOPlane(byte[] cp, int numObj) {
        int i, x, y, w, h, d, index=0, object;
        double tilt, pan;

        clearImgBuffer();

        pan = RoboUtility.bytesToInt(cp, index) / RoboConstant.SEND_SCALE;
        index += RoboConstant.INT_BYTES;
        tilt = RoboUtility.bytesToInt(cp, index) / RoboConstant.SEND_SCALE;
        index += RoboConstant.INT_BYTES;

        if (oplaneCounter==0) {
            long time = System.currentTimeMillis();
            writeLog("#"+time+"," + pan + "," + tilt + "\n", 1); // !! TEMPORARY
            panStr = pan + "";
            tiltStr = tilt + "";
//            writeLog("#,"+pan + "," + tilt + "\n", 1); // !! TEMPORARY
        }

        // change the roll only if the pan and tilt changes
        if (pan!=lastPan || tilt!=lastTilt) {
            // down is positive as it is from VisualCortex
            roll     = Math.asin(Math.sin(pan) * Math.sin(-tilt));
            pos_cos_roll = Math.cos(roll);
            pos_sin_roll = Math.sin(roll);
            neg_cos_roll = Math.cos(-roll);
            neg_sin_roll = Math.sin(-roll);
            topLeftX = pan2TopLeft(pan);
            topLeftY = tilt2TopLeft(tilt);
            lastPan  = pan;
            lastTilt = tilt;
        }
/* not really needed for driving robot
        // display the tilt and pan
        panField.setText("" + displayDouble.format(pan*180/Math.PI));
        tiltField.setText("" + displayDouble.format(tilt*180/Math.PI));
        erollField.setText("" + displayDouble.format(roll*180/Math.PI));
*/        
        drawImageBorder();
        // draw the horizon
        if (horizonCheckBox.isSelected()) {
            drawHorizon();
        }

        // write all object data into logfile
        for (i=0; i<numObj; i++) {
            object = RoboUtility.bytesToInt(cp, index);

            // extract the object data and write it to logfile
            index += RoboConstant.INT_BYTES;
            x = RoboUtility.bytesToInt(cp, index);
            index += RoboConstant.INT_BYTES;
            y = RoboUtility.bytesToInt(cp, index);
            index += RoboConstant.INT_BYTES;
            w = RoboUtility.bytesToInt(cp, index);
            index += RoboConstant.INT_BYTES;
            h = RoboUtility.bytesToInt(cp, index);
            index += RoboConstant.INT_BYTES;
            d = RoboUtility.bytesToInt(cp, index);
            index += RoboConstant.INT_BYTES;

            resetObjectVariables();
            if (oplaneCounter==0) {
                if (object==0 && x==0 && y==0 && w==0 && h==0 && d==0) {
                    // don't draw anything
                    writeLog(object + "," + x + "," + y + "," + w + "," + h + "," + d + "\n", 1);
                    break;
                } else {
            	    drawObj(object, x, y, w, h, d, pan, tilt);
                    switch (object) {
                        case RoboConstant.BALL: 
                            object00_num = object + "";
                            object00_x = x + "";
                            object00_y = y + "";
                            object00_w = w + "";
                            object00_h = h + "";                             
                            object00_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_GOAL:
                            object01_num = object + "";
                            object01_x = x + "";
                            object01_y = y + "";
                            object01_w = w + "";
                            object01_h = h + "";                             
                            object01_d = d + "";                            
                            break;
                        case RoboConstant.YELLOW_GOAL:
                            object02_num = object + "";
                            object02_x = x + "";
                            object02_y = y + "";
                            object02_w = w + "";
                            object02_h = h + "";                             
                            object02_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_LEFT_BEACON:
                            object03_num = object + "";
                            object03_x = x + "";
                            object03_y = y + "";
                            object03_w = w + "";
                            object03_h = h + "";                             
                            object03_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_RIGHT_BEACON:
                            object04_num = object + "";
                            object04_x = x + "";
                            object04_y = y + "";
                            object04_w = w + "";
                            object04_h = h + "";                             
                            object04_d = d + "";                            
                            break;
                        case RoboConstant.GREEN_LEFT_BEACON:
                            object05_num = object + "";
                            object05_x = x + "";
                            object05_y = y + "";
                            object05_w = w + "";
                            object05_h = h + "";                             
                            object05_d = d + "";                            
                            break;
                        case RoboConstant.GREEN_RIGHT_BEACON:
                            object06_num = object + "";
                            object06_x = x + "";
                            object06_y = y + "";
                            object06_w = w + "";
                            object06_h = h + "";                             
                            object06_d = d + "";                            
                            break;
                        case RoboConstant.YELLOW_LEFT_BEACON:
                            object07_num = object + "";
                            object07_x = x + "";
                            object07_y = y + "";
                            object07_w = w + "";
                            object07_h = h + "";                             
                            object07_d = d + "";                            
                            break;
                        case RoboConstant.YELLOW_RIGHT_BEACON:
                            object08_num = object + "";
                            object08_x = x + "";
                            object08_y = y + "";
                            object08_w = w + "";
                            object08_h = h + "";                             
                            object08_d = d + "";                            
                            break;
                        case RoboConstant.RED_DOG:            
                            object09_num = object + "";
                            object09_x = x + "";
                            object09_y = y + "";
                            object09_w = w + "";
                            object09_h = h + "";                             
                            object09_d = d + "";                            
                            break;
                        case RoboConstant.RED_DOG2:
                            object10_num = object + "";
                            object10_x = x + "";
                            object10_y = y + "";
                            object10_w = w + "";
                            object10_h = h + "";                             
                            object10_d = d + "";                            
                            break;
                        case RoboConstant.RED_DOG3:
                            object11_num = object + "";
                            object11_x = x + "";
                            object11_y = y + "";
                            object11_w = w + "";
                            object11_h = h + "";                             
                            object11_d = d + "";                            
                            break;
                        case RoboConstant.RED_DOG4:
                            object12_num = object + "";
                            object12_x = x + "";
                            object12_y = y + "";
                            object12_w = w + "";
                            object12_h = h + "";                             
                            object12_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_DOG:
                            object13_num = object + "";
                            object13_x = x + "";
                            object13_y = y + "";
                            object13_w = w + "";
                            object13_h = h + "";                             
                            object13_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_DOG2:
                            object14_num = object + "";
                            object14_x = x + "";
                            object14_y = y + "";
                            object14_w = w + "";
                            object14_h = h + "";                             
                            object14_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_DOG3:
                            object15_num = object + "";
                            object15_x = x + "";
                            object15_y = y + "";
                            object15_w = w + "";
                            object15_h = h + "";                             
                            object15_d = d + "";                            
                            break;
                        case RoboConstant.BLUE_DOG4:
                            object16_num = object + "";
                            object16_x = x + "";
                            object16_y = y + "";
                            object16_w = w + "";
                            object16_h = h + "";                             
                            object16_d = d + "";                            
                            break;
                    }

                }
                writeLog(object + "," + x + "," + y + "," + w + "," + h + "," + d + "\n", 1);
            }

        }
//        executeActions();

        if (oplaneCounter==0) {
            writeLog(END_OPLANE + "\n", 1);
        }

        oplaneCounter = (oplaneCounter+1) % OPLANE_LOG_INTERVAL;

        if (!isVisible()) {
            setVisible(true);
        }

        paintArea.repaint();
    }

    // based on inputs received, executes a certain action
    public void executeActions() {
        int x = 0;
        int y = 0;
        int z = 0;
        int button = 0;

       
//        x = moveX();

//        Y yDecisions = new Y(this.parent);
//        y = yDecisions.Y();

//        z = moveZ();

        if (!(x == last_x && y == last_y && z == last_z)) {
           String val = x + " " + y + " " + z;
           System.out.println("MOVING: "+ val);  
           parent.getRoboCommander().send("j", val);
           last_x = x;
           last_y = y;
           last_z = z;
        }

        button bDecisions = new button(this.parent);
        button = bDecisions.button(); 

        if (button != 0) {
           System.out.println("KICKING: "+ (button-1));  
           parent.getRoboCommander().send("K", (button-1)+"");
        }
        


        if (data == null) {
            System.out.println("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@NULL");
        } else {
            System.out.println("[7] = "+data[7]);
            System.out.println("[11] = "+data[11]);
            System.out.println("[20] = "+data[20]);
            System.out.println("[pan] = "+panStr);
            System.out.println("[tilt] = "+tiltStr);

            System.out.println("===========");
    /*        for (int i = 0 ; i < data.length ; i++) {
                System.out.println(data[i] + ",");
           }
*/
        }

    }

    // reset object data variables
    public void resetObjectVariables() {
        object00_num = "?";
        object00_x = "?";
        object00_y = "?";
        object00_w = "?";
        object00_h = "?";
        object00_d = "?";
        object01_num = "?";
        object01_x = "?";
        object01_y = "?";
        object01_w = "?";
        object01_h = "?";
        object01_d = "?";
        object02_num = "?";
        object02_x = "?";
        object02_y = "?";
        object02_w = "?";
        object02_h = "?";
        object02_d = "?";
        object03_num = "?";
        object03_x = "?";
        object03_y = "?";
        object03_w = "?";
        object03_h = "?";
        object03_d = "?";
        object04_num = "?";
        object04_x = "?";
        object04_y = "?";
        object04_w = "?";
        object04_h = "?";
        object04_d = "?";
        object05_num = "?";
        object05_x = "?";
        object05_y = "?";
        object05_w = "?";
        object05_h = "?";
        object05_d = "?";
        object06_num = "?";
        object06_x = "?";
        object06_y = "?";
        object06_w = "?";
        object06_h = "?";
        object06_d = "?";
        object07_num = "?";
        object07_x = "?";
        object07_y = "?";
        object07_w = "?";
        object07_h = "?";
        object07_d = "?";
        object08_num = "?";
        object08_x = "?";
        object08_y = "?";
        object08_w = "?";
        object08_h = "?";
        object08_d = "?";
        object09_num = "?";
        object09_x = "?";
        object09_y = "?";
        object09_w = "?";
        object09_h = "?";
        object09_d = "?";
        object10_num = "?";
        object10_x = "?";
        object10_y = "?";
        object10_w = "?";
        object10_h = "?";
        object10_d = "?";
        object11_num = "?";
        object11_x = "?";
        object11_y = "?";
        object11_w = "?";
        object11_h = "?";
        object11_d = "?";
        object12_num = "?";
        object12_x = "?";
        object12_y = "?";
        object12_w = "?";
        object12_h = "?";
        object12_d = "?";
        object13_num = "?";
        object13_x = "?";
        object13_y = "?";
        object13_w = "?";
        object13_h = "?";
        object13_d = "?";
        object14_num = "?";
        object14_x = "?";
        object14_y = "?";
        object14_w = "?";
        object14_h = "?";
        object14_d = "?";
        object15_num = "?";
        object15_x = "?";
        object15_y = "?";
        object15_w = "?";
        object15_h = "?";
        object15_d = "?";
        object16_num = "?";
        object16_x = "?";
        object16_y = "?";
        object16_w = "?";
        object16_h = "?";
        object16_d = "?";
    }

    // writes out to logfile
    public void writeLog(String msg, int log) {
        fileLocked = true;
        // write information from HumanControl
        if (log == 1) {
            if (oplaneCounter==0 && startOPlaneLog) {
                try {
                    if (oplaneLogFile==null) {
                        oplaneLogFile = new PrintWriter(new BufferedWriter(new FileWriter(RoboConstant.DEFAULT_OPLANE_LOG_FILE)));
                    }
                    oplaneLogFile.print(msg);
                    oplaneLogFile.flush();
                } catch (IOException e) {
                    JOptionPane.showMessageDialog(null, "Error: Cannot write to file " + RoboConstant.DEFAULT_OPLANE_LOG_FILE, "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        // write information from RoboCommander
        } else if (log == 2) {
            if (oplaneCounter==0 && startOPlaneLog) {
                try {
                    if (oplaneLogFile2==null) {
                        oplaneLogFile2 = new PrintWriter(new BufferedWriter(new FileWriter(RoboConstant.DEFAULT_OPLANE_LOG_FILE)));
                    }
                    oplaneLogFile2.print(msg);
                    oplaneLogFile2.flush();
                } catch (IOException e) {
                    JOptionPane.showMessageDialog(null, "Error: Cannot write to file " + RoboConstant.DEFAULT_OPLANE_LOG_FILE, "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        // write information from world model thread
        } else if (log == 3) {
            if (oplaneCounter==0 && startOPlaneLog) {
                try {
                    if (oplaneLogFile3==null) {
                        oplaneLogFile3 = new PrintWriter(new BufferedWriter(new FileWriter(RoboConstant.DEFAULT_OPLANE_LOG_FILE)));
                    }
                    oplaneLogFile3.print(msg);
                    oplaneLogFile3.flush();
                } catch (IOException e) {
                    JOptionPane.showMessageDialog(null, "Error: Cannot write to file " + RoboConstant.DEFAULT_OPLANE_LOG_FILE, "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        }
        System.out.flush();
        fileLocked = false;
    }

    // given an array of object data, draws the horizon and displays tilt/pan
    // then draws the object
    private void startDrawObj(boolean firstTime, double[] array) {
        double pan = (int) array[6];
        double tilt = (int) array[7];

        // change the roll only if the pan and tilt changes
        if (pan!=lastPan || tilt!=lastTilt) {
            // down is positive as it is from VisualCortex
            roll     = Math.asin(Math.sin(pan) * Math.sin(-tilt));
            pos_cos_roll = Math.cos(roll);
            pos_sin_roll = Math.sin(roll);
            neg_cos_roll = Math.cos(-roll);
            neg_sin_roll = Math.sin(-roll);
            topLeftX = pan2TopLeft(pan);
            topLeftY = tilt2TopLeft(tilt);
            lastPan  = pan;
            lastTilt = tilt;
        }

        // display the tilt and pan
        panField.setText("" + displayDouble.format(pan*180/Math.PI));
        tiltField.setText("" + displayDouble.format(tilt*180/Math.PI));
        erollField.setText("" + displayDouble.format(roll*180/Math.PI));
        
        if (firstTime) {
            drawImageBorder();
            // draw the horizon
            if (horizonCheckBox.isSelected()) {
                drawHorizon();
            }
        }
        drawObj((int) array[0], (int) array[1], (int) array[2], (int) array[3], (int) array[4], (int) array[5], array[6], array[7]);
    }
    // given an object's coordinates and object type, paints the object
    public void drawObj(int objectNum, int x, int y, int w, int h, int distance, double pan, double tilt) {
        switch (objectNum) {
            case RoboConstant.BALL:
                drawBall(RoboConstant.BALL_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.BLUE_GOAL:
                drawGoal(RoboConstant.BLUE_GOAL_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.YELLOW_GOAL:
                drawGoal(RoboConstant.YELLOW_GOAL_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.BLUE_LEFT_BEACON:
                drawBeacon(RoboConstant.BLUE_LEFT_BEACON_TOP_COLOR, 
                           RoboConstant.BLUE_LEFT_BEACON_BOTTOM_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.BLUE_RIGHT_BEACON:
                drawBeacon(RoboConstant.BLUE_RIGHT_BEACON_TOP_COLOR, 
                           RoboConstant.BLUE_RIGHT_BEACON_BOTTOM_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.GREEN_LEFT_BEACON:
                drawBeacon(RoboConstant.GREEN_LEFT_BEACON_TOP_COLOR, 
                           RoboConstant.GREEN_LEFT_BEACON_BOTTOM_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.GREEN_RIGHT_BEACON:
                drawBeacon(RoboConstant.GREEN_RIGHT_BEACON_TOP_COLOR, 
                           RoboConstant.GREEN_RIGHT_BEACON_BOTTOM_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.YELLOW_LEFT_BEACON:
                drawBeacon(RoboConstant.YELLOW_LEFT_BEACON_TOP_COLOR, 
                           RoboConstant.YELLOW_LEFT_BEACON_BOTTOM_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.YELLOW_RIGHT_BEACON:
                drawBeacon(RoboConstant.YELLOW_RIGHT_BEACON_TOP_COLOR, 
                           RoboConstant.YELLOW_RIGHT_BEACON_BOTTOM_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.RED_DOG:            
            case RoboConstant.RED_DOG2:
            case RoboConstant.RED_DOG3:
            case RoboConstant.RED_DOG4:
                drawRobot(RoboConstant.RED_DOG_COLOR, x, y, w, h, distance);
                break;
            case RoboConstant.BLUE_DOG:
            case RoboConstant.BLUE_DOG2:
            case RoboConstant.BLUE_DOG3:
            case RoboConstant.BLUE_DOG4:
                drawRobot(RoboConstant.BLUE_DOG_COLOR, x, y, w, h, distance);
                break;

        }
    }
    
    // draw the rectangle border of the robot vision
    private void drawImageBorder() {
        // draw the rotated border
        int[] rotatedXArray = {0, RoboConstant.CPLANE_WIDTH, RoboConstant.CPLANE_WIDTH, 0};
        int[] rotatedYArray = {0, 0, RoboConstant.CPLANE_HEIGHT, RoboConstant.CPLANE_HEIGHT};

        double newX, newY, dx, dy;
        
        offscreen.setColor(RoboConstant.NORMAL_BORDER_COLOUR);
        if (robotRelativeCheckBox.isSelected()) {
            int[] normalXArray = {0, RoboConstant.CPLANE_WIDTH, RoboConstant.CPLANE_WIDTH, 0};
            int[] normalYArray = {0, 0, RoboConstant.CPLANE_HEIGHT, RoboConstant.CPLANE_HEIGHT};

            for (int j=0; j<normalXArray.length; j++) {
                normalXArray[j] = normalXArray[j] * SCALE + topLeftX;
                normalYArray[j] = normalYArray[j] * SCALE + topLeftY;
            }

            offscreen.drawPolygon(normalXArray, normalYArray, normalXArray.length);
        } else {
            dx = RoboConstant.CPLANE_WIDTH / 2.0;
            dy = RoboConstant.CPLANE_HEIGHT / 2.0;

            for (int j=0; j<rotatedXArray.length; j++) {
                newX = rotatedXArray[j] * neg_cos_roll - rotatedYArray[j] * neg_sin_roll - dx*neg_cos_roll + dy*neg_sin_roll + dx;
                newY = rotatedXArray[j] * neg_sin_roll + rotatedYArray[j] * neg_cos_roll - dx*neg_sin_roll - dy*neg_cos_roll + dy;

                rotatedXArray[j] = (int) (newX * SCALE + topLeftX);
                rotatedYArray[j] = (int) (newY * SCALE + topLeftY);
            }

            offscreen.drawPolygon(rotatedXArray, rotatedYArray, rotatedXArray.length);
        }
    }
    
    // draw a horizontal green line to denote the horinzon of the plane
    public void drawHorizon() {
        double newX, newY, dx, dy;

        offscreen.setColor(RoboConstant.NORMAL_BORDER_COLOUR);
        if (robotRelativeCheckBox.isSelected()) {
            dx = RoboConstant.CPLANE_WIDTH / 2.0;
            dy = RoboConstant.CPLANE_HEIGHT / 2.0;

            int[] xArray = {0, RoboConstant.CPLANE_WIDTH};
            int[] yArray = {(int)dy, (int)dy};

            for (int j=0; j<xArray.length; j++) {
                newX = xArray[j] * pos_cos_roll - yArray[j] * pos_sin_roll - dx*pos_cos_roll + dy*pos_sin_roll + dx;
                newY = xArray[j] * pos_sin_roll + yArray[j] * pos_cos_roll - dx*pos_sin_roll - dy*pos_cos_roll + dy;

                xArray[j] = (int) (newX * SCALE + topLeftX);
                yArray[j] = (int) (newY * SCALE + topLeftY);
            }

            offscreen.drawLine(xArray[0], yArray[0], xArray[1], yArray[1]);
        } else {
            int[] normalXArray = {0, RoboConstant.CPLANE_WIDTH};
            int[] normalYArray = {RoboConstant.CPLANE_HEIGHT/2, RoboConstant.CPLANE_HEIGHT/2};

            for (int j=0; j<normalXArray.length; j++) {
                normalXArray[j] = normalXArray[j] * SCALE + topLeftX;
                normalYArray[j] = normalYArray[j] * SCALE + topLeftY;
            }

            offscreen.drawLine(normalXArray[0], normalYArray[0], normalXArray[1], normalYArray[1]);
        }
    }
    
    // draws the ball
    public void drawBall(Color c, int x, int y, int w, int h, int distance) {
        if (distance < 30) {
            offscreen.setColor(RoboConstant.BALL_KICKABLE_COLOR);
        } else {
            offscreen.setColor(RoboConstant.BALL_COLOR);
        }
        offscreen.fillOval(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);
        
        offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
        offscreen.drawRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);

        if (distanceCheckBox.isSelected()) {
            // draw the distance in the center
            offscreen.drawString(distance + "", topLeftX+x*SCALE+w*SCALE/2, topLeftY+y*SCALE+h*SCALE/2);
        }
    }
    
    // draws the goal
    public void drawGoal(Color c, int x, int y, int w, int h, int distance) {
        if (robotRelativeCheckBox.isSelected()) {
            int[] xArray = {x, x+w, x+w, x};
            int[] yArray = {y, y, y+h, y+h};
            double dx, dy;
            double xCenter, yCenter; // center coordinate of shape
            double newX, newY;

            dx      = RoboConstant.CPLANE_WIDTH / 2.0;
            dy      = RoboConstant.CPLANE_HEIGHT / 2.0;
            xCenter = 0;
            yCenter = 0;
            
            /*
             * The roll is the effective roll, which transform the object to the line up with the plane
             * ie, edges are horinzontal with the edges of the plane
             * To rotate it back, we have to rotate about the centre of the CPlane
             *
             * 1. translate centre of the CPlane to the origin
             * 2. rotate about the origin
             * 3. translate origin to the centre of CPlane
             *     [1 0 dx] [cos  -sin  0] [1 0 -dx]
             * M = [0 1 dy] [sin   cos  0] [0 1 -dy]
             *     [0 0 1 ] [ 0     0   1] [0 0  1 ]
             *
             *     [cos  -sin   -dx*cos + dy*sin + dx]
             * M = [sin   cos   -dx*sin - dy*cos + dy]
             *     [ 0     0              1          ]
             *
             * so 
             * newX = x*cos - y*sin - dx*cos + dy*sin + dx
             * newY = x*sin + y*cos - dx*sin - dy*cos + dy  
             *
             * rotate around center of WINDOW
             *
            **/
            for (int j=0; j<xArray.length; j++) {
                newX = xArray[j] * pos_cos_roll - yArray[j] * pos_sin_roll - dx*pos_cos_roll + dy*pos_sin_roll + dx;
                newY = xArray[j] * pos_sin_roll + yArray[j] * pos_cos_roll - dx*pos_sin_roll - dy*pos_cos_roll + dy;
                
                xArray[j] = (int) (newX * SCALE + topLeftX);
                yArray[j] = (int) (newY * SCALE + topLeftY);
                
                xCenter += xArray[j];
                yCenter += yArray[j];
            }

            offscreen.setColor(c);
            offscreen.fillPolygon(xArray, yArray, xArray.length);

            offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
            offscreen.drawPolygon(xArray, yArray, xArray.length);

            if (distanceCheckBox.isSelected()) {
                // draw the distance in the center
                offscreen.drawString(distance + "", (int)(xCenter/(double)(xArray.length)), (int)(yCenter/(double)(yArray.length)));
            }
            
        } else {
            offscreen.setColor(c);
            offscreen.fillRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);

            offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
            offscreen.drawRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);

            if (distanceCheckBox.isSelected()) {
                // draw the distance in the center
                offscreen.drawString(distance + "", topLeftX+x*SCALE+w*SCALE/2, topLeftY+y*SCALE+h*SCALE/2);
            }
        }
    }

    // draws in the bottom and top half colours of a beacon
    public void drawBeacon(Color top, Color bottom, int x, int y, int w, int h, int distance) {
        if (robotRelativeCheckBox.isSelected()) {
            int[] xTopArray = {x, x+w, x+w, x};
            int[] yTopArray = {y, y, y+h/2, y+h/2};
            int[] xBottomArray = {x, x+w, x+w, x};
            int[] yBottomArray = {y+h-h/2, y+h-h/2, y+h, y+h};
            double dx, dy;
            double xCenter, yCenter; // center coordinate of shape
            double newX, newY;

            dx      = x + w/2.0;
            dy      = y + h/2.0;
            xCenter = 0;
            yCenter = 0;
            
            // rotate around the center of beacon
            for (int j=0; j<xTopArray.length; j++) {
            
                newX = xTopArray[j] * pos_cos_roll - yTopArray[j] * pos_sin_roll - dx*pos_cos_roll + dy*pos_sin_roll + dx;
                newY = xTopArray[j] * pos_sin_roll + yTopArray[j] * pos_cos_roll - dx*pos_sin_roll - dy*pos_cos_roll + dy;
                
                xTopArray[j] = (int) (newX * SCALE + topLeftX);
                yTopArray[j] = (int) (newY * SCALE + topLeftY);

                newX = xBottomArray[j] * pos_cos_roll - yBottomArray[j] * pos_sin_roll - dx*pos_cos_roll + dy*pos_sin_roll + dx;
                newY = xBottomArray[j] * pos_sin_roll + yBottomArray[j] * pos_cos_roll - dx*pos_sin_roll - dy*pos_cos_roll + dy;
                
                xBottomArray[j] = (int) (newX * SCALE + topLeftX);
                yBottomArray[j] = (int) (newY * SCALE + topLeftY);

                xCenter += xTopArray[j];
                yCenter += yTopArray[j] + yBottomArray[j];
            }

            offscreen.setColor(top);
            offscreen.fillPolygon(xTopArray, yTopArray, xTopArray.length);
            
            offscreen.setColor(bottom);
            offscreen.fillPolygon(xBottomArray, yBottomArray, xBottomArray.length);

            offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
            offscreen.drawPolygon(xTopArray, yTopArray, xTopArray.length);
            offscreen.drawPolygon(xBottomArray, yBottomArray, xBottomArray.length);

            if (distanceCheckBox.isSelected()) {
                // draw the distance in the center
                offscreen.drawString(distance + "", (int)(xCenter/(double)xTopArray.length), (int)(yCenter/(double)(yTopArray.length+yBottomArray.length)));
            }
            
        } else {
            offscreen.setColor(top);
            offscreen.fillRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE/2);

            offscreen.setColor(bottom);
            offscreen.fillRect(topLeftX+x*SCALE, topLeftY+y*SCALE+((h+1)*SCALE)/2, w*SCALE, h*SCALE/2);

            offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
            offscreen.drawRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE/2);
            offscreen.drawRect(topLeftX+x*SCALE, topLeftY+y*SCALE+h*SCALE/2, w*SCALE, h*SCALE-h*SCALE/2);

            if (distanceCheckBox.isSelected()) {
                // draw the distance in the center
                offscreen.drawString(distance + "", topLeftX+x*SCALE+w*SCALE/2, topLeftY+y*SCALE+h*SCALE/2);
            }
        }
    }
    
    // draws in the bottom and top half colours of a beacon
    public void drawRobot(Color c, int x, int y, int w, int h, int distance) {
        if (robotRelativeCheckBox.isSelected()) {
            int[] xArray = {x, x+w, x+w, x};
            int[] yArray = {y, y, y+h, y+h};
            double dx, dy;
            double xCenter, yCenter; // center coordinate of shape
            double newX, newY;

            dx      = x + w/2.0;
            dy      = y + h/2.0;
            xCenter = 0;
            yCenter = 0;
            
            // rotate around the center of robot
            for (int j=0; j<xArray.length; j++) {
                newX = xArray[j] * pos_cos_roll - yArray[j] * pos_sin_roll - dx*pos_cos_roll + dy*pos_sin_roll + dx;
                newY = xArray[j] * pos_sin_roll + yArray[j] * pos_cos_roll - dx*pos_sin_roll - dy*pos_cos_roll + dy;
                
                xArray[j] = (int) (newX * SCALE + topLeftX);
                yArray[j] = (int) (newY * SCALE + topLeftY);
                
                xCenter += xArray[j];
                yCenter += yArray[j];
            }

            offscreen.setColor(c);
//            offscreen.fillPolygon(xArray, yArray, xArray.length);
            offscreen.drawPolygon(xArray, yArray, xArray.length);

//            offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
//            offscreen.drawPolygon(xArray, yArray, xArray.length);

            if (distanceCheckBox.isSelected()) {
                // draw the distance in the center
                offscreen.drawString(distance + "", (int)(xCenter/(double)(xArray.length)), (int)(yCenter/(double)(yArray.length)));
            }
            
        } else {
            offscreen.setColor(c);
//            offscreen.fillRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);
            offscreen.drawRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);

//            offscreen.setColor(RoboConstant.OBJECT_BORDER_COLOUR);
//            offscreen.drawRect(topLeftX+x*SCALE, topLeftY+y*SCALE, w*SCALE, h*SCALE);

            if (distanceCheckBox.isSelected()) {
                // draw the distance in the center
                offscreen.drawString(distance + "", topLeftX+x*SCALE+w*SCALE/2, topLeftY+y*SCALE+h*SCALE/2);
            }
        }
    }
    
    // return topLeftX of a frame given the robot pan
    private int pan2TopLeft(double pan) {
        return (int) ((RoboConstant.LEFT_PAN_RADIAN-pan) / 
                      (double)(RoboConstant.LEFT_PAN_RADIAN+RoboConstant.RIGHT_PAN_RADIAN) * 
                      (IMAGE_WIDTH-FRAME_WIDTH));
    }
    
    // return topLeftY of a frame given the robot tilt
    private int tilt2TopLeft(double tilt) {
        // down is positive (because VisualCortex is)
        return (int) ((RoboConstant.UP_TILT_RADIAN+tilt) / 
                      (double)(RoboConstant.UP_TILT_RADIAN+RoboConstant.DOWN_TILT_RADIAN) * 
                      (IMAGE_HEIGHT-FRAME_HEIGHT));
    }

    // return the pan in degree given topLeftX
    private int topLeft2PanDegree(int topLeftX) {
        return (int) (RoboConstant.LEFT_PAN_DEGREE - 
                     (topLeftX/(double)(IMAGE_WIDTH-FRAME_WIDTH) * 
                     (RoboConstant.LEFT_PAN_DEGREE+RoboConstant.RIGHT_PAN_DEGREE)));
    }

    // return the tilt in degree given topLeftY
    private int topLeft2TiltDegree(int topLeftY) {
        // up is positive (because Behaviours is)
        return (int) (RoboConstant.UP_TILT_DEGREE-
                     (topLeftY/(double)(IMAGE_HEIGHT-FRAME_HEIGHT) * 
                     (RoboConstant.UP_TILT_DEGREE+RoboConstant.DOWN_TILT_DEGREE)));
    }
    
    // get methods used in RoboCommander
    public JTextField getPlayBackFileField() {
        return playBackFileField;
    }
    public JButton getLoadButton() {
        return loadButton;
    }
    public void setCommands(String name, String value) {
        nameField.setText(name);
        valueField.setText(value);
    }

    // read oplane info from socket
    private class OPlaneReceiver extends Thread {
        
        public void run() {
            try {

                // accept a client
                ServerSocket ss = new ServerSocket(oplanePort);
                System.out.println("HumanControl (receive," + oplanePort + "): ready");
                Socket sock    = ss.accept();
                InputStream in = sock.getInputStream();
                ss.close();
                
                int read, numRead;
                int backupObjectSize, dataTotalSize;
                boolean dropPacket;
                
                byte sizeBuffer[] = new byte[4];
                byte[] localBuff = new byte[RoboConstant.OBJ_SIZE];
                
                oplaneDrawer.start();

                while (true) {
                    dropPacket = false;
                    
                    if (!canDrawBuffer1) {
                        readInBuffer1 = true;
                        localBuff     = buffer1;
                    } else if (!canDrawBuffer2) {
                        readInBuffer2 = true;
                        localBuff     = buffer2;
                    } else {
                        localBuff = new byte[RoboConstant.OBJ_SIZE];
                        dropPacket = true;
                    }
                    
                    if (bufferDebugMsg) {
                        if (!dropPacket) {
                            if (readInBuffer1) {
                                System.out.println("reading in buffer 1");
                            } else if (readInBuffer2) {
                                System.out.println("reading in buffer 2");
                            }
                        }
                    }

                    // read the number of object
                    read = 0;
                    while (read != sizeBuffer.length) {
                        try {
                            numRead = in.read(sizeBuffer, read, sizeBuffer.length-read);
                            if (numRead <= 0) {
                                ss.close();
                                return;
                            }
                            read += numRead;
                        } catch (ArrayIndexOutOfBoundsException e) {
                            System.out.println("HumanControl.java: error reading object data.");
                            e.printStackTrace();
                            System.exit(1);
                        }
                    }
                    if (readInBuffer1) {
                        dataBufferSize1  = RoboUtility.bytesToInt(sizeBuffer, 0);
                        backupObjectSize = dataBufferSize1;
                        if (bufferDebugMsg) {
                            System.out.println("dataBufferSize1: " + dataBufferSize1 + "; dataTotalSize: " + dataTotalSize);
                        }
                    } else if (readInBuffer2) {
                        dataBufferSize2 = RoboUtility.bytesToInt(sizeBuffer, 0);
                        backupObjectSize = dataBufferSize2;
                        if (bufferDebugMsg) {
                            System.out.println("dataBufferSize2: " + dataBufferSize2 + "; dataTotalSize: " + dataTotalSize);
                        }
                    } else {
                        // use temporary variable to store the object size
                        backupObjectSize = RoboUtility.bytesToInt(sizeBuffer, 0);
                    }
                    dataTotalSize   = RoboConstant.ONE_OBJ_SIZE * backupObjectSize;
                    // add spaces for the pan and tilt (the doubles are stored as INT_BYTES)
                    dataTotalSize += RoboConstant.INT_BYTES * RoboConstant.NUM_ROBOT_DATA;

                    // read the object data
                    read = 0;
                    while (read != dataTotalSize) {
                        try {
                            numRead = in.read(localBuff, read, dataTotalSize-read);
                            if (numRead <= 0) {
                                ss.close();
                                return;
                            }
                            read += numRead;
                        } catch (ArrayIndexOutOfBoundsException e) {
                            System.out.println("HumanControl.java: error reading object data.");
                            e.printStackTrace();
                            System.exit(1);
                        }
                    }

                    if (bufferDebugMsg) {
                        if (!dropPacket) {
                            if (readInBuffer1) {
                                System.out.println("reading in buffer 1 **");
                            } else if (readInBuffer2) {
                                System.out.println("reading in buffer 2 **");
                            }
                        }
                    }

                    if (dropPacket && bufferDebugMsg) {
                        System.out.println("Packet dropped");
                    } else if (readInBuffer1) {
                        readInBuffer1  = false;
                        canDrawBuffer1 = true;
                    } else if (readInBuffer2) {
                        readInBuffer2  = false;
                        canDrawBuffer2 = true;
                    }
                    
                }

            } catch (IOException e) {
                e.printStackTrace();
            }

        }

    }

    // draw the cplane from buffer
    private class OPlaneDrawer extends Thread {

        public void run() {
            if (debugMsg) {
                System.out.println("Drawer started()");
            }
            while (true) {
                if (!readInBuffer1 && canDrawBuffer1) {
                    if (bufferDebugMsg) {
                        System.out.println("----- drawing buffer 1");
                    }
                    drawOPlane(buffer1, dataBufferSize1);
                    canDrawBuffer1 = false;
                    if (bufferDebugMsg) {
                        System.out.println("----- drawing buffer 1 **");
                    }
                } else if (!readInBuffer2 && canDrawBuffer2) {
                    if (bufferDebugMsg) {
                        System.out.println("----- drawing buffer 2");
                    }
                    drawOPlane(buffer2, dataBufferSize2);
                    canDrawBuffer2 = false;
                    if (bufferDebugMsg) {
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
    
    private class OPlane {
        Plane[] planes;
        String name, value;

        OPlane(Vector v, String name, String value) {
            planes = new Plane[v.size()];
            v.copyInto(planes);
            this.name  = name;
            this.value = value;
        }

    }

    private class Plane {
        double[] data;

        Plane(String[] array, String pan, String tilt) {
            int i;
            data = new double[LOG_FILE_PLANE_LEN+2];
            for (i=0; i<data.length-2; i++) {
                data[i] = Integer.parseInt(array[i]);
            }
            data[data.length-2] = Double.parseDouble(pan);
            data[data.length-1] = Double.parseDouble(tilt);
        }

        public String toString() {
            String s = "";
            for (int i=0; i<data.length; i++) {
                s += data[i] + " ";
            }
            return s;
        }
    }
}
