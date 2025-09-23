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

#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_Bgra8UnormPolyfillTest = TransformTest;

TEST_F(IR_Bgra8UnormPolyfillTest, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";

    Run(Bgra8UnormPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, NoModify_ModuleScopeVariable_Rgba) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({value, coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load, coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%value:vec4<f32>, %coords:vec2<u32>):void {
  $B2: {
    %5:texture_storage_2d<rgba8unorm, write> = load %texture
    %6:void = textureStore %5, %coords, %value
    ret
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, NoModify_UserFunctionParameter_Rgba) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* func = b.Function("foo", ty.void_());
    auto* texture = b.FunctionParam("texture", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({texture, coords, value});
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%texture:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %texture, %coords, %value
    ret
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, ModuleScopeVariable) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({value, coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load, coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%value:vec4<f32>, %coords:vec2<u32>):void {
  $B2: {
    %5:texture_storage_2d<bgra8unorm, write> = load %texture
    %6:void = textureStore %5, %coords, %value
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%value:vec4<f32>, %coords:vec2<u32>):void {
  $B2: {
    %5:texture_storage_2d<rgba8unorm, write> = load %texture
    %6:vec4<f32> = swizzle %value, zyxw
    %7:void = textureStore %5, %coords, %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, UserFunctionParameter) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* func = b.Function("foo", ty.void_());
    auto* texture = b.FunctionParam("texture", texture_ty);
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({texture, coords, value});
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%texture:texture_storage_2d<bgra8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {
  $B1: {
    %5:void = textureStore %texture, %coords, %value
    ret
  }
}
)";
    auto* expect = R"(
%foo = func(%texture:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {
  $B1: {
    %5:vec4<f32> = swizzle %value, zyxw
    %6:void = textureStore %texture, %coords, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, ModuleScopePassedToUserFunction) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* bar = b.Function("bar", ty.void_());
    {
        auto* texture = b.FunctionParam("texture", texture_ty);
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        bar->SetParams({texture, coords, value});
        b.Append(bar->Block(), [&] {
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
            b.Return(bar);
        });
    }

    auto* foo = b.Function("foo", ty.void_());
    {
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        foo->SetParams({coords, value});
        b.Append(foo->Block(), [&] {
            auto* load = b.Load(var->Result());
            b.Call(ty.void_(), bar, load, coords, value);
            b.Return(foo);
        });
    }

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 2)
}

%bar = func(%texture_1:texture_storage_2d<bgra8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {  # %texture_1: 'texture'
  $B2: {
    %6:void = textureStore %texture_1, %coords, %value
    ret
  }
}
%foo = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void {  # %coords_1: 'coords', %value_1: 'value'
  $B3: {
    %10:texture_storage_2d<bgra8unorm, write> = load %texture
    %11:void = call %bar, %10, %coords_1, %value_1
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
}

%bar = func(%texture_1:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {  # %texture_1: 'texture'
  $B2: {
    %6:vec4<f32> = swizzle %value, zyxw
    %7:void = textureStore %texture_1, %coords, %6
    ret
  }
}
%foo = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void {  # %coords_1: 'coords', %value_1: 'value'
  $B3: {
    %11:texture_storage_2d<rgba8unorm, write> = load %texture
    %12:void = call %bar, %11, %coords_1, %value_1
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, ModuleScopePassedToUserFunction_MultipleTextures) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* var_a = b.Var("texture_a", ty.ptr(handle, texture_ty));
    auto* var_b = b.Var("texture_b", ty.ptr(handle, texture_ty));
    auto* var_c = b.Var("texture_c", ty.ptr(handle, texture_ty));
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(1, 3);
    var_c->SetBindingPoint(1, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);
    mod.root_block->Append(var_c);

    auto* bar = b.Function("bar", ty.void_());
    {
        auto* texture_a = b.FunctionParam("texture_a", texture_ty);
        auto* texture_b = b.FunctionParam("texture_b", texture_ty);
        auto* texture_c = b.FunctionParam("texture_b", texture_ty);
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        bar->SetParams({texture_a, texture_b, texture_c, coords, value});
        b.Append(bar->Block(), [&] {
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture_a, coords, value);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture_b, coords, value);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture_c, coords, value);
            b.Return(bar);
        });
    }

    auto* foo = b.Function("foo", ty.void_());
    {
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        foo->SetParams({coords, value});
        b.Append(foo->Block(), [&] {
            auto* load_a = b.Load(var_a->Result());
            auto* load_b = b.Load(var_b->Result());
            auto* load_c = b.Load(var_c->Result());
            b.Call(ty.void_(), bar, load_a, load_b, load_c, coords, value);
            b.Return(foo);
        });
    }

    auto* src = R"(
$B1: {  # root
  %texture_a:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 2)
  %texture_b:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 3)
  %texture_c:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 4)
}

%bar = func(%texture_a_1:texture_storage_2d<bgra8unorm, write>, %texture_b_1:texture_storage_2d<bgra8unorm, write>, %texture_b_2:texture_storage_2d<bgra8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {  # %texture_a_1: 'texture_a', %texture_b_1: 'texture_b', %texture_b_2: 'texture_b'
  $B2: {
    %10:void = textureStore %texture_a_1, %coords, %value
    %11:void = textureStore %texture_b_1, %coords, %value
    %12:void = textureStore %texture_b_2, %coords, %value
    ret
  }
}
%foo = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void {  # %coords_1: 'coords', %value_1: 'value'
  $B3: {
    %16:texture_storage_2d<bgra8unorm, write> = load %texture_a
    %17:texture_storage_2d<bgra8unorm, write> = load %texture_b
    %18:texture_storage_2d<bgra8unorm, write> = load %texture_c
    %19:void = call %bar, %16, %17, %18, %coords_1, %value_1
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture_a:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
  %texture_b:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 3)
  %texture_c:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 4)
}

%bar = func(%texture_a_1:texture_storage_2d<rgba8unorm, write>, %texture_b_1:texture_storage_2d<rgba8unorm, write>, %texture_b_2:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {  # %texture_a_1: 'texture_a', %texture_b_1: 'texture_b', %texture_b_2: 'texture_b'
  $B2: {
    %10:vec4<f32> = swizzle %value, zyxw
    %11:void = textureStore %texture_a_1, %coords, %10
    %12:vec4<f32> = swizzle %value, zyxw
    %13:void = textureStore %texture_b_1, %coords, %12
    %14:vec4<f32> = swizzle %value, zyxw
    %15:void = textureStore %texture_b_2, %coords, %14
    ret
  }
}
%foo = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void {  # %coords_1: 'coords', %value_1: 'value'
  $B3: {
    %19:texture_storage_2d<rgba8unorm, write> = load %texture_a
    %20:texture_storage_2d<rgba8unorm, write> = load %texture_b
    %21:texture_storage_2d<rgba8unorm, write> = load %texture_c
    %22:void = call %bar, %19, %20, %21, %coords_1, %value_1
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, MutipleUsesOfOneTexture) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* var_a = b.Var("texture", ty.ptr(handle, texture_ty));
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* bar = b.Function("bar", ty.void_());
    {
        auto* texture = b.FunctionParam("texture", texture_ty);
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        bar->SetParams({texture, coords, value});
        b.Append(bar->Block(), [&] {
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, texture, coords, value);
            b.Return(bar);
        });
    }

    auto* foo = b.Function("foo", ty.void_());
    {
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        foo->SetParams({coords, value});
        b.Append(foo->Block(), [&] {
            auto* load_a = b.Load(var_a->Result());
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load_a, coords, value);
            auto* load_b = b.Load(var_a->Result());
            b.Call(ty.void_(), bar, load_b, coords, value);
            auto* load_c = b.Load(var_a->Result());
            b.Call(ty.void_(), bar, load_c, coords, value);
            b.Return(foo);
        });
    }

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 2)
}

%bar = func(%texture_1:texture_storage_2d<bgra8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {  # %texture_1: 'texture'
  $B2: {
    %6:void = textureStore %texture_1, %coords, %value
    %7:void = textureStore %texture_1, %coords, %value
    %8:void = textureStore %texture_1, %coords, %value
    ret
  }
}
%foo = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void {  # %coords_1: 'coords', %value_1: 'value'
  $B3: {
    %12:texture_storage_2d<bgra8unorm, write> = load %texture
    %13:void = textureStore %12, %coords_1, %value_1
    %14:texture_storage_2d<bgra8unorm, write> = load %texture
    %15:void = call %bar, %14, %coords_1, %value_1
    %16:texture_storage_2d<bgra8unorm, write> = load %texture
    %17:void = call %bar, %16, %coords_1, %value_1
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
}

%bar = func(%texture_1:texture_storage_2d<rgba8unorm, write>, %coords:vec2<u32>, %value:vec4<f32>):void {  # %texture_1: 'texture'
  $B2: {
    %6:vec4<f32> = swizzle %value, zyxw
    %7:void = textureStore %texture_1, %coords, %6
    %8:vec4<f32> = swizzle %value, zyxw
    %9:void = textureStore %texture_1, %coords, %8
    %10:vec4<f32> = swizzle %value, zyxw
    %11:void = textureStore %texture_1, %coords, %10
    ret
  }
}
%foo = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void {  # %coords_1: 'coords', %value_1: 'value'
  $B3: {
    %15:texture_storage_2d<rgba8unorm, write> = load %texture
    %16:vec4<f32> = swizzle %value_1, zyxw
    %17:void = textureStore %15, %coords_1, %16
    %18:texture_storage_2d<rgba8unorm, write> = load %texture
    %19:void = call %bar, %18, %coords_1, %value_1
    %20:texture_storage_2d<rgba8unorm, write> = load %texture
    %21:void = call %bar, %20, %coords_1, %value_1
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, ArrayedImage) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2dArray, format, write);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    auto* index = b.FunctionParam("index", ty.u32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({coords, index, value});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load, coords, index, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d_array<bgra8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>, %index:u32, %value:vec4<f32>):void {
  $B2: {
    %6:texture_storage_2d_array<bgra8unorm, write> = load %texture
    %7:void = textureStore %6, %coords, %index, %value
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d_array<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>, %index:u32, %value:vec4<f32>):void {
  $B2: {
    %6:texture_storage_2d_array<rgba8unorm, write> = load %texture
    %7:vec4<f32> = swizzle %value, zyxw
    %8:void = textureStore %6, %coords, %index, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, TextureDimensions) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, write);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* dims = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, load);
        b.Return(func, dims);
        mod.SetName(dims, "dims");
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<bgra8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_storage_2d<bgra8unorm, write> = load %texture
    %dims:vec2<u32> = textureDimensions %3
    ret %dims
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(1, 2)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:texture_storage_2d<rgba8unorm, write> = load %texture
    %dims:vec2<u32> = textureDimensions %3
    ret %dims
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, TextureLoad) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, read);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
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
  %texture:ptr<handle, texture_storage_2d<bgra8unorm, read>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>):vec4<f32> {
  $B2: {
    %4:texture_storage_2d<bgra8unorm, read> = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, read>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>):vec4<f32> {
  $B2: {
    %4:texture_storage_2d<rgba8unorm, read> = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    %6:vec4<f32> = swizzle %result, zyxw
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, TextureLoadAndStore) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* texture_ty = ty.storage_texture(core::type::TextureDimension::k2d, format, read_write);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coords);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load, coords, result);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<bgra8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>):void {
  $B2: {
    %4:texture_storage_2d<bgra8unorm, read_write> = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    %6:void = textureStore %4, %coords, %result
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>):void {
  $B2: {
    %4:texture_storage_2d<rgba8unorm, read_write> = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    %6:vec4<f32> = swizzle %result, zyxw
    %7:vec4<f32> = swizzle %6, zyxw
    %8:void = textureStore %4, %coords, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, TexelBuffer_TextureLoad) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* buffer_ty = ty.texel_buffer(format, core::Access::kRead);

    auto* var = b.Var("buffer", ty.ptr(handle, buffer_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* coord = b.FunctionParam("coord", ty.u32());
    func->SetParams({coord});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coord);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<handle, texel_buffer<bgra8unorm, read>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coord:u32):vec4<f32> {
  $B2: {
    %4:texel_buffer<bgra8unorm, read> = load %buffer
    %result:vec4<f32> = textureLoad %4, %coord
    ret %result
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<handle, texel_buffer<rgba8unorm, read>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coord:u32):vec4<f32> {
  $B2: {
    %4:texel_buffer<rgba8unorm, read> = load %buffer
    %result:vec4<f32> = textureLoad %4, %coord
    %6:vec4<f32> = swizzle %result, zyxw
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, TexelBuffer_TextureStore) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* buffer_ty = ty.texel_buffer(format, core::Access::kReadWrite);

    auto* var = b.Var("buffer", ty.ptr(handle, buffer_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* coord = b.FunctionParam("coord", ty.u32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({coord, value});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load, coord, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<handle, texel_buffer<bgra8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coord:u32, %value:vec4<f32>):void {
  $B2: {
    %5:texel_buffer<bgra8unorm, read_write> = load %buffer
    %6:void = textureStore %5, %coord, %value
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<handle, texel_buffer<rgba8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coord:u32, %value:vec4<f32>):void {
  $B2: {
    %5:texel_buffer<rgba8unorm, read_write> = load %buffer
    %6:vec4<f32> = swizzle %value, zyxw
    %7:void = textureStore %5, %coord, %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_Bgra8UnormPolyfillTest, TexelBuffer_TextureLoadAndStore) {
    auto format = core::TexelFormat::kBgra8Unorm;
    auto* buffer_ty = ty.texel_buffer(format, core::Access::kReadWrite);

    auto* var = b.Var("buffer", ty.ptr(handle, buffer_ty));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* coord = b.FunctionParam("coord", ty.u32());
    auto* value = b.FunctionParam("value", ty.vec4<f32>());
    func->SetParams({coord, value});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coord);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, load, coord, result);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<handle, texel_buffer<bgra8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coord:u32, %value:vec4<f32>):void {
  $B2: {
    %5:texel_buffer<bgra8unorm, read_write> = load %buffer
    %result:vec4<f32> = textureLoad %5, %coord
    %7:void = textureStore %5, %coord, %result
    ret
  }
}
)";
    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<handle, texel_buffer<rgba8unorm, read_write>, read> = var undef @binding_point(1, 2)
}

%foo = func(%coord:u32, %value:vec4<f32>):void {
  $B2: {
    %5:texel_buffer<rgba8unorm, read_write> = load %buffer
    %result:vec4<f32> = textureLoad %5, %coord
    %7:vec4<f32> = swizzle %result, zyxw
    %8:vec4<f32> = swizzle %7, zyxw
    %9:void = textureStore %5, %coord, %8
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(Bgra8UnormPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
