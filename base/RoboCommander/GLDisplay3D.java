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
 *  UNSW 2003 Robocup (Ross Edwards)
 *
 *  Last modification background information
 * $Id$
 *
 *  Copyright (c) 2003 UNSW
 * All Rights Reserved
 *
 */
 
import java.awt.*;
import javax.swing.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import net.java.games.jogl.*;
import net.java.games.jogl.util.*;
// import gl4java.drawable.*;
// // import gl4java.awt.*;
// import gl4java.utils.glut.*;
import Jama.*;

public class GLDisplay3D extends JFrame implements 
    GLEventListener, KeyListener, MouseListener, MouseMotionListener {

    BorderLayout mainLayout = new BorderLayout();
    JPanel mainDisplay = new JPanel();
    BorderLayout borderLayout1 = new BorderLayout();
    JPanel jPanel1 = new JPanel();
    FlowLayout flowLayout1 = new FlowLayout();
    JPanel jPanel2 = new JPanel();
    JToggleButton chkPause = new JToggleButton();
	JToggleButton chkNaturalRotation = new JToggleButton();
	JButton output = new JButton();
    GridLayout gridLayout1 = new GridLayout();
    
    /** Opengl pipelines to render the scene */
    GL gl;
    GLU glu;
    // GLContext glj;
    GLCanvas glc;
    
    /** viewing information - viewdir & up should be kept unit length */
    float[] clearcol = {0.0f,0.0f,0.0f,1.0f};
    float[] viewpos = {600f,200f,0f,1f};
    float[] viewdir = {-1f,0f,0f,0f};
    float[] up = {0f,0f,1f};
	float[] universalUp = {0f,0f,1f};
    
    /** 3d graphics variables used */
	static final double CENTER_RAD = 3;
    static final int numSlices = 8;
    static final int numStacks = 8;
    static final double cylRad = 20;
    static final double cylHeight = 360;
	int wireFactor = 1;
	static final double MOVE_FAC = 2;
    
    /** width and height of display area */
    static final int WIDTH = 500, HEIGHT = 500;
    int width = WIDTH, height = HEIGHT;
	
	/** for calculations */
	public static final double EPSILON = 0.000001;
	public static final double THRESHOLD = 0.02;
    
    /** data */
    public static final double FIELD_LENGTH = GLField.HEIGHT;
    public static final double FIELD_WIDTH = GLField.WIDTH;
    public static final double HEADING_MAX = 360;
	public static final double MOUSE_ANGLE_MOVE = 0.02;
    public static final int RED = 1, BLUE = -1;
    int teamColour = RED;
    int x, y;
	
	Color oldCol = Color.yellow;
	double oldx=0, oldy=0, oldh=0;
	double oldvx=1, oldvy=1, oldvh=1;
	double oldcxy=0, oldcxh=0, oldcyh=0;
	Color visCol = Color.red;
	double visx=0, visy=0, vish=0;
	double visvx=1, visvy=1, visvh=1;
	double viscxy=0, viscxh=0, viscyh=0;
	Color newCol = Color.white;
	double newx=0, newy=0, newh=0;
	double newvx=1, newvy=1, newvh=1;
	double newcxy=0, newcxh=0, newcyh=0;
    
	int b = 0;
	
    public GLDisplay3D() {
        try {
            swingInit();
            glInit();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
    
    private void swingInit() {
        this.getContentPane().setLayout(mainLayout);
        this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        this.addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                System.out.println("GLDisplay3D: Window is not allowed to be closed");
                System.exit(0);
            }
        });
        mainDisplay.setLayout(borderLayout1);
        jPanel1.setPreferredSize(new Dimension(150, 150));
        jPanel1.setLayout(flowLayout1);
        chkPause.setText("Pause");
        chkPause.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                modelClicked(e);
            }
        });
		chkNaturalRotation.setText("Natural Rotation");
        chkNaturalRotation.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
				for(int i=0 ; i<up.length ; i++)
					universalUp[i] = up[i];
                modelClicked(e);
            }
        });
		output.setText("Output");
		output.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				printOut();
			}
		});
        jPanel2.setLayout(gridLayout1);
        gridLayout1.setRows(3);
        gridLayout1.setColumns(1);
        this.getContentPane().add(mainDisplay, BorderLayout.CENTER);
        mainDisplay.add(jPanel1, BorderLayout.EAST);
        jPanel1.add(jPanel2, null);
        jPanel2.add(chkPause, null);
		jPanel2.add(chkNaturalRotation,null);
		jPanel2.add(output,null);
    }
    
    private void glInit() {
    
        /** 
         * Create a new GLComponent from the factory 
         */
        GLCapabilities caps = new GLCapabilities();
        caps.setDoubleBuffered( true );
        // caps.setTrueColor(true);
        //get a  GLCanvas from the factory
 	    glc = GLDrawableFactory.getFactory().createGLCanvas(caps);
		glc.setSize(WIDTH, HEIGHT);
    
        /** Add the GLComponent to the Panel and display it */
        mainDisplay.add( "Center", glc );
    
        /**
         * Add the GLEventListener to handle GL events
         * and initialize!
         */
        glc.addGLEventListener( this );
        glc.addKeyListener(this);
		glc.addMouseListener(this);
        glc.addMouseMotionListener(this);
        pack();
        show();
    }
    
    
    /* executed once to initialise the associated GLDrawable */
    public void init(GLDrawable drawable) {
    	System.out.println("GLDisplay3D.init()");
        gl = drawable.getGL();
        glu = drawable.getGLU();
        // glj = drawable.getGLContext();
        gl.glEnable(gl.GL_LIGHTING);
        
        /** 
         * Set the background colour when the GLComponent 
         * is cleared 
         */ 
        gl.glClearColor(clearcol[0],clearcol[1],clearcol[2],clearcol[3] );
    }
    
    
    /** 
     * Executed if the associated GLDrawable is being destroyed.
     * We don't need to do anythin but must include this method
     * to implement GLEventListener.
     */ 
    public void cleanup(GLDrawable drawable) {
    }
     /** This method is called before the display method.
    */
    public void preDisplay(GLDrawable drawable) {
    }

    /** This method is called after the display method.
    */
    public void postDisplay(GLDrawable drawable) {
    }
    
    
    /** 
     * Executed if the associated GLDrawable is resized
     */ 
    public void reshape(GLDrawable component, int x, int y, int width, int height){
        gl.glViewport(x, y, width, height);
        this.width = width;
        this.height = height;
        gl.glMatrixMode(gl.GL_PROJECTION);
        gl.glLoadIdentity();
        glu.gluPerspective(60, 1, 1, 10000);
        
        gl.glEnable(gl.GL_DEPTH_TEST);

    }
    
	public void displayChanged(GLDrawable drawable,
                           boolean modeChanged,
                           boolean deviceChanged) {
    	display(drawable);
    }
                           
    /**
     * Executed exactly once to initialize the
     * associated GLComponent
     */
    public void initialize( GLDrawable component ) {
        /**
         * Set the background colour when the GLComponent
         * is cleared
         */
        gl.glClearColor(clearcol[0],clearcol[1],clearcol[2],clearcol[3]);

    }
    
    
    /** This method handles the painting of the GLComponent */
    public void display(GLDrawable component) {

        // Use two-sided lighting
        gl.glLightModeli(gl.GL_LIGHT_MODEL_TWO_SIDE, 1);
        
        // Source code given starts with day-time. 
        gl.glClearColor(clearcol[0],clearcol[1],clearcol[2],clearcol[3]);
        
        // just use ambient light
        float[] ambientlight = {0.8f, 0.8f, 0.8f, 1.0f};
        gl.glLightModelfv(gl.GL_LIGHT_MODEL_AMBIENT, ambientlight);
        
        gl.glShadeModel(gl.GL_SMOOTH); // Use Gouraud shading. 
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT);
        gl.glEnable(gl.GL_COLOR_MATERIAL);

        // Clear the matrix stack. 
        gl.glMatrixMode(gl.GL_MODELVIEW);
        gl.glLoadIdentity();
        
        glu.gluLookAt(viewpos[0],viewpos[1], viewpos[2], 
                      viewpos[0]+viewdir[0],viewpos[1]+viewdir[1], viewpos[2]+viewdir[2], 
                      up[0], up[1], up[2]);
                      
        gl.glPushMatrix();
        
        // draw axis
        setColor(Color.white);
        gl.glLineWidth(2);
	    gl.glBegin(gl.GL_LINES);
        gl.glVertex3d(0, 0, 0);
	    gl.glVertex3d(0, FIELD_LENGTH, 0);
        gl.glVertex3d(0, 0, 0);
        gl.glVertex3d(FIELD_WIDTH, 0, 0);
        gl.glVertex3d(0, 0, 0);
        gl.glVertex3d(0, 0, HEADING_MAX);
	    gl.glEnd();
        
        gl.glPopMatrix();
                      
        gl.glPushMatrix();
        
        // draw beacons and goals
        if(teamColour == RED) {
            drawCylinder(Color.pink, Color.yellow, 0d, 0d);
            drawCylinder(Color.yellow, Color.pink, FIELD_WIDTH, 0d);
            drawCylinder(Color.pink, Color.green, 0d, FIELD_LENGTH/2);
            drawCylinder(Color.green, Color.pink, FIELD_WIDTH, FIELD_LENGTH/2);
            drawCylinder(Color.pink, Color.cyan, 0d, FIELD_LENGTH);
            drawCylinder(Color.cyan, Color.pink, FIELD_WIDTH, FIELD_LENGTH);
        } else { // BLUE
            drawCylinder(Color.cyan, Color.pink, 0d, 0d);
            drawCylinder(Color.pink, Color.cyan, FIELD_WIDTH, 0d);
            drawCylinder(Color.green, Color.pink, 0d, FIELD_LENGTH/2);
            drawCylinder(Color.pink, Color.green, FIELD_WIDTH, FIELD_LENGTH/2);
            drawCylinder(Color.yellow, Color.pink, 0d, FIELD_LENGTH);
            drawCylinder(Color.pink, Color.yellow, FIELD_WIDTH, FIELD_LENGTH);
        }
		
        gl.glPopMatrix();
		
		GLUT glut = new GLUT();	// GLUTFuncLightImpl(gl, glu);
		
		// draw old position
		gl.glPushMatrix();
		
		gl.glTranslated(oldx,oldy,oldh);
		setColor(oldCol);
		glut.glutSolidSphere(glu, CENTER_RAD, numSlices, numStacks);
		drawEllipsoid(oldvx, oldvy, oldvh, oldcxy, oldcxh, oldcyh);
		gl.glPopMatrix();
		
		// draw visual position
		gl.glPushMatrix();
		gl.glTranslated(visx,visy,vish);
		setColor(visCol);
		glut.glutSolidSphere(glu, CENTER_RAD, numSlices, numStacks);
		drawEllipsoid(visvx, visvy, visvh, viscxy, viscxh, viscyh);
		gl.glPopMatrix();
		
		// draw new position
		gl.glPushMatrix();
		gl.glTranslated(newx,newy,newh);
		setColor(newCol);
		glut.glutSolidSphere(glu, CENTER_RAD, numSlices, numStacks);
		drawEllipsoid(newvx, newvy, newvh, newcxy, newcxh, newcyh);
		gl.glPopMatrix();
    }
    
    
    void modelClicked(MouseEvent e) {
        repaint();
    }
    
    /** This sets the current GL drawing colour
	    @param c colour to set GL to use
    */
    public void setColor(Color c){
	    gl.glColor3ub((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue());
    }
    
    /** This sets the current GL drawing colour with the given alpha
	    @param c colour to set GL to use
        @param alpha the byte alpha value to use
    */
    public void setColor(Color c, byte alpha){
	    gl.glColor4b((byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue(),alpha);
    }
    
    /** This normalises the given vector (float[]) */
    private void normalise(double[] vec) {
        double lengthsq = 0f;
        for(int i=0 ; i<vec.length ; i++)
            lengthsq += vec[i]*vec[i];
        double length = Math.sqrt(lengthsq);
        for(int i=0 ; i<vec.length ; i++)
            vec[i] /= length;
    }
    
    /** This draws a cylinder given the parameters
      * @param col1 the first colour
      * @param col2 the second colour
      * @param x the x coordinate
      * @param y the y coordinate
      **/
    public void drawCylinder(Color col1, Color col2, double x, double y) {
        gl.glPushMatrix();
        gl.glTranslated(x,y,0d);
        GLUquadric qobj = glu.gluNewQuadric();
        glu.gluQuadricDrawStyle(qobj,glu.GLU_LINE);
        setColor(col1);
        glu.gluCylinder(qobj, cylRad, cylRad, cylHeight/2, 8, 8);
        gl.glTranslated(0d,0d,cylHeight/2);
        setColor(col2);
        glu.gluCylinder(qobj, cylRad, cylRad, cylHeight/2, 8, 8);
        gl.glPopMatrix();
    }
    
    
    /********************/
    /** For KeyListener */
    /********************/  
    public void keyTyped(KeyEvent evt){
        
    }
    
    public void keyReleased(KeyEvent evt){
        
    }


    public void keyPressed(KeyEvent evt){

        if(evt.getKeyCode() == KeyEvent.VK_UP){
            viewpos[0] += MOVE_FAC*up[0];
            viewpos[1] += MOVE_FAC*up[1];
            viewpos[2] += MOVE_FAC*up[2];
        } else if(evt.getKeyCode() == KeyEvent.VK_DOWN){
            viewpos[0] -= MOVE_FAC*up[0];
            viewpos[1] -= MOVE_FAC*up[1];
            viewpos[2] -= MOVE_FAC*up[2];
        } else if(evt.getKeyCode() == KeyEvent.VK_LEFT){
            viewpos[0] += MOVE_FAC*(up[1]*viewdir[2] - up[2]*viewdir[1]);
            viewpos[1] += MOVE_FAC*(up[2]*viewdir[0] - up[0]*viewdir[2]);
            viewpos[2] += MOVE_FAC*(up[0]*viewdir[1] - up[1]*viewdir[0]);
        } else if(evt.getKeyCode() == KeyEvent.VK_RIGHT){
            viewpos[0] -= MOVE_FAC*(up[1]*viewdir[2] - up[2]*viewdir[1]);
            viewpos[1] -= MOVE_FAC*(up[2]*viewdir[0] - up[0]*viewdir[2]);
            viewpos[2] -= MOVE_FAC*(up[0]*viewdir[1] - up[1]*viewdir[0]);
        } else if(evt.getKeyCode() == KeyEvent.VK_Q){
            viewpos[0] += MOVE_FAC*viewdir[0];
            viewpos[1] += MOVE_FAC*viewdir[1];
            viewpos[2] += MOVE_FAC*viewdir[2];
        } else if(evt.getKeyCode() == KeyEvent.VK_A){
            viewpos[0] -= MOVE_FAC*viewdir[0];
            viewpos[1] -= MOVE_FAC*viewdir[1];
            viewpos[2] -= MOVE_FAC*viewdir[2];
        }
        
        glc.repaint();
        
    }
    
    
    /**********************************************/
    /** For MouseListener and MouseMotionListener */
    /**********************************************/
    
    public void mouseMoved(MouseEvent evt){

    }
    
    public void mouseEntered(MouseEvent evt){
        
    }
    public void mouseExited(MouseEvent evt){
        
    }
    
    public void mouseClicked(MouseEvent evt){
        x = evt.getX();
        y = evt.getY();
    }
    public void mouseReleased(MouseEvent evt){

    }
    public void mousePressed(MouseEvent evt){
        x = evt.getX();
        y = evt.getY();
    }

    public void mouseDragged(MouseEvent evt){
        int dx = evt.getX() - x;
        int dy = evt.getY() - y;
		vertRot(dy*MOUSE_ANGLE_MOVE);
		horzRot(-dx*MOUSE_ANGLE_MOVE);
		
		x = evt.getX();
		y = evt.getY();
		
		glc.repaint();
    }
    
    
    /** Performs a verticle rotation on the viewing plane of the given amount
      * positive is down, negative up - this and below are a hack for matrix
	  * multiplication using rotation matrices*/
    private void vertRot(double aRad) {
		double c = Math.cos(aRad);
		double s = Math.sin(aRad);
		double t = 1-c;
		double x= up[1]*viewdir[2] - up[2]*viewdir[1];
		double y= up[2]*viewdir[0] - up[0]*viewdir[2];
		double z= up[0]*viewdir[1] - up[1]*viewdir[0];
		
		float[] olddir = new float[3];
		for(int i=0 ; i<olddir.length ; i++)
			olddir[i] = viewdir[i];
		
		viewdir[0] = (float)((t*x*x+c)*olddir[0] + (t*x*y-s*z)*olddir[1] + (t*x*z+s*y)*olddir[2]);
		viewdir[1] = (float)((t*x*y+s*z)*olddir[0] + (t*y*y+c)*olddir[1] + (t*y*z-s*x)*olddir[2]);
		viewdir[2] = (float)((t*x*z-s*y)*olddir[0] + (t*y*z+s*x)*olddir[1] + (t*z*z+c)*olddir[2]);
		
		float[] oldup = new float[3];
		for(int i=0 ; i<oldup.length ; i++)
			oldup[i] = up[i];
		
		up[0] = (float)((t*x*x+c)*oldup[0] + (t*x*y-s*z)*oldup[1] + (t*x*z+s*y)*oldup[2]);
		up[1] = (float)((t*x*y+s*z)*oldup[0] + (t*y*y+c)*oldup[1] + (t*y*z-s*x)*oldup[2]);
		up[2] = (float)((t*x*z-s*y)*oldup[0] + (t*y*z+s*x)*oldup[1] + (t*z*z+c)*oldup[2]);
		
		if(!chkNaturalRotation.isSelected()) {
			for(int i=0 ; i<up.length ; i++)
				universalUp[i] = up[i];
		}
		
    }
    
    
    /** Performs a horizontal rotation on the viewing plane of the given amount
      * positive is right, negative is left */
    private void horzRot(double aRad) {
		double c = Math.cos(aRad);
		double s = Math.sin(aRad);
		double t = 1-c;
		double x, y, z;
		if(chkNaturalRotation.isSelected()) {
			x=universalUp[0];
			y=universalUp[1];
			z=universalUp[2];
			
			float[] oldup = new float[3];
			for(int i=0 ; i<oldup.length ; i++)
			oldup[i] = up[i];
			
			up[0] = (float)((t*x*x+c)*oldup[0] + (t*x*y-s*z)*oldup[1] + (t*x*z+s*y)*oldup[2]);
			up[1] = (float)((t*x*y+s*z)*oldup[0] + (t*y*y+c)*oldup[1] + (t*y*z-s*x)*oldup[2]);
			up[2] = (float)((t*x*z-s*y)*oldup[0] + (t*y*z+s*x)*oldup[1] + (t*z*z+c)*oldup[2]);
		} else {
			x=up[0];
			y=up[1];
			z=up[2];
		}
			
		float[] olddir = new float[3];
		for(int i=0 ; i<olddir.length ; i++)
		olddir[i] = viewdir[i];
		
		viewdir[0] = (float)((t*x*x+c)*olddir[0] + (t*x*y-s*z)*olddir[1] + (t*x*z+s*y)*olddir[2]);
		viewdir[1] = (float)((t*x*y+s*z)*olddir[0] + (t*y*y+c)*olddir[1] + (t*y*z-s*x)*olddir[2]);
		viewdir[2] = (float)((t*x*z-s*y)*olddir[0] + (t*y*z+s*x)*olddir[1] + (t*z*z+c)*olddir[2]);
    }
	
	
	/** Draws a 95% confidence ellipsoid around (0,0,0) given x, y, h variances and covariances */
	private void drawEllipsoid(double vx, double vy, double vh, double cxy, double cxh, double cyh) {
	
		// get glut
		GLUT glut = new GLUT(); // GLUTFuncLightImpl(gl, glu);
	
		double[][] vals = {{vx,cxy,cxh},{cxy,vy,cyh},{cxh,cyh,vh}};	
		Matrix m = new Matrix(vals);
		EigenvalueDecomposition e = new EigenvalueDecomposition(m);
		double[] evals = e.getRealEigenvalues();
		Matrix evecs = e.getV();
		
		gl.glPushMatrix();
		
		double nyx = 0d, nyy = 1d, nyz = 0d;
		double ax=1, ay=0, az=0;
		double angRad1=0;
		if(evecs.get(1,0) != 0 || evecs.get(2,0) != 0) {
			double l =
		
			Math.sqrt(evecs.get(2,0)*evecs.get(2,0)+evecs.get(1,0)*evecs.get(1,0));
			// axis of rotation e1 x ev
			ax = 0;
			ay = -evecs.get(2,0)/l;
			az = evecs.get(1,0)/l;
			angRad1 = Math.acos(evecs.get(0,0));
			double cs = Math.cos(angRad1);
			double sn = Math.sin(angRad1);
			double t = 1-cs;
			nyx = (t*ax*ay-sn*az);
			nyy = (t*ay*ay+cs);
			nyz = (t*ay*az+sn*ax);
		}
		
		double angRad2 =
		Math.acos(nyx*evecs.get(0,1)+nyy*evecs.get(1,1)+nyz*evecs.get(2,1));
		// apply second rotation first (matrix operation order)
		gl.glRotated(angRad2*180d/Math.PI, evecs.get(0,0), evecs.get(1,0), evecs.get(2,0));
		gl.glRotated(angRad1*180d/Math.PI, ax, ay, az);
		double cf1 = 2*Math.sqrt(evals[0]);
		if(cf1 < 1d) cf1 = 1d;
		double cf2 = 2*Math.sqrt(evals[1]);
		if(cf2 < 1d) cf2 = 1d;
		double cf3 = 2*Math.sqrt(evals[2]);
		if(cf3 < 1d) cf3 = 1d;
		if(b>0) {
			System.out.println("cfs = {"+cf1+", "+cf2+", "+cf3+"}");
			b--;
		}
		gl.glScaled(cf1, cf2, cf3);
		double max = cf1 > cf2 ? (cf1 > cf3 ? cf1 : cf3) : (cf2 > cf3 ? cf2 : cf3);
		if(max < 50)
			wireFactor = 1;
		else
			wireFactor = 2;
		glut.glutWireSphere(glu,1,numSlices*wireFactor, numStacks*wireFactor);
		
		gl.glPopMatrix();
	
	}
	
	
	/** access method for setting data for old position */
	public void setOld(double x, double y, double h, double vx, double vy, double vh, double cxy, double cxh, double cyh) {
		if(!chkPause.isSelected()) {
			oldx = x;
			oldy = y;
			oldh = h;
			oldvx = vx;
			oldvy = vy;
			oldvh = vh;
			oldcxy = cxy;
			oldcxh = cxh;
			oldcyh = cyh;
			glc.repaint();
			
			double dx = oldx-newx;
			double dy = oldy-newy;
			if(Math.sqrt(dx*dx + dy*dy) > 50) {
				chkPause.doClick();
				System.out.println("Big jump");
				printOut();
			}
		}
	}
	
	/** access method for setting data for visual position */
	public void setVis(double x, double y, double h, double vx, double vy, double vh, 
						 double cxy, double cxh, double cyh) {
		if(!chkPause.isSelected()) {
			visx = x;
			visy = y;
			vish = h;
			visvx = vx;
			visvy = vy;
			visvh = vh;
			viscxy = cxy;
			viscxh = cxh;
			viscyh = cyh;
		}
	}
	
	/** access method for setting data for new position */
	public void setNew(double x, double y, double h, double vx, double vy, double vh, 
						 double cxy, double cxh, double cyh) {
		if(!chkPause.isSelected()) {
			newx = x;
			newy = y;
			newh = h;
			newvx = vx;
			newvy = vy;
			newvh = vh;
			newcxy = cxy;
			newcxh = cxh;
			newcyh = cyh;
		}
	}
	
	public void printOut() {
		System.out.println("Old -");
		System.out.println("x="+oldx+" y="+oldy+" h="+oldh);
		System.out.println("\t"+oldvx+"\t"+oldcxy+"\t"+oldcxh);
		System.out.println("\t"+oldcxy+"\t"+oldvy+"\t"+oldcyh);
		System.out.println("\t"+oldcxh+"\t"+oldcyh+"\t"+oldvh);
		System.out.println();
		
		System.out.println("Vision -");
		System.out.println("x="+visx+" y="+visy+" h="+vish);
		System.out.println("\t"+visvx+"\t"+viscxy+"\t"+viscxh);
		System.out.println("\t"+viscxy+"\t"+visvy+"\t"+viscyh);
		System.out.println("\t"+viscxh+"\t"+viscyh+"\t"+visvh);
		System.out.println();
		
		System.out.println("New -");
		System.out.println("x="+newx+" y="+newy+" h="+newh);
		System.out.println("\t"+newvx+"\t"+newcxy+"\t"+newcxh);
		System.out.println("\t"+newcxy+"\t"+newvy+"\t"+newcyh);
		System.out.println("\t"+newcxh+"\t"+newcyh+"\t"+newvh);
		System.out.println();
		
		b = 3;
		
	}
	
	public void setTeam(int team) {
		teamColour = team;
	}

    
    public static void main(String[] args) {
        GLDisplay3D g = new GLDisplay3D();
        g.show();
    }
}
