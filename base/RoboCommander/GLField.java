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
 * $Id: GLField.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;


public class GLField extends GLRoboObject {
    // large field
    public static final int WIDTH = 360, HEIGHT = 540/*, CORNER = 0, WALL = 0*/;
    public static final int SIDE_THRESH = 20, END_THRESH = 30;
	public static final int RADIUS = 18, BOX_WIDTH = 130, BOX_DEPTH = 65;
	public static final int STEP = 20;

    protected static final Color[][] BEACON_COLORS = {
        {Color.blue,Color.pink},{Color.pink,Color.blue},
        {Color.yellow,Color.pink},{Color.pink,Color.yellow}
    };

    protected static final int[][] BEACON_LOC = {
        {-15, 3*HEIGHT/4}, {WIDTH + 15, 3*HEIGHT/4},
        {-15, HEIGHT/4}, {WIDTH + 15, HEIGHT/4}};

    protected static final Color[] GOAL_COLORS = {
        Color.blue,Color.yellow
    };

    protected static final int[][] GOAL_LOC = {
        {WIDTH/2, HEIGHT+15}, {WIDTH/2, -15}
    };

    /*
    protected static final int[][] FIELD_CORNER = {
        {CORNER - WALL - WIDTH/2 ,0 + WALL - HEIGHT/2},

        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , 0 + WALL - HEIGHT/2},
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , 0 - HEIGHT/2},
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , 0 - HEIGHT/2},
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , 0 + WALL - HEIGHT/2},
        

        {WIDTH - CORNER + WALL - WIDTH/2,0 + WALL - HEIGHT/2},
        {WIDTH - WALL - WIDTH/2,CORNER - WALL - HEIGHT/2},
        {WIDTH - WALL - WIDTH/2,HEIGHT - CORNER + WALL - HEIGHT/2},
        {WIDTH - CORNER + WALL - WIDTH/2,HEIGHT - WALL - HEIGHT/2},

        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - WALL - HEIGHT/2},
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - HEIGHT/2},
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - HEIGHT/2},
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - WALL - HEIGHT/2},

        {CORNER - WALL - WIDTH/2,HEIGHT - WALL - HEIGHT/2},
        {0 + WALL - WIDTH/2,HEIGHT - CORNER + WALL - HEIGHT/2},
        {0 + WALL - WIDTH/2,CORNER - WALL - HEIGHT/2},
        {CORNER - WALL - WIDTH/2 ,0 + WALL - HEIGHT/2}
    };

    protected static final int[][] FIELD_CORNER1 = {
        {CORNER - WALL - WIDTH/2 ,0 + WALL - HEIGHT/2},
        {WIDTH - CORNER + WALL - WIDTH/2,0 + WALL - HEIGHT/2},
        {WIDTH - WALL - WIDTH/2,CORNER - WALL - HEIGHT/2},
        {WIDTH - WALL - WIDTH/2,HEIGHT - CORNER + WALL - HEIGHT/2},
        {WIDTH - CORNER + WALL - WIDTH/2,HEIGHT - WALL - HEIGHT/2},
        {CORNER - WALL - WIDTH/2,HEIGHT - WALL - HEIGHT/2},
        {0 + WALL - WIDTH/2,HEIGHT - CORNER + WALL - HEIGHT/2},
        {0 + WALL - WIDTH/2,CORNER - WALL - HEIGHT/2},
        {CORNER - WALL - WIDTH/2 ,0 + WALL - HEIGHT/2}
    };

    protected static final int[][] FIELD_CORNER2 = {
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , 0 + WALL - HEIGHT/2},
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , 0 - HEIGHT/2},
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , 0 - HEIGHT/2},
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , 0 + WALL - HEIGHT/2}
    };

    
    protected static final int[][] FIELD_CORNER3 = {
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - WALL - HEIGHT/2},
        {WIDTH/2 + GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - HEIGHT/2},
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - HEIGHT/2},
        {WIDTH/2 - GLGoal.WIDTH/2 - WIDTH/2 , HEIGHT - WALL - HEIGHT/2}
    };
	*/
    
	protected static final int[] GOAL_VIEW = {2, 4};
	protected static final int[] BEACON_VIEW = 
		{16, 8, 256, 128};
        
    protected static final int[] LINE_VIEW = 
        {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    protected static final int[] LINE_POS = 
        {  0,   0,     0, WIDTH,    //0  Home goal line
         115,   0,   115,    65,    //1  Left edge of home goal box
         245,   0,   245,    65,    //2  Right edge of home goal box
         115,  65,   245,    65,    //3  Top edge of home goal box
           0, HEIGHT/2, WIDTH, HEIGHT/2,  //4  Midline, near line
         115, HEIGHT-65,  245, HEIGHT-65,  //5 Top edge of target goal box
         115, HEIGHT-65,  115, HEIGHT,  //6 Left edge of target goal box
         245, HEIGHT-65,  245, HEIGHT,  //6 Right edge of target goal box
         0,   HEIGHT,   WIDTH, HEIGHT,  //8 Target goal line
         0,  0,  0,  HEIGHT,  //9 Left edge of field
         WIDTH, 0,  WIDTH, HEIGHT,  //10 Right edge of field
         //40,  10,  250, 10,   //11 Baseline
         //40,  430, 250, 430  //12 Target line
         //40,  10,  10,  40,   //13 Bottom left cutoff
         //250, 10,  280, 40,   //14 Bottom right cutoff
         //10,  400, 40,  430,  //15 Top left cutoff
         //280, 400, 250, 430   //16 Top right cutoff
     };

    protected static final int DEFAULT_X = WIDTH/2, DEFAULT_Y = HEIGHT/2;

    protected Color fill;
    protected double x, y, cf;

    protected GLBeacon[] beacons;
    protected GLGoal[] goals;

	protected int view;
    protected int team;

    public GLField() {
        fill = Color.green;
        beacons = new GLBeacon[BEACON_COLORS.length];
        goals  = new GLGoal[2];
        x = DEFAULT_X;
        y = DEFAULT_Y;
        initObjects();

    }

    private void initObjects() {
        // beacons
        for (int i = 0; i < BEACON_COLORS.length; i++) {

            GLBeacon b = new GLBeacon();
            b.setColor(BEACON_COLORS[i][0],BEACON_COLORS[i][1]);
            b.setLocation(BEACON_LOC[i][0],BEACON_LOC[i][1]);
            beacons[i] = b;
        }

        // goals
        for (int i = 0; i < GOAL_COLORS.length; i++) {

            GLGoal g = new GLGoal();
            g.setColor(GOAL_COLORS[i]);
            g.setLocation(GOAL_LOC[i][0],GOAL_LOC[i][1]);
            goals[i] = g;
        }

    }

    // field center is at WIDTH/2, HEIGHT/2
    // but drawn such at 0,0 is at center of drawing axis
    public void paint(GL gl, GLU glu, GLDrawable glc) {

        gl.glDisable(gl.GL_TEXTURE_2D);
        gl.glLineWidth(LINE_WIDTH);

        gl.glPushMatrix();
        gl.glTranslated(x,y,0);

        //draw the field as polygon
        setColor(gl,fill);
        gl.glBegin( gl.GL_POLYGON );
//        for (int i = 0; i < FIELD_CORNER1.length; i++) {
//            gl.glVertex2d(FIELD_CORNER1[i][0],FIELD_CORNER1[i][1]);
//        }
//        gl.glEnd();
//        gl.glBegin( gl.GL_POLYGON );
//        for (int i = 0; i < FIELD_CORNER2.length; i++) {
//            gl.glVertex2d(FIELD_CORNER2[i][0],FIELD_CORNER2[i][1]);
//        }
//        gl.glEnd();
//        gl.glBegin( gl.GL_POLYGON );
//        for (int i = 0; i < FIELD_CORNER3.length; i++) {
//            gl.glVertex2d(FIELD_CORNER3[i][0],FIELD_CORNER3[i][1]);
//        }
        gl.glVertex2d(-WIDTH/2 - SIDE_THRESH, -HEIGHT/2 - END_THRESH);
        gl.glVertex2d(-WIDTH/2 - SIDE_THRESH, HEIGHT/2 + END_THRESH);
        gl.glVertex2d(WIDTH/2 + SIDE_THRESH, HEIGHT/2 + END_THRESH);
        gl.glVertex2d(WIDTH/2 + SIDE_THRESH, -HEIGHT/2 - END_THRESH);
        gl.glEnd();

        // sidelines
        setColor(gl,Color.white);
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(-WIDTH/2.0, -HEIGHT/2);
        gl.glVertex2d(-WIDTH/2.0, HEIGHT/2);
        gl.glEnd();
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(WIDTH/2.0, -HEIGHT/2);
        gl.glVertex2d(WIDTH/2.0, HEIGHT/2);
        gl.glEnd();

        // goal lines
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(-WIDTH/2.0, -HEIGHT/2);
        gl.glVertex2d(WIDTH/2.0, -HEIGHT/2);
        gl.glEnd();
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(-WIDTH/2.0, HEIGHT/2);
        gl.glVertex2d(WIDTH/2.0, HEIGHT/2);
        gl.glEnd();


        // white center line
        gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(-WIDTH/2.0,0);
        gl.glVertex2d(WIDTH/2.0,0);
        gl.glEnd();
		gl.glBegin( gl.GL_LINE_STRIP );
        for (double t = 0; t <= STEP; t++) {
			double val = (t/STEP)*Math.PI*2;
            double tx = RADIUS*Math.cos(val);
            double ty = RADIUS*Math.sin(val);
            gl.glVertex2d(tx, ty);
        }
		gl.glEnd();

		// white goal boxes
		gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(BOX_WIDTH/2.0,-HEIGHT/2);
        gl.glVertex2d(BOX_WIDTH/2.0,-HEIGHT/2+BOX_DEPTH);
        gl.glVertex2d(-BOX_WIDTH/2.0,-HEIGHT/2+BOX_DEPTH);
        gl.glVertex2d(-BOX_WIDTH/2.0,-HEIGHT/2);
        gl.glEnd();
		
		
		gl.glBegin( gl.GL_LINE_STRIP );
        gl.glVertex2d(BOX_WIDTH/2.0,HEIGHT/2);
        gl.glVertex2d(BOX_WIDTH/2.0,HEIGHT/2-BOX_DEPTH);
        gl.glVertex2d(-BOX_WIDTH/2.0,HEIGHT/2-BOX_DEPTH);
        gl.glVertex2d(-BOX_WIDTH/2.0,HEIGHT/2);
        gl.glEnd();

        // black bounding box
        //setColor(gl,Color.black);
        //gl.glBegin( gl.GL_LINE_STRIP );
        //for (int i = 0; i < FIELD_CORNER.length; i++) {
        //    gl.glVertex2d(FIELD_CORNER[i][0],FIELD_CORNER[i][1]);
        //}
        //gl.glEnd();

        gl.glPopMatrix();
		
        // draw beacons and goal
        for (int i = 0; i < beacons.length; i++) {
            beacons[i].paint(gl,glu,glc,(view & BEACON_VIEW[i]) != 0);
        }
        for (int i = 0; i < goals.length; i++) {
            goals[i].paint(gl,glu,glc,(view & GOAL_VIEW[i]) != 0);
        }
        
        // draw lines we can see
        for(int i=0 ; i < LINE_VIEW.length ; i++) {
            if((view & (1 << LINE_VIEW[i])) != 0) {
                int x1 = LINE_POS[i*4];
                int x2 = LINE_POS[i*4+2];
                int y1 = LINE_POS[i*4+1];
                int y2 = LINE_POS[i*4+3];
                if(team == -1) {
                    x1 = HEIGHT - x1;
                    x2 = HEIGHT - x2;
                    y1 = WIDTH - y1;
                    y2 = WIDTH - y2;
                }
                setColor(gl,Color.MAGENTA);
                gl.glBegin( gl.GL_LINE_STRIP );
                gl.glVertex2d(x1, y1);
                gl.glVertex2d(x2,y2);
                gl.glEnd();
            }
        }

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
	
	/** Sets the variable view according to what the dog can see aswell as team
	  */
	public void setView(int v, int t) {
		view = v;
        team = t;
	}
}
