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

#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_BlockDecoratedStructsTest = TransformTest;

TEST_F(IR_BlockDecoratedStructsTest, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, Scalar_Uniform) {
    auto* buffer = b.Var(ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.i32());

    auto* block = func->Block();
    auto* load = block->Append(b.Load(buffer));
    block->Append(b.Return(func, load));

    auto* expect = R"(
tint_symbol = struct @align(4), @block {
  inner:i32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol, read> = var @binding_point(0, 0)
}

%foo = func():i32 {
  $B2: {
    %3:ptr<uniform, i32, read> = access %1, 0u
    %4:i32 = load %3
    ret %4
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, Scalar_Storage) {
    auto* buffer = b.Var(ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Store(buffer, 42_i));
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
tint_symbol = struct @align(4), @block {
  inner:i32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol, read_write> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, i32, read_write> = access %1, 0u
    store %3, 42i
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, Scalar_PushConstant) {
    auto* buffer = b.Var(ty.ptr<push_constant, i32>());
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(buffer));
    });

    auto* expect = R"(
tint_symbol = struct @align(4), @block {
  inner:i32 @offset(0)
}

$B1: {  # root
  %1:ptr<push_constant, tint_symbol, read> = var
}

%foo = func():i32 {
  $B2: {
    %3:ptr<push_constant, i32, read> = access %1, 0u
    %4:i32 = load %3
    ret %4
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, RuntimeArray) {
    auto* buffer = b.Var(ty.ptr<storage, array<i32>>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, i32>(), buffer, 1_u);
        b.Store(access, 42_i);
        b.Return(func);
    });

    auto* expect = R"(
tint_symbol = struct @align(4), @block {
  inner:array<i32> @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol, read_write> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %1, 0u
    %4:ptr<storage, i32, read_write> = access %3, 1u
    store %4, 42i
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, RuntimeArray_InStruct) {
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("i"), ty.i32()},
                                                   {mod.symbols.New("arr"), ty.array<i32>()},
                                               });

    auto* buffer = b.Var(ty.ptr(storage, structure, core::Access::kReadWrite));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* i32_ptr = ty.ptr<storage, i32>();

    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* val_ptr = b.Access(i32_ptr, buffer, 0_u);
        auto* load = b.Load(val_ptr);
        auto* elem_ptr = b.Access(i32_ptr, buffer, 1_u, 3_u);
        b.Store(elem_ptr, load);
        b.Return(func);
    });

    auto* expect = R"(
MyStruct = struct @align(4), @block {
  i:i32 @offset(0)
  arr:array<i32> @offset(4)
}

$B1: {  # root
  %1:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, i32, read_write> = access %1, 0u
    %4:i32 = load %3
    %5:ptr<storage, i32, read_write> = access %1, 1u, 3u
    store %5, %4
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, RuntimeArray_InStruct_ArrayLengthViaLets) {
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("i"), ty.i32()},
                                                   {mod.symbols.New("arr"), ty.array<i32>()},
                                               });

    auto* buffer = b.Var(ty.ptr(storage, structure, core::Access::kReadWrite));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* let_root = b.Let("root", buffer->Result(0));
        auto* let_arr = b.Let("arr", b.Access(ty.ptr(storage, ty.array<i32>()), let_root, 1_u));
        auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, let_arr);
        b.Return(func, length);
    });

    auto* expect = R"(
MyStruct = struct @align(4), @block {
  i:i32 @offset(0)
  arr:array<i32> @offset(4)
}

$B1: {  # root
  %1:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %root:ptr<storage, MyStruct, read_write> = let %1
    %4:ptr<storage, array<i32>, read_write> = access %root, 1u
    %arr:ptr<storage, array<i32>, read_write> = let %4
    %6:u32 = arrayLength %arr
    ret %6
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, StructUsedElsewhere) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.i32()},
                                                                 {mod.symbols.New("b"), ty.i32()},
                                                             });

    auto* buffer = b.Var(ty.ptr(storage, structure, core::Access::kReadWrite));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* private_var = b.Var(ty.ptr<private_, read_write>(structure));
    mod.root_block->Append(private_var);

    auto* func = b.Function("foo", ty.void_());
    auto* load = func->Block()->Append(b.Load(private_var));
    func->Block()->Append(b.Store(buffer, load));
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

tint_symbol = struct @align(4), @block {
  inner:MyStruct @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol, read_write> = var @binding_point(0, 0)
  %2:ptr<private, MyStruct, read_write> = var
}

%foo = func():void {
  $B2: {
    %4:MyStruct = load %2
    %5:ptr<storage, MyStruct, read_write> = access %1, 0u
    store %5, %4
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, MultipleBuffers) {
    auto* buffer_a = b.Var(ty.ptr<storage, i32>());
    auto* buffer_b = b.Var(ty.ptr<storage, i32>());
    auto* buffer_c = b.Var(ty.ptr<storage, i32>());
    buffer_a->SetBindingPoint(0, 0);
    buffer_b->SetBindingPoint(0, 1);
    buffer_c->SetBindingPoint(0, 2);
    auto* root = mod.root_block.Get();
    root->Append(buffer_a);
    root->Append(buffer_b);
    root->Append(buffer_c);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* load_b = b.Load(buffer_b);
        auto* load_c = b.Load(buffer_c);
        b.Store(buffer_a, b.Add(ty.i32(), load_b, load_c));
        b.Return(func);
    });

    auto* expect = R"(
tint_symbol = struct @align(4), @block {
  inner:i32 @offset(0)
}

tint_symbol_1 = struct @align(4), @block {
  inner:i32 @offset(0)
}

tint_symbol_2 = struct @align(4), @block {
  inner:i32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol, read_write> = var @binding_point(0, 0)
  %2:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 1)
  %3:ptr<storage, tint_symbol_2, read_write> = var @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %2, 0u
    %6:i32 = load %5
    %7:ptr<storage, i32, read_write> = access %3, 0u
    %8:i32 = load %7
    %9:i32 = add %6, %8
    %10:ptr<storage, i32, read_write> = access %1, 0u
    store %10, %9
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, PushConstantAlreadyHasBlockAttribute) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("i"), ty.i32()},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var(ty.ptr(push_constant, structure));
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* val_ptr = b.Access<ptr<push_constant, i32>>(buffer, 0_u);
        b.Return(func, b.Load(val_ptr));
    });

    auto* src = R"(
MyStruct = struct @align(4), @block {
  i:i32 @offset(0)
}

$B1: {  # root
  %1:ptr<push_constant, MyStruct, read> = var
}

%foo = func():i32 {
  $B2: {
    %3:ptr<push_constant, i32, read> = access %1, 0u
    %4:i32 = load %3
    ret %4
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, PropagateVarName) {
    auto* buffer = b.Var("my_var", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Store(buffer, 42_i));
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
my_var_block = struct @align(4), @block {
  inner:i32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, my_var_block, read_write> = var @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, i32, read_write> = access %1, 0u
    store %3, 42i
    ret
  }
}
)";

    Run(BlockDecoratedStructs);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
