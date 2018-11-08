/*

Copyright 2003 The University of New South Wales (UNSW) and National  
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

// reads in 2 files of millisecond time data
// and writes out the difference in time, along with min/max/avg stats
// by Terry Tam (Robocup 2003)

import java.io.*;
import java.util.StringTokenizer;
import java.util.Vector;

public class LatencyStats {

    private static Vector v1, v2;
    private static long t1, t2;
    private static String line;

    private static long sum = 0;
    private static long diff = 0;
    private static long min, max = 0;

    public static void main(String[] args) throws Exception {

        if (args.length != 1) {
            System.out.println("Usage: java LatencyStats 'test_num'");
            System.exit(1);
        }
        
        String TEST_NUM = args[0];

        final String START_FILE = TEST_NUM + "start";
        final String END_FILE   = TEST_NUM + "end";
        final String STATS_FILE = TEST_NUM + "stats";

        BufferedReader file1 = new BufferedReader(new FileReader(START_FILE));
        BufferedReader file2 = new BufferedReader(new FileReader(END_FILE));        
        PrintWriter stats    = new PrintWriter(
                                    new BufferedWriter(
                                        new FileWriter(STATS_FILE)
                                    )
                               );

        v1 = new Vector();
        v2 = new Vector();

        // put file1 data into vector
        while (true) {
            line = file1.readLine();
            if (line==null) {
                break;
            }

            try {
                StringTokenizer st = new StringTokenizer(line, "=");
                st.nextToken();
                t1 = Long.parseLong(st.nextToken());
            } catch (Exception e1) {
                System.out.println("LatencyStats: "+START_FILE+" not in correct format");
            }

            v1.addElement(new Long(t1));
        }

        // put file2 data into vector
        while (true) {
            line = file2.readLine();
            if (line==null) {
                break;
            }

            try {
               StringTokenizer st = new StringTokenizer(line, "=");
               st.nextToken();
               t2 = Long.parseLong(st.nextToken());
            } catch (Exception e1) {
                System.out.println("LatencyStats: "+END_FILE+" not in correct format");
            }

            v2.addElement(new Long(t2));
        }

        // check if any UDP packets dropped from Client.c -> HumanControl.java
        if (v1.size() != v2.size()) {
            System.out.println("LatencyStats: vector sizes unequal!");
        }

        // check that there is at least 1 row of sample data
        if (v1.size() < 1 || v2.size() < 1) {
            System.out.println("LatencyStats: vector sizes < 1!");
        }

        // write out time differences into stats file
        for (int i=0 ; i < v1.size() ; i++) {
            diff = ((Long)(v2.elementAt(i))).longValue() - 
                   ((Long)(v1.elementAt(i))).longValue();
            writeFile(stats, diff+"");

            // initialise variables if first pass
            if (i == 0) {
                min = diff;
                max = diff;
            } else {
                if (diff < min) {
                    min = diff;
                } else if (diff > max) {
                    max = diff;
                }
            }
            sum += diff;
        }

        long avg = sum / v1.size();
        writeFile(stats, "avg latency = "+avg);                
        writeFile(stats, "min latency = "+min);                
        writeFile(stats, "max latency = "+max);                

    } // end main

    public static void writeFile(PrintWriter w, String msg) {
        w.println(msg);
        w.flush();
    } // end writeFile

} // end class
