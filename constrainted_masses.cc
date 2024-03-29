#include "constrainted_masses.h"
#include "renderer.h"
#include <armadillo>
#include <cmath>

ConstraintedMasses::Mass::Mass(
		const irrvec &init_pos,
		float mass,
		const irrvec &init_v,
		float radius)
{
	m_pos = init_pos;
	m_v = init_v;
	m_mass = mass;

	m_scene_node = SimulatorRenderer::get_smgr()->addSphereSceneNode(radius);
	if (!m_scene_node) {
		throw std::runtime_error("Couldn't create scene node.");
	}
	m_scene_node->getMesh()->getMeshBuffer(0)->getMaterial().MaterialType = irr::EMT_SOLID;
	m_scene_node->getMesh()->getMeshBuffer(0)->getMaterial().Shininess = 10;
	m_scene_node->setMaterialFlag(irr::EMF_LIGHTING, true);
	m_scene_node->setPosition(m_pos);
	m_scene_node->setVisible(true);
}

ConstraintedMasses::Mass::~Mass() {
	m_scene_node->remove();
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
	m_masses(masses)
{}

ConstraintedMasses::~ConstraintedMasses() {
	for (auto constraint_mesh : m_constraint_mesh) {
		constraint_mesh->remove();
	}
}

void ConstraintedMasses::add_constraint(int i, int j) {
	if (i < j) {
		m_constraints.push_back({i, j});
	} else {
		m_constraints.push_back({j, i});
	}
	float constraint_length = (m_masses[i]->m_pos - m_masses[j]->m_pos).getLength();
	m_constraint_lengths.push_back(constraint_length);

	// Add the cylinder shape on the constraint.
	auto smgr = SimulatorRenderer::get_smgr();
	irr::IMesh* mesh = smgr->getGeometryCreator()->createArrowMesh(
			10, 20,
			/* height */ constraint_length,
			/* head height */ constraint_length,
			/* width */ 0.3,
			/* head width */ 0.3,
			0xffff0000, 0xffff0000);
	auto scene_node = smgr->addMeshSceneNode(mesh);
	scene_node->getMesh()->getMeshBuffer(0)->getMaterial().MaterialType = irr::EMT_SOLID;
	scene_node->getMesh()->getMeshBuffer(0)->getMaterial().Shininess = 10;
	scene_node->setMaterialFlag(irr::EMF_LIGHTING, true);
	scene_node->setVisible(true);
	m_constraint_mesh.push_back(scene_node);
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

	// Apply Centrifugal forces, according to the masses tengencial
	// velocity.
	// Note: the centrifugal forces must be calculated *after* the masses
	// are updated, in order to prevent the system from accelerating itself
	// on high velocities.
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

	// Apply the tension forces according to the external mass forces.
	apply_tension_forces(time_delta);

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

	// Add some friction
	//for (auto mass : m_masses) {
	//	mass->apply_force(-mass->m_v * 0.05);
	//}

	// Update all the masses.
	for (auto mass : m_masses) {
		mass->update(time_delta);
	}
}

void ConstraintedMasses::update_ui() {
	for (auto mass : m_masses) {
		mass->update_ui();
	}

	// Update the constraint's cylinder transformation.
	for (unsigned int i=0; i<m_constraints.size(); i++) {
		int mass_0 = std::get<0>(m_constraints[i]);
		int mass_1 = std::get<1>(m_constraints[i]);
		irrvec position = m_masses[mass_0]->m_pos;
		irrvec diff = m_masses[mass_1]->m_pos - m_masses[mass_0]->m_pos;
		diff.normalize();

		// Calculate the rotation via quaternions.
		float rot_angle = std::acos(diff.Y);
		irrvec rot_axis = irrvec(0.00001, 1.000001, 0.00001).crossProduct(diff).normalize();
		irr::quaternion q;
		q.fromAngleAxis(rot_angle, rot_axis);
		irrvec rotation;
		q.toEuler(rotation);
		rotation *= irr::RADTODEG;

		// Update position and rotation of the scene node.
		m_constraint_mesh[i]->setPosition(position);
		m_constraint_mesh[i]->setRotation(rotation);
	}
}

