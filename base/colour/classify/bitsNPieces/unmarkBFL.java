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

public class unmarkBFL {

	static final int PIC_WIDTH = 208;
    static final int PIC_HEIGHT = 160;

	public static void main(String[] args) {

		byte[] lineY = new byte[PIC_WIDTH];
		byte[] lineU = new byte[PIC_WIDTH];
		byte[] lineV = new byte[PIC_WIDTH];
		byte[] lineC = new byte[PIC_WIDTH];
		byte[] clearLineC = new byte[PIC_WIDTH];

		for (int i=0; i<clearLineC.length; i++) {
			clearLineC[i] = (byte)255;
		}

		for (int i=0; i< args.length; i++) {

			try {
				FileOutputStream out = null;
				FileInputStream in = null;
				
				File inFile = new File(args[i]);
				File cleanDirFile = new File(inFile.getParentFile(), "clean");
				File outFile = new File(cleanDirFile, inFile.getName());
				
				try {
					in = new FileInputStream(inFile);
					out = new FileOutputStream(outFile);
					
					for(int y=0; y < PIC_HEIGHT; y++) {
						in.read(lineY);
						in.read(lineU);
						in.read(lineV);
						in.read(lineC);

						out.write(lineY);
						out.write(lineU);
						out.write(lineV);
						out.write(clearLineC);
					}
					
				} finally {
					if (out != null)
						out.close();
					if (in != null)
						in.close();
				}
			} catch (IOException e) {
				throw new RuntimeException(e.toString());
			}
	
		}

	}

}
