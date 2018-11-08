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

//ERS7
public class RobotDefinition {
    //VISION
    public static final int CPLANE_HEIGHT = 160;
    public static final int CPLANE_WIDTH = 208;
    public static final double VERTICAL_VIEW_ANGLE_DEG = 45.2;
    public static final double VERTICAL_VIEW_ANGLE_RAD = Utils.radians(VERTICAL_VIEW_ANGLE_DEG);
    public static final double APERTURE_DISTANCE = CPLANE_HEIGHT / (2 * Math.tan(VERTICAL_VIEW_ANGLE_RAD / 2) ) ;

    //ACTUATOR

    //in millimeters

    public static final double BODY_LENGTH = 130.0;
    public static final double BODY_WIDTH = 125.0;

    public static final double NECK_LENGTH = 80.0;

    //Distance from head tilt center to camera PLANE
    public static final double CAMERA_OFFSET_Y = 81.06;
    public static final double CAMERA_OFFSET_Z = 14.6;


    //temporary variables
    public static double neckHeight = 100;
    public static double bodyTilt = 13.5;
}
