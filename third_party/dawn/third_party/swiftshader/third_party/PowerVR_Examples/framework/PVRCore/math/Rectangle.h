/*!
\brief Contains A rectangle class.
\file PVRCore/math/Rectangle.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/Types.h"
#include "PVRCore/glm.h"

template<typename>
struct GenericOffset2D;
namespace pvr {
/// <summary>A class representing an axis-aligned rectangle. Internal representation TopLeft and size.</summary>
/// <typeparam name="TYPE">The datatype of the units of the rectangle (int, float etc.)</typeparam>
template<typename TYPE>
struct Rectangle
{
	TYPE x; //!< The x-coordinate of the left side of the rectangle
	TYPE y; //!< The y-coordinate of the bottom side of the rectangle
	TYPE width; //!< The width of the rectangle
	TYPE height; //!< The height of the rectangle

	/// <summary>Get the offset of the rectangle, i.e. the position of its minimum (bottom-left) vertex</summary>
	/// <returns>The position of its minimum (bottom-left) vertex</returns>
	glm::tvec2<TYPE, glm::highp> offset() const { return glm::tvec2<TYPE, glm::highp>(x, y); }

	/// <summary>Get the extent (i.e. size) of the rectangle</summary>
	/// <returns>A vec2 containing the sizes per component (width in x, height in y)</returns>
	glm::tvec2<TYPE, glm::highp> extent() const { return glm::tvec2<TYPE, glm::highp>(width, height); }

	/// <summary>Get the position of the center of the rectangle</summary>
	/// <returns>The position of the center of the rectangle</returns>
	glm::tvec2<TYPE, glm::highp> center() const { return offset() + extent() / TYPE(2); }

	/// <summary>Create a rectangle with uninitialized values.</summary>
	Rectangle() {}

	/// <summary>Copy construct a rectangle from two corners as offsets. It is an error and will have undefined results
	/// for any component (x or y) of offset1 to be less than the corresponding component of offset0</summary>
	/// <param name="offset0">Minimum (lesser value for both axes, usually means bottom-left) corner of the new rectangle</param>
	/// <param name="offset1">Maximum (greater value for both axes, usually means top-right) corner of the rectangle</param>
	Rectangle(const GenericOffset2D<TYPE>& offset0, const GenericOffset2D<TYPE>& offset1)
	{
		x = offset0.x;
		y = offset0.y;
		width = offset1.x - x;
		height = offset1.y - y;
	}

	/// <summary>Create a rectangle with initial values.</summary>
	/// <param name="TX">The x-coordinate of the left of the rectangle</param>
	/// <param name="TY">The y-coordinate of the bottom of the rectangle</param>
	/// <param name="TWidth">The width of the rectangle</param>
	/// <param name="THeight">The height of the rectangle</param>
	Rectangle(TYPE TX, TYPE TY, TYPE TWidth, TYPE THeight) : x(TX), y(TY), width(TWidth), height(THeight) {}

	/// <summary>Create a rectangle with initial values.</summary>
	/// <param name="bottomLeft">The bottom-left corner of the rectangle (bottom, left)</param>
	/// <param name="dimensions">The dimensions(width, height)</param>
	Rectangle(glm::tvec2<TYPE, glm::precision::defaultp> bottomLeft, glm::tvec2<TYPE, glm::precision::defaultp> dimensions)
		: x(bottomLeft.x), y(bottomLeft.y), width(dimensions.x), height(dimensions.y)
	{}

	/// <summary>Test equality.</summary>
	/// <param name="rhs">Rectangle to test against.</param>
	/// <returns>True if rectangles are exactly equal (corners completely coincide), otherwise false.</returns>
	bool operator==(const Rectangle& rhs) const { return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height); }

	/// <summary>Test equality.</summary>
	/// <param name="rhs">Rectangle to test against.</param>
	/// <returns>True if rectangles are not exactly equal (at least one corner different), otherwise false.</returns>
	bool operator!=(const Rectangle& rhs) const { return !(*this == rhs); }

	/// <summary>Expand this rectangle so that it also contains the given rectangle. Equivalently: Set this
	/// rectangle's min corner to the min of this and rect min corner, and the max corner to the max of this
	/// and rect's max corner.</summary>
	/// <param name="rect">The other rectangle to extend.</param>
	void expand(const Rectangle& rect)
	{
		auto minx = glm::min(x, rect.x);
		auto miny = glm::min(y, rect.y);
		auto maxx = glm::max(x + height, rect.x + rect.width);
		auto maxy = glm::max(y + height, rect.y + rect.width);

		x = minx;
		y = miny;
		width = maxx - minx;
		height = maxy - miny;
	}
};

/// <summary>An integer 2D rectangle</summary>
typedef Rectangle<int32_t> Rectanglei;

/// <summary>A floating point 2D rectangle</summary>
typedef Rectangle<float> Rectanglef;
} // namespace pvr
