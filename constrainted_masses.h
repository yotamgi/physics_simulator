#include "common.h"
#include <irrlicht.h>
#include <vector>
#include <stdexcept>

class ConstraintedMasses {
public:
	class Mass {
	public:
		Mass(const irrvec &init_pos,
				float mass,
				const irrvec &init_v,
				float radius);
		void apply_force(const irrvec &f) { m_total_force += f; }
		irrvec get_pos() {return m_pos;}
		irrvec get_v() {return m_v;}
	private:
		void update(float time_delta);
		void update_ui();

		irrvec m_total_force;
		irrvec m_pos;
		irrvec m_v;
		irr::scene::IMeshSceneNode* m_scene_node;
		float m_mass;
		friend class ConstraintedMasses;
	};
	ConstraintedMasses(std::vector<Mass*> masses);
	void update(float time_delta);
	void update_ui();
	void add_constraint(int i, int j);
private:
	void apply_tension_forces(float time_delta);
	irrvec _t(int i, int j);

	std::vector<Mass*> m_masses;
	std::vector<std::pair<int, int> > m_constraints;
	std::vector<float> m_constraint_lengths;
	std::vector<irr::scene::IMeshSceneNode *> m_constraint_mesh;
};

