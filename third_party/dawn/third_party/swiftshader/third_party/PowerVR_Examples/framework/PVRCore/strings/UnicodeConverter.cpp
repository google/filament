/*!
\brief Implementations for the functions from UnicodeConverter.h
\file PVRCore/strings/UnicodeConverter.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PVRCore/strings/UnicodeConverter.h"
using std::vector;

namespace pvr {
namespace utils {

#define VALID_ASCII 0x80
#define TAIL_MASK 0x3F
#define BYTES_PER_TAIL 6

#define UTF16_SURG_H_MARK 0xD800
#define UTF16_SURG_H_END 0xDBFF
#define UTF16_SURG_L_MARK 0xDC00
#define UTF16_SURG_L_END 0xDFFF

#define UNICODE_NONCHAR_MARK 0xFDD0
#define UNICODE_NONCHAR_END 0xFDEF
#define UNICODE_RESERVED 0xFFFE
#define UNICODE_MAX 0x10FFFF

// A table which allows quick lookup to determine the number of bytes of a UTF8 code point.
static const utf8 c_utf8TailLengths[256] = {
	// clang-format off
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0,
	// clang-format on
};

// A table which allows quick lookup to determine whether a UTF8 sequence is 'overlong'.
static const utf32 c_utf32MinimumValues[4] = {
	0x00000000, // 0 tail bytes
	0x00000080, // 1 tail bytes
	0x00000800, // 2 tail bytes
	0x00010000, // 3 tail bytes
};

uint32_t UnicodeConverter::unicodeCount(const utf8* unicodeString)
{
	const utf8* currentCharacter = unicodeString;

	uint32_t characterCount = 0;
	while (*currentCharacter)
	{
		// Quick optimisation for ASCII characters
		while (*currentCharacter && ((*currentCharacter) > 0))
		{
			characterCount++;
			currentCharacter++;
		}

		// Done
#if defined(USE_UTF8)
		if (*currentCharacter)
		{
			utf8 tailMask = *currentCharacter & 0xF0;
			switch (tailMask)
			{
			case 0xF0: currentCharacter++;
			case 0xE0: currentCharacter++;
			case 0xC0: currentCharacter++; break;
			default:
				// Invalid tail char
				return 0;
			}

			currentCharacter++;
			characterCount++;
		}
#endif
	}

	return characterCount;
}

uint32_t UnicodeConverter::unicodeCount(const utf16* unicodeString)
{
	const utf16* currentCharacter = unicodeString;
	uint32_t characterCount = 0;
	while (*currentCharacter)
	{
		if (currentCharacter[0] >= UTF16_SURG_H_MARK && currentCharacter[0] <= UTF16_SURG_H_END && currentCharacter[1] >= UTF16_SURG_L_MARK && currentCharacter[1] <= UTF16_SURG_L_END)
		{ currentCharacter += 2; }
		else
		{
			currentCharacter += 1;
		}

		characterCount++;
	}

	return characterCount;
}

uint32_t UnicodeConverter::unicodeCount(const utf32* unicodeString)
{
	uint32_t characterCount = 0;
	const utf32* currentCharacter = unicodeString;
	while (*currentCharacter != 0)
	{
		++characterCount;
		++currentCharacter;
	}

	return characterCount;
}

void UnicodeConverter::convertAsciiToUnicode(const char* asciiString, vector<utf8>& unicodeString)
{
	uint32_t stringLength = static_cast<uint32_t>(strlen(asciiString));
	if (!isAsciiChar(asciiString)) { throw UnicodeConversionError("Parameter [asciiString] was not actually a valid ASCII string"); }
	// Make sure to include the NULL terminator.
	unicodeString.resize(stringLength + 1);
	for (uint32_t i = 0; i < stringLength; ++i) { unicodeString[i] = static_cast<utf8>(asciiString[i]); }
}

void UnicodeConverter::convertUTF8ToUTF16(const utf8* /*unicodeString*/, vector<utf16>& /*unicodeStringOut*/)
{
	throw InvalidOperationError("UTF8 to UTF16 conversion not implmented");
}

void UnicodeConverter::convertUTF8ToUTF32(const utf8* unicodeString, vector<utf32>& unicodeStringOut)
{
	uint32_t stringLength = static_cast<uint32_t>(strlen(reinterpret_cast<const char*>(unicodeString)));
	const utf8* currentCharacter = unicodeString;
	while (*currentCharacter)
	{
		// Quick optimisation for ASCII characters
		while (*currentCharacter && isAsciiChar((char)*currentCharacter))
		{
			unicodeStringOut.emplace_back(*currentCharacter);
			++currentCharacter;
		}

		// Check that we haven't reached the end.
		if (*currentCharacter != 0)
		{
			// Get the tail length and current character
			utf32 codePoint = *currentCharacter;
			uint32_t tailLength = c_utf8TailLengths[codePoint];

			// Increment the character
			++currentCharacter;

			// Check for invalid tail length. Maximum 4 bytes for each UTF8 character.
			// Also check to make sure the tail length is inside the provided buffer.
			if (tailLength != 0 && (currentCharacter + tailLength <= unicodeString + stringLength))
			{
				// Get the data out of the first char. This depends on the length of the tail.
				codePoint &= (TAIL_MASK >> tailLength);

				// Get the data out of each tail char
				for (uint32_t i = 0; i < tailLength; ++i)
				{
					// Check for invalid tail bytes
					if ((currentCharacter[i] & 0xC0) != 0x80)
					{
						// Invalid tail char.
						throw UnicodeConversionError("Parameter [unicodeString] contained invalid tail chars");
					}

					codePoint = (codePoint << BYTES_PER_TAIL) + (currentCharacter[i] & TAIL_MASK);
				}

				currentCharacter += tailLength;

				// Check overlong values.
				if (codePoint >= c_utf32MinimumValues[tailLength])
				{
					if (isValidCodePoint(codePoint)) { unicodeStringOut.emplace_back(codePoint); }
					else
					{
						throw UnicodeConversionError("Parameter [unicodeString] contained invalid code points");
					}
				}
				else
				{
					throw UnicodeConversionError("Parameter [unicodeString] code point was too long");
				}
			}
			else
			{
				throw UnicodeConversionError("Parameter [unicodeString] contained invalid tail lenght");
			}
		}
	}
}

void UnicodeConverter::convertUTF16ToUTF8(const utf16* /*unicodeString*/, vector<utf8>& /*unicodeStringOut*/)
{
	throw InvalidOperationError("UTF16 to UTF8 conversion not implmented");
}

void UnicodeConverter::convertUTF16ToUTF32(const utf16* unicodeString, vector<utf32>& unicodeStringOut)
{
	const uint16_t* currentCharacter = unicodeString;

	// Determine the number of shorts
	while (*++currentCharacter)
		;
	uint32_t uiBufferLen = static_cast<uint32_t>(currentCharacter - unicodeString);

	// Reset to start.
	currentCharacter = unicodeString;

	while (*currentCharacter)
	{
		// Straight copy. We'll check for surrogate pairs next...
		uint32_t codePoint = *currentCharacter;
		++currentCharacter;

		// Check for a surrogate pair indicator.
		if (codePoint >= UTF16_SURG_H_MARK && codePoint <= UTF16_SURG_H_END)
		{
			// Make sure the next 2 bytes are in range...
			if (currentCharacter + 1 < unicodeString + uiBufferLen && *currentCharacter != 0)
			{
				// Check that the next value is in the low surrogate range.
				if (*currentCharacter >= UTF16_SURG_L_MARK && *currentCharacter <= UTF16_SURG_L_END)
				{
					codePoint = ((codePoint - UTF16_SURG_H_MARK) << 10) + (*currentCharacter - UTF16_SURG_L_MARK) + 0x10000;
					++currentCharacter;
				}
				else
				{
					throw UnicodeConversionError("Parameter [unicodeString] contained a character that was not in the low surrogate range");
				}
			}
			else
			{
				throw UnicodeConversionError("Parameter [unicodeString] contained a character that was out of range");
			}
		}

		// Check that the code point is valid
		if (isValidCodePoint(codePoint)) { unicodeStringOut.emplace_back(codePoint); }
		else
		{
			throw UnicodeConversionError("Parameter [unicodeString] contained an invalid code point");
		}
	}
}

void UnicodeConverter::convertUTF32ToUTF8(const utf32* /*unicodeString*/, vector<utf8>& /*unicodeStringOut*/)
{
	throw InvalidOperationError("UTF32 to UTF8 conversion not implmented");
}

void UnicodeConverter::convertUTF32ToUTF16(const utf32* /*unicodeString*/, vector<utf16>& /*unicodeStringOut*/)
{
	throw InvalidOperationError("UTF32 to UTF16 conversion not implmented");
}

bool UnicodeConverter::isAsciiChar(char asciiChar)
{
	// Make sure that the ascii std::string is limited to encodings with the first 7 bits. Any outside of this range are part of the system's locale.
	if ((asciiChar & VALID_ASCII) != 0) { return false; }

	return true;
}

bool UnicodeConverter::isAsciiChar(const char* asciiString)
{
	uint32_t stringLength = static_cast<uint32_t>(strlen(asciiString));

	for (uint32_t i = 0; i < stringLength; ++i)
	{
		if (!isAsciiChar(asciiString[i])) { return false; }
	}

	return true;
}

bool UnicodeConverter::isValidUnicode(const utf8* unicodeString)
{
	uint32_t stringLength = unicodeCount(unicodeString);
	const utf8* currentCharacter = unicodeString;
	while (*currentCharacter != 0)
	{
		// Quick optimisation for ASCII characters - these are always valid.
		while (*currentCharacter && isAsciiChar(static_cast<char>(*currentCharacter))) { currentCharacter++; }

		// Check that we haven't hit the end with the previous loop.
		if (*currentCharacter != 0)
		{
			// Get the code point
			utf32 codePoint = *currentCharacter;

			// Move to the next character
			++currentCharacter;

			// Get the tail length for this codepoint
			uint32_t tailLength = c_utf8TailLengths[codePoint];

			// Check for invalid tail length, characters - there are only a few possibilities here due to the lookup table.
			// Also check to make sure the tail length is inside the provided buffer.
			if (tailLength == 0 || (currentCharacter + tailLength > unicodeString + stringLength)) { return false; }

			// Get the data out of the first char. This depends on the length of the tail.
			codePoint &= (TAIL_MASK >> tailLength);

			// Get the data out of each tail char, making sure that the tail is valid unicode.
			for (uint32_t i = 0; i < tailLength; ++i)
			{
				// Check for invalid tail bytes
				if ((currentCharacter[i] & 0xC0) != 0x80) { return false; }

				codePoint = (codePoint << BYTES_PER_TAIL) + (currentCharacter[i] & TAIL_MASK);
			}
			currentCharacter += tailLength;

			// Check for 'overlong' values - i.e. values which have a tail but don't actually need it..
			if (codePoint < c_utf32MinimumValues[tailLength]) { return false; }

			// Check that it's a valid code point
			if (!isValidCodePoint(codePoint)) { return false; }
		}
	}

	return true;
}

bool UnicodeConverter::isValidUnicode(const utf16* unicodeString)
{
	const uint16_t* currentCharacter = unicodeString;

	// Determine the number of shorts
	while (*++currentCharacter)
		;
	uint32_t uiBufferLen = static_cast<uint32_t>(currentCharacter - unicodeString);

	// Reset to start.
	currentCharacter = unicodeString;

	while (*currentCharacter)
	{
		// Straight copy. We'll check for surrogate pairs next...
		uint32_t codePoint = *currentCharacter;
		++currentCharacter;

		// Check for a surrogate pair indicator.
		if (codePoint >= UTF16_SURG_H_MARK && codePoint <= UTF16_SURG_H_END)
		{
			// Make sure the next 2 bytes are in range...
			if (currentCharacter + 1 < unicodeString + uiBufferLen && *currentCharacter != 0)
			{
				// Check that the next value is in the low surrogate range.
				if (*currentCharacter >= UTF16_SURG_L_MARK && *currentCharacter <= UTF16_SURG_L_END)
				{
					codePoint = ((codePoint - UTF16_SURG_H_MARK) << 10) + (*currentCharacter - UTF16_SURG_L_MARK) + 0x10000;
					++currentCharacter;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		// Check that the code point is valid
		if (!isValidCodePoint(codePoint)) { return false; }
	}

	return true;
}

bool UnicodeConverter::isValidUnicode(const utf32* unicodeString)
{
	uint32_t stringLength = unicodeCount(unicodeString);

	for (uint32_t i = 0; i < stringLength; ++i)
	{
		if (!isValidCodePoint(unicodeString[i])) { return false; }
	}

	return true;
}

bool UnicodeConverter::isValidCodePoint(utf32 codePoint)
{
	// Check that this value isn't a UTF16 surrogate mask.
	if (codePoint >= UTF16_SURG_H_MARK && codePoint <= UTF16_SURG_L_END) { return false; }

	// Check non-char values
	if (codePoint >= UNICODE_NONCHAR_MARK && codePoint <= UNICODE_NONCHAR_END) { return false; }

	// Check reserved values
	if ((codePoint & UNICODE_RESERVED) == UNICODE_RESERVED) { return false; }

	// Check max value.
	if (codePoint > UNICODE_MAX) { return false; }

	return true;
}
} // namespace utils
} // namespace pvr
//!\endcond
