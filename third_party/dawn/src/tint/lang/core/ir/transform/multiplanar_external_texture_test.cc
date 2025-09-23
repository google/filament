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

#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/transform/multiplanar_options.h"
#include "src/tint/lang/core/type/external_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_MultiplanarExternalTextureTest = TransformTest;

TEST_F(IR_MultiplanarExternalTextureTest, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";

    Run(MultiplanarExternalTexture, tint::transform::multiplanar::BindingsMap{});
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, DeclWithNoUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    ret
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func():void {
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap options{};
    options[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, LoadWithNoUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %3:texture_external = load %texture
    ret
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func():void {
  $B2: {
    %5:texture_2d<f32> = load %texture_plane0
    %6:texture_2d<f32> = load %texture_plane1
    %7:tint_ExternalTextureParams = load %texture_params
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureDimensions) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, load);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_external = load %texture
    %result:vec2<u32> = textureDimensions %3
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func():vec2<u32> {
  $B2: {
    %5:texture_2d<f32> = load %texture_plane0
    %6:texture_2d<f32> = load %texture_plane1
    %7:tint_ExternalTextureParams = load %texture_params
    %8:vec2<u32> = access %7, 12u
    %result:vec2<u32> = add %8, vec2<u32>(1u)
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureLoad) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coords);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>):vec4<f32> {
  $B2: {
    %4:texture_external = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func(%coords:vec2<u32>):vec4<f32> {
  $B2: {
    %6:texture_2d<f32> = load %texture_plane0
    %7:texture_2d<f32> = load %texture_plane1
    %8:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %tint_TextureLoadExternal, %6, %7, %8, %coords
    ret %result
  }
}
%tint_TextureLoadExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %coords_1: 'coords'
  $B3: {
    %15:u32 = access %params, 1u
    %16:mat3x4<f32> = access %params, 2u
    %17:mat3x2<f32> = access %params, 7u
    %18:vec2<u32> = access %params, 12u
    %19:vec2<f32> = access %params, 13u
    %20:vec2<u32> = min %coords_1, %18
    %21:vec2<f32> = convert %20
    %22:vec3<f32> = construct %21, 1.0f
    %23:vec2<f32> = mul %17, %22
    %24:vec2<f32> = round %23
    %25:vec2<u32> = convert %24
    %26:u32 = access %params, 0u
    %27:bool = eq %26, 1u
    %28:vec3<f32>, %29:f32 = if %27 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %30:vec4<f32> = textureLoad %plane_0, %25, 0u
        %31:vec3<f32> = swizzle %30, xyz
        %32:f32 = access %30, 3u
        exit_if %31, %32  # if_1
      }
      $B5: {  # false
        %33:vec4<f32> = textureLoad %plane_0, %25, 0u
        %34:f32 = access %33, 0u
        %35:vec2<f32> = mul %24, %19
        %36:vec2<u32> = convert %35
        %37:vec4<f32> = textureLoad %plane_1, %36, 0u
        %38:vec2<f32> = swizzle %37, xy
        %39:vec4<f32> = construct %34, %38, 1.0f
        %40:vec3<f32> = mul %39, %16
        exit_if %40, 1.0f  # if_1
      }
    }
    %41:bool = eq %15, 0u
    %42:vec3<f32> = if %41 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %43:tint_GammaTransferParams = access %params, 3u
        %44:tint_GammaTransferParams = access %params, 4u
        %45:mat3x3<f32> = access %params, 5u
        %46:vec3<f32> = call %tint_GammaCorrection, %28, %43
        %48:vec3<f32> = mul %45, %46
        %49:vec3<f32> = call %tint_GammaCorrection, %48, %44
        exit_if %49  # if_2
      }
      $B7: {  # false
        exit_if %28  # if_2
      }
    }
    %50:vec4<f32> = construct %42, %29
    ret %50
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %53:f32 = access %params_1, 0u
    %54:f32 = access %params_1, 1u
    %55:f32 = access %params_1, 2u
    %56:f32 = access %params_1, 3u
    %57:f32 = access %params_1, 4u
    %58:f32 = access %params_1, 5u
    %59:f32 = access %params_1, 6u
    %60:vec3<f32> = construct %53
    %61:vec3<f32> = construct %57
    %62:vec3<f32> = abs %v
    %63:vec3<f32> = sign %v
    %64:vec3<bool> = lt %62, %61
    %65:vec3<f32> = mul %56, %62
    %66:vec3<f32> = add %65, %59
    %67:vec3<f32> = mul %63, %66
    %68:vec3<f32> = mul %54, %62
    %69:vec3<f32> = add %68, %55
    %70:vec3<f32> = pow %69, %60
    %71:vec3<f32> = add %70, %58
    %72:vec3<f32> = mul %63, %71
    %73:vec3<f32> = select %72, %67, %64
    ret %73
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureLoad_SignedCoords) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coords);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<i32>):vec4<f32> {
  $B2: {
    %4:texture_external = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func(%coords:vec2<i32>):vec4<f32> {
  $B2: {
    %6:texture_2d<f32> = load %texture_plane0
    %7:texture_2d<f32> = load %texture_plane1
    %8:tint_ExternalTextureParams = load %texture_params
    %9:vec2<u32> = convert %coords
    %result:vec4<f32> = call %tint_TextureLoadExternal, %6, %7, %8, %9
    ret %result
  }
}
%tint_TextureLoadExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %coords_1: 'coords'
  $B3: {
    %16:u32 = access %params, 1u
    %17:mat3x4<f32> = access %params, 2u
    %18:mat3x2<f32> = access %params, 7u
    %19:vec2<u32> = access %params, 12u
    %20:vec2<f32> = access %params, 13u
    %21:vec2<u32> = min %coords_1, %19
    %22:vec2<f32> = convert %21
    %23:vec3<f32> = construct %22, 1.0f
    %24:vec2<f32> = mul %18, %23
    %25:vec2<f32> = round %24
    %26:vec2<u32> = convert %25
    %27:u32 = access %params, 0u
    %28:bool = eq %27, 1u
    %29:vec3<f32>, %30:f32 = if %28 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %31:vec4<f32> = textureLoad %plane_0, %26, 0u
        %32:vec3<f32> = swizzle %31, xyz
        %33:f32 = access %31, 3u
        exit_if %32, %33  # if_1
      }
      $B5: {  # false
        %34:vec4<f32> = textureLoad %plane_0, %26, 0u
        %35:f32 = access %34, 0u
        %36:vec2<f32> = mul %25, %20
        %37:vec2<u32> = convert %36
        %38:vec4<f32> = textureLoad %plane_1, %37, 0u
        %39:vec2<f32> = swizzle %38, xy
        %40:vec4<f32> = construct %35, %39, 1.0f
        %41:vec3<f32> = mul %40, %17
        exit_if %41, 1.0f  # if_1
      }
    }
    %42:bool = eq %16, 0u
    %43:vec3<f32> = if %42 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %44:tint_GammaTransferParams = access %params, 3u
        %45:tint_GammaTransferParams = access %params, 4u
        %46:mat3x3<f32> = access %params, 5u
        %47:vec3<f32> = call %tint_GammaCorrection, %29, %44
        %49:vec3<f32> = mul %46, %47
        %50:vec3<f32> = call %tint_GammaCorrection, %49, %45
        exit_if %50  # if_2
      }
      $B7: {  # false
        exit_if %29  # if_2
      }
    }
    %51:vec4<f32> = construct %43, %30
    ret %51
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %54:f32 = access %params_1, 0u
    %55:f32 = access %params_1, 1u
    %56:f32 = access %params_1, 2u
    %57:f32 = access %params_1, 3u
    %58:f32 = access %params_1, 4u
    %59:f32 = access %params_1, 5u
    %60:f32 = access %params_1, 6u
    %61:vec3<f32> = construct %54
    %62:vec3<f32> = construct %58
    %63:vec3<f32> = abs %v
    %64:vec3<f32> = sign %v
    %65:vec3<bool> = lt %63, %62
    %66:vec3<f32> = mul %57, %63
    %67:vec3<f32> = add %66, %60
    %68:vec3<f32> = mul %64, %67
    %69:vec3<f32> = mul %55, %63
    %70:vec3<f32> = add %69, %56
    %71:vec3<f32> = pow %70, %61
    %72:vec3<f32> = add %71, %59
    %73:vec3<f32> = mul %64, %72
    %74:vec3<f32> = select %73, %68, %65
    ret %74
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureSampleBaseClampToEdge) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* sampler = b.FunctionParam("sampler", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    func->SetParams({sampler, coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load,
                              sampler, coords);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> {
  $B2: {
    %5:texture_external = load %texture
    %result:vec4<f32> = textureSampleBaseClampToEdge %5, %sampler, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> {
  $B2: {
    %7:texture_2d<f32> = load %texture_plane0
    %8:texture_2d<f32> = load %texture_plane1
    %9:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %tint_TextureSampleExternal, %7, %8, %9, %sampler, %coords
    ret %result
  }
}
%tint_TextureSampleExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %tint_sampler:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %coords_1: 'coords'
  $B3: {
    %17:u32 = access %params, 1u
    %18:mat3x4<f32> = access %params, 2u
    %19:mat3x2<f32> = access %params, 6u
    %20:vec2<f32> = access %params, 8u
    %21:vec2<f32> = access %params, 9u
    %22:vec2<f32> = access %params, 10u
    %23:vec2<f32> = access %params, 11u
    %24:vec3<f32> = construct %coords_1, 1.0f
    %25:vec2<f32> = mul %19, %24
    %26:vec2<f32> = clamp %25, %20, %21
    %27:u32 = access %params, 0u
    %28:bool = eq %27, 1u
    %29:vec3<f32>, %30:f32 = if %28 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %31:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %26, 0.0f
        %32:vec3<f32> = swizzle %31, xyz
        %33:f32 = access %31, 3u
        exit_if %32, %33  # if_1
      }
      $B5: {  # false
        %34:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %26, 0.0f
        %35:f32 = access %34, 0u
        %36:vec2<f32> = clamp %25, %22, %23
        %37:vec4<f32> = textureSampleLevel %plane_1, %tint_sampler, %36, 0.0f
        %38:vec2<f32> = swizzle %37, xy
        %39:vec4<f32> = construct %35, %38, 1.0f
        %40:vec3<f32> = mul %39, %18
        exit_if %40, 1.0f  # if_1
      }
    }
    %41:bool = eq %17, 0u
    %42:vec3<f32> = if %41 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %43:tint_GammaTransferParams = access %params, 3u
        %44:tint_GammaTransferParams = access %params, 4u
        %45:mat3x3<f32> = access %params, 5u
        %46:vec3<f32> = call %tint_GammaCorrection, %29, %43
        %48:vec3<f32> = mul %45, %46
        %49:vec3<f32> = call %tint_GammaCorrection, %48, %44
        exit_if %49  # if_2
      }
      $B7: {  # false
        exit_if %29  # if_2
      }
    }
    %50:vec4<f32> = construct %42, %30
    ret %50
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %53:f32 = access %params_1, 0u
    %54:f32 = access %params_1, 1u
    %55:f32 = access %params_1, 2u
    %56:f32 = access %params_1, 3u
    %57:f32 = access %params_1, 4u
    %58:f32 = access %params_1, 5u
    %59:f32 = access %params_1, 6u
    %60:vec3<f32> = construct %53
    %61:vec3<f32> = construct %57
    %62:vec3<f32> = abs %v
    %63:vec3<f32> = sign %v
    %64:vec3<bool> = lt %62, %61
    %65:vec3<f32> = mul %56, %62
    %66:vec3<f32> = add %65, %59
    %67:vec3<f32> = mul %63, %66
    %68:vec3<f32> = mul %54, %62
    %69:vec3<f32> = add %68, %55
    %70:vec3<f32> = pow %69, %60
    %71:vec3<f32> = add %70, %58
    %72:vec3<f32> = mul %63, %71
    %73:vec3<f32> = select %72, %67, %64
    ret %73
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, ViaUserFunctionParameter) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4<f32>());
    {
        auto* texture = b.FunctionParam("texture", ty.external_texture());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4<f32>());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        bar->SetParams({sampler, coords});
        b.Append(bar->Block(), [&] {
            auto* load = b.Load(var->Result());
            auto* result = b.Call(ty.vec4<f32>(), foo, load, sampler, coords);
            b.Return(bar, result);
            mod.SetName(result, "result");
        });
    }

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func(%texture_1:texture_external, %sampler:sampler, %coords:vec2<f32>):vec4<f32> {  # %texture_1: 'texture'
  $B2: {
    %result:vec4<f32> = textureSampleBaseClampToEdge %texture_1, %sampler, %coords
    ret %result
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %sampler_1: 'sampler', %coords_1: 'coords'
  $B3: {
    %10:texture_external = load %texture
    %result_1:vec4<f32> = call %foo, %10, %sampler_1, %coords_1  # %result_1: 'result'
    ret %result_1
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func(%texture_plane0_1:texture_2d<f32>, %texture_plane1_1:texture_2d<f32>, %texture_params_1:tint_ExternalTextureParams, %sampler:sampler, %coords:vec2<f32>):vec4<f32> {  # %texture_plane0_1: 'texture_plane0', %texture_plane1_1: 'texture_plane1', %texture_params_1: 'texture_params'
  $B2: {
    %result:vec4<f32> = call %tint_TextureSampleExternal, %texture_plane0_1, %texture_plane1_1, %texture_params_1, %sampler, %coords
    ret %result
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %sampler_1: 'sampler', %coords_1: 'coords'
  $B3: {
    %15:texture_2d<f32> = load %texture_plane0
    %16:texture_2d<f32> = load %texture_plane1
    %17:tint_ExternalTextureParams = load %texture_params
    %result_1:vec4<f32> = call %foo, %15, %16, %17, %sampler_1, %coords_1  # %result_1: 'result'
    ret %result_1
  }
}
%tint_TextureSampleExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %tint_sampler:sampler, %coords_2:vec2<f32>):vec4<f32> {  # %coords_2: 'coords'
  $B4: {
    %24:u32 = access %params, 1u
    %25:mat3x4<f32> = access %params, 2u
    %26:mat3x2<f32> = access %params, 6u
    %27:vec2<f32> = access %params, 8u
    %28:vec2<f32> = access %params, 9u
    %29:vec2<f32> = access %params, 10u
    %30:vec2<f32> = access %params, 11u
    %31:vec3<f32> = construct %coords_2, 1.0f
    %32:vec2<f32> = mul %26, %31
    %33:vec2<f32> = clamp %32, %27, %28
    %34:u32 = access %params, 0u
    %35:bool = eq %34, 1u
    %36:vec3<f32>, %37:f32 = if %35 [t: $B5, f: $B6] {  # if_1
      $B5: {  # true
        %38:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %33, 0.0f
        %39:vec3<f32> = swizzle %38, xyz
        %40:f32 = access %38, 3u
        exit_if %39, %40  # if_1
      }
      $B6: {  # false
        %41:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %33, 0.0f
        %42:f32 = access %41, 0u
        %43:vec2<f32> = clamp %32, %29, %30
        %44:vec4<f32> = textureSampleLevel %plane_1, %tint_sampler, %43, 0.0f
        %45:vec2<f32> = swizzle %44, xy
        %46:vec4<f32> = construct %42, %45, 1.0f
        %47:vec3<f32> = mul %46, %25
        exit_if %47, 1.0f  # if_1
      }
    }
    %48:bool = eq %24, 0u
    %49:vec3<f32> = if %48 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %50:tint_GammaTransferParams = access %params, 3u
        %51:tint_GammaTransferParams = access %params, 4u
        %52:mat3x3<f32> = access %params, 5u
        %53:vec3<f32> = call %tint_GammaCorrection, %36, %50
        %55:vec3<f32> = mul %52, %53
        %56:vec3<f32> = call %tint_GammaCorrection, %55, %51
        exit_if %56  # if_2
      }
      $B8: {  # false
        exit_if %36  # if_2
      }
    }
    %57:vec4<f32> = construct %49, %37
    ret %57
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> {  # %params_1: 'params'
  $B9: {
    %60:f32 = access %params_1, 0u
    %61:f32 = access %params_1, 1u
    %62:f32 = access %params_1, 2u
    %63:f32 = access %params_1, 3u
    %64:f32 = access %params_1, 4u
    %65:f32 = access %params_1, 5u
    %66:f32 = access %params_1, 6u
    %67:vec3<f32> = construct %60
    %68:vec3<f32> = construct %64
    %69:vec3<f32> = abs %v
    %70:vec3<f32> = sign %v
    %71:vec3<bool> = lt %69, %68
    %72:vec3<f32> = mul %63, %69
    %73:vec3<f32> = add %72, %66
    %74:vec3<f32> = mul %70, %73
    %75:vec3<f32> = mul %61, %69
    %76:vec3<f32> = add %75, %62
    %77:vec3<f32> = pow %76, %67
    %78:vec3<f32> = add %77, %65
    %79:vec3<f32> = mul %70, %78
    %80:vec3<f32> = select %79, %74, %71
    ret %80
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultipleUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4<f32>());
    {
        auto* texture = b.FunctionParam("texture", ty.external_texture());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4<f32>());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords_f = b.FunctionParam("coords", ty.vec2<f32>());
        bar->SetParams({sampler, coords_f});
        b.Append(bar->Block(), [&] {
            auto* load_a = b.Load(var->Result());
            b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, load_a);
            auto* load_b = b.Load(var->Result());
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load_b, sampler,
                   coords_f);
            auto* load_c = b.Load(var->Result());
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load_c, sampler,
                   coords_f);
            auto* load_d = b.Load(var->Result());
            auto* result_a = b.Call(ty.vec4<f32>(), foo, load_d, sampler, coords_f);
            auto* result_b = b.Call(ty.vec4<f32>(), foo, load_d, sampler, coords_f);
            b.Return(bar, b.Add(ty.vec4<f32>(), result_a, result_b));
            mod.SetName(result_a, "result_a");
            mod.SetName(result_b, "result_b");
        });
    }

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
}

%foo = func(%texture_1:texture_external, %sampler:sampler, %coords:vec2<f32>):vec4<f32> {  # %texture_1: 'texture'
  $B2: {
    %result:vec4<f32> = textureSampleBaseClampToEdge %texture_1, %sampler, %coords
    ret %result
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %sampler_1: 'sampler', %coords_1: 'coords'
  $B3: {
    %10:texture_external = load %texture
    %11:vec2<u32> = textureDimensions %10
    %12:texture_external = load %texture
    %13:vec4<f32> = textureSampleBaseClampToEdge %12, %sampler_1, %coords_1
    %14:texture_external = load %texture
    %15:vec4<f32> = textureSampleBaseClampToEdge %14, %sampler_1, %coords_1
    %16:texture_external = load %texture
    %result_a:vec4<f32> = call %foo, %16, %sampler_1, %coords_1
    %result_b:vec4<f32> = call %foo, %16, %sampler_1, %coords_1
    %19:vec4<f32> = add %result_a, %result_b
    ret %19
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
}

%foo = func(%texture_plane0_1:texture_2d<f32>, %texture_plane1_1:texture_2d<f32>, %texture_params_1:tint_ExternalTextureParams, %sampler:sampler, %coords:vec2<f32>):vec4<f32> {  # %texture_plane0_1: 'texture_plane0', %texture_plane1_1: 'texture_plane1', %texture_params_1: 'texture_params'
  $B2: {
    %result:vec4<f32> = call %tint_TextureSampleExternal, %texture_plane0_1, %texture_plane1_1, %texture_params_1, %sampler, %coords
    ret %result
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %sampler_1: 'sampler', %coords_1: 'coords'
  $B3: {
    %15:texture_2d<f32> = load %texture_plane0
    %16:texture_2d<f32> = load %texture_plane1
    %17:tint_ExternalTextureParams = load %texture_params
    %18:vec2<u32> = access %17, 12u
    %19:vec2<u32> = add %18, vec2<u32>(1u)
    %20:texture_2d<f32> = load %texture_plane0
    %21:texture_2d<f32> = load %texture_plane1
    %22:tint_ExternalTextureParams = load %texture_params
    %23:vec4<f32> = call %tint_TextureSampleExternal, %20, %21, %22, %sampler_1, %coords_1
    %24:texture_2d<f32> = load %texture_plane0
    %25:texture_2d<f32> = load %texture_plane1
    %26:tint_ExternalTextureParams = load %texture_params
    %27:vec4<f32> = call %tint_TextureSampleExternal, %24, %25, %26, %sampler_1, %coords_1
    %28:texture_2d<f32> = load %texture_plane0
    %29:texture_2d<f32> = load %texture_plane1
    %30:tint_ExternalTextureParams = load %texture_params
    %result_a:vec4<f32> = call %foo, %28, %29, %30, %sampler_1, %coords_1
    %result_b:vec4<f32> = call %foo, %28, %29, %30, %sampler_1, %coords_1
    %33:vec4<f32> = add %result_a, %result_b
    ret %33
  }
}
%tint_TextureSampleExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %tint_sampler:sampler, %coords_2:vec2<f32>):vec4<f32> {  # %coords_2: 'coords'
  $B4: {
    %39:u32 = access %params, 1u
    %40:mat3x4<f32> = access %params, 2u
    %41:mat3x2<f32> = access %params, 6u
    %42:vec2<f32> = access %params, 8u
    %43:vec2<f32> = access %params, 9u
    %44:vec2<f32> = access %params, 10u
    %45:vec2<f32> = access %params, 11u
    %46:vec3<f32> = construct %coords_2, 1.0f
    %47:vec2<f32> = mul %41, %46
    %48:vec2<f32> = clamp %47, %42, %43
    %49:u32 = access %params, 0u
    %50:bool = eq %49, 1u
    %51:vec3<f32>, %52:f32 = if %50 [t: $B5, f: $B6] {  # if_1
      $B5: {  # true
        %53:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %48, 0.0f
        %54:vec3<f32> = swizzle %53, xyz
        %55:f32 = access %53, 3u
        exit_if %54, %55  # if_1
      }
      $B6: {  # false
        %56:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %48, 0.0f
        %57:f32 = access %56, 0u
        %58:vec2<f32> = clamp %47, %44, %45
        %59:vec4<f32> = textureSampleLevel %plane_1, %tint_sampler, %58, 0.0f
        %60:vec2<f32> = swizzle %59, xy
        %61:vec4<f32> = construct %57, %60, 1.0f
        %62:vec3<f32> = mul %61, %40
        exit_if %62, 1.0f  # if_1
      }
    }
    %63:bool = eq %39, 0u
    %64:vec3<f32> = if %63 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %65:tint_GammaTransferParams = access %params, 3u
        %66:tint_GammaTransferParams = access %params, 4u
        %67:mat3x3<f32> = access %params, 5u
        %68:vec3<f32> = call %tint_GammaCorrection, %51, %65
        %70:vec3<f32> = mul %67, %68
        %71:vec3<f32> = call %tint_GammaCorrection, %70, %66
        exit_if %71  # if_2
      }
      $B8: {  # false
        exit_if %51  # if_2
      }
    }
    %72:vec4<f32> = construct %64, %52
    ret %72
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> {  # %params_1: 'params'
  $B9: {
    %75:f32 = access %params_1, 0u
    %76:f32 = access %params_1, 1u
    %77:f32 = access %params_1, 2u
    %78:f32 = access %params_1, 3u
    %79:f32 = access %params_1, 4u
    %80:f32 = access %params_1, 5u
    %81:f32 = access %params_1, 6u
    %82:vec3<f32> = construct %75
    %83:vec3<f32> = construct %79
    %84:vec3<f32> = abs %v
    %85:vec3<f32> = sign %v
    %86:vec3<bool> = lt %84, %83
    %87:vec3<f32> = mul %78, %84
    %88:vec3<f32> = add %87, %81
    %89:vec3<f32> = mul %85, %88
    %90:vec3<f32> = mul %76, %84
    %91:vec3<f32> = add %90, %77
    %92:vec3<f32> = pow %91, %82
    %93:vec3<f32> = add %92, %80
    %94:vec3<f32> = mul %85, %93
    %95:vec3<f32> = select %94, %89, %86
    ret %95
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultipleTextures) {
    auto* var_a = b.Var("texture_a", ty.ptr(handle, ty.external_texture()));
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var("texture_b", ty.ptr(handle, ty.external_texture()));
    var_b->SetBindingPoint(2, 2);
    mod.root_block->Append(var_b);

    auto* var_c = b.Var("texture_c", ty.ptr(handle, ty.external_texture()));
    var_c->SetBindingPoint(3, 2);
    mod.root_block->Append(var_c);

    auto* foo = b.Function("foo", ty.void_());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    foo->SetParams({coords});
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a->Result());
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load_a, coords);
        auto* load_b = b.Load(var_b->Result());
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load_b, coords);
        auto* load_c = b.Load(var_c->Result());
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load_c, coords);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %texture_a:ptr<handle, texture_external, read> = var undef @binding_point(1, 2)
  %texture_b:ptr<handle, texture_external, read> = var undef @binding_point(2, 2)
  %texture_c:ptr<handle, texture_external, read> = var undef @binding_point(3, 2)
}

%foo = func(%coords:vec2<u32>):void {
  $B2: {
    %6:texture_external = load %texture_a
    %7:vec4<f32> = textureLoad %6, %coords
    %8:texture_external = load %texture_b
    %9:vec4<f32> = textureLoad %8, %coords
    %10:texture_external = load %texture_c
    %11:vec4<f32> = textureLoad %10, %coords
    ret
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  sampleTransform:mat3x2<f32> @offset(176)
  loadTransform:mat3x2<f32> @offset(200)
  samplePlane0RectMin:vec2<f32> @offset(224)
  samplePlane0RectMax:vec2<f32> @offset(232)
  samplePlane1RectMin:vec2<f32> @offset(240)
  samplePlane1RectMax:vec2<f32> @offset(248)
  apparentSize:vec2<u32> @offset(256)
  plane1CoordFactor:vec2<f32> @offset(264)
}

$B1: {  # root
  %texture_a_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_a_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_a_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_b_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 2)
  %texture_b_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 3)
  %texture_b_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(2, 4)
  %texture_c_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
  %texture_c_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 3)
  %texture_c_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(3, 4)
}

%foo = func(%coords:vec2<u32>):void {
  $B2: {
    %12:texture_2d<f32> = load %texture_a_plane0
    %13:texture_2d<f32> = load %texture_a_plane1
    %14:tint_ExternalTextureParams = load %texture_a_params
    %15:vec4<f32> = call %tint_TextureLoadExternal, %12, %13, %14, %coords
    %17:texture_2d<f32> = load %texture_b_plane0
    %18:texture_2d<f32> = load %texture_b_plane1
    %19:tint_ExternalTextureParams = load %texture_b_params
    %20:vec4<f32> = call %tint_TextureLoadExternal, %17, %18, %19, %coords
    %21:texture_2d<f32> = load %texture_c_plane0
    %22:texture_2d<f32> = load %texture_c_plane1
    %23:tint_ExternalTextureParams = load %texture_c_params
    %24:vec4<f32> = call %tint_TextureLoadExternal, %21, %22, %23, %coords
    ret
  }
}
%tint_TextureLoadExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %coords_1: 'coords'
  $B3: {
    %29:u32 = access %params, 1u
    %30:mat3x4<f32> = access %params, 2u
    %31:mat3x2<f32> = access %params, 7u
    %32:vec2<u32> = access %params, 12u
    %33:vec2<f32> = access %params, 13u
    %34:vec2<u32> = min %coords_1, %32
    %35:vec2<f32> = convert %34
    %36:vec3<f32> = construct %35, 1.0f
    %37:vec2<f32> = mul %31, %36
    %38:vec2<f32> = round %37
    %39:vec2<u32> = convert %38
    %40:u32 = access %params, 0u
    %41:bool = eq %40, 1u
    %42:vec3<f32>, %43:f32 = if %41 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %44:vec4<f32> = textureLoad %plane_0, %39, 0u
        %45:vec3<f32> = swizzle %44, xyz
        %46:f32 = access %44, 3u
        exit_if %45, %46  # if_1
      }
      $B5: {  # false
        %47:vec4<f32> = textureLoad %plane_0, %39, 0u
        %48:f32 = access %47, 0u
        %49:vec2<f32> = mul %38, %33
        %50:vec2<u32> = convert %49
        %51:vec4<f32> = textureLoad %plane_1, %50, 0u
        %52:vec2<f32> = swizzle %51, xy
        %53:vec4<f32> = construct %48, %52, 1.0f
        %54:vec3<f32> = mul %53, %30
        exit_if %54, 1.0f  # if_1
      }
    }
    %55:bool = eq %29, 0u
    %56:vec3<f32> = if %55 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %57:tint_GammaTransferParams = access %params, 3u
        %58:tint_GammaTransferParams = access %params, 4u
        %59:mat3x3<f32> = access %params, 5u
        %60:vec3<f32> = call %tint_GammaCorrection, %42, %57
        %62:vec3<f32> = mul %59, %60
        %63:vec3<f32> = call %tint_GammaCorrection, %62, %58
        exit_if %63  # if_2
      }
      $B7: {  # false
        exit_if %42  # if_2
      }
    }
    %64:vec4<f32> = construct %56, %43
    ret %64
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %67:f32 = access %params_1, 0u
    %68:f32 = access %params_1, 1u
    %69:f32 = access %params_1, 2u
    %70:f32 = access %params_1, 3u
    %71:f32 = access %params_1, 4u
    %72:f32 = access %params_1, 5u
    %73:f32 = access %params_1, 6u
    %74:vec3<f32> = construct %67
    %75:vec3<f32> = construct %71
    %76:vec3<f32> = abs %v
    %77:vec3<f32> = sign %v
    %78:vec3<bool> = lt %76, %75
    %79:vec3<f32> = mul %70, %76
    %80:vec3<f32> = add %79, %73
    %81:vec3<f32> = mul %77, %80
    %82:vec3<f32> = mul %68, %76
    %83:vec3<f32> = add %82, %69
    %84:vec3<f32> = pow %83, %74
    %85:vec3<f32> = add %84, %72
    %86:vec3<f32> = mul %77, %85
    %87:vec3<f32> = select %86, %81, %78
    ret %87
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    map[{2u, 2u}] = {{2u, 3u}, {2u, 4u}};
    map[{3u, 2u}] = {{3u, 3u}, {3u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
