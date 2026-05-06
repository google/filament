// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/resource_type.h"

#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::type {

const core::type::Type* ResourceTypeToType(core::type::Manager& ty, ResourceType type) {
    switch (type) {
        case ResourceType::kTexture1d_f32_filterable:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32(),
                                      core::TextureFilterable::kFilterable);
        case ResourceType::kTexture1d_f32_unfilterable:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32(),
                                      core::TextureFilterable::kUnfilterable);
        case ResourceType::kTexture1d_i32:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.i32());
        case ResourceType::kTexture1d_u32:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.u32());
        case ResourceType::kTexture2d_f32_filterable:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32(),
                                      core::TextureFilterable::kFilterable);
        case ResourceType::kTexture2d_f32_unfilterable:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32(),
                                      core::TextureFilterable::kUnfilterable);
        case ResourceType::kTexture2d_i32:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32());
        case ResourceType::kTexture2d_u32:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.u32());
        case ResourceType::kTexture2dArray_f32_filterable:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32(),
                                      core::TextureFilterable::kFilterable);
        case ResourceType::kTexture2dArray_f32_unfilterable:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32(),
                                      core::TextureFilterable::kUnfilterable);
        case ResourceType::kTexture2dArray_i32:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.i32());
        case ResourceType::kTexture2dArray_u32:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());
        case ResourceType::kTexture3d_f32_filterable:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32(),
                                      core::TextureFilterable::kFilterable);
        case ResourceType::kTexture3d_f32_unfilterable:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32(),
                                      core::TextureFilterable::kUnfilterable);
        case ResourceType::kTexture3d_i32:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.i32());
        case ResourceType::kTexture3d_u32:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.u32());
        case ResourceType::kTextureCube_f32_filterable:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32(),
                                      core::TextureFilterable::kFilterable);
        case ResourceType::kTextureCube_f32_unfilterable:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32(),
                                      core::TextureFilterable::kUnfilterable);
        case ResourceType::kTextureCube_i32:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.i32());
        case ResourceType::kTextureCube_u32:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.u32());
        case ResourceType::kTextureCubeArray_f32_filterable:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32(),
                                      core::TextureFilterable::kFilterable);
        case ResourceType::kTextureCubeArray_f32_unfilterable:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32(),
                                      core::TextureFilterable::kUnfilterable);
        case ResourceType::kTextureCubeArray_i32:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.i32());
        case ResourceType::kTextureCubeArray_u32:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.u32());

        case ResourceType::kTextureMultisampled2d_f32:
            return ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32());
        case ResourceType::kTextureMultisampled2d_i32:
            return ty.multisampled_texture(core::type::TextureDimension::k2d, ty.i32());
        case ResourceType::kTextureMultisampled2d_u32:
            return ty.multisampled_texture(core::type::TextureDimension::k2d, ty.u32());

        case ResourceType::kTextureDepth2d:
            return ty.depth_texture(core::type::TextureDimension::k2d);
        case ResourceType::kTextureDepth2dArray:
            return ty.depth_texture(core::type::TextureDimension::k2dArray);
        case ResourceType::kTextureDepthCube:
            return ty.depth_texture(core::type::TextureDimension::kCube);
        case ResourceType::kTextureDepthCubeArray:
            return ty.depth_texture(core::type::TextureDimension::kCubeArray);
        case ResourceType::kTextureDepthMultisampled2d:
            return ty.depth_multisampled_texture(core::type::TextureDimension::k2d);

        case ResourceType::kSampler_filtering:
            return ty.sampler(core::SamplerFiltering::kFiltering);
        case ResourceType::kSampler_non_filtering:
            return ty.sampler(core::SamplerFiltering::kNonFiltering);
        case ResourceType::kSampler_comparison:
            return ty.comparison_sampler();
        default:
            TINT_UNREACHABLE();
    }
}

ResourceType TypeToResourceType(const core::type::Type* in_type) {
    return tint::Switch(
        in_type,
        [&](const core::type::SampledTexture* sa) {
            switch (sa->Dim()) {
                case core::type::TextureDimension::k1d:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) {
                            switch (sa->Filterable()) {
                                case core::TextureFilterable::kFilterable:
                                    return ResourceType::kTexture1d_f32_filterable;
                                case core::TextureFilterable::kUnfilterable:
                                    return ResourceType::kTexture1d_f32_unfilterable;
                                case core::TextureFilterable::kUndefined:
                                    TINT_UNREACHABLE();
                            }
                        },
                        [&](const core::type::I32*) { return ResourceType::kTexture1d_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture1d_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::k2d:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) {
                            switch (sa->Filterable()) {
                                case core::TextureFilterable::kFilterable:
                                    return ResourceType::kTexture2d_f32_filterable;
                                case core::TextureFilterable::kUnfilterable:
                                    return ResourceType::kTexture2d_f32_unfilterable;
                                case core::TextureFilterable::kUndefined:
                                    TINT_UNREACHABLE();
                            }
                        },
                        [&](const core::type::I32*) { return ResourceType::kTexture2d_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture2d_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::k2dArray:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) {
                            switch (sa->Filterable()) {
                                case core::TextureFilterable::kFilterable:
                                    return ResourceType::kTexture2dArray_f32_filterable;
                                case core::TextureFilterable::kUnfilterable:
                                    return ResourceType::kTexture2dArray_f32_unfilterable;
                                case core::TextureFilterable::kUndefined:
                                    TINT_UNREACHABLE();
                            }
                        },
                        [&](const core::type::I32*) { return ResourceType::kTexture2dArray_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture2dArray_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::k3d:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) {
                            switch (sa->Filterable()) {
                                case core::TextureFilterable::kFilterable:
                                    return ResourceType::kTexture3d_f32_filterable;
                                case core::TextureFilterable::kUnfilterable:
                                    return ResourceType::kTexture3d_f32_unfilterable;
                                case core::TextureFilterable::kUndefined:
                                    TINT_UNREACHABLE();
                            }
                        },
                        [&](const core::type::I32*) { return ResourceType::kTexture3d_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture3d_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::kCube:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) {
                            switch (sa->Filterable()) {
                                case core::TextureFilterable::kFilterable:
                                    return ResourceType::kTextureCube_f32_filterable;
                                case core::TextureFilterable::kUnfilterable:
                                    return ResourceType::kTextureCube_f32_unfilterable;
                                case core::TextureFilterable::kUndefined:
                                    TINT_UNREACHABLE();
                            }
                        },
                        [&](const core::type::I32*) { return ResourceType::kTextureCube_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTextureCube_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::kCubeArray:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) {
                            switch (sa->Filterable()) {
                                case core::TextureFilterable::kFilterable:
                                    return ResourceType::kTextureCubeArray_f32_filterable;
                                case core::TextureFilterable::kUnfilterable:
                                    return ResourceType::kTextureCubeArray_f32_unfilterable;
                                case core::TextureFilterable::kUndefined:
                                    TINT_UNREACHABLE();
                            }
                        },
                        [&](const core::type::I32*) { return ResourceType::kTextureCubeArray_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTextureCubeArray_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::kNone:
                    TINT_UNREACHABLE();
            }
        },
        [&](const core::type::MultisampledTexture* ms) {
            return tint::Switch(
                ms->Type(),
                [&](const core::type::F32*) { return ResourceType::kTextureMultisampled2d_f32; },
                [&](const core::type::I32*) { return ResourceType::kTextureMultisampled2d_i32; },
                [&](const core::type::U32*) { return ResourceType::kTextureMultisampled2d_u32; },
                TINT_ICE_ON_NO_MATCH);
        },
        [&](const core::type::DepthMultisampledTexture*) {
            return ResourceType::kTextureDepthMultisampled2d;
        },
        [&](const core::type::DepthTexture* de) {
            switch (de->Dim()) {
                case core::type::TextureDimension::k2d:
                    return ResourceType::kTextureDepth2d;
                case core::type::TextureDimension::k2dArray:
                    return ResourceType::kTextureDepth2dArray;
                case core::type::TextureDimension::kCube:
                    return ResourceType::kTextureDepthCube;
                case core::type::TextureDimension::kCubeArray:
                    return ResourceType::kTextureDepthCubeArray;
                default:
                    TINT_UNREACHABLE();
            }
        },
        [&](const core::type::Sampler* s) {
            if (s->IsComparison()) {
                return ResourceType::kSampler_comparison;
            }

            TINT_ASSERT(s->Filtering() != core::SamplerFiltering::kUndefined);
            if (s->Filtering() == core::SamplerFiltering::kFiltering) {
                return ResourceType::kSampler_filtering;
            }
            return ResourceType::kSampler_non_filtering;
        },
        TINT_ICE_ON_NO_MATCH);
}

std::vector<ResourceType> ConvertsFrom(const core::type::Type* in_type) {
    return tint::Switch(
        in_type,
        [&](const core::type::SampledTexture* sa) -> std::vector<ResourceType> {
            if (sa->Filterable() != core::TextureFilterable::kUnfilterable) {
                return {};
            }
            TINT_ASSERT((sa->Type()->Is<core::type::F32>()));

            switch (sa->Dim()) {
                case core::type::TextureDimension::k1d:
                    return {ResourceType::kTexture1d_f32_filterable};
                case core::type::TextureDimension::k2d:
                    return {ResourceType::kTexture2d_f32_filterable, ResourceType::kTextureDepth2d};
                case core::type::TextureDimension::k2dArray:
                    return {ResourceType::kTexture2dArray_f32_filterable,
                            ResourceType::kTextureDepth2dArray};
                case core::type::TextureDimension::k3d:
                    return {ResourceType::kTexture3d_f32_filterable};
                case core::type::TextureDimension::kCube:
                    return {ResourceType::kTextureCube_f32_filterable,
                            ResourceType::kTextureDepthCube};
                case core::type::TextureDimension::kCubeArray:
                    return {ResourceType::kTextureCubeArray_f32_unfilterable,
                            ResourceType::kTextureDepthCubeArray};
                case core::type::TextureDimension::kNone:
                    TINT_UNREACHABLE();
            }
        },
        [&](const core::type::Sampler* s) -> std::vector<ResourceType> {
            if (s->Filtering() == core::SamplerFiltering::kNonFiltering) {
                return {ResourceType::kSampler_filtering};
            }
            return {};
        },
        [](Default) -> std::vector<ResourceType> { return {}; });
}

}  // namespace tint::core::type
