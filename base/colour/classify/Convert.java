/*

Copyright 2003 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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

class Convert
{
    static final int WIDTH = 208;
    static final int HEIGHT = 160-1;

    public static byte y[] = new byte[WIDTH];
    public static byte u[] = new byte[WIDTH];
    public static byte v[] = new byte[WIDTH];
    public static byte dummy[] = new byte[WIDTH];
    public static byte c[] = new byte[WIDTH];

    // see base/colour/ic/BFLPanel.java for list of classified 
    // colours and the corresponding numbers
    public static final int NONE = 127;

    public static void main (String args[])
    {
	FileInputStream in;

	if (args.length != 2) {
	    System.out.println("Usage: java Convert directory output_file");
	    System.exit(1);
	}

	try {
	    PrintWriter fileOut = null;
	    fileOut = new PrintWriter(new FileWriter(new File(args[1])));

	    File f = new File(args[0]); //make sure dir has target images only
	    if (f.isDirectory()) {
		String[] fileList = f.list();
		for(int i=0; i< fileList.length; i++) {
		    process(new FileInputStream(args[0] + File.separator + fileList[i]), fileOut);
		}
	    }
	    else {
		System.out.println("Error: " + args[0] + " is not a directory");
		System.exit(1);
	    }

	    if (fileOut != null) {
		fileOut.close();
		System.out.println("Conversion of BFL Images into ASCII done successfully");
	    }
	} catch (Exception e) {
	    System.out.println("Error: opening file " + args[0]);
	    System.exit(1);
	}
    }

    public static int byte2UInt(byte b)
    {
	int i,s;

	s = (b >> 7) & 0x01; 		// save the sign bit
	b &= 0x7f; 			// strip the sign bit
	i = (((int)b) | (s<<7)); 	// reassemble number

	return i;
    }


    public static boolean validColor (int c)
    {
	if (c >= 0 && c <= 10)
	    return true;

	return false;
    }

    public static void process (FileInputStream in, PrintWriter fileOut)
    {
	for (int i = 0; i < HEIGHT; i++) {
	    try {
		in.read(y);
		in.read(u);
		in.read(v);
		in.read(dummy);
		in.read(dummy);
		in.read(dummy);
		in.read(c);

		for (int j = 0; j < WIDTH; j++)	{
                    // need to keep none for compression
		    if (validColor(byte2UInt(c[j])) || byte2UInt(c[j]) == NONE) {
			fileOut.print(byte2UInt(y[j]) + " ");
			fileOut.print(byte2UInt(u[j]) + " ");
			fileOut.print(byte2UInt(v[j]) + " ");
			fileOut.println(byte2UInt(c[j]));
		    }
		    //else
		    //fileOut.println(NONE);
		}
	    } catch (Exception e) {
		System.out.println("Error: reading file");
		System.exit(1);
	    }
	}
    }
}
