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
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

import java.util.*;


public class CPlane {
    static public final int MAXIMUM_SIZE = 30000;
    boolean isGood = false;
    int[] runlen = null;
    int[] color = null;

    public CPlane() {
    }

    public CPlane(String textline) {
        StringTokenizer st = new StringTokenizer(textline);
        int size = Integer.parseInt(st.nextToken());
        System.out.println("Got Size = " + size);
        if (size > MAXIMUM_SIZE)return;
        runlen = new int[size / 2];
        color = new int[size / 2];
        int i;
        for (i = 0; i < size / 2; i++) {
            if (!st.hasMoreTokens())break;
            runlen[i] = Integer.parseInt(st.nextToken());
            if (!st.hasMoreTokens())break;
            color[i] = Integer.parseInt(st.nextToken());
        }
        isGood = (i == size / 2);
    }

    public boolean isGood() {
        return isGood;
    }

    public BFL toBFL() {
        BFL c = new BFL(true);
        int i = 0, j = 0;
        for (int k = 0; k < runlen.length; k++) {
            for (int u = 0; u < runlen[k]; u++) {
                c.C[i][j] = (byte) color[k];
                j++;
                if (j >= c.WIDTH) {
                    j = 0;
                    i++;
                }
            }
        }
        c.cplaneToRGB();
        return c;
    }

}
