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

#include "src/tint/lang/core/constant/composite.h"

#include "src/tint/lang/core/constant/helper_test.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/fluent_types.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::constant {
namespace {

using ConstantTest_Composite = TestHelper;

TEST_F(ConstantTest_Composite, AllZero) {
    auto* vec3f = create<core::type::Vector>(create<core::type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);
    auto* fNeg1 = constants.Get(-1_f);

    auto* compositePosZeros = constants.Composite(vec3f, Vector{fPos0, fPos0, fPos0});
    auto* compositeNegZeros = constants.Composite(vec3f, Vector{fNeg0, fNeg0, fNeg0});
    auto* compositeMixed = constants.Composite(vec3f, Vector{fNeg0, fPos1, fPos0});
    auto* compositePosNeg = constants.Composite(vec3f, Vector{fNeg1, fPos1, fPos1});

    EXPECT_TRUE(compositePosZeros->AllZero());
    EXPECT_TRUE(compositeNegZeros->AllZero());
    EXPECT_FALSE(compositeMixed->AllZero());
    EXPECT_FALSE(compositePosNeg->AllZero());
}

TEST_F(ConstantTest_Composite, AnyZero) {
    auto* vec3f = create<core::type::Vector>(create<core::type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);
    auto* fNeg1 = constants.Get(-1_f);

    auto* compositePosZeros = constants.Composite(vec3f, Vector{fPos0, fPos0, fPos0});
    auto* compositeNegZeros = constants.Composite(vec3f, Vector{fNeg0, fNeg0, fNeg0});
    auto* compositeMixed = constants.Composite(vec3f, Vector{fNeg0, fPos1, fPos0});
    auto* compositePosNeg = constants.Composite(vec3f, Vector{fNeg1, fPos1, fPos1});

    EXPECT_TRUE(compositePosZeros->AnyZero());
    EXPECT_TRUE(compositeNegZeros->AnyZero());
    EXPECT_TRUE(compositeMixed->AnyZero());
    EXPECT_FALSE(compositePosNeg->AllZero());
}

TEST_F(ConstantTest_Composite, Index) {
    auto* vec2f = create<core::type::Vector>(create<core::type::F32>(), 2u);

    auto* fPos0 = constants.Get(0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* composite = constants.Composite(vec2f, Vector{fPos1, fPos0});

    ASSERT_NE(composite->Index(0), nullptr);
    ASSERT_NE(composite->Index(1), nullptr);
    ASSERT_EQ(composite->Index(2), nullptr);

    EXPECT_TRUE(composite->Index(0)->Is<Scalar<f32>>());
    EXPECT_EQ(composite->Index(0)->As<Scalar<f32>>()->ValueOf(), 1.0);
    EXPECT_TRUE(composite->Index(1)->Is<Scalar<f32>>());
    EXPECT_EQ(composite->Index(1)->As<Scalar<f32>>()->ValueOf(), 0.0);
}

TEST_F(ConstantTest_Composite, Clone) {
    auto* vec2f = create<core::type::Vector>(create<core::type::F32>(), 2u);

    auto* fPos0 = constants.Get(0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* composite = constants.Composite(vec2f, Vector{fPos1, fPos0});

    constant::Manager mgr;
    constant::CloneContext ctx{core::type::CloneContext{{nullptr}, {nullptr, &mgr.types}}, mgr};

    auto* r = composite->As<Composite>()->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<core::type::Vector>());
    EXPECT_FALSE(r->all_zero);
    EXPECT_TRUE(r->any_zero);
    ASSERT_EQ(r->elements.Length(), 2u);
}

}  // namespace
}  // namespace tint::core::constant
