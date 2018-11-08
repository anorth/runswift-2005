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
 * $Id: RoboCommander.java 2090 2003-11-01 16:44:49Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.*;


public class RoboCommander extends JFrame {

    private static final boolean debugMsg      = false;
    private static final boolean delayDebugMsg = false;
    private static final boolean hasTestButton = false;
    private static final boolean showInfoPanel = true;
    private static final boolean replaceLongCommandDebugMsg = false;

    // world model display constants
    private static final int MAX_NUM_PLAYER                = 4;
    private static final int WORLD_MODEL_DISPLAY_PORT_BASE = 5019;
    private int[] WORLD_MODEL_DISPLAY_PORTS                = new int[MAX_NUM_PLAYER];
    private static final String DEFAULT_PLAYER             = "1";

    private static final boolean LATENCY_TEST3      = false;
    private static final boolean LATENCY_TEST4      = false;
    private static int trial3                       = 0;
    private static int trial4                       = 0;



    CommandSender commandSender = new CommandSender("RoboCommander");

    JScrollPane scrollPane    = new JScrollPane();
    JTextArea dataInfoArea    = new JTextArea();
    JPanel userPanel          = new JPanel();

    JMenuBar menuBar        = new JMenuBar();
    JMenu menu              = new JMenu("RoboCommander");
    JMenuItem cplaneControl = new JMenuItem("CPlane Control Interface");
    JMenuItem humanControl  = new JMenuItem("Human Control Interface");

    // GUI for the commands
    JPanel commandPanel        = new JPanel();
    TitledBorder commandBorder = new TitledBorder("Command Panel");
    JLabel nameLabel           = new JLabel("Name:");
    JLabel valueLabel          = new JLabel("Value:");
    JTextField nameField       = new JTextField(RoboConstant.MODE_TRIGGER);
    JTextField valueField      = new JTextField("0");
    JComboBox walktypeList     = new JComboBox();
    JButton sendButton         = new JButton("Send");
    JButton motionButton       = new JButton("Motion");
    JButton testButton         = new JButton("Test");
    JCheckBox controlCheckBox  = new JCheckBox("Manual Control", false);

    // GUI for walk parameters
    JPanel walkPanel           = new JPanel();
    TitledBorder walkBorder    = new TitledBorder("Walk Parameters");

    JPanel walkPanel1          = new JPanel();
    JPanel walkPanel2          = new JPanel();
    JLabel hfLabel             = new JLabel("hf:");
    JLabel hbLabel             = new JLabel("hb:");
    JLabel hdfLabel            = new JLabel("hdf:");
    JLabel hdbLabel            = new JLabel("hdb:");
    JLabel ffoLabel            = new JLabel("ffo:");
    JLabel fsoLabel            = new JLabel("fso:");
    JLabel bfoLabel            = new JLabel("bfo:");
    JLabel bsoLabel            = new JLabel("bso:");
    JLabel PGLabel             = new JLabel("PG:");
    JTextField hfField         = new JTextField("90");
    JTextField hbField         = new JTextField("120");
    JTextField hdfField        = new JTextField("10");
    JTextField hdbField        = new JTextField("25");
    JTextField ffoField        = new JTextField("55");
    JTextField fsoField        = new JTextField("15");
    JTextField bfoField        = new JTextField("-55");
    JTextField bsoField        = new JTextField("5");
    JTextField PGField         = new JTextField("40");
    JComboBox playerList       = new JComboBox();
    JButton setButton          = new JButton("Set Params");
    JButton controlButton      = new JButton("Control");

    // GUI for the command top panels (shortcuts)
    JPanel quickKeyPanel        = new JPanel();
    TitledBorder quickKeyBorder = new TitledBorder("Quick Key Panel");

    JPanel quickKeyPanel1       = new JPanel();
    JButton pauseLocaliseButton = new JButton("Pause Game and localise");
    JButton startGameButton     = new JButton("Start Game");
    JButton stopGameButton      = new JButton("Stop Game");

    JPanel quickKeyPanel2       = new JPanel();
    JLabel playerLabel          = new JLabel("Player: ");
    JButton worldModelButton    = new JButton("World Model");
    JButton cplaneButton        = new JButton("CPlane");
    JButton burstButton         = new JButton("BURST");
    JButton yuvplaneButton      = new JButton("YUVPlane");
    JButton oplaneButton        = new JButton("OPlane");

    // stores forward, left, turn, tilt, tilt2, pan values
    private float speeds[] = {0,0,0,0,0,0};

    RoboWirelessBase parent;
    int sendPort, receivePort;
    OutputStream out;
    PrintWriter startTime, endTime;
    int currentCommand;

    public RoboCommander(RoboWirelessBase parent, int sendPort, int receivePort) {

        super("RoboCommander");
        this.parent      = parent;
        this.sendPort    = sendPort;
        this.receivePort = receivePort;
        try {

            commandSender.start();
            if (parent.socket_frameRate) {
                (new FrameRateDisplay()).start();
            }

            // initialise port numbers for world model display
            for (int i = 0; i < MAX_NUM_PLAYER; i++) {
                WORLD_MODEL_DISPLAY_PORTS[i] = WORLD_MODEL_DISPLAY_PORT_BASE + i;
            }

            WorldModelDisplay[] wmDisplays = new WorldModelDisplay[MAX_NUM_PLAYER];

            // establish threads for world model display
            for (int i = 0; i < MAX_NUM_PLAYER; i++) {
                Integer portNumber = new Integer(WORLD_MODEL_DISPLAY_PORTS[i]);
                wmDisplays[i] = new WorldModelDisplay(portNumber.toString());
            }

            // start threads for world model display
            for (int i = 0; i < MAX_NUM_PLAYER; i++) {
                wmDisplays[i].start();
            }

            init();
            if (showInfoPanel) {
                setSize(650,600);
            } else {
                pack();
            }
            show();
        } catch(Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    private void init() throws Exception {
        menu.setMnemonic(KeyEvent.VK_R);
        menu.setFont(new Font("Dialog", 1, 24));

        cplaneControl.setMnemonic(KeyEvent.VK_C);
        cplaneControl.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C, ActionEvent.CTRL_MASK));
        cplaneControl.setToolTipText("Show the CPlane Control interface");
        cplaneControl.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                showCPlaneControl();
            }
        });
        humanControl.setMnemonic(KeyEvent.VK_H);
        humanControl.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_H, ActionEvent.CTRL_MASK));
        humanControl.setToolTipText("Show the HumanControl RoboCommander interface");
        humanControl.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                showHumanControl();
            }
        });

        menu.add(cplaneControl);
        menu.add(humanControl);

        menuBar.add(menu);

        if (showInfoPanel) {
            dataInfoArea.setFont(new Font("Monospaced", 0, 12));
            scrollPane.getViewport().add(dataInfoArea);
            scrollPane.setAutoscrolls(true);
        }

        nameField.setMinimumSize(new Dimension(60, 21));
        nameField.setPreferredSize(new Dimension(60, 21));
        nameField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                nameField.selectAll();
            }
        });
        nameField.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send();
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
                commandSender.send();
            }
        });
        sendButton.setMnemonic(KeyEvent.VK_S);
        sendButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send();
            }
        });
        motionButton.setMnemonic(KeyEvent.VK_M);
        motionButton.addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                moveDog(e);
            }
        });
        controlCheckBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (controlCheckBox.isSelected()) {
                    // take control
                    walktypeList.setEnabled(true);
                    commandSender.send(RoboConstant.CONTROL_TRIGGER, RoboConstant.CONTROL_START_VALUE);
                } else {
                    // release control
                    walktypeList.setEnabled(false);
                    commandSender.send(RoboConstant.CONTROL_TRIGGER, RoboConstant.CONTROL_END_VALUE);
                }
            }
        });
        if (hasTestButton) {
            if (LATENCY_TEST4) {
                try {
                    startTime = new PrintWriter(new BufferedWriter(new FileWriter("04start")));
                } catch (Exception ex1) {
                    System.out.println("Can't create file 04start");
                }
                try {
                    endTime = new PrintWriter(new BufferedWriter(new FileWriter("04end")));
                } catch (Exception ex) {
                    System.out.println("Can't write file 04end");
                }
            }
            testButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    int numTimes = 10;
                    int[] speeds = {1,2,3,4,5,6,7,8,7,6,5,4,3,2,1,0};

                    if (LATENCY_TEST4) {
                        long time = System.currentTimeMillis();
                        startTime.println("t4start, trial #"+trial4+"="+time);
                        startTime.flush();
                    }

                    for (int j=0 ; j<numTimes ; j++) {
                        for (int i=0; i<speeds.length ; i++) {
                            send("f",speeds[i]+"");
                            if (debugMsg) {
                                System.out.println("Sent: "+"f " + speeds[i]);
                            }
                        }
                    }
                    if (LATENCY_TEST4) {
                        long time = System.currentTimeMillis();
                        endTime.println("t4end, trial #"+trial4+"="+time);
                        endTime.flush();
                        trial4++;
                    }
                }
            });
        }

        commandBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        commandPanel.setBorder(commandBorder);
        commandPanel.add(nameLabel);
        commandPanel.add(nameField);
        commandPanel.add(valueLabel);
        commandPanel.add(valueField);
        commandPanel.add(sendButton);
        commandPanel.add(motionButton);
        commandPanel.add(controlCheckBox);
        if (hasTestButton) {
            commandPanel.add(testButton);
        }

        walktypeList.setEnabled(false);
        for (int i=0; i<RoboConstant.WALKTYPES.length; ++i) {
            walktypeList.addItem(RoboConstant.WALKTYPES[i]);
        }
        walktypeList.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.WALKTYPE_CHANGE, walktypeList.getSelectedIndex() + "");
            }
        });

        PGField.setMinimumSize(new Dimension(35, 21));
        PGField.setPreferredSize(new Dimension(35, 21));
        PGField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                PGField.selectAll();
            }
        });
        hfField.setMinimumSize(new Dimension(35, 21));
        hfField.setPreferredSize(new Dimension(35, 21));
        hfField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                hfField.selectAll();
            }
        });
        hbField.setMinimumSize(new Dimension(35, 21));
        hbField.setPreferredSize(new Dimension(35, 21));
        hbField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                hbField.selectAll();
            }
        });
        hdfField.setMinimumSize(new Dimension(35, 21));
        hdfField.setPreferredSize(new Dimension(35, 21));
        hdfField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                hdfField.selectAll();
            }
        });
        hdbField.setMinimumSize(new Dimension(35, 21));
        hdbField.setPreferredSize(new Dimension(35, 21));
        hdbField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                hdbField.selectAll();
            }
        });
        ffoField.setMinimumSize(new Dimension(35, 21));
        ffoField.setPreferredSize(new Dimension(35, 21));
        ffoField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                ffoField.selectAll();
            }
        });
        fsoField.setMinimumSize(new Dimension(35, 21));
        fsoField.setPreferredSize(new Dimension(35, 21));
        fsoField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                fsoField.selectAll();
            }
        });
        bfoField.setMinimumSize(new Dimension(35, 21));
        bfoField.setPreferredSize(new Dimension(35, 21));
        bfoField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                bfoField.selectAll();
            }
        });
        bsoField.setMinimumSize(new Dimension(35, 21));
        bsoField.setPreferredSize(new Dimension(35, 21));
        bsoField.addFocusListener(new FocusAdapter() {
            public void focusGained(FocusEvent e) {
                bsoField.selectAll();
            }
        });
        setButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    commandSender.send(RoboConstant.WALKPARAM1_CHANGE,
                                       hfField.getText() + " " +
                                       hbField.getText() + " " +
                                       hdfField.getText());
                    Thread.sleep(50);

                    commandSender.send(RoboConstant.WALKPARAM2_CHANGE,
                                       hdbField.getText() + " " +
                                       ffoField.getText() + " " +
                                       fsoField.getText());
                    Thread.sleep(50);
                    commandSender.send(RoboConstant.WALKPARAM3_CHANGE,
                                       bfoField.getText() + " " +
                                       bsoField.getText() + " " +
                                       PGField.getText());
                }
                catch (InterruptedException ie) {
                }
            }
        });
        walkBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        walkPanel.setBorder(walkBorder);
        walkPanel.setLayout(new BorderLayout());

        walkPanel1.add(hfLabel);
        walkPanel1.add(hfField);
        walkPanel1.add(hbLabel);
        walkPanel1.add(hbField);
        walkPanel1.add(hdfLabel);
        walkPanel1.add(hdfField);
        walkPanel1.add(hdbLabel);
        walkPanel1.add(hdbField);
        walkPanel1.add(ffoLabel);
        walkPanel1.add(ffoField);
        walkPanel1.add(fsoLabel);
        walkPanel1.add(fsoField);

        walkPanel2.add(bfoLabel);
        walkPanel2.add(bfoField);
        walkPanel2.add(bsoLabel);
        walkPanel2.add(bsoField);
        walkPanel2.add(PGLabel);
        walkPanel2.add(PGField);
        walkPanel2.add(walktypeList);
        walkPanel2.add(setButton);
        walkPanel2.add(controlButton);

        walkPanel.add(walkPanel1, BorderLayout.NORTH);
        walkPanel.add(walkPanel2, BorderLayout.SOUTH);

        pauseLocaliseButton.setMnemonic(KeyEvent.VK_P);
        pauseLocaliseButton.setToolTipText("(" + RoboConstant.MODE_TRIGGER + ", 5)");
        pauseLocaliseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.MODE_TRIGGER, "5");
            }
        });
        startGameButton.setMnemonic(KeyEvent.VK_G);
        startGameButton.setToolTipText("(" + RoboConstant.MODE_TRIGGER + ", 0)");
        startGameButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.MODE_TRIGGER, "0");
            }
        });
        stopGameButton.setMnemonic(KeyEvent.VK_T);
        stopGameButton.setToolTipText("(" + RoboConstant.MODE_TRIGGER + ", 1)");
        stopGameButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.MODE_TRIGGER, "1");
            }
        });

        quickKeyPanel1.add(pauseLocaliseButton);
        quickKeyPanel1.add(startGameButton);
        quickKeyPanel1.add(stopGameButton);

        worldModelButton.setMnemonic(KeyEvent.VK_W);
        worldModelButton.setToolTipText("(" + RoboConstant.WORLDMODEL_TRIGGER + ", " + DEFAULT_PLAYER + ")");
        worldModelButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.WORLDMODEL_TRIGGER, (String) playerList.getSelectedItem());
                switchVisiblePlayer();
            }
        });
        cplaneButton.setMnemonic(KeyEvent.VK_C);
        cplaneButton.setToolTipText("(" + RoboConstant.CPLANE_TRIGGER + ", " + DEFAULT_PLAYER + ")");
        cplaneButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.CPLANE_TRIGGER, (String) playerList.getSelectedItem());
                switchVisiblePlayer();
            }
        });
        burstButton.setToolTipText("(" + RoboConstant.BURST_TRIGGER + ", " + DEFAULT_PLAYER + ")");
        burstButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.BURST_TRIGGER, (String) playerList.getSelectedItem());
            }
        });
        yuvplaneButton.setMnemonic(KeyEvent.VK_Y);
        yuvplaneButton.setToolTipText("(" + RoboConstant.YUVPLANE_TRIGGER + ", " + DEFAULT_PLAYER + ")");
        yuvplaneButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.YUVPLANE_TRIGGER, (String) playerList.getSelectedItem());
            }
        });
        oplaneButton.setMnemonic(KeyEvent.VK_O);
        oplaneButton.setToolTipText("(" + RoboConstant.OPLANE_TRIGGER + ", " + DEFAULT_PLAYER + ")");
        oplaneButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                commandSender.send(RoboConstant.OPLANE_TRIGGER, (String) playerList.getSelectedItem());
                showHumanControl();
                switchVisiblePlayer();
            }
        });
        for (int i=0; i<=MAX_NUM_PLAYER; i++) {
            playerList.addItem(""+i);
        }
        playerList.setSelectedItem(DEFAULT_PLAYER);
        playerList.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                worldModelButton.setToolTipText("(" + RoboConstant.WORLDMODEL_TRIGGER + ", " + (String) playerList.getSelectedItem() + ")");
                cplaneButton.setToolTipText("(" + RoboConstant.CPLANE_TRIGGER + ", " + (String) playerList.getSelectedItem() + ")");
                oplaneButton.setToolTipText("(" + RoboConstant.OPLANE_TRIGGER + ", " + (String) playerList.getSelectedItem() + ")");
            }
        });
        quickKeyPanel2.add(playerLabel);
        quickKeyPanel2.add(playerList);
        quickKeyPanel2.add(worldModelButton);
        quickKeyPanel2.add(cplaneButton);
        quickKeyPanel2.add(burstButton);
        quickKeyPanel2.add(yuvplaneButton);
        quickKeyPanel2.add(oplaneButton);

        quickKeyBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 14));
        quickKeyPanel.setBorder(quickKeyBorder);
        quickKeyPanel.setLayout(new BorderLayout());
        quickKeyPanel.add(quickKeyPanel1, BorderLayout.CENTER);
        quickKeyPanel.add(quickKeyPanel2, BorderLayout.SOUTH);

        userPanel.setLayout(new BorderLayout());
        userPanel.add(commandPanel, BorderLayout.NORTH);
        userPanel.add(walkPanel, BorderLayout.CENTER);
        userPanel.add(quickKeyPanel, BorderLayout.SOUTH);

        this.setDefaultCloseOperation(3);
        if (showInfoPanel) {
            this.getContentPane().add(menuBar, BorderLayout.NORTH);
            this.getContentPane().add(scrollPane, BorderLayout.CENTER);
            this.getContentPane().add(userPanel, BorderLayout.SOUTH);
        } else {
            this.getContentPane().add(menuBar, BorderLayout.NORTH);
            this.getContentPane().add(userPanel, BorderLayout.CENTER);
        }
    }

    private void switchVisiblePlayer() {
        if (playerList.getSelectedIndex()==0) {
            playerList.setSelectedItem(DEFAULT_PLAYER);
        } else {
            playerList.setSelectedIndex(0);
        }
    }

    private void showHumanControl() {
        parent.getHumanControl().setVisible(true);
    }

    private void showCPlaneControl() {
        parent.getCPlaneDisplay().setVisible(true);
    }

    public void printValues() {
        for (int i = 0; i < speeds.length; i++) {
            System.out.print(speeds[i] + " ");
        }
        System.out.println();
    }

    public void printToConsole(String line) {
        if (showInfoPanel) {
            dataInfoArea.append(line + "\n");
            dataInfoArea.setCaretPosition(dataInfoArea.getDocument().getLength());
        }
    }


    // utility methods to be used in other java files
    public void send(String name) {
        commandSender.send(name, "");
    }
    public void send(String name, String value) {
        commandSender.send(name, value);
    }

    public void setCommands(String name, String value) {
        nameField.setText(name);
        valueField.setText(value);
    }
    public void moveDog(KeyEvent e) {
        int speed, mult;
        int kc = e.getKeyCode();

        if (kc==KeyEvent.VK_PLUS || kc==KeyEvent.VK_EQUALS) {
            // increase speed
            if ((RoboConstant.KEYS[currentCommand]==RoboConstant.K_FORWARD ||
                 RoboConstant.KEYS[currentCommand]==RoboConstant.K_LEFT) &&
                speeds[currentCommand/2]<RoboConstant.MAX_FORWARD_SPEED) {
                speeds[currentCommand/2] += RoboConstant.FORWARD_STEP;
            } else if ((RoboConstant.KEYS[currentCommand]==RoboConstant.K_BACKWARD ||
                        RoboConstant.KEYS[currentCommand]==RoboConstant.K_RIGHT) &&
                       speeds[currentCommand/2]>-RoboConstant.MAX_FORWARD_SPEED) {
                speeds[currentCommand/2] -= RoboConstant.FORWARD_STEP;
            } else if (RoboConstant.KEYS[currentCommand]==RoboConstant.K_TURNCCW &&
                       speeds[currentCommand/2]<RoboConstant.MAX_TURN_SPEED) {
                speeds[currentCommand/2] += RoboConstant.TURN_STEP;
            } else if (RoboConstant.KEYS[currentCommand]==RoboConstant.K_TURNCW &&
                       speeds[currentCommand/2]>-RoboConstant.MAX_TURN_SPEED) {
                speeds[currentCommand/2] -= RoboConstant.TURN_STEP;
            }
            commandSender.send(RoboConstant.COMMANDS[currentCommand], speeds[currentCommand/2] + "");
            return;
        } else if (kc==KeyEvent.VK_MINUS) {
            // reduce speed
            if ((RoboConstant.KEYS[currentCommand]==RoboConstant.K_FORWARD ||
                 RoboConstant.KEYS[currentCommand]==RoboConstant.K_LEFT) &&
                speeds[currentCommand/2]>-RoboConstant.MAX_FORWARD_SPEED) {
                speeds[currentCommand/2] -= RoboConstant.FORWARD_STEP;
            } else if ((RoboConstant.KEYS[currentCommand]==RoboConstant.K_BACKWARD ||
                        RoboConstant.KEYS[currentCommand]==RoboConstant.K_RIGHT) &&
                       speeds[currentCommand/2]<RoboConstant.MAX_FORWARD_SPEED) {
                speeds[currentCommand/2] += RoboConstant.FORWARD_STEP;
            } else if (RoboConstant.KEYS[currentCommand]==RoboConstant.K_TURNCCW &&
                       speeds[currentCommand/2]>-RoboConstant.MAX_TURN_SPEED) {
                speeds[currentCommand/2] -= RoboConstant.TURN_STEP;
            } else if (RoboConstant.KEYS[currentCommand]==RoboConstant.K_TURNCW &&
                       speeds[currentCommand/2]<RoboConstant.MAX_TURN_SPEED) {
                speeds[currentCommand/2] += RoboConstant.TURN_STEP;
            }
            commandSender.send(RoboConstant.COMMANDS[currentCommand], speeds[currentCommand/2] + "");
            return;
        }
        // test which key is pressed
        for (int i = 0; i<RoboConstant.KEYS.length; i++) {
            if (kc==RoboConstant.KEYS[i]) {
                if (i>=RoboConstant.STANCE_KEYS_INDEX) {
                    if (e.isControlDown() && RoboConstant.COMMANDS[i].equals(RoboConstant.C_NORMAL)) {
                        commandSender.send(RoboConstant.COMMANDS[i].toLowerCase(), RoboConstant.KICK_VALUES[i-RoboConstant.STANCE_KEYS_INDEX]+"");
                    } else {
                        commandSender.send(RoboConstant.COMMANDS[i], RoboConstant.KICK_VALUES[i-RoboConstant.STANCE_KEYS_INDEX]+"");
                    }
                } else {

                    currentCommand = i;

                    // otherwise need to take care of reverse movement and speed
                    speed = i/2 * 2;
                    mult  = 1 - i%2*2;
                    if (e.isShiftDown()) {
                        // slow
                        speed++;
                    }

                    // if diff motion then send command
                    if (speeds[i/2] != mult*RoboConstant.SPEEDS[speed]) {
                        if ((speeds[i/2]>0 && mult*RoboConstant.SPEEDS[speed]<0) ||
                            (speeds[i/2]<0 && mult*RoboConstant.SPEEDS[speed]>0)) {
                            speeds[i/2] = 0;
                        } else {
                            speeds[i/2] = mult*RoboConstant.SPEEDS[speed];
                        }

                        commandSender.send(RoboConstant.COMMANDS[i], speeds[i/2] + "");
                    }
                }
                break;
            }

        }

        if (debugMsg) {
            System.out.println("pressed = " + kc );
        }
    }


    // for sending commands
    private class CommandSender extends Thread {
        private String filename;
        OutputStream out;

        public CommandSender(String filename) {
            this.filename = filename;
        }

        public void run() {
            try {
                ServerSocket ss = new ServerSocket(sendPort);
                System.out.println(filename + " (send," + sendPort + "): ready");
                while (true){
                    Socket s = ss.accept();
                    out = s.getOutputStream();
                }
//                ss.close(); //FIXME:enable the thread to stop, and close serversocket
            } catch(Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }

        private void checkStartCPlaneLog(String name, String value) {
            if (name.trim().equals(RoboConstant.CPLANE_TRIGGER) && RoboUtility.valueType(value)==RoboConstant.START_TYPE) {
                parent.getCPlaneDisplay().getPlayBackFileField().setEnabled(false);
                parent.getCPlaneDisplay().getLoadButton().setEnabled(false);
            } else if (name.trim().equals(RoboConstant.CPLANE_TRIGGER) && RoboUtility.valueType(value)==RoboConstant.STOP_TYPE) {
                parent.getCPlaneDisplay().getPlayBackFileField().setEnabled(true);
                parent.getCPlaneDisplay().getLoadButton().setEnabled(true);
            }
        }


        private void checkStartOPlaneLog(String name, String value) {
            if (name.trim().equals(RoboConstant.OPLANE_TRIGGER) && RoboUtility.valueType(value)==RoboConstant.START_TYPE) {
                parent.getHumanControl().getPlayBackFileField().setEnabled(false);
                parent.getHumanControl().getLoadButton().setEnabled(false);
            } else if (name.trim().equals(RoboConstant.OPLANE_TRIGGER) && RoboUtility.valueType(value)==RoboConstant.STOP_TYPE) {
                parent.getHumanControl().getPlayBackFileField().setEnabled(true);
                parent.getHumanControl().getLoadButton().setEnabled(true);
            }
        }

        // sends what ever is in the text fields
        public void send() {
            // something to send
            if (!nameField.getText().equals("") && !valueField.getText().equals("")) {
                send(nameField.getText(), valueField.getText());
            }
        }

        // name and value must be shorter than 255
        public void send(String name, String value) {
            try {
                if (out==null) {
                    System.out.println(filename + " (send," + sendPort + "): not ready yet");
                    return;
                }

                long timeNow = System.currentTimeMillis();
                if (name.equals("q")) {
                    parent.getHumanControl().ballTrkStr = value;
                } else if (name.equals("k")) {
                    StringTokenizer st = new StringTokenizer(value);
                    parent.getHumanControl().head_x = st.nextToken();
                    parent.getHumanControl().head_y = st.nextToken();
                }
                if (parent.getHumanControl() != null)
                    parent.getHumanControl().writeLog("#"+timeNow+":" + name + "," + value + "\n", 2);

                if (parent.socket_cplaneDisplay)
                    checkStartCPlaneLog(name, value);

                checkStartOPlaneLog(name, value);

                // replace commands with something shorter
                if (name.length()>=RoboConstant.NAME_SIZE) {
                    if (replaceLongCommandDebugMsg) {
                        System.out.println("RoboCommander.java: replaced " + name + " with " + name.substring(0, RoboConstant.NAME_SIZE-1));
                    }
                    name = name.substring(0, RoboConstant.NAME_SIZE-1);
                }

                // replace values with something shorter
                if (value.length()>=RoboConstant.VALUE_SIZE) {
                    if (debugMsg) {
                        System.out.println("RoboCommander.java: replaced " + value + " with " + value.substring(0, RoboConstant.VALUE_SIZE-1));
                    }
                    value = value.substring(0, RoboConstant.VALUE_SIZE-1);
                }

                byte[] len1 = new byte[1];
                byte[] len2 = new byte[1];

                len1[0] = (byte) name.length();
                len2[0] = (byte) value.length();

                if (debugMsg) {
                    System.out.println("len12 " + len1 + " " + len2);
                }

                if (LATENCY_TEST3) {
                   long time = System.currentTimeMillis();
                   System.out.println("t3start, trial #"+trial3+"="+time);
                   trial3++;
                }

                out.write(len1);
                out.write(name.getBytes(),0,name.length());
                out.write(len2);
                out.write(value.getBytes(),0,value.length());

                if (delayDebugMsg) {
                    System.out.println("RoboCommander: (" + name + ", " + value + ")");
                }

            } catch (IOException e) {
                e.printStackTrace();
                System.exit(1);
            }
        }

    }

    // receives frame rate info from client
    private class FrameRateDisplay extends Thread {

        public void run() {

            try {
                ServerSocket ss = new ServerSocket(receivePort);
                System.out.println("FrameRateDisplay (receive," + receivePort + "): ready");
                while (true){
                    Socket s = ss.accept();
                    System.out.println("FrameRateDisplay: Got new connection");
                    BufferedReader in = new BufferedReader(new
                        InputStreamReader(s.getInputStream()));


                    double[] data = new double[200]; // temporary for now
                    int loc = 0; // current loc in data array
                    boolean inData = false;

                    for (String line = in.readLine(); line != null;
                         line = in.readLine()) {
                        dataInfoArea.append(line + "\n");
                        dataInfoArea.setCaretPosition(dataInfoArea.getDocument().
                            getLength());
                    }

                    System.out.println("FrameRateDisplay: Connection Closed");
                    in.close();
                    s.close();
                }
//                ss.close(); //FIXME:enable the thread to stop, and close serversocket
            } catch (IOException e) {
                System.err.println("GLDisplay.Reader.run: IOException");
                e.printStackTrace();
            }

        }
    }

    // receives world model display data from client

}
