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

import java.io.*;
import java.awt.*;

public class correctRing
{
    public static String usage = 
	"Usage:\t java correctRing -[txhp] <file/dir name>";

    public static final int WIDTH   = 208;
    public static final int HEIGHT  = 160;
    public static final int DEPTH   = 256;

    public static final int X = WIDTH;
    public static final int Y = HEIGHT;
    public static final int Z = DEPTH;

    public static Point py1 = new Point(-70,20);
    public static Point py2 = new Point(290,20);
    //public static Point py1 = new Point(-74,21);
    //public static Point py2 = new Point(276,21);
    public static Point cy1 = new Point(avg(py1.x,py2.x),avg(py1.y,py2.y));//xz
    public static Point cy2 = new Point(avg(py1.x,py2.x), HEIGHT/2); //xy
    //public static double ky = 2.3;
    public static double ky = 2.0;

    //original
    //public static Point pu1 = new Point(-50,110);
    //public static Point pu2 = new Point(270,110);
    //new
    public static Point pu1 = new Point(-50,130);
    public static Point pu2 = new Point(260,130);
    //public static Point pu1 = new Point(-50,130);
    //public static Point pu2 = new Point(250,130);
    public static Point cu1 = new Point(avg(pu1.x,pu2.x),avg(pu1.y,pu2.y));
    public static Point cu2 = new Point(avg(pu1.x,pu2.x), HEIGHT/2);
    //public static double ku = 1.8;
    public static double ku = 2.0;

    public static Point pv1 = new Point(-65,160);
    public static Point pv2 = new Point(260,160);
    //public static Point pv1 = new Point(-30,220);
    //public static Point pv2 = new Point(250,220);
    public static Point cv1 = new Point(avg(pv1.x,pv2.x),avg(pv1.y,pv2.y));
    public static Point cv2 = new Point(avg(pv1.x,pv2.x), HEIGHT/2);
    //public static double kv = 1.6;
    public static double kv = 2.0;

    protected static final Point ORIGIN    = new Point(0,0);
    protected static final Point EXTREMITY = new Point(WIDTH,HEIGHT);

    public static final int SLICEY_WIDTH  = maxDist(cy2, ORIGIN, EXTREMITY);
    public static final int SLICEU_WIDTH  = maxDist(cu2, ORIGIN, EXTREMITY);
    public static final int SLICEV_WIDTH  = maxDist(cv2, ORIGIN, EXTREMITY);
    public static final int SLICE_HEIGHT  = DEPTH;

    public static final int XY_WIDTH  = max(Math.max(cy2.x, WIDTH-cy2.x),
					    Math.max(cu2.x, WIDTH-cu2.x),
					    Math.max(cv2.x, WIDTH-cv2.x))+1;
    public static final int XY_HEIGHT = HEIGHT;
    
    protected static int sliceY[][] = new int[SLICE_HEIGHT][SLICEY_WIDTH];
    protected static int sliceU[][] = new int[SLICE_HEIGHT][SLICEU_WIDTH];
    protected static int sliceV[][] = new int[SLICE_HEIGHT][SLICEV_WIDTH];

    protected static int xLUT[][]   = new int[XY_HEIGHT][XY_WIDTH];

    public static int avg(int a, int b)
    {
	return (int) Math.round((a+b)/2);
    }

    public static int max(int a, int b, int c)
    {
	return Math.max(Math.max(a, b), c);
    }

    /**
     * Find the maximum distance between a point and any of the 4 corners
     * of a rectangle defined by its bottom left and top right corners.
     * @params o bottom left corner of a rectangle
     * @params p top right corner of a rectangle
     * @params c a point in the rectangle defined by o & p
     */
    public static int maxDist(Point c, Point o, Point p)
    {
	return (int) Math.ceil(
	       Math.max(Math.max(c.distance(o),c.distance(p)),
		        Math.max(c.distance(o.x,p.y),c.distance(p.x,o.y))));
    }

    public static void getSlice(int slice[][], Point q1, Point q2, double k)
    {
	init(slice);
	drawLine(slice, q1, q2, k);
	fillGaps(slice);
    }

    protected static void init(int slice[][])
    {
	for(int i=0; i<slice.length; i++)
	    for(int j=0; j<slice[i].length; j++) 
		slice[i][j] = -1;
    }

    /**
     * Draw electric field lines for 
     * x>=SLICE_WIDTH && x<X && y >=0 && y <HEIGHT
     * where q1.x < 0 && q2.x > X
     */
    protected static void drawLine(int slice[][], Point q1, Point q2, double k)
    {
	double rsq1, rsq2, x, y, sumX, sumY, dx1, dy1, dx2, dy2, d;
	int CX = avg(q1.x,q2.x);

	for(int i=0; i<slice.length; i++) {

	    x=CX; y=i;
	    sumX=0; sumY=0;

	    //System.out.println("x "+x+" y "+y);

	    do {
		//System.out.println("x "+Math.round(x)+" y "+Math.round(y));

		slice[(int) Math.round(y)][(int) Math.round(x)-CX] = i;
		dx1 = q1.x - x; dy1 = q1.y - y;
		dx2 = q2.x - x; dy2 = q2.y - y;

		//System.out.println("dx1 "+dx1+" dy1 "+dy1+" dx2 "+dx2+" dy2 "+dy2);
		rsq1 = dx1*dx1 + dy1*dy1;
		rsq2 = dx2*dx2 + dy2*dy2;

		//System.out.println("rsq1 "+rsq1+" rsq2 "+rsq2);

		sumX += dx2/rsq2 - dx1/rsq1;
		sumY += dy2/rsq2 - dy1/rsq1;

		//System.out.println("sumX "+sumX+" sumY "+sumY+" d "+d);

		d = Math.sqrt(sumX*sumX + sumY*sumY);
		x += sumX/d;
		y += k*sumY/d;

	    } while(y>=0 && Math.round(y)<slice.length && Math.round(x)-CX<slice[i].length);
	}
    }

    /**
     * Flood fill gaps in given slice
     */
    protected static void fillGaps(int slice[][])
    {
	for(int i=0; i<slice.length; i++)
	    for(int j=1, val=i; j<slice[i].length; j++) {
		if(slice[i][j] < 0)
		    slice[i][j] = val;
		else
		    val = slice[i][j];
	    }
    }


    public static void printYUVLUT(int slice[][], String path, boolean append)
    {
	try {
	    PrintWriter out = new PrintWriter(new FileWriter(path,append),true);

	    out.print("{");
	    for(int i=0; i<slice.length; i++) {

		out.print("{"+slice[i][0]);

		for(int j=1; j<slice[i].length; j++)
		    out.print(","+slice[i][j]);

		if(i!=slice.length-1)
		    out.println("},");
		else
		    out.println("}};");
	    }
	} catch (Exception e) {
	    System.out.println(e);
	}
    }

    public static void getXLUT(int xlut[][])
    {
	Point c = new Point(0, xlut.length/2-1);

	for(int i=0; i<xlut.length/2; i++) {
	    xlut[i][0] = (int) Math.round(c.distance(0,i));
	    for(int j=0; j<xlut[i].length; j++)
		xlut[i][j] = (int) Math.round(c.distance(j,i));
	}

	c = new Point(0, xlut.length/2);

	for(int i=xlut.length/2; i<xlut.length; i++) {
	    xlut[i][0] = (int) Math.round(c.distance(0,i));
	    for(int j=0; j<xlut[i].length; j++)
		xlut[i][j] = (int) Math.round(c.distance(j,i));
	}
    }

    public static void printXLUT(String path, boolean append)
    {
	try {
	    PrintWriter out = new PrintWriter(new FileWriter(path,append),true);

	    out.print("{");
	    for(int i=0; i<xLUT.length; i++) {

		out.print("{"+xLUT[i][0]);

		for(int j=1; j<xLUT[i].length; j++) {
		    if (xLUT[i][j] > 255)
                System.out.println("Warning: xLUT value > 255 printing "
                                    + "to uchar at xLUT " + i + ", " + j);
            out.print(","+xLUT[i][j]);
        }

		if(i!=xLUT.length-1)
		    out.println("},");
		else
		    out.println("}};");
	    }
	} catch (Exception e) {
	    System.out.println(e);
	}
    }

    public static void printCorrectPics(String path)
    {
	File dir = new File(path);
	if (!dir.isDirectory()) {
	    System.err.println(path+" is not a directory");
	    return;
	}
	path += File.separator;

	String[] fileList = dir.list();
	String name;
	File rcf = null;
	BufferedInputStream in = null;
	BufferedOutputStream out = null;
	byte rest[] = new byte[4*WIDTH];
	int x;

	try {
	    rcf = new File(path+"rcf");
	    if(!rcf.exists())
		rcf.mkdir();

	    for(int k = 0; k < fileList.length; k++) {

		if(!fileList[k].toLowerCase().endsWith(".bfl"))
		    continue;

		name = path+fileList[k];
		in = new BufferedInputStream(new FileInputStream(name));
		out = new BufferedOutputStream(new FileOutputStream(path+"rcf"+File.separator+fileList[k]));
		System.out.println(name);

		for (int i = 0; i < HEIGHT; i++) {

		    for (int j=0; j < WIDTH; j++)
			out.write(sliceY[in.read()][(xLUT[i][Math.abs(j-cy2.x)])]);

		    for (int j=0; j < WIDTH; j++)
			out.write(sliceU[in.read()][(xLUT[i][Math.abs(j-cu2.x)])]);

		    for (int j=0; j < WIDTH; j++)
			out.write(sliceV[in.read()][(xLUT[i][Math.abs(j-cv2.x)])]);

		    in.read(rest); out.write(rest);
		}
		in.close(); out.close();
	    }
	} catch (Exception e) {
	    System.out.println(e);
	}
    }

    public static void printHeaderFile(String path)
    {
	PrintWriter out = null;
	try {
	    out = new PrintWriter(new FileWriter(path), true);
	    out.println("static const int cyx = "+cy1.x+";");
	    out.println("static const int cux = "+cu1.x+";");
	    out.println("static const int cvx = "+cv1.x+";");
	    out.println();

	    out.println("static const uchar sliceY["+SLICE_HEIGHT+"]["+SLICEY_WIDTH+"]=");
	    out.close();
	    printYUVLUT(sliceY, path, true);

	    out = new PrintWriter(new FileWriter(path, true), true);
	    out.println();
	    out.println("static const uchar sliceU["+SLICE_HEIGHT+"]["+SLICEU_WIDTH+"]=");
	    out.close();
	    printYUVLUT(sliceU, path, true);

	    out = new PrintWriter(new FileWriter(path, true), true);
	    out.println();
	    out.println("static const uchar sliceV["+SLICE_HEIGHT+"]["+SLICEV_WIDTH+"]=");
	    out.close();
	    printYUVLUT(sliceV, path, true);

	    out = new PrintWriter(new FileWriter(path, true), true);
	    out.println();
//	    out.println("static const int xLUT["+XY_HEIGHT+"]["+XY_WIDTH+"]=");
        // We observed that the values are never greater than 255 so we
        // write it out as uchar
	    out.println("static const uchar xLUT["+XY_HEIGHT+"]["+XY_WIDTH+"]=");
	    out.close();
	    printXLUT(path, true);
	} catch (Exception e) {
	    System.out.println(e);
	}
    }

    protected static void checkArgLength(int length, int n)
    {
	if(n != length) {
	    System.err.println(usage);
	    System.exit(0);
	}
    }

    public static void main(String args[])
    {
	if(args.length != 2 || args[0].length() != 2 || 
	   args[0].charAt(0) != '-') {
	    System.err.println(usage);
	    return;
	}

	boolean isCorrectPic = false;
	boolean isYUV        = false;
	boolean isX          = false;
	boolean isHeader     = false;

	String path = args[1];

	System.out.println("SLICEY_WIDTH: "+SLICEY_WIDTH);
	System.out.println("SLICEU_WIDTH: "+SLICEU_WIDTH);
	System.out.println("SLICEV_WIDTH: "+SLICEV_WIDTH);
	System.out.println("SLICE_HEIGHT: "+SLICE_HEIGHT);
	System.out.println("XY_WIDTH: "+XY_WIDTH+" XY_HEIGHT: "+XY_HEIGHT);

	switch(args[0].charAt(1)) {
	case 'p':
	    isCorrectPic = true;
	    break;
	case 't':
	    isYUV = true;
	    break;
	case 'x':
	    isX = true;
	    break;
	case 'h':
	    isHeader = true;
	    break;
	default:
	    System.err.println(usage);
	    return;
	}

	if(isCorrectPic || isYUV || isHeader) {
	    getSlice(sliceY, py1, py2, ky);
	    getSlice(sliceU, pu1, pu2, ku);
	    getSlice(sliceV, pv1, pv2, kv);
	}

	if(isCorrectPic || isX || isHeader)
	    getXLUT(xLUT);

	if(isX)
	    printXLUT(path, false);

	if(isCorrectPic)
	    printCorrectPics(path);

	if(isYUV) {
	    printYUVLUT(sliceY, path+".y", false);
	    printYUVLUT(sliceU, path+".u", false);
	    printYUVLUT(sliceV, path+".v", false);
	}

	if(isHeader)
	    printHeaderFile(path);
    }
}
