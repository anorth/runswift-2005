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


package Tool;

import java.io.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

import RC.*;
import RoboComm.*;
import RoboShare.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0 :
* CHANGE LOG
*  + version 1.1
*     - Be able to get YUV stream from robot, reason about object it seen, if
* objects doesn't make sense, stop and let user correct it.
*  + version 1.4:
*     - Convenient UI for sending commands
 */

public class RobotController extends JFrame implements Observer, YUVListener{
    static public boolean bNoListening = false;
    static public final int CONTROL_IDLE = 0;
    static public final int CONTROL_RECEIVING_NEW = 1;
    static public final int CONTROL_VISION_PROCESSED = 2;
    static public final int CONTROL_STOP_TO_ANALYSE = 3;

    static public final int MODE_FALSE_NEGATIVE = 0;
    static public final int MODE_FALSE_POSITIVE = 1;

    static public final int INPUT_BLOB_RDR = 0;
    static public final int INPUT_OBJECT_RDR = 1;

    static private String [] DOG_COMMAND = {
        "1 mode HEAD_TURN",      //0
        "1 mode IDLE",
        "1 mode HEAD_TURN",      //2
        "1 mode BALL_TRACK",
        "1 mode RANDOM_WALK",     //4
        "1 critical",
        "1 mode turn",            //6
        "1 mode center",
        "1 eval VisionLink.printRDR()", //8
        "1 mode back",
    };

    static private final int comSTART_DOG = 0;
    static private final int comSTOP_DOG = 1;
    static private final int comSPIN_HEAD = 2;
    static private final int comBALL_TRACK = 3;
    static private final int comRANDOM_WALK = 4;
    static private final int comONLY_SEND_CRITICAL_SWITCH = 5;
    static private final int comTURN_AROUND = 6;
    static private final int comGO_TO_CENTER = 7;
    static private final int comPRINT_RDR = 8;
    static private final int comSTEP_BACK = 9;

    static final String [] STATUS_TO_STRING = {
        "Idle",
        "New YUV",
        "YUV Processed",
        "Stoped"
    };
    BorderLayout borderLayout1 = new BorderLayout();
    JTabbedPane jTabbedPane1 = new JTabbedPane();
    JPanel jPanel1 = new JPanel();
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JPanel jPanel2 = new JPanel();
    JPanel jPanel3 = new JPanel();
    JLabel jLabel1 = new JLabel();
    JComboBox cbxType = new JComboBox();
    JTextField jtfName = new JTextField();
    JTextField jtfValue = new JTextField();
    JLabel jLabel2 = new JLabel();
    GridBagLayout gridBagLayout2 = new GridBagLayout();
    JTextArea jtaMessageReceived = new JTextArea();
    JButton btSend = new JButton();
    Border border1;
    TitledBorder titledBorder1;
    Border border2;
    Border border3;
    TitledBorder titledBorder2;
    Border border4;
    JPanel jPanel4 = new JPanel();
    JButton btExit = new JButton();

    static public final int SENDER_PORT = 5005;
    static public final int RECEIVER_PORT = 5006;

    static public final int BUFFER_LEN = 1024;
    CommandSender commandSender = null;
    MessageReceiver messageReceiver = null;
    JButton btListen = new JButton();
    JScrollPane jScrollPane1 = new JScrollPane();
    JPanel jPanel5 = new JPanel();
    GridBagLayout gridBagLayout3 = new GridBagLayout();
    JPanel jPanel6 = new JPanel();
    JPanel jPanel7 = new JPanel();
    JScrollPane jScrollPane2 = new JScrollPane();
    JTextArea jtaStatus = new JTextArea();
    JButton btStartControl = new JButton();
    GridBagLayout gridBagLayout4 = new GridBagLayout();

    YUVStreamClient yuvReceiver = null;
    RobolinkReceiver robolinkReceiver = new RobolinkReceiver();


    int controlState = CONTROL_IDLE;

    RDRApplication parent = null;
    JPanel jPanel8 = new JPanel();
    GridBagLayout gridBagLayout5 = new GridBagLayout();
    JLabel jLabel4 = new JLabel();
    Border border5;
    TitledBorder titledBorder3;
    Border border6;
    GridBagLayout gridBagLayout6 = new GridBagLayout();
    JComboBox cbxModeDetection = new JComboBox();
    Border border7;
    TitledBorder titledBorder4;
    Border border8;
    Component component1;
    JLabel jLabel3 = new JLabel();
    JComboBox cbxInputFrom = new JComboBox();
    JList lstState = new JList();
    Border border9;
    TitledBorder titledBorder5;
    Border border10;
    Border border11;
    TitledBorder titledBorder6;
    Border border12;
    Border border13;
    JButton btStartDog = new JButton();
    JButton btStopDog = new JButton();
    JButton btSpinHead = new JButton();
    JButton btBallTrack = new JButton();
    JButton btRandomWalk = new JButton();
    JButton btClear = new JButton();
    JPanel jPanel9 = new JPanel();
    GridBagLayout gridBagLayout7 = new GridBagLayout();
    JComboBox cbxTargetObj = new JComboBox();
    Border border14;
    TitledBorder titledBorder7;
    JTextField jtfNearDistance = new JTextField();
    JTextField jtfFarDistance = new JTextField();
    JButton btTargetObj = new JButton();
    JButton btNearDistance = new JButton();
    JButton btFarDistance = new JButton();
    JPanel jPanel10 = new JPanel();
    GridBagLayout gridBagLayout8 = new GridBagLayout();
    JButton btOpenRLOG = new JButton();
    JTextField jtfDogIP = new JTextField();
    JTextField jtfFileName = new JTextField();
    JButton btConnect = new JButton();
    JButton btDisconnect = new JButton();
    JComboBox cbxExpectation = new JComboBox();
    JRadioButton radioExpectYes = new JRadioButton();
    JRadioButton radioExpectNo = new JRadioButton();
    JRadioButton jRadioButton3 = new JRadioButton();
    ButtonGroup buttonGroup1 = new ButtonGroup();

    boolean isDoneInitGUI = false;

    public RobotController(RDRApplication parent) {
        super("Robot Controller w/ Intelligence");
        this.parent = parent;
        robolinkReceiver.addYUVListener(this);
        try {
            jbInit();
            startCommunication();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    void startCommunication(){
        commandSender = new CommandSender(SENDER_PORT);
        messageReceiver = new MessageReceiver(RECEIVER_PORT, BUFFER_LEN, new MessageListener() {
            public void onMessage(byte [] message, int len) {
                handleIncomingMessage(message,len);
            }
        });

        if (!bNoListening){
            commandSender.start();
            messageReceiver.start();
            (new ServerSockerHandler("Dummy 5019", 5019)).start();
            (new ServerSockerHandler("Dummy 5020", 5020)).start();
            (new ServerSockerHandler("Dummy 5021", 5021)).start();
            (new ServerSockerHandler("Dummy 5022", 5022)).start();
        }

    }

    void jbInit() throws Exception {
        isDoneInitGUI = false;
        border1 = BorderFactory.createLineBorder(SystemColor.controlText,1);
        titledBorder1 = new TitledBorder(border1,"CommandSender");
        border2 = BorderFactory.createCompoundBorder(titledBorder1,BorderFactory.createEmptyBorder(4,4,4,4));
        border3 = BorderFactory.createLineBorder(Color.black,1);
        titledBorder2 = new TitledBorder(border3,"MessageReceiver");
        border4 = BorderFactory.createCompoundBorder(titledBorder2,BorderFactory.createEmptyBorder(4,4,4,4));
        border5 = BorderFactory.createEtchedBorder(Color.white,new Color(165, 163, 151));
        titledBorder3 = new TitledBorder(border5,"Status");
        border6 = BorderFactory.createCompoundBorder(BorderFactory.createEtchedBorder(Color.white,new Color(165, 163, 151)),BorderFactory.createEmptyBorder(4,4,4,4));
        border7 = new EtchedBorder(EtchedBorder.RAISED,Color.white,new Color(165, 163, 151));
        titledBorder4 = new TitledBorder(border7,"Pause Condition");
        border8 = BorderFactory.createCompoundBorder(new TitledBorder(BorderFactory.createEtchedBorder(Color.white,new Color(165, 163, 151)),"Pause Condition"),BorderFactory.createEmptyBorder(2,2,2,2));
        component1 = Box.createVerticalStrut(8);
        border9 = BorderFactory.createEtchedBorder(Color.white,new Color(178, 178, 178));
        titledBorder5 = new TitledBorder(border9,"State");
        border10 = BorderFactory.createCompoundBorder(new TitledBorder(BorderFactory.createEtchedBorder(Color.white,new Color(178, 178, 178)),"State"),BorderFactory.createEmptyBorder(4,4,4,4));
        border11 = BorderFactory.createEtchedBorder(Color.white,new Color(165, 163, 151));
        titledBorder6 = new TitledBorder(border11,"Log");
        border12 = BorderFactory.createCompoundBorder(titledBorder6,BorderFactory.createEmptyBorder(2,2,2,2));
        border13 = BorderFactory.createLineBorder(Color.lightGray,1);
        border14 = BorderFactory.createEtchedBorder(Color.white,new Color(165, 163, 151));
        titledBorder7 = new TitledBorder(border14,"Commands");
        this.getContentPane().setLayout(borderLayout1);
        jPanel1.setLayout(gridBagLayout1);
        jLabel1.setAlignmentX((float) 0.0);
        jLabel1.setAlignmentY((float) 0.5);
        jLabel1.setText("Command");
        jtfName.setAlignmentX((float) 0.0);
        jtfName.setAlignmentY((float) 0.5);
        jtfName.setPreferredSize(new Dimension(40, 20));
        jtfName.setText("");
        jtfValue.setAlignmentX((float) 0.0);
        jtfValue.setAlignmentY((float) 0.5);
        jtfValue.setPreferredSize(new Dimension(200, 20));
        jtfValue.setText("");
        jtfValue.addActionListener(new RobotController_jtfValue_actionAdapter(this));
        jLabel2.setDebugGraphicsOptions(0);
        jLabel2.setText("Messages");
        jPanel3.setLayout(gridBagLayout2);
        cbxType.setAlignmentX((float) 0.0);
        cbxType.setAlignmentY((float) 0.5);
        cbxType.addActionListener(new RobotController_cbxType_actionAdapter(this));
        cbxType.addItem("-- default --");
        cbxType.addItem("pyc");
        cbxType.addItem("reload");
        cbxType.addItem("start");
        cbxType.addItem("stop");
        cbxType.addItem("pause");
        cbxType.addItem("resume");
        cbxType.addItem("BALL_TRACK");
        cbxType.addItem("HEAD_TURN");
        cbxType.addItem("reloadNNMC");
        cbxType.addItem("reloadRDR");
        cbxType.addItem("sendCritical");
        cbxType.addItem("printRDR");


        btSend.setMnemonic('S');
        btSend.setText("Send");
        btSend.addActionListener(new RobotController_btSend_actionAdapter(this));
        jPanel2.setBorder(border2);
        jPanel3.setBorder(border4);
        btExit.setText("Close");
        btExit.addActionListener(new RobotController_btExit_actionAdapter(this));
        btListen.setText("Listen");
        btListen.setEnabled(false);
        btListen.addActionListener(new RobotController_btListen_actionAdapter(this));
        jScrollPane1.setMinimumSize(new Dimension(23, 23));
        jScrollPane1.setPreferredSize(new Dimension(267, 148));
        jPanel5.setLayout(gridBagLayout3);
        btStartControl.setText("Start Controller");
        btStartControl.addActionListener(new RobotController_btStartControl_actionAdapter(this));
        jPanel7.setLayout(gridBagLayout4);
        jtaStatus.setBackground(Color.white);
        jtaStatus.setBorder(border13);
        jtaStatus.setMaximumSize(new Dimension(2147483647, 2147483647));
        jtaStatus.setText("");
        jtaStatus.addMouseListener(new RobotController_jtaStatus_mouseAdapter(this));
        jPanel6.setLayout(gridBagLayout5);
        jLabel4.setText("Mode");
        jPanel7.setBorder(border6);
        jPanel8.setLayout(gridBagLayout6);
        jPanel8.setBorder(border8);
        jPanel8.setMinimumSize(new Dimension(20, 40));
        jPanel8.setPreferredSize(new Dimension(20, 70));
        jLabel3.setText("Input From");
        lstState.setBackground(SystemColor.control);
        lstState.setBorder(border10);
        lstState.setMaximumSize(new Dimension(2147483647, 2147483647));
        lstState.setMinimumSize(new Dimension(50, 100));
        lstState.setPreferredSize(new Dimension(50, 120));
        lstState.addMouseListener(new RobotController_lstState_mouseAdapter(this));
        lstState.setListData(STATUS_TO_STRING);
        jScrollPane2.setBorder(border12);
        btStartDog.setEnabled(false);
        btStartDog.setText("Go !");
        btStartDog.addActionListener(new RobotController_btStartDog_actionAdapter(this));
        btStopDog.setText("Stop !");
        btStopDog.addActionListener(new RobotController_btStopDog_actionAdapter(this));
        btSpinHead.setMargin(new Insets(2, 4, 2, 4));
        btSpinHead.setText("Head spin");
        btSpinHead.addActionListener(new RobotController_btSpinHead_actionAdapter(this));
        btBallTrack.setMargin(new Insets(2, 4, 2, 4));
        btBallTrack.setText("Ball Track");
        btBallTrack.addActionListener(new RobotController_btBallTrack_actionAdapter(this));
        btRandomWalk.setMargin(new Insets(2, 4, 2, 4));
        btRandomWalk.setText("Random");
        btRandomWalk.addActionListener(new RobotController_btRandomWalk_actionAdapter(this));
        btClear.setText("Clear");
        btClear.addActionListener(new RobotController_btClear_actionAdapter(this));
        jPanel9.setLayout(gridBagLayout7);
        cbxTargetObj.addItem("Ball");
        cbxTargetObj.addItem("Blue Goal");
        cbxTargetObj.addItem("Yellow Goal");
        cbxTargetObj.addItem("Blue On Pink");
        cbxTargetObj.addItem("Pink On Blue");
        cbxTargetObj.addItem("Yellow On Pink");
        cbxTargetObj.addItem("Pink On Yellow");
        jPanel9.setBorder(titledBorder7);
        jtfNearDistance.setText("100");
        jtfFarDistance.setText("200");
        btTargetObj.setText("Target Obj");
        btTargetObj.addActionListener(new RobotController_btTargetObj_actionAdapter(this));
        btNearDistance.setText("Near Dist");
        btNearDistance.addActionListener(new RobotController_btNearDistance_actionAdapter(this));
        btFarDistance.setText("Far Dist");
        btFarDistance.addActionListener(new RobotController_btFarDistance_actionAdapter(this));
        jPanel10.setLayout(gridBagLayout8);
        btOpenRLOG.setText("Open rlog");
        jtfDogIP.setToolTipText("");
        jtfDogIP.setText("192.168.0.199");
        btConnect.setText("Connect");
        btConnect.addActionListener(new RobotController_btConnect_actionAdapter(this));
        jtfFileName.setText("");
        btDisconnect.setText("Disconnect");
        radioExpectYes.setText("Expect Yes");
        radioExpectYes.addActionListener(new RobotController_radioExpectYes_actionAdapter(this));
        radioExpectNo.setText("Expect No");
        radioExpectNo.addActionListener(new RobotController_radioExpectNo_actionAdapter(this));
        jRadioButton3.setText("Don\'t care");
        jRadioButton3.addActionListener(new RobotController_jRadioButton3_actionAdapter(this));

        cbxExpectation.addActionListener(new RobotController_cbxExpectation_actionAdapter(this));

        jButton1.setToolTipText("");
        jButton1.setText("Head pos");
        jTextField1.setText("0 0 0 ");
        btChaseBall.setText("Chase Ball");
        btChaseBall.addActionListener(new RobotController_btChaseBall_actionAdapter(this));
        btTurnAround.setText("Turn Around");
        btTurnAround.addActionListener(new RobotController_btTurnAround_actionAdapter(this));
        btGoToCenter.setText("go2 Center");
        btGoToCenter.addActionListener(new RobotController_btGoToCenter_actionAdapter(this));
        btStepBack.setText("Step Back");
        btStepBack.addActionListener(new RobotController_btStepBack_actionAdapter(this));
        this.getContentPane().add(jTabbedPane1, BorderLayout.CENTER);
        this.getContentPane().add(jPanel4,  BorderLayout.SOUTH);
        jPanel4.add(btListen, null);
        jPanel4.add(btExit, null);
        jTabbedPane1.add(jPanel1,   "Commander");
        jPanel1.add(jPanel2,       new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel1, null);
        jPanel2.add(cbxType, null);
        jPanel2.add(jtfName, null);
        jPanel2.add(jtfValue, null);
        jPanel2.add(btSend, null);

        cbxModeDetection.addItem("False Negative Detected");
        cbxModeDetection.addItem("False Positive Detected");
        cbxModeDetection.addItem("Do Nothing");
        cbxModeDetection.setSelectedIndex(cbxModeDetection.getItemCount() - 1);

        cbxInputFrom.addItem("Blob (Interval) RDR");
        cbxInputFrom.addItem("Object (Relation) RDR ");

        cbxExpectation.addItem("Ball");
        cbxExpectation.addItem("Yellow Goal");
        cbxExpectation.addItem("Blue Goal");
        cbxExpectation.addItem("Blue On Pink");
        cbxExpectation.addItem("Pink On Blue");
        cbxExpectation.addItem("Yellow On Pink");
        cbxExpectation.addItem("Pink On Yellow");
        jPanel1.add(jPanel3,    new GridBagConstraints(1, 1, 1, 2, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel3.add(jLabel2,               new GridBagConstraints(0, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(5, 5, 0, 5), 0, 0));
        jPanel3.add(jScrollPane1,                 new GridBagConstraints(0, 2, 5, 5, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(5, 0, 5, 0), 267, 148));
        jPanel3.add(btClear,     new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jTabbedPane1.add(jPanel5,  "Intelligence");
        jPanel5.add(jPanel6,        new GridBagConstraints(0, 0, 1, 1, 0.5, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel6.add(jPanel8,                              new GridBagConstraints(0, 0, 1, 1, 1.0, 0.2
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 8, 5), 0, 0));
        jPanel8.add(jLabel4,               new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.EAST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel8.add(cbxModeDetection,             new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 5, 5), 0, 0));
        jPanel8.add(jLabel3,           new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.EAST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel8.add(cbxInputFrom,            new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 5, 5), 0, 0));
        jPanel8.add(component1,        new GridBagConstraints(1, 2, 1, 1, 0.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, -13, 0, 20), 0, 0));
        jPanel6.add(btStartControl,                          new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel9.add(btBallTrack,                 new GridBagConstraints(1, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btSpinHead,             new GridBagConstraints(2, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(cbxTargetObj,           new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(jtfNearDistance,   new GridBagConstraints(2, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(jtfFarDistance,   new GridBagConstraints(2, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btTargetObj,   new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btNearDistance,   new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btFarDistance,   new GridBagConstraints(1, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(cbxExpectation, new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(radioExpectYes,  new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(radioExpectNo, new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(jRadioButton3, new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btStartDog, new GridBagConstraints(0, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btStopDog, new GridBagConstraints(0, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.SOUTH, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btRandomWalk,  new GridBagConstraints(1, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(jButton1, new GridBagConstraints(1, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(jTextField1,  new GridBagConstraints(2, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(2, 2, 2, 2), 0, 0));
        jPanel9.add(btChaseBall, new GridBagConstraints(2, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btTurnAround, new GridBagConstraints(0, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel6.add(jPanel9,        new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel5.add(jPanel7,       new GridBagConstraints(1, 0, 1, 1, 0.5, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel7.add(jScrollPane2,         new GridBagConstraints(0, 1, 2, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 5, 5, 5), 0, 0));
        jPanel7.add(lstState,     new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(3, 3, 3, 3), 0, 0));
        jTabbedPane1.add(jPanel10,  "Robolink");
        jPanel10.add(btOpenRLOG,   new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel10.add(jtfDogIP,     new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 12, 0));
        jPanel10.add(jtfFileName,   new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 62, 0));
        jPanel10.add(btConnect,  new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel10.add(btDisconnect, new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jScrollPane2.getViewport().add(jtaStatus, null);
        jScrollPane1.getViewport().add(jtaMessageReceived, null);
        btOpenRLOG.addActionListener(new RobotController_btOpenRLOG_actionAdapter(this));
        buttonGroup1.add(radioExpectYes);
        buttonGroup1.add(radioExpectNo);
        buttonGroup1.add(jRadioButton3);
        jPanel9.add(btGoToCenter, new GridBagConstraints(1, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel9.add(btStepBack,  new GridBagConstraints(2, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        isDoneInitGUI = true;
    }

    public void handleIncomingMessage(byte [] message, int len) {
        jtaMessageReceived.append(new String(message,0,len));
    }

    public static void main(String[] args) {
        RobotController robotController = new RobotController(null);
        robotController.pack();
        robotController.setVisible(true);

    }

    void setControlState(int state){
        controlState = state;
        lstState.setSelectedIndex(state);
    }

    boolean acceptThisFrame(VisionObserver visionObserver){
        int temp = -1;
        switch (cbxInputFrom.getSelectedIndex()){
            case INPUT_BLOB_RDR:
                temp = visionObserver.numberOfBlob;
                break;
            case INPUT_OBJECT_RDR:
                temp = visionObserver.numberOfObject;
                break;
        }

        switch (cbxModeDetection.getSelectedIndex()) {
            case MODE_FALSE_POSITIVE:
                return temp <= 0;
            case MODE_FALSE_NEGATIVE:
                return temp > 0;
        }

        return true;
    }
    //////////////////////// OBSERVER INTERFACE //////////////////////////////
    public void update(Subject o) {
        if (o instanceof VisionObserver){
            if (acceptThisFrame((VisionObserver) o) ) {
                setControlState(CONTROL_VISION_PROCESSED);
            }
            else {
                setControlState( CONTROL_STOP_TO_ANALYSE);
            }
        }
    }

    //////////////////////// YUVLISTENER INTERFACE //////////////////////////////

    public void notifyNewBFL(BFL bfl) {
        if (controlState == CONTROL_RECEIVING_NEW){
            System.out.println("Warning : previous bfl hasn't been processed yet");
        }
        else if (controlState == CONTROL_VISION_PROCESSED || controlState == CONTROL_IDLE){
            setControlState( CONTROL_RECEIVING_NEW );
            if (parent != null)
                parent.notifyNewBFL(bfl);

        }
    }


    /////////////////// EVENT HANDLING CODE/////////////////////////

    void btListen_actionPerformed(ActionEvent e) {
        startCommunication();
    }

    void btSend_actionPerformed(ActionEvent e) {
        try {
            commandSender.sendMessage(jtfName.getText(), jtfValue.getText());
        }
        catch (IOException ex) {
            ex.printStackTrace();
            JOptionPane.showMessageDialog(this,"Cannot send message [" + jtfName.getText() + "," + jtfValue.getText() + "]", "Error",JOptionPane.ERROR_MESSAGE);
        }
    }

    void jtfValue_actionPerformed(ActionEvent e) {
        btSend_actionPerformed(e);
    }

    void btExit_actionPerformed(ActionEvent e) {
        setVisible(false);
    }

    void btStartControl_actionPerformed(ActionEvent e) {
        if (yuvReceiver == null){
            yuvReceiver = new YUVStreamClient();
            yuvReceiver.addYUVListener(this);

        }
        yuvReceiver.start();
    }

    void lstState_mouseClicked(MouseEvent e) {
        if (e.getClickCount() >= 2){
            System.out.println("Setting State to " + STATUS_TO_STRING[ lstState.getSelectedIndex() ]);
            setControlState(lstState.getSelectedIndex());
        }
    }

    void btStartDog_actionPerformed(ActionEvent e) {
        doCommand("pyc", DOG_COMMAND[comSTART_DOG]);
    }

    void btStopDog_actionPerformed(ActionEvent e) {
        doCommand("pyc", DOG_COMMAND[comSTOP_DOG]);
    }

    void doCommand(String name, String value){
        jtfName.setText(name);
        jtfValue.setText(value);
        jtaStatus.append("\nCommand sent : " + name + ", " + value );
        btSend_actionPerformed(null);
    }

    void cbxType_actionPerformed(ActionEvent e) {
        if (cbxType.getSelectedItem().equals("-- default --") ){

        }
        else {
            if (cbxType.getSelectedItem().equals("reload")) {
                doCommand("py", "reload 1");
            }
            else if (cbxType.getSelectedItem().equals("start")) {
                doCommand("mode", "0");
            }
            else if (cbxType.getSelectedItem().equals("stop")) {
                doCommand("mode", "1");
            }
            else if (cbxType.getSelectedItem().equals("BALL_TRACK")) {
                doCommand("pyc", "1 mode BALL_TRACK");
            }
            else if (cbxType.getSelectedItem().equals("HEAD_TURN")) {
                doCommand("pyc", "1 mode HEAD_TURN");
            }
            else if (cbxType.getSelectedItem().equals("pause")) {
                doCommand("pyc", "1 pause");
            }
            else if (cbxType.getSelectedItem().equals("resume")) {
                doCommand("pyc", "1 resume");
            }
            else if (cbxType.getSelectedItem().equals("reloadNNMC")) {
                doCommand("pyc", "1 eval VisionLink.reloadNNMC() ");
            }
            else if (cbxType.getSelectedItem().equals("reloadRDR")) {
                doCommand("pyc", "1 eval VisionLink.reloadRDR() ");
            }
            else if (cbxType.getSelectedItem().equals("sendCritical")) {
                doCommand("pyc",DOG_COMMAND[comONLY_SEND_CRITICAL_SWITCH]);
            }
            else if (cbxType.getSelectedItem().equals("printRDR")) {
                doCommand("pyc",DOG_COMMAND[comPRINT_RDR]);
            }

            else{
                jtfName.setText( (String) cbxType.getSelectedItem());
            }
            cbxType.setSelectedIndex(0);
        }
    }

    void btSpinHead_actionPerformed(ActionEvent e) {
        try {
            commandSender.sendMessage("pyc", DOG_COMMAND[comSPIN_HEAD]);
            jtaStatus.append("\nCommand sent : " + "pyc," +
                             DOG_COMMAND[comSPIN_HEAD]);
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void btRandomWalk_actionPerformed(ActionEvent e) {
        try {
            commandSender.sendMessage("pyc", DOG_COMMAND[comRANDOM_WALK]);
            jtaStatus.append("\nCommand sent : " + "pyc," +
                             DOG_COMMAND[comRANDOM_WALK]);
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void btBallTrack_actionPerformed(ActionEvent e) {
        try {
            commandSender.sendMessage("pyc", DOG_COMMAND[comBALL_TRACK]);
            jtaStatus.append("\nCommand sent : " + "pyc," +
                             DOG_COMMAND[comBALL_TRACK]);
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void btClear_actionPerformed(ActionEvent e) {
        jtaMessageReceived.setText("");
    }

    void btTargetObj_actionPerformed(ActionEvent e) {
        try {
            String comm = "1 exec Player.player.learningObjectID = " + cbxTargetObj.getSelectedIndex();
            commandSender.sendMessage("pyc", comm);
            jtaStatus.append("\nCommand sent : " + "pyc," +
                             comm);
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }

    }

    void btNearDistance_actionPerformed(ActionEvent e) {
        try {
            String comm = "1 exec Player.player.CLOSE_ENOUGH = " + jtfNearDistance.getText();
            commandSender.sendMessage("pyc", comm);
            jtaStatus.append("\nCommand sent : " + "pyc," +
                             comm);
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void btFarDistance_actionPerformed(ActionEvent e) {
        try {
            String comm = "1 exec Player.player.FAR_ENOUGH = " +
                jtfFarDistance.getText();
            commandSender.sendMessage("pyc", comm);
            jtaStatus.append("\nCommand sent : " + "pyc," +
                             comm);
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void btOpenRLOG_actionPerformed(ActionEvent e) {
        JFileChooser chooser = new JFileChooser(RDRApplication.appConf.getRLOGDirectory());
        int ret = chooser.showOpenDialog(this);
        if (ret != JFileChooser.CANCEL_OPTION){
            String fileName = chooser.getSelectedFile().getPath();
            jtfFileName.setText(fileName);
            RDRApplication.appConf.setRLOGDirectory(fileName);
            robolinkReceiver.receiveFromFile(fileName);
        }
    }

    void btConnect_actionPerformed(ActionEvent e) {
        robolinkReceiver.receiveFromNet(jtfDogIP.getText());
    }

    void btDisconnect_actionPerformed(ActionEvent e) {
        robolinkReceiver.disconnect();
    }

    static String [] objectVariable = {
        "expectBall",
        "expectYellowGoal",
        "expectBlueGoal",
        "expectBlueLeft",
        "expectBlueRight",
        "expectYellowLeft",
        "expectYellowRight"
    };
    JButton jButton1 = new JButton();
    JTextField jTextField1 = new JTextField();
    JButton btChaseBall = new JButton();
    JButton btTurnAround = new JButton();
    JButton btGoToCenter = new JButton();
    JButton btStepBack = new JButton();

    void cbxExpectation_actionPerformed(ActionEvent e) {
        changeExpectation(e);
    }

    public void changeExpectation(ActionEvent e) {
        if (!isDoneInitGUI) return;
        int objID = cbxExpectation.getSelectedIndex();
        String selection = "None";
        if (radioExpectYes.isSelected())
            selection = "True";
        else if (radioExpectNo.isSelected())
            selection = "False";

        doCommand("pyc", "1 exec Player.player.hTestLandmark." + objectVariable[objID] + " = " + selection);
    }

    void jtaStatus_mouseClicked(MouseEvent e) {
        if (e.getClickCount() >= 2){
            System.out.println("Double clicked, clearing log.");
            jtaStatus.setText("");
        }
    }

    void btChaseBall_actionPerformed(ActionEvent e) {
        try {
            commandSender.sendMessage("pyc", "1 mode chase");
            jtaStatus.append("\nCommand sent : " + "pyc, 1 mode chase");
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void btTurnAround_actionPerformed(ActionEvent e) {
        doCommand("pyc", DOG_COMMAND[comTURN_AROUND]);
    }

    void btGoToCenter_actionPerformed(ActionEvent e) {
        doCommand("pyc", DOG_COMMAND[comGO_TO_CENTER]);
    }

    void btStepBack_actionPerformed(ActionEvent e) {
        doCommand("pyc", DOG_COMMAND[comSTEP_BACK]);
    }

}

class RobotController_btListen_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btListen_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btListen_actionPerformed(e);
    }
}

class RobotController_btSend_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btSend_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btSend_actionPerformed(e);
    }
}

class RobotController_jtfValue_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_jtfValue_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.jtfValue_actionPerformed(e);
    }
}

class RobotController_btExit_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btExit_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btExit_actionPerformed(e);
    }
}

class RobotController_btStartControl_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btStartControl_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btStartControl_actionPerformed(e);
    }
}

class RobotController_lstState_mouseAdapter extends java.awt.event.MouseAdapter {
    RobotController adaptee;

    RobotController_lstState_mouseAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void mouseClicked(MouseEvent e) {
        adaptee.lstState_mouseClicked(e);
    }
}

class RobotController_btStartDog_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btStartDog_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btStartDog_actionPerformed(e);
    }
}

class RobotController_btStopDog_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btStopDog_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btStopDog_actionPerformed(e);
    }
}

class RobotController_cbxType_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_cbxType_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.cbxType_actionPerformed(e);
    }
}

class RobotController_btSpinHead_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btSpinHead_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btSpinHead_actionPerformed(e);
    }
}

class RobotController_btRandomWalk_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btRandomWalk_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btRandomWalk_actionPerformed(e);
    }
}

class RobotController_btBallTrack_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btBallTrack_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btBallTrack_actionPerformed(e);
    }
}

class RobotController_btClear_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btClear_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btClear_actionPerformed(e);
    }
}

class RobotController_btTargetObj_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btTargetObj_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btTargetObj_actionPerformed(e);
    }
}

class RobotController_btNearDistance_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btNearDistance_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btNearDistance_actionPerformed(e);
    }
}

class RobotController_btFarDistance_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btFarDistance_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btFarDistance_actionPerformed(e);
    }
}

class RobotController_btOpenRLOG_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btOpenRLOG_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenRLOG_actionPerformed(e);
    }
}

class RobotController_btConnect_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btConnect_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btConnect_actionPerformed(e);
    }
}

class RobotController_btDisconnect_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btDisconnect_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btDisconnect_actionPerformed(e);
    }
}

class RobotController_cbxExpectation_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_cbxExpectation_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.cbxExpectation_actionPerformed(e);
    }
}

class RobotController_radioExpectNo_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_radioExpectNo_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.changeExpectation(e);
    }
}

class RobotController_radioExpectYes_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_radioExpectYes_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.changeExpectation(e);
    }
}

class RobotController_jtaStatus_mouseAdapter extends java.awt.event.MouseAdapter {
    RobotController adaptee;

    RobotController_jtaStatus_mouseAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void mouseClicked(MouseEvent e) {
        adaptee.jtaStatus_mouseClicked(e);
    }
}

class RobotController_jRadioButton3_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_jRadioButton3_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.changeExpectation(e);
    }
}

class RobotController_btChaseBall_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btChaseBall_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btChaseBall_actionPerformed(e);
    }
}

class RobotController_btTurnAround_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btTurnAround_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btTurnAround_actionPerformed(e);
    }
}

class RobotController_btGoToCenter_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btGoToCenter_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btGoToCenter_actionPerformed(e);
    }
}

class RobotController_btStepBack_actionAdapter implements java.awt.event.ActionListener {
    RobotController adaptee;

    RobotController_btStepBack_actionAdapter(RobotController adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btStepBack_actionPerformed(e);
    }
}
