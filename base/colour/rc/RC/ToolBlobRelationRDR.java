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
import RoboShare.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
*  CHANGE LOG
* + version 1.1:
*       - added status box
 */

public class ToolBlobRelationRDR
    extends GenericTool
    implements Observer, Subject {
    public static final int PAINT_GOOD_BLOB = 1;
    public static final int PAINT_BAD_BLOB = 2;
    int iPaintOption = PAINT_GOOD_BLOB;

    GridBagLayout gridBagLayout1 = new GridBagLayout();

    Blob firstBlob = null, /* */
        secondBlob = null, /* */
        movingBlob = null; //for GUI effect

    private boolean needUpdating = false;
    ManRDRBlobRelation blobRDR = new ManRDRBlobRelation();
    JButton btOpenRDR = new JButton();
    JButton btSaveRDR = new JButton();
    JButton btSaveRDRAs = new JButton();
    JButton btUndo = new JButton();
    JButton btPrintRDR = new JButton();
    JButton btClearRDR = new JButton();
    JComboBox cbxPaintOption = new JComboBox();

    public int numberOfObject = 0;

    public ToolBlobRelationRDR(RDRApplication parent) {
        super(parent);
//        blobRDR = new BlobIntervalRDR();
        try {
            jbInit();
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        btOpenRDR.setMaximumSize(new Dimension(93, 23));
        btOpenRDR.setMinimumSize(new Dimension(110, 30));
        btOpenRDR.setPreferredSize(new Dimension(110, 30));
        btOpenRDR.setMargin(new Insets(2, 2, 2, 2));
        btOpenRDR.setText("Open Object RDR");
        btOpenRDR.addActionListener(new
                                    BlobRelationRDRTool_btOpenRDR_actionAdapter(this));
        this.setPreferredSize(new Dimension(110, 500));
        this.setLayout(gridBagLayout1);
        btSaveRDR.setMaximumSize(new Dimension(93, 23));
        btSaveRDR.setMinimumSize(new Dimension(110, 30));
        btSaveRDR.setPreferredSize(new Dimension(110, 30));
        btSaveRDR.setMargin(new Insets(2, 2, 2, 2));
        btSaveRDR.setText("Save Object RDR");
        btSaveRDR.addActionListener(new
                                    BlobRelationRDRTool_btSaveRDR_actionAdapter(this));
        btSaveRDRAs.setMaximumSize(new Dimension(93, 23));
        btSaveRDRAs.setMinimumSize(new Dimension(110, 30));
        btSaveRDRAs.setPreferredSize(new Dimension(110, 30));
        btSaveRDRAs.setMargin(new Insets(2, 2, 2, 2));
        btSaveRDRAs.setText("Save RDR As");
        btSaveRDRAs.addActionListener(new
                                    BlobRelationRDRTool_btSaveRDRAs_actionAdapter(this));

        btUndo.setMaximumSize(new Dimension(93, 23));
        btUndo.setMinimumSize(new Dimension(110, 30));
        btUndo.setPreferredSize(new Dimension(110, 30));
        btUndo.setMargin(new Insets(2, 2, 2, 2));
        btUndo.setText("Undo");
        btUndo.addActionListener(new BlobRelationRDRTool_btUndo_actionAdapter(this));
        btPrintRDR.setMaximumSize(new Dimension(93, 23));
        btPrintRDR.setMinimumSize(new Dimension(110, 30));
        btPrintRDR.setPreferredSize(new Dimension(110, 30));
        btPrintRDR.setMargin(new Insets(2, 2, 2, 2));
        btPrintRDR.setText("Print Object RDR");
        btPrintRDR.addActionListener(new
            BlobRelationRDRTool_btPrintRDR_actionAdapter(this));
        btClearRDR.setMinimumSize(new Dimension(110, 30));
        btClearRDR.setPreferredSize(new Dimension(110, 30));
        btClearRDR.setText("Clear RDR");
        btClearRDR.addActionListener(new
                                     BlobRelationRDRTool_btClearRDR_actionAdapter(this));

        cbxPaintOption.addItem("Show Good Blob");
        cbxPaintOption.addItem("Show Bad Blob");
        cbxPaintOption.setPreferredSize(new Dimension(100, 19));
        cbxPaintOption.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                cbxPaintOption_itemStateChanged(e);
            }
        });

        jtaRDRStatus.setText("");
        jtaRDRStatus.setColumns(15);
        jtaRDRStatus.setLineWrap(true);
        jtaRDRStatus.setRows(4);
        setFileName();
        this.add(btOpenRDR,  new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.NORTH, GridBagConstraints.NONE, new Insets(4, 4, 4, 4), 0, 0));
        this.add(btSaveRDR, new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(4, 4, 4, 4), 0, 0));
        this.add(btSaveRDRAs, new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            , GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(4, 4, 4, 4), 0, 0));
        this.add(btUndo, new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(4, 4, 4, 4), 0, 0));
        this.add(btPrintRDR,  new GridBagConstraints(0, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(4, 4, 4, 4), 0, 0));
        this.add(btClearRDR,  new GridBagConstraints(0, 5, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(4, 4, 4, 4), 0, 0));
        this.add(cbxPaintOption,   new GridBagConstraints(0, 6, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(4, 4, 4, 4), 0, 0));
        this.add(jScrollPane1,  new GridBagConstraints(0, 7, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        this.add(lblFileName,  new GridBagConstraints(0, 8, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        jScrollPane1.getViewport().add(jtaRDRStatus, null);

    }

    public String name() {
        return "Blob Relations";
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
        int colour = b.colour;
        ManRDRBlobInterval rdr = parent.getRDRForColour(colour);
        if (rdr != null) {
            return rdr.classify(b) == CommonSense.IS_BLOB;
        }
        else
            return false;
    }

    private void updateDisplayBlobList() {
        ArrayList blobList = getBlobList();
        getBlobber().clearDisplayBlobList();
        getBlobber().setDisplayColor(Color.MAGENTA);
//        System.out.println("BlobRelationRDR : Updating blob list");
        for (Iterator i = blobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
            if (shouldDisplayBlob(b)) {
                getBlobber().addDisplayingBlob(b);
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
                if (blobRDR.classify(b1,b2) == CommonSense.IS_BLOB){
                    numberOfObject ++;
                    Blob object = b1.merge(b2);
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

        if (blobRDR.classify(b1, b2) != C) {
            int res = blobRDR.update(be);
            jtaRDRStatus.setText("U : "+ GenericRDRRule.STATUS_TO_STRING[res]+"\n");
            needUpdating = true;
            parent.repaint();
        }
        else {
            needUpdating = false;
            jtaRDRStatus.setText("Example is consistent. \nNo rule added\n");
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
        if (blobRDR.fileName == null) {
            lblFileName.setText("File:" + blobRDR.fileName);
        }
        else
            lblFileName.setText("File:" + new File(blobRDR.fileName).getName());
    }

    public void saveRDRFile(String filename){
        try {
            blobRDR.saveToFile(filename);
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

    public void openBlobRelationRDRFromFile(String filename){
        blobRDR.loadFromFile(filename);
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

    //////////////////////// OBSERVER INTERFACE //////////////////////////////
    public void update(Subject o) {
        needUpdating = true;
    }

    ///////////////////////////// SUBJECT INTERFACE ///////////////////////////
    private ArrayList observers = new ArrayList();
    JScrollPane jScrollPane1 = new JScrollPane();
    JTextArea jtaRDRStatus = new JTextArea();
    JLabel lblFileName = new JLabel();

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

    //get the blob
    public void mousePressed(MouseEvent e) {
        if (firstBlob == null) {
            int x = parent.bflPanel.getPosX(e.getX());
            int y = parent.bflPanel.getPosY(e.getY());
            System.out.println("Blob at " + x + " " + y);
            firstBlob = getBlobAtCursor(x, y);
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
            if (secondBlob != null && secondBlob != firstBlob) {
                showBlobRelationInfo(firstBlob, secondBlob);
                if (e.isShiftDown()) {
                    classifyBlob(firstBlob, secondBlob, CommonSense.IS_NOT_BLOB);
                }
                else {
                    classifyBlob(firstBlob, secondBlob, CommonSense.IS_BLOB);
                }
            }
        }
        firstBlob = null;
        secondBlob = null;
        getBlobber().setSpecialBlob(null, null);
        parent.repaint();
    }

    void btOpenRDR_actionPerformed(ActionEvent e) {
        openBlobRelationRDR();
    }

    void btSaveRDR_actionPerformed(ActionEvent e) {
        if (blobRDR.fileName == null)
            saveBlobRelationRDRAs();
        else
            saveRDRFile(blobRDR.fileName);
    }

    void btSaveRDRAs_actionPerformed(ActionEvent e) {
        saveBlobRelationRDRAs();
    }


    void btUndo_actionPerformed(ActionEvent e) {
        if (blobRDR.undo()) {
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
        blobRDR.print();
    }

    void btClearRDR_actionPerformed(ActionEvent e) {
        blobRDR = new ManRDRBlobRelation();
        setFileName();
        needUpdating = true;
        parent.repaint();
    }

    void cbxPaintOption_itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED){
            System.out.println("ItemEvent " + e.getItem() + " " +
                               e.getStateChange() + " selected index " + cbxPaintOption.getSelectedIndex());
            switch (cbxPaintOption.getSelectedIndex()) {
                case 0: //GOOD BLOB
                    setPaintOption(PAINT_GOOD_BLOB);
                    break;
                case 1:
                    setPaintOption(PAINT_BAD_BLOB);
                    break;
            }
            parent.repaint();
        }
    }


}

class BlobRelationRDRTool_btOpenRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolBlobRelationRDR adaptee;

    BlobRelationRDRTool_btOpenRDR_actionAdapter(ToolBlobRelationRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btOpenRDR_actionPerformed(e);
    }
}

class BlobRelationRDRTool_btSaveRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolBlobRelationRDR adaptee;

    BlobRelationRDRTool_btSaveRDR_actionAdapter(ToolBlobRelationRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btSaveRDR_actionPerformed(e);
    }
}

class BlobRelationRDRTool_btSaveRDRAs_actionAdapter
    implements java.awt.event.ActionListener {
    ToolBlobRelationRDR adaptee;

    BlobRelationRDRTool_btSaveRDRAs_actionAdapter(ToolBlobRelationRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btSaveRDRAs_actionPerformed(e);
    }
}


class BlobRelationRDRTool_btUndo_actionAdapter
    implements java.awt.event.ActionListener {
    ToolBlobRelationRDR adaptee;

    BlobRelationRDRTool_btUndo_actionAdapter(ToolBlobRelationRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btUndo_actionPerformed(e);
    }
}

class BlobRelationRDRTool_btPrintRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolBlobRelationRDR adaptee;

    BlobRelationRDRTool_btPrintRDR_actionAdapter(ToolBlobRelationRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btPrintRDR_actionPerformed(e);
    }
}

class BlobRelationRDRTool_btClearRDR_actionAdapter
    implements java.awt.event.ActionListener {
    ToolBlobRelationRDR adaptee;

    BlobRelationRDRTool_btClearRDR_actionAdapter(ToolBlobRelationRDR adaptee) {
        this.adaptee = adaptee;
    }

    public void actionPerformed(ActionEvent e) {
        adaptee.btClearRDR_actionPerformed(e);
    }
}
