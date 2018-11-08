TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

DEFINES	+= OFFLINE

HEADERS	+= CPlaneDisplay.h \
	SimpleServer.h \
	CPlane.h

SOURCES	+= CPlaneDisplay.cpp \
	Main.cpp

FORMS	= FormCPlaneDisplay.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
