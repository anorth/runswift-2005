TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on debug

DEFINES	+= OFFLINE

HEADERS	+= FormSubvisionImpl.h \
	FrameReader.h

SOURCES	+= main.cpp \
	FormSubvisionImpl.cpp \
	FrameReader.cpp

FORMS	= FormSubvision.ui \
	dlgConnect.ui \
	DPixmap.ui

#CONFIG	+= qt warn_on debug


DEFINES	+= QT_THREAD_SUPPORT
#LFLAGS += -pg
#comment 3 lines below if don't want to profile
QMAKE_LFLAGS_DEBUG += -pg
QMAKE_CFLAGS_DEBUG += -pg -O3 
QMAKE_CXXFLAGS_DEBUG += -pg -O3

#comment 2 lines below if don't want to see visual cortex cout
#QMAKE_CFLAGS_DEBUG += -DFLV_DEBUG
#QMAKE_CXXFLAGS_DEBUG += -DFLV_DEBUG

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

HEADERS += ../../robot/vision/SubVision.h
HEADERS += ../../robot/vision/SubObject.h
HEADERS += ../../robot/vision/CorrectedImage.h
HEADERS += ../../robot/vision/VisualCortex.h
HEADERS += ../../robot/vision/VisualObject.h
HEADERS += ../../robot/vision/SanityChecks.h
HEADERS += ../../robot/vision/Obstacle.h
HEADERS += ../../robot/vision/Profile.h
HEADERS += ../../robot/share/Common.h
HEADERS += ../../robot/vision/gps.h
SOURCES += ../../robot/vision/SubVision.cc
SOURCES += ../../robot/vision/SubObject.cc
SOURCES += ../../robot/vision/CorrectedImage.cc
SOURCES += ../../robot/vision/VisualCortex.cc
SOURCES += ../../robot/vision/FieldLineVision.cc
SOURCES += ../../robot/vision/SanityChecks.cc
SOURCES += ../../robot/vision/Profile.cc
SOURCES += ../../robot/share/Common.cc
SOURCES += ../../robot/share/CommonData.cc
SOURCES += ../../robot/vision/Obstacle.cc
SOURCES += ../../robot/vision/gps.cc
SOURCES += ../../robot/vision/SlamChallenge.cc
SOURCES += ../../robot/vision/KI2DWithVelocity.cc
SOURCES += ../../robot/vision/KalmanInfo2D.cc
SOURCES += gpsinfo.cpp
SOURCES += calibrateEdge.cpp
SOURCES += flagObject.cpp
SOURCES += houghDisplay.cpp
SOURCES += hough.cpp
