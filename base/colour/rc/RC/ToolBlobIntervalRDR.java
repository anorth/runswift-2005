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

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.2
 * CHANGE LOG :
 * + version 1.1:
 *      - add Open RDR , Undo button/functionality.
 * + version 1.2:
 *      - only update RDR if it is contradict to the rule
 *      - Added RDR Collection (RDR pool to be used in Relation RDR).
*       - Added status box
*  + version 1.3 :
*       - Blob RDR Save As button
 */

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import javax.swing.event.*;
import java.io.*;
import RoboShare.*;

public class ToolBlobIntervalRDR
    extends GenericTool
    implements Observer {
    private static final boolean debugPainting = false;
    private static final boolean forcedUpdate = true;
    public static final int PAINT_GOOD_BLOB = 1;
    public static final int PAINT_BAD_BLOB = 2;
    int iPaintOption = PAINT_GOOD_BLOB;

    FlowLayout flowLayout1 = new FlowLayout();

    JButton showBlobButton = new JButton();
    JButton blobSaveRDR = new JButton();
    JButton blobSaveAsRDR = new JButton();
    JButton blobOpenBFL = new JButton();
    JButton blobOpenRDR = new JButton();
    JButton btPrintBlobRDR = new JButton();
    JButton btClearBlobRDR = new JButton();

    ManRDRBlobInterval blobRDR;

    JCheckBox chbEnabled = new JCheckBox();
    private boolean needUpdating = false;
    JButton btUndo = new JButton();
    JComboBox cbxPaintOption = new JComboBox();
    JButton btAddToCollection = new JButton();
    JScrollPane jScrollPane1 = new JScrollPane();
    JTextArea jtaRDRStatus = new JTextArea();
    JLabel lblFileName = new JLabel();

    public int numberOfBlob = 0;
    public ToolBlobIntervalRDR(RDRApplication parent) {
        super(parent);
        blobRDR = new ManRDRBlobInterval(RuleCollection.H_AA_C_Rule);
        enabled = true;

        try {
            jbInit();
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private void jbInit() throws Exception {
        setLayout(flowLayout1);
        setMaximumSize(new Dimension(100, 30));
        setMinimumSize(new Dimension(100, 30));
        setPreferredSize(new Dimension(100, 30));
        setInputVerifier(null);

        blobOpenBFL.setPreferredSize(new Dimension(110, 30));
        blobOpenBFL.setToolTipText("");
        blobOpenBFL.setText("Open BFL");
        blobOpenBFL.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                blobOpenBFL_actionPerformed(e);
            }
        });
        blobOpenRDR.setPreferredSize(new Dimension(110, 30));
        blobOpenRDR.setToolTipText("");
        blobOpenRDR.setText("Open Blob RDR");
        blobOpenRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                blobOpenRDR_actionPerformed(e);
            }
        });

        blobSaveRDR.setPreferredSize(new Dimension(110, 30));
        blobSaveRDR.setToolTipText("");
        blobSaveRDR.setText("Save Blob RDR");
        blobSaveRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                blobSaveRDR_actionPerformed(e);
            }
        });
        blobSaveAsRDR.setPreferredSize(new Dimension(110, 30));
        blobSaveAsRDR.setToolTipText("");
        blobSaveAsRDR.setText("Save RDR As");
        blobSaveAsRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                blobSaveAsRDR_actionPerformed(e);
            }
        });


        showBlobButton.setMaximumSize(new Dimension(100, 30));
        showBlobButton.setMinimumSize(new Dimension(70, 30));
        showBlobButton.setText("Show Blobs");
        showBlobButton.setPreferredSize(new Dimension(110, 30));
        showBlobButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                showBlobButton_actionPerformed(e);
            }
        });

        btPrintBlobRDR.setPreferredSize(new Dimension(110, 30));
        btPrintBlobRDR.setText("Print Blob RDR");
        btPrintBlobRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                btPrintBlobRDR_actionPerformed(e);
            }
        });

        btClearBlobRDR.setMinimumSize(new Dimension(105, 23));
        btClearBlobRDR.setPreferredSize(new Dimension(110, 30));
        btClearBlobRDR.setText("Clear Blob RDR");
        btClearBlobRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                btClearBlobRDR_actionPerformed(e);
            }
        });

        chbEnabled.setToolTipText("");
        chbEnabled.setText("Enabled?");
        chbEnabled.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                chbEnabled_stateChanged(e);
            }
        });
        chbEnabled.setSelected(enabled);
        btUndo.setPreferredSize(new Dimension(110, 30));
        btUndo.setToolTipText("");
        btUndo.setText("Undo");
        btUndo.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                btUndo_actionPerformed(e);
            }
        });
        cbxPaintOption.addItem("Show Good Blob");
        cbxPaintOption.addItem("Show Bad Blob");
        cbxPaintOption.setPreferredSize(new Dimension(100, 19));
        cbxPaintOption.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                cbxPaintOption_itemStateChanged(e);
            }
        });
        btAddToCollection.setMaximumSize(new Dimension(105, 23));
        btAddToCollection.setMinimumSize(new Dimension(105, 23));
        btAddToCollection.setPreferredSize(new Dimension(105, 30));
        btAddToCollection.setText("Add To Collection");
        btAddToCollection.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                btAddToCollection_actionPerformed(e);
            }
        });
        jtaRDRStatus.setMinimumSize(new Dimension(50, 50));
        jtaRDRStatus.setPreferredSize(new Dimension(100, 60));
        jtaRDRStatus.setText("");
        jtaRDRStatus.setColumns(15);
        jtaRDRStatus.setLineWrap(true);
        jScrollPane1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
        jScrollPane1.setPreferredSize(new Dimension(107, 62));
        setFileName();
        add(blobOpenBFL, null);
        add(blobOpenRDR, null);
        add(blobSaveRDR, null);
        add(blobSaveAsRDR, null);
        add(showBlobButton, null);
        add(btUndo, null);
        add(btPrintBlobRDR, null);
        add(btClearBlobRDR, null);
        add(chbEnabled, null);
        this.add(cbxPaintOption, null);
        this.add(btAddToCollection, null);
        this.add(jScrollPane1, null);
        this.add(lblFileName, null);
        jScrollPane1.getViewport().add(jtaRDRStatus, null);

//        add(cbxDisplayBlobOption, null);

    }

    boolean shouldDisplayBlob(Blob b) {
        if (iPaintOption == PAINT_GOOD_BLOB)
            return blobRDR.classify(b) == CommonSense.IS_BLOB;
        else if (iPaintOption == PAINT_BAD_BLOB)
            return blobRDR.classify(b) != CommonSense.IS_BLOB;
        else{
            System.err.println("BlobIntervalRDRTool : iPaintOption is incorrectly set " + iPaintOption);
            return false;
        }
    }

    private Blobber getBlobber() {
        return parent.blobber;
    }

    void updateDisplayBlobList() {
        numberOfBlob = 0;
        ArrayList blobList = getBlobList();
        getBlobber().clearDisplayBlobList();
        getBlobber().setDisplayColor(Color.MAGENTA);
//        System.out.println("BlobIntervalRDR : Updating blob list");
        for (Iterator i = blobList.iterator(); i.hasNext(); ) {
            Blob b = (Blob) i.next();
            if (shouldDisplayBlob(b)) {
                getBlobber().addDisplayingBlob(b);
                numberOfBlob ++;
//                System.out.println("Displaying blob " + b.toString());
            }
        }
        notifyObservers();
        needUpdating = false;
    }

    private void classifyBlob(Blob b, int C) {
        ExampleBlob be = new ExampleBlob(b, C, blobRDR);
        Pair[] metrics = blobRDR.getAllMetrics(be.example);
        System.out.println("Classifying Blob : " + b.toString());
        for (int i = 0; i < metrics.length; i++)
            System.out.println("  " + metrics[i]);

        if (blobRDR.classify(b) != C) {
            int res = blobRDR.update(be);
            jtaRDRStatus.setText("U : "+ GenericRDRRule.STATUS_TO_STRING[res]+"\n");
            needUpdating = true;
        }
        else {
            needUpdating = false;
            jtaRDRStatus.setText("Example is consistent. \nNo rule added\n");
            System.out.println("Example is consistent. No rule added\n");
        }
    }

    public Blob getBlobAtCursor(int x, int y) {
        return parent.blobber.getBlobAtCursor(x, y);
    }

    public void mousePressed(MouseEvent e) {
        int x = parent.bflPanel.getPosX(e.getX());
        int y = parent.bflPanel.getPosY(e.getY());
        if (SwingUtilities.isMiddleMouseButton(e)){
            Blob [] allOverBlob = getBlobber().getAllBlobsAtCursor(x, y);
            if (allOverBlob.length > 1){
                Blob b = allOverBlob[allOverBlob.length - 1];
                ArrayList blobList = getBlobber().getDisplayingBlobList();
                blobList.remove(b);
                blobList.add(0, b);
                System.out.println("Blob is brought up");
            }
            else{
                System.err.println("There are less than 2 blobs at cursor (" + allOverBlob.length + ")");
            }
        }
        else{
            Blob overBlob = getBlobAtCursor(x, y);
            if (overBlob != null) {
                if (SwingUtilities.isRightMouseButton(e)) {
                    classifyBlob(overBlob, CommonSense.IS_NOT_BLOB);
                }
                else {
                    classifyBlob(overBlob, CommonSense.IS_BLOB);
                }
                parent.repaint();
            }
        }
    }

    public void setPaintOption(int opt) {
        if (iPaintOption != opt) {
            iPaintOption = opt;
            needUpdating = true;
        }
    }

    void blobOpenBFL_actionPerformed(ActionEvent e) {
        parent.openBFL();
    }

    void blobOpenRDR_actionPerformed(ActionEvent e) {
        openBlobIntervalRDR();
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
        blobRDR.loadFromFile(filename);
        setFileName();
        needUpdating = true;
        parent.repaint();
    }

    void blobSaveRDR_actionPerformed(ActionEvent e) {
        if (blobRDR.fileName != null){
            saveBlobRDR(blobRDR.fileName);
        }
        else{
            saveBlobIntervalRDRAs();
        }
    }

    void blobSaveAsRDR_actionPerformed(ActionEvent e) {
        saveBlobIntervalRDRAs();
    }

    public void saveBlobRDR(String filename) {
        blobRDR.saveToFile(filename);
        setFileName();
    }

    public void saveBlobIntervalRDRAs() {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.getLastRDRFileChooser()));
        int returnVal = RDRApplication.RDRFileChooser.showSaveDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().
            getPath();
        RDRApplication.appConf.setLastRDRFileChooser(RDRApplication.RDRFileChooser.getSelectedFile().getParent());
        saveBlobRDR(filename);

    }



    public String name() {
        return "Blob Interval";
    }

    public void paintBFLPanel(Graphics g) {
        if (! enabled) return;
        if (needUpdating)
            updateDisplayBlobList();
    }

    public void setToolEnabled(boolean b){
        enabled = b;
        chbEnabled.setSelected(b);
        needUpdating = b;
    }

    public void setBlobRDR(ManRDRBlobInterval rdr){
        blobRDR = rdr;
        setFileName();
        needUpdating = true;
    }

    //////////////////////// OBSERVER INTERFACE //////////////////////////////
    public void update(Subject o) {
        if (forcedUpdate) updateDisplayBlobList();
        else
            needUpdating = true;
    }


    /////////////////// EVENT HANDLING CODE /////////////////////////////////

    void showBlobButton_actionPerformed(ActionEvent e) {
        parent.blobber.formBlobs();
        parent.repaint();
    }

    void btPrintBlobRDR_actionPerformed(ActionEvent e) {
        blobRDR.print();
    }

    void setFileName(){
        if (blobRDR.fileName == null){
            lblFileName.setText("File:" + blobRDR.fileName);
        }
        else
            lblFileName.setText("File:" + new File(blobRDR.fileName).getName());
    }
    void btClearBlobRDR_actionPerformed(ActionEvent e) {
        MetricBlob [] metrics = blobRDR.getRuleMetrics();
        blobRDR = new ManRDRBlobInterval(metrics);
        setFileName();

        needUpdating = true;
        parent.repaint();
    }

    void btUndo_actionPerformed(ActionEvent e) {
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

    void btAddToCollection_actionPerformed(ActionEvent e) {
        int colour = parent.getSelectedBlobColour();
        if (blobRDR.fileName == null){
            JOptionPane.showMessageDialog(this,"You have to save the RDR first, please try it again!","Error",JOptionPane.ERROR_MESSAGE);
            return;
        }
        if (JOptionPane.showConfirmDialog(this,
            "Do you want to add this Blob RDR for colour " +
                                          CommonSense.getColourName(colour) + "\n" +
                                          "and start new BlobIntervalRDR?",
                                          "Confirm",
                                          JOptionPane.OK_CANCEL_OPTION) !=
            JOptionPane.CANCEL_OPTION) {
            blobRDR.saveToFile(blobRDR.fileName);
            if (parent.addBlobRDRToCollection(blobRDR, colour)){
                btClearBlobRDR_actionPerformed(null); //DANGEROUS
            }
            else{
                JOptionPane.showMessageDialog(this,"RDR Colour out of range " + colour + "[" + CommonSense.getColourName(colour) + "]","Error",JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    void chbEnabled_stateChanged(ChangeEvent e) {
        enabled = chbEnabled.isSelected();
    }

    public void mouseMoved(MouseEvent e) {
        if (e.isShiftDown()){
            int x = parent.bflPanel.getPosX(e.getX());
            int y = parent.bflPanel.getPosY(e.getY());
//            System.out.println("Blob at " + x + " " + y);
            Blob b = getBlobAtCursor(x, y);
            getBlobber().setSpecialBlob(b, null);
            parent.repaint();
        }
    }

}
