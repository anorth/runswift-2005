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


package RoboShare;
import Jama.*;

public class Plane3D {
    Matrix plane;

    private void copy(double A, double B, double C, double D){
        plane = new Matrix(new double[][]{{A},{B},{C},{D}} );
    }

    public Plane3D(double A, double B, double C, double D) {
        copy(A,B,C,D);
    }

    /**
     * Construct Plane given 3d points
     */
    public Plane3D(Point3D A, Point3D B, Point3D C){
        Vector3D AB = new Vector3D(B.minus(A));
        Vector3D AC = new Vector3D(C.minus(A));
        Vector3D N = AB.cross(AC);

        copy(N.getX() , N.getY(), N.getZ() ,- N.dot(A) );
    }

    public double getA(){
        return plane.get(0,0);
    }

    public double getB(){
        return plane.get(1,0);
    }

    public double getC(){
        return plane.get(2,0);
    }

    public double getD(){
        return plane.get(3,0);
    }

    /**
     * Get distance from a point to this plane
     */
    public double distance(Point3D p){
        double A = getA(),
            B = getB(),
            C = getC();
        return Utils.dotProduct(p.getMatrix(),plane) / Math.sqrt( A*A + B*B + C*C ) ;
    }

}
