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


/**
 * <p>Title: </p>
 * <p>Description: Inherit from RuleRDRInterval. Only to provide
* Default rules, contructors and clonedRule </p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: </p>
 * @author not attributable
 * @version 1.0
 *     - Default to IS_NOT_BLOB (conservative), on the other hand,
 *       BlobRDR is defaulted to IS_BLOB (to avoid false negative)
 */

public class RuleRDRRelation extends RuleRDRInterval {

    protected static final Pair[] DEFAULT_RANGE = {
        /* Radius Ratio */
        new Pair(new Integer(1 ), new Integer(100 ) ),
        /* Gradient */
        new Pair(new Integer(0 ), new Integer(100) ),
        /* Radius covering ( proportion of the centroid segment that is covered by the blob ) */
        new Pair(new Integer(0 ), new Integer(100 ) ),
    };

    public RuleRDRRelation(){
        super();
    }

    public RuleRDRRelation(RuleRDRRelation other){
        super(other);
    }


    public RuleRDRRelation(GenericRDRManager manager) { //Default Rule
        initialize();
        rdrManager = manager;
        intervals = new Pair[ManRDRBlobRelation.NUM_METRIC];
        for (int i = 0; i < intervals.length; i++)
            intervals[i] = DEFAULT_RANGE[i];
        C = CommonSense.IS_NOT_BLOB;
    }

    public RuleRDRRelation(GenericRDRManager manager,
                           RuleRDRInterval parent,
                           ExampleBlobRelation e) {
        initialize(manager,parent,e);
    }


    public GenericRDRRule clonedRule(){
        return new RuleRDRRelation(this);
    }

    public GenericRDRRule clonedRuleEmpty(){
        //HACK sothat doesn't copy exception
        RuleRDRBlobInterval newRule = new RuleRDRBlobInterval(this);
        newRule.exception = null;
        newRule.alternative = null;
        return newRule;
    }

}
