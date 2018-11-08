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
 * $Id: ImageDisplay.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Display image of size 176 x 144 pixels
 * Image format: r - red, b - blue, else - white
 *
**/

import java.io.*;
import java.awt.*;


public class ImageDisplay extends Canvas {

    //--- constants ---------------------------
    public static int numRows  = 160;
    public static int numCols  = 208;
    public static int gridSize = 3;

    //--- fields ------------------------------
    char[][] image;
    int      myHeight = numRows * gridSize;
    int      myWidth  = numCols * gridSize;
    
    //--- bounding box ------------------------
    int      boundBoxMinX = -1;
    int      boundBoxMinY = -1;
    int      boundBoxMaxX = -1;
    int      boundBoxMaxY = -1;
    
    
    
    /**
     * Initialise display to blank
     */
    public ImageDisplay() {
        image      = new char[numCols][numRows];
        repaint();
    }



    /**
     * display image specified by path
     */
    public void loadImageFile(String filepath) {
    
        try {
            BufferedReader reader = new BufferedReader(new FileReader(filepath));
            for (int row=0; row<numRows; row++) {
                String line = reader.readLine();
                for (int col=0; col<line.length(); col++)
                    image[col][row] = line.charAt(col);
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        
        repaint();
    }



    /**
     * display image specified by double array
     */
    public void setImage(char[][] image) {
        this.image = image;
        repaint();
    }
    
    
    
    /**
     * Specify bounding box to display
     * (-1,-1,-1,-1) display no box
     */
    public void setBoundingBox(int minX, int minY, int maxX, int maxY) {
        boundBoxMinX = minX;
        boundBoxMinY = minY;
        boundBoxMaxX = maxX;
        boundBoxMaxY = maxY;
    }



    /**
     * paint image on screen
     */
    public void paint(Graphics g) {
    
        g.setColor(Color.white);
        g.fillRect(0,0,myWidth,myHeight);
        
        int gridWidth  = myWidth / numCols;
        int gridHeight = myHeight / numRows;
        
        for (int row=0; row < numRows; row++) {
            for (int col=0; col < numCols; col++) {
                if (Character.toLowerCase(image[col][row]) == 'r') {
                    g.setColor(Color.red);
                    g.fillRect(col*gridWidth, row*gridHeight, gridWidth, gridHeight);
                } else if (Character.toLowerCase(image[col][row]) == 'b') {
                    g.setColor(Color.blue);
                    g.fillRect(col*gridWidth, row*gridHeight, gridWidth, gridHeight);
                }
            }
        }
        
        if (boundBoxMinX != -1) {
            g.setColor(Color.black);
            g.drawRect(boundBoxMinX*gridWidth, boundBoxMinY*gridHeight,
                        (boundBoxMaxX-boundBoxMinX)*gridSize, (boundBoxMaxY-boundBoxMinY)*gridSize);
        }
    }
    
    
    
    /**
     * get image array
     */
    public char[][] getImage() {
        return image;
    }

}
