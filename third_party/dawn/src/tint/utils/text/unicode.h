// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_TEXT_UNICODE_H_
#define SRC_TINT_UTILS_TEXT_UNICODE_H_

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

namespace tint {

/// CodePoint is a unicode code point.
struct CodePoint {
    /// Constructor
    inline CodePoint() = default;

    /// Constructor
    /// @param v the code point value
    inline explicit CodePoint(uint32_t v) : value(v) {}

    /// @returns the code point value
    inline operator uint32_t() const { return value; }

    /// Assignment operator
    /// @param v the new value for the code point
    /// @returns this CodePoint
    inline CodePoint& operator=(uint32_t v) {
        value = v;
        return *this;
    }

    /// @returns true if this CodePoint is in the XID_Start set.
    /// @see https://unicode.org/reports/tr31/
    bool IsXIDStart() const;

    /// @returns true if this CodePoint is in the XID_Continue set.
    /// @see https://unicode.org/reports/tr31/
    bool IsXIDContinue() const;

    /// The code point value
    uint32_t value = 0;
};

namespace utf8 {

/// Returns the length of the utf8 sequence that starts with @p first_code_point.
/// @param first_code_point the first byte of the utf8 sequence
/// @returns the length of the utf8 sequence, or 0 if the sequence is invalid.
uint8_t SequenceLength(uint8_t first_code_point);

/// Decodes the first code point in the utf8 string.
/// @param buffer the buffer containing the utf8 sequence
/// @returns a pair of CodePoint and width in code units (uint8_t).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::span<const uint8_t> buffer);

/// Decodes the first code point in the utf8 string.
/// Note: This cannot be handled via the uint8_t version, because std::byte is not guaranteed to be
///       8-bits wide in the spec.
/// @param buffer the buffer containing the utf8 sequence
/// @returns a pair of CodePoint and width in code units (uint8_t).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::span<const std::byte> buffer);

/// Decodes the first code point in the utf8 string.
/// @param utf8_string the string view that contains the utf8 sequence
/// @returns a pair of CodePoint and width in code units (uint8_t).
///          If the next code point cannot be decoded, then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::string_view utf8_string);

/// Encodes a code point to the utf8 string buffer or queries the number of code units used to
/// encode the code point.
/// @param code_point the code point to encode.
/// @param buffer the utf8 string buffer, or an empty span to query the number of code units
/// that would be written if @p buffer had sufficient space.
/// @returns the number of code units written / would be written (at most 4).
size_t Encode(CodePoint code_point, std::span<uint8_t> buffer);

/// @returns true if all the utf-8 code points in the string are ASCII
/// (code-points 0x00..0x7f).
bool IsASCII(std::string_view);

}  // namespace utf8

namespace utf16 {

/// Decodes the first code point in the utf16 string.
/// @param buffer the buffer containing the utf16 sequence
/// @returns a pair of CodePoint and width in code units (16-bit integers).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::span<const uint16_t> buffer);

/// Decodes the first code point in the utf16 string.
/// @param buffer the buffer containing the utf16 sequence
/// @returns a pair of CodePoint and width in code units (16-bit integers).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::span<const std::byte> buffer);

/// Decodes the first code point in the utf16 string.
/// @param utf16_string the string view that contains the utf16 sequence
/// @returns a pair of CodePoint and width in code units (16-bit integers).
///          If the next code point cannot be decoded then returns [0,0].
std::pair<CodePoint, size_t> Decode(std::string_view utf16_string);

/// Encodes a code point to the utf16 string buffer or queries the number of code units used to
/// encode the code point.
/// @param code_point the code point to encode.
/// @param buffer the utf16 string buffer, or an empty span to query the number of code units
/// that would be written if @p buffer had sufficient space.
/// @returns the number of code units written / would be written (at most 2).
size_t Encode(CodePoint code_point, std::span<uint16_t> buffer);

}  // namespace utf16

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_UNICODE_H_
