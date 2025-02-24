/*!
\brief Contains A 3 dimensional rectangle class.
\file PVRCore/math/Rectangle3D.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/Types.h"
#include "PVRCore/glm.h"

namespace pvr {
/// <summary>A class representing an Axis-Aligned cuboidal region of 3D space. Internal representation offset
/// defined as the absolute coordinates of one corner of the bounding region and extent which defines the
/// second corner of the bounding region relative to the offset.</summary>
/// <typeparam name="TYPE">The datatype of the units of the 3 dimesional rectangle (int, float etc.)</typeparam>
template<typename TYPE>
struct Rectangle3D
{
	TYPE x; //!< The x-coordinate of the left side of the cuboid
	TYPE y; //!< The y-coordinate of the bottom side of the cuboid
	TYPE z; //!< The z-coordinate of the front of the cuboid
	TYPE width; //!< The width of the cuboid
	TYPE height; //!< The height of the cuboid
	TYPE depth; //!< The depth of the cuboid

	/// <summary>The offset of the rectangle is the minimum vertex (the vertex with the smalles coordinate in each of the
	/// x,y,z components</summary>
	/// <returns>The minimum vertex (usually this means "bottom"-"left"-"back")</returns>
	glm::tvec3<TYPE, glm::highp> offset() const { return glm::tvec3<TYPE, glm::highp>(x, y, z); }

	/// <summary>The extent (aka size, aka width/height/depth) of the cuboid.</summary>
	/// <returns>The extent, a 3D vector composed of the size at x,y,z: (width, height, depth)</returns>
	glm::tvec3<TYPE, glm::highp> extent() const { return glm::tvec3<TYPE, glm::highp>(width, height, depth); }

	/// <summary>The position of the center of the cuboid.</summary>
	/// <returns>The center of the cuboid</returns>
	glm::tvec3<TYPE, glm::highp> center() const { return offset() + extent() / TYPE(2); }
	/// <summary>Create a 3d rectangle with uninitialized values.</summary>
	Rectangle3D() {}

	/// <summary>Create a 3 dimensional rectangle with initial values.</summary>
	/// <param name="absoluteX">The absolute x-coordinate of the left of the rectangle</param>
	/// <param name="absoluteY">The absolute y-coordinate of the bottom of the rectangle</param>
	/// <param name="absoluteZ">The absolute z-coordinate of the front of the rectangle</param>
	/// <param name="width">The width of the rectangle</param>
	/// <param name="height">The height of the rectangle</param>
	/// <param name="depth">The height of the rectangle</param>
	Rectangle3D(TYPE absoluteX, TYPE absoluteY, TYPE absoluteZ, TYPE width, TYPE height, TYPE depth)
		: x(absoluteX), y(absoluteY), z(absoluteZ), width(width), height(height), depth(depth)
	{}

	/// <summary>Create a rectangle with initial values.</summary>
	/// <param name="minimumVertex">The minimum vertex of the 3 dimensional rectangle (bottom, left, usually front)</param>
	/// <param name="dimensions">The dimensions(width, height, depth)</param>
	Rectangle3D(glm::tvec3<TYPE, glm::precision::defaultp> minimumVertex, glm::tvec3<TYPE, glm::precision::defaultp> dimensions)
		: x(minimumVertex.x), y(minimumVertex.y), z(minimumVertex.z), width(dimensions.x), height(dimensions.y), depth(dimensions.z)
	{}

	/// <summary>Equality operator (exact).</summary>
	/// <param name="rhs">The right hand side of the operator</param>
	/// <returns>True if the rectangles coincide (all vertices exactly equal), otherwise false.</returns>
	bool operator==(const Rectangle3D& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (width == rhs.width) && (height == rhs.height) && (depth == rhs.depth); }

	/// <summary>Inequality operator (exact).</summary>
	/// <param name="rhs">The right hand side of the operator</param>
	/// <returns>True if the two rectangles do not exactly coincide (at least one vertex not exactly
	/// equal), otherwise false.</returns>
	bool operator!=(const Rectangle3D& rhs) const { return !(*this == rhs); }

	/// <summary>Expand this rectangle to also contain the given rectangle. Equivalently, set this
	/// rectangles minimum vertex to the minimum of this and rect's min vertex, and set this rectangle's
	/// max vertex to the maximum of this rectangle's and</summary>
	/// <param name="rect">The rectangle that we will ensure is covered</param>
	void expand(const Rectangle3D& rect)
	{
		auto minx = glm::min(x, rect.x);
		auto miny = glm::min(y, rect.y);
		auto minz = glm::min(z, rect.z);
		auto maxx = glm::max(x + height, rect.x + rect.width);
		auto maxy = glm::max(y + height, rect.y + rect.width);
		auto maxz = glm::max(z + height, rect.z + rect.width);

		x = minx;
		y = miny;
		z = minz;
		width = maxx - minx;
		height = maxy - miny;
		depth = maxz - minz;
	}
};

/// <summary>An integer 3D rectangle</summary>
typedef Rectangle3D<int32_t> Rectangle3Di;
/// <summary>A float 3D rectangle</summary>
typedef Rectangle3D<float> Rectangle3Df;
} // namespace pvr
