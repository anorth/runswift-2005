#!/usr/bin/python

##  
##     Copyright 2004 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
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
##     must be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##  
##  
##  
##   Last modification background information
##   $Id$
##  
##   Copyright (c) 2004 UNSW
##   All Rights Reserved.
##  
##  



from FormJoystickController import FormJoystickController 
import sys
from qt import *
from qttable import *
import pygame
import math
JOYSTICK_INPUT = ["Button0",
    "Button1",
    "Button2",
    "Button3",
    "Button4",
    "Button5",
    "Button6",
    "Button7",
    "Button8",
    "Button9"
    ]
    
CONFIG_FILE = "joystick.conf"
mapJoyToPlayer = {0:1, 1:2}

##~ CALIBRATION = 63.998046875043258
##~ CALIBRATION = 1.0

EllipticalWalkWT = 4
NormalWalkWT = 0

WALK_COMMAND_SPACE = [ ( (0,0,0)  ,   (NormalWalkWT,0,0,0) ),
                      #forward,backward,left,right
                      ( (100,0,0)  ,   (EllipticalWalkWT,0,0,0) ),
                      ( (-100,0,0) ,   (NormalWalkWT,-6,0,0) ),
                      ( (0,100,0)  ,   (NormalWalkWT,0,3,0) ),
                      ( (0,-100,0)  ,   (NormalWalkWT,0,-3,0) ),
                      #diagonal moves
                      ( (100,100,0)  ,   (NormalWalkWT,3,3,0) ),
                      ( (-100,100,0)  ,   (NormalWalkWT,-3,3,0) ),
                      ( (100,-100,0)  ,   (NormalWalkWT,3,-3,0) ),
                      ( (-100,-100,0)  ,   (NormalWalkWT,-3,-3,0) ),
                      #walk fast and turn
                      ( (100,0,100)  ,   (EllipticalWalkWT,0,0,20) ),
                      ( (100,0,50)  ,   (EllipticalWalkWT,0,0,10) ),
                      ( (100,0,-100)  ,   (EllipticalWalkWT,0,0,-20) ),
                      ( (100,0,-50)  ,   (EllipticalWalkWT,0,0,-10) ),
                      ( (100, 100, 100)   ,   (EllipticalWalkWT,0,0, 20) ),
                      ( (100, -100, -100)  ,  (EllipticalWalkWT,0,0,-20) ),
                      ( (0,100,100)  ,     (EllipticalWalkWT,0,0,10) ),
                      ( (0,-100,-100)  ,   (EllipticalWalkWT,0,0,-10) ),
                      # turn
                      ( (0,0,100)  ,   (NormalWalkWT,0,0,20) ),
                      ( (0,0,50)  ,   (NormalWalkWT,0,0,10) ),
                      ( (0,0,-100)  ,   (NormalWalkWT,0,0,-20) ),
                      ( (0,0,-50)  ,   (NormalWalkWT,0,0,-10) ),
                      ( (-100, -100, -100),  (NormalWalkWT,0,0,-20) ),
                      ( (-100, 100, 100) ,  (NormalWalkWT,0,0,20) )
                      ]

def distance3D(p1,p2):
    (x1,y1,z1) = p1
    (x2,y2,z2) = p2
    return math.sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) )
    
class JoystickController(FormJoystickController):
    def __init__(self,parent = None,name = None,fl = 0):
        FormJoystickController.__init__(self,parent,name,fl)
        self.parent = parent
        self.initGUI()
        self.initJoystick()
    
    def processOneAction(self,actStr):
        arr = actStr.split("|")
        assert(len(arr) == 4)
        arr[2] = arr[2][0:20] #trim to 20 chars
        self.slotNewAction(arr[0],arr[1],arr[2],arr[3])
        
        
    def loadConfiguration(self):
##~         self.actionMap = {}
        f = open(CONFIG_FILE)
        line = f.readline()
        while line != "":
            self.processOneAction(line)
            line = f.readline()
            
    def initGUI(self):
        self.stringList = QStringList()
        for text in JOYSTICK_INPUT:
            self.stringList.append(text)
        
        self.loadConfiguration()
        
    def initJoystick(self):
        self.joystick = []
        for i in range(pygame.joystick.get_count()):
            joystick = pygame.joystick.Joystick(i)
            joystick.init()
            print "Joystick ",i," initialized."
            self.joystick.append(joystick)
                    
        self.pollingTimer = QTimer()
        self.connect(self.pollingTimer, SIGNAL("timeout()"), self.pollJoystick)
        self.bStarted = False
        print "JoystickController started"
        
        
    def slotStartJoystick(self):
        print "JoystickController::slotStartJoystick"
        self.bStarted = True
        queue = pygame.event.get() #throw away out-of-date events
        self.pollingTimer.start(100)
        
    def slotStopJoystick(self):
        print "JoystickController::slotStopJoystick"
        self.bStarted = False
        self.pollingTimer.stop()
        
    def slotStartStopJoystick(self):        
        if not self.bStarted:
            self.slotStartJoystick()
            self.btStart.setText("Stop Joystick")
        else:
            self.slotStopJoystick()
            self.btStart.setText("Start Joystick")
        
    def pollJoystick(self): 
        queue = pygame.event.get()
        print "Queue has ", len(queue), " events"
        self.processQueue(queue)
        
    def setSendingCommand(self,playerNo, commandStr):
        self.ledCommand.setText(commandStr)
        self.nextCommands[playerNo] = commandStr
        
    def sendCommand(self):        
        for playerNo in self.nextCommands.keys():
            commandStr = self.nextCommands[playerNo]
            print "Sent to dog : ",playerNo," command = ", commandStr
            self.parent.slotServerSend(commandStr)
        
    def getConfiguredCommand(self,jcommand):
        for i in range(self.tblConfig.numRows()):
##~             print "|%s|%s|"% (str(self.tblConfig.item(i,1).currentText()),jcommand)
            if str(self.tblConfig.item(i,1).currentText()) == jcommand:
                return str(self.tblConfig.text(i,2))
        return None
        
    def processJCommand(self,jcommand,playerNo):
        if jcommand == "MOTION": 
            verticalAxis = int( -float(str(self.ledVertAxis.text())) * 100)
            horizontalAxis = int( -float(str(self.ledHoriAxis.text())) * 100)
            rotationAxis = int( -float(str(self.ledRotaAxis.text())) * 100)
            #nearest neighbour
            nearestDistance = 10e20
            walkType,forward,left,turnCCW = 0,0,0,0
            for (x,y,z),(wt,f,l,t) in WALK_COMMAND_SPACE:
                d = distance3D((x,y,z),(verticalAxis,horizontalAxis,rotationAxis)) 
                if d < nearestDistance:
                    nearestDistance = d
                    walkType,forward,left,turnCCW = wt,f,l,t            
            commandStr = "__03pyc__40 " + str(playerNo) + " joy move %d %d %d %d"%(forward,left,turnCCW,walkType)
            self.setSendingCommand(playerNo,commandStr)
        else:
            confCommand = self.getConfiguredCommand(jcommand)
##~         print "Process command " , jcommand, confCommand
            if confCommand is not None:                
                commandStr = "__03pyc__20 " + str(playerNo) + " joy butt " + confCommand
                self.setSendingCommand(playerNo,commandStr)
        
    def processEvent(self,event):
        d = event.dict
        jcommand = None
        playerNo = mapJoyToPlayer[int(d['joy'])]
        
        if event.type == pygame.JOYAXISMOTION:
            CALIBRATION = float(str(self.cbxCalibration.currentText()))
            value = str(d['value'] * CALIBRATION)
            if d['axis'] == 0:
                self.ledHoriAxis.setText(value)
            elif d['axis'] == 1:                
                self.ledVertAxis.setText(value)
            elif d['axis'] == 2:
                self.ledRotaAxis.setText(value)
            jcommand = "MOTION"
        elif event.type == pygame.JOYBUTTONUP:            
            jcommand = "Button" + str(d['button'])
            self.ledButtonClicked.setText(jcommand)
        
        if jcommand is not None:
            self.processJCommand(jcommand,playerNo)
            
    def processQueue(self,queue):
        self.nextCommands = {}
        for event in queue:
            self.processEvent(event)
        if self.nextCommands != {}:
            self.sendCommand()
    
    def slotNewAction(self,action = None, input = None, command = None, description = None):
        print "JoystickController.slotNewAction(): "
        currentNRow = self.tblConfig.numRows()
        self.tblConfig.setNumRows(currentNRow + 1)
        newComboTableItem = QComboTableItem(self.tblConfig,self.stringList,True)
        self.tblConfig.setItem(currentNRow,1,newComboTableItem)
        if input is not None:
            newComboTableItem.setCurrentItem(JOYSTICK_INPUT.index(input))
        if action is not None:
            self.tblConfig.setText(currentNRow, 0, action)
        if command is not None:
            self.tblConfig.setText(currentNRow, 2, command)
        if description is not None:
            self.tblConfig.setText(currentNRow, 3, description)

    def slotSaveConfiguration(self):
        print "FormJoystickController.slotSaveConfiguration(): Not implemented yet"

pygame.init()
print "pygame initialized."
print "number of joystick connected", pygame.joystick.get_count()
    
if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    
    
    w = JoystickController()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()

    pygame.quit()
    print "pygame quit."
