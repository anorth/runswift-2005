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
 * $Id: TreeGenerationPanel.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * panel which runs the c4.5 to generation the decision tree
 * 
**/

//package learning;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

public class TreeGenerationPanel extends JPanel {

    MainGUI parent;
    
    // gui for c4.5 panel
    JPanel step6Panel                = new JPanel(new BorderLayout());
    JPanel step6DescPanel            = new JPanel(new BorderLayout());
    JLabel step6DescLabel            = new JLabel("Run c4.5 for the object names and data to generate decision tree");
    TitledBorder step6Border         = new TitledBorder("Step 6");
    JPanel c4_5Panel                 = new JPanel();
    JLabel SelectedObjectLabel       = new JLabel("Selected Object:");
//    JComboBox objectComboBox         = new JComboBox(Constant.DEFAULT_BLOB_NAME);
    JLabel objectLabel               = new JLabel();
    JLabel treeExtensionLabel        = new JLabel("Tree Extension:");
    JTextField treeExtTextField      = new JTextField(5);
    JButton c4_5Button               = new JButton(Constant.C4_5_COMMAND);

    // gui for tree to code panel
    JPanel step7Panel                = new JPanel(new BorderLayout());
    JPanel step7DescPanel            = new JPanel(new BorderLayout());
    JLabel step7DescLabel            = new JLabel("Convert the generated decision tree to code.");
    TitledBorder step7Border         = new TitledBorder("Step 7");
    JPanel treeToCodePanel           = new JPanel();
    JLabel treeToCodeFileLabel       = new JLabel("Append to File:");
    JLabel treeToCodePrefixLabel     = new JLabel(Constant.TEMP_LABEL + "/");
    JLabel treeToCodeObjectLabel     = new JLabel();
//    JTextField treeToCodeTextField   = new JTextField(10);
//    JButton treeToCodeChooseButton   = new JButton("Browse...");
    JLabel dataAttributeFileLabel    = new JLabel();
    JButton treeToCodeButton         = new JButton("Convert Tree to Code");

    // gui for updating to VisualCortex.cc
    JPanel step8Panel                = new JPanel(new BorderLayout());
    JPanel step8DescPanel            = new JPanel(new BorderLayout());
    JLabel step8DescLabel            = new JLabel("Update the generated code to VisualCortex.cc.");
    TitledBorder step8Border         = new TitledBorder("Step 8");
    JPanel codeUpdatePanel           = new JPanel();
    JButton codeUpdateButton         = new JButton("Code Update");
    
    public TreeGenerationPanel(MainGUI parent) {
        this.parent = parent;
        init();
    }
    
    private void init() {
        // use combox box for each objects and a button c4.5 to generate the tree for the selected object
//        objectComboBox.setSelectedIndex(parent.getSanityObject());
//        objectComboBox.setEditable(false);
/*        objectComboBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                treeToCodeObjectLabel.setText((String) objectComboBox.getSelectedItem());
            }
        });*/
        treeExtTextField.setText(this.parent.getProperty(Constant.TREE_OUTPUT_EXTENSION_KEY, Constant.DEFAULT_TREE_OUTPUT_EXTENSION));
        c4_5Button.setMnemonic(KeyEvent.VK_C);
        c4_5Button.setToolTipText("Run c4.5 of the selected object");
        c4_5Button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String treeOutputExtension = treeExtTextField.getText();
                if (!Utility.validString(treeOutputExtension)) {
                    JOptionPane.showMessageDialog(parent, "Invalid tree output extension", "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                parent.updateProperty(Constant.TREE_OUTPUT_EXTENSION_KEY, treeOutputExtension);

                int selectedObjectIndex = parent.getSanityObject();
                String generatedFolder, selectedObjectName, selectedObjectFile;
                generatedFolder    = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                selectedObjectName = parent.getProperty(Constant.BLOB_NAME_KEY[selectedObjectIndex], Constant.DEFAULT_BLOB_NAME[selectedObjectIndex]);
                selectedObjectFile = generatedFolder + File.separator + selectedObjectName + File.separator + selectedObjectName;
                
                // make sure the files object.data and object.names are present
                File dataFile, namesFile;
                dataFile  = new File(selectedObjectFile + "." + Constant.C4_5_DATA_EXT);
                namesFile = new File(selectedObjectFile + "." + Constant.C4_5_NAMES_EXT);
                if (!dataFile.exists()) {
                    JOptionPane.showMessageDialog(parent, "File " + selectedObjectFile + "." + Constant.C4_5_DATA_EXT + " missing. c4.5 aborted", "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                if (!namesFile.exists()) {
                    JOptionPane.showMessageDialog(parent, "File " + selectedObjectFile + "." + Constant.C4_5_NAMES_EXT + " missing. c4.5 aborted", "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
                
                try {
                    PrintWriter treeOutput = new PrintWriter(new BufferedWriter(new FileWriter(selectedObjectFile + "." + treeOutputExtension)));
                    Utility.runShellCommand(Constant.C4_5_COMMAND + " " + Constant.C4_5_COMMAND_ARGU + " " + selectedObjectFile, treeOutput);
                    treeOutput.close();
                } catch (IOException ioe) {
                    Utility.printError(parent, "Error writing to file " + selectedObjectFile + "." + treeOutputExtension);
                    ioe.printStackTrace();
                }
            }
        });
        c4_5Panel.add(SelectedObjectLabel);
        c4_5Panel.add(objectLabel);
        c4_5Panel.add(Box.createRigidArea(new Dimension(3, 0)));
        c4_5Panel.add(treeExtensionLabel);
        c4_5Panel.add(Box.createRigidArea(new Dimension(3, 0)));
        c4_5Panel.add(treeExtTextField);
        c4_5Panel.add(Box.createRigidArea(new Dimension(3, 0)));
        c4_5Panel.add(c4_5Button);
        step6DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step6DescPanel.add(step6DescLabel, BorderLayout.WEST);
        step6Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step6Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step6Border));
        step6Panel.add(step6DescPanel, BorderLayout.NORTH);
        step6Panel.add(c4_5Panel, BorderLayout.CENTER);
        
        // convert the tree to code
//        treeToCodeObjectLabel.setText((String) objectComboBox.getSelectedItem());
/*        treeToCodeTextField.setText(this.parent.getProperty(Constant.DECISION_CODE_FILE_KEY, Constant.DEFAULT_DECISION_CODE_FILE));
        treeToCodeChooseButton.setToolTipText("Choose a file to store the attributes");
        treeToCodeChooseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String path = Utility.chooseButtonActionListener(parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER), JFileChooser.FILES_ONLY);
                if (path==null) {return;}
                String generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                int pos = path.indexOf(generatedFolder);
                if (pos>-1) {
                    // skip the generate folder
                    path = path.substring(0, pos) + path.substring(generatedFolder.length()+pos+1); // add 1 for the File.separator 
                    treeToCodeTextField.setText(path);
                } else {
                    JOptionPane.showMessageDialog(parent, "File " + path + " is not in folder " + Constant.TEMP_LABEL + ". Update aborted", "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        });*/
        dataAttributeFileLabel.setText(parent.getProperty(Constant.ATTRIBUTE_DATA_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_DATA_FILE));
        treeToCodeButton.setMnemonic(KeyEvent.VK_C);
        treeToCodeButton.setToolTipText("Convert the decision tree to code in the specified file");
        treeToCodeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {

// now it appends to the same file. what if user doesn't clear the file, then it will append multiple times
// should specify a different file and update to the same places
                int selectedObjectIndex;
                String codeString, generatedFolder, selectedObjectName, selectedObjectFile, treeOutputExtension;

                generatedFolder     = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                treeOutputExtension = parent.getProperty(Constant.TREE_OUTPUT_EXTENSION_KEY, Constant.DEFAULT_TREE_OUTPUT_EXTENSION);

                selectedObjectIndex = parent.getSanityObject();
                selectedObjectName  = parent.getProperty(Constant.BLOB_NAME_KEY[selectedObjectIndex], Constant.DEFAULT_BLOB_NAME[selectedObjectIndex]);
                selectedObjectFile  = generatedFolder + File.separator + selectedObjectName + File.separator + selectedObjectName;
                codeString          = generatedFolder + File.separator + treeToCodeObjectLabel.getText() + dataAttributeFileLabel.getText();

                // re-generate the data attribute to overwrite the file
                if (Utility.filterDataAttribute(codeString, treeToCodeObjectLabel.getText() + " " + Constant.DATA_TARGET)) {
                    try {
                        PrintWriter codeFile = new PrintWriter(new BufferedWriter(new FileWriter(codeString, true))); // append = true
                        // print an empty line to separate the origianl content in the file and what it will update later
                        codeFile.println();
                        codeFile.flush();
                        // pass the inputFilename and the outputFile pointer to the constructor. Use the full tree, not the simplified one
                        if ((new DecisionTreeToCode()).convert(selectedObjectFile + "." + treeOutputExtension, codeFile, false)) {
                            JOptionPane.showMessageDialog(parent, "Decision Tree converted to code successfully", "Information", JOptionPane.INFORMATION_MESSAGE);
                        } else {
                            JOptionPane.showMessageDialog(parent, "Error when converting Decision Tree to Code", "Error", JOptionPane.ERROR_MESSAGE);
                        }
                        codeFile.close();
                    } catch (IOException ioe) {
                        Utility.printError(parent, "Error in writing to file " + codeString + " or in opening file " + selectedObjectFile + "." + treeOutputExtension);
                        ioe.printStackTrace();
                    }
                }
            }
        });
        treeToCodePanel.add(treeToCodeFileLabel);
        treeToCodePanel.add(Box.createRigidArea(new Dimension(3, 0)));
        treeToCodePanel.add(treeToCodePrefixLabel);
        treeToCodePanel.add(treeToCodeObjectLabel);
        treeToCodePanel.add(dataAttributeFileLabel);
        treeToCodePanel.add(Box.createRigidArea(new Dimension(3, 0)));
        treeToCodePanel.add(treeToCodeButton);
        step7DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step7DescPanel.add(step7DescLabel, BorderLayout.WEST);
        step7Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step7Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step7Border));
        step7Panel.add(step7DescPanel, BorderLayout.NORTH);
        step7Panel.add(treeToCodePanel, BorderLayout.CENTER);
        
        // update the generated file to VisualCortex.cc
        codeUpdateButton.setMnemonic(KeyEvent.VK_U);
        codeUpdateButton.setToolTipText("Automatically update the decision tree code to VisualCortex.cc");
        codeUpdateButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String generatedFolder, codeString;
                generatedFolder = parent.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                codeString      = generatedFolder + File.separator + treeToCodeObjectLabel.getText() + dataAttributeFileLabel.getText();
                // specify the full path to the attribute file
                Utility.updateVisualCortex(treeToCodeObjectLabel.getText() + " " + Constant.DECISION_TARGET, codeString, false); // false = writeBlobOutput
            }
        });
        codeUpdatePanel.add(codeUpdateButton);
        step8DescPanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        step8DescPanel.add(step8DescLabel, BorderLayout.WEST);
        step8Border.setTitleFont(new Font("Helvetica", Font.BOLD, 16));
        step8Panel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 10, 0, 10), step8Border));
        step8Panel.add(step8DescPanel, BorderLayout.NORTH);
        step8Panel.add(codeUpdatePanel, BorderLayout.CENTER);
        
        update();

// make a panel to compile and test sanity
        
        this.setLayout(new GridLayout(6, 1));
        this.add(step6Panel);
        this.add(step7Panel);
        this.add(step8Panel);
        this.add(new JPanel());
        this.add(new JPanel());
        this.add(new JPanel());

    }
    
    public String getName() {
        return "Tree Generation";
    }
    
    public String getTip() {
        return "run the c4.5 to generation the decision tree";
    }
    
    public void update() {
        objectLabel.setText(Constant.DEFAULT_BLOB_NAME[parent.getSanityObject()]);
        treeToCodeObjectLabel.setText(objectLabel.getText());
    }
}
