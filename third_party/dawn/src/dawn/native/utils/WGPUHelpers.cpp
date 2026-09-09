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

#include "src/dawn/native/utils/WGPUHelpers.h"

#include <cstring>
#include <iomanip>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>

#include "src/dawn/common/Constants.h"
#include "src/dawn/native/BindGroup.h"
#include "src/dawn/native/BindGroupLayout.h"
#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/ExternalTexture.h"
#include "src/dawn/native/PipelineLayout.h"
#include "src/dawn/native/Queue.h"
#include "src/dawn/native/Sampler.h"
#include "src/dawn/native/ShaderModule.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn::native::utils {

ResultOrError<Ref<ShaderModuleBase>> CreateShaderModule(
    DeviceBase* device,
    const char* source,
    const std::vector<tint::wgsl::Extension>& internalExtensions) {
    ShaderSourceWGSL wgslDesc;
    wgslDesc.code = source;
    ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device->CreateShaderModule(&descriptor, internalExtensions);
}

ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    std::string_view label,
                                                    wgpu::BufferUsage usage,
                                                    Span<const std::byte> data) {
    BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = data.size();
    descriptor.usage = usage;
    descriptor.mappedAtCreation = true;
    Ref<BufferBase> buffer;
    DAWN_TRY_ASSIGN(buffer, device->CreateBuffer(&descriptor));
    buffer->GetMappedRange().CopyFrom(data);
    DAWN_TRY(buffer->Unmap());
    return buffer;
}

ResultOrError<Ref<PipelineLayoutBase>> MakeBasicPipelineLayout(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& bindGroupLayout,
    uint32_t immediateSize) {
    PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayouts = SpanFromRef<BindGroupIndex>(bindGroupLayout.Get());
    descriptor.immediateSize = immediateSize;
    return device->CreatePipelineLayout(&descriptor);
}

ResultOrError<Ref<BindGroupLayoutBase>> MakeBindGroupLayout(
    DeviceBase* device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer,
    bool allowInternalBinding) {
    std::vector<BindGroupLayoutEntry> entries;
    for (const BindingLayoutEntryInitializationHelper& entry : entriesInitializer) {
        entries.push_back(entry);
    }

    BindGroupLayoutDescriptor descriptor;
    descriptor.entries = entries;
    return device->CreateBindGroupLayout(&descriptor, allowInternalBinding);
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

BindingLayoutEntryInitializationHelper::BindingLayoutEntryInitializationHelper(
    const BindGroupLayoutEntry& entry)
    : BindGroupLayoutEntry(entry) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const Ref<SamplerBase>& sampler)
    : binding(binding), sampler(sampler) {}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const Ref<TextureViewBase>& textureView)
    : binding(binding), textureView(textureView) {}
BindingInitializationHelper::BindingInitializationHelper(
    uint32_t binding,
    const Ref<ExternalTextureBase>& externalTexture)
    : binding(binding), externalTexture(externalTexture) {
    externalBindingEntry.externalTexture = externalTexture.Get();
}

BindingInitializationHelper::BindingInitializationHelper(uint32_t binding,
                                                         const Ref<BufferBase>& buffer,
                                                         uint64_t offset,
                                                         uint64_t size)
    : binding(binding), buffer(buffer), offset(offset), size(size) {}

BindingInitializationHelper::~BindingInitializationHelper() = default;

BindGroupEntry BindingInitializationHelper::GetAsBinding() const {
    BindGroupEntry result;

    result.binding = binding;
    result.sampler = sampler.Get();
    result.textureView = textureView.Get();
    result.buffer = buffer.Get();
    result.offset = offset;
    result.size = size;

    if (externalTexture != nullptr) {
        result.nextInChain = &externalBindingEntry;
    }

    return result;
}

ResultOrError<Ref<BindGroupBase>> MakeBindGroup(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer,
    UsageValidationMode mode) {
    std::vector<BindGroupEntry> entries;
    for (const BindingInitializationHelper& helper : entriesInitializer) {
        entries.push_back(helper.GetAsBinding());
    }

    BindGroupDescriptor descriptor;
    descriptor.layout = layout.Get();
    descriptor.entries = entries;

    return device->CreateBindGroup(&descriptor, mode);
}

const char* GetLabelForTrace(const std::string& label) {
    if (label.length() == 0) {
        return "None";
    }
    return label.c_str();
}

TraceLabel GetLabelForTrace(StringView label) {
    if (label.data == nullptr) {
        return {{}, {}, "None"};
    }
    if (label.length == WGPU_STRLEN) {
        return {{}, {}, label.data};
    }

    TraceLabel result;
    result.storage = {label.data, label.length};
    result.label = result.storage.c_str();
    return result;
}

std::string_view NormalizeMessageString(StringView in) {
    if (in.IsUndefined()) {
        return {};
    }
    return DAWN_UNSAFE_TODO(std::string_view(in.data, strnlen(in.data, in.length)));
}

}  // namespace dawn::native::utils
