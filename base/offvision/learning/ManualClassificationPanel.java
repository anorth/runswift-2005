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
 * UNSW 2003 Robocup (Alex Tang)
 *
 * Last modification background information
 * $Id: ManualClassificationPanel.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * panel which allows manual point and click classification of valid/invalid blobs
 * 
**/

//package learning;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

public class ManualClassificationPanel extends JPanel {

    private static final boolean prefixDebugMsg      = false;
    private static final boolean currentFileDebugMsg = false;

    MainGUI parent;
    
    private PaintAreaPanel paintAreaPanel;
    private JFileChooser fileChooser = new JFileChooser();
    
// hard code for the moment, still deciding whether displayObject is needed
    boolean displayBlob = true;

    boolean blobValidInitial = false;  // true if to initialise the blob as classifed correctly at the beginning

    int fileNumber, startIndex, endIndex;
    
    // GUI for the play back panel
    TitledBorder blobberBorder      = new TitledBorder("Blobber Panel");
    JPanel blobberPanel             = new JPanel(new BorderLayout());
    
    JPanel blobStatePanel           = new JPanel();
    String[] radioButtonText        = {"Valid", "Invalid"};
    JRadioButton[] validRadioButton = new JRadioButton[2];
    ButtonGroup buttonGroup         = new ButtonGroup();
    JButton prevBlobButton          = new JButton("Prev");
    JButton nextBlobButton          = new JButton("Next");
    JTextField currentBlob          = new JTextField(6);
    JLabel totalBlobs               = new JLabel();
    JButton writeObjectDataButton   = new JButton("Write c4.5 Data");
    JButton allInvalidButton        = new JButton("All Invalid");

    JPanel shortCutPanel            = new JPanel();
    JLabel shortCutLabel            = new JLabel("Right Click Short Cut:");
    JCheckBox changeStateCheckBox   = new JCheckBox("Change State?", true);
    JCheckBox writeLogCheckBox      = new JCheckBox("Write Log?", true);
    JCheckBox nextFrameCheckBox     = new JCheckBox("Next Frame?", true);

    TitledBorder playBackBorder     = new TitledBorder("Navigate Panel");
    JPanel playBackPanel            = new JPanel();
    JButton backwardBallButton      = new JButton("<B");
    JButton backwardButton          = new JButton("<");
    JButton forwardButton           = new JButton(">");
    JButton forwardBallButton       = new JButton("B>");
    JTextField currentFrame         = new JTextField(4);
    JLabel totalFrame               = new JLabel();
    
    public ManualClassificationPanel(MainGUI parent) {
        this.parent = parent;
        this.initPlayBackPanel();
    }
    
    public void loadFile(int startIndex, int endIndex, boolean displayBlob) {
        this.startIndex  = startIndex;
        this.endIndex    = endIndex;
        this.displayBlob = displayBlob;
        
        // start the animation at the beginning
        this.fileNumber  = startIndex;

        totalFrame.setText("/" + endIndex);

        paintFile();
    }
    
/*    private void loadFile(String imageFilePrefix, int startIndex, int endIndex) {
        JPanel autoPanel   = new JPanel();
        JLabel statusLabel = new JLabel("Processing...");
        JLabel fileLabel   = new JLabel();
        String blobFile, prefix, ext;

        prefix = Constant.BLOB_DIRECTORY + File.separator + imageFilePrefix;
        ext    = Constant.BLOB_EXTENSION;
        
        paintAreaPanel = new PaintAreaPanel(this, true);
        
        autoPanel = new JPanel();
        autoPanel.add(statusLabel);
        autoPanel.add(fileLabel);

        this.add(autoPanel);
        // process the imagefile from startIndex to endIndex
        for (int i=startIndex; i<=endIndex; i++) {
            blobFile = prefix + '.' + i + '.' + ext;
            if (currentFileDebugMsg) {
                System.out.println("ManualClassification.java: " + blobFile);
            }
            if (!paintAreaPanel.readBlobFile(blobFile)) {
                break;
            }
            fileLabel.setText(""+i);
            paintAreaPanel.writeLog();
        }
        statusLabel.setText("Finish");
        fileLabel.setText("");
        JOptionPane.showMessageDialog(this, "Finish Auto Learning", "Information", JOptionPane.INFORMATION_MESSAGE);
    }
*/    
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
                        paintAreaPanel.changeBlobValidState();
                        paintAreaPanel.repaint();
                    }
                });
                buttonGroup.add(validRadioButton[i]);
            }
            validRadioButton[0].setMnemonic(KeyEvent.VK_V);
            validRadioButton[1].setMnemonic(KeyEvent.VK_I);
            
            prevBlobButton.setToolTipText("Display previous Blob");
            prevBlobButton.setEnabled(false);
            prevBlobButton.setMnemonic(KeyEvent.VK_P);
            prevBlobButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    paintAreaPanel.setPrevBlob();
                    paintAreaPanel.repaint();
                }
            });

            nextBlobButton.setToolTipText("Display next Blob");
            nextBlobButton.setMnemonic(KeyEvent.VK_N);
            nextBlobButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    paintAreaPanel.setNextBlob();
                    paintAreaPanel.repaint();
                }
            });
            currentBlob.setEditable(false);
            currentBlob.setHorizontalAlignment(JTextField.RIGHT);
            
            writeObjectDataButton.setToolTipText("Write the c4.5 data and go to next frame automatically.");
            writeObjectDataButton.setMnemonic(KeyEvent.VK_W);
            writeObjectDataButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    paintAreaPanel.writeLog();
                    writeObjectDataButton.setEnabled(false);
                    
                    // go to the next frame automatically after writing
//                    doNextFrame();
                    doNextBallFrame();
                }
            });

            allInvalidButton.setToolTipText("Set the object classification in all the frames to be invalid");
            allInvalidButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    // remember the current filename
                    int currentFileNumber = fileNumber;
                    for (int i=startIndex; i<=endIndex; i++) {
                        fileNumber = i;
                        
                        // call the read cplane and read blob methods to update the blobList, the blobs are always invalid
                        paintFile(false);
                        paintAreaPanel.writeLog();
                    }
                    fileNumber = currentFileNumber;
                    paintFile();
                }
            });
            
            for (int i=0; i<validRadioButton.length; i++) {
                blobStatePanel.add(validRadioButton[i]);
            }
            blobStatePanel.add(Box.createRigidArea(new Dimension(3, 0)));
            blobStatePanel.add(prevBlobButton);
            blobStatePanel.add(nextBlobButton);
            blobStatePanel.add(Box.createRigidArea(new Dimension(3, 0)));
            blobStatePanel.add(currentBlob);
            blobStatePanel.add(totalBlobs);
            blobStatePanel.add(Box.createRigidArea(new Dimension(3, 0)));
            blobStatePanel.add(writeObjectDataButton);
            blobStatePanel.add(Box.createRigidArea(new Dimension(3, 0)));
            blobStatePanel.add(allInvalidButton);
            
            shortCutLabel.setToolTipText("Check/Uncheck the desire short cut options for mouse right click");
            shortCutPanel.add(shortCutLabel);
            shortCutPanel.add(Box.createRigidArea(new Dimension(3, 0)));
            shortCutPanel.add(changeStateCheckBox);
            shortCutPanel.add(writeLogCheckBox);
            shortCutPanel.add(nextFrameCheckBox);
            
            blobberPanel.setBorder(blobberBorder);
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

        backwardBallButton.setToolTipText("Go to the previous frame with at least a ball");
        backwardBallButton.setEnabled(false);
        backwardBallButton.setMnemonic(KeyEvent.VK_A);
        backwardBallButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                doPreviousBallFrame();
            }
        });

        forwardButton.setToolTipText("Go to the next frame");
        forwardButton.setMnemonic(KeyEvent.VK_RIGHT);
        forwardButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                doNextFrame();
            }
        });
        
        forwardBallButton.setToolTipText("Go to the next frame with at least a ball");
        forwardBallButton.setMnemonic(KeyEvent.VK_D);
        forwardBallButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                doNextBallFrame();
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
                    paintAreaPanel.resetBlobIndex();
                    if (!paintFile()) {
                        fileNumber = startIndex;
                        currentFrame.setText(""+fileNumber);
                    }
                } catch (NumberFormatException e2) {
                    JOptionPane.showMessageDialog(null, "Error: Input must be an integer", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        playBackPanel.setBorder(playBackBorder);
        playBackPanel.add(backwardBallButton);
        playBackPanel.add(backwardButton);
        playBackPanel.add(forwardButton);
        playBackPanel.add(forwardBallButton);
        playBackPanel.add(currentFrame);
        playBackPanel.add(totalFrame);

        // setting the mcp
        paintAreaPanel = new PaintAreaPanel(this, displayBlob);
        paintAreaPanel.setBorder(BorderFactory.createEtchedBorder());
        paintAreaPanel.setPreferredSize(new Dimension(710, 580));

        this.setLayout(new BorderLayout());
        if (displayBlob) {
            this.add(paintAreaPanel, BorderLayout.NORTH);
            this.add(blobberPanel, BorderLayout.CENTER);
            this.add(playBackPanel, BorderLayout.SOUTH);
        } else {
            this.add(paintAreaPanel, BorderLayout.CENTER);
            this.add(playBackPanel, BorderLayout.SOUTH);
        }
    }
    
    // show the previous frame
    boolean doPreviousFrame() {
        boolean canDoPrevious = true;
        
        writeObjectDataButton.setEnabled(true);
        fileNumber--;
        paintAreaPanel.resetBlobIndex();
        if (!paintFile()) {
            fileNumber++;
            currentFrame.setText(""+fileNumber);
            canDoPrevious = false;
        }
        forwardButton.setEnabled(true);
        if (fileNumber==startIndex) {
            backwardButton.setEnabled(false);
        }
        
        return canDoPrevious;
    }
    
    // show the next frame
    boolean doNextFrame() {
        boolean canDoNext = true;

        writeObjectDataButton.setEnabled(true);
        fileNumber++;
        paintAreaPanel.resetBlobIndex();
        if (!paintFile()) {
            fileNumber--;
            currentFrame.setText(""+fileNumber);
            canDoNext = false;
        }
        backwardButton.setEnabled(true);
        if (fileNumber==endIndex) {
            forwardButton.setEnabled(false);
        }
        
        return canDoNext;
    }
    
    // show the previous frame with at least a ball
    void doPreviousBallFrame() {
        while (true) {
            if (!doPreviousFrame() || paintAreaPanel.hasBall()) {
                // if the current frame hits the first frame, or if this frame has at least a ball, then break
                break;
            }
        }
        forwardBallButton.setEnabled(true);
        if (fileNumber==startIndex) {
            backwardBallButton.setEnabled(false);
        }
    }

    // show the next frame with at least a ball
    void doNextBallFrame() {
        while (true) {
            if (!doNextFrame() || paintAreaPanel.hasBall()) {
                // if the current frame hits the last frame, or if this frame has at least a ball, then break
                break;
            }
        }
        backwardBallButton.setEnabled(true);
        if (fileNumber==endIndex) {
            forwardBallButton.setEnabled(false);
        }
    }

    /* 
     * paint the cplane file and blob/object file
     *
     * @param blobValid true if all the blobs are classified correctly initially
     */
    boolean paintFile(boolean blobValid) {
        String cplanePath, prefix, cplaneFrame;
        // get the whole logFile path
        cplanePath = parent.getProperty(Constant.LOG_FILE_KEY, Constant.DEFAULT_LOG_FILE);
        int pos = cplanePath.lastIndexOf('/');
        if (pos==-1) {
            Utility.printError(parent, "Error in cplanePath " + cplanePath + ". No '/' found");
            return false;
        }
        // get the logFile prefix only
        prefix      = cplanePath.substring(pos+1);
        cplaneFrame = parent.getProperty(Constant.IMAGE_FOLDER_KEY[Constant.FRAME_INDEX], Constant.DEFAULT_IMAGE_FOLDER[Constant.FRAME_INDEX], true) + 
                      File.separator + prefix + '.' + fileNumber + '.' + parent.getProperty(Constant.IMAGE_EXTENSION_KEY[Constant.FRAME_INDEX], Constant.DEFAULT_IMAGE_EXTENSION[Constant.FRAME_INDEX]);
        // if the reading of the files are unsuccessful
        if (!paintAreaPanel.readCplFile(cplaneFrame)) {
            Utility.printError(parent, "Cannot read " + cplaneFrame);
            return false;
        }
        
        if (displayBlob) {
            // read blob file
            String blobFile = parent.getProperty(Constant.IMAGE_FOLDER_KEY[Constant.BLOB_INDEX], Constant.DEFAULT_IMAGE_FOLDER[Constant.BLOB_INDEX], true) + 
                              File.separator + prefix + '.' + fileNumber + '.' + parent.getProperty(Constant.IMAGE_EXTENSION_KEY[Constant.BLOB_INDEX], Constant.DEFAULT_IMAGE_EXTENSION[Constant.BLOB_INDEX]);
            if (!paintAreaPanel.readBlobFile(blobFile, blobValid)) {
                Utility.printError(parent, "Cannot read " + blobFile);
                return false;
            }
        } else {
            String objectFile = parent.getProperty(Constant.IMAGE_FOLDER_KEY[Constant.OBJECT_INDEX], Constant.DEFAULT_IMAGE_FOLDER[Constant.OBJECT_INDEX], true) + 
                                File.separator + prefix + '.' + fileNumber + '.' + parent.getProperty(Constant.IMAGE_EXTENSION_KEY[Constant.OBJECT_INDEX], Constant.DEFAULT_IMAGE_EXTENSION[Constant.OBJECT_INDEX]);
            if (!paintAreaPanel.readObjFile(objectFile)) {
                Utility.printError(parent, "Cannot read " + objectFile);
                return false;
            }
        }
        paintScreen();
        return true;
    }
    boolean paintFile() {
        return paintFile(blobValidInitial);
    }

    void paintScreen() {
        currentFrame.setText("" + fileNumber);
        if (fileNumber==1) {
            backwardButton.setEnabled(false);
        } else {
            backwardButton.setEnabled(true);
        }
        paintAreaPanel.repaint();
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

    public MainGUI getMainGUI() {
        return parent;
    }
    
    public String getName() {
        return "Manual Classification";
    }
    
    public String getTip() {
        return "allow manual point and click classification of valid/invalid blobs";
    }
    
}
