/*!
\brief Class used to handle common AABB operations.
\file PVRCore/math/AxisAlignedBox.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/Types.h"
#include "PVRCore/glm.h"
#include <cfloat>

namespace pvr {
/// <summary>Contains mathematical functionality and classes, such as bounding box calculations, intersections etc.</summary>
namespace math {

/// <summary>This class provides functionality to handle the volume enclosed by 6 planes, normally the viewing
/// frustum. The planes are represented in Hessian Normal Form (normal, distance) as vec4( (xyz):[normal],
/// w:[distance from 0,0,0] )</summary>
struct Frustum
{
	glm::vec4 minusX; //!< The minimum X (left) plane expressed as normal-distance
	glm::vec4 plusX; //!< The maximum X (right) plane expressed as normal-distance
	glm::vec4 minusY; //!< The minimum Y (bottom) plane expressed as normal-distance
	glm::vec4 plusY; //!< The maximum Y (top) plane expressed as normal-distance
	glm::vec4 minusZ; //!< The minimum Z (near) plane expressed as normal-distance
	glm::vec4 plusZ; //!< The maximum Z (far) plane expressed as normal-distance

	glm::vec3 points[8]; //!< The set of points making up the frustum
};

/// <summary>This class provides specialized functionality for when a frustum is a "normal"viewing frustum, that
/// is, the following conditions hold (The conditions ARE NOT checked) Note: A "frustum side" of a plane means "the
/// quadrilateral of the plane that is enclosed by the 4 other planes not opposite to it" "Opposite" means the
/// "other" side of the frustum (minusX is opposit plusX).
/// 1) Opposite Frustum sides do not intersect (their planes may do so outside the frustum)
/// 2) The frustum is "opening", or at least not "closing" accross the z axis accross all directions, meaning that
/// the any point of the projection of the negative Z side of the frustum on the positive Z plane, is inside or
/// on the positive Z side of the frustum.
/// 3) Any point of a positive(negative) part of the frustum has a larger(smaller) corresponding coordinate than
/// its opposite part
/// 4) All plane normals point INTO of the frustum
/// These optimizations allow us to greatly reduce the calculations for a viewing frustum.</summary>
struct ViewingFrustum : public Frustum
{
	/// <summary>Check if a Viewing Frustum is valid (the viewing frustum conditions hold)</summary>
	/// <returns>True if a Viewing Frustum, otherwise False</returns>
	bool isFrustum() const
	{
		bool xopp = (glm::dot(glm::vec3(minusX), glm::vec3(plusX)) < 0);
		bool yopp = (glm::dot(glm::vec3(minusY), glm::vec3(plusY)) < 0);
		bool zopp = (glm::dot(glm::vec3(minusZ), glm::vec3(plusZ)) < 0);
		return xopp && yopp && zopp;
	};
};

/// <summary>Calculate the signed (in regards to the plane's normal) distance from a point to a plane</summary>
/// <param name="point">The point</param>
/// <param name="plane">The plane, defined as a vector4, where (xyz: Normal, w: Distance from the origin)</param>
/// <returns>The distance from the point to the plane (along the normal). Positive if on the side of the normal,
/// otherwise false</returns>
inline float distancePointToPlane(const glm::vec3& point, const glm::vec4& plane) { return glm::dot(point, glm::vec3(plane.x, plane.y, plane.z)) + plane.w; }

/// <summary>Calculate if the point is in the positive half-space defined by the plane (i.e. if it is lying on the
/// same side as the normal points</summary>
/// <param name="point">The point</param>
/// <param name="plane">The plane, defined as a vector4, where (xyz: Normal, w: Distance from the origin)</param>
/// <returns>True if the point is on the plane or on the same side as the normal, otherwise false</returns>
inline bool pointOnSide(const glm::vec3& point, const glm::vec4& plane) { return distancePointToPlane(point, plane) >= 0.0f; }

/// <summary>Retrieve the viewing frustum from a projection matrix</summary>
/// <param name="projection_from_world">The projection matrix</param>
/// <param name="frustum_out">The viewing frustum</param>
/// <param name="api">The Graphics API for which to get the planes. Used to infer and use the underlying Device coordinate system.</param>
inline void getFrustumPlanes(pvr::Api api, const glm::mat4& projection_from_world, ViewingFrustum& frustum_out)
{
	const glm::vec4 row0 = glm::row(projection_from_world, 0);
	const glm::vec4 row1 = glm::row(projection_from_world, 1) * (api == pvr::Api::Vulkan ? -1.f : 1.f);
	const glm::vec4 row2 = glm::row(projection_from_world, 2);
	const glm::vec4 row3 = glm::row(projection_from_world, 3);

	frustum_out.minusX = row3 + row0;
	frustum_out.plusX = row3 - row0;
	frustum_out.minusY = row3 + row1;
	frustum_out.plusY = row3 - row1;
	frustum_out.minusZ = row3 + row2;
	frustum_out.plusZ = row3 - row2;

	frustum_out.minusX = frustum_out.minusX * (1 / glm::length(glm::vec3(frustum_out.minusX)));
	frustum_out.plusX = frustum_out.plusX * (1 / glm::length(glm::vec3(frustum_out.plusX)));
	frustum_out.minusY = frustum_out.minusY * (1 / glm::length(glm::vec3(frustum_out.minusY)));
	frustum_out.plusY = frustum_out.plusY * (1 / glm::length(glm::vec3(frustum_out.plusY)));
	frustum_out.minusZ = frustum_out.minusZ * (1 / glm::length(glm::vec3(frustum_out.minusZ)));
	frustum_out.plusZ = frustum_out.plusZ * (1 / glm::length(glm::vec3(frustum_out.plusZ)));
}

/// <summary>Retrieves the point at which 3 planes intersect</summary>
/// <param name="p0">Plane 0</param>
/// <param name="p1">Plane 1</param>
/// <param name="p2">Plane 2</param>
/// <returns>The point at which the 3 planes intersect</returns>
inline glm::vec3 IntersectPlanes(glm::vec4 p0, glm::vec4 p1, glm::vec4 p2)
{
	glm::vec3 bxc = glm::cross(glm::vec3(p1), glm::vec3(p2));
	glm::vec3 cxa = glm::cross(glm::vec3(p2), glm::vec3(p0));
	glm::vec3 axb = glm::cross(glm::vec3(p0), glm::vec3(p1));
	glm::vec3 r = -p0.w * bxc - p1.w * cxa - p2.w * axb;
	return r * (1 / glm::dot(glm::vec3(p0), bxc));
}

/// <summary>Retrieve the points of a viewing frustum by intersecting planes</summary>
/// <param name="frustum_out">The viewing frustum</param>
inline void getFrustumPoints(ViewingFrustum& frustum_out)
{
	frustum_out.points[0] = IntersectPlanes(frustum_out.minusZ, frustum_out.minusX, frustum_out.minusY);
	frustum_out.points[1] = IntersectPlanes(frustum_out.minusZ, frustum_out.minusX, frustum_out.plusY);
	frustum_out.points[2] = IntersectPlanes(frustum_out.minusZ, frustum_out.plusX, frustum_out.plusY);
	frustum_out.points[3] = IntersectPlanes(frustum_out.minusZ, frustum_out.plusX, frustum_out.minusY);
	frustum_out.points[4] = IntersectPlanes(frustum_out.plusZ, frustum_out.minusX, frustum_out.minusY);
	frustum_out.points[5] = IntersectPlanes(frustum_out.plusZ, frustum_out.minusX, frustum_out.plusY);
	frustum_out.points[6] = IntersectPlanes(frustum_out.plusZ, frustum_out.plusX, frustum_out.plusY);
	frustum_out.points[7] = IntersectPlanes(frustum_out.plusZ, frustum_out.plusX, frustum_out.minusY);
}

class AxisAlignedBoxMinMax;

/// <summary>This class provides functionality to handle 3 dimensional Axis Aligned Boxes. Center-halfextent
/// representation.</summary>
class AxisAlignedBox
{
	glm::vec3 _center;
	glm::vec3 _halfExtent;

public:
	/// <summary>Constructor center/halfextent.</summary>
	/// <param name="center">The center of the box</param>
	/// <param name="halfExtent">A vector containing the half-lengths on each axis</param>
	AxisAlignedBox(const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& halfExtent = glm::vec3(0.f)) : _center(center), _halfExtent(halfExtent) {}

	/// <summary>Conversion constructor from AxisAlignedBoxMinMax</summary>
	/// <param name="copyFrom">An AxisAlignedBoxMinMax to convert from. An invalid AxisAlignedBoxMinMax will result in an empty AxisAlignedBox</param>
	AxisAlignedBox(const AxisAlignedBoxMinMax& copyFrom);

	/// <summary>Sets center and extents to zero.</summary>
	void clear()
	{
		_center.x = _center.y = _center.z = 0.0f;
		_halfExtent = glm::vec3(0.f);
	}

	/// <summary>Sets from min and max. All components of min must be than all components in max.</summary>
	/// <param name="min">Minimum coordinates. Must each be less than the corresponding max coordinate.</param>
	/// <param name="max">Maximum coordinates. Must each be greater than the corresponding min coordinate.</param>
	void setMinMax(const glm::vec3& min, const glm::vec3& max)
	{
		// compute the center.
		_center = (max + min) * .5f;
		_halfExtent = (max - min) * .5f;
	}

	/// <summary>Sets from center and half extent.</summary>
	/// <param name="center">The center of the box</param>
	/// <param name="halfExtent">A vector containing the half-lengths on each axis</param>
	void set(const glm::vec3& center, const glm::vec3& halfExtent)
	{
		_center = center;
		_halfExtent = halfExtent;
	}

	/// <summary>Add a new point to the box. The new box will be the minimum box containing the old box and the new
	/// point.</summary>
	/// <param name="point">The point which to add</param>
	void add(const glm::vec3& point) { setMinMax(glm::min(point, getMin()), glm::max(point, getMax())); }

	/// <summary>Merge two axis aligned boxes. The new box will be the minimum box containing both the old and the new
	/// box.</summary>
	/// <param name="aabb">The aabb which to add</param>
	void add(const AxisAlignedBox& aabb)
	{
		add(aabb.getMin());
		add(aabb.getMax());
	}

	/// <summary>Add a new point to the box. The new box will be the minimum box containing the old box and the new
	/// point.</summary>
	/// <param name="x">The x-coord of the point which to add</param>
	/// <param name="y">The y-coord of the point which to add</param>
	/// <param name="z">The z-coord of the point which to add</param>
	void add(float x, float y, float z) { add(glm::vec3(x, y, z)); }

	/// <summary>Return a point consisting of the smallest (minimum) coordinate in each axis.</summary>
	/// <returns>A point consisting of the smallest (minimum) coordinate in each axis</returns>
	glm::vec3 getMin() const { return _center - _halfExtent; }

	/// <summary>Return a point consisting of the largest (maximum) coordinate in each axis.</summary>
	/// <returns>A point consisting of the largest (maximum) coordinate in each axis</returns>
	glm::vec3 getMax() const { return _center + _halfExtent; }

	/// <summary>Return the min and the max.</summary>
	/// <param name="outMin">Output: The min point will be stored here.</param>
	/// <param name="outMax">Output: The max point will be stored here.</param>
	void getMinMax(glm::vec3& outMin, glm::vec3& outMax) const
	{
		outMin = _center - _halfExtent;
		outMax = _center + _halfExtent;
	}

	/// <summary>Get the local bounding box transformed by a provided matrix.</summary>
	/// <param name="m">An affine transformation matrix. Skew will be ignored.</param>
	/// <param name="outAABB">The transformed AABB will be stored here. Previous contents ignored.</param>
	void transform(const glm::mat4& m, AxisAlignedBox& outAABB) const
	{
		outAABB._center = glm::vec3(m[3]) + (glm::mat3(m) * this->center());

		glm::mat3 absModelMatrix;
		absModelMatrix[0] = glm::vec3(fabs(m[0][0]), fabs(m[0][1]), fabs(m[0][2]));
		absModelMatrix[1] = glm::vec3(fabs(m[1][0]), fabs(m[1][1]), fabs(m[1][2]));
		absModelMatrix[2] = glm::vec3(fabs(m[2][0]), fabs(m[2][1]), fabs(m[2][2]));

		outAABB._halfExtent = glm::vec3(absModelMatrix * this->getHalfExtent());
	}

	/// <summary>Get the size (width, height, depth) of the AABB.</summary>
	/// <returns>The size of the AABB.</returns>
	glm::vec3 getSize() const { return _halfExtent + _halfExtent; }

	/// <summary>Get the half-size (half-width, half-height, half-depth) of the AABB.</summary>
	/// <returns>A vector whose coordinates are each half the size of the coresponding axis.</returns>
	glm::vec3 getHalfExtent() const { return _halfExtent; }

	/// <summary>Get the (-x +y +z) corner of the box.</summary>
	/// <returns>A vector containing (min x, max y, max z)</returns>
	glm::vec3 topLeftFar() const { return _center + glm::vec3(-_halfExtent.x, _halfExtent.y, _halfExtent.z); }

	/// <summary>Get the center of the (+y +z) edge of the box.</summary>
	/// <returns>A vector containing (center x, max y, max z)</returns>
	glm::vec3 topCenterFar() const { return _center + glm::vec3(0, _halfExtent.y, _halfExtent.z); }

	/// <summary>Get the +x +y +z corner of the box.</summary>
	/// <returns>A vector containing (max x, max y, max z)</returns>
	glm::vec3 topRightFar() const { return _center + _halfExtent; }

	/// <summary>Get the -x +y -z corner of the box.</summary>
	/// <returns>A vector containing (min x, max y, min z)</returns>
	glm::vec3 topLeftNear() const { return _center + glm::vec3(-_halfExtent.x, _halfExtent.y, -_halfExtent.z); }

	/// <summary>Get the center of the edge with ( +y , -z).</summary>
	/// <returns>A vector containing (center of x, max y, min z)</returns>
	glm::vec3 topCenterNear() const { return _center + glm::vec3(0., _halfExtent.y, -_halfExtent.z); }

	/// <summary>Get the +x +y -z corner of the box.</summary>
	/// <returns>A vector containing (max x, max y, min z)</returns>
	glm::vec3 topRightNear() const { return _center + glm::vec3(_halfExtent.x, _halfExtent.y, -_halfExtent.z); }

	/// <summary>Get the center the box.</summary>
	/// <returns>A vector containing (center x, center y, center z)</returns>
	glm::vec3 center() const { return _center; }

	/// <summary>Get the center of the (-x -z) edge of the box.</summary>
	/// <returns>A vector containing (min x, center y, min z)</returns>
	glm::vec3 centerLeftNear() const { return _center + glm::vec3(-_halfExtent.x, 0, -_halfExtent.z); }
	/// <summary>Get the center of the (-z) face of the box.</summary>
	/// <returns>A vector containing (center x, center y, min z)</returns>
	glm::vec3 centerNear() const { return _center + glm::vec3(0, 0, -_halfExtent.z); }

	/// <summary>Get the center of the (+x -z) edge of the box.</summary>
	/// <returns>A vector containing (max x, center y, min z)</returns>
	glm::vec3 centerRightNear() const { return _center + glm::vec3(_halfExtent.x, 0, -_halfExtent.z); }

	/// <summary>Get the center of the (-x +z) edge of the box.</summary>
	/// <returns>A vector containing (min x, center y, max z)</returns>
	glm::vec3 centerLeftFar() const { return _center + glm::vec3(-_halfExtent.x, 0, _halfExtent.z); }

	/// <summary>Get the center of the (+z) face of the box.</summary>
	/// <returns>A vector containing (center x, center y, max z)</returns>
	glm::vec3 centerFar() const { return _center + glm::vec3(0, 0, _halfExtent.z); }

	/// <summary>Get the center of the (+x +z) edge of the box.</summary>
	/// <returns>A vector containing (max x, center y, max z)</returns>
	glm::vec3 centerRightFar() const { return _center + glm::vec3(_halfExtent.x, 0, _halfExtent.z); }

	/// <summary>Get the (-x -y -z) corner of the box.</summary>
	/// <returns>A vector containing (min x, min y, min z)</returns>
	glm::vec3 bottomLeftNear() const { return _center + glm::vec3(-_halfExtent.x, -_halfExtent.y, -_halfExtent.z); }

	/// <summary>Get the center of the (-x -z) edge of the box.</summary>
	/// <returns>A vector containing (center x, min y, min z)</returns>
	glm::vec3 bottomCenterNear() const { return _center + glm::vec3(0, -_halfExtent.y, -_halfExtent.z); }

	/// <summary>Get the (+x -y -z) corner of the box.</summary>
	/// <returns>A vector containing (max x, min y, min z)</returns>
	glm::vec3 bottomRightNear() const { return _center + glm::vec3(_halfExtent.x, -_halfExtent.y, -_halfExtent.z); }

	/// <summary>Get the (-x -y +z) corner of the box.</summary>
	/// <returns>A vector containing (min x, min y, max z)</returns>
	glm::vec3 bottomLeftFar() const { return _center + glm::vec3(-_halfExtent.x, -_halfExtent.y, _halfExtent.z); }

	/// <summary>Get the center of the (-y +z) edge of the box.</summary>
	/// <returns>A vector containing (center x, min y, max z)</returns>
	glm::vec3 bottomCenterFar() const { return _center + glm::vec3(0, -_halfExtent.y, _halfExtent.z); }

	/// <summary>Get the (+x -y +z) corner of the box.</summary>
	/// <returns>A vector containing (max x, min y, max z)</returns>
	glm::vec3 bottomRightFar() const { return _center + glm::vec3(_halfExtent.x, -_halfExtent.y, _halfExtent.z); }

	/// <summary>Set this AABB as the minimum AABB that contains itself and the AABB provided.</summary>
	/// <param name="rhs">The AABB to merge with.</param>
	void mergeBox(const AxisAlignedBox& rhs) { setMinMax(glm::min(getMin(), rhs.getMin()), glm::max(getMax(), rhs.getMax())); }

	/// <summary>Equality operator. AABBS are equal when center and size the same (equivalently all vertices coincide)</summary>
	/// <param name="rhs">The right hand side.</param>
	/// <returns>True if the AABBs completely coincide, otherwise false</returns>
	bool operator==(const AxisAlignedBox& rhs) const { return _center == rhs._center && _halfExtent == rhs._halfExtent; }

	/// <summary>Inequality operator. AABBS are equal when either center or size are not the same (equivalently at least one vertex does not)</summary>
	/// <param name="rhs">The right hand side.</param>
	/// <returns>True if the AABBs completely coincide, otherwise false</returns>
	bool operator!=(const AxisAlignedBox& rhs) const { return !(*this == rhs); }
};

namespace {
/// <summary>Test if none of 8 points are in the positive side of the plane.</summary>
/// <param name="blf">Point 0.</summary>
/// <param name="tlf">Point 1.</summary>
/// <param name="brf">Point 2.</summary>
/// <param name="trf">Point 3.</summary>
/// <param name="bln">Point 0.</summary>
/// <param name="tln">Point 1.</summary>
/// <param name="brn">Point 2.</summary>
/// <param name="trn">Point 3.</summary>
/// <param name="plane">The plane, expressed as Normal(xyz) plus Distance from Origin (w)</summary>
/// <returns>True if none of the 8 points are on the plane or towards the positive side, otherwise
/// false.</returns>
inline bool noPointsOnSide(glm::vec3 blf, glm::vec3 tlf, glm::vec3 brf, glm::vec3 trf, glm::vec3 bln, glm::vec3 tln, glm::vec3 brn, glm::vec3 trn, glm::vec4 plane)
{
	if (pointOnSide(blf, plane) || pointOnSide(tlf, plane) || pointOnSide(brf, plane) || pointOnSide(trf, plane) || pointOnSide(bln, plane) || pointOnSide(tln, plane) ||
		pointOnSide(brn, plane) || pointOnSide(trn, plane))
	{ return false; }
	return true;
}
} // namespace

/// <summary>Test if an AABB intersects or is inside a frustum.</summary>
/// <param name="box">A box</param>
/// <param name="frustum">A frustum</param>
/// <returns>False if the AABB is completely outside the frustum, otherwise true</returns>
inline bool aabbInFrustum(const AxisAlignedBox& box, const ViewingFrustum& frustum)
{
	glm::vec3 blf = box.bottomLeftFar();
	glm::vec3 tlf = box.topLeftFar();
	glm::vec3 brf = box.bottomRightFar();
	glm::vec3 trf = box.topRightFar();

	glm::vec3 bln = box.bottomLeftNear();
	glm::vec3 tln = box.topLeftNear();
	glm::vec3 brn = box.bottomRightNear();
	glm::vec3 trn = box.topRightNear();

	if (noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.minusX) || noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.plusX) ||
		noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.minusY) || noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.plusY) ||
		noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.minusZ) || noPointsOnSide(blf, tlf, brf, trf, bln, tln, brn, trn, frustum.plusZ))
	{ return false; }
	return true;
}

/// <summary>An AABB with a min-max representation. A newly constructed AxisAlignedBoxMinMax is always invalid (min>max), but in such a way that adding
/// any point to it will make it valid.</summary>
class AxisAlignedBoxMinMax
{
	glm::vec3 _min;
	glm::vec3 _max;

public:
	/// <summary>Constructor. The created AxisAlignedBoxMinMax has the min set at the highest-valued float number, and the max set at the lowest-valued float number,
	/// so but will immediately become valid when the first value is added</summary>
	/// <param name="copyFrom">An AxisAlignedBox to convert from.</param>
	AxisAlignedBoxMinMax() : _min(std::numeric_limits<float>::max()), _max(std::numeric_limits<float>::lowest()) {}

	/// <summary>Conversion from AxisAlignedBox.</summary>
	/// <param name="copyFrom">An AxisAlignedBox to convert from.</param>
	explicit AxisAlignedBoxMinMax(const AxisAlignedBox& copyFrom) : _min(copyFrom.getMin()), _max(copyFrom.getMax()) {}

	/// <summary>Set the minimum corner.</summary>
	/// <param name="min">The minimum corner</param>
	void setMin(const glm::vec3& min) { _min = min; }
	/// <summary>Set the maximum corner.</summary>
	/// <param name="max">The maximum corner</param>
	void setMax(const glm::vec3& max) { _max = max; }
	/// <summary>Get the minimum corner.</summary>
	/// <returns>The minumum corner</param>
	const glm::vec3& getMin() const { return _min; }
	/// <summary>Get the maximum corner.</summary>
	/// <returns>The maximum corner</param>
	const glm::vec3& getMax() const { return _max; }

	/// <summary>Ensure a point is contained in the AABB. If already in the AABB, do nothing.
	/// If not in the AABB, expand the AABB to exactly contain it.</summary>
	/// <param name="point">The point to add</param>
	/// <returns>The new point.</param>
	void add(const glm::vec3& point)
	{
		_min = glm::min(_min, point);
		_max = glm::max(_max, point);
	}
};

inline AxisAlignedBox::AxisAlignedBox(const AxisAlignedBoxMinMax& copyFrom) { setMinMax(copyFrom.getMin(), copyFrom.getMax()); }

} // namespace math
} // namespace pvr
