%module pendulum

%include "std_vector.i"

%{
#define SWIG_FILE_WITH_INIT
#include "pendulum.h"
%}

namespace std {
    %template(VectorPendulumNode) vector<PendulumNodeInfo>;
};

%include "pendulum.h"

