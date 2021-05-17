#ifndef __SIM_RENDERER_H__
#define __SIM_RENDERER_H__

#include "common.h"
#include <stdint.h>
#include <cstdint>


class SimulatorRenderer {
public:
	static void init();
	static irr::ISceneManager* get_smgr();
	static bool running();
	static void die();
	static std::uint32_t get_time();
	static void render_frame();
private:
	SimulatorRenderer() {}
	static irr::ISceneManager* m_smgr;
	static irr::IrrlichtDevice* m_device;
	static int m_last_fps;
};


#endif // __SIM_RENDERER_H__
