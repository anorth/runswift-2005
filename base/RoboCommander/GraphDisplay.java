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
 * $Id: GraphDisplay.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * reads joystick data from a socket, and sends them via RoboCommander
 *
**/

import java.awt.*;
import javax.swing.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import net.java.games.jogl.*;
// // import gl4java.drawable.*;
// // import gl4java.awt.*;

public class GraphDisplay extends JFrame implements GLEventListener {
    public static final int WIDTH = 550, HEIGHT = 450;
    public static final float LINE_WIDTH = 1f;

    BorderLayout mainLayout = new BorderLayout();
    JPanel mainDisplay = new JPanel();
    BorderLayout borderLayout1 = new BorderLayout();
    JPanel header = new JPanel();
    FlowLayout flowLayout1 = new FlowLayout();
    JLabel heading  = new JLabel();
    JPanel jPanel1 = new JPanel();
    JPanel jPanel2 = new JPanel();
    GridLayout gridLayout1 = new GridLayout();

    JPanel jPanel0 = new JPanel();
    FlowLayout flowLayout0 = new FlowLayout();
    BorderLayout borderLayout2 = new BorderLayout();
    JLabel jLabel2 = new JLabel();
    JTextField fName = new JTextField();
    JButton saveButton = new JButton(); 
    JButton clearButton = new JButton(); 
    JPanel jPanel3 = new JPanel();
    BorderLayout borderLayout3 = new BorderLayout();
    JLabel jLabel3 = new JLabel();
    JButton recB = new JButton();
    JButton stopB = new JButton();
    JLabel tLabel = new JLabel();
    JPanel jPanel4 =  new JPanel();
    JPanel jPanel5 =  new JPanel();
    JPanel jPanel6 = new JPanel();
    
     /** OpenGL and GLU pipelines used to render the scene */
    GL gl_ = null;
    //CoreGL coregl_ = new CoreGL();

    GLU glu_ = null;
    //CoreGLU coreglu_ = new CoreGLU();
    
    // GLContext glj = null;

    /** OpenGL rendering context */
    //GLComponent glc = null;
    GLCanvas glc = null;

    /** the models */
    GLGraphLine glLines=new GLGraphLine();
    GLGrapher glGrapher = new GLGrapher(glLines);
    JToggleButton[] chkLine = new JToggleButton[GLGraphLine.NUM_GRAPH_LINES];


    double width=WIDTH, height=HEIGHT;
    RoboCommander rc;
    public GraphDisplay(String name, RoboCommander rc) {
        try {        
	    this.rc=rc;
	    setVisible(false);
            jbInit(name);
            glInit();            
        }
        catch(Exception e) {
            e.printStackTrace();
        }
    }
    
    /* executed once to initialise the associated GLDrawable */
    public void init(GLDrawable drawable) {
    	System.out.println("GraphDisplay.init()");
    	gl_ = drawable.getGL();
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
        glc = GLDrawableFactory.getFactory().createGLCanvas(caps);
		glc.setSize(WIDTH, HEIGHT);
	
        /** Add the GLComponent to the Panel and display it */
        mainDisplay.add( "Center", glc );

        /**
         * Add the GLEventListener to handle GL events
         * and initialize!
         */
        glc.addGLEventListener( this );
        pack();
    }

	// ?? - no idea - Ross
    public void initialize(){
        // glc.initialize();
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
        
        gl_.glTranslated(width/2.0,height/2.0,0);
        gl_.glScaled(width/(float)WIDTH,width/(float)WIDTH,1);
        gl_.glTranslated(-WIDTH/2.0,-HEIGHT/2.0,0);

        glGrapher.paint(gl_,glu_,glc);
    }

    /**
     * Returns a valid OpenGL pipeline when asked
     * by GLComponent
     */

    public GL getGL() {
        return gl_;
    }

    public void processData() {
	if (!isVisible()) {
	    show();
	    initialize();
	    setVisible(true);
	}
        repaint();
    }
    
    public void processData(double[] data, int start, int len) {
        int base = start;
        int dataLen = 0;
	if (!isVisible()) {
	    show();
	    initialize();
	    setVisible(true);
	}
        glLines.processData(data,base,len);
        base += dataLen;
        repaint();        
    }


    public void reset() {
	glLines.reset();
    }

    private void jbInit(String name) throws Exception {
        this.getContentPane().setLayout(mainLayout);
        this.addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                System.out.println("Window is not allowed to be closed");
            }
        });
        this.getContentPane().add(mainDisplay, BorderLayout.CENTER);
	this.getContentPane().add(heading, BorderLayout.NORTH);
	for (int i=0;i<chkLine.length;i++) {
	    chkLine[i] = new JToggleButton();
	    chkLine[i].setSelected(false);
	    chkLine[i].setFont(new Font("Helvetica",1,9));
	    chkLine[i].setForeground(Color.white);
	    chkLine[i].setText(".");
	    chkLine[i].setBackground(glLines.lineColours[i]);
	    chkLine[i].addMouseListener(new java.awt.event.MouseAdapter() {
		    public void mouseClicked(MouseEvent e) {
			graphClicked(e);
		    }
	    });
	}
	heading.setBackground(Color.lightGray);
        heading.setFont(new java.awt.Font("Dialog", 1, 28));
        heading.setHorizontalAlignment(SwingConstants.CENTER);
        heading.setText(name);
	jPanel1.setPreferredSize(new Dimension(130, 50));
	jPanel1.setLayout(flowLayout1);
        jPanel1.add(jPanel2, null);
        gridLayout1.setColumns(4);
	gridLayout1.setRows(2);
        jPanel2.setLayout(gridLayout1);
	for (int i=0;i<chkLine.length;i++) {
	    jPanel2.add(chkLine[i],null);
	}
	// Start Stop Controls
        jPanel3.setLayout(borderLayout3);
        jLabel3.setText("Controls");
        recB.setText("Record");
        recB.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                recClicked(e);
            }
        });
	recB.setForeground(Color.red);
        stopB.setText("Stop");
        stopB.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                stopClicked(e);
            }
        });
	stopB.setForeground(Color.darkGray);
	jPanel0.setLayout(new GridLayout(2,2));
	jPanel0.add(jLabel2);
	jPanel0.add(Box.createHorizontalGlue());
	jPanel0.add(fName);
	jPanel0.add(saveButton);

	// Target Information
	tLabel.setText("No Target");
	// Graph Controls
	clearButton.setText("Reset");
        clearButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                clearClicked(e);
            }
        });
	jPanel6.setLayout(new GridLayout(2,1));
	jPanel6.add(tLabel);
	jPanel6.add(clearButton);

	// Filename controls
        jLabel2.setText("Filename");
        fName.setMinimumSize(new Dimension(100, 21));
        fName.setPreferredSize(new Dimension(100, 21));
        fName.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusGained(FocusEvent e) {
                    fName_focusGained(e);
                }
            });
        fName.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    fName_actionPerformed(e);
                }
            });
        saveButton.setText("Save");
        saveButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                saveClicked(e);
            }
        });
	jPanel3.setLayout(new GridLayout(2,1));
	//	jPanel3.add(jLabel3);
	//	jPanel3.add(Box.createHorizontalGlue());
	jPanel3.add(recB);
	jPanel3.add(stopB);

	jPanel4.setLayout(new GridLayout(1,3,25,1));
	jPanel4.add(jPanel0);
	jPanel4.add(jPanel6);
	jPanel4.add(jPanel3);
	jPanel5.setLayout(new BorderLayout());
	jPanel5.add(jPanel1, BorderLayout.NORTH);
	jPanel5.add(jPanel4, BorderLayout.SOUTH);
        mainDisplay.setLayout(borderLayout1);
        mainDisplay.add(jPanel5, BorderLayout.SOUTH);
        setLocation(620,390);
    }

    void setLabels(String[] lines, int size) {
	if (size > chkLine.length) {
	  for (int i=0;i<chkLine.length;i++) {
	    if (chkLine[i].equals("."))
	      chkLine[i].setEnabled(false);
	    else 
	      chkLine[i].setEnabled(true);
		
	    chkLine[i].setText(lines[i]);
	  }
	}
    }

    void modelClicked(MouseEvent e) {
        repaint();
    }

    void graphClicked(MouseEvent e) {
	for (int i=0;i<chkLine.length;i++) {
	    if (!chkLine[i].isSelected())
		glLines.drawLine[i] = true;
	    else
		glLines.drawLine[i] = false;
	}
	repaint();
    }

    void fName_focusGained(FocusEvent e) {
        fName.selectAll();
    }

    void fName_actionPerformed(ActionEvent e) {
	//        savefile();
    }

    void saveClicked(MouseEvent e) {
	if (!fName.getText().equals(""))
	    savefile(fName.getText());
    }
    void clearClicked(MouseEvent e) {
	fName.setText("");
	rc.send("reset");
	reset();
    }
    
    void stopClicked(MouseEvent e) {
	rc.send("off");
    }

    void recClicked(MouseEvent e) {
	rc.send("on");
    }

    void savefile(String filename) {
	rc.send("save", filename);
	reset();
    }

    void processTarget(int[] data, int size) {
	if (data[size-1]!=0)
	    tLabel.setText("z: " + data[size-1]);
	else
	    tLabel.setText("No Target");
    }

}


