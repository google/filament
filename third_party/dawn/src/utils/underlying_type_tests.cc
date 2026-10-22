// Copyright 2026 The Dawn & Tint Authors
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

#include <stdint.h>

#include <cstddef>

#include "src/utils/underlying_type.h"

namespace dawn {
namespace {

enum class SignedEnum : int32_t {
    A,
    B,
    C,
};
enum class UnsignedEnum : uint32_t {
    A,
    B,
    C,
};

// Tests for the HasUnsignedUnderlyingType concept.
static_assert(HasUnsignedUnderlyingType<size_t>);
static_assert(HasUnsignedUnderlyingType<uint8_t>);
static_assert(HasUnsignedUnderlyingType<uint16_t>);
static_assert(HasUnsignedUnderlyingType<uint32_t>);
static_assert(HasUnsignedUnderlyingType<uint64_t>);
static_assert(!HasUnsignedUnderlyingType<int8_t>);
static_assert(!HasUnsignedUnderlyingType<int16_t>);
static_assert(!HasUnsignedUnderlyingType<int32_t>);
static_assert(!HasUnsignedUnderlyingType<int64_t>);

static_assert(!HasUnsignedUnderlyingType<SignedEnum>);
static_assert(HasUnsignedUnderlyingType<UnsignedEnum>);

// Tests for UnderlyingType
static_assert(std::is_same_v<UnderlyingType<size_t>, size_t>);
static_assert(std::is_same_v<UnderlyingType<uint8_t>, uint8_t>);
static_assert(std::is_same_v<UnderlyingType<uint16_t>, uint16_t>);
static_assert(std::is_same_v<UnderlyingType<uint32_t>, uint32_t>);
static_assert(std::is_same_v<UnderlyingType<uint64_t>, uint64_t>);
static_assert(std::is_same_v<UnderlyingType<int8_t>, int8_t>);
static_assert(std::is_same_v<UnderlyingType<int16_t>, int16_t>);
static_assert(std::is_same_v<UnderlyingType<int32_t>, int32_t>);
static_assert(std::is_same_v<UnderlyingType<int64_t>, int64_t>);

static_assert(std::is_same_v<UnderlyingType<SignedEnum>, int32_t>);
static_assert(std::is_same_v<UnderlyingType<UnsignedEnum>, uint32_t>);

}  // anonymous namespace
}  // namespace dawn
