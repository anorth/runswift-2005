#/usr/local/OPEN_R_SDK/OPEN_R/bin/stubgen2 stub.cfg

PREFIX		= /usr/local/OPEN_R_SDK
OPENR_ROOT  = $(PREFIX)/OPEN_R

#MEMSTICK	= /mnt/ms
include ../makefile.config

# modify to reflect location of sources
MODULE_ROOT     = $(shell pwd)

CXX             = $(PREFIX)/bin/mipsel-linux-g++
STRIP           = $(PREFIX)/bin/mipsel-linux-strip

MKBIN           = $(OPENR_ROOT)/bin/mkbin
STUBGEN         = $(OPENR_ROOT)/bin/stubgen2
MKBINFLAGS      = -v -p $(PREFIX)
LIBS            = -lObjectComm -lOPENR

WARNFLAGS= -W -Wshadow -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Wall

SYSINCLUDES = 	-isystem$(OPENR_ROOT)/include/R4000 \
	-isystem$(OPENR_ROOT)/include/MCOOP \
	-isystem$(OPENR_ROOT)/include

CXXFLAGS        = -O2 $(WARNFLAGS) $(SYSINCLUDES) \
		-DOPENR_DEBUG \
		-I. \
		-DAPERIOS

.PHONY: $(OBJECT) install clean distclean mkdepend

CC_SRCS         = \
	Common.cc \
	CommonData.cc \
	CommandData.cc \
	FileUtility.cc \
	PWalkDef.cc

OBJS		= $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS)))

OBJECT      = $(OBJS)

all: $(OBJECT)


%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f *.o

distclean: clean

depend:

mkdepend:
