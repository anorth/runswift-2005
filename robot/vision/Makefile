#/usr/local/OPEN_R_SDK/OPEN_R/bin/stubgen2 stub.cfg

PREFIX		= /usr/local/OPEN_R_SDK
OPENR_ROOT  = $(PREFIX)/OPEN_R

#MEMSTICK	= /mnt/ms
include ../makefile.config

# modify to reflect location of sources
MODULE_ROOT     = $(shell pwd)

CXX             = $(PREFIX)/bin/mipsel-linux-g++
STRIP           = $(PREFIX)/bin/mipsel-linux-strip
CP = rsync -cr
MKBIN           = $(OPENR_ROOT)/bin/mkbin
STUBGEN         = $(OPENR_ROOT)/bin/stubgen2
MKBINFLAGS      = -v -p $(PREFIX)

LIBS            = -lObjectComm -lOPENR -lInternet -lantMCOOP -lERA201D1 -L../behaviours -lBehaviour -L../pythonlib -lpython2.3

# The more the better, but:
# -Wunreachable-code: there is a lot protected by if (false)
# -Wfloat-equal: there are many comparisons with 0.0 that are probably ok
# -Wwrite-strings: python lib functions that take strings have non-const types
# -Wredundant-decls: there are a couple in python lib
WARNFLAGS= -W -Wcast-qual -Wconversion -Wall -Wno-unused -Winline -Wshadow
#			-Wunreachable-code -Wfloat-equal -Wwrite-strings -Wredundant-decls 

PYTHONLFLAGS= -Wno-conversion -Wno-redundant-decls

SYSINCLUDES = 	-isystem$(OPENR_ROOT)/include/R4000 \
	-isystem$(OPENR_ROOT)/include/MCOOP \
	-isystem$(OPENR_ROOT)/include

CXXFLAGS = -O2 $(WARNFLAGS) $(SYSINCLUDES) \
		-g \
		-DOPENR_DEBUG \
		-funroll-loops \
		-finline-limit=2400 \
		-I. \
		-DAPERIOS \
		-I../pythonlib/include/python2.3\
#		-finline-functions \
#		-DCOMPILE_ALL_CPP \

.PHONY: $(OBJECT) install clean distclean mkdepend

OBJECT      = vision.bin

all: $(OBJECT)

EXT_OBJS = ../share/Common.o ../share/CommandData.o ../share/PWalkDef.o ../share/CommonData.o ../share/FileUtility.o 
EXT_LIBS = ../behaviours/libBehaviour.a ../pythonlib/libpython2.3.a

CC_SRCS         = \
	VisionStub.cc \
	Vision.cc \
	CorrectedImage.cc \
	Profile.cc \
	CommonSense.cc \
	DistanceCalibrator.cc \
	VisualCortex.cc \
	SubVision.cc \
	SubObject.cc \
	FieldLineVision.cc \
	SanityChecks.cc \
	gps.cc	\
	KalmanInfo2D.cc \
	KI2DWithVelocity.cc \
	Odometer.cc \
	InfoManager.cc \
	PyEmbed.cc \
	Robolink.cc\
	Obstacle.cc \
	SlamChallenge.cc


OBJS		= $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)

../behaviours/libBehaviour.a:
	(cd ../behaviours && make libBehaviour.a)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

VisionStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

$(OBJECT): $(OBJS) $(EXT_OBJS) $(EXT_LIBS) VisionStub.o vision.ocf 
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(OBJECT).gz: $(OBJECT)
	gzip -c $(OBJECT) > $(OBJECT).gz

# modify to dump compressed executable to where memory stick is mounted
install: $(OBJECT).gz
	$(CP) $(OBJECT).gz $(MEMSTICK)/open-r/mw/objs/vision.bin

clean:
	rm -f *.o *.bin *.elf *.snap.cc *.bin.gz

distclean: clean
	rm -f VisionStub.* def.h entry.h tags depend

depend: VisionStub.cc
	$(CXX) -M $(CXXFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

include depend
