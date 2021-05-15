#include "constrainted_masses.h"

#include <irrlicht.h>
#include <IGeometryCreator.h>
#include <iostream>
#include <stdlib.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

std::ostream& operator<<(std::ostream& os, const vector3df& vec)
{
	os << "(" << vec.X << ", " << vec.Y << ", " << vec.Z << ")";
	return os;
}

int main()
{
	IrrlichtDevice *device =
		createDevice( video::EDT_OPENGL, dimension2d<u32>(1024, 768), 16,
			false, false, false, 0);

	if (!device)
		return 1;
	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	auto gcreator = smgr->getGeometryCreator();
	IMesh* mesh = gcreator->createArrowMesh(
			10, 20,
			/* height */   30., 
			/* head height */ 15., 
			/* width */ 1.5, 
			/* head width */ 4.,
			0xFF0000FF, 0xFFFF00FF);
	if (!mesh)
	{
		device->drop();
		return 1;
	}
	IMeshSceneNode* node = smgr->addMeshSceneNode(mesh);
	if (node)
	{
		node->setMaterialFlag(EMF_LIGHTING, false);
	}
	smgr->addLightSceneNode(NULL, vector3df(0, 60, -60),
				video::SColor(255, 150, 255, 255), 50.);
	smgr->setAmbientLight(video::SColor(255, 10, 10, 10));
	node->setPosition(vector3df(30, 0, 0));
	smgr->addCameraSceneNode(0, vector3df(0,0,-30),
			vector3df(0,0,1));

	// Build the Constrainted Masses object.
	ConstraintedMasses::Mass m0(smgr, vector3df(-3, 15, 0),
			driver, 0.5,
			vector3df(0,  0, 0), 0.5);
	ConstraintedMasses::Mass m1(smgr, vector3df(  0, 15, 0),
			driver, 0.5,
			vector3df(0 , 0, 0), 0.5);
	ConstraintedMasses::Mass m2(smgr, vector3df( 3, 15, 0),
			driver, 0.5,
			vector3df(0,  0, 0), 0.5);
	ConstraintedMasses::Mass m3(smgr, vector3df( 6, 15, 0),
			driver, 0.5,
			vector3df(0,  0, 0), 0.5);
	std::vector<ConstraintedMasses::Mass*> masses{&m0, &m1, &m2, &m3};
	ConstraintedMasses cm(masses);
	cm.add_constraint(0, 1);
	cm.add_constraint(1, 2);
	cm.add_constraint(2, 3);

	u32 then = device->getTimer()->getTime();
	int last_fps = -1;

	while(device->run())
	{
		// Work out a frame delta time.
		const u32 now = device->getTimer()->getTime();
		f32 frame_time_delta = (f32)(now - then) / 1000.f; // Time in seconds
		then = now;
		if (frame_time_delta > 0.05) {
			frame_time_delta = 0.05;
		}

		// Update stuff.
		m0.apply_force(vector3df(0, -10, 0));
		m1.apply_force(vector3df(0, -10, 0));
		m2.apply_force(vector3df(0, -10, 0));
		m3.apply_force(vector3df(0, -10, 0));
		if (m3.get_pos().Y < 0 ) {
			m3.apply_force(vector3df(0, -m3.get_pos().Y, 0) * 20);
		}
		cm.update(frame_time_delta);
		cm.update_ui();

		driver->beginScene(true, true, SColor(255,0,0,0));

		smgr->drawAll();
		guienv->drawAll();

		driver->endScene();

		// Update FPS
		int fps = driver->getFPS();

		if (last_fps != fps)
		{
			core::stringw tmp(L"Physical Simulation [");
			tmp += driver->getName();
			tmp += L"] fps: ";
			tmp += fps;

			device->setWindowCaption(tmp.c_str());
			last_fps = fps;
		}

	}
	device->drop();

	return 0;
}

