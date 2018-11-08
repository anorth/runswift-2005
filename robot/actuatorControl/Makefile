# modify to reflect OPEN_R_SDK location on local system
#   /usr/local/OPEN_R_SDK/OPEN_R/bin/stubgen2 stub.cfg

PREFIX		= /usr/local/OPEN_R_SDK
OPENR_ROOT      = $(PREFIX)/OPEN_R

#MEMSTICK	= /mnt/ms
include ../makefile.config

# modify to reflect location of sources
MODULE_ROOT     = $(shell pwd)

CXX             = $(PREFIX)/bin/mipsel-linux-g++
STRIP           = $(PREFIX)/bin/mipsel-linux-strip
MKBIN           = $(OPENR_ROOT)/bin/mkbin
STUBGEN         = $(OPENR_ROOT)/bin/stubgen2
MKBINFLAGS      = -v -p $(PREFIX)
LIBS            = -lObjectComm -lOPENR -lERA201D1
CP = rsync -cr

WARNFLAGS= -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Wall
#    -Wshadow

SYSINCLUDES = 	-isystem$(OPENR_ROOT)/include/R4000 \
	-isystem$(OPENR_ROOT)/include/MCOOP \
	-isystem$(OPENR_ROOT)/include

CXXFLAGS        = -O2 $(WARNFLAGS) $(SYSINCLUDES)\
		-g \
		-DOPENR_DEBUG \
		-I. \
		-DAPERIOS

.PHONY: $(OBJECT) install clean distclean mkdepend

OBJECT      = actuator.bin

all: $(OBJECT)

EXT_OBJS = ../share/Common.o ../share/CommonData.o

CC_SRCS = \
	legsGeometry.cc \
	ActuatorControlStub.cc \
        EffectorCommander.cc \
	PWalk.cc \
	QuadWalk.cc \
	NormalWalk.cc \
	OffsetWalk.cc \
	AlanWalk.cc \
	BackWalk.cc \
	EllipticalWalk.cc \
	ActionPro.cc \
	Indicators.cc \
	ActuatorControl.cc \
	HeadMotion.cc \
	HeadKinematics.cc \
	JointLimit.cc \
	JointAgent.cc \
	JointCommand.cc \
	SkellipticalWalk.cc \
	SkiWalk.cc \
	MacAddress.cc

OBJS		= $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

ActuatorControlStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

$(OBJECT): $(OBJS) ActuatorControlStub.o actuatorControl.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(OBJECT).gz: $(OBJECT)
	gzip -c $(OBJECT) > $(OBJECT).gz

# modify to dump compressed executable to where memory stick is mounted
install: $(OBJECT).gz
	$(CP) $(OBJECT).gz $(MEMSTICK)/open-r/mw/objs/actuator.bin

clean:
	rm -f *.o *.bin *.elf *.snap.cc *.bin.gz

distclean: clean
	rm -f ActuatorControlStub.* def.h entry.h tags depend

depend: ActuatorControlStub.cc
	$(CXX) -M $(CXXFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

normalWalk: QuadWalk.cc NormalWalk.cc
	g++ $(CXXFLAGS) -DCALWALK_OFFLINE -DOFFLINE -o normalWalk QuadWalk.cc NormalWalk.cc

skelWalk: legsGeometry.cc SkellipticalWalk.cc
	g++ $(CXXFLAGS) -DCALWALK_OFFLINE -DOFFLINE -o skelWalk legsGeometry.cc SkellipticalWalk.cc

legsGeo: legsGeometry.cc
	g++ $(CXXFLAGS) -DLEGS_GEO -DOFFLINE -o legsGeo legsGeometry.cc

include depend
