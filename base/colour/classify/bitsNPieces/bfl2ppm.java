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

public class bfl2ppm {

    static final int PIC_WIDTH = 208;
    static final int PIC_HEIGHT = 160;
    
    static final int MAX_PPM_VAL = 255;
    static String PPM_MAGIC = "P6";

    static final Color[] colors = {Color.orange.darker(), Color.cyan, Color.green.darker(), Color.yellow, Color.pink, Color.blue,
                            Color.red, Color.green, Color.darkGray, Color.black, Color.white, Color.lightGray };
                            
    public static void main(String[] args) {
		readFile("pics", args, 5);
		makePalette("palette.ppm", 40);
	}

    
    static void readFile(String outputFileName, String[] inputFilename, int numPicWidth) {

		int fullWidth = PIC_WIDTH*numPicWidth;
		int numPicHeight = (int)Math.ceil(inputFilename.length/(double)numPicWidth);
		int fullHeight = (PIC_HEIGHT - 1) * numPicHeight;

		byte[] lineY = new byte[PIC_WIDTH];
		byte[] lineU = new byte[PIC_WIDTH];
		byte[] lineV = new byte[PIC_WIDTH];
		byte[] lineC = new byte[PIC_WIDTH];


		FileOutputStream YUVPPM_file = null;
		FileOutputStream RGBPPM_file = null;
		FileOutputStream CLASSPPM_file = null;

		try {
			try {
				YUVPPM_file = new FileOutputStream(outputFileName + ".yuv.ppm");
				RGBPPM_file = new FileOutputStream(outputFileName + ".rgb.ppm");
				CLASSPPM_file = new FileOutputStream(outputFileName + ".c.ppm");
	
				PrintStream YUVPPM = new PrintStream(new BufferedOutputStream( YUVPPM_file ));
				PrintStream RGBPPM = new PrintStream(new BufferedOutputStream( RGBPPM_file ));
				PrintStream CLASSPPM = new PrintStream(new BufferedOutputStream( CLASSPPM_file ));
			
				YUVPPM.println(PPM_MAGIC);
				YUVPPM.println(fullWidth + " " + fullHeight);
				YUVPPM.println(MAX_PPM_VAL);
				
				RGBPPM.println(PPM_MAGIC);
				RGBPPM.println(fullWidth + " " + fullHeight);
				RGBPPM.println(MAX_PPM_VAL);
				
				CLASSPPM.println(PPM_MAGIC);
				CLASSPPM.println(fullWidth + " " + fullHeight);
				CLASSPPM.println(MAX_PPM_VAL);
				
				int n=0;
				
				for (int j=0; j< numPicHeight; j++, n += numPicWidth) {
					FileInputStream[] BFL = new FileInputStream[numPicWidth];
			
					try {
						for (int i=0; i<numPicWidth; i++) {
							int imageNum = (n+i)%inputFilename.length;
							BFL[i] = new FileInputStream(inputFilename[imageNum]);
						}
						
						for(int y=0; y < PIC_HEIGHT-1; y++) {
				
							for (int i=0; i<numPicWidth; i++) {
				
								BFL[i].read(lineY);
								BFL[i].read(lineU);
								BFL[i].read(lineV);
								BFL[i].read(lineC);
				
								for (int x=0; x < PIC_WIDTH ; x++) {
									
									int pix_y = lineY[x] & 0xff;
									int pix_u = lineU[x] & 0xff;
									int pix_v = lineV[x] & 0xff;
									int pix_c = lineC[x] & 0xff;
									
									if (pix_c >= colors.length)	// last colour is 'unlabelled'
										pix_c = colors.length-1;
									
									YUVPPM.write(lineY[x]);
									YUVPPM.write(lineU[x]);
									YUVPPM.write(lineV[x]);
									
									CLASSPPM.write(colors[pix_c].getRed());
									CLASSPPM.write(colors[pix_c].getGreen());
									CLASSPPM.write(colors[pix_c].getBlue());
	
									//change range of u and v for rgb conversion
									pix_u -= 128;
									pix_v -= 128;
									
									int r = (int)((1.164*pix_y) + (1.596*pix_u));
									int g = (int)((1.164*pix_y) - (0.813*pix_u) - (0.391*pix_v));
									int b = (int)((1.164*pix_y) + (1.596*pix_v));
								
									
									if (r<0) r=0; else if (r>255) r=255;
									if (g<0) g=0; else if (g>255) g=255;
									if (b<0) b=0; else if (b>255) b=255;
				
									RGBPPM.write(r);
									RGBPPM.write(g);
									RGBPPM.write(b);
								}
							}
						}
					} finally {
						for (int i=0; i<numPicWidth; i++) {
							if (BFL[i] != null)
								BFL[i].close();
						}
					}
				}
				YUVPPM.flush();
				RGBPPM.flush();
				CLASSPPM.flush();
			} finally {
				if (YUVPPM_file != null)
					YUVPPM_file.close();
				if (RGBPPM_file != null)
					RGBPPM_file.close();
				if (CLASSPPM_file != null)
					CLASSPPM_file.close();
			}
        } catch (FileNotFoundException ex) {
            System.out.println("file not found: " + ex);
        } catch (IOException ex) {
            System.out.println("an IOException occured: " + ex);
        }
    }

	public static void makePalette(String fileName, int boxSize) {
		
		int fullWidth = boxSize*colors.length;
		int fullHeight = boxSize;
	
		try {
			FileOutputStream paletteFile_raw = null;
			try {
				paletteFile_raw = new FileOutputStream(fileName);
				PrintStream paletteFile = new PrintStream(new BufferedOutputStream(paletteFile_raw));
				
				
				paletteFile.println(PPM_MAGIC);
				paletteFile.println(fullWidth + " " + fullHeight);
				paletteFile.println(MAX_PPM_VAL);
				
				for (int y=0; y<fullHeight; y++) {
					for (int col=0; col<colors.length; col++) {
						for (int boxX = 0; boxX < boxSize; boxX++) {
							paletteFile.write(colors[col].getRed());
							paletteFile.write(colors[col].getGreen());
							paletteFile.write(colors[col].getBlue());
						}
					}
				}
				
				paletteFile.flush();
			} finally {
				if (paletteFile_raw != null) {
					paletteFile_raw.close();
				}
			}
		} catch (IOException ex) {
			throw new RuntimeException(ex.toString());
		}
		
	}

}
