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

/**
 *
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
*  CHANGE LOG:
*  + version 1.1:
*       - added cross, dot products
 */

public class Vector3D {
    Matrix vector;
    public Vector3D(double x, double y, double z) {
        vector = new Matrix(new double[][]{{x},{y},{z},{0}} );
    }

    public Vector3D(Matrix a) {
        if (a.getRowDimension() != 4 || a.getColumnDimension() != 1) {
            throw new RuntimeException("Vector3D : matrix dim is not 4x1");
        }
        vector = a.copy();
    }

    public double getX() {
        return vector.get(0,0);
    }

    public double getY(){
        return vector.get(1,0);
    }

    public double getZ(){
        return vector.get(2,0);
    }

    public void transform(Matrix other){
        vector = other.times(vector);
    }

    public Vector3D cross(Vector3D A) {
        double x1 = getX();
        double y1 = getY();
        double z1 = getZ();
        double x2 = A.getX();
        double y2 = A.getY();
        double z2 = A.getZ();
        return new Vector3D( y1 * z2 - y2 * z1 ,
                             z1 * x2 - z2 * x1 ,
                             x1 * y2 - x2 * y1);
    }

    public double dot(Point3D A){
        return getX() * A.getX() + getY() * A.getY() + getZ() * A.getZ();
    }

    public double dot(Vector3D A){
        return getX() * A.getX() + getY() * A.getY() + getZ() * A.getZ();
    }

    public String toString(){
        return "<" + getX() + "," + getY() + "," + getZ() + ">";
    }
}
