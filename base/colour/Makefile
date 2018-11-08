CXX=g++
CXXFLAGS=-Wall -O2

all: rlog2bfl mergennmc

rlog2bfl: rlog2bfl.cc ../../robot/vision/CorrectedImage.cc
	g++ rlog2bfl.cc ../../robot/vision/CorrectedImage.cc -DOFFLINE -o rlog2bfl

mergennmc: mergennmc.c
	gcc mergennmc.c -o mergennmc

clean:
	rm mergennmc rlog2bfl
