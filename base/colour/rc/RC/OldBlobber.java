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
//  blob.java
//  rc
//
//  Created by Claude Sammut on Wed Aug 25 2004.
//

class OldBlobber {
    int label;
    int colour;
    int area;
    int perimeter;
    int min_x, min_y, max_x, max_y;
    int centre_x, centre_y;

    static final int MAX_BLOBS = 5000;
    static OldBlobber blob[] = new OldBlobber[MAX_BLOBS];
    static int blob_count = 0;

    public OldBlobber(int label, int colour) {
        this.label = label;
        this.colour = colour;
        min_x = BFL.WIDTH - 1;
        min_y = BFL.HEIGHT - 1;
        max_x = 0;
        max_y = 0;
    }

    public static int[][] blobber(byte I1[][]) {
        int I2[][] = new int[BFL.HEIGHT][BFL.WIDTH];
        int equiv[] = new int[MAX_BLOBS + 1];

        blob_count = 0;

        for (int i = 0; i < MAX_BLOBS; i++) {
            blob[i] = null;

            // Top left pixel is special

        }
        if (I1[0][0] <= CommonSense.RED_ROBOT) {
            I2[0][0] = ++blob_count;
            equiv[blob_count] = blob_count;
        }

        // Process top row

        for (int j = 1; j < BFL.WIDTH; j++) {
            if (I1[0][j] <= CommonSense.RED_ROBOT) {
                if (I1[0][j] != I1[0][j - 1]) {
                    I2[0][j] = ++blob_count;
                    equiv[blob_count] = blob_count;
                }
                else if (I1[0][j - 1] == I1[0][j]) {
                    I2[0][j] = I2[0][j - 1];
                }
            }
        }

        for (int i = 1; i < BFL.HEIGHT; i++) {
            // Check first column

            if (I1[i][0] <= CommonSense.RED_ROBOT) {
                if (I1[i][0] != I1[i - 1][0]) {
                    I2[i][0] = ++blob_count;
                    equiv[blob_count] = blob_count;
                }
                else if (I1[i - 1][0] == I1[i][0]) {
                    I2[i][0] = I2[i - 1][0];
                }
            }

            for (int j = 1; j < BFL.WIDTH; j++) {
                if (I1[i][j] > CommonSense.RED_ROBOT) {
                    continue;
                }

                byte centre = I1[i][j];
                byte above = I1[i - 1][j];
                byte left = I1[i][j - 1];

                if (centre == above && centre != left) {
                    I2[i][j] = I2[i - 1][j];
                }
                else if (centre == left && centre != above) {
                    I2[i][j] = I2[i][j - 1];
                }
                else if (centre == above && centre == left) {
                    if (I2[i - 1][j] == I2[i][j - 1]) { // same colour, same label
                        I2[i][j] = I2[i - 1][j];
                    }
                    else if (I2[i - 1][j] != 0 && I2[i][j - 1] == 0) {
                        int above_label = I2[i - 1][j];

                        while (equiv[above_label] != above_label) {
                            above_label = equiv[above_label];

                        }
                        equiv[I2[i][j -
                            1]] = equiv[I2[i][j]] = I2[i][j] = above_label;
                    }
                    else if (I2[i][j - 1] != 0 && I2[i - 1][j] == 0) {
                        int left_label = I2[i][j - 1];

                        while (equiv[left_label] != left_label) {
                            left_label = equiv[left_label];

                        }
                        equiv[I2[i -
                            1][j]] = equiv[I2[i][j]] = I2[i][j] = left_label;
                    }
                    else if (I2[i - 1][j] != 0 && I2[i][j - 1] != 0) {
                        int above_label = I2[i - 1][j];
                        int left_label = I2[i][j - 1];

                        while (equiv[above_label] != above_label) {
                            above_label = equiv[above_label];
                        }
                        while (equiv[left_label] != left_label) {
                            left_label = equiv[left_label];

                        }
                        if (above_label < left_label) {
                            equiv[I2[i][j -
                                1]] = equiv[I2[i][j]] = I2[i][j] = above_label;
                        }
                        else {
                            equiv[I2[i -
                                1][j]] = equiv[I2[i][j]] = I2[i][j] = left_label;
                        }
                    }
                    else {
                        I2[i][j] = ++blob_count;
                        equiv[blob_count] = blob_count;
                    }
                }
                else if (centre == I1[i - 1][j - 1]) { // above left
                    I2[i][j] = I2[i - 1][j - 1];
                }
                else if (j < BFL.WIDTH - 1 && centre == I1[i - 1][j + 1]) { // above right
                    I2[i][j] = I2[i - 1][j + 1];
                }
                else {
                    I2[i][j] = ++blob_count;
                    equiv[blob_count] = blob_count;
                }

            }
        }

        for (int i = 0; i < BFL.HEIGHT; i++) {
            for (int j = 0; j < BFL.WIDTH; j++) {
                if (I2[i][j] == 0) {
                    continue;
                }

                I2[i][j] = equiv[I2[i][j]];
                OldBlobber b = blob[I2[i][j]];
                if (b == null) {
                    blob[I2[i][j]] = b = new OldBlobber(I2[i][j], I1[i][j]);

                }
                b.area++;

                if (i == 0 || I2[i - 1][j] != I2[i][j]) {
                    b.perimeter++;
                }
                else if (i == BFL.HEIGHT - 1 || I2[i + 1][j] != I2[i][j]) {
                    b.perimeter++;
                }
                else if (j == 0 || I2[i][j - 1] != I2[i][j]) {
                    b.perimeter++;
                }
                else if (j == BFL.WIDTH - 1 || I2[i][j + 1] != I2[i][j]) {
                    b.perimeter++;

                }
                if (j < b.min_x) {
                    b.min_x = j;
                }
                if (j > b.max_x) {
                    b.max_x = j;
                }
                if (i < b.min_y) {
                    b.min_y = i;
                }
                if (i > b.max_y) {
                    b.max_y = i;

                }
                b.centre_x = b.min_x + (b.max_x - b.min_x) / 2;
                b.centre_y = b.min_y + (b.max_y - b.min_y) / 2;
            }
        }

        trimBlobs(I1);
        return I2;
    }

    public static void trimBlobs(byte I[][]) {
        // Remove all small or very narrow blobs

        for (int i = 0; i < blob_count; i++) {
            if (blob[i] != null) {
                if (blob[i].area < 4) {
                    System.err.println("Too small (" + blob[i].area + "): " +
                                       blob[i].colour + " " + blob[i].centre_x +
                                       " " + blob[i].centre_y);
                    blob[i] = null;
                }
                else if (blob[i].max_x - blob[i].min_x < 2 ||
                         blob[i].max_y - blob[i].min_y < 2) {
                    System.err.println("Too narrow (" + blob[i].area + "): " +
                                       blob[i].colour + " " + blob[i].centre_x +
                                       " " + blob[i].centre_y);
                    blob[i] = null;
                }
            }
        }

        // Remove blobs contained in other blobs

        for (int i = 0; i < blob_count; i++) {
            if (blob[i] == null) {
                continue;
            }

            for (int j = i + 1; j < blob_count; j++) {
                if (blob[j] == null) {
                    continue;
                }

                if (contains(blob[i], blob[j])) {
                    System.err.println("contains(" + blob[i].colour + ", " +
                                       blob[j].colour + ")");
                    blob[j] = null;
                }
            }
        }

        // Check relative heights of blobs

        for (int i = 0; i < blob_count; i++) {
            if (blob[i] == null) {
                continue;
            }

            switch (blob[i].colour) {
                // Blobs too high
                case CommonSense.ORANGE:
                case CommonSense.BLUE_ROBOT:
                    if (height_above_field(I, blob[i]) > 10) {
                        System.err.println("Too high: " + blob[i].colour + " " +
                                           blob[i].centre_x + " " +
                                           blob[i].centre_y + " " +
                                           height_above_field(I, blob[i]));
                        blob[i] = null;
                    }
                    break;

                    // Blue and yellow goals
                case CommonSense.BLUE:
                case CommonSense.YELLOW:
                    if (height_above_field(I, blob[i]) <= 10) {
                        break;
                    }
                    else {
                        System.err.println("Check goals (" + blob[i].area +
                                           "): " + blob[i].colour + " " +
                                           blob[i].centre_x + " " +
                                           blob[i].centre_y + " " +
                                           height_above_field(I, blob[i]));
                    }

                    // Remove beacon colour blobs that are not parts of beacons
                    // They must be above or below pink blobs
                    // or they may be obscured by robots
                case CommonSense.GREEN:
                    boolean beacon = false;

                    for (int j = 0; j < blob_count; j++) {
                        if (blob[j] == null) {
                            continue;
                        }

                        switch (blob[j].colour) {
                            case CommonSense.PINK:
                                if (over(blob[i], blob[j]) ||
                                    over(blob[j], blob[i])) {
                                    beacon = true;
                                }
                                break;
                        }
                    }

                    if (!beacon) {
                        System.err.println("Not a beacon (" + blob[i].area +
                                           "): " + blob[i].colour + " " +
                                           blob[i].centre_x + " " +
                                           blob[i].centre_y + " " +
                                           height_above_field(I, blob[i]));
                        blob[i] = null;
                    }
                    break;
            }
        }

        // If a pink blob overlaps with red robot, change pink to read

        for (int i = 0; i < blob_count; i++) {
            if (blob[i] == null) {
                continue;
            }

            if (blob[i].colour == CommonSense.PINK) {
                for (int j = 0; j < blob_count; j++) {
                    if (blob[j] == null) {
                        continue;
                    }

                    if (blob[j].colour == CommonSense.RED_ROBOT &&
                        overlaps(blob[i], blob[j])) {
                        blob[i].colour = CommonSense.RED_ROBOT;
                        break;
                    }
                }
            }
        }
    }

    private static boolean contains(OldBlobber b1, OldBlobber b2) {
        return b1.min_x < b2.min_x && b1.max_x > b2.max_x &&
            b1.min_y < b2.min_y && b1.max_y > b2.max_y;
    }

    private static boolean above(OldBlobber b1, OldBlobber b2) {
        return b1.centre_y < b2.centre_y;
    }

    private static boolean over(OldBlobber b1, OldBlobber b2) {
        return b1.centre_y < b2.centre_y
            && b1.centre_x > b2.min_x
            && b1.centre_x < b2.max_x
            ;
    }

    private static boolean overlaps(OldBlobber b1, OldBlobber b2) {
        return (b1.min_x > b2.min_x && b1.min_x < b2.max_x ||
                b1.max_x > b2.min_x && b1.max_x < b2.max_x)
            &&
            (b1.min_y > b2.min_y && b1.min_y < b2.max_y ||
             b1.max_y > b2.min_y && b1.max_y < b2.max_y)
            ;
    }

    private static int height_above_field(byte I[][], OldBlobber b) {
        int min_height = BFL.HEIGHT;

        for (int j = b.min_x; j <= b.max_x; j++) {
            int height = 0;

            for (int i = b.max_y; i < BFL.HEIGHT && I[i][j] != CommonSense.GREEN_FIELD;
                 i++) {
                height++;

            }
            if (height == 0) {
                return height;
            }
            if (height < min_height) {
                min_height = height;
            }
        }

        return min_height;
    }

    public static void showBlobs(BFLPanel panel) {
        System.err.println("*********************");
        for (int i = 0; i < blob_count; i++) {
            if (blob[i] == null) {
                continue;
            }

            OldBlobber b = blob[i];

            panel.drawBoundingBox(Integer.toString(b.colour), b.min_x, b.min_y,
                                  b.max_x, b.max_y);
            System.err.println(b.label + "(" + b.colour + " - " + b.area + ")" +
                               ": " + b.min_x + " " + b.min_y + " " + b.max_x +
                               " " + b.max_y);
        }
    }
}
