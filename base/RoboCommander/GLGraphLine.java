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

/*
 * UNSW 2002 Robocup (Andres Olave)
 *
 * Last modification background information
 * $Id: GLGraphLine.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;
import java.util.*;

public class GLGraphLine implements GLGraphType /* , GLEnum */  {
    public final static int NUM_GRAPH_LINES=7;
    private final static float MAX_YVAL=50;
    private final static float NEXT_POINT_INTERVAL=3f;
    public Color[] lineColours = {
        Color.orange,
        Color.black,
        Color.pink,
        Color.green,
        Color.blue,
        Color.yellow,
        Color.red
    };

    public boolean[] drawLine = {true, true, true, true, true, true, true};
    
    public String[] lineNames = {
        "2D Distance",
        "Z(perp) Distance",
        "Camera Distance",
        "Camera-Centroid",
        "Calibrated Z",
        "Calibrated 2D",
        "Calibrated Camera"
    };
    
    private ArrayList[] lines = new ArrayList[NUM_GRAPH_LINES];
    
    public GLGraphLine() {
        for(int i=0;i<NUM_GRAPH_LINES;i++) {
            lines[i]=new ArrayList();
        }
    }

    public void processData(double[] data, int start, int len) {
    //    System.err.println("Processing " + len);
        for (int i=start, j=0;i<NUM_GRAPH_LINES;i++,j++) {
        //  System.err.println(i +  " " + data[i]);
            lines[j].add(new Float(data[i]));
        }
    }

    public void paint(GL gl, GLU glu, GLDrawable glc) {
        gl.glDisable(gl.GL_TEXTURE_2D);
        gl.glLineWidth(GraphDisplay.LINE_WIDTH);
        gl.glPushMatrix();
    //    gl.glTranslated(GLGrapher.X_ORIGIN, GLGrapher.Y_ORIGIN, 0);
        for (int j=0;j<NUM_GRAPH_LINES;j++) {
            if (drawLine[j]) {
                // && ((Float)lines[j].get(lines[j].size()-1)).floatValue()!=0) {
                GLGrapher.setColor(gl, lineColours[j]);
                //        gl.glTranslated(GLGrapher.X_ORIGIN, GLGrapher.Y_ORIGIN, 0);
                gl.glBegin(gl.GL_LINE_STRIP);
                float x=0;
                for (int i=0; i<lines[j].size(); x+=NEXT_POINT_INTERVAL,i++) {
            
                    gl.glVertex2d(x + GLGrapher.X_ORIGIN, ((Float)lines[j].get(i)).floatValue() / MAX_YVAL* GLGrapher.Y_AXIS_LENGTH  + GLGrapher.Y_ORIGIN);
                    //            gl.glVertex2d(x + GLGrapher.X_ORIGIN, 0 + GLGrapher.Y_ORIGIN);
                }
                gl.glEnd();
            }
        }
        gl.glPopMatrix();
    }
       
    public void reset() {
        for (int j=0;j<NUM_GRAPH_LINES;j++) {
            lines[j].clear();
        }
    }
}
