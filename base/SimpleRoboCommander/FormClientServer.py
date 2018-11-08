# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'FormClientServer.ui'
#
# Created: Wed May 18 16:30:45 2005
#      by: The PyQt User Interface Compiler (pyuic) 3.14.1
#
# WARNING! All changes made in this file will be lost!


from qt import *
#Import customized SteeringButton
from SteeringButton import *


class FormClientServer(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)

        if not name:
            self.setName("FormClientServer")


        FormClientServerLayout = QGridLayout(self,1,1,11,6,"FormClientServerLayout")

        self.tabWidget2 = QTabWidget(self,"tabWidget2")

        self.tab = QWidget(self.tabWidget2,"tab")
        tabLayout = QGridLayout(self.tab,1,1,11,6,"tabLayout")

        self.groupBox2 = QGroupBox(self.tab,"groupBox2")
        self.groupBox2.setColumnLayout(0,Qt.Vertical)
        self.groupBox2.layout().setSpacing(6)
        self.groupBox2.layout().setMargin(11)
        groupBox2Layout = QGridLayout(self.groupBox2.layout())
        groupBox2Layout.setAlignment(Qt.AlignTop)

        self.ledLocalPort = QLineEdit(self.groupBox2,"ledLocalPort")

        groupBox2Layout.addMultiCellWidget(self.ledLocalPort,3,3,1,2)

        self.textLabel1 = QLabel(self.groupBox2,"textLabel1")

        groupBox2Layout.addWidget(self.textLabel1,3,0)

        self.textLabel1_2 = QLabel(self.groupBox2,"textLabel1_2")

        groupBox2Layout.addWidget(self.textLabel1_2,2,0)

        self.btDisconnectServer = QPushButton(self.groupBox2,"btDisconnectServer")

        groupBox2Layout.addWidget(self.btDisconnectServer,3,3)

        self.btListen = QPushButton(self.groupBox2,"btListen")

        groupBox2Layout.addWidget(self.btListen,2,3)

        self.ledLocalIP = QLineEdit(self.groupBox2,"ledLocalIP")

        groupBox2Layout.addMultiCellWidget(self.ledLocalIP,2,2,1,2)

        self.btServerSend = QPushButton(self.groupBox2,"btServerSend")

        groupBox2Layout.addWidget(self.btServerSend,0,3)

        self.cbxCommand = QComboBox(0,self.groupBox2,"cbxCommand")
        self.cbxCommand.setEditable(1)

        groupBox2Layout.addMultiCellWidget(self.cbxCommand,0,0,0,1)

        self.ledServerSend = QLineEdit(self.groupBox2,"ledServerSend")

        groupBox2Layout.addWidget(self.ledServerSend,0,2)

        self.txtServerBoard = QTextEdit(self.groupBox2,"txtServerBoard")
        self.txtServerBoard.setTabChangesFocus(1)

        groupBox2Layout.addMultiCellWidget(self.txtServerBoard,1,1,0,3)

        tabLayout.addWidget(self.groupBox2,0,0)

        self.groupBox3 = QGroupBox(self.tab,"groupBox3")
        self.groupBox3.setColumnLayout(0,Qt.Vertical)
        self.groupBox3.layout().setSpacing(6)
        self.groupBox3.layout().setMargin(11)
        groupBox3Layout = QGridLayout(self.groupBox3.layout())
        groupBox3Layout.setAlignment(Qt.AlignTop)

        self.btJointDebugger = QPushButton(self.groupBox3,"btJointDebugger")

        groupBox3Layout.addWidget(self.btJointDebugger,0,1)

        self.btCPlaneDisplay = QPushButton(self.groupBox3,"btCPlaneDisplay")

        groupBox3Layout.addWidget(self.btCPlaneDisplay,0,0)

        self.btWorldModelDisplay = QPushButton(self.groupBox3,"btWorldModelDisplay")

        groupBox3Layout.addWidget(self.btWorldModelDisplay,1,0)

        self.btWalkLearner = QPushButton(self.groupBox3,"btWalkLearner")

        groupBox3Layout.addMultiCellWidget(self.btWalkLearner,0,0,2,3)

        self.btShowCalibrater = QPushButton(self.groupBox3,"btShowCalibrater")

        groupBox3Layout.addWidget(self.btShowCalibrater,1,1)

        self.btHelpMe = QPushButton(self.groupBox3,"btHelpMe")

        groupBox3Layout.addWidget(self.btHelpMe,1,3)

        self.btJoystickController = QPushButton(self.groupBox3,"btJoystickController")

        groupBox3Layout.addWidget(self.btJoystickController,1,2)

        tabLayout.addWidget(self.groupBox3,1,0)
        self.tabWidget2.insertTab(self.tab,QString.fromLatin1(""))

        self.tab_2 = QWidget(self.tabWidget2,"tab_2")
        tabLayout_2 = QGridLayout(self.tab_2,1,1,11,6,"tabLayout_2")

        self.btConnect = QPushButton(self.tab_2,"btConnect")

        tabLayout_2.addMultiCellWidget(self.btConnect,0,0,7,8)

        self.textLabel2 = QLabel(self.tab_2,"textLabel2")

        tabLayout_2.addWidget(self.textLabel2,0,0)

        self.ledRemoteIP = QLineEdit(self.tab_2,"ledRemoteIP")

        tabLayout_2.addMultiCellWidget(self.ledRemoteIP,0,0,1,3)

        self.textLabel2_2 = QLabel(self.tab_2,"textLabel2_2")

        tabLayout_2.addWidget(self.textLabel2_2,0,4)

        self.ledRemotePort = QLineEdit(self.tab_2,"ledRemotePort")

        tabLayout_2.addMultiCellWidget(self.ledRemotePort,0,0,5,6)

        self.ledSend = QLineEdit(self.tab_2,"ledSend")

        tabLayout_2.addMultiCellWidget(self.ledSend,1,1,1,6)

        self.btSend = QPushButton(self.tab_2,"btSend")

        tabLayout_2.addMultiCellWidget(self.btSend,1,1,7,8)

        self.txtBoard = QTextEdit(self.tab_2,"txtBoard")
        self.txtBoard.setTabChangesFocus(1)

        tabLayout_2.addMultiCellWidget(self.txtBoard,2,2,0,8)

        self.btTelnetClearOutput = QPushButton(self.tab_2,"btTelnetClearOutput")

        tabLayout_2.addMultiCellWidget(self.btTelnetClearOutput,3,3,3,4)

        self.btTelnetSaveAndClear = QPushButton(self.tab_2,"btTelnetSaveAndClear")

        tabLayout_2.addWidget(self.btTelnetSaveAndClear,3,5)

        self.btDisconnectClient = QPushButton(self.tab_2,"btDisconnectClient")

        tabLayout_2.addMultiCellWidget(self.btDisconnectClient,3,3,6,7)
        spacer1 = QSpacerItem(90,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        tabLayout_2.addItem(spacer1,3,8)

        self.ledBufferMaxLines = QLineEdit(self.tab_2,"ledBufferMaxLines")

        tabLayout_2.addWidget(self.ledBufferMaxLines,3,2)

        self.textLabel2_4 = QLabel(self.tab_2,"textLabel2_4")

        tabLayout_2.addMultiCellWidget(self.textLabel2_4,3,3,0,1)
        self.tabWidget2.insertTab(self.tab_2,QString.fromLatin1(""))

        self.tab_3 = QWidget(self.tabWidget2,"tab_3")
        tabLayout_3 = QGridLayout(self.tab_3,1,1,11,6,"tabLayout_3")

        self.groupBox3_2 = QGroupBox(self.tab_3,"groupBox3_2")
        self.groupBox3_2.setColumnLayout(0,Qt.Vertical)
        self.groupBox3_2.layout().setSpacing(6)
        self.groupBox3_2.layout().setMargin(11)
        groupBox3_2Layout = QGridLayout(self.groupBox3_2.layout())
        groupBox3_2Layout.setAlignment(Qt.AlignTop)

        self.btToolStartStop = QPushButton(self.groupBox3_2,"btToolStartStop")

        groupBox3_2Layout.addWidget(self.btToolStartStop,1,0)

        self.btToolRelaxDog = QPushButton(self.groupBox3_2,"btToolRelaxDog")

        groupBox3_2Layout.addWidget(self.btToolRelaxDog,2,0)

        self.btToolSendYUV = QPushButton(self.groupBox3_2,"btToolSendYUV")

        groupBox3_2Layout.addWidget(self.btToolSendYUV,3,0)

        self.btToolTurnOff = QPushButton(self.groupBox3_2,"btToolTurnOff")

        groupBox3_2Layout.addWidget(self.btToolTurnOff,9,0)

        self.btToolReloadPython = QPushButton(self.groupBox3_2,"btToolReloadPython")

        groupBox3_2Layout.addWidget(self.btToolReloadPython,5,0)

        self.btToolSetGain = QPushButton(self.groupBox3_2,"btToolSetGain")

        groupBox3_2Layout.addWidget(self.btToolSetGain,6,0)

        self.btToolSendWorldModel = QPushButton(self.groupBox3_2,"btToolSendWorldModel")

        groupBox3_2Layout.addWidget(self.btToolSendWorldModel,4,0)
        spacer7 = QSpacerItem(20,80,QSizePolicy.Minimum,QSizePolicy.Expanding)
        groupBox3_2Layout.addItem(spacer7,8,0)

        self.btToolSendCPlane = QPushButton(self.groupBox3_2,"btToolSendCPlane")

        groupBox3_2Layout.addWidget(self.btToolSendCPlane,0,0)

        self.btToolPauseResume = QPushButton(self.groupBox3_2,"btToolPauseResume")

        groupBox3_2Layout.addWidget(self.btToolPauseResume,7,0)

        tabLayout_3.addWidget(self.groupBox3_2,0,0)

        self.groupBox4 = QGroupBox(self.tab_3,"groupBox4")
        self.groupBox4.setColumnLayout(0,Qt.Vertical)
        self.groupBox4.layout().setSpacing(6)
        self.groupBox4.layout().setMargin(11)
        groupBox4Layout = QGridLayout(self.groupBox4.layout())
        groupBox4Layout.setAlignment(Qt.AlignTop)
        spacer4 = QSpacerItem(20,40,QSizePolicy.Minimum,QSizePolicy.Expanding)
        groupBox4Layout.addItem(spacer4,5,1)

        self.btToolHeadCommand = QPushButton(self.groupBox4,"btToolHeadCommand")

        groupBox4Layout.addWidget(self.btToolHeadCommand,2,2)

        self.btToolWalkCommand = QPushButton(self.groupBox4,"btToolWalkCommand")

        groupBox4Layout.addWidget(self.btToolWalkCommand,1,2)

        self.btToolSetPlayer = QPushButton(self.groupBox4,"btToolSetPlayer")

        groupBox4Layout.addWidget(self.btToolSetPlayer,3,2)

        self.cbxPlayerNumber = QComboBox(0,self.groupBox4,"cbxPlayerNumber")

        groupBox4Layout.addWidget(self.cbxPlayerNumber,0,2)

        self.textLabel1_3 = QLabel(self.groupBox4,"textLabel1_3")

        groupBox4Layout.addMultiCellWidget(self.textLabel1_3,0,0,0,1)

        self.ledHeadCommand = QLineEdit(self.groupBox4,"ledHeadCommand")

        groupBox4Layout.addMultiCellWidget(self.ledHeadCommand,2,2,0,1)

        self.cbxWalkType = QComboBox(0,self.groupBox4,"cbxWalkType")

        groupBox4Layout.addWidget(self.cbxWalkType,1,0)

        self.ledWalkCommand = QLineEdit(self.groupBox4,"ledWalkCommand")

        groupBox4Layout.addWidget(self.ledWalkCommand,1,1)

        self.groupBox5 = QGroupBox(self.groupBox4,"groupBox5")
        self.groupBox5.setColumnLayout(0,Qt.Vertical)
        self.groupBox5.layout().setSpacing(6)
        self.groupBox5.layout().setMargin(11)
        groupBox5Layout = QGridLayout(self.groupBox5.layout())
        groupBox5Layout.setAlignment(Qt.AlignTop)

        self.btToolDown = QPushButton(self.groupBox5,"btToolDown")

        groupBox5Layout.addWidget(self.btToolDown,2,1)

        self.btToolRightTurn = QPushButton(self.groupBox5,"btToolRightTurn")

        groupBox5Layout.addWidget(self.btToolRightTurn,0,2)

        self.btToolLeftTurn = QPushButton(self.groupBox5,"btToolLeftTurn")

        groupBox5Layout.addWidget(self.btToolLeftTurn,0,0)

        self.btToolUp = QPushButton(self.groupBox5,"btToolUp")

        groupBox5Layout.addWidget(self.btToolUp,0,1)

        self.btToolRight = QPushButton(self.groupBox5,"btToolRight")

        groupBox5Layout.addWidget(self.btToolRight,1,2)

        self.btToolLeft = QPushButton(self.groupBox5,"btToolLeft")

        groupBox5Layout.addWidget(self.btToolLeft,1,0)

        self.btToolStop = QPushButton(self.groupBox5,"btToolStop")

        groupBox5Layout.addWidget(self.btToolStop,1,1)

        self.btSteering = SteeringButton(self.groupBox5,"btSteering")

        groupBox5Layout.addWidget(self.btSteering,2,0)

        groupBox4Layout.addMultiCellWidget(self.groupBox5,6,6,0,2)

        self.ledPlayerType = QLineEdit(self.groupBox4,"ledPlayerType")

        groupBox4Layout.addMultiCellWidget(self.ledPlayerType,3,3,0,1)

        self.groupBox9 = QGroupBox(self.groupBox4,"groupBox9")
        self.groupBox9.setColumnLayout(0,Qt.Vertical)
        self.groupBox9.layout().setSpacing(6)
        self.groupBox9.layout().setMargin(11)
        groupBox9Layout = QGridLayout(self.groupBox9.layout())
        groupBox9Layout.setAlignment(Qt.AlignTop)

        self.ledGeneral = QLineEdit(self.groupBox9,"ledGeneral")

        groupBox9Layout.addMultiCellWidget(self.ledGeneral,0,0,0,3)

        self.btSetCPlaneInterval = QPushButton(self.groupBox9,"btSetCPlaneInterval")

        groupBox9Layout.addWidget(self.btSetCPlaneInterval,1,0)

        self.btBlah = QPushButton(self.groupBox9,"btBlah")

        groupBox9Layout.addWidget(self.btBlah,1,1)

        self.btBlah_2 = QPushButton(self.groupBox9,"btBlah_2")

        groupBox9Layout.addWidget(self.btBlah_2,1,2)

        self.btBlah_3 = QPushButton(self.groupBox9,"btBlah_3")

        groupBox9Layout.addWidget(self.btBlah_3,1,3)

        groupBox4Layout.addMultiCellWidget(self.groupBox9,4,4,0,2)

        tabLayout_3.addWidget(self.groupBox4,0,1)
        self.tabWidget2.insertTab(self.tab_3,QString.fromLatin1(""))

        self.tab_4 = QWidget(self.tabWidget2,"tab_4")
        tabLayout_4 = QGridLayout(self.tab_4,1,1,11,6,"tabLayout_4")

        self.lcdPlayerNo = QLCDNumber(self.tab_4,"lcdPlayerNo")
        self.lcdPlayerNo.setSegmentStyle(QLCDNumber.Outline)
        self.lcdPlayerNo.setProperty("value",QVariant(9))
        self.lcdPlayerNo.setProperty("intValue",QVariant(9))

        tabLayout_4.addWidget(self.lcdPlayerNo,1,0)

        self.groupBox7 = QGroupBox(self.tab_4,"groupBox7")
        self.groupBox7.setColumnLayout(0,Qt.Vertical)
        self.groupBox7.layout().setSpacing(6)
        self.groupBox7.layout().setMargin(11)
        groupBox7Layout = QGridLayout(self.groupBox7.layout())
        groupBox7Layout.setAlignment(Qt.AlignTop)

        self.ledFilePath = QLineEdit(self.groupBox7,"ledFilePath")

        groupBox7Layout.addMultiCellWidget(self.ledFilePath,0,0,0,4)

        self.btPythonOpenDir = QPushButton(self.groupBox7,"btPythonOpenDir")

        groupBox7Layout.addMultiCellWidget(self.btPythonOpenDir,0,0,5,6)

        self.textLabel1_4 = QLabel(self.groupBox7,"textLabel1_4")

        groupBox7Layout.addMultiCellWidget(self.textLabel1_4,1,1,0,1)

        self.lviewPythonFiles = QListView(self.groupBox7,"lviewPythonFiles")
        self.lviewPythonFiles.addColumn(self.__tr("File names"))
        self.lviewPythonFiles.addColumn(self.__tr("Status"))
        self.lviewPythonFiles.addColumn(self.__tr("Size"))

        groupBox7Layout.addMultiCellWidget(self.lviewPythonFiles,2,2,0,6)

        self.btPythonUploadAll = QPushButton(self.groupBox7,"btPythonUploadAll")

        groupBox7Layout.addWidget(self.btPythonUploadAll,3,6)

        self.btPythonUpload = QPushButton(self.groupBox7,"btPythonUpload")

        groupBox7Layout.addMultiCellWidget(self.btPythonUpload,3,3,4,5)

        self.ledDogIp = QLineEdit(self.groupBox7,"ledDogIp")

        groupBox7Layout.addWidget(self.ledDogIp,3,3)

        self.btPythonRefresh = QPushButton(self.groupBox7,"btPythonRefresh")

        groupBox7Layout.addWidget(self.btPythonRefresh,3,0)

        self.textLabel2_3 = QLabel(self.groupBox7,"textLabel2_3")

        groupBox7Layout.addWidget(self.textLabel2_3,3,2)
        spacer4_2 = QSpacerItem(61,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        groupBox7Layout.addItem(spacer4_2,3,1)

        tabLayout_4.addMultiCellWidget(self.groupBox7,0,0,0,1)

        self.btPythonReload = QPushButton(self.tab_4,"btPythonReload")

        tabLayout_4.addWidget(self.btPythonReload,1,1)
        self.tabWidget2.insertTab(self.tab_4,QString.fromLatin1(""))

        self.tab_5 = QWidget(self.tabWidget2,"tab_5")
        tabLayout_5 = QGridLayout(self.tab_5,1,1,11,6,"tabLayout_5")

        self.groupBox14 = QGroupBox(self.tab_5,"groupBox14")
        self.groupBox14.setColumnLayout(0,Qt.Vertical)
        self.groupBox14.layout().setSpacing(6)
        self.groupBox14.layout().setMargin(11)
        groupBox14Layout = QGridLayout(self.groupBox14.layout())
        groupBox14Layout.setAlignment(Qt.AlignTop)

        self.btDebugClear = QPushButton(self.groupBox14,"btDebugClear")

        groupBox14Layout.addMultiCellWidget(self.btDebugClear,4,4,0,2)

        self.textLabel1_5 = QLabel(self.groupBox14,"textLabel1_5")
        self.textLabel1_5.setSizePolicy(QSizePolicy(QSizePolicy.Fixed,QSizePolicy.Fixed,0,0,self.textLabel1_5.sizePolicy().hasHeightForWidth()))

        groupBox14Layout.addWidget(self.textLabel1_5,0,0)

        self.btDebugEvaluate = QPushButton(self.groupBox14,"btDebugEvaluate")

        groupBox14Layout.addMultiCellWidget(self.btDebugEvaluate,1,1,0,1)

        self.btDebugExecute = QPushButton(self.groupBox14,"btDebugExecute")

        groupBox14Layout.addWidget(self.btDebugExecute,1,2)

        self.cbxDebugExpression = QComboBox(0,self.groupBox14,"cbxDebugExpression")
        self.cbxDebugExpression.setEditable(1)
        self.cbxDebugExpression.setAutoCompletion(0)
        self.cbxDebugExpression.setDuplicatesEnabled(0)

        groupBox14Layout.addMultiCellWidget(self.cbxDebugExpression,0,0,1,2)

        self.tedDebugResult = QTextEdit(self.groupBox14,"tedDebugResult")
        self.tedDebugResult.setTabChangesFocus(1)

        groupBox14Layout.addMultiCellWidget(self.tedDebugResult,3,3,0,2)

        self.textLabel1_6 = QLabel(self.groupBox14,"textLabel1_6")

        groupBox14Layout.addMultiCellWidget(self.textLabel1_6,2,2,0,2)

        tabLayout_5.addWidget(self.groupBox14,0,0)

        self.groupBox4_2 = QGroupBox(self.tab_5,"groupBox4_2")
        self.groupBox4_2.setColumnLayout(0,Qt.Vertical)
        self.groupBox4_2.layout().setSpacing(6)
        self.groupBox4_2.layout().setMargin(11)
        groupBox4_2Layout = QGridLayout(self.groupBox4_2.layout())
        groupBox4_2Layout.setAlignment(Qt.AlignTop)
        spacer1_3 = QSpacerItem(250,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        groupBox4_2Layout.addItem(spacer1_3,0,2)

        self.btDebuggingFlag = QPushButton(self.groupBox4_2,"btDebuggingFlag")

        groupBox4_2Layout.addWidget(self.btDebuggingFlag,0,1)

        self.cbxDebuggingFlag = QComboBox(0,self.groupBox4_2,"cbxDebuggingFlag")

        groupBox4_2Layout.addWidget(self.cbxDebuggingFlag,0,0)

        self.tedLogger = QTextEdit(self.groupBox4_2,"tedLogger")
        self.tedLogger.setTabChangesFocus(1)

        groupBox4_2Layout.addMultiCellWidget(self.tedLogger,1,1,0,2)

        tabLayout_5.addWidget(self.groupBox4_2,1,0)
        self.tabWidget2.insertTab(self.tab_5,QString.fromLatin1(""))

        self.tab_6 = QWidget(self.tabWidget2,"tab_6")
        tabLayout_6 = QGridLayout(self.tab_6,1,1,11,6,"tabLayout_6")

        self.groupBox10 = QGroupBox(self.tab_6,"groupBox10")
        self.groupBox10.setEnabled(1)
        self.groupBox10.setColumnLayout(0,Qt.Vertical)
        self.groupBox10.layout().setSpacing(6)
        self.groupBox10.layout().setMargin(11)
        groupBox10Layout = QGridLayout(self.groupBox10.layout())
        groupBox10Layout.setAlignment(Qt.AlignTop)

        self.ledFTPRemoteDir = QLineEdit(self.groupBox10,"ledFTPRemoteDir")

        groupBox10Layout.addMultiCellWidget(self.ledFTPRemoteDir,1,1,1,4)

        self.textLabel2_5 = QLabel(self.groupBox10,"textLabel2_5")

        groupBox10Layout.addWidget(self.textLabel2_5,1,0)

        self.textLabel2_5_2 = QLabel(self.groupBox10,"textLabel2_5_2")

        groupBox10Layout.addWidget(self.textLabel2_5_2,0,0)

        self.ledFTPLocalDir = QLineEdit(self.groupBox10,"ledFTPLocalDir")

        groupBox10Layout.addMultiCellWidget(self.ledFTPLocalDir,0,0,1,4)

        self.spbFTPToIndex = QSpinBox(self.groupBox10,"spbFTPToIndex")
        self.spbFTPToIndex.setMaxValue(999)

        groupBox10Layout.addWidget(self.spbFTPToIndex,2,4)

        self.textLabel1_7_2 = QLabel(self.groupBox10,"textLabel1_7_2")

        groupBox10Layout.addWidget(self.textLabel1_7_2,2,3)

        self.spbFTPFromIndex = QSpinBox(self.groupBox10,"spbFTPFromIndex")
        self.spbFTPFromIndex.setMaxValue(999)

        groupBox10Layout.addWidget(self.spbFTPFromIndex,2,2)

        self.textLabel1_7 = QLabel(self.groupBox10,"textLabel1_7")

        groupBox10Layout.addWidget(self.textLabel1_7,2,1)

        self.btFTPDeleteYUVFiles = QPushButton(self.groupBox10,"btFTPDeleteYUVFiles")

        groupBox10Layout.addWidget(self.btFTPDeleteYUVFiles,2,5)

        self.btFTPOpenLocalDir = QPushButton(self.groupBox10,"btFTPOpenLocalDir")

        groupBox10Layout.addWidget(self.btFTPOpenLocalDir,0,5)

        self.btFTPList = QPushButton(self.groupBox10,"btFTPList")

        groupBox10Layout.addWidget(self.btFTPList,1,5)

        self.btFTPUpload = QPushButton(self.groupBox10,"btFTPUpload")

        groupBox10Layout.addWidget(self.btFTPUpload,0,6)

        self.btFTPDownload = QPushButton(self.groupBox10,"btFTPDownload")

        groupBox10Layout.addWidget(self.btFTPDownload,1,6)

        self.groupBox11 = QGroupBox(self.groupBox10,"groupBox11")
        self.groupBox11.setEnabled(1)
        self.groupBox11.setSizePolicy(QSizePolicy(QSizePolicy.Preferred,QSizePolicy.Maximum,0,0,self.groupBox11.sizePolicy().hasHeightForWidth()))
        self.groupBox11.setColumnLayout(0,Qt.Vertical)
        self.groupBox11.layout().setSpacing(6)
        self.groupBox11.layout().setMargin(11)
        groupBox11Layout = QGridLayout(self.groupBox11.layout())
        groupBox11Layout.setAlignment(Qt.AlignTop)

        self.splitter4 = QSplitter(self.groupBox11,"splitter4")
        self.splitter4.setOrientation(QSplitter.Horizontal)

        LayoutWidget = QWidget(self.splitter4,"layout7")
        layout7 = QVBoxLayout(LayoutWidget,11,6,"layout7")

        self.textLabel3 = QLabel(LayoutWidget,"textLabel3")
        layout7.addWidget(self.textLabel3)

        self.tedFTPRemote = QTextEdit(LayoutWidget,"tedFTPRemote")
        self.tedFTPRemote.setSizePolicy(QSizePolicy(QSizePolicy.Preferred,QSizePolicy.Expanding,0,0,self.tedFTPRemote.sizePolicy().hasHeightForWidth()))
        self.tedFTPRemote.setWordWrap(QTextEdit.FixedColumnWidth)
        layout7.addWidget(self.tedFTPRemote)

        LayoutWidget_2 = QWidget(self.splitter4,"layout6")
        layout6 = QVBoxLayout(LayoutWidget_2,11,6,"layout6")

        self.textLabel3_2 = QLabel(LayoutWidget_2,"textLabel3_2")
        layout6.addWidget(self.textLabel3_2)

        self.lviewFTPLocal = QListView(LayoutWidget_2,"lviewFTPLocal")
        self.lviewFTPLocal.addColumn(self.__tr("Filename"))
        self.lviewFTPLocal.addColumn(self.__tr("Size"))
        self.lviewFTPLocal.setSizePolicy(QSizePolicy(QSizePolicy.Expanding,QSizePolicy.Expanding,0,0,self.lviewFTPLocal.sizePolicy().hasHeightForWidth()))
        self.lviewFTPLocal.setBackgroundOrigin(QListView.WidgetOrigin)
        self.lviewFTPLocal.setFrameShape(QListView.MenuBarPanel)
        layout6.addWidget(self.lviewFTPLocal)

        groupBox11Layout.addWidget(self.splitter4,0,0)

        groupBox10Layout.addMultiCellWidget(self.groupBox11,3,3,0,6)

        tabLayout_6.addWidget(self.groupBox10,0,0)
        self.tabWidget2.insertTab(self.tab_6,QString.fromLatin1(""))

        FormClientServerLayout.addWidget(self.tabWidget2,0,0)

        self.languageChange()

        self.resize(QSize(647,565).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.btConnect,SIGNAL("clicked()"),self.slotConnect)
        self.connect(self.btCPlaneDisplay,SIGNAL("pressed()"),self.slotShowHideCPlaneDisplay)
        self.connect(self.btDebugClear,SIGNAL("pressed()"),self.tedDebugResult.clear)
        self.connect(self.btDebugEvaluate,SIGNAL("pressed()"),self.slotDebugEvaluateExpression)
        self.connect(self.btDebugExecute,SIGNAL("clicked()"),self.slotDebugExecuteExpression)
        self.connect(self.btDebuggingFlag,SIGNAL("clicked()"),self.slotDebugSetDBFlag)
        self.connect(self.btDisconnectClient,SIGNAL("clicked()"),self.slotDisconnectClient)
        self.connect(self.btDisconnectServer,SIGNAL("clicked()"),self.slotDisconnectServer)
        self.connect(self.btHelpMe,SIGNAL("clicked()"),self.slotShowHelp)
        self.connect(self.btJointDebugger,SIGNAL("pressed()"),self.slotShowHideJointDebugger)
        self.connect(self.btListen,SIGNAL("clicked()"),self.slotListen)
        self.connect(self.btPythonOpenDir,SIGNAL("clicked()"),self.slotPythonOpenDir)
        self.connect(self.btPythonRefresh,SIGNAL("clicked()"),self.slotPythonRefresh)
        self.connect(self.btPythonReload,SIGNAL("pressed()"),self.slotPythonReload)
        self.connect(self.btPythonUpload,SIGNAL("pressed()"),self.slotPythonUpload)
        self.connect(self.btPythonUploadAll,SIGNAL("clicked()"),self.slotPythonUploadAll)
        self.connect(self.btSend,SIGNAL("clicked()"),self.slotSend)
        self.connect(self.btServerSend,SIGNAL("clicked()"),self.slotServerSend)
        self.connect(self.btSetCPlaneInterval,SIGNAL("clicked()"),self.slotToolSetCPlaneInterval)
        self.connect(self.btTelnetClearOutput,SIGNAL("clicked()"),self.txtBoard.clear)
        self.connect(self.btTelnetSaveAndClear,SIGNAL("clicked()"),self.slotTelnetSaveAndClear)
        self.connect(self.btToolDown,SIGNAL("clicked()"),self.slotToolMoveDown)
        self.connect(self.btToolHeadCommand,SIGNAL("clicked()"),self.slotToolHeadCommand)
        self.connect(self.btToolLeft,SIGNAL("clicked()"),self.slotToolMoveLeft)
        self.connect(self.btToolLeftTurn,SIGNAL("clicked()"),self.slotToolMoveLeftTurn)
        self.connect(self.btToolRelaxDog,SIGNAL("clicked()"),self.slotToolRelaxDog)
        self.connect(self.btToolReloadPython,SIGNAL("pressed()"),self.btPythonReload.animateClick)
        self.connect(self.btToolRightTurn,SIGNAL("clicked()"),self.slotToolMoveRightTurn)
        self.connect(self.btToolSendCPlane,SIGNAL("clicked()"),self.slotToolSendCPlane)
        self.connect(self.btToolSendWorldModel,SIGNAL("clicked()"),self.slotToolSendWorldModel)
        self.connect(self.btToolSendYUV,SIGNAL("clicked()"),self.slotToolSendYUV)
        self.connect(self.btToolSetGain,SIGNAL("clicked()"),self.slotToolSetGain)
        self.connect(self.btToolSetPlayer,SIGNAL("clicked()"),self.slotToolSetPlayer)
        self.connect(self.btToolStartStop,SIGNAL("clicked()"),self.slotToolStartStopRobot)
        self.connect(self.btToolTurnOff,SIGNAL("clicked()"),self.slotToolTurnOff)
        self.connect(self.btToolUp,SIGNAL("clicked()"),self.slotToolMoveUp)
        self.connect(self.btToolWalkCommand,SIGNAL("clicked()"),self.slotToolWalkCommand)
        self.connect(self.btWalkLearner,SIGNAL("pressed()"),self.slotShowHideWalkLearner)
        self.connect(self.btWorldModelDisplay,SIGNAL("pressed()"),self.slotShowHideWorldModelDisplay)
        self.connect(self.cbxCommand,SIGNAL("activated(const QString&)"),self.slotCommandTypeChanged)
        self.connect(self.cbxDebuggingFlag,SIGNAL("activated(const QString&)"),self.slotDebugChangeDBFlag)
        self.connect(self.cbxPlayerNumber,SIGNAL("activated(const QString&)"),self.lcdPlayerNo.display)
        self.connect(self.ledBufferMaxLines,SIGNAL("textChanged(const QString&)"),self.slotTelnetChangeBufferSize)
        self.connect(self.ledFilePath,SIGNAL("returnPressed()"),self.slotPythonListFile)
        self.connect(self.ledHeadCommand,SIGNAL("returnPressed()"),self.btToolHeadCommand.animateClick)
        self.connect(self.ledServerSend,SIGNAL("returnPressed()"),self.slotServerSend)
        self.connect(self.ledWalkCommand,SIGNAL("returnPressed()"),self.btToolWalkCommand.animateClick)
        self.connect(self.lviewPythonFiles,SIGNAL("clicked(QListViewItem*)"),self.slotPythonRefresh)
        self.connect(self.lviewPythonFiles,SIGNAL("doubleClicked(QListViewItem*)"),self.slotPythonSetModified)
        self.connect(self.btToolPauseResume,SIGNAL("pressed()"),self.slotToolPauseResume)
        self.connect(self.btFTPList,SIGNAL("pressed()"),self.slotFTPList)
        self.connect(self.btFTPUpload,SIGNAL("pressed()"),self.slotFTPUpload)
        self.connect(self.btFTPDownload,SIGNAL("pressed()"),self.slotFTPDownload)
        self.connect(self.btFTPDeleteYUVFiles,SIGNAL("pressed()"),self.slotFTPDeleteYUVFiles)
        self.connect(self.btFTPOpenLocalDir,SIGNAL("pressed()"),self.slotFTPOpenDir)
        self.connect(self.btShowCalibrater,SIGNAL("clicked()"),self.slotShowCalibraterInfo)
        self.connect(self.btJoystickController,SIGNAL("clicked()"),self.slotShowHideJoystickController)
        self.connect(self.btToolStop,SIGNAL("clicked()"),self.slotToolStop)
        self.connect(self.btToolRight,SIGNAL("clicked()"),self.slotToolMoveRight)

        self.setTabOrder(self.cbxCommand,self.ledServerSend)
        self.setTabOrder(self.ledServerSend,self.btServerSend)
        self.setTabOrder(self.btServerSend,self.txtServerBoard)
        self.setTabOrder(self.txtServerBoard,self.ledLocalIP)
        self.setTabOrder(self.ledLocalIP,self.btListen)
        self.setTabOrder(self.btListen,self.ledLocalPort)
        self.setTabOrder(self.ledLocalPort,self.btDisconnectServer)
        self.setTabOrder(self.btDisconnectServer,self.btCPlaneDisplay)
        self.setTabOrder(self.btCPlaneDisplay,self.btJointDebugger)
        self.setTabOrder(self.btJointDebugger,self.btWalkLearner)
        self.setTabOrder(self.btWalkLearner,self.tabWidget2)
        self.setTabOrder(self.tabWidget2,self.ledRemoteIP)
        self.setTabOrder(self.ledRemoteIP,self.ledRemotePort)
        self.setTabOrder(self.ledRemotePort,self.btConnect)
        self.setTabOrder(self.btConnect,self.ledSend)
        self.setTabOrder(self.ledSend,self.btSend)
        self.setTabOrder(self.btSend,self.txtBoard)
        self.setTabOrder(self.txtBoard,self.ledBufferMaxLines)
        self.setTabOrder(self.ledBufferMaxLines,self.btTelnetClearOutput)
        self.setTabOrder(self.btTelnetClearOutput,self.btTelnetSaveAndClear)
        self.setTabOrder(self.btTelnetSaveAndClear,self.btDisconnectClient)
        self.setTabOrder(self.btDisconnectClient,self.btToolSendCPlane)
        self.setTabOrder(self.btToolSendCPlane,self.btToolStartStop)
        self.setTabOrder(self.btToolStartStop,self.btToolRelaxDog)
        self.setTabOrder(self.btToolRelaxDog,self.btToolSendYUV)
        self.setTabOrder(self.btToolSendYUV,self.btToolSendWorldModel)
        self.setTabOrder(self.btToolSendWorldModel,self.btToolTurnOff)
        self.setTabOrder(self.btToolTurnOff,self.btToolReloadPython)
        self.setTabOrder(self.btToolReloadPython,self.cbxPlayerNumber)
        self.setTabOrder(self.cbxPlayerNumber,self.cbxWalkType)
        self.setTabOrder(self.cbxWalkType,self.ledWalkCommand)
        self.setTabOrder(self.ledWalkCommand,self.btToolWalkCommand)
        self.setTabOrder(self.btToolWalkCommand,self.ledHeadCommand)
        self.setTabOrder(self.ledHeadCommand,self.btToolHeadCommand)
        self.setTabOrder(self.btToolHeadCommand,self.ledPlayerType)
        self.setTabOrder(self.ledPlayerType,self.btToolSetPlayer)
        self.setTabOrder(self.btToolSetPlayer,self.btToolLeftTurn)
        self.setTabOrder(self.btToolLeftTurn,self.btToolUp)
        self.setTabOrder(self.btToolUp,self.btToolRightTurn)
        self.setTabOrder(self.btToolRightTurn,self.btToolLeft)
        self.setTabOrder(self.btToolLeft,self.btToolRight)
        self.setTabOrder(self.btToolRight,self.btToolDown)
        self.setTabOrder(self.btToolDown,self.ledFilePath)
        self.setTabOrder(self.ledFilePath,self.btPythonOpenDir)
        self.setTabOrder(self.btPythonOpenDir,self.lviewPythonFiles)
        self.setTabOrder(self.lviewPythonFiles,self.btPythonRefresh)
        self.setTabOrder(self.btPythonRefresh,self.ledDogIp)
        self.setTabOrder(self.ledDogIp,self.btPythonUpload)
        self.setTabOrder(self.btPythonUpload,self.btPythonUploadAll)
        self.setTabOrder(self.btPythonUploadAll,self.btPythonReload)
        self.setTabOrder(self.btPythonReload,self.cbxDebugExpression)
        self.setTabOrder(self.cbxDebugExpression,self.tedDebugResult)
        self.setTabOrder(self.tedDebugResult,self.btDebugClear)
        self.setTabOrder(self.btDebugClear,self.btDebugEvaluate)


    def languageChange(self):
        self.setCaption(self.__tr("Simple Robo-Commander (c) rUNSWift 2004"))
        self.groupBox2.setTitle(self.__tr("Commander"))
        self.ledLocalPort.setText(self.__tr("5005"))
        self.textLabel1.setText(self.__tr("Port"))
        self.textLabel1_2.setText(self.__tr("IP"))
        self.btDisconnectServer.setText(self.__tr("Disconnect"))
        self.btListen.setText(self.__tr("Listen"))
        self.btServerSend.setText(self.__tr("&Send"))
        self.cbxCommand.clear()
        self.cbxCommand.insertItem(QString.null)
        self.cbxCommand.insertItem(self.__tr("wrl"))
        self.cbxCommand.insertItem(self.__tr("mode"))
        self.cbxCommand.insertItem(self.__tr("wt"))
        self.cbxCommand.insertItem(self.__tr("head"))
        self.cbxCommand.insertItem(self.__tr("py"))
        self.cbxCommand.insertItem(self.__tr("wtcal"))
        self.cbxCommand.insertItem(self.__tr("f"))
        self.cbxCommand.insertItem(self.__tr("pyc"))
        self.groupBox3.setTitle(self.__tr("Control Panel"))
        self.btJointDebugger.setText(self.__tr("Show &Joint Debugger"))
        self.btJointDebugger.setAccel(self.__tr("Alt+J"))
        self.btCPlaneDisplay.setText(self.__tr("Display &CPlane"))
        self.btCPlaneDisplay.setAccel(self.__tr("Alt+C"))
        self.btWorldModelDisplay.setText(self.__tr("Display WorldModel"))
        self.btWorldModelDisplay.setAccel(QString.null)
        self.btWalkLearner.setText(self.__tr("Show &Walk Learner"))
        self.btWalkLearner.setAccel(self.__tr("Alt+W"))
        self.btShowCalibrater.setText(self.__tr("Calibrater Info"))
        self.btHelpMe.setText(self.__tr("Help"))
        self.btJoystickController.setText(self.__tr("&Show Joystick Controller"))
        self.tabWidget2.changeTab(self.tab,self.__tr("&Commands"))
        self.btConnect.setText(self.__tr("Connect"))
        self.textLabel2.setText(self.__tr("IP"))
        self.ledRemoteIP.setText(self.__tr("192.168.0."))
        self.textLabel2_2.setText(self.__tr("Port"))
        self.ledRemotePort.setText(self.__tr("59000"))
        self.ledSend.setText(QString.null)
        self.btSend.setText(self.__tr("Send"))
        self.btTelnetClearOutput.setText(self.__tr("&Clear output"))
        self.btTelnetSaveAndClear.setText(self.__tr("&Save + Clear"))
        self.btDisconnectClient.setText(self.__tr("Disconnect"))
        self.ledBufferMaxLines.setText(self.__tr("100"))
        QToolTip.add(self.ledBufferMaxLines,self.__tr("0 means UNLIMITED"))
        self.textLabel2_4.setText(self.__tr("Buffer Size"))
        self.tabWidget2.changeTab(self.tab_2,self.__tr("&Telnet"))
        self.groupBox3_2.setTitle(self.__tr("Simple"))
        self.btToolStartStop.setText(self.__tr("&Start Robot (mode)"))
        self.btToolRelaxDog.setText(self.__tr("&Relax dog"))
        self.btToolSendYUV.setText(self.__tr("Send &YUV"))
        self.btToolTurnOff.setText(self.__tr("Turn Off"))
        self.btToolReloadPython.setText(self.__tr("Reload &Python"))
        self.btToolSetGain.setText(self.__tr("Set &High Gain"))
        self.btToolSendWorldModel.setText(self.__tr("Send &World Model"))
        QToolTip.add(self.btToolSendWorldModel,self.__tr("Hold Ctrl to send ALL world model. Select player number to send individual world model. Player number 0 -> stop sending world models."))
        self.btToolSendCPlane.setText(self.__tr("Send &CPlane"))
        self.btToolPauseResume.setText(self.__tr("&Pause"))
        self.groupBox4.setTitle(self.__tr("Complex"))
        self.btToolHeadCommand.setText(self.__tr("&Head Command"))
        self.btToolWalkCommand.setText(self.__tr("&Walk Command"))
        self.btToolSetPlayer.setText(self.__tr("Set &Player Type"))
        self.cbxPlayerNumber.clear()
        self.cbxPlayerNumber.insertItem(self.__tr("9"))
        self.cbxPlayerNumber.insertItem(self.__tr("1"))
        self.cbxPlayerNumber.insertItem(self.__tr("0"))
        self.cbxPlayerNumber.insertItem(self.__tr("2"))
        self.cbxPlayerNumber.insertItem(self.__tr("3"))
        self.cbxPlayerNumber.insertItem(self.__tr("4"))
        self.textLabel1_3.setText(self.__tr("Send to player number : "))
        self.cbxWalkType.clear()
        self.cbxWalkType.insertItem(self.__tr("Normal Walk"))
        self.cbxWalkType.insertItem(self.__tr("1"))
        self.cbxWalkType.insertItem(self.__tr("2"))
        self.cbxWalkType.insertItem(self.__tr("3"))
        self.cbxWalkType.insertItem(self.__tr("Elliptical Walk"))
        self.cbxWalkType.insertItem(self.__tr("5"))
        self.cbxWalkType.insertItem(self.__tr("Chest Push"))
        self.cbxWalkType.insertItem(self.__tr("Forward Kick"))
        self.cbxWalkType.insertItem(self.__tr("Fast Kick"))
        self.cbxWalkType.insertItem(self.__tr("9"))
        self.cbxWalkType.insertItem(self.__tr("10"))
        self.cbxWalkType.insertItem(self.__tr("11"))
        self.cbxWalkType.insertItem(self.__tr("Back Kick"))
        self.cbxWalkType.insertItem(self.__tr("13"))
        self.cbxWalkType.insertItem(self.__tr("14"))
        self.cbxWalkType.insertItem(self.__tr("15"))
        self.cbxWalkType.insertItem(self.__tr("Upenn Right"))
        self.cbxWalkType.insertItem(self.__tr("Upenn Left"))
        self.cbxWalkType.insertItem(self.__tr("18"))
        self.cbxWalkType.insertItem(self.__tr("19"))
        self.cbxWalkType.insertItem(self.__tr("SkellipticalWalk"))
        self.cbxWalkType.insertItem(self.__tr("DiveKick"))
        self.cbxWalkType.insertItem(self.__tr("HandKickRight"))
        self.cbxWalkType.insertItem(self.__tr("HandKickLeft"))
        self.groupBox5.setTitle(self.__tr("\"Steering Wheel\""))
        self.btToolDown.setText(self.__tr("v"))
        self.btToolRightTurn.setText(self.__tr("/"))
        self.btToolLeftTurn.setText(self.__tr("\\"))
        self.btToolUp.setText(self.__tr("^"))
        self.btToolRight.setText(self.__tr(">"))
        self.btToolLeft.setText(self.__tr("<"))
        self.btToolStop.setText(self.__tr("STOP!"))
        self.groupBox9.setTitle(self.__tr("Misc"))
        self.btSetCPlaneInterval.setText(self.__tr("Set CPLANE_NTERVAL"))
        self.btBlah.setText(self.__tr("Blah"))
        self.btBlah_2.setText(self.__tr("Blah"))
        self.btBlah_3.setText(self.__tr("Blah"))
        self.tabWidget2.changeTab(self.tab_3,self.__tr("Too&ls"))
        self.groupBox7.setTitle(self.__tr("Python sources"))
        self.ledFilePath.setText(self.__tr("../../robot/PyCode"))
        self.btPythonOpenDir.setText(self.__tr("&Open Dir"))
        self.btPythonOpenDir.setAccel(self.__tr("Alt+O"))
        self.textLabel1_4.setText(self.__tr("Python files"))
        self.lviewPythonFiles.header().setLabel(0,self.__tr("File names"))
        self.lviewPythonFiles.header().setLabel(1,self.__tr("Status"))
        self.lviewPythonFiles.header().setLabel(2,self.__tr("Size"))
        QToolTip.add(self.lviewPythonFiles,self.__tr("Double click on file to mark as modified.\n"
"Single click to refresh status."))
        self.btPythonUploadAll.setText(self.__tr("Upload &All"))
        self.btPythonUpload.setText(self.__tr("&Upload"))
        self.ledDogIp.setText(self.__tr("192.168.0."))
        self.btPythonRefresh.setText(self.__tr("&Refresh"))
        self.btPythonRefresh.setAccel(self.__tr("Alt+R"))
        self.textLabel2_3.setText(self.__tr("IP"))
        self.btPythonReload.setText(self.__tr("&Reload Python module"))
        self.tabWidget2.changeTab(self.tab_4,self.__tr("&Python"))
        self.groupBox14.setTitle(self.__tr("Python Debugger"))
        self.btDebugClear.setText(self.__tr("&Clear"))
        self.textLabel1_5.setText(self.__tr("Expression"))
        self.btDebugEvaluate.setText(self.__tr("&Evaluate"))
        self.btDebugExecute.setText(self.__tr("e&Xecute"))
        self.btDebugExecute.setAccel(self.__tr("Alt+X"))
        self.textLabel1_6.setText(self.__tr("Evaluation Result"))
        self.groupBox4_2.setTitle(self.__tr("Base station Debugging"))
        self.btDebuggingFlag.setText(self.__tr("True"))
        self.cbxDebuggingFlag.clear()
        self.cbxDebuggingFlag.insertItem(self.__tr("bDebugMessaging"))
        self.cbxDebuggingFlag.insertItem(self.__tr("WalkLearner.bDebugThread"))
        self.tabWidget2.changeTab(self.tab_5,self.__tr("&Debug"))
        self.groupBox10.setTitle(self.__tr("Dog's IP : "))
        self.textLabel2_5.setText(self.__tr("Remote Directory"))
        self.textLabel2_5_2.setText(self.__tr("Local Directory"))
        self.textLabel1_7_2.setText(self.__tr("To"))
        self.textLabel1_7.setText(self.__tr("From"))
        self.btFTPDeleteYUVFiles.setText(self.__tr("Delete"))
        self.btFTPOpenLocalDir.setText(self.__tr("Open Dir"))
        self.btFTPList.setText(self.__tr("List"))
        self.btFTPUpload.setText(self.__tr("Upload"))
        self.btFTPDownload.setText(self.__tr("Download"))
        self.groupBox11.setTitle(self.__tr("File Explorer"))
        self.textLabel3.setText(self.__tr("Remote"))
        self.textLabel3_2.setText(self.__tr("Local"))
        self.lviewFTPLocal.header().setLabel(0,self.__tr("Filename"))
        self.lviewFTPLocal.header().setLabel(1,self.__tr("Size"))
        self.lviewFTPLocal.clear()
        item = QListViewItem(self.lviewFTPLocal,None)
        item.setText(0,self.__tr("Dummy"))
        item.setText(1,self.__tr("Dummy"))

        self.tabWidget2.changeTab(self.tab_6,self.__tr("FTP"))


    def slotListen(self):
        print "FormClientServer.slotListen(): Not implemented yet"

    def slotDisconnectServer(self):
        print "FormClientServer.slotDisconnectServer(): Not implemented yet"

    def slotDisconnectClient(self):
        print "FormClientServer.slotDisconnectClient(): Not implemented yet"

    def slotConnect(self):
        print "FormClientServer.slotConnect(): Not implemented yet"

    def slotSend(self):
        print "FormClientServer.slotSend(): Not implemented yet"

    def slotServerSend(self):
        print "FormClientServer.slotServerSend(): Not implemented yet"

    def slotCommandTypeChanged(self):
        print "FormClientServer.slotCommandTypeChanged(): Not implemented yet"

    def slotShowHideJointDebugger(self):
        print "FormClientServer.slotShowHideJointDebugger(): Not implemented yet"

    def slotShowHideWalkLearner(self):
        print "FormClientServer.slotShowHideWalkLearner(): Not implemented yet"

    def slotShowHideCPlaneDisplay(self):
        print "FormClientServer.slotShowHideCPlaneDisplay(): Not implemented yet"

    def slotToolSendCPlane(self):
        print "FormClientServer.slotToolSendCPlane(): Not implemented yet"

    def slotToolStartStopRobot(self):
        print "FormClientServer.slotToolStartStopRobot(): Not implemented yet"

    def slotToolWalkCommand(self):
        print "FormClientServer.slotToolWalkCommand(): Not implemented yet"

    def slotToolHeadCommand(self):
        print "FormClientServer.slotToolHeadCommand(): Not implemented yet"

    def slotToolRelaxDog(self):
        print "FormClientServer.slotToolRelaxDog(): Not implemented yet"

    def slotToolSendYUV(self):
        print "FormClientServer.slotToolSendYUV(): Not implemented yet"

    def slotToolSendWorldModel(self):
        print "FormClientServer.slotToolSendWorldModel(): Not implemented yet"

    def slotToolSetPlayer(self):
        print "FormClientServer.slotToolSetPlayer(): Not implemented yet"

    def slotToolMoveUp(self):
        print "FormClientServer.slotToolMoveUp(): Not implemented yet"

    def slotToolMoveDown(self):
        print "FormClientServer.slotToolMoveDown(): Not implemented yet"

    def slotToolMoveLeft(self):
        print "FormClientServer.slotToolMoveLeft(): Not implemented yet"

    def slotToolMoveRight(self):
        print "FormClientServer.slotToolMoveRight(): Not implemented yet"

    def slotToolMoveRightTurn(self):
        print "FormClientServer.slotToolMoveRightTurn(): Not implemented yet"

    def slotToolMoveLeftTurn(self):
        print "FormClientServer.slotToolMoveLeftTurn(): Not implemented yet"

    def slotPythonReload(self):
        print "FormClientServer.slotPythonReload(): Not implemented yet"

    def slotPythonOpenDir(self):
        print "FormClientServer.slotPythonOpenDir(): Not implemented yet"

    def slotPythonUpload(self):
        print "FormClientServer.slotPythonUpload(): Not implemented yet"

    def slotPythonShowFile(self):
        print "FormClientServer.slotPythonShowFile(): Not implemented yet"

    def slotToolTurnOff(self):
        print "FormClientServer.slotToolTurnOff(): Not implemented yet"

    def slotPythonUploadAll(self):
        print "FormClientServer.slotPythonUploadAll(): Not implemented yet"

    def slotPythonRefresh(self):
        print "FormClientServer.slotPythonRefresh(): Not implemented yet"

    def slotTelnetChangeBufferSize(self):
        print "FormClientServer.slotTelnetChangeBufferSize(): Not implemented yet"

    def slotTelnetSaveAndClear(self):
        print "FormClientServer.slotTelnetSaveAndClear(): Not implemented yet"

    def slotDebugEvaluateExpression(self):
        print "FormClientServer.slotDebugEvaluateExpression(): Not implemented yet"

    def slotShowHideWorldModelDisplay(self):
        print "FormClientServer.slotShowHideWorldModelDisplay(): Not implemented yet"

    def slotToolSetGain(self):
        print "FormClientServer.slotToolSetGain(): Not implemented yet"

    def slotDebugChangeDBFlag(self):
        print "FormClientServer.slotDebugChangeDBFlag(): Not implemented yet"

    def slotDebugSetDBFlag(self):
        print "FormClientServer.slotDebugSetDBFlag(): Not implemented yet"

    def slotDebugExecuteExpression(self):
        print "FormClientServer.slotDebugExecuteExpression(): Not implemented yet"

    def slotPythonSetModified(self):
        print "FormClientServer.slotPythonSetModified(): Not implemented yet"

    def slotPythonListFile(self):
        print "FormClientServer.slotPythonListFile(): Not implemented yet"

    def slotToolSetCPlaneInterval(self):
        print "FormClientServer.slotToolSetCPlaneInterval(): Not implemented yet"

    def slotShowHelp(self):
        print "FormClientServer.slotShowHelp(): Not implemented yet"

    def slotToolPauseResume(self):
        print "FormClientServer.slotToolPauseResume(): Not implemented yet"

    def slotFTPList(self):
        print "FormClientServer.slotFTPList(): Not implemented yet"

    def slotFTPUpload(self):
        print "FormClientServer.slotFTPUpload(): Not implemented yet"

    def slotFTPDownload(self):
        print "FormClientServer.slotFTPDownload(): Not implemented yet"

    def slotFTPDeleteYUVFiles(self):
        print "FormClientServer.slotFTPDeleteYUVFiles(): Not implemented yet"

    def slotFTPOpenDir(self):
        print "FormClientServer.slotFTPOpenDir(): Not implemented yet"

    def slotShowCalibraterInfo(self):
        print "FormClientServer.slotShowCalibraterInfo(): Not implemented yet"

    def slotShowHideJoystickController(self):
        print "FormClientServer.slotShowHideJoystickController(): Not implemented yet"

    def slotToolStop(self):
        print "FormClientServer.slotToolStop(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("FormClientServer",s,c)
