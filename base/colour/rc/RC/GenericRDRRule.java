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
 * CHANGE LOG
 * + version 1.1:
 *      - Refactor to have most of RDR algorithm template here. leave satisfies, try_add, copyRDRRule abstract
 */

public abstract class GenericRDRRule {
    private static final boolean debug = false;

    public static final int UNCHANGED = 0;
    public static final int MODIFIED = 1;
    public static final int EXTENDED = 2;
    public static final int NEW_EXCEPTION = 3;
    public static final int NEW_ALTERNATIVE = 4;

    public static final String [] STATUS_TO_STRING = {
        "Unchanged",
        "Modified",
        "Extended",
        "New Exception",
        "New Alternative"
    };
    //class members
    GenericRDRRule exception;
    GenericRDRRule alternative;
    GenericRDRManager rdrManager;
    GenericRDRRule parent;
    GenericExample cornerstone;
    GenericRDRRule history;
    int C;

    //abtract methods
    abstract public boolean satisfies(GenericExample e);
    abstract protected boolean try_add(GenericExample e);
    protected void copyRDRRule(GenericRDRRule copy){
        C = copy.C;
        exception = copy.exception;
        alternative = copy.alternative;
        rdrManager = copy.rdrManager;
        history = copy.history;
        cornerstone = copy.cornerstone;
        parent = copy.parent;
    }


    abstract public GenericRDRRule clonedRule();
    abstract public GenericRDRRule clonedRuleEmpty();

    //implemented methods

    public void initialize(){
        exception = null;
        alternative = null;
        rdrManager = null;
        parent = null;
        cornerstone = null;
        history = null;
        C = 0;
    }

    public void initialize(GenericRDRManager manager) {
        initialize();
        rdrManager = manager;
    }

    public void initialize(GenericRDRManager manager,
                           GenericRDRRule parent,
                           GenericExample e) {
        initialize();
        rdrManager = manager;
        this.parent = parent;
        C = e.C;
        e.rule = this;
        cornerstone = e;
    }


    /********************************************************************************/
    /* return true if one of the RDRs in the tree is satisfied by the examples      */
    /********************************************************************************/

    boolean isClassifiedByThisRule(GenericExample e) {
        if (satisfies(e)) {
            return true;
        }

        if (exception != null && exception.isClassifiedByThisRule(e)) {
            return true;
        }

        if (alternative != null && alternative.isClassifiedByThisRule(e)) {
            return true;
        }

        return false;
    }

    public int update(GenericExample e) {

        if (satisfies(e) &&
            (e.C != this.C ||
             (exception != null && exception.isClassifiedByThisRule(e)))) {
            if (exception == null) {
                exception = this.clonedRule() ;
                exception.initialize(rdrManager, this, e);
                return NEW_EXCEPTION;
            }
            else {
                return exception.update(e);
            }
        }
        else if (satisfies(e) && e.C == this.C) {
            if (debug) System.out.println("RDR: classified rules");
            return UNCHANGED;
        }
        else if (!satisfies(e) && try_add(e)) {
            if (debug) System.out.println("RDR: Extend current space");
            e.rule = this;
            return EXTENDED;
        }
        else
        if (alternative == null) {
            if (debug) System.out.println("RDR: new alternative");
            alternative = this.clonedRule();
            alternative.initialize(rdrManager, this, e);
            return NEW_ALTERNATIVE;
        }
        else {
            return alternative.update(e);
        }
    }


    boolean classify(GenericExample e) {
        if (satisfies(e)) {
            if (exception == null || !exception.classify(e)) {
                e.C = C;
                e.rule = this;
            }
            return true;
        }

        if (alternative != null && alternative.classify(e)) {
            return true;
        }

        return false;
    }

    /*******************************************************************************
     *			undo the last change to the RDR
     * How is it done ?
     *    when an example is added, it either does 3 things:
     *       1.Exception is created
     *       2.The current rule is extended/modified (try_add)
     *       3.New Alternative is created
     * In case 2, rule is backed up by history. In the other 2 cases, exception/alternative
     * is removed from its parent node.
     *******************************************************************************/

    void undo() {

        if (history == null) {
            if (parent.exception == this) {
                parent.exception = null;
            }
            if (parent.alternative == this) {
                parent.alternative = null;
            }
        }
        else {
            copyRDRRule(history);
        }
    }

    /********************************************************************************/
    /*			record RDR before making a change			*/
    /********************************************************************************/

    protected void backup(GenericRDRRule old) {
        GenericRDRRule r = old.clonedRule();
        history = r;
    }

    /********************************************************************************/
    /* print RDR - initial call should have 0 indent				*/
    /********************************************************************************/

    void print(int indent) {
        for (int i = 0; i < indent; i++) {
            System.out.print("|\t");
        }
        System.out.println(this);

        if (exception != null) {
            exception.print(indent + 1);
        }
        if (alternative != null) {
            alternative.print(indent);
        }
    }

    private String indent(int level, String str) {
        String res = new String();
        for (int i = 0; i < level; i++) {
            res += "\t";
        }
        res += str;
        return res;
    }

    public String toString() {
        return "GenericRDRRule";
    }

    public int getNumberOfNodes(){
        int c = 1;
        if (exception != null) c += exception.getNumberOfNodes();
        if (alternative != null) c += alternative.getNumberOfNodes();
        return c;
    }
}
