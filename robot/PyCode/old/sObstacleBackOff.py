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


import Action
import Constant
import Global
import Indicator
import VisionLink

gObstacleBackOffTriggering = False
BACKOFF_TIMER = 45
gBackOffTimer = 0

gLastCalledFrame = 0
gNumObstacleLeft = 0
gNumObstacleRight = 0

def DecideNextAction():
    Indicator.setDefault()
    perform(3,5,0)
    

def resetEachFrame():
    global gLastCalledFrame, gNumObstacleLeft, gNumObstacleRight
    if gLastCalledFrame != Global.frame:   
        gNumObstacleLeft = VisionLink.getNoObstacleInBox(-50,80,0,50)
        gNumObstacleRight = VisionLink.getNoObstacleInBox(0,80,50,50)
        gLastCalledFrame = Global.frame 
    
def perform(forward,left,turnccw,forceDir=False):
    if not isObstacleBackOffTriggering(): 
        return Constant.STATE_SUCCESS
    
    if forceDir:
        Action.walk(forward,left,turnccw)
        if left > 0: 
            Indicator.showFacePattern([0,0,0,0,5])
        else:
            Indicator.showFacePattern([5,0,0,0,0])        
        
    # Go left!
    elif gNumObstacleLeft < gNumObstacleRight:         
        Action.walk(forward,left,turnccw)
        Indicator.showFacePattern([0,0,0,0,5])
                    
    # Go right!
    else:    
        Action.walk(forward,-left,turnccw)
        Indicator.showFacePattern([5,0,0,0,0])

    return Constant.STATE_EXECUTING
   

def isObstacleBackOffTriggering(): 
    global gObstacleBackOffTriggering
    global gBackOffTimer
    
    if gObstacleBackOffTriggering: 
        if gBackOffTimer > BACKOFF_TIMER: 
            gBackOffTimer = 0
            gObstacleBackOffTriggering = False
        else: 
            gBackOffTimer += 1
    else:
        gObstacleBackOffTriggering = isObstacleInFrontOfMe() 
        print "Last Frame : ", gLastCalledFrame
        print "Obstacle Points Left: ", gNumObstacleLeft, 
        print "Obstacle Points Right: ", gNumObstacleRight   
    
    return gObstacleBackOffTriggering
    
def isObstacleInFrontOfMe(): 
    resetEachFrame()    
    
    if gNumObstacleLeft + gNumObstacleRight > 30:
        return True
    return False
            
    
 
    
