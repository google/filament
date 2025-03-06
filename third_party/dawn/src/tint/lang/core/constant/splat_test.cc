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

#include "src/tint/lang/core/constant/helper_test.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/fluent_types.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::constant {
namespace {

using ConstantTest_Splat = TestHelper;

TEST_F(ConstantTest_Splat, AllZero) {
    auto* vec3f = create<core::type::Vector>(create<core::type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* SpfPos0 = constants.Splat(vec3f, fPos0);
    auto* SpfNeg0 = constants.Splat(vec3f, fNeg0);
    auto* SpfPos1 = constants.Splat(vec3f, fPos1);

    EXPECT_TRUE(SpfPos0->AllZero());
    EXPECT_TRUE(SpfNeg0->AllZero());
    EXPECT_FALSE(SpfPos1->AllZero());
}

TEST_F(ConstantTest_Splat, AnyZero) {
    auto* vec3f = create<core::type::Vector>(create<core::type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* SpfPos0 = constants.Splat(vec3f, fPos0);
    auto* SpfNeg0 = constants.Splat(vec3f, fNeg0);
    auto* SpfPos1 = constants.Splat(vec3f, fPos1);

    EXPECT_TRUE(SpfPos0->AnyZero());
    EXPECT_TRUE(SpfNeg0->AnyZero());
    EXPECT_FALSE(SpfPos1->AnyZero());
}

TEST_F(ConstantTest_Splat, Index) {
    auto* vec3f = create<core::type::Vector>(create<core::type::F32>(), 3u);

    auto* f1 = constants.Get(1_f);
    auto* sp = constants.Splat(vec3f, f1);

    ASSERT_NE(sp->Index(0), nullptr);
    ASSERT_NE(sp->Index(1), nullptr);
    ASSERT_NE(sp->Index(2), nullptr);
    EXPECT_EQ(sp->Index(3), nullptr);

    EXPECT_EQ(sp->Index(0)->As<Scalar<f32>>()->ValueOf(), 1.f);
    EXPECT_EQ(sp->Index(1)->As<Scalar<f32>>()->ValueOf(), 1.f);
    EXPECT_EQ(sp->Index(2)->As<Scalar<f32>>()->ValueOf(), 1.f);
}

TEST_F(ConstantTest_Splat, Clone) {
    auto* vec2i = create<core::type::Vector>(create<core::type::I32>(), 2u);
    auto* val = constants.Get(12_i);
    auto* sp = constants.Splat(vec2i, val);

    constant::Manager mgr;
    constant::CloneContext ctx{core::type::CloneContext{{nullptr}, {nullptr, &mgr.types}}, mgr};

    auto* r = sp->Clone(ctx);
    EXPECT_NE(r, sp);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<core::type::Vector>());
    EXPECT_TRUE(r->el->Is<Scalar<i32>>());
    EXPECT_EQ(r->count, 2u);
}

}  // namespace
}  // namespace tint::core::constant
