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
*         - Add perspectiveProjection transformation
*         - fix camera2World bug
 */
public class TransformationMatrix {

    static public Matrix identity(){
        return Matrix.identity(4,4);
    }

    static public Matrix translate(double x,double y, double z){
        double [][] arr = {
            {1 , 0 , 0 , x},
            {0 , 1 , 0 , y},
            {0 , 0 , 1 , z},
            {0 , 0 , 0 , 1}
        };
        return new Matrix(arr);
    }


    static public Matrix rotationXY(double angle){
        double siny = Math.sin(angle);
        double cosy = Math.cos(angle);

        double [][] arr = {
            {cosy,siny,  0 ,0},
            {-siny,cosy, 0 ,0},
            {0    ,   0, 1 ,0},
            {0    ,   0, 0 ,1}
        };
        return new Matrix(arr);
    }

    static public Matrix rotationYZ(double angle){
        double siny = Math.sin(angle);
        double cosy = Math.cos(angle);

        double [][] arr = {
            {1 ,    0,    0, 0},
            {0 , cosy, siny, 0},
            {0 ,-siny, cosy, 0},
            {0    ,   0, 0 , 1}
        };
        return new Matrix(arr);
    }

    static public Matrix rotationXZ(double angle){
        double siny = Math.sin(angle);
        double cosy = Math.cos(angle);

        double [][] arr = {
            {cosy, 0 ,-siny, 0},
            {  0 , 1 ,    0, 0},
            {siny, 0 , cosy, 0},
            {0    ,   0, 0 , 1},
        };
        return new Matrix(arr);
    }

    static public Matrix worldToLocal(Point3D u, Point3D v, Point3D n, Point3D o){
        double [][] arr = {
            {u.getX(), v.getX(), n.getX(), o.getX() },
            {u.getY(), v.getY(), n.getY(), o.getY() },
            {u.getZ(), v.getZ(), n.getZ(), o.getZ() },
            {0       ,        0,       0 ,        1 }
        };
        return new Matrix(arr);
    }


    static public Matrix localToWorld(Point3D u, Point3D v, Point3D n, Point3D o){
        double [][] arr = {
            {u.getX(), u.getY(), u.getZ(), -o.getX() },
            {v.getX(), v.getY(), v.getZ(), -o.getY() },
            {n.getX(), n.getY(), n.getZ(), -o.getZ() },
            {0       ,        0,       0 ,        1  }
        };
        return new Matrix(arr);
    }

    static public Matrix cameraToWorld(double pan, double tilt, double crane, double neckHeight, double bodyTilt){
        Matrix a = translate(0, RobotDefinition.CAMERA_OFFSET_Y, -RobotDefinition.CAMERA_OFFSET_Z);
        a = rotationYZ(-crane).times(a);
        a = rotationXY(-pan).times(a);
        a = translate(0,0,RobotDefinition.NECK_LENGTH).times(a);
        a = rotationYZ(- (tilt - bodyTilt) ).times(a);
        a = translate(0,0,neckHeight).times(a);
        return a;
    }

    public static Matrix translateToZero(Matrix a) {
        Matrix res = a.copy();
        res.set(0, 3, 0.0);
        res.set(1, 3, 0.0);
        res.set(2, 3, 0.0);
        return res;
    }

    public static Matrix perspectiveProjection(double d) {
        double [][] arr = {
            {1 , 0 , 0  ,  0},
            {0 , 1 , 0  ,  0},
            {0 , 0 , 0  ,  0},
            {0 , 0 , 1/d , 1}
        };
        return new Matrix(arr);
    }

    static public Matrix paramaterizedCameraToWorld(double pan, double tilt, double crane,
                                       double neckHeight, double bodyTilt, double neckLength) {
        Matrix a = translate(0, RobotDefinition.CAMERA_OFFSET_Y,
                             -RobotDefinition.CAMERA_OFFSET_Z);
        a = rotationYZ( -crane).times(a);
        a = rotationXY( -pan).times(a);
        a = translate(0, 0, neckLength).times(a);
        a = rotationYZ( - (tilt - bodyTilt)).times(a);
        a = translate(0, 0, neckHeight).times(a);
        return a;
    }

}
