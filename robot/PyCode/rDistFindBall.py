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
import hTrack
import hTeam
import Indicator
import sFindBall


TARGET_PT_ACC_SMALL_CIRCLE = 15
TARGET_PT_ACC_LARGE_CIRCLE = 50
gTargetPointReqAccuracy = TARGET_PT_ACC_SMALL_CIRCLE


def DecideNextAction(): 
    perform()

def resetPerform():
    pass


def perform(params = None):
    id(params) # ignore
    selfX, selfY = Global.selfLoc.getPos()
    
    defenseIndex = Global.myPlayerNum - 1

    # Variable lighting challenge and penalty shooter go to centre
    if Global.lightingChallenge or Global.penaltyShot:
        findBallInCentre()
        return
    # If wireless is down we use fixed positions
    elif hTeam.wirelessIsDown():
        if Global.myRole != Constant.ATTACKER:
            # We don't actually use this, but just in case
            findBallInDefensiveHalf()
        elif Global.myPlayerNum % 2 == 0:
            findBallInOffensiveHalfLeft()
        elif Global.myPlayerNum % 2 == 1:
            findBallInOffensiveHalfRight()

        return

    smallestY = selfY   
    for i in Global.otherValidForwards:
        mate = Global.teamPlayers[i]

        mateLoc = Global.teammatesLoc[i]
        if smallestY > mateLoc.getY(): 
            smallestY = mateLoc.getY() 
            defenseIndex = i
      
                
    # Furthest back robot moves to defensive position
    if defenseIndex == Global.myPlayerNum - 1: 
        Indicator.showFacePattern([1,1,1,1,1])      
        findBallInDefensiveHalf()    
        return 
    
    offenseLeftIndex = Global.myPlayerNum - 1    
    for i in Global.otherValidForwards: 
        mate = Global.teamPlayers[i]
        if i != defenseIndex:
                
            mateLoc = Global.teammatesLoc[i]
            if selfX > mateLoc.getX():
                offenseLeftIndex = i
                
    if offenseLeftIndex == Global.myPlayerNum - 1: 
        Indicator.showFacePattern([5,0,0,0,0]) 
        findBallInOffensiveHalfLeft()

    else:
        Indicator.showFacePattern([0,0,0,0,5]) 
        findBallInOffensiveHalfRight()      

    
   
        

# Find a ball in defensive half.
def findBallInDefensiveHalf():    
    tx = Constant.FIELD_WIDTH / 2
    ty = Constant.FIELD_LENGTH * 0.3
    findBall(tx,ty)     
    
        
def findBallInOffensiveHalfLeft(): 
    tx = 80
    if hTeam.wirelessIsDown():
        ty = Constant.FIELD_LENGTH * 0.5
    else:
        ty = Constant.FIELD_LENGTH * 0.7
    findBall(tx,ty)
    
    
def findBallInOffensiveHalfRight():
    tx = Constant.FIELD_WIDTH - 80
    if hTeam.wirelessIsDown():
        ty = Constant.FIELD_LENGTH * 0.5
    else:
        ty = Constant.FIELD_LENGTH * 0.7
    findBall(tx,ty)

# Find a ball in centre field
def findBallInCentre():    
    tx = Constant.FIELD_WIDTH / 2
    ty = Constant.FIELD_LENGTH / 2 + 80
    findBall(tx,ty)     
        
        
gSpinCounter = 0
def findBall(tx,ty): 
    global gSpinCounter
    global gTargetPointReqAccuracy

    selfX, selfY = Global.selfLoc.getPos()
    distSquared = hMath.getDistSquaredBetween(selfX,selfY,tx,ty)
                    
    if gSpinCounter > 0:
        sFindBall.findBySpin() 
        gSpinCounter -= 1
 
    elif distSquared <= hMath.SQUARE(gTargetPointReqAccuracy):      
        gTargetPointReqAccuracy = TARGET_PT_ACC_LARGE_CIRCLE              
        sFindBall.findBySpin()
        gSpinCounter = 100
                              
    else:
        gTargetPointReqAccuracy = TARGET_PT_ACC_SMALL_CIRCLE 
        hTrack.scan()
        hTrack.saGoToTarget(tx,ty)
