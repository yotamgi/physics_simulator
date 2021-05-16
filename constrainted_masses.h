#include "common.h"
#include <irrlicht.h>
#include <vector>
#include <stdexcept>

class ConstraintedMasses {
public:
	class Mass {
	public:
		Mass(irr::scene::ISceneManager* smgr,
				const irrvec &init_pos,
				irr::video::IVideoDriver *driver,
				float mass,
				const irrvec &init_v,
				float radius);
		void apply_force(const irrvec &f) { m_total_force += f; }
		irrvec get_pos() {return m_pos;}
	private:
		void update(float time_delta);
		void update_ui();

		irrvec m_total_force;
		irrvec m_pos;
		irrvec m_v;
		irr::scene::IMeshSceneNode* m_scene_node;
		irr::video::IVideoDriver* m_driver;
		float m_mass;
		friend class ConstraintedMasses;
	};
	ConstraintedMasses(std::vector<Mass*> masses,
			irr::scene::ISceneManager* smgr);
	void update(float time_delta);
	void update_ui();
	void add_constraint(int i, int j);
private:
	void apply_tension_forces(float time_delta);
	irrvec _t(int i, int j);

	std::vector<Mass*> m_masses;
	irr::scene::ISceneManager* m_smgr;
	std::vector<std::pair<int, int> > m_constraints;
	std::vector<float> m_constraint_lengths;
	std::vector<irr::scene::IMeshSceneNode *> m_constraint_mesh;
};

