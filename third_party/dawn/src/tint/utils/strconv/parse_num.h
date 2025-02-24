// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_STRCONV_PARSE_NUM_H_
#define SRC_TINT_UTILS_STRCONV_PARSE_NUM_H_

#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/result/result.h"

namespace tint::strconv {

/// Error returned by the number parsing functions
enum class ParseNumberError : uint8_t {
    /// The number was unparsable
    kUnparsable,
    /// The parsed number is not representable by the target datatype
    kResultOutOfRange,
};

/// @param str the string
/// @returns the string @p str parsed as a float
Result<float, ParseNumberError> ParseFloat(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a double
Result<double, ParseNumberError> ParseDouble(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int
Result<int, ParseNumberError> ParseInt(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a unsigned int
Result<unsigned int, ParseNumberError> ParseUint(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int64_t
Result<int64_t, ParseNumberError> ParseInt64(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint64_t
Result<uint64_t, ParseNumberError> ParseUint64(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int32_t
Result<int32_t, ParseNumberError> ParseInt32(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint32_t
Result<uint32_t, ParseNumberError> ParseUint32(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int16_t
Result<int16_t, ParseNumberError> ParseInt16(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint16_t
Result<uint16_t, ParseNumberError> ParseUint16(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a int8_t
Result<int8_t, ParseNumberError> ParseInt8(std::string_view str);

/// @param str the string
/// @returns the string @p str parsed as a uint8_t
Result<uint8_t, ParseNumberError> ParseUint8(std::string_view str);

/// Disables the false-positive unreachable-code compiler warnings
TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

/// @param str the string
/// @returns the string @p str parsed as a the number @p T
template <typename T>
inline Result<T, ParseNumberError> ParseNumber(std::string_view str) {
    if constexpr (std::is_same_v<T, float>) {
        return ParseFloat(str);
    }
    if constexpr (std::is_same_v<T, double>) {
        return ParseDouble(str);
    }
    if constexpr (std::is_same_v<T, int>) {
        return ParseInt(str);
    }
    if constexpr (std::is_same_v<T, unsigned int>) {
        return ParseUint(str);
    }
    if constexpr (std::is_same_v<T, int64_t>) {
        return ParseInt64(str);
    }
    if constexpr (std::is_same_v<T, uint64_t>) {
        return ParseUint64(str);
    }
    if constexpr (std::is_same_v<T, int32_t>) {
        return ParseInt32(str);
    }
    if constexpr (std::is_same_v<T, uint32_t>) {
        return ParseUint32(str);
    }
    if constexpr (std::is_same_v<T, int16_t>) {
        return ParseInt16(str);
    }
    if constexpr (std::is_same_v<T, uint16_t>) {
        return ParseUint16(str);
    }
    if constexpr (std::is_same_v<T, int8_t>) {
        return ParseInt8(str);
    }
    if constexpr (std::is_same_v<T, uint8_t>) {
        return ParseUint8(str);
    }
    return ParseNumberError::kUnparsable;
}

/// Re-enables the unreachable-code compiler warnings
TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);

}  // namespace tint::strconv

#endif  // SRC_TINT_UTILS_STRCONV_PARSE_NUM_H_
