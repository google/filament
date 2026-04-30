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

#include <string>

#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/transform/multiplanar_options.h"

namespace tint::core::ir::transform {
namespace {

constexpr std::string_view kExternalTextureParams = R"(
tint_TransferFunctionParams = struct @align(4) {
  mode:u32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  G:f32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  srcTransferFunction:tint_TransferFunctionParams @offset(64)
  dstTransferFunction:tint_TransferFunctionParams @offset(96)
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
)";

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

TEST_F(IR_MultiplanarExternalTextureTest, MultiplanarDeclWithNoUses) {
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
}

%foo = func():void {
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap options{};
    options[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, YcbcrDeclWithNoUses) {
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func():void {
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap options{};
    options[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultiplanarLoadWithNoUses) {
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
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
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, YCBCRLoadWithNoUses) {
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func():void {
  $B2: {
    %5:texture_2d<f32> = load %texture
    %6:sampler = load %texture_ycbcr_sampler
    %7:tint_ExternalTextureParams = load %texture_params
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultiplanarTextureDimensions) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2u());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec2u(), core::BuiltinFn::kTextureDimensions, load);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
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
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, YcbcrTextureDimensions) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2u());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec2u(), core::BuiltinFn::kTextureDimensions, load);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func():vec2<u32> {
  $B2: {
    %5:texture_2d<f32> = load %texture
    %6:sampler = load %texture_ycbcr_sampler
    %7:tint_ExternalTextureParams = load %texture_params
    %8:vec2<u32> = access %7, 12u
    %result:vec2<u32> = add %8, vec2<u32>(1u)
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultiplanarTextureLoad) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4f());
    auto* coords = b.FunctionParam("coords", ty.vec2u());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
}

%foo = func(%coords:vec2<u32>):vec4<f32> {
  $B2: {
    %6:texture_2d<f32> = load %texture_plane0
    %7:texture_2d<f32> = load %texture_plane1
    %8:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %tint_TextureLoadMultiplanarExternal, %6, %7, %8, %coords
    ret %result
  }
}
%tint_TextureLoadMultiplanarExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %coords_1: 'coords'
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
        %43:tint_TransferFunctionParams = access %params, 3u
        %44:tint_TransferFunctionParams = access %params, 4u
        %45:mat3x3<f32> = access %params, 5u
        %46:vec3<f32> = call %tint_ApplySrcTransferFunction, %28, %43
        %48:vec3<f32> = mul %45, %46
        %49:vec3<f32> = call %tint_ApplyGammaTransferFunction, %48, %44
        exit_if %49  # if_2
      }
      $B7: {  # false
        exit_if %28  # if_2
      }
    }
    %51:vec4<f32> = construct %42, %29
    ret %51
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %55:bool = eq %mode, 0u
    if %55 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %56:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %56
      }
      $B10: {  # false
        %57:bool = eq %mode, 1u
        if %57 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %58:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %58
          }
          $B12: {  # false
            %60:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %60
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B13: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %71:vec3<f32> = construct %G
    %72:vec3<f32> = construct %D
    %73:vec3<f32> = abs %v_1
    %74:vec3<f32> = sign %v_1
    %75:vec3<bool> = lt %73, %72
    %76:vec3<f32> = mul %C, %73
    %77:vec3<f32> = add %76, %F
    %78:vec3<f32> = mul %74, %77
    %79:vec3<f32> = mul %A, %73
    %80:vec3<f32> = add %79, %B
    %81:vec3<f32> = pow %80, %71
    %82:vec3<f32> = add %81, %E
    %83:vec3<f32> = mul %74, %82
    %84:vec3<f32> = select %83, %78, %75
    ret %84
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B14: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %94:bool = lte %v_2, %cutoff
    if %94 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %95:f32 = mul %v_2, %v_2
        %96:f32 = div %95, %lower_scale
        ret %96
      }
      $B16: {  # false
        %97:f32 = sub %v_2, %C_1
        %98:f32 = div %97, %A_1
        %99:f32 = exp %98
        %100:f32 = add %B_1, %99
        %101:f32 = div %100, %upper_scale
        ret %101
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %104:f32 = swizzle %v_3, x
    %105:f32 = call %tint_ApplyHLGSingleChannel, %104, %params_4
    %106:f32 = swizzle %v_3, y
    %107:f32 = call %tint_ApplyHLGSingleChannel, %106, %params_4
    %108:f32 = swizzle %v_3, z
    %109:f32 = call %tint_ApplyHLGSingleChannel, %108, %params_4
    %110:vec3<f32> = construct %105, %107, %109
    ret %110
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %118:vec3<f32> = construct %c1
    %119:vec3<f32> = construct %c2
    %120:vec3<f32> = construct %c3
    %121:vec3<f32> = construct %m1
    %122:vec3<f32> = construct %m2
    %123:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %124:vec3<f32> = div vec3<f32>(1.0f), %122
    %125:vec3<f32> = pow %123, %124
    %126:vec3<f32> = sub %125, %118
    %127:vec3<f32> = max %126, vec3<f32>(0.0f)
    %128:vec3<f32> = mul %120, %125
    %129:vec3<f32> = sub %119, %128
    %130:vec3<f32> = div %127, %129
    %131:vec3<f32> = div vec3<f32>(1.0f), %121
    %132:vec3<f32> = pow %130, %131
    ret %132
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, YcbcrTextureLoad) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4f());
    auto* coords = b.FunctionParam("coords", ty.vec2u());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func(%coords:vec2<u32>):vec4<f32> {
  $B2: {
    %6:texture_2d<f32> = load %texture
    %7:sampler = load %texture_ycbcr_sampler
    %8:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %tint_TextureLoadYcbcrExternal, %6, %8, %coords
    ret %result
  }
}
%tint_TextureLoadYcbcrExternal = func(%texture_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %texture_1: 'texture', %coords_1: 'coords'
  $B3: {
    %14:u32 = access %params, 1u
    %15:mat3x4<f32> = access %params, 2u
    %16:mat3x2<f32> = access %params, 7u
    %17:vec2<u32> = access %params, 12u
    %18:vec2<u32> = min %coords_1, %17
    %19:vec2<f32> = convert %18
    %20:vec3<f32> = construct %19, 1.0f
    %21:vec2<f32> = mul %16, %20
    %22:vec2<f32> = round %21
    %23:vec2<u32> = convert %22
    %24:vec4<f32> = textureLoad %texture_1, %23, 0u
    %25:vec3<f32> = swizzle %24, xyz
    %26:vec4<f32> = construct %25, 1.0f
    %27:vec3<f32> = mul %26, %15
    %28:bool = eq %14, 0u
    %29:vec3<f32> = if %28 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %30:tint_TransferFunctionParams = access %params, 3u
        %31:tint_TransferFunctionParams = access %params, 4u
        %32:mat3x3<f32> = access %params, 5u
        %33:vec3<f32> = call %tint_ApplySrcTransferFunction, %27, %30
        %35:vec3<f32> = mul %32, %33
        %36:vec3<f32> = call %tint_ApplyGammaTransferFunction, %35, %31
        exit_if %36  # if_1
      }
      $B5: {  # false
        exit_if %27  # if_1
      }
    }
    %38:vec4<f32> = construct %29, 1.0f
    ret %38
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B6: {
    %mode:u32 = access %params_1, 0u
    %42:bool = eq %mode, 0u
    if %42 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %43:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %43
      }
      $B8: {  # false
        %44:bool = eq %mode, 1u
        if %44 [t: $B9, f: $B10] {  # if_3
          $B9: {  # true
            %45:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %45
          }
          $B10: {  # false
            %47:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %47
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B11: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %58:vec3<f32> = construct %G
    %59:vec3<f32> = construct %D
    %60:vec3<f32> = abs %v_1
    %61:vec3<f32> = sign %v_1
    %62:vec3<bool> = lt %60, %59
    %63:vec3<f32> = mul %C, %60
    %64:vec3<f32> = add %63, %F
    %65:vec3<f32> = mul %61, %64
    %66:vec3<f32> = mul %A, %60
    %67:vec3<f32> = add %66, %B
    %68:vec3<f32> = pow %67, %58
    %69:vec3<f32> = add %68, %E
    %70:vec3<f32> = mul %61, %69
    %71:vec3<f32> = select %70, %65, %62
    ret %71
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B12: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %81:bool = lte %v_2, %cutoff
    if %81 [t: $B13, f: $B14] {  # if_4
      $B13: {  # true
        %82:f32 = mul %v_2, %v_2
        %83:f32 = div %82, %lower_scale
        ret %83
      }
      $B14: {  # false
        %84:f32 = sub %v_2, %C_1
        %85:f32 = div %84, %A_1
        %86:f32 = exp %85
        %87:f32 = add %B_1, %86
        %88:f32 = div %87, %upper_scale
        ret %88
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B15: {
    %91:f32 = swizzle %v_3, x
    %92:f32 = call %tint_ApplyHLGSingleChannel, %91, %params_4
    %93:f32 = swizzle %v_3, y
    %94:f32 = call %tint_ApplyHLGSingleChannel, %93, %params_4
    %95:f32 = swizzle %v_3, z
    %96:f32 = call %tint_ApplyHLGSingleChannel, %95, %params_4
    %97:vec3<f32> = construct %92, %94, %96
    ret %97
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B16: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %105:vec3<f32> = construct %c1
    %106:vec3<f32> = construct %c2
    %107:vec3<f32> = construct %c3
    %108:vec3<f32> = construct %m1
    %109:vec3<f32> = construct %m2
    %110:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %111:vec3<f32> = div vec3<f32>(1.0f), %109
    %112:vec3<f32> = pow %110, %111
    %113:vec3<f32> = sub %112, %105
    %114:vec3<f32> = max %113, vec3<f32>(0.0f)
    %115:vec3<f32> = mul %107, %112
    %116:vec3<f32> = sub %106, %115
    %117:vec3<f32> = div %114, %116
    %118:vec3<f32> = div vec3<f32>(1.0f), %108
    %119:vec3<f32> = pow %117, %118
    ret %119
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultiplanarTextureLoad_SignedCoords) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4f());
    auto* coords = b.FunctionParam("coords", ty.vec2i());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
}

%foo = func(%coords:vec2<i32>):vec4<f32> {
  $B2: {
    %6:texture_2d<f32> = load %texture_plane0
    %7:texture_2d<f32> = load %texture_plane1
    %8:tint_ExternalTextureParams = load %texture_params
    %9:vec2<u32> = convert %coords
    %result:vec4<f32> = call %tint_TextureLoadMultiplanarExternal, %6, %7, %8, %9
    ret %result
  }
}
%tint_TextureLoadMultiplanarExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %coords_1: 'coords'
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
        %44:tint_TransferFunctionParams = access %params, 3u
        %45:tint_TransferFunctionParams = access %params, 4u
        %46:mat3x3<f32> = access %params, 5u
        %47:vec3<f32> = call %tint_ApplySrcTransferFunction, %29, %44
        %49:vec3<f32> = mul %46, %47
        %50:vec3<f32> = call %tint_ApplyGammaTransferFunction, %49, %45
        exit_if %50  # if_2
      }
      $B7: {  # false
        exit_if %29  # if_2
      }
    }
    %52:vec4<f32> = construct %43, %30
    ret %52
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %56:bool = eq %mode, 0u
    if %56 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %57:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %57
      }
      $B10: {  # false
        %58:bool = eq %mode, 1u
        if %58 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %59:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %59
          }
          $B12: {  # false
            %61:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %61
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B13: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %72:vec3<f32> = construct %G
    %73:vec3<f32> = construct %D
    %74:vec3<f32> = abs %v_1
    %75:vec3<f32> = sign %v_1
    %76:vec3<bool> = lt %74, %73
    %77:vec3<f32> = mul %C, %74
    %78:vec3<f32> = add %77, %F
    %79:vec3<f32> = mul %75, %78
    %80:vec3<f32> = mul %A, %74
    %81:vec3<f32> = add %80, %B
    %82:vec3<f32> = pow %81, %72
    %83:vec3<f32> = add %82, %E
    %84:vec3<f32> = mul %75, %83
    %85:vec3<f32> = select %84, %79, %76
    ret %85
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B14: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %95:bool = lte %v_2, %cutoff
    if %95 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %96:f32 = mul %v_2, %v_2
        %97:f32 = div %96, %lower_scale
        ret %97
      }
      $B16: {  # false
        %98:f32 = sub %v_2, %C_1
        %99:f32 = div %98, %A_1
        %100:f32 = exp %99
        %101:f32 = add %B_1, %100
        %102:f32 = div %101, %upper_scale
        ret %102
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %105:f32 = swizzle %v_3, x
    %106:f32 = call %tint_ApplyHLGSingleChannel, %105, %params_4
    %107:f32 = swizzle %v_3, y
    %108:f32 = call %tint_ApplyHLGSingleChannel, %107, %params_4
    %109:f32 = swizzle %v_3, z
    %110:f32 = call %tint_ApplyHLGSingleChannel, %109, %params_4
    %111:vec3<f32> = construct %106, %108, %110
    ret %111
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %119:vec3<f32> = construct %c1
    %120:vec3<f32> = construct %c2
    %121:vec3<f32> = construct %c3
    %122:vec3<f32> = construct %m1
    %123:vec3<f32> = construct %m2
    %124:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %125:vec3<f32> = div vec3<f32>(1.0f), %123
    %126:vec3<f32> = pow %124, %125
    %127:vec3<f32> = sub %126, %119
    %128:vec3<f32> = max %127, vec3<f32>(0.0f)
    %129:vec3<f32> = mul %121, %126
    %130:vec3<f32> = sub %120, %129
    %131:vec3<f32> = div %128, %130
    %132:vec3<f32> = div vec3<f32>(1.0f), %122
    %133:vec3<f32> = pow %131, %132
    ret %133
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, YcbcrTextureLoad_SignedCoords) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4f());
    auto* coords = b.FunctionParam("coords", ty.vec2i());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func(%coords:vec2<i32>):vec4<f32> {
  $B2: {
    %6:texture_2d<f32> = load %texture
    %7:sampler = load %texture_ycbcr_sampler
    %8:tint_ExternalTextureParams = load %texture_params
    %9:vec2<u32> = convert %coords
    %result:vec4<f32> = call %tint_TextureLoadYcbcrExternal, %6, %8, %9
    ret %result
  }
}
%tint_TextureLoadYcbcrExternal = func(%texture_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %texture_1: 'texture', %coords_1: 'coords'
  $B3: {
    %15:u32 = access %params, 1u
    %16:mat3x4<f32> = access %params, 2u
    %17:mat3x2<f32> = access %params, 7u
    %18:vec2<u32> = access %params, 12u
    %19:vec2<u32> = min %coords_1, %18
    %20:vec2<f32> = convert %19
    %21:vec3<f32> = construct %20, 1.0f
    %22:vec2<f32> = mul %17, %21
    %23:vec2<f32> = round %22
    %24:vec2<u32> = convert %23
    %25:vec4<f32> = textureLoad %texture_1, %24, 0u
    %26:vec3<f32> = swizzle %25, xyz
    %27:vec4<f32> = construct %26, 1.0f
    %28:vec3<f32> = mul %27, %16
    %29:bool = eq %15, 0u
    %30:vec3<f32> = if %29 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %31:tint_TransferFunctionParams = access %params, 3u
        %32:tint_TransferFunctionParams = access %params, 4u
        %33:mat3x3<f32> = access %params, 5u
        %34:vec3<f32> = call %tint_ApplySrcTransferFunction, %28, %31
        %36:vec3<f32> = mul %33, %34
        %37:vec3<f32> = call %tint_ApplyGammaTransferFunction, %36, %32
        exit_if %37  # if_1
      }
      $B5: {  # false
        exit_if %28  # if_1
      }
    }
    %39:vec4<f32> = construct %30, 1.0f
    ret %39
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B6: {
    %mode:u32 = access %params_1, 0u
    %43:bool = eq %mode, 0u
    if %43 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %44:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %44
      }
      $B8: {  # false
        %45:bool = eq %mode, 1u
        if %45 [t: $B9, f: $B10] {  # if_3
          $B9: {  # true
            %46:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %46
          }
          $B10: {  # false
            %48:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %48
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B11: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %59:vec3<f32> = construct %G
    %60:vec3<f32> = construct %D
    %61:vec3<f32> = abs %v_1
    %62:vec3<f32> = sign %v_1
    %63:vec3<bool> = lt %61, %60
    %64:vec3<f32> = mul %C, %61
    %65:vec3<f32> = add %64, %F
    %66:vec3<f32> = mul %62, %65
    %67:vec3<f32> = mul %A, %61
    %68:vec3<f32> = add %67, %B
    %69:vec3<f32> = pow %68, %59
    %70:vec3<f32> = add %69, %E
    %71:vec3<f32> = mul %62, %70
    %72:vec3<f32> = select %71, %66, %63
    ret %72
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B12: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %82:bool = lte %v_2, %cutoff
    if %82 [t: $B13, f: $B14] {  # if_4
      $B13: {  # true
        %83:f32 = mul %v_2, %v_2
        %84:f32 = div %83, %lower_scale
        ret %84
      }
      $B14: {  # false
        %85:f32 = sub %v_2, %C_1
        %86:f32 = div %85, %A_1
        %87:f32 = exp %86
        %88:f32 = add %B_1, %87
        %89:f32 = div %88, %upper_scale
        ret %89
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B15: {
    %92:f32 = swizzle %v_3, x
    %93:f32 = call %tint_ApplyHLGSingleChannel, %92, %params_4
    %94:f32 = swizzle %v_3, y
    %95:f32 = call %tint_ApplyHLGSingleChannel, %94, %params_4
    %96:f32 = swizzle %v_3, z
    %97:f32 = call %tint_ApplyHLGSingleChannel, %96, %params_4
    %98:vec3<f32> = construct %93, %95, %97
    ret %98
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B16: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %106:vec3<f32> = construct %c1
    %107:vec3<f32> = construct %c2
    %108:vec3<f32> = construct %c3
    %109:vec3<f32> = construct %m1
    %110:vec3<f32> = construct %m2
    %111:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %112:vec3<f32> = div vec3<f32>(1.0f), %110
    %113:vec3<f32> = pow %111, %112
    %114:vec3<f32> = sub %113, %106
    %115:vec3<f32> = max %114, vec3<f32>(0.0f)
    %116:vec3<f32> = mul %108, %113
    %117:vec3<f32> = sub %107, %116
    %118:vec3<f32> = div %115, %117
    %119:vec3<f32> = div vec3<f32>(1.0f), %109
    %120:vec3<f32> = pow %118, %119
    ret %120
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, Multiplanar_TextureSampleBaseClampToEdge) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4f());
    auto* sampler = b.FunctionParam("sampler", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2f());
    func->SetParams({sampler, coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load,
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> {
  $B2: {
    %7:texture_2d<f32> = load %texture_plane0
    %8:texture_2d<f32> = load %texture_plane1
    %9:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %tint_TextureSampleClampToEdgeMultiplanarExternal, %7, %8, %9, %sampler, %coords
    ret %result
  }
}
%tint_TextureSampleClampToEdgeMultiplanarExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %tint_sampler:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %coords_1: 'coords'
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
        %43:tint_TransferFunctionParams = access %params, 3u
        %44:tint_TransferFunctionParams = access %params, 4u
        %45:mat3x3<f32> = access %params, 5u
        %46:vec3<f32> = call %tint_ApplySrcTransferFunction, %29, %43
        %48:vec3<f32> = mul %45, %46
        %49:vec3<f32> = call %tint_ApplyGammaTransferFunction, %48, %44
        exit_if %49  # if_2
      }
      $B7: {  # false
        exit_if %29  # if_2
      }
    }
    %51:vec4<f32> = construct %42, %30
    ret %51
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %55:bool = eq %mode, 0u
    if %55 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %56:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %56
      }
      $B10: {  # false
        %57:bool = eq %mode, 1u
        if %57 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %58:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %58
          }
          $B12: {  # false
            %60:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %60
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B13: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %71:vec3<f32> = construct %G
    %72:vec3<f32> = construct %D
    %73:vec3<f32> = abs %v_1
    %74:vec3<f32> = sign %v_1
    %75:vec3<bool> = lt %73, %72
    %76:vec3<f32> = mul %C, %73
    %77:vec3<f32> = add %76, %F
    %78:vec3<f32> = mul %74, %77
    %79:vec3<f32> = mul %A, %73
    %80:vec3<f32> = add %79, %B
    %81:vec3<f32> = pow %80, %71
    %82:vec3<f32> = add %81, %E
    %83:vec3<f32> = mul %74, %82
    %84:vec3<f32> = select %83, %78, %75
    ret %84
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B14: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %94:bool = lte %v_2, %cutoff
    if %94 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %95:f32 = mul %v_2, %v_2
        %96:f32 = div %95, %lower_scale
        ret %96
      }
      $B16: {  # false
        %97:f32 = sub %v_2, %C_1
        %98:f32 = div %97, %A_1
        %99:f32 = exp %98
        %100:f32 = add %B_1, %99
        %101:f32 = div %100, %upper_scale
        ret %101
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %104:f32 = swizzle %v_3, x
    %105:f32 = call %tint_ApplyHLGSingleChannel, %104, %params_4
    %106:f32 = swizzle %v_3, y
    %107:f32 = call %tint_ApplyHLGSingleChannel, %106, %params_4
    %108:f32 = swizzle %v_3, z
    %109:f32 = call %tint_ApplyHLGSingleChannel, %108, %params_4
    %110:vec3<f32> = construct %105, %107, %109
    ret %110
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %118:vec3<f32> = construct %c1
    %119:vec3<f32> = construct %c2
    %120:vec3<f32> = construct %c3
    %121:vec3<f32> = construct %m1
    %122:vec3<f32> = construct %m2
    %123:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %124:vec3<f32> = div vec3<f32>(1.0f), %122
    %125:vec3<f32> = pow %123, %124
    %126:vec3<f32> = sub %125, %118
    %127:vec3<f32> = max %126, vec3<f32>(0.0f)
    %128:vec3<f32> = mul %120, %125
    %129:vec3<f32> = sub %119, %128
    %130:vec3<f32> = div %127, %129
    %131:vec3<f32> = div vec3<f32>(1.0f), %121
    %132:vec3<f32> = pow %130, %131
    ret %132
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, Ycbcr_TextureSampleBaseClampToEdge) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4f());
    auto* sampler = b.FunctionParam("sampler", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2f());
    func->SetParams({sampler, coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load,
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> {
  $B2: {
    %7:texture_2d<f32> = load %texture
    %8:sampler = load %texture_ycbcr_sampler
    %9:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %tint_TextureSampleClampToEdgeYcbcrExternal, %7, %8, %9, %coords
    ret %result
  }
}
%tint_TextureSampleClampToEdgeYcbcrExternal = func(%texture_1:texture_2d<f32>, %ycbcr_sampler:sampler, %params:tint_ExternalTextureParams, %coords_1:vec2<f32>):vec4<f32> {  # %texture_1: 'texture', %coords_1: 'coords'
  $B3: {
    %16:u32 = access %params, 1u
    %17:mat3x4<f32> = access %params, 2u
    %18:mat3x2<f32> = access %params, 6u
    %19:vec2<f32> = access %params, 8u
    %20:vec2<f32> = access %params, 9u
    %21:vec3<f32> = construct %coords_1, 1.0f
    %22:vec2<f32> = mul %18, %21
    %23:vec2<f32> = clamp %22, %19, %20
    %24:vec4<f32> = textureSampleLevel %texture_1, %ycbcr_sampler, %23, 0.0f
    %25:vec3<f32> = swizzle %24, xyz
    %26:vec4<f32> = construct %25, 1.0f
    %27:vec3<f32> = mul %26, %17
    %28:f32 = swizzle %24, w
    %29:bool = eq %16, 0u
    %30:vec3<f32> = if %29 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %31:tint_TransferFunctionParams = access %params, 3u
        %32:tint_TransferFunctionParams = access %params, 4u
        %33:mat3x3<f32> = access %params, 5u
        %34:vec3<f32> = call %tint_ApplySrcTransferFunction, %27, %31
        %36:vec3<f32> = mul %33, %34
        %37:vec3<f32> = call %tint_ApplyGammaTransferFunction, %36, %32
        exit_if %37  # if_1
      }
      $B5: {  # false
        exit_if %27  # if_1
      }
    }
    %39:vec4<f32> = construct %30, %28
    ret %39
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B6: {
    %mode:u32 = access %params_1, 0u
    %43:bool = eq %mode, 0u
    if %43 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %44:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %44
      }
      $B8: {  # false
        %45:bool = eq %mode, 1u
        if %45 [t: $B9, f: $B10] {  # if_3
          $B9: {  # true
            %46:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %46
          }
          $B10: {  # false
            %48:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %48
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B11: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %59:vec3<f32> = construct %G
    %60:vec3<f32> = construct %D
    %61:vec3<f32> = abs %v_1
    %62:vec3<f32> = sign %v_1
    %63:vec3<bool> = lt %61, %60
    %64:vec3<f32> = mul %C, %61
    %65:vec3<f32> = add %64, %F
    %66:vec3<f32> = mul %62, %65
    %67:vec3<f32> = mul %A, %61
    %68:vec3<f32> = add %67, %B
    %69:vec3<f32> = pow %68, %59
    %70:vec3<f32> = add %69, %E
    %71:vec3<f32> = mul %62, %70
    %72:vec3<f32> = select %71, %66, %63
    ret %72
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B12: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %82:bool = lte %v_2, %cutoff
    if %82 [t: $B13, f: $B14] {  # if_4
      $B13: {  # true
        %83:f32 = mul %v_2, %v_2
        %84:f32 = div %83, %lower_scale
        ret %84
      }
      $B14: {  # false
        %85:f32 = sub %v_2, %C_1
        %86:f32 = div %85, %A_1
        %87:f32 = exp %86
        %88:f32 = add %B_1, %87
        %89:f32 = div %88, %upper_scale
        ret %89
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B15: {
    %92:f32 = swizzle %v_3, x
    %93:f32 = call %tint_ApplyHLGSingleChannel, %92, %params_4
    %94:f32 = swizzle %v_3, y
    %95:f32 = call %tint_ApplyHLGSingleChannel, %94, %params_4
    %96:f32 = swizzle %v_3, z
    %97:f32 = call %tint_ApplyHLGSingleChannel, %96, %params_4
    %98:vec3<f32> = construct %93, %95, %97
    ret %98
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B16: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %106:vec3<f32> = construct %c1
    %107:vec3<f32> = construct %c2
    %108:vec3<f32> = construct %c3
    %109:vec3<f32> = construct %m1
    %110:vec3<f32> = construct %m2
    %111:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %112:vec3<f32> = div vec3<f32>(1.0f), %110
    %113:vec3<f32> = pow %111, %112
    %114:vec3<f32> = sub %113, %106
    %115:vec3<f32> = max %114, vec3<f32>(0.0f)
    %116:vec3<f32> = mul %108, %113
    %117:vec3<f32> = sub %107, %116
    %118:vec3<f32> = div %115, %117
    %119:vec3<f32> = div vec3<f32>(1.0f), %109
    %120:vec3<f32> = pow %118, %119
    ret %120
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, Multiplanar_ViaUserFunctionParameter) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4f());
    {
        auto* texture = b.FunctionParam("texture", ty.external_texture());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4f());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        bar->SetParams({sampler, coords});
        b.Append(bar->Block(), [&] {
            auto* load = b.Load(var->Result());
            auto* result = b.Call(ty.vec4f(), foo, load, sampler, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
}

%foo = func(%texture_plane0_1:texture_2d<f32>, %texture_plane1_1:texture_2d<f32>, %texture_params_1:tint_ExternalTextureParams, %sampler:sampler, %coords:vec2<f32>):vec4<f32> {  # %texture_plane0_1: 'texture_plane0', %texture_plane1_1: 'texture_plane1', %texture_params_1: 'texture_params'
  $B2: {
    %result:vec4<f32> = call %tint_TextureSampleClampToEdgeMultiplanarExternal, %texture_plane0_1, %texture_plane1_1, %texture_params_1, %sampler, %coords
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
%tint_TextureSampleClampToEdgeMultiplanarExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %tint_sampler:sampler, %coords_2:vec2<f32>):vec4<f32> {  # %coords_2: 'coords'
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
        %50:tint_TransferFunctionParams = access %params, 3u
        %51:tint_TransferFunctionParams = access %params, 4u
        %52:mat3x3<f32> = access %params, 5u
        %53:vec3<f32> = call %tint_ApplySrcTransferFunction, %36, %50
        %55:vec3<f32> = mul %52, %53
        %56:vec3<f32> = call %tint_ApplyGammaTransferFunction, %55, %51
        exit_if %56  # if_2
      }
      $B8: {  # false
        exit_if %36  # if_2
      }
    }
    %58:vec4<f32> = construct %49, %37
    ret %58
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B9: {
    %mode:u32 = access %params_1, 0u
    %62:bool = eq %mode, 0u
    if %62 [t: $B10, f: $B11] {  # if_3
      $B10: {  # true
        %63:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %63
      }
      $B11: {  # false
        %64:bool = eq %mode, 1u
        if %64 [t: $B12, f: $B13] {  # if_4
          $B12: {  # true
            %65:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %65
          }
          $B13: {  # false
            %67:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %67
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B14: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %78:vec3<f32> = construct %G
    %79:vec3<f32> = construct %D
    %80:vec3<f32> = abs %v_1
    %81:vec3<f32> = sign %v_1
    %82:vec3<bool> = lt %80, %79
    %83:vec3<f32> = mul %C, %80
    %84:vec3<f32> = add %83, %F
    %85:vec3<f32> = mul %81, %84
    %86:vec3<f32> = mul %A, %80
    %87:vec3<f32> = add %86, %B
    %88:vec3<f32> = pow %87, %78
    %89:vec3<f32> = add %88, %E
    %90:vec3<f32> = mul %81, %89
    %91:vec3<f32> = select %90, %85, %82
    ret %91
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B15: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %101:bool = lte %v_2, %cutoff
    if %101 [t: $B16, f: $B17] {  # if_5
      $B16: {  # true
        %102:f32 = mul %v_2, %v_2
        %103:f32 = div %102, %lower_scale
        ret %103
      }
      $B17: {  # false
        %104:f32 = sub %v_2, %C_1
        %105:f32 = div %104, %A_1
        %106:f32 = exp %105
        %107:f32 = add %B_1, %106
        %108:f32 = div %107, %upper_scale
        ret %108
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B18: {
    %111:f32 = swizzle %v_3, x
    %112:f32 = call %tint_ApplyHLGSingleChannel, %111, %params_4
    %113:f32 = swizzle %v_3, y
    %114:f32 = call %tint_ApplyHLGSingleChannel, %113, %params_4
    %115:f32 = swizzle %v_3, z
    %116:f32 = call %tint_ApplyHLGSingleChannel, %115, %params_4
    %117:vec3<f32> = construct %112, %114, %116
    ret %117
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B19: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %125:vec3<f32> = construct %c1
    %126:vec3<f32> = construct %c2
    %127:vec3<f32> = construct %c3
    %128:vec3<f32> = construct %m1
    %129:vec3<f32> = construct %m2
    %130:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %131:vec3<f32> = div vec3<f32>(1.0f), %129
    %132:vec3<f32> = pow %130, %131
    %133:vec3<f32> = sub %132, %125
    %134:vec3<f32> = max %133, vec3<f32>(0.0f)
    %135:vec3<f32> = mul %127, %132
    %136:vec3<f32> = sub %126, %135
    %137:vec3<f32> = div %134, %136
    %138:vec3<f32> = div vec3<f32>(1.0f), %128
    %139:vec3<f32> = pow %137, %138
    ret %139
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, Ycbcr_ViaUserFunctionParameter) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4f());
    {
        auto* texture = b.FunctionParam("texture", ty.external_texture());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4f());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        bar->SetParams({sampler, coords});
        b.Append(bar->Block(), [&] {
            auto* load = b.Load(var->Result());
            auto* result = b.Call(ty.vec4f(), foo, load, sampler, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_ycbcr_sampler:ptr<handle, sampler, read> = var undef @binding_point(1, 3)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> {
  $B2: {
    %7:texture_2d<f32> = load %texture
    %8:sampler = load %texture_ycbcr_sampler
    %9:tint_ExternalTextureParams = load %texture_params
    %10:vec4<f32> = call %tint_TextureSampleClampToEdgeYcbcrExternal, %7, %8, %9, %coords
    ret %10
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> {  # %sampler_1: 'sampler', %coords_1: 'coords'
  $B3: {
    %result:vec4<f32> = call %foo, %sampler_1, %coords_1
    ret %result
  }
}
%tint_TextureSampleClampToEdgeYcbcrExternal = func(%texture_1:texture_2d<f32>, %ycbcr_sampler:sampler, %params:tint_ExternalTextureParams, %coords_2:vec2<f32>):vec4<f32> {  # %texture_1: 'texture', %coords_2: 'coords'
  $B4: {
    %20:u32 = access %params, 1u
    %21:mat3x4<f32> = access %params, 2u
    %22:mat3x2<f32> = access %params, 6u
    %23:vec2<f32> = access %params, 8u
    %24:vec2<f32> = access %params, 9u
    %25:vec3<f32> = construct %coords_2, 1.0f
    %26:vec2<f32> = mul %22, %25
    %27:vec2<f32> = clamp %26, %23, %24
    %28:vec4<f32> = textureSampleLevel %texture_1, %ycbcr_sampler, %27, 0.0f
    %29:vec3<f32> = swizzle %28, xyz
    %30:vec4<f32> = construct %29, 1.0f
    %31:vec3<f32> = mul %30, %21
    %32:f32 = swizzle %28, w
    %33:bool = eq %20, 0u
    %34:vec3<f32> = if %33 [t: $B5, f: $B6] {  # if_1
      $B5: {  # true
        %35:tint_TransferFunctionParams = access %params, 3u
        %36:tint_TransferFunctionParams = access %params, 4u
        %37:mat3x3<f32> = access %params, 5u
        %38:vec3<f32> = call %tint_ApplySrcTransferFunction, %31, %35
        %40:vec3<f32> = mul %37, %38
        %41:vec3<f32> = call %tint_ApplyGammaTransferFunction, %40, %36
        exit_if %41  # if_1
      }
      $B6: {  # false
        exit_if %31  # if_1
      }
    }
    %43:vec4<f32> = construct %34, %32
    ret %43
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B7: {
    %mode:u32 = access %params_1, 0u
    %47:bool = eq %mode, 0u
    if %47 [t: $B8, f: $B9] {  # if_2
      $B8: {  # true
        %48:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %48
      }
      $B9: {  # false
        %49:bool = eq %mode, 1u
        if %49 [t: $B10, f: $B11] {  # if_3
          $B10: {  # true
            %50:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %50
          }
          $B11: {  # false
            %52:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %52
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B12: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %63:vec3<f32> = construct %G
    %64:vec3<f32> = construct %D
    %65:vec3<f32> = abs %v_1
    %66:vec3<f32> = sign %v_1
    %67:vec3<bool> = lt %65, %64
    %68:vec3<f32> = mul %C, %65
    %69:vec3<f32> = add %68, %F
    %70:vec3<f32> = mul %66, %69
    %71:vec3<f32> = mul %A, %65
    %72:vec3<f32> = add %71, %B
    %73:vec3<f32> = pow %72, %63
    %74:vec3<f32> = add %73, %E
    %75:vec3<f32> = mul %66, %74
    %76:vec3<f32> = select %75, %70, %67
    ret %76
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B13: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %86:bool = lte %v_2, %cutoff
    if %86 [t: $B14, f: $B15] {  # if_4
      $B14: {  # true
        %87:f32 = mul %v_2, %v_2
        %88:f32 = div %87, %lower_scale
        ret %88
      }
      $B15: {  # false
        %89:f32 = sub %v_2, %C_1
        %90:f32 = div %89, %A_1
        %91:f32 = exp %90
        %92:f32 = add %B_1, %91
        %93:f32 = div %92, %upper_scale
        ret %93
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B16: {
    %96:f32 = swizzle %v_3, x
    %97:f32 = call %tint_ApplyHLGSingleChannel, %96, %params_4
    %98:f32 = swizzle %v_3, y
    %99:f32 = call %tint_ApplyHLGSingleChannel, %98, %params_4
    %100:f32 = swizzle %v_3, z
    %101:f32 = call %tint_ApplyHLGSingleChannel, %100, %params_4
    %102:vec3<f32> = construct %97, %99, %101
    ret %102
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B17: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %110:vec3<f32> = construct %c1
    %111:vec3<f32> = construct %c2
    %112:vec3<f32> = construct %c3
    %113:vec3<f32> = construct %m1
    %114:vec3<f32> = construct %m2
    %115:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %116:vec3<f32> = div vec3<f32>(1.0f), %114
    %117:vec3<f32> = pow %115, %116
    %118:vec3<f32> = sub %117, %110
    %119:vec3<f32> = max %118, vec3<f32>(0.0f)
    %120:vec3<f32> = mul %112, %117
    %121:vec3<f32> = sub %111, %120
    %122:vec3<f32> = div %119, %121
    %123:vec3<f32> = div vec3<f32>(1.0f), %113
    %124:vec3<f32> = pow %122, %123
    ret %124
  }
}
)";

    EXPECT_EQ(src, str());

    Run(DirectVariableAccess,
        DirectVariableAccessOptions{.transform_handle = HandleTransformLevel::kExternal});

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::YCBCRTexture{{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultipleUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.external_texture()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4f());
    {
        auto* texture = b.FunctionParam("texture", ty.external_texture());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4f());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords_f = b.FunctionParam("coords", ty.vec2f());
        bar->SetParams({sampler, coords_f});
        b.Append(bar->Block(), [&] {
            auto* load_a = b.Load(var->Result());
            b.Call(ty.vec2u(), core::BuiltinFn::kTextureDimensions, load_a);
            auto* load_b = b.Load(var->Result());
            b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load_b, sampler,
                   coords_f);
            auto* load_c = b.Load(var->Result());
            b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load_c, sampler,
                   coords_f);
            auto* load_d = b.Load(var->Result());
            auto* result_a = b.Call(ty.vec4f(), foo, load_d, sampler, coords_f);
            auto* result_b = b.Call(ty.vec4f(), foo, load_d, sampler, coords_f);
            b.Return(bar, b.Add(result_a, result_b));
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
}

%foo = func(%texture_plane0_1:texture_2d<f32>, %texture_plane1_1:texture_2d<f32>, %texture_params_1:tint_ExternalTextureParams, %sampler:sampler, %coords:vec2<f32>):vec4<f32> {  # %texture_plane0_1: 'texture_plane0', %texture_plane1_1: 'texture_plane1', %texture_params_1: 'texture_params'
  $B2: {
    %result:vec4<f32> = call %tint_TextureSampleClampToEdgeMultiplanarExternal, %texture_plane0_1, %texture_plane1_1, %texture_params_1, %sampler, %coords
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
    %23:vec4<f32> = call %tint_TextureSampleClampToEdgeMultiplanarExternal, %20, %21, %22, %sampler_1, %coords_1
    %24:texture_2d<f32> = load %texture_plane0
    %25:texture_2d<f32> = load %texture_plane1
    %26:tint_ExternalTextureParams = load %texture_params
    %27:vec4<f32> = call %tint_TextureSampleClampToEdgeMultiplanarExternal, %24, %25, %26, %sampler_1, %coords_1
    %28:texture_2d<f32> = load %texture_plane0
    %29:texture_2d<f32> = load %texture_plane1
    %30:tint_ExternalTextureParams = load %texture_params
    %result_a:vec4<f32> = call %foo, %28, %29, %30, %sampler_1, %coords_1
    %result_b:vec4<f32> = call %foo, %28, %29, %30, %sampler_1, %coords_1
    %33:vec4<f32> = add %result_a, %result_b
    ret %33
  }
}
%tint_TextureSampleClampToEdgeMultiplanarExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %tint_sampler:sampler, %coords_2:vec2<f32>):vec4<f32> {  # %coords_2: 'coords'
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
        %65:tint_TransferFunctionParams = access %params, 3u
        %66:tint_TransferFunctionParams = access %params, 4u
        %67:mat3x3<f32> = access %params, 5u
        %68:vec3<f32> = call %tint_ApplySrcTransferFunction, %51, %65
        %70:vec3<f32> = mul %67, %68
        %71:vec3<f32> = call %tint_ApplyGammaTransferFunction, %70, %66
        exit_if %71  # if_2
      }
      $B8: {  # false
        exit_if %51  # if_2
      }
    }
    %73:vec4<f32> = construct %64, %52
    ret %73
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B9: {
    %mode:u32 = access %params_1, 0u
    %77:bool = eq %mode, 0u
    if %77 [t: $B10, f: $B11] {  # if_3
      $B10: {  # true
        %78:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %78
      }
      $B11: {  # false
        %79:bool = eq %mode, 1u
        if %79 [t: $B12, f: $B13] {  # if_4
          $B12: {  # true
            %80:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %80
          }
          $B13: {  # false
            %82:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %82
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B14: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %93:vec3<f32> = construct %G
    %94:vec3<f32> = construct %D
    %95:vec3<f32> = abs %v_1
    %96:vec3<f32> = sign %v_1
    %97:vec3<bool> = lt %95, %94
    %98:vec3<f32> = mul %C, %95
    %99:vec3<f32> = add %98, %F
    %100:vec3<f32> = mul %96, %99
    %101:vec3<f32> = mul %A, %95
    %102:vec3<f32> = add %101, %B
    %103:vec3<f32> = pow %102, %93
    %104:vec3<f32> = add %103, %E
    %105:vec3<f32> = mul %96, %104
    %106:vec3<f32> = select %105, %100, %97
    ret %106
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B15: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %116:bool = lte %v_2, %cutoff
    if %116 [t: $B16, f: $B17] {  # if_5
      $B16: {  # true
        %117:f32 = mul %v_2, %v_2
        %118:f32 = div %117, %lower_scale
        ret %118
      }
      $B17: {  # false
        %119:f32 = sub %v_2, %C_1
        %120:f32 = div %119, %A_1
        %121:f32 = exp %120
        %122:f32 = add %B_1, %121
        %123:f32 = div %122, %upper_scale
        ret %123
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B18: {
    %126:f32 = swizzle %v_3, x
    %127:f32 = call %tint_ApplyHLGSingleChannel, %126, %params_4
    %128:f32 = swizzle %v_3, y
    %129:f32 = call %tint_ApplyHLGSingleChannel, %128, %params_4
    %130:f32 = swizzle %v_3, z
    %131:f32 = call %tint_ApplyHLGSingleChannel, %130, %params_4
    %132:vec3<f32> = construct %127, %129, %131
    ret %132
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B19: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %140:vec3<f32> = construct %c1
    %141:vec3<f32> = construct %c2
    %142:vec3<f32> = construct %c3
    %143:vec3<f32> = construct %m1
    %144:vec3<f32> = construct %m2
    %145:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %146:vec3<f32> = div vec3<f32>(1.0f), %144
    %147:vec3<f32> = pow %145, %146
    %148:vec3<f32> = sub %147, %140
    %149:vec3<f32> = max %148, vec3<f32>(0.0f)
    %150:vec3<f32> = mul %142, %147
    %151:vec3<f32> = sub %141, %150
    %152:vec3<f32> = div %149, %151
    %153:vec3<f32> = div vec3<f32>(1.0f), %143
    %154:vec3<f32> = pow %152, %153
    ret %154
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
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
    auto* coords = b.FunctionParam("coords", ty.vec2u());
    foo->SetParams({coords});
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a->Result());
        b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load_a, coords);
        auto* load_b = b.Load(var_b->Result());
        b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load_b, coords);
        auto* load_c = b.Load(var_c->Result());
        b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, load_c, coords);
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
    auto expect = std::string(kExternalTextureParams) + R"(
$B1: {  # root
  %texture_a_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(1, 4)
  %texture_a_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 2)
  %texture_a_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(1, 3)
  %texture_b_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(2, 4)
  %texture_b_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 2)
  %texture_b_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(2, 3)
  %texture_c_params:ptr<uniform, tint_ExternalTextureParams, read> = var undef @binding_point(3, 4)
  %texture_c_plane0:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
  %texture_c_plane1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 3)
}

%foo = func(%coords:vec2<u32>):void {
  $B2: {
    %12:texture_2d<f32> = load %texture_a_plane0
    %13:texture_2d<f32> = load %texture_a_plane1
    %14:tint_ExternalTextureParams = load %texture_a_params
    %15:vec4<f32> = call %tint_TextureLoadMultiplanarExternal, %12, %13, %14, %coords
    %17:texture_2d<f32> = load %texture_b_plane0
    %18:texture_2d<f32> = load %texture_b_plane1
    %19:tint_ExternalTextureParams = load %texture_b_params
    %20:vec4<f32> = call %tint_TextureLoadMultiplanarExternal, %17, %18, %19, %coords
    %21:texture_2d<f32> = load %texture_c_plane0
    %22:texture_2d<f32> = load %texture_c_plane1
    %23:tint_ExternalTextureParams = load %texture_c_params
    %24:vec4<f32> = call %tint_TextureLoadMultiplanarExternal, %21, %22, %23, %coords
    ret
  }
}
%tint_TextureLoadMultiplanarExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> {  # %coords_1: 'coords'
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
        %57:tint_TransferFunctionParams = access %params, 3u
        %58:tint_TransferFunctionParams = access %params, 4u
        %59:mat3x3<f32> = access %params, 5u
        %60:vec3<f32> = call %tint_ApplySrcTransferFunction, %42, %57
        %62:vec3<f32> = mul %59, %60
        %63:vec3<f32> = call %tint_ApplyGammaTransferFunction, %62, %58
        exit_if %63  # if_2
      }
      $B7: {  # false
        exit_if %42  # if_2
      }
    }
    %65:vec4<f32> = construct %56, %43
    ret %65
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %69:bool = eq %mode, 0u
    if %69 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %70:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %70
      }
      $B10: {  # false
        %71:bool = eq %mode, 1u
        if %71 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %72:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %72
          }
          $B12: {  # false
            %74:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %74
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B13: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %85:vec3<f32> = construct %G
    %86:vec3<f32> = construct %D
    %87:vec3<f32> = abs %v_1
    %88:vec3<f32> = sign %v_1
    %89:vec3<bool> = lt %87, %86
    %90:vec3<f32> = mul %C, %87
    %91:vec3<f32> = add %90, %F
    %92:vec3<f32> = mul %88, %91
    %93:vec3<f32> = mul %A, %87
    %94:vec3<f32> = add %93, %B
    %95:vec3<f32> = pow %94, %85
    %96:vec3<f32> = add %95, %E
    %97:vec3<f32> = mul %88, %96
    %98:vec3<f32> = select %97, %92, %89
    ret %98
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B14: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %108:bool = lte %v_2, %cutoff
    if %108 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %109:f32 = mul %v_2, %v_2
        %110:f32 = div %109, %lower_scale
        ret %110
      }
      $B16: {  # false
        %111:f32 = sub %v_2, %C_1
        %112:f32 = div %111, %A_1
        %113:f32 = exp %112
        %114:f32 = add %B_1, %113
        %115:f32 = div %114, %upper_scale
        ret %115
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %118:f32 = swizzle %v_3, x
    %119:f32 = call %tint_ApplyHLGSingleChannel, %118, %params_4
    %120:f32 = swizzle %v_3, y
    %121:f32 = call %tint_ApplyHLGSingleChannel, %120, %params_4
    %122:f32 = swizzle %v_3, z
    %123:f32 = call %tint_ApplyHLGSingleChannel, %122, %params_4
    %124:vec3<f32> = construct %119, %121, %123
    ret %124
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %132:vec3<f32> = construct %c1
    %133:vec3<f32> = construct %c2
    %134:vec3<f32> = construct %c3
    %135:vec3<f32> = construct %m1
    %136:vec3<f32> = construct %m2
    %137:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %138:vec3<f32> = div vec3<f32>(1.0f), %136
    %139:vec3<f32> = pow %137, %138
    %140:vec3<f32> = sub %139, %132
    %141:vec3<f32> = max %140, vec3<f32>(0.0f)
    %142:vec3<f32> = mul %134, %139
    %143:vec3<f32> = sub %133, %142
    %144:vec3<f32> = div %141, %143
    %145:vec3<f32> = div vec3<f32>(1.0f), %135
    %146:vec3<f32> = pow %144, %145
    ret %146
  }
}
)";

    EXPECT_EQ(src, str());

    tint::transform::multiplanar::BindingsMap map{};
    map[{1u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{1u, 3u}, {1u, 4u}};
    map[{2u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{2u, 3u}, {2u, 4u}};
    map[{3u, 2u}] = tint::transform::multiplanar::MultiplanarTexture{{3u, 3u}, {3u, 4u}};
    Run(MultiplanarExternalTexture, map);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
