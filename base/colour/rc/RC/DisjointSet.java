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

public class DisjointSet{
   public int rank;
   public DisjointSet parent;
   int colour ;
   int startX, endX, Y;
   Blob b;

   public DisjointSet(int c, int x1, int x2, int y, double roll, BFL bfl){
       colour = c;
       startX = x1;
       endX = x2;
       Y = y;

       rank = 0;
       parent = this;

       b = new Blob(c,bfl,roll);
       b.add(x1,y);
       b.add(x2,y);
       b.area += x2 - x1 - 1;

       Point2D rotatedUV = Utils.getRotatedPixel(x1,y,roll);
       b.addRotated( rotatedUV.getRawX(), rotatedUV.getRawY() );
       rotatedUV = Utils.getRotatedPixel(x2,y,roll);
       b.addRotated( rotatedUV.getRawX(), rotatedUV.getRawY() );
   }

   DisjointSet getroot() {
       if (parent != this)
           parent = parent.getroot();
       return parent;
   }

   boolean isRoot(){
       return parent == this;
   }

   boolean canBeJoined(DisjointSet other) {
       return this.endX >= other.startX && other.endX >= this.startX;
   }

   void join(DisjointSet target) {
       DisjointSet x = getroot(),
           y = target.getroot();
//       System.out.println("Merging " + x.b + "   " + y.b);
       if (x.rank < y.rank){
           x.parent = y;
           y.b.inMerge(x.b);

       }
       else {
           y.parent = x;
           x.b.inMerge(y.b);
           if (x.rank == y.rank)
               x.rank++;
       }
   }
}
