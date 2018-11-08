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
 * UNSW 2003 Robocup (Eileen Mak)
 *
 * Last modification background information
 * $Id: OrientTester.java 1953 2003-08-21 03:51:39Z eileenm $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Orientation testing
**/

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;



public class OrientTester extends JFrame {

	//--- fields -----------------------------------------
	JButton bnYes;
	JButton bnNo;
	JButton bnDiscard;
	JButton bnFinish;
	
	ImageDisplay imgDisplay;
	OrientTester self;
	
	JPanel  pBackward;
	JPanel  pRight;
	JPanel  pForward;
	JPanel  pLeft;
	JPanel  pRightBackward;
	JPanel  pLeftBackward;
	JPanel  pRightForward;
	JPanel  pLeftForward;
	
	JLabel  lbNumTest;
	
	String   testingDir;
	String[] testingSet;
	int      testingPos;
	int      numYes;
	int      numNo;
	int      numTest;
	
	
	
	
	/**
	 * Init layout and listeners
	 */
	public OrientTester(String testingDir) {
	
		this.testingDir = testingDir;
		getContentPane().setLayout(new BorderLayout());
		setTitle("Orientation Tester");
		self = this;
		
		//--- init test data --------------------------------------
		File dir     = new File(testingDir);
		testingSet   = dir.list();
		testingPos   = -1;
		numYes       = 0;
		numNo        = 0;
		numTest      = 0;
		
		//--- robot image display ---------------------------------
		imgDisplay = new ImageDisplay();
		getContentPane().add(imgDisplay, BorderLayout.CENTER);
		
		
		//--- control panel objects -------------------------------
		bnYes = new JButton("Yes");
		bnYes.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                numYes++;
				numTest++;
				doNext();
            }
        });
		
	    bnNo  = new JButton("No");
		bnNo.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                numNo++;
				numTest++;
				doNext();
            }
        });
		
	    bnDiscard = new JButton("Discard");
		bnDiscard.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                doNext();
            }
        });
		
	    bnFinish  = new JButton("Finish");
		bnFinish.addActionListener( new ActionListener() {
            public void actionPerformed(ActionEvent e) {
			
				int percentCorrect = 0;
				if (numTest>0)
					percentCorrect = (int)(((double)numYes)/numTest * 100);
			
				JOptionPane.showMessageDialog(self, "Accuracy = "+percentCorrect+"%"+
				                                    "\nCorrect = "+numYes+
											        "\nIncorrect = "+numNo+
													"\nTotal Tested = "+numTest);
				self.dispose();
            }
        });
		
		lbNumTest = new JLabel("0", JLabel.CENTER);
		
		pBackward      = new JPanel();
		pRight         = new JPanel();
		pForward       = new JPanel();
		pLeft          = new JPanel();
		pRightBackward = new JPanel();
		pLeftBackward  = new JPanel();
		pRightForward  = new JPanel();
		pLeftForward   = new JPanel();
		
		
		//--- control panel layout -----------------
		JPanel controlPanel = new JPanel();
		controlPanel.setLayout(new GridLayout(7,1,5,5));
		
		JPanel indicatorPanel = new JPanel();
		indicatorPanel.setLayout(new GridLayout(3,3,2,2));
		indicatorPanel.add(pLeftBackward);
		indicatorPanel.add(pBackward);
		indicatorPanel.add(pRightBackward);
		indicatorPanel.add(pLeft);
		indicatorPanel.add(new JPanel());
		indicatorPanel.add(pRight);
		indicatorPanel.add(pLeftForward);
		indicatorPanel.add(pForward);
		indicatorPanel.add(pRightForward);
		
		controlPanel.add(indicatorPanel);
		controlPanel.add(lbNumTest);
		controlPanel.add(new JSeparator());
		controlPanel.add(bnYes);
		controlPanel.add(bnNo);
		controlPanel.add(bnDiscard);
		controlPanel.add(bnFinish);
		
		getContentPane().add(controlPanel, BorderLayout.EAST);
		
		
		//--- show it ------------------------------
		setSize(new Dimension(625,460));
        setVisible(true);
		
		
		//--- start testing ------------------------
		doNext();
	}
	
	
	
	
	/**
	 * Recognize the next image
	 */
	private void doNext() {
		testingPos++;
		
		if (testingPos == testingSet.length) {
			bnFinish.doClick();
		} else {
			imgDisplay.loadImageFile(testingDir + "/" + testingSet[testingPos]);
			lbNumTest.setText((testingPos+1) + "");
			TrainingData d = new TrainingData();
			d.calAttr(imgDisplay.getImage());
d.classi = recognise(d.attributes);
			
			clearIndicator();
			
			if (d.classi == OrientClassifier.BACKWARD) {
                pBackward.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.RIGHT) {
                pRight.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.FORWARD) {
                pForward.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.LEFT) {
                pLeft.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.RIGHT_BACKWARD) {
                pRightBackward.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.LEFT_BACKWARD) {
                pLeftBackward.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.RIGHT_FORWARD) {
                pRightForward.setBackground(Color.red);
            } else if (d.classi == OrientClassifier.LEFT_FORWARD) {
                pLeftForward.setBackground(Color.red);
            }
        }
	}
	
	
	
	/**
	 * Turn all orientation indicator off
	 */
	private void clearIndicator() {
		pForward.setBackground(Color.gray);
		pRight.setBackground(Color.gray);
		pBackward.setBackground(Color.gray);
		pLeft.setBackground(Color.gray);
		pLeftBackward.setBackground(Color.gray);
		pRightBackward.setBackground(Color.gray);
		pLeftForward.setBackground(Color.gray);
		pRightForward.setBackground(Color.gray);
	}
	
	
	
	
	/**
	 * Testing GUI... (Debug only, DO NOT USE)
	 */
	public static void main(String arg[]) {
		OrientTester tester = new OrientTester("/home/eileenm/sampleImages/trainingSet");
	}
	
	
	
	
	
	/////////////////////////////////////////////////////////////////
	// Recognition using decision tree
	// Adding new attributes (note - attribute order matters):
	// 1) Add attribute name to end of TrainingData.attributeNames[]
	// 2) Add possible attribute values to end of TrainingData.attributeValues[]
	// 3) Add feature extraction method to TrainingData's Feature extraction section
	// 4) Add call to feature extraction method to end of TrainingData.calAttr
	// 5) Run ReTrain using OrientClassifier, Save and Generate Data (Orient.*)
	// 6) Run C4.5 -f Orient
	// 7) Convert decision tree to programming code
	// 8) Update recognise method with new programming code
	/////////////////////////////////////////////////////////////////	
	public int recognise(String[] attr) {
		
		//--- get the attributes -----------------------
		String colour                = attr[0];
		int    boundBoxHeight        = Integer.parseInt(attr[1]);
		int    boundBoxRatio         = Integer.parseInt(attr[2]);
		int    largestPatchRatio     = Integer.parseInt(attr[3]);
		int    leftDensity           = Integer.parseInt(attr[4]);
		int    rightDensity          = Integer.parseInt(attr[5]);
		int    backSlashDensity      = Integer.parseInt(attr[6]);
		int    forwardSlashDensity   = Integer.parseInt(attr[7]);
		
		
		//--- start decision tree ----------------------------
		if (rightDensity <= 58) {
			if (backSlashDensity <= 33) {
				if (rightDensity <= 43) {
					return 90;
				} else {
					return 135;
				}
			} else {
				if (largestPatchRatio <= 141) {
					return -45;
				} else {
					return 90;
				}
			}
		} else {	
			if (boundBoxRatio <= 111) {
				if (largestPatchRatio <= 152) {
					return 180;
				} else {
					return 90;
				}
			} else {
				if (leftDensity <= 65) {
					if (forwardSlashDensity <= 35) {
						if (largestPatchRatio <= 128) {
							return -135;
						} else {
							if (leftDensity <= 55) {
								return -90;
							} else {
								return -135;
							}
						}
					} else {
						if (largestPatchRatio <= 155) {
							return 45;
						} else {
							return -90;
						}
					}
				} else {
					if (forwardSlashDensity <= 39) {
						return -45;
					} else {
						if (largestPatchRatio <= 131) {
							return 0;
						} else {
							return 90;
						}
					}
				}
			}
		}
		//--- end decision tree ----------------------------

	}
}
