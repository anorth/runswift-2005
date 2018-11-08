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

import RoboShare.*;

//
//  Edge.java
//  rc
//
// Sobel edge detector
//
//  Created by Claude Sammut on Sat Aug 28 2004.
//

public class Edge {
    int red[][] = new int[BFL.HEIGHT][BFL.WIDTH];
    int green[][] = new int[BFL.HEIGHT][BFL.WIDTH];
    int blue[][] = new int[BFL.HEIGHT][BFL.WIDTH];
    boolean edgeMap[][] = new boolean[BFL.HEIGHT][BFL.WIDTH];

    private static final int MASK_SIZE = 3;

    private static final int sobel_row_mask[][] = {
        {
         -1, -2, -1}
        , {
        0, 0, 0}
        , {
        1, 2, 1}
    };

    private static final int sobel_col_mask[][] = {
        {
         -1, 0, 1}
        , {
         -2, 0, 2}
        , {
         -1, 0, 1}
    };

    Edge(BFL bfl) {
        getRGB(bfl);
        sobel(300);
    }

    private void getRGB(BFL bfl) {
        for (int i = 0; i < BFL.HEIGHT; i++) {
            for (int j = 0; j < BFL.WIDTH; j++) {
                red[i][j] = (bfl.rgbPlane[i][j] >>> 16) & 0x0FF;
                green[i][j] = (bfl.rgbPlane[i][j] >>> 8) & 0x0FF;
                blue[i][j] = bfl.rgbPlane[i][j] & 0x0FF;
            }
        }
    }

    private void sobel(int threshold) {
        for (int i = 1; i < BFL.HEIGHT - 1; i++) {
            for (int j = 1; j < BFL.WIDTH - 1; j++) {
                double edge = sobel_edge_magnitude(red, i, j) +
                    sobel_edge_magnitude(green, i, j) +
                    sobel_edge_magnitude(blue, i, j);

                edgeMap[i][j] = edge > threshold;
            }
        }
    }

    private double sobel_edge_magnitude(int I[][], int i, int j) {
        int s1 = apply_sobel_mask(sobel_row_mask, I, i, j);
        int s2 = apply_sobel_mask(sobel_col_mask, I, i, j);

        return Math.abs(s1) + Math.abs(s2);
    }

    private int apply_sobel_mask(int mask[][], int I[][], int i, int j) {
        int sum = 0;

        for (int m = 0; m < MASK_SIZE; m++) {
            for (int n = 0; n < MASK_SIZE; n++) {
                sum += mask[m][n] * I[i - 1 + m][j - 1 + n];

            }
        }
        return sum;
    }
}
