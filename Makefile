CXXFLAGS=-g -I/usr/include/irrlicht/ -Wall
LDFLAGS=-lIrrlicht -llapack -larmadillo

OBJECTS=main.o constrainted_masses.o

all: sim

main.o: main.cc
	g++  -c ${CXXFLAGS} ./main.cc -o main.o

constrainted_masses.o: constrainted_masses.cc
	g++  -c ${CXXFLAGS} ./constrainted_masses.cc -o constrainted_masses.o

sim: ${OBJECTS}
	g++ ${OBJECTS} -o sim ${LDFLAGS}

clean:
	rm sim ${OBJECTS}
