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
 * @version 1.1
 * CHANGE LOG :
 * + version 1.1:
 *     - change undo interface to return valid/invalid state
 *     - add loadFromFile(String) interface
 */

abstract public class GenericRDRManager {
    protected GenericExample allExamples;

    protected String fileName;

    protected void addExample(GenericExample e) {
        e.other_examples = allExamples;
        allExamples = e;
    }

    public GenericRDRRule getRule(){return null;} ;

    public boolean undo() {
        if (allExamples != null) {
            GenericExample last_example = allExamples;
            allExamples = allExamples.other_examples;

            if (last_example != null && last_example.rule != null) {
                last_example.rule.undo();
                return true;
            }
        }
        return false;
    }

    public void loadFromFile(String fileName) {
        System.out.println("Error : Subclass doesnot implement this function.");
    }

    public void print(){
        getRule().print(0);
        System.out.println("Number of nodes " + getRule().getNumberOfNodes());
        System.out.println("Number of examples " + getNumberCornerstones());
    }

    public int getNumberCornerstones() {
        int countExample = 0;
        for (GenericExample e = allExamples; e != null ; e = e.other_examples) countExample++;
        return countExample;
    }

    abstract public int getNumberOfMetrics();
}
