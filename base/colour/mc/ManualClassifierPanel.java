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

import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import javax.swing.*;
import javax.swing.SwingUtilities;
import java.io.*;



public class ManualClassifierPanel extends JPanel {
    public final static int MAX_VAL = 128;
    public final static int PIC_WIDTH = 208;
    public final static int PIC_HEIGHT = 160;
    public final static int FACTOR = 2; // for the colour cube
    public final static int CLOSEUP = 5;

	public final static int MAYBE_BIT = 0x10;
	public final static int cNONE = 0x7E;

    // see base/colour/ic/BFLPanel.java for list of classified 
    // colours and the corresponding numbers
    public static final Color[] classifyColours = {
        Color.orange,                   // orange
        new Color(127,127,255),         // blue
        new Color(0,127,0),             // green
        Color.yellow,                   // yellow
        Color.pink,                     // pink
        new Color(0,0,200),             // blue dog
        Color.red,                      // red dog
        new Color(127,255,127),         // green field
        Color.white.darker().darker(),  // robot grey
        Color.white,                    // field white
        Color.black,                    // black
        Color.gray,                     // field lines
        Color.lightGray,                // field edges
        Color.cyan						// if we have an empty pixel
    };
    public static final String[] colourNames = {
        "Orange", 
        "Blue", 
        "Green", 
        "Yellow", 
        "Pink", 
        "Blue Dog", 
        "Red Dog",
        "Green Field", 
        "Robot Grey", 
        "Field White", 
        "Black",
        "Field Lines",
        "Field Edges",
        "None"
    };

    private byte[][] y = new byte[PIC_HEIGHT][PIC_WIDTH];
    private byte[][] u = new byte[PIC_HEIGHT][PIC_WIDTH];
    private byte[][] v = new byte[PIC_HEIGHT][PIC_WIDTH];
    private byte[] cal = new byte[MAX_VAL * MAX_VAL * MAX_VAL];
    private int scale = 3;
    private int hx = -1, hy = -1, lastx = -1, lasty = -1;
    private ManualClassifier parent;
    private Image im;
    private boolean repaint = false;

    //RAYMOND: added colorcube slicer display. 
    private boolean useSlicer = false;
    private int ySliceValue = MAX_VAL/2;
    private boolean showClassifiedCube = true;

    // constructor
    public ManualClassifierPanel(ManualClassifier parent) {
        this.parent = parent;        
        addMouseMotionListener(new ManualClassifierListener(this));
        addMouseListener(new ManualClassifierListener(this));
    }

    // reads calibration file
    public boolean readCalFile(String filename) {
        try {
            FileInputStream inCal = new FileInputStream(filename);
            inCal.read(cal);
        } catch (FileNotFoundException ex) {
            return false;
        }
        catch (IOException ex) {
            return false;
        }
        return true;
    }
    
    // writes calibration file
    public boolean saveCalFile(String filename) {
        try {
            FileOutputStream outCal = new FileOutputStream(filename);
            outCal.write(cal);
        } catch (FileNotFoundException ex) {
            return false;
        }
        catch (IOException ex) {
            return false;
        }
        return true;
    }

    // reads yuv component of bfl file
    public boolean readBflFile(String filename) {
        try {
	    byte[] line = new byte[4*PIC_WIDTH]; // not interested in the training version of the image

            FileInputStream inBfl = new FileInputStream(filename);
            for (int i = 0; i < PIC_HEIGHT; i++) {
                inBfl.read(y[i]);
                inBfl.read(u[i]);
                inBfl.read(v[i]);
                inBfl.read(line);
            }
        } catch (FileNotFoundException ex) {
            return false;
        }
        catch (IOException ex) {
            return false;
        }
        return true;
    }
    
    // paints the panel
    public void paintComponent(Graphics g) {
        if ((getWidth() / (2 * PIC_WIDTH)) != scale || repaint) {
            im = createImage(getWidth(), getHeight());
            Graphics img = im.getGraphics();
            scale = getWidth() / (2 * PIC_WIDTH);
            for(int i = 0; i < PIC_HEIGHT; i++) {
                for(int j = 0; j < PIC_WIDTH; j++) {
                    int cy = (y[i][j] & 0xff);
                    int cu = (u[i][j] & 0xff);
                    int cv = (v[i][j] & 0xff);
                    int c = classify(cy / FACTOR, cu / FACTOR, cv / FACTOR);
                    boolean m = isMaybeCol(cy / FACTOR, cu / FACTOR, cv / FACTOR);
                    if (useSlicer)
                    {
                      if (i < MAX_VAL && j < MAX_VAL)
                      {
                        c = classify((ySliceValue&0xFF) , (i&0xFF) , (j&0xFF) );
                        m = isMaybeCol((ySliceValue&0xFF) , (i&0xFF) , (j&0xFF) );
                        if (showClassifiedCube)
                        {
                      	  drawClassifiedPixel(j * scale, i * scale, c, m, img);
                        }
                        else
                        {
                          img.setColor(new Color(yuv2rgb(ySliceValue*2, i*2, j*2)));
                          img.fillRect(j*scale, i*scale, scale, scale);
                        }
                      }
                      else
                      {
                        img.setColor(Color.black);
			img.fillRect(j*scale, i*scale, scale, scale);
                      }
                    }
                    else
                    {
                      drawClassifiedPixel(j * scale, i * scale, c, m, img);
                    }
                    drawOriginalPixel((j + PIC_WIDTH) * scale, i * scale, cy, cu, cv, img);
                }
            }
            repaint = false;
        }
        if (im != null) {
			drawHighLightPixel(im.getGraphics());
			// draw buffered image to screen
			g.drawImage(im, 0, 0, this);
        }
    }
    

    public void setHighLight(int mx, int my)
    {
	    if (useSlicer)
	    {
		    setHighLightSlicer(mx, my);
	    }
	    else
	    {
		    setHighLightCplane(mx, my);
	    }
    }
    
		public void setHighLightSlicer(int mx, int my)
		{
		   if (mx >= 0 && mx < PIC_WIDTH * 2 * scale && my >= 0 && my < PIC_HEIGHT * scale) 
			 {
          if (mx >= PIC_WIDTH * scale) 
					{ // yuv display
						mx -= PIC_WIDTH * scale;
            int cy = (y[my / scale][mx / scale] & 0xff);
            int cu = (u[my / scale][mx / scale] & 0xff);
            int cv = (v[my / scale][mx / scale] & 0xff);
            int c = classify(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            boolean m = isMaybeCol(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            hx = mx / scale;
            hy = my / scale;
            parent.setYUVCInfo(cy, cu, cv, colourNames[c], m);

            // construct 25 closeup to send
            Color[] closeup_o = new Color[CLOSEUP * CLOSEUP];
            Color[] closeup_c = new Color[CLOSEUP * CLOSEUP];
            boolean[] closeup_m = new boolean[CLOSEUP * CLOSEUP];
            int index = 0;
            for (int i = hy - 2; i <= hy + 2; i++) {
                for (int j = hx - 2; j <= hx + 2; j++) {
                    if (i < PIC_HEIGHT && i >= 0 && j < PIC_WIDTH && j >= 0) {
                        cy = (y[i][j] & 0xff);
                        cu = (u[i][j] & 0xff);
                        cv = (v[i][j] & 0xff);
                        closeup_o[index] = new Color(yuv2rgb(cy, cu, cv));
                        closeup_m[index] = isMaybeCol(cy/FACTOR, cu/FACTOR, cv/FACTOR);
                        closeup_c[index++] = classifyColours[classify(cy/FACTOR, cu/FACTOR, cv/FACTOR)];
                    } else {
                        closeup_o[index] = Color.black;
                        closeup_m[index] = false;
                        closeup_c[index++] = Color.black;
                    }
                }
            }
            parent.setCloseUpInfo(closeup_o, closeup_c, closeup_m);
            parent.setXYInfo(hx, hy);
 
			}
			else 
			{//slicer
			
            int cy = (char)(ySliceValue*2) & 0xFF; //note, we multiply by 2 because 
            int cu = (char)(my/scale*2) & 0xFF;    //normally cy/u/v go up to 256
            int cv = (char)(mx/scale*2) & 0xFF;
            int c = classify(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            boolean m = isMaybeCol(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            hx = mx / scale;
            hy = my / scale;
            parent.setYUVCInfo(cy, cu, cv, colourNames[c], m);

            // construct 25 closeup to send
            Color[] closeup_o = new Color[CLOSEUP * CLOSEUP];
            Color[] closeup_c = new Color[CLOSEUP * CLOSEUP];
            boolean[] closeup_m = new boolean[CLOSEUP * CLOSEUP];
            int index = 0;
            for (int i = hy - 2; i <= hy + 2; i++) {
                for (int j = hx - 2; j <= hx + 2; j++) {
                    if (i < MAX_VAL*scale && i >= 0 && j < MAX_VAL*scale && j >= 0) {
                        cy = ySliceValue*2;
                        cu = i*2;
                        cv = j*2;
                        closeup_o[index] = Color.black;
                        closeup_m[index] = isMaybeCol(cy/FACTOR, cu/FACTOR, cv/FACTOR);
                        closeup_c[index++] = classifyColours[classify(cy/FACTOR, cu/FACTOR, cv/FACTOR)];
                    } else {
                        closeup_o[index] = Color.black;
                        closeup_m[index] = false;
                        closeup_c[index++] = Color.black;
                    }
                }
            }
            parent.setCloseUpInfo(closeup_o, closeup_c, closeup_m);
            parent.setXYInfo(hx, hy);
        }
			 } else {
            parent.setYUVCInfo(-1, -1, -1, "", false);
            parent.setXYInfo(-1, -1);
            hx = hy = -1;
        }
       repaint();
		}
    
    public void setHighLightCplane(int mx, int my) {
        if (mx >= 0 && mx < PIC_WIDTH * 2 * scale && my >= 0 && my < PIC_HEIGHT * scale) {
            if (mx >= PIC_WIDTH * scale) { // if we are pting to second image
                mx -= PIC_WIDTH * scale;
            }
            int cy = (y[my / scale][mx / scale] & 0xff);
            int cu = (u[my / scale][mx / scale] & 0xff);
            int cv = (v[my / scale][mx / scale] & 0xff);
            int c = classify(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            boolean m = isMaybeCol(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            hx = mx / scale;
            hy = my / scale;
            parent.setYUVCInfo(cy, cu, cv, colourNames[c], m);

            // construct 25 closeup to send
            Color[] closeup_o = new Color[CLOSEUP * CLOSEUP];
            Color[] closeup_c = new Color[CLOSEUP * CLOSEUP];
            boolean[] closeup_m = new boolean[CLOSEUP * CLOSEUP];
            int index = 0;
            for (int i = hy - 2; i <= hy + 2; i++) {
                for (int j = hx - 2; j <= hx + 2; j++) {
                    if (i < PIC_HEIGHT && i >= 0 && j < PIC_WIDTH && j >= 0) {
                        cy = (y[i][j] & 0xff);
                        cu = (u[i][j] & 0xff);
                        cv = (v[i][j] & 0xff);
                        closeup_o[index] = new Color(yuv2rgb(cy, cu, cv));
                        closeup_m[index] = isMaybeCol(cy/FACTOR, cu/FACTOR, cv/FACTOR);
                        closeup_c[index++] = classifyColours[classify(cy/FACTOR, cu/FACTOR, cv/FACTOR)];
                    } else {
                        closeup_o[index] = Color.black;
                        closeup_m[index] = false;
                        closeup_c[index++] = Color.black;
                    }
                }
            }
            parent.setCloseUpInfo(closeup_o, closeup_c, closeup_m);
            parent.setXYInfo(hx, hy);
        } else {
            parent.setYUVCInfo(-1, -1, -1, "", false);
            parent.setXYInfo(-1, -1);
            hx = hy = -1;
        }
        repaint();
    }
    
    public void setCalibration(int mx, int my) {
      if (useSlicer)
      {
        if (mx >= 0 && mx < PIC_WIDTH * 2 * scale && my >= 0 && my < PIC_HEIGHT * scale) {
            if (mx >= PIC_WIDTH * scale) 
            {
              //we've clicked into the YUV display
                mx -= PIC_WIDTH * scale;
              int cy = (y[my / scale][mx / scale] & 0xff);
              int cu = (u[my / scale][mx / scale] & 0xff);
              int cv = (v[my / scale][mx / scale] & 0xff);
              byte c = parent.getSelectedColour();
              boolean m = parent.getMaybe();
              if (m) {
              	c = (byte) (c | MAYBE_BIT);
              }
              int d = parent.getExpandDistance();
              for (int i = cy - d; i <= cy + d; i++) {
                  for (int j = cu - d; j <= cu + d; j++) {
                      for (int k = cv - d; k <= cv + d; k++) {
                          if (i < MAX_VAL * 2 && i >= 0 && j < MAX_VAL * 2 && j >= 0 && k < MAX_VAL * 2 && k >= 0) {
                              cal[((i / FACTOR) * MAX_VAL * MAX_VAL) + ((j / FACTOR) * MAX_VAL) + (k / FACTOR)] = c;
                          }
                      }
                  }
              }
            }
            else if (my < MAX_VAL*scale && mx < MAX_VAL*scale)
            {
              //we've clicked into the slicer display
              int cy = (char)(ySliceValue*2) & 0xFF; //note, we multiply by 2 because 
              int cu = (char)(my/scale*2) & 0xFF;    //normally cy/u/v go up to 256
              int cv = (char)(mx/scale*2) & 0xFF;
              byte c = parent.getSelectedColour();
              boolean m = parent.getMaybe();
              if (m) {
              	c = (byte) (c | MAYBE_BIT);
              }
              int d = parent.getExpandDistance();
              for (int i = cy - d; i <= cy + d; i++) {
                  for (int j = cu - d; j <= cu + d; j++) {
                      for (int k = cv - d; k <= cv + d; k++) {
                          if (i < MAX_VAL * 2 && i >= 0 && j < MAX_VAL * 2 && j >= 0 && k < MAX_VAL * 2 && k >= 0) {
                              cal[((i / FACTOR) * MAX_VAL * MAX_VAL) + ((j / FACTOR) * MAX_VAL) + (k / FACTOR)] = c;
                          }
                      }
                  }
              }
            }
            repaint = true;
        }
      }
      else
      {
        if (mx >= 0 && mx < PIC_WIDTH * 2 * scale && my >= 0 && my < PIC_HEIGHT * scale) {
            if (mx >= PIC_WIDTH * scale) { // if we are pting to second image
                mx -= PIC_WIDTH * scale;
            }
            int cy = (y[my / scale][mx / scale] & 0xff);
            int cu = (u[my / scale][mx / scale] & 0xff);
            int cv = (v[my / scale][mx / scale] & 0xff);
            byte c = parent.getSelectedColour();
            boolean m = parent.getMaybe();
            if (m) {
            	c = (byte) (c | MAYBE_BIT);
            }
            int d = parent.getExpandDistance();
            for (int i = cy - d; i <= cy + d; i++) {
                for (int j = cu - d; j <= cu + d; j++) {
                    for (int k = cv - d; k <= cv + d; k++) {
                        if (i < MAX_VAL * 2 && i >= 0 && j < MAX_VAL * 2 && j >= 0 && k < MAX_VAL * 2 && k >= 0) {
                            cal[((i / FACTOR) * MAX_VAL * MAX_VAL) + ((j / FACTOR) * MAX_VAL) + (k / FACTOR)] = c;
                        }
                    }
                }
            }
            repaint = true;
        }
      }
        repaint();
    }
    
    public void repaintAll() {
        repaint = true;
        repaint();
    }

    public void setYSlice(int iny)
    {
      if (iny > 0 && iny < MAX_VAL)
      {
        ySliceValue = iny;
        repaint = true;
        this.repaint();
      }
    }

    public void setUseSlicer(boolean inUseSlicer)
    {
      useSlicer = inUseSlicer;
      repaint = true;
      this.repaint();
    }

    public boolean getUseSlicer()
    {
      return useSlicer;
    }

    

    public int getYSlice()
    {
      return ySliceValue;
    }
    
    public void setShowClassifiedCube(boolean inClassifiedCube)
    {
      showClassifiedCube = inClassifiedCube;
      repaint = true;
      this.repaint();
    }
   
    public boolean getShowClassifiedCube()
    {
      return showClassifiedCube;
    }
    /**************************************************************
     * private
     **************************************************************/
    
    // classifies a pixel
    private int classify(int cy, int cu, int cv) {
        int result = (int)(cal[cy * MAX_VAL * MAX_VAL + cu * MAX_VAL + cv]);
        if (result == cNONE)
        	result = classifyColours.length-1;
        return (result & ~MAYBE_BIT);
    }
    
    private boolean isMaybeCol(int cy, int cu, int cv) {
        int result = (int)(cal[cy * MAX_VAL * MAX_VAL + cu * MAX_VAL + cv]);
        return ((result & MAYBE_BIT) != 0);
    }
    
    //draw a pixel square with classified colours
    private void drawClassifiedPixel(int px, int py, int c, boolean m, Graphics g) {
        g.setColor(classifyColours[c]);
           g.fillRect(px, py, scale, scale);
        if (m) {
        	if (c != 6)
	        	g.setColor(Color.red);
	        else
	        	g.setColor(Color.black);
	        if (scale >= 3)
	        	g.drawRect(px+scale/2, py+scale/2, 0, 0);
	        else if (scale == 2) {
	        	g.drawRect(px, py, 0, 0);
	        }
        }
    }

    //draw a pixel square with original colours
    private void drawOriginalPixel(int px, int py, int cy, int cu, int cv, Graphics g) {
        g.setColor(new Color(yuv2rgb(cy, cu, cv)));
           g.fillRect(px, py, scale, scale);
    }
    
    // draw a high lighted pixel
    private void drawHighLightPixel(Graphics g) {
        // remove previous high light
        if (lastx >= 0 && lasty >= 0) {
            int cy = (y[lasty][lastx] & 0xff);
            int cu = (u[lasty][lastx] & 0xff);
            int cv = (v[lasty][lastx] & 0xff);
            int c = classify(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            boolean m = isMaybeCol(cy / FACTOR, cu / FACTOR, cv / FACTOR);
            drawClassifiedPixel(lastx * scale, lasty * scale, c, m, g);
            drawOriginalPixel((lastx + PIC_WIDTH) * scale, lasty * scale, cy, cu, cv, g);
        }            
        // high lighting
        if (hx >= 0 && hy >= 0 && !useSlicer) {
            g.setColor(new Color(100, 100, 255));
            g.fillRect(hx * scale, hy * scale, scale, scale);
            g.fillRect((hx + PIC_WIDTH) * scale, hy * scale, scale, scale);
            g.setColor(new Color(255, 100, 100));
            g.drawRect(hx * scale + 1, hy * scale + 1, scale - 2, scale - 2);
            g.drawRect((hx + PIC_WIDTH) * scale + 1, hy * scale + 1, scale - 2, scale - 2);
            lastx = hx; lasty = hy;
        } else {
            lastx = -1;
            lasty = -1;
        }
    }
    
    private int yuv2rgb(int cy, int cu, int cv) {
        // converts from yuv to rgb
        cu-=128; cv-=128;
        int cr = (int)((1.164*cy) + (1.596*cu));
        int cg = (int)((1.164*cy) - (0.813*cu) - (0.391*cv));
        int cb = (int)((1.164*cy) + (1.596*cv));
        
        // I dont think this should be neccesary... but it is
        // did i make a mistake somewhere?
        if (cr < 0) cr = 0;       if (cg < 0) cg = 0;       if (cb < 0) cb = 0;
        if (cr > 255) cr = 255;   if (cg > 255) cg = 255;   if (cb > 255) cb = 255;

        return (int)((cr<<16) | (cg<<8) | cb);
    }
}
