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
import javax.swing.*;
import java.io.*;

/**
 * Display a BFL file in RGB colours
 */
public class BFLViewer extends JPanel
{
    protected static final int PIC_WIDTH   = 208;
    protected static final int PIC_HEIGHT  = 160;

    protected static Image     bi;
    protected static Graphics  big;

    protected static  int[][]  rgbPlane    = new  int[PIC_HEIGHT][PIC_WIDTH];

    public BFLViewer(String fileName)
    {
	readFile(fileName);
    }

    public void readFile(String fileName)
    {
	try {
	    FileInputStream file = new FileInputStream(fileName);

	    byte[] Y             = new byte[PIC_WIDTH];
	    byte[] U             = new byte[PIC_WIDTH];
	    byte[] V             = new byte[PIC_WIDTH];
	    byte[] line          = new byte[PIC_WIDTH];
	    byte[] C             = new byte[PIC_WIDTH]; //symbolic colour

	    for (int i = 0; i < PIC_HEIGHT; i++) {

		file.read(Y);
		file.read(U);
		file.read(V);
		file.read(line);
		file.read(line);
		file.read(line);
		file.read(C);

		for (int j = 0; j < PIC_WIDTH; j++)
		    rgbPlane[i][j] = YUVtoRGB(Y[j], U[j], V[j]);
	    }

	    file.close();

	} catch (Exception e) {
	    System.err.println(e);
	}
    }

    public static int YUVtoRGB(byte yp, byte up, byte vp)
    {
	int y = yp & 0xff;
	int u = up & 0xff;
	int v = vp & 0xff;

	//change range of u and v for rgb conversion
	u -= 128;
	v -= 128;

	int  r  = clip((int) ((1.164 * y) + (1.596 * u)));
	int  g  = clip((int) ((1.164 * y) - (0.813 * u) - (0.391 * v)));
	int  b  = clip((int) ((1.164 * y) + (1.596 * v)));

	return ((r << 16) | (g << 8) | b);
    }

    public static int clip(int val)
    {
	if (val < 0)
	    return 0;
	else if (val > 255)
	    return 255;
	return val;
    }

    public void display()
    {
	paintComponent(getGraphics());
    }

    public void paintComponent(Graphics g)
    {
	bi = createImage(getWidth(), getHeight());
	big = bi.getGraphics();

	for (int y=0; y < PIC_HEIGHT; y++)
	    for (int x=0; x < PIC_WIDTH; x++) {
		big.setColor(new Color(rgbPlane[y][x]));
		big.fillRect(x, y, 1, 1);
		big.drawRect(x, y, 1, 1);
	    }

	//draw buffered image to screen
	g.drawImage(bi, 0, 0, this);
    }

    public static void main(String[] args)
    {
	if(args.length != 1) {
	    System.err.println("usage: java BFLViewer <file>");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("BFLViewer");
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    frame.setSize(225, 195);
	    BFLViewer viewer = new BFLViewer(args[0]);
	    frame.setContentPane(viewer);
	    frame.setVisible(true);
	    viewer.display();
	    frame.setTitle(args[0]);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}
