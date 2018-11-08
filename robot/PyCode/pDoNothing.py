##     Copyright 2005 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     should be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# DoNothing.py
#
# DoNothing is a simple high-level Behaviour/Role that just stands there.
import Indicator
import Action
indCycle = 0

def DecideNextAction():
    global indCycle
    indCycle += 1
    if indCycle < 10:
        Indicator.showHeadColor((False, True, False))
    elif indCycle < 20:
        Indicator.showHeadColor((False, False, True))
    else:
        indCycle = 0

#    doMore() # rather than do nothing


###########################################################
# you can do more stuff below (than nothing). =p
###########################################################

import hFrameReset
#import VisionLink

#set to true to make head look at around beacon height
#use to help calculate beacon distance constants
gLookUp = False

gResetTimer = 0 

def doMore():
    global gResetTimer
    global gLookUp
       
    hFrameReset.frameReset()
        
    if gLookUp:  
        Action.setHeadParams(0,0,30,Action.HTAbs_h) 
        return   

    #numObsLeft = VisionLink.getNoObstacleInBox(-35,80,0,50)
    #numObsRight = VisionLink.getNoObstacleInBox(0,80,35,50)   
        
    #print "Num Obstacle Left : ", numObsLeft    
    #print "Num Obstacle Right : ", numObsRight

##     far = VisionLink.getAnySensor(Constant.ssINFRARED_NEAR)
##     near = VisionLink.getAnySensor(Constant.ssINFRARED_FAR)
##     print "NEAR : ", near
##     print "FAR : ", far

##     minValue, maxValue = VisionLink.getHeadingToBestGap()
       # min-max is the range of heading to where you can shoot
       # +ve is left, -ve is right   26 is the left most. -27 is right most if looking front   
##     heading = (minValue + maxValue) / 2.0
##     print "heading ", heading

