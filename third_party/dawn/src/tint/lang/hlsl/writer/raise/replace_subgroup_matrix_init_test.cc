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

#include "src/tint/lang/hlsl/writer/raise/replace_subgroup_matrix_init.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterReplaceSubgroupMatrixInitTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Var_Zero) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Var("v", function, ty.subgroup_matrix_result(ty.f32(), 16, 8));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, subgroup_matrix_result<f32, 16, 8>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %v:ptr<function, subgroup_matrix_result<f32, 16, 8>, read_write> = var %2
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Var_Zero_Array) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Var("v", function, ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 8), 4u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, array<subgroup_matrix_result<f32, 16, 8>, 4>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:array<subgroup_matrix_result<f32, 16, 8>, 4> = construct %2, %2, %2, %2
    %v:ptr<function, array<subgroup_matrix_result<f32, 16, 8>, 4>, read_write> = var %3
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Var_Zero_Struct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.u32()},
                            {mod.symbols.New("m"), ty.subgroup_matrix_result(ty.f32(), 16, 8)},
                            {mod.symbols.New("b"), ty.f32()},
                        });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Var("v", function, s);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  m:subgroup_matrix_result<f32, 16, 8> @offset(4)
  b:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %v:ptr<function, S, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  m:subgroup_matrix_result<f32, 16, 8> @offset(4)
  b:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:S = construct 0u, %2, 0.0f
    %v:ptr<function, S, read_write> = var %3
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Zero) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 8));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Zero_I8) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Construct(ty.subgroup_matrix_result(ty.i8(), 16, 8));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<i8, 16, 8> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<i8, 16, 8> = hlsl.Splat<subgroup_matrix_result<i8, 16, 8>> 0i
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Zero_U8) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Construct(ty.subgroup_matrix_result(ty.u8(), 16, 8));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<u8, 16, 8> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<u8, 16, 8> = hlsl.Splat<subgroup_matrix_result<u8, 16, 8>> 0u
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Zero_Array) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Construct(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 8), 4u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f32, 16, 8>, 4> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:array<subgroup_matrix_result<f32, 16, 8>, 4> = construct %2, %2, %2, %2
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Zero_Struct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.u32()},
                            {mod.symbols.New("m"), ty.subgroup_matrix_result(ty.f32(), 16, 8)},
                            {mod.symbols.New("b"), ty.f32()},
                        });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Construct(s);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  m:subgroup_matrix_result<f32, 16, 8> @offset(4)
  b:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:S = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  m:subgroup_matrix_result<f32, 16, 8> @offset(4)
  b:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:S = construct 0u, %2, 0.0f
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Zero_StructOfArrayOfStruct) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"),
                            {
                                {mod.symbols.New("m"), ty.subgroup_matrix_result(ty.f32(), 16, 8)},
                            });
    auto* other =
        ty.Struct(mod.symbols.New("Other"), {
                                                {mod.symbols.New("arr"), ty.array<u32, 4>()},
                                            });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("o1"), other},
                                                {mod.symbols.New("arr"), ty.array(inner, 4u)},
                                                {mod.symbols.New("o2"), ty.array(other, 4u)},
                                            });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Construct(outer);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  m:subgroup_matrix_result<f32, 16, 8> @offset(0)
}

Other = struct @align(4) {
  arr:array<u32, 4> @offset(0)
}

Outer = struct @align(4) {
  o1:Other @offset(0)
  arr_1:array<Inner, 4> @offset(16)
  o2:array<Other, 4> @offset(16)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:Outer = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  m:subgroup_matrix_result<f32, 16, 8> @offset(0)
}

Other = struct @align(4) {
  arr:array<u32, 4> @offset(0)
}

Outer = struct @align(4) {
  o1:Other @offset(0)
  arr_1:array<Inner, 4> @offset(16)
  o2:array<Other, 4> @offset(16)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:Inner = construct %2
    %4:array<Inner, 4> = construct %3, %3, %3, %3
    %5:Outer = construct Other(array<u32, 4>(0u)), %4, array<Other, 4>(Other(array<u32, 4>(0u)))
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Value) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 8), 1.0_f);
        auto* v = b.Var("v", function, ty.subgroup_matrix_result(ty.f32(), 16, 8));
        v->SetInitializer(construct->Result());
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = construct 1.0f
    %v:ptr<function, subgroup_matrix_result<f32, 16, 8>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 1.0f
    %v:ptr<function, subgroup_matrix_result<f32, 16, 8>, read_write> = var %2
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Value_Array) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* arg = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 8), 1.0_f)->Result();
        auto* construct = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 8), 4u),
                                      Vector{arg, arg, arg, arg});
        auto* v = b.Var("v", function, ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 8), 4u));
        v->SetInitializer(construct->Result());
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = construct 1.0f
    %3:array<subgroup_matrix_result<f32, 16, 8>, 4> = construct %2, %2, %2, %2
    %v:ptr<function, array<subgroup_matrix_result<f32, 16, 8>, 4>, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 1.0f
    %3:array<subgroup_matrix_result<f32, 16, 8>, 4> = construct %2, %2, %2, %2
    %v:ptr<function, array<subgroup_matrix_result<f32, 16, 8>, 4>, read_write> = var %3
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Construct_Value_Struct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.u32()},
                            {mod.symbols.New("m"), ty.subgroup_matrix_result(ty.f32(), 16, 8)},
                            {mod.symbols.New("b"), ty.f32()},
                        });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* arg = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 8), 1.0_f);
        auto* construct = b.Construct(s, 42_u, arg, 1.0_f);
        auto* v = b.Var("v", function, s);
        v->SetInitializer(construct->Result());
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  m:subgroup_matrix_result<f32, 16, 8> @offset(4)
  b:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = construct 1.0f
    %3:S = construct 42u, %2, 1.0f
    %v:ptr<function, S, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  m:subgroup_matrix_result<f32, 16, 8> @offset(4)
  b:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 1.0f
    %3:S = construct 42u, %2, 1.0f
    %v:ptr<function, S, read_write> = var %3
    ret
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Unreachable_Basic) {
    auto* func = b.Function("foo", ty.subgroup_matrix_result(ty.f32(), 16, 8));
    b.Append(func->Block(), [&] { b.Unreachable(); });

    auto* src = R"(
%foo = func():subgroup_matrix_result<f32, 16, 8> {
  $B1: {
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():subgroup_matrix_result<f32, 16, 8> {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    ret %2
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Unreachable_Struct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("m"), ty.subgroup_matrix_result(ty.f32(), 16, 8)},
                        });
    auto* func = b.Function("foo", s);
    b.Append(func->Block(), [&] { b.Unreachable(); });

    auto* src = R"(
S = struct @align(4) {
  m:subgroup_matrix_result<f32, 16, 8> @offset(0)
}

%foo = func():S {
  $B1: {
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  m:subgroup_matrix_result<f32, 16, 8> @offset(0)
}

%foo = func():S {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:S = construct %2
    ret %3
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Unreachable_Array) {
    auto* func = b.Function("foo", ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 8), 4u));
    b.Append(func->Block(), [&] { b.Unreachable(); });

    auto* src = R"(
%foo = func():array<subgroup_matrix_result<f32, 16, 8>, 4> {
  $B1: {
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():array<subgroup_matrix_result<f32, 16, 8>, 4> {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    %3:array<subgroup_matrix_result<f32, 16, 8>, 4> = construct %2, %2, %2, %2
    ret %3
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceSubgroupMatrixInitTest, Unreachable_InControlFlow) {
    auto* func = b.Function("foo", ty.subgroup_matrix_result(ty.f32(), 16, 8));
    auto* cond = b.FunctionParam("cond", ty.bool_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* if_inst = b.If(cond);
        b.Append(if_inst->True(), [&] { b.Unreachable(); });
        b.Append(if_inst->False(), [&] {
            b.Return(func, b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 8), 1.0_f));
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%cond:bool):subgroup_matrix_result<f32, 16, 8> {
  $B1: {
    if %cond [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        unreachable
      }
      $B3: {  # false
        %3:subgroup_matrix_result<f32, 16, 8> = construct 1.0f
        ret %3
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond:bool):subgroup_matrix_result<f32, 16, 8> {
  $B1: {
    if %cond [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
        ret %3
      }
      $B3: {  # false
        %4:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 1.0f
        ret %4
      }
    }
    %5:subgroup_matrix_result<f32, 16, 8> = hlsl.Splat<subgroup_matrix_result<f32, 16, 8>> 0.0f
    ret %5
  }
}
)";
    Run(ReplaceSubgroupMatrixInit);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
