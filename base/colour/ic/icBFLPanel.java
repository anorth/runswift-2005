/*

Copyright 2004 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.*;

public class icBFLPanel extends icBasicBFLPanel implements MouseListener, MouseMotionListener
{
    protected static final int PIC_WIDTH   = icConstant.PIC_WIDTH;
    protected static final int PIC_HEIGHT  = icConstant.PIC_HEIGHT;

    protected int      currentTool   = icConstant.TOOL_BRUSH;
    protected boolean  painting      = false;
    public    boolean  canUndo       = false;
    public    boolean  canClear      = true;

    //the start points for the line tool
    protected int      lineX         = -1;
    protected int      lineY         = -1;

    protected int      brushSize     = 0;
    protected int      wandThreshold = 25;

    public icBFLPanel()
    {
	super();
	addMouseListener(this);
	addMouseMotionListener(this);
    }

    public icBFLPanel(String fileName)
    {
	this();
	readFile(fileName);
    }

    /**
     * Highlights using the current tool
     */
    void highlight(int x, int y, boolean painting)
    {
	if(fileName == null) {
	    return;
	}

	if (x < 0 || x >= PIC_WIDTH || y < 0 || y >= PIC_HEIGHT)
	    return;

	if(currentTool == icConstant.TOOL_ZOOM) {
	    zoom(x, y, painting);
	    return;
	}

	if(icConstant.isPlane(color) || 
	   (painting && color == icConstant.COLOR_ALL)) {
	    //jTextArea1.setText("Please select a color to classify first!")
	    return;
	}

	switch(currentTool)
	    {
	    case icConstant.TOOL_BRUSH:
		brush(x, y, painting);
		break;
	    case icConstant.TOOL_LINE:
		line(x, y, painting);
		break;
	    case icConstant.TOOL_FILL:
		fill(x, y, painting);
		break;
	    case icConstant.TOOL_WAND:
		wand(x, y, painting, new Color(rgbPlane[x][y]), wandThreshold);
		break;
	    }
	if(!canClear) {
	    canClear = true;
	    firePropertyChange(""+icConstant.TOOL_CLEAR, false, canClear);
	}
    }

    /**
     * higlights a single pixel (x,y) and sets the Cplane value
     * if painting is true we are highligting
     * otherwise we are unhighlighting
     */
    void drawPixel(int x, int y, boolean painting)
    {
	//check bounds of x and y
	if (!(0 <= x && x < PIC_WIDTH && 0 <= y && y < PIC_HEIGHT))
	    return;

	if (painting) {
	    //not allowed to draw over already classified pixels
	    if (C[x][y] != icConstant.COLOR_NONE || 
		color   == icConstant.COLOR_ALL  || icConstant.isPlane(color))
		return;

	    C[x][y] = color;
	    drawBox(x, y, getGraphics());
	    }
	else { //erasing
	    //not allowed to erase wrong color unless in "all" view
	    if (C[x][y] != color && color != icConstant.COLOR_ALL)
		return;

	    C[x][y] = icConstant.COLOR_NONE;
	    drawSquare(x, y, getGraphics());
	}
    }

    /**
     * Draw a square the with brushSize pixels around (x,y)
     */
    public void brush(int x, int y, boolean painting)
    {
	for (int i = x - brushSize; i <= x + brushSize; i++)
	    for (int j = y - brushSize; j <= y + brushSize; j++)
		drawPixel(i, j, painting);
    }

    /**
     * Draw a line between (lineX,lineY) and (x,y)
     */
    public void line(int x, int y, boolean painting)
    {
	//initialise the starting point of the line
	if (painting) {
	    lineX = x;
	    lineY = y;
	    return;
	}
	//else

	//if a start point hasn't been selected, return
	if (lineX == -1)
	    return;

	//check whether the line should be based on X or Y
	if (Math.abs(y - lineY) < Math.abs(x - lineX)) {

	    double  m  = ((double) y - lineY) / ((double) x - lineX);
	    double  b  = lineY - m * lineX;

	    if (x > lineX) {
		int  temp = x;
		x = lineX;
		lineX = temp;
	    }

	    for (double i = x; i <= lineX + 1; i += .1) {
		int  c  = (int) i;
		int  d  = (int) (m * i + b);
		drawPixel(c, d, true);
	    }
	}
	else {
	    double  m  = ((double) x - lineX) / ((double) y - lineY);
	    double  b  = lineX - m * lineY;

	    if (y > lineY) {
		int  temp = y;
		y = lineY;
		lineY = temp;
	    }

	    for (double i = y; i <= lineY + 1; i += .1) {
		int  c  = (int) i;
		int  d  = (int) (m * i + b);
		drawPixel(d, c, true);
	    }
	}

	//reset the starting points (-1 = no starting point selected)
	lineX = -1;
	lineY = -1;
    }

    /**
     * Select a region using the magic wand starting at (x,y)
     * painting is used to determine whether to select or deselect
     */
    public void wand(int x, int y, boolean painting, 
		     Color startc, int threshold)
    {
	// stack overflow occurs sometimes
	if ((painting && C[x][y] != icConstant.COLOR_NONE) ||
	    (!painting && C[x][y] != color))
	    return;

	Color curc = new Color(rgbPlane[x][y]);

	//if current pixel is close enough to current one
	if ((Math.abs(curc.getRed()   - startc.getRed())   < threshold) &&
	    (Math.abs(curc.getGreen() - startc.getGreen()) < threshold) &&
	    (Math.abs(curc.getBlue()  - startc.getBlue())  < threshold)) {

	    drawPixel(x, y, painting);

	    //recurse over the four neighbours
	    if(x + 1 < PIC_WIDTH)
		wand(x + 1, y, painting, startc, threshold); //right

	    if(y + 1 < PIC_HEIGHT)
		wand(x, y + 1, painting, startc, threshold); //down

	    if(x - 1 >= 0)
		wand(x - 1, y, painting, startc, threshold); //left

	    if(y - 1 >= 0)
		wand(x, y - 1, painting, startc, threshold); //up
	}
    }

    public void fill(int x, int y, boolean painting)
    {
	// stack overflow occurs sometimes
	if ((painting && C[x][y] != icConstant.COLOR_NONE) ||
	    (!painting && C[x][y] != color))
	    return;

	//draw current pixel
	drawPixel(x, y, painting);

	//recurse over the four neighbours
	if(x + 1 < PIC_WIDTH)
	    fill(x + 1, y, painting); //right

	if(y + 1 < PIC_HEIGHT)
	    fill(x, y + 1, painting); //down

	if(x - 1 >= 0)
	    fill(x - 1, y, painting); //left

	if(y - 1 >= 0)
	   fill(x, y - 1, painting); //up
    }

    public void zoom(int x, int y, boolean painting)
    {
	if (painting)
	    setQuadrant(x, y);
	else
	    setQuadrant(-1, -1);
	display();
    }

    /**
     * Set the quadrant to zoom on (0 is all quadrants)
     */
    public void setQuadrant(int x, int y)
    {
	if (x == -1 && y == -1) {
	    quadrant = 0;
	    STARTX = 0;
	    STARTY = 0;
	    ENDX = PIC_WIDTH;
	    ENDY = PIC_HEIGHT;
	}
	else {
	    quadrant = 1;
	    STARTX = Math.max(x - PIC_WIDTH / 4, 0);
	    STARTY = Math.max(y - PIC_HEIGHT / 4, 0);
	    ENDX = STARTX + PIC_WIDTH / 2;
	    ENDY = STARTY + PIC_HEIGHT / 2;

	    //correct out of bounds zooming
	    if (ENDX > PIC_WIDTH) {
		STARTX = PIC_WIDTH / 2;
		ENDX = PIC_WIDTH;
	    }
	    if (ENDY > PIC_HEIGHT) {
		STARTY = PIC_HEIGHT / 2;
		ENDY = PIC_HEIGHT;
	    }
	}
    }

    /**
     * clears all layers of any classifcation information
     */
    public void clear()
    {
	saveUndo();
	canClear = false;

	for (int j = 0; j < PIC_WIDTH; j++)
	    for (int i = 0; i < PIC_HEIGHT; i++)
		C[j][i] = icConstant.COLOR_NONE;
	display();
    }

    public void saveUndo()
    {
	for (int h = 0; h < PIC_HEIGHT; h ++)
	    for (int w = 0; w < PIC_WIDTH; w ++)
		prevC[w][h] = C[w][h];

	if(!canUndo) {
	    canUndo = true;
	    firePropertyChange(""+icConstant.TOOL_UNDO, false, canUndo);
	}
    }

    public void undo()
    {
	canUndo = false;
	canClear = true;

	for (int h = 0; h < PIC_HEIGHT; h ++)
	    for (int w = 0; w < PIC_WIDTH; w ++)
		C[w][h] = prevC[w][h];
	display();
    }

    public void setBrushSize(int size)
    {
	brushSize = size;
    }

    public void setWandThreshold(int threshold)
    {
	wandThreshold = threshold;
    }

    /**
     * convert a screen coordinate to picture relative
     */
    public int convertX(int x)
    {
	//divide by scale and add offset
	return x / SCALE + STARTX;
    }

    /**
     * convert a screen coordinate to picture relative
     */
    public int convertY(int y)
    {
	//divide by scale and add offset
	return y / SCALE + STARTY;
    }

    public void mousePressed(MouseEvent e)
    { 
	switch(currentTool)
	    {
	    case icConstant.TOOL_BRUSH: case icConstant.TOOL_LINE: 
	    case icConstant.TOOL_FILL:  case icConstant.TOOL_WAND:
		saveUndo();
		break;
	    }
    }

    public void mouseClicked(MouseEvent e)
    {
	highlight(convertX(e.getX()), convertY(e.getY()), 
		  !SwingUtilities.isRightMouseButton(e));
    }

    public void mouseDragged(MouseEvent e)
    {
	mouseClicked(e);
    }

    //Unused MouseEvents
    public void mouseMoved(MouseEvent e)
    { }
    public void mouseReleased(MouseEvent e)
    { }
    public void mouseEntered(MouseEvent e)
    { }
    public void mouseExited(MouseEvent e)
    { }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java icBFLPanel <file>");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("icBFLPanel");
	    frame.setSize(550, 480);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	    icBFLPanel bflPane = new icBFLPanel();
	    frame.getContentPane().add(bflPane);
	    frame.setVisible(true);

	    if(args.length == 1)
		bflPane.readFile(args[0]);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
