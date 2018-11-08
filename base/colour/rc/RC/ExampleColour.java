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
//  example.java
//  rc
//
//  Created by Claude Sammut on Thu Apr 08 2004.
//

/** @refactor todo
 * Refactor to be subclass of GenericExample */

import java.io.*;
import RoboShare.Utils;

public class ExampleColour
    implements Serializable, Comparable {
    
    static final boolean RectToPolarConvert = false;
    static final int centerU = 128;
    static final int centerV = 128;
    
    public int attr[];
    public int C;
    public ExampleColour other_examples = null;
    RuleRDRColour rule;

//    static example all = null;

//	static int ByteToUInt(byte b) {
//		//return b & 0xff;
//		 return b + Byte.MAX_VALUE + 1;
//	}

    public ExampleColour(byte Y, byte U, byte V, int C) {
        attr = new int[3];
        attr[0] = Utils.unsignedByteToInt(Y);
        if (RectToPolarConvert) {
        	double realU = Utils.unsignedByteToInt(U) - centerU;
        	double realV = Utils.unsignedByteToInt(V) - centerV;
        	attr[1] = (int)Math.sqrt(realU*realU + realV*realV);
        	attr[2] = (int)(Math.atan2(realU, realV)/(2*Math.PI)*0xff);
        } else {
        	attr[1] = Utils.unsignedByteToInt(U);
        	attr[2] = Utils.unsignedByteToInt(V);
        }
        this.C = C;
    }

    ExampleColour(byte Y, byte U, byte V, byte radius, int C) {
        attr = new int[4];
        attr[0] = Utils.unsignedByteToInt(Y);
        if (RectToPolarConvert) {
        	double realU = Utils.unsignedByteToInt(U) - centerU;
        	double realV = Utils.unsignedByteToInt(V) - centerV;
        	attr[1] = (int)Math.sqrt(realU*realU + realV*realV);
        	attr[2] = (int)(Math.atan2(realU, realV)/(2*Math.PI)*0xff);
        } else {
        	attr[1] = Utils.unsignedByteToInt(U);
        	attr[2] = Utils.unsignedByteToInt(V);
        }
        attr[3] = Utils.unsignedByteToInt(radius);
        this.C = C;
    }

    ExampleColour(int [] values, int C){
        attr = new int[values.length];
        for (int i=0;i<values.length;i++)
            attr[i] = values[i];
        this.C = C;
    }

    public String attrToString() {
        if (attr == null || attr.length == 0) {
            return "";
        }

        String res = String.valueOf(attr[0]);
        for (int i = 1; i < attr.length; i++) {
            res += " " + attr[i];
        }
        return res;
    }

//    static void dump() {
//        for (example e = example.all; e != null; e = e.other_examples) {
//            System.out.println(e.attrToString() + " " + e.C);
//        }
//    }





    void print() {
        System.out.print("<" + attrToString() + " " +
                         C + ">");
    }

    public int attrLength() {
        return attr.length;
    }

    public int compareTo(Object other){
        ExampleColour o = (ExampleColour) other;
        if (C != o.C) return RCUtils.compareTo(C,o.C);
        for (int i = 0 ; i < attr.length;i++)
            if (attr[i] != o.attr[i])
                return RCUtils.compareTo(attr[i],o.attr[i]);
        return 0;
    }
}
