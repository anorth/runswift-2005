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



# Do something smart when facing towards own goal. 

import Constant
import hMath
import Global
import sFindBall
import sGetBehindBall
import sUpennKick

gLastFrameCalled = 0


NO_GET_BEHIND_DURATION = 60
gNoGetBehindCounter = 0
gDirection = None


def DecideNextAction():    
    if perform() != Constant.STATE_EXECUTING: 
        sFindBall.perform()


def resetPerform(): 
    global gDirection
    global gNoGetBehindCounter
        
    gDirection = None
    gNoGetBehindCounter = 0

def perform(): 
    global gLastFrameCalled   
    global gDirection  
    global gNoGetBehindCounter

    
    if gLastFrameCalled != Global.frame - 1: 
        resetPerform()
    gLastFrameCalled = Global.frame     
    
    
    gNoGetBehindCounter = max(gNoGetBehindCounter-1,0)
    
    selfLoc = Global.selfLoc
    selfX, selfY = selfLoc.getPos() 
    selfH = selfLoc.getHeading()
                    
    ballX, ballY = Global.ballX, Global.ballY
    ballD = Global.ballD
    ballH = Global.ballH
       
    
    selfH2BallH = selfH + ballH
    ballH2OGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.OWN_GOAL_X,Constant.OWN_GOAL_Y)
    selfH2OGoalH = hMath.normalizeAngle_180(selfH2BallH - ballH2OGoalH)
    
    selfH2SafeH = hMath.normalizeAngle_180(selfH2OGoalH + 180)
    
    if abs(selfH2OGoalH) > 110:
        resetPerform()
        return Constant.STATE_SUCCESS

    
    # If we are far away from the ball, then just use our implicit get behind.
    #if Global.ballD > 60:
    #    sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_LOTS)
    #    # Reset the direction.
    #    gDirection = None
    #    return Constant.STATE_EXECUTING
            
    # First time to enter the function, so choose the direction.    
    if gDirection == None:       
        # go left
        if selfH2OGoalH < 0: 
            gDirection = Constant.dANTICLOCKWISE
        # go right    
        else:
            gDirection = Constant.dCLOCKWISE
            

    if gNoGetBehindCounter > 0:             
        sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_PRIORITY)

    else:           
        sFindBall.perform(True)                        
        r = sGetBehindBall.performBall(dist=25,direction=gDirection,accuracy=10)
        if r == Constant.STATE_SUCCESS: 
            sFindBall.perform(doGetBehind = sFindBall.GET_BEHIND_PRIORITY)        
            gNoGetBehindCounter = NO_GET_BEHIND_DURATION
        
    return Constant.STATE_EXECUTING           
