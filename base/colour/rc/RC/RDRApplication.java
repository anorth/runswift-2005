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
 * @version 1.4
 * CHANGE LOG:
 * + version 1.1:
 *     - Added Show No Blob option.
 *     - Fix Thumbnail refreshing bug
 * + version 1.2:
 *     - Save BFL as, batch saving. (sothat new colour calibration is embeded in bfl files
 *     - Added "Next" "Previous" button in the main toolbar
 *     - BlobRDR menu : open, open recents
 *     - Multiple Blob RDR control
 *     - Enable/Disable tools when activating tool tab.
 *     - Blob relation rdr
 * + version 1.3:
 *     - Make Next/Previous button to jump to the next bfl that has blob
 *     - Add RoboCommander to control the dog from station
 *     - Add RoboController tool
*      - Add Blob Viewer tool to view blobber (seem to be useless atm) due to..
*      - ... Middle Button trick. Change the order of the "clickable" blob when middle mouse
*       button is clicked.
*      - Add Blob Relation RDR (Object RDR) menu
*      - Fix bug Next/Previous button to jump to the next bfl that has HIGHLIGHTED
*        blob (by force BlobIntervalRDRTool to update, as opposed to lazy update)
*  + version 1.4:
*      - put blob rdr & object rdr together in one new tool
*      - Blobber Colour selection dialog in ToolCombined (for visible blob list)
*      - Speed up BFL directory loading
 */

/** @maybetodo no need CPLANE anymore
 * Fix bug clicking on MyToolBar when loading CPLANE_LOG and BFL at the same time
 */

/** @maybetodo
* Try decision tree to induct rdr */

/** @todo
* left right nagivation key */

import java.io.*;
import java.text.*;
import java.util.*;

import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

import RoboComm.*;
import RoboShare.*;
import Tool.*;
import java.awt.*;

public class RDRApplication
    extends RDRFrame
    implements YUVListener{
    static public MyConfiguration appConf = new MyConfiguration();
    BlobViewer viewer = null;


    public Blobber blobber = null;

    static public final int SKIP_NONE = 0;
    static public final int SKIP_UNTIL_BLOB = 1;
    static public final int SKIP_UNTIL_HIGHLIGHTED_BLOB = 2;
    static public final int SKIP_UNTIL_OBJECT = 3;
    static public final int SKIP_UNTIL_NO_HIGHLIGHTED_BLOB = 4;
    static public final int SKIP_UNTIL_NO_OBJECT = 5;
    static public final int SKIP_SINGLE_GOOD_BLOB = 6;
    static public final int SKIP_SINGLE_GOOD_OBJECT = 7;
    int iSkipOption = SKIP_NONE;

    /**
     * How to add new tool:
     *   create a new variable
     *   add it in the list allTool
     */
    protected ToolColourRDR toolColourRDR = new ToolColourRDR(this);
//    protected BlobRDRTool toolBlobRDRTool = new BlobRDRTool(this);
    protected ToolBlobIntervalRDR toolBlobIntervalRDR = new ToolBlobIntervalRDR(this);
    protected ToolBlobRelationRDR toolBlobRelationRDR = new ToolBlobRelationRDR(this);
    public ToolCombinedRDR toolCombinedRDR = new ToolCombinedRDR(this);
    protected ToolAutoColour toolAutoColour = new ToolAutoColour(this);

    //add new tool here
    protected GenericTool allTool [] = {
        toolColourRDR,
//        toolBlobRDRTool,
//        toolBlobIntervalRDR,
//        toolBlobRelationRDR,
        toolCombinedRDR,
        toolAutoColour,
    };

    RobotController robotController = null;
    EvaluationTool evaTool = null;
    ReLearningTool relearningTool = null;
    public VisionObserver visionObserver = new VisionObserver();

    boolean bReceiveStream = false;
    static public JFileChooser BFLFileChooser = new JFileChooser();
    static public JFileChooser RDRFileChooser = new JFileChooser();
    static public JFileChooser DirChooser = new JFileChooser();

    ArrayList cplaneList = null;


    public RDRApplication() {
        super();

        initGUI();
        bfl = new BFL(false);
        initMemberVariables();
    }

    private void initGUI(){
        BFLFileChooser.setFileFilter(new MyFileFilter("bfl","YUV File format"));
        RDRFileChooser.setFileFilter(new MyFileFilter("xml", "Ripple Down Rule file (XML)"));
        DirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        if (appConf.getLastBFLDirectory() != ""){
            itemRecentBFLDirectory.setText(appConf.getLastBFLDirectory());
            itemRecentBFLDirectory.setActionCommand(appConf.getLastBFLDirectory());
            itemRecentBFLDirectory.setEnabled(true);
        }
        else
            itemRecentBFLDirectory.setEnabled(false);

        if (appConf.getLastBFLFileChooser() != ""){
            itemRecentBFLFile.setText(appConf.getLastBFLFileChooser());
            itemRecentBFLFile.setActionCommand(appConf.getLastBFLFileChooser());
            itemRecentBFLFile.setEnabled(true);
        }
        else
            itemRecentBFLFile.setEnabled(false);

        if (appConf.getLastColourRDR() != "") {
            itemRecentColourRDR.setText(appConf.getLastColourRDR());
            itemRecentColourRDR.setActionCommand(appConf.getLastColourRDR());
            itemRecentColourRDR.setEnabled(true);
        }
        else
            itemRecentColourRDR.setEnabled(false);

        if (appConf.getLastBlobIntervalRDR() != "") {
            itemRecentBlobIntervalRDR.setText(appConf.getLastBlobIntervalRDR());
            itemRecentBlobIntervalRDR.setActionCommand(appConf.getLastBlobIntervalRDR());
            itemRecentBlobIntervalRDR.setEnabled(true);
        }
        else
            itemRecentBlobIntervalRDR.setEnabled(false);

        if (appConf.getLastObjectRDRFile() != "") {
            itemRecentBlobIntervalRDR.setText(appConf.getLastObjectRDRFile());
            itemRecentBlobIntervalRDR.setActionCommand(appConf.getLastObjectRDRFile());
            itemRecentBlobIntervalRDR.setEnabled(true);
        }
        else
            itemRecentBlobIntervalRDR.setEnabled(false);



    }

    private void initMemberVariables(){

        blobber = new Blobber(bflPanel);

        for (int i=0;i<allTool.length;i++){
            jTabbedPane1.add(allTool[i], allTool[i].name());
            bflPanel.registerPainter(allTool[i]);
        }

        //register painter
        bflPanel.registerPainter(blobber);

        //connect subject <--> observer
        bfl.addObserver(toolColourRDR); //this must be before blobber.
        bfl.addObserver(blobber);

//        blobber.addObserver(toolBlobRDRTool);
        blobber.addObserver(toolBlobIntervalRDR);
        blobber.addObserver(toolBlobRelationRDR);
        blobber.addObserver(toolCombinedRDR);
        blobber.addObserver(visionObserver);

        toolBlobRelationRDR.addObserver(visionObserver);
        toolBlobIntervalRDR.addObserver(visionObserver);
        toolCombinedRDR.addObserver(visionObserver);
    }

    void showHackedData(){
        int [] sensorData = bfl.getHackedSensorData();
        jtfPan.setText( new DecimalFormat("00.00").format(Utils.micro2deg(sensorData[6])) );
        jtfTilt.setText( new DecimalFormat("00.00").format(Utils.micro2deg(sensorData[7])) );
        jtfCrane.setText( new DecimalFormat("00.00").format(Utils.micro2deg(sensorData[2])) );
//        for (int i=0;i<sensorData.length;i++)
//            System.out.print(" " + Utils.micro2deg(sensorData[i]) );
//        System.out.println("");
    }

    public void openBFL(String filename){

        if (filename.startsWith("CPLANE_LOG")){
            int index = Integer.parseInt(filename.substring(10).trim());
            bfl = ((CPlane)cplaneList.get(index)).toBFL();
        }
        else{
            try {
                bfl.readFile(filename);
                this.setTitle(filename);
            }
            catch (FileNotFoundException ex) {
                JOptionPane.showMessageDialog(this,"File not found","Error", JOptionPane.ERROR_MESSAGE);
            }
            catch (IOException ex) {
                ex.printStackTrace();
            }
        }

        showHackedData();
        repaint(500);
    }

    protected void openBFL() {
        BFLFileChooser.setCurrentDirectory(new File(appConf.getLastBFLFileChooser()).getParentFile() );
        int returnVal = BFLFileChooser.showOpenDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = BFLFileChooser.getSelectedFile().getPath();
        appConf.setLastBFLFileChooser(filename);

        openBFL(filename);
        if (bfl.fileName != null)
            mytoolbarThumbnail.addNewThumbnail(bfl.toImage(), bfl.fileName);
        repaint(500);
    }

    protected void openBFLDirectory(String directory) {
        try {
            File file = new File(directory);
            String[] allBFLFile = file.list(new FilenameFilter() {
                public boolean accept(File dir, String filename) {
                    return filename.endsWith("bfl") || filename.endsWith("BFL");

                }
            });
            if (allBFLFile != null) {
                for (int i = 0; i < allBFLFile.length; i++) {
                    File theFile = new File(file.getAbsolutePath(),
                                            allBFLFile[i]);
                    System.out.println("Opening " + theFile.getAbsolutePath());
                    BFL newBFL = new BFL(false);
                    setTitle(theFile.getAbsolutePath());
                    newBFL.readFile(theFile.getAbsolutePath());
                    mytoolbarThumbnail.addNewThumbnail(newBFL.toImage(),
                                                       newBFL.fileName);

                }
                if (mytoolbarThumbnail.count() > 0){
                    mytoolbarThumbnail.activatePreviousThumbnail();
                }

            }
            else {
                JOptionPane.showMessageDialog(this,
                                              "No images found in directory " +
                                              directory, "Error",
                                              JOptionPane.ERROR_MESSAGE);
            }
        }
        catch (HeadlessException ex) {
        }
        catch (FileNotFoundException ex) {
            System.err.println("BFL File not found ");
        }
        catch (IOException ex) {
            System.err.println("BFL File can not be read ");
        }
    }

    public void notifyNewBFL(BFL streamedBFL) {
        if (bReceiveStream) {
            bfl.copy(streamedBFL);
            showHackedData();
            mytoolbarThumbnail.addNewThumbnail(bfl.toImage(), bfl.fileName);
            this.setTitle("Streamed BFL : " + bfl.fileName);
            repaint();
        }
    }

    private void showBlobInformation(Blob b){
        jtfPos.setText(b.xMin + "/" + b.yMin);
        jtfSize.setText(b.getWidth() + "/" + b.getHeight() );
        jtfArea.setText("" + b.area);
        jtfDistance.setText("" + (new HeightMetric(-1)).getMetric(b));
    }

    private void shutDown(){
        appConf.shutDown();
        System.exit(0);
    }

    void loadCPlaneImages(ArrayList cplaneList){
        this.cplaneList = cplaneList;
        int i=0;
        for (Iterator it = cplaneList.iterator();it.hasNext();i++){
            CPlane c = (CPlane) it.next();
            mytoolbarThumbnail.addNewThumbnail(c.toBFL().toImage(), MyToolBar.CPLANE_LOG_PREFIX + " " + i);
        }
    }

    private ManRDRBlobInterval [] rdrCollection = new ManRDRBlobInterval[CommonSense.NUMBER_USABLE_COLOR];

    public ManRDRBlobInterval getRDRForColour(int colour){
        if (colour >= 0 && colour < rdrCollection.length){
            return rdrCollection[colour];
        }
        else
            return null;
    }

    boolean addBlobRDRToCollection(ManRDRBlobInterval rdr, int colour){
        if (colour < 0 || colour >= rdrCollection.length){
            return false;
        }
        else{
            rdrCollection[colour] = rdr;
            updateRDRList();
            return true;
        }
    }

    boolean deleteBlobRDRFromCollection(int colour){
        if (colour < 0 || colour > rdrCollection.length){
            return false;
        }
        else{
            rdrCollection[colour] = null;
            updateRDRList();
            return true;
        }
    }

    private void updateRDRList(){
        lstRDRCollection.removeAll();
        int count = 0;
        for (int i=0;i<rdrCollection.length;i++)
            if (rdrCollection[i] != null){
                count ++;
            }
        String [] listData = new String[count];
        count = 0;
        for (int i=0;i<rdrCollection.length;i++)
            if (rdrCollection[i] != null){
                listData[count++] = new String(CommonSense.COLOUR_NAME[i]);
            }
        lstRDRCollection.setListData(listData);
    }

    private void deleteSelectedRDRFromCollection(){
        if (lstRDRCollection.getSelectedValue() == null){
            JOptionPane.showMessageDialog(this,"No RDR is selected","Error",JOptionPane.ERROR_MESSAGE);
        }
        else{
            int colour = CommonSense.getColourFromName((String)lstRDRCollection.getSelectedValue());
            if (colour != -1){
                deleteBlobRDRFromCollection(colour);
                updateRDRList();
            }
        }
    }

    public int getSelectedBlobColour(){
        return blobber.currentColor;
    }

/////////////////////////////////// Event handling code ////////////////////////


    // Overridden so we can exit when window is closed

    protected void processWindowEvent(WindowEvent e) {
        super.processWindowEvent(e);
        if (e.getID() == WindowEvent.WINDOW_CLOSING) {
            shutDown();
        }
    }

    void itemBFLSaveAs_actionPerformed(ActionEvent e) {
        BFLFileChooser.setCurrentDirectory(new File(appConf.getLastBFLFileChooser()) );
        int returnVal = BFLFileChooser.showSaveDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = BFLFileChooser.getSelectedFile().getPath();
        try {
            bfl.saveFile(filename);
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    void itemBFLSave_actionPerformed(ActionEvent e) {
        if (bfl.fileName == null) {
            itemBFLSaveAs_actionPerformed(e);
        }
        else {
            try {
                bfl.saveFile();
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }

    }




    void jTabbedPane1_stateChanged(ChangeEvent e) {
        Object component = jTabbedPane1.getSelectedComponent();
        for (int i=0;i<allTool.length;i++){
            allTool[i].setToolEnabled(component == allTool[i]);
        }

        if (component == toolColourRDR) {
            cardLayout1.show(jPanel1, "colourPanel");
            System.out.println("Colour");
        }
        else if (jTabbedPane1.getSelectedComponent() == toolBlobIntervalRDR) {
            cardLayout1.show(jPanel1, "blobInfoPanel");
            System.out.println("Blob");
        }
        else if (jTabbedPane1.getSelectedComponent() == toolBlobRelationRDR) {
            cardLayout1.show(jPanel1, "blobInfoPanel");
            System.out.println("Blob");
            cbxBlobColor.setSelectedIndex(CommonSense.NUMBER_USABLE_COLOR); //set to ALL (which is the last index)
        }

        repaint();

    }



//    //just for testing
//    void cbxBlobColor_actionPerformed(ActionEvent e) {
//        System.out.println("action:" + e.getActionCommand() +
//                           "index : " + cbxBlobColor.getSelectedIndex());
//
//    }

    void cbxBlobColor_itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED){
            System.out.println("ItemEvent " + e.getItem() + " " +
                               e.getStateChange());
            if (blobber != null){
                blobber.showOnlyBlobColor(cbxBlobColor.getSelectedIndex(),
                                          e.getItem().toString());
                repaint();
            }
        }
    }



    GenericTool getCurrentTool(){
        return (GenericTool) jTabbedPane1.getSelectedComponent();
    }


    void itemRecentBFLFile_actionPerformed(ActionEvent e) {
        openBFL(e.getActionCommand());
        if (bfl.fileName != null)
            mytoolbarThumbnail.addNewThumbnail(bfl.toImage(), bfl.fileName);
    }

    void itemRecentColourRDR_actionPerformed(ActionEvent e) {
        toolColourRDR.loadRDRFromFile(e.getActionCommand());
    }

    void displayColor(int color){
        toolColourRDR.setDisplayColor(color);
        repaint();
    }


    void itemOpenBFLDirectory_actionPerformed(ActionEvent e) {

        DirChooser.setCurrentDirectory(new File(appConf.getLastBFLDirectory() ) );
        DirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        int returnVal = DirChooser.showOpenDialog(this);
        if (returnVal != JFileChooser.CANCEL_OPTION) {
            String directory = DirChooser.getSelectedFile().getPath();
            appConf.setLastBFLDirectory(directory);
            openBFLDirectory(directory);
        }
    }

    void itemPreference_actionPerformed(ActionEvent e) {
        PreferenceDialog prefDialog = new PreferenceDialog(this, "Preferences",true);
        prefDialog.setVisible(true);
    }

    void itemExit_actionPerformed(ActionEvent e) {
        shutDown();
    }

    void itemOpenColourRDR_actionPerformed(ActionEvent e) {
        toolColourRDR.btLoadColourRDR_actionPerformed(e);
    }

    void itemOpenCPLANE_LOG_actionPerformed(ActionEvent e) {
        JFileChooser chooser = new JFileChooser(appConf.getWorkingDirectory());
        int ret = chooser.showOpenDialog(this);
        if (ret != JFileChooser.CANCEL_OPTION){
            String fileName = chooser.getSelectedFile().getPath();
            CPlaneLog cplaneLog = new CPlaneLog(fileName);
            loadCPlaneImages(cplaneLog.getCPlaneList() );
        }
    }

    void cbxShowBlobOption_itemStateChanged(ItemEvent e) {

        if (e.getStateChange() == ItemEvent.SELECTED){
            System.out.println("ItemEvent " + e.getItem() + " " +
                               e.getStateChange() + " selected index " + cbxShowBlobOption.getSelectedIndex());

            if (blobber == null ) return;

            switch (cbxShowBlobOption.getSelectedIndex()) {
                case 0: //ALL BLOB
                    blobber.setPaintOption(Blobber.PAINT_ALL_BLOB);
                    break;
                case 1: //GOOD BLOB
                    blobber.setPaintOption(Blobber.PAINT_HIGHLIGHTED_ONLY);
                    break;
                case 2: //NO BLOB
                    blobber.setPaintOption(Blobber.PAINT_NO_BLOB);
                    break;
                case 3: //NON HIGHLIGHTED BLOB
                    blobber.setPaintOption(Blobber.PAINT_NON_HIGHLIGHTED);
                    break;

            }
            repaint();
        }

    }

    void itemRecentBFLDirectory_actionPerformed(ActionEvent e) {
        openBFLDirectory(e.getActionCommand());
    }

    void itemBFLOpen_actionPerformed(ActionEvent e) {
        openBFL();
    }

    void itemJointViewer_actionPerformed(ActionEvent e) {
        JointTool jointTool = new JointTool();
        jointTool.setSensorData(bfl.getHackedSensorData());
        jointTool.pack();
        jointTool.setVisible(true);
    }

    //mediator : pass events to tool classes.

    void bflPanel_mouseClicked(MouseEvent e) {
        getCurrentTool().mouseClicked(e);
    }

    void bflPanel_mouseDragged(MouseEvent e) {
        getCurrentTool().mouseDragged(e);
    }

    void bflPanel_mouseMoved(MouseEvent e) {
        if (bfl != null && blobber != null) {
            int x = bflPanel.getPosX(e.getX());
            int y = bflPanel.getPosY(e.getY());
            if (!bfl.outOfBound(x, y)) {
                jtfStatus.setText("Pos = " + x + "," + y +
                                  ", C = " + bfl.getC(x, y) +
                                  " Y = " + bfl.getY(x, y) +
                                  " U = " + bfl.getU(x, y) +
                                  " V = " + bfl.getV(x, y) +
                                  " Elev = " + bfl.getElevation(x, y));
//                                  " HeigthMetric = " + bfl.getElevation(x,y) );
                Blob overBlob = blobber.getBlobAtCursor(x, y);
                if (overBlob != null) {
                    showBlobInformation(overBlob);
                }
            }
        }
        if (getCurrentTool() != null)
            getCurrentTool().mouseMoved(e);
    }

    void bflPanel_mousePressed(MouseEvent e) {
        getCurrentTool().mousePressed(e);
    }

    void bflPanel_mouseReleased(MouseEvent e) {
        getCurrentTool().mouseReleased(e);
    }

    public void applyColourRDRToAllOpenBFL(){
        toolColourRDR.apply_rdrAll(mytoolbarThumbnail.getAllBFLFiles());
    }

    void itemApplyAll_actionPerformed(ActionEvent e) {
        applyColourRDRToAllOpenBFL();
    }

    void itemOpenBlobRDR_actionPerformed(ActionEvent e) {
        toolBlobIntervalRDR.openBlobIntervalRDR();
    }

    void itemSaveBlobRDR_actionPerformed(ActionEvent e) {
        toolBlobIntervalRDR.saveBlobIntervalRDRAs();
    }

    void btRemoveRDR_actionPerformed(ActionEvent e) {
        deleteSelectedRDRFromCollection();
    }

    void srlRDRCollection_mouseClicked(MouseEvent e) {
        if (e.getClickCount() >= 2){
            String colourName = (String)lstRDRCollection.getSelectedValue();
            if (colourName != null){
                int colour = CommonSense.getColourFromName(colourName);
                if (colour != -1){
                    if (JOptionPane.showConfirmDialog(this,"Current Blob RDR will be lost if you haven't saved it.Are you sure want to open " + colourName + " 's blob rdr?", "Confirm", JOptionPane.YES_NO_OPTION) !=
                        JOptionPane.NO_OPTION){
                        ManRDRBlobInterval rdr = getRDRForColour(colour);
                        if (rdr != null){
                            cbxBlobColor.setSelectedItem(colourName);
                            toolBlobIntervalRDR.setBlobRDR(rdr);
                            repaint();
                        }
                    }
                }
            }
        }
    }

    void itemRobotController_actionPerformed(ActionEvent e) {
        if (robotController == null){

            robotController = new RobotController(this);
            robotController.pack();
            visionObserver.addObserver(robotController);
        }
        robotController.setVisible(true);
    }

    private boolean satisfiedSkipOption(){
        System.out.println("Displaying blob : " + blobber.displayBlobList.size());
        System.out.println("All blob : " + blobber.visibleBlobList.size());
        System.out.println("Number of blobs : " + visionObserver.numberOfBlob);
        System.out.println("Number of good blobs : " + visionObserver.numberOfGoodBlob);
        System.out.println("Number of objects : " + visionObserver.numberOfObject);
        switch (iSkipOption){
            case SKIP_UNTIL_BLOB:
                return blobber.visibleBlobList.size() == 0;
            case SKIP_UNTIL_HIGHLIGHTED_BLOB:
                return visionObserver.numberOfGoodBlob == 0;
            case SKIP_UNTIL_OBJECT:
                return visionObserver.numberOfObject == 0;
            case SKIP_UNTIL_NO_OBJECT:
                return visionObserver.numberOfObject > 0;
            case SKIP_UNTIL_NO_HIGHLIGHTED_BLOB:
                return visionObserver.numberOfGoodBlob > 0;
            case SKIP_SINGLE_GOOD_BLOB:
                return visionObserver.numberOfGoodBlob == 1;
            case SKIP_SINGLE_GOOD_OBJECT:
                return visionObserver.numberOfObject == 1;
            default:

                return false;
        }
    }

    void btNext_actionPerformed(ActionEvent e) {
        int count = 0;
        long t = System.currentTimeMillis();
        System.out.println("Scrolling ... "  );
        do{
            mytoolbarThumbnail.activateNextThumbnail();
            repaint();

        } while ( satisfiedSkipOption() && count ++ < mytoolbarThumbnail.count());
        System.out.println("Done , in " + (System.currentTimeMillis() - t) / 1000000.0);
    }

    void btPrevious_actionPerformed(ActionEvent e) {
        int count = 0;
        do{
            mytoolbarThumbnail.activatePreviousThumbnail();
            repaint();
        } while ( satisfiedSkipOption() && count ++ < mytoolbarThumbnail.count());
    }

    void cbxSkipOption_itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED){
            iSkipOption = cbxSkipOption.getSelectedIndex();
        }

    }

    void itemBlobViewer_actionPerformed(ActionEvent e) {
        if (viewer == null) {
            viewer = new BlobViewer(this);
            viewer.pack();
        }
        viewer.showBlobs();
    }

    void itemOpenObjectRDR_actionPerformed(ActionEvent e) {
        toolBlobRelationRDR.openBlobRelationRDR();
    }

    void itemSaveObjectRDR_actionPerformed(ActionEvent e) {
        toolBlobRelationRDR.saveBlobRelationRDRAs();
    }

    void itemRecentObjectRDR_actionPerformed(ActionEvent e) {
        toolBlobRelationRDR.openBlobRelationRDRFromFile(e.getActionCommand());
    }

    void btPrevious_keyPressed(KeyEvent e) {
        handleKeyPress(e);
        e.consume();
    }

    void btNext_keyPressed(KeyEvent e) {
        handleKeyPress(e);
        e.consume();
    }

    void handleKeyPress(KeyEvent e) {
        System.out.println("Key pressed " + e.getKeyChar() + " " + e.getKeyCode());
        switch (e.getKeyCode()) {
            case 39:
                mytoolbarThumbnail.activateNextThumbnail();
                break;
            case 37:
                mytoolbarThumbnail.activatePreviousThumbnail();
                break;
        }

    }

    void copyFile(String distDir){
        String fileName = RCUtils.generateUniqueName(distDir);
        try {
            bfl.saveToFile(fileName);
            System.out.println("Copy successfully to " + fileName);
        }
        catch (FileNotFoundException ex) {
            System.out.println("Directory Not Found");
        }
        catch (IOException ex) {
            System.out.println("IOException");
        }
   }

   static final String pathSeparatorChar= "\\";

    void classifyImage(ActionEvent e) {
        String ballImageDir = appConf.getYUVDirectory() + pathSeparatorChar + "ballImages";
        String goalImageDir = appConf.getYUVDirectory() + pathSeparatorChar + "goalImages";
        String beaconImageDir = appConf.getYUVDirectory() + pathSeparatorChar + "beaconImages";
        String beaconPinkYellowImageDir = beaconImageDir + pathSeparatorChar + "pinkYellow";
        String beaconPinkBlueImageDir = beaconImageDir + pathSeparatorChar + "pinkBlue";
        String noneImageDir = appConf.getYUVDirectory() + pathSeparatorChar + "noneImages";
        String notsureImageDir = appConf.getYUVDirectory() + pathSeparatorChar + "notsureImages";

        if (bfl.fileName == null){
            System.out.println("BFL file is empty");
            return;
        }
        if (e.getSource() == btSortBall){
            copyFile(ballImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortBallCopy){
            copyFile(ballImageDir);
        }
        else if (e.getSource() == btSortGoal){
            copyFile(goalImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortGoalCopy){
            copyFile(goalImageDir);
        }
        else if (e.getSource() == btSortBeacon){
            copyFile(beaconImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortBeaconCopy){
            copyFile(beaconImageDir);
        }
        else if (e.getSource() == btSortNone){
            copyFile(noneImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortNoneCopy){
            copyFile(noneImageDir);
        }
        else if (e.getSource() == btSortNotSure){
            copyFile(notsureImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortPinkYellow){
            copyFile(beaconPinkYellowImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortPinkYellowCopy){
            copyFile(beaconPinkYellowImageDir);
        }
        else if (e.getSource() == btSortPinkBlue){
            copyFile(beaconPinkBlueImageDir);
            deleteBFL();
        }
        else if (e.getSource() == btSortPinkBlueCopy){
            copyFile(beaconPinkBlueImageDir);
        }
        else if (e.getSource() == btTwo){
            renamePrefix("2_");
        }
        else if (e.getSource() == btThree){
            renamePrefix("3_");
        }

        else if (e.getSource() == btDelete){
            deleteBFL();
        }


    }

    public void deleteBFL() {
        mytoolbarThumbnail.removeThumbnail(bfl.fileName);
        (new File(bfl.fileName)).delete();
        btNext_actionPerformed(null);
    }

    public void renamePrefix(String prefix){
        String oldFile = bfl.fileName;
        File file = new File(oldFile);
        String newName = prefix + file.getName();
        try {
            System.out.println("Saving to " + newName);
            bfl.saveToFile(new File(file.getParent(), newName).getAbsolutePath());
            deleteBFL();
        }
        catch (FileNotFoundException ex) {
            ex.printStackTrace();
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    void itemEvaluationTool_actionPerformed(ActionEvent e) {
        if (evaTool == null){
            evaTool = new EvaluationTool(this);
            evaTool.pack();
        }
        evaTool.setVisible(true);
    }

    void itemReLearningTool_actionPerformed(ActionEvent e) {
        if (relearningTool == null){
            relearningTool = new ReLearningTool(this);
            relearningTool.pack();
        }
        relearningTool.setVisible(true);

    }


    //for automatic evaluation tool
    public void loadColourRDR(String fileName){
        toolColourRDR.loadRDRFromFile(fileName);
        toolColourRDR.chbUseRDR.setSelected(true);
    }

//    public void loadBlobIntervalRDR(String fileName, boolean isGoal){
//
//        toolCombinedRDR.chbIsGoal.setSelected(isGoal);
//
//        toolCombinedRDR.loadBlobIntervalRDRFromFile(fileName);
//
//    }

    public void loadBlobRelationRDR(String fileName){
        toolCombinedRDR.openBlobRelationRDRFromFile(fileName);
    }

    public void addToThumbnailBar(String fileName){
        BFL bfl = new BFL(false);
        try {
            bfl.readFile(fileName);
            mytoolbarThumbnail.addNewThumbnail(bfl.toImage(), bfl.fileName);
        }
        catch (FileNotFoundException ex) {
            System.out.println("Could not open " + fileName);
        }
        catch (IOException ex) {
            System.out.println("Could not open " + fileName);
        }
    }

    public MyToolBar getThumbnailToolbar(){
        return mytoolbarThumbnail;
    }

    void itemRDRInfo_actionPerformed(ActionEvent e) {

        //debuggin bfl
        bfl.printDebugging();
        //rdr info
        String infoMessage = new String();
        infoMessage += "<html>";
        infoMessage += "<b>Blob RDR Rule Space</b>:<br>";
        String [] metricNames = toolCombinedRDR.blobRDR.getMetricsDescription();
        for (int i = 0; i < metricNames.length ; i++) {
            infoMessage += "     <i>" +
                metricNames[i] +
                "<br>";
        }
        infoMessage += "<br><b>Object RDR Rule Space</b>:<br>";
        for (int i = 0; i < toolCombinedRDR.blobRelationRDR.getNumberOfMetrics() ; i++) {
            infoMessage += "     <i>" +
                toolCombinedRDR.blobRelationRDR.getMetric(i ).toString() +
                "<br>";
        }
        infoMessage += "</html>";
//        System.out.println(infoMessage);
        JOptionPane.showMessageDialog(this,
                                      infoMessage,
                                      "RDR Rule Information",
                                      JOptionPane.INFORMATION_MESSAGE);
    }

    public Blobber getBlobber(){
        return blobber;
    }
}
