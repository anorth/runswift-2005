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

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 */

import java.util.*;
import java.io.*;
import RoboShare.*;

public class CPlaneLog {
    private ArrayList cplaneList = new ArrayList();
    ;

    public CPlaneLog(String filename) {
        try {
            BufferedReader reader = new BufferedReader(new FileReader(
                filename));
            String version = reader.readLine();
            String line;
            int count = 0;
            while ( (line = reader.readLine()) != null) {
                CPlane cplane = new CPlane(line);
                if (cplane.isGood()) {
                    cplaneList.add(cplane);
                }
                System.out.println("Got frame " + (++count));
            }

        }
        catch (FileNotFoundException ex) {
            ex.printStackTrace();
        }
        catch (IOException ex1) {
            ex1.printStackTrace();
        }

    }

    public ArrayList getCPlaneList() {
        return cplaneList;
    }

    public static void main(String[] args) {
        CPlaneLog CPlaneLog1 = new CPlaneLog(args[1]);
    }

}
