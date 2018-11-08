CXXFLAGS=

all: udpcheck

udpcheck: DSocket.o DPacket.o BinStream.o udpcheck.o
	$(CXX) $+ -lncurses -o $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
