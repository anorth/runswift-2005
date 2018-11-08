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


#include "walkBasePyLink.h"

#include <sstream>

using namespace std;
#include <iostream>



namespace WalkBasePyLink {
  static char* LearningAlgoModuleName = "lrnWalk";
  static char* WalkBasePyLinkModuleName = "WalkBasePyLink";

  // python modules
  static PyObject* LearningAlgo = NULL;
  static PyObject* WalkBasePyLink = NULL; // this file
  // python functions
  static PyObject* setEvaluation_PyFunc = NULL;
  static PyObject* getNewPolicy_PyFunc = NULL;
  static PyObject* getBestParams_PyFunc = NULL;
  static PyObject* setInitParams_PyFunc = NULL;
  
  // variable to hold function return values
  static Policy newPolicy;
  static BestParams bestParams;
  
  PyObject* WalkBasePyLink_setCallbacks(PyObject*, PyObject*);  // foward declration
  
  // WalkBasePyLink method list
  static PyMethodDef WalkBasePyLinkMethods[] = {
    {"setCallbacks", WalkBasePyLink_setCallbacks, 
     METH_VARARGS, "setting the callback functions"},
    {NULL, NULL, 0, NULL} /* Sentinel */
  };
  
  bool initPythonModule() {
    //Py_VerboseFlag += 2; // for verbose print out
    Py_Initialize();
    // seting syspath
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('.')");
    //PyRun_SimpleString("sys.path.append('./PyWalkLearner')");
    
    // make setCallBack function available to the Python module
    if ((WalkBasePyLink = PyImport_AddModule(WalkBasePyLinkModuleName)) == NULL) {
      cout << "error creating Module " << WalkBasePyLinkModuleName << endl;
      return false;
    } else if (Py_InitModule(WalkBasePyLinkModuleName, WalkBasePyLinkMethods) == NULL) {
      cout << "error linking Module " << WalkBasePyLinkModuleName << endl;
      return false;
    }
    
    // init the Python module
    if ((LearningAlgo = PyImport_ImportModule(LearningAlgoModuleName)) == NULL) {
      cout << "error importing Python File " << LearningAlgoModuleName << endl;
      return false;
    }
    return true;
  }
  
  /* callback setting function */
  PyObject* WalkBasePyLink_setCallbacks(PyObject* /* self */, PyObject* args) {
    PyObject *result = NULL;
    PyObject* func1, * func2, * func3, * func4;
    
    cout << "Setting Python WalkBasePyLink callbacks" << endl;
    
    if (PyArg_ParseTuple(args, "OOOO:setCallbacks", &func1, &func2, &func3, &func4)) {
      if ( !PyCallable_Check(func1) || !PyCallable_Check(func2) 
	  || !PyCallable_Check(func3) || !PyCallable_Check(func4) ) {
	PyErr_SetString(PyExc_TypeError, "parameter must be callable");
	return NULL;
      }
      Py_XDECREF(getNewPolicy_PyFunc); /* Dispose of previous callback */
      Py_XINCREF(func1);               /* Add a reference to new callback */
      getNewPolicy_PyFunc = func1;     /* Remember new callback */ 
      
      Py_XDECREF(setEvaluation_PyFunc);
      Py_XINCREF(func2);
      setEvaluation_PyFunc = func2;
     
      Py_XDECREF(getBestParams_PyFunc);
      Py_XINCREF(func3);
      getBestParams_PyFunc = func3;
      
      Py_XDECREF(setInitParams_PyFunc);
      Py_XINCREF(func4);
      setInitParams_PyFunc = func4;
      /* Boilerplate to return "None" */
      Py_INCREF(Py_None);
      result = Py_None;
    }
    return result;
  }
    
  // generic python function call
  PyObject* callPyFunc(PyObject* func, PyObject* args) {
    if (!func) {
      cout << "Python callback not initialized.  Unable to call it." << endl;
      return NULL;
    }
    // check python callback arguments
    if (!args) {
      dout << "Unable to build args." << endl;
      if (!PyErr_Occurred())
	dout << "But no Python exception available!?!" << endl;
      else {
	dout << " ";
	PyErr_Print();
    }
      return NULL;
    }
    // call the function
    PyObject *result = PyEval_CallObject(func, args);
    Py_DECREF(args);
    // check the return value
    if (result == NULL) {
      dout << "Got NULL from python call." << endl;
      if (PyErr_Occurred() == NULL)
	dout << "But no Python exception available!?!" << endl;
      else
	PyErr_Print();
      return NULL;
    }
    return result;
  }

  Policy* getNewPolicy() {
      newPolicy.params.clear();
      PyObject* args = Py_BuildValue("()");
      PyObject* result = callPyFunc(getNewPolicy_PyFunc, args);
      if (!result || !PyTuple_Check(result)) {
	  dout << "returning result is a NOT tuple!!" << endl;
	  return &newPolicy;
      }
      // read the first 2 items from result tuple
      PyObject* policyID = PyTuple_GetItem(result, 0);
      newPolicy.id = PyInt_AsLong(policyID);
      PyObject* params = PyTuple_GetItem(result, 1);
      if (!PyTuple_Check(params)) {
	  dout << "Second argument of returning value is a NOT tuple!!" << endl;
	  dout << "Setting Policy ID to -1." << endl;
	  newPolicy.id = -1;
	  return &newPolicy;
      }
      int size = PyTuple_Size(params);
      for(int i=0; i<size; i++) {
	  newPolicy.params.push_back(
		  PyFloat_AsDouble(
			  PyTuple_GetItem(params, i)
			  )
		  );
      }
      return &newPolicy;
  }	

  void setInitParams(const char* params) {
      PyObject* args = Py_BuildValue("(s)", params);
      callPyFunc(setInitParams_PyFunc, args);
  }
  
  void setEvaluation(int policyID, int value) {
      PyObject* args = Py_BuildValue("ii", policyID, value);
      callPyFunc(setEvaluation_PyFunc, args);
  }
  
  BestParams* getBestParams(void) {
      PyObject* args = Py_BuildValue("()");
      PyObject* result = callPyFunc(getBestParams_PyFunc, args);
      if (!result || !PyTuple_Check(result)) {
	  dout << "returning result is a NOT tuple!!" << endl;
	  return NULL;
      }
      // read the first 2 items from result tuple
      PyObject* params = PyTuple_GetItem(result, 0);
      if (!PyTuple_Check(params)) {
	  dout << "first argument of returning value is a NOT tuple!!" << endl;
	  return NULL;
      }
      int size = PyTuple_Size(params);
      bestParams.params.clear();
      for(int i=0; i<size; i++) {
	bestParams.params.push_back( PyFloat_AsDouble(PyTuple_GetItem(params, i)) );
      }
      PyObject* value = PyTuple_GetItem(result, 1);
      bestParams.value = PyFloat_AsDouble(value);
      return &bestParams;
  }
}

/*
int main() {
  if (!WalkBasePyLink::initialisePyModule())
    return -1;
  getNewPolicy();
  return 0;
}
*/
