#include "renderer.h"
#include <iostream>

irr::ISceneManager* SimulatorRenderer::m_smgr;
irr::IrrlichtDevice* SimulatorRenderer::m_device;
int SimulatorRenderer::m_last_fps;

void SimulatorRenderer::init() {
	m_last_fps = -1;

	m_device = irr::createDevice(irr::video::EDT_OPENGL,
			irr::dimension2d<std::uint32_t>(1024, 768), 16,
			false, false, false, 0);
	if (!m_device)
		return;

	m_smgr = m_device->getSceneManager();
	m_smgr->addLightSceneNode(NULL, irrvec(0, 60, -60),
				irr::SColor(255, 150, 255, 255), 50.);
	m_smgr->setAmbientLight(irr::SColor(255, 10, 10, 10));
	auto camera = m_smgr->addCameraSceneNodeFPS(NULL, 100., 0.05);
	camera->setPosition(irrvec(0, 0, -30));
}

irr::ISceneManager* SimulatorRenderer::get_smgr() {
	return m_smgr;
}

bool SimulatorRenderer::running() {
	return m_device->run();
}

void SimulatorRenderer::die() {
	m_device->drop();
}

void SimulatorRenderer::render_frame() {
	irr::IVideoDriver* driver = m_device->getVideoDriver();
	irr::IGUIEnvironment* guienv = m_device->getGUIEnvironment();

	// Render.
	driver->beginScene(true, true, irr::SColor(255,0,0,0));
	m_smgr->drawAll();
	guienv->drawAll();
	driver->endScene();

	// Update FPS
	int fps = driver->getFPS();
	if (m_last_fps != fps)
	{
		irr::stringw tmp(L"Physical Simulation [");
		tmp += driver->getName();
		tmp += L"] fps: ";
		tmp += fps;

		m_device->setWindowCaption(tmp.c_str());
		m_last_fps = fps;
	}
}

std::uint32_t SimulatorRenderer::get_time() {
	return m_device->getTimer()->getTime();
}
