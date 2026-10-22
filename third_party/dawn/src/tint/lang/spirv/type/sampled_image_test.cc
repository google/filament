// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/type/sampled_image.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::spirv::type {
namespace {

TEST(SampledImageTest, Equals) {
    core::type::I32 i32;
    core::type::U32 u32;
    Image img_a{&i32,
                Dim::kD1,
                Depth::kNotDepth,
                Arrayed::kNonArrayed,
                Multisampled::kSingleSampled,
                Sampled::kSamplingCompatible,
                core::TexelFormat::kRgba32Float,
                core::Access::kRead};
    Image img_c{&i32,
                Dim::kD1,
                Depth::kNotDepth,
                Arrayed::kNonArrayed,
                Multisampled::kSingleSampled,
                Sampled::kSamplingCompatible,
                core::TexelFormat::kRgba32Float,
                core::Access::kRead};
    SampledImage a{&img_a};
    SampledImage b{&img_a};
    SampledImage c{&img_c};

    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(SampledImageTest, FriendlyName) {
    core::type::I32 i32;
    Image img_a{&i32,
                Dim::kD1,
                Depth::kNotDepth,
                Arrayed::kNonArrayed,
                Multisampled::kSingleSampled,
                Sampled::kSamplingCompatible,
                core::TexelFormat::kRgba32Float,
                core::Access::kRead};
    SampledImage s{&img_a};
    EXPECT_EQ(s.FriendlyName(),
              "spirv.sampled_image<spirv.image<i32, 1d, not_depth, non_arrayed, single_sampled, "
              "sampling_compatible, rgba32float, read>>");
}

}  // namespace
}  // namespace tint::spirv::type
