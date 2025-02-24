/*!
\brief Contains several definitions used throughout the PowerVR Framework.
\file PVRCore/Utils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <cstdint>
#include <array>
#include <cassert>
#include <cstring>
#ifndef PVRCORE_NO_GLM
#include "glm.h"
#endif

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvr {
/// <summary>Contains assorted utility functions (test endianness, unicode conversions etc.)</summary>
namespace utils {

/// <summary>Pack 4 values (red, green, blue, alpha) in the range of 0-255 into a single 32 bit unsigned Integer
/// unsigned.</summary>
/// <param name="r">Red channel (8 bit)</param>
/// <param name="g">Blue channel (8 bit)</param>
/// <param name="b">Red channel (8 bit)</param>
/// <param name="a">Red channel (8 bit)</param>
/// <returns>32 bit RGBA value</returns>
inline uint32_t packRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return (static_cast<uint32_t>(((a) << 24) | ((b) << 16) | ((g) << 8) | (r))); }

/// <summary>Pack 4 values (red, green, blue, alpha) in the range of 0.0-1.0 into a single 32 bit unsigned Integer
/// unsigned.</summary>
/// <param name="r">Red channel (normalized 0.0-1.0)</param>
/// <param name="g">Blue channel (normalized 0.0-1.0)</param>
/// <param name="b">Red channel (normalized 0.0-1.0)</param>
/// <param name="a">Red channel (normalized 0.0-1.0)</param>
/// <returns>32 bit RGBA value</returns>
inline uint32_t packRGBA(float r, float g, float b, float a)
{
	return packRGBA(static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255), static_cast<uint8_t>(a * 255));
}

/// <summary>Take a value's bit representation and reinterpret them as another type. Second template parameter
/// should normally be implicitly declared. First template parameter is mandatory.</summary>
/// <param name="value">The value to reinterpret</param>
/// <typeparam name="Toutput_">Output value type. Must be explicitly defined.</typeparam>
/// <typeparam name="Tinput_">Input value type. Should not need to be explicitly defined, can be inferred.
/// </typeparam>
/// <returns>The reinterpreted value</returns>
template<typename Toutput_, typename Tinput_>
Toutput_ reinterpretBits(const Tinput_& value)
{
	Toutput_ ret = static_cast<Toutput_>(0);
	memcpy(&ret, &value, sizeof(Tinput_));
	return ret;
}

/// <summary>Store the bits of a value in a static array of char.</summary>
/// <param name="value">The value to reinterpret</param>
/// <typeparam name="T1_">Input value type. Should not need to be explicitly defined, can be inferred.</typeparam>
/// <returns>A StaticArray<T1_> with a size exactly equal to the size of T1_ in characters, containing the bit
/// representation of value.</returns>
template<typename T1_>
std::array<T1_, sizeof(T1_)> readBits(const T1_& value)
{
	std::array<char, sizeof(T1_)> retval;
	memcpy(&retval[0], &value, sizeof(T1_));
	return retval;
}

/// <summary>Typed memset. Sets each byte of the destination object to a source value.</summary>
/// <typeparam name="T">Type of target object.</typeparam>
/// <param name="dst">Reference to the object whose bytes we will set to the value</param>
/// <param name="i">The conversion of this object to unsigned char will be the value that is set to each byte.</param>
template<typename T>
inline void memSet(T& dst, int32_t i)
{
	memset(&dst, i, sizeof(T));
}

/// <summary>Typed memcopy. Copies the bits of an object to another object. Although T1 can be different to T2,
/// sizeof(T1) must be equal to sizeof(T2)</summary>
/// <typeparam name="T1">Type of target object.</typeparam>
/// <typeparam name="T2">Type of the source object.</typeparam>
/// <param name="dst">Reference to the destination object</param>
/// <param name="src">Reference to the source object</param>
template<typename T1, typename T2>
inline void memCopy(T1& dst, const T2& src)
{
	assert(sizeof(T1) == sizeof(T2));
	memcpy(&dst, &src, sizeof(T1));
}

/// <summary>Copy from volatile memory (facilitate from volatile
/// variables to nonvolatile)</summary>
/// <param name="dst">Copy destination</param>
/// <param name="src">Copy source</param>
template<typename T1, typename T2>
inline void memCopyFromVolatile(T1& dst, const volatile T2& src)
{
	assert(sizeof(T1) == sizeof(T2));
	memcpy(&dst, &const_cast<const T2&>(src), sizeof(T1));
}

/// <summary>Copy to volatile memory (facilitate from normal
/// variables to volatile)</summary>
/// <param name="dst">Copy destination</param>
/// <param name="src">Copy source</param>
template<typename T1, typename T2>
inline void memCopyToVolatile(volatile T1& dst, const T2& src)
{
	assert(sizeof(T1) == sizeof(T2));
	memcpy(&const_cast<T1&>(dst), &src, sizeof(T1));
}

#ifndef PVRCORE_NO_GLM

/// <summary>Convert the linear rgb color values in to srgb color space.</summary>
/// <param name="lRGB">lRGB</param>
/// <returns>sRGB color value</returns>
inline glm::vec3 convertLRGBtoSRGB(const glm::vec3& lRGB)
{
	return glm::mix(lRGB * 12.92f, glm::pow(lRGB, glm::vec3(0.416f)) * 1.055f - 0.055f, glm::vec3(lRGB.x > 0.00313f, lRGB.y > 0.00313f, lRGB.z > 0.00313f));
}

/// <summary>Convert the linear rgb color values in to srgb color space.
/// The Alpha value get unmodified.</summary>
/// <param name="lRGB">lRGB</param>
/// <returns>sRGB color value</returns>
inline glm::vec4 convertLRGBtoSRGB(const glm::vec4& lRGB) { return glm::vec4(convertLRGBtoSRGB(glm::vec3(lRGB)), lRGB.a); }

#endif
} // namespace utils
} // namespace pvr
