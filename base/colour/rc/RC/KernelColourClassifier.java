/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


package RC;

import java.util.*;
import RoboShare.Utils;
import java.io.*;

class examplePt {
	int y;
	int u;
	int v;
	int c;
	
	examplePt(int y, int u, int v, int c) {
		this.y = y;
		this.u = u;
		this.v = v;
		this.c = c;
	}
}
	
public class KernelColourClassifier implements ColourClassifier {
	float[][][][] colourVals;
	
	static final int resolution = 2;
	
	static final int YMAX = 128;
	static final int UMAX = 128;
	static final int VMAX = 128;
	static final int CMAX = 13;
	
	static final int YKernelRadius = 5;
	static final int UKernelRadius = 5;
	static final int VKernelRadius = 5;
	
	static final float INITIAL_NONE_VALUE = 0.001f;
	
	float[][][] kernel;
	
	int[][][] nnmc;
	
	LinkedList undoList;	// for java 1.4
	// LinkedList<examplePt> undoList;	// java 1.5
	
	public KernelColourClassifier() {
		colourVals = new float[YMAX][UMAX][VMAX][CMAX];
		kernel = new float[2*YKernelRadius + 1][2*UKernelRadius+1][2*VKernelRadius+1];
		nnmc = new int[YMAX][UMAX][VMAX];
		undoList = new LinkedList();
		// undoList = new LinkedList<examplePt>();	// java 1.5
		
		for (int y = -YKernelRadius; y <= YKernelRadius; y++) {
			for (int u = -UKernelRadius; u <= UKernelRadius; u++) {
				for(int v = -VKernelRadius; v <= VKernelRadius; v++) {
					float thisVal = rawKernelVal(y, u, v);
					kernel[y+YKernelRadius][u+UKernelRadius][v+VKernelRadius] = thisVal;
					//System.out.println("Y: " + y + " U: " + u + " V: " + v + " K: " + thisVal);
				}
			}
		}
		
		for (int y = 0; y<YMAX; y++) {
			for (int u = 0; u<YMAX; u++) {
				for (int v = 0; v<YMAX; v++) {
					colourVals[y][u][v][CMAX-1] = INITIAL_NONE_VALUE;
					nnmc[y][u][v] = CMAX-1;
				}
			}
		}
	}
	
	float kernelVal(int dY, int dU, int dV) {
		return kernel[dY+YKernelRadius][dU+UKernelRadius][dV+VKernelRadius];
	}
	
	float rawKernelVal(int dY, int dU, int dV) {
		final double uvInvVar = 10;
		float result = 0;

		double dColSqr = (double)dU*dU/UKernelRadius/UKernelRadius +
				(double)dV*dV/VKernelRadius/VKernelRadius + 
				(double)dY*dY*dY*dY/YKernelRadius/YKernelRadius/YKernelRadius/YKernelRadius;

		if (dColSqr > 1)
			return 0;

		result += Math.exp(-dColSqr*uvInvVar);
		result -= Math.exp(-uvInvVar);
		
		if (dY == 0 && dU == 0 && dV == 0)
			result *= 1.1;
		
		return result;
	}
	
	int byteToInt(byte b) {
		return b & 0xff;
	}
	
	boolean addKernel(int y, int u, int v, int C, float mult) {
		boolean madeChange = false;
		
		for (int dy = -YKernelRadius; dy <= YKernelRadius; dy++) {
			for (int du = -UKernelRadius; du <= UKernelRadius; du++) {
				for(int dv = -VKernelRadius; dv <= VKernelRadius; dv++) {
					int ty = y + dy;
					int tu = u + du;
					int tv = v + dv;
					
					if ((ty >= YMAX) || (ty < 0)) {
						continue;
					}
					
					if ((tu >= UMAX) || (tu < 0)) {
						continue;
					}
					
					if ((tv >= VMAX) || (tv < 0)) {
						continue;
					}
					
					colourVals[ty][tu][tv][C] += mult*kernelVal(dy,du,dv);
					
					int maxCol = 0;
					float maxVal = colourVals[ty][tu][tv][0];
					
					for (int c = 1; c < CMAX; c++) {
						if (colourVals[ty][tu][tv][c] > maxVal) {
							maxVal = colourVals[ty][tu][tv][c];
							maxCol = c;
						}
					}
					
					if (nnmc[ty][tu][tv] != maxCol) {
						madeChange = true;
						nnmc[ty][tu][tv] = maxCol;
					}
				}
			}
		}
		return madeChange;
	}
	
	public boolean training_example(byte Y, byte U, byte V, int c) {
		int y = byteToInt(Y)/resolution;
		int u = byteToInt(U)/resolution;
		int v = byteToInt(V)/resolution;
		
		if (c >= CMAX)
			c = CMAX-1;
		
		boolean shouldAdd = false;
		
		for (int dy = -YKernelRadius; dy <= YKernelRadius && !shouldAdd; dy++) {
			for (int du = -UKernelRadius; du <= UKernelRadius && !shouldAdd; du++) {
				for(int dv = -VKernelRadius; dv <= VKernelRadius && !shouldAdd; dv++) {
					int ty = y + dy;
					int tu = u + du;
					int tv = v + dv;
					
					if ((ty >= YMAX) || (ty < 0)) {
						continue;
					}
					
					if ((tu >= UMAX) || (tu < 0)) {
						continue;
					}
					
					if ((tv >= VMAX) || (tv < 0)) {
						continue;
					}
					
					if (nnmc[ty][tu][tv] != c)
						shouldAdd = true;
				}
			}
		}
		
		if (!shouldAdd)
			return false;
		
		examplePt pt = new examplePt(y, u, v, c);
		undoList.add(pt);
		
		boolean changed = addKernel(y,u,v,c,1);
		
		return changed;
	}
	
	public void undo() {
		examplePt ptToRemove = (examplePt)undoList.removeLast();	// for java 1.4
		// examplePt ptToRemove = undoList.removeLast();	// java 1.5
		addKernel(ptToRemove.y,ptToRemove.u,ptToRemove.v,ptToRemove.c,-1);
	}
	
	public int test_example(byte Y, byte U, byte V) {
		int y = byteToInt(Y)/resolution;
		int u = byteToInt(U)/resolution;
		int v = byteToInt(V)/resolution;
		
		int result = nnmc[y][u][v];
		
		if (result >= CMAX-1)
			result = 127;
		
		return result;
	}
	
	public void print() {
		System.out.println("Ha! - not going to bother printing large table of numbers");
	}
	
	public void saveToFile(String fileName) {
		FileOutputStream fout = null;
		try {
			fout = new FileOutputStream(fileName);
			DataOutputStream out = new DataOutputStream(new BufferedOutputStream(fout));
			for (int y = 0; y<YMAX; y++) {
				for (int u = 0; u<YMAX; u++) {
					for (int v = 0; v<YMAX; v++) {
						for (int c = 0; c<CMAX; c++) {
							out.writeFloat(colourVals[y][u][v][c]);
						}
					}
				}
			}
			out.flush();
		} catch (IOException e) {
			System.err.println("IOException saving file: " + fileName + ": " + e);
		} finally {
			if (fout != null) {
				try {
					fout.close();
				} catch (IOException e) {
					System.err.println("IOException closing file: " + fileName + ": " + e);
				}
			}
		}
	}
	
	public void loadFromFile(String fileName) {
		if (fileName.endsWith(".xml")) {
			loadFromRDR(fileName);
			return;
		}
	
		FileInputStream fin = null;
		try {
			fin = new FileInputStream(fileName);
			DataInputStream in = new DataInputStream(new BufferedInputStream(fin));
			for (int y = 0; y<YMAX; y++) {
				for (int u = 0; u<YMAX; u++) {
					for (int v = 0; v<YMAX; v++) {
						int bestCol = CMAX-1;
						float maxVal = -1;
						for (int c = 0; c<CMAX; c++) {
							colourVals[y][u][v][c] = in.readFloat();
							if (colourVals[y][u][v][c] > maxVal) {
								maxVal = colourVals[y][u][v][c];
								bestCol = c;
							}
						}
						nnmc[y][u][v] = bestCol;
					}
				}
			}
		} catch (IOException e) {
			System.err.println("IOException loading file: " + fileName + ": " + e);
		} finally {
			if (fin != null) {
				try {
					fin.close();
				} catch (IOException e) {
					System.err.println("IOException closing file: " + fileName + ": " + e);
				}
			}
		}
	}
	
	public void loadFromRDR(String fileName) {
		System.out.println("Initializing Kernel classifier from RDR: " + fileName);
		ManRDRColour RDR = new ManRDRColour(3);
		RDR.loadFromFile(fileName);
		System.out.println("RDR read - initing");
		for (int y = 0; y<YMAX; y++) {
			for (int u = 0; u<YMAX; u++) {
				for (int v = 0; v<YMAX; v++) {
					for (int c = 0; c<CMAX; c++) {
						colourVals[y][u][v][c] = 0;
					}
					int RDRc = RDR.test_example((byte)(y*resolution), (byte)(u*resolution), (byte)(v*resolution));
					if (RDRc >= CMAX-1) {
						colourVals[y][u][v][CMAX-1] = INITIAL_NONE_VALUE;
						nnmc[y][u][v] = CMAX-1;
					} else {
						colourVals[y][u][v][RDRc] = kernelVal(0,0,0)/2;
						nnmc[y][u][v] = RDRc;
					}
				}
			}
		}
		System.out.println("done initing.");
	}
	
}
