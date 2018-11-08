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

public class equalise
{
    public static final int WIDTH = 208;
    public static final int HEIGHT = 160;

    protected static byte y[] = new byte[WIDTH];
    protected static byte u[] = new byte[WIDTH];
    protected static byte v[] = new byte[WIDTH];
    protected static byte y2[] = new byte[WIDTH];
    protected static byte y3[] = new byte[WIDTH];
    protected static byte y4[] = new byte[WIDTH];
    protected static byte c[] = new byte[WIDTH];

    public static int levels = 256;
    public static int max = 255;
    public static int range = 255;

    public static int histogramY[] = new int[levels];
    public static int histogramU[] = new int[levels];
    public static int histogramV[] = new int[levels];

    public static byte[] lutY = new byte[levels];
    public static byte[] lutU = new byte[levels];
    public static byte[] lutV = new byte[levels];

    /**
     * Get the collective Y, U and V histograms of every bfl image in 
     * a given directory
     */
    public static void getHistograms(String path)
    {
	File dir = new File(path);
	if (!dir.isDirectory()) {
	    System.out.println(path+" is not a directory");
	    return;
	}

	try {
	    String[] fileList = dir.list();
	    FileInputStream  in  = null;

	    clearHistograms();

	    for(int k = 0; k < fileList.length; k++) {

		if(!fileList[k].toLowerCase().endsWith(".bfl"))
		    continue;

		in = new FileInputStream(path+File.separator+fileList[k]);

		for (int i = 0; i < HEIGHT; i++) {

		    in.read(y);
		    in.read(u);
		    in.read(v);
		    in.read(y2);
		    in.read(y3);
		    in.read(y4);
		    in.read(c);

		    for (int j = 0; j < WIDTH; j++) {
			histogramY[icConstant.byte2UInt(y[j])]++;
			histogramU[icConstant.byte2UInt(u[j])]++;
			histogramV[icConstant.byte2UInt(v[j])]++;
		    }
		}
		in.close();
	    }
	} catch (Exception e) {
	    System.out.println(e);
	}
    }

    /**
     * Equalise a given 256 level histogram into a look-up table
     */
    public static void getLUT(int[] histogram, byte[] lut)
    {
	double sum;

	sum = getWeightedValue(histogram, 0);
	for (int i=1; i<max; i++)
	    sum += 2 * getWeightedValue(histogram, i);
	sum += getWeightedValue(histogram, max);
        
	double scale = range/sum;
	double delta;
        
	lut[0] = icConstant.uInt2Byte(0);
	sum = getWeightedValue(histogram, 0);
	for (int i=1; i<max; i++) {
	    delta = getWeightedValue(histogram, i);
	    sum += delta;
	    lut[i] = icConstant.uInt2Byte((int) Math.round(sum*scale));
	    sum += delta;
	}
	lut[max] = icConstant.uInt2Byte(max);
    }

    protected static double getWeightedValue(int[] histogram, int i) {
        return Math.sqrt((double) histogram[i]);
    }

    /**
     * Apply the respective Y, U, V look-up tables to the respective Y, U, V
     * values of every bfl image in a given directory, 
     * then output the result to <file name>.eql.
     */
    public static void applyLUT(String path)
    {
	File dir = new File(path);
	if (!dir.isDirectory()) {
	    System.out.println(path+" is not a directory");
	    return;
	}

	try {
	    String[] fileList = dir.list();
	    FileInputStream  in  = null;
	    FileOutputStream out = null;
	    String name;

	    clearHistograms();

	    for(int k = 0; k < fileList.length; k++) {

		if(!fileList[k].toLowerCase().endsWith(".bfl"))
		    continue;

		name = path+File.separator+fileList[k];
		in = new FileInputStream(name);
		out = new FileOutputStream(name+".eql");

		for (int i = 0; i < HEIGHT; i++) {

		    in.read(y);
		    in.read(u);
		    in.read(v);
		    in.read(y2);
		    in.read(y3);
		    in.read(y4);
		    in.read(c);

		    for (int j = 0; j < WIDTH; j++) {
			y[j] = lutY[icConstant.byte2UInt(y[j])];
			u[j] = lutU[icConstant.byte2UInt(u[j])];
			v[j] = lutV[icConstant.byte2UInt(v[j])];

			histogramY[icConstant.byte2UInt(y[j])]++;
			histogramU[icConstant.byte2UInt(u[j])]++;
			histogramV[icConstant.byte2UInt(v[j])]++;
		    }

		    out.write(y);
		    out.write(u);
		    out.write(v);
		    out.write(y2);
		    out.write(y3);
		    out.write(y4);
		    out.write(c);
		}
		in.close();
		out.close();
	    }
	} catch (Exception e) {
	    System.out.println(e);
	}
    }

    public static void clearHistograms()
    {
	for(int i=0; i < levels; i++) {
	    histogramY[i] = 0;
	    histogramU[i] = 0;
	    histogramV[i] = 0;
	}
    }

    public static void main(String args[])
    {
	if (args.length != 1) {
	    System.out.println("Usage: java equalise <directory>");
	    System.exit(1);
	}

	System.out.println(args[0]);

	getHistograms(args[0]);
	icConstant.print("Old HistogramY: ", histogramY);
	icConstant.print("Old HistogramU: ", histogramU);
	icConstant.print("Old HistogramV: ", histogramV);

	getLUT(histogramY, lutY);
	icConstant.print("Look up table Y: ", lutY);

	getLUT(histogramU, lutU);
	icConstant.print("Look up table U: ", lutU);

	getLUT(histogramV, lutV);
	icConstant.print("Look up table V: ", lutV);

	applyLUT(args[0]);
	icConstant.print("New HistogramY: ", histogramY);
	icConstant.print("New HistogramU: ", histogramU);
	icConstant.print("New HistogramV: ", histogramV);
    }
}
