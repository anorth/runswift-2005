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


import Global
import Constant
import hMath
import Action
import math
import Indicator
import hTrack
import VisionLink

bStopMotion = False
actionGenerator = None

turningLeft = True


def DecideNextAction():
    global bStopMotion, actionGenerator
    if not bStopMotion:
        if VisionLink.getPressSensorCount(2) > 10: # disable motion
            bStopMotion = True
        if not actionGenerator:
            actionGenerator = spinWalkGenFunc()
        try:
            actionGenerator.next()
        except StopIteration:
            reset()
    else:
        reset()
        Indicator.showHeadColor(Indicator.RGB_LIGHT_BLUE)
        if VisionLink.getPressSensorCount(0) > 5: # enable motion
            bStopMotion = False
            global turningLeft
            turningLeft = not turningLeft

def reset():
    global actionGenerator
    actionGenerator = None
    Action.standStill()



def spinWalkGenFunc():
    for _ in xrange(150):
        Indicator.showHeadColor(Indicator.RGB_GREEN)        
        hTrack.stationaryLocalise(5)
        VisionLink.setIfGPSVisionUpdate(True)        
        yield True

    while True:
        VisionLink.setIfGPSVisionUpdate(False) # turn off vision update
        Indicator.showHeadColor(Indicator.RGB_YELLOW)
        target = (Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2)
        walk(target, Global.selfLoc.getPos(), Global.selfLoc.getHeading())
        yield True

maxFwd = 7
maxLeft = 5
maxTurn = 30

def walk(target, selfPos, selfH):
    relX    = target[0] - selfPos[0]
    relY    = target[1] - selfPos[1]
    relD     = hMath.getLength((relX, relY))
    
    if relX == 0 and relY == 0:
        relThetaRad = 0
    else:
        relThetaRad = hMath.DEG2RAD(hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY, relX)) - selfH))

    sinTheta = math.sin(relThetaRad)
    cosTheta = math.cos(relThetaRad)
    
    leftSign = relD * sinTheta
    leftAbs1 = abs(leftSign)    #leftAbs1: the distance left component
    if cosTheta != 0:        #leftAbs2: the max capping due to max forward
        leftAbs2 = abs(maxFwd * (sinTheta / cosTheta))
    else:
        leftAbs2 = 100000 # huge number
    #print "left1:", leftAbs1, "left2:", leftAbs2
    leftAbs = min(leftAbs1, leftAbs2, maxLeft)        
    left = hMath.getSign(leftAbs, leftSign)

    if sinTheta != 0:
        forward = left * (cosTheta / sinTheta)
    else:
        fwdSign = relD
        fwdAbs = min(maxFwd, abs(relD))
        forward = hMath.getSign(fwdAbs, fwdSign)

    if turningLeft:
        turnCCW = maxTurn
    else:
        turnCCW = -maxTurn
        
    print __name__, forward, left, turnCCW  
    Action.walk(forward, left, turnCCW,walkType=Action.SkellipticalWalkWT)
