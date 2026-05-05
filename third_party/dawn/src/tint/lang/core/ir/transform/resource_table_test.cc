// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/resource_table.h"

#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/resource_table.h"
#include "src/tint/lang/core/type/resource_type.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

class Helper : public ResourceTableHelper {
  public:
    ~Helper() override = default;

    // Returns a map of types to the var which is used to access the memory of that type
    Hashmap<const core::type::Type*, core::ir::Var*, 4> GenerateVars(
        core::ir::Builder& b,
        const BindingPoint& bp,
        const std::vector<ResourceType>& types) const override {
        Hashmap<const core::type::Type*, core::ir::Var*, 4> res;

        for (auto& type : types) {
            auto* t = core::type::ResourceTypeToType(b.ir.Types(), type);
            auto* ty = b.ir.Types().Get<core::type::ResourceTable>(t);

            auto* v = b.Var(b.ir.Types().ptr(handle, ty));
            v->SetBindingPoint(bp.group, bp.binding);
            res.Add(t, v);
        }

        return res;
    }
};

using IR_ResourceTableTest = TransformTest;

TEST_F(IR_ResourceTableTest, NoResources) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture_ty =
        ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kWrite);

    auto* var = b.Var("texture", ty.ptr(handle, texture_ty));
    var->SetBindingPoint(3, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = src;

    EXPECT_EQ(src, str());

    Helper helper;
    Run(ResourceTable,
        ResourceTableConfig{
            .resource_table_binding = {0, 1},
            .storage_buffer_binding = {1, 2},
            .default_binding_type_order = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, MissingConfig) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t",
              b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource, Vector{texture_ty}, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:bool = hasResource<texture_2d_array<u32>> 1u
    %t:bool = let %2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Helper helper;
    auto result = RunWithFailure(ResourceTable, std::nullopt, &helper);
    EXPECT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, "hasResource and getResource require a resource table");
}

TEST_F(IR_ResourceTableTest, HasResource) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t",
              b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource, Vector{texture_ty}, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:bool = hasResource<texture_2d_array<u32>> 1u
    %t:bool = let %2
    ret
  }
}
)";

    auto* expect = R"(
tint_resource_table_metadata_struct = struct @align(4) {
  array_length:u32 @offset(0)
  bindings:array<u32> @offset(4)
}

$B1: {  # root
  %1:ptr<handle, resource_table<texture_1d<f32, filterable>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %6:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %6
    %8:bool = lt 1u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:bool = eq %11, 12u
        exit_if %12  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %t:bool = let %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Helper helper;
    Run(ResourceTable,
        ResourceTableConfig{
            .resource_table_binding = {0, 1},
            .storage_buffer_binding = {1, 2},
            .default_binding_type_order =
                {
                    ResourceType::kTexture1d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, HasResource_UnfilterableFromFilterable) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32(),
                                          core::TextureFilterable::kUnfilterable);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t",
              b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource, Vector{texture_ty}, 2_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:bool = hasResource<texture_2d<f32, unfilterable>> 2u
    %t:bool = let %2
    ret
  }
}
)";

    auto* expect = R"(
tint_resource_table_metadata_struct = struct @align(4) {
  array_length:u32 @offset(0)
  bindings:array<u32> @offset(4)
}

$B1: {  # root
  %1:ptr<handle, resource_table<texture_1d<f32, filterable>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d<f32, filterable>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %6:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %6
    %8:bool = lt 2u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %11:u32 = load %10
        %12:vec3<u32> = construct %11
        %13:vec3<u32> = construct 6u, 5u, 28u
        %14:vec3<bool> = eq %12, %13
        %15:bool = any %14
        exit_if %15  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %t:bool = let %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Helper helper;
    Run(ResourceTable,
        ResourceTableConfig{
            .resource_table_binding = {0, 1},
            .storage_buffer_binding = {1, 2},
            .default_binding_type_order =
                {
                    ResourceType::kTexture1d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2d_f32_filterable,
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource, Vector{texture_ty}, 1_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d_array<u32> = getResource<texture_2d_array<u32>> 1u
    ret
  }
}
)";

    auto* expect = R"(
tint_resource_table_metadata_struct = struct @align(4) {
  array_length:u32 @offset(0)
  bindings:array<u32> @offset(4)
}

$B1: {  # root
  %1:ptr<handle, resource_table<texture_1d<f32, filterable>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %6:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %6
    %8:bool = lt 1u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:bool = eq %11, 12u
        exit_if %12  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %13:u32 = if %9 [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        exit_if 1u  # if_2
      }
      $B6: {  # false
        %14:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %15:u32 = load %14
        %16:u32 = add 2u, %15
        exit_if %16  # if_2
      }
    }
    %17:ptr<handle, texture_2d_array<u32>, read> = access %3, %13
    %18:texture_2d_array<u32> = load %17
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Helper helper;
    Run(ResourceTable,
        ResourceTableConfig{
            .resource_table_binding = {0, 1},
            .storage_buffer_binding = {1, 2},
            .default_binding_type_order =
                {
                    ResourceType::kTexture1d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_UnfilterableFromFilterable_2d_WithDepth) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32(),
                                          core::TextureFilterable::kUnfilterable);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource, Vector{texture_ty}, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d<f32, unfilterable> = getResource<texture_2d<f32, unfilterable>> 2u
    ret
  }
}
)";

    auto* expect = R"(
tint_resource_table_metadata_struct = struct @align(4) {
  array_length:u32 @offset(0)
  bindings:array<u32> @offset(4)
}

$B1: {  # root
  %1:ptr<handle, resource_table<texture_1d<f32, filterable>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d<f32, unfilterable>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %6:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %6
    %8:bool = lt 2u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %11:u32 = load %10
        %12:vec3<u32> = construct %11
        %13:vec3<u32> = construct 6u, 5u, 28u
        %14:vec3<bool> = eq %12, %13
        %15:bool = any %14
        exit_if %15  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %16:u32 = if %9 [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        exit_if 2u  # if_2
      }
      $B6: {  # false
        %17:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %18:u32 = load %17
        %19:u32 = add 2u, %18
        exit_if %19  # if_2
      }
    }
    %20:ptr<handle, texture_2d<f32, unfilterable>, read> = access %3, %16
    %21:texture_2d<f32, unfilterable> = load %20
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Helper helper;
    Run(ResourceTable,
        ResourceTableConfig{
            .resource_table_binding = {0, 1},
            .storage_buffer_binding = {1, 2},
            .default_binding_type_order =
                {
                    ResourceType::kTexture1d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2d_f32_unfilterable,
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_UnfilterableFromFilterable_1d_NoDepth) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32(),
                                          core::TextureFilterable::kUnfilterable);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource, Vector{texture_ty}, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_1d<f32, unfilterable> = getResource<texture_1d<f32, unfilterable>> 2u
    ret
  }
}
)";

    auto* expect = R"(
tint_resource_table_metadata_struct = struct @align(4) {
  array_length:u32 @offset(0)
  bindings:array<u32> @offset(4)
}

$B1: {  # root
  %1:ptr<handle, resource_table<texture_1d<f32, unfilterable>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d<f32, unfilterable>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %6:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %6
    %8:bool = lt 2u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %11:u32 = load %10
        %12:vec2<u32> = construct %11
        %13:vec2<u32> = construct 2u, 1u
        %14:vec2<bool> = eq %12, %13
        %15:bool = any %14
        exit_if %15  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %16:u32 = if %9 [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        exit_if 2u  # if_2
      }
      $B6: {  # false
        %17:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %18:u32 = load %17
        %19:u32 = add 0u, %18
        exit_if %19  # if_2
      }
    }
    %20:ptr<handle, texture_1d<f32, unfilterable>, read> = access %1, %16
    %21:texture_1d<f32, unfilterable> = load %20
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Helper helper;
    Run(ResourceTable,
        ResourceTableConfig{
            .resource_table_binding = {0, 1},
            .storage_buffer_binding = {1, 2},
            .default_binding_type_order =
                {
                    ResourceType::kTexture1d_f32_unfilterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2d_f32_unfilterable,
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
