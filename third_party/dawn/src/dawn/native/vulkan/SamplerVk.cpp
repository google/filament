// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/SamplerVk.h"

#include <algorithm>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "vulkan/vulkan_core.h"

namespace dawn::native::vulkan {

namespace {
VkSamplerAddressMode VulkanSamplerAddressMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case wgpu::AddressMode::MirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case wgpu::AddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case wgpu::AddressMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkSamplerMipmapMode VulkanMipMapMode(wgpu::MipmapFilterMode filter) {
    switch (filter) {
        case wgpu::MipmapFilterMode::Linear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case wgpu::MipmapFilterMode::Nearest:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case wgpu::MipmapFilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}
}  // anonymous namespace

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    Ref<Sampler> sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize(descriptor));
    return sampler;
}

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const StaticSamplerSpecialization& s) {
    SamplerDescriptor samplerDesc;
    samplerDesc.magFilter = s.magFilter;
    samplerDesc.minFilter = s.minFilter;

    YCbCrVkDescriptor yCbCrDesc;
    if (s.isYCbCr) {
        yCbCrDesc = StaticSamplerSpecialization::GetYCbCrForTextureView(s.vkFormat,
                                                                        s.androidExternalFormat);
        samplerDesc.nextInChain = &yCbCrDesc;
    }

    // Create the Sampler, skipping validation: the frontend requires the YCbCrVulkanSamplers
    // feature enabled to be able to use YCbCrVkDescriptor. However the
    // OpaqueYCbCrAndroidForExternalTexture can be used without YCbCrVulkanSamplers, in which case
    // a validation error is emitted.
    Ref<SamplerBase> sampler;
    DAWN_TRY_ASSIGN(sampler, device->CreateSampler(&samplerDesc, ValidationMode::Skip));
    device->CacheStaticSampler(ToBackend(sampler));
    return ToBackend(sampler);
}

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.magFilter = ToVulkanSamplerFilter(descriptor->magFilter);
    createInfo.minFilter = ToVulkanSamplerFilter(descriptor->minFilter);
    createInfo.mipmapMode = VulkanMipMapMode(descriptor->mipmapFilter);
    createInfo.addressModeU = VulkanSamplerAddressMode(descriptor->addressModeU);
    createInfo.addressModeV = VulkanSamplerAddressMode(descriptor->addressModeV);
    createInfo.addressModeW = VulkanSamplerAddressMode(descriptor->addressModeW);
    createInfo.mipLodBias = 0.0f;
    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        createInfo.compareOp = ToVulkanCompareOp(descriptor->compare);
        createInfo.compareEnable = VK_TRUE;
    } else {
        // Still set the compareOp so it's not garbage.
        createInfo.compareOp = VK_COMPARE_OP_NEVER;
        createInfo.compareEnable = VK_FALSE;
    }
    createInfo.minLod = descriptor->lodMinClamp;
    createInfo.maxLod = descriptor->lodMaxClamp;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    Device* device = ToBackend(GetDevice());
    uint16_t maxAnisotropy = GetMaxAnisotropy();
    if (device->GetDeviceInfo().features.samplerAnisotropy == VK_TRUE && maxAnisotropy > 1) {
        createInfo.anisotropyEnable = VK_TRUE;
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
        createInfo.maxAnisotropy =
            std::min(static_cast<float>(maxAnisotropy),
                     device->GetDeviceInfo().properties.limits.maxSamplerAnisotropy);
    } else {
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1;
    }

    VkSamplerYcbcrConversionInfo samplerYCbCrInfo = {};
    if (IsYCbCr()) {
        DAWN_TRY_ASSIGN(mSamplerYCbCrConversion,
                        CreateSamplerYCbCrConversion(device, GetYCbCrVkDescriptor()));

        samplerYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        samplerYCbCrInfo.pNext = nullptr;
        samplerYCbCrInfo.conversion = mSamplerYCbCrConversion;

        createInfo.pNext = &samplerYCbCrInfo;
    }

    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateSampler(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreateSampler"));

    SetLabelImpl();

    return {};
}

Sampler::~Sampler() = default;

void Sampler::DestroyImpl(DestroyReason reason) {
    SamplerBase::DestroyImpl(reason);
    if (mSamplerYCbCrConversion != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mSamplerYCbCrConversion);
        mSamplerYCbCrConversion = VK_NULL_HANDLE;
    }
    if (mHandle != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

const VkSampler& Sampler::GetHandle() const {
    return mHandle;
}

void Sampler::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_Sampler", GetLabel());
}

// static
StaticSamplerSpecialization StaticSamplerSpecialization::From(const TextureView* view,
                                                              const Sampler* sampler) {
    bool isYCbCr =
        view->GetTexture()->GetFormat().format == wgpu::TextureFormat::OpaqueYCbCrAndroid;
    bool filterable = !isYCbCr || view->IsYCbCrFilterable();

    StaticSamplerSpecialization spec = {
        .minFilter = (sampler != nullptr && filterable) ? sampler->GetMinFilter()
                                                        : wgpu::FilterMode::Nearest,
        .magFilter = (sampler != nullptr && filterable) ? sampler->GetMagFilter()
                                                        : wgpu::FilterMode::Nearest,
        .isYCbCr = isYCbCr,

        // Put default values for the YCbCr part.
        .vkFormat = VK_FORMAT_UNDEFINED,
        .androidExternalFormat = 0,
    };

    if (isYCbCr) {
        auto stm = static_cast<SharedTextureMemoryContentsVk*>(
            view->GetTexture()->GetSharedResourceMemoryContents());
        DAWN_ASSERT(stm != nullptr);

        spec.vkFormat = static_cast<VkFormat>(stm->GetYCbCrVkDesc().vkFormat);
        spec.androidExternalFormat = stm->GetYCbCrVkDesc().externalFormat;
    }

    return spec;
}

// static
YCbCrVkDescriptor StaticSamplerSpecialization::GetYCbCrForTextureView(
    VkFormat vkFormat,
    uint32_t androidExternalFormat) {
    YCbCrVkDescriptor yCbCrDesc;
    yCbCrDesc.vkFormat = vkFormat;
    yCbCrDesc.externalFormat = androidExternalFormat;
    yCbCrDesc.vkYCbCrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    yCbCrDesc.vkYCbCrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    yCbCrDesc.vkComponentSwizzleRed = VK_COMPONENT_SWIZZLE_R;
    yCbCrDesc.vkComponentSwizzleGreen = VK_COMPONENT_SWIZZLE_G;
    yCbCrDesc.vkComponentSwizzleBlue = VK_COMPONENT_SWIZZLE_B;
    yCbCrDesc.vkComponentSwizzleAlpha = VK_COMPONENT_SWIZZLE_A;
    yCbCrDesc.vkXChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    yCbCrDesc.vkYChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    yCbCrDesc.vkChromaFilter = wgpu::FilterMode::Nearest;
    yCbCrDesc.forceExplicitReconstruction = false;
    return yCbCrDesc;
}

}  // namespace dawn::native::vulkan
