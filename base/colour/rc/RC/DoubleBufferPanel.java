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

/**
 * <p>Title: DoubleBufferPanel class </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author http://codeproject.com/java/javadoublebuffer.asp
 * @version 1.0
 */
import javax.swing.*;
import java.awt.*;

public class DoubleBufferPanel
    extends JPanel {
    //    class variables
    private int bufferWidth;
    private int bufferHeight;
    private Image bufferImage;
    protected Graphics bufferGraphics;
    protected Graphics directGraphics = null;

    public DoubleBufferPanel() {
        super();
        setDoubleBuffered(true);
    }

    public void paintComponent(Graphics g) {
    	if (false) {
			directGraphics = g;
			//    checks the buffersize with the current panelsize
			//    or initialises the image with the first paint
			if (bufferWidth != getSize().width ||
				bufferHeight != getSize().height ||
				bufferImage == null || bufferGraphics == null){
				resetBuffer();
			}
	
			if (bufferGraphics != null) {
				//this clears the offscreen image, not the onscreen one
				bufferGraphics.clearRect(0, 0, bufferWidth, bufferHeight);
	
				//calls the paintbuffer method with
				//the offscreen graphics as a param
				paintBuffer(bufferGraphics);
	
				//we finaly paint the offscreen image onto the onscreen image
	//            bufferImage.flush();
				g.drawImage(bufferImage, 0, 0, this);
	//            System.out.println("drawned on screen");
			}
		} else {
			g.clearRect(0, 0, getSize().width, getSize().height);
			paintBuffer(g);
		}
    }

    private void resetBuffer() {
        // always keep track of the image size
        bufferWidth = getSize().width;
        bufferHeight = getSize().height;

        //    clean up the previous image
        if (bufferGraphics != null) {
            bufferGraphics.dispose();
            bufferGraphics = null;
        }
        if (bufferImage != null) {
            bufferImage.flush();
            bufferImage = null;
        }
        System.gc();

        //    create the new image with the size of the panel
        bufferImage = createImage(bufferWidth, bufferHeight);
        bufferGraphics = bufferImage.getGraphics();
    }
/*
    public Graphics getGraphics(){
        return bufferGraphics;
    }
*/
    public void paintBuffer(Graphics g) {
        //in classes extended from this one, add something to paint here!
        //always remember, g is the offscreen graphics
    }

}
