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

#include "src/tint/lang/wgsl/resolver/eval_test.h"

namespace tint::core::constant::test {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct Case {
    Value input;
    struct Success {
        Value value;
    };
    struct Failure {
        builder::CreatePtrs create_ptrs;
    };
    tint::Result<Success, Failure> expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "input: " << c.input;
    if (c.expected == Success) {
        o << ", expected: " << c.expected.Get().value;
    } else {
        o << ", expected failed bitcast to " << c.expected.Failure().create_ptrs;
    }
    return o;
}

template <typename TO, typename FROM>
Case Success(FROM input, TO expected) {
    return Case{input, Case::Success{expected}};
}

template <typename TO, typename FROM>
Case Failure(FROM input) {
    return Case{input, Case::Failure{builder::CreatePtrsFor<TO>()}};
}

using ConstEvalBitcastTest = ConstEvalTestWithParam<Case>;

TEST_P(ConstEvalBitcastTest, Test) {
    const auto& input = GetParam().input;
    const auto& expected = GetParam().expected;

    // Get the target type CreatePtrs
    builder::CreatePtrs target_create_ptrs;
    if (expected == tint::Success) {
        target_create_ptrs = expected.Get().value.create_ptrs;
    } else {
        target_create_ptrs = expected.Failure().create_ptrs;
    }

    auto target_ty = target_create_ptrs.ast(*this);
    ASSERT_NE(target_ty, nullptr);
    auto* input_val = input.Expr(*this);
    const ast::Expression* expr = Bitcast(Source{{12, 34}}, target_ty, input_val);

    WrapInFunction(expr);

    auto* target_sem_ty = target_create_ptrs.sem(*this);

    if (expected == tint::Success) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        auto* sem = Sem().GetVal(expr);
        ASSERT_NE(sem, nullptr);
        EXPECT_TYPE(sem->Type(), target_sem_ty);
        ASSERT_NE(sem->ConstantValue(), nullptr);
        EXPECT_TYPE(sem->ConstantValue()->Type(), target_sem_ty);

        auto expected_values = expected.Get().value.args;
        auto got_values = ScalarsFrom(sem->ConstantValue());
        EXPECT_EQ(expected_values, got_values);
    } else {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_THAT(r()->error(), testing::StartsWith("12:34 error:"));
        EXPECT_THAT(r()->error(), testing::HasSubstr("cannot be represented as"));
    }
}

const u32 nan_as_u32 = tint::Bitcast<u32>(std::numeric_limits<float>::quiet_NaN());
const i32 nan_as_i32 = tint::Bitcast<i32>(std::numeric_limits<float>::quiet_NaN());
const u32 inf_as_u32 = tint::Bitcast<u32>(std::numeric_limits<float>::infinity());
const i32 inf_as_i32 = tint::Bitcast<i32>(std::numeric_limits<float>::infinity());
const u32 neg_inf_as_u32 = tint::Bitcast<u32>(-std::numeric_limits<float>::infinity());
const i32 neg_inf_as_i32 = tint::Bitcast<i32>(-std::numeric_limits<float>::infinity());
const AInt u32_highest_plus_one = AInt(AInt(u32::kHighestValue).value + 1);
const AInt i32_lowest_minus_one = AInt(AInt(i32::kLowestValue).value - 1);

INSTANTIATE_TEST_SUITE_P(
    Bitcast,
    ConstEvalBitcastTest,
    testing::ValuesIn({
        // Bitcast to same (concrete) type, no change
        Success(Val(0_u), Val(0_u)),                        //
        Success(Val(0_i), Val(0_i)),                        //
        Success(Val(0_f), Val(0_f)),                        //
        Success(Val(123_u), Val(123_u)),                    //
        Success(Val(123_i), Val(123_i)),                    //
        Success(Val(123.456_f), Val(123.456_f)),            //
        Success(Val(u32::Highest()), Val(u32::Highest())),  //
        Success(Val(u32::Lowest()), Val(u32::Lowest())),    //
        Success(Val(i32::Highest()), Val(i32::Highest())),  //
        Success(Val(i32::Lowest()), Val(i32::Lowest())),    //
        Success(Val(f32::Highest()), Val(f32::Highest())),  //
        Success(Val(f32::Lowest()), Val(f32::Lowest())),    //

        // Bitcast to different type
        Success(Val(0_u), Val(0_i)),               //
        Success(Val(0_u), Val(0_f)),               //
        Success(Val(0_i), Val(0_u)),               //
        Success(Val(0_i), Val(0_f)),               //
        Success(Val(0.0_f), Val(0_i)),             //
        Success(Val(0.0_f), Val(0_u)),             //
        Success(Val(1_u), Val(1_i)),               //
        Success(Val(1_u), Val(1.4013e-45_f)),      //
        Success(Val(1_i), Val(1_u)),               //
        Success(Val(1_i), Val(1.4013e-45_f)),      //
        Success(Val(1.0_f), Val(0x3F800000_u)),    //
        Success(Val(1.0_f), Val(0x3F800000_i)),    //
        Success(Val(123_u), Val(123_i)),           //
        Success(Val(123_u), Val(1.7236e-43_f)),    //
        Success(Val(123_i), Val(123_u)),           //
        Success(Val(123_i), Val(1.7236e-43_f)),    //
        Success(Val(123.0_f), Val(0x42F60000_u)),  //
        Success(Val(123.0_f), Val(0x42F60000_i)),  //

        // Abstracts
        Success(Val(0_a), Val(0_i)),                                                  //
        Success(Val(0_a), Val(0_f)),                                                  //
        Success(Val(0_a), Val(0_u)),                                                  //
        Success(Val(0_a), Val(0_f)),                                                  //
        Success(Val(0_a), Val(0_i)),                                                  //
        Success(Val(0_a), Val(0_u)),                                                  //
        Success(Val(1_a), Val(1_i)),                                                  //
        Success(Val(1_a), Val(1.4013e-45_f)),                                         //
        Success(Val(1_a), Val(1_u)),                                                  //
        Success(Val(1_a), Val(1.4013e-45_f)),                                         //
        Success(Val(1.0_a), Val(0x3F800000_u)),                                       //
        Success(Val(1.0_a), Val(0x3F800000_i)),                                       //
        Success(Val(123_a), Val(123_i)),                                              //
        Success(Val(123_a), Val(1.7236e-43_f)),                                       //
        Success(Val(123_a), Val(123_u)),                                              //
        Success(Val(123_a), Val(1.7236e-43_f)),                                       //
        Success(Val(123.0_a), Val(0x42F60000_u)),                                     //
        Success(Val(123.0_a), Val(0x42F60000_i)),                                     //
        Success(Val(AInt(u32::Highest())), Val(u32::Highest())),                      //
        Success(Val(AInt(u32::Lowest())), Val(u32::Lowest())),                        //
        Success(Val(AInt(i32::Highest())), Val(tint::Bitcast<u32>(i32::Highest()))),  //
        Success(Val(AInt(i32::Lowest())), Val(tint::Bitcast<u32>(i32::Lowest()))),    //
        Success(Val(AInt(i32::Highest())), Val(i32::Highest())),                      //
        Success(Val(AInt(i32::Lowest())), Val(i32::Lowest())),                        //

        // u32 <-> i32 sign bit
        Success(Val(0xFFFFFFFF_u), Val(-1_i)),           //
        Success(Val(-1_i), Val(0xFFFFFFFF_u)),           //
        Success(Val(0x80000000_u), Val(i32::Lowest())),  //
        Success(Val(i32::Lowest()), Val(0x80000000_u)),  //

        // Vector tests
        Success(Vec(0_u, 1_u, 123_u), Vec(0_i, 1_i, 123_i)),
        Success(Vec(0.0_f, 1.0_f, 123.0_f), Vec(0_i, 0x3F800000_i, 0x42F60000_i)),
        Success(Vec(0xffffffff_a, -1_a), Vec(0xffffffff_u, 0xffffffff_u)),

        // Unrepresentable
        Failure<f32>(Val(nan_as_u32)),                 //
        Failure<f32>(Val(nan_as_i32)),                 //
        Failure<f32>(Val(inf_as_u32)),                 //
        Failure<f32>(Val(inf_as_i32)),                 //
        Failure<f32>(Val(neg_inf_as_u32)),             //
        Failure<f32>(Val(neg_inf_as_i32)),             //
        Failure<vec2<f32>>(Vec(nan_as_u32, 0_u)),      //
        Failure<vec2<f32>>(Vec(nan_as_i32, 0_i)),      //
        Failure<vec2<f32>>(Vec(inf_as_u32, 0_u)),      //
        Failure<vec2<f32>>(Vec(inf_as_i32, 0_i)),      //
        Failure<vec2<f32>>(Vec(neg_inf_as_u32, 0_u)),  //
        Failure<vec2<f32>>(Vec(neg_inf_as_i32, 0_i)),  //
        Failure<vec2<f32>>(Vec(0_u, nan_as_u32)),      //
        Failure<vec2<f32>>(Vec(0_i, nan_as_i32)),      //
        Failure<vec2<f32>>(Vec(0_u, inf_as_u32)),      //
        Failure<vec2<f32>>(Vec(0_i, inf_as_i32)),      //
        Failure<vec2<f32>>(Vec(0_u, neg_inf_as_u32)),  //
        Failure<vec2<f32>>(Vec(0_i, neg_inf_as_i32)),  //

        // Abstract too large
        Failure<u32>(Val(u32_highest_plus_one)),             //
        Failure<u32>(Val(i32_lowest_minus_one)),             //
        Failure<vec2<u32>>(Vec(0_a, u32_highest_plus_one)),  //
        Failure<vec2<u32>>(Vec(i32_lowest_minus_one, 0_a)),  //
    }));

}  // namespace
}  // namespace tint::core::constant::test
