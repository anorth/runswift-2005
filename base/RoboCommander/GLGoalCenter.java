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
 * $Id: GLGoalCenter.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;


public class GLGoalCenter extends GLRoboObject {
    protected static final int WIDTH = 20;

    protected Color fill;
    protected double x, y, cf;

    public GLGoalCenter() {
        fill = Color.cyan;
    }

    public void paint(GL gl, GLU glu, GLDrawable glc) {

        gl.glDisable(gl.GL_TEXTURE_2D);
        gl.glLineWidth(LINE_WIDTH);
        setColor(gl,fill);

        gl.glPushMatrix();
        gl.glTranslated(x,y,0);

        drawCF(gl,cf,WIDTH/2 + 5,-WIDTH/2);

        // big cross
        setColor(gl,fill);
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(-WIDTH/2,0);
        gl.glVertex2d(WIDTH/2,0);
        gl.glEnd();

        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(0,-WIDTH/2);
        gl.glVertex2d(0,WIDTH/2);
        gl.glEnd();

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
    public void setColor(Color c) {
        fill = c;
    }
}
