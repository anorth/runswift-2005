#
# Copyright 2002 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

PREFIX=/usr/local/OPEN_R_SDK
OPENR_ROOT= $(PREFIX)/OPEN_R

#MEMSTICK	= /mnt/ms
include ../makefile.config

# modify to reflect location of sources
MODULE_ROOT     = $(shell pwd)

CXX=$(PREFIX)/bin/mipsel-linux-g++
STRIP=$(PREFIX)/bin/mipsel-linux-strip
MKBIN=$(PREFIX)/OPEN_R/bin/mkbin
CP = rsync -cr
STUBGEN=$(PREFIX)/OPEN_R/bin/stubgen2
MKBINFLAGS=-v -p $(PREFIX)
LIBS=-L$(PREFIX)/OPEN_R/lib -lObjectComm -lOPENR -lInternet -lantMCOOP
WARNFLAGS= -W -Wshadow -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls \
        -Winline -Wall -Wfloat-equal

SYSINCLUDES = 	-isystem$(OPENR_ROOT)/include/R4000 \
	-isystem$(OPENR_ROOT)/include/MCOOP \
	-isystem$(OPENR_ROOT)/include

CXXFLAGS= $(WARNFLAGS) $(SYSINCLUDES) -g -O2 -I. -DAPERIOS
#-DOPENR_DEBUG

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
#CXXFLAGS+= -DOPENR_DEBUG

.PHONY: all install clean distclean mkdepend

OBJECT = Wireless.bin

all: Wireless.bin

EXT_OBJS = ../share/CommandData.o

CC_SRCS         = WirelessStub.cc \
                  Wireless.cc

OBJS		= $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

WirelessStub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

$(OBJECT): $(OBJS) WirelessStub.o Wireless.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(OBJECT).gz: $(OBJECT)
	gzip -c $(OBJECT) > $(OBJECT).gz

install: $(OBJECT).gz
	$(CP) $(OBJECT).gz $(MEMSTICK)/open-r/mw/objs/wireless.bin

clean:
	rm -f *.o *.bin *.elf *.snap.cc *.bin.gz

distclean: clean
	rm -f WirelessStub.* def.h entry.h tags depend

depend: WirelessStub.cc
	$(CXX) -M $(CXXFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

include depend

