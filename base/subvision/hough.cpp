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


#include <iostream>
//#include <math.h>
#include "hough.h"

using namespace std;
// This function computes the hough transform and populates 
// the accumulator array

void HoughLine::ComputeTransform(double * edgepoints, int size)
{
    cerr << __func__ <<endl;
    if (size < 1)
        return;
    int t, r, x, y, i;

    // and max_theta is set to 360 degrees

    h_width = max_degrees;
    h_height = max_radius;
    cout << __func__ << " max radius/2:" << max_radius/2 << endl;
    // Loop through edge pixel
    for (i = 0; i < size; i++)
    {
        x = (int) TOPLEFT2CENTER_X(edgepoints[i*2]);
        y = (int) TOPLEFT2CENTER_Y(edgepoints[i*2 + 1]);
        if (i == 0)
            cout << __func__ << " x:" << x << " y:" << y <<endl;
        // Draw the curve in the Hough space
        for (t = 0; t < max_degrees; t++)
        {
                //r = (int)((x - CPLANE_WIDTH/2)*COS[t] + (y - CPLANE_HEIGHT/2)*SIN[t]);
                r = (int)((x)*COS[t] + (y)*SIN[t]);
                if ((r > -max_radius/2) && (r < max_radius/2) && (r != 0))
                        ++accumulator[(r+max_radius/2)*max_degrees + t];
                if (i == 0 && t == 0)
                    cout << __func__ << " r:" << r << " t:" << t <<endl;
                        
        }
    }
}

// This function detects lines in a given image
// by looking for maximas in the accumulator array

int HoughLine::DetectLines(double * edgepoints, int size , int *rt, int rtsize)
{
    cerr << __func__ <<endl;
    int rt_counter = 0;
    int r, t, dr, dt;
    int maxima, max_value = 0;
    
    if (size < 1 || rtsize < 1)
        return 0;
        
    ComputeTransform(edgepoints, size);

    

// Find the maximum accumulator value. We use this 
// later to consider only the top 30% of the values 
// when looking for local maximas

    for (r = 0; r < h_height; r++)
        for (t = 0; t < h_width; t++)
            if (accumulator[r * max_degrees + t] > max_value)
                max_value = accumulator[r * max_degrees + t];


    cout << __func__ << " best accumulator:" << max_value << endl;
// Scan the accumulator array for local maximas
// If one is found, draw the corresponding line in the 
// original image

    for (r = 4; r < h_height - 5; r++)
        for (t = 4; t < h_width - 5; t++)
        {
            if (accumulator[r * max_degrees + t] > 0.5 * max_value)
            {
                    maxima = 1;

                    // Check a 9x9 window
                    for (dr = r-4; dr < r+5; dr++)
                            for (dt = t-5; dt < t+5; dt++)
                                    if (accumulator[dr * max_degrees + dt] > accumulator[r * max_degrees + t])
                                            maxima = 0;


                    if (maxima == 1)
                    {   
                        std::cout << __func__ << "before r:" << r << " t:" << t <<std::endl;
                        rt[rt_counter*2] = r - max_radius/2;
                        rt[rt_counter*2 + 1] = t;
                        
                        std::cout << __func__ << " r:" << rt[rt_counter*2] << " t:" << rt[rt_counter*2 + 1] <<std::endl;
                        ++rt_counter;
                        if (rt_counter == rtsize)
                        {
                            return rt_counter;
                        }
                        
                    }
            }
        }
        return rt_counter;
}


// The constructor. Initializes the SIN and COS tables
HoughLine::HoughLine()
{
    cerr << __func__ <<endl;
    double theta;
    
    pi = 3.141592654;
    accumulator = new int[max_radius*max_degrees];
    
    // Fill in the SIN and COS lookup tables
    for (int t = 0; t < 360; t++)
    {
            theta = (t - 180) * pi / 180;
            SIN[t] = (float) sin(theta);
            COS[t] = (float) cos(theta);
    }
}


// The destructor. Frees all allocated memory
HoughLine::~HoughLine()
{
    cerr << "~" << __func__ <<endl;
    delete accumulator;
    cerr << "~" << __func__ << " end"<<endl;   
}



