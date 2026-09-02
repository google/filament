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

#include "src/dawn/native/ResourceTableDefaultResources.h"

#include "src/dawn/common/ityp_array.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/ResourceTable.h"
#include "src/dawn/native/Texture.h"
#include "tint/tint.h"

namespace dawn::native {

namespace {
constexpr auto kDefaults = ityp::array<ResourceTableSlot, tint::ResourceType, 35>{
    tint::ResourceType::kTexture1d_f32_filterable,
    tint::ResourceType::kTexture2d_f32_filterable,
    tint::ResourceType::kTexture2dArray_f32_filterable,
    tint::ResourceType::kTextureCube_f32_filterable,
    tint::ResourceType::kTextureCubeArray_f32_filterable,
    tint::ResourceType::kTexture3d_f32_filterable,
    tint::ResourceType::kTexture1d_f32_unfilterable,
    tint::ResourceType::kTexture2d_f32_unfilterable,
    tint::ResourceType::kTexture2dArray_f32_unfilterable,
    tint::ResourceType::kTextureCube_f32_unfilterable,
    tint::ResourceType::kTextureCubeArray_f32_unfilterable,
    tint::ResourceType::kTexture3d_f32_unfilterable,

    tint::ResourceType::kTexture1d_u32,
    tint::ResourceType::kTexture2d_u32,
    tint::ResourceType::kTexture2dArray_u32,
    tint::ResourceType::kTextureCube_u32,
    tint::ResourceType::kTextureCubeArray_u32,
    tint::ResourceType::kTexture3d_u32,

    tint::ResourceType::kTexture1d_i32,
    tint::ResourceType::kTexture2d_i32,
    tint::ResourceType::kTexture2dArray_i32,
    tint::ResourceType::kTextureCube_i32,
    tint::ResourceType::kTextureCubeArray_i32,
    tint::ResourceType::kTexture3d_i32,

    tint::ResourceType::kTextureMultisampled2d_f32,
    tint::ResourceType::kTextureMultisampled2d_u32,
    tint::ResourceType::kTextureMultisampled2d_i32,

    tint::ResourceType::kTextureDepth2d,
    tint::ResourceType::kTextureDepth2dArray,
    tint::ResourceType::kTextureDepthCube,
    tint::ResourceType::kTextureDepthCubeArray,
    tint::ResourceType::kTextureDepthMultisampled2d,

    // If more samplers are added, update kNumDefaultSamplers.
    tint::ResourceType::kSampler_filtering,
    tint::ResourceType::kSampler_non_filtering,
    tint::ResourceType::kSampler_comparison,
};
constexpr auto kNumDefaultSamplers = ResourceTableSlot{3u};

// This helper function is used in ASSERTs to check that the default resources are compatible with
// the typeIds that they will be used as defaults for.
[[maybe_unused]] bool AreTypeIDCompatible(tint::ResourceType resourceTypeId,
                                          tint::ResourceType slotTypeId) {
    if (resourceTypeId == slotTypeId) {
        return true;
    }

    // Cases where conversions are allowed:
    switch (slotTypeId) {
        case tint::ResourceType::kTexture1d_f32_unfilterable:
            return resourceTypeId == tint::ResourceType::kTexture1d_f32_filterable;

        case tint::ResourceType::kTexture2d_f32_unfilterable:
            return resourceTypeId == tint::ResourceType::kTexture2d_f32_filterable ||
                   resourceTypeId == tint::ResourceType::kTextureDepth2d;

        case tint::ResourceType::kTexture2dArray_f32_unfilterable:
            return resourceTypeId == tint::ResourceType::kTexture2dArray_f32_filterable ||
                   resourceTypeId == tint::ResourceType::kTextureDepth2dArray;

        case tint::ResourceType::kTextureCube_f32_unfilterable:
            return resourceTypeId == tint::ResourceType::kTextureCube_f32_filterable ||
                   resourceTypeId == tint::ResourceType::kTextureDepthCube;

        case tint::ResourceType::kTextureCubeArray_f32_unfilterable:
            return resourceTypeId == tint::ResourceType::kTextureCubeArray_f32_filterable ||
                   resourceTypeId == tint::ResourceType::kTextureDepthCubeArray;

        case tint::ResourceType::kTexture3d_f32_unfilterable:
            return resourceTypeId == tint::ResourceType::kTexture3d_f32_filterable;

        case tint::ResourceType::kSampler_filtering:
            return resourceTypeId == tint::ResourceType::kSampler_non_filtering;

        default:
            return false;
    }
}
}  // namespace

// static
ResultOrError<std::unique_ptr<ResourceTableDefaultResources>> ResourceTableDefaultResources::Create(
    DeviceBase* device) {
    auto defaults = std::make_unique<ResourceTableDefaultResources>();
    DAWN_TRY(defaults->Initialize(device));
    return defaults;
}

// static
ityp::span<ResourceTableSlot, const tint::ResourceType> ResourceTableDefaultResources::GetOrder() {
    return kDefaults;
}

// static
ResourceTableSlot ResourceTableDefaultResources::GetCount() {
    return GetOrder().size();
}

// static
ResourceTableSlot ResourceTableDefaultResources::GetSamplerCount() {
    return kNumDefaultSamplers;
}

// static
ResourceTableSlot ResourceTableDefaultResources::GetNonSamplerCount() {
    return GetCount() - GetSamplerCount();
}

TextureViewBase* ResourceTableDefaultResources::GetPlaceholderSampleableTexture() const {
    return mPlaceholderSampleableTexture.Get();
}

SamplerBase* ResourceTableDefaultResources::GetPlaceholderSampler() const {
    return mPlaceholderSampler.Get();
}

ityp::span<ResourceTableSlot, const ResourceTableDefaultResources::Resource>
ResourceTableDefaultResources::GetResources() const {
    return mDefaultResources;
}

MaybeError ResourceTableDefaultResources::Initialize(DeviceBase* device) {
    // Each resource is added in the order specified by GetOrder()
    auto AddDefaultTextureView = [&](TextureBase* texture, const TextureViewDescriptor* viewDesc =
                                                               nullptr) -> MaybeError {
        Ref<TextureViewBase> view;
        DAWN_TRY_ASSIGN(view, device->CreateTextureView(texture, viewDesc));

        // Check that the resource we will have will match the order of default textures that we
        // will give to the shader compilation.
        DAWN_ASSERT(AreTypeIDCompatible(ResourceTableBase::ComputeTypeId(view),
                                        GetOrder()[mDefaultResources.size()]));
        mDefaultResources.push_back(view);

        return {};
    };

    auto AddDefaultSampler = [&](Ref<SamplerBase> sampler) {
        // Check that the resource we will have will match the order of default textures that we
        // will give to the shader compilation.
        DAWN_ASSERT(AreTypeIDCompatible(ResourceTableBase::ComputeTypeId(sampler),
                                        GetOrder()[mDefaultResources.size()]));
        mDefaultResources.push_back(sampler);
    };

    // Create the color format single-sampled views.
    for (auto format :
         {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Uint, wgpu::TextureFormat::R8Sint}) {
        // Create the necessary 1/2/3D textures.
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {1},
            .format = format,
        };

        tDesc.size = {1};
        tDesc.dimension = wgpu::TextureDimension::e1D;
        Ref<TextureBase> t1D;
        DAWN_TRY_ASSIGN(t1D, device->CreateTexture(&tDesc));

        tDesc.size = {1, 1, 6};
        tDesc.dimension = wgpu::TextureDimension::e2D;
        Ref<TextureBase> t2D;
        DAWN_TRY_ASSIGN(t2D, device->CreateTexture(&tDesc));

        tDesc.size = {1, 1, 1};
        tDesc.dimension = wgpu::TextureDimension::e3D;
        Ref<TextureBase> t3D;
        DAWN_TRY_ASSIGN(t3D, device->CreateTexture(&tDesc));

        // There are two different sets of default resources for R8Unorm because we first add all
        // the filterable f32 sample types, then all the unfilterable f32 sample types.
        uint32_t timesToAdd = format == wgpu::TextureFormat::R8Unorm ? 2 : 1;
        for (uint32_t i = 0; i < timesToAdd; i++) {
            // Create all the default binding view, reusing the 2D texture between
            // 2D/2DArray/Cube/CubeArray.
            DAWN_TRY(AddDefaultTextureView(t1D.Get()));

            TextureViewDescriptor vDesc{
                .label = "default SampledTexture resource",
            };
            vDesc.arrayLayerCount = 1;
            vDesc.dimension = wgpu::TextureViewDimension::e2D;
            DAWN_TRY(AddDefaultTextureView(t2D.Get(), &vDesc));
            vDesc.dimension = wgpu::TextureViewDimension::e2DArray;
            DAWN_TRY(AddDefaultTextureView(t2D.Get(), &vDesc));
            vDesc.arrayLayerCount = 6;
            vDesc.dimension = wgpu::TextureViewDimension::Cube;
            DAWN_TRY(AddDefaultTextureView(t2D.Get(), &vDesc));
            vDesc.dimension = wgpu::TextureViewDimension::CubeArray;
            DAWN_TRY(AddDefaultTextureView(t2D.Get(), &vDesc));

            DAWN_TRY(AddDefaultTextureView(t3D.Get()));
        }

        if (mPlaceholderSampleableTexture == nullptr) {
            DAWN_TRY_ASSIGN(mPlaceholderSampleableTexture,
                            device->CreateTextureView(t2D.Get(), nullptr));
        }
    }

    // Create the color format multi-sampled views.
    for (auto format :
         {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Uint, wgpu::TextureFormat::R8Sint}) {
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {1, 1},
            .format = format,
            .sampleCount = 4,
        };

        Ref<TextureBase> t;
        DAWN_TRY_ASSIGN(t, device->CreateTexture(&tDesc));
        DAWN_TRY(AddDefaultTextureView(t.Get()));
    }

    // Create the single-sampled depth texture default resource.
    {
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {1, 1, 6},
            .format = wgpu::TextureFormat::Depth16Unorm,
        };
        Ref<TextureBase> t;
        DAWN_TRY_ASSIGN(t, device->CreateTexture(&tDesc));

        TextureViewDescriptor vDesc{
            .label = "default SampledTexture resource",
        };
        vDesc.arrayLayerCount = 1;
        vDesc.dimension = wgpu::TextureViewDimension::e2D;
        DAWN_TRY(AddDefaultTextureView(t.Get(), &vDesc));
        vDesc.dimension = wgpu::TextureViewDimension::e2DArray;
        DAWN_TRY(AddDefaultTextureView(t.Get(), &vDesc));
        vDesc.arrayLayerCount = 6;
        vDesc.dimension = wgpu::TextureViewDimension::Cube;
        DAWN_TRY(AddDefaultTextureView(t.Get(), &vDesc));
        vDesc.dimension = wgpu::TextureViewDimension::CubeArray;
        DAWN_TRY(AddDefaultTextureView(t.Get(), &vDesc));
    }

    // Create the multi-sampled depth texture default resource.
    {
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {1, 1},
            .format = wgpu::TextureFormat::Depth16Unorm,
            .sampleCount = 4,
        };

        Ref<TextureBase> t;
        DAWN_TRY_ASSIGN(t, device->CreateTexture(&tDesc));
        DAWN_TRY(AddDefaultTextureView(t.Get()));
    }

    // Create the samplers.
    {
        // Filtering and non-filtering both use a non-filtering sampler
        {
            SamplerDescriptor sDesc{
                .label = "default Sampler resource",
                .magFilter = wgpu::FilterMode::Nearest,
                .minFilter = wgpu::FilterMode::Nearest,
                .mipmapFilter = wgpu::MipmapFilterMode::Nearest,
            };
            Ref<SamplerBase> s;
            DAWN_TRY_ASSIGN(s, device->CreateSampler(&sDesc));
            AddDefaultSampler(s);  // Filtering
            AddDefaultSampler(s);  // Non-filtering

            mPlaceholderSampler = s;
        }
        // Comparison
        {
            SamplerDescriptor sDesc{
                .label = "default Sampler resource",
                .compare = wgpu::CompareFunction::Always,
            };
            Ref<SamplerBase> s;
            DAWN_TRY_ASSIGN(s, device->CreateSampler(&sDesc));
            AddDefaultSampler(s);
        }
    }

    return {};
}

}  // namespace dawn::native
