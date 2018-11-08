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
import Indicator
import math
import VisionLink


CLOSE_BALL_DIST = 20
MID_BALL_DIST = 40
FAR_BALL_DIST = 150 

WIRELESS_BALL_CLOSE_DIST = 50

SPIN_BREAK_TIME = 15

# Number of frames a hint is valid for
HINT_VALID_FRAMES = 15

# gHint is a tuple of (distance, heading) that can be set by other behaviours
# by calling setHint(). Behaviours should call this when they are performing
# actions that may move the ball making it difficult to track, e.g. kicking
# Heading zero degrees is to the right.
gHint = None
gHintFrame = 0

gLastBackFrame = 0
gStartSpinFrame = 0
gLastSpinFrame = 0 

gIsClockwise = True

gHeadOnly = False

def DecideNextAction():  
    #print "Camera Frame : ", Global.cameraFrame
    Indicator.setDefault() 
    Indicator.showBallIndicator()
    r = perform()
    if r == Constant.STATE_SUCCESS: 
        resetPerform() 
    

def resetPerform(): 
    global gLastBackFrame

    global gStartSpinFrame
    global gLastSpinFrame
    
    gLastBackFrame = 0
    gStartSpinFrame = 0
    gLastSpinFrame = 0


# Set the ball hint (local)
def setHint(dist, head):
    global gHint, gHintFrame
    gHint = (dist, head)
    gHintFrame = Global.frame

def perform(headOnly=False): 
    global gHeadOnly 
    global gLastBackFrame
    global gStartSpinFrame
    global gLastSpinFrame
          
    gHeadOnly = headOnly   
    #print "Camera Frame : ", Global.cameraFrame, 

    ## Decide which state to be in
    if Global.vBall.isVisible():     
        #print " trackBall()", 
        #print " vball (", Global.vBall.getX(),\
        #  ", ", Global.vBall.getY(),\
        #  ", ", Global.vBall.getHeading(),")", 
        trackBall()
         
    else:  

        if Global.lostBall < Constant.LOST_BALL_LAST_VISUAL:     
            #print " findByLastVisual()",
            findByLastVisual()
        
        elif gHint != None and Global.lostBall < Constant.LOST_BALL_HINT\
            and Global.frame - gHintFrame < HINT_VALID_FRAMES:
            #print " findByHint()",
            findByHint() 
                                      
        elif Global.lostBall < Constant.LOST_BALL_GPS:             
            if not findByImageEdges():
                #print " findByGps()",
                findByGps()                    
        
        # If you see no green features, then may be we are in scrum or against wall. 
        # Thus back off and do some dodgying to be in better place to see the ball.       
        elif not gHeadOnly\
            and Global.lostBall < Constant.LOST_BALL_SCAN\
            and (Global.frame - gLastBackFrame < Constant.LOST_BALL_SCAN - Constant.LOST_BALL_GPS
                or VisionLink.getFieldFeatureCount() <= 0):
            gLastBackFrame = Global.frame
            #print " findByBackOff"
            findByBackingOff()
                    
        # Use scanning except when we were spinning just then.     
        elif gHeadOnly\
            or (Global.lostBall < Constant.LOST_BALL_SCAN
                and Global.frame - gLastSpinFrame > Constant.LOST_BALL_GPS + 1):
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
            
                             
    #print " lostBall :",Global.lostBall
    #print " lball (", Global.gpsLocalBall.getX(),\
    #      ", ", Global.gpsLocalBall.getY(),\
    #      ", ", Global.gpsLocalBall.getHeading(),")"
    #print "sFindBall : ", str(Action.finalValues[Action.Forward:])
    #print ""
    
    return Constant.STATE_EXECUTING
          



def trackBall(): 
    global gIsClockwise
 
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
            ballD = Global.weightedVisBallDist

        if ballD < 40: 
            hTrack.panLow = True
        else:
            hTrack.panLow = False

        # Check which direction does the ball appear from the image
        # 1 = top right, 2 = bottom right, 
        # 3 = top left, 4 = bottom left
        imgDir = Global.vBall.getImgDirection() 
        #if imgDir == 1 or imgDir == 4: 
        #    hTrack.panLow = False
        #elif imgDir == 2 or imgDir == 3:
        #    hTrack.panLow = True
        #else:
        #    hTrack.panLow = True

        if imgDir == 1 or imgDir == 2: 
            hTrack.panDirection = Constant.dCLOCKWISE
            gIsClockwise = True
        elif imgDir == 3 or imgDir == 4: 
            hTrack.panDirection = Constant.dANTICLOCKWISE 
            gIsClockwise = False
     
        
    if not gHeadOnly:
        walkToBall(Global.ballD, Global.ballH, getBehind = True)

    hTrack.trackVisualBall()




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
                    getBehind = True)


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
    if Global.lastVisBall.getCentroid()[1] > Constant.IMAGE_HEIGHT\
        and abs(ballH) < 15:
        #print " findByDown()",
        Action.setHeadParams(0,0,3.5 * Constant.BallRadius, Action.HTAbs_xyz)
        if not gHeadOnly:            
            Action.walk(10,0,0,minorWalkType=Action.SkeFastForwardMWT)
        return True
    return False   


def findByGps():
            
    if abs(Global.gpsLocalBall.getHeading()) < 90:            
        hTrack.trackGpsBall()
    else: 
        Action.setHeadToLastPoint()

    if not gHeadOnly:
        # Make sure we use gps's information, not wireless 
        walkToBall(Global.gpsLocalBall.getDistance(),
                   Global.gpsLocalBall.getHeading(),
                   getBehind = True)
            

def findByWireless():  

    hTrack.scan(highSpeed=10,minPan=-60,maxPan=60)
    
    if not gHeadOnly:      
        selfX, selfY = Global.selfLoc.getPos()
        # Make sure we get a ball distance from wireless info.
        ballDSquared = hMath.getDistSquaredBetween(selfX,selfY,Global.sharedBallX,Global.sharedBallY)
        if ballDSquared < hMath.SQUARE(WIRELESS_BALL_CLOSE_DIST): 
            findBySpin()
        else:             
            Action.walk(0,0,Global.ballH,minorWalkType=Action.SkeFastForwardMWT)


def findByBackingOff():   

    hTrack.scan(lookDown=False)
    Action.walk(-Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)    

    # After you backed off for a while, then use cccluded ball strategy if we need it.
    if Global.lostBall >= Constant.LOST_BALL_GPS + 20:
        ballX, ballY = Global.gpsGlobalBall.getPos()        
        import sDodgyDog
        if sDodgyDog.shouldIBeDodgy(ballX,ballY): 
            sDodgyDog.dodgyDogTo(ballX,ballY) 
        

def findByScan(): 

    # Nobu: I've put in the hHeadOnly test since the bird was stopping when
    # it lost the ball. 
    if gHeadOnly:
        hTrack.scan()
    
    else:             
            
        # If the last seen ball is far away, then continue walking for a while.
        # Use obstacle occluded as well?
        if Global.weightedVisBallDist > 150:
            
            hTrack.scan(lowCrane=0,highCrane=0,minPan=-60,maxPan=60,lookDown=False)
            Action.walk(Action.MAX_FORWARD,0,0,minorWalkType=Action.SkeFastForwardMWT)     
                   
        else:
            hTrack.scan()
            Action.walk(0,0,0,minorWalkType=Action.SkeFastForwardMWT)
    

def findBySpin():
      
    # Decide which direction to spin.
    if gIsClockwise: 
        turnCCW = -60
        pan = -70
    else: 
        turnCCW = 60
        pan = 70

    Action.setHeadParams(pan,-10,0,Action.HTAbs_h)
    Action.walk(0,0,turnCCW,minorWalkType=Action.SkeFastForwardMWT) 



# Walks to the "ball" at local (ballD, ballH). If useVelocity and ball velocity
# is above some threshold then will walk to a predicted interception point,
# taking walk speed and ball velocity into account. If getBehind (and velocity
# is not used) then the path to the ball will be adjusted to favour moving
# a) between the ball and our goal or b) to line up the ball with the target
# goal depending on our position
TIME_TO_PREPARE = 1
DISTANCE_CONSTANT = 10.0
def walkToBall(ballD, ballH, getBehind=False, useVelocity=False): 
    vel = VisionLink.getGPSBallVInfo(Constant.CTLocal)        
    velX, velY = vel[0], vel[1] 

    if useVelocity\
        and (abs(velX) > 2 or abs(velY) > 2):
        
        #Indicator.showFacePattern([3,3,3,3,3])
        
        velXY = hMath.getDistanceBetween(0,0,velX,velY)
        if velXY == 0: 
            velXY = 0.01
        
        ballX, ballY = Global.ballX, Global.ballY        
        tx, ty = ballX, ballY    
        offsetX = DISTANCE_CONSTANT * velX / velXY
        offsetY = DISTANCE_CONSTANT * velY / velXY 
               
        for i in range(30): 
            # time that the ball needs to get to the target point          
            timeBall = ((DISTANCE_CONSTANT * i) / velXY) * (1 / 30.0)  # seconds
            
            # time that the robot needs to get to the target point
            tx += offsetX
            ty += offsetY
            td = hMath.getDistanceBetween(0,0,tx,ty)
            #
            timeRobot = (td / 30.0) # seconds

            
            # If the robot can reach to the target point comfortably,
            # then go to the target point.
            if timeRobot + TIME_TO_PREPARE < timeBall:       
                th = hMath.getHeadingToRelative(tx,ty) 
                relh = hMath.normalizeAngle_180(th - ballH) 
                
                fComp = math.cos(hMath.DEG2RAD(relh))
                lComp = math.sin(hMath.DEG2RAD(relh)) 
                
                if fComp > 0: 
                    maxF = Action.MAX_FORWARD_SKE
                else:
                    maxF = Action.MAX_BACKWARD_SKE
                
                if lComp > 0: 
                    maxL = Action.MAX_LEFT_SKE
                else: 
                    maxL = Action.MAX_RIGHT_SKE
                 
                if maxF * abs(fComp) >= maxL * abs(lComp): 
                    scale = 1.0 / abs(fComp)
                else:
                    scale = 1.0 / abs(lComp) 
                
                fComp = scale * fComp
                lComp = scale * lComp 
            
                fwd = maxF * fComp
                left = maxL * lComp    

                Action.walk(fwd,left,ballH/2,"ddd")         
                return     
                        
    walkToStillBall(ballD, ballH, getBehind)        
        
        
        
optimalH = math.degrees(math.atan2(Action.MAX_SKE_PG22_LEFT_STP, 
                                   Action.MAX_SKE_PG22_FWD_STP))        
def walkToStillBall(ballD, ballH, getBehind = False):    
    # Calculate getBehind adjustment
    leftAdj = 0
    if getBehind:
        myx, myy = Global.selfLoc.getPos()
        myh = Global.selfLoc.getHeading()
        ballx, bally = hMath.getGlobalCoordinate(myx, myy, myh, 
                                    *hMath.polarToCart(ballD, ballH+90))

        # In defense we get between our goal and the ball. In attack we line
        # up the ball and target goal. In the (small) midfield we line up with
        # upfield
        reversePoints = False
        if bally < Constant.FIELD_LENGTH/2 - 20:
            lineX, lineY = Constant.OWN_GOAL_X, Constant.OWN_GOAL_Y
        elif bally > Constant.FIELD_LENGTH/2 + 20:
            lineX, lineY = Constant.TARGET_GOAL_X, Constant.TARGET_GOAL_Y
            reversePoints = True
        else:
            lineX, lineY = ballx, 0

        # Draw a line between ball and own goal. Find self offset from the line
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
        else:
            leftAdj = 0
        
        # From in front of the ball adjust even more to get more behind
        if myy > bally:
            leftAdj *= 1.5
        
        # If the line was to a target to line up, reverse direction
        if reversePoints:
            leftAdj = -leftAdj
    
    Indicator.showFacePattern([0]*5)
    if ballD < MID_BALL_DIST and abs(ballH) > 30:
        # Turn on the spot
        ballH = hMath.CLIP(ballH,70)
        Action.walk(0,0,ballH,minorWalkType=Action.SkeFastForwardMWT)
    else:
        # Walk direct - with getBehind adjustment
        if leftAdj > 0:
            Indicator.showFacePattern([0, 0, 0, 2, 3])
        elif leftAdj < 0:
            Indicator.showFacePattern([3, 2, 0, 0, 0])
        if abs(ballH) < 30:
            Action.walk(Action.MAX_FORWARD, leftAdj, ballH,
                        minorWalkType=Action.SkeFastForwardMWT)
        else:
            ballH = hMath.CLIP(ballH,70)
            Action.walk(0,0,ballH,minorWalkType=Action.SkeFastForwardMWT) 
    
    """        
    else: # diagonal walk
        if ballH > 0:
            turnccw = ballH - optimalH
            left = Action.MAX_LEFT                        
        else:
            turnccw = ballH + optimalH
            left = -Action.MAX_LEFT
        if abs(turnccw) > 30:
            Action.walk(0,0, turnccw, minorWalkType=Action.SkeFastForwardMWT)
        else:
            Action.walk(Action.MAX_FORWARD,
                        left,
                        turnccw,
                        minorWalkType=Action.SkePG22MWT)
    """
         
