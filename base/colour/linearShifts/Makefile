all: reconfigureShifts getDogShifts.class

reconfigureShifts: reconfigureShifts.cc ../../../robot/vision/LinearShifts.h
	g++ -Wall -DOFFLINE reconfigureShifts.cc -o reconfigureShifts

getDogShifts.class: getDogShifts.java
	javac getDogShifts.java

clean:
	rm getDogShifts.class reconfigureShifts
