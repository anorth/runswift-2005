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
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.beans.*;
import com.sun.image.codec.jpeg.*;

public class icBasicBFLPanel extends JPanel
{
    protected String fileName;

    public boolean isFilter                = false;
    public byte    color                   = icConstant.COLOR_NONE;
    public int     filterPlane             = icConstant.FILTER_YPLANE;
    public int     filterResult            = icConstant.FILTER_RES_YUV;

    protected static final int FACTOR      = icConstant.FACTOR;
    protected static final int PIC_WIDTH   = icConstant.PIC_WIDTH;
    protected static final int PIC_HEIGHT  = icConstant.PIC_HEIGHT;

    //arrays for colour data
    public static byte[][]  YLL            = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  U              = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  V              = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  YLH            = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  YHL            = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  YHH            = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  C              = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  prevC          = new byte[PIC_WIDTH][PIC_HEIGHT];

    public    static  int[][]  rgbPlane    = new  int[PIC_WIDTH][PIC_HEIGHT];
    public    static  int[][]  rgbOverlay  = new  int[PIC_WIDTH][PIC_HEIGHT];
    public    static  int      edgeMask    = 0;

    //protected static byte[][][]colorCube   = new byte[128][128][128];

    public static byte[][]  filterY        = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  filterU        = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  filterV        = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static byte[][]  filterC        = new byte[PIC_WIDTH][PIC_HEIGHT];
    public static  int[][]  filterRGB      = new  int[PIC_WIDTH][PIC_HEIGHT];

    protected int              SCALE       = 1;
    protected int              OLDSCALE    = -1;
    protected Image            bi;
    protected Graphics         big;

    protected int       STARTX       = 0;
    protected int       STARTY       = 0;

    protected int       ENDX         = PIC_WIDTH;
    protected int       ENDY         = PIC_HEIGHT;

    protected int       quadrant     = 0;

    /**  Constructor for the BFLPanel object */
    public icBasicBFLPanel()
    {
	setBorder(BorderFactory.createEtchedBorder());

	/*
	try {
	    FileInputStream file = new FileInputStream("./pics.cal");

	    //load color cube
	    for(int i=0; i<128; i++)
		for(int j=0; j<128; j++)
		    file.read(colorCube[i][j]);

	} catch(Exception e) {
	    System.err.println(e);
	}
	*/
    }

    public icBasicBFLPanel(String fileName)
    {
	this();
	readFile(fileName);
    }

    public String getFileName()
    {
	return fileName;
    }

    /**
     * reads the BFL file and stores the YUVC information in four arrays.
     * UPDATE: Also performs edge detection and stores the result in 
     *         a PIC_WIDTH by PIC_HEIGHT array (non-thresholded). 
     */
    public void readFile(String fileName)
    {
	try {
	    FileInputStream file = new FileInputStream(fileName);

	    byte[] lineYLL  = new byte[PIC_WIDTH];
	    byte[] lineU    = new byte[PIC_WIDTH];
	    byte[] lineV    = new byte[PIC_WIDTH];
	    byte[] lineYLH  = new byte[PIC_WIDTH];
	    byte[] lineYHL  = new byte[PIC_WIDTH];
	    byte[] lineYHH  = new byte[PIC_WIDTH];
	    byte[] lineC    = new byte[PIC_WIDTH];

	    int y00, y10, y01, y11, sign;

	    for (int i = 0; i < PIC_HEIGHT; i++) {

		file.read(lineYLL);
		file.read(lineU);
		file.read(lineV);
		file.read(lineYLH);
		file.read(lineYHL);
		file.read(lineYHH);
		file.read(lineC);

		for (int j = 0; j < PIC_WIDTH; j++) {

		    filterY[j][i] = YLL[j][i] = lineYLL[j];
		    filterU[j][i] = U[j][i]   = lineU[j];
		    filterV[j][i] = V[j][i]   = lineV[j];
		    YLH[j][i] = lineYLH[j];
		    YHL[j][i] = lineYHL[j];
		    YHH[j][i] = lineYHH[j];
		    C[j][i]   = lineC[j];

		    filterRGB[j][i] = rgbPlane[j][i] = 
			icColor.YUVtoRGB(lineYLL[j], lineU[j], lineV[j]).getRGB();

		    /*
		    sign = YLL[j][i]&0x80;
		    y11 = (YLL[j][i]&0xff);
		    y01 = y11 + (YLH[j][i]&0xff);
		    y10 = y11 + (YHL[j][i]&0xff);
		    y00 = y11 + (YHH[j][i]&0xff);

		    System.out.println(icColor.addSign(y00,sign)+" "+
				       icColor.addSign(y10,sign)+" "+
				       icColor.addSign(y01,sign)+" "+
				       YLL[j][i]+" "+sign);

		    rgbPlane[2*j][2*i] = icColor.YUVtoRGB(icColor.addSign(y00,sign), 
					 lineU[j], lineV[j]).getRGB();
		    rgbPlane[2*j+1][2*i] = icColor.YUVtoRGB(icColor.addSign(y10,sign), 
					 lineU[j], lineV[j]).getRGB();
		    rgbPlane[2*j][2*i+1] = icColor.YUVtoRGB(icColor.addSign(y01,sign), 
					 lineU[j], lineV[j]).getRGB();
		    rgbPlane[2*j+1][2*i+1] = icColor.YUVtoRGB(YLL[j][i], 
					 lineU[j], lineV[j]).getRGB();
		    */

		    //filterC[j][i] = colorCube[(filterY[j][i])][(U[j][i])][(V[j][i])];
		    filterC[j][i] = 
			icCal.getCPlane(icConstant.byte2UInt(YLL[j][i])/FACTOR,
					icConstant.byte2UInt(U[j][i])/FACTOR,
					icConstant.byte2UInt(V[j][i])/FACTOR);
		    //System.out.print("("+(Y[j][i]+128)/FACTOR+","+(U[j][i]+128)/FACTOR+","+(V[j][i]+128)/FACTOR+") ");
		    //System.out.print(filterC[j][i]+" ");
		}
		//System.out.println();
	    }

	    file.close();

	} catch (Exception e) {
	    System.err.println(e);
	}

	firePropertyChange(""+icConstant.FILE_OPEN, 
			   this.fileName, fileName);
	this.fileName = fileName;
	//System.out.println("readFile: "+fileName);
	((JFrame) getTopLevelAncestor()).setTitle(fileName);
	repaint();
    }

    public void readFile()
    {
	if(fileName != null)
	    readFile(fileName);
    }

    public void writeFile(String path)
    {
	try {
	    FileOutputStream file = new FileOutputStream(path);

	    byte[] lineYLL  = new byte[PIC_WIDTH];
	    byte[] lineU    = new byte[PIC_WIDTH];
	    byte[] lineV    = new byte[PIC_WIDTH];
	    byte[] lineYLH  = new byte[PIC_WIDTH];
	    byte[] lineYHL  = new byte[PIC_WIDTH];
	    byte[] lineYHH  = new byte[PIC_WIDTH];
	    byte[] lineC    = new byte[PIC_WIDTH];

	    for (int i = 0; i < PIC_HEIGHT; i++) {
		for (int j = 0; j < PIC_WIDTH; j++) {
		    lineYLL[j] = YLL[j][i];
		    lineU[j]   = U[j][i];
		    lineV[j]   = V[j][i];
		    lineYLH[j] = YLH[j][i];
		    lineYHL[j] = YHL[j][i];
		    lineYHH[j] = YHH[j][i];
		    lineC[j]   = C[j][i];
		}
		file.write(lineYLL);
		file.write(lineU);
		file.write(lineV);
		file.write(lineYLH);
		file.write(lineYHL);
		file.write(lineYHH);
		file.write(lineC);
	    }
	    file.close();

	} catch (Exception e) {
	    System.err.println(e);
	}
	fileName = path;
	//System.out.println("writeFile: "+fileName);
	((JFrame) getTopLevelAncestor()).setTitle(fileName);
    }

    public void writeFile()
    {
	if(fileName != null)
	    writeFile(fileName);
    }

    /**
     * Write whatever is on screen into a jpeg file
     */
    public void writeJpegFile(String path)
    {
	try {
	    FileOutputStream file = new FileOutputStream(path);
	    BufferedImage bi = new BufferedImage(PIC_WIDTH * SCALE, 
						 PIC_HEIGHT * SCALE, 
						 BufferedImage.TYPE_INT_RGB);
	    Graphics g = bi.createGraphics();
            g.drawImage(this.bi, 0, 0, null);
            g.dispose();

	    JPEGImageEncoder encoder = JPEGCodec.createJPEGEncoder(file);
	    encoder.encode(bi);
            file.close();
        } catch (Exception e) {
	    System.err.println(e);
	}
    }

    public void writeFilterFile(String path)
    {
	try {
	    FileOutputStream file = new FileOutputStream(path);

	    byte[] lineYLL  = new byte[PIC_WIDTH];
	    byte[] lineU    = new byte[PIC_WIDTH];
	    byte[] lineV    = new byte[PIC_WIDTH];
	    byte[] lineYLH  = new byte[PIC_WIDTH];
	    byte[] lineYHL  = new byte[PIC_WIDTH];
	    byte[] lineYHH  = new byte[PIC_WIDTH];
	    byte[] lineC    = new byte[PIC_WIDTH];

	    for (int i = 0; i < PIC_HEIGHT; i++) {
		for (int j = 0; j < PIC_WIDTH; j++) {
		    lineYLL[j] = filterY[j][i];
		    lineU[j]   = filterU[j][i];
		    lineV[j]   = filterV[j][i];
		    lineYLH[j] = YLH[j][i];
		    lineYHL[j] = YHL[j][i];
		    lineYHH[j] = YHH[j][i];
		    lineC[j]   = filterC[j][i];
		}
		file.write(lineYLL);
		file.write(lineU);
		file.write(lineV);
		file.write(lineYLH);
		file.write(lineYHL);
		file.write(lineYHH);
		file.write(lineC);
	    }
	    file.close();

	} catch (Exception e) {
	    System.err.println(e);
	}
    }

    /**
     * Run length encoded cplane
     */
    public void writeCPlaneLogFile(String path)
    {
	try {
	    PrintWriter file = new PrintWriter(new FileOutputStream(path));
	    int start, size = 0;
	    String str = "";

	    for (int i = 0; i < PIC_HEIGHT; i++) {
		start = 0;
		for (int j = 1; j < PIC_WIDTH; j++) {

		    if (C[j][i] != C[j-1][i] || j == PIC_WIDTH - 1) {

			if(j == PIC_WIDTH - 1) j++;

			str += (j-start)+" " + 
			    icConstant.byte2UInt(C[j-1][i])+" ";
			size++;
			start = j;
		    }
		}
	    }
	    file.println(icConstant.CPLANE_VERSION);
	    file.println(size*2 + " " + str + "-226800 0 0");
	    file.close();

	} catch (Exception e) {
	    System.err.println(e);
	}
    }

    public void display()
    {
	paintComponent(getGraphics());
    }

    public void paintComponent(Graphics g)
    {

	SCALE = Math.min(getWidth()/PIC_WIDTH, getHeight()/PIC_HEIGHT);

	//recreate offscreen buffers only if the scale has changed
	if(SCALE != OLDSCALE) {
		bi = createImage(getWidth(), getHeight());
		big = bi.getGraphics();
		OLDSCALE = SCALE;
	}

	//set scale twice as big if we are zoomed on a quadrant
	if(quadrant != 0)
	    SCALE *= 2;

	for (int x = STARTX; x < ENDX; x++)
	    for (int y = STARTY; y < ENDY; y++) {

		drawSquare(x, y, big);

		/*
		drawSquare(2*(x - STARTX), 2*(y - STARTY), SCALE, 
			   new Color(rgbPlane[2*x][2*y]), big);
		drawSquare(2*(x - STARTX)+1, 2*(y - STARTY), SCALE, 
			   new Color(rgbPlane[2*x+1][2*y]), big);
		drawSquare(2*(x - STARTX), 2*(y - STARTY)+1, SCALE, 
			   new Color(rgbPlane[2*x][2*y+1]), big);
		drawSquare(2*(x - STARTX)+1, 2*(y - STARTY)+1, SCALE, 
			   new Color(rgbPlane[2*x+1][2*y+1]), big);
		*/

		//if pixel is classified draw marking box
		if ((color == icConstant.COLOR_ALL && 
		     C[x][y] != icConstant.COLOR_NONE) ||
		    (color != icConstant.COLOR_NONE && C[x][y] == color))
		    drawBox(x, y, big);
	    }

	//draw buffered image to screen
	g.drawImage(bi, 0, 0, this);
    }

    /**
     * Draw pixel in correct rgb color
     *
     * @param  x     Row index
     * @param  y     Column index
     * @param  picg  Graphics area of the current panel
     */
    public void drawSquare(int x, int y, Graphics picg)
    {
	int  xd  = x - STARTX;
	int  yd  = y - STARTY;

	Color c = null;
	int sign = YLL[x][y]&0x80;
	int y11 = (YLL[x][y]&0xff);

	if(!isFilter)
	    switch(color) {
	    case icConstant.PLANE_INVHSB:
		c = icColor.RGBtoInvertedHSB(rgbPlane[x][y]);
		break;
	    case icConstant.PLANE_GRAYHSB:
		c = icColor.RGBtoGrayHSB(rgbPlane[x][y]);
		break;
	    case icConstant.PLANE_YUVHSF:

		byte y01 = icColor.addSign(y11 + (YLH[x][y]&0xff), sign);
		byte y10 = icColor.addSign(y11 + (YHL[x][y]&0xff), sign);
		byte y00 = icColor.addSign(y11 + (YHH[x][y]&0xff), sign);

		c = icColor.YUVtoRGB((byte) ((YLL[x][y]+y01+y10+y00)/4), 
				     U[x][y], V[x][y]);
		break;
	    case icConstant.PLANE_YLL:
		c = icColor.YtoRGB(YLL[x][y]);
		break;
	    case icConstant.PLANE_YLH:
		c = icColor.YtoRGB(icColor.addSign(y11+YLH[x][y]&0xff, sign));
		break;
	    case icConstant.PLANE_YHL:
		c = icColor.YtoRGB(icColor.addSign(y11+YHL[x][y]&0xff, sign));
		break;
	    case icConstant.PLANE_YHH:
		c = icColor.YtoRGB(icColor.addSign(y11+YHH[x][y]&0xff, sign));
		break;
	    case icConstant.PLANE_U:
		c = icColor.UtoRGB(YLL[x][y], U[x][y]);
		break;
	    case icConstant.PLANE_V:
		c = icColor.VtoRGB(YLL[x][y], V[x][y]);
		break;
	    case icConstant.PLANE_C:
		c = icColor.getCPlaneColor(C[x][y]);
		break;
	    default:
		c = new Color(rgbPlane[x][y]);
		break;
	    }
	else {
	    switch(filterResult) {
	    case icConstant.FILTER_RES_C:
		c = icColor.getCPlaneColor(filterC[x][y]);
		break;
	    case icConstant.FILTER_RES_Y:
		c = icColor.YtoRGB(filterY[x][y]);
		break;
	    case icConstant.FILTER_RES_U:
		c = icColor.UtoRGB(filterY[x][y], filterU[x][y]);
		break;
	    case icConstant.FILTER_RES_V:
		c = icColor.VtoRGB(filterY[x][y], filterV[x][y]);
		break;
	    case icConstant.FILTER_RES_YUV:
		c = new Color(filterRGB[x][y]);
		break;
	    }
	}

	picg.setColor(c);
	picg.fillRect(xd * SCALE, yd * SCALE, SCALE, SCALE);
	picg.drawRect(xd * SCALE, yd * SCALE, SCALE, SCALE);

	//Draw edge as line
	if ((edgeMask & rgbOverlay[x][y]) != 0) {
	    picg.setColor(Color.magenta);
	    picg.drawLine(xd * SCALE, yd * SCALE + SCALE / 2, 
			  (xd + 1) * SCALE, yd * SCALE + SCALE / 2);
	}
    }


    /**
     * Draw inverted HSB box representing cplane classification
     */
    public void drawBox(int x, int y, Graphics picg)
    {
	int  xd  = x - STARTX;
	int  yd  = y - STARTY;

	//picg.setColor((new Color(rgbPlane[x][y] ^ 0x0FFFFF)));
	picg.setColor(icColor.RGBtoInvertedHSB(rgbPlane[x][y]));
	picg.drawRect(xd * SCALE + 1, yd * SCALE + 1, SCALE - 2, SCALE - 2);
    }

    public void applyFilter(int filter)
    {
	if(!isFilter) //Shouldn't happen
	    return;

	if(filter==icConstant.FILTER_NONE) {
	    icFilter.copy(YLL, filterY);
	    icFilter.copy(U, filterU);
	    icFilter.copy(V, filterV);
	}
	else if(filter==icConstant.FILTER_EQUALALL) {
	    icFilter.applyFilter(filterY, icConstant.FILTER_EQUAL);
	    icFilter.applyFilter(filterU, icConstant.FILTER_EQUAL);
	    icFilter.applyFilter(filterV, icConstant.FILTER_EQUAL);
	}
	else
	    switch(filterPlane)
		{
		case icConstant.FILTER_CPLANE:
		    icFilter.applyFilter(filterC, filter);
		    display();
		    return;
		case icConstant.FILTER_YPLANE:
		    icFilter.applyFilter(filterY, filter);
		    break;
		case icConstant.FILTER_UPLANE:
		    icFilter.applyFilter(filterU, filter);
		    break;
		case icConstant.FILTER_VPLANE:
		    icFilter.applyFilter(filterV, filter);
		    break;
		}

	byte y, u, v;

	for (int i = 0; i < PIC_HEIGHT; i++)
	    for (int j = 0; j < PIC_WIDTH; j++) {
		//filterC[j][i] = colorCube[(filterY[j][i])][(U[j][i])][(V[j][i])];
		y = filterY[j][i]; u = filterU[j][i]; v = filterV[j][i];

		filterRGB[j][i] = icColor.YUVtoRGB(y, u, v).getRGB();
		filterC[j][i]   = 
		    icCal.getCPlane(icConstant.byte2UInt(y)/FACTOR,
				    icConstant.byte2UInt(u)/FACTOR, 
				    icConstant.byte2UInt(v)/FACTOR);
	    }
	display();
    }

    public static void main(String[] args)
    {
	if(args.length > 1) {
	    System.err.println("java icBasicBFLPanel <file>");
	    System.exit(1);
	}

	try {
	    JFrame frame = new JFrame("icBasicBFLPanel");
	    frame.setSize(550, 480);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	    icBasicBFLPanel bflPane = new icBasicBFLPanel();
	    frame.getContentPane().add(bflPane);
	    frame.setVisible(true);

	    if(args.length == 1)
		bflPane.readFile(args[0]);

	} catch (Exception e) {
	    System.err.println(e);
	}
    }
}

