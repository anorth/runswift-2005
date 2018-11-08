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



import Constant
import Global
import hMath
#import math

CLOSE_EDGE_DIST      = 40
BALL_CLOSE_EDGE_DIST = 50

X_POS_POINT_A = (80,Constant.FIELD_LENGTH-100)
X_POS_POINT_B = (Constant.FIELD_WIDTH-80,Constant.FIELD_LENGTH-100) 
X_POS_POINT_C = (Constant.FIELD_WIDTH/2,200)
X_POS_POINT_D = (80,100)
X_POS_POINT_E = (Constant.FIELD_WIDTH-80,100) 
SUPPORTER_X_OFFSET = 120    
SUPPORTER_Y_OFFSET = 150 
WINGER_Y_OFFSET = 120

STRIKER_LEFT_X = 60
STRIKER_RIGHT_X = Constant.FIELD_WIDTH - STRIKER_LEFT_X

FRONT_THIRD = Constant.FIELD_LENGTH * 0.6
BACK_THIRD = Constant.FIELD_LENGTH * 0.22

# =====================================================
# Striker Position
# =====================================================
# Returns the desired striker home position (based on global ball coords)
gStrikerX = None 

def getStrikerPos(lostBall = False):
    return getStrikerPos1(lostBall)

def getStrikerPos1(lostBall = False):
    global gStrikerX
    yOffset = 80.0
    xOffset = 40.0

    ballX, ballY = Global.ballX, Global.ballY 
    
    if ballX < STRIKER_LEFT_X + xOffset:
        gStrikerX = STRIKER_RIGHT_X
    elif ballX > STRIKER_RIGHT_X - xOffset:
        gStrikerX = STRIKER_LEFT_X
    elif gStrikerX == None:
        if ballX < Constant.FIELD_WIDTH/2:
            gStrikerX = STRIKER_RIGHT_X
        else:
            gStrikerX = STRIKER_LEFT_X
    
    debug = False                
    #ball in our half
    if ballY <= Constant.FIELD_LENGTH/2:
        if debug:
            print "getStrikerPos1: Zone 1"
        targetX = gStrikerX    
        targetY = Constant.FIELD_LENGTH/2 + yOffset
        
        if lostBall:
            targetY = targetY + yOffset
           
        return (targetX, targetY, xOffset)    
    #ball in close side of their half
    elif ballY < FRONT_THIRD + yOffset:
        if debug:
            print "getStrikerPos1: Zone 2"
        targetX = gStrikerX   
        targetY = max(ballY - yOffset/2, Constant.FIELD_LENGTH/2)
        
        if lostBall:
            if gStrikerX == STRIKER_LEFT_X:
                targetX = STRIKER_LEFT_X - STRIKER_LEFT_X/2
            else:
                targetX = STRIKER_RIGHT_X + STRIKER_LEFT_X/2 
        
        return (targetX, targetY, xOffset)    
    #ball in far side of their half
    else:
        if debug:
            print "getStrikerPos1: Zone 3"
        gStrikerX = None
        targetX = Constant.FIELD_WIDTH/2
        targetY = min(ballY - yOffset/2, Constant.FIELD_LENGTH - yOffset)
        
        if lostBall:
            targetY = targetY - yOffset
        
        return (targetX, targetY, 0)
        

# =====================================================
# Supporter Position
# =====================================================
# Returns the desired supporter home position (based on global ball coords)
def getSupporterPos(supportAttacker=False):
    id(supportAttacker)
    return getSupporterPos4() #getSupporterPos2(supportAttacker)

def getSupporterPos1(supportAttacker):

    if not supportAttacker:
        supX = Global.ballX
        supY = Global.ballY
    else:

        mateI = getIndexOfSupportingAttacker()
        if mateI != None: 
            supX = Global.teammatesLoc[mateI].getX()
            supY = Global.teammatesLoc[mateI].getY()
        else:
            supX = Global.ballX
            supY = Global.ballY

                        
    # debug = True
    # First draw a straight line from the ball to the goal center.
    # TODISCUSS: Is drawing the line to the best gap and integrate with DKD range a good idea?
    topEquation = Constant.TARGET_GOAL_Y - supY
    bottomEquation = Constant.TARGET_GOAL_X - supX
    if bottomEquation == 0:
        bottomEquation += 0.01
    m = topEquation / bottomEquation
    b = Constant.TARGET_GOAL_Y - m * Constant.TARGET_GOAL_X
    
    # Get the x value matching my global y value.
    XEDGE = 60
    YEDGE = 100
    
    if m == 0:
        m += 0.01
    xMatchingMyY  = (Global.selfLoc.getPos()[1] - b) / m

    # Am I left or right of the support line?
    if Global.selfLoc.getPos()[0] <= xMatchingMyY:
        onLeftSideOfLine = True
    else:
        onLeftSideOfLine = False
        
    # Deal special cases first, the left side support.
    if supY > 0.25 * Constant.FIELD_LENGTH\
        and supX <= Constant.ON_EDGE_OFFSET:
        x = 60
        y = min(Constant.FIELD_LENGTH - YEDGE, supY - 120)
        
    elif supY > 0.25 * Constant.FIELD_LENGTH\
        and supX >= Constant.FIELD_WIDTH - Constant.ON_EDGE_OFFSET: 
        x = Constant.FIELD_WIDTH - 60
        y = min(Constant.FIELD_LENGTH - YEDGE, supY - 120)
                      
    # Top horizontal strip.
    elif supY >= 0.75 * Constant.FIELD_LENGTH:
        
        if onLeftSideOfLine:
            x = max(XEDGE, supX - 50)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY - 120)

        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 50)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY - 120)        
    
    # Bottom horizontal strip.
    elif Constant.GOALBOX_DEPTH <= supY\
        and supY <= 0.25 * Constant.FIELD_LENGTH:

        if onLeftSideOfLine:
            x = max(XEDGE, supX - 50)
            y = max(YEDGE, supY + 120)
        
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 50)
            y = max(YEDGE, supY + 120)


    # Own goal near case.
    elif supY < Constant.GOALBOX_DEPTH:
        
        if onLeftSideOfLine:
            x = max(XEDGE, supX - 50)
            y = max(YEDGE, supY + 120)
        
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 50)
            y = max(YEDGE, supY + 120)
    
    # The last case - middle horizontal strip.
    else:
        if onLeftSideOfLine:
            x = max(XEDGE, supX - 50)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY - 120)
        
        else:
            x = min(Constant.FIELD_WIDTH - XEDGE, supX + 50)
            y = min(Constant.FIELD_LENGTH - YEDGE, supY - 120)
    
    return (x,y,0) 

# Closer supporter positioning
def getSupporterPos2(supportAttacker=False): 
    id(supportAttacker)

    yOffset = 80

    ballX, ballY = Global.ballX, Global.ballY 
    
    targetX = min(max(ballX,70),Constant.FIELD_WIDTH-70)
    targetY = max(ballY-yOffset,X_POS_POINT_D[1])
        
    return (targetX,targetY,40)    


def getSupporterPos3(supportAttacker=False): 
    
    if not supportAttacker:
        supX, supY = Global.ballX, Global.ballY
    else:
        mateI = getIndexOfSupportingAttacker()
        if mateI != None: 
            supX = Global.teammatesLoc[mateI].getX()
            supY = Global.teammatesLoc[mateI].getY()
        else:
            supX = Global.ballX
            supY = Global.ballY

    targetY = max(supY-SUPPORTER_Y_OFFSET,X_POS_POINT_D[1])        
          
    
    ratioX = SUPPORTER_X_OFFSET / (Constant.FIELD_WIDTH/2.0)
    offsetX = ratioX * (Constant.FIELD_WIDTH/2 - supX)
    targetX = supX + offsetX                       
    
    targetX = max(min(targetX,Constant.FIELD_WIDTH-100),100)
    rangeX = (ratioX/3.0) * min(supX,Constant.FIELD_WIDTH-supX)
        
    return (targetX,targetY,rangeX)

# Close supporter positioning as for getSupporterPos2 but supporter stays
# forward when ball in defensive third
def getSupporterPos4():
    yOffset = 80
    rangeX = 40
    #centreWidth = 30

    ballX, ballY = Global.ballX, Global.ballY 
    
    if ballY >= Constant.FIELD_LENGTH * 0.75:
        # Wide 'L' offset in offense
        targetX = min(max(ballX, rangeX * 1.5),
                      Constant.FIELD_WIDTH - rangeX * 1.5)
        targetY = ballY - (yOffset * 0.80)
        return (targetX, targetY, rangeX * 1.5)    
    if ballY >= Constant.FIELD_LENGTH/2 + 15:
        # Standard 'L' offset in front half, staying clear of sideline
        targetX = min(max(ballX, rangeX),Constant.FIELD_WIDTH - rangeX)
        targetY = ballY-yOffset
        return (targetX, targetY, rangeX)    
    elif ballY > BACK_THIRD + yOffset:
        # Wider and more forward to stay clear of defender position. 
        # Supporter doesn't go into the back third.
        targetX = min(max(ballX, rangeX),Constant.FIELD_WIDTH - rangeX)
        targetY = max(ballY - yOffset/2.0, X_POS_POINT_D[1])
        return (targetX, targetY, 2*rangeX)    
    else:
        # In defense the supporter hangs around the centre circle waiting
        # for the ball to come forward or drop-in
        targetX = Constant.FIELD_WIDTH/2
        targetY = Constant.FIELD_LENGTH * 0.45
        return (targetX, targetY, 0)


def getIndexOfSupportingAttacker():
    mateD = None
    mateI = None
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]

        if mate.hasGrabbedBall(): 
            mateI = i
            break

        if mateD == None\
            or mateD > mate.getTimeToReachBall():

            mateD = mate.getTimeToReachBall()
            mateI = i        

    return mateI



    
    


# =====================================================
# Winger Position
# =====================================================

def getWingerPos():
    return getWingerPos2()

# X Positioning
def getWingerPos1(): 

    ballX, ballY = Global.ballX, Global.ballY
    targetY = max(ballY-WINGER_Y_OFFSET,X_POS_POINT_D[1])
    
    if ballX > Constant.FIELD_WIDTH/2: 
        # use points A and C
        if X_POS_POINT_C[1] <= targetY:
            targetY = getAdjustedTargetY(targetY,WINGER_Y_OFFSET)
            relX = X_POS_POINT_A[0] - X_POS_POINT_C[0]    
            relY = X_POS_POINT_A[1] - X_POS_POINT_C[1]            
            
        # use points C and D    
        else:                
            relX = X_POS_POINT_C[0] - X_POS_POINT_D[0]
            relY = X_POS_POINT_C[1] - X_POS_POINT_D[1]       
        
    else: 
        # use points B and C        
        if X_POS_POINT_C[1] <= targetY:
            targetY = getAdjustedTargetY(targetY,WINGER_Y_OFFSET)
            relX = X_POS_POINT_B[0] - X_POS_POINT_C[0]
            relY = X_POS_POINT_B[1] - X_POS_POINT_C[1]      
        
        # use points C and E
        else: 
            relX = X_POS_POINT_C[0] - X_POS_POINT_E[0]
            relY = X_POS_POINT_C[1] - X_POS_POINT_E[1]     
            
    if relX == 0: 
        relX = 0.01
    m = (relY + 0.0) / relX
    b = X_POS_POINT_C[1] - m * X_POS_POINT_C[0]
    targetX = (targetY - b) / m
    return (targetX,targetY)  


# Modified version of X Positioning
def getWingerPos2(): 
    yOffset = 160
    
    ballX, ballY = Global.ballX, Global.ballY
    targetY = max(ballY-yOffset,X_POS_POINT_D[1])
    
    if ballX > Constant.FIELD_WIDTH/2 + 25: 
        # use points A and C
        if X_POS_POINT_C[1] <= targetY:
            #targetY = getAdjustedTargetY(targetY,yOffset)
            relX = X_POS_POINT_A[0] - X_POS_POINT_C[0]    
            relY = X_POS_POINT_A[1] - X_POS_POINT_C[1]            
            
        # use points C and D    
        else:                
            relX = X_POS_POINT_C[0] - X_POS_POINT_D[0]
            relY = X_POS_POINT_C[1] - X_POS_POINT_D[1]      
        
    elif ballX < Constant.FIELD_WIDTH/2 - 25: 
        # use points B and C        
        if X_POS_POINT_C[1] <= targetY:
            #targetY = getAdjustedTargetY(targetY,yOffset)
            relX = X_POS_POINT_B[0] - X_POS_POINT_C[0]
            relY = X_POS_POINT_B[1] - X_POS_POINT_C[1]      
        
        # use points C and E
        else: 
            relX = X_POS_POINT_C[0] - X_POS_POINT_E[0]
            relY = X_POS_POINT_C[1] - X_POS_POINT_E[1]     
    
    else: 
        
        if X_POS_POINT_C[1] <= targetY: 
            targetX = ballX         
            return (targetX,max(targetY-80,X_POS_POINT_D[1]))
                    
        else:
            if ballX > Constant.FIELD_WIDTH/2:
                relX = X_POS_POINT_C[0] - X_POS_POINT_D[0]
                relY = X_POS_POINT_C[1] - X_POS_POINT_D[1]                
            else: 
                relX = X_POS_POINT_C[0] - X_POS_POINT_E[0]
                relY = X_POS_POINT_C[1] - X_POS_POINT_E[1]  
    
            
    if relX == 0: 
        relX = 0.01
    m = (relY + 0.0) / relX
    b = X_POS_POINT_C[1] - m * X_POS_POINT_C[0]
    targetX = (targetY - b) / m
    return (targetX,targetY)  



# Modified version of X Positioning. Y position clipped to halfway.
def getWingerPos3(): 
    yOffset = 160
    
    ballX, ballY = Global.ballX, Global.ballY
    targetY = min(max(ballY-yOffset,X_POS_POINT_D[1]),Constant.FIELD_LENGTH*0.45)
    
    if ballX > Constant.FIELD_WIDTH/2 + 25: 
        # use points A and C
        if X_POS_POINT_C[1] <= targetY:
            #targetY = getAdjustedTargetY(targetY,yOffset)
            relX = X_POS_POINT_A[0] - X_POS_POINT_C[0]    
            relY = X_POS_POINT_A[1] - X_POS_POINT_C[1]            
            
        # use points C and D    
        else:                
            relX = X_POS_POINT_C[0] - X_POS_POINT_D[0]
            relY = X_POS_POINT_C[1] - X_POS_POINT_D[1]      
        
    elif ballX < Constant.FIELD_WIDTH/2 - 25: 
        # use points B and C        
        if X_POS_POINT_C[1] <= targetY:
            #targetY = getAdjustedTargetY(targetY,yOffset)
            relX = X_POS_POINT_B[0] - X_POS_POINT_C[0]
            relY = X_POS_POINT_B[1] - X_POS_POINT_C[1]      
        
        # use points C and E
        else: 
            relX = X_POS_POINT_C[0] - X_POS_POINT_E[0]
            relY = X_POS_POINT_C[1] - X_POS_POINT_E[1]     
    
    else: 
        
        if X_POS_POINT_C[1] <= targetY: 
            targetX = ballX         
            return (targetX,max(targetY-80,X_POS_POINT_D[1]))
                    
        else:
            if ballX > Constant.FIELD_WIDTH/2:
                relX = X_POS_POINT_C[0] - X_POS_POINT_D[0]
                relY = X_POS_POINT_C[1] - X_POS_POINT_D[1]                
            else: 
                relX = X_POS_POINT_C[0] - X_POS_POINT_E[0]
                relY = X_POS_POINT_C[1] - X_POS_POINT_E[1]  
    
            
    if relX == 0: 
        relX = 0.01
    m = (relY + 0.0) / relX
    b = X_POS_POINT_C[1] - m * X_POS_POINT_C[0]
    targetX = (targetY - b) / m
    return (targetX,targetY)


def getWingerPos4(): 
    ballX, ballY = Global.ballX, Global.ballY
    targetY = max(ballY-WINGER_Y_OFFSET,X_POS_POINT_D[1])
    
    #if pointC1[1] <= targetY:
    #    targetY = getAdjustedTargetY(targetY)
    
    ballX = min(max(ballX,70),Constant.FIELD_WIDTH-70)
        
    if targetY < Constant.FIELD_LENGTH * 0.4:
        if ballX < Constant.FIELD_WIDTH/2: 
            targetX = ballX + Constant.FIELD_WIDTH/2       
            targetX = min(targetX,Constant.FIELD_WIDTH-140)
        else:  
            targetX = ballX - Constant.FIELD_WIDTH/2    
            targetX = max(targetX,140)       
    else:
        if ballX < Constant.FIELD_WIDTH/2: 
            targetX = ballX + Constant.FIELD_WIDTH/2 + 100        
            targetX = min(targetX,Constant.FIELD_WIDTH-70)
        else:  
            targetX = ballX - Constant.FIELD_WIDTH/2 - 100
            targetX = max(targetX,70)

    return (targetX,targetY)     
   
       

    
def getAdjustedTargetY(targetY,yOffset):
    if targetY > X_POS_POINT_A[1]:
        return X_POS_POINT_A[1]
    else: 
        relY = X_POS_POINT_A[1] - X_POS_POINT_C[1]
        if relY == 0: 
            relY = 0.1
        return targetY + ((yOffset + 0.0) / relY) * (targetY - X_POS_POINT_C[1]) 

# =====================================================
# Defender Position
# =====================================================
# Returns the desired defender home position (based on global ball coords)
def getDefenderPos():

    # If the ball is a long way forward we sit behind the halfway line a little
    # toward the side where the ball is. This is to trap a ball cleared upfield
    # and be close the the drop-in point on this side of the field.
    if Global.ballY > FRONT_THIRD:
        targetX = Constant.FIELD_WIDTH/2 + \
                    (Global.ballX - Constant.FIELD_WIDTH/2)/2
        targetY = Constant.FIELD_LENGTH * 0.32
        # If there is an opponent robot in this half of the field, make sure
        # we are behind it. TODO.
    
    # If the ball is (close to) in our half then position ourselves 2/5 of the
    # way between the goal and ball, clipped to in front of the
    # penalty area
    elif Global.ballY > BACK_THIRD:
        targetX = (Global.ballX + Constant.FIELD_WIDTH/2) / 2
        targetY = max(Constant.GOALBOX_DEPTH + 27, Global.ballY * 0.4)
    
    # Otherwise if the ball is really close to our goal the supporter will clear
    # away and we should be close to support the attacker
    else:
        ballX, ballY = Global.ballX, Global.ballY 

        xOffset = 70
        #if ballX < Constant.GOALBOX_DEPTH:
        #    xOffset = 50
        
        # Fixed x offset, but don't go too close to the sideline (blocks
        # attacker clearing upfield)
        if ballX > Constant.FIELD_WIDTH/2:
            targetX = min(ballX - xOffset, \
                        Constant.FIELD_WIDTH/2 + 80)
        else:
            targetX = max(ballX + xOffset, \
                        Constant.FIELD_WIDTH/2 - 80)

        targetY = ballY

        #if (Constant.FIELD_WIDTH - Constant.GOALBOX_WIDTH - 10)/2 \
        #        < targetX \
        #        < (Constant.FIELD_WIDTH + Constant.GOALBOX_WIDTH + 10)/2:
        # Don't target inside goalbox
        targetY = max(targetY, Constant.GOALBOX_DEPTH + 20)

    return (targetX, targetY)

# =====================================================
# Cases about my own positions
# =====================================================
def selfInOwnGoalBox(offset = 0, sideoffset = Constant.LARGE_VAL):
    x, y = Global.selfLoc.getPos()
    
    if sideoffset == Constant.LARGE_VAL:
        return inGoalBox(x, y, True, offset)
        
    if (x >= Constant.MIN_GOALBOX_EDGE_X - sideoffset and
        x <= Constant.MAX_GOALBOX_EDGE_X + sideoffset and
        y <= Constant.OWN_GOALBOX_EDGE_Y + offset):
        return True
    return False
    
def isAlongEdge(offset = CLOSE_EDGE_DIST):
    return (isOnBEdge(offset) or
            isOnLEdge(offset) or
            isOnREdge(offset) or
            isOnTLEdge(offset) or
            isOnTREdge(offset) )
            
    
def isOnLEdge(offset = CLOSE_EDGE_DIST):
    return (Global.selfLoc.getX() < (offset))

def isOnREdge(offset = CLOSE_EDGE_DIST):
    return (Global.selfLoc.getX() > (Constant.FIELD_WIDTH - offset))

def isOnTEdge(offset = CLOSE_EDGE_DIST):
    return (Global.selfLoc.getY()> (Constant.FIELD_LENGTH - offset) )
    
def isOnBEdge (offset = CLOSE_EDGE_DIST):
    return (Global.selfLoc.getY() < (offset))

def isOnTLEdge(offset = CLOSE_EDGE_DIST):
    return ( (Global.selfLoc.getX() < Constant.LEFT_GOAL_POST) and
             (Global.selfLoc.getY() > Constant.FIELD_LENGTH - offset) )
    
def isOnTREdge(offset = CLOSE_EDGE_DIST):
    return ( (Global.selfLoc.getX() > Constant.RIGHT_GOAL_POST) and
             (Global.selfLoc.getY() > Constant.FIELD_LENGTH - offset) )
    
def isOnTheField(x,y,offset = 0):
    return x >= offset and x <= Constant.FIELD_WIDTH - offset and \
        y >=offset and y <= Constant.FIELD_WIDTH - offset

# =====================================================
# Cases about the ball positions
# =====================================================
def ballInOwnGoalBox(offset = 0, sideoffset = Constant.LARGE_VAL):
    if sideoffset == Constant.LARGE_VAL:
        return inGoalBox(Global.ballX, Global.ballY, True, offset)
        
    if (Global.ballX >= Constant.MIN_GOALBOX_EDGE_X - sideoffset and
        Global.ballX <= Constant.MAX_GOALBOX_EDGE_X + sideoffset and
        Global.ballY <= Constant.OWN_GOALBOX_EDGE_Y + offset):
        return True
    return False

def ballInTargetGoalBox(offset = 0, sideoffset = Constant.LARGE_VAL):
    if sideoffset == Constant.LARGE_VAL:
        return inGoalBox(Global.ballX, Global.ballY, False, offset)
        
    if (Global.ballX >= Constant.MIN_GOALBOX_EDGE_X - sideoffset and
        Global.ballX <= Constant.MAX_GOALBOX_EDGE_X + sideoffset and
        Global.ballY >= Constant.TOP_GOALBOX_EDGE_Y + offset):
        return True    
    return False
        
def ballOnEdge(offset = BALL_CLOSE_EDGE_DIST):
    return ballOnLEdge(offset)\
        or ballOnREdge(offset)\
        or ballOnTEdge(offset)\
        or ballOnBEdge(offset)

def ballOnLEdge(offset = BALL_CLOSE_EDGE_DIST):
    return Global.ballX < offset

def ballOnREdge(offset = BALL_CLOSE_EDGE_DIST):
    return Global.ballX > Constant.FIELD_WIDTH - offset

def ballOnTEdge(offset = BALL_CLOSE_EDGE_DIST):
    return Global.ballY > Constant.FIELD_LENGTH - offset
    
def ballOnBEdge(offset = BALL_CLOSE_EDGE_DIST):
    return Global.ballY < offset

    
def ballInTLCorner():
    return ( (Global.ballY > (Constant.FIELD_LENGTH - 40)) and
        (Global.ballX < (40)) )

def ballInTRCorner():    
    return ( (Global.ballY > (Constant.FIELD_LENGTH - 40)) and
        (Global.ballX > (Constant.FIELD_WIDTH - 40)) )

def ballInBLCorner():
    return (Global.ballX <= (40)) and \
           (Global.ballY <= (40))
    
def ballInBRCorner():
    return (Global.ballX >= (Constant.FIELD_WIDTH -40)) and\
           (Global.ballY <= (40))
        
def ballOnTLEdge(offset = BALL_CLOSE_EDGE_DIST):        
    return ( (Global.ballX < Constant.LEFT_GOAL_POST) and
             (Global.ballY > (Constant.FIELD_LENGTH - offset)) )
             

def ballOnTREdge(offset = BALL_CLOSE_EDGE_DIST):        
    return ( (Global.ballX > Constant.RIGHT_GOAL_POST) and
             (Global.ballY > (Constant.FIELD_LENGTH - offset)) )

def ballNotAlongEdge(offset = BALL_CLOSE_EDGE_DIST):    
    return (not ballOnBEdge(offset) and 
            not ballOnLEdge(offset) and
            not ballOnREdge(offset) and
            not ballOnTLEdge(offset) and
            not ballOnTREdge(offset) )

# check if the ball is in triangle top left corner.
def ballInTriTLCorner(): 
    # draw a line to make a triangle corner.        
    m = 1.0
    b = Constant.FIELD_LENGTH - 100
    y = m * Global.ballX + b 
    return y < Global.ballY   

# check if the ball is in triangle top right corner.
def ballInTriTRCorner():
    # draw a line to make a triangle corner.        
    m = 1.0
    b = (Constant.FIELD_LENGTH - 100) - m * (Constant.FIELD_WIDTH)
    y = m * Global.ballX + b    
    return y < Global.ballY

    
def isInFrontTargetGoal():
    posx = Global.selfLoc.getX()
    return ( (abs(posx - Constant.FIELD_WIDTH / 2)) < Constant.GOAL_WIDTH/2 + 10 )

def ballInOwnHalf():
    bally = Global.gpsGlobalBall.getPos()[1]
    return (bally < Constant.FIELD_LENGTH/ 2)

def isInRange(min_val, max_val):
    realmin = hMath.normalizeAngle_0_360(min_val)
    realmax = hMath.normalizeAngle_0_360(max_val)
    if realmax < realmin:
        realmax = realmax + 360
    
    head = Global.selfLoc.getHeading()
    
    if head < realmin:
        realmin = realmin - 360
        realmax = realmax - 360
        
    if head > realmax:
        realmin = realmin + 360
        realmax = realmax + 360
            
    return min_val <= head and head <= max_val



def inGoalBox(x, y, ownGoalBox = True, offset = 0):
    if ownGoalBox:
        return (y <= Constant.GOALBOX_DEPTH + offset and 
                Constant.MIN_GOALBOX_EDGE_X - offset <= x and
                x <= Constant.MAX_GOALBOX_EDGE_X + offset)
    else:
        return (y >= Constant.TOP_GOALBOX_EDGE_Y - offset and 
                Constant.MIN_GOALBOX_EDGE_X - offset <= x and
                x <= Constant.MAX_GOALBOX_EDGE_X + offset)

def isHeadingForward(h):
    return h > 0 and h <= 180
    
def isHeadingLeftward(h):
    return h > 90 and h <= 270
    
def edgeIsOnMyRight(x,y,h):
    #if near left edge heading backward or near right edge heading forward
    return ( x < Constant.FIELD_WIDTH/2 and not isHeadingForward(h) ) \
        or ( x > Constant.FIELD_WIDTH/2 and isHeadingForward(h) ) \
        or ( y < Constant.FIELD_LENGTH/2 and not isHeadingLeftward(h) ) \
        or ( x > Constant.FIELD_LENGTH/2 and isHeadingLeftward(h) )

def isInTopLeftCorner(x,y,offset = 50):
    return x < offset and y > Constant.FIELD_LENGTH - offset
    
def isInTopRightCorner(x,y,offset = 50):
    return x > Constant.FIELD_WIDTH - offset and y > Constant.FIELD_LENGTH - offset
    
def isInBottomLeftCorner(x,y,offset = 50):
    return x < offset and y < offset

def isInBottomRightCorner(x,y,offset = 50):
    return x > Constant.FIELD_WIDTH - offset and y < offset
    
def isInTopEdgeCorner(_,y,offset = 50):
    return y > Constant.FIELD_LENGTH - offset

def isInBottomEdgeCorner(_,y,offset = 50):
    return y < offset
    
def isInRightEdgeCorner(x,_,offset = 50):
    return x > Constant.FIELD_WIDTH - offset 
    
def isInLeftEdgeCorner(x,_,offset = 50):
    return x < offset


