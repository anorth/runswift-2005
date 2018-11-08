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
 * $Id: AttributeGenerationPanel.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * panel which generates the necessary c4.5 attributes from VisualCortex.cc
 * 
**/

//package learning;

import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

public class AttributeGenerationPanel extends JPanel {

    public static final boolean propertyDebugMsg   = false;
    public static final boolean boxLayoutDebugMsg  = false;

    MainGUI parent;
    
    // gui for the reset panel
    JPanel userPanel                 = new JPanel(new GridLayout(2, 1));
    JPanel userTopPanel              = new JPanel();
    JPanel userBottomPanel           = new JPanel();
    TitledBorder userBorder          = new TitledBorder("User Preferences");
    JButton resetButton              = new JButton("Reset");
    JLabel generatedLabel            = new JLabel(Constant.TEMP_LABEL + " = ");
    JTextField generatedFolderField  = new JTextField(25);
    
    // do not allow user to change the preference as it hasn't been tested
//    JButton generatedChooseButton    = new JButton("Browse...");
//    JButton generatedUpdateButton    = new JButton("Update");
    JComboBox folderLocation         = new JComboBox(Constant.IMAGE_FOLDER_LABEL);
    JLabel folderLabel               = new JLabel("Directory =");
    JLabel folderPrefixLabel         = new JLabel(Constant.TEMP_LABEL + File.separator);
    JTextField folderTextField       = new JTextField(10);
//    JButton folderChooseButton       = new JButton("Browse...");
//    JButton folderUpdateButton       = new JButton("Folder Update");
    
    // gui for attribute panel
    JPanel step1Panel                  = new JPanel(new BorderLayout());
    JPanel step1DescPanel              = new JPanel(new BorderLayout());
    JLabel step1DescLabel              = new JLabel("Get the attributes from VisualCortex.cc.");
    TitledBorder step1Border           = new TitledBorder("Step 1");
    JPanel attributePanel              = new JPanel(new GridLayout(2, 1));
    JPanel attributeTopPanel           = new JPanel();
    JPanel attributeBottomPanel        = new JPanel();
    JComboBox objectComboBox           = new JComboBox(Constant.DEFAULT_BLOB_NAME);
    JLabel attributeNamesFileLabel     = new JLabel("Attribute Names File:");
    JLabel attributeNamesPrefixLabel   = new JLabel(Constant.TEMP_LABEL + "/");
    JLabel attributeNamesObjectLabel   = new JLabel();
    JTextField attributeNamesTextField = new JTextField(10);
    JButton attributeNamesChooseButton = new JButton("Browse...");
    JLabel attributeDataFileLabel      = new JLabel("Attribute Data File:");
    JLabel attributeDataPrefixLabel    = new JLabel(Constant.TEMP_LABEL + "/");
    JLabel attributeDataObjectLabel    = new JLabel();
    JTextField attributeDataTextField  = new JTextField(10);
    JButton attributeDataChooseButton  = new JButton("Browse...");
    JButton attributeFilterButton      = new JButton("Filter Attribute");
    
    // gui for updating to VisualCortex.cc
    JPanel step2Panel                = new JPanel(new BorderLayout());
    JPanel step2DescPanel            = new JPanel(new BorderLayout());
    JLabel step2DescLabel            = new JLabel("Update the generated code to VisualCortex.cc.");
    TitledBorder step2Border         = new TitledBorder("Step 2");
    JPanel codeUpdatePanel           = new JPanel();
    JButton codeUpdateButton         = new JButton("Code Update");
    
    // gui for compile panel
    JPanel step3Panel                = new JPanel(new BorderLayout());
    JPanel step3DescPanel            = new JPanel(new BorderLayout());
    JLabel step3DescLabel            = new JLabel("Compile the updated code.");
    TitledBorder step3Border         = new TitledBorder("Step 3");
    JPanel compilePanel              = new JPanel();
    JButton cleanButton              = new JButton(Constant.CLEAN_COMMAND);
    JButton compileButton            = new JButton(Constant.COMPILE_COMMAND);
    
    // gui for the split panel
    JPanel step4Panel                = new JPanel(new BorderLayout());
    JPanel step4DescPanel            = new JPanel(new BorderLayout());
    JLabel step4DescLabel            = new JLabel("Split logFile to frames");
    TitledBorder step4Border         = new TitledBorder("Step 4");
    JPanel splitPanel                = new JPanel();
    JLabel logFileLabel              = new JLabel("Log File Name:");
    JTextField logFileTextField      = new JTextField(30);
    JButton logFileChooseButton      = new JButton("Open Log File...");
    JButton splitButton              = new JButton("Split Frames");

    // gui for the offvision
    JPanel step5Panel                = new JPanel(new BorderLayout());
    JPanel step5DescPanel            = new JPanel(new BorderLayout());
    JLabel step5DescLabel            = new JLabel("Run the Offvision code to generate blobs from 'logFile' from 'start' to 'end'.");
    TitledBorder step5Border         = new TitledBorder("Step 5");
    JPanel offvisionPanel            = new JPanel();
    JPanel offvisionTopPanel         = new JPanel();
    JLabel startIndexLabel           = new JLabel("Start:");
    JLabel endIndexLabel             = new JLabel("End:");
    JTextField startIndexTextField   = new JTextField(4);
    JTextField endIndexTextField     = new JTextField(4);
    JLabel namelistFileLabel         = new JLabel("Namelist File:");
    JLabel namelistPrefixLabel       = new JLabel(Constant.TEMP_LABEL + "/");
    JTextField namelistFileTextField = new JTextField(10);
    JButton namelistFileChooseButton = new JButton("Browse...");
    JButton offvisionButton          = new JButton("Offvision");
    JPanel offvisionBottomPanel      = new JPanel();
    JComboBox imageExtension         = new JComboBox(Constant.IMAGE_EXTENSION_LABEL);
    JTextField extensionTextField    = new JTextField(4);
//    JButton extensionUpdateButton    = new JButton("Extension Update");
        
    public AttributeGenerationPanel(MainGUI parent) {
        this.parent = parent;
        init();
    }
    
    private void init() {
        generatedFolderField.setText(this.parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER));
        generatedFolderField.setEditable(false);
/*        generatedChooseButton.setToolTipText("Choose a folder to store all the generated folder and files");
        generatedChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                generatedFolderField.setText(Utility.chooseButtonActionListener(System.getProperty("user.dir", "."), JFileChooser.DIRECTORIES_ONLY));
            }
        });
        generatedUpdateButton.setMnemonic(KeyEvent.VK_U);
        generatedUpdateButton.setToolTipText("Update the location of the folder to store generated files");
        generatedUpdateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String generatedFolderString = generatedFolderField.getText();
                if (!Utility.validString(generatedFolderString)) {
                    JOptionPane.showMessageDialog(parent, "Invalid folder " + generatedFolderString, "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                String oldFolderString;
                File oldFolder, newFolder;
                oldFolderString = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                oldFolder       = new File(oldFolderString);
                parent.updateProperty(Constant.GENERATED_FOLDER_KEY, generatedFolderString);
                newFolder       = new File(generatedFolderString);
                if (newFolder.exists()) {
                    // update back the old one
                    parent.updateProperty(Constant.GENERATED_FOLDER_KEY, oldFolderString);
                    JOptionPane.showMessageDialog(parent, "'" + generatedFolderString + "' already exists. Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                } else {
                    oldFolder.renameTo(newFolder);
                    JOptionPane.showMessageDialog(parent, Constant.TEMP_LABEL + " folder successfully renamed to '" + generatedFolderString + "'", "Information", JOptionPane.INFORMATION_MESSAGE);
                }
                generatedFolderField.setText(generatedFolderString);
            }
        });*/
        // set the folder names for each object
        for (int i=0; i<Constant.BLOB_NAME_LABEL.length; i++) {
            folderLocation.addItem(Constant.BLOB_NAME_LABEL[i]);
        }
        folderLocation.setSelectedIndex(0);
        folderLocation.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                folderTextField.setText(getFolderText(false));
            }
        });
        folderTextField.setText(getFolderText(false));
        folderTextField.setEditable(false);
/*        folderChooseButton.setToolTipText("Choose a folder to store the corresponding generated files");
        folderChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String path = Utility.chooseButtonActionListener(parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER), JFileChooser.DIRECTORIES_ONLY);
                if (path==null) {return;}
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                int pos = path.indexOf(generatedFolder);
                if (pos>-1) {
                    // skip the generate folder
                    path = path.substring(0, pos) + path.substring(generatedFolder.length()+pos+1); // add 1 for the File.separator 
                    folderTextField.setText(path);
                } else {
                    JOptionPane.showMessageDialog(parent, "File " + path + " is not in folder " + Constant.TEMP_LABEL + ". Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        folderUpdateButton.setMnemonic(KeyEvent.VK_F);
        folderUpdateButton.setToolTipText("Update the location of the corresponding folder");
        folderUpdateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String folderString = folderTextField.getText();
                if (!Utility.validString(folderString)) {
                    JOptionPane.showMessageDialog(parent, "Invalid folder " + folderString, "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                int selectedIndex = folderLocation.getSelectedIndex();
                String key;
                if (selectedIndex<Constant.IMAGE_FOLDER_KEY.length) {
                    key = Constant.IMAGE_FOLDER_KEY[selectedIndex];
                } else {
                    selectedIndex -= Constant.IMAGE_FOLDER_KEY.length;
                    key = Constant.BLOB_NAME_KEY[selectedIndex];
                }

                // rename the old folder to the new
                String oldFolderString, newFolderString;
                File oldFolder, newFolder;
                oldFolderString = getFolderText(true);
                oldFolder = new File(oldFolderString);
                parent.updateProperty(key, folderString); // this one has to be in the middle
                newFolderString = getFolderText(true);
                newFolder = new File(newFolderString);
                if (newFolder.exists()) {
                    String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                    int pos = oldFolderString.indexOf(generatedFolder);
                    // skip the generate folder
                    oldFolderString = oldFolderString.substring(0, pos) + oldFolderString.substring(generatedFolder.length()+pos+1); // add 1 for the File.separator 
                    // update back the old one
                    parent.updateProperty(key, oldFolderString);
                    JOptionPane.showMessageDialog(parent, "'" + Constant.TEMP_LABEL + "/" + folderString + "' already exists. Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                } else {
                    oldFolder.renameTo(newFolder);
                    JOptionPane.showMessageDialog(parent, Constant.IMAGE_FOLDER_LABEL[selectedIndex] + " successfully updated to '" + Constant.TEMP_LABEL + "/" + folderString + "'", "Information", JOptionPane.INFORMATION_MESSAGE);
                }
                folderTextField.setText(getFolderText(false));
            }
        });*/
        userTopPanel.add(generatedLabel);
        userTopPanel.add(generatedFolderField);
        userTopPanel.add(Box.createRigidArea(new Dimension(3, 0)));
//        userTopPanel.add(generatedChooseButton);
//        userTopPanel.add(generatedUpdateButton);
        userBottomPanel.add(folderLocation);
        userBottomPanel.add(folderLabel);
        userBottomPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        userBottomPanel.add(folderPrefixLabel);
        userBottomPanel.add(folderTextField);
//        userBottomPanel.add(Box.createRigidArea(new Dimension(3, 0)));
//        userBottomPanel.add(folderChooseButton);
//        userBottomPanel.add(folderUpdateButton);
        userBorder.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        userPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), userBorder));
        userPanel.add(userTopPanel);
        userPanel.add(userBottomPanel);
        

        objectComboBox.setSelectedIndex(parent.getSanityObject());
        objectComboBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                attributeNamesObjectLabel.setText((String) objectComboBox.getSelectedItem());
                attributeDataObjectLabel.setText((String) objectComboBox.getSelectedItem());
                parent.setSanityObject(objectComboBox.getSelectedIndex());
            }
        });
        attributeNamesObjectLabel.setText((String) objectComboBox.getSelectedItem());
        attributeNamesTextField.setText(this.parent.getProperty(Constant.ATTRIBUTE_NAMES_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_NAMES_FILE));
        attributeNamesChooseButton.setToolTipText("Choose a file to store the attribute names");
        attributeNamesChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String path = Utility.chooseButtonActionListener(parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER), JFileChooser.FILES_ONLY);
                if (path==null) {return;}
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                int pos = path.indexOf(generatedFolder);
                if (pos>-1) {
                    // skip the generate folder
                    path = path.substring(0, pos) + path.substring(generatedFolder.length()+pos+1); // add 1 for the File.separator 
                    attributeNamesTextField.setText(path);
                } else {
                    JOptionPane.showMessageDialog(parent, "File " + path + " is not in folder " + Constant.TEMP_LABEL + ". Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        attributeDataObjectLabel.setText((String) objectComboBox.getSelectedItem());
        attributeDataTextField.setText(this.parent.getProperty(Constant.ATTRIBUTE_DATA_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_DATA_FILE));
        attributeDataChooseButton.setToolTipText("Choose a file to store the attribute data");
        attributeDataChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String path = Utility.chooseButtonActionListener(parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER), JFileChooser.FILES_ONLY);
                if (path==null) {return;}
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                int pos = path.indexOf(generatedFolder);
                if (pos>-1) {
                    // skip the generate folder
                    path = path.substring(0, pos) + path.substring(generatedFolder.length()+pos+1); // add 1 for the File.separator 
                    attributeDataTextField.setText(path);
                } else {
                    JOptionPane.showMessageDialog(parent, "File " + path + " is not in folder " + Constant.TEMP_LABEL + ". Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        attributeFilterButton.setMnemonic(KeyEvent.VK_A);
        attributeFilterButton.setToolTipText("Get c4.5 attributes from VisualCortex.cc");
        attributeFilterButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String attributeNamesFileString = attributeNamesTextField.getText();
                String attributeDataFileString  = attributeDataTextField.getText();
                if (!Utility.validString(attributeNamesFileString)) {
                    JOptionPane.showMessageDialog(parent, "Invalid Names File", "Error", JOptionPane.ERROR_MESSAGE);
                    attributeNamesTextField.requestFocus();
                    return;
                }
                if (!Utility.validString(attributeDataFileString)) {
                    JOptionPane.showMessageDialog(parent, "Invalid Data File", "Error", JOptionPane.ERROR_MESSAGE);
                    attributeDataTextField.requestFocus();
                    return;
                }

// if name is changed, might be better to remove and write a new one
                String selectedItem    = (String) objectComboBox.getSelectedItem();
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                parent.updateProperty(Constant.ATTRIBUTE_NAMES_FILE_KEY, attributeNamesFileString);
                parent.updateProperty(Constant.ATTRIBUTE_DATA_FILE_KEY, attributeDataFileString);
                Utility.filterAttribute(generatedFolder + File.separator + selectedItem + parent.getProperty(Constant.ATTRIBUTE_NAMES_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_NAMES_FILE), 
                                        selectedItem + " " + Constant.NAMES_TARGET, 
                                        generatedFolder + File.separator + selectedItem + parent.getProperty(Constant.ATTRIBUTE_DATA_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_DATA_FILE),
                                        selectedItem + " " + Constant.DATA_TARGET);
            }
        });
        attributeTopPanel.add(objectComboBox);
        attributeTopPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        attributeTopPanel.add(attributeNamesFileLabel);
        attributeTopPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        attributeTopPanel.add(attributeNamesPrefixLabel);
        attributeTopPanel.add(attributeNamesObjectLabel);
        attributeTopPanel.add(attributeNamesTextField);
        attributeTopPanel.add(attributeNamesChooseButton);
        attributeBottomPanel.add(attributeDataFileLabel);
        attributeBottomPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        attributeBottomPanel.add(attributeDataPrefixLabel);
        attributeBottomPanel.add(attributeDataObjectLabel);
        attributeBottomPanel.add(attributeDataTextField);
        attributeBottomPanel.add(attributeDataChooseButton);
        attributeBottomPanel.add(attributeFilterButton);
        attributePanel.add(attributeTopPanel);
        attributePanel.add(attributeBottomPanel);
        if (boxLayoutDebugMsg) {
            attributePanel.add(Box.createHorizontalGlue());
        }
        step1DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step1DescPanel.add(step1DescLabel, BorderLayout.WEST);
        step1Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step1Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step1Border));
        step1Panel.add(step1DescPanel, BorderLayout.NORTH);
        step1Panel.add(attributePanel, BorderLayout.CENTER);

        // update the generated file to VisualCortex.cc
        codeUpdateButton.setMnemonic(KeyEvent.VK_U);
        codeUpdateButton.setToolTipText("Automatically copy the generated attributes code to VisualCortex.cc");
        codeUpdateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String selectedItem    = (String) objectComboBox.getSelectedItem();
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                Utility.updateVisualCortex(Constant.ATTRIBUTE_NAMES_CODE_TARGET, generatedFolder + File.separator + selectedItem + parent.getProperty(Constant.ATTRIBUTE_NAMES_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_NAMES_FILE), true); // true = writeBlobOutput
                Utility.updateVisualCortex(Constant.ATTRIBUTE_DATA_CODE_TARGET, generatedFolder + File.separator + selectedItem + parent.getProperty(Constant.ATTRIBUTE_DATA_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_DATA_FILE), true); // true = writeBlobOutput
            }
        });
        if (boxLayoutDebugMsg) {
            codeUpdatePanel.setLayout(new BoxLayout(codeUpdatePanel, BoxLayout.X_AXIS));
            codeUpdatePanel.add(Box.createHorizontalGlue());
        }
        codeUpdatePanel.add(codeUpdateButton);
        if (boxLayoutDebugMsg) {
            codeUpdatePanel.add(Box.createHorizontalGlue());
        }
        step2DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step2DescPanel.add(step2DescLabel, BorderLayout.WEST);
        step2Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step2Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step2Border));
        step2Panel.add(step2DescPanel, BorderLayout.NORTH);
        step2Panel.add(codeUpdatePanel, BorderLayout.CENTER);

        cleanButton.setMnemonic(KeyEvent.VK_C);
        cleanButton.setToolTipText("clean the Offvision and Robot code (using " + Constant.CLEAN_COMMAND + ")");
        cleanButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                Utility.runShellCommand(Constant.CLEAN_COMMAND);
            }
        });

        compileButton.setMnemonic(KeyEvent.VK_M);
        compileButton.setToolTipText("Compile the Offvision and Robot code (using " + Constant.COMPILE_COMMAND + ")");
        compileButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                // make sure the flag PRE_SANITY_BLOB is set before compiling
                int response = JOptionPane.showOptionDialog(parent, "Make sure you have the flag 'PRE_SANITY_BLOB' set in the Makefile. Click OK to continue", 
                                                           "Warning", JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE, null, Constant.OK_CANCEL_OPTIONS, Constant.OK_CANCEL_OPTIONS[0]);
                if (response!=JOptionPane.YES_OPTION) {
                    return;
                }
// might put a progress bar when compile button is fired
                Utility.runShellCommand(Constant.COMPILE_COMMAND);
            }
        });
        if (boxLayoutDebugMsg) {
            compilePanel.setLayout(new BoxLayout(compilePanel, BoxLayout.X_AXIS));
            compilePanel.add(Box.createHorizontalGlue());
        }
        compilePanel.add(cleanButton);
        compilePanel.add(Box.createHorizontalGlue());
        compilePanel.add(compileButton);
        if (boxLayoutDebugMsg) {
            compilePanel.add(Box.createHorizontalGlue());
        }
        step3DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step3DescPanel.add(step3DescLabel, BorderLayout.WEST);
        step3Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step3Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step3Border));
        step3Panel.add(step3DescPanel, BorderLayout.NORTH);
        step3Panel.add(compilePanel, BorderLayout.CENTER);
        
        logFileTextField.setText(this.parent.getProperty(Constant.LOG_FILE_KEY, Constant.DEFAULT_LOG_FILE));
        logFileChooseButton.setToolTipText("Choose a log file");
        logFileChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                logFileTextField.setText(Utility.chooseButtonActionListener(System.getProperty("user.dir", "."), JFileChooser.FILES_ONLY));
            }
        });
        splitButton.setMnemonic(KeyEvent.VK_S);
        splitButton.setToolTipText("Split the logFile into frames");
        splitButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String logFileString = logFileTextField.getText();
                if (!Utility.validString(logFileString)) {
                    JOptionPane.showMessageDialog(parent, "Invalid log file", "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                parent.updateProperty(Constant.LOG_FILE_KEY, logFileString);

                int lineCounter = 0;

                // get name of logFile
                int pos = logFileString.lastIndexOf('/');
                String logFileName = logFileString;
                if (pos>-1) {
                    logFileName = logFileString.substring(pos);
                }

                String frameFolderPath = parent.getProperty(Constant.IMAGE_FOLDER_KEY[Constant.FRAME_INDEX], Constant.DEFAULT_IMAGE_FOLDER[Constant.FRAME_INDEX], true);
                String frameExtension  = parent.getProperty(Constant.IMAGE_EXTENSION_KEY[Constant.FRAME_INDEX], Constant.DEFAULT_IMAGE_EXTENSION[Constant.FRAME_INDEX]);
                if (frameExtension.length()>0) {
                    frameExtension = "." + frameExtension; // include the dot as well
                }
                
                try {
                    // open logFile
                    BufferedReader logFile = new BufferedReader(new FileReader(logFileString));
                    String line;
                    PrintWriter newFile;
                    // read logFile line by line
                    // skip the first line as it is a version number
                    logFile.readLine();
// this one takes long as well. Might put a progress bar when compile button is fired
                    while ((line=logFile.readLine())!=null) {
                        // keep a line counter
                        lineCounter++;

                        // create a new line with the logFilename and line counter
                        newFile = new PrintWriter(new BufferedWriter(new FileWriter(frameFolderPath + logFileName + "." + lineCounter + frameExtension)));

                        // write each line to the new file
                        newFile.println(line);
                        newFile.flush();
                        newFile.close();
                    }
                    logFile.close();
                    // automatically set the end line to (maximum-1)
                    // it is minus one because the last frame is usually incorrect in size
                    endIndexTextField.setText(""+(lineCounter-1));
                    JOptionPane.showMessageDialog(null, "Frames splitted Successfully", "Information", JOptionPane.INFORMATION_MESSAGE);
                } catch (IOException ioe) {
                    JOptionPane.showMessageDialog(null, "Error in opening file " + logFileString + " or\nwriting to " + frameFolderPath + logFileName + "." + lineCounter, "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        splitPanel.add(logFileLabel);
        splitPanel.add(logFileTextField);
        splitPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        splitPanel.add(logFileChooseButton);
        splitPanel.add(splitButton);
        step4DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step4DescPanel.add(step4DescLabel, BorderLayout.WEST);
        step4Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step4Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step4Border));
        step4Panel.add(step4DescPanel, BorderLayout.NORTH);
        step4Panel.add(splitPanel, BorderLayout.CENTER);

        startIndexTextField.setText(this.parent.getProperty(Constant.START_INDEX_KEY, Constant.DEFAULT_START_INDEX));
        endIndexTextField.setText(this.parent.getProperty(Constant.END_INDEX_KEY, Constant.DEFAULT_END_INDEX));
        namelistFileTextField.setText(this.parent.getProperty(Constant.NAME_LIST_FILE_KEY, Constant.DEFAULT_NAME_LIST_FILE));
        namelistFileChooseButton.setToolTipText("Choose a file to store the listing");
        namelistFileChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String path = Utility.chooseButtonActionListener(parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER), JFileChooser.FILES_ONLY);
                if (path==null) {return;}
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                int pos = path.indexOf(generatedFolder);
                if (pos>-1) {
                    // skip the generate folder
                    path = path.substring(0, pos) + path.substring(generatedFolder.length()+pos+1); // add 1 for the File.separator 
                    namelistFileTextField.setText(path);
                } else {
                    JOptionPane.showMessageDialog(parent, "File " + path + " is not in folder " + Constant.TEMP_LABEL + ". Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        offvisionButton.setMnemonic(KeyEvent.VK_O);
        offvisionButton.setToolTipText("Run the offvision with the generated namelist file");
        offvisionButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String startString, endString, logFile, namelistFile;
                startString  = startIndexTextField.getText();
                endString    = endIndexTextField.getText();
                logFile      = logFileTextField.getText();
                namelistFile = namelistFileTextField.getText();
                if (!Utility.validString(startString) || !Utility.validString(endString) || 
                    !Utility.validString(logFile) || !Utility.validString(namelistFile)) {
                    JOptionPane.showMessageDialog(parent, "Invalid start, end or log file or list file", "Error", JOptionPane.ERROR_MESSAGE);
                    startIndexTextField.requestFocus();
                    return;
                }
                if (Integer.parseInt(startString)<0 || Integer.parseInt(startString)>Integer.parseInt(endString)) {
                    JOptionPane.showMessageDialog(parent, "Invalid start index", "Error", JOptionPane.ERROR_MESSAGE);
                    startIndexTextField.requestFocus();
                    return;
                }
                parent.updateProperty(Constant.START_INDEX_KEY, startString);
                parent.updateProperty(Constant.END_INDEX_KEY, endString);
                parent.updateProperty(Constant.NAME_LIST_FILE_KEY, namelistFile);
                
                // the full path of the name list file
                String fullNamelistFile = parent.getProperty(Constant.NAME_LIST_FILE_KEY, Constant.DEFAULT_NAME_LIST_FILE, true);
                
                // change logFile
                int pos = logFile.lastIndexOf('/');
                String logFileName = logFile;
                if (pos>-1) {
                    logFileName = logFile.substring(pos+1); // skip the '/' as well
                }

                int start, end;
                start = Integer.parseInt(startIndexTextField.getText());
                end   = Integer.parseInt(endIndexTextField.getText());
                try {
                    PrintWriter outputFile = new PrintWriter(new BufferedWriter(new FileWriter(fullNamelistFile)));
                    for (int i=start; i<=end; i++) {
                        outputFile.println(logFileName + "." + i);
                        outputFile.flush();
                    }
                    outputFile.println("quit");
                    outputFile.flush();
                    outputFile.close();
                } catch (IOException ioe) {
                    JOptionPane.showMessageDialog(parent, "Error when writing to " + fullNamelistFile, "Error", JOptionPane.ERROR_MESSAGE);
                    ioe.printStackTrace();
                }
                
                String offvisionCommand = Constant.OFFVISION_COMMAND + " " + fullNamelistFile + " " + parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER) + " ";
                for (int i=0; i<Constant.IMAGE_FOLDER_KEY.length; i++) {
                    offvisionCommand += parent.getProperty(Constant.IMAGE_FOLDER_KEY[i], Constant.DEFAULT_IMAGE_FOLDER[i]) + " ";
                    offvisionCommand += parent.getProperty(Constant.IMAGE_EXTENSION_KEY[i], Constant.DEFAULT_IMAGE_EXTENSION[i]) + " ";
                }
// just the ball at the moment
                offvisionCommand += parent.getProperty(Constant.BLOB_NAME_KEY[0], Constant.DEFAULT_BLOB_NAME[0]);
                
                // run shell command
                Utility.runShellCommand(offvisionCommand);
            }
        });

        // frame, object, blob extension
        imageExtension.setSelectedIndex(0);
        imageExtension.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                extensionTextField.setText(getExtensionText());
            }
        });
        extensionTextField.setText(getExtensionText());
        extensionTextField.setEditable(false);
/*        extensionUpdateButton.setMnemonic(KeyEvent.VK_E);
        extensionUpdateButton.setToolTipText("Update the file extension of the corresponding image");
        extensionUpdateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String newExtension = extensionTextField.getText();
                if (!Utility.validString(newExtension)) {
                    JOptionPane.showMessageDialog(parent, "Invalid extension " + newExtension, "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                // make sure the extension are not the same as the old ones
                String oldExtension;
                for (int i=0; i<Constant.IMAGE_EXTENSION_KEY.length; i++) {
                    oldExtension = parent.getProperty(Constant.IMAGE_EXTENSION_KEY[i], Constant.DEFAULT_IMAGE_EXTENSION[i]);
                    if (oldExtension.equals(newExtension)) {
                        JOptionPane.showMessageDialog(parent, "Extension " + newExtension + " has been used. Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                        return;
                    }
                }
                int selectedIndex = imageExtension.getSelectedIndex();
                parent.updateProperty(Constant.IMAGE_EXTENSION_KEY[selectedIndex], newExtension);
                JOptionPane.showMessageDialog(parent, Constant.IMAGE_EXTENSION_LABEL[selectedIndex] + " successfully updated to " + newExtension, "Information", JOptionPane.INFORMATION_MESSAGE);
            }
        });
*/        
        offvisionTopPanel.add(startIndexLabel);
        offvisionTopPanel.add(startIndexTextField);
        offvisionTopPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        offvisionTopPanel.add(endIndexLabel);
        offvisionTopPanel.add(endIndexTextField);
        offvisionTopPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        offvisionTopPanel.add(namelistFileLabel);
        offvisionTopPanel.add(Box.createRigidArea(new Dimension(1, 0)));
        offvisionTopPanel.add(namelistPrefixLabel);
        offvisionTopPanel.add(namelistFileTextField);
        offvisionTopPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        offvisionTopPanel.add(namelistFileChooseButton);
        offvisionTopPanel.add(offvisionButton);
        offvisionBottomPanel.add(imageExtension);
        offvisionBottomPanel.add(Box.createRigidArea(new Dimension(3, 0)));
        offvisionBottomPanel.add(extensionTextField);
//        offvisionBottomPanel.add(Box.createRigidArea(new Dimension(3, 0)));
//        offvisionBottomPanel.add(extensionUpdateButton);
        offvisionPanel.setLayout(new BoxLayout(offvisionPanel, BoxLayout.Y_AXIS));
        offvisionPanel.add(offvisionTopPanel);
        offvisionPanel.add(offvisionBottomPanel);
        step5DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step5DescPanel.add(step5DescLabel, BorderLayout.WEST);
        step5Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step5Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step5Border));
        step5Panel.add(step5DescPanel, BorderLayout.NORTH);
        step5Panel.add(offvisionPanel, BorderLayout.CENTER);

/*        this.setLayout(new GridLayout(6, 1));
        this.setBorder(compoundBevel);
        this.add(userPanel);
        this.add(step1Panel);
        this.add(step2Panel);
        this.add(step3Panel);
        this.add(step4Panel);
        this.add(step5Panel);
*/
        this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        this.add(userPanel);
        this.add(step1Panel);
        this.add(step2Panel);
        this.add(step3Panel);
        this.add(step4Panel);
        this.add(step5Panel);
    }
    
    private String getFolderText(boolean filePath) {
        int selectedIndex = folderLocation.getSelectedIndex();
        if (selectedIndex<Constant.DEFAULT_IMAGE_FOLDER.length) {
            return parent.getProperty(Constant.IMAGE_FOLDER_KEY[selectedIndex], Constant.DEFAULT_IMAGE_FOLDER[selectedIndex], filePath);
        } else {
            selectedIndex -= Constant.DEFAULT_IMAGE_FOLDER.length;
            return parent.getProperty(Constant.BLOB_NAME_KEY[selectedIndex], Constant.DEFAULT_BLOB_NAME[selectedIndex], filePath);
        }
    }
    
    private String getExtensionText() {
        int selectedIndex = imageExtension.getSelectedIndex();
        return parent.getProperty(Constant.IMAGE_EXTENSION_KEY[selectedIndex], Constant.DEFAULT_IMAGE_EXTENSION[selectedIndex]);
    }
    
    public String getName() {
        return "Attribute Generation";
    }
    
    public String getTip() {
        return "generate the necessary c4.5 attributes";
    }
}
