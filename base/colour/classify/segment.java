import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;

public class segment
{
    public static final int WIDTH = 208;
    public static final int HEIGHT = 160;

    protected static byte y[] = new byte[WIDTH];
    protected static byte u[] = new byte[WIDTH];
    protected static byte rest[] = new byte[4*WIDTH];
    protected static byte c[] = new byte[WIDTH];

    protected static byte prevY[];
    protected static byte prevU[];

    protected static byte parse1 = 12;
    protected static byte parse2 = 13;
    protected static byte parse3 = 14;

    protected static int threshold = 25;

    public static String usage = "Usage: java segment <file.bfl>";

    public static int byte2UInt(byte b)
    {
	return b & 0xff;
    }

    public static byte uInt2Byte(int i)
    {
	return (byte) (i & 0xff);
    }

    public static int getCrossVal(byte val1, byte val2, byte val3, byte val4)
    {
	return (Math.abs(byte2UInt(val1) - byte2UInt(val4)) + 
		Math.abs(byte2UInt(val2) - byte2UInt(val3)));
    }

    public static void parseSegment(byte[] y, byte[] u, byte[] c, int start, int end, long rhsSumY, long rhsSumYSqr, long rhsSumU, long rhsSumUSqr, int cp)
    {
	long lhsSumY, lhsSumYSqr, lhsSumU, lhsSumUSqr;
	long minLhsSumY=0, minLhsSumYSqr=0, minLhsSumU=0, minLhsSumUSqr=0;
	long minRhsSumY=0, minRhsSumYSqr=0, minRhsSumU=0, minRhsSumUSqr=0;

	double minRSS = Double.MAX_VALUE;
	int minPos = start;
	int lhsLen = 0, rhsLen = end - start + 1;

	double lhsVarY=0, lhsVarU=0, rhsVarY=0, rhsVarU=0, rss;

	int yp = byte2UInt(y[start]), up = byte2UInt(u[start]);

	lhsSumY = yp; lhsSumYSqr = yp*yp;
	lhsSumU = up; lhsSumUSqr = up*up;

	rhsSumY -= yp; rhsSumYSqr -= yp*yp;
	rhsSumU -= up; rhsSumUSqr -= up*up;

	lhsLen++; rhsLen--;

	for(int i = start+1; i<end; i++) {

	    yp = byte2UInt(y[i]); up = byte2UInt(u[i]);

	    lhsSumY += yp; lhsSumYSqr += yp*yp;
	    lhsSumU += up; lhsSumUSqr += up*up;

	    rhsSumY -=  yp; rhsSumYSqr -= yp*yp;
	    rhsSumU -=  up; rhsSumUSqr -= up*up;

	    lhsLen++; rhsLen--;

	    double lhsRSSY = lhsSumYSqr - (double) lhsSumY*lhsSumY/lhsLen;
	    double lhsRSSU = lhsSumUSqr - (double) lhsSumU*lhsSumU/lhsLen;

	    double rhsRSSY = rhsSumYSqr - (double) rhsSumY*rhsSumY/rhsLen;
	    double rhsRSSU = rhsSumUSqr - (double) rhsSumU*rhsSumU/rhsLen;

	    rss  = lhsRSSY + lhsRSSU + rhsRSSY + rhsRSSU;

	    if(rss < minRSS) {
		minPos = i;
		minRSS = rss;

		minLhsSumY=lhsSumY; minLhsSumYSqr=lhsSumYSqr;
		minLhsSumU=lhsSumU; minLhsSumUSqr=lhsSumUSqr;

		minRhsSumY=rhsSumY; minRhsSumYSqr=rhsSumYSqr;
		minRhsSumU=rhsSumU; minRhsSumUSqr=rhsSumUSqr;

		lhsVarY = lhsRSSY/lhsLen;
		lhsVarU = lhsRSSU/lhsLen;
		rhsVarY = rhsRSSY/rhsLen;
		rhsVarU = rhsRSSU/rhsLen;
	    }
	}
	c[minPos] = uInt2Byte(cp);

	System.out.print("\t\tparse2: lhs("+start+","+minPos+","+Math.round(lhsVarY)+","+Math.round(lhsVarU)+")");
	System.out.print("\trhs: ("+(minPos+1)+","+end+","+Math.round(rhsVarY)+","+Math.round(rhsVarU)+")");
	System.out.println("\tmin rss: "+Math.round(minRSS));

	int minLHSLen = minPos - start + 1;
	int minRHSLen = end - minPos;

	if(minLHSLen > 10 && (lhsVarY > 15 || lhsVarU > 15)) {
	    System.out.print("\t");
	    parseSegment(y, u, c, start, minPos, 
			 minLhsSumY, minLhsSumYSqr, minLhsSumU, minLhsSumUSqr, cp+1);
	}

	if(minRHSLen > 10 && (rhsVarY > 15 || rhsVarU > 15)) {
	    System.out.print("\t");
	    parseSegment(y, u, c, minPos+1, end,
			 minRhsSumY, minRhsSumYSqr, minRhsSumU, minRhsSumUSqr, cp+1);
	}
    }

    public static void main(String args[])
    {
	if (args.length != 1 || !args[0].toLowerCase().endsWith(".bfl")) {
	    System.err.println(usage);
	    return;
	}

	try {
	    String file = args[0];

	    FileInputStream in = new FileInputStream(file);
	    FileOutputStream out = new FileOutputStream(file+".seg");

	    System.out.println(file);

	    int totalY = 0;
	    int numPixel = 0;

	    //first row
	    in.read(y); in.read(u); in.read(rest); in.read(c);
	    out.write(y); out.write(u); out.write(rest); out.write(c);
	    prevY = (byte[]) y.clone(); prevU = (byte[]) u.clone();

	    for (int i = 1; i < HEIGHT - 1; i++) {

		in.read(y); in.read(u); in.read(rest); in.read(c);
		out.write(y); out.write(u); out.write(rest);

		boolean prevEdge = true, curEdge;
		int segmentStart = 0;
		int yp = byte2UInt(y[0]), up = byte2UInt(u[0]);
		long sumY = yp, sumYSqr = yp*yp;
		long sumU = up, sumUSqr = up*up;

		System.out.print(i);

		for(int j=1; j < WIDTH; j++) {

		    yp = byte2UInt(y[j]);
		    up = byte2UInt(u[j]);

		    totalY += yp; numPixel++;

		    curEdge = (getCrossVal(prevY[j-1], prevY[j], y[j-1], y[j]) >= threshold || getCrossVal(prevU[j-1], prevU[j], u[j-1], u[j]) >= threshold);

		    if(curEdge)
			c[j] = parse1;

		    if((!prevEdge && curEdge) || j == WIDTH - 1) {

			int n = j - segmentStart;
			double varY = Math.round((double) sumYSqr/n - (double) (sumY*sumY)/(n*n));

			double varU = Math.round((double) sumUSqr/n - (double) (sumU*sumU)/(n*n));

			System.out.print("\tSegment: ("+segmentStart+","+(j-1)+")");
			System.out.println("\tvarY: "+varY+"\tvarU: "+varU);

			if(n > 10 && (varY > 50 || varU > 50))
			    parseSegment(y, u, c, segmentStart, j-1, sumY, sumYSqr, sumU, sumUSqr, 11);

			//reset for next segment
			segmentStart = j;
			sumY = sumYSqr = 0;
			sumU = sumUSqr = 0;
		    }

		    prevEdge = curEdge;
		    sumY += yp; sumYSqr += yp*yp;
		    sumU += up; sumUSqr += up*up;

		}

		out.write(c);
		prevY = (byte[]) y.clone(); prevU = (byte[]) u.clone();
		System.out.println();
	    
	    }

	    //last row
	    in.read(y); in.read(u); in.read(rest); in.read(c);
	    out.write(y); out.write(u); out.write(rest); out.write(c);

	    System.out.println("Average intensity: "+(totalY/numPixel));

	} catch (Exception e) {
	    System.out.println(e);
	}
    }
}
