// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/utils/WGPUHelpers.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <mutex>
#include <queue>
#include <sstream>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/common/Numeric.h"

namespace dawn::utils {

wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device, const char* source) {
    wgpu::ShaderSourceWGSL wgslDesc;
    wgslDesc.code = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device, const std::string& source) {
    return CreateShaderModule(device, source.c_str());
}

wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  const void* data,
                                  uint64_t size,
                                  wgpu::BufferUsage usage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = size;
    descriptor.usage = usage | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    device.GetQueue().WriteBuffer(buffer, 0, data, size);
    return buffer;
}

ComboRenderPassDescriptor::ComboRenderPassDescriptor(
    const std::vector<wgpu::TextureView>& colorAttachmentInfo,
    wgpu::TextureView depthStencil) {
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        cColorAttachments[i].loadOp = wgpu::LoadOp::Clear;
        cColorAttachments[i].storeOp = wgpu::StoreOp::Store;
        cColorAttachments[i].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
    }

    cDepthStencilAttachmentInfo.depthClearValue = 1.0f;
    cDepthStencilAttachmentInfo.stencilClearValue = 0;
    cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
    cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
    cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
    cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;

    colorAttachmentCount = colorAttachmentInfo.size();
    uint32_t colorAttachmentIndex = 0;
    for (const wgpu::TextureView& colorAttachment : colorAttachmentInfo) {
        if (colorAttachment.Get() != nullptr) {
            cColorAttachments[colorAttachmentIndex].view = colorAttachment;
        }
        ++colorAttachmentIndex;
    }
    colorAttachments = cColorAttachments.data();

    if (depthStencil.Get() != nullptr) {
        cDepthStencilAttachmentInfo.view = depthStencil;
        depthStencilAttachment = &cDepthStencilAttachmentInfo;
    } else {
        depthStencilAttachment = nullptr;
    }
}

ComboRenderPassDescriptor::~ComboRenderPassDescriptor() = default;

ComboRenderPassDescriptor::ComboRenderPassDescriptor(const ComboRenderPassDescriptor& other) {
    *this = other;
}

const ComboRenderPassDescriptor& ComboRenderPassDescriptor::operator=(
    const ComboRenderPassDescriptor& otherRenderPass) {
    cDepthStencilAttachmentInfo = otherRenderPass.cDepthStencilAttachmentInfo;
    cColorAttachments = otherRenderPass.cColorAttachments;
    colorAttachmentCount = otherRenderPass.colorAttachmentCount;

    colorAttachments = cColorAttachments.data();

    if (otherRenderPass.depthStencilAttachment != nullptr) {
        // Assign desc.depthStencilAttachment to this->depthStencilAttachmentInfo;
        depthStencilAttachment = &cDepthStencilAttachmentInfo;
    } else {
        depthStencilAttachment = nullptr;
    }

    return *this;
}
void ComboRenderPassDescriptor::UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth16Unorm:
            cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
            break;
        case wgpu::TextureFormat::Stencil8:
            cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
            cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
            break;
        default:
            break;
    }
}

BasicRenderPass::BasicRenderPass()
    : width(0),
      height(0),
      color(nullptr),
      colorFormat(wgpu::TextureFormat::RGBA8Unorm),
      renderPassInfo() {}

BasicRenderPass::BasicRenderPass(uint32_t texWidth,
                                 uint32_t texHeight,
                                 wgpu::Texture colorAttachment,
                                 wgpu::TextureFormat textureFormat)
    : width(texWidth),
      height(texHeight),
      color(colorAttachment),
      colorFormat(textureFormat),
      renderPassInfo({colorAttachment.CreateView()}) {}

BasicRenderPass CreateBasicRenderPass(const wgpu::Device& device,
                                      uint32_t width,
                                      uint32_t height,
                                      wgpu::TextureFormat format) {
    DAWN_ASSERT(width > 0 && height > 0);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture color = device.CreateTexture(&descriptor);

    return BasicRenderPass(width, height, color, format);
}

wgpu::TexelCopyBufferInfo CreateTexelCopyBufferInfo(wgpu::Buffer buffer,
                                                    uint64_t offset,
                                                    uint32_t bytesPerRow,
                                                    uint32_t rowsPerImage) {
    wgpu::TexelCopyBufferInfo texelCopyBufferInfo = {};
    texelCopyBufferInfo.buffer = buffer;
    texelCopyBufferInfo.layout = CreateTexelCopyBufferLayout(offset, bytesPerRow, rowsPerImage);

    return texelCopyBufferInfo;
}

wgpu::TexelCopyTextureInfo CreateTexelCopyTextureInfo(wgpu::Texture texture,
                                                      uint32_t mipLevel,
                                                      wgpu::Origin3D origin,
                                                      wgpu::TextureAspect aspect) {
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo;
    texelCopyTextureInfo.texture = texture;
    texelCopyTextureInfo.mipLevel = mipLevel;
    texelCopyTextureInfo.origin = origin;
    texelCopyTextureInfo.aspect = aspect;

    return texelCopyTextureInfo;
}

wgpu::TexelCopyBufferLayout CreateTexelCopyBufferLayout(uint64_t offset,
                                                        uint32_t bytesPerRow,
                                                        uint32_t rowsPerImage) {
    wgpu::TexelCopyBufferLayout texelCopyBufferLayout;
    texelCopyBufferLayout.offset = offset;
    texelCopyBufferLayout.bytesPerRow = bytesPerRow;
    texelCopyBufferLayout.rowsPerImage = rowsPerImage;

    return texelCopyBufferLayout;
}

wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                             const wgpu::BindGroupLayout* bindGroupLayout) {
    wgpu::PipelineLayoutDescriptor descriptor;
    if (bindGroupLayout != nullptr) {
        descriptor.bindGroupLayoutCount = 1;
        descriptor.bindGroupLayouts = bindGroupLayout;
    } else {
        descriptor.bindGroupLayoutCount = 0;
        descriptor.bindGroupLayouts = nullptr;
    }
    return device.CreatePipelineLayout(&descriptor);
}

wgpu::PipelineLayout MakePipelineLayout(const wgpu::Device& device,
                                        std::vector<wgpu::BindGroupLayout> bgls) {
    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = uint32_t(bgls.size());
    descriptor.bindGroupLayouts = bgls.data();
    return device.CreatePipelineLayout(&descriptor);
}

wgpu::BindGroupLayout MakeBindGroupLayout(
    const wgpu::Device& device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    for (const BindingLayoutEntryInitializationHelper& entry : entriesInitializer) {
        entries.push_back(entry);
    }

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = entries.size();
    descriptor.entries = entries.data();
    return device.CreateBindGroupLayout(&descriptor);
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::BufferBindingType bufferType,
    bool bufferHasDynamicOffset,
    uint64_t bufferMinBindingSize) {
    binding = entryBinding;
    visibility = entryVisibility;
    buffer.type = bufferType;
    buffer.hasDynamicOffset = bufferHasDynamicOffset;
    buffer.minBindingSize = bufferMinBindingSize;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::SamplerBindingType samplerType) {
    binding = entryBinding;
    visibility = entryVisibility;
    sampler.type = samplerType;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::TextureSampleType textureSampleType,
    wgpu::TextureViewDimension textureViewDimension,
    bool textureMultisampled) {
    binding = entryBinding;
    visibility = entryVisibility;
    texture.sampleType = textureSampleType;
    texture.viewDimension = textureViewDimension;
    texture.multisampled = textureMultisampled;
}

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::StorageTextureAccess storageTextureAccess,
    wgpu::TextureFormat format,
    wgpu::TextureViewDimension textureViewDimension) {
    binding = entryBinding;
    visibility = entryVisibility;
    storageTexture.access = storageTextureAccess;
    storageTexture.format = format;
    storageTexture.viewDimension = textureViewDimension;
}

#ifndef __EMSCRIPTEN__
// ExternalTextureBindingLayout never contains data, so just make one that can be reused instead
// of declaring a new one every time it's needed.
wgpu::ExternalTextureBindingLayout kExternalTextureBindingLayout = {};

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    uint32_t entryBinding,
    wgpu::ShaderStage entryVisibility,
    wgpu::ExternalTextureBindingLayout* bindingLayout) {
    binding = entryBinding;
    visibility = entryVisibility;
    nextInChain = bindingLayout;
}

BindingInitializationHelper::BindingInitializationHelper(
    uint32_t binding,
    const wgpu::ExternalTexture& externalTexture)
    : binding(binding) {
    externalTextureBindingEntry.externalTexture = externalTexture;
}
#endif  // __EMSCRIPTEN__

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    const wgpu::BindGroupLayoutEntry& entry)
    : wgpu::BindGroupLayoutEntry(entry) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const wgpu::Sampler& sampler)
    : binding(binding), sampler(sampler) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const wgpu::TextureView& textureView)
    : binding(binding), textureView(textureView) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const wgpu::Buffer& buffer,
                                                         uint64_t offset,
                                                         uint64_t size)
    : binding(binding), buffer(buffer), offset(offset), size(size) {}

BindingInitializationHelper::BindingInitializationHelper(const BindingInitializationHelper&) =
    default;

BindingInitializationHelper::~BindingInitializationHelper() = default;

wgpu::BindGroupEntry BindingInitializationHelper::GetAsBinding() const {
    wgpu::BindGroupEntry result;

    result.binding = binding;
    result.sampler = sampler;
    result.textureView = textureView;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
#ifndef __EMSCRIPTEN__
    if (externalTextureBindingEntry.externalTexture != nullptr) {
        result.nextInChain = &externalTextureBindingEntry;
    }
#endif  // __EMSCRIPTEN__

    return result;
}

wgpu::BindGroup MakeBindGroup(
    const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer) {
    std::vector<wgpu::BindGroupEntry> entries;
    for (const BindingInitializationHelper& helper : entriesInitializer) {
        entries.push_back(helper.GetAsBinding());
    }

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = checked_cast<uint32_t>(entries.size());
    descriptor.entries = entries.data();

    return device.CreateBindGroup(&descriptor);
}

ColorSpaceConversionInfo GetYUVBT709ToRGBSRGBColorSpaceConversionInfo() {
    ColorSpaceConversionInfo info;
    info.yuvToRgbConversionMatrix = {1.164384f, 0.0f,       1.792741f,  -0.972945f,
                                     1.164384f, -0.213249f, -0.532909f, 0.301483f,
                                     1.164384f, 2.112402f,  0.0f,       -1.133402f};
    info.gamutConversionMatrix = {1.0f, 0.0f, 0.0f,  //
                                  0.0f, 1.0f, 0.0f,  //
                                  0.0f, 0.0f, 1.0f};
    info.srcTransferFunctionParameters = {2.2, 1.0 / 1.099, 0.099 / 1.099, 1 / 4.5, 0.081,
                                          0.0, 0.0};
    info.dstTransferFunctionParameters = {1 / 2.4, 1.137119, 0.0, 12.92, 0.0031308, -0.055, 0.0};
    return info;
}

ColorSpaceConversionInfo GetNoopRGBColorSpaceConversionInfo() {
    ColorSpaceConversionInfo info;

    // YUV to RGB is not used as the data is RGB.
    info.yuvToRgbConversionMatrix = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // Identity gamut conversion matrix.
    info.gamutConversionMatrix = {1.0f, 0.0f, 0.0f,  //
                                  0.0f, 1.0f, 0.0f,  //
                                  0.0f, 0.0f, 1.0f};

    // Set A = G = 1 and everything else to 0 to turn the code below into pow(x, 1) which is x
    //
    //    if (abs(v) < params.D) {
    //        return sign(v) * (params.C * abs(v) + params.F);
    //    }
    //    return pow(A * x + B, G) + E
    //
    // Note that for some reason the order of the data is G A B C D E F
    info.srcTransferFunctionParameters = {1, 1, 0, 0, 0, 0, 0};
    info.dstTransferFunctionParameters = {1, 1, 0, 0, 0, 0, 0};

    return info;
}

bool BackendRequiresCompat(wgpu::BackendType backend) {
    switch (backend) {
        case wgpu::BackendType::D3D12:
        case wgpu::BackendType::Metal:
        case wgpu::BackendType::Vulkan:
        case wgpu::BackendType::WebGPU:
        case wgpu::BackendType::Null:
            return false;
        case wgpu::BackendType::D3D11:
        case wgpu::BackendType::OpenGL:
        case wgpu::BackendType::OpenGLES:
            return true;
        case wgpu::BackendType::Undefined:
            DAWN_UNREACHABLE();
    }
}

const absl::flat_hash_map<wgpu::FeatureName, absl::flat_hash_set<wgpu::FeatureName>>
    kImplicitlyEnabledFeaturesMap = {
        {wgpu::FeatureName::TextureFormatsTier1, {wgpu::FeatureName::RG11B10UfloatRenderable}},
        {wgpu::FeatureName::TextureFormatsTier2, {wgpu::FeatureName::TextureFormatsTier1}},
        // Add other implicit enabling rules here
};

absl::flat_hash_set<wgpu::FeatureName> FeatureAndImplicitlyEnabled(wgpu::FeatureName featureName) {
    absl::flat_hash_set<wgpu::FeatureName> allFeatures;
    std::queue<wgpu::FeatureName> q;

    q.push(featureName);
    allFeatures.insert(featureName);

    const auto& implicitMap = kImplicitlyEnabledFeaturesMap;

    while (!q.empty()) {
        wgpu::FeatureName current = q.front();
        q.pop();

        auto it = implicitMap.find(current);
        if (it != implicitMap.end()) {
            for (wgpu::FeatureName implicitlyEnabled : it->second) {
                if (allFeatures.insert(implicitlyEnabled).second) {
                    q.push(implicitlyEnabled);
                }
            }
        }
    }

    return allFeatures;
}

int8_t ConvertFloatToSnorm8(float value) {
    float roundedValue = (value >= 0) ? (value + 0.5f) : (value - 0.5f);
    float clampedValue = std::clamp(roundedValue, -128.0f, 127.0f);
    return static_cast<int8_t>(clampedValue);
}

int16_t ConvertFloatToSnorm16(float value) {
    float roundedValue = (value >= 0) ? (value + 0.5f) : (value - 0.5f);
    float clampedValue = std::clamp(roundedValue, -32768.0f, 32767.0f);
    return static_cast<int16_t>(clampedValue);
}

uint16_t ConvertFloatToUnorm16(float value) {
    float roundedValue = value + 0.5f;
    float clampedValue = std::clamp(roundedValue, 0.0f, 65535.0f);
    return static_cast<uint16_t>(clampedValue);
}

}  // namespace dawn::utils
