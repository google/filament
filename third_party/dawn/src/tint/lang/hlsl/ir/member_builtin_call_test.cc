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

#include "src/tint/lang/hlsl/ir/member_builtin_call.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::ir {
namespace {

using IR_HlslMemberBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_HlslMemberBuiltinCallTest, Clone) {
    auto* buf = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kReadWrite);

    auto* t = b.FunctionParam("t", buf);
    auto* builtin = b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kLoad, t, 2_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().u32(), new_b->Result()->Type());

    EXPECT_EQ(BuiltinFn::kLoad, new_b->Func());

    EXPECT_TRUE(new_b->Object()->Type()->Is<hlsl::type::ByteAddressBuffer>());

    auto args = new_b->Args();
    ASSERT_EQ(1u, args.Length());
    EXPECT_TRUE(args[0]->Type()->Is<core::type::U32>());
}

TEST_F(IR_HlslMemberBuiltinCallTest, DoesNotMatchNonMemberFunction) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* builtin =
            b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kAsint, t, 2_u);
        b.Return(func, builtin);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason,
        R"(:7:17 error: asint: no matching call to 'asint(hlsl.byte_address_buffer<read>, u32)'

    %3:u32 = %t.asint 2u
                ^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = %t.asint 2u
    ret %3
  }
}
)");
}

TEST_F(IR_HlslMemberBuiltinCallTest, DoesNotMatchIncorrectType) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* builtin =
            b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kStore, t, 2_u, 2_u);
        b.Return(func, builtin);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason,
        R"(:7:17 error: Store: no matching call to 'Store(hlsl.byte_address_buffer<read>, u32, u32)'

1 candidate function:
 • 'Store(byte_address_buffer<write' or 'read_write>  ✗ , offset: u32  ✓ , value: u32  ✓ )'

    %3:u32 = %t.Store 2u, 2u
                ^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = %t.Store 2u, 2u
    ret %3
  }
}
)");
}

TEST_F(IR_HlslMemberBuiltinCallTest, Valid) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kLoad, t, 0_u);
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_HlslMemberBuiltinCallTest, MissingResults) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* builtin =
            b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kLoad, t, 0_u);
        builtin->ClearResults();
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(:7:16 error: Load: expected exactly 1 results, got 0
    undef = %t.Load 0u
               ^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    undef = %t.Load 0u
    ret
  }
}
)");
}

TEST_F(IR_HlslMemberBuiltinCallTest, TooFewArgs) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kLoad, t);
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(:7:17 error: Load: no matching call to 'Load(hlsl.byte_address_buffer<read>)'

24 candidate functions:
 • 'Load(byte_address_buffer<read' or 'read_write>  ✓ , offset: u32  ✗ ) -> u32'
 • 'Load(texture: texture_depth_2d  ✗ , location: vec3<i32>  ✗ ) -> vec4<f32>'
 • 'Load(texture: texture_depth_2d_array  ✗ , location: vec4<i32>  ✗ ) -> vec4<f32>'
 • 'Load(texture: texture_1d<T>  ✗ , location: vec2<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_2d<T>  ✗ , location: vec3<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_2d_array<T>  ✗ , location: vec4<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_3d<T>  ✗ , location: vec4<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: rasterizer_ordered_texture_2d<F>  ✗ , location: vec2<C>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'C' is 'i32' or 'u32'
 • 'Load(texture: rasterizer_ordered_texture_2d<F>  ✗ , location: vec2<C>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'C' is 'i32' or 'u32'
 • 'Load(texture: rasterizer_ordered_texture_2d<F>  ✗ , location: vec2<C>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'C' is 'i32' or 'u32'
 • 'Load(texture: texture_storage_1d<F, A>  ✗ , location: vec2<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d<F, A>  ✗ , location: vec3<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d_array<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_3d<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_1d<F, A>  ✗ , location: vec2<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d<F, A>  ✗ , location: vec3<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d_array<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_3d<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_1d<F, A>  ✗ , location: vec2<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d<F, A>  ✗ , location: vec3<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d_array<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_3d<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_depth_multisampled_2d  ✗ , location: vec2<i32>  ✗ , sample_index: i32  ✗ ) -> vec4<f32>'
 • 'Load(texture: texture_multisampled_2d<T>  ✗ , location: vec2<i32>  ✗ , sample_index: i32  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %3:u32 = %t.Load
                ^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = %t.Load
    ret
  }
}
)");
}

TEST_F(IR_HlslMemberBuiltinCallTest, TooManyArgs) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kLoad, t, 0_u, 1_u, 2_u);
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason,
        R"(:7:17 error: Load: no matching call to 'Load(hlsl.byte_address_buffer<read>, u32, u32, u32)'

24 candidate functions:
 • 'Load(byte_address_buffer<read' or 'read_write>  ✓ , offset: u32  ✓ ) -> u32' where:
      ✗  overload expects 2 arguments, call passed 4 arguments
 • 'Load(texture: texture_depth_multisampled_2d  ✗ , location: vec2<i32>  ✗ , sample_index: i32  ✗ ) -> vec4<f32>'
 • 'Load(texture: texture_multisampled_2d<T>  ✗ , location: vec2<i32>  ✗ , sample_index: i32  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_depth_2d  ✗ , location: vec3<i32>  ✗ ) -> vec4<f32>'
 • 'Load(texture: texture_depth_2d_array  ✗ , location: vec4<i32>  ✗ ) -> vec4<f32>'
 • 'Load(texture: texture_1d<T>  ✗ , location: vec2<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_2d<T>  ✗ , location: vec3<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_2d_array<T>  ✗ , location: vec4<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: texture_3d<T>  ✗ , location: vec4<i32>  ✗ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'Load(texture: rasterizer_ordered_texture_2d<F>  ✗ , location: vec2<C>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'C' is 'i32' or 'u32'
 • 'Load(texture: rasterizer_ordered_texture_2d<F>  ✗ , location: vec2<C>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'C' is 'i32' or 'u32'
 • 'Load(texture: rasterizer_ordered_texture_2d<F>  ✗ , location: vec2<C>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'C' is 'i32' or 'u32'
 • 'Load(texture: texture_storage_1d<F, A>  ✗ , location: vec2<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d<F, A>  ✗ , location: vec3<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d_array<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_3d<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<f32>' where:
      ✗  'F' is 'r8unorm', 'bgra8unorm', 'rgba8unorm', 'rgba8snorm', 'rgba16float', 'r32float', 'rg32float' or 'rgba32float'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_1d<F, A>  ✗ , location: vec2<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d<F, A>  ✗ , location: vec3<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d_array<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_3d<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<u32>' where:
      ✗  'F' is 'rgba8uint', 'rgba16uint', 'r32uint', 'rg32uint' or 'rgba32uint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_1d<F, A>  ✗ , location: vec2<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d<F, A>  ✗ , location: vec3<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_2d_array<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'
 • 'Load(texture: texture_storage_3d<F, A>  ✗ , location: vec4<i32>  ✗ ) -> vec4<i32>' where:
      ✗  'F' is 'rgba8sint', 'rgba16sint', 'r32sint', 'rg32sint' or 'rgba32sint'
      ✗  'A' is 'read' or 'read_write'

    %3:u32 = %t.Load 0u, 1u, 2u
                ^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = %t.Load 0u, 1u, 2u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::hlsl::ir
