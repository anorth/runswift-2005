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

public class Point3D {
    Matrix point;
    public Point3D(double x, double y, double z) {
        point = new Matrix(new double[][]{{x},{y},{z},{1}} );
    }

    public Point3D(Matrix a){
        point = new Matrix(a.getArray() );
        if (a.getRowDimension() != 4 || a.getColumnDimension() != 1){
            throw new RuntimeException("Point3D : matrix dim is not 4x1");
        }
    }

    public Matrix getMatrix(){
        return point;
    }

    public double getX(){
        return point.get(0,0);
    }

    public double getY(){
        return point.get(1,0);
    }

    public double getZ(){
        return point.get(2,0);
    }

    public void transform(Matrix other) {
        point = other.times(point);
    }

    public Matrix minus(Point3D other){
        return point.minus(other.getMatrix());
    }

    //Normalize after perspective projection (sothat W = 1);
    public void normalize(){
        double w = point.get(3,0);
        point.set(0,0, point.get(0,0) / w);
        point.set(1,0, point.get(1,0) / w);
        point.set(2,0, point.get(2,0) / w);
        point.set(3,0, 1.0);
    }

    public String toString(){
        return "[" + getX() + "," + getY() + "," + getZ() + "]";
    }
}
