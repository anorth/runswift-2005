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
 * $Id: Utility.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * utility functions that are independent of any class
 *
**/

//package learning;

import java.io.*;
import javax.swing.*;

class Utility {

    static final boolean printStackDebugMsg = false;

    // convert a object index to a file index
    public static int objectToFile(int objectIndex) {
        switch (objectIndex) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                return objectIndex;
            case 9:
            case 10:
            case 11:
            case 12:
                return 9;
            case 13:
            case 14:
            case 15:
            case 16:
                return 10;
            default:
                System.out.println("Utility.java: Invalid objectIndex " + objectIndex);
                return -1;
        }
    }
    
    public static void printError(Object o, String msg) {
        if (o!=null) {
            System.out.println(o.getClass().getName() + ": " + msg);
        } else {
            System.out.println("Utility.java: " + msg);
        }
    }
    
    // return true iff the input is a valid string
    public static boolean validString(String filename) {
        if (filename==null || filename.trim().length()==0) {
            return false;
        }
        return true;
    }

    // print out the attributes specified in the VisualCortex.cc
    public static void filterAttribute(String namesFilename, String namesTarget, String dataFilename, String dataTarget) {
        boolean namesValue = filterNamesAttribute(namesFilename, namesTarget);
        boolean dataValue  = filterDataAttribute(dataFilename, dataTarget);
        if (namesValue && dataValue) {
            JOptionPane.showMessageDialog(null, "Attributes generated successfully", "Information", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    public static boolean filterNamesAttribute(String namesFilename, String namesTarget) {
        try {
            PrintWriter namesOutputFile = new PrintWriter(new BufferedWriter(new FileWriter(namesFilename)));
            process(namesOutputFile, namesTarget);
            namesOutputFile.close();
            return true;
        } catch (IOException e) {
            JOptionPane.showMessageDialog(null, "Error in writing to " + namesFilename + " or\nopening file " + Constant.VISUAL_CORTEX, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                e.printStackTrace();
            }
        }
        return false;
    }

    public static boolean filterDataAttribute(String dataFilename, String dataTarget) {
        try {
            PrintWriter dataOutputFile = new PrintWriter(new BufferedWriter(new FileWriter(dataFilename)));
            process(dataOutputFile, dataTarget);
            dataOutputFile.close();
            return true;
        } catch (IOException e) {
            JOptionPane.showMessageDialog(null, "Error in writing to " + dataFilename + " or\nopening file " + Constant.VISUAL_CORTEX, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                e.printStackTrace();
            }
        }
        return false;
    }
    
    // helper function for filterAttribute. It writes all the line starts with prefix to the ouptutFile
    private static void process(PrintWriter outputFile, String type) throws IOException {
        String prefix    = "// c4.5 " + type + " ";
        int prefixLength = prefix.length();
        
        // print the section
        printSection(outputFile, type);
        BufferedReader input = new BufferedReader(new FileReader(Constant.VISUAL_CORTEX));
        String line;
        while (true) {
            line = input.readLine();
            if (line==null) {
                break;
            }
            line = trimFront(line);
            if (line.length()>=prefixLength && line.substring(0, prefixLength).equals(prefix)) {
                outputFile.println(line.substring(prefixLength));
                outputFile.flush();
            }
        }
    }

    // helper function for filterAttribute. It prints the section heading    
    private static void printSection(PrintWriter outputFile, String type) throws IOException {
        String title = type + " for c4.5";
        for (int i=0; i<(title.length()+3*2); i++) {
            outputFile.print('/');
        }
        outputFile.println();
        outputFile.println("// " + title + " //");
        for (int i=0; i<(title.length()+3*2); i++) {
            outputFile.print('/');
        }
        outputFile.println();
    }
    
    // helper function for filterAttribute. It returns the string with the front of the space trimmed
    private static String trimFront(String line) {
        int index = 0;
        while (index<line.length() && Character.isWhitespace(line.charAt(index))) {
            index++;
        }
        return line.substring(index);
    }
    
    // pop up a file chooser when the button is clicked
    // return the path of the selected directory/file, null otherwise
    public static String chooseButtonActionListener(String currentDir, int mode) {
        // start the file chooser in the directory of the specified file
        JFileChooser chooser = new JFileChooser(currentDir);
        chooser.setFileSelectionMode(mode);
        int returnVal = chooser.showOpenDialog(null);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            return chooser.getSelectedFile().getPath();
        }
        return null;
    }
    
    // copy the content of 'filename' to VisualCortex starting after the 'target'
    // @param writeBlobOutput    if true, the new file will also contain *blobOutput
    public static void updateVisualCortex(String target, String filename, boolean writeBlobOutput) {
        try {
            String line;
            BufferedReader originalFile, attributeFile;
            PrintWriter newFile;
            int blobOutputIndex;
            originalFile  = new BufferedReader(new FileReader(Constant.VISUAL_CORTEX));
            attributeFile = new BufferedReader(new FileReader(filename));
            newFile       = new PrintWriter(new BufferedWriter(new FileWriter(Constant.TEMP_VISUAL_CORTEX)));
            
            // keep writing originalFile to newFile until the line consists of 'target'
            while (true) {
                line = originalFile.readLine();
                if (line==null) {
                    printError(null, "No '" + target + "' found in file " + Constant.VISUAL_CORTEX);
                    
                    // close the files
                    newFile.close();
                    attributeFile.close();
                    originalFile.close();
                    
                    // moved the temp file to VisualCortex.cc
                    File newVisualCortex = new File(Constant.TEMP_VISUAL_CORTEX);
                    newVisualCortex.delete();
                    return;
                }
                if (line.indexOf(target)==-1) {
                    newFile.println(line);
                    newFile.flush();
                } else {
                    // break after writing this target line
                    newFile.println(line);
                    newFile.flush();
                    break;
                }
            }
            
            // an empty line afterwards
            newFile.println();
            newFile.flush();
            
            // keep writing attributeFile to newFile
            while ((line=attributeFile.readLine())!=null) {
                blobOutputIndex = line.indexOf(Constant.BLOB_OUTPUT_TARGET);

                // if the target is not found
                if (blobOutputIndex==-1) {
                    newFile.println(line);
                    newFile.flush();
                } else {
                    // if the target is found
                    if (writeBlobOutput) {
                        // we choose to write it
                        newFile.println(line);
                        newFile.flush();
                    } else {
                        // if we choose not to write it, then comment it
                        newFile.println("//" + line);
                        newFile.flush();
                    }
                }
            }
            attributeFile.close();
            
            // an empty line afterwards
            newFile.println();
            newFile.flush();
            
            // skip originalFile until 'target' is reached again
            while ((line=originalFile.readLine())!=null) {
                if (line.indexOf(target)>-1) {
                    // write the target in as well
                    newFile.println(line);
                    newFile.flush();
                    break;
                }
            }

            // keep writing the rest of originalFile to newFile
            while ((line=originalFile.readLine())!=null) {
                newFile.println(line);
                newFile.flush();
            }
            newFile.close();
            originalFile.close();

            // rename newFile to the original name
            File newVisualCortex      = new File(Constant.TEMP_VISUAL_CORTEX);
            File originalVisualCortex = new File(Constant.VISUAL_CORTEX);
            if (newVisualCortex.renameTo(originalVisualCortex)) {
                JOptionPane.showMessageDialog(null, "Code update to target '" + target + "' completed", "Information", JOptionPane.INFORMATION_MESSAGE);
            }
        } catch (IOException e) {
            JOptionPane.showMessageDialog(null, "Error when opening file " + Constant.VISUAL_CORTEX + " or " + filename + " or\nwriting to " +  Constant.TEMP_VISUAL_CORTEX, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                printError(null, "Error when opening file " + Constant.VISUAL_CORTEX + " or " + filename + " or writing to " +  Constant.TEMP_VISUAL_CORTEX);
                e.printStackTrace();
            }
        }
    }
    
    public static void runShellCommand(String command) {
        try {
            String line;
            Process process = Runtime.getRuntime().exec(command);

            // show the output
            BufferedReader processOutput = new BufferedReader(new InputStreamReader(process.getInputStream()));
            while ((line=processOutput.readLine()) != null) {
                System.out.println(line);
            }
            processOutput.close();

            // error from the exit value
            if (process.waitFor()!=0) {
                // show the error
                BufferedReader processError = new BufferedReader(new InputStreamReader(process.getErrorStream()));
                while ((line=processError.readLine()) != null) {
                    Utility.printError(null, line);
                }
                processError.close();
            } else {
                if (command.length()>70) {
                    JOptionPane.showMessageDialog(null, "Command '" + command.substring(0, 70) + "...' run successfully", "Information", JOptionPane.INFORMATION_MESSAGE);
                } else {
                    JOptionPane.showMessageDialog(null, "Command '" + command + "' run successfully", "Information", JOptionPane.INFORMATION_MESSAGE);
                }
            }
            System.out.println("Command '" + command + "' finished");
        } catch (InterruptedException ie) {
            JOptionPane.showMessageDialog(null, "Interrupted Error in running shell command: " + command, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                Utility.printError(null, "Interrupted Error in running shell command: " + command);
                ie.printStackTrace();
            }
        } catch (IOException ioe) {
            JOptionPane.showMessageDialog(null, "I/O Error in running shell command: " + command, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                Utility.printError(null, "I/O Error in running shell command: " + command);
                ioe.printStackTrace();
            }
        }
    }

    public static void runShellCommand(String command, PrintWriter output) {
        try {
            String line;
            BufferedReader processOutput;
            Process process = Runtime.getRuntime().exec(command);

            // save the output in a file
            processOutput = new BufferedReader(new InputStreamReader(process.getInputStream()));
            while ((line=processOutput.readLine()) != null) {
                output.println(line);
                output.flush();
            }
            output.close();
            processOutput.close();

            // error from the exit value
            if (process.waitFor()!=0) {
                // show the error
                BufferedReader processError = new BufferedReader(new InputStreamReader(process.getErrorStream()));
                while ((line=processError.readLine()) != null) {
                    Utility.printError(null, line);
                }
                processError.close();
            } else {
                JOptionPane.showMessageDialog(null, "Command '" + command + "' run successfully", "Information", JOptionPane.INFORMATION_MESSAGE);
            }
            System.out.println("Command '" + command + "' finished");
        } catch (InterruptedException ie) {
            JOptionPane.showMessageDialog(null, "Interrupted Error in running shell command: " + command, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                Utility.printError(null, "Interrupted Error in running shell command: " + command);
                ie.printStackTrace();
            }
        } catch (IOException ioe) {
            JOptionPane.showMessageDialog(null, "I/O Error in running shell command: " + command, "Error", JOptionPane.ERROR_MESSAGE);
            if (printStackDebugMsg) {
                Utility.printError(null, "I/O Error in running shell command: " + command);
                ioe.printStackTrace();
            }
        }
    }

}
