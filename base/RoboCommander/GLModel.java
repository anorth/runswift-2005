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
 * $Id: GLModel.java 2001 2003-09-24 12:02:35Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;


class GLModel /* implements GLEnum */ {
	
    protected static final int WIDTH = 10;
    protected static final int WIDTH2 = 20;
    protected static final double STEP  = Math.PI / 15;
	
    public static final int RED = 1, BLUE = -1;
    public static final int NUM_TEAM_MEMBER     = 3;//4 // maximum number of dogs per team
    public static final int ROBOT_ATTRIB_NUM    = 4; // number of attributs for the dogs (x,y,h,cf)
    public static final int ROBOT_ATTRIB_NUM2   = 9; // new number of attributs for the dogs
	public static final int VISUAL_PULL_ATTR_NUM = 3; // number of attributes in visual pull vector
    public static final int TEAMMATE_ATTRIB_NUM = 5; // number of attributs for the dogs (x,y,h,cf)
    public static final int OTHER_ATTRIB_NUM    = 3; // number of attributs for objects other than dogs (x,y,cf)
	public static final int BALL_ATTRIB_NUM     = 9; // number of attributs for the ball (x,y,vx,vy,varx,vary,varxy,varvx,varvy)
	public static final int WBALL_ATTRIB_NUM    = 3; // number of attributs for the wireless ball (robot num, x, y)
	static final double INVALID_FLAG = -1234; //if the dog sets its its teammates heading variance to be this
										     //number, then that teammate doesnt exist

	boolean haveSetColors = false;
	
	// true if we can see the ball
	boolean ballVis = false;
    
    GLRoboDog[] team; //      = new GLRoboDog[NUM_TEAM_MEMBER];
                //opponents = new GLRoboDog[NUM_TEAM_MEMBER];
                
    //GLRoboDog[] robots    = new GLRoboDog[NUM_TEAM_MEMBER*2]; // red dogs first then blue dogs
    GLRoboDog   self      = new GLRoboDog();

    GLGoalCenter targetGoal = new GLGoalCenter(),
                 ownGoal    = new GLGoalCenter();
    GLBall       ball       = new GLBall();
	
	// for visual pull
	double vpx, vpy, vph;
    
    // for direct update
    double directx, directy, directh;
	
	boolean showWBall = false;
	boolean showPull = false;
	int sbRobotNum;
	double wirelessBallx, wirelessBally;
	Color wBallColour = Color.magenta;
    
    int teamColor;
    
    GLModel() {
	    team = new GLRoboDog[NUM_TEAM_MEMBER];

		team[0] = new GLRoboDog();
		team[1] = new GLRoboDog();
		team[2] = new GLRoboDog();
//		team[3] = new GLRoboDog();

        targetGoal.setColor(Color.red);
        ownGoal.setColor(Color.black);
        self.setSelf(true);
        
        directx = -1;
        directy = -1;
        directh = -1;
    }
	
	//private void setAllColors

    public void processData(int playerNum, int tColor, double[] info, int start, int len) {
        teamColor = tColor;
        // find out the number of dogs
        //int numDogs = (len - ROBOT_ATTRIB_NUM - 3*OTHER_ATTRIB_NUM) / ROBOT_ATTRIB_NUM;
        
        int base = start;
        
        //System.out.println("GLModel.processData: Len = " + len + " Number of robots = " + numDogs +
        //                   " start = " + start);

        if (teamColor == RED) {

            self.setColor(Color.red);

            if (team[0] != null) {
                team[0].setColor(Color.red);
            }

            if (team[1] != null) {
                team[1].setColor(Color.red);
            }

            if (team[2] != null) {
                team[2].setColor(Color.red);
            }

//            if (team[3] != null) {
//                team[3].setColor(Color.red);
//            }

            ball.setTeam(Color.red);

        } else {
            self.setColor(Color.blue);

            if (team[0] != null) {
                team[0].setColor(Color.blue);
            }

            if (team[1] != null) {
                team[1].setColor(Color.blue);
            }

            if (team[2] != null) {
                team[2].setColor(Color.blue);
            }

//            if (team[3] != null) {
//                team[3].setColor(Color.blue);
//            }

            ball.setTeam(Color.blue);
        }            

        /////////
        // self
        /////////
        self.setLocation(info[base], info[base + 1], info[base + 2]);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+base+"] = "+info[base]+ " (setLocation1)");
            System.out.println("["+(base+1)+"] = "+info[base+1]+ " (setLocation2)");
            System.out.println("["+(base+2)+"] = "+info[base+2]+ " (setLocation3)");
        }
        self.setCovariances(info[base+3],info[base+4],info[base+5],info[base+6]);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base+3)+"] = "+info[base+3]+ " (setCovariance1)");
            System.out.println("["+(base+4)+"] = "+info[base+4]+ " (setCovariance2)");
            System.out.println("["+(base+5)+"] = "+info[base+5]+ " (setCovariance3)");
            System.out.println("["+(base+6)+"] = "+info[base+6]+ " (setCovariance4)");
        }
        base += ROBOT_ATTRIB_NUM2;

		///////////////
		// visual pull
		///////////////
		vpx = info[base];
		vpy = info[base+1];
		vph = info[base+2];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+base+"] = "+vpx+ " (vpx)");
            System.out.println("["+(base+1)+"] = "+vpy+ " (vpy)");
            System.out.println("["+(base+2)+"] = "+vph+ " (vph)");
        }
		base += VISUAL_PULL_ATTR_NUM;

        ////////////
        // ball info
        ////////////
        ball.setLocation(info[base],info[base+1]);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+base+"] = "+info[base]+ " (ballLocation1)");
            System.out.println("["+(base+1)+"] = "+info[base+1]+ " (ballLocation2)");
        }
	    ball.setVelocity(info[base+2],info[base+3]);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base+2)+"] = "+info[base+2]+ " (setVelocity1)");
            System.out.println("["+(base+3)+"] = "+info[base+3]+ " (setVelocity2)");
        }
        ball.setCovariance(info[base+4],info[base+5],info[base+6],info[base+7],info[base+8]);
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base+4)+"] = "+info[base+4]+ " (setCovariance1)");
            System.out.println("["+(base+5)+"] = "+info[base+5]+ " (setCovariance2)");
            System.out.println("["+(base+6)+"] = "+info[base+6]+ " (setCovariance3)");
            System.out.println("["+(base+7)+"] = "+info[base+7]+ " (setCovariance4)");
            System.out.println("["+(base+8)+"] = "+info[base+8]+ " (setCovariance5)");
        }
        base += BALL_ATTRIB_NUM;
		
		/////////////////
		// wireless ball
		/////////////////
		sbRobotNum = (int)info[base];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+base+"] = "+info[base]+ " (sbRobotNum)");
        }
		wirelessBallx = info[base+1];
		wirelessBally = info[base+2];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base+1)+"] = "+info[base+1]+ " (wirelessBallx)");
            System.out.println("["+(base+2)+"] = "+info[base+2]+ " (wirelessBally)");
        }
		base += WBALL_ATTRIB_NUM;
        
		////////////
		// teammates
		////////////
		if ( len <= ROBOT_ATTRIB_NUM2+BALL_ATTRIB_NUM ) {
			team[0]=null;
			team[1]=null;
			team[2]=null;
			team[3]=null;
		}
		else {
	        for (int i=0; i<NUM_TEAM_MEMBER; i++) {

				if (info[base+4]!=INVALID_FLAG) {

					team[i].setLocation(info[base], info[base + 1], info[base + 2]);

                    if (RoboWirelessBase.worldModelDebug) {
                        System.out.println("["+base+"] = "+info[base]+ " (teammate setLocation1)");
                        System.out.println("["+(base+1)+"] = "+info[base+1]+ " (teammate setLocation2)");
                        System.out.println("["+(base+2)+"] = "+info[base+2]+ " (teammate setLocation3)");
                    }

        			team[i].setCovariances(info[base+3],info[base+3],info[base+4],0);

                    if (RoboWirelessBase.worldModelDebug) {
                        System.out.println("["+(base+3)+"] = "+info[base+3]+ " (teammate setCovariance1)");
                        System.out.println("["+(base+4)+"] = "+info[base+4]+ " (teammate setCovariance2)");
                    }
				} else {
					team[i]=null;
				}
				base += TEAMMATE_ATTRIB_NUM;
			}
		}

        /*
		/////////////
        // goal info
        /////////////
        targetGoal.setLocation(info[base],info[base+1]);
        targetGoal.setConfidenceFactor(info[base+2]);
        base += OTHER_ATTRIB_NUM;
        ownGoal.setLocation(info[base],info[base+1]);
        ownGoal.setConfidenceFactor(info[base+2]);
        base += OTHER_ATTRIB_NUM;
        */
    }

    private GLRoboDog createTeamRobot() {
        GLRoboDog r = new GLRoboDog();
        if (teamColor == (RED)) {
            r.setColor(Color.red);
        } else {
            r.setColor(Color.blue);
        }
        return r;
    }
    
    private GLRoboDog createOpponentRobot() {
        GLRoboDog r = new GLRoboDog();
        if (teamColor == (BLUE)) {
            r.setColor(Color.red);
        } else {
            r.setColor(Color.blue);
        }
        return r;
    }
        
    public void paint(GL gl, GLU glu, GLDrawable glc) {
        // draw dogs

		for (int i = 0; i < NUM_TEAM_MEMBER; i++) {
            if (team[i] != null) {
                team[i].paint(gl,glu,glc);
			}
        }
        
        // draw goals
        targetGoal.paint(gl,glu,glc);
        ownGoal.paint(gl,glu,glc);

        self.paint(gl,glu,glc);
		
		if(showPull) {
			self.paintArrow(gl,glu,glc,10*vpx,10*vpy);
		}
        
        // draw ball
        ball.paint(gl,glu,glc,ballVis);
		
		// draw wirless ball
		if(showWBall) {
			gl.glPushMatrix();
		 	if(teamColor == RED) {
		 		gl.glTranslated(GLField.WIDTH-wirelessBallx,wirelessBally,0);
			} else {
				gl.glTranslated(wirelessBallx,GLField.HEIGHT-wirelessBally,0);
			}
			setColor(gl,wBallColour);
			gl.glBegin( gl.GL_POLYGON );
			for (double t = 0; t <= Math.PI*2; t += STEP) {
            	double tx = WIDTH/2*Math.cos(t);
            	double ty = WIDTH/2*Math.sin(t);
            	gl.glVertex2d(tx, ty);
        	}
        	gl.glEnd();
			
			// black circle
        	setColor(gl,Color.black);
        	gl.glBegin( gl.GL_LINE_STRIP );
        	for (double t = 0; t <= Math.PI*2; t += STEP) {
            	double tx = WIDTH/2*Math.cos(t);
            	double ty = WIDTH/2*Math.sin(t);
            	gl.glVertex2d(tx, ty);
        	}
        	gl.glEnd();
			
			gl.glPopMatrix();
		}
        
        if(directx >= 0.0 && directy >= 0.0 && directh >= 0.0) {
			gl.glPushMatrix();
		 	if(teamColor == RED) {
		 		gl.glTranslated(GLField.WIDTH-directx,directy,0);
                gl.glRotated(180-directh,0,0,1);
			} else {
				gl.glTranslated(directx,GLField.HEIGHT-directy,0);
                gl.glRotated(-directh,0,0,1);
			}
            
            setColor(gl,Color.magenta);
            gl.glBegin( gl.GL_POLYGON );        
            gl.glVertex2d(WIDTH2/2, 0);
            gl.glVertex2d(0, WIDTH2/2*2/3);
            gl.glVertex2d(0, WIDTH2/2*1/4);
            gl.glVertex2d(-WIDTH2/2*2/3, WIDTH2/2*1/4);
            gl.glVertex2d(-WIDTH2/2*2/3, -WIDTH2/2*1/4);
            gl.glVertex2d(0, -WIDTH2/2*1/4); 
            gl.glVertex2d(0, -WIDTH2/2*2/3);        
            gl.glEnd();
            
			gl.glPopMatrix();
        }
    }
    
    public void createTestObjects() {
        double[] randomData = {Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     Math.random()*200,Math.random()*300,Math.random()*360,Math.random()*1000,
                     // goals
                     70 + Math.random() * 60, 300 + Math.random()*30, Math.random()*1000,
                     70 + Math.random() * 60, -Math.random()*30, Math.random()*1000,
                     //ball
                     Math.random()*200,Math.random()*300,Math.random()*1000};
        processData(1,RED,randomData,0,41);
    }
 
    public void setBallColor(Color c) {
        ball.setColor(c);
    }
	
	public void setSeeBall(boolean b) {
		ballVis = b;
	}
	
	public void switchWBall() {
		if(showWBall)
			showWBall = false;
		else
			showWBall = true;
	}
	
	public void switchVPull() {
		if(showPull)
			showPull = false;
		else {
			System.out.println("vPull={" + vpx + "," + vpy + "," + vph + "}");
			showPull = true;
		}
	}
	
	public void setColor(GL gl, Color c){
        //gl.glColor3ub((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue());
    	gl.glColor4ub((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue(),(byte)c.getAlpha());
	}
    
    public void setDirect(double dx, double dy, double dh) {
        directx = dx;
        directy = dy;
        directh = dh;
    }
    
    public void printDirect() {
        System.out.println("Direct GPS Update {"+directx+","+directy+","+directh+"}");
    }
}



