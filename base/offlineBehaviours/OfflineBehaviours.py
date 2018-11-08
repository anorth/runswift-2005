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


#!/usr/bin/python
from qtcanvas import *
from frmOfflineBehaviours import *

import math
import random
import os
import imp
import thread
import threading
import time
import sys

sys.path.append(os.getcwd()+"/PyCode")

import VisionLink
import Constant
import Global
import Action
import hMath
import hFrameReset
import sSelKick

NUM_OBSTACLES = 10

#Z values for field objects, higher Z's
#are drawn on top of lower Z's
gBallZ = 40
gObsZ = 30
gRobotZ = 20
gGoalsZ = 10
gBeaconsZ = 10


def field2CanvasX(num):
    return num + Constant.SIDELINE_FRINGE_WIDTH

def field2CanvasY(num):
    return -num + Constant.FIELD_LENGTH + Constant.GOAL_FRINGE_WIDTH

def Canvas2FieldX(num):
    return num - Constant.SIDELINE_FRINGE_WIDTH

def Canvas2FieldY(num):
    return -(num - Constant.FIELD_LENGTH - Constant.GOAL_FRINGE_WIDTH) 


class Ball(QCanvasEllipse):
    global gBallZ
    
    def __init__(self, x, y, *args):
        self.x = x
        self.y = y
        self.z = gBallZ
        self.viewClass = QCanvasEllipse
        self.viewClass.__init__(self, *args)

    def x(self):
        return self.x

    def y(self):
        return self.y

    def setX(self, num):
        self.x = num
        self.viewClass.setX(self, field2CanvasX(num))

    def setY(self, num):
        self.y = num
        self.viewClass.setY(self, field2CanvasY(num))

    def move(self, x, y):
        (self.x, self.y) = (x, y)
        self.viewClass.move(self, field2CanvasX(x), field2CanvasY(y))       

class Obstacle(QCanvasRectangle):
    global gObsZ
    
    def __init__(self, x, y, *args):
        self.x = x
        self.y = y
        self.z = gObsZ
        self.viewClass = QCanvasEllipse
        self.viewClass.__init__(self, *args)
        
    def x(self):
        return self.x

    def y(self):
        return self.y

    def setX(self, num):
        self.x = num
        self.viewClass.setX(self, field2CanvasX(num-5))

    def setY(self, num):
        self.y = num
        self.viewClass.setY(self, field2CanvasY(num+5))

    def move(self, x, y):
        (self.x, self.y) = (x, y)
        self.viewClass.move(self, field2CanvasX(x-5), field2CanvasY(y+5))       
    
class Robot(QCanvasPolygon):
    global gRobotZ
    
    def __init__(self, x, y, h, *args):
        (self.x, self.y, self.h) = (x, y, h)
        self.z = gRobotZ
        self.viewClass = QCanvasPolygon
        self.viewClass.__init__(self, *args)
        points = QPointArray(3)
        points.setPoint(0, 0, 0)

        # Calculate orientation of the triange to draw
        head = (self.h - 90) * math.pi / 180.0
        pt = (30 * math.sin(head + math.pi/8), 30 * math.cos(head + math.pi/8))
        points.setPoint(1, pt[0], pt[1])
        pt = (30 * math.sin(head - math.pi/8), 30 * math.cos(head - math.pi/8))
        points.setPoint(2, pt[0], pt[1])
        self.viewClass.setPoints(self, points)
        self.viewClass.setX(self, field2CanvasX(self.x))
        self.viewClass.setY(self, field2CanvasY(self.y))

    def x(self):
        return self.x

    def y(self):
        return self.y

    def setX(self, num):
        self.x = num
        self.viewClass.setX(self, field2CanvasX(num))

    def setY(self, num):
        self.y = num
        self.viewClass.setY(self, field2CanvasY(num))
    
    def move(self, x, y):
        (self.x, self.y) = (x, y)
        self.viewClass.move(self, field2CanvasX(x), field2CanvasY(y))      

    def setHeading(self, heading):
        self.h = heading

        # Calculate orientation of the triange to draw
        points = self.viewClass.points(self)
        head = (self.h - 90) * math.pi / 180.0
        pt = (30 * math.sin(head + math.pi/8), 30 * math.cos(head + math.pi/8))
        points.setPoint(1, pt[0], pt[1])
        pt = (30 * math.sin(head - math.pi/8), 30 * math.cos(head - math.pi/8))
        points.setPoint(2, pt[0], pt[1])
        self.viewClass.setPoints(self, points)

    def heading(self):
        return self.h

#    def setPlayer(self, player):
#        self.player = player
    
#    def runPlayer():
#        pass

class OfflineBehaviours(frmOfflineBehaviours):

    # Members:
    # history :: [(x, y, h)] - a history of positions for player
    # player :: Robot - the robot object
    # obstacles :: [Obstacle] - obstacles
    ## teammates :: [Robot] - teammates (not intelligent)
    ## opponents :: [Robot] - opponents (not intelligent)

    ### //history :: {robot => [(x, y, h)]} - a history of positions for player

    def __init__(self,app = None, parent = None,name = None,modal = 0,fl = 0):
        frmOfflineBehaviours.__init__(self,parent,name,modal,fl)
        
        #seed random number generation
        random.seed()
        
        self.app = app
        self.p_thread = None
        self.killThread = False

        self.initFieldCanvas()
        self.initFieldCanvasView()
        self.initFieldViewObjects()        
        self.initGameViewObjects()
        
        # change the slide bar values
        obj = self.objectList[self.cbxObjects.currentItem()]
        self.sldPosX.setValue(obj.x)
        self.sldPosY.setValue(obj.y)

        # Load the behaviour names into the combobox
        behaviours = os.listdir("PyCode")
        for b in behaviours:
            if b[-3:] == ".py" and (b[0] == "p" or b[0] == "r" or b[0] == "t"):
                self.comboPlayer.insertItem(b)
        self.slotReloadPlayer()


    def initFieldCanvas(self):
        self.objectList = []   # mobile object Views ie. ball & robots        
        self.fieldCanvas = QCanvas(400, 600)
        self.fieldCanvas.setBackgroundColor(Qt.green)

    
    def initFieldCanvasView(self):    
        self.fieldCanvasView = FieldCanvasView(self, self.fieldCanvas, self.frameField)
        self.fieldWM = QWMatrix()
        self.fieldCanvasView.setWorldMatrix(self.fieldWM)
        self.zoomRatio = 1.0
        layoutFrameField = QGridLayout(self.frameField)
        layoutFrameField.addWidget(self.fieldCanvasView,0,0) #, Qt.AlignCenter)
                
        self.fieldCanvasView.show()


    def initFieldViewObjects(self):
        global gBeaconsZ, gGoalsZ, gObsZ
        
        self.landmarkList = [] # to hold references to the landmark objects
        ## -------------- GOALS -------------------
        # upper blue goal
        goal = QCanvasRectangle( Constant.SIDELINE_FRINGE_WIDTH+Constant.LEFT_GOAL_POST, 0,
                                 Constant.GOAL_WIDTH, Constant.GOAL_LENGTH,
                                 self.fieldCanvas)
        goal.setBrush(QBrush(Qt.blue))
        goal.setZ(gGoalsZ)
        self.landmarkList.append(goal)
        # lower yellow goal
        goal = QCanvasRectangle( Constant.SIDELINE_FRINGE_WIDTH + Constant.LEFT_GOAL_POST,
                                 (Constant.FIELD_LENGTH + 2 * Constant.GOAL_FRINGE_WIDTH) - Constant.GOAL_FRINGE_WIDTH-1,
                                 Constant.GOAL_WIDTH, Constant.GOAL_LENGTH+1,
                                 self.fieldCanvas)
        goal.setBrush(QBrush(Qt.yellow))
        goal.setZ(gGoalsZ)
        self.landmarkList.append(goal)

        ## -------------- FIELD LINES -----------------------
        boxes = []
        # upper half field
        boxes.append( (0, 0,
                       Constant.FIELD_WIDTH, Constant.FIELD_LENGTH/2) # x, y, width, height
                      )
        # upper goal box
        boxes.append( (Constant.MIN_GOALBOX_EDGE_X, 0,
                       Constant.GOALBOX_WIDTH, Constant.GOALBOX_DEPTH)
                      )
        # lower half field
        boxes.append( (0, Constant.FIELD_LENGTH/2,
                       Constant.FIELD_WIDTH, Constant.FIELD_LENGTH/2)
                      )
        # lower goal box
        boxes.append( (Constant.MIN_GOALBOX_EDGE_X,
                       Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH,
                       Constant.GOALBOX_WIDTH, Constant.GOALBOX_DEPTH)
                      )
        
        for box in boxes:
            obj = QCanvasRectangle(self.fieldCanvas)
            obj.setX(box[0] + Constant.SIDELINE_FRINGE_WIDTH)
            obj.setY(box[1] + Constant.GOAL_FRINGE_WIDTH)
            obj.setSize(box[2], box[3])
            obj.setPen(QPen(Qt.white, 2)) #QPen(color, width)
            self.landmarkList.append(obj)

        ## -------------- BEACONS -----------------------
        # !!!!!!!!!!!!!not implemented yet!!!!!!!!!!!!!!
        pinks = []
        whites = []
        blues = []
        yellows = []

        # -------------- DISPLAY ALL OBJECTS --------------
        for obj in self.landmarkList:
            obj.show()

        

    def initGameViewObjects(self):
        global gRobotZ, gBallZ, gObsZ

        # -------------- PLAYER (RED) ---------------
        self.robot = Robot(150, 200, 30, self.fieldCanvas)
        self.robot.setBrush(QBrush(Qt.red))
        self.robot.setZ(gRobotZ)
        self.objectList.append(self.robot)

        # --------------- BALL ----------------------
        self.ballView = Ball(0, 0, Constant.BallRadius*2, Constant.BallRadius*2,
                                 self.fieldCanvas)
        self.ballView.move(Constant.FIELD_WIDTH/2, Constant.FIELD_LENGTH/2)
        self.ballView.setBrush(QBrush(Qt.magenta))
        self.ballView.setZ(gBallZ)
        self.objectList.append(self.ballView)

        # --------------- OBSTACLES -----------------
        self.obstacles = []
        brush = QBrush(Qt.darkRed)
        for i in xrange(NUM_OBSTACLES):
            obs = Obstacle(0, 0, 0, 0, 10, 10, self.fieldCanvas)
            obs.move(10*len(self.obstacles), -15)
            obs.setBrush(brush)
            obs.setZ(gObsZ)
            self.obstacles.append(obs)

        self.objectList.extend(self.obstacles)

        # -------------- TEAMMATES (RED) ------------
#        self.teammates = []
#        brush = QBrush(Qt.darkRed)
#        for i in xrange(Constant.NUM_TEAM_MEMBER - 1):
#            robot = Robot(0, 0, 90, self.fieldCanvas)
#            if i == 0: # goalie
#                robot.move(Constant.FIELD_WIDTH/2, Constant.GOALBOX_DEPTH/2)
#                goalieBrush = QBrush(Qt.red)
#                goalieBrush.setStyle(Qt.Dense2Pattern)                
#                robot.setBrush(goalieBrush)
#            else:
#                robot.move(Constant.FIELD_WIDTH/3*(i), Constant.FIELD_LENGTH/3)
#                robot.setBrush(brush)
#            self.teammates.append(robot)
#
#        self.objectList.extend(self.teammates)
#                

        # ------------- OPPONENTS (BLUE) ------------
#        self.opponents = []
#        blueDogColor = QColor(0, 0, 32)
#        for i in xrange(Constant.NUM_TEAM_MEMBER):
#            robot = Robot(0, 0, -90, self.fieldCanvas)
#            if i == 0: # goalie
#                robot.move(Constant.FIELD_WIDTH/2, \
#                            Constant.FIELD_LENGTH - Constant.GOALBOX_DEPTH/2)
#                goalieBrush = QBrush(blueDogColor)
#                goalieBrush.setStyle(Qt.Dense2Pattern)                
#                robot.setBrush(goalieBrush)
#            else:
#                robot.move(i*Constant.FIELD_WIDTH/4, Constant.FIELD_LENGTH/4*3) 
#                robot.setBrush(QBrush(blueDogColor))                 
#            self.opponents.append(robot)
#
#        self.objectList.extend(self.opponents)        
       
        # show all the objects
        for obj in self.objectList:
            obj.show()
        for obj in self.obstacles:
            obj.show()


    ##### Slots #####

    def slotSetPosX(self, num):
        self.objectList[self.cbxObjects.currentItem()].setX(num)
        self.fieldCanvas.update()
        
    def slotSetPosY(self, num):
        self.objectList[self.cbxObjects.currentItem()].setY(num)
        self.fieldCanvas.update()        

    def slotObjectSelected(self, idx):
        obj = self.objectList[idx]
        self.sldPosX.setValue(obj.x)
        self.sldPosY.setValue(obj.y)

#    def slotSetNumObstacles(self, numobs):
#        print "slotSetNumObstacles", numobs
#        for obj in self.obstacles:
#            obj.hide()
#        if numobs < len(self.obstacles):
#            self.obstacles[numobs:] = []
#        else:
#            while numobs > len(self.obstacles):
#                obs = Obstacle(0, 0, 0, 0, 10, 10, self.fieldCanvas)
#                obs.move(10*len(self.obstacles), Constant.FIELD_LENGTH/2)
#                obs.setBrush(QBrush(Qt.darkRed))
#                self.obstacles.append(obs)
#                # FIXME: no way to remove these?
#                self.objectList.append(obs)
#        print "obstacles", self.obstacles
#        for obj in self.obstacles:
#            print "showing", obj
#            obj.show()
#        self.fieldCanvas.update()

    def slotCheckShowMovement(self, isChecked):
        if self.isActionShown:
            if isChecked:
                for obj in self.movementList:
                    obj.show()
            else:
                for obj in self.movementList:
                    obj.hide() 
            self.fieldCanvas.update()
     

    def slotReloadPlayer(self):
        print "slotReloadPlayer",
        #if self.p_thread != None:
        #    self.p_thread.exit()
        #    self.p_thread = None

        playerName = self.comboPlayer.currentText().ascii()[:-3]
        print playerName, "...",
        filename = "PyCode/" + playerName + ".py"
        file = open(filename)
        self.player = imp.load_source(playerName, filename, file)
        sys.modules[playerName] = self.player
        file.close()
        reload(self.player)
        print "done."
        #self.hideMyActions()
        
    def slotZoomIn(self):
        self.fieldWM.scale(2,2)
        self.zoomRatio *= 2.0
        self.lblZoomRatio.setText(str(self.zoomRatio) + "x")
        self.fieldCanvasView.setWorldMatrix(self.fieldWM)

    def slotZoomOut(self):
        self.fieldWM.scale(0.5, 0.5)
        self.zoomRatio /= 2.0
        self.lblZoomRatio.setText(str(self.zoomRatio) + "x")
        self.fieldCanvasView.setWorldMatrix(self.fieldWM)

    def slotZoomReset(self):
        self.fieldWM.scale(1/self.zoomRatio, 1/self.zoomRatio)        
        self.zoomRatio = 1.0
        self.lblZoomRatio.setText(str(self.zoomRatio) + "x")
        self.fieldCanvasView.setWorldMatrix(self.fieldWM)
             
    def slotRealTime(self):
        self.sldTimeWarp.setValue(0)  
     
    def slotRandomiseObs(self):
        for obj in self.obstacles:
            obj.setX(random.randint(-20,380))
            obj.setY(random.randint(-30,570))
          
        # change the slide bar values
        obj = self.objectList[self.cbxObjects.currentItem()]
        self.sldPosX.setValue(obj.x)
        self.sldPosY.setValue(obj.y)
        
        #update canvas  
        self.app.lock()
        self.fieldCanvas.update()
        self.app.unlock()    
    
    # Run the python command in the interpreter input field. Output goes
    # to the output field
    def slotInterpret(self):
        cmd = self.txtInterpreterInput.text().ascii()
        self.txtInterpreterInput.selectAll()
        if cmd[-1] == ';':
            cmd = compile(cmd, "errors.out", "single")
            val = eval(cmd);
        else:
            val = eval(cmd);
            self.txtInterpreterOutput.append(str(val))

    # Runs the player for one frame
    def slotStepPlayer(self, player = None):
        if player == None:
            player = self.player
        
        # Set the robot and ball positions
        VisionLink.me = Global.WMObj(self.robot.x, self.robot.y,\
                                    0, self.robot.h, 0)
        VisionLink.ball = Global.WMObj(self.ballView.x,\
                                    self.ballView.y, 0, 0, 0)

        # FIXME: catch exceptions to reset killThread
        
        # Call the offline process frame function to set up frame info
        obsTuples = map ((lambda o : (o.x, o.y)), self.obstacles)
        VisionLink.clearObstacles()
        for obs in obsTuples:
            VisionLink.setObstacle(*obs)
        VisionLink.processFrame(self.robot.x, self.robot.y, self.robot.h,
                                self.ballView.x, self.ballView.y, 0, 0)

        # Run the behaviour for one frame We skip past Behaviou and Player
        # modules to allow us to switch players easily so here we mimic
        # Behaviou.processFrame
        hFrameReset.framePreset()
        hFrameReset.frameReset()
        hFrameReset.offlineReset()

        player.DecideNextAction()

        hFrameReset.framePostset()

        # Read the last action and update state
        forward = Action.finalValues[Action.Forward]
        left = Action.finalValues[Action.Left]
        turn = Action.finalValues[Action.TurnCCW]

        forward *= abs(float(Action.finalValues[Action.ForwardStep]) \
                    / Action.MAX_SKE_FWD_STP)
        left *= abs(float(Action.finalValues[Action.LeftStep]) \
                    / Action.MAX_SKE_LEFT_STP)
        turn *= abs(float(Action.finalValues[Action.TurnCCWStep]) \
                    / Action.MAX_SKE_TURNCCW_STP)

        sum = abs(forward) + abs(left)
        if (sum > 35):  # clip total to estimate real life
            forward = float(forward) / (sum) * 35
            left = float(left) / (sum) * 35

        print "mvmt:", int(forward), int(left), int(turn)
        self.txtMvmtSpeed.setText("%d, %d, %d" % \
                (Action.finalValues[Action.Forward], \
                Action.finalValues[Action.Left], \
                Action.finalValues[Action.TurnCCW]))
        self.txtMvmtStep.setText("%d, %d, %d" % \
                (Action.finalValues[Action.ForwardStep], \
                Action.finalValues[Action.LeftStep], \
                Action.finalValues[Action.TurnCCWStep]))
        self.txtWalkType.setText(str(Action.finalValues[Action.WalkType]))
        self.txtKick.setText(str(sSelKick.perform()[0]))

        forward /= 30.0
        left /= 30.0
        turn /= 30.0
        #print "clipped to:", "%.2f"%forward, "%.2f"%left, "%.2f"%turn

        heading = self.robot.h * math.pi / 180.0

        newy = self.robot.y + forward * math.sin(heading) \
            + left * math.cos(heading)
        newx = self.robot.x + forward * math.cos(heading) \
            - left * math.sin(heading)
        self.robot.move(newx, newy)

        #self.robot.y += forward * math.cos(heading - math.pi/2)
        #self.robot.y += left * math.sin(heading - math.pi/2)
        #self.robot.x += forward * math.sin(heading - math.pi/2)
        #self.robot.x += left * math.cos(heading - math.pi/2)
        self.robot.setHeading(hMath.normalizeAngle_180(self.robot.h + turn))

        self.app.lock()
        self.fieldCanvas.update()
        self.app.unlock()

    # Run player until further notice (spawns a thread)
    def slotRunPlayer(self):
        print "Running player"
        # Reload to clear state
        reload(self.player)

        if self.p_thread == None:
            # Start a new thread to run the behaviour
            self.p_thread = thread.start_new_thread(self.runPlayer, \
                                                    (self.player,))
            self.btnRunPlayer.setText("Stop")
        else:
            self.killThread = True
            time.sleep(0.5)
            self.p_thread = None
            self.btnRunPlayer.setText("Run")

    # Run a behaviour. This is expected to be called in a
    # thread separate from the main thread as it returns only when killThread
    # is set True. 
    def runPlayer(self, player):
        #print "runPlayer: selfLoc at", ("%x" % id(Global.selfLoc)),\
        #        "= ", Global.selfLoc
        while True:
            if self.killThread: # Quit
                self.killThread = False
                break

            if self.sldTimeWarp.value() == 29:
                time.sleep(1.0/200.0)
            else:
                time.sleep(1.0/(30.0 + self.sldTimeWarp.value()))
            self.slotStepPlayer(player)


    def drawMyMovement(self, idx):
        forward = Action.finalValues[Action.Forward]
        left = Action.finalValues[Action.Left]
        heading = VisionLink.me.getHeading() * math.pi / 180
        
        offsetX = math.cos(heading)*forward - math.sin(heading)*left
        offsetY = math.sin(heading)*forward + math.cos(heading)*left

        x = field2CanvasX(VisionLink.me.getX())
        y = field2CanvasY(VisionLink.me.getY())
        x2 = field2CanvasX(VisionLink.me.getX()+offsetX)
        y2 = field2CanvasY(VisionLink.me.getY()+offsetY)
        #print "x=", x, "y=", y, "x2=", x2, "y2=", y2
                
        obj = self.movementList[idx]
        obj.setPoints(x, y, x2, y2)
        obj.show()


class FieldCanvasView(QCanvasView):
    def __init__(self, parent, *args):
        self.parent = parent
        self.chosenObjIdx = -1
        QCanvasView.__init__(self, *args)

    
    # if mouse is pressed, find the closest object
    def mousePressEvent(self, event):
        minDist = 1000000 # large num
        minObjIdx = -1
        
        for i in xrange( len(self.parent.objectList) ):
            obj = self.parent.objectList[i]
            dist = math.sqrt(
                (field2CanvasX( obj.x ) - event.x()) ** 2 +
                (field2CanvasY( obj.y ) - event.y()) ** 2
                )
            if dist < 10 and dist < minDist:
                minDist = dist
                minObjIdx = i
                
        if minObjIdx != -1:
            print "Selected obj", minObjIdx
            self.parent.cbxObjects.setCurrentItem(minObjIdx)
            self.chosenObjIdx = minObjIdx



    # If mouse tracking is disabled (the default),
    # the widget only receives mouse move events
    def mouseMoveEvent(self, event):
        if self.chosenObjIdx != -1:
            self.parent.sldPosX.setValue(Canvas2FieldX(event.x()))
            self.parent.sldPosY.setValue(Canvas2FieldY(event.y()))


    def mouseReleaseEvent(self, event):
        self.chosenObjIdx = -1

##### Utility functions #####



if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))    
    w = OfflineBehaviours(a)
    a.setMainWidget(w)
    w.show()
    a.exec_loop()    
