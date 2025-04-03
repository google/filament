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

#include "src/tint/lang/core/ir/transform/combine_access_instructions.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_CombineAccessInstructionsTest = TransformTest;

TEST_F(IR_CombineAccessInstructionsTest, NoModify_NoChaining) {
    auto* vec = ty.vec3<f32>();
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        b.Load(access_arr);

        auto* access_mat = b.Access(ty.ptr(uniform, mat), buffer, 0_u, 1_u);
        b.Load(access_mat);

        auto* access_vec = b.Access(ty.ptr(uniform, vec), buffer, 0_u, 1_u, 2_u);
        b.Load(access_vec);

        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:array<mat3x3<f32>, 4> = load %3
    %5:ptr<uniform, mat3x3<f32>, read> = access %buffer, 0u, 1u
    %6:mat3x3<f32> = load %5
    %7:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 2u
    %8:vec3<f32> = load %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, SimpleChain) {
    auto* vec = ty.vec3<f32>();
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 1_u);
        auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 2_u);
        b.Load(access_vec);

        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:ptr<uniform, mat3x3<f32>, read> = access %3, 1u
    %5:ptr<uniform, vec3<f32>, read> = access %4, 2u
    %6:vec3<f32> = load %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 2u
    %4:vec3<f32> = load %3
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, MutipleChains_FromRoot) {
    auto* vec = ty.vec3<f32>();
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);

        {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 1_u);
            auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 2_u);
            b.Load(access_vec);
        }
        {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 2_u);
            auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 0_u);
            b.Load(access_vec);
        }
        {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 3_u);
            auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 1_u);
            b.Load(access_vec);
        }

        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:ptr<uniform, mat3x3<f32>, read> = access %3, 1u
    %5:ptr<uniform, vec3<f32>, read> = access %4, 2u
    %6:vec3<f32> = load %5
    %7:ptr<uniform, mat3x3<f32>, read> = access %3, 2u
    %8:ptr<uniform, vec3<f32>, read> = access %7, 0u
    %9:vec3<f32> = load %8
    %10:ptr<uniform, mat3x3<f32>, read> = access %3, 3u
    %11:ptr<uniform, vec3<f32>, read> = access %10, 1u
    %12:vec3<f32> = load %11
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 2u
    %4:vec3<f32> = load %3
    %5:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 2u, 0u
    %6:vec3<f32> = load %5
    %7:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 3u, 1u
    %8:vec3<f32> = load %7
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, MutipleChains_FromMiddle) {
    auto* vec = ty.vec3<f32>();
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 1_u);

        {
            auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 2_u);
            b.Load(access_vec);
        }
        {
            auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 0_u);
            b.Load(access_vec);
        }
        {
            auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 1_u);
            b.Load(access_vec);
        }

        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:ptr<uniform, mat3x3<f32>, read> = access %3, 1u
    %5:ptr<uniform, vec3<f32>, read> = access %4, 2u
    %6:vec3<f32> = load %5
    %7:ptr<uniform, vec3<f32>, read> = access %4, 0u
    %8:vec3<f32> = load %7
    %9:ptr<uniform, vec3<f32>, read> = access %4, 1u
    %10:vec3<f32> = load %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 2u
    %4:vec3<f32> = load %3
    %5:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 0u
    %6:vec3<f32> = load %5
    %7:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 1u
    %8:vec3<f32> = load %7
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, OtherUses_Root) {
    auto* vec = ty.vec3<f32>();
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        b.Load(access_arr);
        auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 1_u);
        auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 2_u);
        b.Load(access_vec);

        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:array<mat3x3<f32>, 4> = load %3
    %5:ptr<uniform, mat3x3<f32>, read> = access %3, 1u
    %6:ptr<uniform, vec3<f32>, read> = access %5, 2u
    %7:vec3<f32> = load %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:array<mat3x3<f32>, 4> = load %3
    %5:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 2u
    %6:vec3<f32> = load %5
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, OtherUses_Middle) {
    auto* vec = ty.vec3<f32>();
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 1_u);
        b.Load(access_mat);
        auto* access_vec = b.Access(ty.ptr(uniform, vec), access_mat, 2_u);
        b.Load(access_vec);

        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    %4:ptr<uniform, mat3x3<f32>, read> = access %3, 1u
    %5:mat3x3<f32> = load %4
    %6:ptr<uniform, vec3<f32>, read> = access %4, 2u
    %7:vec3<f32> = load %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, mat3x3<f32>, read> = access %buffer, 0u, 1u
    %4:mat3x3<f32> = load %3
    %5:ptr<uniform, vec3<f32>, read> = access %buffer, 0u, 1u, 2u
    %6:vec3<f32> = load %5
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, AccessResultUsesAsAccessIndex) {
    auto* func = b.Function("foo", ty.f32());
    auto* indices = b.FunctionParam("indices", ty.array<u32, 4>());
    auto* values = b.FunctionParam("values", ty.array<f32, 4>());
    func->SetParams({indices, values});
    b.Append(func->Block(), [&] {
        auto* access_index = b.Access(ty.u32(), indices, 1_u);
        auto* access_value = b.Access(ty.f32(), values, access_index);
        b.Return(func, access_value);
    });

    auto* src = R"(
%foo = func(%indices:array<u32, 4>, %values:array<f32, 4>):f32 {
  $B1: {
    %4:u32 = access %indices, 1u
    %5:f32 = access %values, %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, UseInDifferentBlock_If) {
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 1_u);
            b.Load(access_mat);
            b.ExitIf(ifelse);
        });
        b.Append(ifelse->False(), [&] {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 2_u);
            b.Load(access_mat);
            b.ExitIf(ifelse);
        });
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    if true [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %4:ptr<uniform, mat3x3<f32>, read> = access %3, 1u
        %5:mat3x3<f32> = load %4
        exit_if  # if_1
      }
      $B4: {  # false
        %6:ptr<uniform, mat3x3<f32>, read> = access %3, 2u
        %7:mat3x3<f32> = load %6
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    if true [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %3:ptr<uniform, mat3x3<f32>, read> = access %buffer, 0u, 1u
        %4:mat3x3<f32> = load %3
        exit_if  # if_1
      }
      $B4: {  # false
        %5:ptr<uniform, mat3x3<f32>, read> = access %buffer, 0u, 2u
        %6:mat3x3<f32> = load %5
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_CombineAccessInstructionsTest, UseInDifferentBlock_Loop) {
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* access_arr = b.Access(ty.ptr(uniform, arr), buffer, 0_u);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 2_u);
            b.Load(access_mat);
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            auto* access_mat = b.Access(ty.ptr(uniform, mat), access_arr, 3_u);
            b.Load(access_mat);
            b.BreakIf(loop, true);
        });
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<mat3x3<f32>, 4>, read> = access %buffer, 0u
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %4:ptr<uniform, mat3x3<f32>, read> = access %3, 2u
        %5:mat3x3<f32> = load %4
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:ptr<uniform, mat3x3<f32>, read> = access %3, 3u
        %7:mat3x3<f32> = load %6
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:array<mat3x3<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %3:ptr<uniform, mat3x3<f32>, read> = access %buffer, 0u, 2u
        %4:mat3x3<f32> = load %3
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:ptr<uniform, mat3x3<f32>, read> = access %buffer, 0u, 3u
        %6:mat3x3<f32> = load %5
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";

    Run(CombineAccessInstructions);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
