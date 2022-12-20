CXXFLAGS=-g -I/usr/include/irrlicht/ -Wall -I. -fPIC
LDFLAGS=-lIrrlicht -llapack -larmadillo

OBJECTS=constrainted_masses.o renderer.o

all: four_masses scenes/pendulum/_pendulum.so mould_effect

constrainted_masses.o: constrainted_masses.cc
	g++  -c ${CXXFLAGS} ./constrainted_masses.cc -o constrainted_masses.o

renderer.o: renderer.cc
	g++  -c ${CXXFLAGS} ./renderer.cc -o renderer.o

scenes/four_masses.o: scenes/four_masses.cc
	g++  -c ${CXXFLAGS} ./scenes/four_masses.cc -o scenes/four_masses.o

four_masses: ${OBJECTS} scenes/four_masses.o
	g++ ${OBJECTS} scenes/four_masses.o -o four_masses ${LDFLAGS}

mould_effect: ${OBJECTS} scenes/mould_effect.o
	g++ ${OBJECTS} scenes/mould_effect.o -o mould_effect ${LDFLAGS}

scenes/pendulum/pendulum.o: scenes/pendulum/pendulum.cc
	g++  -c ${CXXFLAGS} ./scenes/pendulum/pendulum.cc -o scenes/pendulum/pendulum.o

scenes/pendulum/_pendulum.so: ${OBJECTS} scenes/pendulum/pendulum.o ./scenes/pendulum/pendulum.i
	swig -c++ -python ./scenes/pendulum/pendulum.i
	g++ -c ${CXXFLAGS} ./scenes/pendulum/pendulum_wrap.cxx -o ./scenes/pendulum/pendulum_wrap.o -I /usr/include/python3.10/
	g++ ${OBJECTS} scenes/pendulum/pendulum.o scenes/pendulum/pendulum_wrap.o -o scenes/pendulum/_pendulum.so ${LDFLAGS} -fPIC -shared

clean:
	rm -f four_masses ${OBJECTS} scenes/four_masses.o scenes/pendulum/pendulum.o \
		./scenes/pendulum/pendulum_wrap.cxx ./scenes/pendulum/pendulum_wrap.o \
		scenes/pendulum/pendulum.py scenes/pendulum/_pendulum.so
