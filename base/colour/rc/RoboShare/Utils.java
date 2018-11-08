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



/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
 * CHANGE LOG:
 * + version 1.1:
 *     - Added dotProduct, getRotatedPixel , getElevation...
 * + version 1.6:
 *     - Fix a big bug in elevation calculation.
 */

package RoboShare;
import Jama.*;

public class Utils {
    static public double micro2deg(int t){
        return ( 1.0 * t / 1000000 ) * 180 / Math.PI;
    }
    static public double micro2rad(int t){
        return t / 1000000.0 ;
    }

    // FIXME: This is wrong. It (and some similar functions) were wrong and
    // we haven't fixed it since it will invalidate old RDRs. Swap for the
    // correct version some day.
    public static int unsignedByteToInt(byte c){
//        return c & 0xff; // correct!
        return c + Byte.MAX_VALUE + 1; // wrong
    }

    public static int byteToIntBIG(byte i1,byte i2,byte i3,byte i4){
        int t = unsignedByteToInt(i1);
        t = t * 256 + unsignedByteToInt(i2);
        t = t * 256 + unsignedByteToInt(i3);
        t = t * 256 + unsignedByteToInt(i4);
        return t;
    }
    public static int byteToIntLITTLE(byte i1,byte i2,byte i3,byte i4){
        int t = unsignedByteToInt(i4);
        t = t * 256 + unsignedByteToInt(i3);
        t = t * 256 + unsignedByteToInt(i2);
        t = t * 256 + unsignedByteToInt(i1);
        return t;
    }
    public static double radians(double deg){
        return deg / 180 * Math.PI;
    }
    public static double degrees(double rad){
       return rad * 180 / Math.PI;
   }

   /**
    * Get elevation from Image Coordinate : 0,0 at the top left corner, width,height
    * at the bottom right corner
    * @param imgX int
    * @param imgY int
    * @param camera2World Matrix
    * @return double
    */

   public static double getElevationImageXY(int imgX, int imgY, Matrix camera2World) {
       imgX = Utils.toImageCoordinateX(imgX);
       imgY = Utils.toImageCoordinateY(imgY);
       return Utils.getElevation(imgX, imgY, camera2World);
   }

   public static double getElevationImageXYRadians(int imgX, int imgY, Matrix camera2World) {
       imgX = Utils.toImageCoordinateX(imgX);
       imgY = Utils.toImageCoordinateY(imgY);
       return Utils.getElevationRadians(imgX, imgY, camera2World);
   }

   /**
    * Get elevation from Image Plane Coordinate : 0,0 at the center , x axis going
    * right and y axis going up
    * @param ipx double
    * @param ipy double
    * @param camera2World Matrix
    * @return double
    */

   public static double getElevation(double ipx, double ipy, Matrix camera2World) {
       return Utils.degrees(getElevationRadians(ipx,ipy,camera2World));
   }

   public static double getElevationRadians(double ipx, double ipy, Matrix camera2World) {
       Vector3D a = new Vector3D(ipx, RobotDefinition.APERTURE_DISTANCE, ipy);
       a.transform(camera2World);
       Vector3D b = new Vector3D(208, 0, -52);
       b.transform(camera2World);
       double projectXY = Math.sqrt(sqr(a.getX()) + sqr(a.getY()));
       return Math.atan2(a.getZ(), projectXY);
   }


   public static double getHeading(double ipx, double ipy, Matrix camera2World) {
       Vector3D a = new Vector3D(ipx, RobotDefinition.APERTURE_DISTANCE, ipy);
       a.transform(camera2World);
       return Utils.degrees( Math.atan2( a.getX(), a.getY() ) );
   }

   //convert coordinate centered on the windows to the normal image coordinate
   /**
    * Position X,Y : 0,0 is the center of the window
    * Coordinate X,Y : 0,0 is the top-left of the window
    * @param x int
    * @return int
    */

   public static int toImagePositionX(int x){
       return x + RobotDefinition.CPLANE_WIDTH / 2;
   }

   public static int toImagePositionY(int y){
       return - y + RobotDefinition.CPLANE_HEIGHT / 2;
   }

   public static int toImageCoordinateX(int x){
       return x - RobotDefinition.CPLANE_WIDTH / 2;
   }

   public static int toImageCoordinateY(int y){
       return - y + RobotDefinition.CPLANE_HEIGHT / 2;
   }

   public static double dotProduct(Matrix a, Matrix b){
       double sum = 0;
       int m = a.getRowDimension(),
           n = a.getColumnDimension();
       double [][] A = a.getArray();
       double [][] B = b.getArray();
       for (int i=0;i<m;i++){
           for (int j=0;j<n;j++){
               sum += A[i][j] * B[i][j];
           }
       }
       return sum;
   }

   public static Point2D getRotatedPixel(int x,int y, double roll){
       Point2D p = new Point2D(x,y);
       p.toImageCoordinate();
       p.rotate(roll);
       return p;
   }

   public static double sqr(double a){
       return a * a;
   }

   public static double distance(Point2D a, Point2D b){
       return Math.sqrt( sqr( a.getRawX() - b.getRawX() ) + sqr( a.getRawY() - b.getRawY() ) );
   }

   /**
    * Normalise the angle ( in range (-PI,PI) sothat it falls between 0,90 ( x and x+90 are the same)
    * @param r double
    * @return double
    */
   public static double normalise90Rad(double r){
       if (r < 0) r = -r;
       return Math.min(r,Math.PI - r);
   }

   public static double getCameraHeight(Matrix cameraToWorld){
       return cameraToWorld.get(2,3);
   }
}

