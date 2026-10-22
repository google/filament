// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/extract_ternary_values.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/hlsl/ir/ternary.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriter_ExtractTernaryValuesTest = core::ir::transform::TransformTest;

TEST_F(HlslWriter_ExtractTernaryValuesTest, ConstantCondWithCall) {
    auto* helper = b.Function("bar", ty.i32());
    b.Append(helper->Block(), [&] { b.Return(helper, 2_i); });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* call = b.Call(helper);
        auto* result = b.InstructionResult(ty.i32());
        auto* ternary = b.ir.CreateInstruction<hlsl::ir::Ternary>(
            result, Vector<core::ir::Value*, 3>{b.Value(1_i), call->Result(), b.Value(false)});
        b.Append(ternary);
        b.Return(func);
    });

    auto* src = R"(
%bar = func():i32 {
  $B1: {
    ret 2i
  }
}
%foo = func():void {
  $B2: {
    %3:i32 = call %bar
    %4:i32 = hlsl.ternary 1i, %3, false
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%bar = func():i32 {
  $B1: {
    ret 2i
  }
}
%foo = func():void {
  $B2: {
    %3:i32 = call %bar
    %4:i32 = let %3
    %5:i32 = hlsl.ternary 1i, %4, false
    ret
  }
}
)";

    Run(ExtractTernaryValues);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_ExtractTernaryValuesTest, ConstantCondNoSideEffects) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* result = b.InstructionResult(ty.i32());
        auto* ternary = b.ir.CreateInstruction<hlsl::ir::Ternary>(
            result, Vector<core::ir::Value*, 3>{b.Value(1_i), b.Value(2_i), b.Value(true)});
        b.Append(ternary);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:i32 = hlsl.ternary 1i, 2i, true
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    // Expect no change
    auto* expect = src;

    Run(ExtractTernaryValues);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_ExtractTernaryValuesTest, NonConstantCond) {
    auto* cond = b.FunctionParam<bool>("cond");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* result = b.InstructionResult(ty.i32());
        auto* ternary = b.ir.CreateInstruction<hlsl::ir::Ternary>(
            result, Vector<core::ir::Value*, 3>{b.Value(1_i), b.Value(2_i), cond});
        b.Append(ternary);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%cond:bool):void {
  $B1: {
    %3:i32 = hlsl.ternary 1i, 2i, %cond
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    // Expect no change
    auto* expect = src;

    Run(ExtractTernaryValues);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_ExtractTernaryValuesTest, VectorCond) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* result = b.InstructionResult(ty.vec4<i32>());
        auto* cond = b.Composite(ty.vec4<bool>(), true, false, true, false);
        auto* false_val = b.Composite(ty.vec4<i32>(), 1_i, 2_i, 3_i, 4_i);
        auto* true_val = b.Composite(ty.vec4<i32>(), 5_i, 6_i, 7_i, 8_i);
        auto* ternary = b.ir.CreateInstruction<hlsl::ir::Ternary>(
            result, Vector<core::ir::Value*, 3>{false_val, true_val, cond});
        b.Append(ternary);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:vec4<i32> = hlsl.ternary vec4<i32>(1i, 2i, 3i, 4i), vec4<i32>(5i, 6i, 7i, 8i), vec4<bool>(true, false, true, false)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    // Expect no change
    auto* expect = src;

    Run(ExtractTernaryValues);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
