// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Discard_TooManyOperands) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* d = b.Discard();
        d->SetOperands(Vector{b.Value(0_i)});
        b.Return(func);
    });

    auto* ep = FragmentEntryPoint("ep");
    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: discard: expected exactly 0 operands, got 1
    discard
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Discard_TooManyResults) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* d = b.Discard();
        d->SetResults(Vector{b.InstructionResult(ty.i32())});
        b.Return(func);
    });

    auto* ep = FragmentEntryPoint("ep");
    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: discard: expected exactly 0 results, got 1
    discard
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Discard_RootBlock) {
    mod.root_block->Append(b.Discard());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: discard: root block: invalid instruction: tint::core::ir::Discard
  discard
  ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Discard_NotInFragmentViaFunction) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Discard();
        b.Return(func);
    });

    auto* ep = ComputeEntryPoint("ep");

    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:5 error: discard: cannot be called in non-fragment entry point
    discard
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Discard_NotInFragmentViaEntryPoint) {
    auto* ep = ComputeEntryPoint("ep");

    b.Append(ep->Block(), [&] {
        b.Discard();
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:5 error: discard: cannot be called in non-fragment entry point
    discard
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Terminate_RootBlock) {
    auto f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    mod.root_block->Append(b.TerminateInvocation());
    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: terminate_invocation: root block: invalid instruction: tint::core::ir::TerminateInvocation
  terminate_invocation
  ^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Terminate_MissingResult) {
    auto* terminate_func = b.Function("terminate_func", ty.void_());
    b.Append(terminate_func->Block(), [&] {
        auto* r = b.TerminateInvocation();
        r->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:5 error: terminate_invocation: expected exactly 0 results, got 1
    terminate_invocation
    ^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_TerminatorInMiddle) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Return(f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: must be the last instruction in the block
    ret
    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, If_RootBlock) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.Unreachable());
    mod.root_block->Append(if_);

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:2:3 error: if: root block: invalid instruction: tint::core::ir::If
  if true [t: $B2] {  # if_1
  ^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, If_EmptyFalse) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, If_EmptyTrue) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:4:7 error: block does not end in a terminator instruction
      $B2: {  # true
      ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, If_ConditionIsBool) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(1_i);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:8 error: if: condition type must be 'bool'
    if 1i [t: $B2, f: $B3] {  # if_1
       ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, If_ConditionIsNullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(nullptr);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:8 error: if: operand is undefined
    if undef [t: $B2, f: $B3] {  # if_1
       ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, If_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    if_->SetResults(Vector<InstructionResult*, 1>{nullptr});

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: if: result is undefined
    undef = if true [t: $B2, f: $B3] {  # if_1
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Loop_RootBlock) {
    auto* l = b.Loop();
    l->Body()->Append(b.ExitLoop(l));
    mod.root_block->Append(l);

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: loop: root block: invalid instruction: tint::core::ir::Loop
  loop [b: $B2] {  # loop_1
  ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Loop_OnlyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto* l = b.Loop();
    l->Body()->Append(b.ExitLoop(l));

    auto sb = b.Append(f->Block());
    sb.Append(l);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Loop_EmptyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(b.Loop());
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:4:7 error: block does not end in a terminator instruction
      $B2: {  # body
      ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Loop_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();
    loop->Body()->Append(b.Return(f));

    loop->SetResults(Vector<InstructionResult*, 1>{nullptr});

    f->Block()->Append(loop);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: loop: result is undefined
    undef = loop [b: $B2] {  # loop_1
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Loop_TooManyOperands) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetOperands(Vector{b.Value(42_i)});
        b.Append(loop->Body(), [&] {  //
            b.Return(f);
        });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: loop: expected exactly 0 operands, got 1
    loop [b: $B2] {  # loop_1
    ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Switch_RootBlock) {
    auto* switch_ = b.Switch(1_i);
    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_));
    mod.root_block->Append(switch_);

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: switch: root block: invalid instruction: tint::core::ir::Switch
  switch 1i [c: (default, $B2)] {  # switch_1
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_NullIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(mod.CreateInstruction<ExitIf>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:9 error: exit_if: has no parent control instruction
        exit_if  # undef
        ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_LessOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:9 error: exit_if: provides 1 value but 'if' expects 2 values
        exit_if 1i  # if_1
        ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_MoreOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f, 3_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:9 error: exit_if: provides 3 values but 'if' expects 2 values
        exit_if 1i, 2.0f, 3i  # if_1
        ^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_WithResult) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_IncorrectResultType) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:21 error: exit_if: operand with type 'i32' does not match 'if' target type 'f32'
        exit_if 1i, 2i  # if_1
                    ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_NotInParentIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.ExitIf(if_);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:5 error: exit_if: found outside all control instructions
    exit_if  # if_1
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpsOverIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_inner = b.If(true);

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(if_inner);
        b.ExitIf(if_outer);
    });

    b.Append(if_inner->True(), [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpOverSwitch) {
    auto* f = b.Function("my_func", ty.void_());

    auto* switch_inner = b.Switch(1_i);

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(switch_inner);
        b.ExitIf(if_outer);
    });

    auto* c = b.Case(switch_inner, {b.Constant(1_i), nullptr});
    b.Append(c, [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpOverLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(loop);
        b.ExitIf(if_outer);
    });

    b.Append(loop->Body(), [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_NullSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    def->Append(mod.CreateInstruction<ExitSwitch>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:9 error: exit_switch: has no parent control instruction
        exit_switch  # undef
        ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_LessOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(1_i);

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:5:9 error: exit_switch: provides 1 value but 'switch' expects 2 values
        exit_switch 1i  # switch_1
        ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_MoreOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(1_i);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:5:9 error: exit_switch: provides 3 values but 'switch' expects 2 values
        exit_switch 1i, 2.0f, 3i  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_WithResult) {
    auto* switch_ = b.Switch(1_i);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_IncorrectResultType) {
    auto* switch_ = b.Switch(1_i);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:25 error: exit_switch: operand with type 'i32' does not match 'switch' target type 'f32'
        exit_switch 1i, 2i  # switch_1
                        ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_NotInParentSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.DefaultCase(switch_);
    def->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(switch_);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitSwitch(switch_); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:10:9 error: exit_switch: switch not found in parent control instructions
        exit_switch  # switch_1
        ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_JumpsOverIfs) {
    // switch(true) {
    //   default: {
    //     if (true) {
    //      if (false) {
    //         break;
    //       }
    //     }
    //     break;
    //  }
    auto* switch_ = b.Switch(1_i);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.Append(inner_if_->True(), [&] { b.ExitSwitch(switch_); });
            b.Return(f);
        });
        b.ExitSwitch(switch_);
    });

    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* inner = b.Switch(0_i);
        b.ExitSwitch(switch_);

        auto* inner_def = b.DefaultCase(inner);
        b.Append(inner_def, [&] { b.ExitSwitch(switch_); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: in block
          $B3: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch 0i [c: (default, $B3)] {  # switch_2
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverLoop) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitSwitch(switch_); });
        b.ExitSwitch(switch_);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: in block
          $B3: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B3] {  # loop_1
        ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_OutsideOfLoop) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Continue(loop);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:5 error: continue: called outside of associated loop
    continue  # -> $B3
    ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_InLoopInit) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.Continue(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:9 error: continue: must only be called from loop body
        continue  # -> $B4
        ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_InLoopBody) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_InLoopContinuing) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] { b.Continue(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:9 error: continue: must only be called from loop body
        continue  # -> $B3
        ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_UnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_f); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:5:9 error: continue: provides 2 values but 'loop' block $B3 expects 0 values
        continue 1i, 2.0f  # -> $B3
        ^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_MissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:5:9 error: continue: provides 0 values but 'loop' block $B3 expects 2 values
        continue  # -> $B3
        ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_MismatchedInt) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_i, 3_u, false); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:22 error: continue: operand with type 'i32' does not match 'loop' block $B3 target type 'f32'
        continue 1i, 2i, 3u, false  # -> $B3
                     ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_MismatchedFloat) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_f, 3_f, false); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:28 error: continue: operand with type 'f32' does not match 'loop' block $B3 target type 'u32'
        continue 1i, 2.0f, 3.0f, false  # -> $B3
                           ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Continue_MatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_f, 3_u, false); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, NextIteration_OutsideOfLoop) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:5 error: next_iteration: called outside of associated loop
    next_iteration  # -> $B2
    ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_InLoopInit) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_InLoopBody) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.NextIteration(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:9 error: next_iteration: must only be called from loop initializer or continuing
        next_iteration  # -> $B2
        ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_InLoopContinuing) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] { b.NextIteration(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_UnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_f); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:9 error: next_iteration: provides 2 values but 'loop' block $B3 expects 0 values
        next_iteration 1i, 2.0f  # -> $B3
        ^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_MissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:9 error: next_iteration: provides 0 values but 'loop' block $B3 expects 2 values
        next_iteration  # -> $B3
        ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_MismatchedInt) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_i, 3_u, false); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:28 error: next_iteration: operand with type 'i32' does not match 'loop' block $B3 target type 'f32'
        next_iteration 1i, 2i, 3u, false  # -> $B3
                           ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_MismatchedFloat) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_f, 3_f, false); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:34 error: next_iteration: operand with type 'f32' does not match 'loop' block $B3 target type 'u32'
        next_iteration 1i, 2.0f, 3.0f, false  # -> $B3
                                 ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, NextIteration_MatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_f, 3_u, false); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, LoopBodyParamsWithoutInitializer) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:5 error: loop: loop with body block parameters must have an initializer
    loop [b: $B2] {  # loop_1
    ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ContinuingUseValueBeforeContinue) {
    auto* f = b.Function("my_func", ty.void_());
    auto* value = b.Let("value", 1_i);
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Append(value);
            b.Append(b.If(true)->True(), [&] { b.Continue(loop); });
            b.ExitLoop(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Let("use", value);
            b.NextIteration(loop);
        });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ContinuingUseValueAfterContinue) {
    auto* f = b.Function("my_func", ty.void_());
    auto* value = b.Let("value", 1_i);
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Append(b.If(true)->True(), [&] { b.Continue(loop); });
            b.Append(value);
            b.ExitLoop(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Let("use", value);
            b.NextIteration(loop);
        });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:14:24 error: let: %value cannot be used in continuing block as it is declared after the first 'continue' in the loop's body
        %use:i32 = let %value
                       ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterUnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, b.Values(1_i, 2_i), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:8:9 error: break_if: provides 2 values but 'loop' block $B2 expects 0 values
        break_if true next_iteration: [ 1i, 2i ]  # -> [t: exit_loop loop_1, f: $B2]
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, Empty, Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:11:9 error: break_if: provides 0 values but 'loop' block $B3 expects 2 values
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
        ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMismatchedInt) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(),
                 [&] { b.NextIteration(loop, nullptr, nullptr, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, b.Values(1_i, 2_i, 3_u, false), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:11:45 error: break_if: operand with type 'i32' does not match 'loop' block $B3 target type 'f32'
        break_if true next_iteration: [ 1i, 2i, 3u, false ]  # -> [t: exit_loop loop_1, f: $B3]
                                            ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMismatchedFloat) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(),
                 [&] { b.NextIteration(loop, nullptr, nullptr, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, b.Values(1_i, 2_f, 3_f, false), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:11:51 error: break_if: operand with type 'f32' does not match 'loop' block $B3 target type 'u32'
        break_if true next_iteration: [ 1i, 2.0f, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B3]
                                                  ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(),
                 [&] { b.NextIteration(loop, nullptr, nullptr, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, b.Values(1_i, 2_f, 3_u, false), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_ExitUnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_i)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:8:9 error: break_if: provides 2 values but 'loop' expects 0 values
        break_if true exit_loop: [ 1i, 2i ]  # -> [t: exit_loop loop_1, f: $B2]
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<i32>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, Empty, Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:8:9 error: break_if: provides 0 values but 'loop' expects 2 values
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
        ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMismatchedInt) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<f32>(),
                         b.InstructionResult<u32>(), b.InstructionResult<bool>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_i, 3_u, false)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:40 error: break_if: operand with type 'i32' does not match 'loop' target type 'f32'
        break_if true exit_loop: [ 1i, 2i, 3u, false ]  # -> [t: exit_loop loop_1, f: $B2]
                                       ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMismatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<f32>(),
                         b.InstructionResult<u32>(), b.InstructionResult<bool>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_f, 3_f, false)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:46 error: break_if: operand with type 'f32' does not match 'loop' target type 'u32'
        break_if true exit_loop: [ 1i, 2.0f, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B2]
                                             ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<f32>(),
                         b.InstructionResult<u32>(), b.InstructionResult<bool>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_f, 3_u, false)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_NullLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(mod.CreateInstruction<ExitLoop>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:9 error: exit_loop: has no parent control instruction
        exit_loop  # undef
        ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_LessOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:9 error: exit_loop: provides 1 value but 'loop' expects 2 values
        exit_loop 1i  # loop_1
        ^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_MoreOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:9 error: exit_loop: provides 3 values but 'loop' expects 2 values
        exit_loop 1i, 2.0f, 3i  # loop_1
        ^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_WithResult) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_IncorrectResultType) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:23 error: exit_loop: operand with type 'i32' does not match 'loop' target type 'f32'
        exit_loop 1i, 2i  # loop_1
                      ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_NotInParentLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(loop);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitLoop(loop); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:13:9 error: exit_loop: loop not found in parent control instructions
        exit_loop  # loop_1
        ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_JumpsOverIfs) {
    // loop {
    //   if (true) {
    //    if (false) {
    //       break;
    //     }
    //   }
    //   break;
    // }
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.Append(loop->Body(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.Append(inner_if_->True(), [&] { b.ExitLoop(loop); });
            b.Return(f);
        });
        b.ExitLoop(loop);
    });

    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverSwitch) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] {
        auto* inner = b.Switch(1_i);
        b.ExitLoop(loop);

        auto* inner_def = b.DefaultCase(inner);
        b.Append(inner_def, [&] { b.ExitLoop(loop); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverLoop) {
    auto* outer_loop = b.Loop();

    outer_loop->Continuing()->Append(b.NextIteration(outer_loop));

    b.Append(outer_loop->Body(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(outer_loop); });
        b.ExitLoop(outer_loop);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(outer_loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideContinuing) {
    auto* loop = b.Loop();

    loop->Continuing()->Append(b.ExitLoop(loop));
    loop->Body()->Append(b.Continue(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:9 error: exit_loop: loop exit jumps out of continuing block
        exit_loop  # loop_1
        ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideContinuingNested) {
    auto* loop = b.Loop();

    b.Append(loop->Continuing(), [&]() {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:10:13 error: exit_loop: loop exit jumps out of continuing block
            exit_loop  # loop_1
            ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideInitializer) {
    auto* loop = b.Loop();

    loop->Initializer()->Append(b.ExitLoop(loop));
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:9 error: exit_loop: loop exit not permitted in loop initializer
        exit_loop  # loop_1
        ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideInitializerNested) {
    auto* loop = b.Loop();

    b.Append(loop->Initializer(), [&]() {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:7:13 error: exit_loop: loop exit not permitted in loop initializer
            exit_loop  # loop_1
            ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_WithValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_UnexpectedResult) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        auto* r = b.Return(f, 42_i);
        r->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: expected exactly 0 results, got 1
    ret 42i
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_NotFunction) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* var = b.Var(ty.ptr<function, f32>());
        auto* r = b.Return(nullptr);
        r->SetOperand(0, var->Result());
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:4:5 error: return: expected function for first operand
    ret
    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_MissingFunction) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* r = b.Return(f);
        r->ClearOperands();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: expected between 1 and 2 operands, got 0
    ret
    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_UnexpectedValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: unexpected return value
    ret 42i
    ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_UnexpectedValue_NullValue_WithVoid) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f, nullptr);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: unexpected return value
    ret undef
    ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_MissingValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: expected return value
    ret
    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_WrongValueType) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:5 error: return: return value type 'f32' does not match function return type 'i32'
    ret 42.0f
    ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_RootBlock) {
    auto f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    mod.root_block->Append(b.Return(f));
    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: return: root block: invalid instruction: tint::core::ir::Return
  ret
  ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Return_MissingResult) {
    auto* ret_func = b.Function("ret_func", ty.void_());
    b.Append(ret_func->Block(), [&] {
        auto* r = b.Return(ret_func);
        r->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: return: expected exactly 0 results, got 1
    ret
    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unreachable_UnexpectedResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* u = b.Unreachable();
        u->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: unreachable: expected exactly 0 results, got 1
    unreachable
    ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unreachable_UnexpectedOperand) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* u = b.Unreachable();
        u->SetOperands(Vector{b.Value(0_i)});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: unreachable: expected exactly 0 operands, got 1
    unreachable
    ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unreachable_RootBlock) {
    auto f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    mod.root_block->Append(b.Unreachable());
    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: unreachable: root block: invalid instruction: tint::core::ir::Unreachable
  unreachable
  ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unreachable_MissingResult) {
    auto* unreachable_func = b.Function("unreachable_func", ty.void_());
    b.Append(unreachable_func->Block(), [&] {
        auto* r = b.Unreachable();
        r->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: unreachable: expected exactly 0 results, got 1
    unreachable
    ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Switch_ConditionPointer) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* s = b.Switch(b.Var("a", b.Zero<i32>()));
        b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(error: switch: condition type 'ptr<function, i32, read_write>' must be an integer scalar
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Switch_NoCases) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Switch(1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: switch: missing default case for switch
    switch 1i [] {  # switch_1
    ^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Switch_NoDefaultCase) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.Case(s, {b.Constant(0_i)}), [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: switch: missing default case for switch
    switch 1i [c: (0i, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Switch_NoCondition) {
    auto* f = b.Function("my_func", ty.void_());

    auto* s = b.ir.CreateInstruction<ir::Switch>();
    f->Block()->Append(s);
    b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(error: switch: operand is undefined
)")) << res.Failure();
}

}  // namespace tint::core::ir
