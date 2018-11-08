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
 * UNSW 2002 Robocup (David Wang)
 *
 * Last modification background information
 * $Id: GLBeacon.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;


public class GLBeacon extends GLRoboObject {
    public static final int WIDTH = 10, HEIGHT = 20;
    protected static final double STEP  = Math.PI / 15;
	protected static double RADIUS = 15;


    protected Color topFill,bottomFill;
    protected double x, y, cf;

    public GLBeacon() {
        topFill = Color.black;
        bottomFill = Color.black;
    }
	
	public void paint(GL gl, GLU glu, GLDrawable glc) {
		paint(gl, glu, glc, false);
	}

    public void paint(GL gl, GLU glu, GLDrawable glc, boolean selected) {

        gl.glDisable(gl.GL_TEXTURE_2D);
        gl.glLineWidth(LINE_WIDTH);

         // draw circle
        gl.glPushMatrix();
        gl.glTranslated(x,y,0);


        setColor(gl,topFill);
        gl.glBegin( gl.GL_POLYGON );
        gl.glVertex2d(WIDTH/2.0,HEIGHT/2.0);
        gl.glVertex2d(- WIDTH/2.0,HEIGHT/2.0);
        gl.glVertex2d(- WIDTH/2.0,0);
        gl.glVertex2d(WIDTH/2.0,0);
        gl.glEnd();

        setColor(gl,bottomFill);
        gl.glBegin( gl.GL_POLYGON );
        gl.glVertex2d(WIDTH/2.0,0);
        gl.glVertex2d(- WIDTH/2.0,0);
        gl.glVertex2d(- WIDTH/2.0,-HEIGHT/2.0);
        gl.glVertex2d(WIDTH/2.0,-HEIGHT/2.0);
        gl.glEnd();

        setColor(gl,Color.black);
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(WIDTH/2.0,HEIGHT/2.0);
        gl.glVertex2d(- WIDTH/2.0,HEIGHT/2.0);
        gl.glVertex2d(- WIDTH/2.0,-HEIGHT/2.0);
        gl.glVertex2d(WIDTH/2.0,-HEIGHT/2.0);
        gl.glVertex2d(WIDTH/2.0,HEIGHT/2.0);
        gl.glEnd();
		
		if(selected) {
			setColor(gl,Color.MAGENTA);
        	gl.glBegin( gl.GL_LINE_STRIP );
			for (double t = 0; t <= Math.PI*2; t += STEP) {
            	double tx = RADIUS*Math.cos(t);
            	double ty = RADIUS*Math.sin(t);
            	gl.glVertex2d(tx, ty);
        	}
        	gl.glEnd();
		}

        gl.glPopMatrix();

    }
    public void setLocation(double x, double y) {
        setLocation(x,y,0);
    }
    public void setLocation(double x, double y, double h) {
        this.x = x;
        this.y = y;
    }
    public void setConfidenceFactor(double cf) {
        this.cf = cf;
    }
    public void setColor(Color top, Color bottom) {
        topFill = top;
        bottomFill = bottom;
    }
}
