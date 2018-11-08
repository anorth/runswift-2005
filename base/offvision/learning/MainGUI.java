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
 * $Id: MainGUI.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Machine Learning Main GUI
 * 
**/

//package learning;

import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class MainGUI extends JFrame {

    public static final boolean propertyDebugMsg = false;
    public static final boolean folderDebugMsg = false;

    // user preferences
    private Properties properties = new Properties();
    
    // gui
    JTabbedPane tabbedPane = new JTabbedPane();
    
    JScrollPane scrollPane;

    AttributeGenerationPanel attributeGenerationPanel;
    ManualClassificationPanel manualClassificationPanel;
    TreeGenerationPanel treeGenerationPanel;
    
    static int sanityObject = 0;  // the object by which we are doing sanity learning on
    
    public static void main(String[] args) {
        if (args.length==1) {
            sanityObject = Integer.parseInt(args[0]);
        }
        System.out.println("Sanity Learning on object " + Constant.DEFAULT_BLOB_NAME[sanityObject] + " with index " + sanityObject);
        new MainGUI();
    }
    
    public MainGUI() {
        loadProperties();
        init();
    }
    
    void init() {
        attributeGenerationPanel  = new AttributeGenerationPanel(this);
        manualClassificationPanel = new ManualClassificationPanel(this);
        treeGenerationPanel       = new TreeGenerationPanel(this);

        tabbedPane.addTab(attributeGenerationPanel.getName(), null, attributeGenerationPanel, attributeGenerationPanel.getTip());
        tabbedPane.addTab(manualClassificationPanel.getName(), null, manualClassificationPanel, manualClassificationPanel.getTip());
        tabbedPane.addTab(treeGenerationPanel.getName(), null, treeGenerationPanel, treeGenerationPanel.getTip());
        tabbedPane.setSelectedIndex(0);
        tabbedPane.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
// execute load property of each panel to save frequent use property as fields
// and get a new copy everytime the pane changed
                if (tabbedPane.getSelectedIndex()==1) {
                    int startIndex, endIndex;
                    startIndex = Integer.parseInt(properties.getProperty(Constant.START_INDEX_KEY, Constant.DEFAULT_START_INDEX));
                    endIndex   = Integer.parseInt(properties.getProperty(Constant.END_INDEX_KEY, Constant.DEFAULT_END_INDEX));
                    manualClassificationPanel.loadFile(startIndex, endIndex, true);
                } else if (tabbedPane.getSelectedIndex()==2) {
                    treeGenerationPanel.update();
                }
            }
        });
        
        this.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                try {
                    properties.store(new FileOutputStream(new File(Constant.PROPERTY_FILE)), null);
                    System.out.println("Preferences saved to " + Constant.PROPERTY_FILE);
                } catch (IOException e2) {
                    e2.printStackTrace();
                }
                System.exit(0);
            }
        });

        scrollPane = new JScrollPane(tabbedPane);

        this.getContentPane().add(scrollPane, BorderLayout.CENTER);
        this.setSize(800, 600);

        this.setResizable(true);
        this.setTitle("Machine Learning on Sanity Checks");

        // centering the window
        Dimension frameSize, screenSize;
        screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        frameSize  = this.getSize();
        // avoid the differences become negative
        if (frameSize.height > screenSize.height) {
            frameSize.height = screenSize.height;
        }
        if (frameSize.width > screenSize.width) {
            frameSize.width = screenSize.width;
        }
        this.setLocation((screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2);
        this.setVisible(true);
    }

    // load the property file, create if does not exist
    void loadProperties() {
        File generatedFolder, propertyFile, folder;
        propertyFile = new File(Constant.PROPERTY_FILE);
        
        try {
            if (propertyFile.exists()) {
                if (propertyDebugMsg) {
                    System.out.println(Constant.PROPERTY_FILE + " exists");
                }
                properties.load(new FileInputStream(propertyFile));
            } else {
                if (propertyDebugMsg) {
                    System.out.println(Constant.PROPERTY_FILE + " does not exist");
                }
                // store the properties
                properties.setProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER);
                properties.setProperty(Constant.ATTRIBUTE_NAMES_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_NAMES_FILE);
                properties.setProperty(Constant.ATTRIBUTE_DATA_FILE_KEY, Constant.DEFAULT_ATTRIBUTE_DATA_FILE);
                properties.setProperty(Constant.START_INDEX_KEY, Constant.DEFAULT_START_INDEX);
                properties.setProperty(Constant.END_INDEX_KEY, Constant.DEFAULT_END_INDEX);
                properties.setProperty(Constant.LOG_FILE_KEY, Constant.DEFAULT_LOG_FILE);
                for (int i=0; i<Constant.IMAGE_FOLDER_KEY.length; i++) {
                    properties.setProperty(Constant.IMAGE_FOLDER_KEY[i], Constant.DEFAULT_IMAGE_FOLDER[i]);
                }
                for (int i=0; i<Constant.IMAGE_EXTENSION_KEY.length; i++) {
                    properties.setProperty(Constant.IMAGE_EXTENSION_KEY[i], Constant.DEFAULT_IMAGE_EXTENSION[i]);
                }
                for (int i=0; i<Constant.BLOB_NAME_KEY.length; i++) {
                    properties.setProperty(Constant.BLOB_NAME_KEY[i], Constant.DEFAULT_BLOB_NAME[i]);
                }
                properties.setProperty(Constant.NAME_LIST_FILE_KEY, Constant.DEFAULT_NAME_LIST_FILE);
                properties.setProperty(Constant.TREE_OUTPUT_EXTENSION_KEY, Constant.DEFAULT_TREE_OUTPUT_EXTENSION);
                properties.setProperty(Constant.DECISION_CODE_FILE_KEY, Constant.DEFAULT_DECISION_CODE_FILE);
                properties.store(new FileOutputStream(propertyFile), null);
            }
        } catch (IOException e) {
            Utility.printError(this, "Error in loading Properties");
//            e.printStackTrace();
        }

        // create the generated folder directory if does not exists. It stores all the generated files
        generatedFolder = new File(properties.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER));
        if (!generatedFolder.exists()) {
            generatedFolder.mkdirs();
        }

        // create the folders to store the generated files
        // image folders
        for (int i=0; i<Constant.IMAGE_FOLDER_KEY.length; i++) {
            if (folderDebugMsg) {
                System.out.print(getProperty(Constant.IMAGE_FOLDER_KEY[i], Constant.DEFAULT_IMAGE_FOLDER[i], true) + ": ");
            }
            folder = new File(getProperty(Constant.IMAGE_FOLDER_KEY[i], Constant.DEFAULT_IMAGE_FOLDER[i], true));
            if (folderDebugMsg) {
                System.out.println(folder.exists());
            }
            if (!folder.exists()) {
                folder.mkdir();
            }
        }
        // object folders
        for (int i=0; i<Constant.BLOB_NAME_KEY.length; i++) {
            if (folderDebugMsg) {
                System.out.print(getProperty(Constant.BLOB_NAME_KEY[i], Constant.DEFAULT_BLOB_NAME[i], true) + ": ");
            }
            folder = new File(getProperty(Constant.BLOB_NAME_KEY[i], Constant.DEFAULT_BLOB_NAME[i], true));
            if (folderDebugMsg) {
                System.out.println(folder.exists());
            }
            if (!folder.exists()) {
                folder.mkdir();
            }
        }

    }
    
    // update the properties given the key and new value. 
    // this methods will performs checking to see whether the value is really new
    void updateProperty(String key, String newValue) {
        if (!newValue.equals(properties.getProperty(key))) {
            properties.setProperty(key, newValue);
        }
    }
    
    String getProperty(String key, String defaultValue) {
        return getProperty(key, defaultValue, false);
    }
    String getProperty(String key, String defaultValue, boolean isFile) {
        String prefix = "";
        if (isFile) {
            prefix = properties.getProperty(Constant.GENERATED_FOLDER_KEY, Constant.DEFAULT_GENERATED_FOLDER) + File.separator;
        }
        return prefix + properties.getProperty(key, defaultValue);
    }
    
    void setSanityObject(int sanityObject) {
        this.sanityObject = sanityObject;
    }
    
    int getSanityObject() {
        return sanityObject;
    }
}
