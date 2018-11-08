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


/* This file implements functions which related to walk calibration
  */
#include "walkBase.h"

void WalkBase::initNewDataSet(const struct rlnk_atomicaction& action) {
    // create new dataset and init some of its parameters
    DataSet newSet;
    newSet.forwardDist = edtForwardDist->text().toDouble();
    newSet.leftDist = edtLeftDist->text().toDouble();
    newSet.turnAmount = edtTurnAmount->text().toDouble();
    memcpy(&newSet.params, &action, sizeof(action));    
    // add the new set
    dataSets.push_back(newSet);

    // update the combo box
    QString s;
    s.sprintf("F:%.2f L:%.2f T:%.2f", action.forwardMaxStep, action.leftMaxStep, action.turnCCWMaxStep);
    cmbDataSets->insertItem(s);
    statusBar()->message("New data set created.");
}


void WalkBase::insertDataTableEntry(const AActionData& data) { 
    int row_num = tblData->numRows();
    tblData->insertRows(row_num);
    QCheckTableItem* chkValid = new QCheckTableItem(tblData,"");
    chkValid->setChecked(data.isValid);
    tblData->setItem(row_num,0,chkValid); // Valid.
    tblData->setText(row_num,1,QString::number(data.action.PG)); // PG
    tblData->setText(row_num,2, walkTypeToStr[data.action.walkType]); //walkType
    tblData->setText(row_num,3,QString::number(data.action.forward)); // Forward
    tblData->setText(row_num,4,QString::number(data.action.left)); // left
    tblData->setText(row_num,5,QString::number(data.action.turn)); // turn
    tblData->setText(row_num,6,QString::number(data.action.timeElapsed)); // time elapsed
}


void WalkBase::slotScheduleStarted(bool started) {
    if (started) {
	btnScheduleStart->setText("Stop");
	lblScheduleValue->setText(edtScheduleFrom->text());
	sendScheduleCommand();
    } else {
	btnScheduleStart->setText("Start");
	lblScheduleValue->setText("N/A");
    }
    edtScheduleRunsPerSet->setEnabled(!started);
    edtScheduleFrom->setEnabled(!started);
    edtScheduleTo->setEnabled(!started);
    edtScheduleStepSize->setEnabled(!started);
    btnCalSendCommand->setEnabled(!started);
}

void WalkBase::sendScheduleCommand() {
    double currentValue = lblScheduleValue->text().toDouble();    
    switch( cmbMode->currentItem() ) {
    case FORWARD_ONLY: //forward only
	sendCalCommand(currentValue, 0, 0);
	break;
    case SIDEWAYS_ONLY: // sideway onlys
    case GRAB_SIDEWAY:
	sendCalCommand(0, currentValue, 0);
	break;
    case TURN_ONLY: // turn only
    case GRAB_TURNING:
	sendCalCommand(0, 0, currentValue);
	break;
    default:
	cout << __FILE__ << " : " << __PRETTY_FUNCTION__ << " Unknown Mode: " 
		<< cmbMode->currentItem() << endl;
    }
}

void WalkBase::checkSchedule() {
    if ( ! btnScheduleStart->isOn() )
	return; // if schedule is off    
    if (dataSets.back().dataVector.size() < edtScheduleRunsPerSet->text().toUInt())
	return; // if not enough runs in current data set    
    double currentValue = lblScheduleValue->text().toDouble();
    double stepSize = edtScheduleStepSize->text().toDouble();
    double endValue = edtScheduleTo->text().toDouble();    
    // check whether it's end of the schedule
    currentValue += stepSize;
    if ( (stepSize > 0 && currentValue > endValue)
	|| (stepSize < 0 && currentValue < endValue)
	|| stepSize == 0) {
	btnScheduleStart->toggle();
	sendCalCommand(0, 0, 0);
	return; // if end of the schedule
    } 
    // set gui value and send the command
    lblScheduleValue->setText(QString::number(currentValue));
    sendScheduleCommand();
}


void WalkBase::updateData() {
    double total = 0.0;
    int numValidData = 0;
    
    // calculate mean
    if (dataSets.empty() || currSetIndex >= (int)dataSets.size())
	return;
    vector<AActionData>& dataVector = dataSets[currSetIndex].dataVector;
    for (vector<AActionData>::iterator iter = dataVector.begin();
    iter != dataVector.end(); ++iter) {
	if ( !iter->isValid )
	    continue; 
	++numValidData;
	total += iter->action.timeElapsed;
    }
    if (numValidData == 0)
	return; // if no valid data    
    double timeElapsedMean = total / numValidData;
   
    // calculate standard deviation
    double temp;
    total = 0.0;
    for (vector<AActionData>::iterator iter = dataVector.begin();
    iter != dataVector.end();
    ++iter) {
	if ( !iter->isValid )
	    continue; 
	temp = iter->action.timeElapsed - timeElapsedMean;
	total += temp * temp;
    }
    double timeElapsedStddev = sqrt( total / numValidData );
        
    // now update corresponding GUI fields
    lblElapsedStddev->setText( QString::number( timeElapsedStddev) );
    lblNumberDataValid->setText( QString::number(numValidData) );
    lblNumberData->setText( QString::number( dataVector.size() ) );
    edtElapsedMean->setText( QString::number(timeElapsedMean) ); // this will trigger the actual value updates    
}

void WalkBase::slotClearData() {
    dataSets[currSetIndex].dataVector.clear();
    clearGUIData();
}
    
void WalkBase::clearGUIData() {
    tblData->setNumRows(0);    
    edtElapsedMean->clear(); 
    lblElapsedStddev->clear();
    lblNumberDataValid->clear();
    lblNumberData->clear();
    lblForwardActualPerSec->clear();
    lblForwardActual->clear();  
    lblForwardRatio->clear();
}

void WalkBase::slotEdtForwardDistChanged() {
    dataSets[currSetIndex].forwardDist = edtForwardDist->text().toDouble();
}

void WalkBase::slotEdtLeftDistChanged() {
    dataSets[currSetIndex].leftDist = edtLeftDist->text().toDouble();
}

void WalkBase::slotEdtTurnAmountChanged() {
    dataSets[currSetIndex].turnAmount = edtTurnAmount->text().toDouble();
}

void WalkBase::slotCalForwardActual() {
    if (!edtForwardDist->isEnabled())
	return; // if not activated
    double timeElapsedMean = edtElapsedMean->text().toDouble();
    if (timeElapsedMean == 0.0)
	return; // avoid dividing 0
    double fwdDist = edtForwardDist->text().toDouble();
    if (cmbMode->currentItem() == FORWARD_TURN)
	fwdDist *= PI; // expecting input is the diameter of a circle
    double fwdSpeed =  fwdDist / timeElapsedMean * 1000.0;
    double fwdStepSpeed = fwdSpeed/1000.0 * (dataSets[currSetIndex].params.PG * 2 * 8);
    lblForwardActualPerSec->setText(QString::number(fwdSpeed));
    lblForwardActual->setText(QString::number(fwdStepSpeed));
    lblForwardRatio->setText( QString::number(dataSets[currSetIndex].params.forward / fwdStepSpeed));
}

void WalkBase::slotCalLeftActual() {
    if (!edtLeftDist->isEnabled())
	return; // if not activated
    double timeElapsedMean = edtElapsedMean->text().toDouble();    
    if (timeElapsedMean == 0.0)
	return; //avoid dividing 0
    double leftDist = edtLeftDist->text().toDouble();    
    if (cmbMode->currentItem() == SIDEWAYS_TURN)
	leftDist *= PI; // expecting input is the diameter of a circle
    double leftSpeed = leftDist / timeElapsedMean * 1000; // cm / sec
    double leftStepSpeed = leftSpeed * (dataSets[currSetIndex].params.PG * 2 * 8)/1000;
    lblLeftActualPerSec->setText(QString::number(leftSpeed));
    lblLeftActual->setText(QString::number(leftStepSpeed));
    lblLeftRatio->setText( QString::number(dataSets[currSetIndex].params.left / leftStepSpeed));
}

void WalkBase::slotCalTurnActual() {
    if (!edtTurnAmount->isEnabled())
	return; // if not activated
    double timeElapsedMean = edtElapsedMean->text().toDouble();    
    if (timeElapsedMean == 0.0)
	return; //avoid dividing 0
    double turnSpeed = edtTurnAmount->text().toDouble() / timeElapsedMean * 1000; // cm / sec
    double turnStepSpeed = turnSpeed * (dataSets[currSetIndex].params.PG * 2 * 8)/1000.0;
    lblTurnActualPerSec->setText(QString::number(turnSpeed));
    lblTurnActual->setText(QString::number(turnStepSpeed));
    lblTurnRatio->setText( QString::number(dataSets[currSetIndex].params.turn / turnStepSpeed));
}

void WalkBase::slotSendTurnAdjustmentCmd() {
    static const char* name = "pyc";
    double forward = edtForwardAdjust->text().toDouble();
    double left = edtLeftAdjust->text().toDouble();
    double turn = edtTurnAdjust->text().toDouble();
    QString s;    
    s.sprintf("9 adj %d %c %.2f %.2f %.2f", cmbWalkType->currentItem(), 
	      getGainOption(), 
	      forward, left, turn);
    char *value = (char *)s.ascii();
    sendCommand(name, value);
}

// This function used by both schedule and manual command
void WalkBase::sendCalCommand(double forward, double left, double turn) {
    if (btnStandStill->isOn())
	btnStandStill->toggle();    
    static const char *name = "pyc";
    QString s;
    s.sprintf( "9 cal %d %d %c %d %d %.2f %.2f %.2f", 
	       cmbMode->currentItem(), 
	       cmbWalkType->currentItem(), 
	       getGainOption(), 
	       edtTurnAdjustment->text().toInt(),
	       edtMaxTurningAngle->text().toInt(),
	       forward, left, turn);
    char *value = (char *) s.ascii();
    sendCommand(name, value);
}

// Manual button "SEND" pressed
void WalkBase::slotCalSendCommand(){
    sendCalCommand(edtForward->text().toDouble(), edtLeft->text().toDouble(), edtTurn->text().toDouble()); 
}


void WalkBase::slotTblDataChanged(int row, int col) {
    if (col != 0)
	return; // must be the first column
    QCheckTableItem* item = (QCheckTableItem*) tblData->item(row, col);    
    // assume the row number in the data table correspond to the index in the data vector 
    dataSets[currSetIndex].dataVector[row].isValid = item->isChecked();        
    updateData();
}

void WalkBase::slotCmbDataSetsChanged(int index) {
    clearGUIData();
    currSetIndex = index;
    DataSet& dataSet = dataSets[currSetIndex];    
    for (vector<AActionData>::iterator iter = dataSet.dataVector.begin();
    iter != dataSet.dataVector.end(); ++iter) {
	insertDataTableEntry(*iter);
    }
    edtForwardDist->setText(QString::number(dataSet.forwardDist));
    edtLeftDist->setText(QString::number(dataSet.leftDist));
    edtTurnAmount->setText(QString::number(dataSet.turnAmount));
    updateData();
}


void WalkBase::slotResultCopyOver() {
    //update the table values
    int row_num = tblResult->numRows();
    tblResult->insertRows(row_num);
    switch (cmbMode->currentItem()) {
    case FORWARD_ONLY:
	tblResult->setText(row_num,0,QString::number(dataSets[currSetIndex].params.forward));
	tblResult->setText(row_num,1,lblForwardActual->text());
	break;
    case SIDEWAYS_ONLY:
    case GRAB_SIDEWAY:
	tblResult->setText(row_num,0,QString::number(dataSets[currSetIndex].params.left));
	tblResult->setText(row_num,1,lblLeftActual->text());
	break;
    case TURN_ONLY:
    case GRAB_TURNING:
	tblResult->setText(row_num,0,QString::number(dataSets[currSetIndex].params.turn));
	tblResult->setText(row_num,1,lblTurnActual->text());
	break;
    default:
	cout << "calWalk.cpp:: Unknown Mode: " << cmbMode->currentItem() << endl;
	break;
    }    
}

void WalkBase::slotCalSaveResult() {
    QString filename = QFileDialog::getSaveFileName(
	    ".",
	    "Result files (*.rst)",
	    this, "save result file", "Choose a result file to save");
    if ( filename.isNull() )
	return;
    QFile* fout = new QFile(filename);
    if (fout->open(IO_WriteOnly) ) {
	QTextStream stream( fout );
	//stream << "#Normal Walk - Forward Only" << endl;
	for (int i = 0; i< tblResult->numRows(); i++)
	    stream << tblResult->text(i, 0) << " " << tblResult->text(i, 1) << endl;
    }
    fout->close();
    delete fout;
}

void WalkBase::slotSaveData() {
/*    QString filename = QFileDialog::getSaveFileName(
	    ".",
	    "Data files (*.dat)",
	    this, "save data file", "Choose a file to save");
    if ( filename.isNull() )
	return; */
    rlnk_atomicaction action;
    initNewDataSet(action);
    dout << dataSets.size() << endl;
    dout << sizeof(dataSets) << endl;
/*    QFile* fout = new QFile(filename);    
    if (fout->open(IO_WriteOnly) ) {
	//fout->
	QTextStream stream( fout );
	for (int i = 0; i< tblResult->numRows(); i++)
	    stream << tblResult->text(i, 0) << " " << tblResult->text(i, 1) << endl;
    }
    fout->close(); */
}


void WalkBase::slotCalLoadResult() {
    //cout << "slotLoadResult() begin" << endl;
    tblResult->setNumRows(0);
    QString filename = QFileDialog::getOpenFileName(
	    ".",
	    "Result files (*.rst)",
	    this, "load result file", "Choose a result file to open");
    if ( filename.isNull() )
	return;
    QFile* fin = new QFile(filename);
    double command=0, actual=0;
    int row_num;
    if ( fin->open(IO_ReadOnly) ) {
	QTextStream stream( fin );
	while ( !stream.eof() ) {
	    stream >> command >> actual;
	    row_num = tblResult->numRows();
	    tblResult->insertRows(row_num);
	    tblResult->setText(row_num,0,QString::number(command));
	    tblResult->setText(row_num,1, QString::number(actual));
	}
    }
    fin->close();
    delete fin;
}

