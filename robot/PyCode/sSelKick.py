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


# Kick selection

import Action
import Constant
import Global
import hMath
import hWhere
import hStuck
import sGrab
#import VisionLink


KT_PAW_KICK          = 1
KT_GRAB_TURN_KICK    = 2  # no visual aim
KT_GRAB_TURN_SHOOT   = 3  # requires visual on goal
KT_UPENN_LEFT        = 4
KT_UPENN_RIGHT       = 5
KT_TURN_KICK         = 6
#KT_HEAD_LEFT        = 7
#KT_HEAD_RIGHT       = 8
KT_DRIBBLE           = 9
KT_GRAB_DRIBBLE      = 10
KT_GRAB_DRIBBLE_GOAL = 11
KT_CHEST_PUSH        = 12
KT_AVOID_OWN_GOAL    = 13
KT_GRAB_DRIBBLE_STOP = 14  # Grab dribble and stop without kicking

def DecideNextAction(): 
    perform()
    Action.stopLegs()
    import hFWHead 
    hFWHead.DecideNextAction()



# This function is a kick selection function which returns
# a tuple of kick type and dkd.
# This function assumes you are very close to the ball and
# you are facing towards the ball (e.g. when grabbing). 
# We should add obstacle points as an extra kick selection criteria,
# we should also return desired kick direction. 
# (kickType,dkd,direction)

selfX = 0
selfY = 0
selfH = 0
ballX = 0
ballY = 0
ballD = 0
ballH = 0

EDGE_DIST = 70

gPerform = None
gPerformFrame = -1

gLastAvoidOwnGoalFrame = 0


def perform():
    global gObsToBall
    global selfX, selfY, selfH, ballX, ballY, ballD, ballH
    global gPerform, gPerformFrame
    
    if gPerformFrame == Global.frame:
        return gPerform
          
    selfX, selfY = Global.selfLoc.getPos()
    selfH = Global.selfLoc.getHeading() 
    
    ballX = Global.ballX
    ballY = Global.ballY
    ballH = Global.ballH
    ballD = Global.ballD
    
    ballH2TGoalH = hMath.getHeadingBetween(ballX, ballY,\
                                           Constant.TARGET_GOAL_X,
                                           Constant.TARGET_GOAL_Y)

    # Special variable lighting challenge kick selection
    if Global.lightingChallenge:
        # In the rear of the field just kick the ball out - it will get replaced
        # at the centre circle
        if ballY < Constant.FIELD_LENGTH * 0.35:
            rtn = selectKickOut()
        elif ballY < Constant.FIELD_LENGTH * 0.55:
            rtn = (KT_GRAB_DRIBBLE, ballH2TGoalH, Constant.dAUTO)    
        else:
            rtn = (KT_GRAB_DRIBBLE_GOAL, ballH2TGoalH, Constant.dAUTO)    

    # Special penalty shooter kick selection
    elif Global.penaltyShot:
        if ballY < Constant.FIELD_LENGTH * 0.55:
            rtn = (KT_GRAB_DRIBBLE_STOP, ballH2TGoalH, Constant.dAUTO)    
        else:
            return (KT_GRAB_DRIBBLE_GOAL, ballH2TGoalH, Constant.dAUTO)    

    elif not sGrab.isGrabbed and Global.frame - gLastAvoidOwnGoalFrame < 5: 
        rtn = (KT_AVOID_OWN_GOAL, ballH2TGoalH, Constant.dAUTO)
        
    # Usual attacker kick selection
    elif ballY < Constant.FIELD_LENGTH * 0.4:              
        rtn = selectKickInDefensiveThird()

    elif ballY < Constant.FIELD_LENGTH * 0.6:    
        rtn = selectKickInMiddleThird()
    
    else: 
        rtn = selectKickInOffensiveThird()
        
    gPerform = rtn
    gPerformFrame = Global.frame
    return rtn


def selectKickInDefensiveThird():
    global gLastAvoidOwnGoalFrame

    selfH2BallH = hMath.normalizeAngle_0_360(selfH + ballH)

    ballH2OGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.OWN_GOAL_X,Constant.OWN_GOAL_Y)
    ballH2MinOGoalBoxH = hMath.getHeadingBetween(ballX,ballY,\
                                                 Constant.MIN_GOALBOX_EDGE_X,Constant.OWN_GOAL_Y)
    ballH2MaxOGoalBoxH = hMath.getHeadingBetween(ballX,ballY,\
                                                 Constant.MAX_GOALBOX_EDGE_X,Constant.OWN_GOAL_Y)
                                           
    selfH2OGoalH = hMath.normalizeAngle_180(selfH2BallH - ballH2OGoalH)

    ballH2CenterH = hMath.getHeadingBetween(ballX,ballY,\
                                            Constant.FIELD_WIDTH/2.0,ballY+200)    
    selfH2CenterH = hMath.normalizeAngle_180(selfH2BallH - ballH2CenterH)
            
    
    if sGrab.isGrabbed: 
        return (KT_GRAB_DRIBBLE,ballH2CenterH,Constant.dAUTO)    
        
    elif abs(selfH2OGoalH) < 60\
        or ballH2MinOGoalBoxH < selfH2BallH < ballH2MaxOGoalBoxH\
        or Global.vOGoal.isVisible():         
        gLastAvoidOwnGoalFrame = Global.frame
        return (KT_AVOID_OWN_GOAL,ballH2CenterH,Constant.dAUTO)
        
    # If we are stuck in defensive area, do something!!!      
    elif hStuck.gUseContestDetect and hStuck.isBallContested():                      
        if 45 <= abs(selfH2CenterH) < 60:
            if selfH2CenterH < 0:                              
                return (KT_UPENN_RIGHT,ballH2CenterH,Constant.dAUTO)   
            else:
                return (KT_UPENN_LEFT,ballH2CenterH,Constant.dAUTO)        
        elif (ballD < sGrab.CLOSE_DIST or Global.frame - sGrab.gLastApproachFrame < 5)\
            and ((hWhere.isOnLEdge(EDGE_DIST) and abs(selfH2CenterH) > 10)
                or (hWhere.isOnREdge(EDGE_DIST) and abs(selfH2CenterH) > 10)
                or selfH >= 180): 
            return (KT_GRAB_DRIBBLE,ballH2CenterH,Constant.dAUTO)                         
        else:            
            return (KT_DRIBBLE,ballH2CenterH,Constant.dAUTO)        
            
            
    # If the ball is in the left edge. 
    elif hWhere.ballOnLEdge():
        return selectKickInLeftEdge()

    # If the ball is in the right edge.
    elif hWhere.ballOnREdge():
        return selectKickInRightEdge()      

    # If the ball is not in the edge of the field
    else:                   
        if 45 <= abs(selfH2CenterH) < 60:
            if selfH2CenterH < 0:                              
                return (KT_UPENN_RIGHT,ballH2CenterH,Constant.dAUTO)   
            else:
                return (KT_UPENN_LEFT,ballH2CenterH,Constant.dAUTO)

        elif abs(selfH2CenterH) < 90:
                    
            if abs(selfH2CenterH) < 20:
                return (KT_DRIBBLE,ballH2CenterH,Constant.dAUTO)
            else:
                return (KT_GRAB_DRIBBLE,ballH2CenterH,Constant.dAUTO)

        # Otherwise we should do get behind??
        # turn kick
        else:
            return (KT_GRAB_DRIBBLE,ballH2CenterH,Constant.dAUTO)


def selectKickInMiddleThird(): 
    global gLastAvoidOwnGoalFrame

    selfH2BallH = hMath.normalizeAngle_0_360(selfH + ballH)
    ballH2OGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.OWN_GOAL_X,Constant.OWN_GOAL_Y)                                           
    selfH2OGoalH = hMath.normalizeAngle_180(selfH2BallH - ballH2OGoalH)

    ballH2TGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y-50)    
    selfH2TGoalH = hMath.normalizeAngle_180(selfH2BallH - ballH2TGoalH)   
    
    ballH2CenterH = hMath.getHeadingBetween(ballX,ballY,\
                                            Constant.FIELD_WIDTH/2.0,ballY+200)    
    selfH2CenterH = hMath.normalizeAngle_180(selfH2BallH - ballH2CenterH)
    
    if sGrab.isGrabbed: 
        if hWhere.ballOnLEdge() or hWhere.ballOnREdge(): 
            return (KT_GRAB_DRIBBLE,ballH2CenterH,Constant.dAUTO)
        else:
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)    
    
    elif abs(selfH2OGoalH) < 40 or Global.vOGoal.isVisible():         
        gLastAvoidOwnGoalFrame = Global.frame
        return (KT_AVOID_OWN_GOAL,ballH2TGoalH,Constant.dAUTO)            
    
    # If we are stuck in midfield area, do something!!! 
    # FIXME : smart strategy              
    elif hStuck.gUseContestDetect and hStuck.isBallContested():               
        if 45 <= abs(selfH2TGoalH) < 60:
            if selfH2TGoalH < 0:                              
                return (KT_UPENN_RIGHT,ballH2TGoalH,Constant.dAUTO)   
            else:
                return (KT_UPENN_LEFT,ballH2TGoalH,Constant.dAUTO)
        elif (ballD < sGrab.CLOSE_DIST or Global.frame - sGrab.gLastApproachFrame < 5)\
            and ((hWhere.isOnLEdge(EDGE_DIST) and abs(selfH2TGoalH) > 10) 
                or (hWhere.isOnREdge(EDGE_DIST) and abs(selfH2TGoalH) > 10)
                or selfH >= 180): 
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO) 

        else:            
            return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO) 
                

    # If the ball is in the left edge. 
    elif hWhere.ballOnLEdge():           
        return selectKickInLeftEdge()

    # If the ball is in the right edge.
    elif hWhere.ballOnREdge():
        return selectKickInRightEdge()

    # If the ball is not in the edges of the field
    else:                
                    
        if 45 <= abs(selfH2TGoalH) < 60:
            if selfH2TGoalH < 0: 
                return (KT_UPENN_RIGHT,ballH2TGoalH,Constant.dAUTO)
            else:
                return (KT_UPENN_LEFT,ballH2TGoalH,Constant.dAUTO)             
                
        elif abs(selfH2TGoalH) < 90:
                        
            if abs(selfH2TGoalH) < 10:
                return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO)            
            else:
                return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
                                
        else:   
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)                          


            
        
def selectKickInOffensiveThird(): 
    
    tGoalY = Constant.TARGET_GOAL_Y + 20
    
    # If the ball's y is somehow futher away than Constant.TARGET_GOAL_Y,
    # then hack the target goal y.
    if ballY > tGoalY:
        tGoalY = ballY + 50
    
    selfH2BallH = hMath.normalizeAngle_0_360(selfH + ballH)
    
    ballH2TGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.TARGET_GOAL_X,tGoalY)                                           
    selfH2TGoalH = hMath.normalizeAngle_180(selfH2BallH - ballH2TGoalH) 
    
    ballH2CrossingH = hMath.getHeadingBetween(ballX,ballY,\
                                              Constant.TARGET_GOAL_X,tGoalY-40)
    selfH2CrossingH = hMath.normalizeAngle_180(selfH2BallH - ballH2CrossingH)
    
    ballH2CenterH = hMath.getHeadingBetween(ballX,ballY,\
                                            Constant.FIELD_WIDTH/2.0,ballY+200)    
    selfH2CenterH = hMath.normalizeAngle_180(selfH2BallH - ballH2CenterH) 

    if sGrab.isGrabbed: 
        if hWhere.ballInTriTLCorner()\
            or hWhere.ballInTriTRCorner():            
            return (KT_GRAB_DRIBBLE,ballH2CrossingH,Constant.dAUTO)
        elif hWhere.ballOnLEdge(100)\
            or hWhere.ballOnREdge(100):
            return (KT_GRAB_DRIBBLE,ballH2CenterH,Constant.dAUTO)
        else:
            return (KT_GRAB_DRIBBLE_GOAL,ballH2TGoalH,Constant.dAUTO)

    # If we are stuck in offensive area, do something!!!        
    # FIXME : smart strategy
    elif hStuck.gUseContestDetect and hStuck.isBallContested():
        dkd = ballH2TGoalH        
        if hWhere.ballInTriTLCorner() or hWhere.ballInTriTRCorner(): 
            dkd = ballH2CrossingH
        selfH2dkd = hMath.normalizeAngle_180(selfH2BallH - dkd)
                 
        if 40 <= abs(selfH2dkd) < 75:
            if selfH2dkd < 0:                              
                return (KT_UPENN_RIGHT,dkd,Constant.dAUTO)   
            else:
                return (KT_UPENN_LEFT,dkd,Constant.dAUTO)
                
        elif (ballD < sGrab.CLOSE_DIST or Global.frame - sGrab.gLastApproachFrame < 5)\
            and (hWhere.isOnLEdge(EDGE_DIST)
                or hWhere.isOnREdge(EDGE_DIST)
                or hWhere.ballInTriTLCorner()
                or hWhere.ballInTriTRCorner()
                or selfH >= 180): 
            return (KT_GRAB_DRIBBLE,dkd,Constant.dAUTO)                 
                
        else:            
            return (KT_DRIBBLE,dkd,Constant.dAUTO)    


    # If the ball is in the corner, kick it across the field.
    elif hWhere.ballInTriTLCorner():
        if abs(selfH2CrossingH) < 10:
            return (KT_DRIBBLE,ballH2CrossingH,Constant.dAUTO)     

        elif 45 <= abs(selfH2CrossingH) < 60:

            if selfH2CrossingH < 0: 
                return (KT_UPENN_RIGHT,ballH2CrossingH,Constant.dAUTO)
            else: 
                return (KT_UPENN_LEFT,ballH2CrossingH,Constant.dAUTO)  

        else: 
            return (KT_GRAB_DRIBBLE,ballH2CrossingH,Constant.dAUTO)

        # If there's already a supporter waiting, then TURN KICK????     


    # If the ball is in the left edge. 
    elif hWhere.ballOnLEdge(100):   
        if hStuck.isBallContested():
            return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
        else:    
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)


    # If the ball is in the corner, kick it across the field.                        
    elif hWhere.ballInTriTRCorner():
        if abs(selfH2CrossingH) < 10:
            return (KT_DRIBBLE,ballH2CrossingH,Constant.dAUTO)      

        elif 45 <= abs(selfH2CrossingH) < 60:
            if selfH2CrossingH < 0: 
                return (KT_UPENN_RIGHT,ballH2CrossingH,Constant.dAUTO)
            else: 
                return (KT_UPENN_LEFT,ballH2CrossingH,Constant.dAUTO)

        else:
            return (KT_GRAB_DRIBBLE,ballH2CrossingH,Constant.dAUTO)
            
            # If there's already a supporter waiting, then TURN KICK????            

    # If the ball is in the right edge.
    elif hWhere.ballOnREdge(100):                  
        if hStuck.isBallContested():
            return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
        else:
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)

    # If the ball is really close to the goal, dribble
    elif not sGrab.isGrabbed\
        and ballY > (Constant.FIELD_LENGTH - 30)\
        and ballX > (Constant.FIELD_WIDTH/2 - Constant.GOAL_WIDTH/2 + 15)\
        and ballX < (Constant.FIELD_WIDTH/2 + Constant.GOAL_WIDTH/2 - 15)\
        and abs(selfH2TGoalH) < 45:
        
        return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
        
    # If the ball is really close to the goal, grab dribble into goal
    elif ballY > (Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH)\
        and ballX > (Constant.FIELD_WIDTH/2 - Constant.GOAL_WIDTH/2 - 50)\
        and ballX < (Constant.FIELD_WIDTH/2 + Constant.GOAL_WIDTH/2 + 50):
        
        return (KT_GRAB_DRIBBLE_GOAL,ballH2TGoalH,Constant.dAUTO)
    
    
    elif hStuck.isBallContested():
        if sGrab.isGrabbed: 
            return (KT_GRAB_DRIBBLE_GOAL,ballH2TGoalH,Constant.dAUTO)
        elif abs(selfH2TGoalH) < 40:          
            return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
            
    # If the robot is a little far, then dribble
    elif ballY < Constant.FIELD_LENGTH * 0.65 and abs(selfH2TGoalH) < 15:
        return (KT_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
        
    return (KT_GRAB_DRIBBLE_GOAL,ballH2TGoalH,Constant.dAUTO)    


def selectKickInLeftEdge(): 
    selfH2BallH = hMath.normalizeAngle_0_360(selfH + ballH) 
    ballH2TGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y)
    selfH2TGoalH = hMath.normalizeAngle_180(selfH2BallH - ballH2TGoalH)
    
    ballH2CenterH = hMath.getHeadingBetween(ballX,ballY,\
                                            Constant.FIELD_WIDTH/2.0,ballY+200)    
    selfH2CenterH = hMath.normalizeAngle_180(selfH2BallH - ballH2CenterH)
    
    # We are safe to do something...
    if 0 <= selfH2BallH <= 135: 
        # 
        if 45 <= abs(selfH2TGoalH) <= 60:      
            if selfH2TGoalH > 0: 
                return (KT_UPENN_LEFT,ballH2TGoalH,Constant.dAUTO)
            else:
                return (KT_UPENN_RIGHT,ballH2TGoalH,Constant.dAUTO)
        # 
        else:
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
    
    # Otherwise, if we just approach, the ball might go out. 
    # Hence, get-around-the-ball action.
    else: 
        return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)  


def selectKickInRightEdge(): 
    selfH2BallH = hMath.normalizeAngle_0_360(selfH + ballH) 
    ballH2TGoalH = hMath.getHeadingBetween(ballX,ballY,\
                                           Constant.TARGET_GOAL_X,Constant.TARGET_GOAL_Y)
    
    # We are safe to do something...
    if 45 <= selfH2BallH <= 180: 
        #
        if 45 <= abs(selfH2BallH - ballH2TGoalH) <= 60:       
            if selfH2BallH - ballH2TGoalH > 0: 
                return (KT_UPENN_LEFT,ballH2TGoalH,Constant.dAUTO)
            else:
                return (KT_UPENN_RIGHT,ballH2TGoalH,Constant.dAUTO)
        # 
        else:
            return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)
    
    # Otherwise, if we just approach, the ball might go out. 
    # Hence, get-around-the-ball action.
    else: 
        return (KT_GRAB_DRIBBLE,ballH2TGoalH,Constant.dAUTO)          

# Returns a kick to kick the ball out of the field
def selectKickOut():
    # Out the goal line
    if ballY < Constant.FIELD_LENGTH * 0.2:
        return (KT_GRAB_TURN_KICK, -90, Constant.dAUTO)    
    elif ballY > Constant.FIELD_LENGTH * 0.8:
        return (KT_GRAB_TURN_KICK, 90, Constant.dAUTO)    
    # Out the sides
    elif ballX < Constant.FIELD_WIDTH * 0.5:
        return (KT_GRAB_TURN_KICK, 180, Constant.dAUTO)    
    else:
        return (KT_GRAB_TURN_KICK, 0, Constant.dAUTO)    
        

def doINeedToGrab(kickType): 
    return kickType == KT_GRAB_TURN_SHOOT\
        or kickType == KT_GRAB_TURN_KICK\
        or kickType == KT_TURN_KICK\
        or kickType == KT_GRAB_DRIBBLE\
        or kickType == KT_GRAB_DRIBBLE_GOAL\
        or kickType == KT_GRAB_DRIBBLE_STOP

  
    
def test(argv):
    global selfX, selfY, selfH, ballX, ballY, ballH 
    id(argv)
    selfX = 30
    selfY = 150
    selfH = 270
    ballX = 10
    ballY = 80   
    ballH = hMath.getHeadingToMe(selfX,selfY,selfH,ballX,ballY)
    print str(perform())

if __name__ == "__main__":
    import sys 
    test(sys.argv)
