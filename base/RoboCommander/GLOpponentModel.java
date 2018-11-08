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
 * UNSW 2003 Robocup
 *
 * Last modification background information
 * $$
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;
// import gl4java.awt.*;


public class GLOpponentModel {

	protected static final int NOSHOW_MODE = 0;
	protected static final int CENTER_ONLY_MODE = 1;
	protected static final int VARIANCES_MODE = 2;
	protected int mode;

	protected int NUM_OPPONENTS = 2;
	protected static final int NUM_ATTR = 5;
	GLOpponent opponent[] = new GLOpponent[NUM_OPPONENTS];
	
	public GLOpponentModel() {
	
		mode = NOSHOW_MODE;
		for(int i=0 ; i<NUM_OPPONENTS ; i++)
			opponent[i] = new GLOpponent();
			
		/*opponent[0].setLocation(100,200);
		opponent[0].setCovariances(400,1600,0);
		opponent[0].setTeam(1);*/
		
	}
	
	public void paint(GL gl, GLU glu, GLDrawable glc) {
		if(mode == VARIANCES_MODE) {
			for(int i=0 ; i<NUM_OPPONENTS ; i++)
			opponent[i].paint(gl, glu, glc);
		}
		
		if(mode == VARIANCES_MODE || mode == CENTER_ONLY_MODE) {
			for(int i=0 ; i<NUM_OPPONENTS ; i++)
				opponent[i].paintCenter(gl, glu, glc);
		}
	}
	
	public void processData(int team, double[] data, int base, int dataLen) {
	
		if(dataLen%NUM_ATTR != 0) {
			System.out.println("Error - total attributes not divisible by attributes per opponent");
			System.out.println(dataLen + " " + NUM_ATTR);
			return;
		}
		
		int l = (data.length-base) > dataLen ? dataLen : (data.length-base);
		int no = l/NUM_ATTR;
		if(no != NUM_OPPONENTS) {
			NUM_OPPONENTS = no;
			opponent = new GLOpponent[NUM_OPPONENTS];
			for(int i=0 ; i<NUM_OPPONENTS ; i++)
				opponent[i] = new GLOpponent();
		}
		
		int oppTeam = GLOpponent.BLUE;
		if(team == GLOpponent.BLUE)
			oppTeam = GLOpponent.RED;
		for(int i=0 ; i<NUM_OPPONENTS ; i++) {
			
			int index = base+i*NUM_ATTR;
			opponent[i].setTeam(oppTeam);
			opponent[i].setLocation(data[index], data[index+1]);
            if (RoboWirelessBase.worldModelDebug) {
                System.out.println("["+index+"] = "+data[index]+ " (opponent location 1)");
                System.out.println("["+(index+1)+"] = "+data[index+1]+ " (opponent location 2)");
            }
			opponent[i].setCovariances(data[index+2], data[index+3], data[index+4]);
            if (RoboWirelessBase.worldModelDebug) {
                System.out.println("["+(index+2)+"] = "+data[index+2]+ " (opponent covariances 3)");
                System.out.println("["+(index+3)+"] = "+data[index+3]+ " (opponent covariances 4)");
                System.out.println("["+(index+4)+"] = "+data[index+4]+ " (opponent covariances 5)");
            }		
		}
	
	}
	
	public String changeMode() {
		if(mode == NOSHOW_MODE)  {
			mode = CENTER_ONLY_MODE;
			return "center";
		} else if(mode == CENTER_ONLY_MODE) {
			mode = VARIANCES_MODE;
			return "variances";
		} else { // VARIANCES_MODE
			mode = NOSHOW_MODE;
			return "off";
		}
	}
	
	public void dumpData() {
		for(int i=0 ; i<NUM_OPPONENTS ; i++) {
			System.out.print(i+" - ");
			opponent[i].dumpData();
			System.out.println();
		}
	}

}


class GLOpponent extends GLRoboObject {

	public static final int RED = 1, BLUE = -1;

	protected double x, y, xvar, yvar, xycov;
	protected int team = RED;
	
	protected Color colCenterRed = new Color((float)0.6,(float)0.1,(float)0.1,(float)0.3);
	protected Color colInsideRed = new Color((float)0.7,(float)0.3,(float)0.3,(float)0.3);
	protected Color colOutsideRed = new Color((float)0.8,(float)0.4,(float)0.4,(float)0.3);
	
	protected Color colCenterBlue = new Color((float)0.1,(float)0.1,(float)0.6,(float)0.3);
	protected Color colInsideBlue = new Color((float)0.3,(float)0.3,(float)0.7,(float)0.3);
	protected Color colOutsideBlue = new Color((float)0.4,(float)0.4,(float)0.8,(float)0.3);
	
	public GLOpponent() {
	}
	
	public void paintCenter(GL gl, GLU glu, GLDrawable glc) {
	
		gl.glPushMatrix();
		if(team == BLUE)
            gl.glTranslated(GLField.WIDTH-x,y,0);
        else
            gl.glTranslated(x,GLField.HEIGHT-y,0);
			
		// draw unit and size 2 disc
		// use opposite colour of our team colour
		// note - alpha not working 
		if(team == BLUE)
        	setColor(gl, colCenterBlue);
		else // red 
        	setColor(gl, colCenterRed);
        gl.glBegin( gl.GL_POLYGON );
        for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = 10*Math.cos(t);
            double ty = 10*Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
		
		gl.glPopMatrix();
	
	}
	
	public void paint(GL gl, GLU glu, GLDrawable glc) {
		double b = -(xvar + yvar);
        double c = xvar*yvar - xycov*xycov;
		
		// determine eigenvalues which are variances & standard deviations
        double v1 = (-b + Math.sqrt(b*b - 4*c))/2;
		double sd1 = Math.sqrt(v1);
        double v2 = -b - v1;
		double sd2 = Math.sqrt(v2);
		
		// determine the eigenvector (only need one as other is orthogonal)
        double ev1 = 0;
        double ev2 = 0;
        if(xvar-v1 == 0) {
            ev1 = 1;
            ev2 = 0;
        } else if(xycov == 0) {
            ev1 = 0;
            ev2 = 1;
        } else {
            ev1 = xycov;
            ev2 = v1-xvar;
        }
        
        // determine angle to match eigenvectors to axis and rotate then scale
        double angle = -Math.atan2(ev2, ev1)*(180/Math.PI);
		
		gl.glPushMatrix();
		if(team == BLUE)
            gl.glTranslated(GLField.WIDTH-x,y,0);
        else
            gl.glTranslated(x,GLField.HEIGHT-y,0);
        gl.glRotated(angle, 0, 0, 1);
        gl.glScaled(sd1, sd2, 1);
        
        // draw unit and size 2 disc
		// use opposite colour of our team colour
		// note - alpha not working 
		if(team == BLUE)
        	setColor(gl, colOutsideBlue);
		else // red
        	setColor(gl, colOutsideRed);
        gl.glBegin( gl.GL_POLYGON );
        for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = 2*Math.cos(t);
            double ty = 2*Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
		
        if(team == BLUE)
        	setColor(gl, colInsideBlue);
		else // red
        	setColor(gl, colInsideRed);
		gl.glBegin( gl.GL_POLYGON );
        for (double t = 0; t <= Math.PI*2 + STEP; t += STEP) {
            double tx = Math.cos(t);
            double ty = Math.sin(t);
            gl.glVertex2d(tx, ty);
        }
        gl.glEnd();
        
        gl.glPopMatrix();
	}
	
	public void setLocation(double x, double y) {
		this.x = x;
		this.y = y;
	}
	
	public void setLocation(double x, double y, double h) {
        this.x = x;
        this.y = y;
    }
	
	public void setConfidenceFactor(double cf) {
        //this.cf = cf;
    }
	
	public void setCovariances(double varX, double varY, double varXY) {
        xvar = varX;
        yvar = varY;
        xycov = varXY;
    }
	
	public void setTeam(int t) {
		team = t;
	}
	
	public void dumpData() {
		System.out.println("{"+x+","+y+"}");
		System.out.println("\tcov="+"{{"+xvar+","+xycov+"},{"+xycov+","+yvar+"}}");
	}

}
