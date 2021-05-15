#include <irrlicht.h>
#include <vector>
#include <stdexcept>

class ConstraintedMasses {
public:
	class Mass {
	public:
		Mass(irr::scene::ISceneManager* smgr,
				const irr::core::vector3df &init_pos,
				irr::video::IVideoDriver *driver,
				float mass,
				const irr::core::vector3df &init_v,
				float radius);
		void apply_force(const irr::core::vector3df &f) { m_total_force += f; }
		irr::core::vector3df get_pos() {return m_pos;}
	private:
		void update(float time_delta);
		void update_ui();

		irr::core::vector3df m_total_force;
		irr::core::vector3df m_pos;
		irr::core::vector3df m_v;
		irr::scene::IMeshSceneNode* m_scene_node;
		irr::video::IVideoDriver* m_driver;
		float m_mass;
		friend class ConstraintedMasses;
	};
	ConstraintedMasses(std::vector<Mass*> masses);
	void update(float time_delta);
	void update_ui();
	void add_constraint(int i, int j);
private:
	void apply_tension_forces(float time_delta);
	irr::core::vector3df _t(int i, int j);

	std::vector<Mass*> m_masses;
	std::vector<std::pair<int, int> > m_constraints;
	std::vector<float> m_constraint_lengths;
};

