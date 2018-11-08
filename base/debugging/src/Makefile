all: EmonLog.o  Function.o  FunctionStore.o  StackAnalyser.o  StackDump.o  main.o
	g++ -g -o ../StackedIt EmonLog.o  Function.o  FunctionStore.o  StackAnalyser.o  StackDump.o  main.o 

Function.o: Function.cc Function.h
	g++ -g -c   Function.cc

FunctionStore.o: FunctionStore.cc FunctionStore.h Function.h 
	g++ -g -c   FunctionStore.cc

StackAnalyser.o: StackAnalyser.cc StackAnalyser.h StackDump.h \
  FunctionStore.h Function.h Trace.h
	g++ -g -c   StackAnalyser.cc

StackDump.o: StackDump.cc StackDump.h 
	g++ -g -c   StackDump.cc

main.o: main.cc StackDump.h \
  FunctionStore.h Function.h StackAnalyser.h
	g++ -g -c   main.cc

EmonLog.o:	EmonLog.h EmonLog.cc
	g++ -g -c   EmonLog.cc

clean:
	rm -f *.o ../StackedIt ../StackedIt.exe
