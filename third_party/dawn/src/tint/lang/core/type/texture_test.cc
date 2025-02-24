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

#include "src/tint/lang/core/type/texture.h"

#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::core::type {
namespace {

using TextureTypeDimTest = TestParamHelper<TextureDimension>;

TEST_P(TextureTypeDimTest, DimMustMatch) {
    // Check that the dim() query returns the right dimensionality.
    F32 f32;
    // TextureType is an abstract class, so use concrete class
    // SampledTexture in its stead.
    SampledTexture st(GetParam(), &f32);
    EXPECT_EQ(st.Dim(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(Dimensions,
                         TextureTypeDimTest,
                         ::testing::Values(TextureDimension::k1d,
                                           TextureDimension::k2d,
                                           TextureDimension::k2dArray,
                                           TextureDimension::k3d,
                                           TextureDimension::kCube,
                                           TextureDimension::kCubeArray));

}  // namespace
}  // namespace tint::core::type
