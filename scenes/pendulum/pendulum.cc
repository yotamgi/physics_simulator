#include "scenes/pendulum/pendulum.h"
#include "constrainted_masses.h"
#include "common.h"
#include "renderer.h"
#include <cstdint>
#include <iostream>

const int MIN_UPDATE_RATE = 20;
const float MAX_TIME_DELTA = 1. / MIN_UPDATE_RATE;
const int MAX_FRAME_RATE = 40;

ConstraintedMasses *cm = NULL;
std::vector<ConstraintedMasses::Mass *> masses;
std::uint32_t then;
float time_since_last_frame = 0;
float time_delta = 0;

PendulumNodeInfo::PendulumNodeInfo(float x, float y, float z,
			float vx, float vy, float vz):
	x(x), y(y), z(z), vx(vx), vy(vy), vz(vz) {}

void init(int num_nodes) {
	SimulatorRenderer::init();

	// Build the Constrainted Masses object by adding pendulum nodes.
	for (int i=0; i<num_nodes; i++) {
		float height = 7 * i;
		masses.push_back(new ConstraintedMasses::Mass(irrvec(0, height, 0),
			0.5, irrvec(0,  0, 0), 0.5));
	}
	cm = new ConstraintedMasses(masses);
	for (int i=1; i<num_nodes; i++)
		cm->add_constraint(i-1, i);

	then = SimulatorRenderer::get_time();
}

void build_info(PendulumInfo &info) {
	for (ConstraintedMasses::Mass *mass : masses) {
		irrvec pos = mass->get_pos();
		irrvec v = mass->get_v();
		info.pendulum_infos.push_back(
				PendulumNodeInfo(pos.X, pos.Y,
				pos.Z, v.X, v.Y, v.Z));
	}
}

PendulumInfo step(float force_x, float force_z, float time_delta) {

	if (!SimulatorRenderer::running()) {
		SimulatorRenderer::die();
		throw std::runtime_error("Graphic device closed.");
	}

	// Apply gravitation.
	for (ConstraintedMasses::Mass *mass : masses) {
		mass->apply_force(irrvec(0, -10, 0));
	}

	// Connect m0 to the y=0 plane;
	ConstraintedMasses::Mass *base_mass = masses[0];
	base_mass->apply_force(-irrvec(0, base_mass->get_pos().Y, 0) * 10);

	// Apply the force requested.
	base_mass->apply_force(irrvec(force_x, 0, force_z));

	// Update stuff.
	cm->update(time_delta);

	// Draw at most 60 frames per sec.
	const std::uint32_t now = SimulatorRenderer::get_time();
	float time_since_last_frame = (float)(now - then) / 1000.f; // Time in seconds
	if (time_since_last_frame > 1./MAX_FRAME_RATE) {
		cm->update_ui();
		SimulatorRenderer::render_frame();
		then = now;
	}

	PendulumInfo info;
	build_info(info);
	return info;
}
