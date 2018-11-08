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
 * UNSW 2001 Robocup
 *
 * Last modification background information
 * $Id: 
 *
 * Copyright (c) 2003 UNSW
 * All Rights Reserved.
 *
 * convert the c4.5 output to c code
 *
**/

import java.io.*;
import java.util.*;

public class DecisionTreeToCodeYUV {

    public static final boolean dumpMainTreeDebugMsg    = false;
    public static final boolean dumpSubTreeListDebugMsg = false;
    public static final boolean dumpFullTreeDebugMsg    = false;
    public static final boolean displayTreeDebugMsg     = false;

    // the line in the inputfile where the decision starts
    private static final String treeStart = "Simplified Decision Tree:";
    
    // constants do with indention
    private static final int SPACE           = 1;
    private static final int TAB             = 2;
    private static final String INDENT_SPACE = "    ";
    private static final String INDENT_TAB   = "\t";
    
    private int indentionMethod = SPACE;
        
    public DecisionTreeToCodeYUV(String filename, int depth) {
        BufferedReader in;
        Vector mainTree;
        Vector subTreeList;
        Vector fullTree;
        
        try {
            in = new BufferedReader(new FileReader(filename));
            
            while (true) {
                String s = in.readLine();
                if (s.equals(treeStart)) {
                    // skip one empty line before mainTree starts
                    in.readLine();
                    break;
                }
            }
            
            mainTree = readMainTree(in);
            if (dumpMainTreeDebugMsg) {
                dumpMainTree(mainTree);
            }
            
            subTreeList = readSubTrees(in);
            if (dumpSubTreeListDebugMsg) {
                dumpSubTreeList(subTreeList);
            }
            
            fullTree = constructFullTree(mainTree, subTreeList);
            if (dumpFullTreeDebugMsg) {
                dumpMainTree(fullTree);
            }
            
            if (displayTreeDebugMsg) {
                displayTree(fullTree);
            }
            outputSource(fullTree, depth);
        } catch (Exception e) {
            System.out.println(e.getMessage());
            System.out.println("DecisionTreeToCodeYUV.java: Error in opening file " + filename);
            e.printStackTrace();
            System.exit(0);
        }
    }
    
    private int findDepth(String s) {
        StringTokenizer st = new StringTokenizer(s);
        String token;
        int depth = 0;
        
        int count = st.countTokens();
        for (int j = 0; j < count; j++) {
            token = st.nextToken();
            if (token.equals("|")) {
                depth++;
            } else {
                break;
            }
        }
        
        return depth;
    }
    
    private boolean isLeaf(String s, int depth) {
        return getColor(s, depth) == null;
        
    }
    
    private String getColor(String s, int depth) {
        String color = null;
        StringTokenizer st = new StringTokenizer(s);
        
        for (int i = 0; i < depth + 4; i++) {
            st.nextToken();
        }
        
        try {
            color = st.nextToken();
        } catch (NoSuchElementException e) {
            color = null;    
        }
    
        return color;
        
    }
    
    
    private String getCondition(String s, int depth) {
        StringTokenizer st = new StringTokenizer(s);
        
        for (int i = 0; i < depth; i++) {
            st.nextToken();
        }
        
        return st.nextToken() + " " + st.nextToken() + " " + st.nextToken();
    }
    
    
    private void outputSource(Vector tree, int depth) {
        String s;
        int if_line = -1;
        int else_line = -1;
        
        for (int i = 0; i < tree.size(); i++) {
            s = (String) tree.elementAt(i);
            
            if (findDepth(s) == depth) {
                
                if (if_line == -1) {    
                    if_line = i;
                } else if (else_line == -1) {
                    else_line = i;
                } else {
                    System.out.println("Error: invalid tree");
                    System.exit(1);
                }
                
            }
        }
        
        String if_string = (String) tree.elementAt(if_line);
        String else_string = (String) tree.elementAt(else_line);
        String condition = getCondition(if_string, depth);
        String if_color = getColor(if_string, depth);
        String else_color = getColor(else_string, depth);
        String tabs = makeIndention(depth);
        
        
        System.out.println(tabs + "if (" + condition + ")");
        System.out.println(tabs + "{");
        if (if_color != null) {
            System.out.println(tabs + "\treturn " + if_color + ";");
        } else {
            // recursive    
            Vector sub_tree = new Vector();
            for (int i = if_line + 1; i < else_line; i++) {
                sub_tree.addElement(tree.elementAt(i));
            }
            
            outputSource (sub_tree, depth + 1);
            
        }
        System.out.println(tabs + "}");
        
        
        System.out.println(tabs + "else");
        System.out.println(tabs + "{");
        if (else_color != null) {
            System.out.println(tabs + "\treturn " + else_color + ";");
        } else {
            // recursive    
            Vector sub_tree = new Vector();
            for (int i = else_line + 1; i < tree.size(); i++) {
                sub_tree.addElement(tree.elementAt(i));
            }
            
            outputSource (sub_tree, depth + 1);
            
        }
        System.out.println(tabs + "}");
    }
    
    public boolean isNewSubTree(String s) {
        StringTokenizer st = new StringTokenizer(s);
        if (st.countTokens() > 0) {
            return st.nextToken().equals("Subtree");
        }
        return false;
    }
    
    
    public Vector readSubTrees(BufferedReader in) {
        Vector subTreeList = new Vector();
        Vector subTree;
        String s;
        
        String subTreeTag = "Subtree";
        String subTreeEnd = "Tree saved";
        
        while (true) {
            try {
                s = in.readLine();
                if (s.equals(subTreeEnd)) {
                    break;
                } else if (isNewSubTree(s)) {
                    in.readLine();
                    subTree = new Vector();
                    
                    while (true) {
                        s = in.readLine();
                        if (s.length() == 0) {
                            break;
                        }
                        subTree.addElement(s);
                    }
                    subTreeList.addElement(subTree);
                }
            } catch (Exception e) {
                System.out.println("Error: reading file");
                e.printStackTrace();
                System.exit(1);
            }
        }
        return subTreeList;
    }
    
    public Vector readMainTree(BufferedReader in) {
        Vector mainTree = new Vector();
        String s;
        
        while (true) {
            try {
                s = in.readLine();
                if (s == null || s.length() == 0) {
                    break;
                }
                
                mainTree.addElement(s);
            } catch (IOException e) {
                System.out.println("Error: reading file");
                e.printStackTrace();
                System.exit(1);
            }
        }
        return mainTree;
    }
    
    public boolean isExtendSubTree(String s) {
        StringTokenizer st = new StringTokenizer(s);
        int count = st.countTokens();
        
        for (int i = 0; i < count-1; i++) {
            st.nextToken();
        }
        
        try {
            String laststr = st.nextToken();
            if (laststr.length() < 3)
                return false;
            String temp = laststr.substring(0, 3);
            return temp.equals(":[S");
        } catch (StringIndexOutOfBoundsException e) {
            e.printStackTrace();
            return false;
            
        }
    }
    
    
    public String reconstructBranch(String s, int depth) {
        StringTokenizer st = new StringTokenizer(s);
        String newBranch = "";
        int count = st.countTokens();
        
        for (int i = 0; i < depth; i++) {
            newBranch = newBranch + st.nextToken() + "   ";
        }
        
        for (int i = depth; i < count-1; i++) {
            newBranch = newBranch + st.nextToken() + " ";
        }
        return newBranch + ":";
    }
    
    private String makeBars(int i) {
        String bars = new String("");
        for (int j = 0; j < i; j++) {
            bars = bars + "|   ";
        }
    
        return bars;
    }
    
    public Vector constructFullTree(Vector mainTree, Vector subTreeList) {
        Vector fullTree = new Vector();
        int subTreeIndex = 0;
        
        
        for (int i = 0; i < mainTree.size(); i++) {
            String s = (String) mainTree.elementAt(i);
            
            if (isExtendSubTree(s)) {
                //System.out.println(s);
                int depth = findDepth(s);
                String newBranch = reconstructBranch(s, depth);
                //System.out.println(newBranch);
                fullTree.addElement(newBranch);
                
                /* adding subtree */
                Vector subTree = (Vector) subTreeList.elementAt(subTreeIndex);
                
                for (int j = 0; j < subTree.size(); j++) {
                    String bars = makeBars(depth+1);
                    fullTree.addElement(bars + "" + subTree.elementAt(j));
                }
                subTreeIndex++;
            } else {
                fullTree.addElement(s);
            }
        }
        return fullTree;
    }
    
    
    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Usage: java DecisionTreeToCodeYUV treefile depth");
            System.exit(0);
        }
        new DecisionTreeToCodeYUV(args[0], Integer.parseInt(args[1]));
    }
    
    ////////////////////
    // Helper methods //
    ////////////////////

    // make the indention to align the output code
    private String makeIndention(int n) {
        String length;
        if (indentionMethod==TAB) {
            length = INDENT_TAB;
        } else {
            length = INDENT_SPACE;
        }
        String indents = "";
        for (int j=0; j<n; j++) {
            indents += length;
        }
        return indents;
    }
    
    
    ///////////////////
    // debug methods //
    ///////////////////
    private void displayTree(Vector tree) {
        for (Enumeration e=tree.elements(); e.hasMoreElements();) {
            System.out.println((String)e.nextElement());
        }
    }
    
    private void dumpSubTreeList(Vector subTreeList) {
        for (int i = 0; i < subTreeList.size(); i++) {
            Vector subTree = (Vector) subTreeList.elementAt(i);
            System.out.println("SubTree " + (i+1) + ": ");
            for (int j = 0; j < subTree.size(); j++) {
                System.out.println(subTree.elementAt(j));
            }
        }
    }
    
    
    private void dumpMainTree(Vector mainTree) {
        System.out.println("Main Tree: ");
        for (int i = 0; i < mainTree.size(); i++) {
            System.out.println(mainTree.elementAt(i));
        }
    }
    
    
}
