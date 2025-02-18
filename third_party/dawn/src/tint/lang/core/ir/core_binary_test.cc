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

#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_BinaryTest = IRTestHelper;
using IR_BinaryDeathTest = IR_BinaryTest;

TEST_F(IR_BinaryDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            b.Add(nullptr, u32(1), u32(2));
        },
        "internal compiler error");
}

TEST_F(IR_BinaryTest, Result) {
    auto* a = b.Add(mod.Types().i32(), 4_i, 2_i);

    EXPECT_EQ(a->Results().Length(), 1u);
    EXPECT_TRUE(a->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(a, a->Result(0)->Instruction());
}

TEST_F(IR_BinaryTest, CreateAnd) {
    auto* inst = b.And(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);
    ASSERT_NE(inst->Results()[0]->Type(), nullptr);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateOr) {
    auto* inst = b.Or(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kOr);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateXor) {
    auto* inst = b.Xor(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kXor);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateEqual) {
    auto* inst = b.Equal(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateNotEqual) {
    auto* inst = b.NotEqual(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kNotEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateLessThan) {
    auto* inst = b.LessThan(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kLessThan);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateGreaterThan) {
    auto* inst = b.GreaterThan(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kGreaterThan);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateLessThanEqual) {
    auto* inst = b.LessThanEqual(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kLessThanEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateGreaterThanEqual) {
    auto* inst = b.GreaterThanEqual(mod.Types().bool_(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kGreaterThanEqual);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateShiftLeft) {
    auto* inst = b.ShiftLeft(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kShiftLeft);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateShiftRight) {
    auto* inst = b.ShiftRight(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kShiftRight);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateAdd) {
    auto* inst = b.Add(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kAdd);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateSubtract) {
    auto* inst = b.Subtract(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kSubtract);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateMultiply) {
    auto* inst = b.Multiply(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kMultiply);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateDivide) {
    auto* inst = b.Divide(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kDivide);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateModulo) {
    auto* inst = b.Modulo(mod.Types().i32(), 4_i, 2_i);

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kModulo);

    ASSERT_TRUE(inst->LHS()->Is<Constant>());
    auto lhs = inst->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, Binary_Usage) {
    auto* inst = b.And(mod.Types().i32(), 4_i, 2_i);

    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);

    ASSERT_NE(inst->LHS(), nullptr);
    EXPECT_THAT(inst->LHS()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->RHS(), nullptr);
    EXPECT_THAT(inst->RHS()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Binary_Usage_DuplicateValue) {
    auto val = 4_i;
    auto* inst = b.And(mod.Types().i32(), val, val);

    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);
    ASSERT_EQ(inst->LHS(), inst->RHS());

    ASSERT_NE(inst->LHS(), nullptr);
    EXPECT_THAT(inst->LHS()->UsagesUnsorted(),
                testing::UnorderedElementsAre(Usage{inst, 0u}, Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Binary_Usage_SetOperand) {
    auto* rhs_a = b.Constant(2_i);
    auto* rhs_b = b.Constant(3_i);
    auto* inst = b.And(mod.Types().i32(), 4_i, rhs_a);

    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);

    EXPECT_THAT(rhs_a->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
    EXPECT_THAT(rhs_b->UsagesUnsorted(), testing::UnorderedElementsAre());
    inst->SetOperand(1, rhs_b);
    EXPECT_THAT(rhs_a->UsagesUnsorted(), testing::UnorderedElementsAre());
    EXPECT_THAT(rhs_b->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Clone) {
    auto* lhs = b.Constant(2_i);
    auto* rhs = b.Constant(4_i);
    auto* inst = b.And(mod.Types().i32(), lhs, rhs);

    auto* c = clone_ctx.Clone(inst);

    EXPECT_NE(inst, c);

    EXPECT_EQ(mod.Types().i32(), c->Result(0)->Type());
    EXPECT_EQ(BinaryOp::kAnd, c->Op());

    auto new_lhs = c->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(new_lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, new_lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    auto new_rhs = c->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(new_rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(4_i, new_rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

}  // namespace
}  // namespace tint::core::ir
