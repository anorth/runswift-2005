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
 * First Test: Old : Serializable was stackoverflow -> need to set stack size to be large.
 *             New : use XML
 *
 */

import java.io.*;
import RoboShare.*;
import Jama.*;

public class Testing {

    public Testing() {

    }

    static void testAtomic(){
        final int DEFAULT_LEN = 3;
        RuleRDRColour default_rule = new RuleRDRColour(new ManRDRColour(DEFAULT_LEN) );
        byte y = 100, u = 100, v = 100;
        ExampleColour myexample = new ExampleColour(y,u,v,3);
        default_rule.update(myexample);
        default_rule.print(0);
        myexample.C = 4;
        default_rule.update(myexample);
        System.out.println("After updating");
        default_rule.print(0);
    }

    static void testUndo(){
        final int DEFAULT_LEN = 3;
        ManRDRColour default_rule = new ManRDRColour(DEFAULT_LEN);

        default_rule.training_example((byte) 10, (byte)10, (byte) 10, 1 );
        default_rule.training_example((byte) 20, (byte)20, (byte) 20, 1 );
        System.out.println("Before updating");
        default_rule.getRule().print(0);

        default_rule.training_example((byte) 35, (byte)35, (byte) 35, 1 );
        System.out.println("After updating");
        default_rule.getRule().print(0);
        default_rule.training_example((byte) 25, (byte)25, (byte) 25, 1 );
//        example.training_example((byte) 15, (byte)15, (byte) 15, 2 );
        System.out.println("After updating");
        default_rule.getRule().print(0);
        default_rule.undo();
        System.out.println("Undone");
        default_rule.getRule().print(0);
        default_rule.undo();
        System.out.println("Undone");
        default_rule.getRule().print(0);


    }

    static void testTryAdd(){
        final int DEFAULT_LEN = 3;
        ManRDRColour default_rule = new ManRDRColour(DEFAULT_LEN);

        default_rule.training_example((byte) 10, (byte)10, (byte) 10, 1 );
        default_rule.training_example((byte) 20, (byte)20, (byte) 20, 2 );
        System.out.println("Before updating");
        default_rule.getRule().print(0);


    }


    static void testXYZ_Coord(){
        XYZ_Coord a = new XYZ_Coord(1,0,0);
        a.rotateXY(Utils.radians(45));
        System.out.println(" result = " + a);
        a = new XYZ_Coord(0,1,0);
        a.rotateYZ(Utils.radians(45));
        System.out.println(" result = " + a);
    }

    static void printMatrix(Matrix a){
        System.out.println("----  Matrix ----- ");
        for (int i = 0; i < a.getRowDimension(); i++){
            for (int j = 0; j < a.getColumnDimension(); j++) {
                System.out.print(a.get(i, j) + "  ");
            }
            System.out.println("");
        }
    }

    public static void main(String[] args)  {
//        testAtomic();
//        testUndo();
//        testTryAdd();
//        testXYZ_Coord();
//        System.out.println(RCUtils.generateUniqueName());
//        System.out.println(RobotDefinition.APERTURE_DISTANCE);
//        byte i1 = 13;
//        byte i2 = 57;
//        byte i3 = 131 - 256;
//        byte i4 = 211 - 256;
//        System.out.println(" input = " + i1 + " " + i2 + " " + i3 + " " + i4);
//        System.out.println("byteToInt = " + Utils.byteToIntLITTLE(i1,i2,i3,i4));
//        System.out.println("byteToInt = " + Utils.byteToIntBIG(i1,i2,i3,i4));

//        BFL bfl = new BFL(false);
//        try {
//            bfl.readFile("D:\\My Documents\\Robocup\\Experiments\\JBuilder\\working\\YUVStreamed\\Three beacons at the same time\\2005_02_22_10.bfl");
//            printMatrix(bfl.getCamera2WorldMatrix());
//        }
//        catch (FileNotFoundException ex) {
//        }
//        catch (IOException ex) {
//        }
//
//        DisjointSet a = new DisjointSet(0,0,10,0,0,bfl);
//        DisjointSet b = new DisjointSet(0,20,30,0,0,bfl);
//        DisjointSet c = new DisjointSet(0,5,25,10,0,bfl);
//        c.join(a);
//        c.join(b);
//        System.out.println("Area = " + c.getroot().b.toString() );

//        B b = new B();
//        ((A)b).f();
//        System.out.println("Output = " + b.x);

        int c = 130;
        System.out.println("c = " + c + "  byte(c) = " +(byte) c );
    }

}

class A{
    public A(){}
    int x;
    void f(){
        x = 1;
    }
}
class B extends A{
    public B(){}
    int x;
    void f(){
        x = 2;
    }
}
