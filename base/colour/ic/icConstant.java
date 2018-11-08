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

public abstract class icConstant
{
    //Old dog resolution
    //public final static int PIC_WIDTH              =  176;
    //public final static int PIC_HEIGHT             =  144;

    //New dog resolution
    public final static int PIC_WIDTH              =  208;
    public final static int PIC_HEIGHT             =  160;

    public final static int  FACTOR                =  2; //Compression factor

    public final static byte MIN_VALUE             =  uInt2Byte(0);
    public final static byte MAX_VALUE             =  uInt2Byte(255);

    public final static String CPLANE_VERSION      =  "v1206"; //random string

    public static boolean DEBUG                    =  false;
    //Colors

    public final static byte COLOR                 =  0;
    public final static byte COLOR_ORANGE          =  0;
    public final static byte COLOR_GOALBLUE        =  1;
    public final static byte COLOR_GREEN           =  2;
    public final static byte COLOR_YELLOW          =  3;
    public final static byte COLOR_PINK            =  4;
    public final static byte COLOR_ROBOTBLUE       =  5;
    public final static byte COLOR_ROBOTRED        =  6;
    public final static byte COLOR_FIELDGREEN      =  7;
    public final static byte COLOR_ROBOTGREY       =  8;
    public final static byte COLOR_WHITE           =  9;
    public final static byte COLOR_BLACK           =  10;
    public final static byte COLOR_FUCHSIA         =  11;
    public final static byte COLOR_MAROON          =  12;
    public final static byte COLOR_OLIVE           =  13;
    public final static byte COLOR_PURPLE          =  14;
    public final static byte COLOR_TEAL            =  15;
    public final static byte COLOR_ALL             =  16;
    public final static byte COLOR_LAST_USABLE     =  COLOR_BLACK;
    public final static byte COLOR_END             =  COLOR_ALL;
    public final static byte COLOR_LAST_LABEL      =  COLOR_BLACK+1;

    public final static byte COLOR_NONE            =  127;

    public final static byte PLANE                 =  100;
    public final static byte PLANE_INVHSB          =  PLANE + 0;
    public final static byte PLANE_GRAYHSB         =  PLANE + 1;
    public final static byte PLANE_YUVHSF          =  PLANE + 2;
    public final static byte PLANE_YLL             =  PLANE + 3;
    public final static byte PLANE_YLH             =  PLANE + 4;
    public final static byte PLANE_YHL             =  PLANE + 5;
    public final static byte PLANE_YHH             =  PLANE + 6;
    public final static byte PLANE_U               =  PLANE + 7;
    public final static byte PLANE_V               =  PLANE + 8;
    public final static byte PLANE_C               =  PLANE + 9;
    public final static byte PLANE_FILTERRGB       =  PLANE + 10;
    public final static byte PLANE_FILTERC         =  PLANE + 11;
    public final static byte PLANE_END             =  PLANE_FILTERC;


    //Tools

    public static final int TOOL                   =  200;
    public static final int TOOL_BRUSH             =  TOOL + 0;
    public static final int TOOL_LINE              =  TOOL + 1;
    public static final int TOOL_FILL              =  TOOL + 2;
    public static final int TOOL_WAND              =  TOOL + 3;
    public static final int TOOL_ZOOM              =  TOOL + 4;
    public static final int TOOL_CLEAR             =  TOOL + 5;
    public static final int TOOL_UNDO              =  TOOL + 6;
    public static final int TOOL_END               =  TOOL_ZOOM;


    public final static int FILE                   =  300;
    public final static int FILE_OPEN              =  FILE + 0;
    public final static int FILE_SAVE              =  FILE + 1;
    public final static int FILE_SAVEAS            =  FILE + 2;
    public final static int FILE_EXIT              =  FILE + 3;
    public final static int FILE_END               =  FILE_EXIT;

    public final static int VIEW                   =  400;
    public final static int VIEW_COLOR             =  VIEW + 0;
    public final static int VIEW_PLANE             =  VIEW + 1;
    public final static int VIEW_TOOL              =  VIEW + 2;
    public final static int VIEW_FILTER            =  VIEW + 3;
    public final static int VIEW_EDGE              =  VIEW + 4;
    public final static int VIEW_END               =  VIEW_EDGE;


    //Filters

    public final static int FILTER                 =  500;
    public final static int FILTER_NONE            =  FILTER + 0;
    public final static int FILTER_SHARPEN         =  FILTER + 1;
    public final static int FILTER_SMOOTH          =  FILTER + 2;
    public final static int FILTER_SOBEL           =  FILTER + 3;
    public final static int FILTER_ROBERTS         =  FILTER + 4;

    public final static int FILTER_ERODE           =  FILTER + 5;
    public final static int FILTER_DILATE          =  FILTER + 6;
    public final static int FILTER_OPEN            =  FILTER + 7;
    public final static int FILTER_CLOSE           =  FILTER + 8;

    public final static int FILTER_ROBOC           =  FILTER + 9;
    public final static int FILTER_ADDBLUE         =  FILTER + 10;
    public final static int FILTER_EQUAL           =  FILTER + 11;
    public final static int FILTER_EQUALALL        =  FILTER + 12;

    public final static int FILTER_LAB             =  FILTER + 13;

    public final static int FILTER_END             =  FILTER_LAB;

    public final static int FILTER_PLANE           =  600;
    public final static int FILTER_YPLANE          =  FILTER_PLANE + 0;
    public final static int FILTER_UPLANE          =  FILTER_PLANE + 1;
    public final static int FILTER_VPLANE          =  FILTER_PLANE + 2;
    public final static int FILTER_CPLANE          =  FILTER_PLANE + 3;
    public final static int FILTER_PLANE_END       =  FILTER_CPLANE;

    public final static int FILTER_RESULT          =  700;
    public final static int FILTER_RES_Y           =  FILTER_RESULT + 0;
    public final static int FILTER_RES_U           =  FILTER_RESULT + 1;
    public final static int FILTER_RES_V           =  FILTER_RESULT + 2;
    public final static int FILTER_RES_YUV         =  FILTER_RESULT + 3;
    public final static int FILTER_RES_C           =  FILTER_RESULT + 4;
    public final static int FILTER_RESULT_END      =  FILTER_RES_C;

    public final static int EDGE_PLANE             =  800;
    public final static int EDGE_YPLANE_HI         =  EDGE_PLANE + 0;
    public final static int EDGE_UPLANE_HI         =  EDGE_PLANE + 1;
    public final static int EDGE_VPLANE_HI         =  EDGE_PLANE + 2;
    public final static int EDGE_CPLANE            =  EDGE_PLANE + 3;
    public final static int EDGE_YPLANE_LO         =  EDGE_PLANE + 4;
    public final static int EDGE_UPLANE_LO         =  EDGE_PLANE + 5;
    public final static int EDGE_VPLANE_LO         =  EDGE_PLANE + 6;
    public final static int EDGE_COMBO             =  EDGE_PLANE + 7;
    public final static int EDGE_PLANE_END         =  EDGE_COMBO;

    public final static int CHANGE_PLANE           =  900;


    //Names

    public final static String ColorName[] = 
           {"Orange", "Blue", "Green", "Yellow", "Pink", 
	    "Blue Dog", "Red Dog", 
	    "Field Green", "Robot Grey", "White", "Black", 
	    "All", "None"};

    public final static String ColorNameNoSpace[] = 
           {"Orange", "Blue", "Green", "Yellow", "Pink", 
	    "Blue-Dog", "Red-Dog", 
	    "Field-Green", "Robot-Grey", "White", "Black", 
	    "All", "None"};

    public final static String PlaneName[] = 
	   {"InvHSB", "GrayHSB", 
	    "YUVHPF", "YLL", "YLH", "YHL", "YHH", "U", "V", 
	    "C" };

    public final static String FileToolName[] = 
    { "Open...", "Save", "Save As..." };

    public final static String DrawToolName[] = 
        {"Brush", "bs",  //brush slider
	 "Line",  "Fill",
	 "Wand",  "ws",  //wand slider
	 "Zoom" };

    public final static String OtherToolName[] = 
    { "Clear", "Undo" };

    public final static String FilterName[] = 
        { "None", "",
	  "Sharpen", "Smooth", "Sobel", "Cross", "",
	  "Erode",   "Dilate", 
	  "Open",    "Close", "",
	  "Robot C", "Add Blue", "Equalise", "Equalise All", "CIE Lab" };

    public final static String FilterPlaneName[] = { "Y", "U", "V", "C" };

    public final static String FilterResultName[] = { "Y", "U", "V", "YUV", "C" };

    public final static String EdgePlaneName[] = 
    { "Y",  "U",  "V", "C",  
      "Y",  "U",  "V", "Combo" };

    public final static String MenuName[] = { "File", "View" };

    public final static String FileMenuName[][] = 
        { {"Open...", "O"},  {}, 
	  {"Save", "S"}, {"Save As...", "A"}, {}, 
	  {"Exit", "x"} };

    public final static String ViewMenuName[][] = 
        { {"Color", "C"}, {"Plane",  "P"}, {},
	  {"Tool",  "T"}, {}, 
	  {"Filter", "F"}, {"Edge", "E"} };


    //Methods

    public static boolean isColor(byte c)
    {
	return (c >= COLOR && c <= COLOR_END);
    }

    public static boolean isPlane(byte p)
    {
	return (p >= PLANE && p <= PLANE_END);
    }

    public static boolean isTool(int t)
    {
	return (t >= TOOL && t <= TOOL_END);
    }

    public static boolean isFilter(int f)
    {
	return (f >= FILTER && f <= FILTER_END);
    }

    public static boolean isFilterPlane(int p)
    {
	return (p >= FILTER_PLANE && p <= FILTER_PLANE_END);
    }

    public static boolean isFilterResult(int p)
    {
	return (p >= FILTER_RESULT && p <= FILTER_RESULT_END);
    }

    public static boolean isEdgePlane (int p)
    {
	return (p >= EDGE_PLANE && p <= EDGE_PLANE_END);
    }

    public static boolean isMenuFile(int f)
    {
	return (f >= FILE && f <= FILE_END);
    }

    public static boolean isMenuView(int v)
    {
	return (v >= VIEW && v <= VIEW_END);
    }

    public static int byte2UInt(byte b)
    {
	return b & 0xff;
    }

    //obsolete
    public static int byte2UIntOld(byte b)
    {
	int i,s;

	s = (b >> 7) & 0x01; 		// save the sign bit
	b &= 0x7f; 			// strip the sign bit
	i = (((int) b) | (s<<7)); 	// reassemble number

	return i;
    }

    public static byte uInt2Byte(int i)
    {
	return (byte) (i & 0xff);
    }

    //obsolete
    public static byte uInt2ByteOld(int i)
    {
	byte b;
	int s;

	s = (i >> 7) & 0x01; 	        // save the sign bit
	i &= 0x7f; 			// strip the sign bit
	b = (byte) (i | (s<<7)); 	// reassemble number

	return b;
    }

    public static boolean fitMask(int val, int mask)
    {
	return (val & mask) != 0;
    }

    public static int addMask(int val, int mask)
    {
	return val | mask;
    }

    public static int subtractMask(int val, int mask)
    {
	return val & ~mask;
    }

    public static void print(String title, byte[] array)
    {
	if(!DEBUG) return;

	System.out.println(title);
	for(int i=0; i < array.length; i++)
	    System.out.print(icConstant.byte2UInt(array[i]) + "\t");
	System.out.println();
    }

    public static void print(String title, int[] array)
    {
	if(!DEBUG) return;

	System.out.println(title);
	for(int i=0; i < array.length; i++)
	    System.out.print(array[i] + "\t");
	System.out.println();
    }
}
