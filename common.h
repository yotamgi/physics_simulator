#ifndef __SIM_COMMON_H__
#define __SIM_COMMON_H__

#include <irrlicht.h>
#include <iostream>

typedef irr::core::vector3df irrvec;

namespace irr {
	using namespace core;
	using namespace video;
	using namespace scene;
	using namespace io;
	using namespace gui;
}

static inline std::ostream& operator<<(std::ostream& os, const irrvec& vec)
{
	os << "(" << vec.X << ", " << vec.Y << ", " << vec.Z << ")";
	return os;
}

#endif // __SIM_COMMON_H__
