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

#include "src/tint/lang/core/ir/transform/preserve_padding.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_PreservePaddingTest : public TransformTest {
  protected:
    const type::Struct* MakeStructWithoutPadding() {
        auto* structure =
            ty.Struct(mod.symbols.New("MyStruct"), {
                                                       {mod.symbols.New("a"), ty.vec4<u32>()},
                                                       {mod.symbols.New("b"), ty.vec4<u32>()},
                                                       {mod.symbols.New("c"), ty.vec4<u32>()},
                                                   });
        return structure;
    }

    const type::Struct* MakeStructWithTrailingPadding() {
        auto* structure =
            ty.Struct(mod.symbols.New("MyStruct"), {
                                                       {mod.symbols.New("a"), ty.vec4<u32>()},
                                                       {mod.symbols.New("b"), ty.u32()},
                                                   });
        return structure;
    }

    const type::Struct* MakeStructWithInternalPadding() {
        auto* structure =
            ty.Struct(mod.symbols.New("MyStruct"), {
                                                       {mod.symbols.New("a"), ty.vec4<u32>()},
                                                       {mod.symbols.New("b"), ty.u32()},
                                                       {mod.symbols.New("c"), ty.vec4<u32>()},
                                                   });
        return structure;
    }
};

TEST_F(IR_PreservePaddingTest, NoModify_Workgroup) {
    auto* structure = MakeStructWithTrailingPadding();
    auto* buffer = b.Var("buffer", ty.ptr(workgroup, structure));
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<workgroup, MyStruct, read_write> = var
}

%foo = func(%value:MyStruct):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_Private) {
    auto* structure = MakeStructWithTrailingPadding();
    auto* buffer = b.Var("buffer", ty.ptr(private_, structure));
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<private, MyStruct, read_write> = var
}

%foo = func(%value:MyStruct):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_Function) {
    auto* structure = MakeStructWithTrailingPadding();

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* buffer = b.Var("buffer", ty.ptr(function, structure));
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

%foo = func(%value:MyStruct):void {
  $B1: {
    %buffer:ptr<function, MyStruct, read_write> = var
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_StructWithoutPadding) {
    auto* structure = MakeStructWithoutPadding();
    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:vec4<u32> @offset(16)
  c:vec4<u32> @offset(32)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_MatrixWithoutPadding) {
    auto* mat = ty.mat4x4<f32>();
    auto* buffer = b.Var("buffer", ty.ptr(storage, mat));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", mat);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, mat4x4<f32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:mat4x4<f32>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_ArrayWithoutPadding) {
    auto* arr = ty.array<vec4<f32>, 4>();
    auto* buffer = b.Var("buffer", ty.ptr(storage, arr));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", arr);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<vec4<f32>, 4>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_Vec3) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.vec3<f32>()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, vec3<f32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NoModify_LoadStructWithTrailingPadding) {
    auto* structure = MakeStructWithTrailingPadding();

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", structure);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():MyStruct {
  $B2: {
    %3:MyStruct = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, Struct_TrailingPadding) {
    auto* structure = MakeStructWithTrailingPadding();

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, MyStruct, read_write>, %value_param:MyStruct):void {
  $B3: {
    %8:ptr<storage, vec4<u32>, read_write> = access %target, 0u
    %9:vec4<u32> = access %value_param, 0u
    store %8, %9
    %10:ptr<storage, u32, read_write> = access %target, 1u
    %11:u32 = access %value_param, 1u
    store %10, %11
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, Struct_InternalPadding) {
    auto* structure = MakeStructWithInternalPadding();

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
  c:vec4<u32> @offset(32)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
  c:vec4<u32> @offset(32)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, MyStruct, read_write>, %value_param:MyStruct):void {
  $B3: {
    %8:ptr<storage, vec4<u32>, read_write> = access %target, 0u
    %9:vec4<u32> = access %value_param, 0u
    store %8, %9
    %10:ptr<storage, u32, read_write> = access %target, 1u
    %11:u32 = access %value_param, 1u
    store %10, %11
    %12:ptr<storage, vec4<u32>, read_write> = access %target, 2u
    %13:vec4<u32> = access %value_param, 2u
    store %12, %13
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, NestedStructWithPadding) {
    auto* inner = MakeStructWithInternalPadding();
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("inner"), inner},
                                                      });

    auto* buffer = b.Var("buffer", ty.ptr(storage, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", outer);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
  c:vec4<u32> @offset(32)
}

Outer = struct @align(16) {
  inner:MyStruct @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:Outer):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
  c:vec4<u32> @offset(32)
}

Outer = struct @align(16) {
  inner:MyStruct @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:Outer):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, Outer, read_write>, %value_param:Outer):void {
  $B3: {
    %8:ptr<storage, MyStruct, read_write> = access %target, 0u
    %9:MyStruct = access %value_param, 0u
    %10:void = call %tint_store_and_preserve_padding_1, %8, %9
    ret
  }
}
%tint_store_and_preserve_padding_1 = func(%target_1:ptr<storage, MyStruct, read_write>, %value_param_1:MyStruct):void {  # %tint_store_and_preserve_padding_1: 'tint_store_and_preserve_padding', %target_1: 'target', %value_param_1: 'value_param'
  $B4: {
    %14:ptr<storage, vec4<u32>, read_write> = access %target_1, 0u
    %15:vec4<u32> = access %value_param_1, 0u
    store %14, %15
    %16:ptr<storage, u32, read_write> = access %target_1, 1u
    %17:u32 = access %value_param_1, 1u
    store %16, %17
    %18:ptr<storage, vec4<u32>, read_write> = access %target_1, 2u
    %19:vec4<u32> = access %value_param_1, 2u
    store %18, %19
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, StructWithPadding_InArray) {
    auto* structure = MakeStructWithTrailingPadding();
    auto* arr = ty.array(structure, 4);

    auto* buffer = b.Var("buffer", ty.ptr(storage, arr));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", arr);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, array<MyStruct, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<MyStruct, 4>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:u32 @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, array<MyStruct, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<MyStruct, 4>):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, array<MyStruct, 4>, read_write>, %value_param:array<MyStruct, 4>):void {
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 4u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:ptr<storage, MyStruct, read_write> = access %target, %idx
        %11:MyStruct = access %value_param, %idx
        %12:void = call %tint_store_and_preserve_padding_1, %10, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %14:u32 = add %idx, 1u
        next_iteration %14  # -> $B5
      }
    }
    ret
  }
}
%tint_store_and_preserve_padding_1 = func(%target_1:ptr<storage, MyStruct, read_write>, %value_param_1:MyStruct):void {  # %tint_store_and_preserve_padding_1: 'tint_store_and_preserve_padding', %target_1: 'target', %value_param_1: 'value_param'
  $B8: {
    %17:ptr<storage, vec4<u32>, read_write> = access %target_1, 0u
    %18:vec4<u32> = access %value_param_1, 0u
    store %17, %18
    %19:ptr<storage, u32, read_write> = access %target_1, 1u
    %20:u32 = access %value_param_1, 1u
    store %19, %20
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, Mat3x3) {
    auto* mat = ty.mat3x3<f32>();

    auto* buffer = b.Var("buffer", ty.ptr(storage, mat));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", mat);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, mat3x3<f32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:mat3x3<f32>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, mat3x3<f32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:mat3x3<f32>):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, mat3x3<f32>, read_write>, %value_param:mat3x3<f32>):void {
  $B3: {
    %8:ptr<storage, vec3<f32>, read_write> = access %target, 0u
    %9:vec3<f32> = access %value_param, 0u
    store %8, %9
    %10:ptr<storage, vec3<f32>, read_write> = access %target, 1u
    %11:vec3<f32> = access %value_param, 1u
    store %10, %11
    %12:ptr<storage, vec3<f32>, read_write> = access %target, 2u
    %13:vec3<f32> = access %value_param, 2u
    store %12, %13
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, Mat3x3_InStruct) {
    auto* mat = ty.mat3x3<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                                 {mod.symbols.New("b"), mat},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", structure);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  a:mat3x3<f32> @offset(0)
  b:mat3x3<f32> @offset(48)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  a:mat3x3<f32> @offset(0)
  b:mat3x3<f32> @offset(48)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:MyStruct):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, MyStruct, read_write>, %value_param:MyStruct):void {
  $B3: {
    %8:ptr<storage, mat3x3<f32>, read_write> = access %target, 0u
    %9:mat3x3<f32> = access %value_param, 0u
    %10:void = call %tint_store_and_preserve_padding_1, %8, %9
    %12:ptr<storage, mat3x3<f32>, read_write> = access %target, 1u
    %13:mat3x3<f32> = access %value_param, 1u
    %14:void = call %tint_store_and_preserve_padding_1, %12, %13
    ret
  }
}
%tint_store_and_preserve_padding_1 = func(%target_1:ptr<storage, mat3x3<f32>, read_write>, %value_param_1:mat3x3<f32>):void {  # %tint_store_and_preserve_padding_1: 'tint_store_and_preserve_padding', %target_1: 'target', %value_param_1: 'value_param'
  $B4: {
    %17:ptr<storage, vec3<f32>, read_write> = access %target_1, 0u
    %18:vec3<f32> = access %value_param_1, 0u
    store %17, %18
    %19:ptr<storage, vec3<f32>, read_write> = access %target_1, 1u
    %20:vec3<f32> = access %value_param_1, 1u
    store %19, %20
    %21:ptr<storage, vec3<f32>, read_write> = access %target_1, 2u
    %22:vec3<f32> = access %value_param_1, 2u
    store %21, %22
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, Mat3x3_Array) {
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);

    auto* buffer = b.Var("buffer", ty.ptr(storage, arr));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", arr);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<mat3x3<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<mat3x3<f32>, 4>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<mat3x3<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<mat3x3<f32>, 4>):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, array<mat3x3<f32>, 4>, read_write>, %value_param:array<mat3x3<f32>, 4>):void {
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 4u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:ptr<storage, mat3x3<f32>, read_write> = access %target, %idx
        %11:mat3x3<f32> = access %value_param, %idx
        %12:void = call %tint_store_and_preserve_padding_1, %10, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %14:u32 = add %idx, 1u
        next_iteration %14  # -> $B5
      }
    }
    ret
  }
}
%tint_store_and_preserve_padding_1 = func(%target_1:ptr<storage, mat3x3<f32>, read_write>, %value_param_1:mat3x3<f32>):void {  # %tint_store_and_preserve_padding_1: 'tint_store_and_preserve_padding', %target_1: 'target', %value_param_1: 'value_param'
  $B8: {
    %17:ptr<storage, vec3<f32>, read_write> = access %target_1, 0u
    %18:vec3<f32> = access %value_param_1, 0u
    store %17, %18
    %19:ptr<storage, vec3<f32>, read_write> = access %target_1, 1u
    %20:vec3<f32> = access %value_param_1, 1u
    store %19, %20
    %21:ptr<storage, vec3<f32>, read_write> = access %target_1, 2u
    %22:vec3<f32> = access %value_param_1, 2u
    store %21, %22
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, Vec3_Array) {
    auto* arr = ty.array(ty.vec3<f32>(), 4);

    auto* buffer = b.Var("buffer", ty.ptr(storage, arr));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", arr);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec3<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<vec3<f32>, 4>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec3<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<vec3<f32>, 4>):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, array<vec3<f32>, 4>, read_write>, %value_param:array<vec3<f32>, 4>):void {
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 4u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:ptr<storage, vec3<f32>, read_write> = access %target, %idx
        %11:vec3<f32> = access %value_param, %idx
        store %10, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %12:u32 = add %idx, 1u
        next_iteration %12  # -> $B5
      }
    }
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, ComplexNesting) {
    auto* inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("a"), ty.u32()},
                                                {mod.symbols.New("b"), ty.array<vec3<f32>, 4>()},
                                                {mod.symbols.New("c"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("d"), ty.u32()},
                                            });

    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("a"), ty.u32()},
                                                {mod.symbols.New("inner"), inner},
                                                {mod.symbols.New("inner_arr"), ty.array(inner, 4)},
                                                {mod.symbols.New("b"), ty.u32()},
                                            });

    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.array(outer, 3)));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", ty.array(outer, 3));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(buffer, value);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(16) {
  a:u32 @offset(0)
  b:array<vec3<f32>, 4> @offset(16)
  c:mat3x3<f32> @offset(80)
  d:u32 @offset(128)
}

Outer = struct @align(16) {
  a_1:u32 @offset(0)
  inner:Inner @offset(16)
  inner_arr:array<Inner, 4> @offset(160)
  b_1:u32 @offset(736)
}

$B1: {  # root
  %buffer:ptr<storage, array<Outer, 3>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<Outer, 3>):void {
  $B2: {
    store %buffer, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  a:u32 @offset(0)
  b:array<vec3<f32>, 4> @offset(16)
  c:mat3x3<f32> @offset(80)
  d:u32 @offset(128)
}

Outer = struct @align(16) {
  a_1:u32 @offset(0)
  inner:Inner @offset(16)
  inner_arr:array<Inner, 4> @offset(160)
  b_1:u32 @offset(736)
}

$B1: {  # root
  %buffer:ptr<storage, array<Outer, 3>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:array<Outer, 3>):void {
  $B2: {
    %4:void = call %tint_store_and_preserve_padding, %buffer, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, array<Outer, 3>, read_write>, %value_param:array<Outer, 3>):void {
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 3u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:ptr<storage, Outer, read_write> = access %target, %idx
        %11:Outer = access %value_param, %idx
        %12:void = call %tint_store_and_preserve_padding_1, %10, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %14:u32 = add %idx, 1u
        next_iteration %14  # -> $B5
      }
    }
    ret
  }
}
%tint_store_and_preserve_padding_1 = func(%target_1:ptr<storage, Outer, read_write>, %value_param_1:Outer):void {  # %tint_store_and_preserve_padding_1: 'tint_store_and_preserve_padding', %target_1: 'target', %value_param_1: 'value_param'
  $B8: {
    %17:ptr<storage, u32, read_write> = access %target_1, 0u
    %18:u32 = access %value_param_1, 0u
    store %17, %18
    %19:ptr<storage, Inner, read_write> = access %target_1, 1u
    %20:Inner = access %value_param_1, 1u
    %21:void = call %tint_store_and_preserve_padding_2, %19, %20
    %23:ptr<storage, array<Inner, 4>, read_write> = access %target_1, 2u
    %24:array<Inner, 4> = access %value_param_1, 2u
    %25:void = call %tint_store_and_preserve_padding_3, %23, %24
    %27:ptr<storage, u32, read_write> = access %target_1, 3u
    %28:u32 = access %value_param_1, 3u
    store %27, %28
    ret
  }
}
%tint_store_and_preserve_padding_2 = func(%target_2:ptr<storage, Inner, read_write>, %value_param_2:Inner):void {  # %tint_store_and_preserve_padding_2: 'tint_store_and_preserve_padding', %target_2: 'target', %value_param_2: 'value_param'
  $B9: {
    %31:ptr<storage, u32, read_write> = access %target_2, 0u
    %32:u32 = access %value_param_2, 0u
    store %31, %32
    %33:ptr<storage, array<vec3<f32>, 4>, read_write> = access %target_2, 1u
    %34:array<vec3<f32>, 4> = access %value_param_2, 1u
    %35:void = call %tint_store_and_preserve_padding_4, %33, %34
    %37:ptr<storage, mat3x3<f32>, read_write> = access %target_2, 2u
    %38:mat3x3<f32> = access %value_param_2, 2u
    %39:void = call %tint_store_and_preserve_padding_5, %37, %38
    %41:ptr<storage, u32, read_write> = access %target_2, 3u
    %42:u32 = access %value_param_2, 3u
    store %41, %42
    ret
  }
}
%tint_store_and_preserve_padding_4 = func(%target_3:ptr<storage, array<vec3<f32>, 4>, read_write>, %value_param_3:array<vec3<f32>, 4>):void {  # %tint_store_and_preserve_padding_4: 'tint_store_and_preserve_padding', %target_3: 'target', %value_param_3: 'value_param'
  $B10: {
    loop [i: $B11, b: $B12, c: $B13] {  # loop_2
      $B11: {  # initializer
        next_iteration 0u  # -> $B12
      }
      $B12 (%idx_1:u32): {  # body
        %46:bool = gte %idx_1, 4u
        if %46 [t: $B14] {  # if_2
          $B14: {  # true
            exit_loop  # loop_2
          }
        }
        %47:ptr<storage, vec3<f32>, read_write> = access %target_3, %idx_1
        %48:vec3<f32> = access %value_param_3, %idx_1
        store %47, %48
        continue  # -> $B13
      }
      $B13: {  # continuing
        %49:u32 = add %idx_1, 1u
        next_iteration %49  # -> $B12
      }
    }
    ret
  }
}
%tint_store_and_preserve_padding_5 = func(%target_4:ptr<storage, mat3x3<f32>, read_write>, %value_param_4:mat3x3<f32>):void {  # %tint_store_and_preserve_padding_5: 'tint_store_and_preserve_padding', %target_4: 'target', %value_param_4: 'value_param'
  $B15: {
    %52:ptr<storage, vec3<f32>, read_write> = access %target_4, 0u
    %53:vec3<f32> = access %value_param_4, 0u
    store %52, %53
    %54:ptr<storage, vec3<f32>, read_write> = access %target_4, 1u
    %55:vec3<f32> = access %value_param_4, 1u
    store %54, %55
    %56:ptr<storage, vec3<f32>, read_write> = access %target_4, 2u
    %57:vec3<f32> = access %value_param_4, 2u
    store %56, %57
    ret
  }
}
%tint_store_and_preserve_padding_3 = func(%target_5:ptr<storage, array<Inner, 4>, read_write>, %value_param_5:array<Inner, 4>):void {  # %tint_store_and_preserve_padding_3: 'tint_store_and_preserve_padding', %target_5: 'target', %value_param_5: 'value_param'
  $B16: {
    loop [i: $B17, b: $B18, c: $B19] {  # loop_3
      $B17: {  # initializer
        next_iteration 0u  # -> $B18
      }
      $B18 (%idx_2:u32): {  # body
        %61:bool = gte %idx_2, 4u
        if %61 [t: $B20] {  # if_3
          $B20: {  # true
            exit_loop  # loop_3
          }
        }
        %62:ptr<storage, Inner, read_write> = access %target_5, %idx_2
        %63:Inner = access %value_param_5, %idx_2
        %64:void = call %tint_store_and_preserve_padding_2, %62, %63
        continue  # -> $B19
      }
      $B19: {  # continuing
        %65:u32 = add %idx_2, 1u
        next_iteration %65  # -> $B18
      }
    }
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreservePaddingTest, MultipleStoresSameType) {
    auto* mat = ty.mat3x3<f32>();
    auto* arr = ty.array(mat, 4);

    auto* buffer = b.Var("buffer", ty.ptr(storage, arr));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* value = b.FunctionParam("value", mat);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, mat), buffer, 0_u), value);
        b.Store(b.Access(ty.ptr(storage, mat), buffer, 1_u), value);
        b.Store(b.Access(ty.ptr(storage, mat), buffer, 2_u), value);
        b.Store(b.Access(ty.ptr(storage, mat), buffer, 3_u), value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<mat3x3<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:mat3x3<f32>):void {
  $B2: {
    %4:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 0u
    store %4, %value
    %5:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 1u
    store %5, %value
    %6:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 2u
    store %6, %value
    %7:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 3u
    store %7, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<mat3x3<f32>, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%value:mat3x3<f32>):void {
  $B2: {
    %4:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 0u
    %5:void = call %tint_store_and_preserve_padding, %4, %value
    %7:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 1u
    %8:void = call %tint_store_and_preserve_padding, %7, %value
    %9:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 2u
    %10:void = call %tint_store_and_preserve_padding, %9, %value
    %11:ptr<storage, mat3x3<f32>, read_write> = access %buffer, 3u
    %12:void = call %tint_store_and_preserve_padding, %11, %value
    ret
  }
}
%tint_store_and_preserve_padding = func(%target:ptr<storage, mat3x3<f32>, read_write>, %value_param:mat3x3<f32>):void {
  $B3: {
    %15:ptr<storage, vec3<f32>, read_write> = access %target, 0u
    %16:vec3<f32> = access %value_param, 0u
    store %15, %16
    %17:ptr<storage, vec3<f32>, read_write> = access %target, 1u
    %18:vec3<f32> = access %value_param, 1u
    store %17, %18
    %19:ptr<storage, vec3<f32>, read_write> = access %target, 2u
    %20:vec3<f32> = access %value_param, 2u
    store %19, %20
    ret
  }
}
)";

    Run(PreservePadding);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
