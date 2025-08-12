// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/raise/texture_builtins_from_uniform.h"

#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/common/options.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_TextureBuiltinsFromUniformTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumLevels) {
    auto* t = b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                           core::Access::kRead));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = div 0u, 4u
    %4:u32 = mod 0u, 4u
    %5:ptr<uniform, vec4<u32>, read> = access %1, 0u, %3
    %6:vec4<u32> = load %5
    %7:u32 = access %6, %4
    %len:u32 = let %7
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 1, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumSamples) {
    auto* t = b.Var(ty.ptr(handle, ty.depth_multisampled_texture(core::type::TextureDimension::k2d),
                           core::Access::kRead));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %1
    %4:u32 = textureNumSamples %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = div 0u, 4u
    %4:u32 = mod 0u, 4u
    %5:ptr<uniform, vec4<u32>, read> = access %1, 0u, %3
    %6:vec4<u32> = load %5
    %7:u32 = access %6, %4
    %len:u32 = let %7
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 1, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumLevelsNonZeroMetadataOffset) {
    auto* t = b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                           core::Access::kRead));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 11> @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = div 42u, 4u
    %4:u32 = mod 42u, 4u
    %5:ptr<uniform, vec4<u32>, read> = access %1, 0u, %3
    %6:vec4<u32> = load %5
    %7:u32 = access %6, %4
    %len:u32 = let %7
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 42, .count = 1, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest,
       TextureNumLevelsBindindArrayAccessThroughPointer) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* textures = b.Var(ty.ptr<handle>(ty.binding_array(texture_type, 3)));
    textures->SetBindingPoint(0, 0);
    b.ir.root_block->Append(textures);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* ptr_texture = b.Access(ty.ptr<handle>(texture_type), textures, 1_u);
        auto* texture = b.Load(ptr_texture);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, texture));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<handle, texture_2d<f32>, read> = access %1, 1u
    %4:texture_2d<f32> = load %3
    %5:u32 = textureNumLevels %4
    %len:u32 = let %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = add 0u, 1u
    %5:u32 = div %4, 4u
    %6:u32 = mod %4, 4u
    %7:ptr<uniform, vec4<u32>, read> = access %2, 0u, %5
    %8:vec4<u32> = load %7
    %9:u32 = access %8, %6
    %len:u32 = let %9
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 3, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumLevelsBindindArrayAccessThroughValue) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* textures = b.Var(ty.ptr<handle>(ty.binding_array(texture_type, 3)));
    textures->SetBindingPoint(0, 0);
    b.ir.root_block->Append(textures);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* textures_value = b.Load(textures);
        auto* texture = b.Access(texture_type, textures_value, 1_u);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, texture));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:binding_array<texture_2d<f32>, 3> = load %1
    %4:texture_2d<f32> = access %3, 1u
    %5:u32 = textureNumLevels %4
    %len:u32 = let %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:binding_array<texture_2d<f32>, 3> = load %1
    %5:texture_2d<f32> = access %4, 1u
    %6:u32 = add 0u, 1u
    %7:u32 = div %6, 4u
    %8:u32 = mod %6, 4u
    %9:ptr<uniform, vec4<u32>, read> = access %2, 0u, %7
    %10:vec4<u32> = load %9
    %11:u32 = access %10, %8
    %len:u32 = let %11
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 3, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest,
       TextureNumLevelsBindindArrayAccessThroughValue_I32Index) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* textures = b.Var(ty.ptr<handle>(ty.binding_array(texture_type, 3)));
    textures->SetBindingPoint(0, 0);
    b.ir.root_block->Append(textures);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* textures_value = b.Load(textures);
        auto* texture = b.Access(texture_type, textures_value, 1_i);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, texture));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:binding_array<texture_2d<f32>, 3> = load %1
    %4:texture_2d<f32> = access %3, 1i
    %5:u32 = textureNumLevels %4
    %len:u32 = let %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:binding_array<texture_2d<f32>, 3> = load %1
    %5:texture_2d<f32> = access %4, 1i
    %6:u32 = convert 1i
    %7:u32 = add 0u, %6
    %8:u32 = div %7, 4u
    %9:u32 = mod %7, 4u
    %10:ptr<uniform, vec4<u32>, read> = access %2, 0u, %8
    %11:vec4<u32> = load %10
    %12:u32 = access %11, %9
    %len:u32 = let %12
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 3, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumLevels) {
    auto* t = b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                           core::Access::kRead));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    %6:u32 = textureNumLevels %3
    %len2:u32 = let %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = div 0u, 4u
    %4:u32 = mod 0u, 4u
    %5:ptr<uniform, vec4<u32>, read> = access %1, 0u, %3
    %6:vec4<u32> = load %5
    %7:u32 = access %6, %4
    %len:u32 = let %7
    %9:u32 = div 0u, 4u
    %10:u32 = mod 0u, 4u
    %11:ptr<uniform, vec4<u32>, read> = access %1, 0u, %9
    %12:vec4<u32> = load %11
    %13:u32 = access %12, %10
    %len2:u32 = let %13
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 1, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumSamples) {
    auto* t = b.Var(ty.ptr(handle, ty.depth_multisampled_texture(core::type::TextureDimension::k2d),
                           core::Access::kRead));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %1
    %4:u32 = textureNumSamples %3
    %len:u32 = let %4
    %6:u32 = textureNumSamples %3
    %len2:u32 = let %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(16) {
  metadata:array<vec4<u32>, 1> @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var undef @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = div 0u, 4u
    %4:u32 = mod 0u, 4u
    %5:ptr<uniform, vec4<u32>, read> = access %1, 0u, %3
    %6:vec4<u32> = load %5
    %7:u32 = access %6, %4
    %len:u32 = let %7
    %9:u32 = div 0u, 4u
    %10:u32 = mod 0u, 4u
    %11:ptr<uniform, vec4<u32>, read> = access %1, 0u, %9
    %12:vec4<u32> = load %11
    %13:u32 = access %12, %10
    %len2:u32 = let %13
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{30u}, {{.offset = 0, .count = 1, .binding = {0}}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
