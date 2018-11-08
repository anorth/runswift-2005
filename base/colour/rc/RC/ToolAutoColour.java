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

import java.io.*;
import java.util.*;

import java.awt.*;
import java.awt.event.*;

import RoboComm.*;
import RoboShare.*;
import javax.swing.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

public class ToolAutoColour
    extends GenericTool {
    private static final boolean testHistogram = true;
    GridBagLayout gridBagLayout1 = new GridBagLayout();
    JSpinner spbThresholdY = new JSpinner();
    JLabel jLabel1 = new JLabel();
    JLabel jLabel2 = new JLabel();
    JLabel jLabel3 = new JLabel();
    JSpinner spbThresholdU = new JSpinner();
    JSpinner spbThresholdV = new JSpinner();

    int drawingMinX = -1;
    int drawingMaxX = -1;
    int drawingMinY = -1;
    int drawingMaxY = -1;

        public ToolAutoColour(RDRApplication parent) {
        super(parent);
        try {
            jbInit();
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private void jbInit() throws Exception {
        this.setLayout(gridBagLayout1);
        jLabel1.setText("Y Threshold");
        jLabel2.setText("U Threshold");
        jLabel3.setText("V Threshold");
        chbSample.setText("Sample/Draw");

        spbThresholdY.setValue(new Integer(200));
        spbThresholdU.setValue(new Integer(200));
        spbThresholdV.setValue(new Integer(200));
        this.add(spbThresholdY,  new GridBagConstraints(1, 1, 1, 1, 0.9, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        this.add(jLabel1,  new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        this.add(jLabel2,  new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        this.add(jLabel3,  new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        this.add(spbThresholdU,  new GridBagConstraints(1, 2, 1, 1, 0.9, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        this.add(spbThresholdV,  new GridBagConstraints(1, 3, 1, 1, 0.9, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(5, 5, 5, 5), 0, 0));
        this.add(chbSample,  new GridBagConstraints(0, 0, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
    }

    int getDiffY(int x,int y){
        return getDiffY( x,y, getBFL().getY(x, y) );
    }

    int getDiffY(int x,int y, int myY ){
        return Math.abs(myY - getBFL().getY(x-1, y)) +
            Math.abs(myY - getBFL().getY(x, y-1)) +
            Math.abs(myY - getBFL().getY(x+1, y)) +
            Math.abs(myY - getBFL().getY(x, y+1)) ;
    }

    int getDiffU(int x,int y){
        return getDiffU( x, y, getBFL().getU(x, y) );
    }

    int getDiffU(int x,int y, int myU ){

        return Math.abs(myU - getBFL().getU(x-1, y)) +
            Math.abs(myU - getBFL().getU(x, y-1)) +
            Math.abs(myU - getBFL().getU(x+1, y)) +
            Math.abs(myU - getBFL().getU(x, y+1)) ;
    }

    int getDiffV(int x,int y){
        return getDiffV(x,y,getBFL().getV(x, y));
    }

    int getDiffV(int x,int y, int myV){
        return Math.abs(myV - getBFL().getV(x-1, y)) +
            Math.abs(myV - getBFL().getV(x, y-1)) +
            Math.abs(myV - getBFL().getV(x+1, y)) +
            Math.abs(myV - getBFL().getV(x, y+1)) ;
    }

    void sampleRectangle(int startX, int startY, int endX, int endY) {
        if (startX > endX) {
            int tmp = startX;
            startX = endX;
            endX = tmp;
        }
        if (startY > endY) {
            int tmp = startY;
            startY = endY;
            endY = tmp;
        }

        System.out.println("Sampling rectange " + startX + "   " + startY +
                           "  " + endX + "  " + endY);

        for (int x = startX+1; x < endX; x++)
            for (int y = startY+1; y < endY; y++) {
                int YY = getDiffY(x,y) ;
                int UU = getDiffU(x, y);
                int VV = getDiffV(x, y);
                int C = getBFL().getC(x, y);
                System.out.println("" + YY + "\t" + UU + "\t" + VV + "\t" + C);
                parent.bflPanel.drawColourBox(x, y);
            }

        drawingMinX = startX;
        drawingMaxX = endX;
        drawingMinY = startY;
        drawingMaxY = endY;
        parent.repaint();
    }

    void autoColour(int startX, int startY, int endX, int endY) {
        if (startX > endX) {
            int tmp = startX;
            startX = endX;
            endX = tmp;
        }
        if (startY > endY) {
            int tmp = startY;
            startY = endY;
            endY = tmp;
        }
        int cX = (startX + endX) / 2;
        int cY = (startY + endY) / 2;
        if (getBFL().outOfBound(cX,cY)) return;

        System.out.println("auto colour rectange " + startX + "   " + startY +
                           "  " + endX + "  " + endY +
                           "\n   Center : " + cX + " " + cY );

        int myY = getBFL().getY(cX, cY);
        int myU = getBFL().getU(cX, cY);
        int myV = getBFL().getV(cX, cY);

        for (int x = startX+1; x < endX; x++)
            for (int y = startY+1; y < endY; y++) {
                int YY = getDiffY(x,y, myY) ;
                int UU = getDiffU(x, y, myU);
                int VV = getDiffV(x, y, myV);
                int C = getBFL().getC(x, y);
                if (YY < ((Integer)spbThresholdY.getValue()).intValue() &&
                    UU < ((Integer)spbThresholdU.getValue()).intValue() &&
                    VV < ((Integer)spbThresholdV.getValue()).intValue() )
                    parent.bflPanel.drawColourBox(x, y);
                else
                    parent.bflPanel.drawBox(x, y);

            }

        drawingMinX = startX;
        drawingMaxX = endX;
        drawingMinY = startY;
        drawingMaxY = endY;
    }


    public void mouseReleased(MouseEvent e) {
        int x = parent.bflPanel.getPosX(e.getX());
        int y = parent.bflPanel.getPosY(e.getY());
        if (testHistogram && startX != -1 && startY != -1 &&
            Math.abs(x - startX) + Math.abs(y - startY) > 20) {
            if (chbSample.isSelected())
                sampleRectangle(startX, startY, x, y);
            else
                autoColour(startX, startY, x, y);
            parent.repaint();
            startX = -1;
            startY = -1;
        }
    }

    int startX = -1;
    int startY = -1;
    JCheckBox chbSample = new JCheckBox();

    public void mouseDragged(MouseEvent e) {
        if (startX == -1 && startY == -1) {
            startX = parent.bflPanel.getPosX(e.getX());
            startY = parent.bflPanel.getPosY(e.getY());

        }
//        else{
//            int x = parent.bflPanel.getPosX(e.getX());
//            int y = parent.bflPanel.getPosY(e.getY());
//            parent.bflPanel.drawBoundingBoxDirect(startX,startY,x,y);
//        }
    }

    public void paintBFLPanel(Graphics g) {
        if (!enabled) return;
        if (chbSample.isSelected()){
            drawingRectangle(drawingMinX, drawingMinY, drawingMaxX, drawingMaxY);
        }
        else{
            autoColour(drawingMinX, drawingMinY, drawingMaxX, drawingMaxY);
        }

    }

    private void drawingRectangle(int minX, int minY, int maxX, int maxY) {
        for (int x = minX; x <= maxX; x++)
            for (int y = minY; y <= maxY; y++) {
                parent.bflPanel.drawColourBox(x, y);
            }
    }
	public String name(){
		return "Auto Colour";
	}
}
