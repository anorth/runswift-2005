#GLLIBS= -lglut32 -lglu32 -lopengl32
GLLIBS= -lglut
CC = c++
all: colourcube

colourcube: colourcube.o trackball.o
	$(CC) -o colourcube colourcube.o trackball.o $(GLLIBS)

trackball.o: trackball.c
	c++ -c trackball.c

clean: 
	rm *.o colourcube
