ROBOT_DIR=../../robot
VISION_DIR=$(ROBOT_DIR)/vision
BEHAVIOURS_DIR=$(ROBOT_DIR)/behaviours
SHARE_DIR=$(ROBOT_DIR)/share

B_LIB = libBehaviours.a

ifeq ($(shell uname),Linux)
	VGRIND = -DVALGRIND
	PYINCLUDE = -I/usr/include/python2.3/
	PYLIB = -L/usr/lib/python2.3/config -lpython2.3 -lpthread -ldl -lutil -lm
else
	VGRIND = 
	PYINCLUDE = -I/sw/include/python2.3/
	PYLIB =  -L/sw/lib/python2.3/config/ -lpython2.3
endif

CXXFLAGS = -g -DOFFLINE $(VGRIND) $(PYINCLUDE)
# CXXFLAGS = -g -DOFFLINE -DPRE_SANITY_BLOB

include $(BEHAVIOURS_DIR)/Makefile.behaviours.srcs.offline

# the following list of files should be the same as those in behaviours/Makefile
BEHAVIOUR_FILES = $(addprefix $(BEHAVIOURS_DIR)/,$(CC_SRCS))
BEHAVIOUR_OBJS = $(patsubst %.cc,%.o,$(filter %.cc,$(BEHAVIOUR_FILES)))

#copied from share/Makefile CC_SRCS
SHARE_SRCS         = \
	Common.cc \
	CommandData.cc \
	PWalkDef.cc
    
SHARE_FILES = $(addprefix $(SHARE_DIR)/,$(SHARE_SRCS))
SHARE_OBJS = $(patsubst %.cc,%.o,$(filter %.cc,$(SHARE_FILES)))

all: offlinevision interface

OFFVISION_SRCS = OfflineVision.cc \
                $(VISION_DIR)/CommonSense.cc \
                $(VISION_DIR)/KalmanInfo2D.cc \
                $(VISION_DIR)/KI2DWithVelocity.cc \
                $(VISION_DIR)/gps.cc \
                $(VISION_DIR)/GeneralisedColourDetect.cc \
                $(VISION_DIR)/Blob.cc \
                $(VISION_DIR)/SubVision.cc \
                $(VISION_DIR)/VisualObject.cc \
                $(VISION_DIR)/DistanceCalibrator.cc \
                $(VISION_DIR)/VisualCortex.cc \
                $(VISION_DIR)/PyEmbed.cc \
                $(VISION_DIR)/Robolink.cc \
                $(SHARE_DIR)/FileUtility.cc \
#                $(SHARE_DIR)/Common.cc \
#                $(SHARE_DIR)/CommandData.cc 


OFFVISION_OBJS = $(patsubst %.cc,%.o,$(filter %.cc,$(OFFVISION_SRCS)))                 
                
offlinevision: $(B_LIB) $(OFFVISION_OBJS)
	g++ $(CXXFLAGS) -Wall -o offvision $(OFFVISION_OBJS)\
		$(B_LIB) $(PYLIB)\

test:
	$(BEHAVIOUR_FILES)
		                                
interface:
	javac -classpath "$(CLASSPATH)":../work/RoboCommander *.java
        
l:
	javac learning/*.java
        
lc:
	rm -f learning/*.class
	
$(VISION_DIR)/VisualCortex.o: $(VISION_DIR)/VisualCortex.cc $(VISION_DIR)/PointProjection.h
	g++ $(CXXFLAGS) -o $@ -c $<

%.o: %.cc
	g++ $(CXXFLAGS) -o $@ -c $<

$(B_LIB): $(BEHAVIOUR_OBJS) $(SHARE_OBJS)
	ar rc $(B_LIB) $(BEHAVIOUR_OBJS) $(SHARE_OBJS)
	ranlib $(B_LIB)

clean:
	rm -f *.o offvision *.class
	rm -f libBehaviours.a $(BEHAVIOUR_OBJS)

distclean:
	rm -f frames/* objects/* blobs/* sanityLearning/*/*
