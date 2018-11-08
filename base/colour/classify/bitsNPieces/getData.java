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

public class getData
{
    public static final int WIDTH = 208;
    public static final int HEIGHT = 160;

    protected static byte y[] = new byte[WIDTH];
    protected static byte u[] = new byte[WIDTH];
    protected static byte v[] = new byte[WIDTH];
    protected static byte rest[] = new byte[4*WIDTH];

    protected static boolean isNumeric = false;
    protected static boolean isSlice   = false;

    public static String usage = "Usage: java getData -[ns] <image directory>";

    public static int byte2UInt(byte b)
    {
	int i,s;

	s = (b >> 7) & 0x01; 		// save the sign bit
	b &= 0x7f; 			// strip the sign bit
	i = (((int) b) | (s<<7)); 	// reassemble number

	return i;
    }

    public static void main(String args[])
    {
	if (args.length < 1 || args.length > 2) {
	    System.err.println(usage);
	    return;
	}

	String path;

	if(args[0].charAt(0) == '-') { //options

	    if(args.length != 2) {
		System.err.println(usage);
		return;
	    }

	    for(int i=1; i<args[0].length(); i++) {
		if(args[0].charAt(i) == 'n')
		    isNumeric = true;
		else if(args[0].charAt(i) == 's')
		    isSlice = true;
		else {
		    System.err.println("Unknown option: "+args[0].charAt(i));
		    return;
		}
	    }
	    path = args[1];
	}
	else {
	    if(args.length != 1) {
		System.err.println(usage);
		return;
	    }
	    path = args[0];
	    isSlice = true; //output slices by default if no options selected
	}

	File dir = new File(path);
	if (!dir.isDirectory()) {
	    System.err.println(path+" is not a directory");
	    return;
	}
	path += File.separator;

	String[] fileList = dir.list();
	String name;

	try {
	    FileInputStream in = null;

	    PrintWriter outY = null;
	    PrintWriter outU = null;
	    PrintWriter outV = null;

	    PrintWriter picOutY = null;
	    PrintWriter picOutU = null;
	    PrintWriter picOutV = null;

	    if(isSlice) {
		outY = new PrintWriter(new FileWriter(path+"sliceY.xls"),true);
		outU = new PrintWriter(new FileWriter(path+"sliceU.xls"),true);
		outV = new PrintWriter(new FileWriter(path+"sliceV.xls"),true);

		outY.print(" \t"); outU.print(" \t"); outV.print(" \t");
		for(int i=0; i<WIDTH; i++) {
		    outY.print(i+"\t"); outU.print(i+"\t"); outV.print(i+"\t");
		}
		outY.println(); outU.println(); outV.println();
	    }

	    for(int k = 0; k < fileList.length; k++) {

		if(!fileList[k].toLowerCase().endsWith(".bfl"))
		    continue;

		name = path+fileList[k];
		in = new FileInputStream(name);
		System.out.println(name);
		name = name.substring(0,name.length()-4);

		if(isNumeric) {
		    picOutY = new PrintWriter(new FileWriter(name+"Y.xls"), true);
		    picOutU = new PrintWriter(new FileWriter(name+"U.xls"), true);
		    picOutV = new PrintWriter(new FileWriter(name+"V.xls"), true);
		}

		name = fileList[k].substring(0,fileList[k].length()-4);

		for (int i = 0; i < HEIGHT; i++) {

		    in.read(y);
		    in.read(u);
		    in.read(v);
		    in.read(rest);

		    if(isNumeric) {
			for (int j = 0; j < WIDTH; j++) {
			    picOutY.print(byte2UInt(y[j])+"\t");
			    picOutU.print(byte2UInt(u[j])+"\t");
			    picOutV.print(byte2UInt(v[j])+"\t");
			}
			picOutY.println();
			picOutU.println();
			picOutV.println();
		    }

		    if(isSlice && (i==79 || i==80)) {
			outY.print(name+" "+i+"\t");
			outU.print(name+" "+i+"\t");
			outV.print(name+" "+i+"\t");

			for (int j = 0; j < WIDTH; j++) {
			    outY.print(byte2UInt(y[j])+"\t");
			    outU.print(byte2UInt(u[j])+"\t");
			    outV.print(byte2UInt(v[j])+"\t");
			}
			outY.println();
			outU.println();
			outV.println();
		    }
		}
		in.close();
		if(isNumeric) {
		    picOutY.close();
		    picOutU.close();
		    picOutV.close();
		}
	    }
	} catch (Exception e) {
	    System.out.println(e);
	}
    }
}
