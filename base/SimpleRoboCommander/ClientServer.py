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



import traceback, sys,os, commands
from qt import *
from qtnetwork import *
from FormClientServer import FormClientServer
from DlgPyChecker import DlgPyChecker
from SimpleServer import SimpleServer
from HelpMe import HelpMe
from JointDebugger.JointDebugger import JointDebugger
from WalkLearner.WalkLearner import WalkLearner
#from JoystickController.JoystickController import JoystickController
from ftplib import *
from Common import *
from CalibraterInfo import CalibraterInfo

import time
import os

MAXIMUM_LINE_LENGTH = 70
TO_BE_UPLOADED = "To be uploaded"
BE_MODIFIED = "Be modified"
UP_TO_DATE = "Up to date"


bDebugMessaging = True
bDebugAll       = False
bDebugFTP       = True
bDebugFileLoading = False
        
def commandString(name,value):
    return "__%.2d%s__%.2d%s" % (len(name), name, len(value), value)
    
class ClientServer(FormClientServer):
    def __init__(self,bFTPOnly = False,bWorldModel = False, useJavaCPlane = False, bWalkLearnerLoadFromFile = False, parent = None,name = None,fl = 0):
        FormClientServer.__init__(self,parent,name,fl)
        self.initClassVariables(bWalkLearnerLoadFromFile)
        self.initServerSockets(bFTPOnly,bWorldModel)
        self.initCustomizedWidgets()
        self.initOtherGUI()
        self.connectSignals()
        self.useJavaCPlane = useJavaCPlane
        if bWorldModel:
            self.slotShowHideWorldModelDisplay()
        
    def initClassVariables(self, bContinueFromFile):
        #TODO: initialize JointDebugger() with self as a parent
        self.jointDebugger = JointDebugger(self, "JointDebugger",Qt.WType_TopLevel)
        self.walkLearner = WalkLearner(self, bContinueFromFile, "WalkLearner", Qt.WType_TopLevel)
        self.calibraterInfo = CalibraterInfo(self,"CalibraterInfo",Qt.WType_TopLevel)
        self.joystickController = QWidget(self,"JoystickController",Qt.WType_TopLevel)
        self.bHidenJointDebuger = False
        self.bHidenWalkLearner = False
        self.bHidenJoystickController = False
        self.bStartRobot = False
        self.walkForward = 0
        self.walkLeft = 0
        self.walkTurn = 0
        self.pythonFileItemMap = {}
        self.ftpHandler = FTP()
        self.fileListToUpload = []
        self.ftpCommandID = -1 #to wait for QFtp signal 
        self.telnetBufferSize = 0 #unlimited
        self.telnetNumberOfLines = 0
        self.bHighGainSet = True
        self.bPaused = False
        self.bShiftBeingPressed = False
        self.bHidenCalibraterInfo = True
        self.pydir=os.environ.get("ROBOCUP_DIR")
        if self.pydir:
            self.pydir += "/robot/PyCode"
        else:
            self.pydir = "."           
        
    def initServerSockets(self, bFTPOnly,bWorldModel):
        self.clientSocket = None
        self.serverSocket = None
        self.serverSendSocket = None
        if not bFTPOnly:
            if not bWorldModel:
                self.dummySockets = [SimpleServer(5019),\
                SimpleServer(5006),\
                SimpleServer(5020),\
                SimpleServer(5021),\
                SimpleServer(5022)]
            else:
                self.dummySockets = [
                    SimpleServer(5006)
                    ]   
        else:
            self.dummySockets = []

        for i in range(len(self.dummySockets)):
            QObject.connect(self.dummySockets[i], PYSIGNAL("newConnect()"),\
                self.newConnection)

        self.socketList = []        
        if not bFTPOnly:
            self.slotListen()

    def connectSignals(self):
        QObject.connect(self.btSteering, PYSIGNAL("up()"), self.btToolUp, SLOT("animateClick()"))
        QObject.connect(self.btSteering, PYSIGNAL("down()"), self.btToolDown, SLOT("animateClick()"))
        QObject.connect(self.btSteering, PYSIGNAL("left()"), self.btToolLeft, SLOT("animateClick()"))
        QObject.connect(self.btSteering, PYSIGNAL("right()"), self.btToolRight, SLOT("animateClick()"))
        QObject.connect(self.btSteering, PYSIGNAL("leftTurn()"), self.btToolLeftTurn, SLOT("animateClick()"))
        QObject.connect(self.btSteering, PYSIGNAL("rightTurn()"), self.btToolRightTurn, SLOT("animateClick()"))
        QObject.connect(self.btSteering, PYSIGNAL("brake()"), self.slotToolBrake)
        QObject.connect(self.btSteering, SIGNAL("clicked()"), self.slotToolStartSteering)
            
    def initCustomizedWidgets(self):
        self.btSteering.setText("Click this to control")
        
    def initOtherGUI(self):
        self.lviewPythonFiles.setResizeMode(QListView.AllColumns)
        self.progressDialog = QProgressDialog("Waiting...", "Cancel" ,  5, \
            self, "progressing...",True)
        
    
    def slotListen(self):
        try:
            port = int( str(self.ledLocalPort.text()) )
            self.serverSocket = SimpleServer(port)
            QObject.connect(self.serverSocket, PYSIGNAL("newConnect()"),\
                self.establishSendConnection)
            self.ledLocalIP.setText( self.serverSocket.address().toString() )
        except:
            traceback.print_exc(file = sys.stdout)
            print "Incorrect port : " ,str(self.ledLocalPort.text())
        
    def newConnection(self,socket):
        self.socketList.append(socket)
        self.reportServer("New Connection\n")
        QObject.connect(socket, PYSIGNAL("logText"),
            self.dispatchMessages)

        QObject.connect(socket, PYSIGNAL("connectionClose()"),
            self.clientClosed)

    def dispatchMessages(self,msg):
##~         print "DM Got " , msg
        msg = str(msg)
        if msg[0:4] == "%%%%":
            self.jointDebugger.gotNewMessage(msg[4:])
        elif msg[0:4] == "&&&&":
            self.walkLearner.slotGotNewMessage(msg[4:])
        elif msg[0:4] == "****":
            self.slotDebugGotNewPythonMessage(msg[4:])
        elif msg[0:4] == "----":
            self.calibraterInfo.slotGotNewMessage(msg[4:])
        else:
            self.txtServerBoard.append(firstNLine(2,msg))#only print 2 lines to avoid <RINFO> problem
            
            
    def establishSendConnection(self,socket):
        self.serverSendSocket = socket
        QObject.connect(socket, PYSIGNAL("connectionClose()"),
            self.clientClosed)
            
    def clientClosed(self):
        #TODO : still not be able to detect connection close
        print "Client closed"
    
    def slotDisconnectServer(self):
        self.reportServer("Listening Connection Closed")
        self.serverSocket = None

    def slotDisconnectClient(self):
        self.reportClient("Connection Closed")
        self.clientSocket.close()

    def slotConnect(self):
        try:
            ip = str( self.ledRemoteIP.text() )
            port = int (str( self.ledRemotePort.text() ) )
            self.clientSocket = QSocket()
            QObject.connect( self.clientSocket, SIGNAL("readyRead()"),
                self.readClient );
            self.clientSocket.connectToHost(ip,port)
            print self.clientSocket.state()
            print self.clientSocket.peerName()
            self.reportClient("Connection established , local port = " + \
                str(self.clientSocket.port())+"\n")
        except:
            traceback.print_exc(file = sys.stdout)
            
    def readClient(self):
#        ts = QTextStream (self.clientSocket);
        bytes = self.clientSocket.bytesAvailable()
#        print "readClient", bytes, "b available"
        while self.clientSocket.canReadLine():
#        while bytes > 0:
#            print "reading block", bytes, "available",
            str = self.clientSocket.readLine();
#            str = self.clientSocket.readBlock(bytes)
#            bytes = self.clientSocket.bytesAvailable()
#            str = ts.read().ascii();
#            print "done", len(str), "bytes"
#            str.truncate(str.length() - 2)
#            self.txtBoard.append(qstr.ascii())
            self.txtBoard.append(str)
            self.telnetNumberOfLines += 1
            if self.telnetBufferSize > 0 and self.telnetNumberOfLines > self.telnetBufferSize:
                self.txtBoard.removeParagraph(1)
                self.txtBoard.scrollToBottom()
                self.telnetNumberOfLines = self.txtBoard.lines()
##~                 print "now got ", self.telnetNumberOfLines
        
    def slotSend(self):
        msg = str(self.ledSend.text()) + "\r\n"
        if bDebugMessaging:
            print "Sending " , msg, type(msg)
        
        ts = QDataStream(self.clientSocket)
        print "Device " , ts.device()
##~         print "Device " , self.clientSocket.socketDevice()
        
        ts.writeRawBytes(msg)
        self.clientSocket.flush()
##~         self.txtBoard.setText("")
        
    def slotServerSend(self, msg = None):               
        if msg is None:
            msg = str(self.ledServerSend.text())
            if msg.find("__") == -1:
                name = str( self.cbxCommand.currentText() )
                value = str(self.ledServerSend.text())
                if len(value) > 99:
                    QMessageBox.critical( self, "Message too long",\
                        "You need to trim it down.")
                    return
                msg = commandString(name,value) 
                    
        if bDebugMessaging:
            print "Server Sending " , msg
            print "Decode ", decodeMsg(msg)
            print "Type",type(msg), " len",len(decodeMsg(msg) )
        self.sendToClient(self.serverSendSocket,decodeMsg(msg))
        
    
    def sendToClient(self,socket, msg):                
        ds = QDataStream(socket)
        ds.writeRawBytes(msg)
        socket.flush()
        
    def reportClient(self,str):
        self.txtBoard.append(str )
        
    def reportServer(self,str):
        self.txtServerBoard.append(str )
        
    def slotCommandTypeChanged(self,text):        
        prefix = "__" + twoDigits(str(text.length())) + str(text) + "__0"
        self.ledServerSend.setText(prefix)

    def slotShowHideJointDebugger(self):
        if self.bHidenJointDebuger:
            self.jointDebugger.hide()
            self.btJointDebugger.setText("&Show Joint Debugger")
        else:
            self.jointDebugger.show()
            self.btJointDebugger.setText("&Hide Joint Debugger")
            
        self.bHidenJointDebuger = not self.bHidenJointDebuger
        
    def slotShowHideJoystickController(self):
        if self.bHidenJoystickController:
            self.joystickController.hide()
            self.btJoystickController.setText("&Show Joystick Controller")
        else:
            self.joystickController.show()
            self.btJoystickController.setText("&Hide Joystick Controller")
            
        self.bHidenJoystickController = not self.bHidenJoystickController 
        
    def slotShowHideWalkLearner(self):
        if self.bHidenWalkLearner:
            self.walkLearner.hide()
            self.btWalkLearner.setText("Show &Walk Learner")
        else:
            self.walkLearner.show()
            self.btWalkLearner.setText("Hide &Walk Learner")
            
        self.bHidenWalkLearner= not self.bHidenWalkLearner
    
    def slotShowHideCPlaneDisplay(self):
        print "ClientServer.slotShowHideCPlaneDisplay():"
        if self.useJavaCPlane:
            runShellCommand("java -cp ../RoboCommander/:$CLASSPATH CPlaneDisplay")
        else:
            runShellCommand("CPlaneClient/CPlaneClient CPlaneClient/CPLANE_LOG")
        
        
    
    def slotToolSendCPlane(self):
        print "ClientServer.slotToolSendCPlane(): "
        playerNo = str(self.cbxPlayerNumber.currentText())
        msg = "__01d__03" + playerNo
        self.slotServerSend(msg)

    def slotToolStartStopRobot(self):
        print "ClientServer.slotToolStartRobot(): "
        if self.bStartRobot:
            self.btToolStartStop.setText("&Start Robot (mode)")
            msg = "__04mode__011" #stop
        else:
            self.btToolStartStop.setText("&Stop Robot (mode)")
            msg = "__04mode__010" #start
            
        self.bStartRobot = not self.bStartRobot
        self.slotServerSend(msg);
    
    def slotShowHideWorldModelDisplay(self):
        print "ClientServer.slotShowHideWorldModelDisplay(): "  
        runShellCommand("java -cp ../RoboCommander/:$CLASSPATH WorldModelDisplay")
        print "java WorldModelDisplay is running..."
##~         self.btWorldModelDisplay.setEnabled(False)
        
    def slotToolWalkCommand(self):
        print "ClientServer.slotToolWalkCommand(): "  
        maxLength = 20;
        
        #change walktype
        walkType = self.cbxWalkType.currentItem()
        msg = "__02wt__"+str(maxLength) + str(walkType)
        self.slotServerSend(msg)
        
        #change walkcommand
        walkCommand = str( self.ledWalkCommand.text())
        walkCommand = walkCommand[0:maxLength]
        
        msg = "__01j__"+str(maxLength) + walkCommand
        self.slotServerSend(msg);

    def slotToolHeadCommand(self):
        print "FormClientServer.slotToolHeadCommand(): Not implemented yet"
        headCommand = str( self.ledHeadCommand.text())
        maxLength = 20;
        headCommand = headCommand[0:maxLength]
        msg = "__04head__"+str(maxLength) + headCommand
        self.slotServerSend(msg);

    def slotToolRelaxDog(self):
        print "ClientServer.slotToolRelaxDog(): "
        msg = "__05relax__010"
        self.slotServerSend(msg);
        
    def slotToolSendYUV(self):
        print "ClientServer.slotToolSendYUV(): "
        playerNo = str(self.cbxPlayerNumber.currentText())
        msg = "__01y__03" + playerNo
        self.slotServerSend(msg)
        
        
    def slotToolSendWorldModel(self):
        print "ClientServer.slotToolSendWorldModel(): "
        if self.bShiftBeingPressed:
            #send all world model
            msg = "__01w__039"
        else:
            playerNo = str(self.cbxPlayerNumber.currentText())
            msg = "__01w__03" + playerNo
        self.slotServerSend(msg)
        
    def slotToolTurnOff(self):
        print "ClientServer.slotToolTurnOff():"
        msg = "__03off__010"
        self.slotServerSend(msg);

    def slotToolSetPlayer(self):
        print "ClientServer.slotToolSetPlayer():"
        playType = str(self.ledPlayerType.text())
        playType = playType[0:3]
        msg = "__01p__03" + playType
        self.slotServerSend(msg);

    def slotToolSetGain(self):
        print "ClientServer.slotToolSetGain(): "
        if self.bHighGainSet:
            self.btToolSetGain.setText("&Set High Gain")
            msg = "__03pid__010" #low
        else:
            self.btToolSetGain.setText("&Set Low Gain")
            msg = "__03pid__011" #high

        self.bHighGainSet = not self.bHighGainSet
        self.slotServerSend(msg);
    
    def sendWalkCommand(self):
        msg = "__01j__20%d %d %d"%(self.walkForward, self.walkLeft, self.walkTurn)  #walk command
        self.ledWalkCommand.setText("%d %d %d" % (self.walkForward, self.walkLeft, self.walkTurn))
        self.slotServerSend(msg);

    def slotToolBrake(self):
        self.slotToolStop()

    def slotToolMoveUp(self):
        print "ClientServer.slotToolMoveUp()"    
        self.walkForward += 2
        self.sendWalkCommand()

    def slotToolMoveDown(self):
        print "ClientServer.slotToolMoveDown()"
        self.walkForward -= 2
        self.sendWalkCommand()
        
    def slotToolMoveLeft(self):
        print "ClientServer.slotToolMoveLeft()"
        self.walkLeft += 2
        self.sendWalkCommand()
        
    def slotToolMoveRight(self):
        print "ClientServer.slotToolMoveRight()"
        self.walkLeft -= 2
        self.sendWalkCommand()

    def slotToolStop(self):
        print "FormClientServer.slotToolStop()"
        self.walkForward = 0
        self.walkLeft = 0
        self.walkTurn = 0
        self.sendWalkCommand()

        
    def slotToolStartSteering(self):
        print "ClientServer.slotToolStartSteering(): "
        msg = "__01p__010" #set remote player
        self.slotServerSend(msg)
    
    def slotToolMoveRightTurn(self):
        print "ClientServer.slotToolMoveRightTurn(): "
        self.walkTurn -= 2
        self.sendWalkCommand();

    def slotToolMoveLeftTurn(self):
        print "ClientServer.slotToolMoveLeftTurn(): "
        self.walkTurn += 2
        self.sendWalkCommand();
    
    def slotPythonReload(self):
        print "ClientServer.slotPythonReload(): "
        playerNo = str(self.lcdPlayerNo.intValue())
        msg = "__02py__10reload " + playerNo
        self.slotServerSend(msg)

    def loadTextFromFile(self,fileName):
        f = open(fileName,"r")
        lines = f.readlines()
##~         self.tedPython.clear()
##~         for line in lines:
##~             self.tedPython.append(line)
        
    def resetFileList(self):
        self.lviewPythonFiles.clear()
        self.pythonFileItemMap = {}
        
    def slotPythonOpenDir(self):
        print "ClientServer.slotPythonOpenDir(): "
        dir = QFileDialog.getExistingDirectory(self.ledFilePath.text(),\
            self,\
            "Open python source directory"\
            "Choose a directory")
        if dir:
            self.ledFilePath.setText(dir)
        self.slotPythonListFile()
        
    def slotPythonListFile(self):
        dir = str(self.ledFilePath.text() )
        if str(dir) != "":
            self.resetFileList()
            self.slotPythonRefresh()
            self.slotResetPythonFilesTimestamp()
            self.slotPythonRefresh()
        
    
    def uploadedSuccessfully(self,absFileName):
        try:
            file = open(absFileName,"r")
            fileName = str(QFileInfo(absFileName).fileName())
            
            _,timestamp = getSizeAndTimeStamp(absFileName)
            print "Got timestamp", timestamp
            if not self.pythonFileItemMap.has_key(fileName):
                raise "Weird, fileName should has appeared in the dictionary"
            self.pythonFileItemMap[fileName][1] = timestamp
            response = self.ftpHandler.storlines("STOR " + convertTo8dot3Name(fileName),file)
            if bDebugFTP:
                print "AIBO FTP:response", response
            if isNotGoodFtpRespons(self.ftpHandler.lastresp) :
                print "Uploading failed : ",response
                return False
            else:
                return True

        except IOError:
            QMessageBox.critical( self, "Upload error",\
                "Can't open file %s for reading."%fileName)
            return False
        
            
    def startUploading(self):
        dirPath = str(self.ledFilePath.text())
        for fileName in self.fileListToUpload:
            absFileName = dirPath + "/" + fileName
            if not self.uploadedSuccessfully(absFileName):
                print "File %s not uploaded " % absFileName
        self.ftpHandler.close()

    def loggedIntoTheDog(self):
        host = str(self.ledDogIp.text())
        response = self.ftpHandler.connect(host)
        if bDebugFTP:
            print "AIBO FTP:response", response
        if isNotGoodFtpRespons(self.ftpHandler.lastresp):
            print "Failed:",response
            return False
        response = self.ftpHandler.login("a","a")
        if bDebugFTP:
            print "AIBO FTP:response", response
        if isNotGoodFtpRespons(self.ftpHandler.lastresp):
            print "Failed:",response    
            return False
        response = self.ftpHandler.cwd("PyCode")
        if bDebugFTP:
            print "AIBO FTP:response", response
        if isNotGoodFtpRespons(self.ftpHandler.lastresp):
            print "Failed:",response
            return False
        
        return True
            
    def uploadFiles(self, fileList):
        print "Uploading", fileList
        if fileList == []:
            print "Nothing to be uploaded"
            return True
            
        if self.loggedIntoTheDog():
            self.fileListToUpload = fileList
            self.startUploading()
            return True
        else:
            return False
        
    def slotPythonUpload(self):
        print "ClientServer.slotPythonUpload(): "
        
            
        print self.lviewPythonFiles.currentItem().text(0)
        item = self.lviewPythonFiles.currentItem()
        if not self.runPyChecker([str(item.text(0))]) : 
            return 
        # Add this file to the reload list
        self.createReloadFile([str(item.text(0))])
        if self.uploadFiles([str(item.text(0)), "Reload.py"]):
            self.slotRefreshFileItem(item.text(0))
        print "Done"
    
    #assumming Reload file is in dirPath
    def getCurrentReloadingModules(self):
        dirPath = str(self.ledFilePath.text())
        f = open(self.pydir + "/Reload.py","r")
        lines = f.readlines()
        f.close()
        
        moduleList = []
        for line in lines:
            if "import" in line.split():
                moduleList.append(line.split()[1])
        
        return moduleList
        
    #assumming Reload file is in dirPath    
    def writeReloadedModulesToFile(self,moduleList):
        dirPath = str(self.ledFilePath.text())
        f = open(self.pydir + "/Reload.py","w")
        
        #I thought Daniel's comment was a good one, so I keep it here
        f.write("""
#===============================================================================
#   Python Behaviours : 2004 (c) 
#
# Reload.py
#
# A handy module that reloads all files that need to be reloaded. Right now 
# this is hard coded, but in the future ClientServer.py will create this file
# automatically, depending on what files are uploaded to the dog.
#
#===============================================================================

#This file is AUTO-GENERATED by SimpleRoboCommander
#Date :??/??/??  Time:??:??:??
""")

        moduleList = map( getOnlyFileName, moduleList)
        if "Player" not in moduleList:
            moduleList.append("Player")
        filteredModuleList = []
        for i in range(len(moduleList)):            
            if moduleList[i][:8] == "Behaviou": continue #skip Behaviou
            if moduleList[i][:8] == "VisionLi": continue #skip VisionLink
            if moduleList[i][:8] == "Reload": continue #skip Reload, this one is dangerous, lead to circular reloading
            filteredModuleList.append(moduleList[i])
        
        for moduleName in filteredModuleList :
            
            f.write( "import %s\n" % moduleName )
            
        f.write("""

#===============================================================================
# These following statments are sort of like main function in C.
#===============================================================================

# Usually only the top-level module have these, but this module (not the top-level
# module) is an exception because only Behaviou.py is importing this, and we 
# want to use this module to really reload codes once imported.


# The reload(module) function is a python built-in function. When called, the 
# module.py will be read, interpreted, and top-level statements executed if there 
# is any.
""")
        for moduleName in filteredModuleList :
            f.write( "reload(%s)\n" % moduleName )
        f.write("\n\n# That's it , nothing's complicated , huh?\n")
        f.close()
        
    # Add listPythonModules to the Reload.py file. (Old modules
    # remain as well
    def createReloadFile(self, listPythonModules):
        print "Adding", listPythonModules, "to Reload.py"
        oldReloadList = self.getCurrentReloadingModules()
        listPythonModules = listPythonModules +\
            [x + ".py" for x in oldReloadList \
                if x + ".py" not in listPythonModules]
        listPythonModules.sort()
        self.writeReloadedModulesToFile(listPythonModules)
                
    def slotPythonUploadAll(self):
        print "ClientServer.slotPythonUploadAll(): "
        
        listAll = []
        item = self.lviewPythonFiles.firstChild()
        while item is not None:
            if str(item.text(1)) != UP_TO_DATE: 
                listAll.append( str(item.text(0)) )
            item = item.nextSibling()
        
        if not self.runPyChecker(listAll) : 
            return 
            
        self.createReloadFile(listAll)
        if self.uploadFiles(listAll + ["Reload.py"]):
            self.slotRefreshAllFileItems()
        print "Done"
        
    def addNewFileItem(self,fileName):
        listItem = QListViewItem(self.lviewPythonFiles, fileName)
        absFileName = str(self.ledFilePath.text()) + "/" + fileName
        size,_ = getSizeAndTimeStamp(absFileName)
        listItem.setText(1,TO_BE_UPLOADED) #status
        listItem.setText(2,str(size))
        self.pythonFileItemMap[fileName] = [listItem,0]
        
    def slotRefreshFileItem(self,fileName):
        fileName = str(fileName)
        if self.lviewPythonFiles.findItem(fileName, 0) is not None:            
            absFileName = str(self.ledFilePath.text()) + "/" + fileName
            size,timestamp = getSizeAndTimeStamp(absFileName)
            listItem , oldTimeStamp  = self.pythonFileItemMap[fileName]

            if bDebugFileLoading:
                print "Old %d New %d" % (oldTimeStamp, timestamp)
            if oldTimeStamp == 0:
                listItem.setText(1,TO_BE_UPLOADED)
            elif timestamp != oldTimeStamp:
                listItem.setText(1,BE_MODIFIED)
            else:
                listItem.setText(1,UP_TO_DATE)
            
    def slotRefreshAllFileItems(self):
        for fileName in self.pythonFileItemMap.keys():
            self.slotRefreshFileItem(fileName)
       
    def slotResetPythonFilesTimestamp(self):
        print "ClientServer.slotSetPythonFilesTimestamp(): "
        for fileName in self.pythonFileItemMap.keys():
            absFileName = str(self.ledFilePath.text()) + "/" + fileName
            _,timestamp = getSizeAndTimeStamp(absFileName)
            self.pythonFileItemMap[fileName][1] = timestamp        
                
    def slotPythonRefresh(self):
        print "ClientServer.slotPythonRefresh(): "
        
        path = self.ledFilePath.text()
        if str(path) == "": return
        
        dir = QDir( path ) 
        if not dir.exists():
            QMessageBox.critical( self, "Opening dir",\
                "Directory does not exist, please specify again.")
                    
        dir.setNameFilter("*.py")
        fileList = dir.entryInfoList()
        for fileInfo in fileList:
            fileName = str(fileInfo.fileName())
            print "Loading ", fileName, fileInfo.size()
            
            if not self.pythonFileItemMap.has_key(fileName):
                self.addNewFileItem(fileName )
            else:
                self.slotRefreshFileItem(fileName)
        
    def runPyChecker(self,uploadingFiles):
        listFileToBeChecked = uploadingFiles[:]
        dirPath = str(self.ledFilePath.text())     
        listFileToBeChecked = map( lambda x : "%s%s" % (dirPath,x) , listFileToBeChecked)

        if "Behaviou.py" not in listFileToBeChecked:
            listFileToBeChecked.append(self.pydir+"/Behaviou.py")

        pycheckerFileArgs = " ".join(listFileToBeChecked)
        print "Running ","pychecker "+pycheckerFileArgs
        os.chdir(dirPath)
        (status, output) = commands.getstatusoutput("pychecker -e --complexity "\
                                                    +pycheckerFileArgs)
        if status != 0:
            dialog = DlgPyChecker()
            dialog.tedResult.setText(output)
            if dialog.exec_loop() == QDialog.Accepted:
                print "There's complaints, but you ignored them"
                return True
            else:
                print "Error to fix, nothing uploaded"
                return False
        else:
            print "PyChecker passed"
            return True
            
    def slotPythonShowFile(self):
        print "ClientServer.slotPythonShowFile(): "

    def slotTelnetChangeBufferSize(self):
        print "ClientServer.slotTelnetChangeBufferSize(): "
        self.telnetBufferSize = int(str(self.ledBufferMaxLines.text()))

    def slotTelnetSaveAndClear(self):
        print "ClientServer.slotTelnetSaveAndClear(): "
        f = open("telnet.log","w")
        f.write(str(self.txtBoard.text()))
        f.close()
        self.txtBoard.clear()
    
    def slotDebugEvaluateExpression(self):
        print "ClientServer.slotDebugEvaluateExpression(): "
        playerNo = str(self.lcdPlayerNo.intValue())
        expression = str( self.cbxDebugExpression.currentText())
        msg = "__03pyc__99%s eval %s"%( playerNo, expression)
        
        self.slotServerSend(msg)
        
    def slotDebugExecuteExpression(self):
        print "ClientServer.slotDebugExecuteExpression(): "
        playerNo = str(self.lcdPlayerNo.intValue())
        expression = str( self.cbxDebugExpression.currentText())
        msg = "__03pyc__99%s exec %s"%( playerNo, expression)
        self.slotServerSend(msg)
        
    def slotDebugGotNewPythonMessage(self,msg):
        self.tedDebugResult.append(msg)
    
    def slotDebugChangeDBFlag(self):
        print "ClientServer.slotDebugChangeDBFlag(): "
        var = str(self.cbxDebuggingFlag.currentText())
        if eval(str(var)):
            self.btDebuggingFlag.setText("True")
        else:
            self.btDebuggingFlag.setText("False")
        
    def slotDebugSetDBFlag(self):
        global bDebugMessaging
        print "ClientServer.slotDebugSetDBFlag(): "
        var = str(self.cbxDebuggingFlag.currentText())
        print "%s = not %s" % (var, var)
        exec("%s = not %s" % (var, var))
        self.slotDebugChangeDBFlag()
        print bDebugMessaging
    
    def slotPythonSetModified(self,lviewItem):
        print "ClientServer.slotPythonSetModified(): "
        fileName = str( lviewItem.text(0) )
##~         print "Set Modified ", lviewItem.text(0), fileName
        self.pythonFileItemMap[fileName][1] = 0
        self.slotPythonRefresh()
        
    def keyPressEvent( self, event ):
##~         print "got key state " , event.state()
        self.bShiftBeingPressed = True
##~         if event.state() & Qt.ShiftButton:
##~             print "Shift is pressed" 
            
    def keyReleaseEvent ( self, event ):
        self.bShiftBeingPressed = False
##~         print "got key released " , event.state()
##~         if event.state() & Qt.ShiftButton:
##~             print "Shift is released" 

    def slotToolSetCPlaneInterval(self):
        print "ClientServer.slotToolSetCPlaneInterval(): "
        interval = int(str(self.ledGeneral.text()))
        msg = "__03sfr__10 %d"% interval
        self.slotServerSend(msg)
    
    def slotShowHelp(self):
        dialog = HelpMe()
        dialog.exec_loop()
    
    def slotToolPauseResume(self):
        playerNo = str(self.cbxPlayerNumber.currentText())
        if self.bPaused:
            self.btToolPauseResume.setText("&Pause")
            msg = "__03pyc__10%s resume" % playerNo
        else:
            msg = "__03pyc__10%s pause" % playerNo
            self.btToolPauseResume.setText("&Resume")            
        self.slotServerSend(msg)
        self.bPaused = not self.bPaused
        
        
    def slotFTPList(self):
        print "ClientServer.slotFTPList(): Not implemented yet"
        host = str(self.ledDogIp.text())
        response = self.ftpHandler.connect(host)
        self.logMessage("AIBO FTP:response ->"+ response)
        self.ftpHandler.login("a","a")
        if str(self.ledFTPRemoteDir.text()) != "":
            self.ftpHandler.cwd(str(self.ledFTPRemoteDir.text()))
        allFiles = self.ftpHandler.nlst()
##~         print allFiles
        self.tedFTPRemote.clear()
        for line in allFiles:            
            self.tedFTPRemote.append(line + "\n")
        self.ftpHandler.close()

    def slotFTPUpload(self):
        print "ClientServer.slotFTPUpload(): Not implemented yet"

    def slotFTPDownload(self):
        print "ClientServer.slotFTPDownload(): "
        host = str(self.ledDogIp.text())
        response = self.ftpHandler.connect(host)
        self.logMessage("AIBO FTP:response ->"+ response)
        response = self.ftpHandler.login("a","a")
        self.logMessage("AIBO FTP:response ->"+ response)
        
        fromIndex = self.spbFTPFromIndex.value()
        toIndex = self.spbFTPToIndex.value()
        for i in range(fromIndex,toIndex+1):
            fileName = "YUV%.2d.bfl"%i
            if str(self.ledFTPRemoteDir.text()) != "":
                remoteFileName = "%s/%s" % (self.ledFTPRemoteDir.text(), fileName) 
            else:
                remoteFileName = fileName
            if str(self.ledFTPLocalDir.text()) != "":
                localFileName = "%s/%s" % (self.ledFTPLocalDir.text(), fileName) 
            else:
                localFileName = fileName
            response = self.ftpHandler.retrbinary('RETR %s'%remoteFileName, open(localFileName, 'wb').write)
            self.logMessage(response)
        self.ftpHandler.close()
        
        
    def slotFTPDeleteYUVFiles(self):
        print "ClientServer.slotFTPDeleteYUVFiles(): "
        host = str(self.ledDogIp.text())
        response = self.ftpHandler.connect(host)
        self.logMessage("AIBO FTP:response ->"+ response)
        response = self.ftpHandler.login("a","a")
        self.logMessage("AIBO FTP:response ->"+ response)
        
        fromIndex = self.spbFTPFromIndex.value()
        toIndex = self.spbFTPToIndex.value()
        for i in range(fromIndex,toIndex+1):
            fileName = "YUV%.2d.bfl"%i
            if str(self.ledFTPRemoteDir.text()) != "":
                remoteFileName = "%s/%s" % (self.ledFTPRemoteDir.text(), fileName) 
            else:
                remoteFileName = fileName
            try:
                response = self.ftpHandler.delete(remoteFileName)
                self.logMessage(response)
            except: #deleting file not exists
                pass
        self.ftpHandler.close()
        self.slotFTPList()
        

    def slotFTPOpenDir(self):
        print "ClientServer.slotFTPOpenDir(): Not implemented yet"
    
    def logMessage(self,message):
        self.tedLogger.append(message+"\n")
        
    def slotShowCalibraterInfo(self):
        if self.bHidenCalibraterInfo:
            self.calibraterInfo.show()
            self.btShowCalibrater.setText("&Show Calibrater Info")
        else:
            self.calibraterInfo.hide()
            self.btShowCalibrater.setText("&Hide Calibrater Info")
            
        self.bHidenCalibraterInfo= not self.bHidenCalibraterInfo
        
def run(argv):
    a = QApplication(argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    bWorldModel = "-wm" in argv
    bJavaCPlaneDisplay = "-CPD" in argv
    bWalkLearnerLoadFromFile = "-l" in argv
    bFTPOnly = "ftp" in argv
    w = ClientServer(bFTPOnly, bWorldModel,bJavaCPlaneDisplay,bWalkLearnerLoadFromFile)
    a.setMainWidget(w)
    w.show()
    a.exec_loop()

if __name__ == "__main__":
    run(sys.argv)
