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
 * CHANGE LOG :
 * + version 1.1:
 *      - Add serialization code
 * + version 1.2:
 *      - Refactored IntervalRDR out of BlobIntervalRDR (to be used in RelationIntervalRDR)
*  + version 1.6:
*       - Change goal to not using absolute ratio.(have to recompile)
*       - Bundle MetricBlob with ManRDR, add GoalRDR
*
 */

/** @refactoring todo
 * refactoring: use Model pattern for BlobRDR (sothat we know whether the file is modified or not
 * this is currently done by poping up message from the caller.
 * if possible, refactor the whole program to use MVC pattern*/

import java.io.*;
import javax.xml.parsers.*;

import org.w3c.dom.*;
import org.xml.sax.*;
import RoboShare.*;

public class ManRDRBlobInterval
    extends GenericRDRManager {
    //How to add new metric?
    //Make a class extends Metric class, and add an instantiated object here.
    //Add/Modify IntervalRDRRule's DEFAULT_RULE that covers the new metric

    private MetricBlob[] allMetrics = null;

    private int NUM_METRIC = 0;

    protected RuleRDRInterval rdrRule;

    public ManRDRBlobInterval( MetricBlob [] ruleMetrics) {
        allMetrics = ruleMetrics;
        NUM_METRIC = allMetrics.length;
        resetRDR();
    }
    public GenericRDRRule getRule(){return rdrRule;} ;

    public int getNumberOfMetrics() { return allMetrics.length;  }

    public int update(ExampleBlob e) {
        addExample(e);
        return rdrRule.update(e);
    }

    public int classify(Blob b) {
        ExampleBlob be = new ExampleBlob(b, CommonSense.NOT_KNOWN, this);
        rdrRule.classify(be);
        return be.C;
    }

    public int classify(ExampleBlob be) {
        rdrRule.classify(be);
        return be.C;
    }

    /*********************************************************
     * SERIALIZATION CODE
     * *******************************************************/

    public String [] getMetricsDescription(){
        String [] descriptions = new String[ getNumberOfMetrics() ];
        for (int i = 0 ; i < descriptions.length; i++){
            descriptions[i] = getMetric(i).name();
        }
        return descriptions;
    }

    public void saveToFile(String fileName) {
        try {
            OutputStream out = new FileOutputStream(fileName);
            PrintStream pstr = new PrintStream(out);
            pstr.println("<root>");
            String [] descriptions = getMetricsDescription();
            for (int i = 0 ; i < descriptions.length;i++)
                pstr.println("<metric name ='" + descriptions[i] + "' />");
            rdrRule.saveRule(pstr, 0);
            saveExample(pstr);
            pstr.println("</root>");
            pstr.flush();
            out.flush();
            out.close();
            this.fileName = fileName;
        }
        catch (FileNotFoundException ex) {
        }
        catch (IOException ex) {
        }
    }


    public void saveExample(PrintStream out) {
        int count = 0;
        out.println("<example>");
        for (GenericExample e = allExamples; e != null; e = e.other_examples) {
            GenericIntervalExample be = (GenericIntervalExample) e;
            out.println("\t<cornerstone metrics='" + be.metricsToString() +
                        "' class='" + be.C + "'/>");
            count++;
        }
        out.println("</example>");
        System.out.println(count + " cornerstone(s) saved");
    }

    public void resetRDR(){
        rdrRule = new RuleRDRBlobInterval(this);
        allExamples = null;
        fileName = null;

    }

    public void loadFromFile(String fileName) {
        resetRDR();
        this.fileName = fileName;
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
            System.out.println("Blob Interval RDR file loaded : " + fileName);
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

    public void loadFromExampleNode(Node node) {
        allExamples = null;
        Node child = node.getFirstChild();
        int count = 0;
        while (child != null) {
            if (addExampleFromNode(child)) count++;
            child = child.getNextSibling();
        }
        System.out.println(count + " cornerstone(s) loaded.");
    }

    private boolean addExampleFromNode(Node node) {
        if (node.getNodeName().equals("cornerstone")) {
            String attr = node.getAttributes().getNamedItem("metrics").
                getNodeValue();
            String cl = node.getAttributes().getNamedItem("class").
                getNodeValue();

            GenericIntervalExample ex = new GenericIntervalExample(attr);
            int C = Integer.parseInt(cl);
            ex.C = C;
            addExample(ex);
            return true;
        }
        else
            return false;
    }

    /*********************************************************
     * Testing
     * *******************************************************/

    static public void main(String[] args) {
//        test3();
        ManRDRBlobInterval rdr = new ManRDRBlobInterval(RuleCollection.H_AA_D_Rule);
        rdr.loadFromFile("D:\\My Documents\\Robocup\\Experiments\\JBuilder\\working\\BlobRDR\\testing new\\testing23.xml");
        rdr.print();
    }



    public Pair[] getAllMetrics(Blob b) {
        Pair[] pairs = new Pair[NUM_METRIC];
        for (int i = 0; i < pairs.length; i++) {
            pairs[i] = new Pair(allMetrics[i].name(),
                                allMetrics[i].getMetric(b));
        }
        return pairs;
    }

    public MetricBlob getMetric(int id) {
        if (id >= 0 && id < allMetrics.length)
            return allMetrics[id];
        else
            return null;
    }

    public MetricBlob [] getRuleMetrics(){
        return allMetrics;
    }

}



