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

#include "src/tint/lang/core/constant/splat.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::core::constant {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using ConstantTest_Value = testing::Test;

TEST_F(ConstantTest_Value, Equal_Scalar_Scalar) {
    Manager constants;
    EXPECT_TRUE(constants.Get(10_i)->Equal(constants.Get(10_i)));
    EXPECT_FALSE(constants.Get(10_i)->Equal(constants.Get(20_i)));
    EXPECT_FALSE(constants.Get(20_i)->Equal(constants.Get(10_i)));

    EXPECT_TRUE(constants.Get(10_u)->Equal(constants.Get(10_u)));
    EXPECT_FALSE(constants.Get(10_u)->Equal(constants.Get(20_u)));
    EXPECT_FALSE(constants.Get(20_u)->Equal(constants.Get(10_u)));

    EXPECT_TRUE(constants.Get(10_f)->Equal(constants.Get(10_f)));
    EXPECT_FALSE(constants.Get(10_f)->Equal(constants.Get(20_f)));
    EXPECT_FALSE(constants.Get(20_f)->Equal(constants.Get(10_f)));
}

TEST_F(ConstantTest_Value, Equal_Splat_Splat) {
    Manager constants;
    auto* vec3f = constants.types.vec(constants.types.f32(), 3u);

    auto* vec3f_1_1_1 = constants.Splat(vec3f, constants.Get(1_f));
    auto* vec3f_2_2_2 = constants.Splat(vec3f, constants.Get(2_f));

    EXPECT_TRUE(vec3f_1_1_1->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_2_2_2->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_1_1_1->Equal(vec3f_2_2_2));
}

TEST_F(ConstantTest_Value, Equal_Composite_Composite) {
    Manager constants;
    auto* vec3f = constants.types.vec(constants.types.f32(), 3u);

    auto* vec3f_1_1_2 = constants.Composite(
        vec3f, Vector{constants.Get(1_f), constants.Get(1_f), constants.Get(2_f)});
    auto* vec3f_1_2_1 = constants.Composite(
        vec3f, Vector{constants.Get(1_f), constants.Get(2_f), constants.Get(1_f)});

    EXPECT_TRUE(vec3f_1_1_2->Equal(vec3f_1_1_2));
    EXPECT_FALSE(vec3f_1_2_1->Equal(vec3f_1_1_2));
    EXPECT_FALSE(vec3f_1_1_2->Equal(vec3f_1_2_1));
}

TEST_F(ConstantTest_Value, Equal_Splat_Composite) {
    Manager constants;
    auto* vec3f = constants.types.vec(constants.types.f32(), 3u);

    auto* vec3f_1_1_1 = constants.Splat(vec3f, constants.Get(1_f));
    auto* vec3f_1_2_1 = constants.Composite(
        vec3f, Vector{constants.Get(1_f), constants.Get(2_f), constants.Get(1_f)});

    EXPECT_TRUE(vec3f_1_1_1->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_1_2_1->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_1_1_1->Equal(vec3f_1_2_1));
}

}  // namespace
}  // namespace tint::core::constant
