/*

Copyright 2004 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
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
 * UNSW 2004 Robocup (Daniel)
 *
 * Last modification background information
 * $Id: ProjPtsListFile.java 
 *
 * Copyright (c) 2004 UNSW
 * All Rights Reserved.
 *
 * Read object files and print statistics to plot graph.
 * For more info, read notes/projPts.txt
 *
**/

import java.io.*;
import java.util.ArrayList;

public class ProjPtsStatGen {

    static final int START_FRAME        = 1;
    static final int BIG_PROJECTION     = 1000;
    static final int MAX_SAMPLE_VALUE   = 400;
    static String    DATAFILE_DIR       = "objects/";
    static String    DATAFILE_SUFFIX    = ".object";

    static ArrayList samples;
    static String    filePrefix;

    /* Given the filename, return the distance. */
    /* ie. LOG_30.1 ==> return 30. */
    static int extractDistFromFilename(String filename) {

        int distStart   = filePrefix.length(),
            distEnd     = filename.lastIndexOf('.');

        return Integer.parseInt(filename.substring(distStart, distEnd));

    }

    /* Calculate and print statistics of the collected values. */
    static void calPrintStat(int dist) {

        if (samples.isEmpty()) {
            return;
        }

        //System.out.println("Calculating stats...");

        int curVal  = 0,
            sum     = 0,
            min     = BIG_PROJECTION,
            max     = 0;

        double  mean    = 0,
                sd      = 0,
                sumSq   = 0;

        /* Found the min, max, mean */ 
        for (int i = 0; i < samples.size(); i++) {

            curVal = ((Integer) samples.get(i)).intValue();
            sum    += curVal;

            if (min > curVal) {
                min = curVal;
            }
            if (max < curVal) {
                max = curVal;
            }
        }

        mean = (double) sum / (double) samples.size(); 

        /* Since it's offline, doesn't matter if we run for loop again. */
        /* Found the standard deviation. */
        for (int i = 0; i < samples.size(); i++) {
            curVal = ((Integer) samples.get(i)).intValue();
            sumSq  += ((double) curVal - mean) * ((double) curVal - mean);
        }
        sd = Math.sqrt(sumSq / (double) samples.size());

        if (true) {
        System.out.println(dist + " "
                            + mean + " "
                            + sd + " "
                            + min + " "
                            + max + " "
                           );
        }

    }

    /* Collect samples in the given file. */ 
    static void collectSamples(String curFilename) {

        try {
            BufferedReader dataIn = new BufferedReader(
                          new FileReader(DATAFILE_DIR + curFilename +
                                          DATAFILE_SUFFIX));

            String  inputStr;
            int     sample;

            while ((inputStr = dataIn.readLine()) != null) {

                sample = Integer.parseInt(inputStr);

                if (sample > 0 && sample < MAX_SAMPLE_VALUE) { 
                    samples.add(new Integer(sample));
                    //System.out.println(curFilename + ": " + sample);
                }
            }
    
        } catch (Exception e) {
            System.err.println(e);
        }
    }

    public static void main(String[] args) {

        if (args.length != 1) {
            System.out.println("Usage: java ProjPtsStatGen filePrefix < filelist > stat.out");
            return;
        }

        /* Go through object files of different distances, until "quit".*/
        BufferedReader  in = new BufferedReader(new InputStreamReader(System.in));
        String          curFilename;
        int             tmpDist;
        int             curDist = 0;

        samples     = new ArrayList();
        filePrefix  = args[0];

        try {

            while (!(curFilename = in.readLine()).equals("quit")) {

                tmpDist = extractDistFromFilename(curFilename);

                /* If a particular distance has finished, calculate stat and print. */
                if (tmpDist != curDist) {

                    calPrintStat(curDist); 
                    samples.clear();
                }

                curDist = tmpDist;
                collectSamples(curFilename);
            }

            /* Get the result for the last one. */
            calPrintStat(curDist); 
            samples.clear();

        } catch (Exception e) {
            System.err.println("main| " + e);
        }

        return;

    }
}
