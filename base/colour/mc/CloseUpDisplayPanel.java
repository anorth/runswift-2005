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
import javax.swing.*;
import java.io.*;
import java.util.Vector;

public class CloseUpDisplayPanel extends JPanel {

    private Vector rgb = new Vector();
    private int size;
    private boolean[] maybeCols;
    
    public void paintComponent(Graphics g) {
        int w = getWidth();
        int h = getHeight();
        Image im = createImage(w, h);
        Graphics img = im.getGraphics();
        
        int scale = Math.min(w, h) / (size + 2);
        int index = 0;
        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                img.setColor((Color)(rgb.get(index)));
                img.fillRect(scale * j, scale * i, scale, scale);
                if ((maybeCols != null) && (maybeCols[index])) {
        			img.setColor(Color.red);
                	img.drawRect(scale * j, scale * i, scale-1, scale-1);
                }
                index++;
            }
        }
        // the middle currently selected square
        img.setColor(Color.black);
        img.drawRect(scale * 3, scale * 3, scale-1, scale-1);
        g.drawImage(im, 0, 0, this);
    }
    
    public void set(Color[] c, boolean[] m) {
        rgb.clear();
        maybeCols = m;
        for (int i = 0; i < c.length; i++) {
	        rgb.add(c[i]);
        }
        size = (int)(Math.sqrt(c.length));
        repaint();
    }
}
