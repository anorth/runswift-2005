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


#include <ostream>
#include "hton.h"
#include "Common.h"

#ifndef ActuatorWirelessData_h_
#define ActuatorWirelessData_h_

enum enumActSensorData {
  asAccelFor, // 0
  asAccelSide,
  asAccelZ,
  asHeadTilt, 
  asHeadPan, 
  asHeadCrane, // 5
  asFrontLeftJoint,
  asFrontLeftShoulder, 
  asFrontLeftKnee,
  asFrontRightJoint,
  asFrontRightShoulder, // 10
  asFrontRightKnee,
  asRearLeftJoint,
  asRearLeftShoulder,
  asRearLeftKnee,
  asRearRightJoint, // 15
  asRearRightShoulder,
  asRearRightKnee,
  asPWMHeadTilt,
  asPWMHeadPan, 
  asPWMHeadCrane, // 20
  asPWMFrontLeftJoint, 
  asPWMFrontLeftShoulder, 
  asPWMFrontLeftKnee,
  asPWMFrontRightJoint, 
  asPWMFrontRightShoulder, //25
  asPWMFrontRightKnee,
  asPWMRearLeftJoint, 
  asPWMRearLeftShoulder, 
  asPWMRearLeftKnee,
  asPWMRearRightJoint, // 30
  asPWMRearRightShoulder, 
  asPWMRearRightKnee,
  ACT_SENSOR_DATA_SIZE // 33
};


struct actSensorPeak {
  long frameID;
  unsigned int stepID;
  long min[ACT_SENSOR_DATA_SIZE];
  long max[ACT_SENSOR_DATA_SIZE];
  double average[3];
  unsigned int count;
  int padding; // to align up to multiples of 8 bytes
 
  void reset() {
    stepID = 0;
    count = 0;
    int i;
    for(i=0;i<ACT_SENSOR_DATA_SIZE;i++) {
      min[i] =  VERY_LARGE_INT;
      max[i] = -VERY_LARGE_INT;
    }
    for(i=0;i<3;i++)
      average[i] = 0;
  }
  
  long checkMinMax(int type, int value) {
    // check min
    if (value < min[type])
      min[type] = value;
    // check max
    if (value > max[type])
      max[type] = value;
    if (type < 3) {
      // calculate average for the accelerators
      average[type] = (average[type] * count + value) / (count + 1);
      if (type == 2) // to increse the counter once every full update
	++count;
    }
    return value;
  }
  
  void dtoh() {
    frameID = dtohl(frameID);
    stepID = dtohl(stepID);
    int i;
    for(i=0;i<ACT_SENSOR_DATA_SIZE;i++) {
      min[i] = dtohl(min[i]);
      max[i] = dtohl(max[i]);
    }
    for(i=0;i<3;i++) {
      average[i] = dtohl(average[i]);
    }
  }

  void htod() {
    frameID = htodl(frameID);
    stepID = htodl(stepID);
    int i;
    for(i=0;i<ACT_SENSOR_DATA_SIZE;i++) {
      min[i] = htodl(min[i]);
      max[i] = htodl(max[i]);
    }
    for(i=0;i<3;i++) {
      average[i] = htodl(average[i]);
    }
  }
};


struct actSensorData {
  long frameID;
  long data[ACT_SENSOR_DATA_SIZE];

  void dtoh() {
    frameID = dtohl(frameID);
    for(int i=0;i<ACT_SENSOR_DATA_SIZE;i++)
      data[i] = dtohl(data[i]);
  }

  void htod() {
    frameID = htodl(frameID);
    for(int i=0;i<ACT_SENSOR_DATA_SIZE;i++)
      data[i] = htodl(data[i]);
  }	
};

struct actCommandData {
  long lastSensedFrameID;
    long cmdHeadTilt, cmdHeadPan, cmdHeadCrane;
    long cmdFrontLeftJoint, cmdFrontLeftShoulder, cmdFrontLeftKnee;
    long cmdFrontRightJoint, cmdFrontRightShoulder, cmdFrontRightKnee;
    long cmdRearLeftJoint, cmdRearLeftShoulder, cmdRearLeftKnee;
    long cmdRearRightJoint, cmdRearRightShoulder, cmdRearRightKnee;

	void dtoh() {
		lastSensedFrameID = dtohl(lastSensedFrameID);
    	cmdHeadTilt = dtohl(cmdHeadTilt);
		cmdHeadPan = dtohl(cmdHeadPan);
		cmdHeadCrane = dtohl(cmdHeadCrane);
    	cmdFrontLeftJoint = dtohl(cmdFrontLeftJoint);
		cmdFrontLeftShoulder = dtohl(cmdFrontLeftShoulder);
		cmdFrontLeftKnee = dtohl(cmdFrontLeftKnee);
    	cmdFrontRightJoint = dtohl(cmdFrontRightJoint);
		cmdFrontRightShoulder = dtohl(cmdFrontRightShoulder);
		cmdFrontRightKnee = dtohl(cmdFrontRightKnee);
    	cmdRearLeftJoint = dtohl(cmdRearLeftJoint);
		cmdRearLeftShoulder = dtohl(cmdRearLeftShoulder);
		cmdRearLeftKnee = dtohl(cmdRearLeftKnee);
    	cmdRearRightJoint = dtohl(cmdRearRightJoint);
		cmdRearRightShoulder = dtohl(cmdRearRightShoulder);
		cmdRearRightKnee = dtohl(cmdRearRightKnee);
	}

	void htod() {
		lastSensedFrameID = htodl(lastSensedFrameID);
    	cmdHeadTilt = htodl(cmdHeadTilt);
		cmdHeadPan = htodl(cmdHeadPan);
		cmdHeadCrane = htodl(cmdHeadCrane);
    	cmdFrontLeftJoint = htodl(cmdFrontLeftJoint);
		cmdFrontLeftShoulder = htodl(cmdFrontLeftShoulder);
		cmdFrontLeftKnee = htodl(cmdFrontLeftKnee);
    	cmdFrontRightJoint = htodl(cmdFrontRightJoint);
		cmdFrontRightShoulder = htodl(cmdFrontRightShoulder);
		cmdFrontRightKnee = htodl(cmdFrontRightKnee);
    	cmdRearLeftJoint = htodl(cmdRearLeftJoint);
		cmdRearLeftShoulder = htodl(cmdRearLeftShoulder);
		cmdRearLeftKnee = htodl(cmdRearLeftKnee);
    	cmdRearRightJoint = htodl(cmdRearRightJoint);
		cmdRearRightShoulder = htodl(cmdRearRightShoulder);
		cmdRearRightKnee = htodl(cmdRearRightKnee);
	}
};

enum ActuatorWirelessDataEntryType{
  actSensorDataT,
  actCommandDataT,
  actSensorPeakT
};

struct ActuatorWirelessDataEntry {
  long type;
  long padding; // to align to multiples of 8 bytes
  
  union {
    actSensorData s;
    actCommandData c;
    actSensorPeak p;
  };
	
  void dtoh() {
    type = dtohl(type);
    switch (type) {
    case actSensorDataT:
      s.dtoh();
      break;
    case actCommandDataT:
      c.dtoh();
      break;
    case actSensorPeakT:
      p.dtoh();
      break;
    default:
      break;
    }
  }

  void htod() {
    switch (type) {
    case actSensorDataT:
      s.htod();
      break;
    case actCommandDataT:
      c.htod();
      break;
    case actSensorPeakT:
      p.htod();
      break;
    default:
      break;
    }
    type = htodl(type);
  }

  void simplePrint(std::ostream &out) {
    switch (type) {
    case actSensorDataT:
      out << s.frameID;
      for (int i=0; i<ACT_SENSOR_DATA_SIZE; i++)
	out << ' ' << s.data[i];
      break;
    case actSensorPeakT:
      out << p.frameID << ' ' << p.stepID;
      for (int i=0; i<ACT_SENSOR_DATA_SIZE; i++)
	out << ' ' << p.min[i] << ' ' << p.max[i];
	break;
    case actCommandDataT:
      out << "to be implemented" << endl;
      break;
    default:
      out << "Unkown type" << endl;
      break;
    }
  }

};

static inline std::ostream &operator<<(std::ostream &out , const actSensorData &s){
  out << s.frameID;
  for (int i=0; i<ACT_SENSOR_DATA_SIZE; i++)
    out << ' ' << s.data[i];
  return out;
}



static inline std::ostream &operator<<(std::ostream &out , const actCommandData &a){
	out << '[';	
	out << a.lastSensedFrameID << endl;
    out << '[' << a.cmdHeadTilt << ", " << a.cmdHeadPan << ", " << a.cmdHeadCrane << ']' << endl;
    out << '[' << a.cmdFrontLeftJoint << ", " << a.cmdFrontLeftShoulder << ", " << a.cmdFrontLeftKnee << ']' << endl;
    out << '[' << a.cmdFrontRightJoint << ", " << a.cmdFrontRightShoulder << ", " << a.cmdFrontRightKnee << ']' << endl;
    out << '[' << a.cmdRearLeftJoint << ", " << a.cmdRearLeftShoulder << ", " << a.cmdRearLeftKnee << ']' << endl;
    out << '[' << a.cmdRearRightJoint << ", " << a.cmdRearRightShoulder << ", " << a.cmdRearRightKnee << ']' << endl;
	out << ']' << endl;

	return out;
}

static inline std::ostream &operator<<(std::ostream &out , const ActuatorWirelessDataEntry &a){
  switch (a.type) {
  case actSensorDataT:
    out << "[ sensor " << a.s << ']';
    break;
  case actCommandDataT:
    out << "[ command " << a.c << ']';
    break;
  default:
    out << "[ Unknown ActuatorWirelessData type ]";
    break;
  }
  return out;
}

#endif // ActuatorWirelessData_h_
