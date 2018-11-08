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
/**
* Copy from robot/share/XYZ_Coord.h
**/


public class XYZ_Coord {
    public double x,y,z;

    public XYZ_Coord() {
        x = 0;
        y = 0;
        z = 0;
    }

    public  XYZ_Coord(double nx, double ny, double nz) {
        x = nx;
        y = ny;
        z = nz;
    }


    public void copy(XYZ_Coord other){
        x = other.x;
        y = other.y;
        z = other.z;
    }

    //clockwise rotation
    public  void rotateXY(double theta) {
        double lx = x;
        double ly = y;

        double sint = Math.sin(theta);
        double cost = Math.cos(theta);
        x = lx * cost + ly * sint ;
        y = - lx * sint + ly * cost ;

    }

    public  void rotateYZ(double theta) {
        double ly = y;
        double lz = z;

        double sint = Math.sin(theta);
        double cost = Math.cos(theta);

        y = ly * cost + lz * sint;
        z = - ly * sint + lz * cost;
    }

    public void rotatZX(double theta) {
        double lx = x;
        double lz = z;

        double sint = Math.sin(theta);
        double cost = Math.cos(theta);

        x = lx * cost - lz * sint;
        z = lx * sint + lz * cost;
    }


    public  void add(final XYZ_Coord opt) {
        x += opt.x;
        y += opt.y;
        z += opt.z;
    }

    public  void sub(final XYZ_Coord opt) {
        x -= opt.x;
        y -= opt.y;
        z -= opt.z;
    }

    public  void addMult(final XYZ_Coord opt, double mult) {
        x += opt.x*mult;
        y += opt.y*mult;
        z += opt.z*mult;
    }

    public  void normalize() {
        double dist = Math.sqrt(x*x + y*y + z*z);

        if (dist != 0) {
            x = x/dist;
            y = y/dist;
            z = z/dist;
        }
    }

    public String toString(){
        return x + " " + y + " " + z;
    }

}
