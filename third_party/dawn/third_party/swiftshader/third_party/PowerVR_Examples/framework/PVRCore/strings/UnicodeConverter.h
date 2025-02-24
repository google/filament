/*!
\brief Utility functions for handling Unicode.
\file PVRCore/strings/UnicodeConverter.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Errors.h"
#include <vector>
#include <cstdint>

namespace pvr {
// UTF types
/// <summary>A UTF-8 (unsigned) character. 8-bit unsigned Integer.</summary>
typedef unsigned char utf8;
/// <summary>A UTF-16 (unsigned) character. 16-bit unsigned Integer.</summary>
typedef unsigned short utf16;
/// <summary>A UTF-32 (unsigned) character. 32-bit unsigned Integer.</summary>
typedef unsigned int utf32;

namespace utils {
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when undertaking unicode conversions</summary>
class UnicodeConversionError : public std::runtime_error
{
public:
	/// <summary>Constructor</summary>
	/// <param name="message">The error message to log.</param>
	UnicodeConversionError(std::string message) : std::runtime_error("Unicode Conversion Failed: " + message) {}
};

/// <summary>Use as a namespace (static functions only). Contains functionality to work with Unicode strings:
/// Conversions between ASCII/UTF8/UTF16/UTF32, parse/count characters in Multibyte systems, validity queries.</summary>
class UnicodeConverter
{
public:
	/// <summary>Count the number of characters in a unicode UTF-8 std::string</summary>
	/// <param name="unicodeString">A UTF-8 std::string.</param>
	/// <returns>The number of characters unicodeString represents.</returns>
	static uint32_t unicodeCount(const utf8* unicodeString);

	/// <summary>Count the number of characters in a unicode UTF-16 std::string</summary>
	/// <param name="unicodeString">A UTF-16 std::string.</param>
	/// <returns>The number of characters unicodeString represents.</returns>
	static uint32_t unicodeCount(const utf16* unicodeString);

	/// <summary>Count the number of characters in a unicode UTF-32 std::string</summary>
	/// <param name="unicodeString">A UTF-32 std::string.</param>
	/// <returns>The number of characters unicodeString represents.</returns>
	static uint32_t unicodeCount(const utf32* unicodeString);

	/// <summary>Convert an ASCII std::string to a UTF8 std::string</summary>
	/// <param name="asciiString">An ASCII std::string</param>
	/// <param name="unicodeString">The resulting UTF-8 std::string stored as an std::vector<utf8></param>
	static void convertAsciiToUnicode(const char* asciiString, std::vector<utf8>& unicodeString);

	/// <summary>Convert an ASCII std::string to a UTF-16 std::string</summary>
	/// <param name="asciiString">An ASCII std::string</param>
	/// <param name="unicodeString">The resulting UTF-16 std::string stored as an std::vector<utf16></param>
	static void convertAsciiToUnicode(const char* asciiString, std::vector<utf16>& unicodeString)
	{
		if (!isAsciiChar(asciiString)) { throw UnicodeConversionError("Parameter [asciiString] was not actually a valid ASCII string"); }
		convertUTF8ToUTF16(reinterpret_cast<const utf8*>(asciiString), unicodeString);
	}

	/// <summary>Convert an ASCII std::string to a UTF-32 std::string</summary>
	/// <param name="asciiString">An ASCII std::string</param>
	/// <param name="utf32StringOut">The resulting UTF-32 std::string stored as an std::vector<utf32></param>
	static void convertAsciiToUnicode(const char* asciiString, std::vector<utf32>& utf32StringOut)
	{
		if (!isAsciiChar(asciiString)) { throw UnicodeConversionError("Parameter [asciiString] was not actually a valid ASCII string"); }
		convertUTF8ToUTF32(reinterpret_cast<const utf8*>(asciiString), utf32StringOut);
	}

	/// <summary>Convert a UTF-8 std::string to a UTF-16 std::string</summary>
	/// <param name="utf8String">A UTF-8 std::string</param>
	/// <param name="utf16StringOut">The resulting UTF-16 std::string stored as an std::vector<utf16></param>
	static void convertUTF8ToUTF16(const utf8* utf8String, std::vector<utf16>& utf16StringOut);

	/// <summary>Convert a UTF-8 std::string to a UTF-32 std::string</summary>
	/// <param name="utf8String">A UTF-8 std::string</param>
	/// <param name="utf32StringOut">The resulting UTF-32 std::string stored as an std::vector<utf32></param>
	/// <returns>True for Success or false for error.</returns>
	static void convertUTF8ToUTF32(const utf8* utf8String, std::vector<utf32>& utf32StringOut);

	/// <summary>Convert a UTF-16 std::string to a UTF-8 std::string</summary>
	/// <param name="utf16String">A UTF-16 std::string</param>
	/// <param name="utf8StringOut">The resulting UTF-8 std::string stored as an std::vector<utf8></param>
	/// <returns>True for Success or false for error.</returns>
	static void convertUTF16ToUTF8(const utf16* utf16String, std::vector<utf8>& utf8StringOut);

	/// <summary>Convert a UTF-16 std::string to a UTF-32 std::string</summary>
	/// <param name="utf16String">A UTF-16 std::string</param>
	/// <param name="utf32StringOut">The resulting UTF-32 std::string stored as an std::vector<utf32></param>
	/// <returns>True for Success or false for error.</returns>
	static void convertUTF16ToUTF32(const utf16* utf16String, std::vector<utf32>& utf32StringOut);

	/// <summary>Convert a UTF-32 std::string to a UTF-8 std::string</summary>
	/// <param name="utf32String">A UTF-32 std::string</param>
	/// <param name="utf8StringOut">The resulting UTF-8 std::string stored as an std::vector<utf8></param>
	static void convertUTF32ToUTF8(const utf32* utf32String, std::vector<utf8>& utf8StringOut);

	/// <summary>Convert a UTF-32 std::string to a UTF-16 std::string</summary>
	/// <param name="utf32String">A UTF-32 std::string</param>
	/// <param name="utf16StringOut">The resulting UTF-16 std::string stored as an std::vector<utf16></param>
	static void convertUTF32ToUTF16(const utf32* utf32String, std::vector<utf16>& utf16StringOut);

	/// <summary>Check if a std::string contains only valid UTF-8 characters</summary>
	/// <param name="unicodeString">A UTF-8 std::string</param>
	/// <returns>True if the std::string does not contains any characters that are not valid UTF-8, false otherwise</returns>
	static bool isValidUnicode(const utf8* unicodeString);

	/// <summary>Check if a std::string contains only valid UTF-16 characters</summary>
	/// <param name="unicodeString">A UTF-16 std::string</param>
	/// <returns>True if the std::string does not contains any characters that are not valid UTF-16, false otherwise</returns>
	static bool isValidUnicode(const utf16* unicodeString);

	/// <summary>Check if a std::string contains only valid UTF-32 characters</summary>
	/// <param name="unicodeString">A UTF-32 std::string</param>
	/// <returns>True if the std::string does not contains any characters that are not valid UTF-32, false otherwise</returns>
	static bool isValidUnicode(const utf32* unicodeString);

	/// <summary>Check if a std::string only contains valid Ascii-7 characters</summary>
	/// <param name="asciiString">A c-style std::string</param>
	/// <returns>True if the std::string only contains values in the range 0..127 (low ascii), False otherwise</returns>
	static bool isAsciiChar(const char* asciiString);

	/// <summary>Check if a character is valid Ascii-7</summary>
	/// <param name="asciiChar">A character</param>
	/// <returns>True if the std::string only contains values in the range 0..127 (low ascii), False otherwise</returns>
	static bool isAsciiChar(char asciiChar);

private:
	static bool isValidCodePoint(utf32 codePoint);
};
} // namespace utils
} // namespace pvr
