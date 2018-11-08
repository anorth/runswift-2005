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

/** @idea_todo
*  Scale horizontal and vertical dimension.
*  Because the camera is  distorted : tan(horizontal angle) / tan(vertical angle) = 1.5 != width / height = 1.3
*  */

/**
 *
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.1
*  CHANGE LOG:
*  + Version 1.1:
*        - Add getRollAngle: calculate roll from horizon
 */

public class HeadGeometry {

    static private int[] binarySearchHorizon(double x1, double y1, double x2,
                                      double y2, Matrix camera2World) {
        double elev1 = Utils.getElevation(x1, y1, camera2World);
        double elev2 = Utils.getElevation(x2, y2, camera2World);
        while (Math.abs(x1 - x2) + Math.abs(y1 - y2) > 0.5) {
            double x = (x1 + x2) / 2;
            double y = (y1 + y2) / 2;
            double elev = Utils.getElevation(x, y, camera2World);
            if (elev * elev1 > 0) {
                elev1 = elev;
                x1 = x;
                y1 = y;
            }
            else {
                elev2 = elev;
                x2 = x;
                y2 = y;
            }
        }
        x1 = Math.round(x1);
        y1 = Math.round(y1);
        return new int[] {
             (int) x1, (int) y1};
    }

    public static int[] calculateHorizon(Matrix camera2World) {

        double mx = RobotDefinition.CPLANE_WIDTH / 2;
        double my = RobotDefinition.CPLANE_HEIGHT / 2;
        double[][] corner = { {-mx, -my} ,  {mx, -my} , { mx, my} , { -mx, my}};

        int[] result = new int[4];
        int k = 0;
        for (int i = 0; i < 4; i++) {
            int j = (i + 1) % 4;
            if (Utils.getElevation(corner[i][0], corner[i][1], camera2World)
                * Utils.getElevation(corner[j][0], corner[j][1], camera2World) <
                0) {
                int[] res = binarySearchHorizon(corner[i][0], corner[i][1],
                                                corner[j][0], corner[j][1],
                                                camera2World);
                result[k++] = res[0];
                result[k++] = res[1];
                if (k == 4)break;
            }
        }
        if (k == 4) return result;
        else return null;
    }

    public static double getRollAngleRad(Matrix camera2World){
        Matrix camera2WorldTranslated = TransformationMatrix.translateToZero(camera2World);
        Matrix world2Camera = camera2WorldTranslated.transpose();

        Vector3D vLeft = new Vector3D(1,0,0);
        Vector3D vForward = new Vector3D(0,1,0);

        vLeft.transform(world2Camera);
        vForward.transform(world2Camera);

        Vector3D nV = vLeft.cross(vForward);
        Vector3D horizonVector = nV.cross( new Vector3D(0,1,0) );
        return Math.atan2(horizonVector.getZ(), horizonVector.getX() ) ;

    }

    //warning : return -180 if camera plane parallel to the ground
    public static double getRollAngle(Matrix camera2World){
        return Utils.degrees(getRollAngleRad(camera2World));
    }

}
