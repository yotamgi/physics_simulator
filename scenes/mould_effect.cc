#include "constrainted_masses.h"
#include "common.h"
#include "renderer.h"

#include <irrlicht.h>
#include <IGeometryCreator.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cstdint>

const int MIN_UPDATE_RATE = 20;
const int MAX_UPDATE_RATE = 200;
const float MIN_TIME_DELTA = 1. / MAX_UPDATE_RATE;
const float MAX_TIME_DELTA = 1. / MIN_UPDATE_RATE;

const int MAX_FRAME_RATE = 70;

const float LINK_MASS = 0.5;
const float LINK_RADIUS = 0.5;
const float CONSTRAINT_COEFF = 500.;


ConstraintedMasses::Mass* create_link(const irrvec &pos) {
	return new ConstraintedMasses::Mass(pos,
		LINK_MASS, irrvec(0, 0, 0), LINK_RADIUS);
}

int main()
{
	SimulatorRenderer::init();

	// Build the Constrainted Masses object.
	std::vector<ConstraintedMasses::Mass *> masses;

	for (size_t i=0; i<30; i++) {
		masses.push_back(create_link(irrvec(5, 0, 0)));
		masses.push_back(create_link(irrvec(3, 0, 0)));
	}
	masses.push_back(create_link(irrvec( 3,  3, 0)));
	masses.push_back(create_link(irrvec( 3,  6, 0)));
	masses.push_back(create_link(irrvec( 0,  6, 0)));
	masses.push_back(create_link(irrvec(-3,  6, 0)));
	masses.push_back(create_link(irrvec(-3,  3, 0)));
	masses.push_back(create_link(irrvec(-3,  0, 0)));
	masses.push_back(create_link(irrvec(-3, -3, 0)));
	masses.push_back(create_link(irrvec(-3, -6, 0)));

	ConstraintedMasses cm(masses);
	for (size_t i=0; i < masses.size()-1; i++) {
		cm.add_constraint(i, i+1);
	}

	std::uint32_t then = SimulatorRenderer::get_time();

	float time_since_last_frame = 0;
	float time_delta = 0;
	while(SimulatorRenderer::running())
	{
		if (time_delta < MIN_TIME_DELTA) {
			usleep((MIN_TIME_DELTA - time_delta) * 1000000);
		}

		// Work out a frame delta time.
		const std::uint32_t now = SimulatorRenderer::get_time();
		time_delta = (float)(now - then) / 1000.f; // Time in seconds
		then = now;
		if (time_delta > MAX_TIME_DELTA) {
			time_delta = MAX_TIME_DELTA;
		}

		// Update stuff.
		for (auto mass : masses) {
			mass->apply_force(irrvec(0, -10, 0));

			if ((mass->get_pos().X >= 0.5) && (mass->get_pos().Y < 0)) {
				mass->apply_force(irrvec(0, -mass->get_pos().Y, 0) * CONSTRAINT_COEFF);
			}
			auto radius = mass->get_pos() - irrvec(0, 0, 0);
			radius.Z = 0;
			if (radius.getLength() < 2.5) {
				mass->apply_force(radius  * CONSTRAINT_COEFF);
			}
		}
		cm.update(time_delta);

		// Draw at most 60 frames per sec.
		time_since_last_frame += time_delta;
		if (time_since_last_frame > 1./MAX_FRAME_RATE) {
			cm.update_ui();
			SimulatorRenderer::render_frame();
			time_since_last_frame = 0;
		}
	}
	SimulatorRenderer::die();

	return 0;
}

