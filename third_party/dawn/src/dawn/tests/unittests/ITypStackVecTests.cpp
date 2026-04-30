// Copyright 2025 The Dawn & Tint Authors
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

#include <utility>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_stack_vec.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class ITypStackVecTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, uint32_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;

    using StackVec = ityp::stack_vec<Key, Val, 10>;
};

// Test creation and initialization of the stack_vec.
TEST_F(ITypStackVecTest, Creation) {
    // Default constructor initializes to 0
    {
        StackVec vec;
        ASSERT_EQ(vec.size(), Key(0));
    }

    // Size constructor initializes contents to 0
    {
        StackVec vec(Key(10));
        ASSERT_EQ(vec.size(), Key(10));

        for (Key i(0); i < Key(10); ++i) {
            ASSERT_EQ(vec[i], Val(0));
        }
    }
}

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using ITypStackVecDeathTest = ITypStackVecTest;

// Out of bounds accesses should crash even in release (the underlying container
// should have asserts enabled).
TEST_F(ITypStackVecDeathTest, OutOfBounds) {
    // MSVC doesn't have asserts (without _MSVC_STL_HARDENING).
    if constexpr (DAWN_COMPILER_IS(MSVC)) {
        GTEST_SKIP();
    }

    StackVec vec(Key(10));
    EXPECT_DEATH(vec[Key(10)], "");

    const StackVec& constVec = vec;
    EXPECT_DEATH(constVec[Key(10)], "");
}

}  // anonymous namespace
}  // namespace dawn
