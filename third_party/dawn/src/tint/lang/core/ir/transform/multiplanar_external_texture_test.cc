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
  ootfParam:vec4<f32> @offset(272)
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
    %15:mat3x4<f32> = access %params, 2u
    %16:mat3x2<f32> = access %params, 7u
    %17:vec2<u32> = access %params, 12u
    %18:vec2<f32> = access %params, 13u
    %19:vec2<u32> = min %coords_1, %17
    %20:vec2<f32> = convert %19
    %21:vec3<f32> = construct %20, 1.0f
    %22:vec2<f32> = mul %16, %21
    %23:vec2<f32> = round %22
    %24:vec2<u32> = convert %23
    %25:u32 = access %params, 0u
    %26:bool = eq %25, 1u
    %27:vec3<f32>, %28:f32 = if %26 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %29:vec4<f32> = textureLoad %plane_0, %24, 0u
        %30:vec3<f32> = swizzle %29, xyz
        %31:f32 = access %29, 3u
        exit_if %30, %31  # if_1
      }
      $B5: {  # false
        %32:vec4<f32> = textureLoad %plane_0, %24, 0u
        %33:f32 = access %32, 0u
        %34:vec2<f32> = mul %23, %18
        %35:vec2<u32> = convert %34
        %36:vec4<f32> = textureLoad %plane_1, %35, 0u
        %37:vec2<f32> = swizzle %36, xy
        %38:vec4<f32> = construct %33, %37, 1.0f
        %39:vec3<f32> = mul %38, %15
        exit_if %39, 1.0f  # if_1
      }
    }
    %40:u32 = access %params, 1u
    %41:bool = eq %40, 0u
    %42:vec3<f32> = if %41 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %43:tint_TransferFunctionParams = access %params, 3u
        %44:tint_TransferFunctionParams = access %params, 4u
        %45:mat3x3<f32> = access %params, 5u
        %46:vec4<f32> = access %params, 14u
        %47:vec3<f32> = call %tint_ApplySrcTransferFunction, %27, %43
        %49:f32 = swizzle %46, w
        %50:bool = neq %49, 0.0f
        %51:vec3<f32> = if %50 [t: $B8, f: $B9] {  # if_3
          $B8: {  # true
            %52:vec3<f32> = swizzle %46, xyz
            %53:f32 = dot %52, %47
            %54:f32 = abs %53
            %55:f32 = pow %54, %49
            %56:f32 = sign %53
            %57:f32 = mul %56, %55
            %58:vec3<f32> = mul %47, %57
            exit_if %58  # if_3
          }
          $B9: {  # false
            exit_if %47  # if_3
          }
        }
        %59:vec3<f32> = mul %45, %51
        %60:vec3<f32> = call %tint_ApplyGammaTransferFunction, %59, %44
        exit_if %60  # if_2
      }
      $B7: {  # false
        exit_if %27  # if_2
      }
    }
    %62:vec4<f32> = construct %42, %28
    ret %62
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B10: {
    %mode:u32 = access %params_1, 0u
    %66:bool = eq %mode, 0u
    if %66 [t: $B11, f: $B12] {  # if_4
      $B11: {  # true
        %67:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %67
      }
      $B12: {  # false
        %68:bool = eq %mode, 1u
        if %68 [t: $B13, f: $B14] {  # if_5
          $B13: {  # true
            %69:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %69
          }
          $B14: {  # false
            %71:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %71
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B15: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %82:vec3<f32> = construct %G
    %83:vec3<f32> = construct %D
    %84:vec3<f32> = abs %v_1
    %85:vec3<f32> = sign %v_1
    %86:vec3<bool> = lt %84, %83
    %87:vec3<f32> = mul %C, %84
    %88:vec3<f32> = add %87, %F
    %89:vec3<f32> = mul %85, %88
    %90:vec3<f32> = mul %A, %84
    %91:vec3<f32> = add %90, %B
    %92:vec3<f32> = pow %91, %82
    %93:vec3<f32> = add %92, %E
    %94:vec3<f32> = mul %85, %93
    %95:vec3<f32> = select %94, %89, %86
    ret %95
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B16: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %105:bool = lte %v_2, %cutoff
    if %105 [t: $B17, f: $B18] {  # if_6
      $B17: {  # true
        %106:f32 = mul %v_2, %v_2
        %107:f32 = div %106, %lower_scale
        ret %107
      }
      $B18: {  # false
        %108:f32 = sub %v_2, %C_1
        %109:f32 = div %108, %A_1
        %110:f32 = exp %109
        %111:f32 = add %B_1, %110
        %112:f32 = div %111, %upper_scale
        ret %112
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B19: {
    %115:f32 = swizzle %v_3, x
    %116:f32 = call %tint_ApplyHLGSingleChannel, %115, %params_4
    %117:f32 = swizzle %v_3, y
    %118:f32 = call %tint_ApplyHLGSingleChannel, %117, %params_4
    %119:f32 = swizzle %v_3, z
    %120:f32 = call %tint_ApplyHLGSingleChannel, %119, %params_4
    %121:vec3<f32> = construct %116, %118, %120
    ret %121
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B20: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %129:vec3<f32> = construct %c1
    %130:vec3<f32> = construct %c2
    %131:vec3<f32> = construct %c3
    %132:vec3<f32> = construct %m1
    %133:vec3<f32> = construct %m2
    %134:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %135:vec3<f32> = div vec3<f32>(1.0f), %133
    %136:vec3<f32> = pow %134, %135
    %137:vec3<f32> = sub %136, %129
    %138:vec3<f32> = max %137, vec3<f32>(0.0f)
    %139:vec3<f32> = mul %131, %136
    %140:vec3<f32> = sub %130, %139
    %141:vec3<f32> = div %138, %140
    %142:vec3<f32> = div vec3<f32>(1.0f), %132
    %143:vec3<f32> = pow %141, %142
    ret %143
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
    %14:mat3x4<f32> = access %params, 2u
    %15:mat3x2<f32> = access %params, 7u
    %16:vec2<u32> = access %params, 12u
    %17:vec2<u32> = min %coords_1, %16
    %18:vec2<f32> = convert %17
    %19:vec3<f32> = construct %18, 1.0f
    %20:vec2<f32> = mul %15, %19
    %21:vec2<f32> = round %20
    %22:vec2<u32> = convert %21
    %23:vec4<f32> = textureLoad %texture_1, %22, 0u
    %24:vec3<f32> = swizzle %23, xyz
    %25:vec4<f32> = construct %24, 1.0f
    %26:vec3<f32> = mul %25, %14
    %27:u32 = access %params, 1u
    %28:bool = eq %27, 0u
    %29:vec3<f32> = if %28 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %30:tint_TransferFunctionParams = access %params, 3u
        %31:tint_TransferFunctionParams = access %params, 4u
        %32:mat3x3<f32> = access %params, 5u
        %33:vec4<f32> = access %params, 14u
        %34:vec3<f32> = call %tint_ApplySrcTransferFunction, %26, %30
        %36:f32 = swizzle %33, w
        %37:bool = neq %36, 0.0f
        %38:vec3<f32> = if %37 [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            %39:vec3<f32> = swizzle %33, xyz
            %40:f32 = dot %39, %34
            %41:f32 = abs %40
            %42:f32 = pow %41, %36
            %43:f32 = sign %40
            %44:f32 = mul %43, %42
            %45:vec3<f32> = mul %34, %44
            exit_if %45  # if_2
          }
          $B7: {  # false
            exit_if %34  # if_2
          }
        }
        %46:vec3<f32> = mul %32, %38
        %47:vec3<f32> = call %tint_ApplyGammaTransferFunction, %46, %31
        exit_if %47  # if_1
      }
      $B5: {  # false
        exit_if %26  # if_1
      }
    }
    %49:vec4<f32> = construct %29, 1.0f
    ret %49
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %53:bool = eq %mode, 0u
    if %53 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %54:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %54
      }
      $B10: {  # false
        %55:bool = eq %mode, 1u
        if %55 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %56:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %56
          }
          $B12: {  # false
            %58:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %58
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
    %69:vec3<f32> = construct %G
    %70:vec3<f32> = construct %D
    %71:vec3<f32> = abs %v_1
    %72:vec3<f32> = sign %v_1
    %73:vec3<bool> = lt %71, %70
    %74:vec3<f32> = mul %C, %71
    %75:vec3<f32> = add %74, %F
    %76:vec3<f32> = mul %72, %75
    %77:vec3<f32> = mul %A, %71
    %78:vec3<f32> = add %77, %B
    %79:vec3<f32> = pow %78, %69
    %80:vec3<f32> = add %79, %E
    %81:vec3<f32> = mul %72, %80
    %82:vec3<f32> = select %81, %76, %73
    ret %82
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
    %92:bool = lte %v_2, %cutoff
    if %92 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %93:f32 = mul %v_2, %v_2
        %94:f32 = div %93, %lower_scale
        ret %94
      }
      $B16: {  # false
        %95:f32 = sub %v_2, %C_1
        %96:f32 = div %95, %A_1
        %97:f32 = exp %96
        %98:f32 = add %B_1, %97
        %99:f32 = div %98, %upper_scale
        ret %99
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %102:f32 = swizzle %v_3, x
    %103:f32 = call %tint_ApplyHLGSingleChannel, %102, %params_4
    %104:f32 = swizzle %v_3, y
    %105:f32 = call %tint_ApplyHLGSingleChannel, %104, %params_4
    %106:f32 = swizzle %v_3, z
    %107:f32 = call %tint_ApplyHLGSingleChannel, %106, %params_4
    %108:vec3<f32> = construct %103, %105, %107
    ret %108
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %116:vec3<f32> = construct %c1
    %117:vec3<f32> = construct %c2
    %118:vec3<f32> = construct %c3
    %119:vec3<f32> = construct %m1
    %120:vec3<f32> = construct %m2
    %121:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %122:vec3<f32> = div vec3<f32>(1.0f), %120
    %123:vec3<f32> = pow %121, %122
    %124:vec3<f32> = sub %123, %116
    %125:vec3<f32> = max %124, vec3<f32>(0.0f)
    %126:vec3<f32> = mul %118, %123
    %127:vec3<f32> = sub %117, %126
    %128:vec3<f32> = div %125, %127
    %129:vec3<f32> = div vec3<f32>(1.0f), %119
    %130:vec3<f32> = pow %128, %129
    ret %130
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
    %41:u32 = access %params, 1u
    %42:bool = eq %41, 0u
    %43:vec3<f32> = if %42 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %44:tint_TransferFunctionParams = access %params, 3u
        %45:tint_TransferFunctionParams = access %params, 4u
        %46:mat3x3<f32> = access %params, 5u
        %47:vec4<f32> = access %params, 14u
        %48:vec3<f32> = call %tint_ApplySrcTransferFunction, %28, %44
        %50:f32 = swizzle %47, w
        %51:bool = neq %50, 0.0f
        %52:vec3<f32> = if %51 [t: $B8, f: $B9] {  # if_3
          $B8: {  # true
            %53:vec3<f32> = swizzle %47, xyz
            %54:f32 = dot %53, %48
            %55:f32 = abs %54
            %56:f32 = pow %55, %50
            %57:f32 = sign %54
            %58:f32 = mul %57, %56
            %59:vec3<f32> = mul %48, %58
            exit_if %59  # if_3
          }
          $B9: {  # false
            exit_if %48  # if_3
          }
        }
        %60:vec3<f32> = mul %46, %52
        %61:vec3<f32> = call %tint_ApplyGammaTransferFunction, %60, %45
        exit_if %61  # if_2
      }
      $B7: {  # false
        exit_if %28  # if_2
      }
    }
    %63:vec4<f32> = construct %43, %29
    ret %63
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B10: {
    %mode:u32 = access %params_1, 0u
    %67:bool = eq %mode, 0u
    if %67 [t: $B11, f: $B12] {  # if_4
      $B11: {  # true
        %68:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %68
      }
      $B12: {  # false
        %69:bool = eq %mode, 1u
        if %69 [t: $B13, f: $B14] {  # if_5
          $B13: {  # true
            %70:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %70
          }
          $B14: {  # false
            %72:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %72
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B15: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %83:vec3<f32> = construct %G
    %84:vec3<f32> = construct %D
    %85:vec3<f32> = abs %v_1
    %86:vec3<f32> = sign %v_1
    %87:vec3<bool> = lt %85, %84
    %88:vec3<f32> = mul %C, %85
    %89:vec3<f32> = add %88, %F
    %90:vec3<f32> = mul %86, %89
    %91:vec3<f32> = mul %A, %85
    %92:vec3<f32> = add %91, %B
    %93:vec3<f32> = pow %92, %83
    %94:vec3<f32> = add %93, %E
    %95:vec3<f32> = mul %86, %94
    %96:vec3<f32> = select %95, %90, %87
    ret %96
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B16: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %106:bool = lte %v_2, %cutoff
    if %106 [t: $B17, f: $B18] {  # if_6
      $B17: {  # true
        %107:f32 = mul %v_2, %v_2
        %108:f32 = div %107, %lower_scale
        ret %108
      }
      $B18: {  # false
        %109:f32 = sub %v_2, %C_1
        %110:f32 = div %109, %A_1
        %111:f32 = exp %110
        %112:f32 = add %B_1, %111
        %113:f32 = div %112, %upper_scale
        ret %113
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B19: {
    %116:f32 = swizzle %v_3, x
    %117:f32 = call %tint_ApplyHLGSingleChannel, %116, %params_4
    %118:f32 = swizzle %v_3, y
    %119:f32 = call %tint_ApplyHLGSingleChannel, %118, %params_4
    %120:f32 = swizzle %v_3, z
    %121:f32 = call %tint_ApplyHLGSingleChannel, %120, %params_4
    %122:vec3<f32> = construct %117, %119, %121
    ret %122
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B20: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %130:vec3<f32> = construct %c1
    %131:vec3<f32> = construct %c2
    %132:vec3<f32> = construct %c3
    %133:vec3<f32> = construct %m1
    %134:vec3<f32> = construct %m2
    %135:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %136:vec3<f32> = div vec3<f32>(1.0f), %134
    %137:vec3<f32> = pow %135, %136
    %138:vec3<f32> = sub %137, %130
    %139:vec3<f32> = max %138, vec3<f32>(0.0f)
    %140:vec3<f32> = mul %132, %137
    %141:vec3<f32> = sub %131, %140
    %142:vec3<f32> = div %139, %141
    %143:vec3<f32> = div vec3<f32>(1.0f), %133
    %144:vec3<f32> = pow %142, %143
    ret %144
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
    %28:u32 = access %params, 1u
    %29:bool = eq %28, 0u
    %30:vec3<f32> = if %29 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %31:tint_TransferFunctionParams = access %params, 3u
        %32:tint_TransferFunctionParams = access %params, 4u
        %33:mat3x3<f32> = access %params, 5u
        %34:vec4<f32> = access %params, 14u
        %35:vec3<f32> = call %tint_ApplySrcTransferFunction, %27, %31
        %37:f32 = swizzle %34, w
        %38:bool = neq %37, 0.0f
        %39:vec3<f32> = if %38 [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            %40:vec3<f32> = swizzle %34, xyz
            %41:f32 = dot %40, %35
            %42:f32 = abs %41
            %43:f32 = pow %42, %37
            %44:f32 = sign %41
            %45:f32 = mul %44, %43
            %46:vec3<f32> = mul %35, %45
            exit_if %46  # if_2
          }
          $B7: {  # false
            exit_if %35  # if_2
          }
        }
        %47:vec3<f32> = mul %33, %39
        %48:vec3<f32> = call %tint_ApplyGammaTransferFunction, %47, %32
        exit_if %48  # if_1
      }
      $B5: {  # false
        exit_if %27  # if_1
      }
    }
    %50:vec4<f32> = construct %30, 1.0f
    ret %50
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %54:bool = eq %mode, 0u
    if %54 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %55:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %55
      }
      $B10: {  # false
        %56:bool = eq %mode, 1u
        if %56 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %57:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %57
          }
          $B12: {  # false
            %59:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %59
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
    %70:vec3<f32> = construct %G
    %71:vec3<f32> = construct %D
    %72:vec3<f32> = abs %v_1
    %73:vec3<f32> = sign %v_1
    %74:vec3<bool> = lt %72, %71
    %75:vec3<f32> = mul %C, %72
    %76:vec3<f32> = add %75, %F
    %77:vec3<f32> = mul %73, %76
    %78:vec3<f32> = mul %A, %72
    %79:vec3<f32> = add %78, %B
    %80:vec3<f32> = pow %79, %70
    %81:vec3<f32> = add %80, %E
    %82:vec3<f32> = mul %73, %81
    %83:vec3<f32> = select %82, %77, %74
    ret %83
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
    %93:bool = lte %v_2, %cutoff
    if %93 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %94:f32 = mul %v_2, %v_2
        %95:f32 = div %94, %lower_scale
        ret %95
      }
      $B16: {  # false
        %96:f32 = sub %v_2, %C_1
        %97:f32 = div %96, %A_1
        %98:f32 = exp %97
        %99:f32 = add %B_1, %98
        %100:f32 = div %99, %upper_scale
        ret %100
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %103:f32 = swizzle %v_3, x
    %104:f32 = call %tint_ApplyHLGSingleChannel, %103, %params_4
    %105:f32 = swizzle %v_3, y
    %106:f32 = call %tint_ApplyHLGSingleChannel, %105, %params_4
    %107:f32 = swizzle %v_3, z
    %108:f32 = call %tint_ApplyHLGSingleChannel, %107, %params_4
    %109:vec3<f32> = construct %104, %106, %108
    ret %109
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %117:vec3<f32> = construct %c1
    %118:vec3<f32> = construct %c2
    %119:vec3<f32> = construct %c3
    %120:vec3<f32> = construct %m1
    %121:vec3<f32> = construct %m2
    %122:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %123:vec3<f32> = div vec3<f32>(1.0f), %121
    %124:vec3<f32> = pow %122, %123
    %125:vec3<f32> = sub %124, %117
    %126:vec3<f32> = max %125, vec3<f32>(0.0f)
    %127:vec3<f32> = mul %119, %124
    %128:vec3<f32> = sub %118, %127
    %129:vec3<f32> = div %126, %128
    %130:vec3<f32> = div vec3<f32>(1.0f), %120
    %131:vec3<f32> = pow %129, %130
    ret %131
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
    %17:mat3x4<f32> = access %params, 2u
    %18:mat3x2<f32> = access %params, 6u
    %19:vec2<f32> = access %params, 8u
    %20:vec2<f32> = access %params, 9u
    %21:vec2<f32> = access %params, 10u
    %22:vec2<f32> = access %params, 11u
    %23:vec3<f32> = construct %coords_1, 1.0f
    %24:vec2<f32> = mul %18, %23
    %25:vec2<f32> = clamp %24, %19, %20
    %26:u32 = access %params, 0u
    %27:bool = eq %26, 1u
    %28:vec3<f32>, %29:f32 = if %27 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %30:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %25, 0.0f
        %31:vec3<f32> = swizzle %30, xyz
        %32:f32 = access %30, 3u
        exit_if %31, %32  # if_1
      }
      $B5: {  # false
        %33:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %25, 0.0f
        %34:f32 = access %33, 0u
        %35:vec2<f32> = clamp %24, %21, %22
        %36:vec4<f32> = textureSampleLevel %plane_1, %tint_sampler, %35, 0.0f
        %37:vec2<f32> = swizzle %36, xy
        %38:vec4<f32> = construct %34, %37, 1.0f
        %39:vec3<f32> = mul %38, %17
        exit_if %39, 1.0f  # if_1
      }
    }
    %40:u32 = access %params, 1u
    %41:bool = eq %40, 0u
    %42:vec3<f32> = if %41 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %43:tint_TransferFunctionParams = access %params, 3u
        %44:tint_TransferFunctionParams = access %params, 4u
        %45:mat3x3<f32> = access %params, 5u
        %46:vec4<f32> = access %params, 14u
        %47:vec3<f32> = call %tint_ApplySrcTransferFunction, %28, %43
        %49:f32 = swizzle %46, w
        %50:bool = neq %49, 0.0f
        %51:vec3<f32> = if %50 [t: $B8, f: $B9] {  # if_3
          $B8: {  # true
            %52:vec3<f32> = swizzle %46, xyz
            %53:f32 = dot %52, %47
            %54:f32 = abs %53
            %55:f32 = pow %54, %49
            %56:f32 = sign %53
            %57:f32 = mul %56, %55
            %58:vec3<f32> = mul %47, %57
            exit_if %58  # if_3
          }
          $B9: {  # false
            exit_if %47  # if_3
          }
        }
        %59:vec3<f32> = mul %45, %51
        %60:vec3<f32> = call %tint_ApplyGammaTransferFunction, %59, %44
        exit_if %60  # if_2
      }
      $B7: {  # false
        exit_if %28  # if_2
      }
    }
    %62:vec4<f32> = construct %42, %29
    ret %62
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B10: {
    %mode:u32 = access %params_1, 0u
    %66:bool = eq %mode, 0u
    if %66 [t: $B11, f: $B12] {  # if_4
      $B11: {  # true
        %67:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %67
      }
      $B12: {  # false
        %68:bool = eq %mode, 1u
        if %68 [t: $B13, f: $B14] {  # if_5
          $B13: {  # true
            %69:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %69
          }
          $B14: {  # false
            %71:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %71
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B15: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %82:vec3<f32> = construct %G
    %83:vec3<f32> = construct %D
    %84:vec3<f32> = abs %v_1
    %85:vec3<f32> = sign %v_1
    %86:vec3<bool> = lt %84, %83
    %87:vec3<f32> = mul %C, %84
    %88:vec3<f32> = add %87, %F
    %89:vec3<f32> = mul %85, %88
    %90:vec3<f32> = mul %A, %84
    %91:vec3<f32> = add %90, %B
    %92:vec3<f32> = pow %91, %82
    %93:vec3<f32> = add %92, %E
    %94:vec3<f32> = mul %85, %93
    %95:vec3<f32> = select %94, %89, %86
    ret %95
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B16: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %105:bool = lte %v_2, %cutoff
    if %105 [t: $B17, f: $B18] {  # if_6
      $B17: {  # true
        %106:f32 = mul %v_2, %v_2
        %107:f32 = div %106, %lower_scale
        ret %107
      }
      $B18: {  # false
        %108:f32 = sub %v_2, %C_1
        %109:f32 = div %108, %A_1
        %110:f32 = exp %109
        %111:f32 = add %B_1, %110
        %112:f32 = div %111, %upper_scale
        ret %112
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B19: {
    %115:f32 = swizzle %v_3, x
    %116:f32 = call %tint_ApplyHLGSingleChannel, %115, %params_4
    %117:f32 = swizzle %v_3, y
    %118:f32 = call %tint_ApplyHLGSingleChannel, %117, %params_4
    %119:f32 = swizzle %v_3, z
    %120:f32 = call %tint_ApplyHLGSingleChannel, %119, %params_4
    %121:vec3<f32> = construct %116, %118, %120
    ret %121
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B20: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %129:vec3<f32> = construct %c1
    %130:vec3<f32> = construct %c2
    %131:vec3<f32> = construct %c3
    %132:vec3<f32> = construct %m1
    %133:vec3<f32> = construct %m2
    %134:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %135:vec3<f32> = div vec3<f32>(1.0f), %133
    %136:vec3<f32> = pow %134, %135
    %137:vec3<f32> = sub %136, %129
    %138:vec3<f32> = max %137, vec3<f32>(0.0f)
    %139:vec3<f32> = mul %131, %136
    %140:vec3<f32> = sub %130, %139
    %141:vec3<f32> = div %138, %140
    %142:vec3<f32> = div vec3<f32>(1.0f), %132
    %143:vec3<f32> = pow %141, %142
    ret %143
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
    %16:mat3x4<f32> = access %params, 2u
    %17:mat3x2<f32> = access %params, 6u
    %18:vec2<f32> = access %params, 8u
    %19:vec2<f32> = access %params, 9u
    %20:vec3<f32> = construct %coords_1, 1.0f
    %21:vec2<f32> = mul %17, %20
    %22:vec2<f32> = clamp %21, %18, %19
    %23:vec4<f32> = textureSampleLevel %texture_1, %ycbcr_sampler, %22, 0.0f
    %24:vec3<f32> = swizzle %23, xyz
    %25:vec4<f32> = construct %24, 1.0f
    %26:vec3<f32> = mul %25, %16
    %27:f32 = swizzle %23, w
    %28:u32 = access %params, 1u
    %29:bool = eq %28, 0u
    %30:vec3<f32> = if %29 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %31:tint_TransferFunctionParams = access %params, 3u
        %32:tint_TransferFunctionParams = access %params, 4u
        %33:mat3x3<f32> = access %params, 5u
        %34:vec4<f32> = access %params, 14u
        %35:vec3<f32> = call %tint_ApplySrcTransferFunction, %26, %31
        %37:f32 = swizzle %34, w
        %38:bool = neq %37, 0.0f
        %39:vec3<f32> = if %38 [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            %40:vec3<f32> = swizzle %34, xyz
            %41:f32 = dot %40, %35
            %42:f32 = abs %41
            %43:f32 = pow %42, %37
            %44:f32 = sign %41
            %45:f32 = mul %44, %43
            %46:vec3<f32> = mul %35, %45
            exit_if %46  # if_2
          }
          $B7: {  # false
            exit_if %35  # if_2
          }
        }
        %47:vec3<f32> = mul %33, %39
        %48:vec3<f32> = call %tint_ApplyGammaTransferFunction, %47, %32
        exit_if %48  # if_1
      }
      $B5: {  # false
        exit_if %26  # if_1
      }
    }
    %50:vec4<f32> = construct %30, %27
    ret %50
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B8: {
    %mode:u32 = access %params_1, 0u
    %54:bool = eq %mode, 0u
    if %54 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        %55:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %55
      }
      $B10: {  # false
        %56:bool = eq %mode, 1u
        if %56 [t: $B11, f: $B12] {  # if_4
          $B11: {  # true
            %57:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %57
          }
          $B12: {  # false
            %59:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %59
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
    %70:vec3<f32> = construct %G
    %71:vec3<f32> = construct %D
    %72:vec3<f32> = abs %v_1
    %73:vec3<f32> = sign %v_1
    %74:vec3<bool> = lt %72, %71
    %75:vec3<f32> = mul %C, %72
    %76:vec3<f32> = add %75, %F
    %77:vec3<f32> = mul %73, %76
    %78:vec3<f32> = mul %A, %72
    %79:vec3<f32> = add %78, %B
    %80:vec3<f32> = pow %79, %70
    %81:vec3<f32> = add %80, %E
    %82:vec3<f32> = mul %73, %81
    %83:vec3<f32> = select %82, %77, %74
    ret %83
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
    %93:bool = lte %v_2, %cutoff
    if %93 [t: $B15, f: $B16] {  # if_5
      $B15: {  # true
        %94:f32 = mul %v_2, %v_2
        %95:f32 = div %94, %lower_scale
        ret %95
      }
      $B16: {  # false
        %96:f32 = sub %v_2, %C_1
        %97:f32 = div %96, %A_1
        %98:f32 = exp %97
        %99:f32 = add %B_1, %98
        %100:f32 = div %99, %upper_scale
        ret %100
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B17: {
    %103:f32 = swizzle %v_3, x
    %104:f32 = call %tint_ApplyHLGSingleChannel, %103, %params_4
    %105:f32 = swizzle %v_3, y
    %106:f32 = call %tint_ApplyHLGSingleChannel, %105, %params_4
    %107:f32 = swizzle %v_3, z
    %108:f32 = call %tint_ApplyHLGSingleChannel, %107, %params_4
    %109:vec3<f32> = construct %104, %106, %108
    ret %109
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B18: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %117:vec3<f32> = construct %c1
    %118:vec3<f32> = construct %c2
    %119:vec3<f32> = construct %c3
    %120:vec3<f32> = construct %m1
    %121:vec3<f32> = construct %m2
    %122:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %123:vec3<f32> = div vec3<f32>(1.0f), %121
    %124:vec3<f32> = pow %122, %123
    %125:vec3<f32> = sub %124, %117
    %126:vec3<f32> = max %125, vec3<f32>(0.0f)
    %127:vec3<f32> = mul %119, %124
    %128:vec3<f32> = sub %118, %127
    %129:vec3<f32> = div %126, %128
    %130:vec3<f32> = div vec3<f32>(1.0f), %120
    %131:vec3<f32> = pow %129, %130
    ret %131
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
    %24:mat3x4<f32> = access %params, 2u
    %25:mat3x2<f32> = access %params, 6u
    %26:vec2<f32> = access %params, 8u
    %27:vec2<f32> = access %params, 9u
    %28:vec2<f32> = access %params, 10u
    %29:vec2<f32> = access %params, 11u
    %30:vec3<f32> = construct %coords_2, 1.0f
    %31:vec2<f32> = mul %25, %30
    %32:vec2<f32> = clamp %31, %26, %27
    %33:u32 = access %params, 0u
    %34:bool = eq %33, 1u
    %35:vec3<f32>, %36:f32 = if %34 [t: $B5, f: $B6] {  # if_1
      $B5: {  # true
        %37:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %32, 0.0f
        %38:vec3<f32> = swizzle %37, xyz
        %39:f32 = access %37, 3u
        exit_if %38, %39  # if_1
      }
      $B6: {  # false
        %40:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %32, 0.0f
        %41:f32 = access %40, 0u
        %42:vec2<f32> = clamp %31, %28, %29
        %43:vec4<f32> = textureSampleLevel %plane_1, %tint_sampler, %42, 0.0f
        %44:vec2<f32> = swizzle %43, xy
        %45:vec4<f32> = construct %41, %44, 1.0f
        %46:vec3<f32> = mul %45, %24
        exit_if %46, 1.0f  # if_1
      }
    }
    %47:u32 = access %params, 1u
    %48:bool = eq %47, 0u
    %49:vec3<f32> = if %48 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %50:tint_TransferFunctionParams = access %params, 3u
        %51:tint_TransferFunctionParams = access %params, 4u
        %52:mat3x3<f32> = access %params, 5u
        %53:vec4<f32> = access %params, 14u
        %54:vec3<f32> = call %tint_ApplySrcTransferFunction, %35, %50
        %56:f32 = swizzle %53, w
        %57:bool = neq %56, 0.0f
        %58:vec3<f32> = if %57 [t: $B9, f: $B10] {  # if_3
          $B9: {  # true
            %59:vec3<f32> = swizzle %53, xyz
            %60:f32 = dot %59, %54
            %61:f32 = abs %60
            %62:f32 = pow %61, %56
            %63:f32 = sign %60
            %64:f32 = mul %63, %62
            %65:vec3<f32> = mul %54, %64
            exit_if %65  # if_3
          }
          $B10: {  # false
            exit_if %54  # if_3
          }
        }
        %66:vec3<f32> = mul %52, %58
        %67:vec3<f32> = call %tint_ApplyGammaTransferFunction, %66, %51
        exit_if %67  # if_2
      }
      $B8: {  # false
        exit_if %35  # if_2
      }
    }
    %69:vec4<f32> = construct %49, %36
    ret %69
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B11: {
    %mode:u32 = access %params_1, 0u
    %73:bool = eq %mode, 0u
    if %73 [t: $B12, f: $B13] {  # if_4
      $B12: {  # true
        %74:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %74
      }
      $B13: {  # false
        %75:bool = eq %mode, 1u
        if %75 [t: $B14, f: $B15] {  # if_5
          $B14: {  # true
            %76:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %76
          }
          $B15: {  # false
            %78:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %78
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B16: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %89:vec3<f32> = construct %G
    %90:vec3<f32> = construct %D
    %91:vec3<f32> = abs %v_1
    %92:vec3<f32> = sign %v_1
    %93:vec3<bool> = lt %91, %90
    %94:vec3<f32> = mul %C, %91
    %95:vec3<f32> = add %94, %F
    %96:vec3<f32> = mul %92, %95
    %97:vec3<f32> = mul %A, %91
    %98:vec3<f32> = add %97, %B
    %99:vec3<f32> = pow %98, %89
    %100:vec3<f32> = add %99, %E
    %101:vec3<f32> = mul %92, %100
    %102:vec3<f32> = select %101, %96, %93
    ret %102
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B17: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %112:bool = lte %v_2, %cutoff
    if %112 [t: $B18, f: $B19] {  # if_6
      $B18: {  # true
        %113:f32 = mul %v_2, %v_2
        %114:f32 = div %113, %lower_scale
        ret %114
      }
      $B19: {  # false
        %115:f32 = sub %v_2, %C_1
        %116:f32 = div %115, %A_1
        %117:f32 = exp %116
        %118:f32 = add %B_1, %117
        %119:f32 = div %118, %upper_scale
        ret %119
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B20: {
    %122:f32 = swizzle %v_3, x
    %123:f32 = call %tint_ApplyHLGSingleChannel, %122, %params_4
    %124:f32 = swizzle %v_3, y
    %125:f32 = call %tint_ApplyHLGSingleChannel, %124, %params_4
    %126:f32 = swizzle %v_3, z
    %127:f32 = call %tint_ApplyHLGSingleChannel, %126, %params_4
    %128:vec3<f32> = construct %123, %125, %127
    ret %128
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B21: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %136:vec3<f32> = construct %c1
    %137:vec3<f32> = construct %c2
    %138:vec3<f32> = construct %c3
    %139:vec3<f32> = construct %m1
    %140:vec3<f32> = construct %m2
    %141:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %142:vec3<f32> = div vec3<f32>(1.0f), %140
    %143:vec3<f32> = pow %141, %142
    %144:vec3<f32> = sub %143, %136
    %145:vec3<f32> = max %144, vec3<f32>(0.0f)
    %146:vec3<f32> = mul %138, %143
    %147:vec3<f32> = sub %137, %146
    %148:vec3<f32> = div %145, %147
    %149:vec3<f32> = div vec3<f32>(1.0f), %139
    %150:vec3<f32> = pow %148, %149
    ret %150
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
    %20:mat3x4<f32> = access %params, 2u
    %21:mat3x2<f32> = access %params, 6u
    %22:vec2<f32> = access %params, 8u
    %23:vec2<f32> = access %params, 9u
    %24:vec3<f32> = construct %coords_2, 1.0f
    %25:vec2<f32> = mul %21, %24
    %26:vec2<f32> = clamp %25, %22, %23
    %27:vec4<f32> = textureSampleLevel %texture_1, %ycbcr_sampler, %26, 0.0f
    %28:vec3<f32> = swizzle %27, xyz
    %29:vec4<f32> = construct %28, 1.0f
    %30:vec3<f32> = mul %29, %20
    %31:f32 = swizzle %27, w
    %32:u32 = access %params, 1u
    %33:bool = eq %32, 0u
    %34:vec3<f32> = if %33 [t: $B5, f: $B6] {  # if_1
      $B5: {  # true
        %35:tint_TransferFunctionParams = access %params, 3u
        %36:tint_TransferFunctionParams = access %params, 4u
        %37:mat3x3<f32> = access %params, 5u
        %38:vec4<f32> = access %params, 14u
        %39:vec3<f32> = call %tint_ApplySrcTransferFunction, %30, %35
        %41:f32 = swizzle %38, w
        %42:bool = neq %41, 0.0f
        %43:vec3<f32> = if %42 [t: $B7, f: $B8] {  # if_2
          $B7: {  # true
            %44:vec3<f32> = swizzle %38, xyz
            %45:f32 = dot %44, %39
            %46:f32 = abs %45
            %47:f32 = pow %46, %41
            %48:f32 = sign %45
            %49:f32 = mul %48, %47
            %50:vec3<f32> = mul %39, %49
            exit_if %50  # if_2
          }
          $B8: {  # false
            exit_if %39  # if_2
          }
        }
        %51:vec3<f32> = mul %37, %43
        %52:vec3<f32> = call %tint_ApplyGammaTransferFunction, %51, %36
        exit_if %52  # if_1
      }
      $B6: {  # false
        exit_if %30  # if_1
      }
    }
    %54:vec4<f32> = construct %34, %31
    ret %54
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B9: {
    %mode:u32 = access %params_1, 0u
    %58:bool = eq %mode, 0u
    if %58 [t: $B10, f: $B11] {  # if_3
      $B10: {  # true
        %59:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %59
      }
      $B11: {  # false
        %60:bool = eq %mode, 1u
        if %60 [t: $B12, f: $B13] {  # if_4
          $B12: {  # true
            %61:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %61
          }
          $B13: {  # false
            %63:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %63
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
    %74:vec3<f32> = construct %G
    %75:vec3<f32> = construct %D
    %76:vec3<f32> = abs %v_1
    %77:vec3<f32> = sign %v_1
    %78:vec3<bool> = lt %76, %75
    %79:vec3<f32> = mul %C, %76
    %80:vec3<f32> = add %79, %F
    %81:vec3<f32> = mul %77, %80
    %82:vec3<f32> = mul %A, %76
    %83:vec3<f32> = add %82, %B
    %84:vec3<f32> = pow %83, %74
    %85:vec3<f32> = add %84, %E
    %86:vec3<f32> = mul %77, %85
    %87:vec3<f32> = select %86, %81, %78
    ret %87
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
    %97:bool = lte %v_2, %cutoff
    if %97 [t: $B16, f: $B17] {  # if_5
      $B16: {  # true
        %98:f32 = mul %v_2, %v_2
        %99:f32 = div %98, %lower_scale
        ret %99
      }
      $B17: {  # false
        %100:f32 = sub %v_2, %C_1
        %101:f32 = div %100, %A_1
        %102:f32 = exp %101
        %103:f32 = add %B_1, %102
        %104:f32 = div %103, %upper_scale
        ret %104
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B18: {
    %107:f32 = swizzle %v_3, x
    %108:f32 = call %tint_ApplyHLGSingleChannel, %107, %params_4
    %109:f32 = swizzle %v_3, y
    %110:f32 = call %tint_ApplyHLGSingleChannel, %109, %params_4
    %111:f32 = swizzle %v_3, z
    %112:f32 = call %tint_ApplyHLGSingleChannel, %111, %params_4
    %113:vec3<f32> = construct %108, %110, %112
    ret %113
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B19: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %121:vec3<f32> = construct %c1
    %122:vec3<f32> = construct %c2
    %123:vec3<f32> = construct %c3
    %124:vec3<f32> = construct %m1
    %125:vec3<f32> = construct %m2
    %126:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %127:vec3<f32> = div vec3<f32>(1.0f), %125
    %128:vec3<f32> = pow %126, %127
    %129:vec3<f32> = sub %128, %121
    %130:vec3<f32> = max %129, vec3<f32>(0.0f)
    %131:vec3<f32> = mul %123, %128
    %132:vec3<f32> = sub %122, %131
    %133:vec3<f32> = div %130, %132
    %134:vec3<f32> = div vec3<f32>(1.0f), %124
    %135:vec3<f32> = pow %133, %134
    ret %135
  }
}
)";

    EXPECT_EQ(src, str());

    Run(DirectVariableAccess,
        DirectVariableAccessConfig{.transform_handle = HandleTransformLevel::kExternal});

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
    %39:mat3x4<f32> = access %params, 2u
    %40:mat3x2<f32> = access %params, 6u
    %41:vec2<f32> = access %params, 8u
    %42:vec2<f32> = access %params, 9u
    %43:vec2<f32> = access %params, 10u
    %44:vec2<f32> = access %params, 11u
    %45:vec3<f32> = construct %coords_2, 1.0f
    %46:vec2<f32> = mul %40, %45
    %47:vec2<f32> = clamp %46, %41, %42
    %48:u32 = access %params, 0u
    %49:bool = eq %48, 1u
    %50:vec3<f32>, %51:f32 = if %49 [t: $B5, f: $B6] {  # if_1
      $B5: {  # true
        %52:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %47, 0.0f
        %53:vec3<f32> = swizzle %52, xyz
        %54:f32 = access %52, 3u
        exit_if %53, %54  # if_1
      }
      $B6: {  # false
        %55:vec4<f32> = textureSampleLevel %plane_0, %tint_sampler, %47, 0.0f
        %56:f32 = access %55, 0u
        %57:vec2<f32> = clamp %46, %43, %44
        %58:vec4<f32> = textureSampleLevel %plane_1, %tint_sampler, %57, 0.0f
        %59:vec2<f32> = swizzle %58, xy
        %60:vec4<f32> = construct %56, %59, 1.0f
        %61:vec3<f32> = mul %60, %39
        exit_if %61, 1.0f  # if_1
      }
    }
    %62:u32 = access %params, 1u
    %63:bool = eq %62, 0u
    %64:vec3<f32> = if %63 [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %65:tint_TransferFunctionParams = access %params, 3u
        %66:tint_TransferFunctionParams = access %params, 4u
        %67:mat3x3<f32> = access %params, 5u
        %68:vec4<f32> = access %params, 14u
        %69:vec3<f32> = call %tint_ApplySrcTransferFunction, %50, %65
        %71:f32 = swizzle %68, w
        %72:bool = neq %71, 0.0f
        %73:vec3<f32> = if %72 [t: $B9, f: $B10] {  # if_3
          $B9: {  # true
            %74:vec3<f32> = swizzle %68, xyz
            %75:f32 = dot %74, %69
            %76:f32 = abs %75
            %77:f32 = pow %76, %71
            %78:f32 = sign %75
            %79:f32 = mul %78, %77
            %80:vec3<f32> = mul %69, %79
            exit_if %80  # if_3
          }
          $B10: {  # false
            exit_if %69  # if_3
          }
        }
        %81:vec3<f32> = mul %67, %73
        %82:vec3<f32> = call %tint_ApplyGammaTransferFunction, %81, %66
        exit_if %82  # if_2
      }
      $B8: {  # false
        exit_if %50  # if_2
      }
    }
    %84:vec4<f32> = construct %64, %51
    ret %84
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B11: {
    %mode:u32 = access %params_1, 0u
    %88:bool = eq %mode, 0u
    if %88 [t: $B12, f: $B13] {  # if_4
      $B12: {  # true
        %89:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %89
      }
      $B13: {  # false
        %90:bool = eq %mode, 1u
        if %90 [t: $B14, f: $B15] {  # if_5
          $B14: {  # true
            %91:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %91
          }
          $B15: {  # false
            %93:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %93
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B16: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %104:vec3<f32> = construct %G
    %105:vec3<f32> = construct %D
    %106:vec3<f32> = abs %v_1
    %107:vec3<f32> = sign %v_1
    %108:vec3<bool> = lt %106, %105
    %109:vec3<f32> = mul %C, %106
    %110:vec3<f32> = add %109, %F
    %111:vec3<f32> = mul %107, %110
    %112:vec3<f32> = mul %A, %106
    %113:vec3<f32> = add %112, %B
    %114:vec3<f32> = pow %113, %104
    %115:vec3<f32> = add %114, %E
    %116:vec3<f32> = mul %107, %115
    %117:vec3<f32> = select %116, %111, %108
    ret %117
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B17: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %127:bool = lte %v_2, %cutoff
    if %127 [t: $B18, f: $B19] {  # if_6
      $B18: {  # true
        %128:f32 = mul %v_2, %v_2
        %129:f32 = div %128, %lower_scale
        ret %129
      }
      $B19: {  # false
        %130:f32 = sub %v_2, %C_1
        %131:f32 = div %130, %A_1
        %132:f32 = exp %131
        %133:f32 = add %B_1, %132
        %134:f32 = div %133, %upper_scale
        ret %134
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B20: {
    %137:f32 = swizzle %v_3, x
    %138:f32 = call %tint_ApplyHLGSingleChannel, %137, %params_4
    %139:f32 = swizzle %v_3, y
    %140:f32 = call %tint_ApplyHLGSingleChannel, %139, %params_4
    %141:f32 = swizzle %v_3, z
    %142:f32 = call %tint_ApplyHLGSingleChannel, %141, %params_4
    %143:vec3<f32> = construct %138, %140, %142
    ret %143
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B21: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %151:vec3<f32> = construct %c1
    %152:vec3<f32> = construct %c2
    %153:vec3<f32> = construct %c3
    %154:vec3<f32> = construct %m1
    %155:vec3<f32> = construct %m2
    %156:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %157:vec3<f32> = div vec3<f32>(1.0f), %155
    %158:vec3<f32> = pow %156, %157
    %159:vec3<f32> = sub %158, %151
    %160:vec3<f32> = max %159, vec3<f32>(0.0f)
    %161:vec3<f32> = mul %153, %158
    %162:vec3<f32> = sub %152, %161
    %163:vec3<f32> = div %160, %162
    %164:vec3<f32> = div vec3<f32>(1.0f), %154
    %165:vec3<f32> = pow %163, %164
    ret %165
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
    %29:mat3x4<f32> = access %params, 2u
    %30:mat3x2<f32> = access %params, 7u
    %31:vec2<u32> = access %params, 12u
    %32:vec2<f32> = access %params, 13u
    %33:vec2<u32> = min %coords_1, %31
    %34:vec2<f32> = convert %33
    %35:vec3<f32> = construct %34, 1.0f
    %36:vec2<f32> = mul %30, %35
    %37:vec2<f32> = round %36
    %38:vec2<u32> = convert %37
    %39:u32 = access %params, 0u
    %40:bool = eq %39, 1u
    %41:vec3<f32>, %42:f32 = if %40 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %43:vec4<f32> = textureLoad %plane_0, %38, 0u
        %44:vec3<f32> = swizzle %43, xyz
        %45:f32 = access %43, 3u
        exit_if %44, %45  # if_1
      }
      $B5: {  # false
        %46:vec4<f32> = textureLoad %plane_0, %38, 0u
        %47:f32 = access %46, 0u
        %48:vec2<f32> = mul %37, %32
        %49:vec2<u32> = convert %48
        %50:vec4<f32> = textureLoad %plane_1, %49, 0u
        %51:vec2<f32> = swizzle %50, xy
        %52:vec4<f32> = construct %47, %51, 1.0f
        %53:vec3<f32> = mul %52, %29
        exit_if %53, 1.0f  # if_1
      }
    }
    %54:u32 = access %params, 1u
    %55:bool = eq %54, 0u
    %56:vec3<f32> = if %55 [t: $B6, f: $B7] {  # if_2
      $B6: {  # true
        %57:tint_TransferFunctionParams = access %params, 3u
        %58:tint_TransferFunctionParams = access %params, 4u
        %59:mat3x3<f32> = access %params, 5u
        %60:vec4<f32> = access %params, 14u
        %61:vec3<f32> = call %tint_ApplySrcTransferFunction, %41, %57
        %63:f32 = swizzle %60, w
        %64:bool = neq %63, 0.0f
        %65:vec3<f32> = if %64 [t: $B8, f: $B9] {  # if_3
          $B8: {  # true
            %66:vec3<f32> = swizzle %60, xyz
            %67:f32 = dot %66, %61
            %68:f32 = abs %67
            %69:f32 = pow %68, %63
            %70:f32 = sign %67
            %71:f32 = mul %70, %69
            %72:vec3<f32> = mul %61, %71
            exit_if %72  # if_3
          }
          $B9: {  # false
            exit_if %61  # if_3
          }
        }
        %73:vec3<f32> = mul %59, %65
        %74:vec3<f32> = call %tint_ApplyGammaTransferFunction, %73, %58
        exit_if %74  # if_2
      }
      $B7: {  # false
        exit_if %41  # if_2
      }
    }
    %76:vec4<f32> = construct %56, %42
    ret %76
  }
}
%tint_ApplySrcTransferFunction = func(%v:vec3<f32>, %params_1:tint_TransferFunctionParams):vec3<f32> {  # %params_1: 'params'
  $B10: {
    %mode:u32 = access %params_1, 0u
    %80:bool = eq %mode, 0u
    if %80 [t: $B11, f: $B12] {  # if_4
      $B11: {  # true
        %81:vec3<f32> = call %tint_ApplyGammaTransferFunction, %v, %params_1
        ret %81
      }
      $B12: {  # false
        %82:bool = eq %mode, 1u
        if %82 [t: $B13, f: $B14] {  # if_5
          $B13: {  # true
            %83:vec3<f32> = call %tint_ApplyHLGTransferFunction, %v, %params_1
            ret %83
          }
          $B14: {  # false
            %85:vec3<f32> = call %tint_ApplyPQTransferFunction, %v, %params_1
            ret %85
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
%tint_ApplyGammaTransferFunction = func(%v_1:vec3<f32>, %params_2:tint_TransferFunctionParams):vec3<f32> {  # %v_1: 'v', %params_2: 'params'
  $B15: {
    %A:f32 = access %params_2, 1u
    %B:f32 = access %params_2, 2u
    %C:f32 = access %params_2, 3u
    %D:f32 = access %params_2, 4u
    %E:f32 = access %params_2, 5u
    %F:f32 = access %params_2, 6u
    %G:f32 = access %params_2, 7u
    %96:vec3<f32> = construct %G
    %97:vec3<f32> = construct %D
    %98:vec3<f32> = abs %v_1
    %99:vec3<f32> = sign %v_1
    %100:vec3<bool> = lt %98, %97
    %101:vec3<f32> = mul %C, %98
    %102:vec3<f32> = add %101, %F
    %103:vec3<f32> = mul %99, %102
    %104:vec3<f32> = mul %A, %98
    %105:vec3<f32> = add %104, %B
    %106:vec3<f32> = pow %105, %96
    %107:vec3<f32> = add %106, %E
    %108:vec3<f32> = mul %99, %107
    %109:vec3<f32> = select %108, %103, %100
    ret %109
  }
}
%tint_ApplyHLGSingleChannel = func(%v_2:f32, %params_3:tint_TransferFunctionParams):f32 {  # %v_2: 'v', %params_3: 'params'
  $B16: {
    %A_1:f32 = access %params_3, 1u  # %A_1: 'A'
    %B_1:f32 = access %params_3, 2u  # %B_1: 'B'
    %C_1:f32 = access %params_3, 3u  # %C_1: 'C'
    %cutoff:f32 = access %params_3, 4u
    %lower_scale:f32 = access %params_3, 5u
    %upper_scale:f32 = access %params_3, 6u
    %119:bool = lte %v_2, %cutoff
    if %119 [t: $B17, f: $B18] {  # if_6
      $B17: {  # true
        %120:f32 = mul %v_2, %v_2
        %121:f32 = div %120, %lower_scale
        ret %121
      }
      $B18: {  # false
        %122:f32 = sub %v_2, %C_1
        %123:f32 = div %122, %A_1
        %124:f32 = exp %123
        %125:f32 = add %B_1, %124
        %126:f32 = div %125, %upper_scale
        ret %126
      }
    }
    unreachable
  }
}
%tint_ApplyHLGTransferFunction = func(%v_3:vec3<f32>, %params_4:tint_TransferFunctionParams):vec3<f32> {  # %v_3: 'v', %params_4: 'params'
  $B19: {
    %129:f32 = swizzle %v_3, x
    %130:f32 = call %tint_ApplyHLGSingleChannel, %129, %params_4
    %131:f32 = swizzle %v_3, y
    %132:f32 = call %tint_ApplyHLGSingleChannel, %131, %params_4
    %133:f32 = swizzle %v_3, z
    %134:f32 = call %tint_ApplyHLGSingleChannel, %133, %params_4
    %135:vec3<f32> = construct %130, %132, %134
    ret %135
  }
}
%tint_ApplyPQTransferFunction = func(%v_4:vec3<f32>, %params_5:tint_TransferFunctionParams):vec3<f32> {  # %v_4: 'v', %params_5: 'params'
  $B20: {
    %m1:f32 = access %params_5, 1u
    %m2:f32 = access %params_5, 2u
    %c1:f32 = access %params_5, 3u
    %c2:f32 = access %params_5, 4u
    %c3:f32 = access %params_5, 5u
    %143:vec3<f32> = construct %c1
    %144:vec3<f32> = construct %c2
    %145:vec3<f32> = construct %c3
    %146:vec3<f32> = construct %m1
    %147:vec3<f32> = construct %m2
    %148:vec3<f32> = clamp %v_4, vec3<f32>(0.0f), vec3<f32>(1.0f)
    %149:vec3<f32> = div vec3<f32>(1.0f), %147
    %150:vec3<f32> = pow %148, %149
    %151:vec3<f32> = sub %150, %143
    %152:vec3<f32> = max %151, vec3<f32>(0.0f)
    %153:vec3<f32> = mul %145, %150
    %154:vec3<f32> = sub %144, %153
    %155:vec3<f32> = div %152, %154
    %156:vec3<f32> = div vec3<f32>(1.0f), %146
    %157:vec3<f32> = pow %155, %156
    ret %157
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
