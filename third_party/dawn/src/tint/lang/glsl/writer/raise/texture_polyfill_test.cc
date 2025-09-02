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

#include "src/tint/lang/glsl/writer/raise/texture_polyfill.h"

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

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_TexturePolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_1d) {
    auto* var = b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()),
                      core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, b.Load(var));
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:texture_1d<f32> = load %v
    %4:u32 = textureDimensions %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:texture_2d<f32> = load %v
    %4:vec2<i32> = glsl.textureSize %3, 0i
    %5:vec2<u32> = bitcast %4
    %6:u32 = swizzle %5, x
    ret %6
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_2d_WithoutLod) {
    auto* var = b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                      core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var));
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_2d<f32> = load %v
    %4:vec2<u32> = textureDimensions %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_2d<f32> = load %v
    %4:vec2<i32> = glsl.textureSize %3, 0i
    %5:vec2<u32> = bitcast %4
    ret %5
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_2d_WithU32Lod) {
    auto* var = b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                      core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var), 3_u);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_2d<f32> = load %v
    %4:vec2<u32> = textureDimensions %3, 3u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_2d<f32> = load %v
    %4:i32 = bitcast 3u
    %5:vec2<i32> = glsl.textureSize %3, %4
    %6:vec2<u32> = bitcast %5
    ret %6
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_2dArray) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:vec2<u32> = textureDimensions %3
    %x:vec2<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:vec2<i32> = swizzle %4, xy
    %6:vec2<u32> = bitcast %5
    %x:vec2<u32> = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_Storage2D) {
    auto* type = ty.storage_texture(core::type::TextureDimension::k2d,
                                    core::TexelFormat::kRg32Float, core::Access::kRead);
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<rg32float, read> = load %v
    %4:vec2<u32> = textureDimensions %3
    %x:vec2<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<rg32float, read> = load %v
    %4:vec2<i32> = glsl.imageSize %3
    %5:vec2<u32> = bitcast %4
    %x:vec2<u32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_DepthMultisampled) {
    auto* type = ty.depth_multisampled_texture(core::type::TextureDimension::k2d);
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_multisampled_2d, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %v
    %4:vec2<u32> = textureDimensions %3
    %x:vec2<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_multisampled_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %v
    %4:vec2<i32> = glsl.textureSize %3
    %5:vec2<u32> = bitcast %4
    %x:vec2<u32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_2DArray) {
    auto* var =
        b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()),
              core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_Depth2DArray) {
    auto* var = b.Var("v", handle, ty.depth_texture(core::type::TextureDimension::k2dArray),
                      core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_2d_array, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_2d_array = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_CubeArray) {
    auto* var =
        b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32()),
              core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_cube_array<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_cube_array<f32> = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_cube_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_cube_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_DepthCubeArray) {
    auto* var = b.Var("v", handle, ty.depth_texture(core::type::TextureDimension::kCubeArray),
                      core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_cube_array, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_cube_array = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_cube_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_cube_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_Storage2DArray) {
    auto* storage_ty = ty.storage_texture(core::type::TextureDimension::k2dArray,
                                          core::TexelFormat::kRg32Float, core::Access::kRead);

    auto* var = b.Var("v", handle, storage_ty, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d_array<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rg32float, read> = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d_array<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rg32float, read> = load %v
    %4:vec3<i32> = glsl.imageSize %3
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_1DF32) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<i32>();
        auto* level = b.Zero<u32>();
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(var), coords, level));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_1d<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_1d<f32> = load %v
    %4:vec4<f32> = textureLoad %3, 0i, 0u
    %x:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<i32> = construct 0i, 0i
    %4:texture_2d<f32> = load %v
    %5:i32 = convert 0u
    %6:vec4<f32> = glsl.texelFetch %4, %3, %5
    %x:vec4<f32> = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_2DLevelI32) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32());
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<i32>();
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, b.Load(var), coords, level));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<i32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d<i32> = load %v
    %4:vec4<i32> = textureLoad %3, vec2<i32>(0i), 0i
    %x:vec4<i32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d<i32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d<i32> = load %v
    %4:vec4<i32> = glsl.texelFetch %3, vec2<i32>(0i), 0i
    %x:vec4<i32> = let %4
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_3DLevelU32) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec3<i32>>();
        auto* level = b.Zero<u32>();
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(var), coords, level));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_3d<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_3d<f32> = load %v
    %4:vec4<f32> = textureLoad %3, vec3<i32>(0i), 0u
    %x:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_3d<f32> = load %v
    %4:i32 = convert 0u
    %5:vec4<f32> = glsl.texelFetch %3, vec3<i32>(0i), %4
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_Multisampled2DI32) {
    auto* type = ty.multisampled_texture(core::type::TextureDimension::k2d, ty.i32());
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* sample_idx = b.Zero<u32>();
        b.Let("x",
              b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, b.Load(var), coords, sample_idx));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_multisampled_2d<i32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_multisampled_2d<i32> = load %v
    %4:vec4<i32> = textureLoad %3, vec2<i32>(0i), 0u
    %x:vec4<i32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_multisampled_2d<i32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_multisampled_2d<i32> = load %v
    %4:i32 = convert 0u
    %5:vec4<i32> = glsl.texelFetch %3, vec2<i32>(0i), %4
    %x:vec4<i32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_Storage2D) {
    auto* type = ty.storage_texture(core::type::TextureDimension::k2d,
                                    core::TexelFormat::kRg32Float, core::Access::kRead);
    auto* var = b.Var("v", handle, type, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(var), coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<rg32float, read> = load %v
    %4:vec4<f32> = textureLoad %3, vec2<i32>(0i)
    %x:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d<rg32float, read>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<rg32float, read> = load %v
    %4:vec4<f32> = glsl.imageLoad %3, vec2<i32>(0i)
    %x:vec4<f32> = let %4
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStore1D) {
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
  %1:ptr<handle, texture_storage_2d<r32float, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<r32float, read_write> = load %1
    %4:vec2<i32> = construct 1i, 0i
    %5:void = glsl.imageStore %3, %4, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStore2D) {
    auto* t = b.Var(ty.ptr(
        handle, ty.storage_texture(core::type::TextureDimension::k2d,
                                   core::TexelFormat::kRgba32Sint, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Splat(ty.vec2<u32>(), 0_u);
        auto* value = b.Composite(ty.vec4<i32>(), 5_i, 0_i, 0_i, 1_i);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d<rgba32sint, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<rgba32sint, read_write> = load %1
    %4:void = textureStore %3, vec2<u32>(0u), vec4<i32>(5i, 0i, 0i, 1i)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d<rgba32sint, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<rgba32sint, read_write> = load %1
    %4:vec2<i32> = convert vec2<u32>(0u)
    %5:void = glsl.imageStore %3, %4, vec4<i32>(5i, 0i, 0i, 1i)
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStore2DArray) {
    auto* t = b.Var(ty.ptr(
        handle, ty.storage_texture(core::type::TextureDimension::k2dArray,
                                   core::TexelFormat::kRgba32Sint, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Splat(ty.vec2<u32>(), 0_u);
        auto* value = b.Composite(ty.vec4<i32>(), 5_i, 0_i, 0_i, 1_i);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, 1_i, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d_array<rgba32sint, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rgba32sint, read_write> = load %1
    %4:void = textureStore %3, vec2<u32>(0u), 1i, vec4<i32>(5i, 0i, 0i, 1i)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d_array<rgba32sint, read_write>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rgba32sint, read_write> = load %1
    %4:vec2<i32> = convert vec2<u32>(0u)
    %5:vec3<i32> = construct %4, 1i
    %6:void = glsl.imageStore %3, %5, vec4<i32>(5i, 0i, 0i, 1i)
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStore3D) {
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
    %4:void = glsl.imageStore %3, vec3<i32>(1i, 2i, 3i), vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStoreArray) {
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
    %6:void = glsl.imageStore %3, %5, vec4<f32>(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f)
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, CombineSamplers_GlobalTextureNoSampler) {
    auto* t = b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                           core::Access::kRead));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, tex);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:vec2<u32> = textureDimensions %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %t:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_2d<f32> = load %t
    %4:vec2<i32> = glsl.textureSize %3, 0i
    %5:vec2<u32> = bitcast %4
    ret %5
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGatherCompare_Depth2d) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec4<f32> = glsl.textureGather %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGatherCompare_Depth2dOffset) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec2<i32> = construct 4i, 5i
    %5:texture_depth_2d = load %t_s
    %6:vec4<f32> = glsl.textureGatherOffset %5, %3, 3.0f, %4
    %x:vec4<f32> = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGatherCompare_DepthCubeArray) {
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
  %t_s:ptr<handle, texture_depth_cube_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 2.5f
    %4:texture_depth_cube_array = load %t_s
    %5:f32 = convert 6u
    %6:vec4<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureGather %4, %6, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGatherCompare_Depth2dArrayOffset) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec2<i32> = construct 4i, 5i
    %5:texture_depth_2d_array = load %t_s
    %6:f32 = convert 6i
    %7:vec3<f32> = construct %3, %6
    %8:vec4<f32> = glsl.textureGatherOffset %5, %7, 3.0f, %4
    %x:vec4<f32> = let %8
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_Alpha) {
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
  %t_s:ptr<handle, texture_2d<i32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<i32> = load %t_s
    %5:i32 = convert 3u
    %6:vec4<i32> = glsl.textureGather %4, %3, %5
    %x:vec4<i32> = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}
TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_RedOffset) {
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
  %t_s:ptr<handle, texture_2d<i32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<i32> = load %t_s
    %5:i32 = convert 0u
    %6:vec4<i32> = glsl.textureGatherOffset %4, %3, vec2<i32>(1i, 3i), %5
    %x:vec4<i32> = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}
TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_GreenArray) {
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
  %t_s:ptr<handle, texture_2d_array<i32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<i32> = load %t_s
    %5:f32 = convert 1u
    %6:vec3<f32> = construct %3, %5
    %7:i32 = convert 1u
    %8:vec4<i32> = glsl.textureGather %4, %6, %7
    %x:vec4<i32> = let %8
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_BlueArrayOffset) {
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
  %t_s:ptr<handle, texture_2d_array<i32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<i32> = load %t_s
    %5:f32 = convert 1i
    %6:vec3<f32> = construct %3, %5
    %7:i32 = convert 2u
    %8:vec4<i32> = glsl.textureGatherOffset %4, %6, vec2<i32>(1i, 2i), %7
    %x:vec4<i32> = let %8
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_Depth) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec4<f32> = glsl.textureGather %4, %3, 0.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}
TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_DepthOffset) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec4<f32> = glsl.textureGatherOffset %4, %3, 0.0f, vec2<i32>(3i, 4i)
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_DepthArray) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4i
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureGather %4, %6, 0.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureGather_DepthArrayOffset) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureGatherOffset %4, %6, 0.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_1d) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 0.5f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_2d) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_2d_Offset) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureOffset %4, %3, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_2d_Array) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.texture %4, %6
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureOffset %4, %6, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_3d) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_3d<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_3d_Offset) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_3d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureOffset %4, %3, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_Cube) {
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
  %t_s:ptr<handle, texture_cube<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_cube<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_Cube_Array) {
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
  %t_s:ptr<handle, texture_cube_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_cube_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:vec4<f32> = glsl.texture %4, %6
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_Depth2d) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 0.0f
    %6:f32 = glsl.texture %4, %5
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_Depth2d_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 0.0f
    %6:f32 = glsl.textureOffset %4, %5, vec2<i32>(4i, 5i)
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_Depth2d_Array) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 0.0f
    %7:f32 = glsl.texture %4, %6
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_Depth2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 0.0f
    %7:vec2<f32> = dpdx %3
    %8:vec2<f32> = dpdy %3
    %9:f32 = glsl.textureGradOffset %4, %6, %7, %8, vec2<i32>(4i, 5i)
    %x:f32 = let %9
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSample_DepthCube_Array) {
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
  %t_s:ptr<handle, texture_depth_cube_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_depth_cube_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:f32 = glsl.texture %4, %6, 0.0f
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_2d) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_2d_Offset) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureOffset %4, %3, vec2<i32>(4i, 5i), 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_2d_Array) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.texture %4, %6, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureOffset %4, %6, vec2<i32>(4i, 5i), 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_3d) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_3d<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_3d_Offset) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_3d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureOffset %4, %3, vec3<i32>(4i, 5i, 6i), 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_Cube) {
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
  %t_s:ptr<handle, texture_cube<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_cube<f32> = load %t_s
    %5:vec4<f32> = glsl.texture %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleBias_Cube_Array) {
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
  %t_s:ptr<handle, texture_cube_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_cube_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:vec4<f32> = glsl.texture %4, %6, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_1d) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 0.5f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureLod %4, %3, 0.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_2d) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureLod %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_2d_Offset) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureLodOffset %4, %3, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_2d_Array) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureLod %4, %6, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_2d_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec3<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureLodOffset %4, %6, 3.0f, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_3d) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_3d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureLod %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_3d_Offset) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_3d<f32> = load %t_s
    %5:vec4<f32> = glsl.textureLodOffset %4, %3, 3.0f, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_Cube) {
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
  %t_s:ptr<handle, texture_cube<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_cube<f32> = load %t_s
    %5:vec4<f32> = glsl.textureLod %4, %3, 3.0f
    %x:vec4<f32> = let %5
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_Cube_Array) {
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
  %t_s:ptr<handle, texture_cube_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_cube_array<f32> = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:vec4<f32> = glsl.textureLod %4, %6, 3.0f
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_Depth2d) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 0.0f
    %6:f32 = convert 3i
    %7:f32 = glsl.textureLod %4, %5, %6
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_Depth2d_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 0.0f
    %6:f32 = convert 3u
    %7:f32 = glsl.textureLodOffset %4, %5, %6, vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_Depth2d_Array) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 0.0f
    %7:f32 = convert 3i
    %8:f32 = glsl.extTextureLod %4, %6, %7
    %x:f32 = let %8
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_Depth2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 0.0f
    %7:f32 = convert 3u
    %8:f32 = glsl.extTextureLodOffset %4, %6, %7, vec2<i32>(4i, 5i)
    %x:f32 = let %8
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleLevel_DepthCube_Array) {
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
  %t_s:ptr<handle, texture_depth_cube_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_depth_cube_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:f32 = convert 3i
    %8:f32 = glsl.extTextureLod %4, %6, 0.0f, %7
    %x:f32 = let %8
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_2d) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec2<f32> = construct 3.0f, 4.0f
    %5:vec2<f32> = construct 6.0f, 7.0f
    %6:texture_2d<f32> = load %t_s
    %7:vec4<f32> = glsl.textureGrad %6, %3, %4, %5
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_2d_Offset) {
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
  %t_s:ptr<handle, texture_2d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec2<f32> = construct 3.0f, 4.0f
    %5:vec2<f32> = construct 6.0f, 7.0f
    %6:texture_2d<f32> = load %t_s
    %7:vec4<f32> = glsl.textureGradOffset %6, %3, %4, %5, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_2d_Array) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec2<f32> = construct 3.0f, 4.0f
    %5:vec2<f32> = construct 6.0f, 7.0f
    %6:texture_2d_array<f32> = load %t_s
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %3, %7
    %9:vec4<f32> = glsl.textureGrad %6, %8, %4, %5
    %x:vec4<f32> = let %9
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_2d_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:vec2<f32> = construct 3.0f, 4.0f
    %5:vec2<f32> = construct 6.0f, 7.0f
    %6:texture_2d_array<f32> = load %t_s
    %7:f32 = convert 4u
    %8:vec3<f32> = construct %3, %7
    %9:vec4<f32> = glsl.textureGradOffset %6, %8, %4, %5, vec2<i32>(4i, 5i)
    %x:vec4<f32> = let %9
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_3d) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %5:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %6:texture_3d<f32> = load %t_s
    %7:vec4<f32> = glsl.textureGrad %6, %3, %4, %5
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_3d_Offset) {
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
  %t_s:ptr<handle, texture_3d<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %5:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %6:texture_3d<f32> = load %t_s
    %7:vec4<f32> = glsl.textureGradOffset %6, %3, %4, %5, vec3<i32>(4i, 5i, 6i)
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_Cube) {
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
  %t_s:ptr<handle, texture_cube<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %5:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %6:texture_cube<f32> = load %t_s
    %7:vec4<f32> = glsl.textureGrad %6, %3, %4, %5
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleGrad_Cube_Array) {
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
  %t_s:ptr<handle, texture_cube_array<f32>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:vec3<f32> = construct 3.0f, 4.0f, 5.0f
    %5:vec3<f32> = construct 6.0f, 7.0f, 8.0f
    %6:texture_cube_array<f32> = load %t_s
    %7:f32 = convert 4u
    %8:vec4<f32> = construct %3, %7
    %9:vec4<f32> = glsl.textureGrad %6, %8, %4, %5
    %x:vec4<f32> = let %9
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompare_2d) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 3.0f
    %6:f32 = glsl.texture %4, %5
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompare_2d_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 3.0f
    %6:f32 = glsl.textureOffset %4, %5, vec2<i32>(4i, 5i)
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompare_2d_Array) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 3.0f
    %7:f32 = glsl.texture %4, %6
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompare_2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 3.0f
    %7:vec2<f32> = dpdx %3
    %8:vec2<f32> = dpdy %3
    %9:f32 = glsl.textureGradOffset %4, %6, %7, %8, vec2<i32>(4i, 5i)
    %x:f32 = let %9
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompare_Cube) {
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
  %t_s:ptr<handle, texture_depth_cube, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_depth_cube = load %t_s
    %5:vec4<f32> = construct %3, 3.0f
    %6:f32 = glsl.texture %4, %5
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompare_Cube_Array) {
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
  %t_s:ptr<handle, texture_depth_cube_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_depth_cube_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:f32 = glsl.texture %4, %6, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompareLevel_2d) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 3.0f
    %6:f32 = glsl.texture %4, %5
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompareLevel_2d_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d = load %t_s
    %5:vec3<f32> = construct %3, 3.0f
    %6:f32 = glsl.textureOffset %4, %5, vec2<i32>(4i, 5i)
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompareLevel_2d_Array) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 3.0f
    %7:f32 = glsl.texture %4, %6
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompareLevel_2d_Array_Offset) {
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
  %t_s:ptr<handle, texture_depth_2d_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:texture_depth_2d_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5, 3.0f
    %7:f32 = glsl.textureGradOffset %4, %6, vec2<f32>(0.0f), vec2<f32>(0.0f), vec2<i32>(4i, 5i)
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompareLevel_Cube) {
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
  %t_s:ptr<handle, texture_depth_cube, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_depth_cube = load %t_s
    %5:vec4<f32> = construct %3, 3.0f
    %6:f32 = glsl.texture %4, %5
    %x:f32 = let %6
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureSampleCompareLevel_Cube_Array) {
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
  %t_s:ptr<handle, texture_depth_cube_array, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %4:texture_depth_cube_array = load %t_s
    %5:f32 = convert 4u
    %6:vec4<f32> = construct %3, %5
    %7:f32 = glsl.texture %4, %6, 3.0f
    %x:f32 = let %7
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {2, 2};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, BindingArray_Texture2d_AccessThroughPointer) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    core::ir::Var* textures = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        textures = b.Var("textures", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
        textures->SetBindingPoint(0, 0);

        sampler = b.Var("sampler", ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* ptr_texture = b.Access(ty.ptr<handle>(texture_type), textures, 1_u);
        auto* t = b.Load(ptr_texture);
        auto* s = b.Load(sampler);
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %textures:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:ptr<handle, texture_2d<f32>, read> = access %textures, 1u
    %6:texture_2d<f32> = load %5
    %7:sampler = load %sampler
    %8:vec4<f32> = textureSample %6, %7, %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %textures_sampler:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:ptr<handle, texture_2d<f32>, read> = access %textures_sampler, 1u
    %5:texture_2d<f32> = load %4
    %6:vec4<f32> = glsl.texture %5, %3
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {4, 0};
    Run(TexturePolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, BindingArray_Texture2d_AccessThroughValue) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    core::ir::Var* textures = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        textures = b.Var("textures", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
        textures->SetBindingPoint(0, 0);

        sampler = b.Var("sampler", ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* textures_value = b.Load(textures);
        auto* t = b.Access(texture_type, textures_value, 1_u);
        auto* s = b.Load(sampler);
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %textures:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:binding_array<texture_2d<f32>, 3> = load %textures
    %6:texture_2d<f32> = access %5, 1u
    %7:sampler = load %sampler
    %8:vec4<f32> = textureSample %6, %7, %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %textures_sampler:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:ptr<handle, texture_2d<f32>, read> = access %textures_sampler, 1u
    %5:texture_2d<f32> = load %4
    %6:vec4<f32> = glsl.texture %5, %3
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {4, 0};
    Run(TexturePolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, BindingArray_Texture2d_TwoSampledUses) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    core::ir::Var* textures = nullptr;
    core::ir::Var* sampler1 = nullptr;
    core::ir::Var* sampler2 = nullptr;
    b.Append(b.ir.root_block, [&] {
        textures = b.Var("textures", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
        textures->SetBindingPoint(0, 0);

        sampler1 = b.Var("sampler1", ty.ptr(handle, ty.sampler()));
        sampler1->SetBindingPoint(0, 1);
        sampler2 = b.Var("sampler2", ty.ptr(handle, ty.sampler()));
        sampler2->SetBindingPoint(0, 2);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* textures_value = b.Load(textures);
        auto* t = b.Access(texture_type, textures_value, 1_u);
        auto* s1 = b.Load(sampler1);
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s1, coords);
        auto* s2 = b.Load(sampler2);
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s2, coords);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %textures:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %sampler1:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
  %sampler2:ptr<handle, sampler, read> = var undef @binding_point(0, 2)
}

%foo = @fragment func():void {
  $B2: {
    %5:vec2<f32> = construct 1.0f, 2.0f
    %6:binding_array<texture_2d<f32>, 3> = load %textures
    %7:texture_2d<f32> = access %6, 1u
    %8:sampler = load %sampler1
    %9:vec4<f32> = textureSample %7, %8, %5
    %10:sampler = load %sampler2
    %11:vec4<f32> = textureSample %7, %10, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %textures_sampler1:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = combined_texture_sampler undef @binding_point(0, 0)
  %textures_sampler2:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:ptr<handle, texture_2d<f32>, read> = access %textures_sampler1, 1u
    %6:texture_2d<f32> = load %5
    %7:vec4<f32> = glsl.texture %6, %4
    %8:ptr<handle, texture_2d<f32>, read> = access %textures_sampler2, 1u
    %9:texture_2d<f32> = load %8
    %10:vec4<f32> = glsl.texture %9, %4
    ret
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowDuplicateBindings};

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {4, 0};
    Run(TexturePolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, BindingArray_Texture2d_SampledAndUnsampledUses) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    core::ir::Var* textures = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        textures = b.Var("textures", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
        textures->SetBindingPoint(0, 0);

        sampler = b.Var("sampler", ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* sample_coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* textures_value = b.Load(textures);
        auto* t = b.Access(texture_type, textures_value, 1_u);
        auto* s = b.Load(sampler);
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, sample_coords);
        auto* load_coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<i32>();
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, load_coords, level);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %textures:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:binding_array<texture_2d<f32>, 3> = load %textures
    %6:texture_2d<f32> = access %5, 1u
    %7:sampler = load %sampler
    %8:vec4<f32> = textureSample %6, %7, %4
    %9:vec4<f32> = textureLoad %6, vec2<i32>(0i), 0i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %textures_sampler:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<f32> = construct 1.0f, 2.0f
    %4:ptr<handle, texture_2d<f32>, read> = access %textures_sampler, 1u
    %5:texture_2d<f32> = load %4
    %6:vec4<f32> = glsl.texture %5, %3
    %7:ptr<handle, texture_2d<f32>, read> = access %textures_sampler, 1u
    %8:texture_2d<f32> = load %7
    %9:vec4<f32> = glsl.texelFetch %8, vec2<i32>(0i), 0i
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {4, 0};
    Run(TexturePolyfill, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, BindingArray_Texture2d_UnsampledUse) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    core::ir::Var* textures = nullptr;
    b.Append(b.ir.root_block, [&] {
        textures = b.Var("textures", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
        textures->SetBindingPoint(0, 0);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* textures_value = b.Load(textures);
        auto* t = b.Access(texture_type, textures_value, 1_u);
        auto* load_coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<i32>();
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, load_coords, level);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %textures:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:binding_array<texture_2d<f32>, 3> = load %textures
    %4:texture_2d<f32> = access %3, 1u
    %5:vec4<f32> = textureLoad %4, vec2<i32>(0i), 0i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %textures:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<handle, texture_2d<f32>, read> = access %textures, 1u
    %4:texture_2d<f32> = load %3
    %5:vec4<f32> = glsl.texelFetch %4, vec2<i32>(0i), 0i
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.placeholder_sampler_bind_point = {4, 0};
    Run(TexturePolyfill, cfg);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
