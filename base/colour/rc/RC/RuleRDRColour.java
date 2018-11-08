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

//
//  rdr.java
//  rc
//
//  Created by Claude Sammut on Thu Apr 08 2004.
//

/** @maybetodo
 * Case when RDR is fooled : overlaping rectangle, alternative */

import java.io.*;
import java.util.*;

import org.w3c.dom.*;
import RoboShare.*;

public class RuleRDRColour
    implements Serializable {
    private static final boolean debug = false;


    interval cond[];

    int C;
    RuleRDRColour exception;
    RuleRDRColour alternative;
    RuleRDRColour parent;
    ExampleColour cornerstone;
    RuleRDRColour history;

    ManRDRColour rdrManager;

    public static int DEFAULT_LEN = 3;
//    static RDRRule default_rule = new RDRRule(DEFAULT_LEN);
    static final boolean debug_parsing = true;

    public class IncompatibleRuleExample
        extends Exception {}

    /** The default RDR spans the entire space */

    RuleRDRColour(ManRDRColour manager) {
        this.rdrManager = manager;
        cond = new interval[manager.getRDRLen()];
        for (int i = 0; i < cond.length; i++) {
            cond[i] = new interval(0, 255); //FIXME: make 255 more general.
        }
        this.C = CommonSense.BACKGROUND;
        exception = null;
        alternative = null;
        cornerstone = null;
        history = null;
    }

    RuleRDRColour(RuleRDRColour parent, ExampleColour e) {
        rdrManager = parent.rdrManager;
        cond = new interval[e.attr.length];

        for (int i = 0; i < cond.length; i++) {
            cond[i] = new interval(e.attr[i]);

        }
        this.C = e.C;
        exception = null;
        alternative = null;
        this.parent = parent;
        cornerstone = e;
        e.rule = this;
    }

//    RDRRule(interval cond0, interval cond1, interval cond2, int C, RDRRule exception,
//        RDRRule alternative) {
//        cond = new interval[3];
//        cond[0] = cond0;
//        cond[1] = cond1;
//        cond[2] = cond2;
//        this.C = C;
//        this.exception = exception;
//        this.alternative = alternative;
//    }

    boolean isAtomic(interval [] cond){
        for (int i=0;i<cond.length;i++)
            if (cond[i].hi != cond[i].lo)
                return false;
        return true;
    }

    int update(ExampleColour e) {
        if (satisfies(e) &&
            (e.C != this.C || (exception != null && exception.isClassifiedByThisRule(e)))) {
            if (exception == null) {
                if (debug) System.out.println("RDR: new exception");
                exception = new RuleRDRColour(this, e);
                if (isAtomic(cond)){
                    System.out.println("Warning: Creating exception of atomic rule");
                }
                return GenericRDRRule.NEW_EXCEPTION;
            }
            else {
                return exception.update(e);
            }
        }
        else if (satisfies(e) && e.C == this.C) {
            if (debug) System.out.println("RDR: classified rules");
            return GenericRDRRule.UNCHANGED;
        }
        else if (!satisfies(e) && try_add(e)) {
            if (debug) System.out.println("RDR: Extend current space");
            e.rule = this;
            return GenericRDRRule.EXTENDED;
        }
        else
        if (alternative == null) {
            if (debug) System.out.println("RDR: new alternative");
            alternative = new RuleRDRColour(this, e);
            return GenericRDRRule.NEW_ALTERNATIVE;
        }
        else {
            return alternative.update(e);
        }
    }

    /********************************************************************************/
    /* return true if an example satisfies the conditions in the given RDR		*/
    /********************************************************************************/

    public boolean satisfies(ExampleColour e) {
        for (int i = 0; i < cond.length; i++) {
            if (e.attr[i] < cond[i].lo || cond[i].hi < e.attr[i]) {
                return false;
            }
        }
        return true;
    }

    /********************************************************************************/
    /* return true if one of the RDRs in the tree is satisfied by the examples      */
    /********************************************************************************/


    boolean isClassifiedByThisRule(ExampleColour e) {
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
    /********************************************************************************/
    /* Generalise the condition (i.e. expand the interval) to include the example   */
    /* Must test if new RDR will cover negative examples.				*/
    /* Returns true if does not.							*/
    /********************************************************************************/

    boolean try_add(ExampleColour e) {
        interval old[] = new interval[cond.length];

        for (int i = 0; i < cond.length; i++) {
            old[i] = new interval(cond[i].lo, cond[i].hi);

        }
        for (int i = 0; i < cond.length; i++) {
            cond[i].add(e.attr[i]);

        }
        for (ExampleColour p = rdrManager.allExamples ; p != null; p = p.other_examples) {
            if (p.C != C && satisfies(p)) {
                for (int i = 0; i < cond.length; i++) {
                    cond[i].lo = old[i].lo;
                    cond[i].hi = old[i].hi;
                }

                return false;
            }
        }

        backup(old);
        return true;
    }

    /********************************************************************************/
    /* return true if one of the RDRs in the tree is satisfied by the examples      */
    /********************************************************************************/

    boolean classify(ExampleColour e) throws IncompatibleRuleExample {
        if (e.attrLength() != cond.length) {
            throw new IncompatibleRuleExample();
        }
        if (satisfies(e)) {
            if (exception == null || !exception.classify(e)) {
                e.C = C;
            }
            return true;
        }

        if (alternative != null && alternative.classify(e)) {
            return true;
        }

        return false;
    }

    /********************************************************************************/
    /*			record RDR before making a change			*/
    /********************************************************************************/

    void backup(interval old[]) {
        RuleRDRColour r = new RuleRDRColour(rdrManager);
        r.parent = history;
        history = r;

        for (int i = 0; i < cond.length; i++) {
            r.cond[i].lo = old[i].lo;
            r.cond[i].hi = old[i].hi;
        }

        r.C = C;
    }

    /********************************************************************************/
    /*			undo the last change to the RDR				*/
    /********************************************************************************/

    void undo(RuleRDRColour r) {
        if (r != this)
            System.out.println("r is not this.");
        if (history == null) {
            if (parent.exception == this) {
                parent.exception = null;
            }
            if (parent.alternative == this) {
                parent.alternative = null;
            }
        }
        else {
            for (int i = 0; i < cond.length; i++) {
                cond[i].lo = history.cond[i].lo;
                cond[i].hi = history.cond[i].hi;
            }

            r.C = history.C;
            history = history.parent;
        }
    }

    /********************************************************************************/
    /* print RDR - initial call should have 0 indent				*/
    /********************************************************************************/

    void print(int indent) {
        for (int i = 0; i < indent; i++) {
            System.out.print("|\t");

        }
        for (int i = 0; i < cond.length; i++) {
            System.out.print("[" + cond[i].lo + " .. " + cond[i].hi + "]");

        }
        System.out.println(" --> " + C);
        /*
          if (cornerstone != null)
          {
           System.out.print(" ");
           cornerstone.print();
          }
          System.out.println();
         */
        if (exception != null) {
            exception.print(indent + 1);
        }
        if (alternative != null) {
            alternative.print(indent);
        }
    }


    public String toString() {
        String res = new String(cond[0].toString());

        for (int i = 1; i < cond.length; i++) {
            res += " ; " + cond[i].toString();
        }
        return res;
    }

    public void saveRule(PrintStream out, int level) {
        out.println(RCUtils.indent(level, "<rdr>"));
        String attr = "ranges= '" + toString() + "'";
        out.println(RCUtils.indent(level, "<interval " + attr + "/>"));
        out.println(RCUtils.indent(level, "<color value= '" + C + "' />"));
        if (exception != null) {
            out.println(RCUtils.indent(level, "<exception>"));
            exception.saveRule(out, level + 1);
            out.println(RCUtils.indent(level, "</exception>"));
        }
        if (alternative != null) {
            out.println(RCUtils.indent(level, "<alternative>"));
            alternative.saveRule(out, level + 1);
            out.println(RCUtils.indent(level, "</alternative>"));
        }

        out.println(RCUtils.indent(level, "</rdr>"));
    }





    private void processAttribute(Node node) {
        if (node.getNodeName().equals("interval")) {
            if (debug_parsing) {
                System.out.println("Got interval.");
            }
            String ranges = node.getAttributes().getNamedItem("ranges").
                getNodeValue();
            StringTokenizer st = new StringTokenizer(ranges, ";");
            cond = new interval[st.countTokens()];

            int i = 0;
            while (st.hasMoreTokens()) {
                String invStr = st.nextToken();
                StringTokenizer invst = new StringTokenizer(invStr);
                cond[i] = new interval(Integer.parseInt(invst.nextToken()),
                                       Integer.parseInt(invst.nextToken()));

                i++;
            }
        }
        else if (node.getNodeName().equals("color")) {
            if (debug_parsing) {
                System.out.println("Got color.");
            }
            String value = node.getAttributes().getNamedItem("value").
                getNodeValue();
            C = Integer.parseInt(value);
        }
        else if (node.getNodeName().equals("exception")) {
            if (debug_parsing) {
                System.out.println("Got exception.");
            }
            exception = new RuleRDRColour(rdrManager);
            exception.loadFromNode(node.getFirstChild());
        }
        else if (node.getNodeName().equals("alternative")) {
            if (debug_parsing) {
                System.out.println("Got alternative.");
            }
            alternative = new RuleRDRColour(rdrManager);
            alternative.loadFromNode(node.getFirstChild());
        }
    }

    private void processNode(Node node) {
        if (node.getNodeName().equals("rdr")) {
            if (debug_parsing) {
                System.out.println("Got rdr.");
            }
            Node child = node.getFirstChild();
            while (child != null) {
                processAttribute(child);
                child = child.getNextSibling();
            }
        }
    }

    public void loadFromNode(Node node) {
        Node child = node;
        while (child != null) {
            processNode(child);
            child = child.getNextSibling();
        }
    }
}
