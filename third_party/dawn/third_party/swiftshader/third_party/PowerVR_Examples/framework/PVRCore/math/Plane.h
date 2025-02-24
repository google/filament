/*!
\brief A Plane3d class containing functionality for representing and working with 3D Planes.
\file PVRCore/math/Plane.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/glm.h"

namespace pvr {
/// <summary>Uses the plane equation Ax + By + Cz + D = 0, where A B C are plane normal, xyz are position on the
/// plane and D is distance to the plane.</summary>
class Plane3d
{
public:
	/// <summary>Constructs a plane from normal and distance. Distance is the scalar number that is the (unsigned) distance
	/// of this plane from (0,0,0).</summary>
	/// <param name="normal">The normal of this plane. MUST BE NORMALISED. If it not normalized, unexpected results
	/// may occur.</param>
	/// <param name="dist">The signed distance, along the plane's normal direction, between the coordinate start and
	/// (0,0,0). This number is defined as the number that the normal must be multiplied with so that the normal's
	/// coordinates define a point on the plane.</param>
	Plane3d(const glm::vec3& normal, float dist) : _norm(normal), _dist(dist) {}

	/// <summary>Constructs a plane from normal and a point on this plane.</summary>
	/// <param name="normal">The normal of this plane. If it is not normalized, unexpected results may occur.</param>
	/// <param name="pointOnPlane">Any point belonging to this plane</param>
	Plane3d(const glm::vec3& normal, const glm::vec3& pointOnPlane) : _norm(normal) { _dist = glm::length(pointOnPlane); }

	/// <summary>Constructs a plane from three points.</summary>
	/// <param name="point0">A point belonging to the plane</param>
	/// <param name="point1">A point belonging to the plane</param>
	/// <param name="point2">A point belonging to the plane</param>
	Plane3d(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2) { set(point0, point1, point2); }

	/// <summary>Sets a plane from normal and distance. Distance is the scalar number that is the distance of this
	/// plane from (0,0,0).</summary>
	/// <param name="normal">The normal of this plane. MUST BE NORMALISED. If it not normalized, unexpected results
	/// may occur.</param>
	/// <param name="dist">The signed distance, along the plane's normal direction, between the coordinate start and
	/// (0,0,0). This number is defined as the number that the normal must be multiplied with so that the normal's
	/// coordinates define a point on the plane.</param>
	void set(const glm::vec3& normal, float dist) { _norm = normal, _dist = dist; }

	/// <summary>Sets a plane from normal and a point on this plane.</summary>
	/// <param name="normal">The normal of this plane. If it is not normalized, unexpected results may occur.</param>
	/// <param name="pointOnPlane">Any point belonging to this plane</param>
	void set(const glm::vec3& normal, const glm::vec3& pointOnPlane)
	{
		_dist = glm::length(pointOnPlane);
		_norm = normal;
	}

	/// <summary>Sets a plane from three points.</summary>
	/// <param name="point0">A point belonging to the plane</param>
	/// <param name="point1">A point belonging to the plane</param>
	/// <param name="point2">A point belonging to the plane</param>
	void set(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2)
	{
		glm::vec3 edge0 = point0 - point1;
		glm::vec3 edge1 = point2 - point1;

		_norm = glm::normalize(glm::cross(edge0, edge1));
		_dist = -glm::dot(_norm, point0);
	}

	/// <summary>Find the signed distance between a point and the plane. Positive means distance along the normal,
	/// negative means distance opposite to the normal's direction.</summary>
	/// <param name="point">The point</param>
	/// <returns>The signed distance between the point and this plane.</returns>
	float distanceTo(const glm::vec3& point) { return glm::dot(_norm, point) - _dist; }

	/// <summary>Get the distance of this plane to the coordinate start (0,0,0).</summary>
	/// <returns>The distance of this plane to the coordinate start (0,0,0)</returns>
	float getDistance() const { return _dist; }

	/// <summary>Get the normal of this plane.</summary>
	/// <returns>The normal of this plane.</returns>
	glm::vec3 getNormal() const { return _norm; }

	/// <summary>Transform the plane with a transformation matrix.</summary>
	/// <param name="transMtx">The matrix to transform this plane with.</param>
	void transform(glm::mat4& transMtx) { glm::transpose(glm::inverse(transMtx)) * glm::vec4(_norm, _dist); }

private:
	glm::vec3 _norm;
	float _dist;
};
} // namespace pvr
