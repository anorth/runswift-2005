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
 * $Id: GLGrapher.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;


public class GLGrapher /* implements GLEnum */ {

    final static float X_AXIS_LENGTH = 400f;
    final static float X_ORIGIN=50f;  // a little way up so that we can display negative values

    final static float Y_AXIS_LENGTH = 350f;
    final static float Y_ORIGIN=60f;
    final static Color AXIS_COLOUR=Color.black;

    final static float GRID_LINE = 70f;
    final static Color GRID_COLOUR=Color.gray;

    final static float DASH_LENGTH = 4f;
    final static float DASH_GAP = 2f;

    Color bgFill;
    GLGraphType gt;

    public GLGrapher(GLGraphType graphType) {
        bgFill = Color.white;
        gt=graphType;
    }

    public static void setColor(GL gl, Color c){
        gl.glColor3ub((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue());
    }

    public void paint(GL gl, GLU glu, GLDrawable glc) {
        gl.glDisable(gl.GL_TEXTURE_2D);
        gl.glLineWidth(GraphDisplay.LINE_WIDTH);

        gl.glPushMatrix();
        //gl.glTranslated(0,0,0);
        setColor(gl, bgFill);
        gl.glBegin(gl.GL_POLYGON);
        gl.glVertex2d(0,0);
        gl.glVertex2d(GraphDisplay.WIDTH,0);
        gl.glVertex2d(GraphDisplay.WIDTH, GraphDisplay.HEIGHT);
        gl.glVertex2d(0, GraphDisplay.HEIGHT);
        gl.glEnd();
    

        setColor(gl, AXIS_COLOUR);
        gl.glBegin(gl.GL_LINES);
        gl.glVertex2d(X_ORIGIN,Y_ORIGIN);
        gl.glVertex2d(X_ORIGIN+X_AXIS_LENGTH,Y_ORIGIN);
        gl.glEnd();

        //        setColour(gl, AXIS_COLOUR);
        gl.glBegin(gl.GL_LINES);
        gl.glVertex2d(X_ORIGIN,Y_ORIGIN);
        gl.glVertex2d(X_ORIGIN,Y_ORIGIN+Y_AXIS_LENGTH);
        gl.glEnd();

        for (int i=(int) (Y_ORIGIN+GRID_LINE); 
             i < (int) (Y_ORIGIN+Y_AXIS_LENGTH) ; 
             i+= (int) GRID_LINE) {
            setColor(gl, GRID_COLOUR);
            for (int j= (int) (X_ORIGIN+DASH_GAP); 
                 j< (int) (X_ORIGIN+X_AXIS_LENGTH);
                     j += (int) (DASH_LENGTH + DASH_GAP)) {
                gl.glBegin(gl.GL_LINES);
                gl.glVertex2d(j, i);
                gl.glVertex2d(j+DASH_LENGTH, i);
            }
        }
        gt.paint(gl,glu,glc);
        gl.glPopMatrix();

    }
}
