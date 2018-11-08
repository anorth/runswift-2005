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

/**
 * filter constants and static methods
 */
public abstract class icFilter
{
    protected static final int PIC_WIDTH  = icConstant.PIC_WIDTH;
    protected static final int PIC_HEIGHT = icConstant.PIC_HEIGHT;

    protected static byte buffer[][] = new byte[PIC_WIDTH][PIC_HEIGHT];
    protected static boolean roboc = false;


    public final static int RobertsMask1[] = { 1,  0, 
					       0, -1 };

    public final static int RobertsMask2[] = {  0, 1, 
					       -1, 0 };

    public final static int SobelMask1[]      = { 1,  2,  1, 
						  0,  0,  0,
						 -1, -2, -1 };

    public final static int SobelMask2[]      = {-1,  0,  1, 
						 -2,  0,  2,
						 -1,  0,  1 };

    public final static int PrewittMask1[]    = {-1, -1, -1, 
						  0,  0,  0,
						  1,  1,  1 };

    public final static int PrewittMask2[]    = {-1,  0,  1, 
						 -1,  0,  1,
						 -1,  0,  1 };

    public final static int LaplacianMask[]   = { 0, -1,  0,
						 -1,  4, -1,
						  0, -1,  0 };

    public final static int SharpenMask[]     = {-1, -1, -1,
						 -1,  2, -1,
						 -1, -1, -1 };

    public final static int SmoothMask[]      = { 1,  1,  1,
						  1, -2,  1,
						  1,  1,  1 };

    public final static byte RoboCValue[] = 
    {6, 5, 3, 4, 2, 7, 8, 0, 10, 1, 9};

    /*
    public final static byte RoboCValue[] = 
    { 10 ROBOTGREY, 9 BLACK, 8 REDDOG, 7 BLUEDOG, 6 ORANGE, 
       5 BLUE, 4 YELLOW, 3 GREEN, 2 PINK, 
       1 FIELDWHITE, 0 GREENFIELD };
    */
    //From and to may be the same
    public static void applyFilter(byte[][] array, int filter)
    {
	switch(filter)
	    {
	    case icConstant.FILTER_SHARPEN:
		applyMask(array, buffer, SharpenMask); copy(buffer, array);
		break;
	    case icConstant.FILTER_SMOOTH:
		applyMask(array, buffer, SmoothMask); copy(buffer, array);
		break;
	    case icConstant.FILTER_SOBEL:
		edge(array, buffer, SobelMask1, SobelMask2); copy(buffer, array);
		break;
	    case icConstant.FILTER_ROBERTS:
		robertsCross(array, buffer); copy(buffer, array);
		break;
	    case icConstant.FILTER_ERODE:
		erode(array, buffer); copy(buffer, array);
		break;
	    case icConstant.FILTER_DILATE:
		dilate(array, buffer); copy(buffer, array);
		break;
	    case icConstant.FILTER_OPEN:
		erode(array, buffer);  dilate(buffer, array);
		break;
	    case icConstant.FILTER_CLOSE:
		dilate(array, buffer); erode(buffer, array);
		break;
	    case icConstant.FILTER_ROBOC:
		roboc = true;
		dilate(array, buffer); erode(buffer, array);
		break;
	    case icConstant.FILTER_ADDBLUE:
		addBlue(array, buffer); addBlue(buffer, array); 
		/*
		addBlue(to, buffer);   addBlue(buffer, to); 
		addBlue(to, buffer);   addBlue(buffer, to); 
		addBlue(to, buffer);   addBlue(buffer, to); 
		addBlue(to, buffer);   addBlue(buffer, to); 
		*/
		break;
	    case icConstant.FILTER_EQUAL:
		equalise(array);
		break;
	    }
	roboc = false;
    }

    /**
     * Apply 3*3 mask to given element in given int array.
     * Does not do anything to edge elements.
     * Only used in old icEdgePanel.
     *
     * @param  m  filter mask in the form of a 3*3 matrix
     * @param  a  array
     * @param  x  row index of array
     * @param  y  column index of array
     */
    public static int applyFilter3(int m[], int x, int y, int a[][])
    {
	if(m.length != 9 || 
	   x == 0 || y == 0 || x == PIC_HEIGHT-1 || y == PIC_WIDTH-1)
	    return -1;

	int sum = 0;
	for(int i=x-1, j=y-1, k=0; i<=x+1 && j<=y+1; i++, j++, k++)
	    sum += m[k]*a[i][j];

	return sum;
    }

    public static void copy(byte[][] from, byte[][] to)
    {
	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++)
		to[w][h] = from[w][h];
    }

    /**
     * Apply 3*3 mask to given element in given int array.
     * Does not do anything to edge elements.
     */
    public static void applyMask(byte[][] from, byte[][] to, int mask[])
    {
	int margin = 1;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w == 0 || h == 0 || w == PIC_WIDTH-1 || h == PIC_HEIGHT-1)
		    to[w][h] = from[w][h];
		else {
		    to[w][h] = 0;
		    for(int i=w-margin, k=0; i<=w+margin; i++)
			for(int j=h-margin; j<=h+margin; j++, k++)
			    to[w][h] += mask[k]*from[i][j];
		}
	    }
    }

    public static void edge(byte[][] from, byte[][] to, int mask1[], int mask2[])
    {
	int margin = 1;
	int a, b;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w == 0 || h == 0 || w == PIC_WIDTH-1 || h == PIC_HEIGHT-1)
		    to[w][h] = icConstant.MAX_VALUE;
		else {
		    a = b = 0;
		    for(int i=w-margin, k=0; i<=w+margin; i++)
			for(int j=h-margin; j<=h+margin; j++, k++) {
			    a += mask1[k]*icConstant.byte2UInt(from[i][j]);
			    b += mask2[k]*icConstant.byte2UInt(from[i][j]);
			}
		    to[w][h] = icConstant.uInt2Byte(icColor.clip(
                                Math.abs(a) + Math.abs(b)));
		}
	    }
    }

    public static void edgeBinary(byte[][] from, byte[][] to, int mask1[], int mask2[])
    {
	int margin = 1; byte threshold = 30;
	byte a, b;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w == 0 || h == 0 || w == PIC_WIDTH-1 || h == PIC_HEIGHT-1)
		    to[w][h] = icConstant.MAX_VALUE;
		else {
		    a = b = 0;
		    for(int i=w-margin, k=0; i<=w+margin; i++)
			for(int j=h-margin; j<=h+margin; j++, k++) {
			    a += mask1[k]*from[i][j];
			    b += mask2[k]*from[i][j];
			}
		    to[w][h] = (Math.abs(a) + Math.abs(b) >= threshold) ? 
			icConstant.MAX_VALUE : icConstant.MIN_VALUE;
		}
	    }
    }

    public static void robertsCrossSqr(byte[][] from, byte[][] to)
    {
	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w == 0 || h == 0 || w == PIC_WIDTH-1 || h == PIC_HEIGHT-1)
		    to[w][h] = icConstant.MAX_VALUE;
		else
		    to[w][h] = icConstant.uInt2Byte(icColor.clip(
			       Math.pow(icConstant.byte2UInt(from[w][h]) - 
					icConstant.byte2UInt(from[w+1][h+1]),2)
			       +
			       Math.pow(icConstant.byte2UInt(from[w+1][h]) - 
					icConstant.byte2UInt(from[w][h+1]),2)));
	    }
    }

    public static void robertsCross(byte[][] from, byte[][] to)
    {
	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w == 0 || h == 0 || w == PIC_WIDTH-1 || h == PIC_HEIGHT-1)
		    to[w][h] = icConstant.MAX_VALUE;
		else
		    to[w][h] = icConstant.uInt2Byte(icColor.clip(
			       Math.abs(icConstant.byte2UInt(from[w][h]) - 
					icConstant.byte2UInt(from[w+1][h+1]))
			       +
			       Math.abs(icConstant.byte2UInt(from[w+1][h]) - 
					icConstant.byte2UInt(from[w][h+1]))));
	    }
    }

    public static void robertsCrossBinary(byte[][] from, byte[][] to)
    {
	byte threshold = 28;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w == 0 || h == 0 || w == PIC_WIDTH-1 || h == PIC_HEIGHT-1)
		    to[w][h] = icConstant.MAX_VALUE;
		else
		    to[w][h] = 
			(Math.abs(icConstant.byte2UInt(from[w][h]) - 
				  icConstant.byte2UInt(from[w+1][h+1]))
			 +
			 Math.abs(icConstant.byte2UInt(from[w+1][h]) - 
				  icConstant.byte2UInt(from[w][h+1]))
			 >= threshold) ? 
			icConstant.MAX_VALUE : icConstant.MIN_VALUE;
	    }
    }

    public static void erode(byte[][] from, byte[][] to)
    {
	int margin = 1;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w < margin || h < margin || w >= PIC_WIDTH-margin || h >= PIC_HEIGHT-margin)
		    to[w][h] = from[w][h];
		else {

		    to[w][h] = from[w-margin][h-margin];

		    for(int i=w-margin; i<=w+margin; i++)
			for(int j=h-margin; j<=h+margin; j++)
			    to[w][h] = min(from[i][j], to[w][h]);
		}
	    }
    }

    public static void dilate(byte[][] from, byte[][] to)
    {
	int margin = 1;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w < margin || h < margin || w >= PIC_WIDTH-margin || h >= PIC_HEIGHT-margin)
		    to[w][h] = from[w][h];
		else {

		    to[w][h] = from[w-margin][h-margin];

		    for(int i=w-margin; i<=w+margin; i++)
			for(int j=h-margin; j<=h+margin; j++)
			    to[w][h] = max(from[i][j], to[w][h]);
		}
	    }
    }

    public static byte min(byte a, byte b)
    {
	if(roboc && a != icConstant.COLOR_NONE && b != icConstant.COLOR_NONE)
	    return (RoboCValue[a] < RoboCValue[b]) ? a : b;

	return (a < b) ? a : b;
    }

    public static byte max(byte a, byte b)
    {
	if(roboc && a != icConstant.COLOR_NONE && b != icConstant.COLOR_NONE)
	    return (RoboCValue[a] > RoboCValue[b]) ? a : b;

	return (a > b) ? a : b;
    }

    public static void addBlue(byte[][] from, byte[][] to)
    {
	int margin = 3, threshold = 4, sum = 0;

	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++) {
		if(w < margin || h < margin || w >= PIC_WIDTH-margin || h >= PIC_HEIGHT-margin)
		    to[w][h] = from[w][h];
		else {
		    sum = 0;
		    for(int i=w-margin; i<=w+margin; i++)
			for(int j=h-margin; j<=h+margin; j++)
			    if(from[i][j] == icConstant.COLOR_ROBOTBLUE) {
				sum++;
				if(sum > threshold)
				    break;
			    }
		    if(sum > threshold && 
		       (from[w][h] == icConstant.COLOR_ROBOTGREY ||
			from[w][h] == icConstant.COLOR_BLACK))
			to[w][h] = icConstant.COLOR_ROBOTBLUE;
		    else
			to[w][h] = from[w][h];
		}
	    }
    }

    public static void clearHistogram(int[] hist)
    {
	for(int i=0; i < 256; i++)
	    hist[i] = 0;
    }

    /**
     * Analyse an 8-bit image into a histogram.
     */
    public static void getHistogram(byte[][] img, int[] hist)
    {
	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++)
		hist[icConstant.byte2UInt(img[w][h])]++;
    }

    public static void equalise(byte[][] array) {
    
	int max = 255;
	int range = 255;
        
        double sum;

        int[] histogram = new int[range+1];
        clearHistogram(histogram);
	getHistogram(array, histogram);
	icConstant.print("Old Histogram: ", histogram);

        sum = getWeightedValue(histogram, 0);
        for (int i=1; i<max; i++)
            sum += 2 * getWeightedValue(histogram, i);
        sum += getWeightedValue(histogram, max);
        
        double scale = range/sum;
        byte[] lut = new byte[range+1];
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
	icConstant.print("Look up table: ", lut);

        //apply table
	for(int h=0; h < PIC_HEIGHT; h++)
	    for(int w=0; w < PIC_WIDTH; w++)
		array[w][h] = lut[icConstant.byte2UInt(array[w][h])];

	//get histogram of new picture
	clearHistogram(histogram);
	getHistogram(array, histogram);
	icConstant.print("New Histogram: ", histogram);
    }

    protected static double getWeightedValue(int[] histogram, int i) {
	//Classic equalisation: return (double) histogram[i];

	//Taking the square root reduces noise
        return Math.sqrt((double) histogram[i]);
    }
}
