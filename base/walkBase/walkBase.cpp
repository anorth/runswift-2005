/*
   Copyright 2005 The University of New South Wales (UNSW) and National  
   ICT Australia (NICTA).

   This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
   redistribute it and/or modify it under the terms of the GNU General  
   Public License as published by the Free Software Foundation; either  
   version 2 of the License, or (at your option) any later version as  
   modified below.  As the original licensors, we add the following  
   conditions to that license:

   In paragraph 2.b), the phrase "distribute or publish" should be  
   interpreted to include entry into a competition, and hence the source  
   of any derived work entered into a competition must be made available  
   to all parties involved in that competition under the terms of this  
   license.

   In addition, if the authors of a derived work publish any conference  
   proceedings, journal articles or other academic papers describing that  
   derived work, then appropriate academic citations to the original work  
   should be included in that publication.

   This rUNSWift source is distributed in the hope that it will be useful,  
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
   General Public License for more details.

   You should have received a copy of the GNU General Public License along  
   with this source code; if not, write to the Free Software Foundation,  
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


/* This file implements functions which are shared by calibration and learning
  */
#include "walkBase.h"

/* class constructor */
WalkBase::WalkBase(QApplication* app, QWidget* parent,
		 const char* name, WFlags fl)
  : FormWalkBase(parent, name, fl) {
    
    // init variables
    currSetIndex = -1;
    isLearning = false;
    app = app;
    file = NULL;    
    sendSocket = NULL;
    receiveSocket = NULL;
    timeStamp = 0;
    
    // learning variables
    lrnWalkType = NORMAL_WALK;
    ignoreActionDataOnce = true;
    
    // init GUI
    for (int i =0; i<tblData->numCols(); i++)
	tblData->setColumnStretchable(i, true);
    //for (int i =0; i<tblOptResults->numCols(); i++)
    //	tblOptResults->setColumnStretchable(i, true);    
    
    QDoubleValidator *f = new QDoubleValidator( -2000, 2000, 2, this ); // Min, max, decimal points   
    edtLeft->setValidator(f);
    edtForward->setValidator(f);
    edtTurn->setValidator(f);
    
    // init embed python for walk optimisation
    WalkBasePyLink::initPythonModule();

    initGUILearningParams();
    
    // signal necessay slots
    slotCmbModeChanged(cmbMode->currentItem());
    
    initJointDebug();
}

WalkBase::~WalkBase() {
    for(int i=0; i<12; i++)
	delete plots[i];
}

/******************************Public Slots *************************/

/* Handles opening of a new file via File->Open menu */
void WalkBase::slotFileOpen() {
    closeDevice(file);
    QString filename = QFileDialog::getOpenFileName(
	    "../logs",
	    "All files (*.*);;Log files (*.RLOG, *.rlog)",
	    this, "Open rlog file", "Choose a rlog file to open");
    if ( filename.isNull() )
	return;
    else if (filename.right(4) != "RLOG" && filename.right(4) != "rlog") {
	cerr << filename << " - unrecognised extension. Expected .rlog/.RLOG" << endl;
	return;
    }    
    file = new QFile(filename);
    file->open(IO_ReadOnly);
    while ( !file->atEnd() && file->size() > file->at() )
	readFromDevice(file);
    updateData();
    file->close();    
}

/* ************************ RECEIVING SOCKET functions**********************************/
void WalkBase::slotReceiveSocketConnect() {
    closeDevice(receiveSocket);
    receiveSocket = new QSocket(this);
    receiveSocket->connectToHost(lblRobotIP->text(), RECEIVE_SOCKET_PORT);
    connect(receiveSocket,SIGNAL(connected()), this, SLOT(slotReceiveSocketConnected()));
    connect(receiveSocket,SIGNAL(error(int)), this, SLOT(slotReceiveSocketError(int)));    
    statusBar()->message("receiveSocket: Connecting");
}

void WalkBase::slotReceiveSocketConnected(){
    statusBar()->message("receiveSocket: Connected");
    connect(receiveSocket, SIGNAL(readyRead()), this, SLOT(slotReceiveSocketReadyRead()) );
    connect(receiveSocket, SIGNAL(connectionClosed()), this, SLOT(slotReceiveSocketClosed()));
}

void WalkBase::slotReceiveSocketReadyRead() {
    while (!receiveSocket->atEnd())
	readFromDevice(receiveSocket);
}

void WalkBase::slotReceiveSocketClosed() {
    statusBar()->message("receiveSocket: Connection closed");
    closeDevice(receiveSocket);
    receiveSocket = NULL;
}

void WalkBase::slotReceiveSocketError(int err) {
    statusBar()->message("receiveSocket: Error! - "+ getQSocketErrorMsg(err));
    closeDevice(receiveSocket);
    receiveSocket = NULL;
}

QString WalkBase::getQSocketErrorMsg(int err) {
    switch (err) {
    case QSocket::ErrConnectionRefused:
	return "Connection refused!";
    case QSocket::ErrHostNotFound:
	return "Host not found!";
    case QSocket::ErrSocketRead:
	return "Read from the socket failed!";
    default:
	return QString::number(err);
    }
}
/* ************************ END of RECEIVING SOCKET functions ***************************/

/* ************************ SENDING SOCKET functions *****************************/
void WalkBase::slotSendSocketConnect(){
    closeDevice(sendSocket);
    sendSocket = new QSocket(this);
    sendSocket->connectToHost(lblRobotIP->text(), SEND_SOCKET_PORT);
    statusBar()->message("sendSocket: Connecting");    
    connect(sendSocket,SIGNAL(connected()), this, SLOT(slotSendSocketConnected()));
    connect(sendSocket,SIGNAL(error(int)), this, SLOT(slotSendSocketError(int)));
}

void WalkBase::slotSendSocketConnected(){
    statusBar()->message("sendSocket: Connected");
    connect(sendSocket, SIGNAL(readyRead()), this, SLOT(slotSendSocketReadyRead()) );
    connect(sendSocket,SIGNAL(connectionClosed()), this, SLOT(slotSendSocketClosed()));
}


void WalkBase::slotSendSocketReadyRead() {
    while (!sendSocket->atEnd())
	readJointValues(sendSocket);
}


void WalkBase::slotSendSocketClosed(){
    statusBar()->message("sendSocket: Connection closed");
    closeDevice(sendSocket);
    sendSocket = NULL;    
}

void WalkBase::slotSendSocketError(int err) {    
    statusBar()->message("sendSocket: Error! - "+ getQSocketErrorMsg(err) );
    closeDevice(sendSocket);
    sendSocket = NULL;
}
/* ************************END of SENDING SOCKET functions *****************************/



/************************END of Public Slots *************************/


void WalkBase::closeDevice(QIODevice *dev) {
    if (dev && dev->isOpen())  {
	dev->close();
	delete dev;
    }
}



/* Reads exactly size bytes from the file stream into the data buffer. This
 * loops over QIODevice::readBlock which may read less that the requested
 * amount of data.
 */
Q_LONG WalkBase::readAllBlock(QIODevice* input, char* data, Q_ULONG size) {
    Q_LONG result;
    Q_ULONG count = 0;
    while (count < size) {
	result = input->readBlock(data + count, size - count);
	if (result < 0) {
	    dout << "Error in readBlock. State: " << input->state() << endl;
	    return -1;
	} else if (result == 0) {
	    //dout << "Received zero bytes, possible EOF." << endl;
	    //dout << " size:" << input->size() << " at: " << input->at() << endl;
	    return count;
	}
	count += result;
    }
    return count;
}


void WalkBase::readFromDevice(QIODevice* input) {
    static robolink_header header;
    // read the header
    readAllBlock(input, (char *)&header, sizeof(header));
    // read atomic action data
    if (header.data_type == RLNK_ATOMIC_ACTION) {
	// read current atomic action values
	gotNewAction(input);
    } else {
	// skip all other data type
	input->at(input->at() + header.data_len);
	//dout << "Skipping data, type: "<< header.data_type << " size: " << header.data_len << endl;
    }
} 





void WalkBase::gotNewAction(QIODevice* input) {
  AActionData data;
  data.isValid = true;
  readAllBlock(input, (char *)&(data.action), sizeof(data.action));    
  // check the whether time elapsed is too small
  if (data.action.timeElapsed < 1500) { // < 1.5 sec
    cout << "time elapsed " << data.action.timeElapsed << " is too short, discarded." << endl;
    return;
  }
  
  if (!isLearning) { // if calibrating
      // init or create new data set if necessay
      if (dataSets.empty()) { // if no data in inSet
	  initNewDataSet(data.action);
	  currSetIndex = 0;
	  return; // skip the first data
      } else {
	  DataSet& inSet = dataSets.back();
	  if (inSet.params.forwardMaxStep != data.action.forwardMaxStep ||
	      inSet.params.leftMaxStep != data.action.leftMaxStep ||
	      inSet.params.turnCCWMaxStep != data.action.turnCCWMaxStep) {
	      // if  NOT receive the same type of data
	      //TODO: should check PG and walktype, too
	      initNewDataSet(data.action);
	      return; // skip the first data
	  }
      }
      dataSets.back().dataVector.push_back(data);      
      checkSchedule();    
  } else { // if learning
      if (ignoreActionDataOnce)
	  ignoreActionDataOnce = false;
      else {
	  dataSets.back().dataVector.push_back(data);
	  updateLearningResults();
      }
  }
  
  //update the table values (if it's the last item in cmbDataSets showing)
  if (currSetIndex == (int)dataSets.size()-1)
    insertDataTableEntry(data);      
  updateData();
}


void WalkBase::slotCmbModeChanged(int index) {
    bool forwardOn = false;
    bool sidewaysOn = false;
    bool turningOn = false;
    if (index == FORWARD_ONLY || index == FORWARD_TURN)
	forwardOn = true;
    if (index == SIDEWAYS_ONLY || index == GRAB_SIDEWAY 
	|| index == SIDEWAYS_TURN)
	sidewaysOn = true;
    if (index == TURN_ONLY || index == GRAB_TURNING || 
	index == FORWARD_TURN || index == SIDEWAYS_TURN)
	turningOn = true;
    
    if (index == FWD_LFT_TRN_ALL || index == MANUAL_LEARNING) {
	forwardOn = sidewaysOn = turningOn = true;
    }
    
    // forward related
    edtForward->setEnabled(forwardOn);
    edtForwardDist->setEnabled(forwardOn);
    lblForwardActual->setEnabled(forwardOn);
    lblForwardActualPerSec->setEnabled(forwardOn);
    lblForwardRatio->setEnabled(forwardOn);
    if ( ! forwardOn ) {
	edtForward->setText("0"); 
    }
    
    // sideways related
    edtLeft->setEnabled(sidewaysOn);
    edtLeftDist->setEnabled(sidewaysOn);    
    lblLeftActual->setEnabled(sidewaysOn);
    lblLeftActualPerSec->setEnabled(sidewaysOn);
    lblLeftRatio->setEnabled(sidewaysOn);
    if ( ! sidewaysOn ) {
	edtLeft->setText("0");
    }
    
    // turning related
    edtTurn->setEnabled(turningOn);
    edtTurnAmount->setEnabled(turningOn);
    lblTurnActual->setEnabled(turningOn);
    lblTurnActualPerSec->setEnabled(turningOn);
    lblTurnRatio->setEnabled(turningOn);
    if ( ! turningOn ) {
	edtTurn->setText("0");
    }
    
    // forward_turn related
    if (index == FORWARD_TURN) {
	lblForward->setText("Fwd Diameter");
    } else {
	lblForward->setText("Forward");
    }
    
    // SIDEWAYS_turn related
    if (index == SIDEWAYS_TURN) {
	lblLeft->setText("Left Diameter");
    } else {
	lblLeft->setText("Left");
    }
}


bool WalkBase::sendCommand(const char* name, const char* value) {
    if ( !sendSocket ) {
	statusBar()->message("SendSocket NOT connected, aborting sending.");
	return false;
    }
    static const int PACKAGE_SIZE = sizeof(PackageDef);
    static const int WDATA_SIZE = sizeof(WDataInfo);
    static const int SEND_BUFFER_SIZE = PACKAGE_SIZE + WDATA_SIZE;
    static char sendBuffer[SEND_BUFFER_SIZE];
    
    WDataInfo nameInfo, valueInfo;
    nameInfo.robot=0;
    nameInfo.type=200;
    nameInfo.size = strlen(name);    
    valueInfo.robot = 0;
    valueInfo.type  = 201;
    valueInfo.size  = strlen(value);
    
    PackageDef package = PackageDef((char*)name, nameInfo, (char*)value, valueInfo);
    
    WDataInfo packageInfo;    
    packageInfo.robot = 0;
    packageInfo.type  = 200;
    packageInfo.size  = sizeof(package);
    
    memcpy(sendBuffer, &packageInfo, WDATA_SIZE);
    memcpy(sendBuffer+WDATA_SIZE, &package, PACKAGE_SIZE);
    
    int sendSize = SEND_BUFFER_SIZE;
    int result = 0;
    char* bufferPtr = sendBuffer;
    while (sendSize > 0) {
	result = sendSocket->writeBlock(bufferPtr, sendSize);
	if (result == -1) {
	    statusBar()->message("Send Error! send socket not connected?");
	    return false;
	}     
	sendSize -= result;
	bufferPtr += result;
    }
    static QString s;
    s.sprintf("Command Sent: %s %s", name, value);
    statusBar()->message(s);
    return true;
}

void WalkBase::slotSendRelaxRobotCmd(){
    static const char* name = "relax";
    static const char* value = "9 0";
    sendCommand(name, value);
}    

void WalkBase::slotSendTurnOffRobotCmd(){
    static const char* name = "off";
    static const char* value = "9 0";
    sendCommand(name, value);
}

char WalkBase::getGainOption() {
    switch(cmbGain->currentItem()) {
    case 0:
	return 'd';
    case 1:
	return 'h';
    case 2:
	return 'l';
    default:
	dout << "Unknown Gain Option: " << cmbGain->currentItem();
	dout << "Use High Gain!" << endl;
	return 'h';
    }
}

void WalkBase::sendPythonCommand(const QString& value) {
    static const char* name = "pyc";
    sendCommand(name, (char*)value.ascii());
}

void WalkBase::slotSendHeadParams(void) {
    QString cmd;
    QTextOStream ts(&cmd);
    ts << "9 hdp " << sldHeadTilt->value() << " "
	    << sldHeadCrane->value();
    sendPythonCommand(cmd);
}

void WalkBase::slotStandStill(bool on) {
    if (on) {
	sendCommand("pyc", "9 std");
	if (timeStamp == 0.0) {
	    lblLrnTimer->setText("N/A");
	} else {
	    gettimeofday(&tval, NULL);
	    double NewTimeStamp = tval.tv_sec * 1000 + tval.tv_usec / 1000;      
	    lblLrnTimer->setText(QString::number(NewTimeStamp - timeStamp));
	    timeStamp = 0.0;
	}
	btnStandStill->setText("Resume Walk");
    } else {
	sendCommand("pyc", "9 rsm");
	btnStandStill->setText("Stand Still!");
	gettimeofday(&tval, NULL);
	timeStamp = tval.tv_sec * 1000 + tval.tv_usec / 1000;	
    }
}
