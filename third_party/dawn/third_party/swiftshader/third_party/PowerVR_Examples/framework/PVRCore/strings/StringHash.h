/*!
\brief A hashed std::string with functionality for fast compares.
\file PVRCore/strings/StringHash.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/strings/CompileTimeHash.h"
#include "PVRCore/strings/StringFunctions.h"
#include "PVRCore/Errors.h"
#include <functional>

namespace pvr {
/// <summary>Implementation of a hashed std::string with functionality for fast compares.</summary>
/// <remarks>In most cases, can be used as a drop-in replacement for std::strings to take advantage of fast
/// hashed comparisons. On debug builds, tests for hash collisions are performed (Assertion + error log if
/// collision found)</remarks>
class StringHash
{
	typedef hash<std::string> HashFn;

public:
	/// <summary>Constructor. Initialize with c-style std::string, which will be copied. Automatically calculates hash.</summary>
	/// <param name="str">A c-style std::string. Will be copied internaly.</param>
	StringHash(const char* str) : _String(str), _Hash(HashFn()(_String)) {}

	/// <summary>Constructor. Initialize with c++style std::string, which will be copied. Automatically calculates hash.</summary>
	/// <param name="right">The std::string to initialize with.</param>
	StringHash(const std::string& right) : _String(right), _Hash(HashFn()(_String)) {}

	/// <summary>Conversion to std::string reference. No-op.</summary>
	/// <returns>A std::string representatation of this hash</returns>
	operator const std::string&() const { return _String; }

	/// <summary>Default constructor. Empty std::string.</summary>
	StringHash() : _String(""), _Hash(HashFn()(_String)) {}

	/// <summary>Copy Constructor.</summary>
	/// <param name="other">String hash from which to copy.</param>
	StringHash(const StringHash& other)
	{
		_String = other._String;
		_Hash = other._Hash;
	}

	/// <summary>Dtor.</summary>
	~StringHash() {}

	/// <summary>Copy assignment operator.</summary>
	/// <param name="other">String hash from which to copy.</param>
	/// <returns>this object</returns>
	StringHash& operator=(const StringHash other)
	{
		_String = other._String;
		_Hash = other._Hash;
		return *this;
	}

	/// <summary>Appends a std::string to the end of this StringHash, recalculates hash.</summary>
	/// <param name="ptr">A std::string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& append(const char* ptr)
	{
		_String.append(ptr);
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Appends a std::string.</summary>
	/// <param name="str">A std::string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& append(const std::string& str)
	{
		_String.append(str);
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Assigns the std::string to the std::string ptr.</summary>
	/// <param name="ptr">A std::string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& assign(const char* ptr)
	{
		_String.assign(ptr);
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Assigns the std::string to the std::string str.</summary>
	/// <param name="str">A std::string</param>
	/// <returns>this (post the operation)</returns>
	StringHash& assign(const std::string& str)
	{
		_String = str;
		_Hash = HashFn()(_String);
		return *this;
	}

	/// <summary>Return the length of this std::string hash</summary>
	/// <returns>Length of this std::string hash</returns>
	size_t size() const { return _String.size(); }

	/// <summary>Return the length of this std::string hash</summary>
	/// <returns>Length of this std::string hash</returns>
	size_t length() const { return _String.length(); }

	/// <summary>Return if the std::string is empty</summary>
	/// <returns>true if the std::string is emtpy (length=0), false otherwise</returns>
	bool empty() const { return _String.empty(); }

	/// <summary>Clear this std::string hash</summary>
	void clear() { assign(std::string()); }

	/// <summary>== Operator. Compares hash values. Extremely fast.</summary>
	/// <param name="str">A hashed std::string to compare with</param>
	/// <returns>True if the strings have the same hash</returns>
	/// <remarks>On debug builds, this operation does a deep check for hash collisions. You may also define
	/// PVR_STRING_HASH_STRONG_COMPARISONS if you want to force deep checks (in which) case IFF hashes are equal the
	/// the strings will be compared char-per-char as well. Only consider this if you think that for some reason you
	/// have an extremely high probability of hash collisions.</remarks>
	bool operator==(const StringHash& str) const
	{
#ifdef DEBUG // Collision detection
		if (_Hash == str.getHash() && _String != str._String)
		{
			throw InvalidDataError(strings::createFormatted("***** STRING HASH COLLISION DETECTED ********************\n"
															"** String [%s] collides with std::string [%s] \n"
															"*********************************************************",
				_String.c_str(), str._String.c_str()));
		}
#endif

#ifndef PVR_STRING_HASH_STRONG_COMPARISONS
		return (_Hash == str.getHash());
#else
		return (_Hash == str.getHash()) && _String == str._String;
#endif
	}

	/// <summary>Equality Operator. This function performs a strcmp(), so it is orders of magnitude slower than comparing
	/// to another StringHash, but still much faster than creating a temporary StringHash for one comparison.</summary>
	/// <param name="str">A std::string to compare with</param>
	/// <returns>True if they are the same.</returns>
	bool operator==(const char* str) const { return (_String.compare(str) == 0); }

	/// <summary>Equality Operator. This function performs a std::string comparison so it is orders of magnitude slower than
	/// comparing to another StringHash, but still much faster than creating a temporary StringHash for one
	/// comparison.</summary>
	/// <param name="str">A std::string to compare with</param>
	/// <returns>True if they are the same.</returns>
	bool operator==(const std::string& str) const { return _String == str; }

	/// <summary>Inequality Operator. Compares hash values. Extremely fast.</summary>
	/// <param name="str">A StringHash to compare with</param>
	/// <returns>True if they don't match</returns>
	bool operator!=(const StringHash& str) const { return !(*this == str); }

	/// <summary>Less than Operator. Compares hash values, except on debug builds where it checks for collisions. Extremely fast.</summary>
	/// <param name="str">A StringHash to compare with</param>
	/// <returns>True if this should be considered less than str, otherwise false.</returns>
	bool operator<(const StringHash& str) const
	{
#ifdef DEBUG // Collision detection
		if (_Hash == str.getHash() && _String != str._String)
		{ throw InvalidDataError(strings::createFormatted("HASH COLLISION DETECTED with %s and %s", _String.c_str(), str._String.c_str()).c_str()); }
		#endif
		return _Hash < str.getHash() || (_Hash == str.getHash() && _String < str._String);
	}

	/// <summary>Greater-than operator</summary>
	/// <param name="str">Right hand side of the operator (StringHash)</param>
	/// <returns>Return true if left-hand side is greater than right-hand side</returns>
	bool operator>(const StringHash& str) const { return str < *this; }

	/// <summary>Less-than or equal operator</summary>
	/// <param name="str">Right hand side of the operator (StringHash)</param>
	/// <returns>Return true if left-hand side is less than or equal to the right-hand side</returns>
	bool operator<=(const StringHash& str) const { return !(str > *this); }

	/// <summary>Greater-than or equal operator</summary>
	/// <param name="str">Right hand side of the operator (StringHash)</param>
	/// <returns>Return true if left-hand side is greater-than or equal to the right-hand side</returns>
	bool operator>=(const StringHash& str) const { return !(str < *this); }

	/// <summary>Get the base string object used by this StringHash object</summary>
	/// <returns>The base string object contained in this std::string hash.</returns>
	const std::string& str() const { return _String; }

	/// <summary>Get the base string object used by this StringHash object</summary>
	/// <returns>The hash value of this StringHash.</returns>
	std::size_t getHash() const { return _Hash; }

	/// <summary>Get the base string object used by this StringHash object</summary>
	/// <returns>A c-string representation of the contained std::string.</returns>
	const char* c_str() const { return _String.c_str(); }

private:
	std::string _String;
	std::size_t _Hash;
};
} // namespace pvr
