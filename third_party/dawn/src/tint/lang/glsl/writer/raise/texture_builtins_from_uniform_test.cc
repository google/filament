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
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
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
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
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
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, u32, read> = access %1, 0u
    %4:u32 = load %3
    %len:u32 = let %4
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumSamples) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d),
        read_write));
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
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
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
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, u32, read> = access %1, 0u
    %4:u32 = load %3
    %len:u32 = let %4
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumLevels) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
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
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
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
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, u32, read> = access %1, 0u
    %4:u32 = load %3
    %len:u32 = let %4
    %6:ptr<uniform, u32, read> = access %1, 0u
    %7:u32 = load %6
    %len2:u32 = let %7
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumSamples) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d),
        read_write));
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
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
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
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, u32, read> = access %1, 0u
    %4:u32 = load %3
    %len:u32 = let %4
    %6:ptr<uniform, u32, read> = access %1, 0u
    %7:u32 = load %6
    %len2:u32 = let %7
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
