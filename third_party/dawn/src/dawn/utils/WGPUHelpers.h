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

#ifndef SRC_DAWN_UTILS_WGPUHELPERS_H_
#define SRC_DAWN_UTILS_WGPUHELPERS_H_

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <initializer_list>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/Constants.h"
#include "dawn/utils/TextureUtils.h"

namespace dawn::utils {

enum Expectation { Success, Failure };

wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device, const char* source);
wgpu::ShaderModule CreateShaderModule(const wgpu::Device& device, const std::string& source);

wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  const void* data,
                                  uint64_t size,
                                  wgpu::BufferUsage usage);

template <typename T>
wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  wgpu::BufferUsage usage,
                                  std::initializer_list<T> data) {
    return CreateBufferFromData(device, data.begin(), uint32_t(sizeof(T) * data.size()), usage);
}

wgpu::TexelCopyBufferInfo CreateTexelCopyBufferInfo(
    wgpu::Buffer buffer,
    uint64_t offset = 0,
    uint32_t bytesPerRow = wgpu::kCopyStrideUndefined,
    uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);
wgpu::TexelCopyTextureInfo CreateTexelCopyTextureInfo(
    wgpu::Texture texture,
    uint32_t level = 0,
    wgpu::Origin3D origin = {0, 0, 0},
    wgpu::TextureAspect aspect = wgpu::TextureAspect::All);
wgpu::TexelCopyBufferLayout CreateTexelCopyBufferLayout(
    uint64_t offset,
    uint32_t bytesPerRow,
    uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);

struct ComboRenderPassDescriptor : public wgpu::RenderPassDescriptor {
  public:
    ComboRenderPassDescriptor(const std::vector<wgpu::TextureView>& colorAttachmentInfo = {},
                              wgpu::TextureView depthStencil = wgpu::TextureView());
    ~ComboRenderPassDescriptor();

    ComboRenderPassDescriptor(const ComboRenderPassDescriptor& otherRenderPass);
    const ComboRenderPassDescriptor& operator=(const ComboRenderPassDescriptor& otherRenderPass);

    void UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat format);

    std::array<wgpu::RenderPassColorAttachment, kMaxColorAttachments> cColorAttachments;
    wgpu::RenderPassDepthStencilAttachment cDepthStencilAttachmentInfo = {};
};

struct BasicRenderPass {
  public:
    BasicRenderPass();
    BasicRenderPass(uint32_t width,
                    uint32_t height,
                    wgpu::Texture color,
                    wgpu::TextureFormat texture = kDefaultColorFormat);

    static constexpr wgpu::TextureFormat kDefaultColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    uint32_t width;
    uint32_t height;
    wgpu::Texture color;
    wgpu::TextureFormat colorFormat;
    ComboRenderPassDescriptor renderPassInfo;
};
BasicRenderPass CreateBasicRenderPass(
    const wgpu::Device& device,
    uint32_t width,
    uint32_t height,
    wgpu::TextureFormat format = BasicRenderPass::kDefaultColorFormat);

wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                             const wgpu::BindGroupLayout* bindGroupLayout);

wgpu::PipelineLayout MakePipelineLayout(const wgpu::Device& device,
                                        std::vector<wgpu::BindGroupLayout> bgls);

#ifndef __EMSCRIPTEN__
extern wgpu::ExternalTextureBindingLayout kExternalTextureBindingLayout;
#endif  // __EMSCRIPTEN__

// Helpers to make creating bind group layouts look nicer:
//
//   dawn::utils::MakeBindGroupLayout(device, {
//       {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
//       {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
//       {3, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}
//   });

struct BindingLayoutEntryInitializationHelper : wgpu::BindGroupLayoutEntry {
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
#ifndef __EMSCRIPTEN__
    BindingLayoutEntryInitializationHelper(uint32_t entryBinding,
                                           wgpu::ShaderStage entryVisibility,
                                           wgpu::ExternalTextureBindingLayout* bindingLayout);
#endif  // __EMSCRIPTEN__

    // NOLINTNEXTLINE(runtime/explicit)
    BindingLayoutEntryInitializationHelper(const wgpu::BindGroupLayoutEntry& entry);
};

wgpu::BindGroupLayout MakeBindGroupLayout(
    const wgpu::Device& device,
    std::initializer_list<BindingLayoutEntryInitializationHelper> entriesInitializer);

// Helpers to make creating bind groups look nicer:
//
//   dawn::utils::MakeBindGroup(device, layout, {
//       {0, mySampler},
//       {1, myBuffer, offset, size},
//       {3, myTextureView}
//   });

// Structure with one constructor per-type of bindings, so that the initializer_list accepts
// bindings with the right type and no extra information.
struct BindingInitializationHelper {
    BindingInitializationHelper(uint32_t binding, const wgpu::Sampler& sampler);
    BindingInitializationHelper(uint32_t binding, const wgpu::TextureView& textureView);
#ifndef __EMSCRIPTEN__
    BindingInitializationHelper(uint32_t binding, const wgpu::ExternalTexture& externalTexture);
#endif  // __EMSCRIPTEN__
    BindingInitializationHelper(uint32_t binding,
                                const wgpu::Buffer& buffer,
                                uint64_t offset = 0,
                                uint64_t size = wgpu::kWholeSize);
    BindingInitializationHelper(const BindingInitializationHelper&);
    ~BindingInitializationHelper();

    wgpu::BindGroupEntry GetAsBinding() const;

    uint32_t binding;
    wgpu::Sampler sampler;
    wgpu::TextureView textureView;
    wgpu::Buffer buffer;
#ifndef __EMSCRIPTEN__
    wgpu::ExternalTextureBindingEntry externalTextureBindingEntry;
#endif  // __EMSCRIPTEN__
    uint64_t offset = 0;
    uint64_t size = 0;
};

wgpu::BindGroup MakeBindGroup(
    const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer);

struct ColorSpaceConversionInfo {
    std::array<float, 12> yuvToRgbConversionMatrix;
    std::array<float, 9> gamutConversionMatrix;
    std::array<float, 7> srcTransferFunctionParameters;
    std::array<float, 7> dstTransferFunctionParameters;
};
ColorSpaceConversionInfo GetYUVBT709ToRGBSRGBColorSpaceConversionInfo();
ColorSpaceConversionInfo GetNoopRGBColorSpaceConversionInfo();

bool BackendRequiresCompat(wgpu::BackendType backend);

absl::flat_hash_set<wgpu::FeatureName> FeatureAndImplicitlyEnabled(wgpu::FeatureName featureName);

int8_t ConvertFloatToSnorm8(float value);

int16_t ConvertFloatToSnorm16(float value);

uint16_t ConvertFloatToUnorm16(float value);

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_WGPUHELPERS_H_
