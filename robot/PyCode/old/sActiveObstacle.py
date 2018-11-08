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



# This code looks at two points out of the three. 
ACTIVATE_PAN = 40

PAN_RIGHT = 70
PAN_FRONT = 0 
PAN_LEFT  = -70

def DecideNextAction(): 
    Action.setHeadParams(-50,0,0,Action.HTAbs_h)
    perform(False)

gLastPan = 0 
gLastTilt = 0
gLastCrane = 0

gIsActivating = False
gPanCounter1  = 0
gPanCounter2  = 0

gPan1 = 0
gPan2 = 0

gLastActiveObstacle = 0

def resetPerform(): 
    global gIsActivating 
    gIsActivating = False

def perform(doTwoPan=True):
    global gLastPan 
    global gLastTilt
    global gLastCrane
    global gPanCounter1
    global gPanCounter2 
    global gPan1
    global gPan2    
    global gIsActivating
    global gLastActiveObstacle
        

    pan = Global.desiredPan
    if not gIsActivating\
        and abs(pan) > 40\
        and Global.frame - gLastActiveObstacle > 60:
         
        gLastPan = pan 
        gLastTilt = Global.desiredTilt
        gLastCrane = Global.desiredCrane 
        
        gIsActivating = True
        gLastActiveObstacle = Global.frame
        
        if doTwoPan:
            gPanCounter1 = 15
        else:
            gPanCounter1 = 0
            
        gPanCounter2 = 10

        # Pick which point to pan at 
        if pan < 0: 
            gPan1 = PAN_RIGHT
            gPan2 = PAN_FRONT
        else:
            gPan1 = PAN_LEFT
            gPan2 = PAN_FRONT
                            
     
    if gIsActivating:       
    
        if gPanCounter1 > 0: 
            Action.setHeadParams(gPan1,-10,0,Action.HTAbs_h)
            gPanCounter1 -= 1
            gLastActiveObstacle = Global.frame
            return Constant.STATE_EXECUTING
        
        elif gPanCounter2 > 0: 
            Action.setHeadParams(gPan2,-10,0,Action.HTAbs_h)
            gPanCounter2 -= 1
            gLastActiveObstacle = Global.frame
            return Constant.STATE_EXECUTING
        
        else: 
            Action.setHeadParams(gLastPan,gLastTilt,gLastCrane,Action.HTAbs_h) 
            resetPerform()            
    
    return Constant.STATE_SUCCESS
            
    
    

