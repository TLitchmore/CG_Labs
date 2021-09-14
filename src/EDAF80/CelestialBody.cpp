#include "CelestialBody.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/helpers.hpp"
#include "core/Log.h"


CelestialBody::CelestialBody(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id)
{
	_body.node.set_geometry(shape);
	_body.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_body.node.set_program(program);
}

glm::mat4 CelestialBody::render(std::chrono::microseconds elapsed_time,
                                glm::mat4 const& view_projection,
                                glm::mat4 const& parent_transform,
                                bool show_basis)
{
	// Convert the duration from microseconds to seconds.
	auto const elapsed_time_s = std::chrono::duration<float>(elapsed_time).count();
	// If a different ratio was needed, for example a duration in
	// milliseconds, the following would have been used:
	// auto const elapsed_time_ms = std::chrono::duration<float, std::milli>(elapsed_time).count();

	_body.spin.rotation_angle = _body.spin.rotation_angle+_body.spin.speed*elapsed_time_s;
	_body.orbit.rotation_angle = _body.orbit.rotation_angle + _body.orbit.speed * elapsed_time_s;
	//Create the orbital axis.
	glm::vec3 orbital_axis = glm::vec3(_body.orbit.radius, 0, 0);
	glm::vec3 xaxis = glm::vec3(1, 0, 0);
	//Creating the y-axis
	glm::vec3 yaxis = glm::vec3(0, 1, 0);

	//Computing the rotation matrix around the y-axis
	glm::mat4 rotation_y = glm::rotate(glm::mat4(1.0f), _body.spin.rotation_angle,yaxis);

	//Creating the z-axis
	glm::vec3 zaxis = glm::vec3(0, 0, 1);
	//Computing the rotation matrix around the z-axis.
	glm::mat4 rotation_z = glm::rotate(glm::mat4(1.0f), _body.spin.axial_tilt, zaxis);

	//Compute the rotation matrix. Z-rotation matrix comes before the y-rotation matrix
	//because the planet spins around the tilted z-axis.
	glm::mat4 rotation_matrix = rotation_z * rotation_y;

	//Compute the translation matrix.
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), orbital_axis);

	//Compute the rotation matrix  for the orbital axis.
	glm::mat4 rotation_orbit_axis = glm::rotate(glm::mat4(1.0f), _body.orbit.rotation_angle, yaxis);

	//Computing the orbital rotation.
	glm::mat4 orbital_rotation = rotation_orbit_axis * translation_matrix;

	//Computing orbital tilt.
	glm::mat4 orbital_tilt = glm::rotate(glm::mat4(1.0f), _body.orbit.inclination, zaxis)*translation_matrix;
	//Computing the scaling matrix.
	glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.0f), _body.scale);
	glm::mat4 world = parent_transform*orbital_rotation*orbital_tilt*rotation_matrix;
	//These are the transforms applied to the children of the current node. The parent transforms of the previous node,
	//the orbital rotation, orbital tilt and the spin tilt in the z-axis is what should be returned to the children of this node.
	glm::mat4 children_matrix = parent_transform * orbital_rotation * orbital_tilt * rotation_z;

	if (show_basis)
	{
		bonobo::renderBasis(1.0f, 2.0f, view_projection, world);
	}

	// Note: The second argument of `node::render()` is supposed to be the
	// parent transform of the node, not the whole world matrix, as the
	// node internally manages its local transforms. However in our case we
	// manage all the local transforms ourselves, so the internal transform
	// of the node is just the identity matrix and we can forward the whole
	// world matrix.
	_body.node.render(view_projection, world);

	return children_matrix;
}

void CelestialBody::add_child(CelestialBody* child)
{
	_children.push_back(child);
}

std::vector<CelestialBody*> const& CelestialBody::get_children() const
{
	return _children;
}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
	_body.orbit.radius = configuration.radius;
	_body.orbit.inclination = configuration.inclination;
	_body.orbit.speed = configuration.speed;
	_body.orbit.rotation_angle = 0.0f;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
	_body.scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
	_body.spin.axial_tilt = configuration.axial_tilt;
	_body.spin.speed = configuration.speed;
	_body.spin.rotation_angle = 0.0f;
}

void CelestialBody::set_ring(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id,
                             glm::vec2 const& scale)
{
	_ring.node.set_geometry(shape);
	_ring.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_ring.node.set_program(program);

	_ring.scale = scale;
}
