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

#include "src/tint/utils/strconv/parse_num.h"

#include <charconv>

#include "absl/strings/charconv.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint::strconv {

namespace {

template <typename T>
Result<T, ParseNumberError> Parse(std::string_view number) {
    T val = 0;
    if constexpr (std::is_floating_point_v<T>) {
        auto result = absl::from_chars(number.data(), number.data() + number.size(), val);
        if (result.ec == std::errc::result_out_of_range) {
            return ParseNumberError::kResultOutOfRange;
        }
        if (result.ec != std::errc() || result.ptr != number.data() + number.size()) {
            return ParseNumberError::kUnparsable;
        }
    } else {
        auto result = std::from_chars(number.data(), number.data() + number.size(), val);
        if (result.ec == std::errc::result_out_of_range) {
            return ParseNumberError::kResultOutOfRange;
        }
        if (result.ec != std::errc() || result.ptr != number.data() + number.size()) {
            return ParseNumberError::kUnparsable;
        }
    }
    return val;
}

}  // namespace

Result<float, ParseNumberError> ParseFloat(std::string_view str) {
    return Parse<float>(str);
}

Result<double, ParseNumberError> ParseDouble(std::string_view str) {
    return Parse<double>(str);
}

Result<int, ParseNumberError> ParseInt(std::string_view str) {
    return Parse<int>(str);
}

Result<unsigned int, ParseNumberError> ParseUint(std::string_view str) {
    return Parse<unsigned int>(str);
}

Result<int64_t, ParseNumberError> ParseInt64(std::string_view str) {
    return Parse<int64_t>(str);
}

Result<uint64_t, ParseNumberError> ParseUint64(std::string_view str) {
    return Parse<uint64_t>(str);
}

Result<int32_t, ParseNumberError> ParseInt32(std::string_view str) {
    return Parse<int32_t>(str);
}

Result<uint32_t, ParseNumberError> ParseUint32(std::string_view str) {
    return Parse<uint32_t>(str);
}

Result<int16_t, ParseNumberError> ParseInt16(std::string_view str) {
    return Parse<int16_t>(str);
}

Result<uint16_t, ParseNumberError> ParseUint16(std::string_view str) {
    return Parse<uint16_t>(str);
}

Result<int8_t, ParseNumberError> ParseInt8(std::string_view str) {
    return Parse<int8_t>(str);
}

Result<uint8_t, ParseNumberError> ParseUint8(std::string_view str) {
    return Parse<uint8_t>(str);
}

}  // namespace tint::strconv

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
