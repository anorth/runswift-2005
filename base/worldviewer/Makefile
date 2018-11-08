CXX=g++
CXXFLAGS=-g -I$(QTDIR)/include -I/usr/include/qt3
LIB=-L$(QTDIR)/lib
LINK=-lqt-mt -lGL

all: worldviewer

worldviewer: worldviewer.o BinStream.o DPacket.o DSocket.o robots.o timer.o drawfield.o gltext.o
	$(CXX) $(LIB) $(LINK) $+ -o $@

worldviewer.o: worldviewer.moc

clean:
	rm *.o

%.moc: %.h
	moc $< -o $@

.moc.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
