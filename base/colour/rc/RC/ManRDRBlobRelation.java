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

import java.io.*;
import javax.xml.parsers.*;

import org.w3c.dom.*;
import org.xml.sax.*;
import RoboShare.*;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.5
* CHANGE LOG
* - version 1.1
*       - Remove centroid distance from relational rdr --> not scale invariant
*   + version 1.5:
*       - Fix Bug : non-symmetric metric in RadiusRatio and RadiusCovering.
*   + version 1.6:
*       - Change radius ratio to be max(widht, height)
 */

/** @todo
* remove static allMetrics */

public class ManRDRBlobRelation
    extends GenericRDRManager {

    /**
     * How to add new metric?
     * Make a class extends Metric class, and add an instantiated object here.
     * Add/Modify IntervalRDRRule's DEFAULT_RULE that covers the new metric
     */
//        new CentroidDistanceMetric(1),
    static public MetricBlobRelation[] allMetrics = {
        new RadiusRatioMetric(1),
        new GradientMetric(2),
        new RadiusCoveringMetric(3)
    };

    static int NUM_METRIC = allMetrics.length ;

    static public Pair[] getAllMetrics(Blob b1, Blob b2) {
        Pair[] pairs = new Pair[NUM_METRIC];
        for (int i = 0; i < pairs.length; i++) {
            pairs[i] = new Pair(allMetrics[i ].name(),
                                allMetrics[i ].getMetric(b1, b2));
        }
        return pairs;
    }

    static MetricBlobRelation getMetric(int id) {
        if (id >= 0 && id < allMetrics.length)
            return allMetrics[id];
        else
            return null;
    }

    protected RuleRDRRelation rdrRule;

    public int getNumberOfMetrics() { return allMetrics.length;  }

    public ManRDRBlobRelation() {
        rdrRule = new RuleRDRRelation(this);
        allExamples = null;
        fileName = null;
    }
    public GenericRDRRule getRule(){return rdrRule;} ;

    public int update(ExampleBlobRelation e) {
        addExample(e);
        return rdrRule.update(e);
    }

    public int classify(Blob b1,Blob b2) {
        ExampleBlobRelation be = new ExampleBlobRelation(b1,b2, CommonSense.NOT_KNOWN);
        rdrRule.classify(be);
        return be.C;
    }

    public int classify(ExampleBlobRelation be) {
        rdrRule.classify(be);
        return be.C;
    }

    /*********************************************************
     * SERIALIZATION CODE
     * *******************************************************/

    public void saveToFile(String fileName) throws FileNotFoundException,
        IOException {
        this.fileName = null;
        OutputStream out = new FileOutputStream(fileName);
        PrintStream pstr = new PrintStream(out);
        pstr.println("<root>");
        rdrRule.saveRule(pstr, 0);
        saveExample(pstr);
        pstr.println("</root>");
        pstr.flush();
        out.flush();
        out.close();
        this.fileName = fileName;
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

    public void loadFromFile(String fileName) {
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
            String blobMetricsString = node.getAttributes().getNamedItem("metrics").
                getNodeValue();
            String cl = node.getAttributes().getNamedItem("class").
                getNodeValue();

            GenericIntervalExample ex = new GenericIntervalExample(blobMetricsString);
            ex.C = Integer.parseInt(cl);
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
        test1();

    }

    static void test1() {

    }

}

class TautologyRelation
    extends MetricBlobRelation {

    public TautologyRelation(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b1, Blob b2) {
        return new Integer(0);
    }

    public String name() {
        return "Tautology";
    }

}

class CentroidDistanceMetric
    extends MetricBlobRelation {

    public CentroidDistanceMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b1, Blob b2) {
        Point2D c1 = b1.getCentroid();
        Point2D c2 = b2.getCentroid();
        return new Integer( toInteger(Utils.distance(c1, c2), 0 , 263.0) );
    }

    public String name() {
        return "Centroid Distance";
    }

}

/**
 * return the absolute angle between the beacon and the horizon (roll).
 */

class GradientMetric
    extends MetricBlobRelation {

    public GradientMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b1, Blob b2) {
        Point2D c1 = b1.getCentroid();
        Point2D c2 = b2.getCentroid();
        double beaconRoll = Math.atan2(c1.getRawX() - c2.getRawX(),
                                       c1.getRawY() - c2.getRawY());
        return new Integer( toInteger(Utils.normalise90Rad(beaconRoll), 0, Math.PI / 2) );
    }


    public String name() {
        return "Gradient";
    }

}

/**
 * return the aspect ratio of the (merged) beacon (like Blob Aspect Ratio, the value always > 1
 */

class AspectRatioBeaconMetric
    extends MetricBlobRelation {

    public AspectRatioBeaconMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b1, Blob b2) {
        double width = Math.max(b1.xRotMax, b2.xRotMax) -
            Math.min(b1.xRotMin, b2.xRotMin);
        double height = Math.max(b1.yRotMax, b2.yRotMax) -
            Math.min(b1.yRotMin, b2.yRotMin);

        if (width > height) {
            double t = width;
            width = height;
            height = t;
        }
        return new Integer(toInteger(height / width, 0, 100.0));
    }



    public String name() {
        return "Aspect Ratio";
    }

}

/* absolute ratio between two radius */

class RadiusRatioMetric
    extends MetricBlobRelation {

    public RadiusRatioMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b1, Blob b2) {
        double radius1 = Math.max(b1.getRawHeight(), b1.getRawWidth());
        double radius2 = Math.max(b2.getRawHeight(), b2.getRawWidth());
        double min = Math.min(radius1, radius2);
        double max = Math.max(radius1, radius2);
        return new Integer(toInteger(1.0 * max / min,0,10));
    }


    public String name() {
        return "Radius Ratio";
    }

}

/**
 * Percent of the distance bt 2 centroids that is covered by the right color
 */
class RadiusCoveringMetric
    extends MetricBlobRelation {

    public RadiusCoveringMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b1, Blob b2) {
//        double radius1 = Math.max(b1.getRawHeight(), b1.getRawWidth());
//        double radius2 = Math.max(b2.getRawHeight(), b2.getRawWidth());
        Point2D c1 = b1.getCentroid();
        c1.unrotated(b1.roll);
        Point2D c2 = b2.getCentroid();
        c2.unrotated(b2.roll);
        if (c1.getX() > c2.getX() || (c1.getX() == c2.getX() && c1.getY() > c2.getY()) ){
            Point2D tmp = c1; c1 = c2; c2 = tmp;
        } //swap to ensure symmetric property.

        int numberOfPixel = Math.max(Math.abs(c2.getX() - c1.getX()),
                                     Math.abs(c2.getY() - c1.getY())) + 1;
        double stepX = 1.0 * (c2.getX() - c1.getX()) / numberOfPixel;
        double stepY = 1.0 * (c2.getY() - c1.getY()) / numberOfPixel;
        double u = c1.getX();
        double v = c1.getY();
        BFL bfl = b1.bfl;
        int count1 = 0,
            count2 = 0;
        for (int i = 0; i < numberOfPixel; i++) {
            if (bfl.getC( (int) u, (int) v) == b1.colour) {
                count1++;
            }
            if (bfl.getC( (int) u, (int) v) == b2.colour) {
                count2++;
            }
            u += stepX;
            v += stepY;
        }
        return new Integer(toInteger( 1.0 * (count1 + count2) / (numberOfPixel ) , 0, 1.0));
    }



    public String name() {
        return "Radius Covering";
    }

}
