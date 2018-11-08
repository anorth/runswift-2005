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
 * $Id: GLRoboObject.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/


import java.awt.*;
import net.java.games.jogl.*;
// // import gl4java.drawable.*;


public abstract class GLRoboObject /* implements GLEnum */ {
    final static BasicStroke STROKE = new BasicStroke(1.3f);
    final static BasicStroke WIDE_STROKE = new BasicStroke(8.0f);
    protected static final float LINE_WIDTH = 1f;
    protected static final float CF_HEIGHT = 20f, CF_WIDTH = 5;
    protected static final double STEP  = Math.PI / 15;
    protected static final double INFINITY = 1000000;

    public abstract void paint(GL gl, GLU glu, GLDrawable glc);
    public abstract void setLocation(double x, double y);
    public abstract void setLocation(double x, double y, double h);
    public abstract void setConfidenceFactor(double cf);

    /** This sets the current GL drawing colour
        @param gl The GL pipeline to use
        @param c colour to set GL to use
    */
    public void setColor(GL gl, Color c){
        //gl.glColor3ub((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue());
    	gl.glColor4ub((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue(),(byte)c.getAlpha());
	}


    /**
     * draws the confidence factor
     * @param xDis is the x displacement to draw the cf
     * @param yDis is the y displacement to draw the cf
     * @param cf is the confidence factor
     */
    public void drawCF(GL gl, double cf, double xDis, double yDis) {
       // draw confidence factor
        gl.glPushMatrix();

        gl.glTranslated(xDis,yDis,0);
        setColor(gl,Color.white);
        gl.glBegin( gl.GL_POLYGON );
        gl.glVertex2d(CF_WIDTH/2,CF_HEIGHT);
        gl.glVertex2d(-CF_WIDTH/2,CF_HEIGHT);
        gl.glVertex2d(-CF_WIDTH/2,0);
        gl.glVertex2d(CF_WIDTH/2,0);
        gl.glEnd();

        setColor(gl,Color.blue);
        gl.glBegin( gl.GL_POLYGON );
        gl.glVertex2d(CF_WIDTH/2,CF_HEIGHT*cf/1000);
        gl.glVertex2d(-CF_WIDTH/2,CF_HEIGHT*cf/1000);
        gl.glVertex2d(-CF_WIDTH/2,0);
        gl.glVertex2d(CF_WIDTH/2,0);
        gl.glEnd();

        setColor(gl,Color.black);
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(CF_WIDTH/2,CF_HEIGHT);
        gl.glVertex2d(-CF_WIDTH/2,CF_HEIGHT);
        gl.glVertex2d(-CF_WIDTH/2,0);
        gl.glVertex2d(CF_WIDTH/2,0);
        gl.glVertex2d(CF_WIDTH/2,CF_HEIGHT);
        gl.glEnd();

        gl.glPopMatrix();
    }

    
    public void drawVar(GL gl,double variance) {
        double confidence = Math.sqrt(variance) * 2;
        setColor(gl,Color.black);
        gl.glBegin( gl.GL_LINE_STRIP );
        for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = confidence/2*Math.cos(t);
            double ty = confidence/2*Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
    }
    
    public void drawVar(GL gl, double varX, double varY, double covXY) {
        //System.out.println("**** varX="+varX+", varY"+varY+", covXY="+covXY);
        double b = -(varX + varY);
        double c = varX*varY - covXY*covXY;
        
        // determine eigenvalues which are variances & 95% confidence intervals
        double v1 = (-b + Math.sqrt(b*b - 4*c))/2;
        double cf1 = 2*Math.sqrt(v1);
        double v2 = -b - v1;
        double cf2 = 2*Math.sqrt(v2);
        
        // determine the eigenvector (only need one as other is orthogonal)
        double ev1 = 0;
        double ev2 = 0;
        if(varX-v1 == 0) {
            ev1 = 1;
            ev2 = 0;
        } else if(covXY == 0) {
            ev1 = 0;
            ev2 = 1;
        } else {
            ev1 = covXY;
            ev2 = v1-varX;
        }
        
        // determine angle to match eigenvectors to axis and rotate then scale
        double angle = -Math.atan2(ev2, ev1)*(180/Math.PI);
        gl.glPushMatrix();
        gl.glRotated(angle, 0, 0, 1);
        gl.glScaled(cf1, cf2, 1);
        
        // draw unit circle
        setColor(gl,Color.red);
        gl.glBegin( gl.GL_LINE_STRIP );
        for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = Math.cos(t);
            double ty = Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
        
        gl.glPopMatrix();
        
    }
	
    //right now this is exactly the same as drawVar except
	//for a change in colour
	//if this function doesn't get modified any further
	//we probably should just have one function, where you can set the colour
	public void drawVelVar(GL gl, double varX, double varY, double covXY) {
        //System.out.println("**** varX="+varX+", varY"+varY+", covXY="+covXY);
        double b = -(varX + varY);
        double c = varX*varY - covXY*covXY;
        
        // determine eigenvalues which are variances & 95% confidence intervals
        double v1 = (-b + Math.sqrt(b*b - 4*c))/2;
        double cf1 = 2*Math.sqrt(v1);
        double v2 = -b - v1;
        double cf2 = 2*Math.sqrt(v2);
        
        // determine the eigenvector (only need one as other is orthogonal)
        double ev1 = 0;
        double ev2 = 0;
        if(varX-v1 == 0) {
            ev1 = 1;
            ev2 = 0;
        } else if(covXY == 0) {
            ev1 = 0;
            ev2 = 1;
        } else {
            ev1 = covXY;
            ev2 = v1-varX;
        }
        
        // determine angle to match eigenvectors to axis and rotate then scale
        double angle = -Math.atan2(ev2, ev1)*(180/Math.PI);
        gl.glPushMatrix();
        gl.glRotated(angle, 0, 0, 1);
        gl.glScaled(cf1, cf2, 1);
        
        // draw unit circle
        setColor(gl,Color.black);
        gl.glBegin( gl.GL_LINE_STRIP );
        for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = Math.cos(t);
            double ty = Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
        
        gl.glPopMatrix();
        
    }	
	
	
    
}
