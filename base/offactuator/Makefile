SOURCE_DIR      = ../../robot
CFG_DIR         = cfg
CFG_FILES       = offsets.txt
POS_DIR         = pos
ACTUATOR_DIR 	= actuatorControl
SHARE_DIR       = share
POS_FILES       = $(notdir $(shell ls $(SOURCE_DIR)/$(POS_DIR)/*.pos) )
SHARE_FILES     = $(notdir $(shell ls $(SOURCE_DIR)/$(SHARE_DIR)/*.cc)  $(shell ls $(SOURCE_DIR)/$(SHARE_DIR)/*.h) )
ACTUATOR_FILES     = $(notdir $(shell ls $(SOURCE_DIR)/$(ACTUATOR_DIR)/*.cc)  $(shell ls $(SOURCE_DIR)/$(ACTUATOR_DIR)/*.h) )


# modify to reflect location of sources
MODULE_ROOT     = $(shell pwd)

CXX             = g++

WARNFLAGS= -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Wall
#    -Wshadow

CXXFLAGS        = $(WARNFLAGS) \
		-g \
		-DOPENR_DEBUG \
		-I$(MODULE_ROOT) \
		-DOFFLINE

.PHONY: $(OBJECT) install clean distclean mkdepend 

OBJECT      = offactuator

all: share actuatorControl pos cfg $(OBJECT)

EXT_OBJS = share/Common.o share/CommonData.o

CC_SRCS = \
	$(ACTUATOR_DIR)/legsGeometry.cc \
	$(ACTUATOR_DIR)/PWalk.cc \
	$(ACTUATOR_DIR)/QuadWalk.cc \
	$(ACTUATOR_DIR)/NormalWalk.cc \
	$(ACTUATOR_DIR)/OffsetWalk.cc \
	$(ACTUATOR_DIR)/AlanWalk.cc \
	$(ACTUATOR_DIR)/SkiWalk.cc \
	$(ACTUATOR_DIR)/SkellipticalWalk.cc \
	$(ACTUATOR_DIR)/BackWalk.cc \
	$(ACTUATOR_DIR)/EllipticalWalk.cc \
	$(ACTUATOR_DIR)/ActionPro.cc \
	$(ACTUATOR_DIR)/JointLimit.cc \
	$(ACTUATOR_DIR)/JointCommand.cc \
	$(ACTUATOR_DIR)/JointAgentMediator.cc \
	$(ACTUATOR_DIR)/HeadMotionMediator.cc \
	$(ACTUATOR_DIR)/MacAddress.cc \
	OfflineActuator.cc \

OBJS = $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

#ActuatorControlStub.cc: stub.cfg
#	$(STUBGEN) stub.cfg

$(OBJECT): $(OBJS) $(EXT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

clean:
	rm -f *.o $(OBJS) 

distclean: clean
	rm -rf cfg pos share actuatorControl

depend: 
	$(CXX) -M $(CXXFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

cfg:
	mkdir $@
	for file in $(CFG_FILES);do\
		(cd $@; ln -s ../$(SOURCE_DIR)/$(CFG_DIR)/$$file $$file);\
	done

pos:
	mkdir $@
	for file in $(POS_FILES);do\
		(cd $@; ln -s ../$(SOURCE_DIR)/$(POS_DIR)/$$file $$file);\
	done

share: 
	mkdir $@
	for file in $(SHARE_FILES);do\
		(cd $@; ln -s ../$(SOURCE_DIR)/$(SHARE_DIR)/$$file $$file);\
	done

actuatorControl:
	mkdir $@
	for file in $(ACTUATOR_FILES);do\
		(cd $@; ln -s ../$(SOURCE_DIR)/$(ACTUATOR_DIR)/$$file $$file);\
	done

include depend
