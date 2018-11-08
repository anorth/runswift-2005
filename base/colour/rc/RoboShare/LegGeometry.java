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


/** @hard todo
 * DEBUG kinematics formular for bodyTilt & neckHeight (sothat it's something like 12 deg)
 * xyz of the rear leg is still not  right even when standing still
 *  */

package RoboShare;

/**
 *
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1:
*  CHANGE LOG:
*  + version 1.1:
*        - Change legs configuration sothat the paw is the actual contact point
*        with the ground, when the dog is standing still.
*        - Attempted to workout the bodyTilt, shoulderHeight and bodyRoll from
*        joint sensors (forward kinematics)
 */

public class LegGeometry {

    static public class JOINT_Values {
        public double rotator, shoulder, knee;
        public JOINT_Values(double theta1, double theta2, double theta3) {
            rotator = theta1;
            shoulder = theta2;
            knee = theta3;
            toRadians();
        }

        public void toRadians() {
            rotator = Utils.radians(rotator);
            shoulder = Utils.radians(shoulder);
            knee = Utils.radians(knee);
        }

    };

    //only care about left legs, right legs are mirrored
//    static final XYZ_Coord FrontElbowToFinger = new XYZ_Coord(28.3, 71.5, 0.0);
//    static final XYZ_Coord FrontShoulderToElbow = new XYZ_Coord(9, 69.5, 4.7);
//    static final XYZ_Coord RearElbowToFinger = new XYZ_Coord( -21.3, 76.5, 0.0);
//    static final XYZ_Coord RearShoulderToElbow = new XYZ_Coord( -9, 69.5, 4.7);

    static final XYZ_Coord FrontElbowToFinger = new XYZ_Coord(-9, 27.0, -19.0); //estimated contact point
    static final XYZ_Coord FrontShoulderToElbow = new XYZ_Coord(9, 69.5, 4.7);

    static final XYZ_Coord RearElbowToFinger = new XYZ_Coord( 9.0, 69.29, 0.0);
    static final XYZ_Coord RearShoulderToElbow = new XYZ_Coord( -9, 69.5, 4.7);


    static private void kinematics(final JOINT_Values theta,
                                   final XYZ_Coord ElbowToFinger,
                                   final XYZ_Coord ShoulderToElbow,
                                   XYZ_Coord pt) {
        pt.copy(ElbowToFinger);
        pt.rotateXY(-theta.knee);
        pt.add(ShoulderToElbow);
        pt.rotateYZ( -theta.shoulder);
        pt.rotateXY(-theta.rotator);
    }

    public static void kinematicsFrontLeft(final JOINT_Values theta,
                                           XYZ_Coord pt) {
        //due to our convention: +alpha = counter clockwise
        theta.knee *= -1;
        theta.rotator *= -1;
        kinematics(theta, FrontElbowToFinger, FrontShoulderToElbow, pt);
    }

    public static void kinematicsRearLeft(final JOINT_Values theta,
                                          XYZ_Coord pt) {
        kinematics(theta, RearElbowToFinger, RearShoulderToElbow, pt);
    }

    public static void kinematicsFrontRight(final JOINT_Values theta,
                                            XYZ_Coord pt) {
        kinematicsFrontLeft(theta, pt);
        pt.z *= -1; //mirror
    }

    public static void kinematicsRearRight(final JOINT_Values theta,
                                           XYZ_Coord pt) {
        kinematicsRearLeft(theta, pt);
        pt.z *= -1; //mirror
    }

    public static Point3D convertToCenterCoordinateFL(XYZ_Coord xyzFrontLeft) {
        double cx = RobotDefinition.BODY_LENGTH / 2;
        double cz = RobotDefinition.BODY_WIDTH / 2;
        return new Point3D(xyzFrontLeft.x + cx,
                           xyzFrontLeft.y,
                           xyzFrontLeft.z + cz);
    }

    public static Point3D convertToCenterCoordinateFR(XYZ_Coord xyzFrontRight) {
        double cx = RobotDefinition.BODY_LENGTH / 2;
        double cz = RobotDefinition.BODY_WIDTH / 2;
        return new Point3D(xyzFrontRight.x + cx,
                           xyzFrontRight.y,
                           xyzFrontRight.z - cz);
    }

    public static Point3D convertToCenterCoordinateRL(XYZ_Coord xyzRearLeft) {
        double cx = RobotDefinition.BODY_LENGTH / 2;
        double cz = RobotDefinition.BODY_WIDTH / 2;
        return new Point3D(xyzRearLeft.x - cx,
                           xyzRearLeft.y,
                           xyzRearLeft.z + cz);
    }

    public static Point3D convertToCenterCoordinateRR(XYZ_Coord xyzRearRight) {
        double cx = RobotDefinition.BODY_LENGTH / 2;
        double cz = RobotDefinition.BODY_WIDTH / 2;
        return new Point3D(xyzRearRight.x - cx,
                           xyzRearRight.y,
                           xyzRearRight.z - cz);
    }

    public static double getFrontHeight(Plane3D p) {
        return p.distance(new Point3D(RobotDefinition.BODY_LENGTH / 2, 0, 0));
    }

    public static double getRearHeight(Plane3D p) {
        return p.distance(new Point3D(-RobotDefinition.BODY_LENGTH / 2, 0, 0));
    }

    public static double getBodyTilt(double frontHeight, double rearHeight){
        return Math.toDegrees(Math.asin( Math.abs(rearHeight - frontHeight) / RobotDefinition.BODY_LENGTH));
    }

}
