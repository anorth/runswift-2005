#
#   Copyright 2004 The University of New South Wales (UNSW) and National  
#   ICT Australia (NICTA).
#
#   This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
#   redistribute it and/or modify it under the terms of the GNU General  
#   Public License as published by the Free Software Foundation; either  
#   version 2 of the License, or (at your option) any later version as  
#   modified below.  As the original licensors, we add the following  
#   conditions to that license:
#
#   In paragraph 2.b), the phrase "distribute or publish" should be  
#   interpreted to include entry into a competition, and hence the source  
#   of any derived work entered into a competition must be made available  
#   to all parties involved in that competition under the terms of this  
#   license.
#
#   In addition, if the authors of a derived work publish any conference  
#   proceedings, journal articles or other academic papers describing that  
#   derived work, then appropriate academic citations to the original work  
#   must be included in that publication.
#
#   This rUNSWift source is distributed in the hope that it will be useful,  
#   but WITHOUT ANY WARRANTY; without even the implied warranty of  
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along  
#   with this source code; if not, write to the Free Software Foundation,  
#   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# 
#  Last modification background information
#  $Id: LandmarkTester.py 4572 2004-12-15 01:28:59Z alexn $
# 
#  Copyright (c) 2004 UNSW
#  All Rights Reserved.
# 




#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# LandmarkTester.py
#
# It will show different leds in eye patterns in seeing different landmarks
#
# The dog has 12 leds on face. In the following, 1 is light on, 0 is off.
#
#
#       1 0 0 0 0         Left -blue beacon
#       0 0 0 0 0 
#       0         0 
#
#       0 0 1 0 0         Blue goal
#       0 0 0 0 0 
#       0         0 
#
#       0 0 0 0 1         Right blue beacon
#       0 0 0 0 0 
#       0         0 
#
#       0 0 0 0 0         Left-yellow beacon
#       1 0 0 0 0 
#       0         0 
#
#       0 0 0 0 0         Yellow goal
#       0 0 1 0 0 
#       0         0 
#
#       0 0 0 0 0         Right-yellow beacon
#       0 0 0 0 1 
#       0         0 
#===============================================================================


"""
Landmark Tester
"""

import Global
import Constant
import VisionLink
import hTrack

import Action
import Indicator


bSendCPlane = True #if you want to send unexpectedCPlane
  
expectBlueLeft = None
expectBlueRight = None
expectYellowLeft = None
expectYellowRight = None
expectYellowGoal = None
expectBlueGoal = None
expectRedDog = None
expectBlueDog = None
expectBall = None

NO_SENDING_CPLANE = 30
lastSendTime = -1000
headCommand = None
def reset():
    global expectBall,expectBlueGoal,expectBlueLeft,expectBlueRight,\
        expectYellowGoal, expectYellowLeft, expectYellowRight
    expectBlueLeft = False
    expectBlueRight = False
    expectYellowLeft = False
    expectYellowRight = False
    expectYellowGoal = False
    expectBlueGoal = False
    expectBall = False    
    
def redDogSeen():
    for i in range(Constant.vobRedDog, Constant.vobRedDog4):
        if VisionLink.getVisualObject(i)[0] > 0:
            return True
    return False

def blueDogSeen():
    for i in range(Constant.vobBlueDog, Constant.vobBlueDog4):
        if VisionLink.getVisualObject(i)[0] > 0:
            return True
    return False
    
def DecideNextAction():
    global lastSendTime
    # Reset the 12 leds on face pattern.
    Indicator.setDefault()
    
    blueLeft    = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobBlueLeftBeacon))
    blueRight   = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobBlueRightBeacon))
    yellowLeft  = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobYellowLeftBeacon)) 
    yellowRight = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobYellowRightBeacon))

    yellowGoal  = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobYellowGoal))
    blueGoal    = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobBlueGoal))
    ball        = Global.VisualObject(*VisionLink.getVisualObject(Constant.vobBall))

    if blueLeft.getConfidence() > 0:
        Indicator.finalValues[Indicator.Led7] = Indicator.LED3_INTENSITY_MAX

    if blueRight.getConfidence() > 0:
        Indicator.finalValues[Indicator.Led8] = Indicator.LED3_INTENSITY_MAX

    if blueGoal.getConfidence() > 0:
        Indicator.finalValues[Indicator.Led11] = Indicator.LED3_INTENSITY_MAX

    if yellowLeft.getConfidence() > 0: 
        Indicator.finalValues[Indicator.Led3] = Indicator.LED3_INTENSITY_MAX

    if yellowRight.getConfidence() > 0:
        Indicator.finalValues[Indicator.Led4] = Indicator.LED3_INTENSITY_MAX

    if yellowGoal.getConfidence() > 0:
        Indicator.finalValues[Indicator.Led12] = Indicator.LED3_INTENSITY_MAX
    
    if redDogSeen():
        Indicator.finalValues[Indicator.Led5] = Indicator.LED3_INTENSITY_MAX
        
    if blueDogSeen():
        Indicator.finalValues[Indicator.Led6] = Indicator.LED3_INTENSITY_MAX
        
    Indicator.superDebug()
    #send unexpected cplanes
    unexpectedCPlane = False
    if expectBall == (ball.getConfidence() == 0):
        unexpectedCPlane = True
    
    if expectBlueGoal == (blueGoal.getConfidence() == 0):
        unexpectedCPlane = True
        
    if expectYellowGoal == (yellowGoal.getConfidence() == 0):
        unexpectedCPlane = True
        
    if expectBlueLeft  == (blueLeft.getConfidence() == 0):
        unexpectedCPlane = True
        
    if expectBlueRight == (blueRight.getConfidence() == 0):
        unexpectedCPlane = True
    
    if expectYellowLeft == (yellowLeft.getConfidence() == 0):
        unexpectedCPlane = True
        
    if expectYellowRight == (yellowRight.getConfidence() == 0):
        unexpectedCPlane = True
        
    if expectRedDog == (not redDogSeen() ):
        unexpectedCPlane = True
    
    if expectBlueDog == (not blueDogSeen() ):
        unexpectedCPlane = True
        
    if bSendCPlane and unexpectedCPlane:
        if Global.frame - lastSendTime > NO_SENDING_CPLANE:
            lastSendTime = Global.frame
            VisionLink.sendCompressedCPlane()
        
    if headCommand == "spin": 
        hTrack.spinningLocalise()
    elif headCommand == "sloc":
        hTrack.stationaryLocalise()
    elif headCommand is not None:
        Action.setHeadParams(*headCommand)
        
def processCommand(cmd):
    global headCommand
    print "LandmarkTester got |%s|"%cmd
    headCommand = None
    if cmd[:4] == "head":
        print "Head command ", cmd[4:]
        headCommand = map(float,cmd[4:].split())
        if len(headCommand) != 4:
            print "HeadCommand should have 4 numbers"
    elif cmd[:4] == "sloc":
        headCommand = "sloc"
    elif cmd[:4] == "spin":
        headCommand = "spin"
