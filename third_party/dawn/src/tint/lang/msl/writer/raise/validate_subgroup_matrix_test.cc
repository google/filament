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

#include "src/tint/lang/msl/writer/raise/validate_subgroup_matrix.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_ValidateSubgroupMatrixTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_ValidateSubgroupMatrixTest, F16_8x8) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f16(), 8u, 8u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_left<f16, 8, 8>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValidateSubgroupMatrix);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, F32_8x8) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 8u, 8u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_left<f32, 8, 8>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValidateSubgroupMatrix);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, Left8x2) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 8u, 2u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_left<f32, 8, 2>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: subgroup_matrix requires dimensions of 8x8 for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, Right2x8) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_right(ty.f32(), 2u, 8u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_right<f32, 2, 8>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: subgroup_matrix requires dimensions of 8x8 for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, Result8x2) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_result(ty.f32(), 8u, 2u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_result<f32, 8, 2>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: subgroup_matrix requires dimensions of 8x8 for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, i8) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.i8(), 8u, 8u)));
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_left<i8, 8, 8>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: subgroup_matrix requires a type of `f32` or `f16` for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, FunctionParam) {
    auto* f2 = b.Function("f", ty.void_());
    f2->AppendParam(b.FunctionParam("p", ty.subgroup_matrix_left(ty.f32(), 8u, 8u)));
    b.Append(f2->Block(), [&] {  //
        b.Return(f2);
    });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* v = b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 8u, 8u)));
        b.Call(ty.void_(), f2, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
%f = func(%p:subgroup_matrix_left<f32, 8, 8>):void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, subgroup_matrix_left<f32, 8, 8>, read_write> = var undef
    %5:subgroup_matrix_left<f32, 8, 8> = load %v
    %6:void = call %f, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValidateSubgroupMatrix);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, InvalidFunctionParam_i8) {
    auto* f2 = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.subgroup_matrix_left(ty.i8(), 8u, 8u));
    f2->AppendParam(p);
    b.Append(f2->Block(), [&] {  //
        b.Return(f2);
    });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* v = b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.i8(), 8u, 8u)));
        b.Call(ty.void_(), f2, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
%f = func(%p:subgroup_matrix_left<i8, 8, 8>):void {
  $B1: {
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, subgroup_matrix_left<i8, 8, 8>, read_write> = var undef
    %5:subgroup_matrix_left<i8, 8, 8> = load %v
    %6:void = call %f, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: subgroup_matrix requires a type of `f32` or `f16` for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, FunctionReturn) {
    auto* f2 = b.Function("f", ty.subgroup_matrix_left(ty.f32(), 8u, 8u));
    b.Append(f2->Block(), [&] {  //
        b.Return(f2, b.Composite(ty.subgroup_matrix_left(ty.f32(), 8u, 8u), 5_f));
    });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Let("v", b.Call(ty.subgroup_matrix_left(ty.f32(), 8u, 8u), f2));
        b.Return(func);
    });

    auto* src = R"(
%f = func():subgroup_matrix_left<f32, 8, 8> {
  $B1: {
    ret subgroup_matrix_left<f32, 8, 8>(5.0f)
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = call %f
    %v:subgroup_matrix_left<f32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValidateSubgroupMatrix);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, InvalidFunctionReturn_i32) {
    auto* f2 = b.Function("f", ty.subgroup_matrix_left(ty.i32(), 8u, 8u));
    b.Append(f2->Block(), [&] {  //
        b.Return(f2, b.Composite(ty.subgroup_matrix_left(ty.i32(), 8u, 8u), 5_i));
    });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* c = b.Call(ty.subgroup_matrix_left(ty.i32(), 8u, 8u), f2);
        b.Let("v", c);
        b.Return(func);
    });

    auto* src = R"(
%f = func():subgroup_matrix_left<i32, 8, 8> {
  $B1: {
    ret subgroup_matrix_left<i32, 8, 8>(5i)
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<i32, 8, 8> = call %f
    %v:subgroup_matrix_left<i32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: subgroup_matrix requires a type of `f32` or `f16` for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, InStruct_F32_8x8) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.subgroup_matrix_left(ty.f32(), 8u, 8u)},
                        });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, s));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:subgroup_matrix_left<f32, 8, 8> @offset(0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, S, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValidateSubgroupMatrix);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, InStruct_8x2) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.subgroup_matrix_left(ty.f32(), 8u, 2u)},
                        });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("v", ty.ptr(function, s));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:subgroup_matrix_left<f32, 8, 2> @offset(0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, S, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: subgroup_matrix requires dimensions of 8x8 for the selected device)");
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, Nested_F32_8x8) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 8u, 8u)));
            b.ExitIf(if_);
        });
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %v:ptr<function, subgroup_matrix_left<f32, 8, 8>, read_write> = var undef
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ValidateSubgroupMatrix);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ValidateSubgroupMatrixTest, Nested_F32_8x2) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Var("v", ty.ptr(function, ty.subgroup_matrix_left(ty.f32(), 8u, 2u)));
            b.ExitIf(if_);
        });
        b.Return(func);
    });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %v:ptr<function, subgroup_matrix_left<f32, 8, 2>, read_write> = var undef
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto result = RunWithFailure(ValidateSubgroupMatrix);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: subgroup_matrix requires dimensions of 8x8 for the selected device)");
}

}  // namespace
}  // namespace tint::msl::writer::raise
