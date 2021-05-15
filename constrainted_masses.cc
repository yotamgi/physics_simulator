#include "constrainted_masses.h"
#include <armadillo>

namespace irr {
	using namespace core;
	using namespace video;
	using namespace scene;
	using namespace io;
	using namespace gui;
}


ConstraintedMasses::Mass::Mass(
		irr::ISceneManager* smgr,
		const irrvec &init_pos, irr::IVideoDriver* driver,
		float mass,
		const irrvec &init_v,
		float radius)
{
	m_pos = init_pos;
	m_v = init_v;
	m_driver = driver;
	m_mass = mass;

	m_scene_node = smgr->addSphereSceneNode(radius);
	if (!m_scene_node) {
		throw std::runtime_error("Couldn't create scene node.");
	}
	m_scene_node->getMesh()->getMeshBuffer(0)->getMaterial().MaterialType = irr::EMT_SOLID;
	m_scene_node->getMesh()->getMeshBuffer(0)->getMaterial().Shininess = 10;
	m_scene_node->setMaterialFlag(irr::EMF_LIGHTING, true);
	m_scene_node->setPosition(m_pos);
	m_scene_node->setVisible(true);
}

void ConstraintedMasses::Mass::update(float time_delta) {
	m_v += m_total_force * time_delta / m_mass;
	m_pos += m_v * time_delta;
	m_total_force = irrvec(0, 0, 0);
}

void ConstraintedMasses::Mass::update_ui() {
	m_scene_node->setPosition(m_pos);
}

ConstraintedMasses::ConstraintedMasses(std::vector<Mass*> masses):
	m_masses(masses) {}

void ConstraintedMasses::add_constraint(int i, int j) {
	if (i < j) {
		m_constraints.push_back({i, j});
	} else {
		m_constraints.push_back({j, i});
	}
	m_constraint_lengths.push_back(
			(m_masses[i]->m_pos - m_masses[j]->m_pos).getLength());
}

irrvec ConstraintedMasses::_t(int i, int j) {
	return (m_masses[j]->m_pos - m_masses[i]->m_pos).normalize();
}

void ConstraintedMasses::apply_tension_forces(float time_delta) {
	int num_constraints = m_constraints.size();
	arma::mat A(num_constraints, num_constraints, arma::fill::zeros);

	// Fill the constraints matrix.
	for (int i=0; i < num_constraints; i++) {
		int i_mass_0 = std::get<0>(m_constraints[i]);
		int i_mass_1 = std::get<1>(m_constraints[i]);

		for (int j=0; j < num_constraints; j++) {
			int j_mass_0 = std::get<0>(m_constraints[j]);
			int j_mass_1 = std::get<1>(m_constraints[j]);
			if ((i_mass_0 == j_mass_0) && (i_mass_1 == j_mass_1)) {
				A(i,j) = 1 / m_masses[i_mass_0]->m_mass
					+ 1 / m_masses[i_mass_1]->m_mass;
			} else if (i_mass_0 == j_mass_0) {
				int common_mass = i_mass_0;
				int mass_a = i_mass_1;
				int mass_b = j_mass_1;

				float cos_angle = _t(common_mass, mass_a).dotProduct(
						_t(common_mass, mass_b));
				A(i,j) = cos_angle / m_masses[common_mass]->m_mass;
			} else if (i_mass_1 == j_mass_1) {
				int common_mass = i_mass_1;
				int mass_a = i_mass_0;
				int mass_b = j_mass_0;

				float cos_angle = _t(common_mass, mass_a).dotProduct(
						_t(common_mass, mass_b));
				A(i,j) = cos_angle / m_masses[common_mass]->m_mass;
			} else if (i_mass_1 == j_mass_0) {
				int common_mass = i_mass_1;
				int mass_a = i_mass_0;
				int mass_b = j_mass_1;

				float cos_angle = _t(common_mass, mass_a).dotProduct(
						_t(common_mass, mass_b));
				A(i,j) = cos_angle / m_masses[common_mass]->m_mass;
			} else if (i_mass_0 == j_mass_1) {
				int common_mass = i_mass_0;
				int mass_a = i_mass_1;
				int mass_b = j_mass_0;

				float cos_angle = _t(common_mass, mass_a).dotProduct(
						_t(common_mass, mass_b));
				A(i,j) = cos_angle / m_masses[common_mass]->m_mass;
			}
		}
	}

	// Build the f vector.
	arma::vec f(num_constraints, arma::fill::zeros);
	for (int i=0; i < num_constraints; i++) {
		int mass_0 = std::get<0>(m_constraints[i]);
		int mass_1 = std::get<1>(m_constraints[i]);
		f(i) = _t(mass_0, mass_1).dotProduct(m_masses[mass_1]->m_total_force)
				/ m_masses[mass_1]->m_mass
			- _t(mass_0, mass_1).dotProduct(m_masses[mass_0]->m_total_force)
				/ m_masses[mass_0]->m_mass;
	}

	// Calculate the per-constraint tension vector,
	arma::vec t = arma::inv(A) * f;

	// Apply the forces.
	for (int i=0; i < num_constraints; i++) {
		int mass_0 = std::get<0>(m_constraints[i]);
		int mass_1 = std::get<1>(m_constraints[i]);
		m_masses[mass_0]->apply_force(t[i] * _t(mass_0, mass_1));
		m_masses[mass_1]->apply_force(t[i] * _t(mass_1, mass_0));
	}
}

void ConstraintedMasses::update(float time_delta) {
	int num_constraints = m_constraints.size();

	// Apply the tension forces according to the external mass forces.
	apply_tension_forces(time_delta);

	// Apply Centrifugal forces, according to the masses tengencial
	// velocity.
	for (int i=0; i < num_constraints; i++) {
		int mass_0 = std::get<0>(m_constraints[i]);
		int mass_1 = std::get<1>(m_constraints[i]);
		irrvec relative_v = m_masses[mass_1]->m_v - m_masses[mass_0]->m_v;
		irrvec relative_v_norm = relative_v;
		relative_v_norm.normalize();
		irrvec tangential_relative_v = relative_v -
			relative_v_norm.dotProduct(_t(mass_1, mass_0)) * relative_v;
		float tangential_relative_v_magnitude = tangential_relative_v.getLength();
		float actual_constraint_length = (m_masses[mass_1]->m_pos
				- m_masses[mass_0]->m_pos).getLength();
		float radial_acc = tangential_relative_v_magnitude
			* tangential_relative_v_magnitude
			/ actual_constraint_length;
		m_masses[mass_0]->apply_force(m_masses[mass_0]->m_mass
				* radial_acc * _t(mass_0, mass_1));
		m_masses[mass_1]->apply_force(m_masses[mass_1]->m_mass
				* radial_acc * _t(mass_1, mass_0));
	}

	// Add some friction
	for (auto mass : m_masses) {
		mass->apply_force(-mass->m_v * 0.4);
	}

	// Fix velocities to not be in the direction of the constraints.
	for (int i=0; i < num_constraints; i++) {
		int mass_0 = std::get<0>(m_constraints[i]);
		int mass_1 = std::get<1>(m_constraints[i]);

		irrvec relative_v = m_masses[mass_1]->m_v - m_masses[mass_0]->m_v;
		float radial_relative_v = relative_v.dotProduct(_t(mass_1, mass_0));

		m_masses[mass_0]->m_v += radial_relative_v * _t(mass_1, mass_0) / 2;
		m_masses[mass_1]->m_v += radial_relative_v * _t(mass_0, mass_1) / 2;

		relative_v = m_masses[mass_1]->m_v - m_masses[mass_0]->m_v;
		radial_relative_v = relative_v.dotProduct(_t(mass_1, mass_0));
	}


	// Fix positions for constraints.
	for (int i=0; i < num_constraints; i++) {
		int mass_0 = std::get<0>(m_constraints[i]);
		int mass_1 = std::get<1>(m_constraints[i]);
		irrvec diff = m_masses[mass_1]->m_pos - m_masses[mass_0]->m_pos;
		m_masses[mass_1]->m_pos = m_masses[mass_0]->m_pos
			+ diff.normalize() * m_constraint_lengths[i];
	}

	// Update all the masses.
	for (auto mass : m_masses) {
		mass->update(time_delta);
	}

}

void ConstraintedMasses::update_ui() {
	for (auto mass : m_masses) {
		mass->update_ui();
	}
}

