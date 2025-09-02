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

#include "src/tint/lang/hlsl/writer/raise/builtin_polyfill.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastIdentity) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<i32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):i32 {
  $B1: {
    %3:i32 = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:i32):i32 {
  $B1: {
    ret %a
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asuint) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<u32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):u32 {
  $B1: {
    %3:u32 = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:i32):u32 {
  $B1: {
    %3:u32 = hlsl.asuint %a
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asint) {
    auto* a = b.FunctionParam<u32>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<i32>(a)); });

    auto* src = R"(
%foo = func(%a:u32):i32 {
  $B1: {
    %3:i32 = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32):i32 {
  $B1: {
    %3:i32 = hlsl.asint %a
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asfloat) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<f32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):f32 {
  $B1: {
    %3:f32 = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:i32):f32 {
  $B1: {
    %3:f32 = hlsl.asfloat %a
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, AsfloatVec) {
    auto* a = b.FunctionParam<vec3<i32>>("a");
    auto* func = b.Function("foo", ty.vec<f32, 3>());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec(ty.f32(), 3), a)); });

    auto* src = R"(
%foo = func(%a:vec3<i32>):vec3<f32> {
  $B1: {
    %3:vec3<f32> = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec3<i32>):vec3<f32> {
  $B1: {
    %3:vec3<f32> = hlsl.asfloat %a
    ret %3
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastFromF16) {
    auto* a = b.FunctionParam<vec2<f16>>("a");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.f32(), a)); });

    auto* src = R"(
%foo = func(%a:vec2<f16>):f32 {
  $B1: {
    %3:f32 = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<f16>):f32 {
  $B1: {
    %3:f32 = call %tint_bitcast_from_f16, %a
    ret %3
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):f32 {
  $B2: {
    %6:vec2<f32> = convert %src
    %7:vec2<u32> = hlsl.f32tof16 %6
    %r:vec2<u32> = let %7
    %9:u32 = swizzle %r, x
    %10:u32 = and %9, 65535u
    %11:u32 = swizzle %r, y
    %12:u32 = and %11, 65535u
    %13:u32 = shl %12, 16u
    %14:u32 = or %10, %13
    %15:f32 = hlsl.asfloat %14
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastToF16) {
    auto* a = b.FunctionParam<f32>("a");
    auto* func = b.Function("foo", ty.vec2<f16>());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec2<f16>(), a)); });

    auto* src = R"(
%foo = func(%a:f32):vec2<f16> {
  $B1: {
    %3:vec2<f16> = bitcast %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:f32):vec2<f16> {
  $B1: {
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    ret %3
  }
}
%tint_bitcast_to_f16 = func(%src:f32):vec2<f16> {
  $B2: {
    %6:u32 = hlsl.asuint %src
    %v:u32 = let %6
    %8:u32 = and %v, 65535u
    %9:f32 = hlsl.f16tof32 %8
    %t_low:f32 = let %9
    %11:u32 = shr %v, 16u
    %12:u32 = and %11, 65535u
    %13:f32 = hlsl.f16tof32 %12
    %t_high:f32 = let %13
    %15:f16 = convert %t_low
    %16:f16 = convert %t_high
    %17:vec2<f16> = construct %15, %16
    ret %17
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastFromVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<f16>>(1_h, 2_h));
        auto* z = b.Load(a);
        b.Let("b", b.Bitcast<i32>(z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:ptr<function, vec2<f16>, read_write> = var %2
    %4:vec2<f16> = load %a
    %5:i32 = bitcast %4
    %b:i32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:ptr<function, vec2<f16>, read_write> = var %2
    %4:vec2<f16> = load %a
    %5:i32 = call %tint_bitcast_from_f16, %4
    %b:i32 = let %5
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):i32 {
  $B2: {
    %9:vec2<f32> = convert %src
    %10:vec2<u32> = hlsl.f32tof16 %9
    %r:vec2<u32> = let %10
    %12:u32 = swizzle %r, x
    %13:u32 = and %12, 65535u
    %14:u32 = swizzle %r, y
    %15:u32 = and %14, 65535u
    %16:u32 = shl %15, 16u
    %17:u32 = or %13, %16
    %18:i32 = hlsl.asint %17
    ret %18
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("b", b.Bitcast<vec4<f16>>(b.Load(a)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:ptr<function, vec2<i32>, read_write> = var %2
    %4:vec2<i32> = load %a
    %5:vec4<f16> = bitcast %4
    %b:vec4<f16> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:ptr<function, vec2<i32>, read_write> = var %2
    %4:vec2<i32> = load %a
    %5:vec4<f16> = call %tint_bitcast_to_f16, %4
    %b:vec4<f16> = let %5
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<i32>):vec4<f16> {
  $B2: {
    %9:vec2<u32> = hlsl.asuint %src
    %v:vec2<u32> = let %9
    %mask:vec2<u32> = let vec2<u32>(65535u)
    %shift:vec2<u32> = let vec2<u32>(16u)
    %13:vec2<u32> = and %v, %mask
    %14:vec2<f32> = hlsl.f16tof32 %13
    %t_low:vec2<f32> = let %14
    %16:vec2<u32> = shr %v, %shift
    %17:vec2<u32> = and %16, %mask
    %18:vec2<f32> = hlsl.f16tof32 %17
    %t_high:vec2<f32> = let %18
    %20:f32 = swizzle %t_low, x
    %21:f32 = swizzle %t_high, x
    %22:f16 = convert %20
    %23:f16 = convert %21
    %24:f32 = swizzle %t_low, y
    %25:f16 = convert %24
    %26:f32 = swizzle %t_high, y
    %27:f16 = convert %26
    %28:vec4<f16> = construct %22, %23, %25, %27
    ret %28
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Sign) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kSign, -1_f));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:f32 = sign -1.0f
    %a:f32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = hlsl.sign -1.0f
    %3:f32 = convert %2
    %a:f32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SignVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.vec3<i32>(), core::BuiltinFn::kSign,
                          b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<i32> = sign vec3<i32>(1i, 2i, 3i)
    %a:vec3<i32> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<i32> = hlsl.sign vec3<i32>(1i, 2i, 3i)
    %a:vec3<i32> = let %2
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureNumLevels) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:ptr<function, vec2<u32>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:void = %t.GetDimensions 0u, %4, %5
    %7:vec2<u32> = load %3
    %8:u32 = swizzle %7, y
    ret %8
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureNumLayers) {
    auto* t = b.FunctionParam(
        "t", ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_cube_array<f32>):u32 {
  $B1: {
    %3:ptr<function, vec3<u32>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:ptr<function, u32, read_write> = access %3, 2u
    %7:void = %t.GetDimensions %4, %5, %6
    %8:vec3<u32> = load %3
    %9:u32 = swizzle %8, z
    ret %9
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureNumSamples) {
    auto* t =
        b.FunctionParam("t", ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumSamples, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumSamples %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<f32>):u32 {
  $B1: {
    %3:ptr<function, vec3<u32>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:ptr<function, u32, read_write> = access %3, 2u
    %7:void = %t.GetDimensions %4, %5, %6
    %8:vec3<u32> = load %3
    %9:u32 = swizzle %8, z
    ret %9
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_1d_WithoutLod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:ptr<function, u32, read_write> = var undef
    %4:void = %t.GetDimensions %3
    %5:u32 = load %3
    ret %5
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_1d_WithI32Lod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, t, 3_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureDimensions %t, 3i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = convert 3i
    %4:ptr<function, vec2<u32>, read_write> = var undef
    %5:ptr<function, u32, read_write> = access %4, 0u
    %6:ptr<function, u32, read_write> = access %4, 1u
    %7:void = %t.GetDimensions %3, %5, %6
    %8:vec2<u32> = load %4
    %9:u32 = swizzle %8, x
    ret %9
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_1d_WithU32Lod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, t, 3_u);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureDimensions %t, 3u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:ptr<function, vec2<u32>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:void = %t.GetDimensions 3u, %4, %5
    %7:vec2<u32> = load %3
    %8:u32 = swizzle %7, x
    ret %8
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithoutLod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:ptr<function, vec2<u32>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:void = %t.GetDimensions %4, %5
    %7:vec2<u32> = load %3
    ret %7
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithI32Lod) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t, 3_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t, 3i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:u32 = convert 3i
    %4:ptr<function, vec3<u32>, read_write> = var undef
    %5:ptr<function, u32, read_write> = access %4, 0u
    %6:ptr<function, u32, read_write> = access %4, 1u
    %7:ptr<function, u32, read_write> = access %4, 2u
    %8:void = %t.GetDimensions %3, %5, %6, %7
    %9:vec3<u32> = load %4
    %10:vec2<u32> = swizzle %9, xy
    ret %10
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_3d) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));
    auto* func = b.Function("foo", ty.vec3<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec3<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>):vec3<u32> {
  $B1: {
    %3:vec3<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_3d<f32>):vec3<u32> {
  $B1: {
    %3:ptr<function, vec3<u32>, read_write> = var undef
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:ptr<function, u32, read_write> = access %3, 2u
    %7:void = %t.GetDimensions %4, %5, %6
    %8:vec3<u32> = load %3
    ret %8
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_1DF32) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<i32>();
        auto* level = b.Zero<u32>();
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = textureLoad %t, 0i, 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):vec4<f32> {
  $B1: {
    %3:i32 = convert 0u
    %4:vec2<i32> = construct 0i, %3
    %5:vec4<f32> = %t.Load %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_2DLevelI32) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<i32>();
        auto* result = b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = textureLoad %t, vec2<i32>(0i), 0i
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec3<i32> = construct vec2<i32>(0i), 0i
    %4:vec4<i32> = %t.Load %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_3DLevelU32) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec3<i32>>();
        auto* level = b.Zero<u32>();
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = textureLoad %t, vec3<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_3d<f32>):vec4<f32> {
  $B1: {
    %3:i32 = convert 0u
    %4:vec4<i32> = construct vec3<i32>(0i), %3
    %5:vec4<f32> = %t.Load %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_Multisampled2DI32) {
    auto* t =
        b.FunctionParam("t", ty.multisampled_texture(core::type::TextureDimension::k2d, ty.i32()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* sample_idx = b.Zero<u32>();
        auto* result = b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, t, coords, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = textureLoad %t, vec2<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<i32>):vec4<i32> {
  $B1: {
    %3:i32 = convert 0u
    %4:vec4<i32> = %t.Load vec2<i32>(0i), %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_Depth2DLevelF32) {
    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<u32>();
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d):f32 {
  $B1: {
    %3:f32 = textureLoad %t, vec2<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d):f32 {
  $B1: {
    %3:i32 = convert 0u
    %4:vec3<i32> = construct vec2<i32>(0i), %3
    %5:vec4<f32> = %t.Load %4
    %6:f32 = swizzle %5, x
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_Depth2DArrayLevelF32) {
    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2dArray));
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* array_idx = b.Zero<u32>();
        auto* sample_idx = b.Zero<u32>();
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureLoad, t, coords, array_idx, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array):f32 {
  $B1: {
    %3:f32 = textureLoad %t, vec2<i32>(0i), 0u, 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array):f32 {
  $B1: {
    %3:i32 = convert 0u
    %4:i32 = convert 0u
    %5:vec4<i32> = construct vec2<i32>(0i), %3, %4
    %6:vec4<f32> = %t.Load %5
    %7:f32 = swizzle %6, x
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_DepthMultisampledF32) {
    auto* t =
        b.FunctionParam("t", ty.depth_multisampled_texture(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* sample_idx = b.Zero<u32>();
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureLoad, t, coords, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_multisampled_2d):f32 {
  $B1: {
    %3:f32 = textureLoad %t, vec2<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_multisampled_2d):f32 {
  $B1: {
    %3:i32 = convert 0u
    %4:vec4<f32> = %t.Load vec2<i32>(0i), %3
    %5:f32 = swizzle %4, x
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureStore1D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k1d,
                                          core::TexelFormat::kR32Float, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_1d<r32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_1d<r32float, read_write> = load %1
    %4:void = textureStore %3, 1i, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_1d<r32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_1d<r32float, read_write> = load %1
    %4:void = hlsl.textureStore %3, 1i, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureStore3D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k3d,
                                          core::TexelFormat::kR32Float, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_3d<r32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_3d<r32float, read_write> = load %1
    %4:void = textureStore %3, vec3<i32>(1i, 2i, 3i), vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_3d<r32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_3d<r32float, read_write> = load %1
    %4:void = hlsl.textureStore %3, vec3<i32>(1i, 2i, 3i), vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureStoreArray) {
    auto* t = b.Var(ty.ptr(
        handle, ty.storage_texture(core::type::TextureDimension::k2dArray,
                                   core::TexelFormat::kRgba32Float, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, .4_f, .3_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, 3_u, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d_array<rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rgba32float, read_write> = load %1
    %4:void = textureStore %3, vec2<i32>(1i, 2i), 3u, vec4<f32>(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d_array<rgba32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rgba32float, read_write> = load %1
    %4:i32 = convert 3u
    %5:vec3<i32> = construct vec2<i32>(1i, 2i), %4
    %6:void = hlsl.textureStore %3, %5, vec4<f32>(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGatherCompare_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* depth_ref = b.Value(3_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords, depth_ref));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:vec4<f32> = textureGatherCompare %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:vec4<f32> = %5.GatherCmp %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGatherCompare_Depth2dOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* depth_ref = b.Value(3_f);
        auto* offset = b.Construct(ty.vec2<i32>(), b.Value(4_i), b.Value(5_i));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     depth_ref, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<i32> = construct 4i, 5i
    %6:texture_depth_2d = load %1
    %7:sampler_comparison = load %2
    %8:vec4<f32> = textureGatherCompare %6, %7, %4, 3.0f, %5
    %x:vec4<f32> = let %8
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<i32> = construct 4i, 5i
    %6:texture_depth_2d = load %1
    %7:sampler_comparison = load %2
    %8:vec4<f32> = %6.GatherCmp %7, %4, 3.0f, %5
    %x:vec4<f32> = let %8
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGatherCompare_DepthCubeArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(2.5_f));
        auto* array_idx = b.Value(6_u);
        auto* depth_ref = b.Value(3_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     array_idx, depth_ref));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 2.5f
    %5:texture_depth_cube_array = load %1
    %6:sampler_comparison = load %2
    %7:vec4<f32> = textureGatherCompare %5, %6, %4, 6u, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 2.5f
    %5:texture_depth_cube_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 6u
    %8:vec4<f32> = construct %4, %7
    %9:vec4<f32> = %5.GatherCmp %6, %8, 3.0f
    %x:vec4<f32> = let %9
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGatherCompare_Depth2dArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(6_i);
        auto* depth_ref = b.Value(3_f);
        auto* offset = b.Construct(ty.vec2<i32>(), b.Value(4_i), b.Value(5_i));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     array_idx, depth_ref, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<i32> = construct 4i, 5i
    %6:texture_depth_2d_array = load %1
    %7:sampler_comparison = load %2
    %8:vec4<f32> = textureGatherCompare %6, %7, %4, 6i, 3.0f, %5
    %x:vec4<f32> = let %8
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<i32> = construct 4i, 5i
    %6:texture_depth_2d_array = load %1
    %7:sampler_comparison = load %2
    %8:f32 = convert 6i
    %9:vec3<f32> = construct %4, %8
    %10:vec4<f32> = %6.GatherCmp %7, %9, 3.0f, %5
    %x:vec4<f32> = let %10
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_Alpha) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 3_u, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<i32> = load %1
    %6:sampler = load %2
    %7:vec4<i32> = textureGather 3u, %5, %6, %4
    %x:vec4<i32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<i32> = load %1
    %6:sampler = load %2
    %7:vec4<i32> = %5.GatherAlpha %6, %4
    %x:vec4<i32> = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}
TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_RedOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(1_i, 3_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 0_u, t, s, coords, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<i32> = load %1
    %6:sampler = load %2
    %7:vec4<i32> = textureGather 0u, %5, %6, %4, vec2<i32>(1i, 3i)
    %x:vec4<i32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<i32> = load %1
    %6:sampler = load %2
    %7:vec4<i32> = %5.GatherRed %6, %4, vec2<i32>(1i, 3i)
    %x:vec4<i32> = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}
TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_GreenArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(1_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 1_u, t, s, coords, array_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<i32> = load %1
    %6:sampler = load %2
    %7:vec4<i32> = textureGather 1u, %5, %6, %4, 1u
    %x:vec4<i32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<i32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 1u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<i32> = %5.GatherGreen %6, %8
    %x:vec4<i32> = let %9
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_BlueArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(1_i);
        auto* offset = b.Composite<vec2<i32>>(1_i, 2_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 2_u, t, s, coords, array_idx,
                                     offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<i32> = load %1
    %6:sampler = load %2
    %7:vec4<i32> = textureGather 2u, %5, %6, %4, 1i, vec2<i32>(1i, 2i)
    %x:vec4<i32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<i32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<i32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 1i
    %8:vec3<f32> = construct %4, %7
    %9:vec4<i32> = %5.GatherBlue %6, %8, vec2<i32>(1i, 2i)
    %x:vec4<i32> = let %9
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_Depth) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureGather %5, %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Gather %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}
TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_DepthOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(3_i, 4_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureGather %5, %6, %4, vec2<i32>(3i, 4i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Gather %6, %4, vec2<i32>(3i, 4i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}
TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_DepthArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords, array_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureGather %5, %6, %4, 4i
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4i
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.Gather %6, %8
    %x:vec4<f32> = let %9
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}
TEST_F(HlslWriter_BuiltinPolyfillTest, TextureGather_DepthArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureGather %5, %6, %4, 4u, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.Gather %6, %8, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_1d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:texture_1d<f32> = load %1
    %5:sampler = load %2
    %6:vec4<f32> = textureSample %4, %5, 1.0f
    %x:vec4<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:texture_1d<f32> = load %1
    %5:sampler = load %2
    %6:vec4<f32> = %4.Sample %5, 1.0f
    %x:vec4<f32> = let %6
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4, 4u
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.Sample %6, %8
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4, 4u, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.Sample %6, %8, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSample %5, %6, %4, 4u
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:vec4<f32> = %5.Sample %6, %8
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:f32 = textureSample %5, %6, %4
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4
    %8:f32 = swizzle %7, x
    %x:f32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_Depth2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:f32 = textureSample %5, %6, %4, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.Sample %6, %4, vec2<i32>(4i, 5i)
    %8:f32 = swizzle %7, x
    %x:f32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_Depth2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = textureSample %5, %6, %4, 4u
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.Sample %6, %8
    %10:f32 = swizzle %9, x
    %x:f32 = let %10
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_Depth2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = textureSample %5, %6, %4, 4u, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.Sample %6, %8, vec2<i32>(4i, 5i)
    %10:f32 = swizzle %9, x
    %x:f32 = let %10
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSample_DepthCube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler = load %2
    %7:f32 = textureSample %5, %6, %4, 4u
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:vec4<f32> = %5.Sample %6, %8
    %10:f32 = swizzle %9, x
    %x:f32 = let %10
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleBias %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleBias %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 4u, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.SampleBias %6, %8, 3.0f
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, array_idx,
                                     3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 4u, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.SampleBias %6, %8, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleBias %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 3.0f, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleBias %6, %4, 3.0f, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleBias %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleBias_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleBias %5, %6, %4, 4u, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:vec4<f32> = %5.SampleBias %6, %8, 3.0f
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompare_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompare %5, %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = %5.SampleCmp %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompare_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompare %5, %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = %5.SampleCmp %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompare_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompare %5, %6, %4, 4u, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:f32 = %5.SampleCmp %6, %8, 3.0f
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompare_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, array_idx, 3_f,
                               offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompare %5, %6, %4, 4u, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:f32 = %5.SampleCmp %6, %8, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompare_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCube)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompare %5, %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube = load %1
    %6:sampler_comparison = load %2
    %7:f32 = %5.SampleCmp %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompare_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompare %5, %6, %4, 4u, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:f32 = %5.SampleCmp %6, %8, 3.0f
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompareLevel %5, %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = %5.SampleCmpLevelZero %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompareLevel %5, %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler_comparison = load %2
    %7:f32 = %5.SampleCmpLevelZero %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, array_idx,
                               3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompareLevel %5, %6, %4, 4u, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:f32 = %5.SampleCmpLevelZero %6, %8, 3.0f
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, array_idx,
                               3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompareLevel %5, %6, %4, 4u, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:f32 = %5.SampleCmpLevelZero %6, %8, 3.0f, vec2<i32>(4i, 5i)
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCube)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompareLevel %5, %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube = load %1
    %6:sampler_comparison = load %2
    %7:f32 = %5.SampleCmpLevelZero %6, %4, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, array_idx,
                               3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = textureSampleCompareLevel %5, %6, %4, 4u, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler_comparison, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler_comparison = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:f32 = %5.SampleCmpLevelZero %6, %8, 3.0f
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(6_f), b.Value(7_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = %7.SampleGrad %8, %4, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(6_f), b.Value(7_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy,
                                     offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, %5, %6, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = %7.SampleGrad %8, %4, %5, %6, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(6_f), b.Value(7_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, array_idx,
                                     ddx, ddy));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d_array<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, 4u, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d_array<f32> = load %1
    %8:sampler = load %2
    %9:f32 = convert 4u
    %10:vec3<f32> = construct %4, %9
    %11:vec4<f32> = %7.SampleGrad %8, %10, %5, %6
    %x:vec4<f32> = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(6_f), b.Value(7_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, array_idx,
                                     ddx, ddy, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d_array<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, 4u, %5, %6, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:vec2<f32> = construct 3.0f, 4.0f
    %6:vec2<f32> = construct 6.0f, 7.0f
    %7:texture_2d_array<f32> = load %1
    %8:sampler = load %2
    %9:f32 = convert 4u
    %10:vec3<f32> = construct %4, %9
    %11:vec4<f32> = %7.SampleGrad %8, %10, %5, %6, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_3d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_3d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = %7.SampleGrad %8, %4, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy,
                                     offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_3d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, %5, %6, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_3d<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = %7.SampleGrad %8, %4, %5, %6, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_cube<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_cube<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = %7.SampleGrad %8, %4, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleGrad_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, array_idx,
                                     ddx, ddy));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_cube_array<f32> = load %1
    %8:sampler = load %2
    %9:vec4<f32> = textureSampleGrad %7, %8, %4, 4u, %5, %6
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %6:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %7:texture_cube_array<f32> = load %1
    %8:sampler = load %2
    %9:f32 = convert 4u
    %10:vec4<f32> = construct %4, %9
    %11:vec4<f32> = %7.SampleGrad %8, %10, %5, %6
    %x:vec4<f32> = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_1d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 0_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:texture_1d<f32> = load %1
    %5:sampler = load %2
    %6:vec4<f32> = textureSampleLevel %4, %5, 1.0f, 0.0f
    %x:vec4<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:texture_1d<f32> = load %1
    %5:sampler = load %2
    %6:vec4<f32> = %4.SampleLevel %5, 1.0f, 0.0f
    %x:vec4<f32> = let %6
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleLevel %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleLevel %6, %4, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx,
                                     3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 4u, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.SampleLevel %6, %8, 3.0f
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx,
                                     3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 4u, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_2d_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:vec4<f32> = %5.SampleLevel %6, %8, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleLevel %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 3.0f, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_3d<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleLevel %6, %4, 3.0f, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = %5.SampleLevel %6, %4, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx,
                                     3_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube_array<f32> = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureSampleLevel %5, %6, %4, 4u, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_cube_array<f32> = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:vec4<f32> = %5.SampleLevel %6, %8, 3.0f
    %x:vec4<f32> = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:f32 = textureSampleLevel %5, %6, %4, 3i
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:f32 = convert 3i
    %8:vec4<f32> = %5.SampleLevel %6, %4, %7
    %9:f32 = swizzle %8, x
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_Depth2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_u, offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:f32 = textureSampleLevel %5, %6, %4, 3u, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:f32 = convert 3u
    %8:vec4<f32> = %5.SampleLevel %6, %4, %7, vec2<i32>(4i, 5i)
    %9:f32 = swizzle %8, x
    %x:f32 = let %9
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_Depth2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx, 3_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = textureSampleLevel %5, %6, %4, 4u, 3i
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:f32 = convert 3i
    %10:vec4<f32> = %5.SampleLevel %6, %8, %9
    %11:f32 = swizzle %10, x
    %x:f32 = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_Depth2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx, 3_u,
                               offset));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = textureSampleLevel %5, %6, %4, 4u, 3u, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %4, %7
    %9:f32 = convert 3u
    %10:vec4<f32> = %5.SampleLevel %6, %8, %9, vec2<i32>(4i, 5i)
    %11:f32 = swizzle %10, x
    %x:f32 = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureSampleLevel_DepthCube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx, 3_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler = load %2
    %7:f32 = textureSampleLevel %5, %6, %4, 4u, 3i
    %x:f32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:texture_depth_cube_array = load %1
    %6:sampler = load %2
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %4, %7
    %9:f32 = convert 3i
    %10:vec4<f32> = %5.SampleLevel %6, %8, %9
    %11:f32 = swizzle %10, x
    %x:f32 = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, QuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2<f32>()));
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<f32> = quantizeToF16 %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<u32> = hlsl.f32tof16 %3
    %5:vec2<f32> = hlsl.f16tof32 %4
    %a:vec2<f32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

struct AtomicData {
    core::BuiltinFn fn;
    const char* atomic;
    const char* interlock;
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const AtomicData& data) {
    out << data.interlock;
    return out;
}
using HlslBuiltinPolyfillWorkgroupAtomic = core::ir::transform::TransformTestWithParam<AtomicData>;
TEST_P(HlslBuiltinPolyfillWorkgroupAtomic, Access) {
    auto param = GetParam();
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), param.fn, var, 123_i));
        b.Return(func);
    });

    std::string src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:i32 = )" + std::string(param.atomic) +
                      R"( %v, 123i
    %x:i32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    std::string expect = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:void = hlsl.)" + std::string(param.interlock) +
                         R"( %v, 123i, %3
    %5:i32 = load %3
    %x:i32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    HlslWriter_BuiltinPolyfillTest,
    HlslBuiltinPolyfillWorkgroupAtomic,
    testing::Values(AtomicData{core::BuiltinFn::kAtomicAdd, "atomicAdd", "InterlockedAdd"},
                    AtomicData{core::BuiltinFn::kAtomicMax, "atomicMax", "InterlockedMax"},
                    AtomicData{core::BuiltinFn::kAtomicMin, "atomicMin", "InterlockedMin"},
                    AtomicData{core::BuiltinFn::kAtomicAnd, "atomicAnd", "InterlockedAnd"},
                    AtomicData{core::BuiltinFn::kAtomicOr, "atomicOr", "InterlockedOr"},
                    AtomicData{core::BuiltinFn::kAtomicXor, "atomicXor", "InterlockedXor"},
                    AtomicData{core::BuiltinFn::kAtomicExchange, "atomicExchange",
                               "InterlockedExchange"}));

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicStore) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:void = atomicStore %3, 123i
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var 0i
    %5:void = hlsl.InterlockedExchange %3, 123i, %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicLoad) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = atomicLoad %3
    %x:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var 0i
    %5:void = hlsl.InterlockedOr %3, 0i, %4
    %6:i32 = load %4
    %x:i32 = let %6
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicSub) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Let("y", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<u32>, read_write>(), var, 2_u), 123_u));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = atomicSub %3, 123i
    %x:i32 = let %4
    %6:ptr<workgroup, atomic<u32>, read_write> = access %v, 2u
    %7:u32 = atomicSub %6, 123u
    %y:u32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var 0i
    %5:i32 = sub 0i, 123i
    %6:void = hlsl.InterlockedAdd %3, %5, %4
    %7:i32 = load %4
    %x:i32 = let %7
    %9:ptr<workgroup, atomic<u32>, read_write> = access %v, 2u
    %10:ptr<function, u32, read_write> = var 0u
    %11:u32 = sub 0u, 123u
    %12:void = hlsl.InterlockedAdd %9, %11, %10
    %13:u32 = load %10
    %y:u32 = let %13
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i,
                          345_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %3, 123i, 345i
    %x:__atomic_compare_exchange_result_i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var 0i
    %5:void = hlsl.InterlockedCompareExchange %3, 123i, 345i, %4
    %6:i32 = load %4
    %7:bool = eq %6, 123i
    %8:__atomic_compare_exchange_result_i32 = construct %6, %7
    %x:__atomic_compare_exchange_result_i32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack2x16Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec2<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack2X16Float, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec2<f32>, read_write> = var vec2<f32>(2.0f)
    %3:vec2<f32> = load %u
    %4:u32 = pack2x16float %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec2<f32>, read_write> = var vec2<f32>(2.0f)
    %3:vec2<f32> = load %u
    %4:vec2<u32> = hlsl.f32tof16 %3
    %5:u32 = swizzle %4, x
    %6:u32 = swizzle %4, y
    %7:u32 = shl %6, 16u
    %8:u32 = or %5, %7
    %a:u32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack2x16Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Float, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec2<f32> = unpack2x16float %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:u32 = and %3, 65535u
    %5:u32 = shr %3, 16u
    %6:vec2<u32> = construct %4, %5
    %7:vec2<f32> = hlsl.f16tof32 %6
    %a:vec2<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack2x16Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec2<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack2X16Snorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec2<f32>, read_write> = var vec2<f32>(2.0f)
    %3:vec2<f32> = load %u
    %4:u32 = pack2x16snorm %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec2<f32>, read_write> = var vec2<f32>(2.0f)
    %3:vec2<f32> = load %u
    %4:vec2<f32> = clamp %3, vec2<f32>(-1.0f), vec2<f32>(1.0f)
    %5:vec2<f32> = mul %4, 32767.0f
    %6:vec2<f32> = round %5
    %7:vec2<i32> = convert %6
    %8:vec2<i32> = and %7, vec2<i32>(65535i)
    %9:i32 = swizzle %8, x
    %10:i32 = swizzle %8, y
    %11:i32 = shl %10, 16u
    %12:i32 = or %9, %11
    %13:u32 = hlsl.asuint %12
    %a:u32 = let %13
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack2x16snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Snorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec2<f32> = unpack2x16snorm %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:i32 = convert %3
    %5:i32 = shl %4, 16u
    %6:vec2<i32> = construct %5, %4
    %7:vec2<i32> = shr %6, vec2<u32>(16u)
    %8:vec2<f32> = convert %7
    %9:vec2<f32> = div %8, 32767.0f
    %10:vec2<f32> = clamp %9, vec2<f32>(-1.0f), vec2<f32>(1.0f)
    %a:vec2<f32> = let %10
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack2x16unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec2<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack2X16Unorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec2<f32>, read_write> = var vec2<f32>(2.0f)
    %3:vec2<f32> = load %u
    %4:u32 = pack2x16unorm %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec2<f32>, read_write> = var vec2<f32>(2.0f)
    %3:vec2<f32> = load %u
    %4:vec2<f32> = clamp %3, vec2<f32>(0.0f), vec2<f32>(1.0f)
    %5:vec2<f32> = mul %4, 65535.0f
    %6:vec2<f32> = round %5
    %7:vec2<u32> = convert %6
    %8:u32 = swizzle %7, x
    %9:u32 = swizzle %7, y
    %10:u32 = shl %9, 16u
    %11:u32 = or %8, %10
    %a:u32 = let %11
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack2x16unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Unorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec2<f32> = unpack2x16unorm %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:u32 = and %3, 65535u
    %5:u32 = shr %3, 16u
    %6:vec2<u32> = construct %4, %5
    %7:vec2<f32> = convert %6
    %8:vec2<f32> = div %7, 65535.0f
    %a:vec2<f32> = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack4x8Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4X8Snorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<f32>, read_write> = var vec4<f32>(2.0f)
    %3:vec4<f32> = load %u
    %4:u32 = pack4x8snorm %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<f32>, read_write> = var vec4<f32>(2.0f)
    %3:vec4<f32> = load %u
    %4:vec4<f32> = clamp %3, vec4<f32>(-1.0f), vec4<f32>(1.0f)
    %5:vec4<f32> = mul %4, 127.0f
    %6:vec4<f32> = round %5
    %7:vec4<i32> = convert %6
    %8:vec4<i32> = and %7, vec4<i32>(255i)
    %9:i32 = swizzle %8, x
    %10:i32 = swizzle %8, y
    %11:i32 = shl %10, 8u
    %12:i32 = swizzle %8, z
    %13:i32 = shl %12, 16u
    %14:i32 = swizzle %8, w
    %15:i32 = shl %14, 24u
    %16:i32 = or %13, %15
    %17:i32 = or %11, %16
    %18:i32 = or %9, %17
    %19:u32 = hlsl.asuint %18
    %a:u32 = let %19
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4x8Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Snorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec4<f32> = unpack4x8snorm %3
    %a:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:i32 = convert %3
    %5:i32 = shl %4, 24u
    %6:i32 = shl %4, 16u
    %7:i32 = shl %4, 8u
    %8:vec4<i32> = construct %5, %6, %7, %4
    %9:vec4<i32> = shr %8, vec4<u32>(24u)
    %10:vec4<f32> = convert %9
    %11:vec4<f32> = div %10, 127.0f
    %12:vec4<f32> = clamp %11, vec4<f32>(-1.0f), vec4<f32>(1.0f)
    %a:vec4<f32> = let %12
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack4x8Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4X8Unorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<f32>, read_write> = var vec4<f32>(2.0f)
    %3:vec4<f32> = load %u
    %4:u32 = pack4x8unorm %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<f32>, read_write> = var vec4<f32>(2.0f)
    %3:vec4<f32> = load %u
    %4:vec4<f32> = clamp %3, vec4<f32>(0.0f), vec4<f32>(1.0f)
    %5:vec4<f32> = mul %4, 255.0f
    %6:vec4<f32> = round %5
    %7:vec4<u32> = convert %6
    %8:u32 = swizzle %7, x
    %9:u32 = swizzle %7, y
    %10:u32 = shl %9, 8u
    %11:u32 = swizzle %7, z
    %12:u32 = shl %11, 16u
    %13:u32 = swizzle %7, w
    %14:u32 = shl %13, 24u
    %15:u32 = or %12, %14
    %16:u32 = or %10, %15
    %17:u32 = or %8, %16
    %a:u32 = let %17
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4x8Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Unorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec4<f32> = unpack4x8unorm %3
    %a:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:u32 = and %3, 255u
    %5:u32 = shr %3, 8u
    %6:u32 = and %5, 255u
    %7:u32 = shr %3, 16u
    %8:u32 = and %7, 255u
    %9:u32 = shr %3, 24u
    %10:vec4<u32> = construct %4, %6, %8, %9
    %11:vec4<f32> = convert %10
    %12:vec4<f32> = div %11, 255.0f
    %a:vec4<f32> = let %12
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack4xI8) {
    capabilities = core::ir::Capabilities{
        core::ir::Capability::kAllowNonCoreTypes,
    };

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<i32>(), 2_i));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XI8, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<i32>, read_write> = var vec4<i32>(2i)
    %3:vec4<i32> = load %u
    %4:u32 = pack4xI8 %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<i32>, read_write> = var vec4<i32>(2i)
    %3:vec4<i32> = load %u
    %4:hlsl.int8_t4_packed = hlsl.pack_s8 %3
    %5:u32 = hlsl.convert %4
    %a:u32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4xI8) {
    capabilities = core::ir::Capabilities{
        core::ir::Capability::kAllowNonCoreTypes,
    };

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<i32>(), core::BuiltinFn::kUnpack4XI8, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec4<i32> = unpack4xI8 %3
    %a:vec4<i32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:hlsl.int8_t4_packed = hlsl.convert<hlsl.int8_t4_packed> %3
    %5:vec4<i32> = hlsl.unpack_s8s32 %4
    %a:vec4<i32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack4xU8) {
    capabilities = core::ir::Capabilities{
        core::ir::Capability::kAllowNonCoreTypes,
    };

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<u32>(), 2_u));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XU8, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<u32>, read_write> = var vec4<u32>(2u)
    %3:vec4<u32> = load %u
    %4:u32 = pack4xU8 %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<u32>, read_write> = var vec4<u32>(2u)
    %3:vec4<u32> = load %u
    %4:hlsl.uint8_t4_packed = hlsl.pack_u8 %3
    %5:u32 = hlsl.convert %4
    %a:u32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4xU8) {
    capabilities = core::ir::Capabilities{
        core::ir::Capability::kAllowNonCoreTypes,
    };

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<u32>(), core::BuiltinFn::kUnpack4XU8, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:vec4<u32> = unpack4xU8 %3
    %a:vec4<u32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:hlsl.uint8_t4_packed = hlsl.convert<hlsl.uint8_t4_packed> %3
    %5:vec4<u32> = hlsl.unpack_u8u32 %4
    %a:vec4<u32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Dot4U8Packed) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kDot4U8Packed, b.Load(u), u32(3_u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:u32 = dot4U8Packed %3, 3u
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %accumulator:ptr<function, u32, read_write> = var 0u
    %5:u32 = hlsl.dot4add_u8packed %3, 3u, %accumulator
    %a:u32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Dot4I8Packed) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kDot4I8Packed, b.Load(u), u32(3_u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %4:i32 = dot4I8Packed %3, 3u
    %a:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var 2u
    %3:u32 = load %u
    %accumulator:ptr<function, i32, read_write> = var 0i
    %5:i32 = hlsl.dot4add_i8packed %3, 3u, %accumulator
    %a:i32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Pack4xI8Clamp) {
    capabilities = core::ir::Capabilities{
        core::ir::Capability::kAllowNonCoreTypes,
    };

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<i32>(), 2_i));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XI8Clamp, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<i32>, read_write> = var vec4<i32>(2i)
    %3:vec4<i32> = load %u
    %4:u32 = pack4xI8Clamp %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, vec4<i32>, read_write> = var vec4<i32>(2i)
    %3:vec4<i32> = load %u
    %4:hlsl.int8_t4_packed = hlsl.pack_clamp_s8 %3
    %5:u32 = hlsl.convert %4
    %a:u32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asinh) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 0.25_f);
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kAsinh, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, f32, read_write> = var 0.25f
    %3:f32 = load %u
    %4:f32 = asinh %3
    %a:f32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, f32, read_write> = var 0.25f
    %3:f32 = load %u
    %4:f32 = mul %3, %3
    %5:f32 = add %4, 1.0f
    %6:f32 = sqrt %5
    %7:f32 = add %3, %6
    %8:f32 = log %7
    %a:f32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Acosh) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 1.25_h);
        b.Let("a", b.Call(ty.f16(), core::BuiltinFn::kAcosh, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, f16, read_write> = var 1.25h
    %3:f16 = load %u
    %4:f16 = acosh %3
    %a:f16 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, f16, read_write> = var 1.25h
    %3:f16 = load %u
    %4:f16 = mul %3, %3
    %5:f16 = sub %4, 1.0h
    %6:f16 = sqrt %5
    %7:f16 = add %3, %6
    %8:f16 = log %7
    %a:f16 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Atanh) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 0.25_f);
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kAtanh, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, f32, read_write> = var 0.25f
    %3:f32 = load %u
    %4:f32 = atanh %3
    %a:f32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, f32, read_write> = var 0.25f
    %3:f32 = load %u
    %4:f32 = add 1.0f, %3
    %5:f32 = sub 1.0f, %3
    %6:f32 = div %4, %5
    %7:f32 = log %6
    %8:f32 = mul %7, 0.5f
    %a:f32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, CountOneBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 1_i);
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kCountOneBits, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %u
    %4:i32 = countOneBits %3
    %a:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %u
    %4:u32 = hlsl.asuint %3
    %5:u32 = countOneBits %4
    %6:i32 = hlsl.asint %5
    %a:i32 = let %6
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, ReverseBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 1_i);
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kReverseBits, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %u
    %4:i32 = reverseBits %3
    %a:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, i32, read_write> = var 1i
    %3:i32 = load %u
    %4:u32 = hlsl.asuint %3
    %5:u32 = reverseBits %4
    %6:i32 = hlsl.asint %5
    %a:i32 = let %6
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SubgroupAndLiteralVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.vec3<i32>(), core::BuiltinFn::kSubgroupAnd,
                          b.Composite(ty.vec3<i32>(), 1_i, 1_i, 1_i)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<i32> = subgroupAnd vec3<i32>(1i)
    %a:vec3<i32> = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %arg:vec3<i32> = let vec3<i32>(1i)
    %3:vec3<u32> = hlsl.asuint %arg
    %4:vec3<u32> = subgroupAnd %3
    %5:vec3<i32> = hlsl.asint %4
    %a:vec3<i32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SubgroupShuffleXor) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupShuffleXor, 1_i, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = subgroupShuffleXor 1i, 1u
    %a:i32 = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = hlsl.WaveGetLaneIndex
    %3:u32 = xor %2, 1u
    %4:i32 = hlsl.WaveReadLaneAt 1i, %3
    %a:i32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SubgroupInclusiveAdd) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let_a = b.Let("a", 2_f);
        b.Let("b", b.Call(ty.f32(), core::BuiltinFn::kSubgroupInclusiveAdd, let_a->Result()));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 2.0f
    %3:f32 = subgroupInclusiveAdd %a
    %b:f32 = let %3
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 2.0f
    %3:f32 = subgroupExclusiveAdd %a
    %4:f32 = add %3, %a
    %b:f32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SubgroupInclusiveMul) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let_a = b.Let("a", 2_f);
        b.Let("b", b.Call(ty.f32(), core::BuiltinFn::kSubgroupInclusiveMul, let_a->Result()));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 2.0f
    %3:f32 = subgroupInclusiveMul %a
    %b:f32 = let %3
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 2.0f
    %3:f32 = subgroupExclusiveMul %a
    %4:f32 = mul %3, %a
    %b:f32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SubgroupShuffleUp) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupShuffleUp, 1_i, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = subgroupShuffleUp 1i, 1u
    %a:i32 = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = hlsl.WaveGetLaneIndex
    %3:u32 = sub %2, 1u
    %4:i32 = hlsl.WaveReadLaneAt 1i, %3
    %a:i32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SubgroupShuffleDown) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupShuffleDown, 1_i, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = subgroupShuffleDown 1i, 1u
    %a:i32 = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = hlsl.WaveGetLaneIndex
    %3:u32 = add %2, 1u
    %4:i32 = hlsl.WaveReadLaneAt 1i, %3
    %a:i32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Modf_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arg_ty = ty.f32();
        auto* v = b.Var(ty.ptr<function>(arg_ty));
        b.Let("a", b.Call(core::type::CreateModfResult(ty, mod.symbols, arg_ty),
                          core::BuiltinFn::kModf, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:f32 = load %2
    %4:__modf_result_f32 = modf %3
    %a:__modf_result_f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:f32 = load %2
    %4:ptr<function, f32, read_write> = var undef
    %5:f32 = load %4
    %6:f32 = hlsl.modf %3, %5
    %7:f32 = load %4
    %8:__modf_result_f32 = construct %6, %7
    %a:__modf_result_f32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Modf_vec_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arg_ty = ty.vec3<f32>();
        auto* v = b.Var(ty.ptr<function>(arg_ty));
        b.Let("a", b.Call(core::type::CreateModfResult(ty, mod.symbols, arg_ty),
                          core::BuiltinFn::kModf, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
__modf_result_vec3_f32 = struct @align(16) {
  fract:vec3<f32> @offset(0)
  whole:vec3<f32> @offset(16)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var undef
    %3:vec3<f32> = load %2
    %4:__modf_result_vec3_f32 = modf %3
    %a:__modf_result_vec3_f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_vec3_f32 = struct @align(16) {
  fract:vec3<f32> @offset(0)
  whole:vec3<f32> @offset(16)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var undef
    %3:vec3<f32> = load %2
    %4:ptr<function, vec3<f32>, read_write> = var undef
    %5:vec3<f32> = load %4
    %6:vec3<f32> = hlsl.modf %3, %5
    %7:vec3<f32> = load %4
    %8:__modf_result_vec3_f32 = construct %6, %7
    %a:__modf_result_vec3_f32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Frexp_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arg_ty = ty.f32();
        auto* v = b.Var(ty.ptr<function>(arg_ty));
        b.Let("a", b.Call(core::type::CreateFrexpResult(ty, mod.symbols, arg_ty),
                          core::BuiltinFn::kFrexp, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:f32 = load %2
    %4:__frexp_result_f32 = frexp %3
    %a:__frexp_result_f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:f32 = load %2
    %4:ptr<function, f32, read_write> = var undef
    %5:f32 = load %4
    %6:f32 = hlsl.frexp %3, %5
    %7:i32 = hlsl.sign %3
    %8:f32 = convert %7
    %9:f32 = mul %8, %6
    %10:f32 = load %4
    %11:i32 = convert %10
    %12:__frexp_result_f32 = construct %9, %11
    %a:__frexp_result_f32 = let %12
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Frexp_vec_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arg_ty = ty.vec3<f32>();
        auto* v = b.Var(ty.ptr<function>(arg_ty));
        b.Let("a", b.Call(core::type::CreateFrexpResult(ty, mod.symbols, arg_ty),
                          core::BuiltinFn::kFrexp, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
__frexp_result_vec3_f32 = struct @align(16) {
  fract:vec3<f32> @offset(0)
  exp:vec3<i32> @offset(16)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var undef
    %3:vec3<f32> = load %2
    %4:__frexp_result_vec3_f32 = frexp %3
    %a:__frexp_result_vec3_f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_vec3_f32 = struct @align(16) {
  fract:vec3<f32> @offset(0)
  exp:vec3<i32> @offset(16)
}

%foo = @fragment func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var undef
    %3:vec3<f32> = load %2
    %4:ptr<function, vec3<f32>, read_write> = var undef
    %5:vec3<f32> = load %4
    %6:vec3<f32> = hlsl.frexp %3, %5
    %7:vec3<i32> = hlsl.sign %3
    %8:vec3<f32> = convert %7
    %9:vec3<f32> = mul %8, %6
    %10:vec3<f32> = load %4
    %11:vec3<i32> = convert %10
    %12:__frexp_result_vec3_f32 = construct %9, %11
    %a:__frexp_result_vec3_f32 = let %12
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
