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
 * $Id: GLDisplay.java 2001 2003-09-24 12:02:35Z ttam186 $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
**/

import java.awt.*;
import javax.swing.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import net.java.games.jogl.*;
// import gl4java.drawable.*;
// // import gl4java.awt.*;


public class GLDisplay extends JFrame implements GLEventListener {

    BorderLayout mainLayout = new BorderLayout();
    JPanel mainDisplay = new JPanel();

     /** OpenGL and GLU pipelines used to render the scene */
    GL gl_ = null;
    //CoreGL coregl_ = new CoreGL();

    GLU glu_ = null;
    //CoreGLU coreglu_ = new CoreGLU();

    // GLContext glj = null;

    /** OpenGL rendering context */
    GLCanvas glc = null;


    /** the models */
    GLModel selfLocalisationModel = new GLModel();
    //GLModel visualMeasurementModel = new GLModel();
    //GLModel wirelessModel = new GLModel();    // not currently used
	GLDisplay3D display3d = null;
    GLField glField = new GLField();
	GLOpponentModel oppModel = new GLOpponentModel();


    int numOfDogs = 1;

    boolean cr = true;

    /** width and height of display area */
    protected static int WIDTH = 300, HEIGHT = 440;
    int width = WIDTH, height = HEIGHT;



    BorderLayout borderLayout1 = new BorderLayout();
    JPanel jPanel1 = new JPanel();
    FlowLayout flowLayout1 = new FlowLayout();
    JPanel jPanel2 = new JPanel();
    JToggleButton chkSelfLoc = new JToggleButton();
	JToggleButton chkWBall = new JToggleButton();
	JToggleButton chkVPull = new JToggleButton();
    JButton oppModelButton = new JButton();
	JButton oppOut = new JButton();
	JButton dirOut = new JButton();
    GridLayout gridLayout1 = new GridLayout();
    JToggleButton chk3D = new JToggleButton();

    public GLDisplay(int p) {
        try {
            jbInit(p);
            glInit();
            //TimedTimer t = new TimedTimer();
            //t.start();
        }
        catch(Exception e) {
            e.printStackTrace();
        }
    }

    /* executed once to initialise the associated GLDrawable */
    public void init(GLDrawable drawable) {
        gl_ = drawable.getGL();
    	System.out.println("GLDisplay.init()");
        glu_ = drawable.getGLU();
        // glj = drawable.getGLContext();
        /**
         * Set the background colour when the GLComponent
         * is cleared
         */
        gl_.glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ); //black
    }


    /**
     * initialises all the gl options
     */
    private void glInit() {
        // note - no balls at the moment
        //worldModel.setBallColor(Color.yellow);
        //wirelessModel.setBallColor(Color.magenta);

        /** Assign the OpenGL and GLU pipelines */
        // gl_ = coregl_;
        // glu_ = coreglu_;

        /**
         * Create a new GLComponent from the factory
         */
        GLCapabilities caps = new GLCapabilities();
        caps.setDoubleBuffered( true );
        //caps.setPixelType( GLCapabilities.RGBA );
        // caps.setTrueColor(true);

        //get a  GLCanvas from the factory
        glc = GLDrawableFactory.getFactory().createGLCanvas(caps /* , WIDTH, HEIGHT */ );
		glc.setSize(WIDTH, HEIGHT);

        /** Add the GLComponent to the Panel and display it */
        mainDisplay.add( "Center", glc );

        /**
         * Add the GLEventListener to handle GL events
         * and initialize!
         */
        glc.addGLEventListener( this );
        pack();
        show();
        initialize();
        //setSize(WIDTH + 20,HEIGHT+20);
    }

    // ??
    public void initialize(){
        //glc.initialize();
    }

    public void repaint() {
        if (glc == null) {
            System.out.println("glc is null");
        } else {
            glc.repaint();
        }
    }

    /**
     * Executed if the associated GLDrawable is being destroyed.
     * We don't need to do anythin but must include this method
     * to implement GLEventListener.
     */
    public void cleanup(GLDrawable drawable) {
    }

    /**
     * Executed if the associated GLDrawable is resized
     */
    public void reshape(GLDrawable gld, int width, int height) {
        gl_.glViewport( 0, 0, width, height );
        this.width = width;
        this.height = height;
        gl_.glMatrixMode( gl_.GL_PROJECTION );
        gl_.glLoadIdentity();
        glu_.gluOrtho2D( 0.0, width, 0.0, height);
    }

    /** This method is called before the display method.
    You probably don't need to use it.
    */
    public void preDisplay(GLDrawable drawable) {
    }

    /** This method is called after the display method.
    You probably don't need to use it.
    */
    public void postDisplay(GLDrawable drawable) {
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
        gl_.glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); //white

    }

    /** Handles resizing of the GLComponent */

    public void reshape( GLDrawable component, int x, int y,
                         int width, int height ) {
        gl_.glViewport( /*component,*/ x, y, width, height );
        this.width = width;
        this.height = height;
        gl_.glMatrixMode( gl_.GL_PROJECTION );
        gl_.glLoadIdentity();
        glu_.gluOrtho2D( 0.0, width, 0.0, height);
    }

	public void displayChanged(GLDrawable drawable,
                           boolean modeChanged,
                           boolean deviceChanged) {
    	display(drawable);
    }
                           
    /** This method handles the painting of the GLComponent */
    public void display( GLDrawable component ) {
        /** Clear the colour buffer */
        gl_.glClear( gl_.GL_COLOR_BUFFER_BIT );
        // load the matrix stack.
        // CAUTION: make sure later that you push off
        // all unneeded matrix
        gl_.glMatrixMode(gl_.GL_MODELVIEW);
        gl_.glLoadIdentity();


        // 10 to leave a bit of room at both ends
        double scaleFactor = height/((double) GLField.HEIGHT + 2 * GLGoal.HEIGHT + 10);
        gl_.glTranslated(GLBeacon.WIDTH*scaleFactor + 10, GLGoal.HEIGHT*scaleFactor + 5*scaleFactor,0);
        gl_.glScaled(scaleFactor,scaleFactor,1);

		// now invert the y axis - this was introduced in the port from GL4java to JOGL
		gl_.glScaled(1,-1,1);
		gl_.glTranslated(0,-GLField.HEIGHT,0);

        // enable polygon antialiasing
        //gl_.glBlendFunc( gl_.GL_SRC_ALPHA_SATURATE, gl_.GL_ONE );
        //gl_.glEnable(gl_.GL_SMOOTH);
        //gl_.glEnable(gl_.GL_POINT_SMOOTH);
        //gl_.glEnable(gl_.GL_LINE_SMOOTH);
        //gl_.glHint(gl_.GL_POINT,gl_.GL_NICEST);
        //gl_.glHint(gl_.GL_LINE,gl_.GL_NICEST);
        //gl_.glEnable( gl_.GL_BLEND );
        //gl_.glBlendFunc( gl_.GL_SRC_ALPHA, gl_.GL_ONE  );
        //gl_.glEnable(gl_.GL_POLYGON_SMOOTH);
        //gl_.glEnable( gl_.GL_BLEND );
        //gl_.glEnable( gl_.GL_POLYGON_SMOOTH );
        //gl_.glDisable( gl_.GL_DEPTH_TEST );

        glField.paint(gl_,glu_,glc);

        // wireless
        //if (chkWireless.isSelected())
        //    wirelessModel.paint(gl_,glu_,glc);

        // visual measurement model
        /*if (chkVisMeasure.isSelected())
            visualMeasurementModel.paint(gl_,glu_,glc);*/

		oppModel.paint(gl_,glu_,glc);

        // self localisation
        if (chkSelfLoc.isSelected())
            selfLocalisationModel.paint(gl_,glu_,glc);

    }

    /**
     * Returns a valid OpenGL pipeline when asked
     * by GLComponent
     */

    public GL getGL() {
        return gl_;
    }


    protected void createTestObjects() {
        // wireless
        //wirelessModel.createTestObjects();

        // visual measurement
        //visualMeasurementModel.createTestObjects();

        // self localisation
        //selfLocalisationModel.createTestObjects();

    }


    public void processData(int playerNum, double[] data, int start, int len) {
        int numModel = 3;
        setTitle("WorldModel " + playerNum);

        int base = start;
        int dataLen = 0;
        int team = (int) data[base];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+base+"] = "+team+" (team)");
        }
		if(display3d != null)
			display3d.setTeam(team);
        base+=1;

		int view = (int)data[base];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+base+"] = "+view+ " (view)");
        }
		selfLocalisationModel.setSeeBall((view & 1) != 0);
		glField.setView(view, team);
		base += 1;

        double directx = data[base++];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base-1)+"] = "+directx+ " (directx)");
        }
        double directy = data[base++];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base-1)+"] = "+directy+ " (directy)");
        }
        double directh = data[base++];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base-1)+"] = "+directh+ " (directh)");
        }
        selfLocalisationModel.setDirect(directx,directy,directh);

        // self localisation
        dataLen = (int) data[base++];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base-1)+"] = "+dataLen+ " (dataLen)");
        }
        selfLocalisationModel.processData(playerNum, team,data,base,dataLen);
		if(display3d != null) {
			display3d.setNew(data[base],data[base+1],data[base+2],data[base+3],
			   data[base+4],data[base+5],data[base+6],data[base+7], data[base+8]);
            if (RoboWirelessBase.worldModelDebug) {
                System.out.println("["+base+"] = "+data[base]+ " (display3d 1)");
                System.out.println("["+(base+1)+"] = "+data[base+1]+ " (display3d 2)");
                System.out.println("["+(base+2)+"] = "+data[base+2]+ " (display3d 3)");
                System.out.println("["+(base+3)+"] = "+data[base+3]+ " (display3d 4)");
                System.out.println("["+(base+4)+"] = "+data[base+4]+ " (display3d 5)");
                System.out.println("["+(base+5)+"] = "+data[base+5]+ " (display3d 6)");
                System.out.println("["+(base+6)+"] = "+data[base+6]+ " (display3d 7)");
                System.out.println("["+(base+7)+"] = "+data[base+7]+ " (display3d 8)");
                System.out.println("["+(base+8)+"] = "+data[base+8]+ " (display3d 9)");
            }
		}
        base += dataLen;

		// old world model
		dataLen = (int) data[base++];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base-1)+"] = "+dataLen+ " (old world model dataLen)");
        }
		if(display3d != null) {
			display3d.setOld(data[base],data[base+1],data[base+2],data[base+3],
			   data[base+4],data[base+5],data[base+6],data[base+7], data[base+8]);
            if (RoboWirelessBase.worldModelDebug) {
                System.out.println("["+base+"] = "+data[base]+ " (oldworldmodel 1)");
                System.out.println("["+(base+1)+"] = "+data[base+1]+ " (oldworldmodel 2)");
                System.out.println("["+(base+2)+"] = "+data[base+2]+ " (oldworldmodel 3)");
                System.out.println("["+(base+3)+"] = "+data[base+3]+ " (oldworldmodel 4)");
                System.out.println("["+(base+4)+"] = "+data[base+4]+ " (oldworldmodel 5)");
                System.out.println("["+(base+5)+"] = "+data[base+5]+ " (oldworldmodel 6)");
                System.out.println("["+(base+6)+"] = "+data[base+6]+ " (oldworldmodel 7)");
                System.out.println("["+(base+7)+"] = "+data[base+7]+ " (oldworldmodel 8)");
                System.out.println("["+(base+8)+"] = "+data[base+8]+ " (oldworldmodel 9)");
            }
		}
        base += dataLen;

		// opponents
		dataLen = (int)data[base++];
        if (RoboWirelessBase.worldModelDebug) {
            System.out.println("["+(base-1)+"] = "+dataLen+ " (opponents dataLen)");
        }
		oppModel.processData(team,data,base,dataLen);
		base += dataLen;

        repaint();
    }

	public void loadFile(String filename) {

		try {
			//DataInputStream dis = new DataInputStream(new FileInputStream(filename));
			BufferedReader in = new BufferedReader(new FileReader(filename));
			String str = in.readLine();

			StringTokenizer st = new StringTokenizer(str);
			int size = Integer.parseInt(st.nextToken());
			int playerNum = Integer.parseInt(st.nextToken());

			double arr[] = new double[size-2];
			for(int i=0 ; i<size-2 ; i++) {
				arr[i] = Double.parseDouble(st.nextToken());
			}

			processData(playerNum, arr, 0, size-2);

		} catch(Exception e) {
			System.out.println("loading of gps file failed("+filename+")!");
			System.out.println(e.getMessage());
			e.printStackTrace();
		}

	}


    class TimedTimer extends Thread {
        long time = System.currentTimeMillis(), counter = 0, timesum = 0;
        boolean start = true;

        public void run() {
            while (true) {
                if (counter == 0 && !start) {
                    System.out.println("Frame rate: " + 100.0/(timesum)*1000 + "fps, repaint time average = " + timesum/100.0);
                    timesum = 0;
                }

                time = System.currentTimeMillis();
                repaint();
                long curTime = System.currentTimeMillis();
                timesum += curTime - time;
                try {
                    sleep((long) (1.0/30*1000));
                } catch (InterruptedException e) {}
                counter++;
                counter %= 100;
                start = false;
            }
        }
    }







    private void jbInit(int p) throws Exception {
        this.getContentPane().setLayout(mainLayout);
        this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

        this.addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                repaint();
                System.out.println("GLDisplay: Window is not allowed to be closed");
            }
        });
        mainDisplay.setLayout(borderLayout1);
        jPanel1.setPreferredSize(new Dimension(100, 100));
        jPanel1.setLayout(flowLayout1);
        chkSelfLoc.setSelected(true);
        chkSelfLoc.setText("Localisation");
        chkSelfLoc.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                modelClicked(e);
            }
        });

		chkWBall.setText("WBall");
		chkWBall.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                selfLocalisationModel.switchWBall();
				repaint();
            }
        });

		chkVPull.setText("VPull");
		chkVPull.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                selfLocalisationModel.switchVPull();
				repaint();
            }
        });

        oppModelButton.setText("Opponents(off)");
        oppModelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				String s = oppModel.changeMode();
				oppModelButton.setText("Opponents("+s+")");
				repaint();
			}
		});
		oppOut.setText("Output Opps");
		oppOut.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				oppModel.dumpData();
			}
		});
        dirOut.setText("DirectGPS Out");
		dirOut.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				selfLocalisationModel.printDirect();
			}
		});
        jPanel2.setLayout(gridLayout1);
        gridLayout1.setRows(7);
        gridLayout1.setColumns(1);
        chk3D.setText("3D model");
        chk3D.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
				display3Dclicked();
            }
        });
        this.getContentPane().add(mainDisplay, BorderLayout.CENTER);
        mainDisplay.add(jPanel1, BorderLayout.EAST);
        jPanel1.add(jPanel2, null);
        jPanel2.add(chkSelfLoc, null);
		jPanel2.add(chkWBall, null);
		jPanel2.add(chkVPull, null);
        jPanel2.add(oppModelButton, null);
		jPanel2.add(oppOut,null);
		jPanel2.add(dirOut,null);
        jPanel2.add(chk3D, null);


        setLocation(300*p,300);
    }

    void modelClicked(MouseEvent e) {
        repaint();
    }

	void display3Dclicked() {
		if(display3d == null)
			display3d = new GLDisplay3D();
		else if(chk3D.isSelected())
			display3d.show();
		else
			display3d.hide();
		repaint();
	}
    public static void main(String [] args){
        GLDisplay d = new GLDisplay(1);
    }
}


