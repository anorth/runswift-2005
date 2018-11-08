/*

   Copyright 2004 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
   must be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/*
 * Last modification background information
 * $Id: PointProjection.h 7369 2005-07-07 05:04:32Z amsi929 $
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
**/



#ifndef _POINT_PROJECTION_H_ 
#define _POINT_PROJECTION_H_ 

#include <iostream> 
#include "../share/Common.h"
#include "../share/VisionDef.h"
#include "../actuatorControl/legsGeometry.h"
#include <cmath>


/* Daniel 2004 */
/* Return 2 points that define the horizon. Also state whether the "up" or
 * "down" side is the "sky". Also state whether a horizon is found.
 */
static /*inline*/ bool getHorizon(

        double *hzArray,            // output: 4 doubles, 2 (x,y) of horizons. 
        bool   *skyIsUp,            // output
        bool   *allAboveHorizon,    // output

        double panx,
        double tilt,
        double crane,

        double face_length,
        double neck_length,
        double neck_base_height,
        double aperture_distance,
        double camera_offset,

        bool debugOutput) {

    *allAboveHorizon = false; 
    *skyIsUp         = true; 

    /* pan tilt crane from motor sensors. */
    panx = DEG2RAD(panx);
    tilt = DEG2RAD(tilt);
    crane = DEG2RAD(-crane); // -ive because we take looking down as positive
    // below, which is opposite of what really is.

    /* z component of the ipy vector on image plane. */
    double projIPY_Z = -sin(crane) * cos(panx) * sin(tilt)
        + cos(crane) * cos(tilt);

    double CameraHeight = -face_length *cos(crane) * cos(panx) * sin(tilt)
        - face_length *sin(crane) * cos(tilt)
        + neck_base_height
        + neck_length *cos(tilt)
        - camera_offset *projIPY_Z;



    /* Equations below are derived from the Zi equation - 
       calcualte the ip_x/ip_y value when Z is 0. See Daniel's report for
       detail. */


    double horizon_y_fix_x; 
    double left_horizon_y;
    double right_horizon_y;

    /* Check the left side of the cplane, fixing ipx = -(WIDTH/2). */
    horizon_y_fix_x = -(CPLANE_WIDTH/2);
    left_horizon_y = 
        ( (horizon_y_fix_x *sin(panx) * sin(tilt)
           + (face_length + aperture_distance) 
           * (cos(crane) * cos(panx) * sin(tilt) + sin(crane) * cos(tilt))
           - neck_base_height
           - neck_length *cos(tilt)
           + CameraHeight
          ) / projIPY_Z
        ) + camera_offset;

    /* Check the right side of the cplane, fixing ipx = (WIDTH/2). */
    horizon_y_fix_x = CPLANE_WIDTH/2;
    right_horizon_y = 
        ( (horizon_y_fix_x *sin(panx) * sin(tilt)
           + (face_length + aperture_distance) 
           * (cos(crane) * cos(panx) * sin(tilt) + sin(crane) * cos(tilt))
           - neck_base_height
           - neck_length *cos(tilt)
           + CameraHeight
          ) / projIPY_Z
        ) + camera_offset;


    double horizon_x_fix_y; 
    double top_horizon_x;
    double bottom_horizon_x;

    /* Check the top side of the cplane, fixing ipy = (HEIGHT/2). */
    horizon_x_fix_y = (CPLANE_HEIGHT/2);
    top_horizon_x =
        ((horizon_x_fix_y - camera_offset) * projIPY_Z
         - (face_length + aperture_distance) 
         *(cos(crane) * cos(panx) * sin(tilt) + sin(crane) * cos(tilt))
         + neck_base_height
         + neck_length *cos(tilt)
         - CameraHeight
        ) / (sin(panx) * sin(tilt)) ;


    /* Check the bottom side of the cplane, fixing ipy = -(HEIGHT/2). */
    horizon_x_fix_y = -(CPLANE_HEIGHT/2);
    bottom_horizon_x =
        ((horizon_x_fix_y - camera_offset) * projIPY_Z
         - (face_length + aperture_distance) 
         *(cos(crane) * cos(panx) * sin(tilt) + sin(crane) * cos(tilt))
         + neck_base_height
         + neck_length *cos(tilt)
         - CameraHeight
        ) / (sin(panx) * sin(tilt)) ;


    /* Now check if there is a horizon on screen, and which 2 points if there
     ** is. It is an horizon point if it is within the cplane range. */
    int  horizonCount = 0;

    if (ABS(left_horizon_y) <= (CPLANE_HEIGHT/2)) {

        hzArray[horizonCount*2]   = -(CPLANE_WIDTH/2); // *2 because 2 points.
        hzArray[horizonCount*2+1] = left_horizon_y;
        horizonCount++;
    }

    if (ABS(top_horizon_x) <= (CPLANE_WIDTH/2)) {

        hzArray[horizonCount*2]   = top_horizon_x;
        hzArray[horizonCount*2+1] = (CPLANE_HEIGHT/2);
        horizonCount++;
    }

    if (ABS(bottom_horizon_x) <= (CPLANE_WIDTH/2)) {

        hzArray[horizonCount*2]   = bottom_horizon_x;
        hzArray[horizonCount*2+1] = -(CPLANE_HEIGHT/2);
        horizonCount++;
    }

    if (ABS(right_horizon_y) <= (CPLANE_HEIGHT/2)) {

        hzArray[horizonCount*2]   = (CPLANE_WIDTH/2);
        hzArray[horizonCount*2+1] = right_horizon_y;
        horizonCount++;
    }

    /*
       cout << "horizon_x, ipy=-80 = " << bottom_horizon_x << endl;
       cout << "horizon_x, ipy= 80 = " << top_horizon_x << endl;
       cout << "horizon_y, ipx=-104 = " << left_horizon_y << endl;
       cout << "horizon_y, ipx= 104 = " << right_horizon_y << endl;
     */

    if (horizonCount == 2) {

        /* Now check also which way is up, in rare case the up side can be the
         ** ground. (eg. When tilt down and pan to left) */

        // Do this by checking a point just above the horizon, if it has a
        // greater Z value or not. */

        double test_ipx = hzArray[0],
        test_ipy = hzArray[1] + 5;

        double TestHeight = -test_ipx *sin(panx) * sin(tilt)
            + (test_ipy - camera_offset) * projIPY_Z
            - (face_length + aperture_distance) * 
            (cos(crane) * cos(panx) * sin(tilt) + sin(crane) * cos(tilt))
            + neck_base_height
            + neck_length *cos(tilt);

        *skyIsUp = (TestHeight > CameraHeight) ? true: false; 
        return true;

    } else if (horizonCount == 0) {

        /* Now there is no hz line no screen, check if whole screen is
         ** above/below horizon. */

        double TestHeight = + (- camera_offset) * projIPY_Z
            - (face_length + aperture_distance) * 
            (cos(crane) * cos(panx) * sin(tilt) + sin(crane) * cos(tilt))
            + neck_base_height
            + neck_length *cos(tilt);

        /* Test whether the center of cplane is within horizon. */
        *allAboveHorizon = (TestHeight > CameraHeight) ? true: false; 

        return false;

    } else {
        /* There can't be more than 2 horizon points! */
        cout << "Something's wrong in calculating horizon!" << endl;
        return false;
    }

    (void)debugOutput; // silence compiler warning
    return false;
}



/* getHeadElev by Daniel. */

/* pan: left +ive, right -ive.
 * tilt: up +ive, down -ive.
 * crane: up +ive, down -ive.
 * All in radian. 
 */
static inline bool getHeadElev(

        double *head,
        double *elev,

        double panx,
        double tilt,
        double crane,

        double ipx,
        double ipy,

        double height_above_base, // base means neck base for now. 

        double face_length,
        double neck_length,
        double NECK_BASE_HEIGHT,
        double aperture_distance,
        double camera_offset,

        bool debugOutput) {

    /* pan tilt crane from motor sensors. */
    //	crane = -crane; // Don't know why -ive??


    panx = DEG2RAD(panx);
    tilt = DEG2RAD(tilt);
    crane = DEG2RAD(-crane); // Don't know why -ive??

    /* xyz component of the ipy vector on image plane. */
    double projIPY_X = -sin(crane) * sin(panx);

    double projIPY_Y = sin(crane) * cos(panx) * cos(tilt)
        + cos(crane) * sin(tilt); // In the report, cos(crane) is cos(pan)!!

    double projIPY_Z = -sin(crane) * cos(panx) * sin(tilt)
        + cos(crane) * cos(tilt);


    /* The coordinates of camera after effect of pan, tilt and crane. */
    // Don't know why they're different to the ones in report?? Just use for
    // now.  
    double Xd = -face_length *cos(crane) * sin(panx) - camera_offset *projIPY_X;

    double Yd = face_length *cos(crane) * cos(panx) * cos(tilt)
        - face_length *sin(crane) * sin(tilt)
        + neck_length *sin(tilt)
        - camera_offset *projIPY_Y;

    double Zd = -face_length *cos(crane) * cos(panx) * sin(tilt)
        - face_length *sin(crane) * cos(tilt)
        + NECK_BASE_HEIGHT
        + neck_length *cos(tilt)
        - camera_offset *projIPY_Z;

    /* TODO: Calculations prior to this point can be calculated in TrigInit, so that
       they don't get calculated a few times in a frame. */

    /* The co-ordinate of the point in "projected" cplane - read
     ** Daniel/Kim's report. */
    double Xi = ipx *cos(panx)
        + (ipy - camera_offset) * projIPY_X
        - (face_length + aperture_distance) * cos(crane) * sin(panx) ;

    double Yi = ipx *sin(panx) * cos(tilt)
        + (ipy - camera_offset) * projIPY_Y
        + (face_length + aperture_distance) * (cos(crane) * cos(panx) * cos(tilt)
                - sin(crane) * sin(tilt))
        + neck_length *sin(tilt);

    double Zi = -ipx *sin(panx) * sin(tilt)
        + (ipy - camera_offset) * projIPY_Z
        - (face_length + aperture_distance) * (cos(crane) * cos(panx) * sin(tilt)
                + sin(crane) * cos(tilt))
        + NECK_BASE_HEIGHT
        + neck_length *cos(tilt);


    double Zf = height_above_base;
    double Xf = Xi + ((Xi-Xd) * ((Zi-Zf)/(Zd-Zi))); 
    double Yf = Yi + ((Yi-Yd) * ((Zi-Zf)/(Zd-Zi))); 

    double projZf = sqrt((Xf*Xf)+(Yf*Yf));

    *head = RAD2DEG(atan(Xf/Yf));
    *elev = RAD2DEG(atan(Zf/projZf));

    /* Check whether (Xi, Yi) and (Xf, Yf) are in the same "quadrant" (ie.
     ** Xi and Xf, Yi and Yf must have the same sign). If not,
     ** the elevation is not reliable because the projected location lies in
     ** somewhere the dog cannot see. */

    if (debugOutput) {
        cout << endl; 
        cout << "panx = " << RAD2DEG(panx) << endl;
        cout << "tilt = " << RAD2DEG(tilt) << endl;
        cout << "crane = " << RAD2DEG(crane) << endl;
        cout << endl; 
        cout << "ipx: " << ipx << endl;
        cout << "ipy: " << ipy << endl;
        cout << endl; 
        cout << "Xd: " << Xd << endl;
        cout << "Yd: " << Yd << endl;
        cout << "Zd: " << Zd << endl;
        cout << endl; 
        cout << "Xi: " << Xi << endl;
        cout << "Yi: " << Yi << endl;
        cout << "Zi: " << Zi << endl;
        cout << endl; 
        cout << "Xf: " << Xf << endl;
        cout << "Yf: " << Yf << endl;
        cout << "Zf: " << Zf << endl;
        cout << endl; 
        cout << "head: " << *head << endl;
        cout << "elev: " << *elev << endl;
    }

    if ((Xi * Xf < 0) || (Yi * Yf < 0)) {
        if (debugOutput) {
            cout << endl;
            cout << "Projected location outside view scope - impossible!" << endl;
        }
        return false;
    }

    return true;
}


/******************************************************************************/
/* Get point projection of multiple points at a time by Andrew */
/* Use same calculation as getPointProjection by Kim*/
/* Much more speed optimised than getPointProjection*/
/* Testing differences are located in VisualCortex projectPoints */
/* Note: If the point is not able to be projected it will return VERY_LARGE_DOUBLE*/
static /*inline*/ bool getMultiPointProjection(
        double *inPoints,
        double *outPoints,
        unsigned numPoints,
        double panx,
        double tilt,
        double crane,
        double face_length,
        double neck_length,
        double NECK_BASE_HEIGHT,
        double aperture_distance,
        double camera_offset,
        bool debugOutput) 
{
    panx = DEG2RAD(panx);
    tilt = DEG2RAD(tilt);
    crane = DEG2RAD(-crane);
    double sinCrane = sin(crane);
    double cosCrane = cos(crane);
    double sinPanx = sin(panx);
    double cosPanx = cos(panx);
    double sinTilt = sin(tilt);
    double cosTilt = cos(tilt);
    
    double projIPY_X = -sinCrane * sinPanx;//-sin(crane) * sin(panx);

    double projIPY_Y = sinCrane * cosPanx * cosTilt //sin(crane) * cos(panx) * cos(tilt)
        + cosCrane * sinTilt;//+ cos(crane) * sin(tilt);

    double projIPY_Z = -sinCrane * cosPanx * sinTilt//-sin(crane) * cos(panx) * sin(tilt)
        + cosCrane * cosTilt;//+ cos(crane) * cos(tilt);

    double Xc = 
        -face_length * cosCrane * sinPanx - camera_offset * projIPY_X;
        //-face_length *cos(crane) * sin(panx) - camera_offset * projIPY_X;

    double Yc = 
        face_length * cosCrane * cosPanx * cosTilt
        //face_length *cos(crane) * cos(panx) * cos(tilt)
        - face_length * sinCrane * sinTilt
        //- face_length *sin(crane) * sin(tilt)
        + neck_length * sinTilt
        //+ neck_length *sin(tilt)
        - camera_offset * projIPY_Y;

    double Zc = 
        -face_length * cosCrane * cosPanx * sinTilt
        //-face_length *cos(crane) * cos(panx) * sin(tilt)
        - face_length * sinCrane * cosTilt
        //- face_length *sin(crane) * cos(tilt)
        + NECK_BASE_HEIGHT
        + neck_length * cosTilt
        //+ neck_length * cos(tilt)
        - camera_offset * projIPY_Z;
        
    double XiHelper = 
        -(face_length + aperture_distance) * cosCrane * sinPanx;
        //-(face_length + aperture_distance) * cos(crane) * sin(panx);
    double YiHelper =
        + (face_length + aperture_distance) * (cosCrane * cosPanx * cosTilt
        //+ (face_length + aperture_distance) * (cos(crane) * cos(panx) * cos(tilt)
        - sinCrane * sinTilt)
        //- sin(crane) * sin(tilt))
        + neck_length * sinTilt;
        //+ neck_length *sin(tilt);
        
    double ZiHelper =
        - (face_length + aperture_distance) * (cosCrane * cosPanx * sinTilt
        //- (face_length + aperture_distance) * (cos(crane) * cos(panx) * sin(tilt)
        + sinCrane * cosTilt)
        //+ sin(crane) * cos(tilt))
        + NECK_BASE_HEIGHT
        + neck_length * cosTilt;
        //+ neck_length *cos(tilt);

    for (unsigned i = 0; i < numPoints; i++ )
    {
        double ipx = inPoints[i*2];
        double ipy = inPoints[i*2 + 1];
        
        double Xi = 
            ipx * cosPanx
            //ipx *cos(panx)
            + (ipy - camera_offset) * projIPY_X
            + XiHelper; 
        /* move to XiHelper
            - (face_length + aperture_distance) * cos(crane) * sin(panx) ;
        */
        double Yi = 
            ipx * sinPanx * cosTilt
            //ipx *sin(panx) * cos(tilt)
            + (ipy - camera_offset) * projIPY_Y
            + YiHelper;
        /* move to YiHelper
            + (face_length + aperture_distance) * (cos(crane) * cos(panx) * cos(tilt)
                    - sin(crane) * sin(tilt))
            + neck_length *sin(tilt);
        */
        double Zi = 
            -ipx * sinPanx * sinTilt
            //-ipx *sin(panx) * sin(tilt)
            + (ipy - camera_offset) * projIPY_Z
            + ZiHelper;
        /* move to YiHelper            
            - (face_length + aperture_distance) * (cos(crane) * cos(panx) * sin(tilt)
                    + sin(crane) * cos(tilt))
            + NECK_BASE_HEIGHT
            + neck_length *cos(tilt);
        */
        if (debugOutput) {
            cout << endl;
            cout << "Ipx = " << ipx << endl;
            cout << "Ipy = " << ipy << endl;
            cout << "panx = " << panx << endl;
            cout << "tilt = " << tilt << endl;
            cout << "crane = " << crane << endl;
            cout << "face_length = " << face_length << endl;
            cout << "neck_length = " << neck_length << endl;
            cout << "NECK_BASE_HEIGHT = " << NECK_BASE_HEIGHT << endl;
            cout << "aperture_distance = " << aperture_distance << endl;
            cout << "camera_offset = " << camera_offset << endl;
            cout << "ProjIPY_X = " << projIPY_X << endl;
            cout << "ProjIPY_Y = " << projIPY_Y << endl;
            cout << "ProjIPY_Z = " << projIPY_Z << endl;
            cout << "Xi = " << Xi << endl;
            cout << "Yi = " << Yi << endl;
            cout << "Zi = " << Zi << endl << endl;
            cout << "Xc = " << Xc << endl;
            cout << "Yc = " << Yc << endl;
            cout << "Zc = " << Zc << endl;
        }
        if (Zc - Zi < EPSILON) {
            //the ray going up away from XY Plane, cannot intersect
            outPoints[i*2] = VERY_LARGE_DOUBLE;
            outPoints[i*2 + 1] = VERY_LARGE_DOUBLE;
    
            if (debugOutput) {
                cout << "Result not on ground" << endl;
                cout << endl;
            }
        }
        else {
            double alpha = Zi / (Zi - Zc);
            outPoints[i*2] = Xi + alpha * (Xc - Xi);
            outPoints[i*2 + 1] = Yi + alpha * (Yc - Yi);
    
            if (debugOutput) {
                cout << "Result x = " << outPoints[i*2] << endl;
                cout << "Result y = " << outPoints[i*2 + 1] << endl;
                cout << endl;
            }
        }
    }
    return true;
}

/******************************************************************************/
static inline bool getMultiPointProjection(
        double *inPoints,
        double *outPoints,
        unsigned numPoints,
        double panx,
        double tilt,
        double crane,
        double face_length,
        double neck_length,
        double NECK_BASE_HEIGHT,
        double aperture_distance,
        double camera_offset) 
{
    return getMultiPointProjection(
        inPoints,
        outPoints,
        numPoints,
        panx,
        tilt,
        crane,
        face_length,
        neck_length,
        NECK_BASE_HEIGHT,
        aperture_distance,
        camera_offset,
        false);
}

/******************************************************************************/
/* getPointProjection by Kim. Calculation is correct but we should not
 ** calculate the coordinate and uint vector of the camera for EACH FUNCTION
 ** CALL because camera position in the same frame is fixed.*/ 

// Calculate point projection of image coordinate onto X-Y plane
// given panx,tilt and crane (tilt2) in DEGREE !!!
// return true if successful, false if not
// To project to different XY plane ( different z value),
// simply change NECK_BASE_HEIGHT.
static /*inline*/ bool getPointProjection(double *x,
        double *y,
        double panx,
        double tilt,
        double crane,
        double ipx,
        double ipy,
        double face_length,
        double neck_length,
        double NECK_BASE_HEIGHT,
        double aperture_distance,
        double camera_offset,
        bool debugOutput) {

#if 0
    // Tested by Andrew, it doesn't show correct projection.
    // Will's Point Projection routine
    
    if (debugOutput) {
        cout << endl;
        cout << "Ipx = " << ipx << endl;
        cout << "Ipy = " << ipy << endl;
        cout << "panx = " << panx << endl;
        cout << "tilt = " << tilt << endl;
        cout << "crane = " << crane << endl;
    }

    panx = DEG2RAD(-panx);
    tilt = DEG2RAD(-tilt);
    crane = DEG2RAD(crane);

    XYZ_Coord head(0, face_length, -camera_offset);
    XYZ_Coord neck(0, 0, neck_length);
    XYZ_Coord stance(0, 0, NECK_BASE_HEIGHT);
    XYZ_Coord vec(-ipy, aperture_distance, ipx);
    // vec.normalize();
    XYZ_Coord camPt;

    if (debugOutput) {
        cout << "head = " << head << endl;
        cout << "neck = " << neck << endl;
        cout << "stance = " << stance << endl;
        cout << "vec = " << vec << endl;
    }

    // tanslate back to top of head

    camPt.add(head);

    if (debugOutput) {
        cout << "Top of Head: " << camPt << ", " << vec << endl;
    }

    // Handle crane

    camPt.rotateYZ(crane);
    vec.rotateYZ(crane);

    if (debugOutput) {
        cout << "After crane: " << camPt << ", " << vec << endl;
    }

    // handle pan

    camPt.rotateXY(panx);
    vec.rotateXY(panx);

    if (debugOutput) {
        cout << "After pan: " << camPt << ", " << vec << endl;
    }

    // tanslate to base of neck

    camPt.add(neck);

    if (debugOutput) {
        cout << "Base of neck: " << camPt << ", " << vec << endl;
    }

    // handle tilt

    camPt.rotateYZ(tilt);
    vec.rotateYZ(tilt);

    if (debugOutput) {
        cout << "After tilt: " << camPt << ", " << vec << endl;
    }

    // relative to ground

    camPt.add(stance);	

    if (debugOutput) {
        cout << "After stance: " << camPt << ", " << vec << endl;
    }

    // find how far we have to go to hit the ground plane

    double dist = camPt.z/-vec.z;

    camPt.addMult(vec, dist);

    if (debugOutput) {
        cout << "Dist: " << dist << endl;
        cout << "ground: " << camPt << endl;
    }

    *x = camPt.x;
    *y = camPt.y;

    return (dist >= 0);

#else
    // Kim's Point Projection Routine

    panx = DEG2RAD(panx);
    tilt = DEG2RAD(tilt);
    crane = DEG2RAD(-crane);

    double projIPY_X = -sin(crane) * sin(panx);

    double projIPY_Y = sin(crane) * cos(panx) * cos(tilt)
        + cos(crane) * sin(tilt);

    double projIPY_Z = -sin(crane) * cos(panx) * sin(tilt)
        + cos(crane) * cos(tilt);



    double Xc = -face_length *cos(crane) * sin(panx) - camera_offset *projIPY_X;

    double Yc = face_length *cos(crane) * cos(panx) * cos(tilt)
        - face_length *sin(crane) * sin(tilt)
        + neck_length *sin(tilt)
        - camera_offset *projIPY_Y;

    double Zc = -face_length *cos(crane) * cos(panx) * sin(tilt)
        - face_length *sin(crane) * cos(tilt)
        + NECK_BASE_HEIGHT
        + neck_length *cos(tilt)
        - camera_offset *projIPY_Z;


    double Xi = ipx *cos(panx)
        + (ipy - camera_offset) * projIPY_X
        - (face_length + aperture_distance) * cos(crane) * sin(panx) ;

    double Yi = ipx *sin(panx) * cos(tilt)
        + (ipy - camera_offset) * projIPY_Y
        + (face_length + aperture_distance) * (cos(crane) * cos(panx) * cos(tilt)
                - sin(crane) * sin(tilt))
        + neck_length *sin(tilt);

    double Zi = -ipx *sin(panx) * sin(tilt)
        + (ipy - camera_offset) * projIPY_Z
        - (face_length + aperture_distance) * (cos(crane) * cos(panx) * sin(tilt)
                + sin(crane) * cos(tilt))
        + NECK_BASE_HEIGHT
        + neck_length *cos(tilt);

    if (debugOutput) {
        cout << endl;
        cout << "Ipx = " << ipx << endl;
        cout << "Ipy = " << ipy << endl;
        cout << "panx = " << panx << endl;
        cout << "tilt = " << tilt << endl;
        cout << "crane = " << crane << endl;
        cout << "face_length = " << face_length << endl;
        cout << "neck_length = " << neck_length << endl;
        cout << "NECK_BASE_HEIGHT = " << NECK_BASE_HEIGHT << endl;
        cout << "aperture_distance = " << aperture_distance << endl;
        cout << "camera_offset = " << camera_offset << endl;
        cout << "ProjIPY_X = " << projIPY_X << endl;
        cout << "ProjIPY_Y = " << projIPY_Y << endl;
        cout << "ProjIPY_Z = " << projIPY_Z << endl;
        cout << "Xi = " << Xi << endl;
        cout << "Yi = " << Yi << endl;
        cout << "Zi = " << Zi << endl << endl;
        cout << "Xc = " << Xc << endl;
        cout << "Yc = " << Yc << endl;
        cout << "Zc = " << Zc << endl;
    }
    if (Zc - Zi < EPSILON) {
        //the ray going up away from XY Plane, cannot intersect
        *x = VERY_LARGE_DOUBLE;
        *y = VERY_LARGE_DOUBLE;

        if (debugOutput) {
            cout << "Result not on ground" << endl;
            cout << endl;
        }

        return false;
    }
    else {
        double alpha = Zi / (Zi - Zc);
        *x = Xi + alpha * (Xc - Xi);
        *y = Yi + alpha * (Yc - Yi);

        if (debugOutput) {
            cout << "Result x = " << *x << endl;
            cout << "Result y = " << *y << endl;
            cout << endl;
        }

        return true;
    }
#endif    
}

static inline bool getPointProjection(double *x,
        double *y,
        double panx,
        double tilt,
        double crane,
        double ipx,
        double ipy,
        double face_length,
        double neck_length,
        double NECK_BASE_HEIGHT,
        double aperture_distance,
        double camera_offset){
    return getPointProjection(x,y,panx,tilt,crane,ipx,ipy, face_length,
            neck_length, NECK_BASE_HEIGHT, aperture_distance, camera_offset,
            false);
}

//Kim: calculate Neck base height from body's tilt. Note that there is a distance between neck center
//and shoulder
static inline double getNeckBaseHeightFromStance(double shoulderHeight, double hipHeight) {
    double bodytilt = asin((hipHeight - shoulderHeight) / BODY_LENGTH);
    return shoulderHeight + NECKBASE_SHOULDER_DISTANCE * cos(bodytilt);
}

//cut down version of above function, for convenience
//level is the hieght from the ground, hence the formular depends on the "stance" of the dog
//level is in milicentimeter
static inline bool getPointProjection(double *x,
        double *y,
        double panx,
        double tilt,
        double crane,
        double ipx,
        double ipy,
        double level,
        double frontHeight,
        double backHeight){
    double neck_base_height = getNeckBaseHeightFromStance(frontHeight, backHeight);
    return getPointProjection(x,y,panx, tilt, crane, ipx, ipy, FACE_LENGTH, NECK_LENGTH, neck_base_height - level , APERTURE_DISTANCE, CAMERA_OFFSET ); 
}

static inline double getElevationFromVector(double x,double y, double z){
    double dist = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));
    double temp1 = sqrt(SQUARE(x) + SQUARE(y));
    double temp3 = acos(temp1 / dist);

    if (z >= 0) 
        return RAD2DEG(temp3);
    else 
        return -RAD2DEG(temp3);
}

static inline double getElevation( double panx,
        double tilt,
        double crane,
        double ipx,
        double ipy,
        double face_length,
        double neck_length,
        double aperture_distance,
        double camera_offset){

    panx = DEG2RAD(panx);
    tilt = DEG2RAD(tilt);
    crane = DEG2RAD(-crane);

    double projIPY_X = -sin(crane) * sin(panx);

    double projIPY_Y = sin(crane) * cos(panx) * cos(tilt)
        + cos(crane) * sin(tilt);

    double projIPY_Z = -sin(crane) * cos(panx) * sin(tilt)
        + cos(crane) * cos(tilt);



    double Xc = -face_length *cos(crane) * sin(panx) - camera_offset *projIPY_X;

    double Yc = face_length *cos(crane) * cos(panx) * cos(tilt)
        - face_length *sin(crane) * sin(tilt)
        + neck_length *sin(tilt)
        - camera_offset *projIPY_Y;

    double Zc = -face_length *cos(crane) * cos(panx) * sin(tilt)
        - face_length *sin(crane) * cos(tilt)
        + neck_length *cos(tilt)
        - camera_offset *projIPY_Z;


    double Xi = ipx *cos(panx)
        + (ipy - camera_offset) * projIPY_X
        - (face_length + aperture_distance) * cos(crane) * sin(panx) ;

    double Yi = ipx *sin(panx) * cos(tilt)
        + (ipy - camera_offset) * projIPY_Y
        + (face_length + aperture_distance) * (cos(crane) * cos(panx) * cos(tilt)
                - sin(crane) * sin(tilt))
        + neck_length *sin(tilt);

    double Zi = -ipx *sin(panx) * sin(tilt)
        + (ipy - camera_offset) * projIPY_Z
        - (face_length + aperture_distance) * (cos(crane) * cos(panx) * sin(tilt)
                + sin(crane) * cos(tilt))
        + neck_length *cos(tilt);

    double dx = Xi - Xc;
    double dy = Yi - Yc;
    double dz = Zi - Zc;
    return getElevationFromVector(dx,dy,dz);
}

static inline double getHeightKnowingDistance( double panx,
        double tilt,
        double crane,
        double ipx,
        double ipy,
        double distance,
        double face_length,
        double neck_length,
        double aperture_distance,
        double camera_offset){

    panx = DEG2RAD(panx);
    tilt = DEG2RAD(tilt);
    crane = DEG2RAD(-crane);

    double projIPY_X = -sin(crane) * sin(panx);

    double projIPY_Y = sin(crane) * cos(panx) * cos(tilt)
        + cos(crane) * sin(tilt);

    double projIPY_Z = -sin(crane) * cos(panx) * sin(tilt)
        + cos(crane) * cos(tilt);



    double Xc = -face_length *cos(crane) * sin(panx) - camera_offset *projIPY_X;

    double Yc = face_length *cos(crane) * cos(panx) * cos(tilt)
        - face_length *sin(crane) * sin(tilt)
        + neck_length *sin(tilt)
        - camera_offset *projIPY_Y;

    double Zc = -face_length *cos(crane) * cos(panx) * sin(tilt)
        - face_length *sin(crane) * cos(tilt)
        + neck_length *cos(tilt)
        - camera_offset *projIPY_Z;


    double Xi = ipx *cos(panx)
        + (ipy - camera_offset) * projIPY_X
        - (face_length + aperture_distance) * cos(crane) * sin(panx) ;

    double Yi = ipx *sin(panx) * cos(tilt)
        + (ipy - camera_offset) * projIPY_Y
        + (face_length + aperture_distance) * (cos(crane) * cos(panx) * cos(tilt)
                - sin(crane) * sin(tilt))
        + neck_length *sin(tilt);

    double Zi = -ipx *sin(panx) * sin(tilt)
        + (ipy - camera_offset) * projIPY_Z
        - (face_length + aperture_distance) * (cos(crane) * cos(panx) * sin(tilt)
                + sin(crane) * cos(tilt))
        + neck_length *cos(tilt);

    double dx = Xi - Xc;
    double dy = Yi - Yc;
    double dz = Zi - Zc;

    double elev = getElevationFromVector(dx,dy,dz);
    //cout << " Zc = " << Zc << " H = " << sin(DEG2RAD(elev)) * distance << endl; 

    return Zc + sin(DEG2RAD(elev)) * distance; 


}

#endif // _POINT_PROJECTION_H_
