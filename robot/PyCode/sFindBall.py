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
import hMath
import hTrack
import hTeam
import hWhere
import Indicator
import math
import VisionLink

CLOSE_BALL_DIST = 30
MID_BALL_DIST = 40
FAR_BALL_DIST = 150 

WIRELESS_BALL_CLOSE_DIST = 70

SPIN_BREAK_TIME = 15

# Number of frames a ball hint is valid for
HINT_VALID_FRAMES = 15

# Get behind constants control the amount of getBehind
GET_BEHIND_NONE     = 0 # No get behind
GET_BEHIND_DEFAULT  = 1 # Default get behind, includes default adjustments
GET_BEHIND_MORE     = 2 # Force doing more getBehind than usual
GET_BEHIND_LOTS     = 3 # Force lots of getbehind
GET_BEHIND_PRIORITY = 4 # Sacrifice foward speed for getting behind
GET_BEHIND_ONLY     = 5 # Don't walk forwards, only get behind
GET_BEHIND_GOALIE   = 6 # Get behind for the goalie

# gHint is a tuple of (distance, heading) that can be set by other behaviours
# by calling setHint(). Behaviours should call this when they are performing
# actions that may move the ball making it difficult to track, e.g. kicking
# Heading zero degrees is to the right.
gHint = None
gHintFrame = 0

gLastCloseFrame = 0  # The last frame when ball was close.

gLastTurnFrame = 0

# Force related variables. 
# This force action is very to similar to hints.
FORCE_VALID_FRAMES = 15
FORCE_SPIN = 1
FORCE_FOLLOW = 2
FORCE_LAST_VISUAL = 3
gForceFrame = 0
gForceType = 0
gForceFired = False      # This goes to True, when we start the force action.
gForceCounter = 0
gForceSpinDir = 0

gLastBackFrame = 0
gStartSpinFrame = 0
gLastSpinFrame = 0 

gIsClockwise = True

gGetBehind = GET_BEHIND_DEFAULT

gHeadOnly = False

gGoalie = False


def DecideNextAction():  
    
    Indicator.setDefault() 
    Indicator.showBallIndicator()
    r = perform()
    if r == Constant.STATE_SUCCESS: 
        resetPerform() 
        

def resetPerform():     
    global gForceType       
    global gForceCounter
    global gForceFired
    global gForceSpinDir
    gForceType = 0
    gForceCounter = 0
    gForceFired = False
    gForceSpinDir = 0
    

# Set the ball hint (local)
def setHint(dist, head):
    global gHint, gHintFrame
    gHint = (dist, head)
    gHintFrame = Global.frame


def setForce(forceType,forceSpinDir=None):
    global gForceFrame
    global gForceType
    global gForceFired
    global gForceCounter
    global gForceSpinDir 
    
    gForceFrame = Global.frame
    gForceType = forceType 
    gForceFired = False
    if gForceType == FORCE_LAST_VISUAL: 
        gForceCounter = Constant.LOST_BALL_GPS
    else:    
        gForceCounter = Constant.LOST_BALL_SCAN
    gForceSpinDir = forceSpinDir
    

# It checks for any special actions for find ball to be forced.
def shouldIForce(): 
    return Global.frame - gForceFrame <= FORCE_VALID_FRAMES\
        or (gForceFired and gForceCounter > 0)          

def perform(headOnly = False, doGetBehind = GET_BEHIND_DEFAULT): 
    global gHeadOnly, gGetBehind
    global gForceCounter, gForceSpinDir
    
    global gLastCloseFrame
    global gLastBackFrame
    global gStartSpinFrame
    global gLastSpinFrame
            
    gHeadOnly = headOnly   
    gGetBehind = doGetBehind
    #print "Camera Frame : ", Global.cameraFrame, 

    if Global.ballD < CLOSE_BALL_DIST: 
        gLastCloseFrame = Global.frame

    # If the ball distance is closer than CLOSE_BALL_DIST,
    # then turn on the grabbing mode, so we can get better ball detection.
    if Global.lostBall < Constant.LOST_BALL_GPS\
        and Global.frame - gLastCloseFrame < 5:
        VisionLink.setGrabbing()
    

    ## Decide which state to be in
    if Global.vBall.isVisible():     
        #print " trackBall()", 
        #print " vball (", Global.vBall.getX(),\
        #  ", ", Global.vBall.getY(),\
        #  ", ", Global.vBall.getHeading(),")", 
        trackBall()
                
    else:          
        
        # If we need to force something, force it.        
        if shouldIForce():
            findByForce()
        
        # Do normal routine from here.
        elif Global.lostBall < Constant.LOST_BALL_LAST_VISUAL:     
            #print " findByLastVisual()",
            findByLastVisual()
        
        elif gHint != None and Global.lostBall < Constant.LOST_BALL_HINT\
            and Global.frame - gHintFrame < HINT_VALID_FRAMES:
            #print " findByHint()",
            findByHint() 
                                      
        elif Global.lostBall < Constant.LOST_BALL_GPS:
                        
            if VisionLink.gameDataIsValid() and VisionLink.getDropInTime() <= 2:        
                findByDropInSignal()
                     
            elif not findByImageEdges():                
                
                # We were recently chasing after a close ball.
                # Maybe use weighted visual ball distance and heading.
                if Global.frame - Global.lostBall - gLastCloseFrame < 5:
                    #print " findByWeightedVisBall"
                    findByWeightedVisBall()            
                else:
                    #print " findByGps()",
                    findByGps()                    
        
        # If you see no green features, then may be we are in scrum or against
        # wall.  So back off and do some dodgying to be in better place to see
        # the ball.       
        elif not gHeadOnly\
            and Global.lostBall < Constant.LOST_BALL_SCAN\
            and (Global.frame - gLastBackFrame < Constant.LOST_BALL_SCAN - Constant.LOST_BALL_GPS
                or (abs(hMath.MICRORAD2DEG(VisionLink.getAnySensor(Constant.ssHEAD_PAN))) <= 45 and VisionLink.getFieldFeatureCount() <= 0)):
            gLastBackFrame = Global.frame
            #print " findByBackOff"
            findByBackingOff()
                    
        # Use scanning except when we were spinning just then.     
        elif gHeadOnly\
            or (Global.lostBall < Constant.LOST_BALL_SCAN
                and Global.frame - gLastSpinFrame > Constant.LOST_BALL_GPS + 1):
            #print " findByScan"
            findByScan()
                    
        # Use wireless only when: 
        # 1. Not head only. 
        # 2. The ball source is wireless. 
        # 3. The robot is not in the middle of spinning.       
        elif not gHeadOnly\
            and Global.ballSource == Constant.WIRELESS_BALL\
            and Global.frame - gStartSpinFrame > Constant.LOST_BALL_SPIN - Constant.LOST_BALL_SCAN - 100:
            #and Global.teammatesLoc[Global.sharedBallSourceRobot-1].getPosVar() < hMath.get95var(50):
            findByWireless()
        
        else:
            if Global.frame - gLastSpinFrame > 1: 
                gStartSpinFrame = Global.frame    
            gLastSpinFrame = Global.frame 
            findBySpin()   
            
    #print ""                       
    #print " lostBall :",Global.lostBall
    #print " lball (", Global.gpsLocalBall.getX(),\
    #       ", ", Global.gpsLocalBall.getY(),\
    #       ", ", Global.gpsLocalBall.getHeading(),")"
    #print "sFindBall : ", str(Action.finalValues[Action.Forward:])
    #print ""
    
    return Constant.STATE_EXECUTING
          



def trackBall(): 
    global gIsClockwise
    
    resetPerform()
 
    # ignore first two visual ball info when the robot was spinning.
    if Global.frame - gLastSpinFrame > 1:
        vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
        velX, velY = vel[0], vel[1]
        #print " vel (",vel[0],",",vel[1],")", 
        if abs(velX) > 2 or abs(velY) > 2: 
            #print " velocity, ",
            ballX = Global.gpsLocalBall.getX() - velX# * 5
            ballY = Global.gpsLocalBall.getY() + velY# * 5
            ballD = hMath.getDistanceBetween(0,0,ballX,ballY)
        else: 
            ballD = Global.ballD

        if ballD < 40: 
            hTrack.panLow = True
        else:
            hTrack.panLow = False

        # Check which direction does the ball appear from the image
        # 1 = top right, 2 = bottom right, 
        # 3 = top left, 4 = bottom left
        imgDir = Global.vBall.getImgDirection() 
        if imgDir == 1 or imgDir == 2: 
            hTrack.panDirection = Constant.dCLOCKWISE
            gIsClockwise = True
        elif imgDir == 3 or imgDir == 4: 
            hTrack.panDirection = Constant.dANTICLOCKWISE 
            gIsClockwise = False        
        
    if not gHeadOnly:
        walkToBall(Global.ballD, Global.ballH, getBehind = gGetBehind)
        
    hTrack.trackVisualBall(useVelocity = (Global.frame - gLastCloseFrame >= 5),
                           isTurning = (Global.frame - gLastTurnFrame < 5))
   

def findByForce(): 
    global gForceType
    global gForceFired
    global gForceCounter
    global gForceSpinDir 

    # If we need to force anything, then force it.
    gForceFired = True        
    if gForceCounter > 0:
        if gForceType == FORCE_SPIN:
            findBySpin(gForceSpinDir)
        
        elif gForceType == FORCE_FOLLOW:
            if gForceCounter < 20:
                findByGps()
            else: 
                hTrack.scan()
                if not gHeadOnly:
                    Action.walk(Action.MAX_FORWARD,0,0)

        elif gForceType == FORCE_LAST_VISUAL: 
            hTrack.scan()
            if not gHeadOnly: 
                walkToBall(Global.fstVisBallDist, Global.fstVisBallHead,
                           getBehind = gGetBehind)  
            Global.lostBall = 0          

        gForceCounter -= 1    


def findByLastVisual(): 
    pan_sensor  = hMath.MICRORAD2DEG(VisionLink.getAnySensor(Constant.ssHEAD_PAN))
    
    # Clip the desired values to the min / max sensor values
    pan = min(max(Global.desiredPan,-88),88)
    
    hTrack.trackVisualBall()     
    if Global.frame - gLastSpinFrame > SPIN_BREAK_TIME: 
        if abs(pan - pan_sensor) > 12:
            Global.lostBall = max(Global.lostBall-1,0)              
    
    if not gHeadOnly:  
        walkToBall(Global.fstVisBallDist, Global.fstVisBallHead,
                   getBehind = gGetBehind)


def findByHint():
    x, y = hMath.polarToCart(*gHint)
    Action.setHeadParams(x, Constant.BallDiameter, y, Action.HTAbs_xyz)


def findByImageEdges(): 

    ballH = Global.lastVisBall.getHeading()
    
    if ballH == None:
        return False

    # If we lost the ball in the bottom edge of the image and 
    # ball heading was reasonably straight.
    # This seem to work well.
    cy = Global.lastVisBall.getCentroid()[1]
    radius = Global.lastVisBall.getHeight() / 2.0
    if cy - radius > Constant.IMAGE_HEIGHT * 0.6:
        if abs(ballH) < 15:
            #print " findByDown()",
            Action.setHeadParams(0,0,3.5*Constant.BallRadius, Action.HTAbs_xyz)
        
        elif ballH > 0: 
            #print " findByLeft()"
            Action.setHeadParams(3,0,3*Constant.BallRadius,Action.HTAbs_xyz)
        else:
            #print " findByRight()"
            Action.setHeadParams(-3,0,3*Constant.BallRadius,Action.HTAbs_xyz)
        
        if not gHeadOnly:            
            Action.walk(10,0,0,minorWalkType=Action.SkeFastForwardMWT)
        return True
        
    return False   


def findByDropInSignal():
    hTrack.scan()    
    if not gHeadOnly:
        # Make sure we use gps's information, not wireless 
        # FIXME: if we are going to dodgy dog then don't get behind
        walkToBall(Global.gpsLocalBall.getDistance(),
                   Global.gpsLocalBall.getHeading(),
                   getBehind = gGetBehind)

# We were recently chasing after a close ball.
# Maybe use weighted visual ball distance and heading.
def findByWeightedVisBall(): 
    d = Global.weightedVisBallDist 
    h = Global.weightedVisBallHead
    if abs(h) < 90:
        x = d * math.sin(hMath.DEG2RAD(h))
        y = d * math.cos(hMath.DEG2RAD(h))
        Action.setHeadParams(x,Constant.BallDiameter,y,Action.HTAbs_xyz)
    else:
        Action.setHeadToLastPoint()    

    if not gHeadOnly:
        # Make sure we use gps's information, not wireless 
        # FIXME: if we are going to dodgy dog then don't get behind
        walkToBall(d,h,getBehind = gGetBehind)


def findByGps():
    if abs(Global.gpsLocalBall.getHeading()) < 90: 
        #print "under 90",           
        hTrack.trackGpsBall()
    else: 
        #print "over 90",
        Action.setHeadToLastPoint()

    if not gHeadOnly:
        # Make sure we use gps's information, not wireless 
        # FIXME: if we are going to dodgy dog then don't get behind
        walkToBall(Global.gpsLocalBall.getDistance(),
                   Global.gpsLocalBall.getHeading(),
                   getBehind = gGetBehind)
            

def findByWireless():  

    hTrack.scan(highSpeed=10,minPan=-60,maxPan=60)
    
    if not gHeadOnly:      
        selfX, selfY = Global.selfLoc.getPos()
        # Make sure we get a ball distance from wireless info.
        if Global.ballD < WIRELESS_BALL_CLOSE_DIST: 
            findBySpin()
        else:             
            Action.walk(0,0,Global.ballH,minorWalkType=Action.SkeFastForwardMWT)


def findByBackingOff():   
    
    hTrack.scan()
    Action.walk(-Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)    

    # After you backed off for a while, then use cccluded ball strategy if we need it.
    if Global.lostBall >= Constant.LOST_BALL_GPS + 20:
        ballX, ballY = Global.gpsGlobalBall.getPos()        
        import sDodgyDog
        if sDodgyDog.shouldIBeDodgy(ballX,ballY): 
            sDodgyDog.dodgyDogTo(ballX,ballY) 
        

def findByScan(): 

    if gHeadOnly:
        hTrack.scan()
    
    else:             
        # If the last seen ball is far away, then continue walking for a while.
        # Use obstacle occluded as well?
        if Global.gpsLocalBall.getDistance() > 150:            
            hTrack.scan()
            #hTrack.scan(minPan=-60,maxPan=60,lookDown=False)
            Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)     
                   
        else:
            hTrack.scan()
            Action.walk(0,0,0,minorWalkType=Action.SkeFastForwardMWT)
    

# In this function, pan increments by 10 or -10 until it reaches max pan value.
# If we give max pan value straight away, the head motor will try to move to that value as fast as possible.
# This is not good, if we actually saw a ball while heading is moving to that max pan value. 
# Because head is moving as fast as possible, it cannot change the head direction quickly,
# hence over turns. 
def findBySpin(turnDir=None):
    pan = Global.desiredPan
    
    turnRate = Action.MAX_TURN
    if Global.lightingChallenge:
        turnRate = 60

    # if turnDir is specified, then force to spin that direction.
    if turnDir != None: 
        if turnDir == Constant.dANTICLOCKWISE:             
            turnCCW = turnRate
            pan += 10
        else:
            pan -= 10
            turnCCW = -turnRate

    # Decide which direction to spin.
    elif gIsClockwise: 
        turnCCW = -turnRate
        pan -= 10
    else: 
        turnCCW = turnRate
        pan += 10

    pan = hMath.CLIP(pan,90)

    Action.setHeadParams(pan,-10,-8,Action.HTAbs_h)
    Action.walk(0,0,turnCCW,minorWalkType=Action.SkeFastForwardMWT) 



# Walks to the "ball" at local (ballD, ballH). If useVelocity and ball velocity
# is above some threshold then will walk to a predicted interception point,
# taking walk speed and ball velocity into account. If getBehind (and velocity
# is not used) then the path to the ball will be adjusted to favour moving
# a) between the ball and our goal or b) to line up the ball with the target
# goal depending on our position
gLastMovingBallFrame = 0
gLastVelX = 0
gLastVelY = 0
FRAME_TO_PREPARE = 10
DISTANCE_CONSTANT = 10.0
def walkToBall(ballD, ballH, getBehind = GET_BEHIND_NONE, useVelocity = False): 
    global gLastMovingBallFrame     
    global gLastVelX
    global gLastVelY
    global gGoalie
    
    if getBehind == GET_BEHIND_GOALIE:
        getBehind = GET_BEHIND_PRIORITY
        gGoalie = True
    
    vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
    velX, velY = vel[0], vel[1] 
    
    if abs(velX) > 2 or velY < -2:
        gLastMovingBallFrame = Global.frame
        gLastVelX = velX
        gLastVelY = velY
            
    if useVelocity\
        and Global.frame - gLastMovingBallFrame < 15\
        and walkToMovingBall(ballD,ballH,gLastVelX,gLastVelY):
        return         
    else:                                
        walkToStillBall(ballD, ballH, getBehind)        
        
     
     
def walkToStillBall(ballD, ballH, getBehind = GET_BEHIND_NONE): 
    global gLastTurnFrame 
    global gGoalie
       
    if getBehind == GET_BEHIND_GOALIE:
        getBehind = GET_BEHIND_PRIORITY
        gGoalie = True    
    
    fwd = Action.MAX_FORWARD

    # Lighting challenge and penalty shooter don't get behind. It makes the
    # grab a bit harder and takes time
    if Global.lightingChallenge or Global.penaltyShot:
        getBehind = GET_BEHIND_NONE

    # Don't get behind when close to our goal box since 
    # this moves us towards it (unless we are the goalie)
    if not gGoalie and hWhere.selfInOwnGoalBox(20, sideoffset = 10):
        getBehind = GET_BEHIND_NONE

    # Calculate getBehind adjustment
    leftAdj = 0
    if getBehind > GET_BEHIND_NONE:
        myx, myy = Global.selfLoc.getPos()
        myh = Global.selfLoc.getHeading()
        ballx, bally = hMath.getGlobalCoordinate(myx, myy, myh, 
                                    *hMath.polarToCart(ballD, ballH+90))

        # In defense we get between our goal and the ball. In attack we line
        # up the ball and target goal. In the (small) midfield we line up with
        # upfield
        reversePoints = False
        if bally < Constant.FIELD_LENGTH/2 - 20 \
                or hTeam.amIFurthestBack(ignoreRoles = []):
            lineX, lineY = Constant.OWN_GOAL_X, Constant.OWN_GOAL_Y
        elif bally > Constant.FIELD_LENGTH/2 + 20:
            lineX, lineY = Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y
            reversePoints = True
        else:
            lineX, lineY = ballx, 0

        # Draw a line between ball and goal. Find self offset from the line
        dist = ((ballx - lineX)*(lineY - myy)-(lineX - myx)*(bally - lineY)) /\
                math.sqrt(hMath.SQUARE(ballx - lineX) \
                            + hMath.SQUARE(bally - lineY))
        # If dist > 0 we are to the right of the line, so adjustment left
        # These adjustments will only be applied assuming we are facing
        # the ball
        if dist > 15:
            leftAdj = 14
        elif dist < -15:
            leftAdj = -12
        elif myy > bally:   # Always sidestep from in front
            if dist >= 0:
                leftAdj = 14
            elif dist < 0:
                leftAdj = -12
        else:
            leftAdj = 0

        if leftAdj != 0:
            # At this point check that sidestepping in this direction is not
            # going to put obstacles between us and the ball. If so it would be
            # better to go direct (and let dodgy dog do its thing)
            (lBallX, lBallY) = Global.gpsLocalBall.getPos()
            nobs = VisionLink.getNoObstacleBetween(leftAdj, 0,
                                              int(lBallX) + leftAdj,
                                              int(lBallY) - 30,
                                              abs(leftAdj), 0,
                                              Constant.MIN_GPS_OBSTACLE_BOX,
                                              Constant.OBS_USE_GPS)
            if nobs > 2 * Constant.MIN_GPS_OBSTACLE_BOX:
                #Indicator.showFacePattern([3, 2, 0, 2, 3])
                leftAdj = 0
            
            # Multiply getBehind in some cases. Only one of these can apply at
            # at time so put them in order of importance.
            leftAdjMult = 1

            if getBehind == GET_BEHIND_DEFAULT:
                # Last man attacker must be behind
                if hTeam.amIFurthestBack(ignoreRoles = []):
                    if myy < bally:
                        getBehind = GET_BEHIND_PRIORITY
                    else:
                        getBehind = GET_BEHIND_LOTS

                # In defense do lots of getBehind
                elif bally < Constant.FIELD_LENGTH * 0.25:
                    getBehind = GET_BEHIND_LOTS

                # From in front of the ball do a little more to get behind the
                # ball
                elif myy > bally:
                    getBehind = GET_BEHIND_MORE

            # Lots of getBehind if requested
            if getBehind >= GET_BEHIND_ONLY:
                fwd = 10        # Walk in a circular arc
                leftAdjMult = 2
            elif getBehind >= GET_BEHIND_PRIORITY:
                fwd = 30        # Slow down so getBehind has more effect
                leftAdjMult = 2
            elif getBehind >= GET_BEHIND_LOTS:
                leftAdjMult = 2
            elif getBehind >= GET_BEHIND_MORE:
                leftAdjMult = 1.5

            leftAdj *= leftAdjMult 
        
            # If the line was to a target to line up, reverse direction
            if reversePoints:
                leftAdj = -leftAdj

    #Indicator.showFacePattern([0]*5)

#    if leftAdj > 0:
#        Indicator.showFacePattern([0, 0, 0, 2, 3])
#    elif leftAdj < 0:
#        Indicator.showFacePattern([3, 2, 0, 0, 0])

#     if gGoalie:
#         Indicator.setDefault()
        
#     # DANGER CASE: Goalie Facing Backwards 
#     if gGoalie and (225 <= myh < 315):
#         Indicator.showHeadColor(Indicator.RGB_ORANGE)
#         #ball to goalie's right
#         if ballx <= myx:
#             print "Ball to my RIGHT"
#             Action.walk(-5,Action.MAX_LEFT,-Action.MAX_TURN,
#             "ddd",minorWalkType=Action.SkeFastForwardMWT) 
#         #ball to goalie's left
#         else:
#             print "Ball to my LEFT"
#             Action.walk(-5,-Action.MAX_LEFT,Action.MAX_TURN,
#             "ddd",minorWalkType=Action.SkeFastForwardMWT)   
            
            
    # Walk direct - with getBehind adjustment
    if abs(ballH) < 30:
#         if gGoalie:
#             Indicator.showHeadColor(Indicator.RGB_GREEN)
        Action.walk(fwd, leftAdj, ballH,
                    minorWalkType=Action.SkeFastForwardMWT)
    # Walk and turn to face
    elif abs(ballH) < 80:                                  
        gLastTurnFrame = Global.frame
        fwd = max(ballD * math.cos(ballH),0)
        left = ballD * math.sin(ballH)
#         if gGoalie:
#             left = leftAdj
#             Indicator.showHeadColor(Indicator.RGB_BLUE)
        Action.walk(fwd,left,ballH,"ddd",minorWalkType=Action.SkeFastForwardMWT)
    
    # Turn on the spot
    else: 
        gLastTurnFrame = Global.frame
        fwd = 0
        left = 0
        turnccw = hMath.CLIP(ballH,60)

#         if gGoalie:
#             fwd = -Action.MAX_FORWARD
#             
#             if abs(ballx - myx) > Constant.DOG_WIDTH * 2:
#                 left = (myx - ballx) * 0.7
#             turnccw = 0
#             Indicator.showHeadColor(Indicator.RGB_PURPLE)
            
        Action.walk(fwd,left,turnccw,"ddd",minorWalkType=Action.SkeFastForwardMWT)


def walkToMovingBall(ballD,ballH,velX,velY):
    #Indicator.showFacePattern([3,3,3,3,3])
    id(ballD)
    
    #print ""
    #print "velocity X : ", velX
    #print "velocity Y : ", velY
    
    ballX = ballD * math.sin(hMath.DEG2RAD(ballH))
    ballY = ballD * math.cos(hMath.DEG2RAD(ballH))
    tx = ballX
    ty = ballY
    dx = velX
    dy = velY
    #print "ball : (", ballX, ",", ballY, ") ", ballH 

    # closest target point so far
    minTarget = (Constant.LARGE_VAL,0,0)
    
    # try out next 150 frames which is 5 seconds
    for i in range(150):         
                   
        tty = ty / 1.5 # assuming we walking in 2 cm / frame (over-estimating)
        ttx = tx / 1.0
        tt = max(tty,ttx)        
        
        # If the robot can reach to the target point comfortably,
        # then go to the target point.        
        if tt + FRAME_TO_PREPARE < i:       

            #print "success"           
            #print "target : (", tx, ",", ty, ") "            
            
            fwd = ty
            left = tx
            if left > 0:
                left = hMath.EXTEND(left,Action.MAX_SKE_FF_LEFT_STP*0.7)
            elif left < 0:
                left = hMath.EXTEND(left,Action.MAX_SKE_FF_RIGHT_STP*0.7)
        
                
            th = hMath.getHeadingToRelative(tx,ty)
            turnCCW = hMath.normalizeAngle_180((th + ballH) / 2)
            Action.walk(fwd,left,turnCCW,"ddd",minorWalkType=Action.SkeFastForwardMWT)
            return True
        
        # If this target point has the smallest difference of time for 
        # ball and time for robot to reach the point, then consider 
        # this target point.
        elif minTarget[0] > tt - i: 
            minTarget = (tt-i,tx,ty)            
                
        # updating next possible target point
        tx += dx
        ty += dy
        
        # updating velocity by decaying
        dx *= 0.98
        dy *= 0.98
    
    
    if minTarget[0] != Constant.LARGE_VAL:    
        fwd = minTarget[2] 
        left = minTarget[1]
        if left > 0:
            left = hMath.EXTEND(left,Action.MAX_SKE_FF_LEFT_STP*0.7)
        elif left < 0:
            left = hMath.EXTEND(left,Action.MAX_SKE_FF_RIGHT_STP*0.7) 
        
        th = hMath.getHeadingToRelative(tx,ty)
        turnCCW = hMath.normalizeAngle_180((th + ballH) / 2)         
        Action.walk(fwd,left,turnCCW,"ddd",minorWalkType=Action.SkeFastForwardMWT)
        return True

    return False
       
            
    
