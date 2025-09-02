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

#include "src/tint/lang/core/ir/transform/array_length_from_immediate.h"

#include <algorithm>
#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ArrayLengthFromImmediatesTest = TransformTest;

uint32_t GetBufferSizesNumElements(
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_size_index) {
    uint32_t max_index = 0;
    for (auto& entry : bindpoint_to_size_index) {
        max_index = std::max(max_index, entry.second);
    }
    return (max_index / 4) + 1;
}

TEST_F(IR_ArrayLengthFromImmediatesTest, NoModify_UserFunction) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* user_func = b.Function("arrayLength", ty.u32());
    auto* param = b.FunctionParam("arr", arr_ptr);
    user_func->SetParams({param});
    b.Append(user_func->Block(), [&] {  //
        b.Return(user_func, 42_u);
    });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(user_func, buffer);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%arrayLength = func(%arr:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    ret 42u
  }
}
%foo = func():void {
  $B3: {
    %5:u32 = call %arrayLength, %buffer
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%arrayLength = func(%arr:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    ret 42u
  }
}
%foo = func():void {
  $B3: {
    %5:u32 = call %arrayLength, %buffer
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    auto immediate_data = PrepareImmediateData(mod, {});
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), 0u, 0u, bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, DirectUse) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, buffer);
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = arrayLength %buffer
    %let:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:u32 = div %6, 4u
    %8:tint_array_lengths_struct = construct %7
    %9:u32 = access %8, 0u
    %let:u32 = let %9
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, DirectUse_NonZeroIndex) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, buffer);
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = arrayLength %buffer
    %let:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 2> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 2>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 1u
    %6:u32 = load_vector_element %5, 3u
    %7:u32 = div %6, 4u
    %8:tint_array_lengths_struct = construct %7
    %9:u32 = access %8, 0u
    %let:u32 = let %9
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 7;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, DirectUse_NotInMap) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 1);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, buffer);
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = arrayLength %buffer
    %let:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = arrayLength %buffer
    %let:u32 = let %3
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;

    auto immediate_data = PrepareImmediateData(mod, {});
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), 0u, 0u, bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, DirectUse_NoEntryPoint) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    b.Append(bar->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, buffer);
        b.Return(bar, len);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func():u32 {
  $B2: {
    %3:u32 = arrayLength %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%tint_array_lengths:tint_array_lengths_struct):u32 {
  $B2: {
    %4:u32 = access %tint_array_lengths, 0u
    ret %4
  }
}
)";
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    auto immediate_data = PrepareImmediateData(mod, {});
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), 0u, 0u, bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, DirectUse_CalledByEntryPoint) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    b.Append(bar->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, buffer);
        b.Return(bar, len);
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* len = b.Call<u32>(bar);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func():u32 {
  $B2: {
    %3:u32 = arrayLength %buffer
    ret %3
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:u32 = call %bar
    %let:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%bar = func(%tint_array_lengths:tint_array_lengths_struct):u32 {
  $B2: {
    %5:u32 = access %tint_array_lengths, 0u
    ret %5
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %8:ptr<immediate, vec4<u32>, read> = access %7, 0u
    %9:u32 = load_vector_element %8, 0u
    %10:u32 = div %9, 4u
    %11:tint_array_lengths_struct = construct %10
    %12:u32 = call %bar, %11
    %let:u32 = let %12
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaAccess_StructMember) {
    auto* arr = ty.array<i32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), arr},
                                                             });
    auto* arr_ptr = ty.ptr<storage>(arr);
    auto* structure_ptr = ty.ptr<storage>(structure);

    auto* buffer = b.Var("buffer", structure_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, b.Access(arr_ptr, buffer, 0_u));
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:array<i32> @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %buffer, 0u
    %4:u32 = arrayLength %3
    %let:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:array<i32> @offset(0)
}

tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:u32 = sub %6, 0u
    %8:u32 = div %7, 4u
    %9:tint_array_lengths_struct = construct %8
    %10:ptr<storage, array<i32>, read_write> = access %buffer, 0u
    %11:u32 = access %9, 0u
    %let:u32 = let %11
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaAccess_StructMember_NonZeroOffset) {
    auto* arr = ty.array<i32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("u1"), ty.u32()},
                                                                 {mod.symbols.New("u2"), ty.u32()},
                                                                 {mod.symbols.New("u3"), ty.u32()},
                                                                 {mod.symbols.New("u4"), ty.u32()},
                                                                 {mod.symbols.New("u5"), ty.u32()},
                                                                 {mod.symbols.New("a"), arr},
                                                             });
    auto* arr_ptr = ty.ptr<storage>(arr);
    auto* structure_ptr = ty.ptr<storage>(structure);

    auto* buffer = b.Var("buffer", structure_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, b.Access(arr_ptr, buffer, 5_u));
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  u1:u32 @offset(0)
  u2:u32 @offset(4)
  u3:u32 @offset(8)
  u4:u32 @offset(12)
  u5:u32 @offset(16)
  a:array<i32> @offset(20)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %buffer, 5u
    %4:u32 = arrayLength %3
    %let:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  u1:u32 @offset(0)
  u2:u32 @offset(4)
  u3:u32 @offset(8)
  u4:u32 @offset(12)
  u5:u32 @offset(16)
  a:array<i32> @offset(20)
}

tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:u32 = sub %6, 20u
    %8:u32 = div %7, 4u
    %9:tint_array_lengths_struct = construct %8
    %10:ptr<storage, array<i32>, read_write> = access %buffer, 5u
    %11:u32 = access %9, 0u
    %let:u32 = let %11
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaLet) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, b.Let("let", buffer));
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %let:ptr<storage, array<i32>, read_write> = let %buffer
    %4:u32 = arrayLength %let
    %let_1:u32 = let %4  # %let_1: 'let'
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:u32 = div %6, 4u
    %8:tint_array_lengths_struct = construct %7
    %let:ptr<storage, array<i32>, read_write> = let %buffer
    %10:u32 = access %8, 0u
    %let_1:u32 = let %10  # %let_1: 'let'
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaParameter) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    auto* param = b.FunctionParam("param", arr_ptr);
    bar->SetParams({param});
    b.Append(bar->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, param);
        b.Return(bar, len);
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* len = b.Call<u32>(bar, buffer);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    %4:u32 = arrayLength %param
    ret %4
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %6:u32 = call %bar, %buffer
    %let:u32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%bar = func(%param:ptr<storage, array<i32>, read_write>, %tint_array_length:u32):u32 {
  $B2: {
    ret %tint_array_length
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %8:ptr<immediate, vec4<u32>, read> = access %7, 0u
    %9:u32 = load_vector_element %8, 0u
    %10:u32 = div %9, 4u
    %11:tint_array_lengths_struct = construct %10
    %12:u32 = access %11, 0u
    %13:u32 = call %bar, %buffer, %12
    %let:u32 = let %13
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaParameterChain) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* zoo = b.Function("foo", ty.u32());
    auto* param_zoo = b.FunctionParam("param_zoo", arr_ptr);
    zoo->SetParams({param_zoo});
    b.Append(zoo->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, param_zoo);
        b.Return(zoo, len);
    });

    auto* bar = b.Function("foo", ty.u32());
    auto* param_bar = b.FunctionParam("param_bar", arr_ptr);
    bar->SetParams({param_bar});
    b.Append(bar->Block(), [&] {
        auto* len = b.Call<u32>(zoo, param_bar);
        b.Return(bar, len);
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* len = b.Call<u32>(bar, buffer);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%param_zoo:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    %4:u32 = arrayLength %param_zoo
    ret %4
  }
}
%foo_1 = func(%param_bar:ptr<storage, array<i32>, read_write>):u32 {  # %foo_1: 'foo'
  $B3: {
    %7:u32 = call %foo, %param_bar
    ret %7
  }
}
%foo_2 = @compute @workgroup_size(1u, 1u, 1u) func():void {  # %foo_2: 'foo'
  $B4: {
    %9:u32 = call %foo_1, %buffer
    %let:u32 = let %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = func(%param_zoo:ptr<storage, array<i32>, read_write>, %tint_array_length:u32):u32 {
  $B2: {
    ret %tint_array_length
  }
}
%foo_1 = func(%param_bar:ptr<storage, array<i32>, read_write>, %tint_array_length_1:u32):u32 {  # %foo_1: 'foo', %tint_array_length_1: 'tint_array_length'
  $B3: {
    %9:u32 = call %foo, %param_bar, %tint_array_length_1
    ret %9
  }
}
%foo_2 = @compute @workgroup_size(1u, 1u, 1u) func():void {  # %foo_2: 'foo'
  $B4: {
    %11:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %12:ptr<immediate, vec4<u32>, read> = access %11, 0u
    %13:u32 = load_vector_element %12, 0u
    %14:u32 = div %13, 4u
    %15:tint_array_lengths_struct = construct %14
    %16:u32 = access %15, 0u
    %17:u32 = call %foo_1, %buffer, %16
    %let:u32 = let %17
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

// Test that when we arrayLength is called on a parameter but the originating variable at the
// callsite is not in the bindpoint map, we reintroduce an arrayLength call instead of passing
// undef to the callee.
TEST_F(IR_ArrayLengthFromImmediatesTest, ViaParameter_NotInMap) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    auto* param = b.FunctionParam("param", arr_ptr);
    bar->SetParams({param});
    b.Append(bar->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, param);
        b.Return(bar, len);
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* len = b.Call<u32>(bar, buffer);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    %4:u32 = arrayLength %param
    ret %4
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %6:u32 = call %bar, %buffer
    %let:u32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<i32>, read_write>, %tint_array_length:u32):u32 {
  $B2: {
    ret %tint_array_length
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %6:u32 = arrayLength %buffer
    %7:u32 = call %bar, %buffer, %6
    %let:u32 = let %7
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    auto immediate_data = PrepareImmediateData(mod, {});
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), 0u, 0u, bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

// Test that we reuse the length parameter for multiple arrayLength calls on the same parameter.
TEST_F(IR_ArrayLengthFromImmediatesTest, ViaParameter_MultipleCallsSameParameter) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    auto* param = b.FunctionParam("param", arr_ptr);
    bar->SetParams({param});
    b.Append(bar->Block(), [&] {
        auto* len_a = b.Call<u32>(BuiltinFn::kArrayLength, param);
        auto* len_b = b.Call<u32>(BuiltinFn::kArrayLength, param);
        auto* len_c = b.Call<u32>(BuiltinFn::kArrayLength, param);
        b.Return(bar, b.Add<u32>(len_a, b.Add<u32>(len_b, len_c)));
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* len = b.Call<u32>(bar, buffer);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    %4:u32 = arrayLength %param
    %5:u32 = arrayLength %param
    %6:u32 = arrayLength %param
    %7:u32 = add %5, %6
    %8:u32 = add %4, %7
    ret %8
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %10:u32 = call %bar, %buffer
    %let:u32 = let %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%bar = func(%param:ptr<storage, array<i32>, read_write>, %tint_array_length:u32):u32 {
  $B2: {
    %6:u32 = add %tint_array_length, %tint_array_length
    %7:u32 = add %tint_array_length, %6
    ret %7
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %9:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %10:ptr<immediate, vec4<u32>, read> = access %9, 0u
    %11:u32 = load_vector_element %10, 0u
    %12:u32 = div %11, 4u
    %13:tint_array_lengths_struct = construct %12
    %14:u32 = access %13, 0u
    %15:u32 = call %bar, %buffer, %14
    %let:u32 = let %15
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaParameter_MultipleCallsDifferentParameters) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    auto* param_a = b.FunctionParam("param_a", arr_ptr);
    auto* param_b = b.FunctionParam("param_b", arr_ptr);
    auto* param_c = b.FunctionParam("param_c", arr_ptr);
    bar->SetParams({param_a, param_b, param_c});
    b.Append(bar->Block(), [&] {
        auto* len_a = b.Call<u32>(BuiltinFn::kArrayLength, param_a);
        auto* len_b = b.Call<u32>(BuiltinFn::kArrayLength, param_b);
        auto* len_c = b.Call<u32>(BuiltinFn::kArrayLength, param_c);
        b.Return(bar, b.Add<u32>(len_a, b.Add<u32>(len_b, len_c)));
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* len = b.Call<u32>(bar, buffer, buffer, buffer);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param_a:ptr<storage, array<i32>, read_write>, %param_b:ptr<storage, array<i32>, read_write>, %param_c:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    %6:u32 = arrayLength %param_a
    %7:u32 = arrayLength %param_b
    %8:u32 = arrayLength %param_c
    %9:u32 = add %7, %8
    %10:u32 = add %6, %9
    ret %10
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %12:u32 = call %bar, %buffer, %buffer, %buffer
    %let:u32 = let %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%bar = func(%param_a:ptr<storage, array<i32>, read_write>, %param_b:ptr<storage, array<i32>, read_write>, %param_c:ptr<storage, array<i32>, read_write>, %tint_array_length:u32, %tint_array_length_1:u32, %tint_array_length_2:u32):u32 {  # %tint_array_length_1: 'tint_array_length', %tint_array_length_2: 'tint_array_length'
  $B2: {
    %10:u32 = add %tint_array_length_1, %tint_array_length_2
    %11:u32 = add %tint_array_length, %10
    ret %11
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %13:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %14:ptr<immediate, vec4<u32>, read> = access %13, 0u
    %15:u32 = load_vector_element %14, 0u
    %16:u32 = div %15, 4u
    %17:tint_array_lengths_struct = construct %16
    %18:u32 = access %17, 0u
    %19:u32 = access %17, 0u
    %20:u32 = access %17, 0u
    %21:u32 = call %bar, %buffer, %buffer, %buffer, %18, %19, %20
    %let:u32 = let %21
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);
    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ViaComplexChain) {
    auto* arr = ty.array<i32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("u1"), ty.u32()},
                                                                 {mod.symbols.New("a"), arr},
                                                             });
    auto* arr_ptr = ty.ptr<storage>(arr);
    auto* structure_ptr = ty.ptr<storage>(structure);

    auto* buffer = b.Var("buffer", structure_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* bar = b.Function("bar", ty.u32());
    auto* param = b.FunctionParam("param", arr_ptr);
    bar->SetParams({param});
    b.Append(bar->Block(), [&] {
        auto* let1 = b.Let("let1", param);
        auto* let2 = b.Let("let2", let1);
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, let2);
        b.Return(bar, len);
    });

    auto* foo = b.ComputeFunction("foo");
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(arr_ptr, buffer, 1_u);
        auto* let = b.Let("let", access);
        auto* len = b.Call<u32>(bar, let);
        b.Let("let", len);
        b.Return(foo);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  u1:u32 @offset(0)
  a:array<i32> @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<i32>, read_write>):u32 {
  $B2: {
    %let1:ptr<storage, array<i32>, read_write> = let %param
    %let2:ptr<storage, array<i32>, read_write> = let %let1
    %6:u32 = arrayLength %let2
    ret %6
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %8:ptr<storage, array<i32>, read_write> = access %buffer, 1u
    %let:ptr<storage, array<i32>, read_write> = let %8
    %10:u32 = call %bar, %let
    %let_1:u32 = let %10  # %let_1: 'let'
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  u1:u32 @offset(0)
  a:array<i32> @offset(4)
}

tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%bar = func(%param:ptr<storage, array<i32>, read_write>, %tint_array_length:u32):u32 {
  $B2: {
    %let1:ptr<storage, array<i32>, read_write> = let %param
    %let2:ptr<storage, array<i32>, read_write> = let %let1
    ret %tint_array_length
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %9:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %10:ptr<immediate, vec4<u32>, read> = access %9, 0u
    %11:u32 = load_vector_element %10, 0u
    %12:u32 = sub %11, 4u
    %13:u32 = div %12, 4u
    %14:tint_array_lengths_struct = construct %13
    %15:ptr<storage, array<i32>, read_write> = access %buffer, 1u
    %let:ptr<storage, array<i32>, read_write> = let %15
    %17:u32 = access %14, 0u
    %18:u32 = call %bar, %let, %17
    %let_1:u32 = let %18  # %let_1: 'let'
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);
    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, ElementStrideLargerThanSize) {
    auto* arr = ty.array<vec3<i32>>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer = b.Var("buffer", arr_ptr);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* len = b.Call<u32>(BuiltinFn::kArrayLength, buffer);
        b.Let("let", len);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec3<i32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = arrayLength %buffer
    %let:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 1> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, array<vec3<i32>>, read_write> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 1>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:u32 = div %6, 16u
    %8:tint_array_lengths_struct = construct %7
    %9:u32 = access %8, 0u
    %let:u32 = let %9
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);
    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayLengthFromImmediatesTest, MultipleVars) {
    auto* arr = ty.array<i32>();
    auto* arr_ptr = ty.ptr<storage>(arr);

    auto* buffer_a = b.Var("buffer_a", arr_ptr);
    auto* buffer_b = b.Var("buffer_b", arr_ptr);
    auto* buffer_c = b.Var("buffer_c", arr_ptr);
    auto* buffer_d = b.Var("buffer_d", arr_ptr);
    auto* buffer_e = b.Var("buffer_e", arr_ptr);
    buffer_a->SetBindingPoint(0, 0);
    buffer_b->SetBindingPoint(0, 1);
    buffer_c->SetBindingPoint(1, 0);
    buffer_d->SetBindingPoint(1, 1);
    buffer_e->SetBindingPoint(2, 3);
    mod.root_block->Append(buffer_a);
    mod.root_block->Append(buffer_b);
    mod.root_block->Append(buffer_c);
    mod.root_block->Append(buffer_d);
    mod.root_block->Append(buffer_e);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call<u32>(BuiltinFn::kArrayLength, buffer_a);
        b.Call<u32>(BuiltinFn::kArrayLength, buffer_b);
        b.Call<u32>(BuiltinFn::kArrayLength, buffer_c);
        b.Call<u32>(BuiltinFn::kArrayLength, buffer_d);
        b.Call<u32>(BuiltinFn::kArrayLength, buffer_e);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer_a:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %buffer_b:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 1)
  %buffer_c:ptr<storage, array<i32>, read_write> = var undef @binding_point(1, 0)
  %buffer_d:ptr<storage, array<i32>, read_write> = var undef @binding_point(1, 1)
  %buffer_e:ptr<storage, array<i32>, read_write> = var undef @binding_point(2, 3)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %7:u32 = arrayLength %buffer_a
    %8:u32 = arrayLength %buffer_b
    %9:u32 = arrayLength %buffer_c
    %10:u32 = arrayLength %buffer_d
    %11:u32 = arrayLength %buffer_e
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_storage_buffer_sizes:array<vec4<u32>, 2> @offset(16)
}

tint_array_lengths_struct = struct @align(4) {
  tint_array_length_0_0:u32 @offset(0)
  tint_array_length_0_1:u32 @offset(4)
  tint_array_length_1_0:u32 @offset(8)
  tint_array_length_1_1:u32 @offset(12)
  tint_array_length_2_3:u32 @offset(16)
}

$B1: {  # root
  %buffer_a:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
  %buffer_b:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 1)
  %buffer_c:ptr<storage, array<i32>, read_write> = var undef @binding_point(1, 0)
  %buffer_d:ptr<storage, array<i32>, read_write> = var undef @binding_point(1, 1)
  %buffer_e:ptr<storage, array<i32>, read_write> = var undef @binding_point(2, 3)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %8:ptr<immediate, array<vec4<u32>, 2>, read> = access %tint_immediate_data, 0u
    %9:ptr<immediate, vec4<u32>, read> = access %8, 0u
    %10:u32 = load_vector_element %9, 0u
    %11:u32 = div %10, 4u
    %12:ptr<immediate, array<vec4<u32>, 2>, read> = access %tint_immediate_data, 0u
    %13:ptr<immediate, vec4<u32>, read> = access %12, 1u
    %14:u32 = load_vector_element %13, 1u
    %15:u32 = div %14, 4u
    %16:ptr<immediate, array<vec4<u32>, 2>, read> = access %tint_immediate_data, 0u
    %17:ptr<immediate, vec4<u32>, read> = access %16, 0u
    %18:u32 = load_vector_element %17, 3u
    %19:u32 = div %18, 4u
    %20:ptr<immediate, array<vec4<u32>, 2>, read> = access %tint_immediate_data, 0u
    %21:ptr<immediate, vec4<u32>, read> = access %20, 0u
    %22:u32 = load_vector_element %21, 2u
    %23:u32 = div %22, 4u
    %24:ptr<immediate, array<vec4<u32>, 2>, read> = access %tint_immediate_data, 0u
    %25:ptr<immediate, vec4<u32>, read> = access %24, 1u
    %26:u32 = load_vector_element %25, 0u
    %27:u32 = div %26, 4u
    %28:tint_array_lengths_struct = construct %11, %15, %19, %23, %27
    %29:u32 = access %28, 0u
    %30:u32 = access %28, 1u
    %31:u32 = access %28, 2u
    %32:u32 = access %28, 3u
    %33:u32 = access %28, 4u
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_index;
    bindpoint_to_index[{0, 0}] = 0;
    bindpoint_to_index[{0, 1}] = 5;
    bindpoint_to_index[{1, 0}] = 3;
    bindpoint_to_index[{1, 1}] = 2;
    bindpoint_to_index[{2, 3}] = 4;

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    constexpr uint32_t buffer_size_start_offset = 16;
    uint32_t num_elements = GetBufferSizesNumElements(bindpoint_to_index);
    immediate_data_config.AddInternalImmediateData(buffer_size_start_offset,
                                                   mod.symbols.New("tint_storage_buffer_sizes"),
                                                   ty.array(ty.vec4<u32>(), num_elements));
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);
    Run(ArrayLengthFromImmediates, immediate_data.Get(), buffer_size_start_offset, num_elements,
        bindpoint_to_index);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
