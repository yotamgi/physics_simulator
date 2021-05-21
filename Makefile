CXXFLAGS=-g -I/usr/include/irrlicht/ -Wall -I. -fPIC
LDFLAGS=-lIrrlicht -llapack -larmadillo

OBJECTS=constrainted_masses.o renderer.o

all: four_masses scenes/_pendulum.so

constrainted_masses.o: constrainted_masses.cc
	g++  -c ${CXXFLAGS} ./constrainted_masses.cc -o constrainted_masses.o

renderer.o: renderer.cc
	g++  -c ${CXXFLAGS} ./renderer.cc -o renderer.o

scenes/four_masses.o: scenes/four_masses.cc
	g++  -c ${CXXFLAGS} ./scenes/four_masses.cc -o scenes/four_masses.o

four_masses: ${OBJECTS} scenes/four_masses.o
	g++ ${OBJECTS} scenes/four_masses.o -o four_masses ${LDFLAGS}

scenes/pendulum.o: scenes/pendulum.cc
	g++  -c ${CXXFLAGS} ./scenes/pendulum.cc -o scenes/pendulum.o

scenes/_pendulum.so: ${OBJECTS} scenes/pendulum.o ./scenes/pendulum.i
	swig -c++ -python ./scenes/pendulum.i
	g++ -c ${CXXFLAGS} ./scenes/pendulum_wrap.cxx -o ./scenes/pendulum_wrap.o -I /usr/include/python3.9/
	g++ ${OBJECTS} scenes/pendulum.o scenes/pendulum_wrap.o -o scenes/_pendulum.so ${LDFLAGS} -fPIC -shared

clean:
	rm -f four_masses ${OBJECTS} scenes/four_masses.o scenes/pendulum.o \
		./scenes/pendulum_wrap.cxx ./scenes/pendulum_wrap.o \
		scenes/pendulum.py scenes/_pendulum.so
