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

#ifndef SRC_DAWN_NATIVE_UTILS_WGPUHELPERS_H_
#define SRC_DAWN_NATIVE_UTILS_WGPUHELPERS_H_

#include <array>
#include <initializer_list>
#include <string>
#include <vector>

#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Error.h"
#include "dawn/native/UsageValidationMode.h"
#include "dawn/native/dawn_platform.h"

namespace tint::wgsl {
enum class Extension : uint8_t;
}

namespace dawn::native::utils {

ResultOrError<Ref<ShaderModuleBase>> CreateShaderModule(
    DeviceBase* device,
    const char* source,
    const std::vector<tint::wgsl::Extension>& internalExtensions = {});

ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    std::string_view label,
                                                    wgpu::BufferUsage usage,
                                                    const void* data,
                                                    uint64_t size);

template <typename T>
ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    wgpu::BufferUsage usage,
                                                    std::initializer_list<T> data) {
    return CreateBufferFromData(device, "", usage, data.begin(), uint32_t(sizeof(T) * data.size()));
}
template <typename T>
ResultOrError<Ref<BufferBase>> CreateBufferFromData(DeviceBase* device,
                                                    std::string_view label,
                                                    wgpu::BufferUsage usage,
                                                    std::initializer_list<T> data) {
    return CreateBufferFromData(device, label, usage, data.begin(),
                                uint32_t(sizeof(T) * data.size()));
}

ResultOrError<Ref<PipelineLayoutBase>> MakeBasicPipelineLayout(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& bindGroupLayout);

// Helpers to make creating bind group layouts look nicer:
//
//   utils::MakeBindGroupLayout(device, {
//       {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
//       {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
//       {3, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}
//   });

struct BindingLayoutEntryInitializationHelper : BindGroupLayoutEntry {
    BindingLayoutEntryInitializationHelper(uint32_t entryBinding,
                                           wgpu::ShaderStage entryVisibility,
                                           wgpu::BufferBindingType bufferType,
                                           bool bufferHasDynamicOffset = false,
                                           uint64_t bufferMinBindingSize = 0);
    BindingLayoutEntryInitializationHelper(uint32_t entryBinding,
                                           wgpu::ShaderStage entryVisibility,
                                           wgpu::SamplerBindingType samplerType);
    BindingLayoutEntryInitializationHelper(
        uint32_t entryBinding,
        wgpu::ShaderStage entryVisibility,
        wgpu::TextureSampleType textureSampleType,
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D,
        bool textureMultisampled = false);
    BindingLayoutEntryInitializationHelper(
        uint32_t entryBinding,
        wgpu::ShaderStage entryVisibility,
        wgpu::StorageTextureAccess storageTextureAccess,
        wgpu::TextureFormat format,
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D);

    explicit BindingLayoutEntryInitializationHelper(const BindGroupLayoutEntry& entry);
};

ResultOrError<Ref<BindGroupLayoutBase>> MakeBindGroupLayout(
    DeviceBase* device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer,
    bool allowInternalBinding = false);

// Helpers to make creating bind groups look nicer:
//
//   utils::MakeBindGroup(device, layout, {
//       {0, mySampler},
//       {1, myBuffer, offset, size},
//       {3, myTextureView}
//   });

// Structure with one constructor per-type of bindings, so that the initializer_list accepts
// bindings with the right type and no extra information.
struct BindingInitializationHelper {
    BindingInitializationHelper(uint32_t binding, const Ref<SamplerBase>& sampler);
    BindingInitializationHelper(uint32_t binding, const Ref<TextureViewBase>& textureView);
    BindingInitializationHelper(uint32_t binding, const Ref<ExternalTextureBase>& externalTexture);
    BindingInitializationHelper(uint32_t binding,
                                const Ref<BufferBase>& buffer,
                                uint64_t offset = 0,
                                uint64_t size = wgpu::kWholeSize);
    ~BindingInitializationHelper();

    BindGroupEntry GetAsBinding() const;

    uint32_t binding;
    Ref<SamplerBase> sampler;
    Ref<TextureViewBase> textureView;
    Ref<BufferBase> buffer;
    Ref<ExternalTextureBase> externalTexture;
    ExternalTextureBindingEntry externalBindingEntry;
    uint64_t offset = 0;
    uint64_t size = 0;
};

// This helper is only used inside dawn native.
ResultOrError<Ref<BindGroupBase>> MakeBindGroup(
    DeviceBase* device,
    const Ref<BindGroupLayoutBase>& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer,
    UsageValidationMode mode);

// Converts a label to be nice for TraceEvent calls. Might perform a copy if the string isn't
// null-terminated as TraceEvent only supports const char*
struct TraceLabel : public NonCopyable {
    std::string storage;
    const char* label;
};
TraceLabel GetLabelForTrace(StringView label);
const char* GetLabelForTrace(const std::string& label);

// Given a std vector, allocate an equivalent array that can be returned in an API's foos/fooCount
// pair of fields. The apiData must eventually be freed using FreeApiSeq.
template <typename T>
void AllocateApiSeqFromStdVector(const T** apiData, size_t* apiSize, const std::vector<T>& vector) {
    size_t size = vector.size();
    *apiSize = size;

    if (size > 0) {
        T* mutableData = new T[size];
        memcpy(mutableData, vector.data(), size * sizeof(T));
        *apiData = mutableData;
    } else {
        *apiData = nullptr;
    }
}

// Free an API sequence that was allocated by AllocateApiSeqFromStdVector
template <typename T>
void FreeApiSeq(T** apiData, size_t* apiSize) {
    delete[] *apiData;
    *apiData = nullptr;
    *apiSize = 0;
}

// Normalize the string, truncating it at the first null-terminator, if any.
std::string_view NormalizeMessageString(StringView in);

}  // namespace dawn::native::utils

#endif  // SRC_DAWN_NATIVE_UTILS_WGPUHELPERS_H_
