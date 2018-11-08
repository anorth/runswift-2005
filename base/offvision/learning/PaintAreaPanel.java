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
 * UNSW 2003 Robocup (Alex)
 *
 * Last modification background information
 * $Id: PaintAreaPanel.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * the panel to display the cplane and objects and blobs
 *
**/

//package learning;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.*;
import javax.swing.event.MouseInputAdapter;
import java.io.*;
import java.util.*;

public class PaintAreaPanel extends JPanel {

    public final static boolean boxBehindPixelDebugMsg = false;
    public final static boolean clickBlobDebugMsg      = false;
    private static final boolean doubleClickDebugMsg   = false;

    public static final int NUM_OBJ    = Constant.detailObjectNames.length;
    public final static int MAX_VAL    = 128;
    public final static int PIC_WIDTH  = 176;
    public final static int PIC_HEIGHT = 144;
    public final static int FACTOR     = 2; // for the colour cube
    
    public final static int NO_SELECTED_BLOB = -1;

    // the desire size gap to the top edge so that object names can be displayed correct
    public final static int TOP_GAP = 15;
    
    // the maximum interval of two consecutive clicks that makes up a double click
    public static final int DOUBLE_CLICK_INTERVAL = 500;

    public static final Color[] classifyColours = {
        Color.orange, new Color(127,127,255), new Color(0,127,0),
        Color.yellow, Color.pink, new Color(0,0,200), Color.red,
        new Color(127,255,127), new Color(200, 200, 200), Color.white, // maybe start
        new Color(127, 63, 0), new Color(63,63,127), new Color(0,63,0),
        new Color(127, 127, 0), new Color(127, 63, 63), new Color(0,0,100),
        new Color(127, 0, 0), new Color(127,255,127), Color.black
    };
    
    private byte[][] y = new byte[PIC_HEIGHT][PIC_WIDTH];
    private byte[][] u = new byte[PIC_HEIGHT][PIC_WIDTH];
    private byte[][] v = new byte[PIC_HEIGHT][PIC_WIDTH];
    private byte[] cal = new byte[MAX_VAL * MAX_VAL * MAX_VAL];
    private int[] cplane = new int[PIC_HEIGHT * PIC_WIDTH];
    
    private VisualObject[] vob = new VisualObject[NUM_OBJ];   // store all the visual objects

    // store a list of generated blobs
    private Vector blobList = new Vector();
    // the index of the blob that is displaying
    private int blobIndex = NO_SELECTED_BLOB;

    // the memory for determining the double click
    private long firstClickTime = 0; 
    private int lastBlobIndex  = NO_SELECTED_BLOB;
    
    private Blob currentBlob;

    private int scale;
    
    private ManualClassificationPanel parent;
    private boolean displayBlob;

    private Image image;
    private Graphics offscreen;
    
    private PrintWriter[] blobFile = new PrintWriter[Constant.BLOB_NAME_KEY.length];

    // constructor
    public PaintAreaPanel(ManualClassificationPanel parent, boolean displayBlob) {
        this.parent      = parent;
        this.displayBlob = displayBlob;
        
        initGUI();
        initBlobFile();
    }
    
    void initGUI() {
        if (displayBlob) {
            this.addMouseListener(new MouseInputAdapter() {
                public void mouseClicked(MouseEvent e) {
                    int index;
                    Blob blob;
                    boolean foundSanityObject = false;

                    // clear the current selected blob
                    currentBlob = null;

                    // find the first sanity object that contains the clicked point
                    for (int i=0; i<blobList.size(); i++) {
                        index = (i+blobIndex+1)%blobList.size();
                        blob  = (Blob) blobList.elementAt(index);
                        if (blob.contains(e.getX(), e.getY()) && blob.getPotentialObject()==parent.getMainGUI().getSanityObject()) {
                            blobIndex = index;
                            if (clickBlobDebugMsg) {
                                System.out.println("PaintAreaPanel.java: Select blob " + blobIndex);
                            }
                            currentBlob = blob;
                            foundSanityObject = true;
                            break;
                        }
                    }
                    // if still cannot find the sanity object, then find anything
                    if (!foundSanityObject) {
                        for (int i=0; i<blobList.size(); i++) {
                            index = (i+blobIndex+1)%blobList.size();
                            blob  = (Blob) blobList.elementAt(index);
                            if (blob.contains(e.getX(), e.getY())) {
                                blobIndex = index;
                                if (clickBlobDebugMsg) {
                                    System.out.println("PaintAreaPanel.java: Select blob " + blobIndex);
                                }
                                currentBlob = blob;
                                break;
                            }
                        }
                    }
                    
                    // no selected blob if no objects found
                    if (currentBlob!=null) {
                        long currentClickTime, clickInterval;
                        currentClickTime = System.currentTimeMillis();
                        clickInterval    = currentClickTime - firstClickTime;
                        // change the validity state when either it is the mouse button 3 or a double click
                        if (e.getButton()==MouseEvent.BUTTON3) {
                            if (parent.isChangeStateSelected()) {
                                changeBlobValidState();
                                if (!parent.isNextFrameSelected()) {
                                    repaint();
                                }
                            }
                            
                            if (parent.isWriteLogSelected()) {
                                writeLog();
                            }
                            
                            if (parent.isNextFrameSelected()) {
                                //parent.doNextFrame();
                                parent.doNextBallFrame();
                                // don't repaint if next frame selected to save effort
                                return;
                            }
                            
                        } else if (blobIndex==lastBlobIndex && clickInterval<DOUBLE_CLICK_INTERVAL) {
                            if (doubleClickDebugMsg) {
                                System.out.println("PainAreaPanel.java: Double click. Click Interval = " + clickInterval);
                            }
                            changeBlobValidState();
                        } else {
                            firstClickTime = currentClickTime;
                            if (doubleClickDebugMsg) {
                                System.out.println("PainAreaPanel.java: Single click");
                            }
                        }
                    } else {
                        blobIndex = NO_SELECTED_BLOB;
                        if (clickBlobDebugMsg) {
                            System.out.println("PaintAreaPanel.java: No blob selection. Point clicked: (" + e.getX() + "," + e.getY() + ")");
                        }
                        firstClickTime = System.currentTimeMillis();
                    }
                    lastBlobIndex = blobIndex;
                    repaint();
                }
            });
        }
    }
    
    void initBlobFile() {
        if (displayBlob) {
            // make the c4.5 data file
            String filename;
            for (int i=0; i<blobFile.length; i++) {
                filename = parent.getMainGUI().getProperty(Constant.BLOB_NAME_KEY[i], Constant.DEFAULT_BLOB_NAME[i], true) + 
                           File.separator + 
                           parent.getMainGUI().getProperty(Constant.BLOB_NAME_KEY[i], Constant.DEFAULT_BLOB_NAME[i]) + 
                           "." + Constant.C4_5_DATA_EXT;
                try {
                    // append to the data
                    blobFile[i] = new PrintWriter(new BufferedWriter(new FileWriter(filename, true)));
                } catch (IOException e) {
                    Utility.printError(this, "Error in write to " + filename);
                    e.printStackTrace();
                }
            }
        }
    }

    // bounding boxes
    public boolean readObjFile(String filename) {
        try {
            // clean the vobs
            for (int i = 0; i < vob.length; i++) {
                vob[i] = new VisualObject(0, 0, 0, 0, 0, 0, 0);
            }
            BufferedReader in = new BufferedReader(new FileReader(filename));
            String line = in.readLine();
            while (line != null) {
                try {
                    StringTokenizer st = new StringTokenizer(line);
                    if (!st.hasMoreTokens()) {
                        throw new NumberFormatException();
                    }
                    String o = st.nextToken();
                    if (!o.equals("o")) {
                        throw new NumberFormatException();
                    }
                    int vobId = Integer.parseInt(st.nextToken());
                    int x = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    int y = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    int h = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    int w = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    double d = Double.parseDouble(st.nextToken());
                    double head = Double.parseDouble(st.nextToken());
		            if (vobId == 0) {
	                    vob[vobId] = new VisualObject(x, y, h, w, d, head, Integer.parseInt(st.nextToken()));
		            } else {
	                    vob[vobId] = new VisualObject(x, y, h, w, d, head, 0);
                    }
                } catch (NumberFormatException ex) {
                    System.out.println("CAUGHT: " + line);
                }                
                line = in.readLine();
            }
        } catch (FileNotFoundException ex) {
            return false;
        } catch (IOException ex) {
            return false;
        }
        return true;
    }
    
    // read the blob file and store all blobs of a frame in a vector
    public boolean readBlobFile(String filename, boolean blobValid) {
        blobList.clear();
        try {
            BufferedReader in = new BufferedReader(new FileReader(filename));
            String line = "";
            int potentialObject, x, y, w, h;
            StringTokenizer st;
            Vector vector;
            while (true) {
                try {
                    vector = new Vector();
                    line   = in.readLine();
                    if (line==null) {
                        break;
                    }
                    st = new StringTokenizer(line);
                    potentialObject = Integer.parseInt(st.nextToken());
                    x  = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    y  = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    w  = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    h  = (int)(Math.round(Double.parseDouble(st.nextToken())));
                    while (st.hasMoreTokens()) {
                        vector.addElement(st.nextToken());
                    }
                    blobList.addElement(new Blob(potentialObject, x, y, h, w, blobValid, vector));
                } catch (NumberFormatException ex) {
                    System.out.println("CAUGHT: " + line);
                }
            }
            parent.setTotalBlobs(blobList.size());
        } catch (FileNotFoundException ex) {
            return false;
        } catch (IOException ex) {
            return false;
        }
        return true;
    }
    
    public boolean readCplFile(String filename) {
        try {
            BufferedReader in = new BufferedReader(new FileReader(filename));
            String line = in.readLine();
            StringTokenizer st = new StringTokenizer(line);
            int total = Integer.parseInt(st.nextToken());

            int k = 0;
            for (int i = 0; i < total/2; i++) {
                int runlength, color;
                runlength = Integer.parseInt(st.nextToken());
                color = Integer.parseInt(st.nextToken());
                for (int j = 0; j < (runlength & 0xff); j++) {
                    cplane[k++] = (color & 0xff);
                }
            }
            
            for (; k < cplane.length; k++) {
                cplane[k] = 9;
            }

        } catch (FileNotFoundException ex) {
            return false;
        } catch (IOException ex) {
            return false;
        }
        return true;
    }
    
    // paints the panel
    public void paintComponent(Graphics g) {
        image = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB);
        offscreen = image.getGraphics();

        scale = getWidth() / PIC_WIDTH;
        int k = 0;
        for(int i = 0; i < PIC_HEIGHT; i++) {
            for(int j = 0; j < PIC_WIDTH; j++) {
                drawClassifiedPixel(j * scale, i * scale, cplane[k++]);
            }
        }
        if (boxBehindPixelDebugMsg) {
            System.out.println("PaintAreaPanel.java: drawing pixels " + System.currentTimeMillis());
        }
        
        if (displayBlob) {
            paintBlob();
        } else {
            for (int i = 0; i < vob.length; i++) {
                if (vob[i]!=null && vob[i].height > 0 && vob[i].width > 0) {
                    boundingBox(vob[i], i);
                }
            }
        }
        
        // draw buffered image to screen
        g.drawImage(image, 0, 0, this);
    }
    
    // change the state of the validity of the blob
    public void changeBlobValidState() {
        currentBlob.setValid(!currentBlob.isValid());
        parent.setValid(currentBlob.isValid());
    }
    
    // set current pointer to the previous blob, false if cannot go to previous blob again
    public boolean setPrevBlob() {
        blobIndex--;
        currentBlob = (Blob) blobList.elementAt(blobIndex);
        if (blobIndex==0) {
            return false;
        }
        return true;
    }
    
    // set current pointer to the next blob, false if cannot go to next blob again
    public boolean setNextBlob() {
        blobIndex++;
        currentBlob = (Blob) blobList.elementAt(blobIndex);
        if (blobIndex==(blobList.size()-1)) {
            return false;
        }
        return true;
    }
    
    // reset blob index to the begining
    public void resetBlobIndex() {
        blobIndex = NO_SELECTED_BLOB;
    }
    
    public void writeLog() {
        int fileIndex;
        Blob blob;
        for (Enumeration e=blobList.elements(); e.hasMoreElements();) {
            blob      = (Blob) e.nextElement();
            fileIndex = Utility.objectToFile(blob.getPotentialObject());
            blobFile[fileIndex].println(blob);
            blobFile[fileIndex].flush();
        }
    }
    
    /**
     * @return true if the current blobList has a ball object
     */
    public boolean hasBall() {
        Blob blob;
        // go through all the blobs in the current frame
        for (Enumeration e=blobList.elements(); e.hasMoreElements();) {
            blob = (Blob) e.nextElement();
            if (blob.getPotentialObject()==parent.getMainGUI().getSanityObject()) {
                return true;
            }
        }
        return false;
    }
    
    /**************************************************************
     * private
     **************************************************************/

    // display the blob with index blobIndex
    private void paintBlob() {
        for (Enumeration e=blobList.elements(); e.hasMoreElements();) {
            ((Blob) e.nextElement()).drawBoundingBox();
        }
        if (blobIndex==NO_SELECTED_BLOB) {
            parent.noSelectedBlob();
            return;
        } else {
            parent.setValid(currentBlob.isValid());
            parent.getCurrentBlobField().setText("" + (blobIndex+1));
        }
        if (blobIndex>=0 && blobIndex<blobList.size()) {
            if (blobIndex==0) {
                parent.setPrevBlobButtonEnabled(false);
                parent.setNextBlobButtonEnabled(true);
            } else if (blobIndex==(blobList.size()-1)) {
                parent.setPrevBlobButtonEnabled(true);
                parent.setNextBlobButtonEnabled(false);
            } else {
                parent.setPrevBlobButtonEnabled(true);
                parent.setNextBlobButtonEnabled(true);
            }
            currentBlob.selectBlob();
        } else {
            System.out.println("PaintAreaPanel.java: invalid blob index: " + blobIndex + ". Cannot draw object");
        }
    }
    

    // draw the bounding box together with a label of the object
    private void boundingBox(VisualObject vobj, int i) {
        vobj.drawBoundingBox();
        if (i == 0) {
            if (vobj.type == 0) {
                offscreen.drawString("fire ball", Math.max(vobj.x, 0) * scale, Math.max(vobj.y, 0) * scale + 10);
            } else if (vobj.type == 1) {
                offscreen.drawString("projected ball", Math.max(vobj.x, 0) * scale, Math.max(vobj.y, 0) * scale + 10);
            } else {
                offscreen.drawString("normal ball", Math.max(vobj.x, 0) * scale, Math.max(vobj.y, 0) * scale + 10);
            }
        } else {
            offscreen.drawString(Constant.detailObjectNames[i], vobj.x * scale, vobj.y * scale + 10);
        }
    }
    
    // classifies a pixel
    private int classify(int cy, int cu, int cv) {
        return (int)(cal[cy * MAX_VAL * MAX_VAL + cu * MAX_VAL + cv]);
    }
    
    //draw a pixel square with classified colours
    private void drawClassifiedPixel(int px, int py, int c) {
        offscreen.setColor(classifyColours[c]);
        offscreen.fillRect(px, py, scale, scale);
    }
    
    private int yuv2rgb(int cy, int cu, int cv) {
        // converts from yuv to rgb
        cu-=128; cv-=128;
        int cr = (int)((1.164*cy) + (1.596*cu));
        int cg = (int)((1.164*cy) - (0.813*cu) - (0.391*cv));
        int cb = (int)((1.164*cy) + (1.596*cv));
        
        // I dont think this should be neccesary... but it is
        // did i make a mistake somewhere?
        if (cr < 0) cr = 0;       if (cg < 0) cg = 0;       if (cb < 0) cb = 0;
        if (cr > 255) cr = 255;   if (cg > 255) cg = 255;   if (cb > 255) cb = 255;

        return (int)((cr<<16) | (cg<<8) | cb);
    }

    private class VisualObject {
        public int x, y, height, width, cx, cy, type;
	public double d, head;

        public VisualObject (int x, int y, int h, int w, double d, double head, int type) {
            this.x = x;
            this.y = y;
            height = h;
            width  = w;
	    this.d = d;
	    this.head = head;
	    this.type = type;
            cx = x + (width / 2);
            cy = y + (height / 2);
        }

        void drawBoundingBox() {
            offscreen.setColor(Color.black);

            // draw the box
            offscreen.drawRect(this.x * scale, this.y * scale, this.width * scale, this.height * scale);

            // draw the lines passing through the centre of box
            offscreen.drawLine(this.cx * scale, this.y * scale, this.cx * scale, (this.y + this.height) * scale);
            offscreen.drawLine(this.x * scale, this.cy * scale, (this.x + this.width) * scale, this.cy * scale);
	    
	       // draw in d and h
            offscreen.drawString(("d: "+d), Math.max(x, 0) * scale, Math.max(y, 0) * scale + 20);
	        offscreen.drawString(("h: "+head), Math.max(x, 0) * scale, Math.max(y, 0) * scale + 30);
        }
    }

    private class Blob {
        public final static int SELECT_BLOB_SIDE = 5;
        public final static int SELECT_BLOB_SIDE_HALF = SELECT_BLOB_SIDE/2;
        public final static int SELECT_BLOB_GAP = 2;

        private int potentialObject, x, y, height, width, cx, cy;
        private Vector attributes;
        private boolean valid;

        public Blob(int potentialObject, int x, int y, int h, int w, boolean blobValid, Vector vector) {
            this.potentialObject = potentialObject;
            this.x               = x;
            this.y               = y;
            this.height          = h;
            this.width           = w;
            this.cx              = x + (width / 2);
            this.cy              = y + (height / 2);
            this.valid           = blobValid;
            this.attributes      = vector;
        }
        
        // return true iff this blob contains the given point
        boolean contains(int x, int y) {
            return (x>=this.x*scale && x<=(this.x+this.width)*scale && y>=this.y*scale && y<=(this.y+this.height)*scale);
        }
        
        void drawBoundingBox() {
            offscreen.setColor(Color.black);
            offscreen.setXORMode(Color.white);

            // draw the box
            if (this.valid) {
                offscreen.fillRect(this.x * scale, this.y * scale, this.width * scale, this.height * scale);
            } else {
                offscreen.drawRect(this.x * scale, this.y * scale, this.width * scale, this.height * scale);
            }

            // draw the lines passing through the centre of box
            offscreen.drawLine(this.cx * scale, this.y * scale, this.cx * scale, (this.y + this.height) * scale);
            offscreen.drawLine(this.x * scale, this.cy * scale, (this.x + this.width) * scale, this.cy * scale);

            // draw the object name at the top and bottom left corner
            if (this.y*scale<TOP_GAP) {
                offscreen.drawString(Constant.detailObjectNames[potentialObject], this.x*scale<0? 0:this.x*scale, TOP_GAP);
            } else {
                offscreen.drawString(Constant.detailObjectNames[potentialObject], this.x*scale<0? 0:this.x*scale, this.y*scale);
            }
            
            if (boxBehindPixelDebugMsg) {
                System.out.println("PaintAreaPanel.java: drawing blob " + System.currentTimeMillis());
            }
        }
        
        // select this blob, represent by having 8 small squares around the bounding box
        void selectBlob() {
            parent.setRadioButtonsEnabled();

            offscreen.setColor(Color.black);
            
            // top left
            offscreen.fillRect(this.x*scale-SELECT_BLOB_SIDE-SELECT_BLOB_GAP, this.y*scale-SELECT_BLOB_SIDE-SELECT_BLOB_GAP, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // top
            offscreen.fillRect(this.cx*scale-SELECT_BLOB_SIDE_HALF, this.y*scale-SELECT_BLOB_SIDE-SELECT_BLOB_GAP, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // top right
            offscreen.fillRect((this.x+this.width)*scale+SELECT_BLOB_GAP, this.y*scale-SELECT_BLOB_SIDE-SELECT_BLOB_GAP, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // right
            offscreen.fillRect((this.x+this.width)*scale+SELECT_BLOB_GAP, this.cy*scale-SELECT_BLOB_SIDE_HALF, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // bottom right
            offscreen.fillRect((this.x+this.width)*scale+SELECT_BLOB_GAP, (this.y+this.height)*scale+SELECT_BLOB_GAP, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // bottom
            offscreen.fillRect(this.cx*scale-SELECT_BLOB_SIDE_HALF, (this.y+this.height)*scale+SELECT_BLOB_GAP, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // bottom left
            offscreen.fillRect(this.x*scale-SELECT_BLOB_SIDE-SELECT_BLOB_GAP, (this.y+this.height)*scale+SELECT_BLOB_GAP, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
            
            // left
            offscreen.fillRect(this.x*scale-SELECT_BLOB_SIDE-SELECT_BLOB_GAP, this.cy*scale-SELECT_BLOB_SIDE_HALF, SELECT_BLOB_SIDE, SELECT_BLOB_SIDE);
        }
        
        int getPotentialObject() {
            return potentialObject;
        }
        
        boolean isValid() {
            return this.valid;
        }
        
        void setValid(boolean b) {
            this.valid = b;
        }
        
        public String toString() {
            String s = "";
            s += this.x + ", " + this.y + ", " + this.width + ", " + this.height + ", ";
            for (Enumeration e=attributes.elements(); e.hasMoreElements();) {
                s += (String) e.nextElement() + " ";
            }
            if (this.valid) {
                s += "Valid";
            } else {
                s += "Invalid";
            }
            return s;
        }
    }
}

