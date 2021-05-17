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

int main()
{
	SimulatorRenderer::init();
	auto smgr = SimulatorRenderer::get_smgr();

	auto gcreator = smgr->getGeometryCreator();
	irr::IMesh* mesh = gcreator->createArrowMesh(
			10, 20,
			/* height */   30., 
			/* head height */ 15., 
			/* width */ 1.5, 
			/* head width */ 4.,
			0xFF0000FF, 0xFFFF00FF);
	if (!mesh)
	{
		SimulatorRenderer::die();
		return 1;
	}
	irr::IMeshSceneNode* node = smgr->addMeshSceneNode(mesh);
	if (node)
	{
		node->setMaterialFlag(irr::EMF_LIGHTING, false);
	}
	node->setPosition(irrvec(20, 0, 0));

	// Build the Constrainted Masses object.
	ConstraintedMasses::Mass m0(irrvec(-3, 15, 0),
			0.5,
			irrvec(0,  0, 0), 0.5);
	ConstraintedMasses::Mass m1(irrvec(  0, 15, 0),
			0.5,
			irrvec(0 , 0, 0), 0.5);
	ConstraintedMasses::Mass m2(irrvec( 3, 15, 0),
			0.5,
			irrvec(0,  0, 0), 0.5);
	ConstraintedMasses::Mass m3(irrvec( 3, 15, 3),
			0.5,
			irrvec(0,  0, 0), 0.5);
	std::vector<ConstraintedMasses::Mass*> masses{&m0, &m1, &m2, &m3};
	ConstraintedMasses cm(masses);
	cm.add_constraint(0, 1);
	cm.add_constraint(1, 2);
	cm.add_constraint(2, 3);

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
		m0.apply_force(irrvec(0, -20, 0));
		m1.apply_force(irrvec(0, -20, 0));
		m2.apply_force(irrvec(0, -20, 0));
		m3.apply_force(irrvec(0, -20, 0));
		if (m3.get_pos().Y < 0 ) {
			m3.apply_force(irrvec(0, -m3.get_pos().Y, 0) * 50);
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

