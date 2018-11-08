TEMPLATE	= app
LANGUAGE	= C++

#CONFIG	+= qt warn_on release
CONFIG	+= qt warn_on debug

DEFINES	+= OFFLINE

HEADERS	+= FormSubvisionImpl.h \
	FrameReader.h

SOURCES	+= FormSubvisionImpl.cpp \
	main.cpp \	
	FrameReader.cpp

FORMS	= FormSubvision.ui \
	dlgConnect.ui

DEFINES	+= QT_THREAD_SUPPORT




unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

HEADERS += ../../robot/vision/SubVision.h
HEADERS += ../../robot/vision/SubObject.h
HEADERS += ../../robot/vision/GeneralisedColourDetect.h
HEADERS += ../../robot/vision/VisualCortex.h
HEADERS += ../../robot/vision/VisualObject.h
HEADERS += ../../robot/vision/SanityChecks.h
HEADERS += ../../robot/share/Common.h
SOURCES += ../../robot/vision/SubVision.cc
SOURCES += ../../robot/vision/SubObject.cc
SOURCES += ../../robot/vision/GeneralisedColourDetect.cc
SOURCES += ../../robot/vision/VisualCortex.cc
SOURCES += ../../robot/vision/FieldLineVision.cc
SOURCES += ../../robot/vision/SanityChecks.cc
SOURCES += ../../robot/share/Common.cc
