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

public class VisionObserver
    extends AbstractSubject
    implements Observer {
//    RDRApplication parent = null;
    public int numberOfObject = 0;
    public int numberOfBlob = 0;
    public int numberOfGoodBlob = 0;

    public VisionObserver() {
    }

    public void setNumberOfObject(int n) {
        numberOfObject = n;
        notifyObservers();
    }

    public void setNumberOfBlob(int n) {
        numberOfBlob = n;
        notifyObservers();
    }

    public void setNumberOfGoodBlob(int n) {
        numberOfGoodBlob = n;
        notifyObservers();
    }

    public void update(Subject o) {
        if (o instanceof ToolBlobRelationRDR) {
            ToolBlobRelationRDR tool = (ToolBlobRelationRDR) o;
            setNumberOfObject(tool.numberOfObject);
        }
        else if (o instanceof ToolBlobIntervalRDR) {
            ToolBlobIntervalRDR tool = (ToolBlobIntervalRDR) o;
            setNumberOfGoodBlob(tool.numberOfBlob);
        }
        else if (o instanceof ToolCombinedRDR) {
            ToolCombinedRDR tool = (ToolCombinedRDR) o;
            setNumberOfGoodBlob(tool.numberOfBlob);
            setNumberOfObject(tool.numberOfObject);
        }
        else if (o instanceof Blobber) {
            Blobber tool = (Blobber) o;
            setNumberOfBlob(tool.visibleBlobList.size());
        }
    }
}
