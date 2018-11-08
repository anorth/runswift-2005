#
# Makefile for OPEN-R Object on OPEN-R LINUX
#
# Copyright (C) 2001 Sony Corporation
# All Rights Reserved.
#

SHELL   = /bin/sh
CC      = g++
CFLAGS  = ${INCLUDE} ${DEFINE} ${OPTIONS}
LD      = ${CC}
OBJ     = ${SOURCE:.cc=.o}


TARGET  = rstat

SOURCE  = \
	rstat.cc \
	RInfoParser.cc \
	Stats.cc \

INCLUDE = \
	-I. \


DEFINE  = \

OPTIONS = \
	-g \
#	-O2 \


LIBRARY = \

LDFLAGS = \



.SUFFIXES :
.SUFFIXES : .o .cc
.cc.o :
	${CC} -c $< -o $@ ${CFLAGS}
 

${TARGET} : ${OBJ}
	${LD} -o $@ ${OBJ} ${LIBRARY} ${LDFLAGS}

all : depend
	${MAKE} libraries ${TARGET}

libraries : ${LIBRARY}

${LIBRARY} : check_update
	@ test "${@D}" = "./" || ( cd ${@D}; ${MAKE} ${@F} )

check_update :

${OBJ} : Makefile


touch :
	touch ${SOURCE}

rmtarget :
	rm -f ${TARGET}

clean : rmtarget
	rm -f ${OBJ}

clobber : clean
	- @ for dir in ${dir $(LIBRARY)}; do \
		( cd $$dir; ${MAKE} clean ) \
	done

rebuild : clobber all


DEPENDENCIES        = _dependencies
UPDATE_DEPENDENCIES = ${CC} -M ${SOURCE} ${CFLAGS} > ${DEPENDENCIES}

${DEPENDENCIES} : ${SOURCE} Makefile
	${UPDATE_DEPENDENCIES}

depend :
	${UPDATE_DEPENDENCIES}


include ${DEPENDENCIES}
