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
 * $Id: TrainingData.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Training data
 *
**/

import java.awt.*;
import javax.swing.*;


public class TrainingData {

    //--- constants ---------------------------
    public static String[] classifications = {"0", "45","90","135","180","-135","-90","-45"};
    public static String[] attributeNames  = {"colour", "boundBoxHeight", "boundBoxRatio", "largestPatchRatio", "leftDensity", "rightDensity","backSlashDensity","forwardSlashDensity"};
    public static String[] attributeValues = {"red, blue", "continuous", "continuous", "continuous", "continuous", "continuous", "continuous", "continuous"};


    //--- fields ------------------------------
    public int      classi;
    public String[] attributes;
    
    
    
    /**
     * Screen output representation (classification.index representation)
     */
    public String toString() {
        String result = "";
        for (int i=0; i<attributes.length; i++)
            result += (" " + attributes[i]);
        result += (" " + classi);
        return result;
    }
    
    
    
    /**
     * Orient.data representation
     */
    public String toDataString() {
        String result = "";
        for (int i=0; i<attributes.length; i++) {
            if (i==0)
                result += attributes[i];
            else
                result += (", " + attributes[i]);
        }
        result += (", " + classi);
        return result;
    }
    
    
    
    
    /**
     * Calculate attribute values
     */
    public void calAttr(char[][] image) {
    
        //--- init -----------------------------------
        attributes = new String[attributeNames.length];
        initScan(image);
        firstStatisticScan(image);
        secondStatisticScan(image);
        
        
        //=== call feature extraction methods ========
        
        //--- 1) Colour ------------------------------
        if (isRed())
            attributes[0] = "red";
        else
            attributes[0] = "blue";
            
        //--- 2) bounding box height -----------------
        attributes[1] = String.valueOf(boundBoxHeight());
        
        //--- 3) bounding box ratio ------------------
        attributes[2] = String.valueOf(boundBoxRatio());
        
        //--- 4) largest patch ratio -----------------
        attributes[3] = String.valueOf(largestPatchRatio());
        
        //--- 5) left density ------------------------
        attributes[4] = String.valueOf(leftDensity());
        
        //--- 6) right density -----------------------
        attributes[5] = String.valueOf(rightDensity());
        
        //--- 7) back slash density ------------------
        attributes[6] = String.valueOf(backSlashDensity());
        
        //--- 8) forward slash density ---------------
        attributes[7] = String.valueOf(forwardSlashDensity());
        
        
        //============================================
        // DEBUG
        // showImage(image);
        // System.out.println(toDataString());    
        //============================================    
    }
    
    
    
    
    
    /////////////////////////////////////////////////////////////
    // Preparation for feature extraction
    /////////////////////////////////////////////////////////////
    
    
    //--- private constants ------------------------
    private int patchThres = 15;
    
    
    //--- private variables ------------------------    
    private int numRedTotal       = 0;
    private int numBlueTotal      = 0;

    private int numTopLeft     = 0;
    private int numTopRight    = 0;
    private int numBottomLeft  = 0;
    private int numBottomRight = 0;
    
    private int numTopLftThird = 0;
    private int numMidLftThird = 0;
    private int numBomLftThird = 0;
    private int numTopMidThird = 0;
    private int numMidMidThird = 0;
    private int numBomMidThird = 0;
    private int numTopRhtThird = 0;
    private int numMidRhtThird = 0;
    private int numBomRhtThird = 0;
    
    private int blueBoundBoxMinX = 1000;
    private int blueBoundBoxMinY = 1000;
    private int blueBoundBoxMaxX = -1;
    private int blueBoundBoxMaxY = -1;
    
    private int redBoundBoxMinX  = 1000;
    private int redBoundBoxMinY  = 1000;
    private int redBoundBoxMaxX  = -1;
    private int redBoundBoxMaxY  = -1;
    
    private int boundBoxMinX = 1000;
    private int boundBoxMinY = 1000;
    private int boundBoxMaxX = -1;
    private int boundBoxMaxY = -1;
    
    private int largestPatchMinX = 1000;
    private int largestPatchMinY = 1000;
    private int largestPatchMaxX = -1;
    private int largestPatchMaxY = -1;
    
    private int tempMinX = 1000;
    private int tempMinY = 1000;
    private int tempMaxX = -1;
    private int tempMaxY = -1;
    
    private int largestPatchSize = 0;
    
    
    
    
    
    //==========================================================
    // Noise removal
    //==========================================================
    
    /**
     * Remove noise from image
     * also work out largest patch bounding box
     */
    private void initScan(char[][] image) {
    
        for (int row=0; row<ImageDisplay.numRows; row++) {
            for (int col=0; col<ImageDisplay.numCols; col++) {

                if (image[col][row] == 'b') {
                    int numPixel = countPixel(image,'b',row,col);
                    if (numPixel >= patchThres) {
                        savePixel(image,'B',row,col);
                        if (numPixel > largestPatchSize) {
                            largestPatchSize = numPixel;
                            largestPatchMinX = tempMinX;
                            largestPatchMaxX = tempMaxX;
                            largestPatchMinY = tempMinY;
                            largestPatchMaxY = tempMaxY;
                        }
                    }
                    tempMinX = 1000;
                    tempMinY = 1000;
                    tempMaxX = -1;
                    tempMaxY = -1;
                } else if (image[col][row] == 'r') {
                    int numPixel = countPixel(image,'r',row,col);
                    if (numPixel >= patchThres) {
                        savePixel(image,'R',row,col);
                        if (numPixel > largestPatchSize) {
                            largestPatchSize = numPixel;
                            largestPatchMinX = tempMinX;
                            largestPatchMaxX = tempMaxX;
                            largestPatchMinY = tempMinY;
                            largestPatchMaxY = tempMaxY;    
                        }
                    }
                    tempMinX = 1000;
                    tempMinY = 1000;
                    tempMaxX = -1;
                    tempMaxY = -1;
                }
                
            }
        }
    }
    
    
    
    /**
     * Count neighbour pixels of the specified colour
     */
    private int countPixel(char[][] image, char colour, int row, int col) {
        
        if (colour == 'b')
            image[col][row] = 'x';
        else
            image[col][row] = 'y';
        
        // large patch bound box
        if (row<tempMinY)
            tempMinY = row;
        if (row>tempMaxY)
            tempMaxY = row;
        if (col<tempMinX)
            tempMinX = col;
        if (col>tempMaxX)
            tempMaxX = col;
        
        //count neighbours
        int count = 1;
        if (row>0 && image[col][row-1]==colour)
            count += countPixel(image,colour,row-1,col);
        if (row<ImageDisplay.numRows-1 && image[col][row+1]==colour)
            count += countPixel(image,colour,row+1,col);
        if (col>0 && image[col-1][row]==colour)
            count += countPixel(image,colour,row,col-1);
        if (col<ImageDisplay.numCols-1 && image[col+1][row]==colour)
            count += countPixel(image,colour,row,col+1);
    
        return count;
    }
    
    
    
    /**
     * Save regions of size no smaller than patchThres
     */
    private void savePixel(char[][] image, char colour, int row, int col) {
        
        image[col][row] = colour;
        
        if (colour == 'B') {
            if (row>0 && image[col][row-1]=='x')
                savePixel(image,colour,row-1,col);
            if (row<ImageDisplay.numRows-1 && image[col][row+1]=='x')
                savePixel(image,colour,row+1,col);
            if (col>0 && image[col-1][row]=='x')
                savePixel(image,colour,row,col-1);
            if (col<ImageDisplay.numCols-1 && image[col+1][row]=='x')
                savePixel(image,colour,row,col+1);
        } else {
            if (row>0 && image[col][row-1]=='y')
                savePixel(image,colour,row-1,col);
            if (row<ImageDisplay.numRows-1 && image[col][row+1]=='y')
                savePixel(image,colour,row+1,col);
            if (col>0 && image[col-1][row]=='y')
                savePixel(image,colour,row,col-1);
            if (col<ImageDisplay.numCols-1 && image[col+1][row]=='y')
                savePixel(image,colour,row,col+1);
        }
    }
    
    
    
    
    
    //================================================================
    // Collecting statistics
    //================================================================
    
    /**
     * Count total num of red / blue pixel to work out the colour of robot
     * also work out the robot bounding box
     */
    private void firstStatisticScan(char[][] image) {
    
        for (int row=0; row<ImageDisplay.numRows; row++) {
            for (int col=0; col<ImageDisplay.numCols; col++) {
            
                //=== count blues ------------------------------------
                if (image[col][row] == 'B') {
                
                    //--- total pixel ---
                    numBlueTotal++;
                    
                    //--- bounding box ---
                    if (row<blueBoundBoxMinY)
                        blueBoundBoxMinY = row;
                    if (row>blueBoundBoxMaxY)
                        blueBoundBoxMaxY = row;
                    if (col<blueBoundBoxMinX)
                        blueBoundBoxMinX = col;
                    if (col>blueBoundBoxMaxX)
                        blueBoundBoxMaxX = col;
                }
                
                
                //=== count red --------------------------------------
                else if (image[col][row] == 'R') {
                
                    //--- total pixel ---
                    numRedTotal++;               
                    
                    //--- bounding box ---
                    if (row<redBoundBoxMinY)
                        redBoundBoxMinY = row;
                    if (row>redBoundBoxMaxY)
                        redBoundBoxMaxY = row;
                    if (col<redBoundBoxMinX)
                        redBoundBoxMinX = col;
                    if (col>redBoundBoxMaxX)
                        redBoundBoxMaxX = col;
                }
                
            }
        }
        
        
        //--- robot bounding box --------------------------------
        if (numRedTotal > numBlueTotal) {
            boundBoxMinX = redBoundBoxMinX;
            boundBoxMaxX = redBoundBoxMaxX;
            boundBoxMinY = redBoundBoxMinY;
            boundBoxMaxY = redBoundBoxMaxY;
        } else {
            boundBoxMinX = blueBoundBoxMinX;
            boundBoxMaxX = blueBoundBoxMaxX;
            boundBoxMinY = blueBoundBoxMinY;
            boundBoxMaxY = blueBoundBoxMaxY;
        }
    }
    
    
    
    
    /**
     * Counting pixels in regions of bounding box for density calculations
     */
    private void secondStatisticScan(char[][] image) {
    
        int  boundBoxWidth  = boundBoxMaxX - boundBoxMinX;
        int  boundBoxHeight = boundBoxMaxY - boundBoxMinY;
        char pColor         = isRed() ? 'R' : 'B';
    
        for (int row=boundBoxMinY; row<=boundBoxMaxY; row++) {
            for (int col=boundBoxMinX; col<=boundBoxMaxX; col++) {
            
                if (image[col][row] == pColor) {
                    //--- pixel in four regions ---
                    if (col<((boundBoxWidth+1)/2)+boundBoxMinX) {  //-- left
                        if (row<((boundBoxHeight+1)/2)+boundBoxMinY) { //-- top
                            numTopLeft++;
                        } else {                                       //-- bottom
                            numBottomLeft++;
                        }
                    } else {                                       //-- right
                        if (row<((boundBoxHeight+1)/2)+boundBoxMinY) { //-- top
                            numTopRight++;
                        } else {                                       //-- bottom
                            numBottomRight++;
                        }
                    }


                    //--- pixel in nine regions ---
                    if (col<((boundBoxWidth+1)/3)+boundBoxMinX) {          //-- left
                        if (row<((boundBoxHeight+1)/3)+boundBoxMinY) { 
                            numTopLftThird++;
                        } else if (row>((boundBoxHeight)/3 * 2)+boundBoxMinY) {
                            numBomLftThird++;
                        } else {
                            numMidLftThird++;
                        }
                    } else if (col>((boundBoxWidth)/3 * 2)+boundBoxMinX) { //-- right
                        if (row<((boundBoxHeight+1)/3)+boundBoxMinY) { 
                            numTopRhtThird++;
                        } else if (row>((boundBoxHeight)/3 * 2)+boundBoxMinY) {
                            numBomRhtThird++;
                        } else {
                            numMidRhtThird++;
                        }
                    } else {                                       //-- mid
                        if (row<((boundBoxHeight+1)/3)+boundBoxMinY) { 
                            numTopMidThird++;
                        } else if (row>((boundBoxHeight)/3 * 2)+boundBoxMinY) {
                            numBomMidThird++;
                        } else {
                            numMidMidThird++;
                        }
                    }
                }
            }
        }
    }
    
    
    
    
    
    /////////////////////////////////////////////////////////////
    // Feature extraction methods starts here
    /////////////////////////////////////////////////////////////
    
    /**
     * True if red pixel is majority
     */
    private boolean isRed() {
        return (numRedTotal>numBlueTotal);
    }
    
    
    
    /**
     * height of bounding box
     */
    private int boundBoxHeight() {
        return (boundBoxMaxY - boundBoxMinY);
    }
    
    
    
    /**
     * ratio of bounding box width / height
     * (width / height) * 100
     */
    private int boundBoxRatio() {
    
        int width  = boundBoxMaxX - boundBoxMinX;
        int height = boundBoxMaxY - boundBoxMinY;
        
        return (int)((((double)width)/height) * 100);
    }
    
    
    
    /**
     * ratio of largest patch bounding box width / height
     */
    private int largestPatchRatio() {
    
        int width  = largestPatchMaxX - largestPatchMinX;
        int height = largestPatchMaxY - largestPatchMinY;
        
        return (int)((((double)width)/height) * 100);
    }
    
    
    
    /**
     * density of left half of robot bounding box
     */
    private int leftDensity() {
    
        int width  = (boundBoxMaxX - boundBoxMinX + 1)/2;
        int height = (boundBoxMaxY - boundBoxMinY + 1)/2;
    
        int colourPixels = numTopLeft + numBottomLeft;
        int totalPixels  = width * height;
        
        return (int)((((double)colourPixels)/totalPixels) * 100);
    }
    
    
    
    /**
     * density of right half of robot bounding box
     */
    private int rightDensity() {
    
        int width  = (boundBoxMaxX - boundBoxMinX + 1)/2;
        int height = (boundBoxMaxY - boundBoxMinY + 1)/2;
    
        int colourPixels = numTopRight + numBottomRight;
        int totalPixels  = width * height;
        
        return (int)((((double)colourPixels)/totalPixels) * 100);
    }
    
    
    
    /**
     * density of \ diagonal of robot bounding box
     */
    private int backSlashDensity() {
    
        int gridWidth  = (boundBoxMaxX - boundBoxMinX + 1)/3;
        int gridHeight = (boundBoxMaxY - boundBoxMinY + 1)/3;
    
        int colourPixels = numTopLftThird + numMidMidThird +
                           numMidRhtThird + numBomMidThird +
                           numBomRhtThird;
        int totalPixels  = gridWidth * gridHeight * 5;
        
        return (int)((((double)colourPixels)/totalPixels)*100);
    }
    
    
    
    /**
     * density of / diagonal of robot bounding box
     */
    private int forwardSlashDensity() {
    
        int gridWidth  = (boundBoxMaxX - boundBoxMinX + 1)/3;
        int gridHeight = (boundBoxMaxY - boundBoxMinY + 1)/3;
    
        int colourPixels = numTopRhtThird + numMidLftThird +
                           numMidMidThird + numBomLftThird +
                           numBomMidThird;
        int totalPixels  = gridWidth * gridHeight * 5;
           
        return (int)((((double)colourPixels)/totalPixels)*100);
    }
    
    
    
    
    
    
    /////////////////////////////////////////////////////////////
    // Debug methods
    /////////////////////////////////////////////////////////////
    
    /**
     * print text image on command line
     */
    private void printImage(char[][] image) {
        for (int row=0; row<ImageDisplay.numRows; row++) {
            for (int col=0; col<ImageDisplay.numCols; col++) {
                System.out.print(image[col][row]);
            }
            System.out.println();
        }
    }
    
    
    /**
     * print graphic image in pop up dialog
     */
    private void showImage(char[][] image) {
    
        char[][] cleanImage = new char[ImageDisplay.numCols][ImageDisplay.numRows];
        for (int row=0; row<ImageDisplay.numRows; row++) {
            for (int col=0; col<ImageDisplay.numCols; col++) {
                cleanImage[col][row] = Character.toLowerCase(image[col][row]);
            }
        }
    
        ImageDisplay imgDis = new ImageDisplay();
        //imgDis.setBoundingBox(boundBoxMinX, boundBoxMinY, boundBoxMaxX, boundBoxMaxY);
        imgDis.setBoundingBox(largestPatchMinX, largestPatchMinY, largestPatchMaxX, largestPatchMaxY);
        imgDis.setImage(cleanImage);
        
        JFrame frame = new JFrame();
        frame.getContentPane().add(imgDis);
        frame.setSize(new Dimension(ImageDisplay.numCols*3, ImageDisplay.numRows*3));
        frame.setVisible(true);
    }
}
