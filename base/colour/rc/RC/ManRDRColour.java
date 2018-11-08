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
 * @version 1.0
 */

import java.io.*;
import javax.xml.parsers.*;
import java.util.*;

import org.w3c.dom.*;
import org.xml.sax.*;
import RoboShare.*;

public class ManRDRColour implements ColourClassifier {

    protected RuleRDRColour rdrRule;
    public ExampleColour allExamples;

    protected String fileName = null; // Name of current RDR file for external use

    protected int rdrLen = 0;
    public ManRDRColour(int rdrLen) {
        this.rdrLen = rdrLen;
        rdrRule = new RuleRDRColour(this);
        allExamples = null;
    }

    public RuleRDRColour getRule() {
        return rdrRule;
    }

    public int getRDRLen() {
        return rdrLen;
    }


    protected void addExample(ExampleColour e){
        e.other_examples = allExamples;
        allExamples = e;
    }

    public boolean training_example(byte Y, byte U, byte V, int C) {
//        System.out.println("Training " + Y + " "+ U + " " + V + " " + C);
        ExampleColour e = new ExampleColour(Y, U, V, C);
        addExample(e);

        return (rdrRule.update(e) != GenericRDRRule.UNCHANGED);
    }

    public boolean training_example(ExampleColour ex) {
//        System.out.println("Training " + Y + " "+ U + " " + V + " " + C);
        if (test_example(ex) == ex.C)
            return false;
        addExample(ex);
        return (rdrRule.update(ex) != GenericRDRRule.UNCHANGED);
    }


    public boolean training_example(byte Y, byte U, byte V, byte radius, int C) {
//        System.out.println("Training " + Y + " "+ U + " " + V + " " + radius + " " + C);
        ExampleColour e = new ExampleColour(Y, U, V, radius, C);
        addExample(e);
        return (rdrRule.update(e) != GenericRDRRule.UNCHANGED);
    }

    public int test_example(byte Y, byte U, byte V) {
        ExampleColour e = new ExampleColour(Y, U, V, CommonSense.NOCOLOUR);
        try {
            rdrRule.classify(e);
        }
        catch (Exception ex) {
            System.out.println("Warning : Incompatible rule-example.");
        }

        return e.C;
    }

    public int test_example(ExampleColour e) {
        int oldC = e.C;
        try {
            rdrRule.classify(e);
        }
        catch (Exception ex) {
            return -1;
        }
        int res = e.C;
        e.C = oldC;
        return res;
    }

    public int test_example(byte Y, byte U, byte V, byte radius) {
        ExampleColour e = new ExampleColour(Y, U, V, radius, CommonSense.NOCOLOUR);

        try {
            rdrRule.classify(e);
        }
        catch (Exception ex) {
            System.out.println("Warning : Incompatible rule-example.");
        }
        return e.C;
    }

    public void undo() {
        if (allExamples != null) {
            ExampleColour last_example = allExamples;
            allExamples = allExamples.other_examples;

            if (last_example != null && last_example.rule != null) {
                last_example.rule.undo(last_example.rule);
            }

//		System.out.println("==========================");
//		rdr.default_rule.print(0);
        }
    }

    public void saveToFile(String fileName) {

        try {
            OutputStream out = new FileOutputStream(fileName);
            PrintStream pstr = new PrintStream(out);
            pstr.println("<root>");
            rdrRule.saveRule(pstr, 0);
            saveExample(pstr);
            pstr.println("</root>");
            pstr.flush();
            out.flush();
            out.close();

        }
        catch (FileNotFoundException ex) {
        }
        catch (IOException ex) {
        }

    }

    public void loadFromFile(String fileName) {
        DocumentBuilderFactory factory =
            DocumentBuilderFactory.newInstance();

        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File(fileName));
            Node root = document.getFirstChild();
            Node rdrChild = root.getFirstChild();
            while (!rdrChild.getNodeName().equals("rdr"))
                rdrChild = rdrChild.getNextSibling();
            Node exampleChild = rdrChild.getNextSibling();
            while (!exampleChild.getNodeName().equals("example"))
                exampleChild = exampleChild.getNextSibling();
            rdrRule.loadFromNode(rdrChild);
            loadFromExampleNode(exampleChild);
        }
        catch (ParserConfigurationException ex) {
            System.out.println("Exception occured");
            ex.printStackTrace();
        }
        catch (IOException ex) {
            System.out.println("File cannot be read.");
            ex.printStackTrace();
        }
        catch (SAXException ex) {
            System.out.println("XML Exception occured");
            ex.printStackTrace();
        }
    }

    private boolean addExampleFromNode(Node node) {
        if (node.getNodeName().equals("cornerstone")) {
            String attr = node.getAttributes().getNamedItem("attr").
                getNodeValue();
            String cl = node.getAttributes().getNamedItem("class").
                getNodeValue();
            StringTokenizer st = new StringTokenizer(attr);
            ArrayList ll = new ArrayList();
            while (st.hasMoreTokens()) {
                ll.add(st.nextToken());
            }
            int[] values = new int[ll.size()];
            for (int i = 0; i < ll.size(); i++) {
                values[i] = Integer.parseInt( (String) ll.get(i));
            }
            int C = Integer.parseInt(cl);
            addExample( new ExampleColour(values, C) );
            return true;
        }
        else
            return false;
    }

    public void loadFromExampleNode(Node node) {
        Node child = node.getFirstChild();
        int count = 0;
        while (child != null) {
            if (addExampleFromNode(child)) count++;
            child = child.getNextSibling();
        }
        System.out.println(count + " cornerstone(s) loaded.");
    }

    public void saveExample(PrintStream out) {
        int count = 0;
        out.println("<example>");
        for (ExampleColour e = allExamples; e != null; e = e.other_examples) {
            out.println("\t<cornerstone attr='" + e.attrToString() +
                        "' class='" + e.C + "'/>");
            count++;
        }
        out.println("</example>");
        System.out.println(count + " cornerstone(s) saved");
    }

    public void print() {
        rdrRule.print(0);
        int countExample = 0;
        for (ExampleColour e = allExamples; e != null; e = e.other_examples)
            countExample++;
        System.out.println("Number of examples " + countExample);
    }

    public static void main(String[] args) {
        ManRDRColour RDR1 = new ManRDRColour(3);
    }

}
