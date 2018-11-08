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
*  @ version 1.6
 * + version 1.1:
 *     - Add DEFAULT_RANGE for new metrics
 *     - make IS_BLOB a default rule
 *     - Add serialization code
 * + version 1.6:
*       - Change XML example format ( to be just metric numbers).
 */

import java.util.*;
import RoboShare.*;

public class RuleRDRBlobInterval extends RuleRDRInterval {

    static int DEFAULT_CLASSIFICATION = CommonSense.IS_NOT_BLOB;

    protected static final Pair[] DEFAULT_RANGE = {
        /* Height : fix : from the floor, not horizon */
        new Pair(new Integer(0 ), new Integer(100 ) ),
        /* Aspect Ratio */
        new Pair(new Integer(0) , new Integer(100 ) ),
        /* Density */
        new Pair(new Integer(0), new Integer(100) ),

    };

    public RuleRDRBlobInterval(){
        super();
    }

    public RuleRDRBlobInterval(GenericRDRManager manager) { //Default Rule
        initialize(manager);
        intervals = new Pair[ manager.getNumberOfMetrics() ];
        for (int i = 0; i < intervals.length; i++)
            intervals[i] = DEFAULT_RANGE[i];
        C = DEFAULT_CLASSIFICATION;
    }

    public RuleRDRBlobInterval(RuleRDRInterval copy) { //Copy constructor
        super(copy);
    }

    public RuleRDRBlobInterval(GenericRDRManager manager,
                               RuleRDRInterval parent,
                               GenericIntervalExample e) {
        initialize(manager, parent, e);
    }

    public GenericRDRRule clonedRule(){
        return new RuleRDRBlobInterval(this);
    }

    public GenericRDRRule clonedRuleEmpty(){
        //HACK sothat doesn't copy exception
        RuleRDRBlobInterval newRule = new RuleRDRBlobInterval(this);
        newRule.exception = null;
        newRule.alternative = null;
        return newRule;
    }

}
