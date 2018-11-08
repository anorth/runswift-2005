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
 * @version 1.2
 * CHANGE LOG:
 * + version 1.2:
 *     - Refactor to be shared by both BlobIntervalRDR and BlobRelationIntervalRDR
*  + version 1.6:
*      - remove most of (interval) rdr code here, leave only contructor for sub classes
 */

import java.io.*;
import org.w3c.dom.*;
import java.util.*;
import RoboShare.*;

abstract public class RuleRDRInterval
    extends GenericRDRRule {


    private static final boolean debug_parsing = true;

    Pair[] intervals;

    public RuleRDRInterval() {
        initialize();
    }

    public RuleRDRInterval(RuleRDRInterval copy) { //Copy constructor
        copyRDRRule(copy);
    }

    public void copyRDRRule(GenericRDRRule other) {
        super.copyRDRRule(other);
        RuleRDRInterval copy = (RuleRDRInterval) other;
        intervals = new Pair[copy.intervals.length];
        for (int i = 0; i < intervals.length; i++) {
            intervals[i] = new Pair(copy.intervals[i]);
        }
    }

    public void initialize(GenericRDRManager manager,
                           GenericRDRRule parent,
                           GenericExample e) {
        if (e instanceof GenericIntervalExample) {
            initialize(manager, parent, (GenericIntervalExample) e);
        }
        else {
            throw new ClassCastException(
                "BlobIntervalRDRRule : example must be BlobExample");
        }
    }

    public void initialize(GenericRDRManager manager,
                           GenericRDRRule parent,
                           GenericIntervalExample e) {
        super.initialize(manager, parent, e);
        intervals = new Pair[manager.getNumberOfMetrics() ];
        for (int i = 0; i < intervals.length; i++) {
            Object t = e.getMetricsArray().get(i);
            intervals[i] = new Pair(t, t);
        }
    }

    /********************************************************************************/
    /* return true if an example satisfies the conditions in the given RDR		*/
    /********************************************************************************/

    public boolean satisfies(GenericExample e) {
        if (e instanceof GenericIntervalExample) {
            return satisfies( (GenericIntervalExample) e);
        }
        else {
            throw new ClassCastException("update must get BlobExample");
        }
    }

    public boolean satisfies(GenericIntervalExample e) {
        ArrayList metrics = e.getMetricsArray();
        for (int i = 0; i < intervals.length; i++) {
            Comparable value = (Comparable) metrics.get(i);
            if (value.compareTo(intervals[i].first()) < 0 ||
                value.compareTo(intervals[i].second()) > 0) {
                return false;
            }
        }
        return true;
    }


    /********************************************************************************/
    /* Generalise the condition (i.e. expand the interval) to include the example   */
    /* Must test if new RDR will cover negative examples.				*/
    /* Returns true if does not.							*/
    /********************************************************************************/
    protected boolean try_add(GenericExample e) {
        if (e instanceof GenericIntervalExample)
            return try_add( (GenericIntervalExample) e);
        else
            throw new ClassCastException(
                "BlobIntervalRDRRule : try_add must get BlobExample");

    }

    protected boolean try_add(GenericIntervalExample e) {
        GenericRDRRule old = clonedRule();

        for (int i = 0; i < intervals.length; i++) {
            intervals[i].add(e.getMetricsArray().get(i));
        }

        for (GenericExample p = rdrManager.allExamples; p != null;
             p = p.other_examples) {
            if (p.C != C && satisfies(p)) {
                copyRDRRule(old);
                return false;
            }
        }

        backup(old);
        return true;
    }


    public String toString() {
        String res = new String();
        for (int i = 0; i < intervals.length; i++) {
            res += intervals[i] + " ";
        }
        res += "-> " + CommonSense.getBlobClassification(C);
        return res;
    }

    /**************************************************************
     *
     *   SERIALIZATION functions
     *
     **************************************************************/

    /**
     * For XML output
     * @return String
     */

    public String serializeAttributes() {
        String res = new String(intervals[0].serialize());
        for (int i = 1; i < intervals.length; i++) {
            res += " ; " + intervals[i].serialize();
        }
        return res;
    }

    public void saveRule(PrintStream out, int level) {
        out.println(RCUtils.indent(level, "<rdr>"));
        String attr = "ranges= '" + serializeAttributes() + "'";
        out.println(RCUtils.indent(level, "<interval " + attr + "/>"));
        out.println(RCUtils.indent(level, "<class value= '" + C + "' />"));
        if (exception != null) {
            out.println(RCUtils.indent(level, "<exception>"));
            ( (RuleRDRInterval) exception).saveRule(out, level + 1);
            out.println(RCUtils.indent(level, "</exception>"));
        }
        if (alternative != null) {
            out.println(RCUtils.indent(level, "<alternative>"));
            ( (RuleRDRInterval) alternative).saveRule(out, level + 1);
            out.println(RCUtils.indent(level, "</alternative>"));
        }

        out.println(RCUtils.indent(level, "</rdr>"));
    }

    public void loadFromNode(Node node) {
        Node child = node;
        while (child != null) {
            processNode(child);
            child = child.getNextSibling();
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

    private void processAttribute(Node node) {
        if (node.getNodeName().equals("interval")) {
            if (debug_parsing) {
                System.out.println("Got interval.");
            }
            String ranges = node.getAttributes().getNamedItem("ranges").
                getNodeValue();
            StringTokenizer st = new StringTokenizer(ranges, ";");
            intervals = new Pair[st.countTokens()];

            int i = 0;
            while (st.hasMoreTokens()) {
                intervals[i] = Pair.loadFromString(st.nextToken());
                i++;
            }
        }
        else if (node.getNodeName().equals("class")) {
            if (debug_parsing) {
                System.out.println("Got class.");
            }
            String value = node.getAttributes().getNamedItem("value").
                getNodeValue();
            C = Integer.parseInt(value);
        }
        else if (node.getNodeName().equals("exception")) {
            if (debug_parsing) {
                System.out.println("Got exception.");
            }
            exception = clonedRuleEmpty();
            ( (RuleRDRInterval) exception).loadFromNode(node.getFirstChild());
        }
        else if (node.getNodeName().equals("alternative")) {
            if (debug_parsing) {
                System.out.println("Got alternative.");
            }
            alternative = clonedRuleEmpty();
            ( (RuleRDRInterval) alternative).loadFromNode(node.getFirstChild());
        }
    }

}
