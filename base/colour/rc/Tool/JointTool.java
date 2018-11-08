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


/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.2
 * CHANGE LOG:
 * + version 1.1:
 *     - Play around with roll angle, faster horizon calculation, forward
 *     kinematics of bodyTilt/Height/Roll
 * + version 1.2:
 *     - Add a noise tool to see how horizon changes when the sensor changes.
 *     - Still hasn't solved some noise problem yet
 */

/** @old todo
 * Investigate why pan is fluctuating??? */

package Tool;

import javax.swing.*;
import java.awt.*;
import javax.swing.table.*;
import java.awt.event.*;
import RoboShare.*;
import javax.swing.border.*;
import Jama.*;
import java.util.*;
import java.text.*;
import javax.swing.event.*;

public class JointTool extends JFrame {
    BorderLayout borderLayout1 = new BorderLayout();
    JPanel jPanel1 = new JPanel();
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JButton btExit = new JButton();
    JButton btCalculate = new JButton();
    JScrollPane jScrollPane2 = new JScrollPane();
    JTable tableSensor = new JTable(new SensorTableModel( ) );
    JLabel jLabel1 = new JLabel();
    JTextField jtfBodyTilt = new JTextField();
    JLabel jLabel3 = new JLabel();
    JTextField jtfIPX = new JTextField();
    JLabel jLabel4 = new JLabel();
    JTextField jtfNeckHeight = new JTextField();
    JLabel jLabel5 = new JLabel();
    JTextField jtfIPY = new JTextField();
    JPanel jPanel2 = new JPanel();
    GridBagLayout gridBagLayout2 = new GridBagLayout();
    JPanel jPanel4 = new JPanel();
    JScrollPane jScrollPane1 = new JScrollPane();
    JTable tableResult = new JTable(new ResultTable() );
    JTabbedPane jTabbedPane1 = new JTabbedPane();
    Border border1;
    TitledBorder titledBorder1;
    Border border2;
    GridBagLayout gridBagLayout3 = new GridBagLayout();
    JLabel jLabel2 = new JLabel();
    JTextField jtfElevation = new JTextField();
    JLabel jLabel6 = new JLabel();
    JTextField jtfHeading = new JTextField();
    Component component1;
    JLabel jLabel7 = new JLabel();
    JTextField jtfHorizon1 = new JTextField();
    JTextField jtfHorizon2 = new JTextField();
    JLabel jLabel8 = new JLabel();
    JTextField jtfRollAngle = new JTextField();
    JTextField jtfRollVector = new JTextField();
    JPanel jPanel3 = new JPanel();
    GridBagLayout gridBagLayout4 = new GridBagLayout();
    JLabel jLabel9 = new JLabel();
    JSlider jslPan = new JSlider();
    JLabel jLabel10 = new JLabel();
    JSlider jslTilt = new JSlider();
    JLabel jLabel11 = new JLabel();
    JSlider jslCrane = new JSlider();
    JSlider jslBodyTilt = new JSlider();
    JSlider jslNeckHeight = new JSlider();
    JSlider jslNeckLength = new JSlider();
    JLabel jLabel12 = new JLabel();
    JLabel jLabel13 = new JLabel();
    JLabel jLabel14 = new JLabel();
    JPanel horizonPanel = new JPanel();
    Border border3;
    JButton btStop = new JButton();
    JButton btStart = new JButton();
    Border border4;
    JLabel jLabel15 = new JLabel();
    JLabel jLabel16 = new JLabel();
    JLabel jLabel17 = new JLabel();
    JLabel jLabel18 = new JLabel();
    JLabel jLabel19 = new JLabel();
    JLabel jLabel20 = new JLabel();
    JLabel jLabel21 = new JLabel();
    FlowLayout flowLayout1 = new FlowLayout();
    FlowLayout flowLayout2 = new FlowLayout();
    FlowLayout flowLayout3 = new FlowLayout();
    FlowLayout flowLayout4 = new FlowLayout();
    FlowLayout flowLayout5 = new FlowLayout();
    FlowLayout flowLayout6 = new FlowLayout();

    Random r = new Random();
	int limitPan = 0;
	int limitTilt = 0;
	int limitCrane = 0;
	int limitBodyTilt = 0;
	int limitNeckHeight = 0;
	int limitNeckLength = 0;
    double pan, tilt, crane, bodyTilt, neckHeight;

    int delay = 10; //milliseconds
    ActionListener taskPerformer = new ActionListener() {
        public void actionPerformed(ActionEvent evt) {
           regenerateHorizon();
        }
    };

    javax.swing.Timer timer = new javax.swing.Timer(delay, taskPerformer);

    public JointTool() {
        super("Joint Tool");
        try {
            jbInit();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        border1 = BorderFactory.createEtchedBorder(Color.red,new Color(165, 163, 151));
        titledBorder1 = new TitledBorder(BorderFactory.createLineBorder(Color.lightGray,1),"Joint tool");
        border2 = BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.white,1),BorderFactory.createEmptyBorder(3,3,3,3));
        component1 = Box.createVerticalStrut(8);
        border3 = BorderFactory.createLineBorder(Color.black,1);
        border4 = BorderFactory.createEmptyBorder(5,5,5,5);
        this.getContentPane().setLayout(borderLayout1);
        jPanel1.setBorder(border2);
        jPanel1.setDebugGraphicsOptions(0);
        jPanel1.setPreferredSize(new Dimension(423, 451));
        jPanel1.setLayout(gridBagLayout1);
        btExit.setText("Exit");
        btCalculate.setText("Calculate");
        btCalculate.addActionListener(new JointTool_btCalculate_actionAdapter(this));
        jScrollPane2.setPreferredSize(new Dimension(200, 402));
        jLabel1.setText("Body Tilt");
        jtfBodyTilt.setMinimumSize(new Dimension(62, 20));
        jtfBodyTilt.setPreferredSize(new Dimension(100, 20));
        jtfBodyTilt.setText(new DecimalFormat("0.00").format(RobotDefinition.bodyTilt) );
        jLabel3.setText("pixel x");
        jtfIPX.setMinimumSize(new Dimension(62, 20));
        jtfIPX.setPreferredSize(new Dimension(100, 20));
        jtfIPX.setText("0.0");
        jLabel4.setText("Neck Height");
        jLabel5.setText("pixel y");
        jtfNeckHeight.setMinimumSize(new Dimension(62, 20));
        jtfNeckHeight.setPreferredSize(new Dimension(100, 20));
        jtfNeckHeight.setText("0.0");
        jtfIPY.setMinimumSize(new Dimension(62, 20));
        jtfIPY.setPreferredSize(new Dimension(100, 20));
        jtfIPY.setText("0.0");
        jPanel2.setLayout(gridBagLayout2);
        jPanel4.setPreferredSize(new Dimension(200, 412));
        jPanel4.setLayout(gridBagLayout3);
        jScrollPane1.setPreferredSize(new Dimension(200, 402));
        jPanel2.setAlignmentX((float) 0.5);
        jPanel2.setAlignmentY((float) 0.0);
        jLabel2.setText("Elevation");
        jtfElevation.setText("");
        jLabel6.setText("Heading");
        jtfHeading.setText("");
        jLabel7.setToolTipText("");
        jLabel7.setText("Horizon");
        jtfHorizon1.setText("");
        jLabel8.setText("Roll");
        jtfRollAngle.setText("");
        jtfRollVector.setText("");
        jPanel3.setLayout(gridBagLayout4);
        jLabel9.setText("Pan");
        jLabel10.setText("Tilt");
        jLabel11.setText("Crane");
        jLabel12.setText("Body Tilt");
        jLabel13.setText("Neck Height");
        jLabel14.setText("Neck Length");
        horizonPanel.setBorder(border3);
        horizonPanel.setMaximumSize(new Dimension(108, 80));
        horizonPanel.setMinimumSize(new Dimension(108, 80));
        horizonPanel.setPreferredSize(new Dimension(108, 80));
        btStop.setText("Stop");
        btStop.addActionListener(new JointTool_btStop_actionAdapter(this));
        btStart.setSelected(false);
        btStart.setText("Start");
        btStart.addActionListener(new JointTool_btStart_actionAdapter(this));
        jslCrane.setPaintLabels(false);
        jslCrane.setPaintTicks(false);
        jslCrane.setPaintTrack(true);
        jslCrane.setPreferredSize(new Dimension(100, 24));
        jslCrane.addChangeListener(new JointTool_jslCrane_changeAdapter(this));
        jslCrane.setLayout(flowLayout1);
        jslNeckHeight.setPaintLabels(false);
        jslNeckHeight.setPaintTicks(false);
        jslNeckHeight.setPaintTrack(true);
        jslNeckHeight.setPreferredSize(new Dimension(100, 24));
        jslNeckHeight.addChangeListener(new JointTool_jslNeckHeight_changeAdapter(this));
        jslNeckHeight.setLayout(flowLayout5);
        jslNeckLength.setPaintLabels(true);
        jslNeckLength.setPreferredSize(new Dimension(100, 24));
        jslNeckLength.addChangeListener(new JointTool_jslNeckLength_changeAdapter(this));
        jslNeckLength.setLayout(flowLayout6);
        jslBodyTilt.setPaintLabels(false);
        jslBodyTilt.setPaintTicks(false);
        jslBodyTilt.setPaintTrack(true);
        jslBodyTilt.setPreferredSize(new Dimension(100, 24));
        jslBodyTilt.addChangeListener(new JointTool_jslBodyTilt_changeAdapter(this));
        jslBodyTilt.setLayout(flowLayout4);
        jslTilt.setPaintLabels(false);
        jslTilt.setPreferredSize(new Dimension(100, 24));
        jslTilt.addChangeListener(new JointTool_jslTilt_changeAdapter(this));
        jslTilt.setLayout(flowLayout2);
//        jLabel15.setBounds(new Rectangle(5, 0, 40, 15));
        jslPan.addChangeListener(new JointTool_jslPan_changeAdapter(this));
        jslPan.setPreferredSize(new Dimension(100, 24));
        jslPan.setLayout(flowLayout3);
        jPanel3.setPreferredSize(new Dimension(168, 484));
        jTabbedPane1.setPreferredSize(new Dimension(205, 410));
        jLabel15.setText("0");
        jLabel16.setText("0");
        jLabel17.setAlignmentY((float) 0.0);
        jLabel17.setText("0");
        jLabel18.setAlignmentY((float) 0.0);
        jLabel18.setText("0");
        jLabel19.setAlignmentY((float) 0.0);
        jLabel19.setText("0");
        jLabel20.setAlignmentY((float) 0.0);
        jLabel20.setText("0");
        jLabel21.setAlignmentY((float) 0.0);
        jLabel21.setText("0");
        jPanel1.add(btCalculate,          new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.SOUTH, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel1.add(jScrollPane2,          new GridBagConstraints(0, 0, 2, 1, 0.5, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel1.add(jTabbedPane1, new GridBagConstraints(2, 0, 1, 1, 0.5, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jScrollPane2.getViewport().add(tableSensor, null);
        jTabbedPane1.add(jPanel4, "Table");
        jPanel4.add(jScrollPane1, new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jTabbedPane1.add(jPanel2, "Camera");
        jScrollPane1.getViewport().add(tableResult, null);
        jPanel2.add(jLabel4,            new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel1,            new GridBagConstraints(1, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfIPX,          new GridBagConstraints(2, 3, 2, 1, 0.0, 0.0
            ,GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel3,            new GridBagConstraints(1, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfBodyTilt,          new GridBagConstraints(2, 2, 2, 1, 0.0, 0.0
            ,GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfNeckHeight,          new GridBagConstraints(2, 1, 2, 1, 0.0, 0.0
            ,GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel5,            new GridBagConstraints(1, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfIPY,           new GridBagConstraints(2, 4, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel2,       new GridBagConstraints(1, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfElevation,      new GridBagConstraints(2, 5, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel6,       new GridBagConstraints(1, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfHeading,      new GridBagConstraints(2, 6, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(component1,     new GridBagConstraints(1, 11, 1, 1, 0.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(jLabel7,     new GridBagConstraints(1, 7, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfHorizon1,    new GridBagConstraints(2, 7, 1, 1, 0.5, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfHorizon2,   new GridBagConstraints(3, 7, 1, 1, 0.5, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jLabel8,  new GridBagConstraints(1, 10, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfRollAngle,  new GridBagConstraints(2, 10, 1, 1, 0.5, 0.5
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 5, 5), 0, 0));
        jPanel2.add(jtfRollVector, new GridBagConstraints(3, 10, 1, 1, 0.5, 0.5
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 5, 5), 0, 0));
        jTabbedPane1.add(jPanel3, "Noise");
        jPanel3.add(jLabel9,        new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 5, 0), 0, 0));
        jPanel3.add(jslPan,           new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 0, 5, 0), 0, 0));
        jslPan.add(jLabel15, null);
        jPanel3.add(jLabel10,        new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 5, 0), 0, 0));
        jPanel3.add(jslTilt,           new GridBagConstraints(1, 1, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 0, 5, 0), 0, 0));
        jslTilt.add(jLabel17, null);
        jPanel3.add(jLabel11,        new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 5, 0), 0, 0));
        jPanel3.add(jslCrane,           new GridBagConstraints(1, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 0, 5, 0), 0, 0));
        jslCrane.add(jLabel18, null);
        jPanel3.add(jslBodyTilt,             new GridBagConstraints(1, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 0, 5, 0), 0, 0));
        jslBodyTilt.add(jLabel19, null);
        jPanel3.add(jslNeckHeight,           new GridBagConstraints(1, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 0, 5, 0), 0, 0));
        jslNeckHeight.add(jLabel20, null);
        jPanel3.add(jslNeckLength,           new GridBagConstraints(1, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(5, 0, 5, 0), 0, 0));
        jslNeckLength.add(jLabel21, null);
        jPanel3.add(jLabel12,        new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 5, 0), 0, 0));
        jPanel3.add(jLabel13,        new GridBagConstraints(0, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 5, 0), 0, 0));
        jPanel3.add(jLabel14,        new GridBagConstraints(0, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 5, 0), 0, 0));
        jPanel3.add(btStart,  new GridBagConstraints(0, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel3.add(horizonPanel,    new GridBagConstraints(1, 6, 1, 3, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel3.add(btStop,  new GridBagConstraints(0, 8, 1, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel1.add(btExit, new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.SOUTHWEST, GridBagConstraints.NONE, new Insets(0, -2, 0, 2), 19, 0));
        this.getContentPane().add(jPanel1, BorderLayout.SOUTH);
    }


    // Overridden so we can exit when window is closed

    protected void processWindowEvent(WindowEvent e) {
        super.processWindowEvent(e);
        if (e.getID() == WindowEvent.WINDOW_CLOSING) {
            this.dispose();
        }
    }

    public static void main(String[] args) {
        JointTool jointTool = new JointTool();
        jointTool.pack();
        jointTool.setVisible(true);
    }

    private void calculateLegKinematics(){
        LegGeometry.JOINT_Values jointFrontLeft = new LegGeometry.JOINT_Values(
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssFL_ROTATOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssFL_ABDUCTOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssFL_KNEE, 1)).
            doubleValue()
            );
        LegGeometry.JOINT_Values jointFrontRight = new LegGeometry.JOINT_Values(
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssFR_ROTATOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssFR_ABDUCTOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssFR_KNEE, 1)).
            doubleValue()
            );
        LegGeometry.JOINT_Values jointRearLeft = new LegGeometry.JOINT_Values(
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssRL_ROTATOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssRL_ABDUCTOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssRL_KNEE, 1)).
            doubleValue()
            );
        LegGeometry.JOINT_Values jointRearRight = new LegGeometry.JOINT_Values(
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssRR_ROTATOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssRR_ABDUCTOR, 1)).
            doubleValue(),
            ( (Double) tableSensor.getValueAt(SensorDefinition.ssRR_KNEE, 1)).
            doubleValue()
            );

        XYZ_Coord xyzFrontLeft = new XYZ_Coord();
        LegGeometry.kinematicsFrontLeft(jointFrontLeft, xyzFrontLeft);
        XYZ_Coord xyzFrontRight = new XYZ_Coord();
        LegGeometry.kinematicsFrontRight(jointFrontRight, xyzFrontRight);
        XYZ_Coord xyzRearLeft = new XYZ_Coord();
        LegGeometry.kinematicsRearLeft(jointRearLeft, xyzRearLeft);
        XYZ_Coord xyzRearRight = new XYZ_Coord();
        LegGeometry.kinematicsRearRight(jointRearRight, xyzRearRight);

        Point3D pawFL = LegGeometry.convertToCenterCoordinateFL(xyzFrontLeft);
        Point3D pawFR = LegGeometry.convertToCenterCoordinateFR(xyzFrontRight);
        Point3D pawRL = LegGeometry.convertToCenterCoordinateRL(xyzRearLeft);
        Point3D pawRR = LegGeometry.convertToCenterCoordinateRR(xyzRearRight);

        Plane3D p = new Plane3D(pawFL, pawFR, pawRL);
        double frontHeight = LegGeometry.getFrontHeight(p);
        double rearHeight = LegGeometry.getRearHeight(p);

        tableResult.setValueAt(new Double(xyzFrontLeft.x), 0, 1);
        tableResult.setValueAt(new Double(xyzFrontLeft.y), 0, 2);
        tableResult.setValueAt(new Double(xyzFrontLeft.z), 0, 3);

        tableResult.setValueAt(new Double(xyzFrontRight.x), 1, 1);
        tableResult.setValueAt(new Double(xyzFrontRight.y), 1, 2);
        tableResult.setValueAt(new Double(xyzFrontRight.z), 1, 3);

        tableResult.setValueAt(new Double(xyzRearLeft.x), 2, 1);
        tableResult.setValueAt(new Double(xyzRearLeft.y), 2, 2);
        tableResult.setValueAt(new Double(xyzRearLeft.z), 2, 3);

        tableResult.setValueAt(new Double(xyzRearRight.x), 3, 1);
        tableResult.setValueAt(new Double(xyzRearRight.y), 3, 2);
        tableResult.setValueAt(new Double(xyzRearRight.z), 3, 3);

        tableResult.setValueAt(new Double(frontHeight), 4, 1);
        tableResult.setValueAt(new Double(rearHeight), 5, 1);
        tableResult.setValueAt(new Double(LegGeometry.getBodyTilt(frontHeight,rearHeight)), 6, 1);

        tableResult.repaint();

    }

    //dodgy drawing, will not retained after a repaint.
    private void drawHorizon(Graphics g, int x1,int y1, int x2, int y2){

        x1 = Utils.toImagePositionX(x1);
        y1 = Utils.toImagePositionY(y1);
        x2 = Utils.toImagePositionX(x2);
        y2 = Utils.toImagePositionY(y2);
        //divive by 2
        x1 /= 2; y1 /= 2; x2 /= 2; y2 /= 2;
        g.clearRect(0,0,105,80);
//        g.setColor(Color.BLACK);
//        g.drawLine(x1,y1,x2,y2);
        g.setColor(Color.RED);
        g.drawLine(x1,y1,x2,y2);
    }

    private void calculateHorizon(Matrix camera2World){
        int[] result = HeadGeometry.calculateHorizon(camera2World);

        if (result != null){
            jtfHorizon1.setText("" + result[0] + " , " + result[1]);
            jtfHorizon2.setText("" + result[2] + " , " + result[3]);
            drawHorizon( horizonPanel.getGraphics(), result[0],result[1],result[2],result[3] );
        }
        else{
            jtfHorizon1.setText("n/a");
            jtfHorizon2.setText("n/a");
        }

    }

    private void calculateCameraKinematics() {
        double pan = ( (Double) tableSensor.getValueAt(SensorDefinition.ssHEAD_PAN, 1)).doubleValue();
        pan = Utils.radians(pan);
        double tilt = ( (Double) tableSensor.getValueAt(SensorDefinition.ssHEAD_TILT1, 1)).doubleValue();
        tilt = Utils.radians(tilt);
        double crane = ( (Double) tableSensor.getValueAt(SensorDefinition.ssHEAD_CRANE, 1)).doubleValue();
        crane = Utils.radians(crane);
        double neckHeight = Double.parseDouble(jtfNeckHeight.getText());
        double bodyTilt = Double.parseDouble(jtfBodyTilt.getText());
        bodyTilt = Utils.radians(bodyTilt);
        double ipx = Double.parseDouble(jtfIPX.getText());
        double ipy = Double.parseDouble(jtfIPY.getText());

        Matrix camera2World = TransformationMatrix.cameraToWorld(pan,tilt,crane,neckHeight,bodyTilt);
        jtfElevation.setText(new DecimalFormat("00.00").format(Utils.getElevation(ipx,ipy,camera2World) ) );

        jtfHeading.setText( new DecimalFormat("00.00").format(Utils.getHeading(ipx,ipy,camera2World) ) );

        calculateHorizon(camera2World);

        RobotDefinition.bodyTilt = Utils.degrees(bodyTilt);
        RobotDefinition.neckHeight = neckHeight;

        /* Attempted to calculate horizon without binary search
         * i'm now convinved that the roll the same direction with the horizon line
         */
        Matrix camera2WorldTranslated = TransformationMatrix.translateToZero(camera2World);
        Matrix world2Camera = camera2WorldTranslated.transpose();

        Vector3D vLeft = new Vector3D(1,0,0);
        Vector3D vForward = new Vector3D(0,1,0);

        vLeft.transform(world2Camera);
        vForward.transform(world2Camera);

        Vector3D nV = vLeft.cross(vForward);
        Vector3D horizonVector = nV.cross( new Vector3D(0,1,0) );
        jtfRollVector.setText(new DecimalFormat("0.0").format(horizonVector.getX()) + "," +
                              new DecimalFormat("0.0").format(horizonVector.getZ()));
        jtfRollAngle.setText(new DecimalFormat("00.00").format( Utils.degrees(Math.atan2(horizonVector.getZ(), horizonVector.getX() ) ) ) );
        System.out.println("Horizon vector " + horizonVector);
        System.out.println("Angle " + Utils.degrees(Math.atan2(horizonVector.getZ(), horizonVector.getX() ) )) ;
    }

    void btCalculate_actionPerformed(ActionEvent e) {
        calculateLegKinematics();
        calculateCameraKinematics();
    }

    public void setSensorData(int [] sensorData){
        for (int i=0;i<sensorData.length;i++)
            tableSensor.setValueAt(new Double(Utils.micro2deg(sensorData[i])) , i, 1);
        tableSensor.repaint();
    }

    void slider_stateChanged(ChangeEvent e) {
        JSlider sl = (JSlider) e.getSource();
        ( (JLabel)sl.getComponent(0)).setText("" + sl.getValue());
    }

    private void regenerateHorizon() {
        limitPan = jslPan.getValue();
        limitTilt = jslTilt.getValue();
        limitCrane = jslCrane.getValue();
        limitBodyTilt = jslBodyTilt.getValue();
        limitNeckHeight = jslNeckHeight.getValue();
        limitNeckLength = jslNeckLength.getValue();

        double randomPan = pan + r.nextInt(2 * limitPan + 1) - limitPan;
        double randomTilt = tilt + r.nextInt(2 * limitTilt + 1) - limitTilt;
        double randomCrane = crane + r.nextInt(2 * limitCrane + 1) - limitCrane;
        double randomBodyTilt = bodyTilt + r.nextInt(2 * limitBodyTilt + 1) -
            limitBodyTilt;
        double randomNeckHeight = neckHeight *
            (100 + r.nextInt(2 * limitNeckHeight + 1) - limitNeckHeight) / 100;
        double randomNeckLength = RobotDefinition.NECK_LENGTH * (100 +
            r.nextInt(2 * limitNeckLength + 1) - limitNeckHeight) / 100;

        randomPan = Utils.radians(randomPan);
        randomTilt = Utils.radians(randomTilt);
        randomCrane = Utils.radians(randomCrane);
        randomBodyTilt = Utils.radians(randomBodyTilt);


        Matrix camera2World = TransformationMatrix.
            paramaterizedCameraToWorld(randomPan,
                                       randomTilt, randomCrane,
                                       randomNeckHeight,randomBodyTilt,
                                       randomNeckLength);


        calculateHorizon(camera2World);

    }

    void btStart_actionPerformed(ActionEvent e) {



        pan = ( (Double) tableSensor.getValueAt(SensorDefinition.ssHEAD_PAN, 1)).doubleValue();

        tilt = ( (Double) tableSensor.getValueAt(SensorDefinition.ssHEAD_TILT1, 1)).doubleValue();

        crane = ( (Double) tableSensor.getValueAt(SensorDefinition.ssHEAD_CRANE, 1)).doubleValue();
        neckHeight = Double.parseDouble(jtfNeckHeight.getText());
        bodyTilt = Double.parseDouble(jtfBodyTilt.getText());

        r.setSeed(123123);
        timer.start();
    }

    void btStop_actionPerformed(ActionEvent e) {
        timer.stop();
    }
}

class SensorTableModel extends AbstractTableModel
    implements TableModel{
    String [] columnsHeader = {"Sensor" , "Value"};
    String [] sensorName = {
        "Mouth",
        "Chin",
        "Head Crane",
        "ssHEAD??",
        "Infrared Near",
        "Infrared far", //5
        "Head Pan",
        "Head Tilt",
        "Front Left Palm",
        "Front Left Knee",
        "Front Left Shoulder",
        "Front Left Rotator",
        "Rear Left Palm",
        "Rear Left Knee",
        "Rear Left Shoulder",
        "Rear Left Rotator",
        "Front Right Palm",
        "Front Right Knee",
        "Front Right Shoulder",
        "Front Right Rotator",
        "Rear Right Palm",
        "Rear Right Knee",
        "Rear Right Shoulder",
        "Rear Right Rotator",
        "Tail Horizontal",
        "Tail Vertical",
        "Accelator Forward",
        "Accelator Side",
        "Accelator Z",
        "Chest Infrared",
        "WLAN button",
        "Back rear button",
        "Back middle button",
        "Back front button"
    };

    double [] sensorValue = new double[sensorName.length];

    public SensorTableModel(){

    }

    public int getColumnCount(){
        return columnsHeader.length;
    }
    public int getRowCount(){
        return sensorName.length;
    }

    public boolean isCellEditable(int row,int col){
        return col > 0 ;
    }

    public Class getColumnClass(int columnIndex){
//        System.out.println(getValueAt(0,columnIndex).getClass());
        return getValueAt(0,columnIndex).getClass();
    }

    public String getColumnName(int col){
        return columnsHeader[col];
    }

    public Object getValueAt(int row,int col){
        if (col == 0){
            return sensorName[row];
        }
        else{
            return new Double(sensorValue[row]);
        }
    }
    public void setValueAt(Object o, int row,int col){
        if (row > 0 && row  < sensorValue.length && col == 1 && o instanceof Double){
            sensorValue[row] = ((Double)o).doubleValue();
        }
    }
}

class ResultTable extends AbstractTableModel
    implements TableModel{
    String [] columnsHeader = {"Name" , "X", "Y", "Z"};
    String [] resultName = {
        "Front Left Paw",
        "Front Right Paw",
        "Rear Left Paw",
        "Rear Right Paw",
        "Front Height",
        "Rear Height",
        "Body Tilt"
    };

    double [][] resultValue = new double[resultName.length][3];

    public ResultTable(){

    }

    public int getColumnCount(){
        return columnsHeader.length;
    }
    public int getRowCount(){
        return resultName.length;
    }

    public boolean isCellEditable(int row,int col){
        return col > 0 ;
    }

    public Class getColumnClass(int columnIndex){
//        System.out.println(getValueAt(0,columnIndex).getClass());
        return getValueAt(0,columnIndex).getClass();
    }

    public String getColumnName(int col){
        return columnsHeader[col];
    }

    public Object getValueAt(int row,int col){
        if (col == 0){
            return resultName[row];
        }
        else{
            return new Double(resultValue[row][col-1]);
        }
    }
    public void setValueAt(Object o, int row,int col){
        if (row >= 0 && row  < resultValue.length && col >= 1 && col <= 3 && o instanceof Double){
            resultValue[row][col-1] = ((Double)o).doubleValue();
        }
    }
}

class JointTool_btCalculate_actionAdapter implements java.awt.event.ActionListener {
    JointTool adaptee;

    JointTool_btCalculate_actionAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btCalculate_actionPerformed(e);
    }
}

class JointTool_jslPan_changeAdapter implements javax.swing.event.ChangeListener {
    JointTool adaptee;

    JointTool_jslPan_changeAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void stateChanged(ChangeEvent e) {
        adaptee.slider_stateChanged(e);
    }
}

class JointTool_btStart_actionAdapter implements java.awt.event.ActionListener {
    JointTool adaptee;

    JointTool_btStart_actionAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btStart_actionPerformed(e);
    }
}

class JointTool_jslTilt_changeAdapter implements javax.swing.event.ChangeListener {
    JointTool adaptee;

    JointTool_jslTilt_changeAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void stateChanged(ChangeEvent e) {
        adaptee.slider_stateChanged(e);
    }
}

class JointTool_jslCrane_changeAdapter implements javax.swing.event.ChangeListener {
    JointTool adaptee;

    JointTool_jslCrane_changeAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void stateChanged(ChangeEvent e) {
        adaptee.slider_stateChanged(e);
    }
}

class JointTool_jslBodyTilt_changeAdapter implements javax.swing.event.ChangeListener {
    JointTool adaptee;

    JointTool_jslBodyTilt_changeAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void stateChanged(ChangeEvent e) {
        adaptee.slider_stateChanged(e);
    }
}

class JointTool_jslNeckHeight_changeAdapter implements javax.swing.event.ChangeListener {
    JointTool adaptee;

    JointTool_jslNeckHeight_changeAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void stateChanged(ChangeEvent e) {
        adaptee.slider_stateChanged(e);
    }
}

class JointTool_jslNeckLength_changeAdapter implements javax.swing.event.ChangeListener {
    JointTool adaptee;

    JointTool_jslNeckLength_changeAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void stateChanged(ChangeEvent e) {
        adaptee.slider_stateChanged(e);
    }
}

class JointTool_btStop_actionAdapter implements java.awt.event.ActionListener {
    JointTool adaptee;

    JointTool_btStop_actionAdapter(JointTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btStop_actionPerformed(e);
    }
}
