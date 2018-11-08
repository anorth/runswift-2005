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


################################################################################
# These are all helper functions for indicators on the dog.
# You *should* use these rather than set the global finalIndicator[] directly.
################################################################################

# variable used for frame debugging.
tail_down_count = 0

# Indices for finalIndicators.
Mouth      = 0
TailV      = 1
TailH      = 2
EarL       = 3
EarR       = 4
Led1       = 5
Led2       = 6
Led3       = 7
Led4       = 8
Led5       = 9
Led6       = 10
Led7       = 11
Led8       = 12
Led9       = 13
Led10      = 14
Led11      = 15
Led12      = 16
Led13      = 17
Led14      = 18
HeadColor  = 19
HeadWhite  = 20
ModeRed    = 21
ModeGreen  = 22
ModeBlue   = 23
Wireless   = 24
BackRearColor      = 25
BackRearWhite      = 26
BackMiddleColor    = 27
BackMiddleWhite    = 28
BackFrontColor     = 29
BackFrontWhite     = 30


# Constants of face and back lights.
LED3_INTENSITY_MIN = 0x00000000
LED3_INTENSITY_MAX = 0x000000FF
LED3_MODE_A        = 0x00000000
LED3_MODE_B        = 0x00000F00
LED2_ON            = 1
LED2_OFF           = 2
LED_UNCHANGED      = -1
LED_ON             = 1
LED_OFF            = 0
MOUTH_OPEN         = -950000
MOUTH_HALF_OPEN    = -500000
MOUTH_CLOSED       = -10000
TAIL_RIGHT         = 1000000
TAIL_LEFT          = -1000000
TAIL_UP            = 1000000
TAIL_DOWN          = 17000
TAIL_H_CENTRED     = (TAIL_LEFT + TAIL_RIGHT) / 2
TAIL_V_CENTRED     = (TAIL_UP + TAIL_DOWN) / 2


# Default value for finalIndicators to use.
DEFAULT = (MOUTH_CLOSED, # Mouth \
                TAIL_UP,            # TailV
                TAIL_H_CENTRED,     # TailH
                0,                  # EarL
                0,                  # EarR
                LED_OFF,            # Led1       = 5
                LED_OFF,            # Led2
                LED_OFF,            # Led3
                LED_OFF,            # Led4
                LED_OFF,            # Led5
                LED_OFF,            # Led6       = 10
                LED_OFF,            # Led7
                LED_OFF,            # Led8
                LED_OFF,            # Led9
                LED_OFF,            # Led10
                LED_OFF,            # Led11      = 15
                LED_OFF,            # Led12
                LED_OFF,            # Led13
                LED_OFF,            # Led14
                LED2_OFF,           # HeadColor
                LED2_OFF,           # HeadWhite  = 20
                LED2_OFF,           # ModeRed
                LED2_OFF,           # ModeGreen
                LED2_OFF,           # ModeBlue
                LED2_OFF,           # Wireless
                LED3_INTENSITY_MIN, # BackRearColor      = 25
                LED3_INTENSITY_MIN, # BackRearWhite
                LED3_INTENSITY_MIN, # BackMiddleColor
                LED3_INTENSITY_MIN, # BackMiddleWhite
                LED3_INTENSITY_MIN, # BackFrontColor
                LED3_INTENSITY_MIN, # BackFrontWhite     = 30
                )

finalValues = list(DEFAULT)
lastValues = None

# Indicator intensity constant.
INTENSITY_DIVIDE = 10

# If this is true then only indicators that are legal in the game are shown
onlyLegalIndicators = True

def framePostset():
    global finalValues, lastValues

    if onlyLegalIndicators:
        #finalValues[Mouth]
        #finalValues[TailV]
        #finalValues[TailH]
        #finalValues[EarL]
        #finalValues[EarR]
        #finalValues[Led1]
        #finalValues[Led2]
        #finalValues[Led3]
        #finalValues[Led4]
        #finalValues[Led5]
        #finalValues[Led6]
        #finalValues[Led7]
        #finalValues[Led8]
        #finalValues[Led9]
        #finalValues[Led10]
        #finalValues[Led11]
        #finalValues[Led12]
        #finalValues[Led13]
        #finalValues[Led14]
        finalValues[HeadColor] = LED2_OFF
        #finalValues[HeadWhite]

        ears = finalValues[ModeRed:ModeBlue + 1]
        if ears == [LED2_ON, LED2_ON, LED2_OFF]:    # white instead of yellow
            finalValues[ModeRed]    = LED2_ON
            finalValues[ModeGreen]  = LED2_ON
            finalValues[ModeBlue]   = LED2_ON
        elif ears == [LED2_OFF, LED2_ON, LED2_OFF]:  # green is ok
            pass
        else:
            finalValues[ModeRed]    = LED2_OFF
            finalValues[ModeGreen]  = LED2_OFF
            finalValues[ModeBlue]   = LED2_OFF

        #finalValues[Wireless]
        finalValues[BackRearColor]      = LED3_INTENSITY_MIN
        #finalValues[BackRearWhite]
        finalValues[BackMiddleColor]    = LED3_INTENSITY_MIN
        #finalValues[BackMiddleWhite]
        #finalValues[BackFrontColor] #Red is ok
        #finalValues[BackFrontWhite]
    
    if finalValues != lastValues:
        VisionLink.sendIndicators(*tuple(finalValues))
        lastValues = finalValues[:] 


import Action
import Global
import Constant
import VisionLink
import hStuck
import hTrack
import hTeam

def superDebug():
    showTeamIndicator()
    showBallIndicator()    
    showRoleIndicator()
    showDropFrameIndicator()
    showTimeCriticalIndicator()
    showGrabbedIndicator()
    #showFrontCollision()
    showTrackingIndicator()
    showWirelessDownIndicator()

#    showHighLowGainIndicator()     # this seems to have disappeared
#    showVisualTeammateIndicator()
        
        
# Set the default indicator parameters of the dog (all off)
def setDefault():
    global finalValues
    finalValues = list(DEFAULT)
    
def showBallIndicator(): 
    global finalValues
    finalValues[BackFrontColor]  = LED3_INTENSITY_MIN
    finalValues[BackMiddleWhite] = LED3_INTENSITY_MIN
    finalValues[BackRearWhite]   = LED3_INTENSITY_MIN
    
    if Global.ballSource == Constant.VISION_BALL: 
        finalValues[BackFrontColor] = LED3_INTENSITY_MAX
        
    elif Global.ballSource == Constant.GPS_BALL: 
        finalValues[BackMiddleWhite] = LED3_INTENSITY_MAX

    else: 
        finalValues[BackRearWhite] = LED3_INTENSITY_MAX

def showTrackingIndicator(): 
    # Is this good comparision?
    if hTrack.gVisBallPan == Action.finalValues[Action.Panx]\
        and hTrack.gVisBallTilt == Action.finalValues[Action.Tilty]\
        and hTrack.gVisBallCrane == Action.finalValues[Action.Cranez]:
                
        showRedEye(True)
        
# Flash back wite LEDs when wireless down. This should be left enabled
# in the competition
gWirelessDownFlashCounter = 0
def showWirelessDownIndicator():
    global gWirelessDownFlashCounter
    
    if not hTeam.wirelessIsDown():
        return

    gWirelessDownFlashCounter += 1

    if gWirelessDownFlashCounter > 12:
        gWirelessDownFlashCounter = 0
    
    if gWirelessDownFlashCounter < 4:
        finalValues[BackFrontWhite] = LED3_INTENSITY_MAX
        finalValues[BackMiddleWhite] = LED3_INTENSITY_MIN
        finalValues[BackRearWhite] = LED3_INTENSITY_MIN
    elif gWirelessDownFlashCounter < 8:
        finalValues[BackFrontWhite] = LED3_INTENSITY_MIN
        finalValues[BackMiddleWhite] = LED3_INTENSITY_MAX
        finalValues[BackRearWhite] = LED3_INTENSITY_MIN
    else:
        finalValues[BackFrontWhite] = LED3_INTENSITY_MIN
        finalValues[BackMiddleWhite] = LED3_INTENSITY_MIN
        finalValues[BackRearWhite] = LED3_INTENSITY_MAX
        
def showGainIndicator():    
    global finalValues
    if Global.isHighGain:
        finalValues[HeadWhite] = LED2_OFF
    else:
        finalValues[HeadWhite] = LED2_ON

def showGrabbedIndicator():    
    global finalValues
    import sGrab
    if sGrab.isGrabbed:
        finalValues[HeadWhite] = LED2_ON
    else:
        finalValues[HeadWhite] = LED2_OFF


        
# Lights back indicators to show the current team (see changeTeamIndicator)
def showTeamIndicator():
    if Global.teamColor == Constant.BLUE_TEAM:
        changeTeamIndicator(True, False)
    else:
        changeTeamIndicator(False, True)

def changeTeamIndicator(isBlueTeam, isRedTeam):
    global finalValues
    if (isBlueTeam and isRedTeam) or (not isBlueTeam and not isRedTeam):
        zero = 0    # which is what #Debug.goAndMakeTraceBack() does
        zero[0] = 0 # which is what #Debug.goAndMakeTraceBack() does
        return

    if isRedTeam:
        finalValues[BackRearColor]  = LED3_INTENSITY_MIN / INTENSITY_DIVIDE
        finalValues[BackRearWhite]  = LED3_INTENSITY_MIN / INTENSITY_DIVIDE
        finalValues[BackFrontColor] = LED3_INTENSITY_MAX / INTENSITY_DIVIDE
        finalValues[BackFrontWhite] = LED3_INTENSITY_MIN / INTENSITY_DIVIDE
    else:
        finalValues[BackRearColor]  = LED3_INTENSITY_MAX / INTENSITY_DIVIDE
        finalValues[BackRearWhite]  = LED3_INTENSITY_MIN / INTENSITY_DIVIDE
        finalValues[BackFrontColor] = LED3_INTENSITY_MIN / INTENSITY_DIVIDE
        finalValues[BackFrontWhite] = LED3_INTENSITY_MIN / INTENSITY_DIVIDE
    
        
def showFrontCollision():
    fl = VisionLink.getSwitchSensorCount(Constant.SWITCH_FL_PALM)
    fr = VisionLink.getSwitchSensorCount(Constant.SWITCH_FR_PALM)
    if fl == 0 and fr == 0:
        showGreenEyes(on=False)
    elif fl > 0 and fr == 0:
        showGreenEyes(Constant.LEFT)
    elif fl == 0 and fr > 0:
        showGreenEyes(Constant.RIGHT)
    else: # fl > 0 and fr > 0:
        showGreenEyes(Constant.MIDDLE)


# WARNING:
# showRedEyes() and showGreenEyes() is not compatible,
# so do NOT call them in the same frame

# Reason: to use a particular mode in the face leds,
# we need to adjust the mode in only one led (led14).
# This could due to a lower level OS bug or something.


#def showRedEye(on = True, both = False):
#    # we should avoid using the red eyes
#    showBlueEyes(on, both)

# WARNING:
# if Led1 or Led2 is on green,
# calling this function will change them to white

# Show the red "eyes" on face (or not if on = False). 4 Leds if both, else 2
def showRedEye(on = True, both = False):
    global finalValues
    if on:
        finalValues[Led14] = LED3_INTENSITY_MAX
        if both:
            finalValues[Led13] = LED3_INTENSITY_MAX
    else :
        finalValues[Led13] =  LED3_INTENSITY_MIN
        finalValues[Led14] =  LED3_INTENSITY_MIN

# similar with showRedEye
def showBlueEyes(on = True, both = False):
    global finalValues
    return # disabled - interferes with red
    if on:
        finalValues[Led14] = LED3_INTENSITY_MAX | LED3_MODE_B # use mode B
        if both:
            finalValues[Led13] = LED3_INTENSITY_MAX 
    else:
        finalValues[Led13] = LED3_INTENSITY_MIN 
        finalValues[Led14] = LED3_INTENSITY_MIN

# WARNING:
# if Led13 or Led14 is on red,
# calling this function will change them to blue

# arguments can be on: True / False
#                  whichEye: Constant.LEFT / Constant.RIGHT / Constant.MIDDLE (both eyes)
#                         NB: Left and Right are following the legs
def showGreenEyes(whichEye=Constant.MIDDLE, on=True):
    global finalValues
    return # disabled - interferes with red
    finalValues[Led14] |= LED3_MODE_B # use mode B
    if on:
        if whichEye > 0: #e.g. Constant.LEFT:
            finalValues[Led1] = LED3_INTENSITY_MIN
            finalValues[Led2] = LED3_INTENSITY_MAX
        elif whichEye < 0: #e.g. Constant.RIGHT:
            finalValues[Led1] = LED3_INTENSITY_MAX            
            finalValues[Led2] = LED3_INTENSITY_MIN
        else: # if eye == Constant.MIDDLE:
            finalValues[Led1] = LED3_INTENSITY_MAX
            finalValues[Led2] = LED3_INTENSITY_MAX
    else:
        finalValues[Led1] = LED3_INTENSITY_MIN
        finalValues[Led2] = LED3_INTENSITY_MIN


# Show pattern in the face.
# Input is a tuple of 5 ints. Each represent the height of a coloumn of lights
# in the face.
# 0 means no light, 3 means 3 blocks of lights.
# For the 1st and 5th column there are 3 blocks, for the middle 3 there can 
# only be 2 blocks.
FACE_LED_COLUMNS = [(Led7, Led3, Led1),
                    (Led9, Led5),
                    (Led12, Led11),
                    (Led10, Led6),
                    (Led8, Led4, Led2)
                    ]

def showFacePattern(pattern):
    global finalValues
    for i in range(len(pattern)):
        leds = FACE_LED_COLUMNS[i]
        for j in range(len(leds)):
            if j < pattern[i]:
                finalValues[leds[j]] = LED3_INTENSITY_MAX
            else:
                finalValues[leds[j]] = LED3_INTENSITY_MIN



# Can only store up to 8 (0-7) color, please use LEDs (showFacePattern) if you
# have more things to show in head (and use OTHER for myRole).

# RGB Color Information for the light on head, used in showHeadColor
RGB_NONE        = (False, False, False)
RGB_BLUE        = (False, False, True)
RGB_GREEN       = (False, True, False)
RGB_LIGHT_BLUE  = (False, True, True)
RGB_RED = RGB_ORANGE = (True, False, False)
RGB_PURPLE      = (True, False, True)
RGB_YELLOW      = (True, True, False)
RGB_WHITE       = (True, True, True)    
    
# Show up the color in the head.
# Input color is a tuple (bool, bool, bool) where each boolean represents
# whether the respective color (red, green, blue) should be on or not
# in the head.
def showHeadColor(color):
    global finalValues
    red   = LED2_OFF
    green = LED2_OFF
    blue  = LED2_OFF

    if (color[0]):
        red     = LED2_ON

    if (color[1]):
        green   = LED2_ON

    if (color[2]):
        blue    = LED2_ON

    finalValues[ModeRed]   =  red
    finalValues[ModeGreen] =  green
    finalValues[ModeBlue]  =  blue

    
# Show which role this player is by changing the head colour.
gToggleRoleIndicator = 0
def showRoleIndicator():
    global finalValues
    global gToggleRoleIndicator
       
    if Global.myRole == Constant.SUPPORTER:
        showHeadColor(RGB_PURPLE)

    elif Global.myRole == Constant.ATTACKER:
        showHeadColor(RGB_YELLOW)

    elif Global.myRole == Constant.WINGER:
        showHeadColor(RGB_BLUE)

    elif Global.myRole == Constant.DEFENDER:
        showHeadColor(RGB_GREEN) 
    
    elif Global.myRole == Constant.STRIKER:
        showHeadColor(RGB_WHITE)

    else:
        showHeadColor(RGB_NONE)
    

    # Incrementing this counter up to 10 and reset to 0.
    if gToggleRoleIndicator > 10: 
        gToggleRoleIndicator = 0
    gToggleRoleIndicator += 1
    
    # If ball is contested, flicker the head light.   
    if Global.myRole == Constant.ATTACKER\
        and hStuck.isBallContested():
        if gToggleRoleIndicator > 4:
            showHeadColor(RGB_LIGHT_BLUE)
        
# Flip the tail up if frames have been dropped (due to processor 
# over-utilisation)
def showDropFrameIndicator():
    global finalValues
    # Frames come at a rate of 30 frames/sec.
    # If cpu can't finish processing the frame in 1/30 sec, frame drops
    # occur.
    
    # Max time before the frame become dropped.
    # CPU_OVER = (1.0 / 30.0) * 1000000.0
    # CPU_80          = CPU_OVER * 0.8
    CAMERA_FRAME_MS = 1000.0 / 30.0
    JOINT_FRAME_MS  = 8     
    FRAME_SKIP      = (CAMERA_FRAME_MS * 1.2 / JOINT_FRAME_MS) + 2

    # Each frame-drop occur will at least incur this many frames of tail down.
    TAIL_DOWN_DURATION = 15;

    # Tail indicator for CPU time.
    global tail_down_count
    
    # OPENR frame ID.
    # cpuUsage = VisionLink.getCPUUsage()
    currentFrameID, lastFrameID = VisionLink.getCurrentPreviousFrameID()
    
    if tail_down_count <= 0:
        if currentFrameID - lastFrameID > FRAME_SKIP and lastFrameID > 0:
            finalValues[TailV] = TAIL_DOWN
            tail_down_count = TAIL_DOWN_DURATION            
    else: 
        finalValues[TailV] = TAIL_DOWN
        tail_down_count -= 1
        
# Show white back lights in time critical mode
def showTimeCriticalIndicator():
    if Global.isTimeCritical:
        #finalValues[BackFrontWhite] = LED3_INTENSITY_MAX
        finalValues[BackMiddleWhite] = LED3_INTENSITY_MAX
        finalValues[BackRearWhite] = LED3_INTENSITY_MAX
    
        
# Indicate whether we or the opponent are kicking off by lighting (or not)
# the middle back button
def showKickOffIndicator():
    global finalValues
    if VisionLink.getKickOffTeam() == VisionLink.getTeamColor():
        finalValues[BackMiddleColor] =\
           LED3_INTENSITY_MAX / INTENSITY_DIVIDE
    else:
        finalValues[BackMiddleColor] = \
            LED3_INTENSITY_MIN / INTENSITY_DIVIDE
        
# Show an estimate of battery charge remaining in a pattern across the head
def showBatteryStatus():
    current = VisionLink.getBatteryLevel()    
    if current >= 100:
        showFacePattern([1,1,1,1,1])
    elif current >= 80:
        showFacePattern([0,1,1,1,1])
    elif current >= 60:
        showFacePattern([0,1,1,1,0])
    elif current >= 40:
        showFacePattern([0,0,1,1,0])
    elif current >= 20:
        showFacePattern([0,0,1,0,0])
    else:
        showFacePattern([0,0,0,0,0])


generator = None
def DecideNextAction():
    #for i in xrange(14):
    #    finalValues[Led1+i] = 0    
    superDebug()
    

def gene():
    global generator    
    if not generator:
        generator = testLedGen()
        return
    if not generator.next():
        generator = None
        print "finish"

def testLedGen():
    counter = 0
    while counter < 60 * 8:
        if counter < 60:
            showRedEye()
        elif counter < 60 * 2:
            showRedEye(both=True)
        elif counter < 60 * 3:
            showBlueEyes()
        elif counter < 60 * 4:
            showBlueEyes(both=True)
        elif counter < 60 * 5:
            showGreenEyes()
        elif counter < 60 * 6:
            showGreenEyes(Constant.LEFT)
        elif counter < 60 * 7:
            showGreenEyes(Constant.RIGHT)
        elif counter < 60* 8:
            showGreenEyes()
            showRedEye()
        counter += 1
        yield True
    yield False

