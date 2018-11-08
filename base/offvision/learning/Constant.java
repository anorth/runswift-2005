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
 * $Id: Constant.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * the constants used in the other java files
 *
**/

//package learning;

import java.io.*;
import java.util.*;

class Constant {

    public static final String VISUAL_CORTEX      = ".." + File.separator + ".." + File.separator + "robot" + File.separator + "vision" + File.separator + "VisualCortex.cc";
    public static final String TEMP_VISUAL_CORTEX = VISUAL_CORTEX + ".temp";

    // constants for the property
    // attribute generation panel
    public static final String PROPERTY_FILE                 = System.getProperty("user.dir", ".") + File.separator + "user.property";
    public static final String GENERATED_FOLDER_KEY          = "generatedFolder";
    public static final String DEFAULT_GENERATED_FOLDER      = System.getProperty("user.dir", ".") + File.separator + "generated";
    public static final String ATTRIBUTE_NAMES_FILE_KEY      = "attributeNamesFile";
    public static final String DEFAULT_ATTRIBUTE_NAMES_FILE  = "NamesAttributes.txt"; // generated
    public static final String ATTRIBUTE_DATA_FILE_KEY       = "attributeDataFile";
    public static final String DEFAULT_ATTRIBUTE_DATA_FILE   = "DataAttributes.txt"; // generated
    public static final String START_INDEX_KEY               = "startIndex";
    public static final String DEFAULT_START_INDEX           = "1";
    public static final String END_INDEX_KEY                 = "endIndex";
    public static final String DEFAULT_END_INDEX             = "10";
    public static final String LOG_FILE_KEY                  = "logFile";
    public static final String DEFAULT_LOG_FILE              = System.getProperty("user.dir", ".") + File.separator + "CPLANE_LOG";
    // directories for the frames
    public static final String[] IMAGE_FOLDER_KEY            = {"frameFolder", "objectFolder", "blobFolder"};
    public static final String[] IMAGE_FOLDER_LABEL          = {"Frame", "Object", "Blob"};
    public static final String[] DEFAULT_IMAGE_FOLDER        = {"frames", "objects", "blobs"}; // contain generated files
    public static final String[] IMAGE_EXTENSION_KEY         = {"frameExtension", "objectExtension", "blobExtension"};
    public static final String[] IMAGE_EXTENSION_LABEL       = {"Frame Extension", "Object Extension", "Blob Extension"};
    public static final String[] DEFAULT_IMAGE_EXTENSION     = {"f", "o", "b"};
    public static final int FRAME_INDEX                      = 0;
    public static final int OBJECT_INDEX                     = 1;
    public static final int BLOB_INDEX                       = 2;
    public static final String[] BLOB_NAME_KEY = {
        "ballFolder", 
        "blueGoalFolder", "yellowGoalFolder",
        "blueTopPinkFolder", "pinkTopBlueFolder", 
        "greenTopPinkFolder", "pinkTopGreenFolder", 
        "yellowTopPinkFolder", "pinkTopYellowFolder",
        "redDogFolder", "blueDogFolder"
    };
    public static final String[] BLOB_NAME_LABEL = {
        "ball", 
        "blueGoal", "yellowGoal",
        "blueTopPink", "pinkTopBlue", 
        "greenTopPink", "pinkTopGreen", 
        "yellowTopPink", "pinkTopYellow",
        "redDog", "blueDog"
    };
    // this should not be changed as this will be the labels used in VisualCortex.cc
    public static final String[] DEFAULT_BLOB_NAME = {
        "ball", 
        "blueGoal", "yellowGoal",
        "blueTopPink", "pinkTopBlue", 
        "greenTopPink", "pinkTopGreen", 
        "yellowTopPink", "pinkTopYellow",
        "redDog", "blueDog"
    };
    public static final String NAME_LIST_FILE_KEY            = "namelist";
    public static final String DEFAULT_NAME_LIST_FILE        = "namelist.txt"; // generated
    // tree generation panel
    public static final String TREE_OUTPUT_EXTENSION_KEY     = "treeOutputExtension";
    public static final String DEFAULT_TREE_OUTPUT_EXTENSION = "dt";
    public static final String DECISION_CODE_FILE_KEY        = "decisionCodeFile";
    public static final String DEFAULT_DECISION_CODE_FILE    = "DecisionCode.cc";
    
    // temp prefix variable label
    public static final String TEMP_LABEL = "$TEMP";
    
    // target to look for in VisualCortex.cc
    public static final String ATTRIBUTE_NAMES_CODE_TARGET = "machine learning names attributes generation code";
    public static final String ATTRIBUTE_DATA_CODE_TARGET  = "machine learning data attributes generation code";
    public static final String DECISION_TARGET             = "decision code"; // will combine with the object, eg DEFAULT_BLOB_NAME[selectedIndex] + " " + DECISION_TARGET
    public static final String NAMES_TARGET                = "names"; // will combine with the object, eg DEFAULT_BLOB_NAME[selectedIndex] + " " + NAMES_TARGET
    public static final String DATA_TARGET                 = "data"; // will combine with the object, eg DEFAULT_BLOB_NAME[selectedIndex] + " " + DATA_TARGET
    public static final String BLOB_OUTPUT_TARGET          = "(*blobOutput) << "; // this is the output of blob for c4.5. Do not need this in the testing code.

    // c4.5 file extension
    public static final String C4_5_DATA_EXT  = "data";
    public static final String C4_5_NAMES_EXT = "names";
    
    // shell commands
    public static final String CLEAN_COMMAND        = "make clean";
    public static final String COMPILE_COMMAND      = "make offlinevision";
    public static final String OFFVISION_COMMAND    = "offvision";
    public static final String C4_5_COMMAND         = "c4.5";
    public static final String C4_5_COMMAND_ARGU    = "-f";
    public static final String JAVA_COMMAND         = "java";

    // options
    public static final Object[] OK_CANCEL_OPTIONS = {"OK", "Cancel"};

    // names for object bounding box
    public static final String[] detailObjectNames = {
        "Ball", 
        "Blue Goal", "Yellow Goal",
        "Blue(top)/Pink", "Pink(top)/Blue", 
        "Green(top)/Pink", "Pink(top)/Green", 
        "Yellow(top)/Pink", "Pink(top)/Yellow",
        "Red Dog1", "Red Dog2", "Red Dog3", "Red Dog4", 
        "Blue Dog", "Blue Dog2", "Blue Dog3", "Blue Dog4"
    };

}
