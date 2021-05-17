CXXFLAGS=-g -I/usr/include/irrlicht/ -Wall -I.
LDFLAGS=-lIrrlicht -llapack -larmadillo

OBJECTS=constrainted_masses.o renderer.o

all: four_masses

constrainted_masses.o: constrainted_masses.cc
	g++  -c ${CXXFLAGS} ./constrainted_masses.cc -o constrainted_masses.o

renderer.o: renderer.cc
	g++  -c ${CXXFLAGS} ./renderer.cc -o renderer.o

scenes/four_masses.o: scenes/four_masses.cc
	g++  -c ${CXXFLAGS} ./scenes/four_masses.cc -o scenes/four_masses.o

four_masses: ${OBJECTS} scenes/four_masses.o
	g++ ${OBJECTS} scenes/four_masses.o -o four_masses ${LDFLAGS}

clean:
	rm -f four_masses ${OBJECTS} scenes/four_masses.o
