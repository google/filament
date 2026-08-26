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

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/resource_table.h"
#include "src/tint/lang/core/type/resource_type.h"
#include "src/tint/lang/core/type/sampled_texture.h"  // IWYU pragma: export
#include "src/tint/lang/core/type/storage_texture.h"  // IWYU pragma: export

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

class Helper : public ResourceTableHelper {
  public:
    Helper() = default;
    ~Helper() override = default;

    // Returns a map of types to the var which is used to access the memory of that type
    Hashmap<const core::type::Type*, core::ir::Var*, 4> GenerateVars(
        core::ir::Builder& b,
        const BindingPoint& bp,
        const std::vector<ResourceType>& types) const override {
        Hashmap<const core::type::Type*, core::ir::Var*, 4> res;

        for (auto& type : types) {
            auto* t = core::type::ResourceTypeToType(b.ir.Types(), type);

            if (res.Contains(t)) {
                continue;
            }

            auto* ty = b.ir.Types().Get<core::type::ResourceTable>(t);

            auto* v = b.Var(b.ir.Types().ptr(handle, ty));
            v->SetBindingPoint(bp.group, bp.binding);
            res.Add(t, v);
        }

        return res;
    }

    Helper(const Helper&) = delete;
    Helper(Helper&&) = delete;
    Helper& operator=(const Helper&) = delete;
    Helper& operator=(Helper&&) = delete;
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
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, MissingConfig) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t", b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource,
                                  Vector<TemplateParameter, 1>{texture_ty}, 1_u));
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
        b.Let("t", b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource,
                                  Vector<TemplateParameter, 1>{texture_ty}, 1_u));
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 1u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:bool = eq %11, 15u
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
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, HasResource_Filterable) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t", b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource,
                                  Vector<TemplateParameter, 1>{texture_ty}, 2_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:bool = hasResource<texture_2d<f32>> 2u
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 2u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %11:u32 = load %10
        %12:vec3<u32> = construct %11
        %13:vec3<u32> = construct 6u, 7u, 34u
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
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, HasResource_Unfilterable) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t", b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource,
                                  Vector<TemplateParameter, 1>{texture_ty}, 2_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:bool = hasResource<texture_2d<f32>> 2u
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 2u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %11:u32 = load %10
        %12:vec3<u32> = construct %11
        %13:vec3<u32> = construct 6u, 7u, 34u
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
                    ResourceType::kTexture1d_f32_unfilterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2d_f32_unfilterable,
                },
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_Texture) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);
        b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, tex);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d_array<u32> = getResource<texture_2d_array<u32>> 1u
    %3:vec2<u32> = textureDimensions %2
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 1u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:bool = eq %11, 15u
        exit_if %12  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %item_idx:u32 = if %9 [t: $B5, f: $B6] {  # if_2
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
    %17:ptr<handle, texture_2d_array<u32>, read> = access %3, %item_idx
    %18:texture_2d_array<u32> = load %17
    %19:vec2<u32> = textureDimensions %18
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
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_ResourceTexture_ResourceSampler) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* sampler_ty = ty.sampler();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);
        auto* sam = b.CallExplicit(sampler_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{sampler_ty}, 2_u);

        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d<f32> = getResource<texture_2d<f32>> 1u
    %3:sampler = getResource<sampler> 2u
    %4:vec4<f32> = textureSample %2, %3, vec2<f32>(0.0f)
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
  %1:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %4:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %8:u32 = load %tint_storage_metadata_length
    %9:bool = lt 1u, %8
    %has_resource:bool = if %9 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %11:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %12:u32 = load %11
        %13:vec3<u32> = construct %12
        %14:vec3<u32> = construct 6u, 7u, 34u
        %15:vec3<bool> = eq %13, %14
        %16:bool = any %15
        exit_if %16  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %texture_kind:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        %18:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %19:u32 = load %18
        exit_if %19  # if_2
      }
      $B6: {  # false
        exit_if 6u  # if_2
      }
    }
    %item_idx:u32 = if %has_resource [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        exit_if 1u  # if_3
      }
      $B8: {  # false
        %21:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %22:u32 = load %21
        %23:u32 = add 1u, %22
        exit_if %23  # if_3
      }
    }
    %24:ptr<handle, texture_2d<f32>, read> = access %1, %item_idx
    %25:texture_2d<f32> = load %24
    %tint_storage_metadata_length_1:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u  # %tint_storage_metadata_length_1: 'tint_storage_metadata_length'
    %27:u32 = load %tint_storage_metadata_length_1
    %28:bool = lt 2u, %27
    %has_resource_1:bool = if %28 [t: $B9, f: $B10] {  # if_4
      $B9: {  # true
        %30:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %31:u32 = load %30
        %32:vec2<u32> = construct %31
        %33:vec2<u32> = construct 40u, 41u
        %34:vec2<bool> = eq %32, %33
        %35:bool = any %34
        exit_if %35  # if_4
      }
      $B10: {  # false
        exit_if false  # if_4
      }
    }  # %has_resource_1: 'has_resource'
    %sampler_kind:u32 = if %has_resource_1 [t: $B11, f: $B12] {  # if_5
      $B11: {  # true
        %37:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %38:u32 = load %37
        exit_if %38  # if_5
      }
      $B12: {  # false
        exit_if 41u  # if_5
      }
    }
    %item_idx_1:u32 = if %has_resource_1 [t: $B13, f: $B14] {  # if_6
      $B13: {  # true
        exit_if 2u  # if_6
      }
      $B14: {  # false
        %40:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %41:u32 = load %40
        %42:u32 = add 5u, %41
        exit_if %42  # if_6
      }
    }  # %item_idx_1: 'item_idx'
    %43:ptr<handle, sampler, read> = access %4, %item_idx_1
    %44:sampler = load %43
    %45:bool = eq %texture_kind, 6u
    %46:bool = eq %sampler_kind, 40u
    %texture_sampler_combo_valid:bool = if %46 [t: $B15, f: $B16] {  # if_7
      $B15: {  # true
        exit_if %45  # if_7
      }
      $B16: {  # false
        exit_if true  # if_7
      }
    }
    %48:vec4<f32> = if %texture_sampler_combo_valid [t: $B17, f: $B18] {  # if_8
      $B17: {  # true
        %49:vec4<f32> = textureSample %25, %44, vec2<f32>(0.0f)
        exit_if %49  # if_8
      }
      $B18: {  # false
        exit_if vec4<f32>(0.0f)  # if_8
      }
    }
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
                    ResourceType::kTexture2d_f32_unfilterable,
                    ResourceType::kTexture2d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                },
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_ResourceTexture_VarSampler) {
    auto* sam_var = b.Var("sampler", ty.ptr(handle, ty.sampler()));
    sam_var->SetBindingPoint(3, 2);
    mod.root_block->Append(sam_var);

    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);

        core::ir::Load* sam = b.Load(sam_var);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    %3:texture_2d<f32> = getResource<texture_2d<f32>> 1u
    %4:sampler = load %sampler
    %5:vec4<f32> = textureSample %3, %4, vec2<f32>(0.0f)
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
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(3, 2)
  %2:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %4:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %5:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %8:sampler = load %sampler
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %10:u32 = load %tint_storage_metadata_length
    %11:bool = lt 1u, %10
    %has_resource:bool = if %11 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %13:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %14:u32 = load %13
        %15:vec3<u32> = construct %14
        %16:vec3<u32> = construct 6u, 7u, 34u
        %17:vec3<bool> = eq %15, %16
        %18:bool = any %17
        exit_if %18  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %texture_kind:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        %20:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %21:u32 = load %20
        exit_if %21  # if_2
      }
      $B6: {  # false
        exit_if 6u  # if_2
      }
    }
    %item_idx:u32 = if %has_resource [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        exit_if 1u  # if_3
      }
      $B8: {  # false
        %23:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %24:u32 = load %23
        %25:u32 = add 1u, %24
        exit_if %25  # if_3
      }
    }
    %26:ptr<handle, texture_2d<f32>, read> = access %2, %item_idx
    %27:texture_2d<f32> = load %26
    %28:bool = eq %texture_kind, 6u
    %29:vec4<f32> = if %28 [t: $B9, f: $B10] {  # if_4
      $B9: {  # true
        %30:vec4<f32> = textureSample %27, %8, vec2<f32>(0.0f)
        exit_if %30  # if_4
      }
      $B10: {  # false
        exit_if vec4<f32>(0.0f)  # if_4
      }
    }
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
                    ResourceType::kTexture2d_f32_unfilterable,
                    ResourceType::kTexture2d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                },
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kSampler_filtering},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_ResourceTexture_VarSamplerNonFiltering) {
    auto* sam_var = b.Var("sampler", ty.ptr(handle, ty.sampler()));
    sam_var->SetBindingPoint(3, 2);
    mod.root_block->Append(sam_var);

    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);

        core::ir::Load* sam = b.Load(sam_var);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    %3:texture_2d<f32> = getResource<texture_2d<f32>> 1u
    %4:sampler = load %sampler
    %5:vec4<f32> = textureSample %3, %4, vec2<f32>(0.0f)
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
  %sampler:ptr<handle, sampler, read> = var undef @binding_point(3, 2)
  %2:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %4:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %5:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %8:sampler = load %sampler
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %10:u32 = load %tint_storage_metadata_length
    %11:bool = lt 1u, %10
    %has_resource:bool = if %11 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %13:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %14:u32 = load %13
        %15:vec3<u32> = construct %14
        %16:vec3<u32> = construct 6u, 7u, 34u
        %17:vec3<bool> = eq %15, %16
        %18:bool = any %17
        exit_if %18  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %texture_kind:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        %20:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %21:u32 = load %20
        exit_if %21  # if_2
      }
      $B6: {  # false
        exit_if 6u  # if_2
      }
    }
    %item_idx:u32 = if %has_resource [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        exit_if 1u  # if_3
      }
      $B8: {  # false
        %23:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %24:u32 = load %23
        %25:u32 = add 1u, %24
        exit_if %25  # if_3
      }
    }
    %26:ptr<handle, texture_2d<f32>, read> = access %2, %item_idx
    %27:texture_2d<f32> = load %26
    %28:vec4<f32> = textureSample %27, %8, vec2<f32>(0.0f)
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
                    ResourceType::kTexture2d_f32_unfilterable,
                    ResourceType::kTexture2d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                },
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kSampler_non_filtering},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_VarTexture_ResourceSampler) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex_var = b.Var("texture", ty.ptr(handle, texture_ty));
    tex_var->SetBindingPoint(3, 2);
    mod.root_block->Append(tex_var);

    auto* sampler_ty = ty.sampler();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* sam = b.CallExplicit(sampler_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{sampler_ty}, 1_u);

        core::ir::Load* tex = b.Load(tex_var);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    %3:sampler = getResource<sampler> 1u
    %4:texture_2d<f32> = load %texture
    %5:vec4<f32> = textureSample %4, %3, vec2<f32>(0.0f)
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
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
  %2:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %4:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %5:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %8:texture_2d<f32> = load %texture
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %10:u32 = load %tint_storage_metadata_length
    %11:bool = lt 1u, %10
    %has_resource:bool = if %11 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %13:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %14:u32 = load %13
        %15:vec2<u32> = construct %14
        %16:vec2<u32> = construct 40u, 41u
        %17:vec2<bool> = eq %15, %16
        %18:bool = any %17
        exit_if %18  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %sampler_kind:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        %20:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %21:u32 = load %20
        exit_if %21  # if_2
      }
      $B6: {  # false
        exit_if 41u  # if_2
      }
    }
    %item_idx:u32 = if %has_resource [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        exit_if 1u  # if_3
      }
      $B8: {  # false
        %23:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %24:u32 = load %23
        %25:u32 = add 5u, %24
        exit_if %25  # if_3
      }
    }
    %26:ptr<handle, sampler, read> = access %5, %item_idx
    %27:sampler = load %26
    %28:vec4<f32> = textureSample %8, %27, vec2<f32>(0.0f)
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
                    ResourceType::kTexture2d_f32_unfilterable,
                    ResourceType::kTexture2d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                },
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kTexture2d_f32_filterable},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_VarTexture_ResourceSampler_unfilterable_texture) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex_var = b.Var("texture", ty.ptr(handle, texture_ty));
    tex_var->SetBindingPoint(3, 2);
    mod.root_block->Append(tex_var);

    auto* sampler_ty = ty.sampler();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* sam = b.CallExplicit(sampler_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{sampler_ty}, 1_u);

        core::ir::Load* tex = b.Load(tex_var);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    %3:sampler = getResource<sampler> 1u
    %4:texture_2d<f32> = load %texture
    %5:vec4<f32> = textureSample %4, %3, vec2<f32>(0.0f)
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
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
  %2:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %4:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %5:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %8:texture_2d<f32> = load %texture
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %10:u32 = load %tint_storage_metadata_length
    %11:bool = lt 1u, %10
    %has_resource:bool = if %11 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %13:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %14:u32 = load %13
        %15:vec2<u32> = construct %14
        %16:vec2<u32> = construct 40u, 41u
        %17:vec2<bool> = eq %15, %16
        %18:bool = any %17
        exit_if %18  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %sampler_kind:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        %20:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %21:u32 = load %20
        exit_if %21  # if_2
      }
      $B6: {  # false
        exit_if 41u  # if_2
      }
    }
    %item_idx:u32 = if %has_resource [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        exit_if 1u  # if_3
      }
      $B8: {  # false
        %23:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %24:u32 = load %23
        %25:u32 = add 5u, %24
        exit_if %25  # if_3
      }
    }
    %26:ptr<handle, sampler, read> = access %5, %item_idx
    %27:sampler = load %26
    %28:bool = eq %sampler_kind, 40u
    %texture_sampler_combo_valid:bool = if %28 [t: $B9, f: $B10] {  # if_4
      $B9: {  # true
        exit_if false  # if_4
      }
      $B10: {  # false
        exit_if true  # if_4
      }
    }
    %30:vec4<f32> = if %texture_sampler_combo_valid [t: $B11, f: $B12] {  # if_5
      $B11: {  # true
        %31:vec4<f32> = textureSample %8, %27, vec2<f32>(0.0f)
        exit_if %31  # if_5
      }
      $B12: {  # false
        exit_if vec4<f32>(0.0f)  # if_5
      }
    }
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
                    ResourceType::kTexture2d_f32_unfilterable,
                    ResourceType::kTexture2d_f32_filterable,
                    ResourceType::kTexture3d_i32,
                    ResourceType::kTexture2dArray_u32,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                },
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kTexture2d_f32_unfilterable},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

// TODO(479179409): This case could be smarter. When reading from the resource_table we could track
// the scope we're in and decide we can reuse a previous load.
TEST_F(IR_ResourceTableTest, GetResource_MultiUse) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);
        b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, tex);
        b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, tex);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d_array<f32> = getResource<texture_2d_array<f32>> 1u
    %3:vec2<u32> = textureDimensions %2
    %4:vec2<u32> = textureDimensions %2
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<f32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 1u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:vec3<u32> = construct %11
        %13:vec3<u32> = construct 11u, 12u, 35u
        %14:vec3<bool> = eq %12, %13
        %15:bool = any %14
        exit_if %15  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %item_idx:u32 = if %9 [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        exit_if 1u  # if_2
      }
      $B6: {  # false
        %17:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %18:u32 = load %17
        %19:u32 = add 3u, %18
        exit_if %19  # if_2
      }
    }
    %20:ptr<handle, texture_2d_array<f32>, read> = access %3, %item_idx
    %21:texture_2d_array<f32> = load %20
    %tint_storage_metadata_length_1:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u  # %tint_storage_metadata_length_1: 'tint_storage_metadata_length'
    %23:u32 = load %tint_storage_metadata_length_1
    %24:bool = lt 1u, %23
    %25:bool = if %24 [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        %26:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %27:u32 = load %26
        %28:vec3<u32> = construct %27
        %29:vec3<u32> = construct 11u, 12u, 35u
        %30:vec3<bool> = eq %28, %29
        %31:bool = any %30
        exit_if %31  # if_3
      }
      $B8: {  # false
        exit_if false  # if_3
      }
    }
    %item_idx_1:u32 = if %25 [t: $B9, f: $B10] {  # if_4
      $B9: {  # true
        exit_if 1u  # if_4
      }
      $B10: {  # false
        %33:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %34:u32 = load %33
        %35:u32 = add 3u, %34
        exit_if %35  # if_4
      }
    }  # %item_idx_1: 'item_idx'
    %36:ptr<handle, texture_2d_array<f32>, read> = access %3, %item_idx_1
    %37:texture_2d_array<f32> = load %36
    %38:vec2<u32> = textureDimensions %21
    %39:vec2<u32> = textureDimensions %37
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
                    ResourceType::kTexture2dArray_f32_unfilterable,
                    ResourceType::kTexture2dArray_f32_filterable,
                },
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_MultiUse_DifferentScope) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, tex);
            b.ExitIf(if_);
        });
        b.Append(if_->False(), [&] {
            b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, tex);
            b.ExitIf(if_);
        });

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d_array<u32> = getResource<texture_2d_array<u32>> 1u
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:vec2<u32> = textureDimensions %2
        exit_if  # if_1
      }
      $B3: {  # false
        %4:vec2<u32> = textureDimensions %2
        exit_if  # if_1
      }
    }
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d_array<u32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 1u, %7
    %9:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:bool = eq %11, 15u
        exit_if %12  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %item_idx:u32 = if %9 [t: $B5, f: $B6] {  # if_2
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
    %17:ptr<handle, texture_2d_array<u32>, read> = access %3, %item_idx
    %18:texture_2d_array<u32> = load %17
    %tint_storage_metadata_length_1:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u  # %tint_storage_metadata_length_1: 'tint_storage_metadata_length'
    %20:u32 = load %tint_storage_metadata_length_1
    %21:bool = lt 1u, %20
    %22:bool = if %21 [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        %23:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %24:u32 = load %23
        %25:bool = eq %24, 15u
        exit_if %25  # if_3
      }
      $B8: {  # false
        exit_if false  # if_3
      }
    }
    %item_idx_1:u32 = if %22 [t: $B9, f: $B10] {  # if_4
      $B9: {  # true
        exit_if 1u  # if_4
      }
      $B10: {  # false
        %27:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %28:u32 = load %27
        %29:u32 = add 2u, %28
        exit_if %29  # if_4
      }
    }  # %item_idx_1: 'item_idx'
    %30:ptr<handle, texture_2d_array<u32>, read> = access %3, %item_idx_1
    %31:texture_2d_array<u32> = load %30
    if true [t: $B11, f: $B12] {  # if_5
      $B11: {  # true
        %32:vec2<u32> = textureDimensions %18
        exit_if  # if_5
      }
      $B12: {  # false
        %33:vec2<u32> = textureDimensions %31
        exit_if  # if_5
      }
    }
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
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_Unused) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                       Vector<TemplateParameter, 1>{texture_ty}, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_2d<f32> = getResource<texture_2d<f32>> 2u
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
  %1:ptr<handle, resource_table<texture_1d<f32>>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<texture_3d<i32>>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<texture_2d<f32>>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
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
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, HasResource_GetSamplerIndexFromMetadata) {
    auto* sampler_ty = ty.sampler();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("t", b.CallExplicit(ty.bool_(), core::BuiltinFn::kHasResource,
                                  Vector<TemplateParameter, 1>{sampler_ty}, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:bool = hasResource<sampler> 1u
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
  %1:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %5:u32 = load %tint_storage_metadata_length
    %6:bool = lt 1u, %5
    %7:bool = if %6 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %8:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %9:u32 = load %8
        %10:u32 = and %9, 65535u
        %11:vec2<u32> = construct %10
        %12:vec2<u32> = construct 40u, 41u
        %13:vec2<bool> = eq %11, %12
        %14:bool = any %13
        exit_if %14  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %t:bool = let %7
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
                    ResourceType::kSampler_non_filtering,
                },
            .get_sampler_index_from_metadata = true,
            .binding_to_resource_type = {},
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_GetSamplerIndexFromMetadata) {
    auto* texture_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex_var = b.Var("texture", ty.ptr(handle, texture_ty));
    tex_var->SetBindingPoint(3, 2);

    mod.root_block->Append(tex_var);

    auto* sampler_ty = ty.sampler();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        core::ir::Load* tex = b.Load(tex_var);
        core::ir::Instruction* sam = b.CallExplicit(sampler_ty, core::BuiltinFn::kGetResource,
                                                    Vector<TemplateParameter, 1>{sampler_ty}, 1_u);

        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f));

        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    %3:texture_2d<f32> = load %texture
    %4:sampler = getResource<sampler> 1u
    %5:vec4<f32> = textureSample %3, %4, vec2<f32>(0.0f)
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
  %texture:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(3, 2)
  %2:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %5:texture_2d<f32> = load %texture
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 1u, %7
    %has_resource:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:u32 = and %11, 65535u
        %13:vec2<u32> = construct %12
        %14:vec2<u32> = construct 40u, 41u
        %15:vec2<bool> = eq %13, %14
        %16:bool = any %15
        exit_if %16  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %sampler_kind:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        %18:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %19:u32 = load %18
        exit_if %19  # if_2
      }
      $B6: {  # false
        exit_if 41u  # if_2
      }
    }
    %item_idx:u32 = if %has_resource [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        exit_if 1u  # if_3
      }
      $B8: {  # false
        %21:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %22:u32 = load %21
        %23:u32 = add 0u, %22
        exit_if %23  # if_3
      }
    }
    %24:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, %item_idx
    %25:u32 = load %24
    %26:u32 = shr %25, 16u
    %27:ptr<handle, sampler, read> = access %2, %26
    %28:sampler = load %27
    %29:vec4<f32> = textureSample %5, %28, vec2<f32>(0.0f)
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
                    ResourceType::kSampler_non_filtering,
                },
            .get_sampler_index_from_metadata = true,
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kTexture2d_f32_filterable},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_ResourceTexture_VarComparisonSampler) {
    auto* sam_var = b.Var("comparison_sampler", ty.ptr(handle, ty.comparison_sampler()));
    sam_var->SetBindingPoint(3, 2);
    mod.root_block->Append(sam_var);

    auto* texture_ty = ty.depth_texture(core::type::TextureDimension::k2d);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);

        core::ir::Load* sam = b.Load(sam_var);
        b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompare, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f), 0_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %comparison_sampler:ptr<handle, sampler_comparison, read> = var undef @binding_point(3, 2)
}

%foo = func():void {
  $B2: {
    %3:texture_depth_2d = getResource<texture_depth_2d> 1u
    %4:sampler_comparison = load %comparison_sampler
    %5:f32 = textureSampleCompare %3, %4, vec2<f32>(0.0f), 0.0f
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
  %comparison_sampler:ptr<handle, sampler_comparison, read> = var undef @binding_point(3, 2)
  %2:ptr<handle, resource_table<texture_depth_2d>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %4:ptr<handle, resource_table<sampler_comparison>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %7:sampler_comparison = load %comparison_sampler
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %9:u32 = load %tint_storage_metadata_length
    %10:bool = lt 1u, %9
    %has_resource:bool = if %10 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %12:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %13:u32 = load %12
        %14:bool = eq %13, 34u
        exit_if %14  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %item_idx:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        exit_if 1u  # if_2
      }
      $B6: {  # false
        %16:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %17:u32 = load %16
        %18:u32 = add 0u, %17
        exit_if %18  # if_2
      }
    }
    %19:ptr<handle, texture_depth_2d, read> = access %2, %item_idx
    %20:texture_depth_2d = load %19
    %21:f32 = textureSampleCompare %20, %7, vec2<f32>(0.0f), 0.0f
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
                    ResourceType::kTextureDepth2d,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                    ResourceType::kSampler_comparison,
                },
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kSampler_comparison},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ResourceTableTest, GetResource_ResourceTexture_ResourcerComparisonSampler) {
    auto* texture_ty = ty.depth_texture(core::type::TextureDimension::k2d);
    auto* sampler_ty = ty.comparison_sampler();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* tex = b.CallExplicit(texture_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{texture_ty}, 1_u);
        auto* sam = b.CallExplicit(sampler_ty, core::BuiltinFn::kGetResource,
                                   Vector<TemplateParameter, 1>{sampler_ty}, 2_u);

        b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompare, tex, sam,
               b.Splat(ty.vec2<f32>(), 0_f), 0_f);
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:texture_depth_2d = getResource<texture_depth_2d> 1u
    %3:sampler_comparison = getResource<sampler_comparison> 2u
    %4:f32 = textureSampleCompare %2, %3, vec2<f32>(0.0f), 0.0f
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
  %1:ptr<handle, resource_table<texture_depth_2d>, read> = var undef @binding_point(0, 1)
  %2:ptr<handle, resource_table<sampler>, read> = var undef @binding_point(0, 1)
  %3:ptr<handle, resource_table<sampler_comparison>, read> = var undef @binding_point(0, 1)
  %tint_resource_table_metadata:ptr<storage, tint_resource_table_metadata_struct, read> = var undef @binding_point(1, 2)
}

%foo = func():void {
  $B2: {
    %tint_storage_metadata_length:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
    %7:u32 = load %tint_storage_metadata_length
    %8:bool = lt 1u, %7
    %has_resource:bool = if %8 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %10:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 1u
        %11:u32 = load %10
        %12:bool = eq %11, 34u
        exit_if %12  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %item_idx:u32 = if %has_resource [t: $B5, f: $B6] {  # if_2
      $B5: {  # true
        exit_if 1u  # if_2
      }
      $B6: {  # false
        %14:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %15:u32 = load %14
        %16:u32 = add 0u, %15
        exit_if %16  # if_2
      }
    }
    %17:ptr<handle, texture_depth_2d, read> = access %1, %item_idx
    %18:texture_depth_2d = load %17
    %tint_storage_metadata_length_1:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u  # %tint_storage_metadata_length_1: 'tint_storage_metadata_length'
    %20:u32 = load %tint_storage_metadata_length_1
    %21:bool = lt 2u, %20
    %has_resource_1:bool = if %21 [t: $B7, f: $B8] {  # if_3
      $B7: {  # true
        %23:ptr<storage, u32, read> = access %tint_resource_table_metadata, 1u, 2u
        %24:u32 = load %23
        %25:bool = eq %24, 42u
        exit_if %25  # if_3
      }
      $B8: {  # false
        exit_if false  # if_3
      }
    }  # %has_resource_1: 'has_resource'
    %item_idx_1:u32 = if %has_resource_1 [t: $B9, f: $B10] {  # if_4
      $B9: {  # true
        exit_if 2u  # if_4
      }
      $B10: {  # false
        %27:ptr<storage, u32, read> = access %tint_resource_table_metadata, 0u
        %28:u32 = load %27
        %29:u32 = add 3u, %28
        exit_if %29  # if_4
      }
    }  # %item_idx_1: 'item_idx'
    %30:ptr<handle, sampler_comparison, read> = access %3, %item_idx_1
    %31:sampler_comparison = load %30
    %32:f32 = textureSampleCompare %18, %31, vec2<f32>(0.0f), 0.0f
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
                    ResourceType::kTextureDepth2d,
                    ResourceType::kSampler_filtering,
                    ResourceType::kSampler_non_filtering,
                    ResourceType::kSampler_comparison,
                },
            .binding_to_resource_type =
                {
                    {BindingPoint{3, 2}, ResourceType::kSampler_comparison},
                },
        },
        &helper);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
