# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'frmVisPolicy.ui'
#
# Created: Thu Jun 9 16:49:25 2005
#      by: The PyQt User Interface Compiler (pyuic) 3.14.1
#
# WARNING! All changes made in this file will be lost!


from qt import *


class frmVisPolicy(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        if not name:
            self.setName("frmVisPolicy")



        self.tabWidget2 = QTabWidget(self,"tabWidget2")
        self.tabWidget2.setGeometry(QRect(429,11,411,614))
        self.tabWidget2.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.MinimumExpanding,0,0,self.tabWidget2.sizePolicy().hasHeightForWidth()))

        self.tab = QWidget(self.tabWidget2,"tab")

        self.groupBox7 = QGroupBox(self.tab,"groupBox7")
        self.groupBox7.setGeometry(QRect(12,230,391,50))
        self.groupBox7.setColumnLayout(0,Qt.Vertical)
        self.groupBox7.layout().setSpacing(6)
        self.groupBox7.layout().setMargin(11)
        groupBox7Layout = QVBoxLayout(self.groupBox7.layout())
        groupBox7Layout.setAlignment(Qt.AlignTop)

        self.chkShowMovement = QCheckBox(self.groupBox7,"chkShowMovement")
        self.chkShowMovement.setChecked(1)
        groupBox7Layout.addWidget(self.chkShowMovement)

        self.groupBox1 = QGroupBox(self.tab,"groupBox1")
        self.groupBox1.setGeometry(QRect(12,14,391,150))

        LayoutWidget = QWidget(self.groupBox1,"layout13")
        LayoutWidget.setGeometry(QRect(10,21,380,126))
        layout13 = QVBoxLayout(LayoutWidget,11,6,"layout13")

        self.cbxObjects = QComboBox(0,LayoutWidget,"cbxObjects")
        layout13.addWidget(self.cbxObjects)

        layout3 = QHBoxLayout(None,0,6,"layout3")

        self.textLabel2 = QLabel(LayoutWidget,"textLabel2")
        layout3.addWidget(self.textLabel2)

        self.sldPosX = QSlider(LayoutWidget,"sldPosX")
        self.sldPosX.setMinValue(-20)
        self.sldPosX.setMaxValue(380)
        self.sldPosX.setPageStep(10)
        self.sldPosX.setOrientation(QSlider.Horizontal)
        layout3.addWidget(self.sldPosX)

        self.lblPosX = QLabel(LayoutWidget,"lblPosX")
        layout3.addWidget(self.lblPosX)
        layout13.addLayout(layout3)

        layout3_2 = QHBoxLayout(None,0,6,"layout3_2")

        self.textLabel2_2 = QLabel(LayoutWidget,"textLabel2_2")
        layout3_2.addWidget(self.textLabel2_2)

        self.sldPosY = QSlider(LayoutWidget,"sldPosY")
        self.sldPosY.setMinValue(-30)
        self.sldPosY.setMaxValue(570)
        self.sldPosY.setPageStep(10)
        self.sldPosY.setOrientation(QSlider.Horizontal)
        layout3_2.addWidget(self.sldPosY)

        self.lblPosY = QLabel(LayoutWidget,"lblPosY")
        layout3_2.addWidget(self.lblPosY)
        layout13.addLayout(layout3_2)

        layout9 = QHBoxLayout(None,0,6,"layout9")

        self.textLabel1 = QLabel(LayoutWidget,"textLabel1")
        layout9.addWidget(self.textLabel1)

        self.sbNumObstacles = QSpinBox(LayoutWidget,"sbNumObstacles")
        self.sbNumObstacles.setMaxValue(50)
        layout9.addWidget(self.sbNumObstacles)
        layout13.addLayout(layout9)
        spacer4 = QSpacerItem(20,20,QSizePolicy.Minimum,QSizePolicy.Expanding)
        layout13.addItem(spacer4)

        LayoutWidget_2 = QWidget(self.tab,"layout14")
        LayoutWidget_2.setGeometry(QRect(12,163,390,420))
        layout14 = QVBoxLayout(LayoutWidget_2,11,6,"layout14")

        self.groupBox2 = QGroupBox(LayoutWidget_2,"groupBox2")
        self.groupBox2.setColumnLayout(0,Qt.Vertical)
        self.groupBox2.layout().setSpacing(6)
        self.groupBox2.layout().setMargin(11)
        groupBox2Layout = QHBoxLayout(self.groupBox2.layout())
        groupBox2Layout.setAlignment(Qt.AlignTop)

        self.lblZoomRatio = QLabel(self.groupBox2,"lblZoomRatio")
        groupBox2Layout.addWidget(self.lblZoomRatio)
        spacer3 = QSpacerItem(30,21,QSizePolicy.Expanding,QSizePolicy.Minimum)
        groupBox2Layout.addItem(spacer3)

        self.btnZoomIn = QPushButton(self.groupBox2,"btnZoomIn")
        groupBox2Layout.addWidget(self.btnZoomIn)

        self.btnZoomReset = QPushButton(self.groupBox2,"btnZoomReset")
        groupBox2Layout.addWidget(self.btnZoomReset)

        self.btnZoomOut = QPushButton(self.groupBox2,"btnZoomOut")
        groupBox2Layout.addWidget(self.btnZoomOut)
        layout14.addWidget(self.groupBox2)
        spacer1 = QSpacerItem(31,250,QSizePolicy.Minimum,QSizePolicy.Expanding)
        layout14.addItem(spacer1)

        layout7 = QHBoxLayout(None,0,6,"layout7")

        self.comboPlayer = QComboBox(0,LayoutWidget_2,"comboPlayer")
        self.comboPlayer.setDuplicatesEnabled(0)
        layout7.addWidget(self.comboPlayer)

        self.btnReloadPlayer = QPushButton(LayoutWidget_2,"btnReloadPlayer")
        layout7.addWidget(self.btnReloadPlayer)

        self.btnRunPlayer = QPushButton(LayoutWidget_2,"btnRunPlayer")
        self.btnRunPlayer.setDefault(1)
        layout7.addWidget(self.btnRunPlayer)
        spacer2 = QSpacerItem(103,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout7.addItem(spacer2)
        layout14.addLayout(layout7)
        self.tabWidget2.insertTab(self.tab,QString.fromLatin1(""))

        self.tab_2 = QWidget(self.tabWidget2,"tab_2")
        self.tabWidget2.insertTab(self.tab_2,QString.fromLatin1(""))

        self.TabPage = QWidget(self.tabWidget2,"TabPage")
        self.tabWidget2.insertTab(self.TabPage,QString.fromLatin1(""))

        self.frameField = QFrame(self,"frameField")
        self.frameField.setGeometry(QRect(11,11,412,614))
        self.frameField.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding,QSizePolicy.MinimumExpanding,0,0,self.frameField.sizePolicy().hasHeightForWidth()))
        self.frameField.setFrameShape(QFrame.Box)
        self.frameField.setFrameShadow(QFrame.Raised)

        self.languageChange()

        self.resize(QSize(851,636).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.sldPosX,SIGNAL("valueChanged(int)"),self.lblPosX.setNum)
        self.connect(self.sldPosY,SIGNAL("valueChanged(int)"),self.lblPosY.setNum)
        self.connect(self.cbxObjects,SIGNAL("activated(int)"),self.slotObjectSelected)
        self.connect(self.sldPosX,SIGNAL("valueChanged(int)"),self.slotSetPosX)
        self.connect(self.sldPosY,SIGNAL("valueChanged(int)"),self.slotSetPosY)
        self.connect(self.btnReloadPlayer,SIGNAL("clicked()"),self.slotReloadPlayer)
        self.connect(self.btnZoomOut,SIGNAL("clicked()"),self.slotZoomOut)
        self.connect(self.btnZoomIn,SIGNAL("clicked()"),self.slotZoomIn)
        self.connect(self.btnRunPlayer,SIGNAL("clicked()"),self.slotRunPlayer)
        self.connect(self.chkShowMovement,SIGNAL("toggled(bool)"),self.slotCheckShowMovement)
        self.connect(self.comboPlayer,SIGNAL("activated(int)"),self.slotReloadPlayer)
        self.connect(self.sbNumObstacles,SIGNAL("valueChanged(int)"),self.slotSetNumObstacles)


    def languageChange(self):
        self.setCaption(self.__tr("Offline Behaviours"))
        self.groupBox7.setTitle(self.__tr("Show Options: "))
        self.chkShowMovement.setText(self.__tr("Show Movement"))
        self.groupBox1.setTitle(self.__tr("Mobile Objects: "))
        self.cbxObjects.clear()
        self.cbxObjects.insertItem(self.__tr("Ball"))
        self.cbxObjects.insertItem(self.__tr("Player"))
        self.cbxObjects.insertItem(self.__tr("Teammate 1 (Goalie)"))
        self.cbxObjects.insertItem(self.__tr("Teammate 3"))
        self.cbxObjects.insertItem(self.__tr("Teammate 4"))
        self.cbxObjects.insertItem(self.__tr("Opponent 1 (Goalie)"))
        self.cbxObjects.insertItem(self.__tr("Opponent 2"))
        self.cbxObjects.insertItem(self.__tr("Opponent 3"))
        self.cbxObjects.insertItem(self.__tr("Opponent 4"))
        self.textLabel2.setText(self.__tr("X:"))
        self.lblPosX.setText(self.__tr("0"))
        self.textLabel2_2.setText(self.__tr("Y:"))
        self.lblPosY.setText(self.__tr("0"))
        self.textLabel1.setText(self.__tr("Number of obstacles"))
        self.groupBox2.setTitle(self.__tr("Field Zoom: "))
        self.lblZoomRatio.setText(self.__tr("1.0x"))
        self.btnZoomIn.setText(self.__tr("Zoom In"))
        self.btnZoomReset.setText(self.__tr("Reset"))
        self.btnZoomOut.setText(self.__tr("Zoom Out"))
        self.btnReloadPlayer.setText(self.__tr("Reload Player"))
        self.btnRunPlayer.setText(self.__tr("Run"))
        self.tabWidget2.changeTab(self.tab,self.__tr("Main"))
        self.tabWidget2.changeTab(self.tab_2,self.__tr("VisionLink Inputs"))
        self.tabWidget2.changeTab(self.TabPage,self.__tr("Outputs"))


    def slotObjectSelected(self,a0):
        print "frmVisPolicy.slotObjectSelected(int): Not implemented yet"

    def slotSetPosX(self,a0):
        print "frmVisPolicy.slotSetPosX(int): Not implemented yet"

    def slotSetPosY(self,a0):
        print "frmVisPolicy.slotSetPosY(int): Not implemented yet"

    def slotReloadPlayer(self):
        print "frmVisPolicy.slotReloadPlayer(): Not implemented yet"

    def slotZoomOut(self):
        print "frmVisPolicy.slotZoomOut(): Not implemented yet"

    def slotZoomIn(self):
        print "frmVisPolicy.slotZoomIn(): Not implemented yet"

    def slotRunPlayer(self):
        print "frmVisPolicy.slotRunPlayer(): Not implemented yet"

    def slotCheckShowMovement(self,a0):
        print "frmVisPolicy.slotCheckShowMovement(bool): Not implemented yet"

    def slotZoomReset(self):
        print "frmVisPolicy.slotZoomReset(): Not implemented yet"

    def slotSelectPlayer(self):
        print "frmVisPolicy.slotSelectPlayer(): Not implemented yet"

    def slotSetNumObstacles(self,a0):
        print "frmVisPolicy.slotSetNumObstacles(int): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("frmVisPolicy",s,c)
