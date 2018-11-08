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
 * $Id: DecisionTreeToCode.java 2212 2004-01-18 11:03:11Z tedwong $
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * Convert the c4.5 output to c code
 * Can handle sub tree, simplified decision tree (need to specified -s argument) and enum type with more than 2 values
 *
**/

//package learning;

import java.io.*;
import java.util.*;

class DecisionTreeToCode {

    public static final boolean printConditionDebugMsg = false;
    public static final boolean displayTreeDebugMsg    = false;
    public static final boolean insertSubTreeDebugMsg  = false;
    public static final boolean displaySubTreeDebugMsg = false;
    public static final boolean addBracketDebugMsg     = false;
    public static final boolean displayLineDebugMsg    = false;
    
    // keyword
    public static final String simplifiedKeyWord = "Simplified";  // key word to indicate the simplified decision tree
    public static final String mainTreeKeyWord   = "Tree:";       // key word to indicate the main tree is starting
    public static final String subTreeKeyWord    = "Subtree";     // key word to indicate the sub tree is starting
    public static final String endLine           = "Tree saved";
    
    public static final char GUARD    = '|';
    public static final char COLON    = ':';
    public static final String INDENT = "    ";
    
    // argument
    public static final String SIMPLIFIED  = "-s";
    public static final String OUTPUT_FILE = "-o";

    // change the equal sign from c4.5 to c
    public static final String C45_EQUAL = " = ";
    public static final String C_EQUAL   = " == ";

    Vector mainTree, subTreeList;
    
    public static void main(String[] args) {
        if (args.length<1) {
            System.out.println("Usage: java DecisionTreeToCode [OPTION] decisionTreeInputFile");
            System.out.println("where OPTION includes");
            System.out.println(SIMPLIFIED + ": use the simplified tree");
            System.out.println(OUTPUT_FILE + " outputFile: generate the output to this file, instead of standard output");
            System.exit(1);
        }
        String outputFilename = "";
        String inputFilename  = "output.dt";
        boolean isSimplified  = false;
        boolean hasOutput     = false;
        for (int i=0; i<args.length; i++) {
            if (args[i].charAt(0)=='-') {
                if (args[i].equals(SIMPLIFIED)) {
                    isSimplified = true;
                } else if (args[i++].equals(OUTPUT_FILE) && i<args.length) {
                    hasOutput      = true;
                    outputFilename = args[i];
                }
            } else {
                inputFilename = args[i];
            }
        }
        PrintWriter outputFile = null;
        try {
            if (hasOutput) {
                outputFile = new PrintWriter(new BufferedWriter(new FileWriter(outputFilename)));
            }
            (new DecisionTreeToCode()).convert(inputFilename, outputFile, isSimplified);
        } catch (IOException e) {
            System.out.println("DecisionTreeToCode.java: Error writing to file " + outputFilename);
            e.printStackTrace();
        }
    }

    boolean convert(String inputFilename, PrintWriter outputFile, boolean isSimplified) {
        if (!readTreeFile(inputFilename, outputFile, isSimplified)) {
            return false;
        }
        if (displayTreeDebugMsg) {
            displayTree(mainTree);
        }
        // main tree is changed
        if (!constructFullTree(mainTree, subTreeList)) {
            return false;
        }
        if (displayTreeDebugMsg) {
            System.out.println();
            displayTree(mainTree);
        }
        if (!convertTreeToCode(mainTree.elements(), outputFile)) {
            return false;
        }
        return true;
    }

    // read the c4.5 output and store in a vector    
    boolean readTreeFile(String inputFilename, PrintWriter outputFile, boolean isSimplified) {
        try {
            String line, output;
            Vector subTree;
            int openBracket;
            BufferedReader input = new BufferedReader(new FileReader(inputFilename));
            
            mainTree    = new Vector();
            subTreeList = new Vector();
            while (true) {
                line = input.readLine();
                if (line==null || line.indexOf(endLine)>-1) break;
                if (line.length()==0) continue;
                if ((!isSimplified && line.indexOf(mainTreeKeyWord)>-1 && line.indexOf(simplifiedKeyWord)==-1) || 
                    (isSimplified && line.indexOf(simplifiedKeyWord)>-1)) {
                    // skip the empty line
                    line = input.readLine();
                    openBracket = line.indexOf('(');
                    if (line.length()>0 && openBracket>-1) {
                        writeOutputln("return" + line.substring(0, openBracket-1) + ";", outputFile);
                    } else {
                        // read the main tree data
                        readTree(input, mainTree);
                    }
                } else if (line.indexOf(subTreeKeyWord)>-1) {
                    subTree = new Vector();
                    
                    // skip the empty line
                    input.readLine();
                    
                    // read the sub tree data
                    readTree(input, subTree);
                    if (displaySubTreeDebugMsg) {
                        System.out.println("Sub Tree");
                        displayTree(subTree);
                    }
                    subTreeList.addElement(subTree);
                }
            }
            return true;
        } catch (IOException e) {
            System.out.println("DecisionTreeToCode.java: Error opening file " + inputFilename);
            e.printStackTrace();
            return false;
        }
    }
    
    // reconstruct the full trees from main tree and the subTree list.
    // The mainTree will be changed
    boolean constructFullTree(Vector mainTree, Vector subTreeList) {
        Vector v;
        DecisionLine line, subTreeLine;
        Enumeration subTreeEnum = subTreeList.elements();
        for (int i=0; i<mainTree.size(); i++) {
            line = (DecisionLine) mainTree.elementAt(i);
            if (line.hasSubTree()) {
                v = (Vector) subTreeEnum.nextElement();
                if (i<mainTree.size()) {
                    if (insertSubTreeDebugMsg) {
                        System.out.println(i + " < " + mainTree.size());
                        displayTree(v);
                        System.out.println();
                    }
                    for (Enumeration e=v.elements(); e.hasMoreElements();) {
                        i++; // keep shifting one index
                        subTreeLine = (DecisionLine) e.nextElement();
                        subTreeLine.addToLine(line);
                        mainTree.add(i, subTreeLine);
                    }
                }
            }
        }
        return true;
    }
    
    // convert a vector of DecisionLine into code
    boolean convertTreeToCode(Enumeration e, PrintWriter outputFile) {
        if (e==null) {
            System.out.println("DecisionTreeToCode.java: Input enumeration is Null");
            return false;
        }
        String output;
        DecisionLine line, previousLine = null;
        while (e.hasMoreElements()) {
            line = (DecisionLine) e.nextElement();
            if (line.getCondition()==null) {
                writeOutputln(makeIndents(line.getIndentCount()) + "}", outputFile);
                continue;
            } else if (line.isIfStatement()) {
                // print the indention
                if ((previousLine!=null && !previousLine.isIfStatement() && previousLine.isDiscrete()) || 
                    (previousLine!=null && previousLine.isIfStatement())) {
                    writeOutput(makeIndents(line.getIndentCount()), outputFile);
                }
                // print the condition
                writeOutputln("if (" + line.getCondition() + ") {", outputFile);

                // print the output
                if (line.getOutput()!=null) {
                    writeOutputln(makeIndents(line.getIndentCount()+1) + "return " + line.getOutput() + ";", outputFile);
                }
            } else {
                writeOutput(makeIndents(line.getIndentCount()) + "} else ", outputFile);
                if (line.isDiscrete()) {
                    writeOutputln("if (" + line.getCondition() + ") {", outputFile);
                }
                if (line.getOutput()!=null) {
                    if (!line.isDiscrete()) {
                        writeOutputln("{", outputFile);
                    }
                    writeOutputln(makeIndents(line.getIndentCount()+1) + "return " + line.getOutput() + ";", outputFile);
                    if (!line.isDiscrete()) {
                        writeOutputln(makeIndents(line.getIndentCount()) + "}", outputFile);
                    }
                }
            }
            previousLine = line;
        }
        return true;
    }
    
    // read tree from input
    void readTree(BufferedReader input, Vector tree) throws IOException {
        DecisionLine currentLine, previousLine = null;
        String line;

        // to remind the list of unclosed brackets
        Vector indents = new Vector();
        Vector depths  = new Vector();
        while (true) {
            line = input.readLine();
            if (line==null || line.length()==0) {
                addCloseBracket(mainTree, indents, depths, null);
                break;
            }
            currentLine = new DecisionLine(line, previousLine);
            if (previousLine!=null && currentLine.getDepth() < previousLine.getDepth()) {
                addCloseBracket(mainTree, indents, depths, currentLine);
            }
            // only add if this depth is greater than the last element in the vector
            if (currentLine.isDiscrete() && (depths.size()==0 || depths.size()>0 && currentLine.getDepth()>((Integer)depths.elementAt(depths.size()-1)).intValue())) {
                depths.addElement(new Integer(currentLine.getDepth()));
                indents.addElement(new Integer(currentLine.getIndentCount()));
                if (addBracketDebugMsg) {
                    System.out.println("depth: " + currentLine.getDepth() + "; indentCount: " + currentLine.getIndentCount());
                }
            }
            tree.addElement(currentLine);
            if (displayLineDebugMsg) {
                System.out.println(currentLine);
            }
            previousLine = currentLine;
        }
    }
    
    // add the close brackets
    void addCloseBracket(Vector tree, Vector indents, Vector depths, DecisionLine currentLine) {
        int depth, indentCount;
        for (int i=(indents.size()-1); i>=0; i--) {
            indentCount = ((Integer) indents.elementAt(i)).intValue();
            depth       = ((Integer) depths.elementAt(i)).intValue();
            if (currentLine!=null && depth<=currentLine.getDepth()) {
                break;
            }
            indents.remove(i);
            depths.remove(i);

            // add the close brackets line in
            tree.addElement(new DecisionLine(depth, indentCount));
            if (currentLine!=null) {
                currentLine.setIndentCount(indentCount-1);
            }
        }
    }
    
    //////////////////
    // Help methods //
    //////////////////
    
    // return the indentation
    static String makeIndents(int indentCount) {
        if (indentCount<0) {
            System.out.println("DecisionTreeToCode.java: Negative indent count");
        }
        String s = "";
        for (int i=0; i<indentCount; i++) {
            s += INDENT;
        }
        return s;
    }
    
    void displayTree(Vector mainTree) {
        for (Enumeration e=mainTree.elements(); e.hasMoreElements();) {
            System.out.println((DecisionLine) e.nextElement());
        }
    }
    
    private void writeOutputln(String s, PrintWriter outputFile) {
        writeOutput(s + "\n", outputFile);
    }
    private void writeOutput(String s, PrintWriter outputFile) {
        if (outputFile==null) {
            System.out.print(s);
        } else {
            outputFile.print(s);
            outputFile.flush();
        }
    }
    
    
    // a class that represent each line in the c4.5 output
    private class DecisionLine {
    
        private int depth           = 0;
        private int indentCount     = 0;
        private String condition    = "";
        private String output       = null;
        private boolean subTree     = false;
        private boolean discrete    = false; // whether the type is discrete, using equals
        private boolean ifStatement = true;
        
        DecisionLine(int depth, int indentCount) {
            this.indentCount = indentCount;
            this.condition   = null;
            this.depth       = depth;
        }
        
        DecisionLine(String line, DecisionLine previous) {
            // set the depth
            int i = setDepth(line);
            
            // whether this line is the if statement
            if (previous!=null) {
                this.ifStatement = this.depth > previous.getDepth();
            }
            
            // set the condition
            int colonPos = line.indexOf(DecisionTreeToCode.COLON);
            if (colonPos==-1) {
                System.out.println("DecisionTreeToCode.java: Invalid line. No colonPos. Line = \"" + line + "\"");
            }
            this.condition = line.substring(i, colonPos).trim();
            if (DecisionTreeToCode.printConditionDebugMsg) {
                System.out.print("condition: " + this.condition + "; " + this.condition.indexOf(" = ") + "; ");
            }
            
            // the equals used in C/C++
            int equalPos = this.condition.indexOf(DecisionTreeToCode.C45_EQUAL);
            if (equalPos>-1) {
                this.discrete  = true;
                this.condition = this.condition.substring(0, equalPos) + DecisionTreeToCode.C_EQUAL + this.condition.substring(equalPos+C45_EQUAL.length());
            }
            if (DecisionTreeToCode.printConditionDebugMsg) {
                System.out.println(this.condition);
            }
            
            // set the number of indention
            if (previous!=null) {
                this.indentCount = previous.getIndentCount();
                // 8 combinations (previous line discrete/continuous, if/else statement, current line discrete/continuous) to increment the indention
                // the only 2 exception are previous continuous else current discrete and previous continuous else current continuous
                if (this.depth>previous.getDepth() && (previous.isDiscrete() || previous.isIfStatement())) {
                    // must indent if both the current and previous line is if statement
                    this.indentCount++;
                } else if (this.depth<previous.getDepth() && this.indentCount>0) {
                    this.indentCount--;
                }
            }

            // whether it has sub tree
            if (line.charAt(line.length()-1)==']') {
                this.subTree = true;
            } else {
                // set the output
                int lastOpenBracketPos = line.lastIndexOf('(');
                if (lastOpenBracketPos > colonPos) {
                    this.output = line.substring(colonPos+2, lastOpenBracketPos-1);
                }
            }
        }
        
        int setDepth(String line) {
            // find the depth
            int i;
            char c;
            for (i=0; i<line.length(); i++) {
                c = line.charAt(i);
                if (c==GUARD) {
                    this.depth++;
                } else if (!Character.isWhitespace(c)) {
                    // no more GUARD as soon as the character is not a white space
                    break;
                }
            }
            return i;
        }
        
        void setIfStatement(boolean b) {
            this.ifStatement = b;
        }
        
        boolean isIfStatement() {
            return this.ifStatement;
        }
        
        boolean hasSubTree() {
            return this.subTree;
        }
        
        boolean isDiscrete() {
            return this.discrete;
        }
        
        String getCondition() {
            return this.condition;
        }
        
        String getOutput() {
            return this.output;
        }
        
        int getDepth() {
            return this.depth;
        }
        
        void setIndentCount(int indentCount) {
            this.indentCount = indentCount;
        }
        
        int getIndentCount() {
            return this.indentCount;
        }
        
        // add the main tree depth to this sub tree depth
        void addToLine(DecisionLine line) {
            this.depth       += 1 + line.getDepth();
            this.indentCount += line.getIndentCount();
            if (line.ifStatement) {
                this.indentCount++;
            }
        }
        
        public String toString() {
            String s = "";
            s += DecisionTreeToCode.makeIndents(this.indentCount);
            s += "[" + this.depth + "] ";
            if (this.ifStatement) {
                s += "If (" + this.condition + ") ";
            } else {
                s += "else [" + this.condition + "] ";
            }
            s += this.output + " ";
            if (this.subTree) {
                s += "hasSubTree";
            }
            return s;
        }
        
    }
}
