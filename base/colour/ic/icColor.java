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
 * ic color constants and static methods
 */
public abstract class icColor
{

    //colors 0 to 10
    public static Color BALL_ORANGE  = Color.orange;
    public static Color GOAL_BLUE    = Color.cyan;     //aqua 0 0 255
    public static Color BEACON_GREEN = Color.green.darker(); //0 128 0
    public static Color GOAL_YELLOW  = Color.yellow;   //255 255 0
    public static Color BEACON_PINK  = Color.pink;
    public static Color ROBOT_BLUE   = new Color(0, 0, 128); //navy
    public static Color ROBOT_RED    = Color.red;
    public static Color FIELD_GREEN  = Color.green;    //lime 0, 255, 0
    public static Color ROBOT_GREY   = Color.gray;     //gray 128 128 128
    public static Color ROBOT_WHITE  = Color.white;    //255 255 255
    public static Color ROBOT_BLACK  = Color.black;    //0 0 0

    //colors 11 to 15
    public static Color FUCHSIA      = Color.magenta;   //255 0 255
    public static Color MAROON       = new Color(128, 0, 0);
    public static Color OLIVE        = new Color(128, 128, 0);
    public static Color PURPLE       = new Color(128, 128, 0);
    public static Color TEAL         = new Color(0, 128, 128);

    public static Color BLUE         = Color.blue;      // 0 0 255
    public static Color SILVER       = Color.lightGray; //silver 192 192 192

    public static byte MAYBY_BIT = 0x10;

    public static Color getCPlaneColor(byte c)
    {
	switch(c & ~MAYBY_BIT) {
	case icConstant.COLOR_ORANGE:
	    return BALL_ORANGE;
	case icConstant.COLOR_GOALBLUE:
	    return GOAL_BLUE;
	case icConstant.COLOR_GREEN:
	    return BEACON_GREEN;
	case icConstant.COLOR_YELLOW:
	    return GOAL_YELLOW;
	case icConstant.COLOR_PINK:
	    return BEACON_PINK;
	case icConstant.COLOR_ROBOTBLUE:
	    return ROBOT_BLUE;
	case icConstant.COLOR_ROBOTRED:
	    return ROBOT_RED;
	case icConstant.COLOR_FIELDGREEN:
	    return FIELD_GREEN;
	case icConstant.COLOR_ROBOTGREY:
	    return ROBOT_GREY;
	case icConstant.COLOR_WHITE:
	    return ROBOT_WHITE;
	case icConstant.COLOR_BLACK:
	    return ROBOT_BLACK;
	case icConstant.COLOR_FUCHSIA:
	    return FUCHSIA;
 	case icConstant.COLOR_MAROON:
	    return MAROON;
	case icConstant.COLOR_OLIVE:
	    return OLIVE;
	case icConstant.COLOR_PURPLE:
	    return PURPLE;
	case icConstant.COLOR_TEAL:
	    return TEAL;
	default:
	    return SILVER;
	}
    }

    public static Color YUVtoRGB(byte yp, byte up, byte vp)
    {
	int y = icConstant.byte2UInt(yp);
	int u = icConstant.byte2UInt(up);
	int v = icConstant.byte2UInt(vp);

	//change range of u and v for rgb conversion
	u -= 128;
	v -= 128;

	int  r  = clip((int) ((1.164 * y) + (1.596 * u)));
	int  g  = clip((int) ((1.164 * y) - (0.813 * u) - (0.391 * v)));
	int  b  = clip((int) ((1.164 * y) + (1.596 * v)));

	return new Color((r << 16) | (g << 8) | b);
    }

    public static Color YtoRGB(byte yp)
    {
	int y = yp & 0xff;

	int  r  = clip((int) (1.164 * y));
	int  g  = clip((int) (1.164 * y));
	int  b  = clip((int) (1.164 * y));

	return new Color((r << 16) | (g << 8) | b);
    }

    public static Color UtoRGB(byte yp, byte up)
    {
	/*
	int y = yp & 0xff;
	int u = up & 0xff - 128;

	int  r  = clip((int) ((1.164 * y) + (1.596 * u)));
	int  g  = clip((int) ((1.164 * y) - (0.813 * u)));
	int  b  = clip((int)  (1.164 * y));

	return new Color((r << 16) | (g << 8) | b);
	*/
	return YtoRGB(up);
    }

    public static Color VtoRGB(byte yp, byte vp)
    {
	/*
	int y = yp & 0xff;
	int v = vp & 0xff - 128;

	int  r  = clip((int)  (1.164 * y));
	int  g  = clip((int) ((1.164 * y) - (0.391 * v)));
	int  b  = clip((int) ((1.164 * y) + (1.596 * v)));

	return new Color((r << 16) | (g << 8) | b);
	*/
	return YtoRGB(vp);
    }

    public static Color RGBtoInvertedHSB(int rgb)
    {
	float hsb[] = Color.RGBtoHSB((rgb >>> 16) & 0x0FF, 
				     (rgb >>> 8)  & 0x0FF,
				      rgb         & 0x0FF, null);
	float hue = (float) 0.5 + hsb[0];
	if (hue > (float) 1.0)
	    hue -= (float) 1.0;
	return Color.getHSBColor((float) hue, (float) 1.0, (float) 1.0);
    }

    public static Color RGBtoGrayHSB(int rgb)
    {
	float hsb[] = Color.RGBtoHSB((rgb >>> 16) & 0x0FF, 
				     (rgb >>> 8)  & 0x0FF,
				      rgb         & 0x0FF, null);
	return Color.getHSBColor((float) 0.0, (float) 0.0, hsb[2]);
    }

    public static double[] YUVtoLAB(byte yp, byte up, byte vp)
    {
	Color  rgb   = YUVtoRGB(yp, up, vp);
	double xyz[] = RGBtoXYZ(rgb.getRed(), rgb.getGreen(), rgb.getBlue());
	double lab[] = XYZtoLAB(xyz[0], xyz[1], xyz[2]);
	return lab;
    }

    public static int[] LABtoRGB(double L, double a, double b)
    {
	double xyz[] = LABtoXYZ(L, a, b);
	int    rgb[] = XYZtoRGB(xyz[0], xyz[1], xyz[2]);
	return rgb;
    }

    public static int[] YCrCbtoRGB(byte yp, byte up, byte vp)
    {
	int y  = icConstant.byte2UInt(yp);
	int cr = icConstant.byte2UInt(up) - 128;
	int cb = icConstant.byte2UInt(vp) - 128;

	y  = clip(y + 1.140 * cr);
	cr = clip(y - 0.394 * cr - 0.581*cb);
	cb = clip(y + 2.028 * cb);

	int[] p = new int[3];
	p[0] = y; p[1] = cr; p[2] = cb;
	return p;
    }

    public static double[] RGBtoXYZ(int r, int g, int b)
    {
	double x = r/255; double y = g/255; double z = b/255;

	x = (x > 0.04045) ? Math.pow((x + 0.055)/1.055, 2.4) : (x/12.92);
	y = (y > 0.04045) ? Math.pow((y + 0.055)/1.055, 2.4) : (y/12.92);
	z = (z > 0.04045) ? Math.pow((z + 0.055)/1.055, 2.4) : (z/12.92);

	x *= 100; y *= 100; z *= 100;

	x = x*0.4124 + y*0.3576 + z*0.1805;
	y = x*0.2126 + y*0.7152 + z*0.0722;
	z = x*0.0193 + y*0.1192 + z*0.9505;

	double p[] = new double[3];
	p[0] = x; p[1] = y; p[2] = z;
	return p;
    }

    public static int[] XYZtoRGB(double x, double y, double z)
    {
	x /= 100;
	y /= 100;
	z /= 100;

	double r = x *  3.2406 + y * -1.5372 + z * -0.4986;
	double g = x * -0.9689 + y *  1.8758 + z *  0.0415;
	double b = x *  0.0557 + y * -0.2040 + z *  1.0570;

	r = (r > 0.0031308) ? 1.055 * Math.pow(r,1/2.4) - 0.055 : 12.92 * r;
	g = (g > 0.0031308) ? 1.055 * Math.pow(g,1/2.4) - 0.055 : 12.92 * g;
	b = (b > 0.0031308) ? 1.055 * Math.pow(b,1/2.4) - 0.055 : 12.92 * b;

	int p[] = new int[3];
	p[0] = clip(r * 255); p[1] = clip(g * 255); p[2] = clip(b * 255);
	return p;
    }

    public static double[] XYZtoLAB(double x, double y, double z)
    {
	double L = x/95.047; double a = y/100.000; double b = z/108.883;

	L = (x > 0.008856) ? Math.pow(L, 1/3) : (7.787*L + 16/116);
	a = (y > 0.008856) ? Math.pow(a, 1/3) : (7.787*a + 16/116);
	b = (z > 0.008856) ? Math.pow(b, 1/3) : (7.787*b + 16/116);

	L = 116*a - 16;
	a = 500*L - a;
	b = 200*a - b;

	double p[] = new double[3];
	p[0] = L; p[1] = a; p[2] = b;
	return p;
    }

    public static double[] LABtoXYZ(double L, double a, double b)
    {
	double y = ( L + 16 ) / 116;
	double x = a / 500 + y;
	double z = y - b / 200;

	double y3 = Math.pow(y,3);
	double x3 = Math.pow(x,3);
	double z3 = Math.pow(z,3);

	y = (y3 > 0.008856) ? y3 : (y - 16/116) / 7.787;
	x = (x3 > 0.008856) ? x3 : (x - 16/116) / 7.787;
	z = (z3 > 0.008856) ? z3 : (z - 16/116) / 7.787;

	x *= 95.047; //Observer= 2 degrees, Illuminant= D65
	y *= 100.000;
	z *= 108.883;

	double p[] = new double[3];
	p[0] = x; p[1] = y; p[2] = z;
	return p;
    }

    public static int clip(int val)
    {
	if (val < 0)
	    return 0;
	else if (val > 255)
	    return 255;
	return val;
    }

    public static int clip(double val)
    {
	if (val < 0)
	    return 0;
	else if (val > 255)
	    return 255;
	return (int) Math.round(val);
    }

    public static byte addSign(int val, int sign)
    {
	if(sign == 0) //+ve
	    return (byte) (val + 128);
	else //-ve
	    return (byte) (val - 128);
    }
}
