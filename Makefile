CXXFLAGS=-g -I/usr/include/irrlicht/ -Wall
LDFLAGS=-lIrrlicht -llapack -larmadillo

OBJECTS=main.o constrainted_masses.o renderer.o

all: sim

main.o: main.cc
	g++  -c ${CXXFLAGS} ./main.cc -o main.o

constrainted_masses.o: constrainted_masses.cc
	g++  -c ${CXXFLAGS} ./constrainted_masses.cc -o constrainted_masses.o

renderer.o: renderer.cc
	g++  -c ${CXXFLAGS} ./renderer.cc -o renderer.o

sim: ${OBJECTS}
	g++ ${OBJECTS} -o sim ${LDFLAGS}

clean:
	rm sim ${OBJECTS}
