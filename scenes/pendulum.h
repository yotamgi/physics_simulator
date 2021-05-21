#ifndef __SIM_PENDULUM_H__
#define __SIM_PENDULUM_H__

#include <vector>


struct PendulumNodeInfo {
	PendulumNodeInfo() {}
	PendulumNodeInfo(float x, float y, float z,
			float vx, float vy, float vz);
	float x, y, z;
	float vx, vy, vz;
};

struct PendulumInfo {
	std::vector<PendulumNodeInfo> pendulum_infos;
};

void init(int num_nodes = 1);

PendulumInfo step(float force_x, float force_y, float time_delta);

#endif // __SIM_PENDULUM_H__
