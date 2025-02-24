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

#include "dawn/common/Compiler.h"
#include "dawn/common/Range.h"
#include "dawn/common/TypedInteger.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

// Test that iterating an empty range doesn't iterate at all.
TEST(RangeTests, Empty) {
    for ([[maybe_unused]] auto i : Range(0)) {
        // Silence a -Wunreachable-code-loop-increment
        if ((0)) {
            continue;
        }
        FAIL();
    }
    for ([[maybe_unused]] auto i : Range(0, 0)) {
        // Silence a -Wunreachable-code-loop-increment
        if ((0)) {
            continue;
        }
        FAIL();
    }
}

// Test iterating with just an end.
TEST(RangeTests, JustEnd) {
    int count = 0;
    for (auto i : Range(45)) {
        ASSERT_EQ(count, i);
        count++;
    }
    ASSERT_EQ(count, 45);
}

// Test iterating with a start and an end.
TEST(RangeTests, StartAndEnd) {
    int count = 18;
    for (auto i : Range(18, 45)) {
        ASSERT_EQ(count, i);
        count++;
    }
    ASSERT_EQ(count, 45);
}

// Test iterating with a start and an end.
TEST(RangeTests, StartAndEnd_ITyp) {
    using Int = TypedInteger<struct IntT, int>;

    Int count{18};
    for (auto i : Range(Int{18}, Int{45})) {
        ASSERT_EQ(count, i);
        count++;
    }
    ASSERT_EQ(count, Int{45});
}

}  // anonymous namespace
}  // namespace dawn
