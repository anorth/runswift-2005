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
 * $Id: GLRoboDog.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;


public class GLRoboDog extends GLRoboObject {
    protected static final int WIDTH = 20;
    protected boolean self; // this is me.


    protected Color fill;
    protected double x, y, h;
	protected double vx, vy, vh, vxy;

    // for graphics card speed test
    //protected double speed;
    //protected double counter;

    public GLRoboDog() {
        //speed = Math.random()*20;
        //counter = 0;
        fill = Color.blue;
    }

    public void paint(GL gl, GLU glu, GLDrawable glc) {
        //if (x == 0 && y == 0 && h == 0 && cf <= 3) {
        //    return;
        //}

        /*double x = this.x;
        double y = this.y;
        y += counter++*speed%200;*/

        gl.glDisable(gl.GL_TEXTURE_2D);
        gl.glLineWidth(LINE_WIDTH);
         // draw circle
        gl.glPushMatrix();
        if(fill.equals(Color.blue))
            gl.glTranslated(x,GLField.HEIGHT-y,0);
        else
            gl.glTranslated(GLField.WIDTH-x,y,0);

        //drawCF(gl,cf,WIDTH/2 + 5,-WIDTH/2);

        gl.glPushMatrix();
        double hDeg = -h;
        if(fill.equals(Color.blue))
            gl.glRotated(hDeg,0,0,1);
        else
            gl.glRotated(180+hDeg,0,0,1);
        
        
        //draw the dog as polygon  (circle)
        // fill circle
        setColor(gl,fill);
        gl.glBegin( gl.GL_POLYGON );
        for (double t = 0; t <= Math.PI*2; t += STEP) {
            double tx = WIDTH/2*Math.cos(t);
            double ty = WIDTH/2*Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();

        // fill white triangle to show heading
        setColor(gl,Color.white);
        gl.glBegin( gl.GL_POLYGON );        
        gl.glVertex2d(WIDTH/2, 0);
        gl.glVertex2d(0, WIDTH/2*2/3);
        gl.glVertex2d(0, WIDTH/2*1/4);
        gl.glVertex2d(-WIDTH/2*2/3, WIDTH/2*1/4);
        gl.glVertex2d(-WIDTH/2*2/3, -WIDTH/2*1/4);
        gl.glVertex2d(0, -WIDTH/2*1/4); 
        gl.glVertex2d(0, -WIDTH/2*2/3);        
        gl.glEnd();
        
        // fill triangle to show heading confidence if self
        if(self) {
			double root2 = Math.sqrt(2);
			boolean finished = false;
            double angleCF = 2*Math.sqrt(vh);
			double ang = 0;
            double a = 50;
			if(angleCF > 45)
				ang = 45*Math.PI/180;
			else {
				ang = angleCF*Math.PI/180;
				finished = true;
			}
			double x = a*Math.cos(ang);
			double y = a*Math.sin(ang);
            Color c = new Color((float)0.3,(float)0.3,(float)0.3,(float)0.7);
            setColor(gl,c);
            gl.glBegin(gl.GL_POLYGON);
            gl.glVertex2d(0,0);
            gl.glVertex2d(x,y);
			gl.glVertex2d(a,0);
            gl.glVertex2d(x,-y);
            gl.glEnd();
			
			if(!finished) {
				if(angleCF > 90)
					ang = 90*Math.PI/180;
				else {
					ang = angleCF*Math.PI/180;
					finished = true;
				}
				x = a*Math.cos(ang);
				y = a*Math.sin(ang);
				
				gl.glBegin(gl.GL_POLYGON);
            	gl.glVertex2d(0,0);
            	gl.glVertex2d(a/root2,a/root2);
            	gl.glVertex2d(x,y);
            	gl.glEnd();
				
				gl.glBegin(gl.GL_POLYGON);
            	gl.glVertex2d(0,0);
            	gl.glVertex2d(a/root2,-a/root2);
            	gl.glVertex2d(x,-y);
            	gl.glEnd();
			}
			
			if(!finished) {
				if(angleCF > 135)
					ang = 135*Math.PI/180;
				else {
					ang = angleCF*Math.PI/180;
					finished = true;
				}
				x = a*Math.cos(ang);
				y = a*Math.sin(ang);
				
				gl.glBegin(gl.GL_POLYGON);
            	gl.glVertex2d(0,0);
            	gl.glVertex2d(0,a);
            	gl.glVertex2d(x,y);
            	gl.glEnd();
				
				gl.glBegin(gl.GL_POLYGON);
            	gl.glVertex2d(0,0);
            	gl.glVertex2d(0,-a);
            	gl.glVertex2d(x,-y);
            	gl.glEnd();
			}
			
			if(!finished) {
				if(angleCF > 180)
					ang = Math.PI;
				else
					ang = angleCF*180/Math.PI;
				x = a*Math.cos(ang);
				y = a*Math.sin(ang);
				
				gl.glBegin(gl.GL_POLYGON);
            	gl.glVertex2d(0,0);
            	gl.glVertex2d(-a/root2,a/root2);
            	gl.glVertex2d(x,y);
            	gl.glEnd();
				
				gl.glBegin(gl.GL_POLYGON);
            	gl.glVertex2d(0,0);
            	gl.glVertex2d(-a/root2,-a/root2);
            	gl.glVertex2d(x,-y);
            	gl.glEnd();
			}
        }


        // black circle
        setColor(gl,Color.black);
        gl.glBegin( gl.GL_LINE_STRIP );
        for (double t = 0; t <= Math.PI*2; t += STEP) {
            double tx = WIDTH/2*Math.cos(t);
            double ty = WIDTH/2*Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
    
        // draw an inner shape if self
        if (self) {
             // black circle
            setColor(gl,Color.black);
            gl.glBegin( gl.GL_LINE_STRIP );
            for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = WIDTH/2*4/3*Math.cos(t);
            double ty = WIDTH/2*4/3*Math.sin(t);
            gl.glVertex2d(tx, ty);
            }
            gl.glEnd();
        }

        gl.glPopMatrix();
        // draws the variance
        //drawVar(gl,cf);
        drawVar(gl,vx,vy,vxy);

        gl.glPopMatrix();

    }
	
	public void paintArrow(GL gl, GLU glu, GLDrawable glc, double ax, double ay) {
		
		gl.glLineWidth(LINE_WIDTH*2);
        gl.glPushMatrix();
        if(fill.equals(Color.blue))
            gl.glTranslated(x,GLField.HEIGHT-y,0);
        else
            gl.glTranslated(GLField.WIDTH-x,y,0);
			
		double ax2, ay2;
        if(fill.equals(Color.blue)) {
			ax2 = ax;
			ay2 = -ay;
            //gl.glRotated(hDeg,0,0,1);
        } else {
			ax2 = -ax;
			ay2 = ay;
            //gl.glRotated(180,0,0,1);
		}
			
		setColor(gl,Color.cyan);
		gl.glBegin( gl.GL_LINES );
        gl.glVertex2d(0.0,0.0);
		gl.glVertex2d(ax2,ay2);
        gl.glEnd();
		
		gl.glPopMatrix();
	}
    
    public void setLocation(double x, double y) {
        setLocation(x,y,0);
    }
    
    public void setLocation(double x, double y, double h) {
        this.x = x;
        this.y = y;
        this.h = h;
    }
    
    public void setConfidenceFactor(double cf) {
        //this.cf = cf;
    }
    
    public void setCovariances(double varX, double varY, double varH, double varXY) {
        vx = varX;
        vy = varY;
        vh = varH;
        vxy = varXY;
    }
    
    public void setColor(Color c) {
        fill = c;
    }
    
    public void setSelf(boolean s) {
        self = s;
    }

}
