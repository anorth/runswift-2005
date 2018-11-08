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


#===============================================================================
#   Python Behaviours : 2005 (c) 
#
# LocChallenge.py
#
# The localisation challenge behaviour. Moves to a series of points on the
#   field, and wags its tail at each point. Uses some stuff in VisualCortex
#   and GPS that is specific to the challenge, which you should turn on with
#   the flag_LOC_CHALLENGE variable in share/Common.h.
# TODO: check the field clipping, don't forget to check the beacon setting.
#
#===============================================================================

# 
# cSLAM.py is the python code for running the localisation challenge.
# 
# setting up cSLAM;
# 
#  - in robot/share/Challenge.h 
#         Uncomment #define LOCALISATION_CHALLENGE
#         Comment #define HARDCODED_PINKLANDMARK
#  - in robot/PyCode/cSLAM.py 
#         Set DO_PAUSE_EVERY_WAG_TAIL to True
#         Set WAG_TAIL_TIME to 180
#         Set paused = True        
#         Set useMinimalMode = False   
#         Set YUVDEBUG = False     
#         Set PRINTDEBUG = False
#         Should it VisionLink.resetGPS() between first and second 
#         phase because the dog is going to be random anyway? 
#         Does it give better or worse?
#         Tweak the number of green when facing the field.
    
# running cSLAM;        
#         
#  - the dog doing the challenge must be a blue dog, i.e. 
#    be careful of pressing the buttons on the back during startup 
#    in case you accidentaly change team colour
# 

"""
Localisation challenger 2005
"""
import Constant
import Global
import VisionLink
import sActiveLocalise
import hMath
#import HelpLong
#import HelpTrack
import math
import Indicator
import Action
import hTrack
import sGreenBackOff

#===============================================================================
# Global variables of this module.
#===============================================================================

POS_TIME = 7                    # Number of frames to confirm location
POS_ACCURACY = 5#3                # Position accuracy required to proceed
POS_MAX_VARIANCE = 1600 #2500 #10000  # Max position variance allowed to proceed
POS_MIN_LOST_VARIANCE = 2500    #Minimum variance to indicate the dog lost has to orientate again.
SURVEY_TIME = 1500              # How many frames dog should survey for
ORIENTATE_TIME = 300#240 #300       # How many frames dog should stat. localise for
WATCH_FEATURE = 40              # How long dog will focus on an edge feature
WAGTAIL_TIME  = 60#180             # How long to wagging the tail
DO_PAUSE_EVERY_WAG_TAIL = True  # Should it pause after wagging the tail
#DO_PAUSE_EVERY_WAG_TAIL = False  # Should it pause after wagging the tail
YUVDEBUG = False                # Debug YUV, default == False
PRINTDEBUG = False              # Printing debugging statements
AFTER_BACK_OFF_MOVE = 30        # After back off move forward for X frame
TURN_COUNTER = 30               # Turn for around 90 degree
MIN_NUM_GREEN_COUNT = 25        # Minimum green to indicate facing the field.
MIN_DISTANCE_FROM_THE_SIDE = 60 # Minimum distance from the side before doing mapping.

# Global coordinates of field line features to face when close to destination
FACING_POSITIONS = [
    #top left corner of enemy goal box
    (Constant.MIN_GOALBOX_EDGE_X, Constant.TOP_GOALBOX_EDGE_Y),
    #top right corner of enemy goal box
    (Constant.MAX_GOALBOX_EDGE_X, Constant.TOP_GOALBOX_EDGE_Y),
    #down left corner of enemy goal box
    (Constant.MIN_GOALBOX_EDGE_X, Constant.FIELD_LENGTH),
    #down right corner of enemy goal box
    (Constant.MAX_GOALBOX_EDGE_X, Constant.FIELD_LENGTH),
    #top left corner of own goal box
    (Constant.MIN_GOALBOX_EDGE_X, Constant.OWN_GOALBOX_EDGE_Y),
    #top right corner of own goal box
    (Constant.MAX_GOALBOX_EDGE_X, Constant.OWN_GOALBOX_EDGE_Y),
    #down left corner of own goal box
    (Constant.MIN_GOALBOX_EDGE_X, 0),
    #down right corner of own goal box
    (Constant.MAX_GOALBOX_EDGE_X, 0),
    #middle with left line
    (0, Constant.FIELD_LENGTH / 2),
    #middle with right line
    (Constant.FIELD_WIDTH, Constant.FIELD_LENGTH / 2),
    #corner field
    (0, 0),
    (0, Constant.FIELD_LENGTH),
    (Constant.FIELD_WIDTH, 0),
    (Constant.FIELD_WIDTH, Constant.FIELD_LENGTH),
    #TODO: add more line detection
    #beside of the goals, middle circle and line
]

# Global coordinates of field line features to localise off
LOCALISE_POSITIONS = FACING_POSITIONS + [
    (Constant.FIELD_WIDTH / 2, Constant.FIELD_LENGTH / 2),
    (0, Constant.BEACON_NEAR_Y),
    (0, Constant.BEACON_FAR_Y),
    (Constant.FIELD_WIDTH, Constant.BEACON_NEAR_Y),
    (Constant.FIELD_WIDTH, Constant.BEACON_FAR_Y),
    #(0, 80),
    #(0, 360),
    #(Constant.FIELD_WIDTH, 80),
    #(Constant.FIELD_WIDTH, 360)
]

points = []                     # These are the points the dog should move to
pointSequence = []              # The order of visiting the points
moving = True                   # Flag to indicate if dog is moving
faceDest = True                 # Should dog face destination as it moves
turning = True                  # Whether dog should only turn
paused = False                   # Is the dog paused or not
#paused = True                   # Is the dog paused or not
                                # Start paused == True
initialised = False             # Has the points file been loaded yet?
pathSelected = False            # Have we chosen the point sequence to visit?
facingPoint = (0, 0)            # Which line feature to face when near dest.
movePoint = (0, 0)              # Coordinates of destination point to move to
moveToPoint = 0                 # Seq index of the point the dog is moving to
distanceToPoint = 1000          # How far dog is from destination point 
localiseCounter = 0             # Determines when to switch localising features
positionedCounter = 0           # How long the dog has been at the point
wagTailCounter = 0              # Keeps track of tail wagging progress
surveyCounter = 0               # How long surveying has been going on
orientateCounter = 0            # How long re-orientation has been going
horizonPan = 0                  # When looking up, this is the pan value to use

#useMinimalMode = False           # Default, use mapping state.
useMinimalMode = True           # This makes the dog skip initial stationary
                                # localise stage when first turned on

sendYUVcounter = 0              #counter to send YUV image
reset          = 1              #Reset everything, not recommended, better to restart the dog
debuggingCounter      = 0       
afterBackOffCounter = AFTER_BACK_OFF_MOVE + 1        # After back off, default
turnAroundCounter = TURN_COUNTER + 1
numberOfTurn = 0
standStillCounter = 0 
leftBoxGreen = 0
middleBoxGreen = 0
rightBoxGreen = 0
pinkInLeft = 0
pinkInMidlle = 0
pinkInRight = 0
turnDirection = Constant.RIGHT       # -1 is right 
turnIndicator = 0 # indicator if we have turn because we are facing outside the field
#WALKTYPE = Action.NormalWalkWT
#WALKTYPE = Action.SkellipticalWalkWT
WALKTYPE = Action.SkeFastForwardMWT

mapSecondPoint = None
ScanCounter = 0
#===============================================================================
# Functions of this module.
#===============================================================================
# The top level decisions for this player
startingTime = VisionLink.getCurrentTime()
def DecideNextAction():
    global surveyCounter, wagTailCounter, positionedCounter, orientateCounter
    global initialised, paused, moving,sendYUVcounter,pathSelected,reset
    global debuggingCounter,afterBackOffCounter,turnAroundCounter,numberOfTurn
    global leftBoxGreen, middleBoxGreen,rightBoxGreen,turnDirection
    global pinkInLeft,pinkInMidlle,pinkInRight,ScanCounter
    
    if reset == 1:
        if not useMinimalMode :
            print "Turn on the vision and motion update"
            VisionLink.resetGPS()
            VisionLink.resetEdgeMap()
            VisionLink.setIfGPSVisionUpdate(True)
            VisionLink.setIfGPSMotionUpdate(True)
            VisionLink.setUseBeacons(True)
            VisionLink.resetPinkMapping()
        else :
            print "Second phase"
            VisionLink.printCPlusPlus() #print the gaussian
            VisionLink.initPinkObject() #only when the mapping already occurred
            VisionLink.resetEdgeMap()
            VisionLink.setIfGPSVisionUpdate(True) #this is must for LocalisePinkUpdate() in gps.cc
            VisionLink.setIfGPSMotionUpdate(True)
            #VisionLink.setUseBeacons(False) #This is set flag to not do gps vision update
            VisionLink.setUseBeacons(True) #This is set flag to not do gps vision update
            VisionLink.resetGPS()
        reset = 0

    
    debuggingCounter +=1
    
    Global.frameReset()
    Action.walk(0, 0, 0,walkType=WALKTYPE)#HelpLong.setDefaultAction()
    
    #HelpLong.setDefaultIndicators()
        
    # Turn face lights on if we're seeing pink blobs to localise off
    if VisionLink.getUsePinkUpdate() == 1:
        Indicator.showFacePattern((2, 1, 0, 1, 2))
    else:
        Indicator.showFacePattern((0, 0, 0, 0, 0))
        
    # Load the points file
    if not initialised:
        InitialiseChallenge()
    
    Indicator.showHeadColor(Indicator.RGB_NONE)
    
    #To check if it localise
    #if ScanCounter > 90: #90
#     if Global.desiredPan > 88 or Global.desiredPan < -88:
#         Indicator.showHeadColor(Indicator.RGB_YELLOW)
#         VisionLink.resetPinkUpdate()
#         ScanCounter = 0
#         hTrack.panLow = False 
#     if ScanCounter
#         VisionLink.setGlobalMaxEdge()
#     hTrack.scan(highSpeed = 3, lowSpeed = 3, lookDown = False)
#     return
        
    if paused:
        DoPause()
        return
        
    if False:    
        if (sendYUVcounter < 0):
            (selfX, selfY) = Global.selfLoc.getPos()
            selfH = Global.selfLoc.getHeading()              
            print "dogX:",selfX, " dogY:", selfY, " dogH:",selfH
            sendYUVcounter = 0
        sendYUVcounter+=1
        
    
    # If we're in the mapping stage of the challenge, continue doing the survey
    if not useMinimalMode and surveyCounter < SURVEY_TIME:
        DoSurvey()
        return
    
    #if lost go orientate    
    if (Global.selfLoc.getPosVar() > POS_MIN_LOST_VARIANCE and sGreenBackOff.perform(True, 30) == Constant.STATE_EXECUTING): 
        #afterBackOffCounter = 0
        orientateCounter = 0
        turnDirection = -1
        return
                
    if sGreenBackOff.perform(True, 120) == Constant.STATE_EXECUTING:
        afterBackOffCounter = 0
        orientateCounter = 0
        turnDirection = -1
        return 
    
    if afterBackOffCounter <= AFTER_BACK_OFF_MOVE:
        Action.walk(Action.MAX_FORWARD, 0 , 0, "ddd", walkType=WALKTYPE)
        #Action.walk(0, 0, hMath.CLIP(turnCCW, 40))
        afterBackOffCounter += 1
        return  
    
    if turnAroundCounter <= TURN_COUNTER:
        Action.walk(0, 0 , Action.MAX_TURN*turnDirection, walkType=WALKTYPE)
        #Action.walk(0, 0 , -40, Action.GrabTurnFast)
        turnAroundCounter +=1
        return
    # If we got this far, we're in the second stage of the challenge, and so are
    # moving to each of the five points loaded from the points file
    
    # Re-orientate (we might have been moved during the break)
    if orientateCounter <= ORIENTATE_TIME:
        if orientateCounter == 0:
            VisionLink.resetEdgeMap()
                
        headPan = (orientateCounter % 120) * 3 - 90.0
        if headPan > 90:
            headTilt = -30
            headPan = 180 - headPan
        else:
            headTilt = -5
        
        #print "VisionLink.getFieldFeatureCount(): ",VisionLink.getFieldFeatureCount()
        
        if (-90 < headPan < -30):
            rightBoxGreen += VisionLink.getFieldFeatureCount()
            if (VisionLink.getUsePinkUpdate()) : 
                pinkInRight = True
        elif (-30 < headPan < 30):
            middleBoxGreen += VisionLink.getFieldFeatureCount()
            if (VisionLink.getUsePinkUpdate()) :
                pinkInMidlle = True
        else:            
            leftBoxGreen += VisionLink.getFieldFeatureCount()
            if (VisionLink.getUsePinkUpdate()) :
                pinkInLeft = True
        
        Action.setHeadParams(headPan, headTilt, 0,Action.HTAbs_h)
        #if Global.desiredPan > 88 or Global.desiredPan < -88:
        #Indicator.showHeadColor(Indicator.RGB_YELLOW)
        #
    
        #hTrack.stationaryLocalise()
        if orientateCounter == ORIENTATE_TIME:
            """
            print "left:", leftBoxGreen
            print "middle:", middleBoxGreen
            print "right:", rightBoxGreen
            """
            VisionLink.resetPinkUpdate()
            Indicator.showHeadColor(Indicator.RGB_YELLOW)
            if (Global.selfLoc.getPosVar() > POS_MIN_LOST_VARIANCE):
                #Should i set global max edge?, 
                #make sure stationary mapping has been run for a while
                print " global max edge, var:" , Global.selfLoc.getPosVar()
                VisionLink.setGlobalMaxEdge()
            if numberOfTurn < 1:
                # turn to direction where more green points. and localise again.
                numberOfTurn +=1
                orientateCounter = 0
                if (rightBoxGreen > 1500 and rightBoxGreen > leftBoxGreen):
                    turnDirection = Constant.RIGHT
                    turnAroundCounter = 0
                elif(leftBoxGreen > 1500 and leftBoxGreen > rightBoxGreen):
                    turnDirection = Constant.LEFT
                    turnAroundCounter = 0
                else:
                    afterBackOffCounter = -AFTER_BACK_OFF_MOVE*2
                    turnAroundCounter = -TURN_COUNTER
            #elif numberOfTurn == 1:
            #    numberOfTurn = 0
            #    VisionLink.printCPlusPlus()
            #    print "======================="
            #2 line below should be commented
            #orientateCounter = 0
            #turnAroundCounter = 0
            leftBoxGreen =  0
            middleBoxGreen = 0
            rightBoxGreen = 0
            pinkInLeft = False
            pinkInMidlle = False
            pinkInRight = False            
        orientateCounter = orientateCounter + 1            
        return
    
    numberOfTurn = 0
        
    # Find the best path between points
    if not pathSelected:
        FindShortestPath()    

    # Calculate gps coordinates of the destination point we want to move to
    CalcMovePoint()
    
    # Check if we've arrived at destination
    if AtDestPoint():
        positionedCounter = positionedCounter + 1
    else:
        positionedCounter = 0
    
    # If we've been at the destination long enough, stop moving
    if positionedCounter > POS_TIME:
        print "OK to wag the tail with posVar:" , Global.selfLoc.getPosVar()
        moving = False
        
    if moving:
        Localise()
        MoveToPoint()
    else:
        WagTail()
        
        if wagTailCounter > WAGTAIL_TIME:
            SwitchToNextPoint()

            
def findMapSecondPoint():
    global mapSecondPoint
    pos = Global.selfLoc.getPos()
    #first quadrant
    if (pos[0] < Constant.FIELD_WIDTH/2 and pos[1] < Constant.FIELD_LENGTH/2):
        secondPoint = (Constant.FIELD_WIDTH - MIN_DISTANCE_FROM_THE_SIDE, Constant.FIELD_WIDTH - MIN_DISTANCE_FROM_THE_SIDE)
    #second quadrant
    elif (pos[1] < Constant.FIELD_LENGTH/2):
        secondPoint = (MIN_DISTANCE_FROM_THE_SIDE, Constant.FIELD_WIDTH - MIN_DISTANCE_FROM_THE_SIDE)
    #third quadrant
    elif (pos[0] < Constant.FIELD_WIDTH/2):
        secondPoint = (Constant.FIELD_WIDTH - MIN_DISTANCE_FROM_THE_SIDE, MIN_DISTANCE_FROM_THE_SIDE)
    #last quadrant
    else:
        secondPoint = (MIN_DISTANCE_FROM_THE_SIDE, MIN_DISTANCE_FROM_THE_SIDE)
    mapSecondPoint = (secondPoint[0],secondPoint[1],hMath.normalizeAngle_0_360(hMath.getHeadingBetween(pos[0],pos[1],secondPoint[0],secondPoint[1])))
    
    
    
#-------------------------------------------------------------------------------
# Loads the points file

def InitialiseChallenge():
    global points, pointSequence, initialised

    points = []

    try:
        # Parse the coordinates contained in the points file
        f = open("/MS/points.cfg")
        for line in f.readlines():
            # Load a single coordinate pair
            centreCoord = map(lambda x:int(x), line.split())
            
            print "cSLAM: read from points.cfg:", centreCoord  

            if len(centreCoord) == 2:
                # Convert challenge coordinates to gps coordinates
                if VisionLink.getTeamColor() == 1:
                    # Convert to red team coordinates
                    gpsCoord = (Constant.FIELD_WIDTH / 2 - centreCoord[1],
                                centreCoord[0] + Constant.FIELD_LENGTH / 2)
                else:
                    # Convert to blue team coordinates
                    gpsCoord = (centreCoord[1] + Constant.FIELD_WIDTH / 2,
                                Constant.FIELD_LENGTH / 2 - centreCoord[0])
        
                # Add to points list
                points = points + [gpsCoord]
                
                print "cSLAM: adjusted to gps coord:", gpsCoord
                
              
    
        f.close()

    except:
        print "file not exist or can't be opened"
        points = []
        
    # If file wasn't loaded, use a default set of points (for testing only!)
    if len(points) < 5:
        print "Coordinate file not loaded successfully, using default points"
        pointBallOut1 = (40,100)
        pointBallOut2 = (Constant.FIELD_WIDTH-40,100)
        pointBallOut3 = (40,Constant.FIELD_LENGTH-100)
        pointBallOut4 = (Constant.FIELD_WIDTH-40,Constant.FIELD_LENGTH-100)
        pointMiddle = (Constant.FIELD_WIDTH/2,Constant.FIELD_LENGTH/2)
        #top left corner of enemy goal box
        pointEnemyGoalBox1 = (Constant.MIN_GOALBOX_EDGE_X, Constant.TOP_GOALBOX_EDGE_Y)
        #top right corner of enemy goal box        
        pointEnemyGoalBox2 = (Constant.MAX_GOALBOX_EDGE_X, Constant.TOP_GOALBOX_EDGE_Y)
        #down left corner of enemy goal box
        pointEnemyGoalBox3 = (Constant.MIN_GOALBOX_EDGE_X, Constant.FIELD_LENGTH)
        #down right corner of enemy goal box
        pointEnemyGoalBox4 = (Constant.MAX_GOALBOX_EDGE_X, Constant.FIELD_LENGTH)
        #middle of enemy goal
        pointEnemyGoalBoxMiddle = (Constant.FIELD_WIDTH/2,Constant.FIELD_LENGTH)
        #top left corner of own goal box
        pointOwnGoalBox1 = (Constant.MIN_GOALBOX_EDGE_X, Constant.OWN_GOALBOX_EDGE_Y)
        #top right corner of own goal box
        pointOwnGoalBox2 = (Constant.MAX_GOALBOX_EDGE_X, Constant.OWN_GOALBOX_EDGE_Y)
        #down left corner of own goal box
        pointOwnGoalBox3 = (Constant.MIN_GOALBOX_EDGE_X, 0)
        #down right corner of own goal box
        pointOwnGoalBox4 = (Constant.MAX_GOALBOX_EDGE_X, 0)
        pointOwnGoalBoxMiddle = (Constant.FIELD_WIDTH/2, 0)
        points = [pointBallOut1,pointOwnGoalBox2,pointMiddle,pointEnemyGoalBoxMiddle,pointBallOut3]
        
        dummy = [pointBallOut1,pointBallOut2,pointBallOut3,pointBallOut4,pointMiddle,pointEnemyGoalBox1,pointEnemyGoalBox2,pointEnemyGoalBox3,pointEnemyGoalBox4,pointEnemyGoalBoxMiddle,pointOwnGoalBox1,pointOwnGoalBox2,pointOwnGoalBox3,pointOwnGoalBox4,pointOwnGoalBoxMiddle]
        if dummy:
            pass
        #points = [(50,50), (100,100), (Constant.FIELD_WIDTH/2,Constant.FIELD_LENGTH/2), (300,300), (Constant.FIELD_WIDTH/2,Constant.FIELD_LENGTH)]
        #points = [(30,380), (95,60), (200,410), (70,220), (220,70)]
    VisionLink.resetEdgeMap()
    initialised = True

#-------------------------------------------------------------------------------
# Waits until head is pressed

def DoPause():
    global paused
    Action.walk(0, 0, 0,walkType=WALKTYPE)
    VisionLink.setIfGPSMotionUpdate(False)
    VisionLink.setIfGPSVisionUpdate(False)
    
    for pressSensor in range(1): # Used to react to back sensors
        if VisionLink.getPressSensorCount(pressSensor) > 8:
            print "Start moving to the next one"
            paused = False
            VisionLink.resetEdgeMap()
            VisionLink.setIfGPSMotionUpdate(True)
            VisionLink.setIfGPSVisionUpdate(True)
            #HelpLong.setDefaultIndicators()
            
#-------------------------------------------------------------------------------
# Scans the field borders for a duration close to a minute
def DoSurvey():
    global surveyCounter, paused, faceDest
    global pathSelected, turnIndicator
    #print "survey:",surveyCounter
    surveyCounter = surveyCounter + 1

    Action.walk(0, 0, 0,walkType=WALKTYPE)
    #Action.standStill()#HelpLong.setDefaultAction()
    VisionLink.setAllowPinkMapping(False)
    
    selfHead = Global.selfLoc.getHeading()
    
    # Initial localisation to get orientated
    if turnIndicator == 0 and VisionLink.getFieldFeatureCount() < MIN_NUM_GREEN_COUNT and surveyCounter < 150:
        Action.walk(0, 0, Action.MAX_TURN, walkType=WALKTYPE)
        
    elif surveyCounter < 150:
        faceDest = 0
        hTrack.stationaryLocalise(10)
        turnIndicator = 1
        
    elif surveyCounter < 300:
        pos = Global.selfLoc.getPos()
        #posVar = Global.selfLoc.getPosVar()
        
        hTrack.stationaryLocalise(10)
#         if ((surveyCounter - 150) % 30) == 0:
#             sActiveLocalise.SmartSetBeacon()
#             #sActiveLocalise.AlternateSetBeacon()
#         sActiveLocalise.DecideNextAction()
        
        if (pos[0] < MIN_DISTANCE_FROM_THE_SIDE or pos[0] > Constant.FIELD_WIDTH - MIN_DISTANCE_FROM_THE_SIDE or pos[1] < MIN_DISTANCE_FROM_THE_SIDE or pos[1] < MIN_DISTANCE_FROM_THE_SIDE):
            saGoToTarget(Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2)
            return
        if not mapSecondPoint :
            findMapSecondPoint()
            faceDest = hMath.normalizeAngle_0_360(mapSecondPoint[2] + 90)
        turnCCW = hMath.normalizeAngle_180(faceDest - selfHead)
        Action.walk(0, 0, turnCCW, walkType=WALKTYPE)
        
     
    # Localise a bit more
    elif surveyCounter < 400:
        if ((surveyCounter - 300) % 25) == 0:
            sActiveLocalise.SmartSetBeacon()
           
        sActiveLocalise.DecideNextAction()
           
    # Move head to start of pan scan
    elif surveyCounter < 420:
        Action.setHeadParams(-90, 0, 0, Action.HTAbs_h)

    # Do a 180 degree head pan at each quadrant direction
    elif surveyCounter < 900 :# (3sec*30fr*4quadrant)  #surveyCounter < SURVEY_TIME - 1: #surveyCounter < 1260:
        scanCount = (surveyCounter - 420) % 130 #90+40

        # Do a quick head pan
        if scanCount < 90:
            VisionLink.setAllowPinkMapping(True)
            hTrack.stationaryLocalise(5)
            #Action.setHeadParams(scanCount - 90, 0, 0, Action.HTAbs_h)

        # Select next quadrant
        elif scanCount == 90:
            faceDest = hMath.normalizeAngle_0_360(faceDest + 90)
            ChooseLeftmostBeacon()

        # Then turn 90 degrees            
        else:
            turnCCW = hMath.normalizeAngle_180((faceDest - selfHead))
            Action.walk(0, 0, turnCCW, walkType=WALKTYPE)
            sActiveLocalise.DecideNextAction()

    elif surveyCounter < 1080:#900 + 6*30:
        OptimisedGoToTarget(mapSecondPoint[0],mapSecondPoint[1])
        #Action.walk(Action.MAX_FORWARD, 0, 0, minorWalkType=Action.SkeFastForwardMWT)
        
    # Do a 180 degree head pan at each quadrant direction
    elif surveyCounter < SURVEY_TIME - 1:# (3sec*30fr*4quadrant)  #surveyCounter < SURVEY_TIME - 1: #surveyCounter < 1260:
        scanCount = (surveyCounter - 1080) % 130 #90+40

        # Do a quick head pan
        if scanCount < 90:
            VisionLink.setAllowPinkMapping(True)
            hTrack.stationaryLocalise(5)
            #Action.setHeadParams(scanCount - 90, 0, 0, Action.HTAbs_h)

        # Select next quadrant
        elif scanCount == 90:
            faceDest = hMath.normalizeAngle_0_360(faceDest + 90)
            ChooseLeftmostBeacon()

        # Then turn 90 degrees            
        else:
            turnCCW = hMath.normalizeAngle_180((faceDest - selfHead))
            Action.walk(0, 0, turnCCW, walkType=WALKTYPE)
            sActiveLocalise.DecideNextAction()
    
    elif surveyCounter < SURVEY_TIME - 1:
        surveyCounter = SURVEY_TIME - 1
        
    # Move to centre position 
    # Dog will be randomly moved, so move to the center would not make any difference
    # would be better keep scanning incase we miss pink landmark.   
    elif surveyCounter < SURVEY_TIME - 1:
        if False:
            #debug for restart
            #should set to false on challenge.
            surveyCounter = 400
            print "RESET"
            return
        if not pathSelected:
            FindShortestPath()
            print "first target :",movePoint[0], movePoint[1]
        if True:            
            print "moving to center"
            #Action.walk(0, 0, 0)
            #hTrack.saGoToTargetFacingHeading(145, 220, 270)
            saGoToTargetFacingHeading(145, 220, 270)
            hTrack.stationaryLocalise()
        else:
            CalcMovePoint()
            hTrack.stationaryLocalise()
            #hTrack.saGoToTarget(movePoint[0], movePoint[1])
            saGoToTarget(movePoint[0], movePoint[1])
        
    else: 
        paused = True
        print "Tyring to turn off vision"
        VisionLink.setIfGPSVisionUpdate(False)
        print "Tyring to turn off beacons"
        VisionLink.setUseBeacons(False)
        print "Ending survey:", hMath.getTimeElapsed(startingTime, VisionLink.getCurrentTime())
        VisionLink.initPinkObject()
        pathSelected = False
        #VisionLink.resetGPS()
        #VisionLink.setUseBeacons(True)

"""
def DoSurvey():
    global surveyCounter, paused, faceDest
    global pathSelected
    #print "survey:",surveyCounter
    surveyCounter = surveyCounter + 1

    Action.walk(0, 0, 0,walkType=WALKTYPE)
    #Action.standStill()#HelpLong.setDefaultAction()
    VisionLink.setAllowPinkMapping(False)
    
    selfHead = Global.selfLoc.getHeading()
    
    # Initial localisation to get orientated
    if surveyCounter < 150:
        faceDest = 0
        hTrack.stationaryLocalise()
    
    # Now get into starting position for pan scan
    elif surveyCounter < 300:
        pos = Global.selfLoc.getPos()
        posVar = Global.selfLoc.getPosVar()
        distanceToMiddle = hMath.getLength(map(lambda m,p:m-p, (Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2), pos))
        #To get better mapping
        #Move to the center if we have good posvar and we are not in the middle yet
        if (surveyCounter < 300 - 60 and posVar < POS_MAX_VARIANCE and distanceToMiddle > 15):
            hTrack.stationaryLocalise()
            #hTrack.saGoToTarget(Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2)
            saGoToTarget(Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2)
        else:            
            if ((surveyCounter - 150) % 30) == 0:
                sActiveLocalise.AlternateSetBeacon()
            sActiveLocalise.DecideNextAction()
            turnCCW = hMath.normalizeAngle_180(faceDest - selfHead)
            Action.walk(0, 0, turnCCW, walkType=WALKTYPE)

    # Localise a bit more
    elif surveyCounter < 400:
        if ((surveyCounter - 300) % 25) == 0:
            sActiveLocalise.SmartSetBeacon()
            
        sActiveLocalise.DecideNextAction()
    
    # Move head to start of pan scan
    elif surveyCounter < 420:
        Action.setHeadParams(-90, 0, 0, Action.HTAbs_h)

    # Do a 180 degree head pan at each quadrant direction
    # until time limit almost exceed.
    elif surveyCounter < SURVEY_TIME - 1: #surveyCounter < 1260:
        scanCount = (surveyCounter - 420) % 220

        # Do a slow head pan
        if scanCount < 180:
            VisionLink.setAllowPinkMapping(True)
            #hTrack.stationaryLocalise()
            Action.setHeadParams(scanCount - 90, 0, 0, Action.HTAbs_h)

        # Select next quadrant
        elif scanCount == 180:
            faceDest = hMath.normalizeAngle_0_360(faceDest + 90)
            ChooseLeftmostBeacon()

        # Then turn 90 degrees            
        else:
            turnCCW = hMath.normalizeAngle_180((faceDest - selfHead) * 0.5)
            Action.walk(0, 0, abs(turnCCW), walkType=WALKTYPE)
            sActiveLocalise.DecideNextAction()
    
    # Move to centre position 
    # Dog will be randomly moved, so move to the center would not make any difference
    # would be better keep scanning incase we miss pink landmark.   
    elif surveyCounter < SURVEY_TIME - 1:
        if False:
            #debug for restart
            #should set to false on challenge.
            surveyCounter = 400
            print "RESET"
            return
        if not pathSelected:
            FindShortestPath()
            print "first target :",movePoint[0], movePoint[1]
        if True:            
            print "moving to center"
            #Action.walk(0, 0, 0)
            #hTrack.saGoToTargetFacingHeading(145, 220, 270)
            saGoToTargetFacingHeading(145, 220, 270)
            hTrack.stationaryLocalise()
        else:
            CalcMovePoint()
            hTrack.stationaryLocalise()
            #hTrack.saGoToTarget(movePoint[0], movePoint[1])
            saGoToTarget(movePoint[0], movePoint[1])
        
    else: 
        paused = True
        VisionLink.setIfGPSVisionUpdate(False)
        VisionLink.setUseBeacons(False)
        VisionLink.initPinkObject()
        pathSelected = False
        VisionLink.resetGPS()
        #VisionLink.setUseBeacons(True)
"""

#-------------------------------------------------------------------------------
# Makes the head look at points of interest (field line points to localise off)

def Localise():
    global localiseCounter, horizonPan

    selfLoc = Global.selfLoc
    selfHead = selfLoc.getHeading()
    possibleFeatures = []
    
    # Find all line features within the robot's field of view
    for featureLoc in LOCALISE_POSITIONS:
        relPos = map(lambda b,s:b-s, featureLoc, selfLoc.getPos())
        featureDist = hMath.getLength(relPos)
        if featureDist == 0:
            featureHead = selfHead
        else:
            featureHead = hMath.RAD2DEG(math.atan2(relPos[1], relPos[0]))
            
        localHead = hMath.normalizeAngle_180(featureHead - selfHead)
        
        if abs(localHead) < 80 and featureDist > 15 and featureDist < 200:
            possibleFeatures.append(tuple(relPos))

    # Use localiseCounter to determine which feature to look at
    localiseCounter = localiseCounter + 1
    if localiseCounter >= len(possibleFeatures) * WATCH_FEATURE:
        localiseCounter = -15

        horizonPan = horizonPan + 30
        if horizonPan > 30:
            horizonPan = -30

    # Look up every once in a while in the hope of seeing a pink blob.
    if localiseCounter < 0:
        Action.setHeadParams(horizonPan, 0, 0, Action.HTAbs_h)
    
    #elif not faceDest:    
    #    relPos = hMath.rotate(possibleFeatures[useFeature], 90 - selfHead)
    #    targetX = -relPos[0]
    #    targetY = relPos[1]
        
    # Otherwise look at an edge feature
    else:
        useFeature = localiseCounter / WATCH_FEATURE
        relPos = hMath.rotate(possibleFeatures[useFeature], 90 - selfHead)
        targetX = -relPos[0]
        targetY = relPos[1]

        Action.setHeadParams(targetX, 0, targetY, Action.HTAbs_xyz)

#-------------------------------------------------------------------------------
# Calculates the gps coordinates that the dog should move to

def CalcMovePoint():
    global points, pointSequence, moveToPoint, movePoint
    global facingPoint, faceDest        

    point = points[pointSequence[moveToPoint]]
    facingVec = (facingPoint[0] - point[0], facingPoint[1] - point[1])
    facingVec = hMath.normalise(facingVec)
    text = "facingPoint:" + `facingPoint`
    debugging(text)
    #print "Before facingVec:",facingVec," faceDest:",faceDest
    if faceDest:
        # If far away from destination point, move to a point behind the
        # actual destination, so we when we get near, we can move forward
        # towards our selected line feature, and localise better.
        facingVec = (5 * facingVec[0], 5 * facingVec[1])
        #facingVec = (-5 * facingVec[0], -5 * facingVec[1])
    else:
        # If we are close, move to the point that would position the centre
        # of the dogs body over the destination.
        facingVec = (10 * facingVec[0], 10 * facingVec[1])

    moveTo = [point[0] + facingVec[0], point[1] + facingVec[1]]
    #moveTo = [point[0], point[1]]
    #movePoint = tuple(moveTo)
    movePoint = ClipToField(moveTo, 15)
    text = "movePoint:" + `movePoint`
    debugging(text)
        
#-------------------------------------------------------------------------------
# Returns true if the dog is close to the destination point, and has small
# enough self variance

def AtDestPoint():
    global distanceToPoint, movePoint

    pos = Global.selfLoc.getPos()
    posVar = Global.selfLoc.getPosVar()
    distanceToPoint = hMath.getLength(map(lambda m,p:m-p, movePoint, pos))
    if distanceToPoint < POS_ACCURACY:
        text = "distanceToPoint < POS_ACCURACY:" + `distanceToPoint`
        debugging(text)
    if distanceToPoint < POS_ACCURACY and posVar < POS_MAX_VARIANCE:
        text = "posVar < POS_MAX_VARIANCE:" + `posVar`
        debugging(text)
    return (distanceToPoint < POS_ACCURACY and posVar < POS_MAX_VARIANCE)

#-------------------------------------------------------------------------------
# Makes the dog move to the given point and heading

def MoveToPoint():
    global distanceToPoint, movePoint, facingPoint, turning, faceDest,standStillCounter,orientateCounter,moving
    if distanceToPoint < 5:
        if faceDest:
            print "Stopped sprinting, faceDest = False"
            
        faceDest = False
    elif distanceToPoint > 25: 
        faceDest = True
    selfLoc = Global.selfLoc.getPos()
    selfHead = Global.selfLoc.getHeading()

    # Choose a heading vector.
    if faceDest:
        text = "faceDest ! When far from location, head straight to it"
        debugging(text)    
        # When far from location, head straight to it
        dX, dY = (movePoint[0] - selfLoc[0], movePoint[1] - selfLoc[1])
    else:
        text = "!faceDest Face line feature (for localising) when close to destination"
        debugging(text)    
        # Face line feature (for localising) when close to destination
        dX, dY = (facingPoint[0] - selfLoc[0], facingPoint[1] - selfLoc[1])

    # Calculate the angle between our self angle and the heading vector (dH).
    if dX == 0 and dY == 0:
        head = selfHead
    else:
        head = hMath.RAD2DEG(math.atan2(dY, dX))
        head = hMath.normalizeAngle_0_360(head)

    dH = hMath.normalizeAngle_180(head - selfHead)
    text = "dh:" + `dH`
    debugging(text)    
    # If far from destination, we want to sprint straight towards destination.
    if faceDest:
        if abs(dH) < 5:
            # If we are facing in the right direction, move forward
            turning = False
        if abs(dH) > 30:
            # If we are facing in the wrong direction, only turn
            turning = True
            
        if not turning:
            # When facing correct direction, sprint using elliptical walk
            #hTrack.saGoToTarget(movePoint[0], movePoint[1], 8, 5, Action.EllipticalWalk)
            
            #hTrack.saGoToTarget(movePoint[0], movePoint[1])
            if WALKTYPE == Action.SkeFastForwardMWT:
                hTrack.saGoToTarget(movePoint[0], movePoint[1])
            else:
                saGoToTarget(movePoint[0], movePoint[1])
                
            standStillCounter = 0
            #speedLim = 8 #1 + distanceToPoint / 5
            #turnLim = 5 #+ abs(dH) / 6            
            #hTrack.saGoToTargetFacingHeading(movePoint[0], movePoint[1],
            #                                   dH, speedLim, turnLim)
        else:
            text = "Turn first faceDest"
            debugging(text)    
            #Action.walk(0, 0, hMath.CLIP(dH * 1.5, 40))
            if WALKTYPE == Action.SkeFastForwardMWT:
                Action.walk(0, 0, dH, minorWalkType=WALKTYPE)
            else:
                Action.walk(0, 0, dH, walkType=WALKTYPE)
            standStillCounter = 0

    # If close to destination point, we want to face the pre-determined line
    # feature.
    else:
        if abs(dH) < 5:
            turning = False
            
        if abs(dH) > 30:
            turning = True

        if turning:
            text = "Turn turning"
            debugging(text)    
            #Action.walk(0, 0, hMath.CLIP(dH * 1.5, 40))
            if WALKTYPE == Action.SkeFastForwardMWT:
                Action.walk(0, 0, dH, "ddd", minorWalkType=WALKTYPE)
            else:
                Action.walk(0, 0, dH, "ddd", walkType=WALKTYPE)
            standStillCounter = 0
        elif distanceToPoint > POS_ACCURACY:
            # Reduce speeds as we approach the destination point
            speedLim = 1 + distanceToPoint / 5
            turnLim = 5 + abs(dH) / 6
            text = "distanceToPoint > POS_ACCURACY distopoint:" + `distanceToPoint`+ " slimit:" + `speedLim` + " turnlimit:" + `turnLim` + " head:" + `head`
            debugging(text)    
            print text
            #Action.walk(0, 0, 0)#redundant in the below function
            
            if WALKTYPE == Action.SkeFastForwardMWT:
                hTrack.saGoToTargetFacingHeading(movePoint[0], movePoint[1],head, None, turnLim)
            else:
                saGoToTargetFacingHeading(movePoint[0], movePoint[1],head, speedLim, turnLim)
                #saGoToTargetFacingHeading(movePoint[0], movePoint[1],head)
            standStillCounter = 0                                               
        else:
            print "standstill"
            if WALKTYPE == Action.SkeFastForwardMWT:
                Action.walk(0, 0, 0,minorWalkType=WALKTYPE)
            else:    
                Action.walk(0, 0, 0,walkType=WALKTYPE)
            #hTrack.stationaryLocalise()
            if standStillCounter > POS_TIME*3 :
                #orientateCounter = 0
                moving = False #wag the tail
                standStillCounter = 0
            else:
                standStillCounter +=1                
            #HelpLong.setDefaultAction()

#-------------------------------------------------------------------------------
# Makes the dog wag its tail

def WagTail():
    global wagTailCounter
    
    wagTailCounter = wagTailCounter + 1

    if (wagTailCounter % 20) < 10:
        tailPos = Indicator.TAIL_RIGHT / 2
    else:
        tailPos = Indicator.TAIL_LEFT / 2

    Indicator.finalValues[Indicator.TailH] = tailPos
    Indicator.finalValues[Indicator.TailV] = Indicator.TAIL_DOWN

#-------------------------------------------------------------------------------
# Switches to the next point in the sequence

def SwitchToNextPoint():
    global paused, moving, wagTailCounter, positionedCounter
    global moveToPoint, points, pointSequence, turning, faceDest
    
    if not DO_PAUSE_EVERY_WAG_TAIL:
        paused = False
    else:
        paused = True
        #need to add few frame before turn off everything
        VisionLink.setIfGPSMotionUpdate(False)
        VisionLink.setIfGPSVisionUpdate(False)
                    
    moving = True
    wagTailCounter = 0
    positionedCounter = 0

    # Switch to next point, cycle to first if necessary
    turning = True
    faceDest = True
    moveToPoint = moveToPoint + 1
    if moveToPoint >= len(points):
        moveToPoint = 0

    ChooseFacingPoint()
    
    print "Now moving to point ", points[pointSequence[moveToPoint]]
    
#-------------------------------------------------------------------------------
# Finds the shortest path between the five points to go to

def FindShortestPath():
    global points, pointSequence, pathSelected

    # Calculate distance look-up table
    num = len(points)
    dist = []
    allPoints = points + [Global.selfLoc.getPos()]
    aPRange = range(len(allPoints))

    #set all the edge distances
    for a in aPRange:
        dist.append([])
    
        for b in aPRange:
            delta = map(lambda x,y:x-y, allPoints[a], allPoints[b])
            dist[a].append(hMath.getLength(delta))
                           
    # Use backtracking to find shortest path between all points    
    (pointSequence, l) = GetShortestPath(num, range(num), dist, 1000 * num)

    ChooseFacingPoint()

    print "Points: ", points
    print "Best sequence: ", pointSequence
    
    pathSelected = True

#-------------------------------------------------------------------------------
# Recursively search all possibile point sequences for shortest path. If there
# were lots of points to visit, this wouldn't be used because it's too slow.
# But seeing there are only 5 points, we can afford to look for the optimal
# solution.

def GetShortestPath(prev, available, dist, max):
    l = 0
    best = max
    path = []
    check = []

    for choice in available:
        d = dist[prev][choice] 

        if d < best:
            newAvailable = available[:]
            newAvailable.remove(choice)
            (check, l) = GetShortestPath(choice, newAvailable, dist, best - d)
            l = d + l
            check = [choice] + check

            if len(check) == len(available):
                best = l
                path = check[:]

    if path == []:
        return ([], 0)
    else:
        return (path, best)
                
#-------------------------------------------------------------------------------
# Choose closest line feature to face towards (when dog gets close to
# destination).

def ChooseFacingPoint():
    global moveToPoint, facingPoint, points, pointSequence

    closestDist = Constant.FIELD_DIAGONAL
    for edgeLoc in FACING_POSITIONS:
        # Calculate distance from destination point to line feature
        p = points[pointSequence[moveToPoint]]
        # Trim the edge loc that can not be seen from inside the goal
        if (Constant.LEFT_GOAL_POST < p[0] < Constant.RIGHT_GOAL_POST) :
            if (p[1] < 10):
                 if (edgeLoc[0] == Constant.MIN_GOALBOX_EDGE_X or edgeLoc[0] == Constant.MAX_GOALBOX_EDGE_X and edgeLoc[1] == 0) : 
                    continue
            elif (p[1] > Constant.TARGET_GOAL_Y - 10):
                 if (edgeLoc[0] == Constant.MIN_GOALBOX_EDGE_X or edgeLoc[0] == Constant.MAX_GOALBOX_EDGE_X and edgeLoc[1] == Constant.FIELD_LENGTH) : 
                    continue
        
        delta = map(lambda e,p:e-p, edgeLoc, p)
        dist = hMath.getLength(delta)
        
        # We want to pick the line feature that is closest to the destination,
        # but not so close that it will be under the dog when it gets there
        if dist > 15 and dist < closestDist:
            closestDist = dist
            facingPoint = edgeLoc
    text = "facingPoint:" + `facingPoint`
    debugging(text)    
    
#-------------------------------------------------------------------------------
# Selects the leftmost beacon to localise off, so that when we turn left we can
# continually track it.

def ChooseLeftmostBeacon():
    selfPos = Global.selfLoc.getPos()
    selfHead = Global.selfLoc.getHeading() 
    beaconPos = VisionLink.getGPSCoordArray()
    curCompHead = -100
    sActiveLocalise.localiseBeacon = 0    
    
    for beacon in beaconPos:
        relPos = map(lambda b,s:b-s, beacon, selfPos)
        if relPos[0] == 0 and relPos[1] == 0:
            beaconHead = 0
        else:
            beaconHead = hMath.RAD2DEG(math.atan2(relPos[1], relPos[0]))
        localHead  = hMath.normalizeAngle_180(beaconHead - selfHead)
    
        if abs(localHead) < 90 and localHead > curCompHead:
            curCompHead  = localHead
            sActiveLocalise.localiseBeacon = beacon


#-------------------------------------------------------------------------------
# Clips the given point to within the field by a buffer amount given by 'space'

def ClipToField(clipPoint, space):
    point = list(clipPoint)

    # Work in one quadrant, so there aren't as many checks
    flipX = (point[0] > Constant.FIELD_WIDTH / 2)
    flipY = (point[1] > Constant.FIELD_LENGTH / 2)
    
    if flipX:
        point[0] = Constant.FIELD_WIDTH - point[0]
    if flipY:
        point[1] = Constant.FIELD_LENGTH - point[1]
    
    
    if point[0] <= Constant.LEFT_GOAL_POST:
        #if it on the side of the goal clip the Y to inside the field
        #clip to inside the field
        point[1] = max(point[1], space)

    else:
        # Clip to the rear of the goal.
        point[1] = max(point[1], space - Constant.GOAL_LENGTH)
        
        if point[1] > 0:
            # Clip to corners of goal mouth.
            deltaGoalCorner = [0, 0]
            deltaGoalCorner[0] = point[0] - Constant.LEFT_GOAL_POST
            deltaGoalCorner[1] = point[1]
            dist = hMath.getLength(deltaGoalCorner)

            if dist < space:
                scaler = (space - dist) / dist
                point[0] = point[0] + deltaGoalCorner[0] * scaler
                point[1] = point[1] + deltaGoalCorner[1] * scaler
                
        else:
            # Clip to goal sides.
            point[0] = max(point[0], space + Constant.LEFT_GOAL_POST)
        
    # Clip to the inside the field
    if point[1] > 0:
        point[0] = max(point[0], space)

    # Make sure we don't move into overlapping buffer regions in
    # different quadrants.
    point[0] = min(point[0], Constant.FIELD_WIDTH / 2)
    point[1] = min(point[1], Constant.FIELD_LENGTH / 2)

    # Flip back to original quadrant
    if flipX:
        point[0] = Constant.FIELD_WIDTH - point[0]
    if flipY:
        point[1] = Constant.FIELD_LENGTH - point[1]
    #print "points in :", clipPoint, "out :", point
    return tuple(point)

    
#--------------------------------------
# Move the point under the dog's neck to a location on the field
# Note that turnCCW is relative to the centre of the dog's body, not under its
# neck
def saGoToTargetFacingHeading(targetX, targetY, targetH, \
        maxSpeed = None, maxTurn = None):
    
    if maxSpeed == None:             
        maxSpeed = Action.MAX_FORWARD
    if maxTurn == None:
        maxTurn = Action.MAX_TURN

    # Variables relative to self localisation.
    selfPos  = Global.selfLoc.getPos()
    selfH    = Global.selfLoc.getHeading()
    relX     = targetX - selfPos[0]
    relY     = targetY - selfPos[1]
    relH     = hMath.normalizeAngle_180(targetH - selfH)
    # now take into account that as we turn, the position of our head will change
    relX  += Constant.DOG_NECK_TO_BODY_CENTER_OFFSET * \
             (math.cos(math.radians(selfH)) - math.cos(math.radians(targetH)))
    relY  += Constant.DOG_NECK_TO_BODY_CENTER_OFFSET * \
             (math.sin(math.radians(selfH)) - math.sin(math.radians(targetH)))
    relD  = hMath.getLength((relX, relY))
    
    if relX == 0 and relY == 0:
        relTheta = 0
    else:
        relTheta = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY, relX)) - selfH)
    

    forward = hMath.CLIP(relD, maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
    left    = hMath.CLIP(relD, maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
    turnccw = hMath.CLIP(relH, maxTurn)    

    Action.walk(forward, left, turnccw,"ddd", walkType=WALKTYPE)
    print "saGoToTargetFacingHeading ", forward, " ", left, " ", turnccw


#--------------------------------------
# Walks to the specified point, turning to face the direction of walk as it
# goes. Appropriate walkParams will be chosen if not specified
def OptimisedGoToTarget(targetX, targetY, maxSpeed = None, maxTurn = None):
    #print "OptimisedGoToTarget"
    if maxSpeed == None:             
        maxSpeed = Action.MAX_FORWARD
    if maxTurn == None:
        maxTurn = Action.MAX_TURN
                                                   
    selfPos = Global.selfLoc.getPos()
    selfH   = Global.selfLoc.getHeading()
    relX    = targetX - selfPos[0]
    relY    = targetY - selfPos[1]
    relD    = hMath.getLength((relX, relY))
    
    if relX == 0 and relY == 0:
        relH = 0
    else:
        relH = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY,relX))-selfH)
    
    if relD < hTrack.CLOSE_WALK_RANGE and abs(relH) > 50:
        left = relX
        turnccw = relH * 0.8
#        print "close walk and turn",relY,left,turnccw
        Action.walk(relY,left,turnccw, "ssd", minorWalkType=Action.SkeFastForwardMWT)
    else:
        if abs(relH) > 80:      # stop and turn
            turnccw = relH * 0.8
            Action.walk(0, 0, turnccw, walkType=WALKTYPE)
        elif abs(relH) > 25: # walk and turn
            turnccw = relH * 0.8
            forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relH))
#            print "walk and turn",forward, 0, turnccw
            Action.walk(forward, 0, turnccw,"ssd", minorWalkType=Action.SkeFastForwardMWT)  
        else:                   # ellipticalwalk and turn            
            turnccw = hMath.CLIP(relH * 0.8, 25)
            forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relH))
#            print "walk type and turn",forward, 0, turnccw            
            Action.walk(forward, 0, turnccw,"ssd", minorWalkType=Action.SkeFastForwardMWT)
    
#--------------------------------------
# Walks to the specified point, turning to face the direction of walk as it
# goes. Appropriate walkParams will be chosen if not specified
def saGoToTarget(targetX, targetY, maxSpeed = None, maxTurn = None):
    print "saGotoTarget"
    if maxSpeed == None:             
        maxSpeed = Action.MAX_FORWARD
    if maxTurn == None:
        maxTurn = Action.MAX_TURN
                                                   
    selfPos = Global.selfLoc.getPos()
    selfH   = Global.selfLoc.getHeading()
    relX    = targetX - selfPos[0]
    relY    = targetY - selfPos[1]
    relD    = hMath.getLength((relX, relY))
    
    forward = 0
    left = 0
    turnccw = 0
    
    if relX == 0 and relY == 0:
        relH = 0
    else:
        relH = hMath.normalizeAngle_180(hMath.RAD2DEG(math.atan2(relY,relX))-selfH)
    
    if relD < hTrack.CLOSE_WALK_RANGE and abs(relH) > 50:
        left = relX
        turnccw = relH * 0.8
#        print "close walk and turn",relY,left,turnccw
        Action.walk(relY,left,turnccw, "ddd", walkType=WALKTYPE)
    else:
        if abs(relH) > 80:      # stop and turn
            turnccw = relH * 0.8
            Action.walk(0, 0, turnccw, walkType=WALKTYPE)
        elif abs(relH) > 25: # walk and turn
            turnccw = relH * 0.8
            forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relH))
#            print "walk and turn",forward, 0, turnccw
            Action.walk(forward, 0, turnccw,"ddd", walkType=WALKTYPE)  
        else:                   # ellipticalwalk and turn            
            turnccw = hMath.CLIP(relH * 0.8, 25)
            forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relH))
#            print "walk type and turn",forward, 0, turnccw            
            Action.walk(forward, 0, turnccw,"ddd", walkType=WALKTYPE)
    print "saGoToTargetFacingHeading ", forward, " ", left, " ", turnccw
#     forward = hMath.CLIP(relD,maxSpeed) * math.cos(hMath.DEG2RAD(relTheta))
#     left    = hMath.CLIP(relD,maxSpeed) * math.sin(hMath.DEG2RAD(relTheta))
#     turnCCW = hMath.CLIP(relTheta, maxTurn)
#    Action.walk(forward, left, turnCCW, walkParams)

def debugging(text):
    global debuggingCounter
    if (not PRINTDEBUG):
        return
    perframe = 15
    if debuggingCounter % perframe == 0:
        debuggingCounter = 0
    elif debuggingCounter % perframe == perframe - 1:
        print text
        
