/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
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
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


package RC;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.5
 * CHANGE LOG:
 * + version 1.1:
 *       - Implement Observer interface (in Observer pattern)
 * + version 1.2:
 *       - Added Batch saving (save change in CPlane in all opened BFLs)
*  + version 1.3:
*        - Show Background colour , to help extending colours space,
* and avoid "competing" between releated colours. (say red/orange If a orange
* point is currently background, then we should classify it as orange, but if
* it is red, then might consider leave it the same, to avoid "racing" between orange/red)
*  + version 1.5:
*       - Option to classify "black" pixel only. This is called "Safe classification",
*        to avoid racing between similar colours
*  + version 1.6:
*       - disable enabled flag in paintBFLPanel, show classified as black when not showing background
*  + version 1.7:
*       - use BACKGROUND to unclassify colour (instead of NOCOLOUR).
 */


import java.io.*;
import java.util.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import RoboComm.*;
import RoboShare.*;


public class ToolColourRDR
    extends GenericTool
    implements Painter, Observer, Subject{
    static private final boolean debug = false;
    private static final boolean debugPainting = false;
    private static final long NO_TOUCH_TIME = 2 * 1000; //millisecond

    JButton btOpenBFL = new JButton();
    JButton btLoadColourRDR = new JButton();
    JButton saveBFLButton = new JButton();
    JButton saveAllBFLButton = new JButton();
    JButton saveRDRButton = new JButton();
    JButton clearButton = new JButton();
    JButton undoButton = new JButton();
    JButton printButton = new JButton();
    JButton saveNNMC = new JButton();
    JButton deleteBFL = new JButton();
//    JToggleButton btListenToDog = new JToggleButton();
    JCheckBox chbReadStream = new JCheckBox();
    JCheckBox chbUseRDR = new JCheckBox();
    JCheckBox chbWYSIWYG = new JCheckBox();
    JCheckBox chbUseNNMC = new JCheckBox();
    JLabel jLabel1 = new JLabel();
    JTextField jtfBrushSize = new JTextField();
    JCheckBox chbShowBackGround = new JCheckBox();

    int currentColor = CommonSense.NOCOLOUR;
    int brushSize = 0;
    protected ColourClassifier colourRDR;
    YUVStreamClient yuvListener = null;
    boolean bClassifyBGOnly = false;

    byte [][][] nnmc = null;

    long lastTimeUpdated = 0;

	LinkedList UpdateBFLlist;

	Thread rdrThread = new Thread() {
		public void run() {
			interLoop: while (!isInterrupted()) {
				BFL bfl;
				synchronized (UpdateBFLlist) {
					while (UpdateBFLlist.isEmpty()) {
						try {
							UpdateBFLlist.wait();
						} catch (InterruptedException e) {
							continue interLoop;
						}
					}
					bfl = (BFL)UpdateBFLlist.removeFirst();
				}
				boolean changeMade = false;
				if (chbUseNNMC.isSelected()){
					//System.out.println("using nnmc");
					if (nnmc == null) saveNNMC_actionPerformed(null);
					for (int i = 0; i < bfl.HEIGHT; i++) {
						for (int j = 0; j < bfl.WIDTH; j++) {
							// FIXME: when unsignedByteToInt is fixed call it instead
							// of the bit-op
							int y = /*Utils.unsignedByteToInt*/( bfl.Y[i][j] &0xff);
							int u = /*Utils.unsignedByteToInt*/( bfl.U[i][j] &0xff);
							int v = /*Utils.unsignedByteToInt*/( bfl.V[i][j] &0xff);
							byte newCol = nnmc[ y  / 2 ] [ u / 2 ] [ v / 2 ];
							if (bfl.C[i][j] != newCol) {
								bfl.C[i][j] = newCol;
								changeMade = true;
							}
						}
					}
				}
				else{
					//System.out.println("using rdr");
					for (int i = 0; i < bfl.HEIGHT; i++) {
						for (int j = 0; j < bfl.WIDTH; j++) {
							byte newCol;
							if (chbWYSIWYG.isSelected()) {
								newCol = (byte) colourRDR.test_example(
									roundTo(bfl.Y[i][j], CommonSense.NNMC_FACTOR),
									roundTo(bfl.U[i][j], CommonSense.NNMC_FACTOR),
									roundTo(bfl.V[i][j], CommonSense.NNMC_FACTOR));
							} else {
								newCol = (byte) colourRDR.test_example(
									bfl.Y[i][j],
									bfl.U[i][j],
									bfl.V[i][j]);
							}
							if (bfl.C[i][j] != newCol) {
								bfl.C[i][j] = newCol;
								changeMade = true;
							}
						}
					}
				}
				
				if (changeMade && bfl.equals(getBFL())) {
					parent.repaint(100);
				}
			}
		}
	};

	void makeNewColourRDR() {
        colourRDR = new KernelColourClassifier(); 
        //colourRDR = new ManRDRColour(3); //Y,U,V -> 3
	}

    public ToolColourRDR(RDRApplication parent) {
        super(parent);
        makeNewColourRDR();
        UpdateBFLlist = new LinkedList();
        rdrThread.start();
        try {
            jbInit();
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    void jbInit() throws Exception {
        btOpenBFL.setPreferredSize(new Dimension(121, 30));
        btOpenBFL.setMaximumSize(new Dimension(121, 30));
        btOpenBFL.setMinimumSize(new Dimension(70, 30));
        btOpenBFL.setHorizontalTextPosition(SwingConstants.CENTER);
        btOpenBFL.setText("Open");
        btOpenBFL.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                btOpenBFL_actionPerformed(e);
            }
        });

        saveBFLButton.setPreferredSize(new Dimension(121, 30));
        saveBFLButton.setMaximumSize(new Dimension(121, 30));
        saveBFLButton.setMinimumSize(new Dimension(70, 30));
        saveBFLButton.setText("Save BFL");
        saveBFLButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                saveBFLButton_actionPerformed(e);
            }
        });

        saveAllBFLButton.setPreferredSize(new Dimension(121, 30));
        saveAllBFLButton.setMaximumSize(new Dimension(121, 30));
        saveAllBFLButton.setMinimumSize(new Dimension(70, 30));
        saveAllBFLButton.setText("Save All BFL");
        saveAllBFLButton.setToolTipText("Apply Current RDR to all BFL files in the Thumbnail bar");
        saveAllBFLButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                saveAllBFLButton_actionPerformed(e);
            }
        });

        btLoadColourRDR.setPreferredSize(new Dimension(121, 30));
        btLoadColourRDR.setMaximumSize(new Dimension(121, 30));
        btLoadColourRDR.setMinimumSize(new Dimension(70, 30));
        btLoadColourRDR.setHorizontalTextPosition(SwingConstants.CENTER);
        btLoadColourRDR.setText("Load RDR");
        btLoadColourRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                btLoadColourRDR_actionPerformed(e);
            }
        });

        clearButton.setPreferredSize(new Dimension(121, 30));
        clearButton.setMaximumSize(new Dimension(121, 30));
        clearButton.setMinimumSize(new Dimension(70, 30));
        clearButton.setText("Clear");
        clearButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                clearButton_actionPerformed(e);
            }
        });

        undoButton.setPreferredSize(new Dimension(121, 30));
        undoButton.setMaximumSize(new Dimension(121, 30));
        undoButton.setMinimumSize(new Dimension(70, 30));
        undoButton.setText("Undo");
        undoButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                undoButton_actionPerformed(e);
            }
        });

        printButton.setMaximumSize(new Dimension(121, 30));
        printButton.setMinimumSize(new Dimension(70, 30));
        printButton.setText("Print");
        printButton.setPreferredSize(new Dimension(121, 30));
        printButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                printButton_actionPerformed(e);
            }
        });

        saveNNMC.setMaximumSize(new Dimension(121, 30));
        saveNNMC.setMinimumSize(new Dimension(70, 30));
        saveNNMC.setPreferredSize(new Dimension(121, 30));
        saveNNMC.setActionCommand("jButton1");
        saveNNMC.setText("Save NNMC");
        saveNNMC.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                saveNNMC_actionPerformed(e);
            }
        });

        deleteBFL.setMaximumSize(new Dimension(121, 30));
        deleteBFL.setMinimumSize(new Dimension(70, 30));
        deleteBFL.setPreferredSize(new Dimension(121, 30));
        deleteBFL.setActionCommand("jButton1");
        deleteBFL.setText("Delete BFL");
        deleteBFL.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                deleteBFL_actionPerformed(e);
            }
        });


//        btListenToDog.setMaximumSize(new Dimension(100, 20));
//        btListenToDog.setPreferredSize(new Dimension(110, 20));
//        btListenToDog.setToolTipText("");
//        btListenToDog.setText("Listen To Dog");
//        btListenToDog.addActionListener(new java.awt.event.ActionListener() {
//            public void actionPerformed(ActionEvent e) {
//                btListenToDog_actionPerformed(e);
//            }
//        });

        chbReadStream.setMaximumSize(new Dimension(121, 30));
        chbReadStream.setPreferredSize(new Dimension(121, 30));
        chbReadStream.setText("Read Stream?");
        chbReadStream.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chbReadStream_actionPerformed(e);
            }
        });


        chbUseRDR.setMaximumSize(new Dimension(121, 30));
        chbUseRDR.setPreferredSize(new Dimension(121, 30));
        chbUseRDR.setText("Use RDR?");
        chbUseRDR.setSelected(true);
        chbUseRDR.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chbUseRDR_actionPerformed(e);
            }
        });
        chbWYSIWYG.setMaximumSize(new Dimension(121, 30));
        chbWYSIWYG.setPreferredSize(new Dimension(121, 30));
        chbWYSIWYG.setText("WYSIWYG?");
        chbWYSIWYG.setSelected(true);
        chbWYSIWYG.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chbWYSIWYG_actionPerformed(e);
            }
        });

        chbUseNNMC.setMaximumSize(new Dimension(121, 30));
        chbUseNNMC.setPreferredSize(new Dimension(121, 30));
        chbUseNNMC.setText("Use NNMC?");
        chbUseNNMC.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chbUseNNMC_actionPerformed(e);
            }
        });

        jLabel1.setMaximumSize(new Dimension(121, 30));
        jLabel1.setPreferredSize(new Dimension(121, 30));
        jLabel1.setToolTipText("");
        jLabel1.setText("Brush Size");

        jtfBrushSize.setMaximumSize(new Dimension(121, 30));
        jtfBrushSize.setMinimumSize(new Dimension(6, 21));
        jtfBrushSize.setPreferredSize(new Dimension(121, 30));
        jtfBrushSize.setRequestFocusEnabled(true);
        jtfBrushSize.setText("1");
        jtfBrushSize.setColumns(1);
        jtfBrushSize.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                jtfBrushSize_actionPerformed(e);
            }
        });

        saveRDRButton.setEnabled(true);
        saveRDRButton.setDebugGraphicsOptions(0);
        saveRDRButton.setPreferredSize(new Dimension(121, 30));
        saveRDRButton.setDisabledSelectedIcon(null);
        saveRDRButton.setMaximumSize(new Dimension(121, 30));
        saveRDRButton.setMinimumSize(new Dimension(70, 30));
        saveRDRButton.setHorizontalTextPosition(SwingConstants.CENTER);
        saveRDRButton.setText("Save RDR");

        saveRDRButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                saveRDRButton_actionPerformed(e);
            }
        });

        chbShowBackGround.setMaximumSize(new Dimension(121, 30));
        chbShowBackGround.setPreferredSize(new Dimension(121, 30));
        chbShowBackGround.setToolTipText("");
        chbShowBackGround.setSelected(true);
        chbShowBackGround.setText("Show Back Ground");


        setLayout(gridBagLayout1);
        setMaximumSize(new Dimension(100, 30));
        setMinimumSize(new Dimension(100, 30));
        setPreferredSize(new Dimension(100, 30));
        setInputVerifier(null);
        chbClassifyBGOnly.setText("Classify BG only?");
        chbClassifyBGOnly.setMaximumSize(new Dimension(121, 30));
        chbClassifyBGOnly.setPreferredSize(new Dimension(121, 30));
        chbClassifyBGOnly.setSelected(bClassifyBGOnly);
        chbClassifyBGOnly.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(ActionEvent e) {
                chbClassifyBGOnly_actionPerformed(e);
            }
        });

//        this.add(btListenToDog, null);
        this.add(chbShowBackGround,                                                           new GridBagConstraints(0, 14, 2, 1, 0.0, 0.1
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(jLabel1,                                                       new GridBagConstraints(0, 8, 1, 1, 0.0, 0.1
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(chbReadStream,                                                          new GridBagConstraints(0, 9, 2, 1, 0.0, 0.1
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(chbUseNNMC,                                                     new GridBagConstraints(0, 12, 2, 1, 0.0, 0.1
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(clearButton,                            new GridBagConstraints(0, 16, 2, 1, 0.0, 0.1
            ,GridBagConstraints.EAST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(chbUseRDR,                            new GridBagConstraints(0, 10, 2, 1, 0.0, 0.1
            ,GridBagConstraints.SOUTHEAST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(chbWYSIWYG,                            new GridBagConstraints(0, 11, 2, 1, 0.0, 0.1
            ,GridBagConstraints.SOUTHEAST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(saveRDRButton,                      new GridBagConstraints(0, 2, 2, 1, 0.0, 0.1
            ,GridBagConstraints.SOUTHWEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(btOpenBFL,                     new GridBagConstraints(0, 0, 2, 1, 0.0, 0.1
            ,GridBagConstraints.SOUTHWEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(btLoadColourRDR,                     new GridBagConstraints(0, 1, 2, 1, 0.0, 0.1
            ,GridBagConstraints.SOUTHWEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(printButton,                     new GridBagConstraints(0, 3, 2, 1, 0.0, 0.1
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(saveNNMC,                   new GridBagConstraints(0, 7, 2, 1, 0.0, 0.1
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(undoButton,                  new GridBagConstraints(0, 5, 2, 1, 0.0, 0.1
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(saveAllBFLButton,               new GridBagConstraints(0, 6, 2, 1, 0.0, 0.1
            ,GridBagConstraints.SOUTHEAST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(deleteBFL,             new GridBagConstraints(0, 15, 2, 1, 0.0, 0.1
            ,GridBagConstraints.EAST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(saveBFLButton,            new GridBagConstraints(0, 4, 2, 1, 0.0, 0.1
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
        this.add(jtfBrushSize,            new GridBagConstraints(1, 8, 1, 1, 0.0, 0.1
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 18, 0));
        this.add(chbClassifyBGOnly,      new GridBagConstraints(0, 13, 2, 1, 0.0, 0.1
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(2, 2, 0, 2), 0, 0));
    }

    public void update(Subject s) {
        if (s instanceof BFL){
            if (chbUseRDR.isSelected() ) {
                lastTimeUpdated = System.currentTimeMillis();
                apply_rdr((BFL) s);
                notifyObservers();
                parent.repaint();
            }
            else { //don't apply rdr, the color is from the original cplane
                //System.out.println("Displaying what the dog sees");
                parent.repaint();
            }
        }
    }

    // this button displays the file choose dialog and reads a BFL file
    void btOpenBFL_actionPerformed(ActionEvent e) {
        parent.openBFL();
    }

    void saveBFLButton_actionPerformed(ActionEvent e) {
        parent.itemBFLSave_actionPerformed(e);
    }

    void saveAllBFLButton_actionPerformed(ActionEvent e) {
        parent.applyColourRDRToAllOpenBFL();
    }


    // this button displays the file choose dialog and reads a BFL file

    void btLoadColourRDR_actionPerformed(ActionEvent event) {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.getLastColourRDR()));
        int returnVal = RDRApplication.RDRFileChooser.showOpenDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().getPath();
        RDRApplication.appConf.setLastColourRDR(filename);
        loadRDRFromFile(filename);

    }

    void clearButton_actionPerformed(ActionEvent e) {
        resetColourRDR();
    }

    void undoButton_actionPerformed(ActionEvent e) {
        undoColourRDR();
    }

    void printButton_actionPerformed(ActionEvent e) {
        colourRDR.print();
    }

    void saveNNMC_actionPerformed(ActionEvent e) {
        saveNNMC("nnmc.cal");
    }

    void deleteBFL_actionPerformed(ActionEvent e) {
        parent.deleteBFL();
    }


//    void btListenToDog_actionPerformed(ActionEvent e) {
//        if (debug) System.out.println("Listen To Dog clicked");
//        if (yuvListener == null) {
//            System.out.println("yuvListener = null, create one.");
//            yuvListener = new YUVStreamClient();
//            yuvListener.addYUVListener(parent);
//        }
//        if (yuvListener.isRunning()) {
//            System.out.println("Stopping");
//            yuvListener.stop();
//            yuvListener = null;
//            System.gc();
//        }
//        else {
//            System.out.println("Starting");
//            yuvListener.start();
//        }
//    }

    void chbReadStream_actionPerformed(ActionEvent e) {
       parent.bReceiveStream = chbReadStream.isSelected();
    }


    // this button displays the file choose dialog and reads a BFL file
    void saveRDRButton_actionPerformed(ActionEvent event) {
        RDRApplication.RDRFileChooser.setCurrentDirectory(new File(RDRApplication.appConf.getLastColourRDR()));
        int returnVal = RDRApplication.RDRFileChooser.showSaveDialog(this);

        if (returnVal == JFileChooser.CANCEL_OPTION) {
            return;
        }

        String filename = RDRApplication.RDRFileChooser.getSelectedFile().getPath();
        colourRDR.saveToFile(filename);
    }

    void jtfBrushSize_actionPerformed(ActionEvent e) {
        try {
            int bsize = Integer.parseInt(jtfBrushSize.getText());
            brushSize = bsize - 1;
            System.out.println("Set size : " + bsize);
        }
        catch (NumberFormatException ex) {
            System.out.println("Invalid brush size : " + jtfBrushSize.getText());
        }
    }

    boolean isHighlighted(int x, int y) {
        BFL bfl = getBFL();
        if (y < 0 || y >= bfl.HEIGHT || x < 0 || x >= bfl.WIDTH) {
            return true;
        }

        return bfl.C[y][x] == currentColor;
    }

    public void apply_rdrAll(String [] bflFileNames){
        for (int i=0;i<bflFileNames.length;i++){
            BFL bfl = new BFL(false);
            try {
                bfl.readFile(bflFileNames[i]);
                apply_rdr(bfl);
                bfl.saveFile();
            }
            catch (FileNotFoundException ex) {
                System.out.println("Error applyRDR to file " + bflFileNames[i]);
            }
            catch (IOException ex) {
                System.out.println("IOException: Error applyRDR to file " + bflFileNames[i]);
            }
        }
        System.out.println("Apply successfully to " + bflFileNames.length + " BFL files.");
    }

    public void apply_rdr(BFL bfl) {
        //System.out.println("apply_rdr");
        
        synchronized (UpdateBFLlist) {
        	if (!UpdateBFLlist.contains(bfl)) {
    	    	UpdateBFLlist.add(bfl);
        		UpdateBFLlist.notify();
        	}
        }
    }

    // rounds b down to the nearest multiple of fac via integer division
    private byte roundTo(byte b, int fac) { return (byte)((b / fac) * fac);}

    private void highlight(int x, int y, boolean painting) {
        if (painting && bClassifyBGOnly 
                && getBFL().getC(x,y) != CommonSense.NOCOLOUR
                && getBFL().getC(x,y) != CommonSense.BACKGROUND)
            return;

        if (debug) System.out.println("Hightlighting " + x + " " + y + " " 
                + currentColor);

        BFL bfl = getBFL();
        if (bfl.outOfBound(x,y)) {
            return;
        }
        byte Y = bfl.getY(x,y);
        byte U = bfl.getU(x,y);
        byte V = bfl.getV(x,y);
        if (chbWYSIWYG.isSelected()) {
            Y = roundTo(Y, CommonSense.NNMC_FACTOR);
            U = roundTo(U, CommonSense.NNMC_FACTOR);
            V = roundTo(V, CommonSense.NNMC_FACTOR);
        }

        int C = 0;
        if (painting){
            C = currentColor;
        }
        else{
        	if ((getBFL().getC(x,y) != currentColor) &&	// don't unclassify a colour that isn't the current colour
        			(currentColor != CommonSense.NOCOLOUR) &&
	        		(currentColor != CommonSense.BACKGROUND))
        		return;
            C = CommonSense.BACKGROUND;
        }

        boolean res = colourRDR.training_example(Y,U,V,C);
        System.out.println("Classifying Y = " + Y + " , U = " + U + " , V = " 
                + V + " ... C = " + C + "  result : "
                + (res?"changed":"unchanged"));
        parent.jtfStatus.setText("Classifying Y = " + Y + " , U = " + U
                + " , V = " + V + " ... C = " + C + "  result : "
                + (res?"changed":"unchanged"));
        if ( res ){
            apply_rdr(getBFL());
        }
    }

    void classifyPixel(int x, int y) {
		for (int i = -brushSize; i <= brushSize; i++) {
			for (int j = -brushSize; j <= brushSize; j++) {
				int xi = x + i;
				int yj = y + j;
				highlight(xi, yj, true);
			}
		}
    }

    void unclassifyPixel(int x, int y) {
        //massive unclassifying, as requested by Alex
        for (int i = -brushSize; i <= brushSize; i++) {
            for (int j = -brushSize; j <= brushSize; j++) {
                int xi = x + i;
                int yj = y + j;
                highlight(xi, yj, false);
            }
        }

    }

    public void mousePressed(MouseEvent e) {
        if (chbReadStream.isSelected() && System.currentTimeMillis() - lastTimeUpdated < NO_TOUCH_TIME){
            System.out.println("Too soon to touch : " + (System.currentTimeMillis() - lastTimeUpdated) );
            return;
        }

        int x = parent.bflPanel.getPosX(e.getX());
        int y = parent.bflPanel.getPosY(e.getY());

        if (SwingUtilities.isRightMouseButton(e)) {
            unclassifyPixel(x,y);
        }
        else {
            classifyPixel(x,y);
        }
        //parent.repaint();
    }



    public void loadRDRFromFile(String filename){
        resetColourRDR();
        colourRDR.loadFromFile(filename);
        apply_rdr(getBFL());
        getBFL().notifyObservers(); //kinda hack here sothat BFL file notifyObserver,
                                    //avoiding circular observing
        parent.repaint();
    }

    void undoColourRDR() {
        colourRDR.undo();
        apply_rdr(getBFL());
        parent.repaint();
    }

    /*
    private byte getPopular(byte c1,byte c2,byte c3, byte c4){
        if (c1 == c2 || c1 == c3 || c1 == c4) return c1;
        else if (c2 == c3 || c2 == c4) return c2;
        else if (c3 == c4) return c3;
        else return c1;
    }
    */

    public void saveNNMC(String filename) {
        try {
            System.out.print("Saving nnmc file, wait...");
            OutputStream out = new FileOutputStream(filename);
            nnmc = new byte[CommonSense.CUBE_SIZE][CommonSense.CUBE_SIZE][CommonSense.CUBE_SIZE];

            for (int y = 0; y < CommonSense.CUBE_SIZE; y++) {
                for (int u = 0; u < CommonSense.CUBE_SIZE; u++) {
                    for (int v = 0; v < CommonSense.CUBE_SIZE; v++) {
                        byte c1 = (byte) colourRDR.test_example(
                                (byte) (2 * y), (byte) (2 * u), (byte) (2 * v));
                        /*
                        // alexn: what is this doing??
                        byte c2 = (byte) colourRDR.test_example(
                                (byte) (2 * y + 1), (byte) (2 * u + 1),
                                (byte) (2 * v + 1));
                        byte c3 = (byte) colourRDR.test_example(
                                (byte) (2 * y + 2), (byte) (2 * u + 2),
                                (byte) (2 * v + 2));
                        byte c4 = (byte) colourRDR.test_example(
                                (byte) (2 * y + 3), (byte) (2 * u + 3), 
                                (byte) (2 * v + 3));
                        //byte c = getPopular(c1,c2,c3,c4);
                         */
                        byte c = c1;
                        nnmc[y][u][v] = c;
                        out.write(c);
                    }
                }
            }
            out.flush();
            out.close();
            System.out.println("  NNMC file '" + filename + "' saved!");
        }
        catch (IOException ex) {
            System.out.println("Can not create file " + filename);
            ex.printStackTrace();
        }

    }

    void resetColourRDR() {
        getBFL().clear_labels();
        makeNewColourRDR();
        parent.repaint();
    }

    public String name(){
        return "Colour";
    }

    public void paintBFLPanel(Graphics g) {
//        if (!enabled )return;
        if (currentColor == CommonSense.NOCOLOUR ) return;

        BFL bfl = getBFL();
        for (int y = 0; y < bfl.HEIGHT; y++) {
            for (int x = 0; x < bfl.WIDTH; x++) {

                //if pixel is classified draw marking box

                if (currentColor == CommonSense.ALLCOLOUR) {
                    parent.bflPanel.drawColourBox(x, y, g);
                }
                else {
                    if (chbShowBackGround.isSelected()) {
                        if (bfl.C[y][x] == currentColor) {
                            parent.bflPanel.drawColourBox(x, y, g);
                        }
                        else if (bfl.C[y][x] == CommonSense.NOCOLOUR ||
                                 bfl.C[y][x] == CommonSense.BACKGROUND) {
                            parent.bflPanel.drawColourBox(x, y, g);
                        }
                    }
                    else if (bfl.C[y][x] == currentColor) {
//                        parent.bflPanel.drawColourBox(x, y, g);
                        parent.bflPanel.drawBox(x, y, g); // black
                    }

                }
            }
        }
        if (debugPainting) System.out.println("ColourRDRTool : Done painting ");
    }

    public void setDisplayColor(int color){
        currentColor = color;
    }

    ///////////////////////////// SUBJECT INTERFACE ///////////////////////////
    private ArrayList observers = new ArrayList();
    JCheckBox chbClassifyBGOnly = new JCheckBox();
    GridBagLayout gridBagLayout1 = new GridBagLayout();

    public void addObserver(Observer o) {
        observers.add(o);
    }

    public void removeObserver(Observer o) {
        observers.remove(o);
    }

    protected void notifyObservers() {
        // loop through and notify each observer
        Iterator i = observers.iterator();
        while (i.hasNext()) {
            Observer o = (Observer) i.next();
            o.update(this);
        }
    }

    void chbClassifyBGOnly_actionPerformed(ActionEvent e) {
        bClassifyBGOnly = chbClassifyBGOnly.isSelected();
    }

    void chbUseRDR_actionPerformed(ActionEvent e) {
        //TODO: this doen's work, need to reload BFL when chb is unchecked
        update(getBFL());
    }
    
    void chbUseNNMC_actionPerformed(ActionEvent e) {
        update(getBFL());
    }
    
    void chbWYSIWYG_actionPerformed(ActionEvent e) {
        update(getBFL());
    }

}
