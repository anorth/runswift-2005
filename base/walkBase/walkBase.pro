TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release debug

LIBS	+= -lpython2.3

DEFINES	+= OFFLINE

INCLUDEPATH	+= ../../robot/pythonlib/include/python2.3

HEADERS	+= walkBase.h \
	walkBasePyLink.h \
	../../robot/share/ActuatorWirelessData.h

SOURCES	+= calWalk.cpp \
	main.cpp \
	../../robot/share/CommonData.cc \
	walkBasePyLink.cpp \
	lrnWalk.cpp \
	walkBase.cpp \
	jointDebug.cpp

FORMS	= formWalkBase.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

