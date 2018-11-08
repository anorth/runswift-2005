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


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "share/ActionProDef.h"
#include "share/PWalkDef.h"
#include "share/Common.h"
#include "share/WalkLearningParams.h"
#include "actuatorControl/ActionPro.h"


using namespace std;

ActionPro actionPro;

AtomicAction action;

int PG;

void initialize(){
    actionPro.initMem();
}

#if TEMP_COMMENTED_OUT
void testPWalk(){
    PWalk pAgent;

    double theta1, theta2, theta3;
    double x,y,z;
    while ( cin >> x >> y >> z){
        pAgent.backJointAngles(x,y,z, theta1,theta2,theta3);
        cout << " T1 = " << RAD2DEG(theta1) <<  "  T2 = " << RAD2DEG(theta2) << "  T3 = " << RAD2DEG(theta3) << endl;
        pAgent.frontJointAngles(x,y,z, theta1,theta2,theta3);
        cout << " T1 = " << RAD2DEG(theta1) <<  "  T2 = " << RAD2DEG(theta2) << "  T3 = " << RAD2DEG(theta3) << endl;
    }
}
#endif


// read a walk parameter file, and set them to the walk
void setWalkParams(fstream &fin) {
  struct WalkLearningParams params;
  double temp, value;
  double fwd, lft, trn;
  string str;  

    
  fin >> temp; // mode, IGNORED!
  fin >> temp >> temp >> temp; // turn adj, max turn angle, gain, IGNORED!
  fin >> str;
  fin >> action.minorWalkType;

  if (str == "SkellipticalWalkWT") {
    params.walkType = SkellipticalWalkWT;    
  } 
  FFCOUT << "WalkType: " << params.walkType << " "
	 << "MinorWalkType: " << action.minorWalkType << endl;

  //params.walkType = SkellipticalWalkWT;
  stringstream ss;
  fin >> temp >> fwd >> temp >> temp; // valid, value, min, max
  fin >> temp >> lft >> temp >> temp;
  fin >> temp >> trn >> temp >> temp;
  while (fin >> temp >> value >> temp >> temp) {
    ss << value << " ";
  }

  switch (params.walkType) {
  case EllipseWalkWT:
    actionPro.pAgent->ellipticalWalk->setLearningParameters(params.ep);
    break;
  case SkellipticalWalkWT:
    params.sp.read(ss);
    PG = params.sp.halfStepTime;
    params.sp.print(FFCOUT << "Setting walk params: ");
    cout << endl;
    actionPro.pAgent->skellipticalWalk->setWalkParams(params.sp);
    break;
  case SkiWalkWT:
    actionPro.pAgent->skiWalk->setWalkParams(params.skp);
    break;
  case NormalWalkWT:
    actionPro.pAgent->normalWalk->setWalkParams(params.np); 
    break;
  default:
    FFCOUT << "Unknown walktype - " << params.walkType  << endl;
    break;
  }


  action.cmd = aaParaWalk;
  action.walkType = params.walkType;
  //action.minorWalkType = 1; // learning
  action.forwardMaxStep = fwd;
  action.leftMaxStep = lft;
  action.turnCCWMaxStep = trn;
  action.forwardSpeed = fwd * (1000.0 / PG / 16);
  action.leftSpeed = lft * (1000.0 / PG / 16);
  action.turnCCWSpeed = trn * (1000.0 / PG / 16);

  FFCOUT << "fwd step: " << action.forwardMaxStep 
	 << " lft step: " << action.leftMaxStep
	 << " trn step: " << action.turnCCWMaxStep << endl;

  actionPro.execute(action);

}

// execute a walk, after the walk parameter is set
void printLocus() {
  for (int i=0;i<=PG*3;i++) {
    actionPro.ContinueStep();
  }
}


// parse the online sensor data file
void parseSensorDataFile(fstream& fin) {
  JointCommand jointCommand;
  double frame, temp;
  slongword joint, shoulder, knee; 
  double a1, a2, a3;

  ofstream p_fout("pwm.dat", ios::out);
  ofstream j_fout("joints.dat", ios::out);

  while(!fin.eof()) {
    fin >> frame;
    fin >> a1 >> a2 >> a3; //ignore acceleators
    fin >> temp >> temp >> temp; //ignore head sensors

    // front left 
    fin >> joint >> shoulder >> knee;
    jointCommand.setJointEnumValue(leftFrontJoint, joint ); 
    jointCommand.setJointEnumValue(leftFrontShoulder, shoulder ); 
    jointCommand.setJointEnumValue(leftFrontKnee, knee);
    
    // front right
    fin >> joint >> shoulder >> knee;
    jointCommand.setJointEnumValue(rightFrontJoint, joint );
    jointCommand.setJointEnumValue(rightFrontShoulder, shoulder ); 
    jointCommand.setJointEnumValue(rightFrontKnee, knee ); 
    
    // rear left
    fin >> joint >> shoulder >> knee;
    jointCommand.setJointEnumValue(leftRearJoint, joint ); 
    jointCommand.setJointEnumValue(leftRearShoulder, shoulder ); 
    jointCommand.setJointEnumValue(leftRearKnee, knee ); 
    
    // rear right
    fin >> joint >> shoulder >> knee;
    jointCommand.setJointEnumValue(rightRearJoint, joint); 
    jointCommand.setJointEnumValue(rightRearShoulder, shoulder); 
    jointCommand.setJointEnumValue(rightRearKnee, knee);

    
    actionPro.jointAgent->executeCommand(&jointCommand);


    fin >> temp >> temp >> temp; //ignore head PWM values

    // put all 4-leg pwm values into one file
    p_fout << frame;
    for(int i=0; i<4; i++) {
      fin >> joint >> shoulder >> knee;
      p_fout << " " << joint << " " << shoulder << " " << knee;
    }

    fin >> temp; //ignore stuck info for now

    p_fout << endl;
  }

  p_fout.flush();
  p_fout.close();
  j_fout.flush();
  j_fout.close();

}


int main(int argc, char* argv[]){
    cout << "Offline Actuator is running " << endl;
    string lastCmd, cmd;

    initialize();

    ofstream fout("locus.dat", ios::out);
    fout << "#locus data file" << endl;
    fout.flush();
    fout.close();

    string filename(argv[1]);
    fstream fin(argv[1], ios::in);  
    if (!fin || fin.bad()) {
      FFCERR << "cannot open file: " << filename << endl;
      exit(-1);
    }    

    if (filename == "actuator.dat") {
      FFCOUT << "Reading sensor data file" << endl;
      parseSensorDataFile(fin);
    }
    else {
      FFCOUT << "Reading walk parameters file" << endl;
      setWalkParams(fin);
      printLocus();
    }
    fin.close();
    system("gnuplot locus.plot");



    /*
    testPWalk();
    return 0;
    */

    

    /*    AtomicAction action;
    int nstep;
    while (1){
        cout << "Enter Command: " ;
        lastCmd = cmd;
        cin >> cmd;
        if (cmd == "Input" || cmd == "in"){
            //17 parameters
	  cin >> action.walkType;
            actionPro.execute(action);
            cout << "Params set " ;
            action.printFull();
        }
        else if (cmd == "ContinueStep" || cmd == "cs" ){
            actionPro.ContinueStep();
        }
        else if (cmd == "ContinueNStep" || cmd == "cns"){
            cin >> nstep;
            for (int i=0;i<nstep;i++)
                actionPro.ContinueStep();
        }
        else if (cmd == "Last Command" || cmd == "lc"){
            for (int i=0;i<nstep;i++)
                actionPro.ContinueStep();
        }
        else if (cmd == "quit" || cmd == "qt"){
            break;
        }
        else if (cmd == "Default" || cmd == "df" ){
            action.headType = ABS_H;
            action.panx = 0;
            action.tilty = 0;
            action.cranez = 0;
            actionPro.execute(action);
            cout << "Params set " ;
            action.printFull();
        }
        else if (cmd == "WalkType" || cmd == "wt"){
	  cin >> action.walkType;
            actionPro.execute(action);
            cout << "You chose walktype " << walkTypeToStr[action.walkType] << endl;
        }
        else
            cout << "Unknown command " << endl;
    }

    */

    return 0;
}

