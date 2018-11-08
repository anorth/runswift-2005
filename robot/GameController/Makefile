include ../makefile.config

PREFIX     = /usr/local/OPEN_R_SDK
OPENR_ROOT = $(PREFIX)/OPEN_R

CXX        = $(PREFIX)/bin/mipsel-linux-g++
STRIP      = $(PREFIX)/bin/mipsel-linux-strip
MKBIN      = $(OPENR_ROOT)/bin/mkbin
STUBGEN    = $(OPENR_ROOT)/bin/stubgen2
MKBINFLAGS = -v -p $(PREFIX)
LIBS       = -lObjectComm -lOPENR -lOPENR -lInternet -lantMCOOP
CP         = rsync -cr

WARNFLAGS = -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Wall

SYSINCLUDES = -isystem$(OPENR_ROOT)/include/R4000 \
			  -isystem$(OPENR_ROOT)/include/MCOOP \
			  -isystem$(OPENR_ROOT)/include

CXXFLAGS = -O2 $(WARNFLAGS) $(SYSINCLUDES)\
		   -g \
		   -DOPENR_DEBUG \
		   -I. \
		   -DAPERIOS

.PHONY: $(OBJECT) install clean distclean mkdepend

OBJECT = GameController.bin

all: $(OBJECT)

CC_SRCS = GameController.cc GameControllerStub.cc

OBJS = $(patsubst %.cc,%.o,$(filter %.cc,$(CC_SRCS))) $(EXT_OBJS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

GameControllerStub.cc: stub.cfg
	$(STUBGEN) stub.cfg

$(OBJECT): $(OBJS) GameControllerStub.o GameController.ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(OBJECT).gz: $(OBJECT)
	gzip -c $(OBJECT) > $(OBJECT).gz

install: $(OBJECT).gz
	$(CP) $(OBJECT).gz $(MEMSTICK)/open-r/mw/objs/GAMECTRL.bin

clean:
	rm -f *.o *.bin *.elf *.snap.cc *.bin.gz *Stub.* depend def.h entry.h

distclean: clean
	rm -f GameControllerStub.* def.h entry.h tags depend

depend: GameControllerStub.cc
	$(CXX) -M $(CXXFLAGS) $(CC_SRCS) > depend

mkdepend:
	rm -f depend
	$(MAKE) depend

include depend	
