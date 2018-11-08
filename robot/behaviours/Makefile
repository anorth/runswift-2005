#/usr/local/OPEN_R_SDK/OPEN_R/bin/stubgen2 stub.cfg

PREFIX		= /usr/local/OPEN_R_SDK
OPENR_ROOT  = $(PREFIX)/OPEN_R

#MEMSTICK	= /mnt/ms
include ../makefile.config

# modify to reflect location of sources
MODULE_ROOT     = $(shell pwd)

CXX             = $(PREFIX)/bin/mipsel-linux-g++
STRIP           = $(PREFIX)/bin/mipsel-linux-strip
RANLIB          = $(PREFIX)/bin/mipsel-linux-ranlib
AR              = $(PREFIX)/bin/mipsel-linux-ar

MKBIN           = $(OPENR_ROOT)/bin/mkbin
STUBGEN         = $(OPENR_ROOT)/bin/stubgen2
MKBINFLAGS      = -v -p $(PREFIX)

LIBS            = -lObjectComm -lOPENR

WARNFLAGS= -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Wall \
    -Wno-unused
#    -Wfloat-equal -Wshadow 
    
PYTHONLFLAGS= -Wno-conversion -Wno-redundant-decls

SYSINCLUDES = 	-isystem$(OPENR_ROOT)/include/R4000 \
	-isystem$(OPENR_ROOT)/include/MCOOP \
	-isystem$(OPENR_ROOT)/include

CXXFLAGS        = -O2 $(WARNFLAGS) $(SYSINCLUDES) \
		-g \
		$(PYTHONLFLAGS) \
		-DOPENR_DEBUG \
		-I. \
		-DAPERIOS \
		-I../pythonlib/include/python2.3\
		#-DCOMPILE_ALL_CPP \

.PHONY: $(OBJECT) install clean distclean mkdepend

OBJECT      = libBehaviour.a

all: $(OBJECT)

# We should remove many from below that are now in Python, e.g. locateBall
CC_SRCS = Behaviours.cc \
		firePawKick.cc \
		getBehindBall.cc \
		locateBall.cc \
		pythonBehaviours.cc \
		runBehindBall.cc \
		UNSWSkills.cc \
		utility.cc \
#		WalkingLearner.cc
 

OBJS		= $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)


%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

BehaviourStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

../vision/VisionStub.cc:
	cd ../vision && make VisionStub.cc

#$(OBJECT): $(OBJS) BehaviourStub.o behaviour.ocf
#	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
#	$(STRIP) $@

$(OBJECT): $(OBJS)
	$(AR) rc $(OBJECT) $(OBJS)
	$(RANLIB) $(OBJECT)

install: $(OBJECT)

clean:
	rm -f *.o *.bin *.elf *.snap.cc *.o *.a

distclean: clean
	rm -f BehaviourStub.* def.h entry.h tags depend

depend: ../vision/VisionStub.cc
	$(CXX) -M $(CXXFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

include depend
