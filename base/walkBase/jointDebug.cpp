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


#include "walkBase.h"
#include "../../robot/share/SharedMemoryDef.h"
#include "../../robot/share/ActuatorWirelessData.h"

#include <fstream>


const JDPlot::QPenSet JDPlot::pens[] = {
    // normal pen, stuck pen
    {QPen(Qt::green, 2), QPen(Qt::red, 2)},
    {QPen(Qt::yellow, 2), QPen(Qt::magenta, 2)},
    {QPen(Qt::cyan, 2), QPen(Qt::blue, 2)}
};
const QPen JDPlot::baseline(Qt::darkGray, 1);
const QPen JDPlot::thresholdLine(Qt::darkRed, 1);
const int JDPlot::INTERVAL = 1;
const int JDPlot::STEP_INTERVAL = 5;
bool JDPlot::stopUpdate = false;
QLabel* JDPlot::lblValue = NULL;

void WalkBase::initJointDebug() {
    //dout << "ActuatorWirelessDataEntrysize: " << sizeof(ActuatorWirelessDataEntry) << endl;
    
    static QFrame* plotFrames[] = {
	frmFLJoint,
	frmFLShoulder,
	frmFLKnee,
	frmFRJoint,
	frmFRShoulder,
	frmFRKnee,
	frmBLJoint,
	frmBLShoulder,
	frmBLKnee,
	frmBRJoint,
	frmBRShoulder,
	frmBRKnee,
	frmAccelForward,
	frmAccelSide,
	frmAccelZ
    };
    for (int i=0; i<15; i++) {
	QGridLayout* layout = new QGridLayout(plotFrames[i]);
	plots[i] = new JDPlot(plotFrames[i]);	
	layout->addWidget(plots[i], 0, 0);
	plots[i]->show();
    }
    for (int i=12; i<15;i++) {
	plots[i]->setRange(140);
    }
    
    JDPlot::lblValue = lblPlotValue;
    
    spxJointsRange->setValue(1500);
    spxShouldersRange->setValue(1000);
    spxKneesRange->setValue(1500);    
}

void WalkBase::slotJointDebugToggle() {
    static const char* name = "jd";
    static const char* value = "9 0";
    sendCommand(name, value);
}

// this function read the joints values sent by the dog
void WalkBase::readJointValues(QIODevice* input) {
    static WDataInfo header;
    static unsigned int remainDataReadSize = 0;
    static char* dataEntryPtr = NULL;
    static ActuatorWirelessDataEntry awd[20];
    static bool fileOpen = false;
    static int entryCount = 0;
    static std::ostream *actFile = NULL;	  
    if (!fileOpen) {
	actFile = new std::ofstream("../offactuator/actuator.dat", ios::out);
	fileOpen = true;
    }
    
    unsigned int size;
    if (remainDataReadSize == 0) {
	// read header first
	if (readAllBlock(input, (char *)&header, sizeof(header)) == -1)
	    return;
	
	/*FFCOUT << "Sending robot ID: " << header.robot
		<< " data type:" << header.type
		<< " data size: " << header.size
		<< " unit size: " << sizeof(ActuatorWirelessDataEntry)
		<< endl; */
	
	dataEntryPtr = (char*)&awd;
	size = header.size;	
	
	if (header.type != shmdtActuatorMessage) {
	    // skip all other data type
	    input->at(input->at() + size);
	    return;
	}	

	entryCount = size / sizeof(ActuatorWirelessDataEntry);
	if (entryCount >= 20) {
	    dout << "too many data points at once!" << endl;
	    return;
	}
    } else {
	size = remainDataReadSize;
    }
    int readCount = readAllBlock(input, dataEntryPtr, size);
    if (readCount == -1) {
	dout << "Error reading the actuator data!" << endl;
	return;
    }    
    remainDataReadSize = size - readCount;
    if (remainDataReadSize > 0) {
	dataEntryPtr += readCount;
	//FFCOUT << "more data to read, size: " << remainDataReadSize << endl;
	return;
    }

    for (int i=0; i<entryCount; i++) {
	awd[i].dtoh();
	awd[i].simplePrint(*actFile);
	*actFile << " "<< btnStuck->isOn() << endl;		
	bool plotted = true;
	if (awd[i].type == actSensorDataT) {
	    actSensorData s = awd[i].s;
	    long num = s.data[asPWMFrontLeftJoint];
	    lblTemp->setText(QString::number(num));		    
	    bool pwmOn = false;
	    bool sensorOn = false;
	    switch (cmbJointType->currentItem()) {
	    case 0: // pwm value
		pwmOn = true;
		break;
	    case 1: // sensor value
		sensorOn = true;
		break;
	    case 2: // both
		pwmOn = true;
		sensorOn = true;
		break;
	    default:
		plotted = false;
		break;
	    }
	    if (pwmOn) {
		for(int i=0; i<12; i++)
		    plots[i]->plot(0, s.data[i+asPWMFrontLeftJoint]);
	    } 	
	    if (sensorOn) {
		for(int i=0; i<12; i++)
		    plots[i]->plot(1, MICRO2DEG(s.data[i+asFrontLeftJoint]), true); // is an angle
	    }
	    if (plotted) {
		for(int i=0; i<15; i++)
		    plots[i]->proceed(JDPlot::INTERVAL);
	    }		    
	} else if (awd[i].type == actSensorPeakT) {
	    actSensorPeak p = awd[i].p;
	    double num = p.average[0]/100000.0;
	    lblTemp->setText(QString::number(num));
	    switch (cmbJointType->currentItem()) {
	    case 3: // peak pwm;
		for (int i=0; i<12; i++) {
		    plots[i]->plot(0, p.min[i+asPWMFrontLeftJoint]);
		    plots[i]->plot(1, p.max[i+asPWMFrontLeftJoint]);
		}	
		break;
	    case 4:// peak sensor
		for (int i=0; i<12; i++) {
		    plots[i]->plot(0, MICRO2DEG(p.min[i+asFrontLeftJoint]));
		    plots[i]->plot(1, MICRO2DEG(p.max[i+asFrontLeftJoint]));
		}		    
		break;
	    default:
		plotted = false;
		break;
	    }
	    for(int i=0; i<3;i++) { // first 3 are accelerators
		plots[i+12]->plot(0, p.min[i]/100000.0);
		plots[i+12]->plot(1, p.max[i]/100000.0);
		plots[i+12]->plot(2, p.average[i]/100000.0);
		plotted = true;
	    }	    
	    if (plotted) {
		for(int i=0; i<15; i++)
		    plots[i]->proceed(JDPlot::STEP_INTERVAL);
	    }	
	}
    }	    
}

void WalkBase::slotBtnStuckToggled(bool on) {
    if (on) {
	btnStuck->setText("Set Free!");
    } else {
	btnStuck->setText("Set Stuck!");
    }
    for(int i=0; i<15; i++) 
	plots[i]->setStuck(on);
}

void WalkBase::slotJointsRangeChanged(int range) {
    for(int i=0;i<4;i++) 
	plots[i*3]->setRange(range);
    slotCmbJointTypeChanged();
}

void WalkBase::slotShouldersRangeChanged(int range) {
    for(int i=0;i<4;i++) 
	plots[i*3+1]->setRange(range);
    slotCmbJointTypeChanged();
}

void WalkBase::slotKneesRangeChanged(int range) {
    for(int i=0;i<4;i++) 
	plots[i*3+2]->setRange(range);
    slotCmbJointTypeChanged();
}

void WalkBase::slotCmbJointTypeChanged() {
    for(int i=0; i<15; i++) 
	plots[i]->reset();
}


void WalkBase::slotSnapShot(bool on) {
    JDPlot::stopUpdate = on;
    if (on) {
	btnSnapShot->setText("Continue");
    } else {
	btnSnapShot->setText("Snap Shot!");
    }
}

/*********************************  JDPlot Methods *******************************/
JDPlot::JDPlot(QWidget * parent, const char * name, WFlags f) 
    : QWidget(parent, name, f) {
    setMouseTracking(true);
    range = 500;
    isStuck = false;
    thresholdValue = 0;
    pixmap = new QPixmap(width(), height());
    painter = new QPainter(pixmap);    
    reset();
};

JDPlot::~JDPlot() {
    painter->end();
    if (painter)
	delete painter;
    if (pixmap)
	delete pixmap;
}

void JDPlot::plot(int id, double value, bool isAngle) {
    if (id >=NUM_LINES) {
	return; // if out of bound
    }
    if (stopUpdate)
	return;    
    int y = value2Y(value, isAngle);
    if (isStuck)
	painter->setPen(pens[id].stuck);
    else
	painter->setPen(pens[id].normal);

    // now plots
    painter->drawLine(currX-interval, lastY[id], currX, y);
    // clear the region ahead
    int space = (int)(width() *0.1);
    painter->fillRect(currX+1, 0, space, height(), QBrush(Qt::black));
    painter->setPen(thresholdLine);    
    int thresholdY = value2Y(thresholdValue);
    painter->drawLine(currX+1, thresholdY, currX+space+1, thresholdY);    
    painter->setPen(baseline);
    painter->drawLine(currX+1, height()/2, currX+space+1, height()/2);

    // finally
    lastY[id] = y;    
}


void JDPlot::proceed(int intr) {
    if (stopUpdate)
	return;    
    interval = intr;
    currX = (currX + interval) % width();
    update();
}

void JDPlot::reset() {
    painter->fillRect(0, 0, width(), height(), QBrush(Qt::black));
    drawThresholdLine();
    painter->setPen(baseline);
    painter->drawLine(0, height()/2, width(), height()/2);
    currX = 0;
    lastY[0] = lastY[1] = height()/2;
    update();
}

void JDPlot::drawThresholdLine(bool clear) {
    if (thresholdValue == 0)
	return; // don't draw it if the overlap the central line
    if (clear)
	painter->setPen(QPen(Qt::black, 1));
    else
	painter->setPen(thresholdLine);    	    	
    int thresholdY = value2Y(thresholdValue);
    painter->drawLine(0, thresholdY, width(), thresholdY);    
}

int JDPlot::value2Y(double value, bool isAngle) {
    return (int)((0.5-value/2.0/ (isAngle?180:range) ) * height());
}

double JDPlot::y2Value(double y, bool isAngle) {    
    return ( 0.5- y / height() ) * 2 * ( isAngle ? 180 : range );
}

void JDPlot::setStuck(bool s) {
    isStuck = s;    
}

void JDPlot::setRange(int r) {
    range = r;
}

void JDPlot::mouseMoveEvent(QMouseEvent* e) {
    double tempY = 0.5 - (double)e->y() / height();
    double value = tempY * 2* range;
    double degree = tempY * 360;
    lblValue->setText(QString::number(value)+" or "
		      +QString::number(degree)+" degrees, "
		      +"Threshold: " + QString::number(thresholdValue));
}

void JDPlot::mousePressEvent(QMouseEvent* e) {
    drawThresholdLine(true);
    thresholdValue = y2Value(e->y());
    drawThresholdLine();
    update();
}


void JDPlot::resizeEvent(QResizeEvent* /*e*/) {
    // assuime pixmap exists
    if (pixmap->height() != height() or pixmap->width() != width() ) {
	if (painter)
	    delete painter;
	delete pixmap;
	pixmap = new QPixmap(width(), height());	
	painter = new QPainter(pixmap);
	reset();
    }
}

void JDPlot::paintEvent(QPaintEvent* e) {
    bitBlt (this, e->rect().topLeft(), pixmap, e->rect(), Qt::CopyROP);
}
