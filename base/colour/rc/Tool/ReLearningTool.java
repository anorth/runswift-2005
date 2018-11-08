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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import RC.*;
import java.io.*;
import RoboShare.*;
import java.util.*;


/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class ReLearningTool extends JFrame {
    JFileChooser optFileChooser = new JFileChooser();
    JFileChooser optDirChooser = new JFileChooser();
    RDRApplication parent;
    int totalImages = 0;
    int mistakeImage = 0;
    int imageFalsePositive = 0;
    int imageFalseNegative = 0;

    int totalExamples = 0;
    int mistakeMade = 0;
    int falsePositive = 0;
    int falseNegative = 0;
    String [] allBFLFiles = null;
    File directory = null;



    BorderLayout borderLayout1 = new BorderLayout();
    JPanel jPanel1 = new JPanel();
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JTextField jtfBFLDirectory = new JTextField();
    JButton btOpenBFLDir = new JButton();
    JTextField jtfOriginalBlobRDR = new JTextField();
    JButton btOpenOriginalRDR = new JButton();
    JTextField jtfNewBlobRDR = new JTextField();
    JButton btOpenNewRDR = new JButton();
    JScrollPane jScrollPane1 = new JScrollPane();
    JTextArea jtaLog = new JTextArea();
    JComboBox cbxNewRuleType = new JComboBox();
    JButton btConvertBlobRDR = new JButton();
    JButton btEvaluateBlob = new JButton();
    JButton btTestingBFL = new JButton();
    JTextField jtfTestingBFL = new JTextField();
    JComboBox cbxRDRType = new JComboBox();
    JButton btClear = new JButton();


    public ReLearningTool(RDRApplication rdrApp) {
        super("ReLearning Tool");
        this.parent = rdrApp;
        optDirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        try {
            jbInit();
            initFromCookies();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    void initFromCookies(){
        jtfBFLDirectory.setText(RDRApplication.appConf.getProperty(MyConfiguration.propEvaluateBFL));
        jtfNewBlobRDR.setText(RDRApplication.appConf.getProperty(MyConfiguration.propOutputRDR));
        jtfOriginalBlobRDR.setText(RDRApplication.appConf.getProperty(MyConfiguration.propBlobRDR));
        jtfTestingBFL.setText(RDRApplication.appConf.getProperty(MyConfiguration.propTestingBFL));
    }

    void jbInit() throws Exception {
        this.getContentPane().setLayout(borderLayout1);
        jPanel1.setLayout(gridBagLayout1);
        btOpenBFLDir.setText("Training BFL ");
        btOpenBFLDir.addActionListener(new ReLearningTool_btOpenBFLDir_actionAdapter(this));
        btOpenOriginalRDR.setText("Original Blob RDR");
        btOpenOriginalRDR.addActionListener(new ReLearningTool_btOpenOriginalRDR_actionAdapter(this));
        btOpenNewRDR.setText("New Blob RDR");
        btOpenNewRDR.addActionListener(new ReLearningTool_btOpenNewRDR_actionAdapter(this));
        jtfBFLDirectory.setText("");
        jtfOriginalBlobRDR.setText("");
        jtfNewBlobRDR.setText("");
        jtaLog.setText("Started..");
        btConvertBlobRDR.setToolTipText("");
        btConvertBlobRDR.setText("Convert Blob");
        btConvertBlobRDR.addActionListener(new ReLearningTool_btConvertBlobRDR_actionAdapter(this));
        jPanel1.setPreferredSize(new Dimension(600, 400));
        btEvaluateBlob.setText("Evaluate");
        btEvaluateBlob.addActionListener(new ReLearningTool_btEvaluateBlob_actionAdapter(this));
        btTestingBFL.setText("Testing BFL");
        btTestingBFL.addActionListener(new ReLearningTool_btTestingBFL_actionAdapter(this));
        jtfTestingBFL.setSelectionStart(11);
        btClear.setText("Clear");
        btClear.addActionListener(new ReLearningTool_btClear_actionAdapter(this));
        this.getContentPane().add(jPanel1,  BorderLayout.CENTER);
        jPanel1.add(jtfBFLDirectory,                        new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(jtfOriginalBlobRDR,                       new GridBagConstraints(0, 2, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(jtfNewBlobRDR,                       new GridBagConstraints(0, 3, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(jScrollPane1,                         new GridBagConstraints(0, 4, 2, 2, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(btOpenOriginalRDR,                    new GridBagConstraints(2, 2, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(btOpenBFLDir,                     new GridBagConstraints(2, 1, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(btOpenNewRDR,                   new GridBagConstraints(2, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 0), 0, 0));
        jPanel1.add(btConvertBlobRDR,                new GridBagConstraints(2, 4, 2, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.HORIZONTAL, new Insets(5, 5, 0, 5), 0, 0));
        jPanel1.add(cbxNewRuleType,           new GridBagConstraints(1, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(btEvaluateBlob,             new GridBagConstraints(3, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(btTestingBFL,      new GridBagConstraints(2, 0, 10, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(jtfTestingBFL,     new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        jPanel1.add(cbxRDRType,   new GridBagConstraints(1, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jPanel1.add(btClear,  new GridBagConstraints(2, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jScrollPane1.getViewport().add(jtaLog, null);

        for (int i = 0 ; i < RuleCollection.ruleMetricOptionStrings.length;i++){
            cbxNewRuleType.addItem(RuleCollection.ruleMetricOptionStrings[i]);
            cbxRDRType.addItem(RuleCollection.ruleMetricOptionStrings[i]);
        }

    }

    public static void main(String[] args) {
        ReLearningTool reLearningTool = new ReLearningTool(null);
        reLearningTool.pack();
        reLearningTool.setVisible(true);
    }
    // Overridden so we can exit when window is closed

   protected void processWindowEvent(WindowEvent e) {
       super.processWindowEvent(e);
       if (parent == null && e.getID() == WindowEvent.WINDOW_CLOSING) {
           RDRApplication.appConf.shutDown();
           System.exit(0);
       }
   }

    void btOpenBFLDir_actionPerformed(ActionEvent e) {
        optDirChooser.setCurrentDirectory(new File(RDRApplication.appConf.getProperty(MyConfiguration.propEvaluateBFL)));
        if (optDirChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optDirChooser.getSelectedFile().getAbsolutePath();
            jtfBFLDirectory.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propEvaluateBFL,fileName);
        }

    }

    void btOpenOriginalRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.getProperty(MyConfiguration.propBlobRDR)));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfOriginalBlobRDR.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propBlobRDR,fileName);
        }

    }

    void btOpenNewRDR_actionPerformed(ActionEvent e) {
        optFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propOutputRDR)));
        if (optFileChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optFileChooser.getSelectedFile().getAbsolutePath();
            jtfNewBlobRDR.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propOutputRDR,
                                               fileName);
        }

    }

    boolean isInteresedColour(int c){
        return Blobber.IS_WANTED_BLOB[c];
//        return c == CommonSense.ORANGE; //for ball
//        return c == CommonSense.YELLOW || c == CommonSense.BLUE  ; //for goal
//        return c == CommonSense.YELLOW || c == CommonSense.BLUE  || c == CommonSense.PINK; //for beacon
    }

    void doRelearnBlobRDR(){
        BFL bfl = new BFL(false);
        Blobber blobber = new Blobber(bfl);

        ManRDRBlobInterval originalRDR = new ManRDRBlobInterval (RuleCollection.ruleMetricOptions[cbxRDRType.getSelectedIndex()]);
        originalRDR.loadFromFile(jtfOriginalBlobRDR.getText());

        ManRDRBlobInterval newRDR = new ManRDRBlobInterval(RuleCollection.ruleMetricOptions[cbxNewRuleType.getSelectedIndex() ] );
        File directory = new File(jtfBFLDirectory.getText());
        String [] allBFLFiles = directory.list(new FilenameFilter() {
            public boolean accept(File dir, String filename) {
                return filename.endsWith("bfl") || filename.endsWith("BFL");
            }
        });
        int numFile = 0;
        int numBlob = 0;
        try {
            for (int i = 0; i < allBFLFiles.length; i++) {
                String fileName = new File(directory, allBFLFiles[i]).getAbsolutePath();
                bfl.readFile(fileName);
                blobber.formBlobs();
                for (Iterator it = blobber.getBlobList().iterator(); it.hasNext() ;){
                    Blob b = (Blob) it.next();
                    if (isInteresedColour(b.colour)){
                        numBlob++;
                        int btype = originalRDR.classify(b);
                        if (btype != newRDR.classify(b)){
                            newRDR.update( new ExampleBlob(b,btype, newRDR) );
                        }
                    }
                }
                numFile++;
            }
        }
        catch (FileNotFoundException ex) {
        }
        catch (IOException ex) {
        }
        finally{
            addMessage("New rule made : ");
            addMessage("   Number of file " + numFile + " out of " + allBFLFiles.length);
            addMessage("   Number of blobs " + numBlob);
            addMessage("   New rule number of cornerstones " + newRDR.getNumberCornerstones());
            newRDR.saveToFile(jtfNewBlobRDR.getText());
            newRDR.print();
        }
    }

    void btConvertBlobRDR_actionPerformed(ActionEvent e) {
        /** @todo Check file existence here */
        doRelearnBlobRDR();
    }

    public void addMessage(String message) {
        jtaLog.append(message + "\n" );
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

    private void setupEvaluateBall(String ballDir, String ballRDRFile,
                                   String colourRDRFile) {
        if (colourRDRFile != null) {
            parent.loadColourRDR(colourRDRFile);
        }

//        parent.loadBlobIntervalRDR(ballRDRFile, false);
        ManRDRBlobInterval newRDR = new ManRDRBlobInterval(RuleCollection.ruleMetricOptions[ cbxNewRuleType.getSelectedIndex() ] );
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

    public boolean evaluateBall(String ballDir, String ballRDRFile) {
        if (parent == null) {
            JOptionPane.showMessageDialog(this, "RDRApplication is not running");
            return false;
        }
        parent.getThumbnailToolbar().clearAllThumbnail();
        setupEvaluateBall(ballDir, ballRDRFile,null);
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

                totalExamples += parent.visionObserver.numberOfBlob;
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

    void btEvaluateBlob_actionPerformed(ActionEvent e) {
        evaluateBall(jtfTestingBFL.getText(), jtfNewBlobRDR.getText());
    }

    void btTestingBFL_actionPerformed(ActionEvent e) {
        optDirChooser.setCurrentDirectory(new File(RDRApplication.appConf.
            getProperty(MyConfiguration.propTestingBFL)));
        if (optDirChooser.showOpenDialog(this) != JFileChooser.CANCEL_OPTION) {
            String fileName = optDirChooser.getSelectedFile().getAbsolutePath();
            jtfTestingBFL.setText(fileName);
            RDRApplication.appConf.setProperty(MyConfiguration.propTestingBFL,
                                               fileName);
        }

    }

    void btClear_actionPerformed(ActionEvent e) {
        jtaLog.setText("");
    }
}

class ReLearningTool_btOpenBFLDir_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btOpenBFLDir_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenBFLDir_actionPerformed(e);
    }
}

class ReLearningTool_btOpenOriginalRDR_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btOpenOriginalRDR_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenOriginalRDR_actionPerformed(e);
    }
}

class ReLearningTool_btOpenNewRDR_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btOpenNewRDR_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenNewRDR_actionPerformed(e);
    }
}

class ReLearningTool_btConvertBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btConvertBlobRDR_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btConvertBlobRDR_actionPerformed(e);
    }
}

class ReLearningTool_btEvaluateBlob_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btEvaluateBlob_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btEvaluateBlob_actionPerformed(e);
    }
}

class ReLearningTool_btTestingBFL_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btTestingBFL_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btTestingBFL_actionPerformed(e);
    }
}

class ReLearningTool_btClear_actionAdapter implements java.awt.event.ActionListener {
    ReLearningTool adaptee;

    ReLearningTool_btClear_actionAdapter(ReLearningTool adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btClear_actionPerformed(e);
    }
}
