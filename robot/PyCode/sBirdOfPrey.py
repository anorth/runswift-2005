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
import Indicator
import Constant
import Debug
import Global
import hMath
import math
import sHoverToBall
import sDodgyDog

gIsBirdOfPreyTriggering = False
gLastCalledFrame = 0
gUseDodgyDog = True

#INI_DESIRED_BALL_ANGLE = 45 # desired local heading to target pos
currentBopMode         = Constant.NORMAL_BOP
lastDBH                = None
insideBound            = 36 * 0.5
outsideBound           = insideBound + 20
maxTurn                = 30
cornerCut              = 10
boxPointOffset         = 15 # avoid goalbox by this much
breakOutMinAngle       = 10
breakOutIncAngle       = 20
minAngle               = 90

# Where we are birding to. This can be set by args to perform()
targetX = None  # Global
targetY = None  # Local
targetDSquared = None
targetH = None
targetDesiredAngle = 45

# Symbolic names for where we are birding to. These can be set by roles to
# ensure birds continue across role switches. They are not used within
# sBirdOfPrey except that resetPerform() will reset birdTo to None 
# (e.g. when it perform returns STATE_SUCCESS
birdTo              = None
BIRD_TO_BALL        = 1
BIRD_TO_POS         = 2

TARGET_PT_ACC_SMALL_CIRCLE = 30
#TARGET_PT_ACC_LARGE_CIRCLE = 30


def isBirdOfPreyTriggering(): 
    global gIsBirdOfPreyTriggering
    
    # If BOP wasn't called in the last frame, reset the trigger value.
    if gLastCalledFrame == Global.frame - 1:
        return gIsBirdOfPreyTriggering    
    
    resetPerform()
    return False
    
# Returns true if we are more than offset in front of an upside-down v-shape
# with apex at (tx, ty). The apex of the V is angle degrees. Offset?,
# includeYou?
def areWeAboveTheLine(offset, angle, includeYou, tx, ty):
    global targetX, targetY
    # Draw the line.
    ang  = 90 - angle / 2.0
    m    = math.tan(hMath.DEG2RAD(ang))
    bpos = (ty + offset) - m * tx 
    bneg = (ty + offset) + m * tx
           
    selfX, selfY = Global.selfLoc.getX(), Global.selfLoc.getY()
                
    # If you are far back then you are ok (hysterisis).
    if (includeYou and selfY < Constant.GOALBOX_DEPTH)\
        or selfY < 40:
        if Debug.defenderTriggerDebug:
            print "No problem because enough back!"
        return False
    
    # If I am behind the threshold line, no need to trigger.
    if includeYou and (selfY < m*selfX + bpos) and (selfY < -m*selfX + bneg):
        if Debug.defenderTriggerDebug:
            print "False because of the line!"
        return False
       
    return True

def resetPerform(): 
    global gIsBirdOfPreyTriggering, targetX, targetY, targetDSquared, targetH, birdTo
    targetX = targetY = targetDSquared = targetH = birdTo = None
    gIsBirdOfPreyTriggering = False
    
# Perform bird to target (tx, ty) with desired heading to target ta.
def perform(tx, ty, ta = 45): 
    global gIsBirdOfPreyTriggering, targetX, targetY, targetDSquared, targetH
    global gLastCalledFrame
    global boxPointOffset,\
           cornerCut,\
           maxTurn,\
           insideBound,\
           outsideBound,\
           lastDBH,\
           currentBopMode,\
           targetDesiredAngle,\
           lockDefender,\
           minAngle,\
           breakOutMinAngle,\
           breakOutIncAngle 

    targetX, targetY = tx, ty
    targetH = hMath.normalizeAngle_180(hMath.getHeadingBetween\
        (Global.selfLoc.getX(), Global.selfLoc.getY(), targetX, targetY)\
            - Global.selfLoc.getHeading())
    targetDSquared = hMath.getDistSquaredBetween(targetX, targetY, \
                                Global.selfLoc.getX(), Global.selfLoc.getY())
    targetDesiredAngle = ta
    
    if shouldIEnd(): 
        resetPerform()
        return Constant.STATE_SUCCESS

    Indicator.showFacePattern([1, 2, 1, 2, 1])
    
    gLastCalledFrame = Global.frame
    
    gIsBirdOfPreyTriggering = True
    
    # Checking preconditions.
    desiredTargetAngle = targetDesiredAngle

    goalx = (Constant.FIELD_WIDTH / 2 + targetX) / 2
    dX    = targetX - goalx
    dY    = targetY
    
    # Determine if we are around the right side of the target.
    usToGoal = math.sqrt(hMath.SQUARE(Global.selfLoc.getPos()[0] - goalx) + \
               hMath.SQUARE(Global.selfLoc.getPos()[1]))
    targetToGoal = math.sqrt(hMath.SQUARE(dX) + hMath.SQUARE(dY))

    if abs(dX) < 0.1:
        if Debug.defenderDebug:
            print "pa"
        if Global.selfLoc.getPos()[0] < goalx:
            if Debug.defenderDebug:
                print "pb"
            desiredBH = desiredTargetAngle
        else:
            if Debug.defenderDebug:
                print "pc"
            desiredBH = -desiredTargetAngle
            
    else:
        if Debug.defenderDebug:
            print "pd"
        m = dY / dX
        b = targetY - m * targetX
        if abs(m) < 0.01:
            desiredBH = 0
        elif dY < 0:
            # This is an error that appeared; target must be close to back line.
            if targetX > Constant.FIELD_WIDTH / 2:
                desiredBH = desiredTargetAngle
            else:
                desiredBH = -desiredTargetAngle
        elif lastDBH > 0:
            if Global.selfLoc.getPos()[0] < ((Global.selfLoc.getPos()[1] - b) / m) + 5:
                desiredBH = desiredTargetAngle
            else:
                desiredBH = -desiredTargetAngle
        else:
            if Global.selfLoc.getPos()[0] < ((Global.selfLoc.getPos()[1] - b) / m) - 5:
                desiredBH = desiredTargetAngle
            else:
                desiredBH = -desiredTargetAngle
        

    if usToGoal < targetToGoal:
        dp = (Global.selfLoc.getPos()[0] - goalx) * dX \
                + Global.selfLoc.getPos()[1] * dY
        
        # use dot product to calculate angle between
        # robot vector and target vector (both from defend pt)
        cosAng = dp / (usToGoal * targetToGoal)
        ang    = math.acos(cosAng)
        dist   = usToGoal * math.sin(ang)
        
        if math.degrees(ang) < minAngle:
            minAngle = math.degrees(ang)
        
        if targetDSquared < hMath.SQUARE(insideBound * 2)\
            or dist < insideBound\
            or (currentBopMode == Constant.CIRCLE_BOP and dist < outsideBound):
            
            if currentBopMode == Constant.CIRCLE_BOP:
                if lastDBH > 0:
                    turnCCW = maxTurn
                else:
                    turnCCW = -maxTurn
            else:
                if desiredBH > 0:
                    turnCCW = maxTurn
                else:
                    turnCCW = -maxTurn
                    
            currentBopMode = Constant.CIRCLE_BOP
            Action.walk(Action.MAX_FORWARD, 0, turnCCW,
                        minorWalkType=Action.SkeFastForwardMWT)
            return Constant.STATE_EXECUTING

    continue_(desiredBH) # function too long for python - so split it in two
    return Constant.STATE_EXECUTING
    
def continue_(desiredBH):
    global lastDBH, currentBopMode, cornerCut
    
    lastDBH = desiredBH
    box  = (Global.selfLoc.getPos()[1] < ((Constant.GOALBOX_DEPTH) + 25)) \
            or  (((currentBopMode == Constant.AVOIDBOX_BOP) \
                or (currentBopMode == Constant.POSTAVOID_BOP)) \
            and (Global.selfLoc.getPos()[1] < ((Constant.GOALBOX_DEPTH) + 50)))

    # Alex Osaka - don't do goal box avoidance but we quit the bird earlier
    box = False

    #if box:
    #    Global.myRole = Constant.DEFENDERGOALBOX
               
    # Global coordinate of where exactly I am heading at.
    desiredHeading = hMath.normalizeAngle_0_360(Global.selfLoc.getHeading() + \
                     (targetH - desiredBH))
                     
    # Move with goalbox avoidance
    if (box and (desiredBH > 0) and (Global.selfLoc.getPos()[0] < \
          ((Constant.FIELD_WIDTH + Constant.GOALBOX_WIDTH) / 2.0) - cornerCut) \
          and (desiredHeading > 180) and (desiredHeading < (350))):
        print "box avoid 1, desiredHeading =", desiredHeading
        currentBopMode = Constant.AVOIDBOX_BOP
        px = ((Constant.FIELD_WIDTH + Constant.GOALBOX_WIDTH) / 2.0) \
                + boxPointOffset
        py = (Constant.GOALBOX_DEPTH) + boxPointOffset
        phead = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(py \
                    - Global.selfLoc.getPos()[1], \
                px - Global.selfLoc.getPos()[0])) - \
                    Global.selfLoc.getHeading())

        Action.walk(Action.MAX_FORWARD, 0, hMath.CLIP(phead / 2.0, maxTurn), \
                    minorWalkType=Action.SkeFastForwardMWT)
        return
        
    elif (box and (desiredBH < 0) and (Global.selfLoc.getPos()[0] > \
          ((Constant.FIELD_WIDTH - Constant.GOALBOX_WIDTH) / 2.0) + cornerCut) \
          and (desiredHeading > 190)):
        
        print "box avoid 2, desiredHeading =", desiredHeading
        currentBopMode = Constant.AVOIDBOX_BOP
        px = ((Constant.FIELD_WIDTH - Constant.GOALBOX_WIDTH) / 2.0) - \
                boxPointOffset
        py = (Constant.GOALBOX_DEPTH) + boxPointOffset

        phead = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(py - \
                    Global.selfLoc.getPos()[1],\
                px - Global.selfLoc.getPos()[0])) - Global.selfLoc.getHeading())

        Action.walk(Action.MAX_FORWARD, 0, hMath.CLIP(phead / 2.0, maxTurn), \
                    minorWalkType=Action.SkeFastForwardMWT)
        return 
        
    elif (box and ((currentBopMode == Constant.AVOIDBOX_BOP) or \
            (currentBopMode == Constant.POSTAVOID_BOP))):
        currentBopMode = Constant.POSTAVOID_BOP
        relh = targetH - desiredBH
        if abs(relh) < 10:
            currentBopMode = Constant.NORMAL_BOP
        else:
            Action.walk(Action.MAX_FORWARD, 0, hMath.CLIP(relh / 2.0, maxTurn),
                        minorWalkType=Action.SkeFastForwardMWT)
            if Debug.defenderDebug:
                print  "!!!!!"
            return
    else:
        currentBopMode = Constant.NORMAL_BOP

    if Debug.defenderDebug:
        print "Before hoverToBall - currentBopMode: %f" %currentBopMode
        
    # Hover To Ball
    #relH    = Global.ballH - desiredBH
    relH    = targetH - desiredBH

    if Debug.defenderDebug:
        print "relH: %f" %relH
  
    #Global.ballH = relH # ARGH I HATE LAST YEAR'S TEAM! SERIOUSLY, WTF?!?!
                         # I agree! =p

    distSquared = True
    sHoverToBall.perform(targetDSquared, relH, distSquared)

    if gUseDodgyDog and sDodgyDog.shouldIBeDodgyAlongHeading(relH):
        sDodgyDog.dodgyDogAlongHeading(relH)
    

def getWalkToPosition(tx, ty, targetOffset=50):
    relX = tx - Constant.OWN_GOAL_X
    relY = ty - Constant.OWN_GOAL_Y
    
    if relX == 0:
        wx = Constant.OWN_GOAL_X
        wy = targetY - targetOffset 
    else:    
        m = (relY + 0.0) / relX
        theta = math.atan(m)
        offsetY = targetOffset * math.sin(theta)
        offsetX = targetOffset * math.cos(theta) 

        wx = tx - offsetY
        wy = ty - offsetX

    return (wx, wy)
    
# Must be called after a perform() so targetX/Y/D/H set
def shouldIEnd(): 
    global targetX, targetY, targetDSquared, targetH
    global boxPointOffset,\
        cornerCut,\
        maxTurn,\
        insideBound,\
        outsideBound,\
        lastDBH,\
        currentBopMode,\
        targetDesiredAngle,\
        lockDefender,\
        minAngle,\
        breakOutMinAngle,\
        breakOutIncAngle

    DEFENCE_OFFSET = 0
    DEFENCE_ANGLE  = 120.0
    if not areWeAboveTheLine(DEFENCE_OFFSET, DEFENCE_ANGLE, False, \
                             targetX, targetY):
        return True
       
    # If in place, don't become the bird anymore.
    if currentBopMode == Constant.CIRCLE_BOP and abs(targetH) < 20:
        currentBopMode = Constant.NORMAL_BOP
        return True

    # If we've reached the goal box don't bird anymore
    if Global.selfLoc.getY() < Constant.GOALBOX_DEPTH + 25:
        return True
            
    tx, ty = getWalkToPosition(targetX, targetY)    
    selfD2TargetSquared = hMath.getDistSquaredBetween(Global.selfLoc.getX(),\
                                            Global.selfLoc.getY(),\
                                            tx,\
                                            ty)
    
    if selfD2TargetSquared < hMath.SQUARE(TARGET_PT_ACC_SMALL_CIRCLE):
        currentBopMode = Constant.NORMAL_BOP
        return True       
    
        
    goalX = (Constant.FIELD_WIDTH / 2 + targetX) / 2        
    selfX, selfY = Global.selfLoc.getX(), Global.selfLoc.getY()
    ## Determine if we are around the right side of the ball.
    selfD2Goal = hMath.getDistanceBetween(selfX,selfY,goalX,0)
    targetD2Goal = hMath.getDistanceBetween(targetX, targetY, goalX, 0)    
       
    if selfD2Goal < targetD2Goal:
        dX = targetX - goalX   
        dY = targetY
        dp = (Global.selfLoc.getPos()[0] - goalX) * dX + Global.selfLoc.getPos()[1] * dY
        
        ## Quit due to something being to close to goal (should never happen)
        if abs(selfD2Goal) < 0.1 or abs(targetD2Goal) < 0.1:
            return True
        
        ## use dot product to calculate angle between
        ## robot vector and target vector (both from defend pt)
        cosAng = dp / (selfD2Goal * targetD2Goal)
        ang    = math.acos(cosAng)
        
        if math.degrees(ang) < minAngle:
            minAngle = math.degrees(ang)
       
        ## Quit due to crossing the line from target to goal 
        if minAngle < breakOutMinAngle:
            return True
        
        ## Quit due to angle of line from target to goal increasing
        if ang - minAngle > breakOutIncAngle:
            return True
            
        else:
            if currentBopMode == Constant.CIRCLE_BOP:
                currentBopMode = Constant.NORMAL_BOP
                return True
                    
    else:
        if currentBopMode == Constant.CIRCLE_BOP:
            currentBopMode = Constant.NORMAL_BOP
            return True
                
    return False 
    

