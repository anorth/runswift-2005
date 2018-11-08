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

import RoboShare.*;

/** @todo
* remove stupid id in MetricBlob class */

public class RuleCollection {
    //please keep this order, only add to the end
    static public final MetricBlob[] H_AA_C_Rule = {
        new HeightMetric(0),
        new AbsoluteAspectRatioMetric(1),
        new CircularityMetric(2),
    };

    static public final MetricBlob[] H_A_SD_Goal_Rule = {
        new HeightMetric(0),
        new AspectRatioMetric(1),
        new ScaledDensityMetric(2),
    };


    static public final MetricBlob[] H_AA_D_Rule = {
        new HeightMetric(0),
        new AbsoluteAspectRatioMetric(1),
        new DensityMetric(2),
    };

    static public final MetricBlob[] H_AA_SD_Rule = {
       new HeightMetric(0),
       new AbsoluteAspectRatioMetric(1),
       new ScaledDensityMetric(2),
   };

   static public final MetricBlob[] H_AA_SC_Rule = {
        new HeightMetric(0),
        new AbsoluteAspectRatioMetric(1),
        new ScaledCircularityMetric(2),
    };

    static public final MetricBlob[][] ruleMetricOptions = {
        RuleCollection.H_AA_C_Rule,
        RuleCollection.H_AA_D_Rule,
        RuleCollection.H_A_SD_Goal_Rule,
        RuleCollection.H_AA_SD_Rule,
        RuleCollection.H_AA_SC_Rule,
    };

    static public final String[] ruleMetricOptionStrings = {
        "H_A_C_Rule",
        "H_A_D_Rule",
        "H_A_SD_Goal_Rule",
        "H_AA_SD_Rule",
        "H_AA_SC_Rule",
    };

}

class AreaMetric
    extends MetricBlob {
    public AreaMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        return new Integer(b.area);
    }

    public String name() {
        return "Area";
    }

}

/**
 * max(width , height) / min(width, height) ( width / height are rotated )
 */

class AspectRatioMetric
    extends MetricBlob {

    public AspectRatioMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        double t = b.getRawWidth() / b.getRawHeight();
        if ( t < 1.0 ) t = - ( 1/t - 1);
        else t -= 1;
        return new Integer( toInteger( t , -3, 3 ) );
    }



    public String name() {
        return "Aspect Ratio";
    }

}

class AbsoluteAspectRatioMetric
    extends MetricBlob {

    public AbsoluteAspectRatioMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        double max = Math.max(b.getWidth() , b.getHeight()) + 1;
        double min = Math.min(b.getWidth() , b.getHeight()) + 1; //to avoid zero division
        return new Integer( toInteger(max / min , 0, 6 ) );
    }



    public String name() {
        return "Absolute Aspect Ratio";
    }

}

class ElevationMetric
    extends MetricBlob {


    public ElevationMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        int x = (b.xMax + b.xMin) / 2;
        int y = (b.yMax + b.yMin) / 2;

        return new Integer(toInteger( Utils.getElevationImageXY(x,y,b.cameraMatrix) , -90, 90 ) ) ;
    }

    public String name() {
        return "Elevation";
    }

}

/**
 * There're serveral options here:
 *   - use sqrt(area )
 *   - use rotated height
 *   - use rotated width
 *   - rotated sqrt( width * rotated height)  (area)
 * */

class HeightMetric
    extends MetricBlob {

    //some scaling so that the range is reasonable.
    //chosen to be BALL_RADIUS * APERTURE_DISTANCE
    static final double MAGIC_SCALE = 45 * 192.18765624795282;

    public HeightMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        int x = (b.xMax + b.xMin) / 2;
        int y = (b.yMax + b.yMin) / 2;

        double elev = Utils.getElevationImageXYRadians(x,y,b.cameraMatrix);
        double t = Math.sin(elev) / Math.sqrt(b.getHeight()); //use b.getHeight()
        // instead of b.area because b.area is sentitive to lightning condition.

//        if (t < 0) t = 0.0;
        int h = toInteger( t * MAGIC_SCALE + Utils.getCameraHeight(b.cameraMatrix), -3000, 3000  ) ;
        return new Integer( h );
    }

    public String name() {
        return "Height";
    }

}

class DensityMetric
    extends MetricBlob {

    public DensityMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        double t = b.area / ( (b.getWidth() + 1.0) * (b.getHeight() + 1.0) );
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "Density";
    }

}



class CircularityMetric
    extends MetricBlob {


    public CircularityMetric(int num) {
        super(num);
    }


    public Comparable getMetric(Blob b) {
        int perimeter = b.getPerimeter();
        double t = ( 4 * Math.PI * b.area) / (perimeter * perimeter);
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "Circularity";
    }

}

class ScaledDensityMetric
    extends MetricBlob {

    public ScaledDensityMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        double t = b.area / ( (b.getWidth() + 1.0) * (b.getHeight() + 1.0) );
        if (b.area < 400){
            t = t / (1 - Math.log(b.area / 400.0));
        }
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "ScaledDensity";
    }
}

class ScaledCircularityMetric
    extends MetricBlob {


    public ScaledCircularityMetric(int num) {
        super(num);
    }


    public Comparable getMetric(Blob b) {
        int perimeter = b.getPerimeter();
        double t = ( 4 * Math.PI * b.area) / (perimeter * perimeter);
        if (b.area < 400){
            t = t / (1 - Math.log(b.area / 400.0));
        }
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "ScaledCircularity";
    }

}

class ScaledSimpleCircularityMetric
    extends MetricBlob {

    public ScaledSimpleCircularityMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {

        double t = b.perimeter * b.perimeter / ( 4 * Math.PI * b.area);
        if (b.area < 400){
            t = t / (1 - Math.log(b.area / 400.0));
        }

        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "ScaledSimpleCircularity";
    }

}

class SimpleCircularityMetric
    extends MetricBlob {

    public SimpleCircularityMetric(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {

        double t = b.perimeter * b.perimeter / ( 4 * Math.PI * b.area);
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "SimpleCircularity";
    }

}

class ScaledCircularityMetric2
    extends MetricBlob {


    public ScaledCircularityMetric2(int num) {
        super(num);
    }


    public Comparable getMetric(Blob b) {
        int perimeter = b.getPerimeter();
        double t = ( 4 * Math.PI * b.area) / (perimeter * perimeter);
        if (b.area < 400){
            t = t * Math.log(b.area) / Math.log(400.0);
        }
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "ScaledCircularity";
    }

}

class ScaledSimpleCircularityMetric2
    extends MetricBlob {

    public ScaledSimpleCircularityMetric2(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {

        double t = b.perimeter * b.perimeter / ( 4 * Math.PI * b.area);
        if (b.area < 400){
            t = t * Math.log(b.area) / Math.log(400.0);
        }

        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "ScaledSimpleCircularity";
    }

}

class ScaledDensityMetric2
    extends MetricBlob {

    public ScaledDensityMetric2(int num) {
        super(num);
    }

    public Comparable getMetric(Blob b) {
        double t = b.area / ( (b.getWidth() + 1.0) * (b.getHeight() + 1.0) );
        if (b.area < 400){
            t = t * Math.log(b.area) / Math.log(400.0);
        }
        return new Integer( toInteger(t, 0.0,1.0) );
    }

    public String name() {
        return "ScaledDensity";
    }

}
