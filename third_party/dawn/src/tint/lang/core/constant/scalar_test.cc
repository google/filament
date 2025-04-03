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

#include "src/tint/lang/core/constant/scalar.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/type/i32.h"

namespace tint::core::constant {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using ConstantTest_Scalar = testing::Test;

TEST_F(ConstantTest_Scalar, AllZero) {
    Manager constants;
    auto* i0 = constants.Get(0_i);
    auto* iPos1 = constants.Get(1_i);
    auto* iNeg1 = constants.Get(-1_i);

    auto* u0 = constants.Get(0_u);
    auto* u1 = constants.Get(1_u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);
    auto* fNeg1 = constants.Get(-1_f);

    auto* f16Pos0 = constants.Get(0_h);
    auto* f16Neg0 = constants.Get(-0_h);
    auto* f16Pos1 = constants.Get(1_h);
    auto* f16Neg1 = constants.Get(-1_h);

    auto* bf = constants.Get(false);
    auto* bt = constants.Get(true);

    auto* afPos0 = constants.Get(0.0_a);
    auto* afNeg0 = constants.Get(-0.0_a);
    auto* afPos1 = constants.Get(1.0_a);
    auto* afNeg1 = constants.Get(-1.0_a);

    auto* ai0 = constants.Get(0_a);
    auto* aiPos1 = constants.Get(1_a);
    auto* aiNeg1 = constants.Get(-1_a);

    EXPECT_TRUE(i0->AllZero());
    EXPECT_FALSE(iPos1->AllZero());
    EXPECT_FALSE(iNeg1->AllZero());

    EXPECT_TRUE(u0->AllZero());
    EXPECT_FALSE(u1->AllZero());

    EXPECT_TRUE(fPos0->AllZero());
    EXPECT_TRUE(fNeg0->AllZero());
    EXPECT_FALSE(fPos1->AllZero());
    EXPECT_FALSE(fNeg1->AllZero());

    EXPECT_TRUE(f16Pos0->AllZero());
    EXPECT_TRUE(f16Neg0->AllZero());
    EXPECT_FALSE(f16Pos1->AllZero());
    EXPECT_FALSE(f16Neg1->AllZero());

    EXPECT_TRUE(bf->AllZero());
    EXPECT_FALSE(bt->AllZero());

    EXPECT_TRUE(afPos0->AllZero());
    EXPECT_TRUE(afNeg0->AllZero());
    EXPECT_FALSE(afPos1->AllZero());
    EXPECT_FALSE(afNeg1->AllZero());

    EXPECT_TRUE(ai0->AllZero());
    EXPECT_FALSE(aiPos1->AllZero());
    EXPECT_FALSE(aiNeg1->AllZero());
}

TEST_F(ConstantTest_Scalar, AnyZero) {
    Manager constants;
    auto* i0 = constants.Get(0_i);
    auto* iPos1 = constants.Get(1_i);
    auto* iNeg1 = constants.Get(-1_i);

    auto* u0 = constants.Get(0_u);
    auto* u1 = constants.Get(1_u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);
    auto* fNeg1 = constants.Get(-1_f);

    auto* f16Pos0 = constants.Get(0_h);
    auto* f16Neg0 = constants.Get(-0_h);
    auto* f16Pos1 = constants.Get(1_h);
    auto* f16Neg1 = constants.Get(-1_h);

    auto* bf = constants.Get(false);
    auto* bt = constants.Get(true);

    auto* afPos0 = constants.Get(0.0_a);
    auto* afNeg0 = constants.Get(-0.0_a);
    auto* afPos1 = constants.Get(1.0_a);
    auto* afNeg1 = constants.Get(-1.0_a);

    auto* ai0 = constants.Get(0_a);
    auto* aiPos1 = constants.Get(1_a);
    auto* aiNeg1 = constants.Get(-1_a);

    EXPECT_TRUE(i0->AnyZero());
    EXPECT_FALSE(iPos1->AnyZero());
    EXPECT_FALSE(iNeg1->AnyZero());

    EXPECT_TRUE(u0->AnyZero());
    EXPECT_FALSE(u1->AnyZero());

    EXPECT_TRUE(fPos0->AnyZero());
    EXPECT_TRUE(fNeg0->AnyZero());
    EXPECT_FALSE(fPos1->AnyZero());
    EXPECT_FALSE(fNeg1->AnyZero());

    EXPECT_TRUE(f16Pos0->AnyZero());
    EXPECT_TRUE(f16Neg0->AnyZero());
    EXPECT_FALSE(f16Pos1->AnyZero());
    EXPECT_FALSE(f16Neg1->AnyZero());

    EXPECT_TRUE(bf->AnyZero());
    EXPECT_FALSE(bt->AnyZero());

    EXPECT_TRUE(afPos0->AnyZero());
    EXPECT_TRUE(afNeg0->AnyZero());
    EXPECT_FALSE(afPos1->AnyZero());
    EXPECT_FALSE(afNeg1->AnyZero());

    EXPECT_TRUE(ai0->AnyZero());
    EXPECT_FALSE(aiPos1->AnyZero());
    EXPECT_FALSE(aiNeg1->AnyZero());
}

TEST_F(ConstantTest_Scalar, ValueOf) {
    Manager constants;
    auto* i1 = constants.Get(1_i);
    auto* u1 = constants.Get(1_u);
    auto* f1 = constants.Get(1_f);
    auto* f16Pos1 = constants.Get(1_h);
    auto* bf = constants.Get(false);
    auto* bt = constants.Get(true);
    auto* af1 = constants.Get(1.0_a);
    auto* ai1 = constants.Get(1_a);

    EXPECT_EQ(i1->ValueOf(), 1);
    EXPECT_EQ(u1->ValueOf(), 1u);
    EXPECT_EQ(f1->ValueOf(), 1.f);
    EXPECT_EQ(f16Pos1->ValueOf(), 1.f);
    EXPECT_FALSE(bf->ValueOf());
    EXPECT_TRUE(bt->ValueOf());
    EXPECT_EQ(af1->ValueOf(), 1.0);
    EXPECT_EQ(ai1->ValueOf(), 1l);
}

TEST_F(ConstantTest_Scalar, Clone) {
    Manager constants;
    auto* val = constants.Get(12_i);

    Manager mgr;
    CloneContext ctx{core::type::CloneContext{{nullptr}, {nullptr, &mgr.types}}, mgr};

    auto* r = val->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<core::type::I32>());
    EXPECT_EQ(r->value, 12);
}

}  // namespace
}  // namespace tint::core::constant
