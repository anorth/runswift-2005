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
 * UNSW 2003 Robocup (Ricky)
 *
 * Last modification background information
 * $Id: OffVision.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * displays simulated cplane of CPLANE_LOG image 
 * also allows manual blob validity classification
 *
**/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.SwingUtilities;
import java.util.StringTokenizer;
import java.io.*;

public class OffVision extends JFrame {

	// public GLDisplay gps;

    private static final boolean prefixDebugMsg      = false;
    private static final boolean currentFileDebugMsg = false;
    private static final boolean loadGPS             = false;

    protected final  int NUM_COLORS         = 16;
    protected        int currentColor       = NUM_COLORS;
    protected		 int maybeColors        = 0;	// 0 = both, 1 = normal, 2 = maybe
    protected final	int MAYBE_BIT			= 0x10;

    public static final char DOT = '.';

    private OffVisionPanel ovp;
    private JFileChooser fileChooser = new JFileChooser();
	
	String gpsPostfix = "gps";
    
    boolean displayBlob;
    String imageFilePrefix, imageFileExt, objectFilePrefix, objectFileExt, blobFilePrefix, blobFileExt;
    int fileNumber;
    
    // GUI for the control panel
    JPanel controlPanel               = new JPanel();
    ButtonGroup colorRadioButtonGroup = new ButtonGroup();
	ButtonGroup maybeRadioButtonGroup = new ButtonGroup();

    Dimension panelDimenBig   = new Dimension(842, 650);
    Dimension panelDimenSmall = new Dimension(634, 490);
		

    // GUI for the play back panel
    TitledBorder blobberBorder      = new TitledBorder("Blobber Panel");
    JPanel blobberPanel             = new JPanel();
    
    JPanel blobStatePanel           = new JPanel();
    String[] radioButtonText        = {"Valid", "Invalid"};
    JRadioButton[] validRadioButton = new JRadioButton[2];
    ButtonGroup buttonGroup         = new ButtonGroup();
    JButton prevBlobButton          = new JButton("Prev");
    JButton nextBlobButton          = new JButton("Next");
    JTextField currentBlob          = new JTextField(6);
    JLabel totalBlobs               = new JLabel();
    JButton writeObjectDataButton   = new JButton("Write c4.5 Data");

    JPanel shortCutPanel            = new JPanel();
    JCheckBox changeStateCheckBox   = new JCheckBox("Change State?", true);
    JCheckBox writeLogCheckBox      = new JCheckBox("Write Log?", true);
    JCheckBox nextFrameCheckBox     = new JCheckBox("Next Frame?", true);

    TitledBorder playBackBorder     = new TitledBorder("Navigate Panel");
    JPanel playBackPanel            = new JPanel();
    JButton backwardButton          = new JButton("<");
    JButton forwardButton           = new JButton(">");
    JTextField currentFrame         = new JTextField(4);
    
    public OffVision(String imageFilePrefix, int fileNumber, String imageFileExt, boolean displayBlob) {
        this.imageFilePrefix  = imageFilePrefix;
        this.fileNumber       = fileNumber;
        this.imageFileExt     = imageFileExt;
        this.objectFilePrefix = imageFilePrefix;
        this.objectFileExt    = imageFileExt + OffVisionConstant.OBJECT_EXTENSION;
        this.blobFilePrefix   = imageFilePrefix;
        this.blobFileExt      = imageFileExt + OffVisionConstant.BLOB_EXTENSION;
        this.displayBlob      = displayBlob;

        // setting the mcp
        ovp = new OffVisionPanel(this, displayBlob);
        ovp.setBorder(BorderFactory.createEtchedBorder());

        ovp.setPreferredSize(panelDimenBig);
	//ovp.setPreferredSize(panelDimenSmall);

        
//        if (loadGPS) { 
//		    gps = new GLDisplay(1);
//        }

        //ovp.readCalFile("nnmc.cal");
        //ovp.readBflFile(imgfile);
        if (!paintFile()) {
            System.out.println("Program exited");
            System.exit(1);
        }
        initPlayBackPanel();
        paintScreen();
    }
    
    public OffVision(String imageFilePrefix, int startIndex, String imageFileExt, int endIndex) {
        JPanel autoPanel   = new JPanel();
        JLabel statusLabel = new JLabel("Processing...");
        JLabel fileLabel   = new JLabel();
        String blobFile, prefix, ext;

        prefix = OffVisionConstant.BLOB_DIRECTORY + File.separator + imageFilePrefix;
        ext    = imageFileExt + OffVisionConstant.BLOB_EXTENSION;
        
        ovp = new OffVisionPanel(this, true);

//        if (loadGPS) { 
//		    gps = new GLDisplay(1);
//        }
        
        autoPanel = new JPanel();
        autoPanel.add(statusLabel);
        autoPanel.add(fileLabel);

        getContentPane().add(autoPanel);
        // process the imagefile from startIndex to endIndex
        for (int i=startIndex; i<=endIndex; i++) {
            blobFile = prefix + DOT + i + DOT + ext;
            if (currentFileDebugMsg) {
                System.out.println("OffVision.java: " + blobFile);
            }
            if (!ovp.readBlobFile(blobFile)) {
                break;
            }
            fileLabel.setText(""+i);
            ovp.writeLog();
        }
        statusLabel.setText("Finish");
        fileLabel.setText("");
        JOptionPane.showMessageDialog(this, "Finish Auto Learning", "Information", JOptionPane.INFORMATION_MESSAGE);
    }
    
    void initPlayBackPanel() {
        // GUI for the play panel
        if (displayBlob) {
            for (int i=0; i<validRadioButton.length; i++) {
                validRadioButton[i] = new JRadioButton(radioButtonText[i], i==1);
                validRadioButton[i].setActionCommand(""+i);
                validRadioButton[i].addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        int choice = Integer.parseInt(buttonGroup.getSelection().getActionCommand());
                        validRadioButton[choice].setSelected(true);
                        ovp.changeBlobValidState();
                        ovp.repaint();
                    }
                });
                buttonGroup.add(validRadioButton[i]);
            }
            
            prevBlobButton.setToolTipText("Display previous Blob");
            prevBlobButton.setEnabled(false);
            prevBlobButton.setMnemonic(KeyEvent.VK_P);
            prevBlobButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    ovp.setPrevBlob();
                    ovp.repaint();
                }
            });

            nextBlobButton.setToolTipText("Display next Blob");
            nextBlobButton.setMnemonic(KeyEvent.VK_N);
            nextBlobButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    ovp.setNextBlob();
                    ovp.repaint();
                }
            });
            currentBlob.setEditable(false);
            currentBlob.setHorizontalAlignment(JTextField.RIGHT);
            
            writeObjectDataButton.setToolTipText("Write the c4.5 data and go to next frame automatically.");
            writeObjectDataButton.setMnemonic(KeyEvent.VK_W);
            writeObjectDataButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    ovp.writeLog();
                    writeObjectDataButton.setEnabled(false);
                    
                    // go to the next frame automatically after writing
                    doNextFrame();
                }
            });
            
            for (int i=0; i<validRadioButton.length; i++) {
                blobStatePanel.add(validRadioButton[i]);
            }
            blobStatePanel.add(new JPanel());
            blobStatePanel.add(prevBlobButton);
            blobStatePanel.add(nextBlobButton);
            blobStatePanel.add(currentBlob);
            blobStatePanel.add(totalBlobs);
            blobStatePanel.add(new JPanel());
            blobStatePanel.add(writeObjectDataButton);
            
            shortCutPanel.add(changeStateCheckBox);
            shortCutPanel.add(writeLogCheckBox);
            shortCutPanel.add(nextFrameCheckBox);
            
            blobberPanel.setBorder(blobberBorder);
            blobberPanel.setLayout(new BorderLayout());
            blobberPanel.add(blobStatePanel, BorderLayout.CENTER);
            blobberPanel.add(shortCutPanel, BorderLayout.SOUTH);
        }

        backwardButton.setToolTipText("Go to the previous frame");
        backwardButton.setEnabled(false);
        backwardButton.setMnemonic(KeyEvent.VK_LEFT);
        backwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                doPreviousFrame();
            }
        });

        forwardButton.setToolTipText("Go to the next frame");
        forwardButton.setMnemonic(KeyEvent.VK_RIGHT);
        forwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                doNextFrame();
            }
        });
        
        currentFrame.setHorizontalAlignment(JTextField.RIGHT);
        currentFrame.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    if (displayBlob) {
                        // set invalid at every new frame
                        setValid(false);
                    }
                    writeObjectDataButton.setEnabled(true);
                    fileNumber = Integer.parseInt(currentFrame.getText());
                    ovp.resetBlobIndex();
                    if (!paintFile()) {
                        fileNumber = 1;
                        currentFrame.setText(""+fileNumber);
                    }
                } catch (NumberFormatException e2) {
                    JOptionPane.showMessageDialog(null, "Error: Input must be an integer", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        
        
        playBackPanel.setBorder(playBackBorder);
        playBackPanel.add(backwardButton);
        playBackPanel.add(forwardButton);
        playBackPanel.add(currentFrame);


        /* Add colour selection panel. */
        controlPanel.setLayout(new GridLayout(3, 8));
        
        /* Add all the buttons for selecting color. */
        for (int i = 0 ; i <= NUM_COLORS; i++) {
            String colorLabels = i + "";
            JRadioButton rb;

            /* Set default to be showing All colour. */
            if (i == NUM_COLORS) {
                rb = new JRadioButton("All");
                rb.setSelected(true);
            } else {
                rb = new JRadioButton(colorLabels);
            }
            controlPanel.add(rb);
            colorRadioButtonGroup.add(rb);
            rb.setActionCommand(colorLabels);

            rb.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    currentColor = Integer.parseInt(e.getActionCommand());
                    //System.out.println("rb.actionPerformed| currentColor: " +
                    //                                   currentColor);
                    ovp.repaint();
                }
            });
        }
        {
        	JRadioButton rb = new JRadioButton("Full Colour");
        	controlPanel.add(rb);
        	maybeRadioButtonGroup.add(rb);
            rb.setActionCommand("1");
            
            rb.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    maybeColors = Integer.parseInt(e.getActionCommand());
                   ovp.repaint();
                }
            });
        }
        {
        	JRadioButton rb = new JRadioButton("Maybe Colour");
        	controlPanel.add(rb);
        	maybeRadioButtonGroup.add(rb);
            rb.setActionCommand("2");
            
            rb.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    maybeColors = Integer.parseInt(e.getActionCommand());
                   ovp.repaint();
                }
            });
        }
        {
        	JRadioButton rb = new JRadioButton("Both");
        	controlPanel.add(rb);
        	maybeRadioButtonGroup.add(rb);
        	rb.setSelected(true);
            rb.setActionCommand("0");
            
            rb.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    maybeColors = Integer.parseInt(e.getActionCommand());
                   ovp.repaint();
                }
            });
        }

        setSize(new Dimension(800, 600));
        setTitle("OffVision Debug");
        getContentPane().setLayout(new BorderLayout());
        if (displayBlob) {
            getContentPane().add(ovp, BorderLayout.NORTH);
            getContentPane().add(blobberPanel, BorderLayout.CENTER);
            getContentPane().add(playBackPanel, BorderLayout.SOUTH);

        /* If not displaying blobberPanel, display the controlPanel. */
        } else {
            getContentPane().add(ovp, BorderLayout.NORTH);
            getContentPane().add(controlPanel, BorderLayout.CENTER);
            getContentPane().add(playBackPanel, BorderLayout.SOUTH);
        }
    }
    
    // show the previous frame
    void doPreviousFrame() {
        if (displayBlob) {
            // set invalid at every new frame
            setValid(false);
        }
        writeObjectDataButton.setEnabled(true);
        fileNumber--;
        ovp.resetBlobIndex();
        if (!paintFile()) {
            fileNumber++;
            currentFrame.setText(""+fileNumber);
        }
    }
    
    // show the next frame
    void doNextFrame() {
        if (displayBlob) {
            // set invalid at every new frame
            setValid(false);
        }
        writeObjectDataButton.setEnabled(true);
        fileNumber++;
        ovp.resetBlobIndex();
        if (!paintFile()) {
            fileNumber--;
            currentFrame.setText(""+fileNumber);
        }
    }
    
    boolean paintFile() {
        String cplaneFile;
		String gpsFile;
	    if(imageFileExt.equals("")) {
            cplaneFile = OffVisionConstant.FRAME_DIRECTORY + File.separator + imageFilePrefix + DOT + fileNumber;
	    } else {
            cplaneFile = OffVisionConstant.FRAME_DIRECTORY + File.separator + imageFilePrefix + DOT + fileNumber + DOT + imageFileExt;
        }
		
		gpsFile = "gps" + File.separator + imageFilePrefix + DOT + fileNumber + DOT + gpsPostfix;

//        if (loadGPS) { 
//            gps.loadFile(gpsFile);
//        }
		
	    // if the reading of the files are unsuccessful
        if (!ovp.readCplFile(cplaneFile)) {
            System.out.println("OffVision.java: Cannot read " + cplaneFile);
            return false;
        } else if (!displayBlob) {
            String objectFile = OffVisionConstant.OBJECT_DIRECTORY + File.separator + objectFilePrefix + DOT + fileNumber + DOT + objectFileExt;
            if (!ovp.readObjFile(objectFile)) {
                System.out.println("OffVision.java: Cannot read " + objectFile);
                return false;
            }
        } else { // read blob file
            String blobFile = OffVisionConstant.BLOB_DIRECTORY + File.separator + blobFilePrefix + DOT + fileNumber + DOT + blobFileExt;
            if (!ovp.readBlobFile(blobFile)) {
                System.out.println("OffVision.java: Cannot read " + blobFile);
                return false;
            }
        }
        paintScreen();
        return true;
    }

    void paintScreen() {
        currentFrame.setText("" + fileNumber);
        if (fileNumber==1) {
            backwardButton.setEnabled(false);
        } else {
            backwardButton.setEnabled(true);
        }
        ovp.repaint();
    }
    
    public void setPrevBlobButtonEnabled(boolean b) {
        prevBlobButton.setEnabled(b);
    }
    public void setNextBlobButtonEnabled(boolean b) {
        nextBlobButton.setEnabled(b);
    }
    public JTextField getCurrentBlobField() {
        return currentBlob;
    }
    public void setTotalBlobs(int n) {
        totalBlobs.setText(" / "+n);
    }
    public void noSelectedBlob() {
        currentBlob.setText("N/A");
        prevBlobButton.setEnabled(false);
        nextBlobButton.setEnabled(false);
        for (int i=0; i<validRadioButton.length; i++) {
            validRadioButton[i].setEnabled(false);
        }
    }
    public void setRadioButtonsEnabled() {
        for (int i=0; i<validRadioButton.length; i++) {
            validRadioButton[i].setEnabled(true);
        }
    }
    public void setValid(boolean valid) {
        if (valid) {
            validRadioButton[0].setSelected(true);
        } else {
            validRadioButton[1].setSelected(true);
        }
    }
    public boolean isChangeStateSelected() {
        return changeStateCheckBox.isSelected();
    }
    public boolean isWriteLogSelected() {
        return writeLogCheckBox.isSelected();
    }
    public boolean isNextFrameSelected() {
        return nextFrameCheckBox.isSelected();
    }

    public void openFile(boolean cal) {
        int returnVal = fileChooser.showOpenDialog(this);
        boolean res = false;
        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }
        String filename = fileChooser.getSelectedFile().getPath();
        if (cal) {
            res = ovp.readCalFile(filename);
        } else {
            res = ovp.readBflFile(filename);
        }
        if (!res) {
            JOptionPane.showMessageDialog(this, "Error opening file", "Error", JOptionPane.ERROR_MESSAGE);
        } else {
            JOptionPane.showMessageDialog(this, "File opened", "Error", JOptionPane.ERROR_MESSAGE);
        }
        ovp.repaint();
    }
    
    
    // close window
    protected void processWindowEvent(WindowEvent e) {
        super.processWindowEvent(e);
        if (e.getID() == WindowEvent.WINDOW_CLOSING) {
            System.exit(0);
        }
    }

    public static void main(String[] args) {
        OffVision ov = null;
        final String BLOB_INPUT = "-blob";
        if (args.length>=1 && args.length<=3) {
            try {
                StringTokenizer st;
                if (args.length>=2) {
                    st = new StringTokenizer(args[1], ".");
                } else {
                    st = new StringTokenizer(args[0], ".");
                }
                String imageFilePrefix = st.nextToken();
                int fileNumber         = Integer.parseInt(st.nextToken());
                String imageFileExt    = (st.hasMoreTokens()) ? st.nextToken() : "";
                if (args.length==3 && args[0].equals(BLOB_INPUT)) {
                    ov = new OffVision(imageFilePrefix, fileNumber, imageFileExt, Integer.parseInt(args[2]));
                } else if (args.length==2) {
                    ov = new OffVision(imageFilePrefix, fileNumber, imageFileExt, args[0].equals(BLOB_INPUT));
                } else {
                    ov = new OffVision(imageFilePrefix, fileNumber, imageFileExt, false);
                }
            } catch (Exception e) {
                System.out.print("OffVision.java: Image and object file should be in the format CONST_PREFIX.NUMBER[.CONST_EXTENSION] ");
                System.out.println("where NUMBER is an integer in the series, and PREFIX and EXTENSION are string.");
                e.printStackTrace();
				System.exit(1);
            }
        } else {
            System.out.println("usage: java OffVision [-blob] <imgfile>");
            System.exit(1);
        }
        
        //Center the window
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension frameSize = ov.getSize();
        if (frameSize.height > screenSize.height) {
            frameSize.height = screenSize.height;
        }
        if (frameSize.width > screenSize.width) {
            frameSize.width = screenSize.width;
        }
        ov.pack();
        ov.setLocation((screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2);
        ov.setVisible(true);        
    }
}
