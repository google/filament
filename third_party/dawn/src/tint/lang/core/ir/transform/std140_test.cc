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

#include "src/tint/lang/core/ir/transform/std140.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_Std140Test = TransformTest;

TEST_F(IR_Std140Test, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NoModify_Mat2x4) {
    auto* mat = ty.mat2x4<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat2x4<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():mat2x4<f32> {
  $B2: {
    %3:ptr<uniform, mat2x4<f32>, read> = access %buffer, 0u
    %4:mat2x4<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NoModify_Mat3x2_StorageBuffer) {
    auto* mat = ty.mat2x4<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat2x4<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():mat2x4<f32> {
  $B2: {
    %3:ptr<storage, mat2x4<f32>, read_write> = access %buffer, 0u
    %4:mat2x4<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Load_Mat2x2f_InArray) {
    auto* mat = ty.mat2x2<f32>();
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("arr"), ty.array(mat, 4u)},
                                               });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(b.Access(ty.ptr(uniform, mat), buffer, 0_u, 2_u));
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  arr:array<mat2x2<f32>, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():mat2x2<f32> {
  $B2: {
    %3:ptr<uniform, mat2x2<f32>, read> = access %buffer, 0u, 2u
    %4:mat2x2<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  arr:array<mat2x2<f32>, 4> @offset(0)
}

mat2x2_f32_std140 = struct @align(8) {
  col0:vec2<f32> @offset(0)
  col1:vec2<f32> @offset(8)
}

MyStruct_std140 = struct @align(8), @block {
  arr:array<mat2x2_f32_std140, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():mat2x2<f32> {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 2u, 0u
    %4:vec2<f32> = load %3
    %5:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 2u, 1u
    %6:vec2<f32> = load %5
    %7:mat2x2<f32> = construct %4, %6
    ret %7
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadMatrix) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():mat3x2<f32> {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %buffer, 0u
    %4:mat3x2<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():mat3x2<f32> {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 0u
    %4:vec2<f32> = load %3
    %5:ptr<uniform, vec2<f32>, read> = access %buffer, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read> = access %buffer, 2u
    %8:vec2<f32> = load %7
    %9:mat3x2<f32> = construct %4, %6, %8
    ret %9
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadConstantColumn) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat->ColumnType());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), buffer, 0_u, 1_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():vec2<f32> {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 1u
    %4:vec2<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():vec2<f32> {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 1u
    %4:vec2<f32> = load %3
    ret %4
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadDynamicColumn) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat->ColumnType());
    auto* column = b.FunctionParam<i32>("column");
    func->AppendParam(column);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), buffer, 0_u, column);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func(%column:i32):vec2<f32> {
  $B2: {
    %4:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, %column
    %5:vec2<f32> = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func(%column:i32):vec2<f32> {
  $B2: {
    %4:ptr<uniform, vec2<f32>, read> = access %buffer, 0u
    %5:vec2<f32> = load %4
    %6:ptr<uniform, vec2<f32>, read> = access %buffer, 1u
    %7:vec2<f32> = load %6
    %8:ptr<uniform, vec2<f32>, read> = access %buffer, 2u
    %9:vec2<f32> = load %8
    %10:mat3x2<f32> = construct %5, %7, %9
    %11:vec2<f32> = access %10, %column
    ret %11
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadElement) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), buffer, 0_u, 1_u);
        auto* load = b.LoadVectorElement(access, 1_u);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 1u
    %4:f32 = load_vector_element %3, 1u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 1u
    %4:f32 = load_vector_element %3, 1u
    ret %4
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadStruct) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", structure);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():MyStruct {
  $B2: {
    %3:MyStruct = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():MyStruct {
  $B2: {
    %3:MyStruct_std140 = load %buffer
    %4:MyStruct = call %tint_convert_MyStruct, %3
    ret %4
  }
}
%tint_convert_MyStruct = func(%tint_input:MyStruct_std140):MyStruct {
  $B3: {
    %7:vec2<f32> = access %tint_input, 0u
    %8:vec2<f32> = access %tint_input, 1u
    %9:vec2<f32> = access %tint_input, 2u
    %10:mat3x2<f32> = construct %7, %8, %9
    %11:MyStruct = construct %10
    ret %11
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadArrayOfStruct) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), mat},
                                                      });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("arr"), ty.array(inner, 4u)},
                                            });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", outer);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  arr:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func():Outer {
  $B2: {
    %3:Outer = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  arr:array<Inner, 4> @offset(0)
}

Inner_std140 = struct @align(8) {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

Outer_std140 = struct @align(8), @block {
  arr:array<Inner_std140, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func():Outer {
  $B2: {
    %3:Outer_std140 = load %buffer
    %4:Outer = call %tint_convert_Outer, %3
    ret %4
  }
}
%tint_convert_Outer = func(%tint_input:Outer_std140):Outer {
  $B3: {
    %7:array<Inner_std140, 4> = access %tint_input, 0u
    %8:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %10:bool = gte %idx, 4u
        if %10 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %11:ptr<function, Inner, read_write> = access %8, %idx
        %12:Inner_std140 = access %7, %idx
        %13:Inner = call %tint_convert_Inner, %12
        store %11, %13
        continue  # -> $B6
      }
      $B6: {  # continuing
        %15:u32 = add %idx, 1u
        next_iteration %15  # -> $B5
      }
    }
    %16:array<Inner, 4> = load %8
    %17:Outer = construct %16
    ret %17
  }
}
%tint_convert_Inner = func(%tint_input_1:Inner_std140):Inner {  # %tint_input_1: 'tint_input'
  $B8: {
    %19:vec2<f32> = access %tint_input_1, 0u
    %20:vec2<f32> = access %tint_input_1, 1u
    %21:vec2<f32> = access %tint_input_1, 2u
    %22:mat3x2<f32> = construct %19, %20, %21
    %23:Inner = construct %22
    ret %23
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadNestedStruct) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), mat},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("inner"), inner},
                                                      });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", inner);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(b.Access(ty.ptr(uniform, inner), buffer, 0_u));
        b.Return(func, load);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  inner:Inner @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func():Inner {
  $B2: {
    %3:ptr<uniform, Inner, read> = access %buffer, 0u
    %4:Inner = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  inner:Inner @offset(0)
}

Inner_std140 = struct @align(8) {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

Outer_std140 = struct @align(8), @block {
  inner:Inner_std140 @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func():Inner {
  $B2: {
    %3:ptr<uniform, Inner_std140, read> = access %buffer, 0u
    %4:Inner_std140 = load %3
    %5:Inner = call %tint_convert_Inner, %4
    ret %5
  }
}
%tint_convert_Inner = func(%tint_input:Inner_std140):Inner {
  $B3: {
    %8:vec2<f32> = access %tint_input, 0u
    %9:vec2<f32> = access %tint_input, 1u
    %10:vec2<f32> = access %tint_input, 2u
    %11:mat3x2<f32> = construct %8, %9, %10
    %12:Inner = construct %11
    ret %12
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadStruct_WithUnmodifedNestedStruct) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.mat4x4<f32>()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("m"), ty.mat3x2<f32>()},
                                                          {mod.symbols.New("inner"), inner},
                                                      });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", outer);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
Inner = struct @align(16) {
  a:mat4x4<f32> @offset(0)
}

Outer = struct @align(16), @block {
  m:mat3x2<f32> @offset(0)
  inner:Inner @offset(32)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func():Outer {
  $B2: {
    %3:Outer = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  a:mat4x4<f32> @offset(0)
}

Outer = struct @align(16), @block {
  m:mat3x2<f32> @offset(0)
  inner:Inner @offset(32)
}

Outer_std140 = struct @align(16), @block {
  m_col0:vec2<f32> @offset(0)
  m_col1:vec2<f32> @offset(8)
  m_col2:vec2<f32> @offset(16)
  inner:Inner @offset(32)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func():Outer {
  $B2: {
    %3:Outer_std140 = load %buffer
    %4:Outer = call %tint_convert_Outer, %3
    ret %4
  }
}
%tint_convert_Outer = func(%tint_input:Outer_std140):Outer {
  $B3: {
    %7:vec2<f32> = access %tint_input, 0u
    %8:vec2<f32> = access %tint_input, 1u
    %9:vec2<f32> = access %tint_input, 2u
    %10:mat3x2<f32> = construct %7, %8, %9
    %11:Inner = access %tint_input, 3u
    %12:Outer = construct %10, %11
    ret %12
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_AccessInstructionWithManyIndices_LoadMatrix) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("m"), ty.array(mat, 4)},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("arr"), arr},
                                                      });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* mat_ptr = b.Access(ty.ptr(uniform, mat), buffer, 0_u, 1_u, 0_u, 2_u);
        b.Let("mat", b.Load(mat_ptr));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  m:array<mat3x2<f32>, 4> @offset(0)
}

Outer = struct @align(8), @block {
  arr:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %buffer, 0u, 1u, 0u, 2u
    %4:mat3x2<f32> = load %3
    %mat:mat3x2<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  m:array<mat3x2<f32>, 4> @offset(0)
}

Outer = struct @align(8), @block {
  arr:array<Inner, 4> @offset(0)
}

mat3x2_f32_std140 = struct @align(8) {
  col0:vec2<f32> @offset(0)
  col1:vec2<f32> @offset(8)
  col2:vec2<f32> @offset(16)
}

Inner_std140 = struct @align(8) {
  m:array<mat3x2_f32_std140, 4> @offset(0)
}

Outer_std140 = struct @align(8), @block {
  arr:array<Inner_std140, 4> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 1u, 0u, 2u, 0u
    %4:vec2<f32> = load %3
    %5:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 1u, 0u, 2u, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read> = access %buffer, 0u, 1u, 0u, 2u, 2u
    %8:vec2<f32> = load %7
    %9:mat3x2<f32> = construct %4, %6, %8
    %mat:mat3x2<f32> = let %9
    ret
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_ChainOfAccessInstructions) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("m"), mat},
                                                          {mod.symbols.New("b"), ty.i32()},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.i32()},
                                                          {mod.symbols.New("arr"), arr},
                                                          {mod.symbols.New("d"), ty.i32()},
                                                      });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ptr = b.Access(ty.ptr(uniform, arr), buffer, 1_u);
        auto* inner_ptr = b.Access(ty.ptr(uniform, inner), arr_ptr, 2_u);
        auto* mat_ptr = b.Access(ty.ptr(uniform, mat), inner_ptr, 1_u);
        auto* col_ptr = b.Access(ty.ptr(uniform, mat->ColumnType()), mat_ptr, 2_u);
        b.Let("arr", b.Load(arr_ptr));
        b.Let("inner", b.Load(inner_ptr));
        b.Let("mat", b.Load(mat_ptr));
        b.Let("col", b.Load(col_ptr));
        b.Let("el", b.LoadVectorElement(col_ptr, 1_u));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<Inner, 4>, read> = access %buffer, 1u
    %4:ptr<uniform, Inner, read> = access %3, 2u
    %5:ptr<uniform, mat3x2<f32>, read> = access %4, 1u
    %6:ptr<uniform, vec2<f32>, read> = access %5, 2u
    %7:array<Inner, 4> = load %3
    %arr:array<Inner, 4> = let %7
    %9:Inner = load %4
    %inner:Inner = let %9
    %11:mat3x2<f32> = load %5
    %mat:mat3x2<f32> = let %11
    %13:vec2<f32> = load %6
    %col:vec2<f32> = let %13
    %15:f32 = load_vector_element %6, 1u
    %el:f32 = let %15
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

Inner_std140 = struct @align(8) {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(8)
  m_col1:vec2<f32> @offset(16)
  m_col2:vec2<f32> @offset(24)
  b:i32 @offset(32)
}

Outer_std140 = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner_std140, 4> @offset(8)
  d:i32 @offset(168)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<Inner_std140, 4>, read> = access %buffer, 1u
    %4:ptr<uniform, Inner_std140, read> = access %3, 2u
    %5:ptr<uniform, vec2<f32>, read> = access %4, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read> = access %4, 2u
    %8:vec2<f32> = load %7
    %9:ptr<uniform, vec2<f32>, read> = access %4, 3u
    %10:vec2<f32> = load %9
    %11:mat3x2<f32> = construct %6, %8, %10
    %12:vec2<f32> = access %11, 2u
    %13:array<Inner_std140, 4> = load %3
    %14:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration 0u  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %16:bool = gte %idx, 4u
        if %16 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %17:ptr<function, Inner, read_write> = access %14, %idx
        %18:Inner_std140 = access %13, %idx
        %19:Inner = call %tint_convert_Inner, %18
        store %17, %19
        continue  # -> $B5
      }
      $B5: {  # continuing
        %21:u32 = add %idx, 1u
        next_iteration %21  # -> $B4
      }
    }
    %22:array<Inner, 4> = load %14
    %arr:array<Inner, 4> = let %22
    %24:Inner_std140 = load %4
    %25:Inner = call %tint_convert_Inner, %24
    %inner:Inner = let %25
    %mat:mat3x2<f32> = let %11
    %col:vec2<f32> = let %12
    %29:f32 = access %12, 1u
    %el:f32 = let %29
    ret
  }
}
%tint_convert_Inner = func(%tint_input:Inner_std140):Inner {
  $B7: {
    %32:i32 = access %tint_input, 0u
    %33:vec2<f32> = access %tint_input, 1u
    %34:vec2<f32> = access %tint_input, 2u
    %35:vec2<f32> = access %tint_input, 3u
    %36:mat3x2<f32> = construct %33, %34, %35
    %37:i32 = access %tint_input, 4u
    %38:Inner = construct %32, %36, %37
    ret %38
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_ChainOfAccessInstructions_ViaLets) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("m"), mat},
                                                          {mod.symbols.New("b"), ty.i32()},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.i32()},
                                                          {mod.symbols.New("arr"), arr},
                                                          {mod.symbols.New("d"), ty.i32()},
                                                      });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ptr = b.Let("arr_ptr", b.Access(ty.ptr(uniform, arr), buffer, 1_u));
        auto* inner_ptr = b.Let("inner_ptr", b.Access(ty.ptr(uniform, inner), arr_ptr, 2_u));
        auto* mat_ptr = b.Let("mat_ptr", b.Access(ty.ptr(uniform, mat), inner_ptr, 1_u));
        auto* col_ptr =
            b.Let("col_ptr", b.Access(ty.ptr(uniform, mat->ColumnType()), mat_ptr, 2_u));
        b.Let("arr", b.Load(arr_ptr));
        b.Let("inner", b.Load(inner_ptr));
        b.Let("mat", b.Load(mat_ptr));
        b.Let("col", b.Load(col_ptr));
        b.Let("el", b.LoadVectorElement(col_ptr, 1_u));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<Inner, 4>, read> = access %buffer, 1u
    %arr_ptr:ptr<uniform, array<Inner, 4>, read> = let %3
    %5:ptr<uniform, Inner, read> = access %arr_ptr, 2u
    %inner_ptr:ptr<uniform, Inner, read> = let %5
    %7:ptr<uniform, mat3x2<f32>, read> = access %inner_ptr, 1u
    %mat_ptr:ptr<uniform, mat3x2<f32>, read> = let %7
    %9:ptr<uniform, vec2<f32>, read> = access %mat_ptr, 2u
    %col_ptr:ptr<uniform, vec2<f32>, read> = let %9
    %11:array<Inner, 4> = load %arr_ptr
    %arr:array<Inner, 4> = let %11
    %13:Inner = load %inner_ptr
    %inner:Inner = let %13
    %15:mat3x2<f32> = load %mat_ptr
    %mat:mat3x2<f32> = let %15
    %17:vec2<f32> = load %col_ptr
    %col:vec2<f32> = let %17
    %19:f32 = load_vector_element %col_ptr, 1u
    %el:f32 = let %19
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

Inner_std140 = struct @align(8) {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(8)
  m_col1:vec2<f32> @offset(16)
  m_col2:vec2<f32> @offset(24)
  b:i32 @offset(32)
}

Outer_std140 = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner_std140, 4> @offset(8)
  d:i32 @offset(168)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, array<Inner_std140, 4>, read> = access %buffer, 1u
    %4:ptr<uniform, Inner_std140, read> = access %3, 2u
    %5:ptr<uniform, vec2<f32>, read> = access %4, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read> = access %4, 2u
    %8:vec2<f32> = load %7
    %9:ptr<uniform, vec2<f32>, read> = access %4, 3u
    %10:vec2<f32> = load %9
    %11:mat3x2<f32> = construct %6, %8, %10
    %12:vec2<f32> = access %11, 2u
    %13:array<Inner_std140, 4> = load %3
    %14:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration 0u  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %16:bool = gte %idx, 4u
        if %16 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %17:ptr<function, Inner, read_write> = access %14, %idx
        %18:Inner_std140 = access %13, %idx
        %19:Inner = call %tint_convert_Inner, %18
        store %17, %19
        continue  # -> $B5
      }
      $B5: {  # continuing
        %21:u32 = add %idx, 1u
        next_iteration %21  # -> $B4
      }
    }
    %22:array<Inner, 4> = load %14
    %arr:array<Inner, 4> = let %22
    %24:Inner_std140 = load %4
    %25:Inner = call %tint_convert_Inner, %24
    %inner:Inner = let %25
    %mat:mat3x2<f32> = let %11
    %col:vec2<f32> = let %12
    %29:f32 = access %12, 1u
    %el:f32 = let %29
    ret
  }
}
%tint_convert_Inner = func(%tint_input:Inner_std140):Inner {
  $B7: {
    %32:i32 = access %tint_input, 0u
    %33:vec2<f32> = access %tint_input, 1u
    %34:vec2<f32> = access %tint_input, 2u
    %35:vec2<f32> = access %tint_input, 3u
    %36:mat3x2<f32> = construct %33, %34, %35
    %37:i32 = access %tint_input, 4u
    %38:Inner = construct %32, %36, %37
    ret %38
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_ChainOfAccessInstructions_DynamicIndices) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("m"), mat},
                                                          {mod.symbols.New("b"), ty.i32()},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.i32()},
                                                          {mod.symbols.New("arr"), arr},
                                                          {mod.symbols.New("d"), ty.i32()},
                                                      });
    outer->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    auto* arr_idx = b.FunctionParam("arr_idx", ty.i32());
    auto* col_idx = b.FunctionParam("col_idx", ty.i32());
    auto* el_idx = b.FunctionParam("el_idx", ty.i32());
    func->SetParams({arr_idx, col_idx, el_idx});
    b.Append(func->Block(), [&] {
        auto* arr_ptr = b.Access(ty.ptr(uniform, arr), buffer, 1_u);
        auto* inner_ptr = b.Access(ty.ptr(uniform, inner), arr_ptr, arr_idx);
        auto* mat_ptr = b.Access(ty.ptr(uniform, mat), inner_ptr, 1_u);
        auto* col_ptr = b.Access(ty.ptr(uniform, mat->ColumnType()), mat_ptr, col_idx);
        b.Let("arr", b.Load(arr_ptr));
        b.Let("inner", b.Load(inner_ptr));
        b.Let("mat", b.Load(mat_ptr));
        b.Let("col", b.Load(col_ptr));
        b.Let("el", b.LoadVectorElement(col_ptr, el_idx));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer, read> = var @binding_point(0, 0)
}

%foo = func(%arr_idx:i32, %col_idx:i32, %el_idx:i32):void {
  $B2: {
    %6:ptr<uniform, array<Inner, 4>, read> = access %buffer, 1u
    %7:ptr<uniform, Inner, read> = access %6, %arr_idx
    %8:ptr<uniform, mat3x2<f32>, read> = access %7, 1u
    %9:ptr<uniform, vec2<f32>, read> = access %8, %col_idx
    %10:array<Inner, 4> = load %6
    %arr:array<Inner, 4> = let %10
    %12:Inner = load %7
    %inner:Inner = let %12
    %14:mat3x2<f32> = load %8
    %mat:mat3x2<f32> = let %14
    %16:vec2<f32> = load %9
    %col:vec2<f32> = let %16
    %18:f32 = load_vector_element %9, %el_idx
    %el:f32 = let %18
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

Inner_std140 = struct @align(8) {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(8)
  m_col1:vec2<f32> @offset(16)
  m_col2:vec2<f32> @offset(24)
  b:i32 @offset(32)
}

Outer_std140 = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner_std140, 4> @offset(8)
  d:i32 @offset(168)
}

$B1: {  # root
  %buffer:ptr<uniform, Outer_std140, read> = var @binding_point(0, 0)
}

%foo = func(%arr_idx:i32, %col_idx:i32, %el_idx:i32):void {
  $B2: {
    %6:ptr<uniform, array<Inner_std140, 4>, read> = access %buffer, 1u
    %7:ptr<uniform, Inner_std140, read> = access %6, %arr_idx
    %8:ptr<uniform, vec2<f32>, read> = access %7, 1u
    %9:vec2<f32> = load %8
    %10:ptr<uniform, vec2<f32>, read> = access %7, 2u
    %11:vec2<f32> = load %10
    %12:ptr<uniform, vec2<f32>, read> = access %7, 3u
    %13:vec2<f32> = load %12
    %14:mat3x2<f32> = construct %9, %11, %13
    %15:vec2<f32> = access %14, %col_idx
    %16:array<Inner_std140, 4> = load %6
    %17:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration 0u  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %19:bool = gte %idx, 4u
        if %19 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %20:ptr<function, Inner, read_write> = access %17, %idx
        %21:Inner_std140 = access %16, %idx
        %22:Inner = call %tint_convert_Inner, %21
        store %20, %22
        continue  # -> $B5
      }
      $B5: {  # continuing
        %24:u32 = add %idx, 1u
        next_iteration %24  # -> $B4
      }
    }
    %25:array<Inner, 4> = load %17
    %arr:array<Inner, 4> = let %25
    %27:Inner_std140 = load %7
    %28:Inner = call %tint_convert_Inner, %27
    %inner:Inner = let %28
    %mat:mat3x2<f32> = let %14
    %col:vec2<f32> = let %15
    %32:f32 = access %15, %el_idx
    %el:f32 = let %32
    ret
  }
}
%tint_convert_Inner = func(%tint_input:Inner_std140):Inner {
  $B7: {
    %35:i32 = access %tint_input, 0u
    %36:vec2<f32> = access %tint_input, 1u
    %37:vec2<f32> = access %tint_input, 2u
    %38:vec2<f32> = access %tint_input, 3u
    %39:mat3x2<f32> = construct %36, %37, %38
    %40:i32 = access %tint_input, 4u
    %41:Inner = construct %35, %39, %40
    ret %41
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NonDefaultAlignAndSize) {
    auto* mat = ty.mat4x2<f32>();
    auto* structure = ty.Get<core::type::Struct>(
        mod.symbols.New("MyStruct"),
        Vector{
            ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 0u, 16u,
                                             core::IOAttributes{}),
            ty.Get<core::type::StructMember>(mod.symbols.New("m"), mat, 1u, 64u, 32u, 64u,
                                             core::IOAttributes{}),
            ty.Get<core::type::StructMember>(mod.symbols.New("b"), ty.i32(), 2u, 128u, 8u, 32u,
                                             core::IOAttributes{}),
        },
        128u, 256u, 160u);
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* a_access = b.Access(ty.ptr(uniform, ty.i32()), buffer, 0_u);
        b.Let("a", b.Load(a_access));
        auto* m_access = b.Access(ty.ptr(uniform, mat), buffer, 1_u);
        b.Let("m", b.Load(m_access));
        auto* b_access = b.Access(ty.ptr(uniform, ty.i32()), buffer, 2_u);
        b.Let("b", b.Load(b_access));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(128), @block {
  a:i32 @offset(0)
  m:mat4x2<f32> @offset(64)
  b:i32 @offset(128)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, i32, read> = access %buffer, 0u
    %4:i32 = load %3
    %a:i32 = let %4
    %6:ptr<uniform, mat4x2<f32>, read> = access %buffer, 1u
    %7:mat4x2<f32> = load %6
    %m:mat4x2<f32> = let %7
    %9:ptr<uniform, i32, read> = access %buffer, 2u
    %10:i32 = load %9
    %b:i32 = let %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(128), @block {
  a:i32 @offset(0)
  m:mat4x2<f32> @offset(64)
  b:i32 @offset(128)
}

MyStruct_std140 = struct @align(128), @block {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(64)
  m_col1:vec2<f32> @offset(72)
  m_col2:vec2<f32> @offset(80)
  m_col3:vec2<f32> @offset(88)
  b:i32 @offset(128)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, i32, read> = access %buffer, 0u
    %4:i32 = load %3
    %a:i32 = let %4
    %6:ptr<uniform, vec2<f32>, read> = access %buffer, 1u
    %7:vec2<f32> = load %6
    %8:ptr<uniform, vec2<f32>, read> = access %buffer, 2u
    %9:vec2<f32> = load %8
    %10:ptr<uniform, vec2<f32>, read> = access %buffer, 3u
    %11:vec2<f32> = load %10
    %12:ptr<uniform, vec2<f32>, read> = access %buffer, 4u
    %13:vec2<f32> = load %12
    %14:mat4x2<f32> = construct %7, %9, %11, %13
    %m:mat4x2<f32> = let %14
    %16:ptr<uniform, i32, read> = access %buffer, 5u
    %17:i32 = load %16
    %b:i32 = let %17
    ret
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat4x3_LoadMatrix) {
    auto* mat = ty.mat4x3<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat4x3<f32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():mat4x3<f32> {
  $B2: {
    %3:ptr<uniform, mat4x3<f32>, read> = access %buffer, 0u
    %4:mat4x3<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16), @block {
  a:mat4x3<f32> @offset(0)
}

MyStruct_std140 = struct @align(16), @block {
  a_col0:vec3<f32> @offset(0)
  a_col1:vec3<f32> @offset(16)
  a_col2:vec3<f32> @offset(32)
  a_col3:vec3<f32> @offset(48)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():mat4x3<f32> {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %buffer, 0u
    %4:vec3<f32> = load %3
    %5:ptr<uniform, vec3<f32>, read> = access %buffer, 1u
    %6:vec3<f32> = load %5
    %7:ptr<uniform, vec3<f32>, read> = access %buffer, 2u
    %8:vec3<f32> = load %7
    %9:ptr<uniform, vec3<f32>, read> = access %buffer, 3u
    %10:vec3<f32> = load %9
    %11:mat4x3<f32> = construct %4, %6, %8, %10
    ret %11
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NotAllMatricesDecomposed) {
    auto* mat4x4 = ty.mat4x4<f32>();
    auto* mat3x2 = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat4x4},
                                                                 {mod.symbols.New("b"), mat3x2},
                                                             });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    {
        auto* func = b.Function("load_struct_a", mat4x4);
        b.Append(func->Block(), [&] {
            auto* load_struct = b.Load(buffer);
            auto* extract_mat = b.Access(mat4x4, load_struct, 0_u);
            b.Return(func, extract_mat);
        });
    }

    {
        auto* func = b.Function("load_struct_b", mat3x2);
        b.Append(func->Block(), [&] {
            auto* load_struct = b.Load(buffer);
            auto* extract_mat = b.Access(mat3x2, load_struct, 1_u);
            b.Return(func, extract_mat);
        });
    }

    {
        auto* func = b.Function("load_mat_a", ty.vec4<f32>());
        b.Append(func->Block(), [&] {
            auto* access_mat = b.Access(ty.ptr(uniform, mat4x4), buffer, 0_u);
            auto* load_mat = b.Load(access_mat);
            auto* extract_vec = b.Access(ty.vec4<f32>(), load_mat, 0_u);
            b.Return(func, extract_vec);
        });
    }

    {
        auto* func = b.Function("load_mat_b", ty.vec2<f32>());
        b.Append(func->Block(), [&] {
            auto* access_mat = b.Access(ty.ptr(uniform, mat3x2), buffer, 1_u);
            auto* load_mat = b.Load(access_mat);
            auto* extract_vec = b.Access(ty.vec2<f32>(), load_mat, 0_u);
            b.Return(func, extract_vec);
        });
    }

    {
        auto* func = b.Function("load_vec_a", ty.f32());
        b.Append(func->Block(), [&] {
            auto* access_vec = b.Access(ty.ptr(uniform, mat4x4->ColumnType()), buffer, 0_u, 1_u);
            auto* load_vec = b.Load(access_vec);
            auto* extract_el = b.Access(ty.f32(), load_vec, 1_u);
            b.Return(func, extract_el);
        });
    }

    {
        auto* func = b.Function("load_vec_b", ty.f32());
        b.Append(func->Block(), [&] {
            auto* access_vec = b.Access(ty.ptr(uniform, mat3x2->ColumnType()), buffer, 1_u, 1_u);
            auto* load_vec = b.Load(access_vec);
            auto* extract_el = b.Access(ty.f32(), load_vec, 1_u);
            b.Return(func, extract_el);
        });
    }

    {
        auto* func = b.Function("lve_a", ty.f32());
        b.Append(func->Block(), [&] {
            auto* access_vec = b.Access(ty.ptr(uniform, mat4x4->ColumnType()), buffer, 0_u, 1_u);
            auto* lve = b.LoadVectorElement(access_vec, 1_u);
            b.Return(func, lve);
        });
    }

    {
        auto* func = b.Function("lve_b", ty.f32());
        b.Append(func->Block(), [&] {
            auto* access_vec = b.Access(ty.ptr(uniform, mat3x2->ColumnType()), buffer, 1_u, 1_u);
            auto* lve = b.LoadVectorElement(access_vec, 1_u);
            b.Return(func, lve);
        });
    }

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat4x4<f32> @offset(0)
  b:mat3x2<f32> @offset(64)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%load_struct_a = func():mat4x4<f32> {
  $B2: {
    %3:MyStruct = load %buffer
    %4:mat4x4<f32> = access %3, 0u
    ret %4
  }
}
%load_struct_b = func():mat3x2<f32> {
  $B3: {
    %6:MyStruct = load %buffer
    %7:mat3x2<f32> = access %6, 1u
    ret %7
  }
}
%load_mat_a = func():vec4<f32> {
  $B4: {
    %9:ptr<uniform, mat4x4<f32>, read> = access %buffer, 0u
    %10:mat4x4<f32> = load %9
    %11:vec4<f32> = access %10, 0u
    ret %11
  }
}
%load_mat_b = func():vec2<f32> {
  $B5: {
    %13:ptr<uniform, mat3x2<f32>, read> = access %buffer, 1u
    %14:mat3x2<f32> = load %13
    %15:vec2<f32> = access %14, 0u
    ret %15
  }
}
%load_vec_a = func():f32 {
  $B6: {
    %17:ptr<uniform, vec4<f32>, read> = access %buffer, 0u, 1u
    %18:vec4<f32> = load %17
    %19:f32 = access %18, 1u
    ret %19
  }
}
%load_vec_b = func():f32 {
  $B7: {
    %21:ptr<uniform, vec2<f32>, read> = access %buffer, 1u, 1u
    %22:vec2<f32> = load %21
    %23:f32 = access %22, 1u
    ret %23
  }
}
%lve_a = func():f32 {
  $B8: {
    %25:ptr<uniform, vec4<f32>, read> = access %buffer, 0u, 1u
    %26:f32 = load_vector_element %25, 1u
    ret %26
  }
}
%lve_b = func():f32 {
  $B9: {
    %28:ptr<uniform, vec2<f32>, read> = access %buffer, 1u, 1u
    %29:f32 = load_vector_element %28, 1u
    ret %29
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16), @block {
  a:mat4x4<f32> @offset(0)
  b:mat3x2<f32> @offset(64)
}

MyStruct_std140 = struct @align(16), @block {
  a:mat4x4<f32> @offset(0)
  b_col0:vec2<f32> @offset(64)
  b_col1:vec2<f32> @offset(72)
  b_col2:vec2<f32> @offset(80)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%load_struct_a = func():mat4x4<f32> {
  $B2: {
    %3:MyStruct_std140 = load %buffer
    %4:MyStruct = call %tint_convert_MyStruct, %3
    %6:mat4x4<f32> = access %4, 0u
    ret %6
  }
}
%load_struct_b = func():mat3x2<f32> {
  $B3: {
    %8:MyStruct_std140 = load %buffer
    %9:MyStruct = call %tint_convert_MyStruct, %8
    %10:mat3x2<f32> = access %9, 1u
    ret %10
  }
}
%load_mat_a = func():vec4<f32> {
  $B4: {
    %12:ptr<uniform, mat4x4<f32>, read> = access %buffer, 0u
    %13:mat4x4<f32> = load %12
    %14:vec4<f32> = access %13, 0u
    ret %14
  }
}
%load_mat_b = func():vec2<f32> {
  $B5: {
    %16:ptr<uniform, vec2<f32>, read> = access %buffer, 1u
    %17:vec2<f32> = load %16
    %18:ptr<uniform, vec2<f32>, read> = access %buffer, 2u
    %19:vec2<f32> = load %18
    %20:ptr<uniform, vec2<f32>, read> = access %buffer, 3u
    %21:vec2<f32> = load %20
    %22:mat3x2<f32> = construct %17, %19, %21
    %23:vec2<f32> = access %22, 0u
    ret %23
  }
}
%load_vec_a = func():f32 {
  $B6: {
    %25:ptr<uniform, vec4<f32>, read> = access %buffer, 0u, 1u
    %26:vec4<f32> = load %25
    %27:f32 = access %26, 1u
    ret %27
  }
}
%load_vec_b = func():f32 {
  $B7: {
    %29:ptr<uniform, vec2<f32>, read> = access %buffer, 2u
    %30:vec2<f32> = load %29
    %31:f32 = access %30, 1u
    ret %31
  }
}
%lve_a = func():f32 {
  $B8: {
    %33:ptr<uniform, vec4<f32>, read> = access %buffer, 0u, 1u
    %34:f32 = load_vector_element %33, 1u
    ret %34
  }
}
%lve_b = func():f32 {
  $B9: {
    %36:ptr<uniform, vec2<f32>, read> = access %buffer, 2u
    %37:f32 = load_vector_element %36, 1u
    ret %37
  }
}
%tint_convert_MyStruct = func(%tint_input:MyStruct_std140):MyStruct {
  $B10: {
    %39:mat4x4<f32> = access %tint_input, 0u
    %40:vec2<f32> = access %tint_input, 1u
    %41:vec2<f32> = access %tint_input, 2u
    %42:vec2<f32> = access %tint_input, 3u
    %43:mat3x2<f32> = construct %40, %41, %42
    %44:MyStruct = construct %39, %43
    ret %44
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, F16) {
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.mat2x2<f16>()},
                                                   {mod.symbols.New("b"), ty.mat2x4<f16>()},
                                                   {mod.symbols.New("c"), ty.mat4x3<f16>()},
                                                   {mod.symbols.New("d"), ty.mat4x4<f16>()},
                                               });
    structure->SetStructFlag(core::type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("struct", b.Load(buffer));
        b.Let("mat", b.Load(b.Access(ty.ptr(uniform, ty.mat4x4<f16>()), buffer, 3_u)));
        b.Let("col", b.Load(b.Access(ty.ptr(uniform, ty.vec3<f16>()), buffer, 2_u, 1_u)));
        b.Let("el", b.LoadVectorElement(b.Access(ty.ptr(uniform, ty.vec4<f16>()), buffer, 1_u, 0_u),
                                        3_u));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat2x2<f16> @offset(0)
  b:mat2x4<f16> @offset(8)
  c:mat4x3<f16> @offset(24)
  d:mat4x4<f16> @offset(56)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:MyStruct = load %buffer
    %struct:MyStruct = let %3
    %5:ptr<uniform, mat4x4<f16>, read> = access %buffer, 3u
    %6:mat4x4<f16> = load %5
    %mat:mat4x4<f16> = let %6
    %8:ptr<uniform, vec3<f16>, read> = access %buffer, 2u, 1u
    %9:vec3<f16> = load %8
    %col:vec3<f16> = let %9
    %11:ptr<uniform, vec4<f16>, read> = access %buffer, 1u, 0u
    %12:f16 = load_vector_element %11, 3u
    %el:f16 = let %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat2x2<f16> @offset(0)
  b:mat2x4<f16> @offset(8)
  c:mat4x3<f16> @offset(24)
  d:mat4x4<f16> @offset(56)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f16> @offset(0)
  a_col1:vec2<f16> @offset(4)
  b_col0:vec4<f16> @offset(8)
  b_col1:vec4<f16> @offset(16)
  c_col0:vec3<f16> @offset(24)
  c_col1:vec3<f16> @offset(32)
  c_col2:vec3<f16> @offset(40)
  c_col3:vec3<f16> @offset(48)
  d_col0:vec4<f16> @offset(56)
  d_col1:vec4<f16> @offset(64)
  d_col2:vec4<f16> @offset(72)
  d_col3:vec4<f16> @offset(80)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_std140, read> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:MyStruct_std140 = load %buffer
    %4:MyStruct = call %tint_convert_MyStruct, %3
    %struct:MyStruct = let %4
    %7:ptr<uniform, vec4<f16>, read> = access %buffer, 8u
    %8:vec4<f16> = load %7
    %9:ptr<uniform, vec4<f16>, read> = access %buffer, 9u
    %10:vec4<f16> = load %9
    %11:ptr<uniform, vec4<f16>, read> = access %buffer, 10u
    %12:vec4<f16> = load %11
    %13:ptr<uniform, vec4<f16>, read> = access %buffer, 11u
    %14:vec4<f16> = load %13
    %15:mat4x4<f16> = construct %8, %10, %12, %14
    %mat:mat4x4<f16> = let %15
    %17:ptr<uniform, vec3<f16>, read> = access %buffer, 5u
    %18:vec3<f16> = load %17
    %col:vec3<f16> = let %18
    %20:ptr<uniform, vec4<f16>, read> = access %buffer, 2u
    %21:f16 = load_vector_element %20, 3u
    %el:f16 = let %21
    ret
  }
}
%tint_convert_MyStruct = func(%tint_input:MyStruct_std140):MyStruct {
  $B3: {
    %24:vec2<f16> = access %tint_input, 0u
    %25:vec2<f16> = access %tint_input, 1u
    %26:mat2x2<f16> = construct %24, %25
    %27:vec4<f16> = access %tint_input, 2u
    %28:vec4<f16> = access %tint_input, 3u
    %29:mat2x4<f16> = construct %27, %28
    %30:vec3<f16> = access %tint_input, 4u
    %31:vec3<f16> = access %tint_input, 5u
    %32:vec3<f16> = access %tint_input, 6u
    %33:vec3<f16> = access %tint_input, 7u
    %34:mat4x3<f16> = construct %30, %31, %32, %33
    %35:vec4<f16> = access %tint_input, 8u
    %36:vec4<f16> = access %tint_input, 9u
    %37:vec4<f16> = access %tint_input, 10u
    %38:vec4<f16> = access %tint_input, 11u
    %39:mat4x4<f16> = construct %35, %36, %37, %38
    %40:MyStruct = construct %26, %29, %34, %39
    ret %40
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x3f_And_ArrayMat4x3f) {  // crbug.com/338727551
    auto* s =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.mat3x3<f32>()},
                                            {mod.symbols.New("b"), ty.array<mat4x3<f32>, 3>()},
                                        });
    s->SetStructFlag(core::type::kBlock);

    auto* u = b.Var("u", ty.ptr(uniform, s));
    u->SetBindingPoint(0, 0);
    mod.root_block->Append(u);

    auto* f = b.Function("F", ty.f32());
    b.Append(f->Block(), [&] {
        auto* p = b.Access<ptr<uniform, vec3<f32>, read>>(u, 1_u, 0_u, 0_u);
        auto* x = b.LoadVectorElement(p, 0_u);
        b.Return(f, x);
    });

    auto* src = R"(
S = struct @align(16), @block {
  a:mat3x3<f32> @offset(0)
  b:array<mat4x3<f32>, 3> @offset(48)
}

$B1: {  # root
  %u:ptr<uniform, S, read> = var @binding_point(0, 0)
}

%F = func():f32 {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %u, 1u, 0u, 0u
    %4:f32 = load_vector_element %3, 0u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16), @block {
  a:mat3x3<f32> @offset(0)
  b:array<mat4x3<f32>, 3> @offset(48)
}

mat4x3_f32_std140 = struct @align(16) {
  col0:vec3<f32> @offset(0)
  col1:vec3<f32> @offset(16)
  col2:vec3<f32> @offset(32)
  col3:vec3<f32> @offset(48)
}

S_std140 = struct @align(16), @block {
  a_col0:vec3<f32> @offset(0)
  a_col1:vec3<f32> @offset(16)
  a_col2:vec3<f32> @offset(32)
  b:array<mat4x3_f32_std140, 3> @offset(48)
}

$B1: {  # root
  %u:ptr<uniform, S_std140, read> = var @binding_point(0, 0)
}

%F = func():f32 {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %u, 3u, 0u, 0u
    %4:f32 = load_vector_element %3, 0u
    ret %4
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x3f_And_ArrayStructMat4x3f) {
    auto* s1 =
        ty.Struct(mod.symbols.New("S1"), {
                                             {mod.symbols.New("c"), ty.mat3x3<f32>()},
                                             {mod.symbols.New("d"), ty.array<mat4x3<f32>, 3>()},
                                         });
    auto* s2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.mat3x3<f32>()},
                                                    {mod.symbols.New("b"), s1},
                                                });
    s2->SetStructFlag(core::type::kBlock);

    auto* u = b.Var("u", ty.ptr(uniform, s2));
    u->SetBindingPoint(0, 0);
    mod.root_block->Append(u);

    auto* f = b.Function("F", ty.f32());
    b.Append(f->Block(), [&] {
        auto* p = b.Access<ptr<uniform, vec3<f32>, read>>(u, 1_u, 1_u, 0_u, 0_u);
        auto* x = b.LoadVectorElement(p, 0_u);
        b.Return(f, x);
    });

    auto* src = R"(
S1 = struct @align(16) {
  c:mat3x3<f32> @offset(0)
  d:array<mat4x3<f32>, 3> @offset(48)
}

S2 = struct @align(16), @block {
  a:mat3x3<f32> @offset(0)
  b:S1 @offset(48)
}

$B1: {  # root
  %u:ptr<uniform, S2, read> = var @binding_point(0, 0)
}

%F = func():f32 {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %u, 1u, 1u, 0u, 0u
    %4:f32 = load_vector_element %3, 0u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S1 = struct @align(16) {
  c:mat3x3<f32> @offset(0)
  d:array<mat4x3<f32>, 3> @offset(48)
}

S2 = struct @align(16), @block {
  a:mat3x3<f32> @offset(0)
  b:S1 @offset(48)
}

mat4x3_f32_std140 = struct @align(16) {
  col0:vec3<f32> @offset(0)
  col1:vec3<f32> @offset(16)
  col2:vec3<f32> @offset(32)
  col3:vec3<f32> @offset(48)
}

S1_std140 = struct @align(16) {
  c_col0:vec3<f32> @offset(0)
  c_col1:vec3<f32> @offset(16)
  c_col2:vec3<f32> @offset(32)
  d:array<mat4x3_f32_std140, 3> @offset(48)
}

S2_std140 = struct @align(16), @block {
  a_col0:vec3<f32> @offset(0)
  a_col1:vec3<f32> @offset(16)
  a_col2:vec3<f32> @offset(32)
  b:S1_std140 @offset(48)
}

$B1: {  # root
  %u:ptr<uniform, S2_std140, read> = var @binding_point(0, 0)
}

%F = func():f32 {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %u, 3u, 3u, 0u, 0u
    %4:f32 = load_vector_element %3, 0u
    ret %4
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x3f_And_ArrayStructMat2x2f) {
    auto* s1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("c"), ty.mat2x2<f32>()},
                                                });
    auto* s2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.mat3x3<f32>()},
                                                    {mod.symbols.New("b"), s1},
                                                });
    s2->SetStructFlag(core::type::kBlock);

    auto* u = b.Var("u", ty.ptr(uniform, s2));
    u->SetBindingPoint(0, 0);
    mod.root_block->Append(u);

    auto* f = b.Function("F", ty.f32());
    b.Append(f->Block(), [&] {
        auto* p = b.Access<ptr<uniform, vec2<f32>, read>>(u, 1_u, 0_u, 0_u);
        auto* x = b.LoadVectorElement(p, 0_u);
        b.Return(f, x);
    });

    auto* src = R"(
S1 = struct @align(8) {
  c:mat2x2<f32> @offset(0)
}

S2 = struct @align(16), @block {
  a:mat3x3<f32> @offset(0)
  b:S1 @offset(48)
}

$B1: {  # root
  %u:ptr<uniform, S2, read> = var @binding_point(0, 0)
}

%F = func():f32 {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %u, 1u, 0u, 0u
    %4:f32 = load_vector_element %3, 0u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S1 = struct @align(8) {
  c:mat2x2<f32> @offset(0)
}

S2 = struct @align(16), @block {
  a:mat3x3<f32> @offset(0)
  b:S1 @offset(48)
}

S1_std140 = struct @align(8) {
  c_col0:vec2<f32> @offset(0)
  c_col1:vec2<f32> @offset(8)
}

S2_std140 = struct @align(16), @block {
  a_col0:vec3<f32> @offset(0)
  a_col1:vec3<f32> @offset(16)
  a_col2:vec3<f32> @offset(32)
  b:S1_std140 @offset(48)
}

$B1: {  # root
  %u:ptr<uniform, S2_std140, read> = var @binding_point(0, 0)
}

%F = func():f32 {
  $B2: {
    %3:ptr<uniform, vec2<f32>, read> = access %u, 3u, 0u
    %4:f32 = load_vector_element %3, 0u
    ret %4
  }
}
)";

    Run(Std140);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
