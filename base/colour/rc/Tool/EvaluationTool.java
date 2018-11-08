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

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import RC.*;
import RoboShare.*;
import java.io.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class EvaluationTool
    extends JFrame {
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JPanel panel1 = new JPanel();
    JLabel jLabel1 = new JLabel();
    JTextField jtfBallImagesDir = new JTextField();
    JLabel jLabel2 = new JLabel();
    JTextField jtfGoalImagesDir = new JTextField();
    JLabel jLabel3 = new JLabel();
    JTextField jtfBeaconImagesDir = new JTextField();
    JButton btChooseBeaconImagesDir = new JButton();
    JButton btChooseGoalImagesDir = new JButton();
    JButton btChooseBallImagesDir = new JButton();
    JTextField jtfColourRDR = new JTextField();
    JButton btChooseCollourRDR = new JButton();
    JCheckBox chbColourRDR = new JCheckBox();
    JButton btEvaluateBall = new JButton();
    JTextField jtfBallRDR = new JTextField();
    JButton btBallRDR = new JButton();
    JTextField jtfGoalRDR = new JTextField();
    JButton btGoalRDR = new JButton();
    JTextField jtfBeaconRDR = new JTextField();
    JButton btBeaconRDR = new JButton();
    JButton btEvaluateGoal = new JButton();
    JButton btEvaluateBeacon = new JButton();

    JFileChooser optFileChooser = new JFileChooser();
    JFileChooser optDirChooser = new JFileChooser();
    BorderLayout borderLayout1 = new BorderLayout();

    RDRApplication parent;
    JScrollPane jScrollPane1 = new JScrollPane();
    JTextArea jtaStatus = new JTextArea();

    int totalImages = 0;
    int mistakeImage = 0;
    int imageFalsePositive = 0;
    int imageFalseNegative = 0;

    int totalExamples = 0;
    int mistakeMade = 0;
    int falsePositive = 0;
    int falseNegative = 0;
    JButton btClear = new JButton();

    String[] allBFLFiles;
    File directory;
    JTextField jtfBlobRDR = new JTextField();
    JButton btBlobRDR = new JButton();
    JComboBox cbxBallRDRType = new JComboBox();
    JComboBox cbxGoalRDRType = new JComboBox();
    JComboBox cbxBlobRDRType = new JComboBox();

    public EvaluationTool(RDRApplication parent) {
        super("Evaluation Tool");
        this.parent = parent;
        try {
            jbInit();
            initialiseFromCookies();
            optDirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        jLabel1.setText("Ball Images");
//        this.getContentPane().setLayout();
        this.getContentPane().setLayout(borderLayout1);
        jtfColourRDR.setMinimumSize(new Dimension(120, 20));
        jtfColourRDR.setPreferredSize(new Dimension(120, 20));
        jtfBallImagesDir.setMinimumSize(new Dimension(120, 20));
        jtfGoalImagesDir.setMinimumSize(new Dimension(120, 20));
        jtfBeaconImagesDir.setMinimumSize(new Dimension(120, 20));
        jtfBallRDR.setMinimumSize(new Dimension(120, 20));
        jtfGoalRDR.setMinimumSize(new Dimension(120, 20));
        jtfBeaconRDR.setMinimumSize(new Dimension(120, 20));
        btEvaluateBall.addActionListener(new
            EvaluationTool_btEvaluateBall_actionAdapter(this));

        jtaStatus.setText("Status board started...");
        btEvaluateGoal.addActionListener(new
            EvaluationTool_btEvaluateGoal_actionAdapter(this));
        jScrollPane1.setMinimumSize(new Dimension(163, 100));
        jScrollPane1.setPreferredSize(new Dimension(163, 100));
        btClear.setText("Clear");
        btClear.addActionListener(new EvaluationTool_btClear_actionAdapter(this));
        btEvaluateBeacon.addActionListener(new
            EvaluationTool_btEvaluateBeacon_actionAdapter(this));
        jtfBlobRDR.setToolTipText("");
        jtfBlobRDR.setText("");
        btBlobRDR.setToolTipText("");
        btBlobRDR.setText("Blob RDR");
        btBlobRDR.addActionListener(new EvaluationTool_btBlobRDR_actionAdapter(this));
        this.getContentPane().add(panel1, BorderLayout.CENTER);
        panel1.setLayout(gridBagLayout1);
        jLabel2.setText("Goal Images");
        jLabel3.setText("Beacon Images");
        jtfBallImagesDir.setText("");
        jtfGoalImagesDir.setText("");
        jtfBeaconImagesDir.setText("");
        btChooseBeaconImagesDir.setText("Choose");
        btChooseBeaconImagesDir.addActionListener(new
            EvaluationTool_btChooseBeaconImagesDir_actionAdapter(this));
        btChooseGoalImagesDir.setText("Choose");
        btChooseGoalImagesDir.addActionListener(new
            EvaluationTool_btChooseGoalImagesDir_actionAdapter(this));
        btChooseBallImagesDir.setText("Choose");
        btChooseBallImagesDir.addActionListener(new
            EvaluationTool_btChooseBallImagesDir_actionAdapter(this));
        btChooseCollourRDR.setText("Choose");
        chbColourRDR.setToolTipText("");
        chbColourRDR.setText("Use Colour RDR");
        jtfColourRDR.setText("");
        btEvaluateBall.setText("Evaluate !");
        btBallRDR.setText("Ball RDR");
        btBallRDR.addActionListener(new EvaluationTool_btBallRDR_actionAdapter(this));
        btGoalRDR.setToolTipText("");
        btGoalRDR.setText("Goal RDR");
        btGoalRDR.addActionListener(new EvaluationTool_btGoalRDR_actionAdapter(this));
        btBeaconRDR.setSelected(false);
        btBeaconRDR.setText("Beacon RDR");
        btBeaconRDR.addActionListener(new
                                      EvaluationTool_btBeaconRDR_actionAdapter(this));
        btEvaluateGoal.setText("Evaluate !");
        btEvaluateBeacon.setText("Evaluate !");
        jtfBeaconRDR.setText("");
        jtfGoalRDR.setText("");
        panel1.add(jLabel1,     new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(jtfBallImagesDir,
                       new GridBagConstraints(1, 1, 2, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jLabel2,     new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(jtfGoalImagesDir,
                       new GridBagConstraints(1, 2, 2, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jLabel3,     new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(jtfBeaconImagesDir,
                       new GridBagConstraints(1, 3, 2, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jtfColourRDR,     new GridBagConstraints(1, 0, 1, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(chbColourRDR,     new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(btChooseBallImagesDir,
                       new GridBagConstraints(3, 1, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btChooseCollourRDR,
                       new GridBagConstraints(3, 0, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btChooseGoalImagesDir,
                       new GridBagConstraints(3, 2, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btChooseBeaconImagesDir,
                       new GridBagConstraints(3, 3, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jtfBallRDR,     new GridBagConstraints(1, 4, 1, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btBallRDR,     new GridBagConstraints(3, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 0), 0, 0));
        panel1.add(jtfGoalRDR,     new GridBagConstraints(1, 5, 1, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btGoalRDR,      new GridBagConstraints(3, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jtfBeaconRDR,      new GridBagConstraints(1, 6, 1, 1, 0.8, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btBeaconRDR,     new GridBagConstraints(3, 6, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 0, 5), 0, 0));
        panel1.add(btEvaluateBall,     new GridBagConstraints(0, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btEvaluateGoal,     new GridBagConstraints(0, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btEvaluateBeacon,
                        new GridBagConstraints(0, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(jScrollPane1,     new GridBagConstraints(0, 8, 6, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(jtfBlobRDR,      new GridBagConstraints(1, 7, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(btBlobRDR,      new GridBagConstraints(3, 7, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        panel1.add(cbxBallRDRType,   new GridBagConstraints(4, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(cbxGoalRDRType,   new GridBagConstraints(4, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        panel1.add(cbxBlobRDRType, new GridBagConstraints(4, 7, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        this.getContentPane().add(btClear, BorderLayout.SOUTH);
        jScrollPane1.getViewport().add(jtaStatus, null);
        btChooseCollourRDR.addActionListener(new
            EvaluationTool_btChooseCollourRDR_actionAdapter(this));

        for (int i = 0 ; i < RuleCollection.ruleMetricOptionStrings.length;i++){
            cbxBallRDRType.addItem(RuleCollection.ruleMetricOptionStrings[i]);
            cbxGoalRDRType.addItem(RuleCollection.ruleMetricOptionStrings[i]);
            cbxBlobRDRType.addItem(RuleCollection.ruleMetricOptionStrings[i]);
        }

    }

    void initialiseFromCookies() {
        jtfColourRDR.setText(RDRApplication.appConf.getLastColourRDR());
        jtfBallImagesDir.setText(RDRApplication.appConf.getProperty(
            MyConfiguration.propBallImageDir));
        jtfGoalImagesDir.setText(RDRApplication.appConf.getProperty(
            MyConfiguration.propGoalImageDir));
        jtfBeaconImagesDir.setText(RDRApplication.appConf.getProperty(
            MyConfiguration.propBeaconImageDir));
        jtfBallRDR.setText(RDRApplication.appConf.getProperty(MyConfiguration.
            propBallRDR));
        jtfGoalRDR.setText(RDRApplication.appConf.getProperty(MyConfiguration.
            propGoalRDR));
        jtfBeaconRDR.setText(RDRApplication.appConf.getProperty(MyConfiguration.
            propBeaconRDR));
        jtfBlobRDR.setText(RDRApplication.appConf.getProperty(MyConfiguration.
            propBlobRDR));

    }

    void btChooseCollourRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getLastColourRDR()));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfColourRDR.setText(fileName);
            RDRApplication.appConf.setLastColourRDR(fileName);
        }

    }

    // Overridden so we can exit when window is closed

    protected void processWindowEvent(WindowEvent e) {
        super.processWindowEvent(e);
        if (parent == null && e.getID() == WindowEvent.WINDOW_CLOSING) {
            RDRApplication.appConf.shutDown();
            System.exit(0);
        }
    }

    void btChooseBallImagesDir_actionPerformed(ActionEvent e) {
        optDirChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propBallImageDir)));
        if (optDirChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optDirChooser.getSelectedFile().getAbsolutePath();
            jtfBallImagesDir.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propBallImageDir,
                                               fileName);
        }
    }

    void btChooseGoalImagesDir_actionPerformed(ActionEvent e) {
        optDirChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propGoalImageDir)));
        if (optDirChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optDirChooser.getSelectedFile().getAbsolutePath();
            jtfGoalImagesDir.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propGoalImageDir,
                                               fileName);
        }

    }

    void btChooseBeaconImagesDir_actionPerformed(ActionEvent e) {
        optDirChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propBeaconImageDir)));
        if (optDirChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optDirChooser.getSelectedFile().getAbsolutePath();
            jtfBeaconImagesDir.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.
                                               propBeaconImageDir,
                                               fileName);
        }

    }

    void btBallRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propBallRDR)));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfBallRDR.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propBallRDR,
                                               fileName);
        }

    }

    void btGoalRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propGoalRDR)));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfGoalRDR.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propGoalRDR,
                                               fileName);
        }

    }

    void btBeaconRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propBeaconRDR)));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfBeaconRDR.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propBeaconRDR,
                                               fileName);
        }
    }

    public static void main(String[] args) {
        EvaluationTool evaTool = new EvaluationTool(null);
        evaTool.pack();
        evaTool.setVisible(true);

    }

    public void addMessage(String message) {
        jtaStatus.append(message + "\n");
    }

    public void reportEvaluation() {
        addMessage("Result : ");
        addMessage("        False negative image: " + imageFalseNegative);
        addMessage("        False positive image : " + imageFalsePositive);
        addMessage("        Mistake image : " + mistakeImage + "(" + 100.0 * mistakeImage / totalImages +"%)");
        addMessage("    Total image : " + totalImages);
        addMessage("        False negative : " + falseNegative);
        addMessage("        False positive : " + falsePositive);
        addMessage("        Mistake : " + mistakeMade+ "(" + 100.0 * mistakeMade / totalExamples +"%)");
        addMessage("    Total : " + totalExamples);
        addMessage("------------------------------------------");
    }

    int getExpectedNumber(String fileName){
        if (fileName.startsWith("2_")){
            return 2;
        }
        else if (fileName.startsWith("3_")){
            return 3;
        }
        else if (fileName.startsWith("0_")){
            return 0;
        }
        else return 1;
    }

    public boolean evaluateBall(String ballDir, String ballRDRFile,
                                String colourRDRFile) {
        if (parent == null) {
            JOptionPane.showMessageDialog(this, "RDRApplication is not running");
            return false;
        }
        parent.getThumbnailToolbar().clearAllThumbnail();
        setupEvaluateBall(ballDir, ballRDRFile, colourRDRFile);
        try {

            for (int i = 0; i < allBFLFiles.length; i++) {
                int expectNumBall = getExpectedNumber(allBFLFiles[i]);
                String bflFileName = directory + "\\" + allBFLFiles[i];
                addMessage("Checking " + bflFileName);

                parent.bfl.readFile(bflFileName);

                addMessage(" ---------- " + parent.visionObserver.numberOfBlob +
                           "  " + parent.visionObserver.numberOfGoodBlob +
                           "  " + parent.visionObserver.numberOfObject);
                int ballSeen = parent.visionObserver.numberOfGoodBlob;

                totalExamples += expectNumBall;
                totalImages++;

                if (ballSeen != expectNumBall){
                    if (ballSeen > expectNumBall) {
                        falsePositive += ballSeen - expectNumBall;
                        imageFalsePositive++;
                    }
                    else {
                        falseNegative += expectNumBall - ballSeen;
                        imageFalseNegative += 1;
                    }
                    parent.addToThumbnailBar(bflFileName);
                }
            }
            mistakeMade = falseNegative + falsePositive;
            mistakeImage = imageFalseNegative + imageFalsePositive;
        }
        catch (FileNotFoundException ex) {
            addMessage("File not found !");
        }
        catch (IOException ex) {
            addMessage("IOException !");
        }

        reportEvaluation();

        return true;
    }

    private void setupEvaluateBall(String ballDir, String ballRDRFile,
                                   String colourRDRFile) {
        if (colourRDRFile != null) {
            parent.loadColourRDR(colourRDRFile);
        }

        ManRDRBlobInterval newRDR = new ManRDRBlobInterval(RuleCollection.ruleMetricOptions[ cbxBallRDRType.getSelectedIndex() ] );
        newRDR.loadFromFile(ballRDRFile);
        parent.toolCombinedRDR.resetBlobRDR(newRDR );

        directory = new File(ballDir);
        allBFLFiles = directory.list(new FilenameFilter() {
            public boolean accept(File dir, String filename) {
                return filename.endsWith("bfl") || filename.endsWith("BFL");
            }
        });

        addMessage("Evaluating on " + allBFLFiles.length + " images...");

        //check orange blob only
        Blobber.IS_WANTED_BLOB[CommonSense.ORANGE] = true;
        Blobber.IS_WANTED_BLOB[CommonSense.BLUE] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.GREEN] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.YELLOW] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.PINK] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.BLUE_ROBOT] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.RED_ROBOT] = false;
        resetResult();
    }

    private void resetResult() {
        totalExamples = 0;
        mistakeMade = 0;
        falsePositive = 0;
        falseNegative = 0;
        totalImages = 0;
        imageFalseNegative = 0;
        imageFalsePositive = 0;
        mistakeImage = 0;
    }

    void btEvaluateBall_actionPerformed(ActionEvent e) {
        String ballDir = jtfBallImagesDir.getText();
        if (!new File(ballDir).exists()) {
            JOptionPane.showMessageDialog(this, ballDir, "Ball Dir not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }

        String colourRDR = null;
        if (chbColourRDR.isSelected()) {
            colourRDR = jtfColourRDR.getText();
            if (! (new File(colourRDR).exists())) {
                JOptionPane.showMessageDialog(this, colourRDR,
                                              "File not exist !",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }

        String ballRDR = jtfBallRDR.getText();
        if (!new File(ballRDR).exists()) {
            JOptionPane.showMessageDialog(this, ballRDR, "Ball RDR not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }

        evaluateBall(ballDir, ballRDR, colourRDR);
    }

    public boolean evaluateGoal(String goalDir, String goalRDRFile,
                                String colourRDRFile) {
        if (parent == null) {
            JOptionPane.showMessageDialog(this, "RDRApplication is not running");
            return false;
        }
        parent.getThumbnailToolbar().clearAllThumbnail();
        setupGoalEvaluation(goalDir, goalRDRFile, colourRDRFile);
        try {

            for (int i = 0; i < allBFLFiles.length; i++) {
                int expectNumGoal = getExpectedNumber(allBFLFiles[i]);
                String bflFileName = directory + "\\" + allBFLFiles[i];
                addMessage("Checking " + bflFileName);

                parent.bfl.readFile(bflFileName);

                addMessage(" ---------- " + parent.visionObserver.numberOfBlob +
                           "  " + parent.visionObserver.numberOfGoodBlob +
                           "   " + parent.visionObserver.numberOfObject);

                int goalSeen = parent.visionObserver.numberOfGoodBlob;
                totalExamples += expectNumGoal;
                totalImages++;
                if (goalSeen != expectNumGoal){
                    if (goalSeen > expectNumGoal) {
                        falsePositive += goalSeen - expectNumGoal;
                        imageFalsePositive += 1;
                    }
                    else if (goalSeen < expectNumGoal) {
                        falseNegative += expectNumGoal - goalSeen;
                        imageFalseNegative++;
                    }
                    parent.addToThumbnailBar(bflFileName);
                }

            }
            mistakeMade = falseNegative + falsePositive;
            mistakeImage = imageFalseNegative + imageFalsePositive;
        }
        catch (FileNotFoundException ex) {
            addMessage("File not found !");
        }
        catch (IOException ex) {
            addMessage("IOException !");
        }

        reportEvaluation();

        return true;
    }

    private void setupGoalEvaluation(String goalDir, String goalRDRFile,
                                     String colourRDRFile) {
        if (colourRDRFile != null) {
            parent.loadColourRDR(colourRDRFile);
        }
        ManRDRBlobInterval newRDR = new ManRDRBlobInterval(RuleCollection.ruleMetricOptions[ cbxGoalRDRType.getSelectedIndex() ] );
        newRDR.loadFromFile(goalRDRFile);
        parent.toolCombinedRDR.resetBlobRDR(newRDR );



        directory = new File(goalDir);
        allBFLFiles = directory.list(new FilenameFilter() {
            public boolean accept(File dir, String filename) {
                return filename.endsWith("bfl") || filename.endsWith("BFL");
            }
        });

        addMessage("Evaluating on " + allBFLFiles.length + " images...");

        //check orange blob only
        Blobber.IS_WANTED_BLOB[CommonSense.ORANGE] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.BLUE] = true;
        Blobber.IS_WANTED_BLOB[CommonSense.GREEN] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.YELLOW] = true;
        Blobber.IS_WANTED_BLOB[CommonSense.PINK] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.BLUE_ROBOT] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.RED_ROBOT] = false;
        resetResult();
    }

    public boolean evaluateBeacon(String beaconDir, String beaconRDRFile,
                                  String blobRDRFile,String colourRDRFile) {
        if (parent == null) {
            JOptionPane.showMessageDialog(this, "RDRApplication is not running");
            return false;
        }
        parent.getThumbnailToolbar().clearAllThumbnail();
        setupBeaconEvaluation(beaconDir, beaconRDRFile, blobRDRFile,
                              colourRDRFile);
        Runnable timeConsumingRunnable = new Runnable() {
            public void run() {

                try {

                    for (int i = 0; i < allBFLFiles.length; i++) {
                        int expectNumBeacon = getExpectedNumber(allBFLFiles[i]);
                        String bflFileName = directory + "\\" + allBFLFiles[i];
                        addMessage("Checking " + bflFileName);

                        parent.bfl.readFile(bflFileName);

                        addMessage(" ---------- " +
                                   parent.visionObserver.numberOfBlob +
                                   "  " +
                                   parent.visionObserver.numberOfGoodBlob +
                                   "   " + parent.visionObserver.numberOfObject);

                        int beaconSeen = parent.visionObserver.numberOfObject;
                        totalExamples += expectNumBeacon;
                        totalImages++;
                        if (beaconSeen != expectNumBeacon) {
                            if (beaconSeen > expectNumBeacon) {
                                falsePositive += beaconSeen - expectNumBeacon;
                                imageFalsePositive += 1;
                            }
                            else {
                                falseNegative += expectNumBeacon - beaconSeen;
                                imageFalseNegative += 1;
                            }
                            parent.addToThumbnailBar(bflFileName);
                        }

                    }
                    mistakeMade = falseNegative + falsePositive;
                    mistakeImage = imageFalseNegative + imageFalsePositive;
                }
                catch (FileNotFoundException ex) {
                    addMessage("File not found !");
                }
                catch (IOException ex) {
                    addMessage("IOException !");
                }

                reportEvaluation();

            }
        };

        new Thread(timeConsumingRunnable).start();

        return true;
    }

    private void setupBeaconEvaluation(String beaconDir, String beaconRDRFile,
                                       String blobRDRFile, String colourRDRFile) {
        if (colourRDRFile != null) {
            parent.loadColourRDR(colourRDRFile);
        }
        ManRDRBlobInterval newRDR = new ManRDRBlobInterval(RuleCollection.ruleMetricOptions[ cbxBlobRDRType.getSelectedIndex() ] );
        newRDR.loadFromFile(blobRDRFile);
        parent.toolCombinedRDR.resetBlobRDR(newRDR);
        parent.loadBlobRelationRDR(beaconRDRFile);

        directory = new File(beaconDir);
        allBFLFiles = directory.list(new FilenameFilter() {
            public boolean accept(File dir, String filename) {
                return filename.endsWith("bfl") || filename.endsWith("BFL");
            }
        });

        addMessage("Evaluating on " + allBFLFiles.length + " images...");

        //check orange blob only
        Blobber.IS_WANTED_BLOB[CommonSense.ORANGE] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.BLUE] = true;
        Blobber.IS_WANTED_BLOB[CommonSense.GREEN] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.YELLOW] = true;
        Blobber.IS_WANTED_BLOB[CommonSense.PINK] = true;
        Blobber.IS_WANTED_BLOB[CommonSense.BLUE_ROBOT] = false;
        Blobber.IS_WANTED_BLOB[CommonSense.RED_ROBOT] = false;
        resetResult();
    }

    void btEvaluateGoal_actionPerformed(ActionEvent e) {
        String goalDir = jtfGoalImagesDir.getText();
        if (!new File(goalDir).exists()) {
            JOptionPane.showMessageDialog(this, goalDir, "Goal Dir not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }

        String colourRDR = null;
        if (chbColourRDR.isSelected()) {
            colourRDR = jtfColourRDR.getText();
            if (! (new File(colourRDR).exists())) {
                JOptionPane.showMessageDialog(this, colourRDR,
                                              "File not exist !",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }

        String goalRDR = jtfGoalRDR.getText();
        if (!new File(goalRDR).exists()) {
            JOptionPane.showMessageDialog(this, goalRDR, "Goal RDR not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }

        evaluateGoal(goalDir, goalRDR, colourRDR);

    }

    void btClear_actionPerformed(ActionEvent e) {
        jtaStatus.setText("");
    }

    void btEvaluateBeacon_actionPerformed(ActionEvent e) {
        String beaconDir = jtfBeaconImagesDir.getText();
        if (!new File(beaconDir).exists()) {
            JOptionPane.showMessageDialog(this, beaconDir,
                                          "Beacon Dir not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }

        String colourRDR = null;
        if (chbColourRDR.isSelected()) {
            colourRDR = jtfColourRDR.getText();
            if (! (new File(colourRDR).exists())) {
                JOptionPane.showMessageDialog(this, colourRDR,
                                              "Colour RDR File not exist !",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }

        String beaconRDR = jtfBeaconRDR.getText();
        if (!new File(beaconRDR).exists()) {
            JOptionPane.showMessageDialog(this, beaconRDR,
                                          "Beacon RDR not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }

        String blobRDR = jtfBlobRDR.getText();
        if (!new File(blobRDR).exists()) {
            JOptionPane.showMessageDialog(this, beaconRDR,
                                          "Blob RDR not exist !",
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }


        evaluateBeacon(beaconDir, beaconRDR,blobRDR, colourRDR);

    }

    void btBlobRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propBlobRDR)));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfBlobRDR.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propBlobRDR,
                                               fileName);
        }

    }

}

class EvaluationTool_btChooseCollourRDR_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btChooseCollourRDR_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btChooseCollourRDR_actionPerformed(e);
    }
}

class EvaluationTool_btChooseBallImagesDir_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btChooseBallImagesDir_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btChooseBallImagesDir_actionPerformed(e);
    }
}

class EvaluationTool_btChooseGoalImagesDir_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btChooseGoalImagesDir_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btChooseGoalImagesDir_actionPerformed(e);
    }
}

class EvaluationTool_btChooseBeaconImagesDir_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btChooseBeaconImagesDir_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btChooseBeaconImagesDir_actionPerformed(e);
    }
}

class EvaluationTool_btBallRDR_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btBallRDR_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btBallRDR_actionPerformed(e);
    }
}

class EvaluationTool_btGoalRDR_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btGoalRDR_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btGoalRDR_actionPerformed(e);
    }
}

class EvaluationTool_btBeaconRDR_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btBeaconRDR_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btBeaconRDR_actionPerformed(e);
    }
}

class EvaluationTool_btEvaluateBall_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btEvaluateBall_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btEvaluateBall_actionPerformed(e);
    }
}

class EvaluationTool_btEvaluateGoal_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btEvaluateGoal_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btEvaluateGoal_actionPerformed(e);
    }
}

class EvaluationTool_btClear_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btClear_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btClear_actionPerformed(e);
    }
}

class EvaluationTool_btEvaluateBeacon_actionAdapter
    implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btEvaluateBeacon_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btEvaluateBeacon_actionPerformed(e);
    }
}

class EvaluationTool_btBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    EvaluationTool adaptee;

    EvaluationTool_btBlobRDR_actionAdapter(EvaluationTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btBlobRDR_actionPerformed(e);
    }
}
