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

CC=g++
#FLAG=-DJOYSTICK_CONTROL_D

ifdef CYGWIN
FLAG= -DLITTLE_ENDIAN
else
FLAG=
endif #CYGWIN

CFLAGS= -g -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Winline -Wall $(FLAG) -DOFFLINE

LIBS= -lpthread

CC_SRCS = \
     client.cc \
     readJoystick.cc

OBJS = $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)

all: client conf.cfg

%.o: %.cc
	$(CC) $(CFLAGS) -o $@ -c $<

share:
	ln -s ../../robot/share .

client: ${OBJS}
	$(CC) -o $@ ${OBJS} $(LIBS)

clean:
	rm -f client *.o

distclean: clean
	rm -rf conf.cfg depend share

conf.cfg: conf.cfg.orig
	cp conf.cfg.orig conf.cfg

depend: $(CC_SRCS) share
	$(CC) -M $(CFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

include depend
