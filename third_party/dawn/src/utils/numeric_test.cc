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

#include "src/utils/numeric.h"

#include "src/utils/gtest.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

using NumericTest = ::testing::Test;

TEST_F(NumericTest, IsCastAlwaysInRange) {
    static_assert(kIsCastAlwaysInRange<uint32_t, uint64_t>);
    static_assert(!kIsCastAlwaysInRange<int32_t, uint32_t>);
    static_assert(!kIsCastAlwaysInRange<uint32_t, int32_t>);
    static_assert(!kIsCastAlwaysInRange<uint64_t, uint32_t>);
}

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using NumericDeathTest = NumericTest;

// Test for checked cast between various types.
template <typename T32, typename T64>
void CheckedCastTestPair() {
    using Prim32 = UnderlyingType<T32>;
    using Prim64 = UnderlyingType<T64>;

    // No-ops
    checked_cast<T32>(T32{std::numeric_limits<Prim32>::max()});
    checked_cast<T64>(T64{std::numeric_limits<Prim64>::max()});

    // Widening
    checked_cast<T64>(T32{std::numeric_limits<Prim32>::max()});

    // Narrowing
    EXPECT_DEATH_IF_SUPPORTED(
        checked_cast<T32>(T64{Prim64{std::numeric_limits<Prim32>::max()} + 1}), "");
    EXPECT_DEATH_IF_SUPPORTED(
        checked_cast<T32>(T64{Prim64{std::numeric_limits<Prim32>::max()} * 2}), "");
    EXPECT_DEATH_IF_SUPPORTED(checked_cast<T32>(T64{std::numeric_limits<Prim64>::max()}), "");
}
template <typename U32, typename I32, typename U64, typename I64>
void CheckedCastTest() {
    static_assert(std::is_same_v<UnderlyingType<U32>, uint32_t>);
    static_assert(std::is_same_v<UnderlyingType<U64>, uint64_t>);
    static_assert(std::is_same_v<UnderlyingType<I32>, int32_t>);
    static_assert(std::is_same_v<UnderlyingType<I64>, int64_t>);
    CheckedCastTestPair<U32, U64>();
    CheckedCastTestPair<U32, I64>();
    CheckedCastTestPair<I32, U64>();
    CheckedCastTestPair<I32, I64>();
}
TEST_F(NumericDeathTest, CheckedCast) {
    using TypedU32 = TypedInteger<struct TypedU32T, uint32_t>;
    using TypedU64 = TypedInteger<struct TypedU64T, uint64_t>;
    using TypedI32 = TypedInteger<struct TypedI32T, int32_t>;
    using TypedI64 = TypedInteger<struct TypedI64T, int64_t>;
    enum class EnumU32 : uint32_t {};
    enum class EnumU64 : uint64_t {};
    enum class EnumI32 : int32_t {};
    enum class EnumI64 : int64_t {};

    // primitive <-> primitive
    CheckedCastTest<uint32_t, int32_t, uint64_t, int64_t>();
    // primitive <-> TypedInteger
    CheckedCastTest<uint32_t, int32_t, TypedU64, TypedI64>();
    CheckedCastTest<TypedU32, TypedI32, uint64_t, int64_t>();
    // TypedInteger <-> TypedInteger
    CheckedCastTest<TypedU32, TypedI32, TypedU64, TypedI64>();
    // primitive <-> enum class
    CheckedCastTest<uint32_t, int32_t, EnumU64, EnumI64>();
    CheckedCastTest<EnumU32, EnumI32, uint64_t, int64_t>();
    // enum class <-> enum class
    CheckedCastTest<EnumU32, EnumI32, EnumU64, EnumI64>();
    // TypedInteger <-> enum class
    CheckedCastTest<TypedU32, TypedI32, EnumU64, EnumI64>();
    CheckedCastTest<EnumU32, EnumI32, EnumU64, EnumI64>();

    // Basic test that dchecked_cast is checked in debug but not release.
    // Not exhaustive like the cases above because those are very slow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(
        dchecked_cast<uint32_t>(uint64_t{std::numeric_limits<uint32_t>::max()} + 1), "");
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(
        dchecked_cast<int32_t>(uint32_t{std::numeric_limits<int32_t>::max()} + 1), "");
}

}  // anonymous namespace
}  // namespace dawn
