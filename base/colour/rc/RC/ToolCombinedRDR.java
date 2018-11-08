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


package RC;

import java.util.*;
import java.io.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import RoboShare.*;

/**
 * <p>Title: Combined Blob RDR & Object RDR into one tool </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
*  CHANGE LOG
* + version 1.6:
*       - add option for default blob classification.
*       - add button for adding prior rule
 */


public class ToolCombinedRDR
    extends GenericTool
    implements Observer, Subject {
    public static final int PAINT_GOOD_BLOB = 1;
    public static final int PAINT_BAD_BLOB = 2;
    int iPaintOption = PAINT_GOOD_BLOB;

    private static final boolean forcedUpdate = true; //sothat visionObserver has update info

    GridBagLayout gridBagLayout1 = new GridBagLayout();

    Blob firstBlob = null, /* */
        secondBlob = null, /* */
        movingBlob = null; //for GUI effect

    private boolean needUpdating = false;
    static MetricBlob [] blobRuleMetrics = RuleCollection.H_AA_SD_Rule;
    ManRDRBlobRelation blobRelationRDR = new ManRDRBlobRelation();
    ManRDRBlobInterval blobRDR = new ManRDRBlobInterval(blobRuleMetrics);

    JButton btOpenRDR = new JButton();
    JButton btSaveRDRAs = new JButton();
    JButton btUndo = new JButton();
    JButton btPrintRDR = new JButton();
    JButton btClearRDR = new JButton();

//    JTextArea jtaRDRStatus = new JTextArea();
    JLabel lblFileName = new JLabel();
    JPanel jPanel1 = new JPanel();
    GridBagLayout gridBagLayout2 = new GridBagLayout();
    Border border1;
    TitledBorder titledBorder1;
    JPanel jPanel2 = new JPanel();

    GridBagLayout gridBagLayout3 = new GridBagLayout();
    JLabel jLabel1 = new JLabel();
    JButton btClearBlobRDR = new JButton();
    JButton btPrintBlobRDR = new JButton();
    JButton btSaveBlobRDR = new JButton();
    JButton btUndoBlobRDR = new JButton();
    JButton btOpenBlobRDR = new JButton();
    Border border2;
    TitledBorder titledBorder2;

    public int numberOfObject = 0;
    public int numberOfBlob = 0;
    public ToolCombinedRDR(RDRApplication parent) {
        super(parent);
//        blobRDR = new BlobIntervalRDR();
        try {
            jbInit();
            setBlobColourSelection(Blobber.IS_WANTED_BLOB);
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        border1 = BorderFactory.createEmptyBorder();
        titledBorder1 = new TitledBorder(new EtchedBorder(EtchedBorder.RAISED,Color.white,new Color(165, 163, 151)),"Relation");
        border2 = new EtchedBorder(EtchedBorder.RAISED,Color.white,new Color(165, 163, 151));
        titledBorder2 = new TitledBorder(border2,"Blob");
        btOpenRDR.setMaximumSize(new Dimension(93, 23));
        btOpenRDR.setMinimumSize(new Dimension(70, 23));
        btOpenRDR.setPreferredSize(new Dimension(71, 23));
        btOpenRDR.setMargin(new Insets(2, 2, 2, 2));
        btOpenRDR.setText("Open");
        btOpenRDR.addActionListener(new
                                    ToolCombinedRDR_btOpenRDR_actionAdapter(this));
        this.setPreferredSize(new Dimension(110, 500));
        this.setLayout(gridBagLayout1);
        btSaveRDRAs.setMaximumSize(new Dimension(93, 23));
        btSaveRDRAs.setMinimumSize(new Dimension(70, 23));
        btSaveRDRAs.setPreferredSize(new Dimension(71, 23));
        btSaveRDRAs.setMargin(new Insets(2, 2, 2, 2));
        btSaveRDRAs.setText("Save As");
        btSaveRDRAs.addActionListener(new
                                    ToolCombinedRDR_btSaveRDRAs_actionAdapter(this));

        btUndo.setMaximumSize(new Dimension(93, 23));
        btUndo.setMinimumSize(new Dimension(70, 23));
        btUndo.setPreferredSize(new Dimension(71, 23));
        btUndo.setMargin(new Insets(2, 2, 2, 2));
        btUndo.setText("Undo");
        btUndo.addActionListener(new ToolCombinedRDR_btUndo_actionAdapter(this));
        btPrintRDR.setMaximumSize(new Dimension(93, 23));
        btPrintRDR.setMinimumSize(new Dimension(70, 23));
        btPrintRDR.setPreferredSize(new Dimension(71, 23));
        btPrintRDR.setMargin(new Insets(2, 2, 2, 2));
        btPrintRDR.setText("Print");
        btPrintRDR.addActionListener(new
            ToolCombinedRDR_btPrintRDR_actionAdapter(this));
        btClearRDR.setMinimumSize(new Dimension(70, 23));
        btClearRDR.setPreferredSize(new Dimension(71, 23));
        btClearRDR.setText("Clear");
        btClearRDR.addActionListener(new
                                     ToolCombinedRDR_btClearRDR_actionAdapter(this));



        setFileName();
        jPanel1.setLayout(gridBagLayout2);
        jPanel1.setBorder(titledBorder1);
        jPanel1.setPreferredSize(new Dimension(100, 180));
        btPrintBlobRDR.setPreferredSize(new Dimension(71, 23));
        btPrintBlobRDR.setText("Print");
        btPrintBlobRDR.addActionListener(new ToolCombinedRDR_btPrintBlobRDR_actionAdapter(this));
        jPanel2.setLayout(gridBagLayout3);
        jLabel1.setToolTipText("");
        jLabel1.setText("File:");
        btClearBlobRDR.setPreferredSize(new Dimension(71, 23));
        btClearBlobRDR.setText("Clear");
        btClearBlobRDR.addActionListener(new ToolCombinedRDR_btClearBlobRDR_actionAdapter(this));
        btSaveBlobRDR.setPreferredSize(new Dimension(71, 23));
        btSaveBlobRDR.setText("Save As");
        btSaveBlobRDR.addActionListener(new ToolCombinedRDR_btSaveBlobRDR_actionAdapter(this));
        btUndoBlobRDR.setPreferredSize(new Dimension(71, 23));
        btUndoBlobRDR.setText("Undo");
        btUndoBlobRDR.addActionListener(new ToolCombinedRDR_btUndoBlobRDR_actionAdapter(this));
        btOpenBlobRDR.setMaximumSize(new Dimension(71, 23));
        btOpenBlobRDR.setPreferredSize(new Dimension(71, 23));
        btOpenBlobRDR.setText("Open");
        btOpenBlobRDR.addActionListener(new ToolCombinedRDR_btOpenBlobRDR_actionAdapter(this));
        jPanel2.setBorder(titledBorder2);
        jPanel2.setMinimumSize(new Dimension(90, 157));
        jPanel2.setPreferredSize(new Dimension(100, 157));
        lstColours.setPreferredSize(new Dimension(0, 0));
        lstColours.addMouseListener(new ToolCombinedRDR_lstColours_mouseAdapter(this));
        chbDefaultRule.setText("Default rule");
        chbDefaultRule.addActionListener(new ToolCombinedRDR_chbDefaultRule_actionAdapter(this));
        chbIsGoal.setToolTipText("");
        chbIsGoal.setText("Is Goal");
        chbIsGoal.addActionListener(new ToolCombinedRDR_chbIsGoal_actionAdapter(this));
        btManualRule.setSelected(false);
        btManualRule.setText("Manual Rule");
        btManualRule.addActionListener(new ToolCombinedRDR_btManualRule_actionAdapter(this));
        jPanel1.add(btPrintRDR,                             new GridBagConstraints(0, 5, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 4, 0, 4), 0, 0));
        jPanel1.add(lblFileName,                       new GridBagConstraints(0, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(2, 2, 0, 0), 0, 0));
        jPanel1.add(btOpenRDR,                new GridBagConstraints(0, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 4, 0, 4), 0, 0));
        jPanel1.add(btSaveRDRAs,        new GridBagConstraints(0, 3, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 4, 0, 4), 0, 0));
        jPanel1.add(btClearRDR,      new GridBagConstraints(0, 6, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 4, 0, 4), 0, 0));
        jPanel1.add(btUndo,    new GridBagConstraints(0, 4, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 4, 0, 4), 0, 0));
        this.add(chbIsGoal,    new GridBagConstraints(0, 10, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        this.add(chbDefaultRule,      new GridBagConstraints(0, 9, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        this.add(jScrollPane1,        new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jScrollPane1.getViewport().add(lstColours, null);
        this.add(jPanel2,               new GridBagConstraints(0, 0, 1, 1, 1.0, 0.5
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btOpenBlobRDR,            new GridBagConstraints(0, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btSaveBlobRDR,           new GridBagConstraints(0, 2, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btUndoBlobRDR,            new GridBagConstraints(0, 3, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btPrintBlobRDR,               new GridBagConstraints(0, 4, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(jLabel1,          new GridBagConstraints(0, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btManualRule,     new GridBagConstraints(0, 6, 2, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
        jPanel2.add(btClearBlobRDR, new GridBagConstraints(0, 5, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(jPanel1,                 new GridBagConstraints(0, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));

    }

    public String name() {
        return "Recognition";
    }

    public Blob getBlobAtCursor(int x, int y) {
        return parent.blobber.getBlobAtCursor(x, y);
    }

    protected void showBlobRelationInfo(Blob firstBlob, Blob secondBlob) {
        System.out.println(" -------- Blob Relation info ");

        MetricBlobRelation m1 = new CentroidDistanceMetric( -1); //BlobRelationRDR.getMetric(BlobRelationRDR.metCENTROID_DISTANCE);
        MetricBlobRelation m2 = new RadiusRatioMetric( -1);
        MetricBlobRelation m3 = new AspectRatioBeaconMetric( -1);
        MetricBlobRelation m4 = new GradientMetric( -1);
        MetricBlobRelation m5 = new RadiusCoveringMetric( -1);

        System.out.println("Centroid Distance : " +
                           m1.toString(m1.getMetric(firstBlob, secondBlob)) + "\n" +
                           "Radius Ratio : " +
                           m2.toString(m2.getMetric(firstBlob, secondBlob)) + "\n" +
                           "Object Aspect Ratio : " +
                           m3.toString(m3.getMetric(firstBlob, secondBlob)) + "\n" +
                           "Gradient : " +
                           m4.toString(m4.getMetric(firstBlob, secondBlob)) + "\n" +
                           "Radius Covering : " +
                           m5.toString(m5.getMetric(firstBlob, secondBlob)));
    }

    protected void clearBlobInformationInfo() {
        getBlobber().setSpecialBlob(null, null);
    }

    private Blobber getBlobber() {
        return parent.blobber;
    }

    boolean shouldDisplayBlob(Blob b) {
        return blobRDR.classify(b) == CommonSense.IS_BLOB;
    }

    private void updateDisplayBlobList() {
        ArrayList blobList = getBlobList();
        getBlobber().clearDisplayBlobList();
        getBlobber().setDisplayColor(Color.MAGENTA);
//        System.out.println("CombinedRDR : Updating blob list");
        numberOfBlob = 0;
        for (Iterator i = blobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
            if (shouldDisplayBlob(b)) {
                getBlobber().addDisplayingBlob(b);
                numberOfBlob ++;
//                System.out.println("Displaying blob " + b.toString());
            }
        }
        needUpdating = false;
    }

    void paintClassifiedObjects(){
        numberOfObject = 0;
        ArrayList displayBlobList = getBlobber().displayBlobList;
        for (int i = 0 ; i < displayBlobList.size(); i++){
            for (int j = i+1 ; j < displayBlobList.size(); j++){
                Blob b1 = (Blob) displayBlobList.get(i);
                Blob b2 = (Blob) displayBlobList.get(j);
                if (b1.colour == b2.colour) continue;
//                showBlobRelationInfo(b1,b2);
                if (blobRelationRDR.classify(b1,b2) == CommonSense.IS_BLOB){
                    numberOfObject ++;
                    Blob object = b1.merge(b2);
                    parent.bflPanel.drawLine(b1.getUnRotatedCentroid().getX(), b1.getUnRotatedCentroid().getY(),
                                             b2.getUnRotatedCentroid().getX(), b2.getUnRotatedCentroid().getY(), Color.CYAN);
                    parent.bflPanel.drawRotatedBoundingBox(object,Color.CYAN);
                }
            }
        }
        notifyObservers();
    }

    public void paintBFLPanel(Graphics g) {
        if (!enabled)return;

        if (needUpdating)
            updateDisplayBlobList();

        paintClassifiedObjects();
    }

    public void setToolEnabled(boolean b) {
        enabled = b;
        needUpdating = b;
    }

    private void classifyBlob(Blob b1, Blob b2, int C) {
        ExampleBlobRelation be = new ExampleBlobRelation(b1, b2, C);
        Pair[] metrics = ManRDRBlobRelation.getAllMetrics(b1, b2);
        System.out.println("Classifying Blob 1 : " + b1.toString() +
                         "\nClassifying Blob 2 : " + b2.toString());
        for (int i = 0; i < metrics.length; i++)
            System.out.println("  " + metrics[i]);

        System.out.println("Class = " + CommonSense.getBlobClassification(C));

        if (blobRelationRDR.classify(b1, b2) != C) {
            int res = blobRelationRDR.update(be);
            System.out.println("U : "+ GenericRDRRule.STATUS_TO_STRING[res]+"\n");
            needUpdating = true;
            parent.repaint();
        }
        else {
            needUpdating = false;
            System.out.println("Example is consistent. No rule added");
        }
    }

    public void setPaintOption(int opt) {
        if (iPaintOption != opt) {
            iPaintOption = opt;
            needUpdating = true;
        }
    }

    void setFileName() {
        if (blobRelationRDR.fileName == null) {
            lblFileName.setText("File:" + blobRelationRDR.fileName);
        }
        else
            lblFileName.setText("File:" + new File(blobRelationRDR.fileName).getName());

        if (blobRDR.fileName == null) {
            jLabel1.setText("File:" + blobRDR.fileName);
        }
        else
            jLabel1.setText("File:" + new File(blobRDR.fileName).getName());

    }

    public void saveRDRFile(String filename){
        try {
            blobRelationRDR.saveToFile(filename);
            setFileName();
        }
        catch (Exception ex) {
            JOptionPane.showMessageDialog(this,"Error saving Relation RDR file to " + filename, "Error" , JOptionPane.ERROR_MESSAGE);
        }

    }

    public void saveBlobRelationRDRAs() {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(
            RDRApplication.appConf.getLastRDRFileChooser()));
        int returnVal = RDRApplication.RDRFileChooser.showSaveDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().
            getPath();

        RDRApplication.appConf.setLastRDRFileChooser(RDRApplication.
            RDRFileChooser.getSelectedFile().getParent());


        saveRDRFile(filename);
    }

    public void saveBlobRDR(String filename) {
        blobRDR.saveToFile(filename);
        setFileName();
    }

    public void saveBlobIntervalRDRAs() {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(
            RDRApplication.appConf.getLastRDRFileChooser()));
        int returnVal = RDRApplication.RDRFileChooser.showSaveDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().
            getPath();
        RDRApplication.appConf.setLastRDRFileChooser(RDRApplication.
            RDRFileChooser.getSelectedFile().getParent());
        saveBlobRDR(filename);

    }

    public void openBlobRelationRDRFromFile(String filename){
        resetObjectRDR();
        blobRelationRDR.loadFromFile(filename);
        setFileName();
        needUpdating = true;
        parent.repaint();
    }

    public void openBlobRelationRDR() {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(
            RDRApplication.appConf.getLastBlobRelationRDR()));
        int returnVal = RDRApplication.RDRFileChooser.showOpenDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().
            getPath();
        RDRApplication.appConf.setLastBlobRelationRDR(filename);
        RDRApplication.appConf.setLastObjectRDRFile(filename);
        openBlobRelationRDRFromFile(filename);
    }

    public void openBlobIntervalRDR() {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.getLastRDRFileChooser()));
        int returnVal = RDRApplication.RDRFileChooser.showOpenDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().
            getPath();
        RDRApplication.appConf.setLastRDRFileChooser(filename);
        RDRApplication.appConf.setLastBlobIntervalRDR(filename);
        loadBlobIntervalRDRFromFile(filename);
    }

    public void loadBlobIntervalRDRFromFile(String filename) {
        blobRDR.loadFromFile(filename);
        setFileName();
        needUpdating = true;
        parent.repaint();
    }


    //////////////////////// OBSERVER INTERFACE //////////////////////////////
    public void update(Subject o) {
        needUpdating = true;
        if (forcedUpdate) {
            updateDisplayBlobList();
            paintClassifiedObjects();
        }
    }

    ///////////////////////////// SUBJECT INTERFACE ///////////////////////////
    private ArrayList observers = new ArrayList();
    JScrollPane jScrollPane1 = new JScrollPane();
    JList lstColours = new JList();
    JCheckBox chbDefaultRule = new JCheckBox();
    JCheckBox chbIsGoal = new JCheckBox();
    JButton btManualRule = new JButton();


    public void addObserver(Observer o) {
        observers.add(o);
    }

    public void removeObserver(Observer o) {
        observers.remove(o);
    }

    protected void notifyObservers() {
        // loop through and notify each observer
        Iterator i = observers.iterator();
        while (i.hasNext()) {
            Observer o = (Observer) i.next();
            o.update(this);
        }
    }

    /////////////////// EVENT HANDLING CODE/////////////////////////

    public void mouseDragged(MouseEvent e) {
        if (firstBlob != null) {
            int x = parent.bflPanel.getPosX(e.getX());
            int y = parent.bflPanel.getPosY(e.getY());
//            System.out.println("Blob at " + x + " " + y);
            movingBlob = getBlobAtCursor(x, y);
        }
        getBlobber().setSpecialBlob(firstBlob, movingBlob);
        parent.repaint();
    }

    private void classifyBlob(Blob b, int C) {
        ExampleBlob be = new ExampleBlob(b, C, blobRDR);
        Pair[] metrics = blobRDR.getAllMetrics(be.example);
        System.out.println("Classifying Blob : " + b.toString());
        for (int i = 0; i < metrics.length; i++)
            System.out.println("  " + metrics[i]);

        if (blobRDR.classify(b) != C) {
            int res = blobRDR.update(be);
            System.out.println("U : " + GenericRDRRule.STATUS_TO_STRING[res] +
                                 "\n");
            needUpdating = true;
        }
        else {
            needUpdating = false;
            System.out.println("Example is consistent. No rule added\n");
        }
    }

    //get the blob
    public void mousePressed(MouseEvent e) {
        int x = parent.bflPanel.getPosX(e.getX());
        int y = parent.bflPanel.getPosY(e.getY());
        if (SwingUtilities.isMiddleMouseButton(e)) {
            Blob[] allOverBlob = getBlobber().getAllBlobsAtCursor(x, y);
            if (allOverBlob.length > 1) {
                Blob b = allOverBlob[allOverBlob.length - 1];
                ArrayList blobList = getBlobber().getDisplayingBlobList();
                blobList.remove(b);
                blobList.add(0, b);
                System.out.println("Blob is brought up");
            }
            else {
                System.err.println(
                    "There are less than 2 blobs at cursor (" +
                    allOverBlob.length + ")");
            }
        }
        else{
            if (firstBlob == null) {
                System.out.println("Blob at " + x + " " + y);
                firstBlob = getBlobAtCursor(x, y);
            }

        }
    }

    //show the connecting blobs
    public void mouseReleased(MouseEvent e) {
        clearBlobInformationInfo();
        if (firstBlob != null) {
            int x = parent.bflPanel.getPosX(e.getX());
            int y = parent.bflPanel.getPosY(e.getY());
            System.out.println("Blob at " + x + " " + y);
            secondBlob = getBlobAtCursor(x, y);
            if (secondBlob != null) {
                if (secondBlob != firstBlob) { //blob relation (pair)
                    showBlobRelationInfo(firstBlob, secondBlob);
                    if (SwingUtilities.isRightMouseButton(e)) {
                        classifyBlob(firstBlob, secondBlob,
                                     CommonSense.IS_NOT_BLOB);
                    }
                    else {
                        if (firstBlob.colour != secondBlob.colour){
                            if (blobRDR.classify(firstBlob) ==
                                CommonSense.IS_NOT_BLOB) {
                                classifyBlob(firstBlob, CommonSense.IS_BLOB);
                            }
                            if (blobRDR.classify(secondBlob) ==
                                CommonSense.IS_NOT_BLOB) {
                                classifyBlob(secondBlob, CommonSense.IS_BLOB);
                            }
                        }
                        classifyBlob(firstBlob, secondBlob, CommonSense.IS_BLOB);
                    }
                }
                else { //Blob classification
                    if (SwingUtilities.isRightMouseButton(e)) {
                        classifyBlob(secondBlob, CommonSense.IS_NOT_BLOB);
                    }
                    else {
                        classifyBlob(secondBlob, CommonSense.IS_BLOB);
                    }
                    parent.repaint();
                }
            }
            firstBlob = null;
            secondBlob = null;
            getBlobber().setSpecialBlob(null, null);
            parent.repaint();
        }
    }

    public void mouseMoved(MouseEvent e) {
        if (e.isShiftDown()) {
            int x = parent.bflPanel.getPosX(e.getX());
            int y = parent.bflPanel.getPosY(e.getY());
//            System.out.println("Blob at " + x + " " + y);
            Blob b = getBlobAtCursor(x, y);
            getBlobber().setSpecialBlob(b, null);
            parent.repaint();
        }
    }

    void btOpenRDR_actionPerformed(ActionEvent e) {
        openBlobRelationRDR();
    }

    void btSaveRDR_actionPerformed(ActionEvent e) {
        if (blobRelationRDR.fileName == null)
            saveBlobRelationRDRAs();
        else
            saveRDRFile(blobRelationRDR.fileName);
    }

    void btSaveRDRAs_actionPerformed(ActionEvent e) {
        saveBlobRelationRDRAs();
    }


    void btUndo_actionPerformed(ActionEvent e) {
        if (blobRelationRDR.undo()) {
            needUpdating = true;
            parent.repaint();
        }
        else {
            JOptionPane.showMessageDialog(this,
                                          "No action in the history. Cannot Undo",
                                          "Undo error",
                                          JOptionPane.ERROR_MESSAGE);
        }
    }

    void btPrintRDR_actionPerformed(ActionEvent e) {
        blobRelationRDR.print();
    }

    void btClearRDR_actionPerformed(ActionEvent e) {
        resetObjectRDR();
        setFileName();
        needUpdating = true;
        parent.repaint();
    }

    public void resetObjectRDR() {
        blobRelationRDR = new ManRDRBlobRelation();

    }

    void btOpenBlobRDR_actionPerformed(ActionEvent e) {
        openBlobIntervalRDR();
    }

    void btSaveBlobRDR_actionPerformed(ActionEvent e) {
        saveBlobIntervalRDRAs();
    }

    void btUndoBlobRDR_actionPerformed(ActionEvent e) {
        if (blobRDR.undo()) {
            needUpdating = true;
            parent.repaint();
        }
        else {
            JOptionPane.showMessageDialog(this,
                "No action in the history. Cannot Undo", "Undo error",
                                          JOptionPane.ERROR_MESSAGE);
        }
    }

    void btPrintBlobRDR_actionPerformed(ActionEvent e) {
        blobRDR.print();
    }

    void btClearBlobRDR_actionPerformed(ActionEvent e) {
        resetBlobRDR();
        parent.repaint();

    }

    public void resetBlobRDR() {
        blobRDR = new ManRDRBlobInterval(blobRDR.getRuleMetrics());
        setFileName();
        needUpdating = true;

    }

    public void resetBlobRDR(ManRDRBlobInterval copy) {
        blobRDR = copy;
        setFileName();
        needUpdating = true;
    }


    void lstColours_mouseClicked(MouseEvent e) {
        ColourSelectionDialog dialog = new ColourSelectionDialog(Blobber.IS_WANTED_BLOB, null,"Show only colours",true);
        dialog.setVisible(true);
        setBlobColourSelection(dialog.colourSelection);
        parent.blobber.setWantedColour(dialog.colourSelection);
        parent.repaint();
    }

    private void setBlobColourSelection(boolean [] selection) {
        Vector listStr = new Vector();
        for (int i = 0 ; i < selection.length  ; i++ ) {
            if (selection[i])
                listStr.add(CommonSense.COLOUR_NAME[i]);
        }
        lstColours.setListData(listStr);
    }

    void chbDefaultRule_actionPerformed(ActionEvent e) {
        if (chbDefaultRule.isSelected())
            RuleRDRBlobInterval.DEFAULT_CLASSIFICATION = CommonSense.IS_BLOB;
        else
            RuleRDRBlobInterval.DEFAULT_CLASSIFICATION = CommonSense.IS_NOT_BLOB;
        System.out.println("Default blob interval rule : " + CommonSense.getBlobClassification(RuleRDRBlobInterval.DEFAULT_CLASSIFICATION ));
    }

    void chbIsGoal_actionPerformed(ActionEvent e) {
        if (chbIsGoal.isSelected()  ){
            if (blobRDR instanceof ManRDRBlobInterval)
                blobRDR = new ManRDRGoal();
        }
        else if (blobRDR instanceof ManRDRGoal)
            blobRDR = new ManRDRBlobInterval(blobRuleMetrics);

        parent.repaint();
    }

    void btManualRule_actionPerformed(ActionEvent e) {
        String input = JOptionPane.showInputDialog(this,"Enter rules","Manual Rule", JOptionPane.YES_NO_CANCEL_OPTION);
        System.out.println("Got " + input);
        if (input != null && input.length() > 3 ){
            GenericIntervalExample ex = new GenericIntervalExample(input.substring(0,input.length() - 3));
            if (input.endsWith("yes")){
                ex.C = CommonSense.IS_BLOB;
            }
            else if (input.endsWith("no")){
                ex.C = CommonSense.IS_NOT_BLOB;
            }
            else return;

            blobRDR.rdrRule.update(ex);
        }

    }


}

class ToolCombinedRDR_btOpenRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btOpenRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btSaveRDRAs_actionAdapter
    implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btSaveRDRAs_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btSaveRDRAs_actionPerformed(e);
    }
}


class ToolCombinedRDR_btUndo_actionAdapter
    implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btUndo_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btUndo_actionPerformed(e);
    }
}

class ToolCombinedRDR_btPrintRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btPrintRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btPrintRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btClearRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btClearRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btClearRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btOpenBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btOpenBlobRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenBlobRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btSaveBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btSaveBlobRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btSaveBlobRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btUndoBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btUndoBlobRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btUndoBlobRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btPrintBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btPrintBlobRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btPrintBlobRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_btClearBlobRDR_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btClearBlobRDR_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btClearBlobRDR_actionPerformed(e);
    }
}

class ToolCombinedRDR_lstColours_mouseAdapter extends java.awt.event.MouseAdapter {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_lstColours_mouseAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void mouseClicked(MouseEvent e) {
        adaptee.lstColours_mouseClicked(e);
    }
}

class ToolCombinedRDR_chbDefaultRule_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_chbDefaultRule_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.chbDefaultRule_actionPerformed(e);
    }
}

class ToolCombinedRDR_chbIsGoal_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_chbIsGoal_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.chbIsGoal_actionPerformed(e);
    }
}

class ToolCombinedRDR_btManualRule_actionAdapter implements java.awt.event.ActionListener {
    ToolCombinedRDR adaptee;

    ToolCombinedRDR_btManualRule_actionAdapter(ToolCombinedRDR adaptee) {
        this.adaptee = adaptee;
    }
    public void actionPerformed(ActionEvent e) {
        adaptee.btManualRule_actionPerformed(e);
    }
}
