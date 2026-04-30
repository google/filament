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

#include "src/tint/lang/hlsl/writer/raise/array_offset_from_uniform.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct IR_ArrayOffsetFromUniformTest : core::ir::transform::TransformTest {
    IR_ArrayOffsetFromUniformTest() { capabilities = kArrayOffsetFromUniformCapabilities; }
};

// Test that offset is added to byte_address_buffer.Load
TEST_F(IR_ArrayOffsetFromUniformTest, Basic) {
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
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %tint_storage_buffer_dynamic_offsets:ptr<uniform, array<u32, 21>, read> = var undef @binding_point(1, 2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 20u
    %5:u32 = load %4
    %6:u32 = add 42u, %5
    %7:u32 = %buffer.Load %6
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 20;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

// Test that nothing changes if no bindpoint_to_offset_index is provided
TEST_F(IR_ArrayOffsetFromUniformTest, NoModify_EmptyBindpointToOffsetIndex) {
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

    auto* expect = src;

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

// Test that nothing changes if no bindpoint_to_offset_index is provided
TEST_F(IR_ArrayOffsetFromUniformTest, NoModify_NoMatchingBindpointToOffsetIndex) {
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

    auto* expect = src;

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 1}] = 20;  // Doesn't match binding point
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

// Test that nothing changes if we call a builtin that takes a byte_address_buffer but no offset
TEST_F(IR_ArrayOffsetFromUniformTest, NoModify_ByteAddressBufferBuiltinWithNoOffsetParam) {
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

    auto* expect = src;

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 20;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayOffsetFromUniformTest, AllLoadsAndStoresU32) {
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
                                                  u32(47),
                                                  b.Composite(ty.vec3u(), 123_u, 124_u, 125_u));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(
            ty.void_(), hlsl::BuiltinFn::kStore4, buffer_rw, u32(47),
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
    %10:void = %buffer_1.Store3 47u, vec3<u32>(123u, 124u, 125u)
    %11:void = %buffer_1.Store4 47u, vec4<u32>(123u, 124u, 125u, 126u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(5, 6)
  %buffer_1:hlsl.byte_address_buffer<read_write> = var undef @binding_point(7, 8)  # %buffer_1: 'buffer'
  %tint_storage_buffer_dynamic_offsets:ptr<uniform, array<u32, 58>, read> = var undef @binding_point(1, 2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %6:u32 = load %5
    %7:u32 = add 42u, %6
    %8:u32 = %buffer.Load %7
    %9:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %10:u32 = load %9
    %11:u32 = add 43u, %10
    %12:vec2<u32> = %buffer.Load2 %11
    %13:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %14:u32 = load %13
    %15:u32 = add 44u, %14
    %16:vec3<u32> = %buffer.Load3 %15
    %17:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %18:u32 = load %17
    %19:u32 = add 45u, %18
    %20:vec4<u32> = %buffer.Load4 %19
    %21:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %22:u32 = load %21
    %23:u32 = add 46u, %22
    %24:void = %buffer_1.Store %23, 123u
    %25:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %26:u32 = load %25
    %27:u32 = add 47u, %26
    %28:void = %buffer_1.Store2 %27, vec2<u32>(123u, 124u)
    %29:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %30:u32 = load %29
    %31:u32 = add 47u, %30
    %32:void = %buffer_1.Store3 %31, vec3<u32>(123u, 124u, 125u)
    %33:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %34:u32 = load %33
    %35:u32 = add 47u, %34
    %36:void = %buffer_1.Store4 %35, vec4<u32>(123u, 124u, 125u, 126u)
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{5, 6}] = 26;
    bindpoint_to_offset_index[{7, 8}] = 57;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayOffsetFromUniformTest, AllLoadsAndStoresF16) {
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
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.f16(), hlsl::BuiltinFn::kLoadF16, buffer_ro,
                                                  42_u);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.vec2h(), hlsl::BuiltinFn::kLoad2F16, buffer_ro,
                                                  43_u);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.vec3h(), hlsl::BuiltinFn::kLoad3F16, buffer_ro,
                                                  44_u);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.vec4h(), hlsl::BuiltinFn::kLoad4F16, buffer_ro,
                                                  45_u);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kStoreF16, buffer_rw,
                                                  46_u, 123.0_h);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kStore2F16,
                                                  buffer_rw, 47_u,
                                                  b.Composite(ty.vec2h(), 123.0_h, 124.0_h));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(
            ty.void_(), hlsl::BuiltinFn::kStore3F16, buffer_rw, 47_u,
            b.Composite(ty.vec3h(), 123.0_h, 124.0_h, 125.0_h));
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(
            ty.void_(), hlsl::BuiltinFn::kStore4F16, buffer_rw, 47_u,
            b.Composite(ty.vec4h(), 123.0_h, 124.0_h, 125.0_h, 126.0_h));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(5, 6)
  %buffer_1:hlsl.byte_address_buffer<read_write> = var undef @binding_point(7, 8)  # %buffer_1: 'buffer'
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:f16 = %buffer.LoadF16 42u
    %5:vec2<f16> = %buffer.Load2F16 43u
    %6:vec3<f16> = %buffer.Load3F16 44u
    %7:vec4<f16> = %buffer.Load4F16 45u
    %8:void = %buffer_1.StoreF16 46u, 123.0h
    %9:void = %buffer_1.Store2F16 47u, vec2<f16>(123.0h, 124.0h)
    %10:void = %buffer_1.Store3F16 47u, vec3<f16>(123.0h, 124.0h, 125.0h)
    %11:void = %buffer_1.Store4F16 47u, vec4<f16>(123.0h, 124.0h, 125.0h, 126.0h)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read> = var undef @binding_point(5, 6)
  %buffer_1:hlsl.byte_address_buffer<read_write> = var undef @binding_point(7, 8)  # %buffer_1: 'buffer'
  %tint_storage_buffer_dynamic_offsets:ptr<uniform, array<u32, 58>, read> = var undef @binding_point(1, 2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %6:u32 = load %5
    %7:u32 = add 42u, %6
    %8:f16 = %buffer.LoadF16 %7
    %9:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %10:u32 = load %9
    %11:u32 = add 43u, %10
    %12:vec2<f16> = %buffer.Load2F16 %11
    %13:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %14:u32 = load %13
    %15:u32 = add 44u, %14
    %16:vec3<f16> = %buffer.Load3F16 %15
    %17:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %18:u32 = load %17
    %19:u32 = add 45u, %18
    %20:vec4<f16> = %buffer.Load4F16 %19
    %21:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %22:u32 = load %21
    %23:u32 = add 46u, %22
    %24:void = %buffer_1.StoreF16 %23, 123.0h
    %25:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %26:u32 = load %25
    %27:u32 = add 47u, %26
    %28:void = %buffer_1.Store2F16 %27, vec2<f16>(123.0h, 124.0h)
    %29:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %30:u32 = load %29
    %31:u32 = add 47u, %30
    %32:void = %buffer_1.Store3F16 %31, vec3<f16>(123.0h, 124.0h, 125.0h)
    %33:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 57u
    %34:u32 = load %33
    %35:u32 = add 47u, %34
    %36:void = %buffer_1.Store4F16 %35, vec4<f16>(123.0h, 124.0h, 125.0h, 126.0h)
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{5, 6}] = 26;
    bindpoint_to_offset_index[{7, 8}] = 57;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

// Test that offset is only added to bindpoint_to_offset_index entries
TEST_F(IR_ArrayOffsetFromUniformTest, OffsetOnlyEntriesInMap) {
    auto* byte_address_buffer_readonly_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* buffer0 = b.Var("buffer0", byte_address_buffer_readonly_ty);
    auto* buffer1 = b.Var("buffer1", byte_address_buffer_readonly_ty);
    buffer0->SetBindingPoint(0, 0);
    buffer1->SetBindingPoint(0, 1);
    mod.root_block->Append(buffer0);
    mod.root_block->Append(buffer1);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto base_offset = u32(42);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer0,
                                                  base_offset);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.u32(), hlsl::BuiltinFn::kLoad, buffer1,
                                                  base_offset);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer0:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %buffer1:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = %buffer0.Load 42u
    %5:u32 = %buffer1.Load 42u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer0:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
  %buffer1:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 1)
  %tint_storage_buffer_dynamic_offsets:ptr<uniform, array<u32, 21>, read> = var undef @binding_point(1, 2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 20u
    %6:u32 = load %5
    %7:u32 = add 42u, %6
    %8:u32 = %buffer0.Load %7
    %9:u32 = %buffer1.Load 42u
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{0, 0}] = 20;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ArrayOffsetFromUniformTest, AllAtomicOps) {
    auto* byte_address_buffer_readwrite_ty =
        ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kReadWrite);

    auto* buffer_rw = b.Var("buffer", byte_address_buffer_readwrite_ty);
    buffer_rw->SetBindingPoint(5, 6);
    mod.root_block->Append(buffer_rw);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto base_offset = u32(42);
        auto* pa = b.Var("a", ty.ptr<function, u32, read_write>());
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(),
                                                  hlsl::BuiltinFn::kInterlockedCompareExchange,
                                                  buffer_rw, base_offset, 1_u, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedExchange,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedAdd,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedMax,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedMin,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedAnd,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedOr,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kInterlockedXor,
                                                  buffer_rw, base_offset, 2_u, pa);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read_write> = var undef @binding_point(5, 6)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, u32, read_write> = var undef
    %4:void = %buffer.InterlockedCompareExchange 42u, 1u, 2u, %a
    %5:void = %buffer.InterlockedExchange 42u, 2u, %a
    %6:void = %buffer.InterlockedAdd 42u, 2u, %a
    %7:void = %buffer.InterlockedMax 42u, 2u, %a
    %8:void = %buffer.InterlockedMin 42u, 2u, %a
    %9:void = %buffer.InterlockedAnd 42u, 2u, %a
    %10:void = %buffer.InterlockedOr 42u, 2u, %a
    %11:void = %buffer.InterlockedXor 42u, 2u, %a
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:hlsl.byte_address_buffer<read_write> = var undef @binding_point(5, 6)
  %tint_storage_buffer_dynamic_offsets:ptr<uniform, array<u32, 27>, read> = var undef @binding_point(1, 2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, u32, read_write> = var undef
    %5:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %6:u32 = load %5
    %7:u32 = add 42u, %6
    %8:void = %buffer.InterlockedCompareExchange %7, 1u, 2u, %a
    %9:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %10:u32 = load %9
    %11:u32 = add 42u, %10
    %12:void = %buffer.InterlockedExchange %11, 2u, %a
    %13:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %14:u32 = load %13
    %15:u32 = add 42u, %14
    %16:void = %buffer.InterlockedAdd %15, 2u, %a
    %17:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %18:u32 = load %17
    %19:u32 = add 42u, %18
    %20:void = %buffer.InterlockedMax %19, 2u, %a
    %21:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %22:u32 = load %21
    %23:u32 = add 42u, %22
    %24:void = %buffer.InterlockedMin %23, 2u, %a
    %25:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %26:u32 = load %25
    %27:u32 = add 42u, %26
    %28:void = %buffer.InterlockedAnd %27, 2u, %a
    %29:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %30:u32 = load %29
    %31:u32 = add 42u, %30
    %32:void = %buffer.InterlockedOr %31, 2u, %a
    %33:ptr<uniform, u32, read> = access %tint_storage_buffer_dynamic_offsets, 26u
    %34:u32 = load %33
    %35:u32 = add 42u, %34
    %36:void = %buffer.InterlockedXor %35, 2u, %a
    ret
  }
}
)";

    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;
    bindpoint_to_offset_index[{5, 6}] = 26;
    Run(ArrayOffsetFromUniform, BindingPoint{1, 2}, bindpoint_to_offset_index);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
