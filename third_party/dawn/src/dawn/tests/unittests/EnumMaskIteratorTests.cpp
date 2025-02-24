// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/EnumMaskIterator.h"

#include "gtest/gtest.h"

namespace dawn::native {

enum class TestAspect : uint8_t {
    Color = 1,
    Depth = 2,
    Stencil = 4,
};

template <>
struct EnumBitmaskSize<TestAspect> {
    static constexpr unsigned value = 3;
};

}  // namespace dawn::native

template <>
struct wgpu::IsWGPUBitmask<dawn::native::TestAspect> {
    static constexpr bool enable = true;
};

namespace dawn::native {

static_assert(EnumBitmaskSize<TestAspect>::value == 3);

TEST(EnumMaskIteratorTests, None) {
    for ([[maybe_unused]] TestAspect aspect : IterateEnumMask(static_cast<TestAspect>(0))) {
        // Silence a -Wunreachable-code-loop-increment
        if ((0)) {
            continue;
        }
        FAIL();
    }
}

TEST(EnumMaskIteratorTests, All) {
    TestAspect expected[] = {TestAspect::Color, TestAspect::Depth, TestAspect::Stencil};
    uint32_t i = 0;
    TestAspect aspects = TestAspect::Color | TestAspect::Depth | TestAspect::Stencil;
    for (TestAspect aspect : IterateEnumMask(aspects)) {
        EXPECT_EQ(aspect, expected[i++]);
    }
}

TEST(EnumMaskIteratorTests, Partial) {
    TestAspect expected[] = {TestAspect::Color, TestAspect::Stencil};
    uint32_t i = 0;
    TestAspect aspects = TestAspect::Stencil | TestAspect::Color;
    for (TestAspect aspect : IterateEnumMask(aspects)) {
        EXPECT_EQ(aspect, expected[i++]);
    }
}

}  // namespace dawn::native
