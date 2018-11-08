unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= /home/kcph007/trunk/base7/work/SimpleRoboCommander/FormClientServer.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/CPlaneClient/FormCPlaneDisplay.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/JointDebugger/FormJointDebugger.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/WalkLearner/FormWalkLearner.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/WalkLearner/WdgRobotInfo.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/DlgPyChecker.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/DlgHelpMe.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/FormCalibraterInfo.ui \
	/home/kcph007/trunk/base7/work/SimpleRoboCommander/JoystickController/FormJoystickController.ui
TEMPLATE	=app
CONFIG	+= qt warn_on release
LANGUAGE	= C++
