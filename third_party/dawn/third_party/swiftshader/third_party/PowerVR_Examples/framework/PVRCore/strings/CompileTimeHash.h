/*!
\brief Hash functions inmplementations.
\file PVRCore/strings/CompileTimeHash.h
\author PowerVR by Imagination, Developer Technology Team.
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Utils.h"
#include <string>
#include <cwchar>
#pragma warning(push)
#pragma warning(disable : 4307)

namespace pvr {
/// <summary>Function object hashing to 32 bit values into a 32 bit unsigned Integer.</summary>
/// <param name="t">The value to hash.</param>
/// <typeparam name="T1_">The type of the value to hash.</typeparam>
/// <returns>The hash of the value.</returns>
template<typename T1_>
inline uint32_t hash32_32(const T1_& t)
{
	uint32_t a = utils::reinterpretBits<uint32_t>(t);
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}

/// <summary>Function object hashing a number of bytes into a 32 bit unsigned Integer.</summary>
/// <param name="bytes">Pointer to a block of memory.</param>
/// <param name="count">Number of bytes to hash.</param>
/// <returns>The hash of the value.</returns>
inline uint32_t hash32_bytes(const void* bytes, size_t count)
{
	/////////////// WARNING // WARNING // WARNING // WARNING // WARNING // WARNING // /////////////
	// IF THIS ALGORITHM IS CHANGED, THE ALGORITHM IN THE BOTTOM OF THE PAGE MUST BE CHANGED
	// AS IT IS AN INDEPENDENT COMPILE TIME IMPLEMENTATION OF TTHIS ALGORITHM.
	/////////////// WARNING // WARNING // WARNING // WARNING // WARNING // WARNING // /////////////

	uint32_t hashValue = 2166136261U;
	const unsigned char* current = static_cast<const unsigned char*>(bytes);
	const unsigned char* end = current + count;
	while (current < end)
	{
		hashValue = (hashValue * 16777619U) ^ *current;
		++current;
	}
	return hashValue;
}

/// <summary>Class template denoting a hash. Specializations only - no default implementation.
/// (int32_t/int64_t/uint32_t/uint64_t/string)</summary>
/// <typeparam name="T">type of the value to hash.</typeparam>
template<typename T>
struct hash
{};

/// <summary>Template specialization of hash specifically for uint32_t</summary>
template<>
struct hash<uint32_t>
{
	/// <summary>Hashes the given uint32_t.</summary>
	/// <param name="value">A uint32_t value to hash</param>
	/// <returns>The hash value</returns>
	uint32_t operator()(uint32_t value) { return hash32_32(value); }
};

/// <summary>Template specialization of hash specifically for int32_t</summary>
template<>
struct hash<int32_t>
{
	/// <summary>Hashes the given int32_t.</summary>
	/// <param name="value">A int32_t value to hash</param>
	/// <returns>The hash value</returns>
	uint32_t operator()(int32_t value) { return hash32_32(value); }
};

/// <summary>Template specialization of hash specifically for uint64_t</summary>
template<>
struct hash<uint64_t>
{
	/// <summary>Hashes the given uint64_t.</summary>
	/// <param name="value">A uint64_t value to hash</param>
	/// <returns>The hash value</returns>
	uint32_t operator()(uint64_t value)
	{
		uint32_t a = static_cast<uint32_t>((value >> 32) | (value & 0x00000000FFFFFFFFull));
		return hash32_32(a);
	}
};

/// <summary>Template specialization of hash specifically for int64_t</summary>
template<>
struct hash<int64_t>
{
	/// <summary>Hashes the given int64_t.</summary>
	/// <param name="value">A int64_t value to hash</param>
	/// <returns>The hash value</returns>
	uint32_t operator()(int64_t value)
	{
		uint32_t a = static_cast<uint32_t>((value >> 32) | (value & 0x00000000FFFFFFFFull));
		return hash32_32(a);
	}
};

/// <summary>Template specialization of hash specifically for an std::basic_string</summary>
template<typename T>
struct hash<std::basic_string<T>>
{
	/// <summary>Hashes the given string.</summary>
	/// <param name="t">A string value to hash</param>
	/// <returns>The hash value</returns>
	uint32_t operator()(const std::string& t) const { return hash32_bytes(t.data(), sizeof(T) * t.size()); }
};

// Compile time hashing - warning this must give the same results as the hash 32 bytes alogithm otherwise classes around the framework may break. This is used to optimise compiler time switch statements
/// <summary>A dummy helper.</summary>
template<uint32_t hashvalue, unsigned char... dummy>
class hasher_helper;

/// <summary>Template specialization for uint32_t values.</summary>
template<uint32_t hashvalue, unsigned char first>
class hasher_helper<hashvalue, first>
{
public:
	static const uint32_t value = hashvalue * 16777619U ^ first; //!< A hashed value
};

/// <summary>Template specialization for uint32_t values.</summary>
template<uint32_t hashvalue, unsigned char first, unsigned char... dummy>
class hasher_helper<hashvalue, first, dummy...>
{
public:
	static const uint32_t value = hasher_helper<hashvalue * 16777619U ^ first, dummy...>::value; //!< A hashed value
};

/// <summary>Template specialization for unsigned char values.</summary>
template<unsigned char... chars>
class HashCompileTime
{
public:
	static const uint32_t value = hasher_helper<2166136261U, chars...>::value; //!< A hashed value
};
} // namespace pvr
#pragma warning(pop)
