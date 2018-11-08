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
import java.awt.Color;

public class ppm2bfl {

    static final int PIC_WIDTH = 208;
    static final int PIC_HEIGHT = 160;
    
    static final int MAX_PPM_VAL = 255;
    static String PPM_MAGIC = "P6";

	static final int[][] rawColors = {{255, 127, 0}, {0, 255, 255}, {0, 178, 0}, {255, 255, 0}, {255, 175, 175}, {0, 0, 255},
			{255, 0, 0}, {0, 255, 0}, {64, 64, 64}, /*{0, 0, 0},*/ {255, 255, 255} /*, {192, 192, 192}*/};

    static Color[] colors;
                            
    public static void main(String[] args) {
    	colors = new Color[rawColors.length];
    	for (int i=0; i<rawColors.length; i++) {
    		colors[i] = new Color(rawColors[i][0], rawColors[i][1], rawColors[i][2]);
    	}
    
		readFile(args[0]);
	}

	static void skipToSpace(InputStream s) throws IOException {
		int b;
		do {
			b = s.read();
		} while (b != ' ' && b != '\t' && b != '\r' && b != '\n' && b != -1);
	}
    
	static int skipPastSpace(InputStream s) throws IOException {
		int b = 0, prevb;
		do {
			prevb = b;
			b = s.read();
		} while (b == ' ' || b == '\t' || b == '\r' || b == '\n');
		
		while (b == '#' && prevb == '\n') {
			do {
				prevb = b;
				b = s.read();
			} while (prevb != '\n');
		}
		
		return b;
	}
	
	static String readNextWord(InputStream s) throws IOException {
		StringBuffer buf = new StringBuffer();
		
		int b = '\n', prevb;
		
		// skip spaces
		
		do {
			prevb = b;
			b = s.read();
		} while (b == ' ' || b == '\t' || b == '\r' || b == '\n');
		
		while (b == '#' && prevb == '\n') {
			do {
				prevb = b;
				b = s.read();
			} while (prevb != '\n');
		}
		
		while (b != ' ' && b != '\t' && b != '\r' && b != '\n' && b != -1) {
			buf.append((char)b);
			b = s.read();
		}
		
		// System.out.println("read: " + buf);
		
		return buf.toString();
	}
    
    static double dist(Color c, int r, int g, int b) {
    	int dr = c.getRed() - r;
    	int dg = c.getGreen() - g;
    	int db = c.getBlue() - b;
    	
    	return Math.sqrt(dr*dr + dg*dg + db*db);
    }
    
    static void readFile(String inputFileName) {
		FileInputStream YUVPPM_file = null;
		FileInputStream CLASSPPM_file = null;

		byte[] lineY = new byte[PIC_WIDTH];
		byte[] lineU = new byte[PIC_WIDTH];
		byte[] lineV = new byte[PIC_WIDTH];
		byte[] lineC = new byte[PIC_WIDTH];
		
		try {
			try {
				YUVPPM_file = new FileInputStream(inputFileName + ".yuv.ppm");
				CLASSPPM_file = new FileInputStream(inputFileName + ".c.ppm");
	
				InputStream YUVPPM = new BufferedInputStream( YUVPPM_file );
				InputStream CLASSPPM = new BufferedInputStream( CLASSPPM_file );
			
				if (!readNextWord(YUVPPM).equals(PPM_MAGIC))	// read MAGIC
					throw new RuntimeException("Bad Magic number in YUV PPM file");
				if (!readNextWord(CLASSPPM).equals(PPM_MAGIC))
					throw new RuntimeException("Bad Magic number in CLASS PPM file");
			
				int yuvwidth = Integer.parseInt(readNextWord(YUVPPM));	// width
				int classwidth = Integer.parseInt(readNextWord(CLASSPPM));
	
				if (classwidth != yuvwidth) {
					throw new RuntimeException("labels and images different widths");
				}
			
				int numPicWidth = classwidth/PIC_WIDTH;
				if (numPicWidth * PIC_WIDTH != classwidth) {
					throw new RuntimeException("not an integral number of images wide");
				}
			
				int yuvheight = Integer.parseInt(readNextWord(YUVPPM));	// height
				int classheight = Integer.parseInt(readNextWord(CLASSPPM));
	
				if (yuvheight != classheight) {
					throw new RuntimeException("labels and images different heights");
				}
			
				int numPicHeight = classheight/(PIC_HEIGHT-1);
				if (numPicWidth * PIC_WIDTH != classwidth) {
					throw new RuntimeException("not an integral number of images wide");
				}
			
				if (Integer.parseInt(readNextWord(YUVPPM)) != MAX_PPM_VAL)
					throw new RuntimeException("Bad max pixel size");
				if (Integer.parseInt(readNextWord(CLASSPPM)) != MAX_PPM_VAL)
					throw new RuntimeException("Bad max pixel size");
				
				int n=0;
				
				for (int j=0; j< numPicHeight; j++, n += numPicWidth) {
					FileOutputStream[] BFL = new FileOutputStream[numPicWidth];
			
					try {
						for (int i=0; i<numPicWidth; i++) {
							BFL[i] = new FileOutputStream(inputFileName+"-"+(n+i)+".bfl");
						}
						
						for(int y=0; y < PIC_HEIGHT-1; y++) {
				
							for (int i=0; i<numPicWidth; i++) {
				
								for (int x=0; x < PIC_WIDTH ; x++) {
									
									int pix_y = YUVPPM.read() & 0xff;
									int pix_u = YUVPPM.read() & 0xff;
									int pix_v = YUVPPM.read() & 0xff;
									int pix_cr = CLASSPPM.read() & 0xff;
									int pix_cg = CLASSPPM.read() & 0xff;
									int pix_cb = CLASSPPM.read() & 0xff;
									
									int best_c = 255;
									
									for (int col=0; col<colors.length; col++) {
										if (rawColors[col][0] == pix_cr &&
											rawColors[col][1] == pix_cg &&
											rawColors[col][2] == pix_cb) {
											best_c = col;
										}
									}
	
									lineY[x] = (byte)pix_y;
									lineU[x] = (byte)pix_u;
									lineV[x] = (byte)pix_v;
									lineC[x] = (byte)best_c;
								}
								BFL[i].write(lineY);
								BFL[i].write(lineU);
								BFL[i].write(lineV);
								BFL[i].write(lineC);
							}
							
						}
						
						// duplicate the last line
						for (int i=0; i<numPicWidth; i++) {
							BFL[i].write(lineY);
							BFL[i].write(lineU);
							BFL[i].write(lineV);
							BFL[i].write(lineC);
							BFL[i].flush();
						}
					} finally {
						for (int i=0; i<numPicWidth; i++) {
							if (BFL[i] != null)
								BFL[i].close();
						}
					}
				}
			} finally {
				if (YUVPPM_file != null)
					YUVPPM_file.close();
				if (CLASSPPM_file != null)
					CLASSPPM_file.close();
			}
		} catch (FileNotFoundException ex) {
			System.out.println("file not found: " + ex);
		} catch (IOException ex) {
			System.out.println("an IOException occured: " + ex);
		}
    }

}
