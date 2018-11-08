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


/* This file implements functions which related to walk learning*/
#include "walkBase.h"

#include "qthread.h"



static const int HUGE_EVALUATION_VALUE = 100000; // 100 sec

// this enum must correspond to the columns in tblPWalkParams, tblEllipticalWalkParams, tblSkellipticalWalkParams
enum WALK_PARAMS_TABLE_COLUMN_ENUM {
    VALID_COL,
    NAME_COL,
    LOWER_LIMIT_COL,
    UPPER_LIMIT_COL,
    INIT_VALUE_COL,    
    CURR_VALUE_COL,
    BEST_VALUE_COL,
    UNIT_COL
};

static const int PWALK_PARAMS_NUM = 12;
static const int NOR_PARAMS_NUM = PWALK_PARAMS_NUM;
static const int ELI_PARAMS_NUM = 5;
static const int SKE_PARAMS_NUM = 22;


class ViewLocusThread : public QThread {
public:
    virtual void run() {
	system("./drawLocus");
    }
};


void WalkBase::slotLrnLoadInitParams() {
    QString filename = QFileDialog::getOpenFileName(".",
						    "Parameter files (*.prm)",
						    this, "load parameter file", "Choose a walk parameter file to open");
    if ( filename.isNull() )
	return;
    QFile fin(filename);
    if ( !fin.open(IO_ReadOnly) ) {
	statusBar()->message("Error opening file: "+filename);
	return;
    }
    QTextStream fs(&fin);
    // read mode
    int mode;
    fs >> mode;
    if (mode >= cmbMode->count()) {
	statusBar()->message("Invalid mode: "+ mode);
	return;
    }	   
    cmbMode->setCurrentItem(mode);
    // read turn adjustment, max turning, gain
    double temp;
    fs >> temp;
    edtTurnAdjustment->setText(QString::number(temp));
    fs >> temp;
    edtMaxTurningAngle->setText(QString::number(temp));    
    int gainIdx;
    fs >> gainIdx;
    cmbMode->setCurrentItem(gainIdx);
    
    // read walk type
    QString type;
    fs >> type;
    QTable* tblExtra;
    if (type == "EllipticalWalkWT") {
	tblExtra = tblEllipticalWalkParams;
	tabWalkType->setCurrentPage(ELLIPTICAL_WALK);
    }
    else if (type == "SkellipticalWalkWT") {
	tblExtra = tblSkellipticalWalkParams;
	tabWalkType->setCurrentPage(SKELLIPTICAL_WALK);
    }
    else { // "NOR" or no type
	tblExtra = NULL;
	tabWalkType->setCurrentPage(NORMAL_WALK);
    }
    // read minor walk type
    QString ignore;
    fs >> ignore;
    
    // read walk params
    int valid;
    double init, min, max;
    int i = 0;
    while (!fin.atEnd()) {
	fs >> valid >> init >> min >> max;
	if ( i < PWALK_PARAMS_NUM) { // common pwalk params
	    QCheckTableItem* item = (QCheckTableItem*) tblPWalkParams->item(i, VALID_COL);
	    item->setChecked((bool)valid);
	    tblPWalkParams->setText(i, INIT_VALUE_COL, QString::number(init));
	    tblPWalkParams->setText(i, LOWER_LIMIT_COL, QString::number(min));
	    tblPWalkParams->setText(i, UPPER_LIMIT_COL, QString::number(max));	    
	}
	else if (tblExtra) { // extra params
	    if ( i >= tblExtra->numRows() + PWALK_PARAMS_NUM)
		break;
	    QCheckTableItem* item = (QCheckTableItem*) tblExtra->item(i-PWALK_PARAMS_NUM, VALID_COL);  	  
	    item->setChecked((bool)valid);	
	    tblExtra->setText(i-PWALK_PARAMS_NUM, INIT_VALUE_COL, QString::number(init));
	    tblExtra->setText(i-PWALK_PARAMS_NUM, LOWER_LIMIT_COL, QString::number(min));
	    tblExtra->setText(i-PWALK_PARAMS_NUM, UPPER_LIMIT_COL, QString::number(max));	    
	}
	i++;      
    }
    fin.close();
}

void WalkBase::slotLrnPolicyTooSlow() {
    vector<AActionData>& dataVector = dataSets.back().dataVector;
    int size = dataVector.size();
    if (size == 0) {
	// if haven't got any time measurement
	tblOptResults->setText(tblOptResults->numRows()-1, 2, "N/A");
	WalkBasePyLink::setEvaluation(currPolicy->id, HUGE_EVALUATION_VALUE);
    } else {
	// get the worst time elasped
	int worst = -1;
	int temp;
	for (int i=0;i<size;i++) {
	    temp = dataVector[i].action.timeElapsed;
	    if (worst < temp)
		worst = temp;
	}
	// if the worst time is better than the best time
	double best = edtOptBestValue->text().toDouble();
	if (worst < best)
	    worst = (int)best * 2;
	tblOptResults->setText(tblOptResults->numRows()-1, 2, QString::number(worst));
	WalkBasePyLink::setEvaluation(currPolicy->id, worst);	       
    }
    ignoreActionDataOnce = true;
    getAndSendNewPolicy();   
}


void WalkBase::initLearningParamsTable(QTable* tbl, int numRows, const char** names) {
    tbl->setNumRows(0);
    tbl->insertRows(0, numRows);
    tbl->setColumnReadOnly(NAME_COL, true);
    QCheckTableItem* chkValid;
    for (int i=0;i<numRows;i++) {
	chkValid = new QCheckTableItem(tbl,"");
	chkValid->setChecked(true);
	tbl->setItem(i, 0, chkValid);
	tbl->setText(i, NAME_COL, names[i]);
    }
    tbl->setColumnReadOnly(CURR_VALUE_COL, true);
    tbl->setColumnReadOnly(BEST_VALUE_COL, true);
    tbl->setColumnReadOnly(UNIT_COL, true);
    tbl->setColumnStretchable(NAME_COL, true);
    tbl->setColumnStretchable(UNIT_COL, true);
}

void WalkBase::updateLearningResults() {    
    vector<AActionData>& dataVector = dataSets.back().dataVector;    
    int size = dataVector.size();
    int row_num = tblOptResults->numRows();
    double temp = dataVector.back().action.timeElapsed;
    tblOptResults->setText(tblOptResults->numRows()-1, 2+size, 
			   QString::number(temp));
    int numModes = 1;
    if (cmbMode->currentItem() == FWD_LFT_TRN_ALL)
	numModes = 6;
    if (numModes == 6 && size !=0 && size %RUNS_PER_POLICY == 0)
	slotLrnSendCurrentParams();
    if (size >= numModes*RUNS_PER_POLICY) {
	double timeAverage = 0;
	for (int i=0; i<numModes; i++) {
	    double timeTotal=0, timeMax= -1, timeMin = 10000000;
	    double temp;
	    for (unsigned int k = 0; k< RUNS_PER_POLICY; k++) {
		temp = dataVector[k+i*RUNS_PER_POLICY].action.timeElapsed;
		timeTotal += temp;	
		if (temp > timeMax)
		    timeMax = temp;
		if (temp < timeMin)
		    timeMin = temp;
	    }
	    timeTotal -= timeMax;
	    timeTotal -= timeMin;
	    timeAverage += timeTotal / (double)(RUNS_PER_POLICY-2);
	}
	tblOptResults->setText(row_num-1, 2, QString::number(timeAverage));
	WalkBasePyLink::setEvaluation(currPolicy->id, (int)timeAverage);
	// get best params
	BestParams* bestParams = WalkBasePyLink::getBestParams();
	if (bestParams) {
	    // update the walk param table
	    updateLrnParams(BEST_VALUE_COL, bestParams->params); 
	    
	    // convert the param vector into a string
	    QString paramStr;
	    QTextOStream ts(&paramStr);
	    int size = bestParams->params.size();
	    for(int i=0;i<size;i++)
		ts << bestParams->params[i] << " ";
	    // update results
	    edtOptBestParams->setText(paramStr);
	    edtOptBestValue->setText(QString::number(bestParams->value));
	    
	    // write to a file
	    const char* walkTypeStr = walkTypeToStr[dataVector[0].action.walkType];
	    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
	    QString filename = QString(walkTypeStr)+"Best"+dateStr+".prm";
	    saveLrnParamsToFile(filename, BEST_VALUE_COL, bestParams->value);	
	} else {
	    edtOptBestParams->setText("N/A");
	    edtOptBestValue->setText("N/A");		
	}
	// send new policy
	getAndSendNewPolicy();
    }
}

void WalkBase::slotLrnSaveInitParams() {
    QString filename = QFileDialog::getSaveFileName(".",
						    "Parameter files (*.prm)",
						    this, "save parameter file", "Choose a walk parameter file to save");
    if ( filename.isNull() )
	return;
    saveLrnParamsToFile(filename, INIT_VALUE_COL, HUGE_EVALUATION_VALUE);
}

void WalkBase::slotLrnDrawLocus() {
    saveLrnParamsToFile("tempLocus.prm", INIT_VALUE_COL, HUGE_EVALUATION_VALUE);
    ViewLocusThread t;
    t.run();
    t.wait();
}

void WalkBase::saveLrnParamsToFile(QString file, int col, double value) {
    QFile fout(file);
    int walkTypeIdx = SkellipticalWalkWT;
    switch (lrnWalkType) {
    case NORMAL_WALK:
	walkTypeIdx = NormalWalkWT;
	break;
    case ELLIPTICAL_WALK:
	walkTypeIdx = EllipseWalkWT;
	break;
    case SKELLIPTICAL_WALK:
	walkTypeIdx = SkellipticalWalkWT;
	break;
    default:
	dout << "Unknown Walk Type: " << lrnWalkType << endl;
    }
    if (fout.open(IO_WriteOnly)) {
	QTextStream fs(&fout);
	fs << cmbMode->currentItem() << endl;
	fs << edtTurnAdjustment->text() 
		<< " " << edtMaxTurningAngle->text() 
		<< " " << cmbGain->currentItem() << endl;
	fs << walkTypeToStr[walkTypeIdx] << endl;
	fs << 1 << endl; // Calibration and Learning minor walk type
	for(int i=0;i<tblPWalkParams->numRows(); i++)
	    saveLrnParams(fs, tblPWalkParams, i, col);
	if (lrnWalkType == SKELLIPTICAL_WALK)
	    for(int i=0;i<tblSkellipticalWalkParams->numRows(); i++)
		saveLrnParams(fs, tblSkellipticalWalkParams, i, col);
	fs << value << endl;
    }
    fout.close();    
}

void WalkBase::saveLrnParams(QTextStream& ts, QTable* tbl, int row, int col) {
    ts << ((QCheckTableItem *)tbl->item(row, VALID_COL))->isChecked()
	    << " " << getWalkParam(tbl, row, col)
	    << " " << tbl->text(row, LOWER_LIMIT_COL)
	    << " " << tbl->text(row, UPPER_LIMIT_COL)
	    << endl;    
}

void WalkBase::slotLrnStarted(bool started) {
    isLearning = started;
    cmbMode->setEnabled(!isLearning);
    cmbGain->setEnabled(!isLearning);
    if (started) {
	/*
	if ( !sendSocket ) {
	    statusBar()->message("SendSocket is NOT connected, abort learning!");
	    btnLrnStart->toggle();
	    return;
	} */
	lrnWalkType = tabWalkType->currentPageIndex();
		
	// getting init, lower limit, upper limit value string
	QString params;
	try {
	    params = getLrnTableInitParams(tblPWalkParams);
	    switch (lrnWalkType) {
	    case NORMAL_WALK:
		break;
	    case SKELLIPTICAL_WALK:
		params += getLrnTableInitParams(tblSkellipticalWalkParams);
		break;
	    case ELLIPTICAL_WALK:
		params += getLrnTableInitParams(tblEllipticalWalkParams);
		break;
	    default:
		dout << "Unknown WalkType: " << lrnWalkType << endl;
		return;
	    }
	} catch (const char* str) {
	    statusBar()->message("Error: " + QString(str));
	    btnLrnStart->toggle();
	    return;		
	}
	WalkBasePyLink::setInitParams(params.ascii());
	
	btnLrnStart->setText("Stop Learning");
	sleep(1);
	tblOptResults->setNumRows(0);
	getAndSendNewPolicy();
    } else {
	btnLrnStart->setText("Start Learning!");
    }
}

QString WalkBase::getLrnTableInitParams(QTable* tbl) {
    QString str;
    QString value, min, max;
    QTextOStream ts(&str);
    int numRows = tbl->numRows();
    for (int i=0; i<numRows; i++) {
	if ( !((QCheckTableItem*)tbl->item(i, VALID_COL))->isChecked())
	    continue;
	value = tbl->text(i, INIT_VALUE_COL);
	min = tbl->text(i, LOWER_LIMIT_COL);
	max = tbl->text(i, UPPER_LIMIT_COL);
	if (value == "" || min == "" || max == "")  {
	    throw "Invalid init values or limit values";
	}
	if (min > max) {
	    throw "Lower limit is greater than upper limit";
	}
	ts << value << " " << min << " " << max << " ";
    }
    return str;
}


void WalkBase::slotLrnSendInitParams() {    
    QString s;
    QTextOStream ts(&s);
    // read the init parameters      
    for(int i=0;i<tblPWalkParams->numRows();i++) {
	ts << tblPWalkParams->text(i, INIT_VALUE_COL) << " ";
    }
    if (lrnWalkType == SKELLIPTICAL_WALK)
	for(int i=0;i<tblSkellipticalWalkParams->numRows();i++) {
	ts << tblSkellipticalWalkParams->text(i, INIT_VALUE_COL) << " ";
    }
    if (btnStandStill->isOn())
	btnStandStill->toggle();
    sendLrnParams(cmbMode->currentItem(), s);
    gettimeofday(&tval, NULL);
    timeStamp = tval.tv_sec * 1000 + tval.tv_usec / 1000;
}


void WalkBase::updateLrnParams(int col, vector<double>& params) {
    // update the walk param table
    int paramIdx = updateLrnParamsTable(tblPWalkParams, col, params, 0);    
    if (lrnWalkType == SKELLIPTICAL_WALK)
	updateLrnParamsTable(tblSkellipticalWalkParams, col, params, paramIdx);
    else if (lrnWalkType == ELLIPTICAL_WALK)
	updateLrnParamsTable(tblEllipticalWalkParams, col, params, paramIdx);    
}

/* update walk param table, and return what's the current index*/
int WalkBase::updateLrnParamsTable(QTable* tbl, int col, vector<double>& params, int idx) {
    int numRows = tbl->numRows();
    for(int i=0; i<numRows; i++) {
	if ( ((QCheckTableItem *)tbl->item(i, VALID_COL))->isChecked()) {
	    //&& tbl->text(i, CURR_VALUE_COL) != "N/A" ) {
	    tbl->setText(i, col, QString::number(params[idx]));
	    idx++;
	}
	else { // if this attribute is not required to be learned
	    tbl->setText(i, col, "N/A");
	}	
    }
    return idx;
}


/* ask the python algorithm to get a new policy and send it to the dog*/
void WalkBase::getAndSendNewPolicy() {  
    currPolicy = WalkBasePyLink::getNewPolicy();
    if (!currPolicy) {
	statusBar()->message("Error calling WalkBasePyLink::getNewPolicy()");
	btnLrnStart->toggle();
	return;
    }
    if (currPolicy->id == -1) {
	statusBar()->message("Finished Learning!");
	slotSendRelaxRobotCmd();	
	btnLrnStart->toggle();
	return;
    }
    updateLrnParams(CURR_VALUE_COL, currPolicy->params);
    
    // parse the parameter into a string
    QString params;
    QTextOStream ts2(&params);
    for(unsigned int i=0; i<currPolicy->params.size(); i++)
	ts2 << currPolicy->params[i] << " ";
    edtLearningCurrentParams->setText(params);
    
    
    // make new dataset
    DataSet newSet;
    dataSets.push_back(newSet);
    dataSets.back().dataVector.clear();
    cmbDataSets->insertItem("LRN: "+params);
    if (dataSets.size() == 1)
	currSetIndex = 0;
    
    // update result table
    int numRows = tblOptResults->numRows();
    tblOptResults->insertRows(numRows);
    tblOptResults->setText(numRows, 0, QString::number(currPolicy->id));
    tblOptResults->setText(numRows, 1, params);
    // send new policy to the dog
    slotLrnSendCurrentParams();
}


void WalkBase::slotLrnTabWalkTypeChanged() {
    if (isLearning) 
	return;  
    lrnWalkType = tabWalkType->currentPageIndex();
}

void WalkBase::initGUILearningParams() {
    int i;
    QTable* tbl;
    // Common PWalk or Normal Walk Params
    static const char* NOR_PARAMS_NAMES[] = {
	"Forward",
	"Left",
	"Turn",
	"PG",
	"Front Height (hf)",  
	"Back Height (hb)",
	"Front Liftup Height (hdf)",
	"Back Liftup Height (hdb)",
	"Front Forward Offset (ffo)",
	"Front Sideways Offset (fso)",
	"Back Forward Offset (bfo)",
	"Back SideWays Offset (bso)"    
    };
    tbl = tblPWalkParams;
    initLearningParamsTable(tbl, NOR_PARAMS_NUM, NOR_PARAMS_NAMES);
    // unit column
    for (i = 0; i<3; i++)
	tbl->setText(i, UNIT_COL, "cm / Step");
    tbl->setText(3, UNIT_COL, "points / Half Step");
    for (i = 4; i < NOR_PARAMS_NUM; i++)
	tbl->setText(i, UNIT_COL, "mm");
    
    // Extra Elliptical Walk Params
    static const char* ELI_PARAMS_NAMES[] = {
	"Front Width",
	"Front Height",
	"Back Width",
	"Back Height",
	"Turn Adjustment"    
    };
    tbl = tblEllipticalWalkParams;
    initLearningParamsTable(tbl, ELI_PARAMS_NUM, ELI_PARAMS_NAMES);
    // unit column
    for (i=0;i<ELI_PARAMS_NUM; i++) {
	tbl->setText(i, UNIT_COL, "mm");
    }
    
    // Extra Skelliptical Walk Params
    static const char* SKE_PARAMS_NAMES[] = {
	"Front Turn/Left Height", // 0
	"Back Turn/Left Height",
	"Front Duty Cycle",
	"Back Duty Cycle",
	"Front Lead In Frac.",
	"Front Lead Down Frac.", // 5
	"Front Lead Out Frac.",
	"Front Lead Up Frac.",
	"Back Lead In Frac.",
	"Back Lead Down Frac.",
	"Back Lead Out Frac.", // 10
	"Back Lead Up Frac.",
	"Thrust Height",
	"Canter Height",
	"Side Offset",
	"Turn Center Forward", // 15
	"Turn Center Left",
	"Front Height for Sideways",
	"Front Back Forward Length Ratio",
	"Front Back Left Length Ratio",
	"Front Roll Walk Ratio", // 20	
	"Front Paw Home Angle"
    };
    
    tbl = tblSkellipticalWalkParams;
    initLearningParamsTable(tbl, SKE_PARAMS_NUM, SKE_PARAMS_NAMES);
    // unit column
    for (i=0;i<2; i++)
	tbl->setText(i, UNIT_COL, "mm");
    for (i=2;i<12;i++)
	tbl->setText(i, UNIT_COL, "%");
    for (i=12;i<18;i++)
	tbl->setText(i, UNIT_COL, "mm");
    for (i=18; i<21;i++)
	tbl->setText(i, UNIT_COL, "%");
    tbl->setText(i, UNIT_COL, "degrees");
}

void WalkBase::slotLrnContinue() {
    ignoreActionDataOnce = true;
    slotLrnSendCurrentParams();
}

void WalkBase::slotLrnSendCurrentParams() {
    // change to necessary mode and fwd/lft/trn values
    int mode = cmbMode->currentItem();
    double forward = getWalkParam(tblPWalkParams, 0, CURR_VALUE_COL).toDouble();
    double left = getWalkParam(tblPWalkParams, 1, CURR_VALUE_COL).toDouble();
    double turnccw = getWalkParam(tblPWalkParams, 2, CURR_VALUE_COL).toDouble();    
    if (mode == FWD_LFT_TRN_ALL) {
	int size = 0;
	if (!dataSets.empty())
	    size = dataSets.back().dataVector.size();
	if (size < 8) {
	    mode = FORWARD_ONLY;
	    if (size >= 4)
		forward *= -1;
	}
	else if (size < 16) {
	    mode = SIDEWAYS_ONLY;
	    if (size >= 12)
		left *= -1;	    
	}
	else if (size < 24) {
	    mode = TURN_ONLY;
	    if (size >= 20)
		turnccw *= -1;	    	    
	}
    }
    
    QString s;
    QTextOStream ts(&s);
    // read the current parameters
    ts << forward << " " << left << " " << turnccw << " ";
    for(int i=3;i<tblPWalkParams->numRows();i++) {
	ts << getWalkParam(tblPWalkParams, i, CURR_VALUE_COL) << " ";
    }
    if (lrnWalkType == SKELLIPTICAL_WALK)
	for(int i=0;i<tblSkellipticalWalkParams->numRows();i++) {      
	ts << getWalkParam(tblSkellipticalWalkParams, i, CURR_VALUE_COL) << " ";
    }
    // send the command    
    sendLrnParams(mode, s);
}

QString WalkBase::getWalkParam(QTable* tbl, int row, int col) {
    QString currValueStr = tbl->text(row, col);
    if ( currValueStr == "N/A" ||
	 !((QCheckTableItem *)tbl->item(row, VALID_COL))->isChecked() )
	return tbl->text(row, INIT_VALUE_COL);
    return currValueStr;
}

void WalkBase::sendLrnParams(int mode, QString params) {
    QString cmd;
    QTextOStream ts(&cmd);
    ts.precision(2);
    ts << "9 lrn " << mode << " "
	    << lrnWalkType << " "
	    << getGainOption() << " "
	    << edtTurnAdjustment->text().toInt() << " "
	    << edtMaxTurningAngle->text().toInt() << " "
	    << params;
    // send the command out
    sendPythonCommand(cmd);
}
