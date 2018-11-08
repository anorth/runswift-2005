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


#ifndef __WalkBase_h_
#define __WalkBase_h_

#include "walkBasePyLink.h"
#include "formWalkBase.h"
#include "../robolink/robolink.h"
#include "../../robot/share/PackageDef.h"
#include "../../robot/share/PWalkDef.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/time.h>

#include <qsocket.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qsocketdevice.h>
#include <qtable.h>
#include <qstatusbar.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qpen.h>
#include <qpainter.h>
#include <qlayout.h>

using namespace std;

//#define DEBUG_ALL
#define PI 3.1415927
#define SEND_SOCKET_PORT 54321
#define RECEIVE_SOCKET_PORT 9000

extern const char *walkTypeToStr[]; //is defined in CommonData.cc
#define RUNS_PER_POLICY 4

typedef struct { // atomic action with extra field for validity
    struct rlnk_atomicaction action;
    bool isValid;
} AActionData;

typedef struct {
    struct rlnk_atomicaction params;
    double forwardDist;
    double leftDist;
    double turnAmount;
    vector <AActionData> dataVector;
} DataSet;

typedef vector<DataSet> DataSetVector;

// this enum correspond to the contents in cmb
enum WalkCommandType {
    CALIBRATION,
    LEARNING
};

// this enum has to correspond to the contents in cmbMode
enum WalkModeEnum {
    FORWARD_ONLY,
    SIDEWAYS_ONLY,
    TURN_ONLY,
    GRAB_TURNING,
    GRAB_SIDEWAY,
    FORWARD_TURN,
    SIDEWAYS_TURN,
    FWD_LFT_TRN_ALL,
    MANUAL_LEARNING
};

// this enum has to correspond to the contents in cmbWalkType
enum WalkTypeEnum {
    NORMAL_WALK,
    ELLIPTICAL_WALK,
    SKELLIPTICAL_WALK
};

class JDPlot : public QWidget {
    Q_OBJECT
    
public:
    typedef struct {
	QPen normal;
	QPen stuck;
    } QPenSet;
    static const int NUM_LINES = 3;  // can plot 3 lines simutaneously
    
protected:
    QPainter* painter;
    QPixmap* pixmap;
    int interval;
    int currX;
    int  lastY[NUM_LINES];
    double thresholdValue;
    bool isStuck;
    
public:
    static const int STEP_INTERVAL;
    static const int INTERVAL;    
    static const QPenSet pens[];
    static const QPen baseline;
    static const QPen thresholdLine;
    static bool stopUpdate;
    static QLabel* lblValue;
    int range;
   
    JDPlot(QWidget* parent = 0, const char * name = 0, WFlags f = 0);    
    ~JDPlot();  
    void mouseMoveEvent(QMouseEvent* );
    void mousePressEvent(QMouseEvent* );    
    void resizeEvent(QResizeEvent* );
    void paintEvent(QPaintEvent* e);
    void plot(int id, double value, bool isAngle=false);
    void proceed(int interval=INTERVAL);
    void drawThresholdLine(bool clear=false);
    int value2Y(double, bool isAngle=false);
    double y2Value(double, bool isAngle=false);
    void reset();
    void setStuck(bool s);
    void setRange(int r);
};


class WalkBase : public FormWalkBase {
    Q_OBJECT

public:
    WalkBase(QApplication* app, QWidget* parent = 0,
      const char* name = 0, WFlags fl = 0 );
    ~WalkBase();

public slots:
    // common slots
  void slotCalForwardActual(void);
  void slotCalLeftActual(void);
  void slotCalTurnActual(void);
  void slotClearData(void);    
  void slotCmbDataSetsChanged(int);  
  void slotCmbModeChanged(int);  
  void slotEdtForwardDistChanged(void);
  void slotEdtLeftDistChanged(void);
  void slotEdtTurnAmountChanged(void);  
  void slotFileOpen(void);  
  // calibration slots
  void slotCalSaveResult(void);  
  void slotCalLoadResult(void);    
   // learning slots
  void slotLrnDrawLocus(void);
  void slotLrnLoadInitParams(void);  
  void slotLrnPolicyTooSlow();
  void slotLrnSaveInitParams();
  void slotLrnSendCurrentParams(void);  
  void slotLrnSendInitParams(void);
  void slotLrnStarted(bool);  
  void slotLrnTabWalkTypeChanged(void);
  void slotLrnContinue(void);
  // joint debug slots
  void slotJointDebugToggle(void);
  void slotBtnStuckToggled(bool);
  void slotJointsRangeChanged(int);
  void slotShouldersRangeChanged(int);
  void slotKneesRangeChanged(int);
  void slotCmbJointTypeChanged();
  void slotSnapShot(bool);
  
  // common slots
  void slotReceiveSocketClosed(void);  
  void slotReceiveSocketConnect(void);
  void slotReceiveSocketConnected(void);
  void slotReceiveSocketError(int);
  void slotReceiveSocketReadyRead(void);
  
  void slotResultCopyOver();
  
  void slotSaveData(void);
  
  void slotSendHeadParams(void);
  
  void slotScheduleStarted(bool);  
  void slotStandStill(bool);
  
  void slotCalSendCommand(void);  
  void slotSendRelaxRobotCmd(void);  
    
  void slotSendSocketClosed(void);  
  void slotSendSocketConnect(void);
  void slotSendSocketConnected(void);
  void slotSendSocketError(int);
  void slotSendSocketReadyRead(void);
  
  void slotSendTurnAdjustmentCmd(void);
  void slotSendTurnOffRobotCmd(void);  
  
  void slotTblDataChanged(int, int);

    
protected:
    // protected data
    DataSetVector dataSets;
    int currSetIndex; // current display dataset    
    bool isLearning; // true = walk learning, false = walk calibration
    Policy* currPolicy;
    QApplication* app;    
    QIODevice* file;
    QSocket* receiveSocket;
    QSocket* sendSocket;
    double timeStamp;
    struct timeval tval;    
    
    // attributes for walk learning
    int lrnWalkType;
    bool ignoreActionDataOnce;
    
    // attributes for Joint Debugging
    JDPlot* plots[15];
	    
    // protected methods
    void checkSchedule(void);    
    void clearGUIData();    
    void closeDevice(QIODevice*);
    
    // joint debug methods
    void readJointValues(QIODevice*);
    void initJointDebug(void);
    
    // learning methods    
    void getAndSendNewPolicy();    
    QString getLrnInitParams();
    QString getLrnTableInitParams(QTable*);
    void initGUILearningParams(void);    
    void initLearningParamsTable(QTable*, int, const char**);
    void updateLrnParams(int, vector<double>&);
    int updateLrnParamsTable(QTable*, int, vector<double>&, int);
    void saveLrnParamsToFile(QString, int, double);
    void sendLrnParams(int, QString);
    QString getWalkParam(QTable*, int, int);
    void WalkBase::saveLrnParams(QTextStream& ts, QTable* tbl, int row, int col);
    
    // common methods
    char getGainOption(void);
    QString getQSocketErrorMsg(int);
    void gotNewAction(QIODevice*);    
    
    void initNewDataSet(const struct rlnk_atomicaction&);
    void insertDataTableEntry(const AActionData&);    
    
    Q_LONG readAllBlock(QIODevice*, char*, Q_ULONG);
    void readFromDevice(QIODevice*);    
   
    void sendPythonCommand(const QString&);
    bool sendCommand(const char*, const char*);

    void sendCalCommand(double, double, double);    
    void sendScheduleCommand();
    
    void updateData();
    void updateLearningResults();
};

#endif //__WalkBase_h_
