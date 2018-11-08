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


from Action import *
import hPauseMotion
import VisionLink
from Constant import *
import Indicator
import hMath
#import sGrabDribble

count = 0

VisionLink.srand(10)

MAX_COUNT = 30 * 300
CHANGE_COMMAND = 30 * 5

#kick(BlockWT)

def DecideNextAction():
    global count
    count +=1
    if count < 60:
        kick(BlockLeftWT)
    elif count < 120:
        kick(BlockRightWT)
    else:
        count = 0

def printSwitches():
    print VisionLink.getSwitchSensorCount(SWITCH_FL_PALM),
    print VisionLink.getSwitchSensorCount(SWITCH_FR_PALM),
    print VisionLink.getSwitchSensorCount(SWITCH_RL_PALM),
    print VisionLink.getSwitchSensorCount(SWITCH_RR_PALM)

def turnGradually():
    global count
    count = (count + 1) % CHANGE_COMMAND
    if count < 30:
        walk(0, 0, 10, minorWalkType = SkeFastForwardMWT)
    elif count < 60:
        walk(0, 0, 20, minorWalkType = SkeFastForwardMWT)
    elif count < 90:
        walk(0, 0, 30, minorWalkType = SkeFastForwardMWT)
    elif count < 120:
        walk(0, 0, 40, minorWalkType = SkeFastForwardMWT)
    elif count < 150:
        walk(0, 0, 50, minorWalkType = SkeFastForwardMWT)

"""
def walkRandomly():
    if count % CHANGE_COMMAND == 0:
        forward = hMath.getSign(random(), random());
        left = hMath.getSign(random(), random());
        turn = hMath.getSign(random(), random());
        Action.walk(forward,
                    left,
                    turn,
                    minorWalkType=Action.SkeFastForwardMWT)
    print __name__, VisionLink.getLastMaxStepPWM(ssFL_ABDUCTOR),
    print VisionLink.getLastMaxStepPWM(ssFR_ABDUCTOR)
    hPauseMotion.check()
"""

STUCK_FRAMES = 30
maxPwmLRJ = [0] * STUCK_FRAMES
maxPwmRRJ = [0] * STUCK_FRAMES

def checkStuck():
    maxPwmLRJ.pop(0)
    maxPwmRRJ.pop(0)

    pwmL = pwm(jLeftRearJoint)
    pwmR = pwm(jRightRearJoint)

    print pwmL, pwmR,
    
    maxPwmLRJ.append( pwmL )
    maxPwmRRJ.append( pwmR )

    numL = len(filter(isOverLimit, maxPwmLRJ))
    numR = len(filter(isOverLimit, maxPwmRRJ))
    print numL, numR

    if numL>= 3 or numR>=3:
        Indicator.showHeadColor(Indicator.RGB_GREEN)
        openMouth()
        hPauseMotion.pause()
    else:
        Indicator.showHeadColor(Indicator.RGB_NONE)
        closeMouth()    

def isOverLimit(value):
    return value > 550

        
def pwm(i):
    return VisionLink.getJointPWMDuty(i)
