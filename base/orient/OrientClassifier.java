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
 * UNSW 2003 Robocup (Eileen Mak)
 *
 * Last modification background information
 * $Id: OrientClassifier.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Orientation classification tool
 *
**/

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;



public class OrientClassifier extends JFrame {

    //--- fields -------------------------------------------------------
    JButton bnTrain;
    JButton bnReTrain;
    JButton bnTest;
    JLabel  lbPos;
    JButton bnBack;
    JButton bnNext;
    JButton bnSave;
    JButton bnGenData;
    
    ImageDisplay imgDisplay;
    
    JRadioButton cgbackward;
    JRadioButton cgright;
    JRadioButton cgforward;
    JRadioButton cgleft;
    JRadioButton cgunclass;
    JRadioButton cgrightb;
    JRadioButton cgleftb;
    JRadioButton cgrightf;
    JRadioButton cgleftf;
    JRadioButton cgdiscard;
    ButtonGroup  classGroup;
    JPanel       classPanel;
    
    OrientClassifier self;
    
    String   trainingDir;
    String[] trainingSet;
    HashMap  trainingMap;
    int      trainingPos;
    
    
    
    //--- constants ----------------------------------------------------
    public static int BACKWARD       = 0;
    public static int RIGHT          = 90;
    public static int FORWARD        = 180;
    public static int LEFT           = -90;
    public static int RIGHT_BACKWARD = 45;
    public static int LEFT_BACKWARD  = -45;
    public static int RIGHT_FORWARD  = 135;
    public static int LEFT_FORWARD   = -135;
    public static int UNCLASSIFIED   = -1;
    public static int DISCARD        = -2;




    /**
     * Init layout and listeners
     */
    public OrientClassifier() {
    
        getContentPane().setLayout(new BorderLayout());
        setTitle("Orientation Classifier");
        self = this;
        
        
        //--- robot image display --------------------------------------
        imgDisplay = new ImageDisplay();
        getContentPane().add(imgDisplay, BorderLayout.CENTER);
        
        
        //--- control panel objects ------------------------------------
        JButton bnExit = new JButton("Exit");
        bnExit.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                System.exit(0);
            }
        });
        
        bnTrain   = new JButton("Training");
        bnTrain.addActionListener(new TrainingListener());
        
        bnReTrain = new JButton("ReTrain");
        bnReTrain.addActionListener(new ReTrainListener());
        
        bnTest    = new JButton("Test");
        bnTest.addActionListener(new TestListener());
        
        lbPos     = new JLabel("", JLabel.CENTER);
        
        bnBack    = new JButton("< Back");
        bnBack.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                displayTrainingSet(false);
            }
        });
        
        bnNext    = new JButton("> Next");
        bnNext.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                displayTrainingSet(true);
            }
        });
        
        bnSave    = new JButton("Save");
        bnSave.addActionListener(new SaveListener());
        
        bnGenData = new JButton("Generate Data");
        bnGenData.addActionListener(new GenerateDataListener());
        
        
        //--- control panel layout -------------------------------------
        JPanel controlPanel = new JPanel();
        controlPanel.setLayout(new GridLayout(10,1,5,2));
        controlPanel.add(bnExit);
        controlPanel.add(bnTrain);
        controlPanel.add(bnReTrain);
        controlPanel.add(bnTest);
        controlPanel.add(new JSeparator());
        controlPanel.add(lbPos);
        controlPanel.add(bnBack);
        controlPanel.add(bnNext);
        controlPanel.add(bnSave);
        controlPanel.add(bnGenData);
        enableTrainButtons(false);
        getContentPane().add(controlPanel, BorderLayout.EAST);
        
        
        //--- classification panel objects -----------------------------
        cgbackward = new JRadioButton("0 [backward]");
        cgright    = new JRadioButton("90 [right]");
        cgforward  = new JRadioButton("180 [forward]");
        cgleft     = new JRadioButton("-90 [left]");
        cgunclass  = new JRadioButton("unclassified");
        cgrightb   = new JRadioButton("45 [right backward]");
        cgleftb    = new JRadioButton("-45 [left backword]");
        cgrightf   = new JRadioButton("135 [right forward]");
        cgleftf    = new JRadioButton("-135 [left forward]");
        cgdiscard  = new JRadioButton("discard");
        
        classGroup = new ButtonGroup();
        classGroup.add(cgbackward);
        classGroup.add(cgright);
        classGroup.add(cgforward);
        classGroup.add(cgleft);
        classGroup.add(cgunclass);
        classGroup.add(cgrightb);
        classGroup.add(cgleftb);
        classGroup.add(cgrightf);
        classGroup.add(cgleftf);
        classGroup.add(cgdiscard);
        
        
        //--- classification panel layout ------------------------------
        classPanel = new JPanel();
        classPanel.setLayout(new GridLayout(3,4,5,5));
        classPanel.add(cgleftb);
        classPanel.add(cgbackward);
        classPanel.add(cgrightb);
        classPanel.add(cgunclass);
        classPanel.add(cgleft);
        classPanel.add(cgdiscard);
        classPanel.add(cgright);
        classPanel.add(new JLabel());
        classPanel.add(cgleftf);        
        classPanel.add(cgforward);
        classPanel.add(cgrightf);
        getContentPane().add(classPanel, BorderLayout.SOUTH);
        
        
        //--- show it --------------------------------------------------
        setSize(new Dimension(670,520));
        setVisible(true);
    }
    
    
    
    /**
     * Button controls
     */
    public void enableTrainButtons(boolean enable) {
        bnBack.setEnabled(enable);
        bnNext.setEnabled(enable);
        bnSave.setEnabled(enable);
        bnGenData.setEnabled(enable);
    }
    
    
    
    /**
     * Display the next training image if isNext is true, otherwise
     * move back to the last image. trainingPos always points to the
     * current image on display.
     */
    public void displayTrainingSet(boolean isNext) {
    
        //--- record classification --------------------------------
        recordCurrentClassification();
    
        //--- update position --------------------------------------
        if (isNext) {
            trainingPos++;
        } else {
            trainingPos--;
        }
        
        
        //--- display image ----------------------------------------
        imgDisplay.loadImageFile(trainingDir + "/" + trainingSet[trainingPos]);
        
        
        //--- change button status ---------------------------------
        if (trainingPos == trainingSet.length-1)
            bnNext.setEnabled(false);
        else
            bnNext.setEnabled(true);
            
        if (trainingPos == 0)
            bnBack.setEnabled(false);
        else
            bnBack.setEnabled(true);
        
        lbPos.setText((trainingPos+1) + " / " + trainingSet.length);
        
        
        //--- change classification --------------------------------
        if (trainingMap.containsKey(trainingSet[trainingPos])) {
            TrainingData d = (TrainingData)trainingMap.get(trainingSet[trainingPos]);
            if (d.classi == BACKWARD) {
                classGroup.setSelected(cgbackward.getModel(),true);
            } else if (d.classi == RIGHT) {
                classGroup.setSelected(cgright.getModel(),true);
            } else if (d.classi == FORWARD) {
                classGroup.setSelected(cgforward.getModel(),true);
            } else if (d.classi == LEFT) {
                classGroup.setSelected(cgleft.getModel(),true);
            } else if (d.classi == RIGHT_BACKWARD) {
                classGroup.setSelected(cgrightb.getModel(),true);
            } else if (d.classi == LEFT_BACKWARD) {
                classGroup.setSelected(cgleftb.getModel(),true);
            } else if (d.classi == RIGHT_FORWARD) {
                classGroup.setSelected(cgrightf.getModel(),true);
            } else if (d.classi == LEFT_FORWARD) {
                classGroup.setSelected(cgleftf.getModel(),true);
            } else if (d.classi == UNCLASSIFIED) {
                classGroup.setSelected(cgunclass.getModel(),true);
            } else if (d.classi == DISCARD) {
                classGroup.setSelected(cgdiscard.getModel(),true);
            }
        } else {
            classGroup.setSelected(cgunclass.getModel(),true);
        }
        classPanel.updateUI();
    }
    
    
    
    /**
     * Record current classification
     */
    public void recordCurrentClassification() {
        if (trainingPos >= 0 && trainingPos <trainingSet.length) {
            TrainingData d = new TrainingData();
            if (classGroup.isSelected(cgbackward.getModel())) {
                d.classi = BACKWARD;
            } else if (classGroup.isSelected(cgright.getModel())) {
                d.classi = RIGHT;
            } else if (classGroup.isSelected(cgforward.getModel())) {
                d.classi = FORWARD;
            } else if (classGroup.isSelected(cgleft.getModel())) {
                d.classi = LEFT;
            } else if (classGroup.isSelected(cgrightb.getModel())) {
                d.classi = RIGHT_BACKWARD;
            } else if (classGroup.isSelected(cgleftb.getModel())) {
                d.classi = LEFT_BACKWARD;
            } else if (classGroup.isSelected(cgrightf.getModel())) {
                d.classi = RIGHT_FORWARD;
            } else if (classGroup.isSelected(cgleftf.getModel())) {
                d.classi = LEFT_FORWARD;
            } else if (classGroup.isSelected(cgunclass.getModel())) {
                d.classi = UNCLASSIFIED;
            } else if (classGroup.isSelected(cgdiscard.getModel())) {
                d.classi = DISCARD;
            }
            d.calAttr(imgDisplay.getImage());
            trainingMap.put(trainingSet[trainingPos], d);
        }
    }



    /**
     * execute
     */
    public static void main(String[] args) {
        OrientClassifier oc = new OrientClassifier();    
    }
    
    
    
    
    
    ////////////////////////////////////////////////////////////////////
    // Action listeners
    ////////////////////////////////////////////////////////////////////
    
    /**
     * Do training
     */
    private class TrainingListener implements ActionListener {
        public void actionPerformed(ActionEvent e) {
        
            //--- allow user to choose training set dir ---------------
            JFileChooser fc = new JFileChooser();
            fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            fc.setMultiSelectionEnabled(false);
            fc.setDialogTitle("Select training set directory");
            int selectOp = fc.showDialog(self, "Start");
            
            
            //--- a dir with training set is chosen -------------------
            if (selectOp == JFileChooser.APPROVE_OPTION) {
          
                File     dir  = fc.getSelectedFile();
                String[] list = dir.list();      //-- include index if exists
                
                
                //--- init global training data -----------------------
                trainingDir   = fc.getSelectedFile().getAbsolutePath();
                trainingMap   = new HashMap();
                trainingPos   = -1;
                
                
                //--- init training set -------------------------------
                String indexFile = trainingDir + "/classification.index";
                if ((new File(indexFile)).exists()) {
                    trainingSet = new String[list.length-1];
                    int count = 0;
                    for (int i=0; i<list.length; i++) {
                        if (list[i].equals("classification.index"))
                            continue;
                        trainingSet[count] = list[i];
                        count++;
                    }
                } else {
                    trainingSet = list;
                }
                
                
                //--- load existing data from index -------------------
                if ((new File(indexFile)).exists()) {
                    try {
                        BufferedReader reader = new BufferedReader(new FileReader(indexFile));
                        String         token, line, filename = "";
                        while (true) {
                            line = reader.readLine();
                            if (line == null)
                                break;
                                
                            StringTokenizer st   = new StringTokenizer(line);
                            int             num  = st.countTokens();
                            String[]        attr = new String[num - 2];
                            TrainingData    data = new TrainingData();
                            for (int i=0; i<num; i++) {
                                token = st.nextToken();
                                if (i==0)
                                    filename = token;
                                else if (i==num-1)
                                    data.classi = Integer.parseInt(token);
                                else
                                    attr[i-1] = token;
                            }
                            data.attributes = attr;
                            trainingMap.put(filename,data);
                        }
                    } catch (Exception ex) {
                        ex.printStackTrace();
                    }
                }
                
                enableTrainButtons(true);
                displayTrainingSet(true);
            }
        }
    }
    
    
    
    
    /**
     * Save index file
     */
    private class SaveListener implements ActionListener {
        public void actionPerformed(ActionEvent e) {
        
            recordCurrentClassification();
            
            try {
                PrintWriter writer = new PrintWriter(new FileWriter(trainingDir + "/classification.index"));
                ArrayList list = new ArrayList(trainingMap.keySet());
                for (int i=0; i<list.size(); i++) {
                    writer.println(list.get(i) + "" + trainingMap.get(list.get(i)));
                }
                writer.flush();
                writer.close();
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }
    
    
    
    
    /**
     * Generate data and names file for C4.5
     */
    private class GenerateDataListener implements ActionListener {
        public void actionPerformed(ActionEvent e) {
    
            recordCurrentClassification();
            
            try {
        
                //--- choose directory to generate training data to ------------
                JFileChooser fc = new JFileChooser();
                fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
                fc.setMultiSelectionEnabled(false);
                fc.setDialogTitle("Select a directory");
                int    selectOp  = fc.showDialog(self, "Generate");
                String selectDir = fc.getSelectedFile().getAbsolutePath();
        
        
                //--- generate Orient.names ------------------------------------
                PrintWriter writer = new PrintWriter(new FileWriter(selectDir + "/Orient.names"));
                for (int i=0; i<TrainingData.classifications.length; i++) {
                    if(i!=0) {
                        writer.print(", ");
                    }
                    writer.print(TrainingData.classifications[i]);
                }
                writer.println(".\n");
                
                for (int i=0; i<TrainingData.attributeNames.length; i++) {
                    writer.print(TrainingData.attributeNames[i] + ": ");
                    writer.println(TrainingData.attributeValues[i] + ".");
                }
                writer.flush();
                writer.close();
        
        
                //--- generate Orient.data -------------------------------------
                writer = new PrintWriter(new FileWriter(selectDir + "/Orient.data"));
                ArrayList list = new ArrayList(trainingMap.values());
                for (int i=0; i<list.size(); i++) {
                    TrainingData data = (TrainingData)list.get(i);
                    if (data.classi == UNCLASSIFIED || data.classi == DISCARD) {
                        continue;
                    }
                    writer.println(data.toDataString());
                }
                writer.flush();
                writer.close();
                
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }
    
    
    
    /**
     * Redo training on classified images with new feature extraction
     * methods.
     */
    private class ReTrainListener implements ActionListener {
        public void actionPerformed(ActionEvent e) {
      
            //--- allow user to choose training set dir ---------------
            JFileChooser fc = new JFileChooser();
            fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            fc.setMultiSelectionEnabled(false);
            fc.setDialogTitle("Select training set directory");
            int selectOp = fc.showDialog(self, "Start");
            
            
            //--- a dir with training set is chosen -------------------
            if (selectOp == JFileChooser.APPROVE_OPTION) {
          
                File     dir  = fc.getSelectedFile();
                String[] list = dir.list();      //-- include index if exists
                
                
                //--- init global training data -----------------------
                trainingDir   = fc.getSelectedFile().getAbsolutePath();
                trainingMap   = new HashMap();
                trainingPos   = -1;
                
                
                //--- init training set -------------------------------
                String indexFile = trainingDir + "/classification.index";
                if ((new File(indexFile)).exists()) {
                    trainingSet = new String[list.length-1];
                    int count = 0;
                    for (int i=0; i<list.length; i++) {
                        if (list[i].equals("classification.index"))
                            continue;
                        trainingSet[count] = list[i];
                        count++;
                    }
                } else {
                    JOptionPane.showMessageDialog(self,"No classification data");
                    return;
                }
                
                
                //--- load existing data from index -------------------
                if ((new File(indexFile)).exists()) {
                    try {
                        BufferedReader reader = new BufferedReader(new FileReader(indexFile));
                        String         token, line, filename = "";
                        while (true) {
                            line = reader.readLine();
                            if (line == null)
                                break;
                                
                            StringTokenizer st   = new StringTokenizer(line);
                            int             num  = st.countTokens();
                            String[]        attr = new String[num - 2];
                            TrainingData    data = new TrainingData();
                            for (int i=0; i<num; i++) {
                                token = st.nextToken();
                                if (i==0)
                                    filename = token;
                                else if (i==num-1)
                                    data.classi = Integer.parseInt(token);
                                else
                                    attr[i-1] = token;
                            }
                            data.attributes = attr;
                            trainingMap.put(filename,data);
                        }
                    } catch (Exception ex) {
                        ex.printStackTrace();
                    }
                }
                
                
                //--- re-calculate all attributes -------------------------
                enableTrainButtons(true);
                for (int pos=0; pos<trainingSet.length; pos++) {
                    bnNext.doClick();
                    
                    //--- following line shows status of re-training
                    //    comment it out if want faster processing
                    System.out.println("Calculating " + trainingSet[pos]);
                }
                imgDisplay.repaint();
            }
        }
    }
    
    
    
    
    /**
     * Do testing on testing set (by specifying the directory
     * containing the testing set)
     */
    private class TestListener implements ActionListener {
        public void actionPerformed(ActionEvent e) {
        
            //--- allow user to choose testing set dir ---------------
            JFileChooser fc = new JFileChooser();
            fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            fc.setMultiSelectionEnabled(false);
            fc.setDialogTitle("Select testing set directory");
            int selectOp = fc.showDialog(self, "Start");
            
            
            //--- a dir with testing set is chosen -------------------
            if (selectOp == JFileChooser.APPROVE_OPTION) {
                String       dir    = fc.getSelectedFile().getAbsolutePath();
                OrientTester tester = new OrientTester(dir);
            }
        }
    }

}
