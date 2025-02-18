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

#include "src/tint/lang/core/ir/traverse.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_TraverseTest = IRTestHelper;

TEST_F(IR_TraverseTest, Blocks) {
    Vector<Instruction*, 8> expect;

    auto fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        expect.Push(b.Var<function, i32>());

        auto* if_ = b.If(true);
        expect.Push(if_);
        b.Append(if_->True(), [&] {
            auto* if2_ = b.If(true);
            expect.Push(if2_);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            expect.Push(b.ExitIf(if_));
        });

        b.Append(if_->False(), [&] { expect.Push(b.ExitIf(if_)); });

        auto* loop_ = b.Loop();
        expect.Push(loop_);
        b.Append(loop_->Initializer(), [&] { expect.Push(b.NextIteration(loop_)); });
        b.Append(loop_->Body(), [&] {
            auto* if2_ = b.If(true);
            expect.Push(if2_);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            expect.Push(b.Continue(loop_));
        });
        b.Append(loop_->Continuing(), [&] { expect.Push(b.NextIteration(loop_)); });

        auto* switch_ = b.Switch(1_i);
        expect.Push(switch_);

        auto* case_0 = b.Case(switch_, {b.Constant(0_i)});
        b.Append(case_0, [&] { expect.Push(b.Var<function, i32>()); });

        auto* case_1 = b.Case(switch_, {b.Constant(1_i)});
        b.Append(case_1, [&] { expect.Push(b.Var<function, i32>()); });

        expect.Push(b.Var<function, i32>());
    });

    Vector<Instruction*, 8> got;
    Traverse(fn->Block(), [&](Instruction* inst) { got.Push(inst); });

    EXPECT_THAT(got, testing::ContainerEq(expect));
}

TEST_F(IR_TraverseTest, Filtered) {
    Vector<ExitIf*, 8> expect;

    auto fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var<function, i32>();

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* if2_ = b.If(true);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            expect.Push(b.ExitIf(if_));
        });

        b.Append(if_->False(), [&] { expect.Push(b.ExitIf(if_)); });

        auto* loop_ = b.Loop();
        b.Append(loop_->Initializer(), [&] { b.NextIteration(loop_); });
        b.Append(loop_->Body(), [&] {
            auto* if2_ = b.If(true);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            b.Continue(loop_);
        });
        b.Append(loop_->Continuing(), [&] { b.NextIteration(loop_); });

        auto* switch_ = b.Switch(1_i);

        auto* case_0 = b.Case(switch_, {b.Constant(0_i)});
        b.Append(case_0, [&] { b.Var<function, i32>(); });

        auto* case_1 = b.Case(switch_, {b.Constant(1_i)});
        b.Append(case_1, [&] { b.Var<function, i32>(); });

        b.Var<function, i32>();
    });

    Vector<ExitIf*, 8> got;
    Traverse(fn->Block(), [&](ExitIf* inst) { got.Push(inst); });

    EXPECT_THAT(got, testing::ContainerEq(expect));
}

}  // namespace
}  // namespace tint::core::ir
