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
 * @version 1.1
 * CHANGE LOG :
 * + version 1.1
 *     - Associate Camera2World matrix with Blob, and roll (save roll computation)
 *     - Add serialization code
 *     - Use rotated with/height instead. (Gonna remove xMin, xMax as they're not used)
 *     minor : Added bfl reference back, for RadiusCovering metric to use
 */

import java.util.*;
import Jama.*;
import RoboShare.*;

public class Blob {
    public int xMin,yMin,xMax,yMax;
    public double xRotMin,yRotMin,xRotMax,yRotMax;
    public int id;
    public int area;
    public int perimeter;
    public int colour;
    public BFL bfl; //needed for calculating the radius(scan each pixel )
    //roll : angle in radian with x axis
    double roll;
    Matrix cameraMatrix;
    public static int VERY_BIG = 1000000;


    public Blob(){
    }

    public Blob(int c, BFL bfl, double roll) {
        area = 0;
        perimeter = 0;
        xMin = VERY_BIG;
        yMin = VERY_BIG;
        xMax = -VERY_BIG;
        yMax = -VERY_BIG;

        xRotMin = VERY_BIG;
        yRotMin = VERY_BIG;
        xRotMax = -VERY_BIG;
        yRotMax = -VERY_BIG;

        colour = c;
        this.roll = roll;
        cameraMatrix = bfl.getCamera2WorldMatrix();
        this.bfl = bfl;
    }

    public void addRotated(double x,double y){
        if (x < xRotMin) xRotMin = x;
        if (x > xRotMax) xRotMax = x;
        if (y < yRotMin) yRotMin = y;
        if (y > yRotMax) yRotMax = y;
        //area += 1;
    }

    public void add(int x,int y){

        if (x < xMin) xMin = x;
        if (x > xMax) xMax = x;
        if (y < yMin) yMin = y;
        if (y > yMax) yMax = y;
        area += 1;
    }

    public boolean contain(int x,int y){
        Point2D p = Utils.getRotatedPixel(x,y,roll);
        return (p.getRawX() >= xRotMin && p.getRawX() <= xRotMax &&
                p.getRawY() >= yRotMin && p.getRawY() <= yRotMax);
    }

    public String toString(){
        int p = getPerimeter() ;
//        int p = 0;
        return "C = " + colour + " Area = " + area + " pos = [" + xMin + " " + yMin + "]  [" + xMax + " " + yMax + "]  perimeter=" + perimeter + " , p2 = " + p + " circ = " + (4 * Math.PI * area / (p * p) );
    }

    public int getWidth(){
        return (int) (xRotMax - xRotMin);
//        return xMax - xMin;
    }

    public int getHeight(){
        return (int) (yRotMax - yRotMin);
//        return yMax - yMin;
    }

    public double getRawWidth(){
        return xRotMax - xRotMin;
    }

    public double getRawHeight(){
        return yRotMax - yRotMin;
    }


    public int getColour(){
        return colour;
    }

    public Point2D getCentroid(){
        return new Point2D( ( xRotMax + xRotMin ) / 2 , ( yRotMax + yRotMin) / 2 );
    }

    public Point2D getUnRotatedCentroid(){
        return new Point2D( ( xMax + xMin ) / 2 , ( yMax + yMin) / 2 );
    }


    /************************************
     * SERIALIZATION CODE
     ************************************/

    public String serialize(){
        return "C = " + colour + ";" +
           " Area = " + area + ";" +
          " xMin = " + xMin + ";" +
          " xMax = " + xMax + ";" +
          " yMin = " + yMin + ";" +
          " yMax = " + yMax + ";" +
          " xRotMin = " + xRotMin + ";" +
          " xRotMax = " + xRotMax + ";" +
          " yRotMin = " + yRotMin + ";" +
          " yRotMax = " + yRotMax + ";" +
          " Roll = " + roll + ";" +
          " Camera Matrix = " + RCUtils.matrixToString(cameraMatrix) + ";" ;
    }

    private static String parseValue(String str){
        StringTokenizer st = new StringTokenizer(str,"=");
        st.nextToken();
        return st.nextToken().trim();
    }

    public static Blob fromString(String str){
        StringTokenizer st = new StringTokenizer(str,";");
        Blob b = new Blob();
        b.colour = Integer.parseInt( parseValue( st.nextToken() ) );
        b.area = Integer.parseInt( parseValue( st.nextToken() ) );
        b.xMin = Integer.parseInt( parseValue( st.nextToken() ) );
        b.xMax = Integer.parseInt( parseValue( st.nextToken() ) );
        b.yMin = Integer.parseInt( parseValue( st.nextToken() ) );
        b.yRotMax = Double.parseDouble( parseValue( st.nextToken() ) );
        b.xRotMin = Double.parseDouble( parseValue( st.nextToken() ) );
        b.xRotMax = Double.parseDouble( parseValue( st.nextToken() ) );
        b.yRotMin = Double.parseDouble( parseValue( st.nextToken() ) );
        b.yRotMax = Double.parseDouble( parseValue( st.nextToken() ) );
        b.roll = Double.parseDouble( parseValue( st.nextToken() ) );
        b.cameraMatrix = RCUtils.stringToMatrix(parseValue( st.nextToken() ) );
        return b;
    }

    public Blob merge(Blob other){
        Blob newB = new Blob(colour,bfl,roll);
        newB.xMin = Math.min(xMin,other.xMin) - 2;
        newB.yMin = Math.min(yMin,other.yMin) - 2;
        newB.xMax = Math.max(xMax,other.xMax) + 2;
        newB.yMax = Math.max(yMax,other.yMax) + 2;
        newB.xRotMin = Math.min(xRotMin,other.xRotMin) - 2;
        newB.yRotMin = Math.min(yRotMin,other.yRotMin) - 2;
        newB.xRotMax = Math.max(xRotMax,other.xRotMax) + 2;
        newB.yRotMax = Math.max(yRotMax,other.yRotMax) + 2;
        newB.area = area + other.area;
        newB.perimeter = perimeter + other.perimeter; //FIXME: not correct
        return newB;
    }

    public void inMerge(Blob other){
        xMin = Math.min(xMin,other.xMin) ;
        yMin = Math.min(yMin,other.yMin) ;
        xMax = Math.max(xMax,other.xMax) ;
        yMax = Math.max(yMax,other.yMax) ;
        xRotMin = Math.min(xRotMin,other.xRotMin) ;
        yRotMin = Math.min(yRotMin,other.yRotMin) ;
        xRotMax = Math.max(xRotMax,other.xRotMax) ;
        yRotMax = Math.max(yRotMax,other.yRotMax) ;
        area += other.area;
        perimeter += other.perimeter;
    }

      static final int UP = 0;
      static final int RIGHT = 1;
      static final int DOWN = 2;
      static final int LEFT = 3;

      static final int nextX [] = {0,1,0,-1};
      static final int nextY [] = {-1,0,1,0};

      static final int neighborLeft[][] = {
          {-1,-1} ,
          {0 ,-1} ,
          {0 ,0 } ,
          {-1, 0}
      };
      static final int neighborRight[][] = {
          {0 ,-1} ,
          {0 ,0 } ,
          {-1, 0} ,
          {-1,-1}
      };
      static public boolean oppositeDir(int d1, int d2) {
          return Math.abs(d1 - d2) == 2;
      }

      static int oppositeDir[] = {2,3,0,1};
      static int turnRight[] = {1,2,3,0};
      static int turnLeft[] = {3,0,1,2};
      //precondition : x,y is not on the boundary of the bfl. ( x != 0 or WIDTH , y != 0 or HEIGHT)
      private boolean isBlobBoundary( int x, int y, int dir) {
          int leftID = bfl.blobID [ x + neighborLeft[dir][0] ] [y + neighborLeft[dir][1] ];
          int rightID = bfl.blobID [ x + neighborRight[dir][0] ] [ y + neighborRight[dir][1] ]  ;


          return (leftID == id) != (rightID == id)  ;
      }

      private boolean isChessboardPosition(int x, int y){

          boolean isBlack1 = bfl.blobID[x][y] == id;
          boolean isBlack2 = bfl.blobID[x-1][y-1] == id;
          if (isBlack1 != isBlack2) return false;
          boolean isBlack3 = (bfl.blobID[x-1][y] == id);
          boolean isBlack4 = (bfl.blobID[x][y-1] == id);
          if (isBlack3 != isBlack4) return false;
          return isBlack1 != isBlack3 ;
      }

      public int getPerimeter() {
          int perimeter = 0;
          int x = xMin;
          int y = yMin;
          int direction = RIGHT;
          while ( y < bfl.HEIGHT - 1 && bfl.blobID[x][y] != id) y++;
          int startX = x;
          int startY = y;
          do {

              perimeter += 1;
              if (isChessboardPosition(x,y)){
                  int oppDir = oppositeDir[direction];
                  int rightID = bfl.blobID[ x + neighborRight[oppDir][0] ][ y + neighborRight[ oppDir][1] ];
                  if (rightID == id){
                      direction = turnRight[direction];
                      x += nextX[direction];
                      y += nextY[direction];
                  }
                  else{
                      direction = turnLeft[direction];
                      x += nextX[direction];
                      y += nextY[direction];
                  }
              }
              else{
                  for (int dir = 0; dir < 4; dir++)
                      if (!oppositeDir(dir, direction) &&
                          isBlobBoundary(x, y, dir)) {
                          x = x + nextX[dir];
                          y = y + nextY[dir];
                          direction = dir;
                          break;
                      }
              }
          }
          while (! (x == startX && y == startY));
//          System.out.println("Got p = " + perimeter);
          return perimeter;
    }
}
