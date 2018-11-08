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


# GreenBackoff. If we don't see any green field for a while, turn around.

import Action
import Indicator
import Constant
import Global
import VisionLink

MIN_NUM_GREEN_COUNT = 5
AVG_FRAME_COUNT = 30
MIN_TIME_BETWEEN_BACKOFF = 30

gLastFrameCalled = 0        # last perform()
gLastBackOffFrame = 0       # last time we backed off
gBackOffGenerator = None
gIsBackOffTriggering = False
gRunningTotal = 2 * MIN_NUM_GREEN_COUNT


def DecideNextAction(): 
    if perform() == Constant.STATE_SUCCESS:
        Action.walk(30,0,0)        


def resetPerform(): 
    global gBackOffGenerator
    global gIsBackOffTriggering
    global gRunningTotal
    gBackOffGenerator = None  
    gIsBackOffTriggering = False
    gRunningTotal = 2 * MIN_NUM_GREEN_COUNT

def perform(doBackOff=True,avgFrameCount=AVG_FRAME_COUNT):
    global gBackOffGenerator 
    global gLastFrameCalled
    global gLastBackOffFrame
    global gIsBackOffTriggering
    global gRunningTotal

    if gLastFrameCalled != Global.frame - 1:
        resetPerform()        
    gLastFrameCalled = Global.frame
        
    count = VisionLink.getFieldFeatureCount()    
    if avgFrameCount == 0: 
        avgFrameCount = 1
    gRunningTotal = gRunningTotal * ((avgFrameCount-1.0) / avgFrameCount)\
                    + count * (1.0 / avgFrameCount)
            
    # Keep backing off if we already are
    if gBackOffGenerator != None:
        if not gBackOffGenerator.next():    # Stop?
            resetPerform()
            return Constant.STATE_SUCCESS
        else:
            gLastBackOffFrame = Global.frame
            Indicator.showFacePattern([3, 2, 2, 2, 3])
            return Constant.STATE_EXECUTING
        
    # If we should start backing off, do so
    elif gRunningTotal < MIN_NUM_GREEN_COUNT \
            and Global.frame - gLastBackOffFrame > MIN_TIME_BETWEEN_BACKOFF:
        gBackOffGenerator = backOffGenerator(doBackOff)       
        gIsBackOffTriggering = True
        return Constant.STATE_EXECUTING
        
    # Otherwise do nothing
    else:
        return Constant.STATE_SUCCESS
        

def backOffGenerator(doBackOff): 
    # back off
    if doBackOff:                
        for _ in range(20):
            Action.walk(-Action.MAX_FORWARD,0,0,
                        minorWalkType=Action.SkeFastForwardMWT)
            yield True
    # turn
    while True: 
        if gRunningTotal > 1.0 * MIN_NUM_GREEN_COUNT:
            yield False
        Action.walk(0,0,-Action.MAX_TURN,minorWalkType=Action.SkeFastForwardMWT)
        yield True
