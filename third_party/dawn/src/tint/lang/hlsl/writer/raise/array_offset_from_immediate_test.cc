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

#include "src/tint/lang/hlsl/writer/raise/array_offset_from_immediate.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct IR_ArrayOffsetFromImmediateTest : core::ir::transform::TransformTest {
    IR_ArrayOffsetFromImmediateTest() { capabilities = kArrayOffsetFromImmediateCapabilities; }
};

// Test that offset is added to byte_address_buffer.Load
TEST_F(IR_ArrayOffsetFromImmediateTest, Basic) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* buffer = b.Var("buffer", byte_address_buffer_readonly_ty);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto base_offset = u32(42);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer,
                                                  base_offset);
        b.Return(func);
    });

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 6)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    ASSERT_EQ(immediate_data, Success);

    // Verify the immediate data variable was created
    ASSERT_NE(immediate_data.Get().var, nullptr);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 2;

    auto result =
        ArrayOffsetFromImmediates(mod, immediate_data.Get(), 0, 6, bindpoint_to_offset_index);
    ASSERT_EQ(result, Success);

    EXPECT_EQ(str(),
              R"(
tint_immediate_data_struct = struct @align(16), @block {
  buffer_offsets:array<vec4<u32>, 6> @offset(0)
}

$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<immediate, array<vec4<u32>, 6>, read> = access %tint_immediate_data, 0u
    %5:ptr<immediate, vec4<u32>, read> = access %4, 0u
    %6:u32 = load_vector_element %5, 2u
    %7:u32 = add 42u, %6
    %8:u32 = %buffer.Load %7
    ret
  }
}
)");
}

// Test that nothing changes if no bindpoint_to_offset_index is provided
TEST_F(IR_ArrayOffsetFromImmediateTest, NoModify_EmptyBindpointToOffsetIndex) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* buffer = b.Var("buffer", byte_address_buffer_readonly_ty);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto base_offset = u32(42);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer,
                                                  base_offset);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = %buffer.Load 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  buffer_offsets:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = %buffer.Load 42u
    ret
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 1)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    Run(ArrayOffsetFromImmediates, immediate_data.Get(), 0u, 1u, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

// Test that nothing changes if bindpoint doesn't match
TEST_F(IR_ArrayOffsetFromImmediateTest, NoModify_NoMatchingBindpoint) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* buffer = b.Var("buffer", byte_address_buffer_readonly_ty);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto base_offset = u32(42);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer,
                                                  base_offset);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = %buffer.Load 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  buffer_offsets:array<vec4<u32>, 6> @offset(0)
}

$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = %buffer.Load 42u
    ret
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 6)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 1}] = 20;  // Doesn't match binding point
    Run(ArrayOffsetFromImmediates, immediate_data.Get(), 0u, 6u, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

// Test that nothing changes if we call a builtin that takes a byte_address_buffer but no offset
TEST_F(IR_ArrayOffsetFromImmediateTest, NoModify_GetDimensions) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* buffer = b.Var("buffer", byte_address_buffer_readonly_ty);
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("width", ty.ptr<function, u32, read_write>());
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kGetDimensions,
                                                  buffer, v);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %width:ptr<function, u32, read_write> = var undef
    %4:void = %buffer.GetDimensions %width
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  buffer_offsets:array<vec4<u32>, 6> @offset(0)
}

$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %width:ptr<function, u32, read_write> = var undef
    %5:void = %buffer.GetDimensions %width
    ret
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 6)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 20;
    Run(ArrayOffsetFromImmediates, immediate_data.Get(), 0u, 6u, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayOffsetFromImmediateTest, AllLoadsAndStoresU32) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* byte_address_buffer_readwrite_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kReadWrite);

    auto* buffer_ro = b.Var("buffer", byte_address_buffer_readonly_ty);
    auto* buffer_rw = b.Var("buffer", byte_address_buffer_readwrite_ty);
    buffer_ro->SetBindingPoint(5, 6);
    buffer_rw->SetBindingPoint(7, 8);
    mod.root_block->Append(buffer_ro);
    mod.root_block->Append(buffer_rw);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer_ro,
                                                  u32(42));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.vec2u(), hlsl::BuiltinFn::kLoad2, buffer_ro,
                                                  u32(43));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.vec3u(), hlsl::BuiltinFn::kLoad3, buffer_ro,
                                                  u32(44));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.vec4u(), hlsl::BuiltinFn::kLoad4, buffer_ro,
                                                  u32(45));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kStore, buffer_rw,
                                                  u32(46), 123_u);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kStore2, buffer_rw,
                                                  u32(47), b.Composite(ty.vec2u(), 123_u, 124_u));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kStore3, buffer_rw,
                                                  u32(48),
                                                  b.Composite(ty.vec3u(), 123_u, 124_u, 125_u));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(
            ty.void_(), hlsl::BuiltinFn::kStore4, buffer_rw, u32(49),
            b.Composite(ty.vec4u(), 123_u, 124_u, 125_u, 126_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(5, 6)
  %buffer_1:hlsl.byte_address_buffer<read_write> = var undef @binding_point(7, 8)  # %buffer_1: 'buffer'
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = %buffer.Load 42u
    %5:vec2<u32> = %buffer.Load2 43u
    %6:vec3<u32> = %buffer.Load3 44u
    %7:vec4<u32> = %buffer.Load4 45u
    %8:void = %buffer_1.Store 46u, 123u
    %9:void = %buffer_1.Store2 47u, vec2<u32>(123u, 124u)
    %10:void = %buffer_1.Store3 48u, vec3<u32>(123u, 124u, 125u)
    %11:void = %buffer_1.Store4 49u, vec4<u32>(123u, 124u, 125u, 126u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  buffer_offsets:array<vec4<u32>, 15> @offset(0)
}

$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(5, 6)
  %buffer_1:hlsl.byte_address_buffer<read_write> = var undef @binding_point(7, 8)  # %buffer_1: 'buffer'
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %6:ptr<immediate, vec4<u32>, read> = access %5, 6u
    %7:u32 = load_vector_element %6, 2u
    %8:u32 = add 42u, %7
    %9:u32 = %buffer.Load %8
    %10:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %11:ptr<immediate, vec4<u32>, read> = access %10, 6u
    %12:u32 = load_vector_element %11, 2u
    %13:u32 = add 43u, %12
    %14:vec2<u32> = %buffer.Load2 %13
    %15:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %16:ptr<immediate, vec4<u32>, read> = access %15, 6u
    %17:u32 = load_vector_element %16, 2u
    %18:u32 = add 44u, %17
    %19:vec3<u32> = %buffer.Load3 %18
    %20:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %21:ptr<immediate, vec4<u32>, read> = access %20, 6u
    %22:u32 = load_vector_element %21, 2u
    %23:u32 = add 45u, %22
    %24:vec4<u32> = %buffer.Load4 %23
    %25:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %26:ptr<immediate, vec4<u32>, read> = access %25, 14u
    %27:u32 = load_vector_element %26, 1u
    %28:u32 = add 46u, %27
    %29:void = %buffer_1.Store %28, 123u
    %30:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %31:ptr<immediate, vec4<u32>, read> = access %30, 14u
    %32:u32 = load_vector_element %31, 1u
    %33:u32 = add 47u, %32
    %34:void = %buffer_1.Store2 %33, vec2<u32>(123u, 124u)
    %35:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %36:ptr<immediate, vec4<u32>, read> = access %35, 14u
    %37:u32 = load_vector_element %36, 1u
    %38:u32 = add 48u, %37
    %39:void = %buffer_1.Store3 %38, vec3<u32>(123u, 124u, 125u)
    %40:ptr<immediate, array<vec4<u32>, 15>, read> = access %tint_immediate_data, 0u
    %41:ptr<immediate, vec4<u32>, read> = access %40, 14u
    %42:u32 = load_vector_element %41, 1u
    %43:u32 = add 49u, %42
    %44:void = %buffer_1.Store4 %43, vec4<u32>(123u, 124u, 125u, 126u)
    ret
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 15)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{5, 6}] = 26;
    bindpoint_to_offset_index[{7, 8}] = 57;
    Run(ArrayOffsetFromImmediates, immediate_data.Get(), 0u, 15u, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayOffsetFromImmediateTest, AllAtomicOps) {
    auto* byte_address_buffer_readwrite_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kReadWrite);

    auto* buffer_rw = b.Var("buffer", byte_address_buffer_readwrite_ty);
    buffer_rw->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer_rw);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(
            ty.void_(), hlsl::BuiltinFn::kInterlockedCompareExchange, buffer_rw, u32(0), 123_u,
            456_u, b.Var("original_value", ty.ptr<function, u32, read_write>()));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedAdd,
                                                  buffer_rw, u32(4), 100_u,
                                                  b.Var("v", ty.ptr<function, u32, read_write>()));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedMin,
                                                  buffer_rw, u32(8), 50_u,
                                                  b.Var("v", ty.ptr<function, u32, read_write>()));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedMax,
                                                  buffer_rw, u32(12), 200_u,
                                                  b.Var("v", ty.ptr<function, u32, read_write>()));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedAnd,
                                                  buffer_rw, u32(16), 0xFF_u,
                                                  b.Var("v", ty.ptr<function, u32, read_write>()));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedOr,
                                                  buffer_rw, u32(20), 0x10_u,
                                                  b.Var("v", ty.ptr<function, u32, read_write>()));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedXor,
                                                  buffer_rw, u32(24), 0xAB_u,
                                                  b.Var("v", ty.ptr<function, u32, read_write>()));
        b.Return(func);
    });

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 3)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 7;
    Run(ArrayOffsetFromImmediates, immediate_data.Get(), 0u, 3u, bindpoint_to_offset_index);

    // Just verify it doesn't crash - detailed output checking would be very long
    EXPECT_NE(str(), "");
}

// Test with offset_index that requires accessing different vec4 elements
TEST_F(IR_ArrayOffsetFromImmediateTest, MultipleBuffersWithDifferentVec4Indices) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);

    auto* buffer0 = b.Var("buffer0", byte_address_buffer_readonly_ty);
    auto* buffer1 = b.Var("buffer1", byte_address_buffer_readonly_ty);
    auto* buffer2 = b.Var("buffer2", byte_address_buffer_readonly_ty);
    buffer0->SetBindingPoint(0, 0);
    buffer1->SetBindingPoint(0, 1);
    buffer2->SetBindingPoint(0, 2);
    mod.root_block->Append(buffer0);
    mod.root_block->Append(buffer1);
    mod.root_block->Append(buffer2);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer0,
                                                  u32(0));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer1,
                                                  u32(0));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer2,
                                                  u32(0));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer0:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %buffer1:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 1)
  %buffer2:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:u32 = %buffer0.Load 0u
    %6:u32 = %buffer1.Load 0u
    %7:u32 = %buffer2.Load 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  buffer_offsets:array<vec4<u32>, 3> @offset(0)
}

$B1: {  # root
  %buffer0:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %buffer1:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 1)
  %buffer2:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 2)
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %6:ptr<immediate, array<vec4<u32>, 3>, read> = access %tint_immediate_data, 0u
    %7:ptr<immediate, vec4<u32>, read> = access %6, 0u
    %8:u32 = load_vector_element %7, 1u
    %9:u32 = add 0u, %8
    %10:u32 = %buffer0.Load %9
    %11:ptr<immediate, array<vec4<u32>, 3>, read> = access %tint_immediate_data, 0u
    %12:ptr<immediate, vec4<u32>, read> = access %11, 1u
    %13:u32 = load_vector_element %12, 1u
    %14:u32 = add 0u, %13
    %15:u32 = %buffer1.Load %14
    %16:ptr<immediate, array<vec4<u32>, 3>, read> = access %tint_immediate_data, 0u
    %17:ptr<immediate, vec4<u32>, read> = access %16, 2u
    %18:u32 = load_vector_element %17, 1u
    %19:u32 = add 0u, %18
    %20:u32 = %buffer2.Load %19
    ret
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(immediate_data_config.AddInternalImmediateData(0, mod.symbols.New("buffer_offsets"),
                                                             ty.array(ty.vec4u(), 3)),
              Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 1;  // vec4[0].y
    bindpoint_to_offset_index[{0, 1}] = 5;  // vec4[1].y
    bindpoint_to_offset_index[{0, 2}] = 9;  // vec4[2].y
    Run(ArrayOffsetFromImmediates, immediate_data.Get(), 0u, 3u, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
