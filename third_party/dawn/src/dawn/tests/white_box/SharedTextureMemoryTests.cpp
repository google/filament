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

#include "dawn/tests/white_box/SharedTextureMemoryTests.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

using testing::SizedStringMatches;

namespace dawn {

namespace {

struct BackendBeginStateVk : public SharedTextureMemoryTestBackend::BackendBeginState {
    wgpu::SharedTextureMemoryVkImageLayoutBeginState imageLayouts{};
};

struct BackendEndStateVk : public SharedTextureMemoryTestBackend::BackendEndState {
    wgpu::SharedTextureMemoryVkImageLayoutEndState imageLayouts{};
};

}  // anonymous namespace

std::unique_ptr<SharedTextureMemoryTestBackend::BackendBeginState>
SharedTextureMemoryTestVulkanBackend::ChainInitialBeginState(
    wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc) {
    auto state = std::make_unique<BackendBeginStateVk>();
    beginDesc->nextInChain = &state->imageLayouts;
    return state;
}

std::unique_ptr<SharedTextureMemoryTestBackend::BackendEndState>
SharedTextureMemoryTestVulkanBackend::ChainEndState(
    wgpu::SharedTextureMemoryEndAccessState* endState) {
    auto state = std::make_unique<BackendEndStateVk>();
    endState->nextInChain = &state->imageLayouts;
    return state;
}

std::unique_ptr<SharedTextureMemoryTestBackend::BackendBeginState>
SharedTextureMemoryTestVulkanBackend::ChainBeginState(
    wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc,
    const wgpu::SharedTextureMemoryEndAccessState& endState) {
    DAWN_ASSERT(endState.nextInChain != nullptr);
    DAWN_ASSERT(endState.nextInChain->sType ==
                wgpu::SType::SharedTextureMemoryVkImageLayoutEndState);
    auto* vkEndState =
        static_cast<wgpu::SharedTextureMemoryVkImageLayoutEndState*>(endState.nextInChain);

    auto state = std::make_unique<BackendBeginStateVk>();
    state->imageLayouts.oldLayout = vkEndState->oldLayout;
    state->imageLayouts.newLayout = vkEndState->newLayout;
    beginDesc->nextInChain = &state->imageLayouts;
    return state;
}

void SharedTextureMemoryNoFeatureTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<SharedTextureMemoryTestParams>::SetUp();
    GetParam().mBackend->SetUp(device);
}

std::vector<wgpu::FeatureName> SharedTextureMemoryTests::GetRequiredFeatures() {
    auto features = GetParam().mBackend->RequiredFeatures(GetAdapter().Get());
    if (!SupportsFeatures(features)) {
        return {};
    }

    const wgpu::FeatureName kOptionalFeatures[] = {
        wgpu::FeatureName::MultiPlanarFormatExtendedUsages,
        wgpu::FeatureName::MultiPlanarRenderTargets,
        wgpu::FeatureName::TransientAttachments,
        wgpu::FeatureName::Unorm16TextureFormats,
        wgpu::FeatureName::BGRA8UnormStorage,
        wgpu::FeatureName::FlexibleTextureViews,
    };
    for (auto feature : kOptionalFeatures) {
        if (SupportsFeatures({feature})) {
            features.push_back(feature);
        }
    }

    return features;
}  // namespace dawn

void SharedTextureMemoryTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<SharedTextureMemoryTestParams>::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(
        !SupportsFeatures(GetParam().mBackend->RequiredFeatures(GetAdapter().Get())));
    // TODO(crbug.com/342213634): Crashes on ChromeOS volteer devices.
    // TODO(crbug.com/407561933): Triggers dawn validation errors
    DAWN_SUPPRESS_TEST_IF(IsChromeOS() && IsVulkan() && IsIntel());

    // Compat cannot create 2D texture view from a 2D array texture.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode() &&
                             !SupportsFeatures({wgpu::FeatureName::FlexibleTextureViews}) &&
                             GetParam().mLayerCount > 1);

    GetParam().mBackend->SetUp(device);
}

void SharedTextureMemoryNoFeatureTests::TearDown() {
    DawnTestWithParams<SharedTextureMemoryTestParams>::TearDown();
    GetParam().mBackend->TearDown();
}

void SharedTextureMemoryTests::TearDown() {
    DawnTestWithParams<SharedTextureMemoryTestParams>::TearDown();
    GetParam().mBackend->TearDown();
}

wgpu::SharedFence SharedTextureMemoryTestBackend::ImportFenceTo(const wgpu::Device& importingDevice,
                                                                const wgpu::SharedFence& fence) {
    wgpu::SharedFenceExportInfo exportInfo;
    fence.ExportInfo(&exportInfo);

    switch (exportInfo.type) {
        case wgpu::SharedFenceType::VkSemaphoreOpaqueFD: {
            wgpu::SharedFenceVkSemaphoreOpaqueFDExportInfo vkExportInfo;
            exportInfo.nextInChain = &vkExportInfo;
            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceVkSemaphoreOpaqueFDDescriptor vkDesc;
            vkDesc.handle = vkExportInfo.handle;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &vkDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        case wgpu::SharedFenceType::SyncFD: {
            wgpu::SharedFenceSyncFDExportInfo vkExportInfo;
            exportInfo.nextInChain = &vkExportInfo;
            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceSyncFDDescriptor vkDesc;
            vkDesc.handle = vkExportInfo.handle;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &vkDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        case wgpu::SharedFenceType::VkSemaphoreZirconHandle: {
            wgpu::SharedFenceVkSemaphoreZirconHandleExportInfo vkExportInfo;
            exportInfo.nextInChain = &vkExportInfo;
            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceVkSemaphoreZirconHandleDescriptor vkDesc;
            vkDesc.handle = vkExportInfo.handle;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &vkDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        case wgpu::SharedFenceType::DXGISharedHandle: {
            wgpu::SharedFenceDXGISharedHandleExportInfo dxgiExportInfo;
            exportInfo.nextInChain = &dxgiExportInfo;
            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceDXGISharedHandleDescriptor dxgiDesc;
            dxgiDesc.handle = dxgiExportInfo.handle;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &dxgiDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        case wgpu::SharedFenceType::MTLSharedEvent: {
            wgpu::SharedFenceMTLSharedEventExportInfo sharedEventInfo;
            exportInfo.nextInChain = &sharedEventInfo;

            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceMTLSharedEventDescriptor sharedEventDesc;
            sharedEventDesc.sharedEvent = sharedEventInfo.sharedEvent;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &sharedEventDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        case wgpu::SharedFenceType::EGLSync: {
            wgpu::SharedFenceEGLSyncExportInfo eglSyncInfo;
            exportInfo.nextInChain = &eglSyncInfo;

            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceEGLSyncDescriptor eglSyncDesc;
            eglSyncDesc.sync = eglSyncInfo.sync;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &eglSyncDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        default:
            DAWN_UNREACHABLE();
    }
}

std::vector<wgpu::SharedTextureMemory> SharedTextureMemoryTestBackend::CreateSharedTextureMemories(
    wgpu::Device& device,
    int layerCount) {
    std::vector<wgpu::SharedTextureMemory> memories;
    for (auto& memory : CreatePerDeviceSharedTextureMemories({device}, layerCount)) {
        DAWN_ASSERT(memory.size() == 1u);
        memories.push_back(std::move(memory[0]));
    }
    // There should be at least one memory to test.
    DAWN_ASSERT(!memories.empty());
    return memories;
}

wgpu::Texture CreateWriteTexture(wgpu::SharedTextureMemory memory) {
    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::TextureDescriptor writeTextureDesc = {};
    writeTextureDesc.format = properties.format;
    writeTextureDesc.size = properties.size;
    writeTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    writeTextureDesc.label = "write texture";

    return memory.CreateTexture(&writeTextureDesc);
}

wgpu::Texture CreateReadTexture(wgpu::SharedTextureMemory memory) {
    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::TextureDescriptor readTextureDesc = {};
    readTextureDesc.format = properties.format;
    readTextureDesc.size = properties.size;
    readTextureDesc.usage = wgpu::TextureUsage::TextureBinding;
    readTextureDesc.label = "read texture";

    return memory.CreateTexture(&readTextureDesc);
}

std::vector<wgpu::SharedTextureMemory>
SharedTextureMemoryTestBackend::CreateSinglePlanarSharedTextureMemories(wgpu::Device& device,
                                                                        int layerCount) {
    std::vector<wgpu::SharedTextureMemory> out;
    for (auto& memory : CreateSharedTextureMemories(device, layerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        if (utils::IsMultiPlanarFormat(properties.format)) {
            continue;
        }

        out.push_back(std::move(memory));
    }
    return out;
}

std::vector<std::vector<wgpu::SharedTextureMemory>>
SharedTextureMemoryTestBackend::CreatePerDeviceSharedTextureMemoriesFilterByUsage(
    const std::vector<wgpu::Device>& devices,
    wgpu::TextureUsage requiredUsage,
    int layerCount) {
    std::vector<std::vector<wgpu::SharedTextureMemory>> out;
    for (auto& memories : CreatePerDeviceSharedTextureMemories(devices, layerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memories[0].GetProperties(&properties);

        if ((properties.usage & requiredUsage) == requiredUsage) {
            // Tests using RenderAttachment will get a TextureView from the
            // texture. This currently doesn't work with multiplanar textures. The
            // superficial problem is that the plane would need to be passed for
            // multiplanar formats, and the deep problem is that the tests fail to
            // create valid backing textures for some multiplanar formats (e.g.,
            // on Apple), which results in a crash when accessing plane 0.
            // TODO(crbug.com/dawn/2263): Fix this and remove this short-circuit.
            if (utils::IsMultiPlanarFormat(properties.format) &&
                (requiredUsage &
                 (wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding))) {
                continue;
            }
            out.push_back(std::move(memories));
        }
    }
    return out;
}

wgpu::Device SharedTextureMemoryTests::CreateDevice() {
    if (GetParam().mBackend->UseSameDevice()) {
        return device;
    }
    return DawnTestBase::CreateDevice();
}

void SharedTextureMemoryTests::UseInRenderPass(wgpu::Device& deviceObj, wgpu::Texture& texture) {
    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();

    for (int layer = 0; layer < GetParam().mLayerCount; ++layer) {
        wgpu::TextureViewDescriptor desc;
        desc.dimension = wgpu::TextureViewDimension::e2D;
        desc.baseArrayLayer = layer;
        desc.arrayLayerCount = 1;
        utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView(&desc)});
        passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
        passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.End();
    }
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    deviceObj.GetQueue().Submit(1, &commandBuffer);
}

void SharedTextureMemoryTests::UseInCopy(wgpu::Device& deviceObj, wgpu::Texture& texture) {
    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    wgpu::TexelCopyTextureInfo source;
    source.texture = texture;

    // Create a destination buffer, large enough for 1 texel of any format.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 128;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst;

    wgpu::TexelCopyBufferInfo destination;
    destination.buffer = deviceObj.CreateBuffer(&bufferDesc);

    wgpu::Extent3D size = {1, 1, 1};
    for (int layer = 0; layer < GetParam().mLayerCount; ++layer) {
        source.origin.z = layer;
        encoder.CopyTextureToBuffer(&source, &destination, &size);
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    deviceObj.GetQueue().Submit(1, &commandBuffer);
}

// Make a command buffer that clears the texture to four different colors in each quadrant.
wgpu::CommandBuffer SharedTextureMemoryTests::MakeFourColorsClearCommandBuffer(
    wgpu::Device& deviceObj,
    wgpu::Texture& texture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(deviceObj, R"(
      struct VertexOut {
          @builtin(position) position : vec4f,
          @location(0) uv : vec2f,
      }

      struct FragmentIn {
          @location(0) uv : vec2f,
      }

      @vertex fn vert_main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
          let pos = array(
            vec2( 1.0,  1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0, -1.0),
            vec2( 1.0,  1.0),
            vec2(-1.0, -1.0),
            vec2(-1.0,  1.0),
          );

          let uv = array(
            vec2(1.0, 0.0),
            vec2(1.0, 1.0),
            vec2(0.0, 1.0),
            vec2(1.0, 0.0),
            vec2(0.0, 1.0),
            vec2(0.0, 0.0),
          );
          return VertexOut(vec4f(pos[VertexIndex], 0.0, 1.0), uv[VertexIndex]);
      }

      @fragment fn frag_main(in: FragmentIn) -> @location(0) vec4f {
          if (in.uv.x < 0.5) {
            if (in.uv.y < 0.5) {
              return vec4f(0.0, 1.0, 0.0, 0.501);
            } else {
              return vec4f(1.0, 0.0, 0.0, 0.501);
            }
          } else {
            if (in.uv.y < 0.5) {
              return vec4f(0.0, 0.0, 1.0, 0.501);
            } else {
              return vec4f(1.0, 1.0, 0.0, 0.501);
            }
          }
      }
    )");

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cTargets[0].format = texture.GetFormat();

    wgpu::RenderPipeline pipeline = deviceObj.CreateRenderPipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseArrayLayer = layer;
        viewDesc.arrayLayerCount = 1;
        utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView(&viewDesc)});
        passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.Draw(6);
        pass.End();
    }
    return encoder.Finish();
}

// Make a command buffer that clears the texture to four different colors in each quadrant.
wgpu::CommandBuffer SharedTextureMemoryTests::MakeFourColorsComputeCommandBuffer(
    wgpu::Device& deviceObj,
    wgpu::Texture& texture) {
    std::string wgslFormat = utils::GetWGSLImageFormatQualifier(texture.GetFormat());

    std::string shader = R"(
      @group(0) @binding(0) var storageImage : texture_storage_2d<)" +
                         wgslFormat + R"(, write>;

      @workgroup_size(1)
      @compute fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
          let dims = textureDimensions(storageImage);
          if (global_id.x < dims.x / 2) {
            if (global_id.y < dims.y / 2) {
              textureStore(storageImage, global_id.xy, vec4f(0.0, 1.0, 0.0, 0.501));
            } else {
              textureStore(storageImage, global_id.xy, vec4f(1.0, 0.0, 0.0, 0.501));
            }
          } else {
            if (global_id.y < dims.y / 2) {
              textureStore(storageImage, global_id.xy, vec4f(0.0, 0.0, 1.0, 0.501));
            } else {
              textureStore(storageImage, global_id.xy, vec4f(1.0, 1.0, 0.0, 0.501));
            }
          }
      }
    )";
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = utils::CreateShaderModule(deviceObj, shader.c_str());

    wgpu::ComputePipeline pipeline = deviceObj.CreateComputePipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
        wgpu::TextureViewDescriptor desc;
        desc.dimension = wgpu::TextureViewDimension::e2D;
        desc.baseArrayLayer = layer;
        desc.arrayLayerCount = 1;
        pass.SetBindGroup(0, utils::MakeBindGroup(deviceObj, pipeline.GetBindGroupLayout(0),
                                                  {{0, texture.CreateView(&desc)}}));
        pass.DispatchWorkgroups(texture.GetWidth(), texture.GetHeight());
    }
    pass.End();
    return encoder.Finish();
}

// Use queue.writeTexture to write four different colors in each quadrant to the texture.
void SharedTextureMemoryTests::WriteFourColorsToRGBA8Texture(wgpu::Device& deviceObj,
                                                             wgpu::Texture& texture) {
    DAWN_ASSERT(texture.GetFormat() == wgpu::TextureFormat::RGBA8Unorm);

    uint32_t width = texture.GetWidth();
    uint32_t height = texture.GetHeight();

    uint32_t bytesPerBlock = utils::GetTexelBlockSizeInBytes(texture.GetFormat());
    uint32_t bytesPerRow = width * bytesPerBlock;
    uint32_t size = bytesPerRow * height;

    std::vector<uint8_t> pixels(size);

    constexpr utils::RGBA8 kTopLeft(0, 0xFF, 0, 0x80);
    constexpr utils::RGBA8 kBottomLeft(0xFF, 0, 0, 0x80);
    constexpr utils::RGBA8 kTopRight(0, 0, 0xFF, 0x80);
    constexpr utils::RGBA8 kBottomRight(0xFF, 0xFF, 0, 0x80);

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            utils::RGBA8* pixel =
                reinterpret_cast<utils::RGBA8*>(&pixels[y * bytesPerRow + x * bytesPerBlock]);
            if (x < width / 2) {
                if (y < height / 2) {
                    *pixel = kTopLeft;
                } else {
                    *pixel = kBottomLeft;
                }
            } else {
                if (y < height / 2) {
                    *pixel = kTopRight;
                } else {
                    *pixel = kBottomRight;
                }
            }
        }
    }

    wgpu::Extent3D writeSize = {width, height, 1};

    wgpu::TexelCopyTextureInfo dest;
    dest.texture = texture;

    wgpu::TexelCopyBufferLayout dataLayout = {
        .offset = 0, .bytesPerRow = bytesPerRow, .rowsPerImage = height};

    for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
        dest.origin.z = layer;
        device.GetQueue().WriteTexture(&dest, pixels.data(), pixels.size(), &dataLayout,
                                       &writeSize);
    }
}

// Make a command buffer that samples the contents of the input texture into an RGBA8Unorm texture.
std::pair<wgpu::CommandBuffer, wgpu::Texture>
SharedTextureMemoryTests::MakeCheckBySamplingCommandBuffer(wgpu::Device& deviceObj,
                                                           wgpu::Texture& texture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(deviceObj, R"(
      @vertex fn vert_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
          let pos = array(
            vec2( 1.0,  1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0, -1.0),
            vec2( 1.0,  1.0),
            vec2(-1.0, -1.0),
            vec2(-1.0,  1.0),
          );
          return vec4f(pos[VertexIndex], 0.0, 1.0);
      }

      @group(0) @binding(0) var t: texture_2d<f32>;

      @fragment fn frag_main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4f {
        return textureLoad(t, vec2u(coord_in.xy), 0);
      }
    )");

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    textureDesc.size = {texture.GetWidth(), texture.GetHeight(), 1};
    textureDesc.label = "intermediate check texture";

    wgpu::Texture colorTarget = deviceObj.CreateTexture(&textureDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cTargets[0].format = colorTarget.GetFormat();

    wgpu::RenderPipeline pipeline = deviceObj.CreateRenderPipeline(&pipelineDesc);

    wgpu::TextureViewDescriptor desc;
    desc.dimension = wgpu::TextureViewDimension::e2D;
    desc.baseArrayLayer = 0;
    desc.arrayLayerCount = 1;
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(deviceObj, pipeline.GetBindGroupLayout(0),
                                                     {{0, texture.CreateView(&desc)}});

    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({colorTarget.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();
    return {encoder.Finish(), colorTarget};
}

// Make a command buffer that samples the contents of the input texture into an RGBA8Unorm texture.
std::pair<wgpu::CommandBuffer, wgpu::Texture>
SharedTextureMemoryTests::MakeCheckBySamplingTwoTexturesCommandBuffer(wgpu::Texture& texture0,
                                                                      wgpu::Texture& texture1) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
      @vertex fn vert_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
          let pos = array(
            vec2( 1.0,  1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0, -1.0),
            vec2( 1.0,  1.0),
            vec2(-1.0, -1.0),
            vec2(-1.0,  1.0),
          );
          return vec4f(pos[VertexIndex], 0.0, 1.0);
      }

      @group(0) @binding(0) var t0: texture_2d<f32>;
      @group(0) @binding(1) var t1: texture_2d<f32>;

      @fragment fn frag_main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4f {
        return (textureLoad(t0, vec2u(coord_in.xy), 0) / 2) +
               (textureLoad(t1, vec2u(coord_in.xy), 0) / 2);
      }
    )");

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    textureDesc.size = {texture0.GetWidth(), texture0.GetHeight(), 1};
    textureDesc.label = "intermediate check texture";

    wgpu::Texture colorTarget = device.CreateTexture(&textureDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cTargets[0].format = colorTarget.GetFormat();

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::TextureViewDescriptor desc;
    desc.dimension = wgpu::TextureViewDimension::e2D;
    desc.baseArrayLayer = 0;
    desc.arrayLayerCount = 1;

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, texture0.CreateView(&desc)}, {1, texture1.CreateView(&desc)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({colorTarget.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();
    return {encoder.Finish(), colorTarget};
}

// Make a command buffer that samples the contents of the input texture 2d array into an RGBA8Unorm
// texture.
std::pair<wgpu::CommandBuffer, wgpu::Texture>
SharedTextureMemoryTests::MakeCheckBySamplingTexture2DArrayCommandBuffer(wgpu::Device& deviceObj,
                                                                         wgpu::Texture& texture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(deviceObj, R"(
      @vertex fn vert_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
          let pos = array(
            vec2( 1.0,  1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0, -1.0),
            vec2( 1.0,  1.0),
            vec2(-1.0, -1.0),
            vec2(-1.0,  1.0),
          );
          return vec4f(pos[VertexIndex], 0.0, 1.0);
      }

      @group(0) @binding(0) var t: texture_2d_array<f32>;

      @fragment fn frag_main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4f {
        return (textureLoad(t, vec2u(coord_in.xy), 0, 0) / 2) +
               (textureLoad(t, vec2u(coord_in.xy), 1, 0) / 2);
      }
    )");

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    textureDesc.size = {texture.GetWidth(), texture.GetHeight(), 1};
    textureDesc.label = "intermediate check texture";

    wgpu::Texture colorTarget = deviceObj.CreateTexture(&textureDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cTargets[0].format = colorTarget.GetFormat();

    wgpu::RenderPipeline pipeline = deviceObj.CreateRenderPipeline(&pipelineDesc);

    wgpu::TextureViewDescriptor desc;
    desc.dimension = wgpu::TextureViewDimension::e2DArray;
    desc.baseArrayLayer = 0;
    desc.arrayLayerCount = texture.GetDepthOrArrayLayers();
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(deviceObj, pipeline.GetBindGroupLayout(0),
                                                     {{0, texture.CreateView(&desc)}});

    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({colorTarget.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();
    return {encoder.Finish(), colorTarget};
}

// Check that the contents of colorTarget are RGBA8Unorm texels that match those written by
// MakeFourColorsClearCommandBuffer.
void SharedTextureMemoryTests::CheckFourColors(wgpu::Device& deviceObj,
                                               wgpu::TextureFormat format,
                                               wgpu::Texture& colorTarget) {
    wgpu::Origin3D tl = {colorTarget.GetWidth() / 4, colorTarget.GetHeight() / 4};
    wgpu::Origin3D bl = {colorTarget.GetWidth() / 4, 3 * colorTarget.GetHeight() / 4};
    wgpu::Origin3D tr = {3 * colorTarget.GetWidth() / 4, colorTarget.GetHeight() / 4};
    wgpu::Origin3D br = {3 * colorTarget.GetWidth() / 4, 3 * colorTarget.GetHeight() / 4};

    std::array<utils::RGBA8, 4> expectedColors;
    uint8_t expectedAlpha;
    switch (format) {
        case wgpu::TextureFormat::RGB10A2Unorm:
            expectedColors = {
                utils::RGBA8::kGreen,
                utils::RGBA8::kRed,
                utils::RGBA8::kBlue,
                utils::RGBA8::kYellow,
            };
            expectedAlpha = 0xAA;
            break;
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::RGBA16Float:
            expectedColors = {
                utils::RGBA8::kGreen,
                utils::RGBA8::kRed,
                utils::RGBA8::kBlue,
                utils::RGBA8::kYellow,
            };
            expectedAlpha = 0x80;
            break;
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG8Unorm:
            expectedColors = {
                utils::RGBA8::kGreen,
                utils::RGBA8::kRed,
                utils::RGBA8::kBlack,
                utils::RGBA8::kYellow,
            };
            expectedAlpha = 0xFF;
            break;
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R8Unorm:
            expectedColors = {
                utils::RGBA8::kBlack,
                utils::RGBA8::kRed,
                utils::RGBA8::kBlack,
                utils::RGBA8::kRed,
            };
            expectedAlpha = 0xFF;
            break;
        default:
            DAWN_UNREACHABLE();
    }
    expectedColors[0].a = expectedAlpha;
    expectedColors[1].a = expectedAlpha;
    expectedColors[2].a = expectedAlpha;
    expectedColors[3].a = expectedAlpha;

    EXPECT_TEXTURE_EQ(deviceObj, &expectedColors[0], colorTarget, tl, {1, 1});
    EXPECT_TEXTURE_EQ(deviceObj, &expectedColors[1], colorTarget, bl, {1, 1});
    EXPECT_TEXTURE_EQ(deviceObj, &expectedColors[2], colorTarget, tr, {1, 1});
    EXPECT_TEXTURE_EQ(deviceObj, &expectedColors[3], colorTarget, br, {1, 1});
}

// Allow tests to be uninstantiated since it's possible no backends are available.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryNoFeatureTests);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryTests);

namespace {

using testing::HasSubstr;
using testing::MockCppCallback;

template <typename T>
T& AsNonConst(const T& rhs) {
    return const_cast<T&>(rhs);
}

// Test that creating shared texture memory without the required features is an error.
// Using the memory thereafter produces errors.
TEST_P(SharedTextureMemoryNoFeatureTests, CreationWithoutFeature) {
    // Create external texture memories with an error filter.
    // We should see a message that the feature is not enabled.
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    const auto& memories =
        GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount);

    MockCppCallback<void (*)(wgpu::PopErrorScopeStatus, wgpu::ErrorType, wgpu::StringView)>
        popErrorScopeCallback;
    EXPECT_CALL(popErrorScopeCallback,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::Validation,
                     SizedStringMatches(HasSubstr("is not enabled"))));

    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, popErrorScopeCallback.Callback());

    for (wgpu::SharedTextureMemory memory : memories) {
        ASSERT_DEVICE_ERROR_MSG(wgpu::Texture texture = memory.CreateTexture(),
                                HasSubstr("is invalid"));

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = true;

        ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(texture, &beginDesc)),
                                HasSubstr("is invalid"));

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture, &endState)),
                                HasSubstr("is invalid"));
    }
}

// Test that it is an error to import a shared texture memory with no chained struct.
TEST_P(SharedTextureMemoryTests, ImportSharedTextureMemoryNoChain) {
    wgpu::SharedTextureMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(
        wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc),
        HasSubstr("chain"));
}

// Test that it is an error to import a shared fence with no chained struct.
// Also test that ExportInfo reports an Undefined type for the error fence.
TEST_P(SharedTextureMemoryTests, ImportSharedFenceNoChain) {
    wgpu::SharedFenceDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&desc),
                            HasSubstr("chain"));

    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.type = static_cast<wgpu::SharedFenceType>(1234);  // should be overrwritten

    // Expect that exporting the fence info writes Undefined, and generates an error.
    ASSERT_DEVICE_ERROR(fence.ExportInfo(&exportInfo));
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType(0));
}

// Test importing a shared texture memory when the device is destroyed
TEST_P(SharedTextureMemoryTests, ImportSharedTextureMemoryDeviceDestroyed) {
    device.Destroy();

    wgpu::SharedTextureMemory memory;
    if (GetParam().mBackend->Name().rfind("OpaqueFD", 0) == 0) {
        // The OpaqueFD backend for `CreateSharedTextureMemory` uses several
        // Vulkan device internals before and after the actual call to
        // ImportSharedTextureMemory. We can't easily make it import with
        // a destroyed device, so create the SharedTextureMemory with
        // an invalid descriptor instead. This still tests that an uncaptured
        // error is not generated on the import call when the device is lost.
        wgpu::SharedTextureMemoryDescriptor desc;
        memory = device.ImportSharedTextureMemory(&desc);
    } else {
        memory = GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);
    }

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    // That the begin access does not succeed since the device is destroyed.
    EXPECT_FALSE(memory.BeginAccess(memory.CreateTexture(), &beginDesc));
}

// Test that SharedTextureMemory::IsDeviceLost() returns the expected value before and
// after destroying the device.
TEST_P(SharedTextureMemoryTests, CheckIsDeviceLostBeforeAndAfterDestroyingDevice) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    EXPECT_FALSE(memory.IsDeviceLost());
    device.Destroy();
    EXPECT_TRUE(memory.IsDeviceLost());
}

// Test that SharedTextureMemory::IsDeviceLost() returns the expected value before and
// after losing the device.
TEST_P(SharedTextureMemoryTests, CheckIsDeviceLostBeforeAndAfterLosingDevice) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    EXPECT_FALSE(memory.IsDeviceLost());
    LoseDeviceForTesting(device);
    EXPECT_TRUE(memory.IsDeviceLost());
}

// Test importing a shared fence when the device is destroyed
TEST_P(SharedTextureMemoryTests, ImportSharedFenceDeviceDestroyed) {
    // Create a shared texture memory and texture
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);
    wgpu::Texture texture = memory.CreateTexture();

    // Begin access to use the texture
    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
    EXPECT_TRUE(memory.BeginAccess(texture, &beginDesc));

    // Use the texture so there is a fence to export on end access.
    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);
    if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
        UseInRenderPass(device, texture);
    } else if (!utils::IsMultiPlanarFormat(properties.format)) {
        if (properties.usage & wgpu::TextureUsage::CopySrc) {
            UseInCopy(device, texture);
        } else if (properties.usage & wgpu::TextureUsage::CopyDst) {
            wgpu::Extent3D writeSize = {1, 1, 1};
            wgpu::TexelCopyTextureInfo dest = {};
            dest.texture = texture;
            wgpu::TexelCopyBufferLayout dataLayout = {};
            uint64_t data[2];
            device.GetQueue().WriteTexture(&dest, &data, sizeof(data), &dataLayout, &writeSize);
        }
    }

    // End access to export a fence.
    wgpu::SharedTextureMemoryEndAccessState endState = {};
    auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
    EXPECT_TRUE(memory.EndAccess(texture, &endState));

    // Destroy the device.
    device.Destroy();

    // Import the shared fence to the destroyed device.
    std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
    for (size_t i = 0; i < endState.fenceCount; ++i) {
        sharedFences[i] = GetParam().mBackend->ImportFenceTo(device, endState.fences[i]);
    }
    beginDesc.fenceCount = endState.fenceCount;
    beginDesc.fences = sharedFences.data();
    beginDesc.signaledValues = endState.signaledValues;
    beginDesc.concurrentRead = false;
    beginDesc.initialized = endState.initialized;
    backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

    // Begin access should fail.
    EXPECT_FALSE(memory.BeginAccess(texture, &beginDesc));
}

// Test calling GetProperties with an error memory. The properties are filled with 0/None/Undefined.
TEST_P(SharedTextureMemoryTests, GetPropertiesErrorMemory) {
    wgpu::SharedTextureMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR(wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc));

    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    EXPECT_EQ(properties.usage, wgpu::TextureUsage::None);
    EXPECT_EQ(properties.size.width, 0u);
    EXPECT_EQ(properties.size.height, 0u);
    EXPECT_EQ(properties.size.depthOrArrayLayers, 0u);
    EXPECT_EQ(properties.format, wgpu::TextureFormat::Undefined);
}

// Tests that a SharedTextureMemory supports expected texture usages.
TEST_P(SharedTextureMemoryTests, TextureUsages) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        // CopySrc and TextureBinding should always be supported.
        // TODO(crbug.com/dawn/2262): TextureBinding support on D3D11/D3D12/Vulkan is actually
        // dependent on the flags passed to the underlying texture (the relevant
        // flag is currently always passed in the test context). Add tests where
        // the D3D/Vulkan texture is not created with the relevant flag.
        wgpu::TextureUsage expectedUsage =
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;

        bool isSinglePlanar = !utils::IsMultiPlanarFormat(properties.format);

        if (isSinglePlanar ||
            device.HasFeature(wgpu::FeatureName::MultiPlanarFormatExtendedUsages)) {
            expectedUsage |= wgpu::TextureUsage::CopyDst;
        }

        // TODO(crbug.com/dawn/2262): RenderAttachment support on D3D11/D3D12/Vulkan is
        // additionally dependent on the flags passed to the underlying
        // texture (the relevant flag is currently always passed in the test
        // context). Add tests where the D3D/Vulkan texture is not created with the
        // relevant flag.
        if ((isSinglePlanar || device.HasFeature(wgpu::FeatureName::MultiPlanarRenderTargets)) &&
            utils::IsRenderableFormat(device, properties.format)) {
            expectedUsage |= wgpu::TextureUsage::RenderAttachment;
        }

        // TODO(crbug.com/dawn/2262): StorageBinding support on D3D11/D3D12/Vulkan is
        // additionally dependent on the flags passed to the underlying
        // texture (the relevant flag is currently always passed in the test
        // context). Add tests where the D3D/Vulkan texture is not created with the
        // relevant flag.
#if !DAWN_PLATFORM_IS(ANDROID)
        if (isSinglePlanar && utils::TextureFormatSupportsStorageTexture(properties.format, device,
                                                                         IsCompatibilityMode())) {
            expectedUsage |= wgpu::TextureUsage::StorageBinding;
        }
#endif
        EXPECT_EQ(properties.usage, expectedUsage) << properties.format;
    }
}

// Test calling GetProperties with an invalid chained struct. An error is
// generated, but the properties are still populated.
TEST_P(SharedTextureMemoryTests, GetPropertiesInvalidChain) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::ChainedStructOut otherStruct;
    wgpu::SharedTextureMemoryProperties properties1;
    properties1.nextInChain = &otherStruct;
    ASSERT_DEVICE_ERROR(memory.GetProperties(&properties1));

    wgpu::SharedTextureMemoryProperties properties2;
    memory.GetProperties(&properties2);

    EXPECT_EQ(properties1.usage, properties2.usage);
    EXPECT_EQ(properties1.size.width, properties2.size.width);
    EXPECT_EQ(properties1.size.height, properties2.size.height);
    EXPECT_EQ(properties1.size.depthOrArrayLayers, properties2.size.depthOrArrayLayers);
    EXPECT_EQ(properties1.format, properties2.format);
}

// Test that calling GetProperties with a chained
// SharedTextureMemoryAHardwareBufferProperties struct will generate an error
// unless the required feature is present. In either case, the base properties
// should still be populated.
TEST_P(SharedTextureMemoryTests, GetPropertiesAHardwareBufferPropertiesRequiresAHBFeature) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::SharedTextureMemoryAHardwareBufferProperties aHBProps;
    wgpu::SharedTextureMemoryProperties properties1;
    properties1.nextInChain = &aHBProps;
    if (device.HasFeature(wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer)) {
        memory.GetProperties(&properties1);
    } else {
        ASSERT_DEVICE_ERROR(memory.GetProperties(&properties1));
    }

    wgpu::SharedTextureMemoryProperties properties2;
    memory.GetProperties(&properties2);

    EXPECT_EQ(properties1.usage, properties2.usage);
    EXPECT_EQ(properties1.size.width, properties2.size.width);
    EXPECT_EQ(properties1.size.height, properties2.size.height);
    EXPECT_EQ(properties1.size.depthOrArrayLayers, properties2.size.depthOrArrayLayers);
    EXPECT_EQ(properties1.format, properties2.format);
}

// Test that texture usages must be a subset of the shared texture memory's usage.
TEST_P(SharedTextureMemoryTests, UsageValidation) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        // SharedTextureMemory should never support TransientAttachment.
        ASSERT_EQ(properties.usage & wgpu::TextureUsage::TransientAttachment, 0);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.size = properties.size;

        for (wgpu::TextureUsage usage : {
                 wgpu::TextureUsage::CopySrc,
                 wgpu::TextureUsage::CopyDst,
                 wgpu::TextureUsage::TextureBinding,
                 wgpu::TextureUsage::StorageBinding,
                 wgpu::TextureUsage::RenderAttachment,
             }) {
            textureDesc.usage = usage;

            // `usage` is valid if it is in the shared texture memory properties.
            if (usage & properties.usage) {
                wgpu::Texture t = memory.CreateTexture(&textureDesc);
                EXPECT_EQ(t.GetUsage(), usage);
            } else {
                ASSERT_DEVICE_ERROR(memory.CreateTexture(&textureDesc));
            }
        }
    }
}

// Test that it is an error if the texture format doesn't match the shared texture memory.
TEST_P(SharedTextureMemoryTests, FormatValidation) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format != wgpu::TextureFormat::RGBA8Unorm
                                 ? wgpu::TextureFormat::RGBA8Unorm
                                 : wgpu::TextureFormat::RGBA16Float;
        textureDesc.size = properties.size;
        textureDesc.usage = properties.usage;

        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor format"));
    }
}

// Test that it is an error if the texture size doesn't match the shared texture memory.
TEST_P(SharedTextureMemoryTests, SizeValidation) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;

        textureDesc.size = {properties.size.width + 1, properties.size.height,
                            properties.size.depthOrArrayLayers};
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor size"));

        textureDesc.size = {properties.size.width, properties.size.height + 1,
                            properties.size.depthOrArrayLayers};
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor size"));

        textureDesc.size = {properties.size.width, properties.size.height,
                            properties.size.depthOrArrayLayers + 1};
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor size"));
    }
}

// Test that it is an error if the texture mip level count is not 1.
TEST_P(SharedTextureMemoryTests, MipLevelValidation) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;
        textureDesc.size = properties.size;
        textureDesc.mipLevelCount = 1u;

        memory.CreateTexture(&textureDesc);

        textureDesc.mipLevelCount = 2u;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc), HasSubstr("(2) is not 1"));
    }
}

// Test that it is an error if the texture sample count is not 1.
TEST_P(SharedTextureMemoryTests, SampleCountValidation) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;
        textureDesc.size = properties.size;
        textureDesc.sampleCount = 1u;

        memory.CreateTexture(&textureDesc);

        textureDesc.sampleCount = 4u;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc), HasSubstr("(4) is not 1"));
    }
}

// Test that it is an error if the texture dimension is not 2D.
TEST_P(SharedTextureMemoryTests, DimensionValidation) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;
        textureDesc.size = properties.size;

        textureDesc.dimension = wgpu::TextureDimension::e1D;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("is not TextureDimension::e2D"));

        textureDesc.dimension = wgpu::TextureDimension::e3D;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("is not TextureDimension::e2D"));
    }
}

// Test that it is an error to call BeginAccess twice in a row on the same texture and memory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccess) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    // It should be an error to BeginAccess twice in a row.
    EXPECT_TRUE(memory.BeginAccess(texture, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(texture, &beginDesc)),
                            HasSubstr("is already used to access"));
}

// Test that it is an error to call BeginAccess concurrently on a write texture
// followed by a read texture on a single SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesWriteRead) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture writeTexture = CreateWriteTexture(memory);
    wgpu::Texture readTexture = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(writeTexture, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(readTexture, &beginDesc)),
                            HasSubstr("is currently accessed for writing"));
}

// Test that it is an error to call BeginAccess concurrently on a write texture
// followed by a read texture on a single SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesWriteConcurrentRead) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());

    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture writeTexture = CreateWriteTexture(memory);
    wgpu::Texture readTexture = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(writeTexture, &beginDesc));
    beginDesc.concurrentRead = true;
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(readTexture, &beginDesc)),
                            HasSubstr("is currently accessed for writing"));
}

// Test that it is an error to call BeginAccess concurrently on a read texture
// followed by a write texture on a single SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesReadWrite) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture writeTexture = CreateWriteTexture(memory);
    wgpu::Texture readTexture = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(readTexture, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(writeTexture, &beginDesc)),
                            HasSubstr("is currently accessed for exclusive reading"));
}

// Test that it is an error to call BeginAccess concurrently on a read texture
// followed by a write texture on a single SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesConcurrentReadWrite) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());

    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture writeTexture = CreateWriteTexture(memory);
    wgpu::Texture readTexture = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = true;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(readTexture, &beginDesc));
    beginDesc.concurrentRead = false;
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(writeTexture, &beginDesc)),
                            HasSubstr("is currently accessed for reading."));
}

// Test that it is an error to call BeginAccess concurrently on two write textures on a single
// SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesWriteWrite) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture writeTexture1 = CreateWriteTexture(memory);
    wgpu::Texture writeTexture2 = CreateWriteTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(writeTexture1, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(writeTexture2, &beginDesc)),
                            HasSubstr("is currently accessed for writing"));
}

// Test that it is valid to call BeginAccess concurrently on two read textures on a single
// SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesReadRead) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture readTexture1 = CreateReadTexture(memory);
    wgpu::Texture readTexture2 = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(readTexture1, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(readTexture2, &beginDesc)),
                            HasSubstr("is currently accessed for exclusive reading"));
}

// Test that it is valid to call BeginAccess concurrently on two read textures on a single
// SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesConcurrentReadConcurrentRead) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());

    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture readTexture1 = CreateReadTexture(memory);
    wgpu::Texture readTexture2 = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = true;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(readTexture1, &beginDesc));
    EXPECT_TRUE(memory.BeginAccess(readTexture2, &beginDesc));

    wgpu::SharedTextureMemoryEndAccessState endState1 = {};
    EXPECT_TRUE(memory.EndAccess(readTexture1, &endState1));
    wgpu::SharedTextureMemoryEndAccessState endState2 = {};
    EXPECT_TRUE(memory.EndAccess(readTexture2, &endState2));
}

// Test that it is valid to call BeginAccess concurrently on read textures on a single
// SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesConcurrentReadRead) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture readTexture1 = CreateReadTexture(memory);
    wgpu::Texture readTexture2 = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    beginDesc.concurrentRead = true;
    EXPECT_TRUE(memory.BeginAccess(readTexture1, &beginDesc));
    beginDesc.concurrentRead = false;
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(readTexture2, &beginDesc)),
                            HasSubstr("is currently accessed for reading."));
}

// Test that it is valid to call BeginAccess concurrently on read textures on a single
// SharedTextureMemory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTexturesReadConcurrentRead) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture readTexture1 = CreateReadTexture(memory);
    wgpu::Texture readTexture2 = CreateReadTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    beginDesc.concurrentRead = false;
    EXPECT_TRUE(memory.BeginAccess(readTexture1, &beginDesc));
    beginDesc.concurrentRead = true;
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(readTexture2, &beginDesc)),
                            HasSubstr("is currently accessed for exclusive reading."));
}

// Test that it is valid to call BeginAccess concurrently on write textures with concurrentRead is
// true.
TEST_P(SharedTextureMemoryTests, ConcurrentWrite) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());

    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::Texture writeTexture = CreateWriteTexture(memory);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    beginDesc.concurrentRead = true;
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(writeTexture, &beginDesc)),
                            HasSubstr("Concurrent reading read-write"));
}

// Test that it is an error to call EndAccess twice in a row on the same memory.
TEST_P(SharedTextureMemoryTests, DoubleEndAccess) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(texture, &beginDesc));

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
    EXPECT_TRUE(memory.EndAccess(texture, &endState));

    // Invalid to end access a second time.
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture, &endState)),
                            HasSubstr("is not currently being accessed"));
}

// Test that it is an error to call EndAccess on a texture that was not the one BeginAccess was
// called on.
TEST_P(SharedTextureMemoryTests, BeginThenEndOnDifferentTexture) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);
    wgpu::Texture texture1 = memory.CreateTexture();
    wgpu::Texture texture2 = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;
    auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

    EXPECT_TRUE(memory.BeginAccess(texture1, &beginDesc));

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture2, &endState)),
                            HasSubstr("is not currently being accessed"));
}

// Test that it is an error to call EndAccess without a preceding BeginAccess.
TEST_P(SharedTextureMemoryTests, EndAccessWithoutBegin) {
    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture, &endState)),
                            HasSubstr("is not currently being accessed"));
}

// Test that it is an error to use the texture on the queue without a preceding BeginAccess.
TEST_P(SharedTextureMemoryTests, UseWithoutBegin) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::SharedTextureMemory memory =
        GetParam().mBackend->CreateSharedTextureMemory(device, GetParam().mLayerCount);

    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::Texture texture = memory.CreateTexture();

    if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
        ASSERT_DEVICE_ERROR_MSG(UseInRenderPass(device, texture),
                                HasSubstr("without current access"));
    } else if (!utils::IsMultiPlanarFormat(properties.format)) {
        if (properties.usage & wgpu::TextureUsage::CopySrc) {
            ASSERT_DEVICE_ERROR_MSG(UseInCopy(device, texture),
                                    HasSubstr("without current access"));
        }
        if (properties.usage & wgpu::TextureUsage::CopyDst) {
            wgpu::Extent3D writeSize = {1, 1, 1};
            wgpu::TexelCopyTextureInfo dest = {};
            dest.texture = texture;
            wgpu::TexelCopyBufferLayout dataLayout = {};
            uint64_t data[2];
            ASSERT_DEVICE_ERROR_MSG(
                device.GetQueue().WriteTexture(&dest, &data, sizeof(data), &dataLayout, &writeSize),
                HasSubstr("without current access"));
        }
    }
}

// Test that it is valid (does not crash) if the memory is dropped while a texture access has begun.
TEST_P(SharedTextureMemoryTests, TextureAccessOutlivesMemory) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    // NOTE: UseInRenderPass()/UseInCopy() do not currently support multiplanar
    // formats.
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSinglePlanarSharedTextureMemories(device,
                                                                      GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = true;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

        // Begin access on a texture, and drop the memory.
        wgpu::Texture texture = memory.CreateTexture();
        memory.BeginAccess(texture, &beginDesc);
        memory = nullptr;

        // Use the texture on the GPU; it should not crash.
        if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
            UseInRenderPass(device, texture);
        } else {
            DAWN_ASSERT(properties.usage & wgpu::TextureUsage::CopySrc);
            UseInCopy(device, texture);
        }
    }
}

// Test that if the texture is uninitialized, it is cleared on first use.
TEST_P(SharedTextureMemoryTests, UninitializedTextureIsCleared) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        // Skipped for multiplanar formats because those must be initialized on import.
        // We also need render attachment usage to initially populate the texture.
        if (utils::IsMultiPlanarFormat(properties.format) ||
            (properties.usage & wgpu::TextureUsage::RenderAttachment) == 0) {
            continue;
        }

        // Helper function to test that unintialized textures are lazy cleared upon use.
        auto DoTest = [&](auto UseTexture) {
            wgpu::Texture texture = memory.CreateTexture();

            wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
            auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

            wgpu::SharedTextureMemoryEndAccessState endState = {};
            auto backendEndState = GetParam().mBackend->ChainEndState(&endState);

            // First fill the texture with data, so we can check that using it uninitialized
            // makes it black.
            {
                wgpu::CommandBuffer commandBuffer =
                    MakeFourColorsClearCommandBuffer(device, texture);

                beginDesc.concurrentRead = false;
                beginDesc.initialized = true;
                memory.BeginAccess(texture, &beginDesc);
                device.GetQueue().Submit(1, &commandBuffer);
                memory.EndAccess(texture, &endState);
            }

            // Now, BeginAccess on the texture as uninitialized.
            beginDesc.fenceCount = endState.fenceCount;
            beginDesc.fences = endState.fences;
            beginDesc.signaledValues = endState.signaledValues;
            beginDesc.concurrentRead = false;
            beginDesc.initialized = false;
            backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);
            memory.BeginAccess(texture, &beginDesc);

            // Use the texture on the GPU which should lazy clear it.
            UseTexture(texture);

            AsNonConst(endState.initialized) = false;  // should be overrwritten
            memory.EndAccess(texture, &endState);
            // The texture should be initialized now.
            EXPECT_TRUE(endState.initialized);

            // Begin access again - and check that the texture contents are zero.
            {
                auto [commandBuffer, colorTarget] =
                    MakeCheckBySamplingCommandBuffer(device, texture);

                beginDesc.fenceCount = endState.fenceCount;
                beginDesc.fences = endState.fences;
                beginDesc.signaledValues = endState.signaledValues;
                beginDesc.concurrentRead = false;
                beginDesc.initialized = endState.initialized;

                backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);
                memory.BeginAccess(texture, &beginDesc);
                device.GetQueue().Submit(1, &commandBuffer);
                memory.EndAccess(texture, &endState);

                uint8_t alphaVal;
                switch (properties.format) {
                    case wgpu::TextureFormat::RGBA8Unorm:
                    case wgpu::TextureFormat::BGRA8Unorm:
                    case wgpu::TextureFormat::RGB10A2Unorm:
                    case wgpu::TextureFormat::RGBA16Float:
                        alphaVal = 0;
                        break;
                    default:
                        // The test checks by sampling. Formats that don't
                        // have alpha return 1 for alpha when sampled in a shader.
                        alphaVal = 255;
                        break;
                }
                std::vector<utils::RGBA8> expected(texture.GetWidth() * texture.GetHeight(),
                                                   utils::RGBA8{0, 0, 0, alphaVal});
                EXPECT_TEXTURE_EQ(device, expected.data(), colorTarget, {0, 0},
                                  {colorTarget.GetWidth(), colorTarget.GetHeight()})
                    << "format: " << static_cast<uint32_t>(properties.format);
            }
        };

        // Test that using a texture in a render pass lazy clears it.
        if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
            DoTest([&](wgpu::Texture& texture) { UseInRenderPass(device, texture); });
        }

        // Teset that using a texture in a copy lazy clears it.
        if (properties.usage & wgpu::TextureUsage::CopySrc) {
            DoTest([&](wgpu::Texture& texture) { UseInCopy(device, texture); });
        }
    }
}

// Test that if the texture is uninitialized, EndAccess writes the state out as uninitialized.
TEST_P(SharedTextureMemoryTests, UninitializedOnEndAccess) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    // It is not possible to run these tests for multiplanar formats for
    // multiple reasons:
    // * Test basic begin+end access exports the state as uninitialized
    // if it starts as uninitialized. Multiplanar formats must be initialized on import.
    // * RenderAttachment gets a TextureView from the texture. This has a
    // superficial problem and a deep problem: The superficial problem is that
    // the plane would need to be passed for multiplanar formats, and the deep
    // problem is that the tests fail to create valid backing textures for some multiplanar
    // formats (e.g., on Apple), which results in a crash when accessing plane
    // 0.
    // TODO(crbug.com/dawn/2263): Fix this and change the below to
    // CreateSharedTextureMemories().
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSinglePlanarSharedTextureMemories(device,
                                                                      GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        {
            wgpu::Texture texture = memory.CreateTexture();
            beginDesc.concurrentRead = false;
            beginDesc.initialized = false;
            memory.BeginAccess(texture, &beginDesc);

            AsNonConst(endState.initialized) = true;  // should be overrwritten
            memory.EndAccess(texture, &endState);
            EXPECT_FALSE(endState.initialized);
        }

        // Test begin access as initialized, then uninitializing the texture
        // exports the state as uninitialized on end access. Requires render
        // attachment usage to uninitialize.
        if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
            wgpu::Texture texture = memory.CreateTexture();

            beginDesc = {};
            beginDesc.concurrentRead = false;
            beginDesc.initialized = true;
            backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);
            memory.BeginAccess(texture, &beginDesc);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::TextureViewDescriptor desc;
            desc.dimension = wgpu::TextureViewDimension::e2D;
            desc.baseArrayLayer = 0;
            desc.arrayLayerCount = 1;
            utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView(&desc)});
            passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            pass.End();
            wgpu::CommandBuffer commandBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &commandBuffer);

            endState = {};
            AsNonConst(endState.initialized) = true;  // should be overrwritten
            memory.EndAccess(texture, &endState);
            EXPECT_FALSE(endState.initialized);
        }
    }
}

// Test copying to texture memory on one device, then sampling it using another device.
TEST_P(SharedTextureMemoryTests, CopyToTextureThenSample) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::TextureBinding, GetParam().mLayerCount)) {
        wgpu::Texture texture = memories[0].CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(texture, &beginDesc);

        // Create a texture of the same size to use as the source content.
        wgpu::TextureDescriptor texDesc;
        texDesc.format = texture.GetFormat();
        texDesc.size = {texture.GetWidth(), texture.GetHeight()};
        texDesc.usage =
            texture.GetUsage() | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture srcTex = devices[0].CreateTexture(&texDesc);

        // Populate the source texture.
        wgpu::CommandBuffer commandBuffer = MakeFourColorsClearCommandBuffer(devices[0], srcTex);
        devices[0].GetQueue().Submit(1, &commandBuffer);

        // Copy from the source texture into `texture`.
        {
            wgpu::CommandEncoder encoder = devices[0].CreateCommandEncoder();
            auto src = utils::CreateTexelCopyTextureInfo(srcTex);
            auto dst = utils::CreateTexelCopyTextureInfo(texture);
            for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
                dst.origin.z = layer;
                encoder.CopyTextureToTexture(&src, &dst, &texDesc.size);
            }
            commandBuffer = encoder.Finish();
        }
        devices[0].GetQueue().Submit(1, &commandBuffer);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(texture, &endState);

        // Sample from the texture

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        texture = memories[1].CreateTexture();

        memories[1].BeginAccess(texture, &beginDesc);

        wgpu::Texture colorTarget;
        std::tie(commandBuffer, colorTarget) =
            MakeCheckBySamplingCommandBuffer(devices[1], texture);
        devices[1].GetQueue().Submit(1, &commandBuffer);
        memories[1].EndAccess(texture, &endState);

        CheckFourColors(devices[1], texture.GetFormat(), colorTarget);
    }
}

// Test that BeginAccess without waiting on anything, followed by EndAccess
// without using the texture, does not export any fences.
TEST_P(SharedTextureMemoryTests, EndWithoutUse) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    for (const auto& memory :
         GetParam().mBackend->CreateSharedTextureMemories(device, GetParam().mLayerCount)) {
        wgpu::Texture texture = memory.CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = true;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memory.BeginAccess(texture, &beginDesc);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memory.EndAccess(texture, &endState);

        EXPECT_EQ(endState.fenceCount, 0u);
    }
}

// Test that BeginAccess, waiting on previous work, followed by EndAccess when the
// texture isn't used at all, doesn't export any new fences from Dawn. It exports the
// old fences.
// If concurrent read is supported, use two read textures. The first EndAccess should
// see no fences. The second should then export all the unacquired fences.
TEST_P(SharedTextureMemoryTests, BeginEndWithoutUse) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::TextureBinding, GetParam().mLayerCount)) {
        wgpu::Texture texture = memories[0].CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(texture, &beginDesc);

        // Create a texture of the same size to use as the source content.
        wgpu::TextureDescriptor texDesc;
        texDesc.format = texture.GetFormat();
        texDesc.size = {texture.GetWidth(), texture.GetHeight()};
        texDesc.usage =
            texture.GetUsage() | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture srcTex = devices[0].CreateTexture(&texDesc);

        // Populate the source texture.
        wgpu::CommandBuffer commandBuffer = MakeFourColorsClearCommandBuffer(devices[0], srcTex);
        devices[0].GetQueue().Submit(1, &commandBuffer);

        // Copy from the source texture into `texture`.
        {
            wgpu::CommandEncoder encoder = devices[0].CreateCommandEncoder();
            auto src = utils::CreateTexelCopyTextureInfo(srcTex);
            auto dst = utils::CreateTexelCopyTextureInfo(texture);
            for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
                dst.origin.z = layer;
                encoder.CopyTextureToTexture(&src, &dst, &texDesc.size);
            }
            commandBuffer = encoder.Finish();
        }
        devices[0].GetQueue().Submit(1, &commandBuffer);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(texture, &endState);

        // Import fences and texture to the other device.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        // Do concurrent read if the backend supports it, and not Vulkan.
        // Note that here, the "backend" means the handle type. So on Vulkan, sync fds do support
        // concurrent reads on different devices. But, in Dawn's Vulkan backend within a single
        // device, support is not implemented yet.
        beginDesc.concurrentRead = GetParam().mBackend->SupportsConcurrentRead() && !IsVulkan();
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        texDesc.size = {texture.GetWidth(), texture.GetHeight(), texture.GetDepthOrArrayLayers()};
        texDesc.usage = wgpu::TextureUsage::TextureBinding;
        texture = memories[1].CreateTexture(&texDesc);

        memories[1].BeginAccess(texture, &beginDesc);

        // Prepare to sample the texture without submit, and end access.
        wgpu::Texture colorTarget;
        std::tie(commandBuffer, colorTarget) =
            MakeCheckBySamplingCommandBuffer(devices[1], texture);
        if (beginDesc.concurrentRead) {
            // If concurrent read, make another texture, and begin+end access without using it.
            wgpu::Texture noopTexture = memories[1].CreateTexture(&texDesc);
            memories[1].BeginAccess(noopTexture, &beginDesc);
            memories[1].EndAccess(noopTexture, &endState);
            EXPECT_EQ(endState.fenceCount, 0u);
        }
        memories[1].EndAccess(texture, &endState);

        // All of the fences should be identical.
        EXPECT_EQ(endState.fenceCount, sharedFences.size());
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            EXPECT_NE(std::find_if(sharedFences.begin(), sharedFences.end(),
                                   [&](const auto& fence) {
                                       return fence.Get() == endState.fences[i].Get();
                                   }),
                      sharedFences.end());
        }
    }
}

// Test copying to texture memory on one device, then sampling it using another device.
TEST_P(SharedTextureMemoryTests, CopyToTextureThenSample2DArray) {
    if (GetParam().mLayerCount != 2) {
        return;
    }
    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::TextureBinding, GetParam().mLayerCount)) {
        wgpu::Texture texture = memories[0].CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(texture, &beginDesc);

        // Create a texture of the same size to use as the source content.
        wgpu::TextureDescriptor texDesc;
        texDesc.format = texture.GetFormat();
        texDesc.size = {texture.GetWidth(), texture.GetHeight()};
        texDesc.usage =
            texture.GetUsage() | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture srcTex = devices[0].CreateTexture(&texDesc);

        // Populate the source texture.
        wgpu::CommandBuffer commandBuffer = MakeFourColorsClearCommandBuffer(devices[0], srcTex);
        devices[0].GetQueue().Submit(1, &commandBuffer);

        // Copy from the source texture into `texture`.
        {
            wgpu::CommandEncoder encoder = devices[0].CreateCommandEncoder();
            auto src = utils::CreateTexelCopyTextureInfo(srcTex);
            auto dst = utils::CreateTexelCopyTextureInfo(texture);
            for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
                dst.origin.z = layer;
                encoder.CopyTextureToTexture(&src, &dst, &texDesc.size);
            }
            commandBuffer = encoder.Finish();
        }
        devices[0].GetQueue().Submit(1, &commandBuffer);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(texture, &endState);

        // Sample from the texture

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        texture = memories[1].CreateTexture();

        memories[1].BeginAccess(texture, &beginDesc);

        wgpu::Texture colorTarget;
        std::tie(commandBuffer, colorTarget) =
            MakeCheckBySamplingTexture2DArrayCommandBuffer(devices[1], texture);
        devices[1].GetQueue().Submit(1, &commandBuffer);
        memories[1].EndAccess(texture, &endState);

        CheckFourColors(devices[1], texture.GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Encode the commands after performing BeginAccess.
TEST_P(SharedTextureMemoryTests, RenderThenSampleEncodeAfterBeginAccess) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        wgpu::Texture texture = memories[0].CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(texture, &beginDesc);

        // Clear the texture
        wgpu::CommandBuffer commandBuffer = MakeFourColorsClearCommandBuffer(devices[0], texture);
        devices[0].GetQueue().Submit(1, &commandBuffer);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(texture, &endState);

        // Sample from the texture

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        texture = memories[1].CreateTexture();

        memories[1].BeginAccess(texture, &beginDesc);

        wgpu::Texture colorTarget;
        std::tie(commandBuffer, colorTarget) =
            MakeCheckBySamplingCommandBuffer(devices[1], texture);
        devices[1].GetQueue().Submit(1, &commandBuffer);
        memories[1].EndAccess(texture, &endState);

        CheckFourColors(devices[1], texture.GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Encode the commands before performing BeginAccess (the access is only held during) QueueSubmit.
TEST_P(SharedTextureMemoryTests, RenderThenSampleEncodeBeforeBeginAccess) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(textures[0], &beginDesc);

        devices[0].GetQueue().Submit(1, &commandBuffer0);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(textures[0], &endState);

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        memories[1].BeginAccess(textures[1], &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(textures[1], &endState);

        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Destroy the texture from the first device after submitting the commands, but before performing
// EndAccess. The second device should still be able to wait on the first device and see the
// results.
TEST_P(SharedTextureMemoryTests, RenderThenTextureDestroyBeforeEndAccessThenSample) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(textures[0], &beginDesc);

        devices[0].GetQueue().Submit(1, &commandBuffer0);

        // Destroy the texture before performing EndAccess.
        textures[0].Destroy();

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(textures[0], &endState);

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        memories[1].BeginAccess(textures[1], &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(textures[1], &endState);

        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    }
}

// Test accessing the memory on one device, dropping all memories, then
// accessing on the second device. Operations on the second device must
// still wait for the preceding operations to complete.
TEST_P(SharedTextureMemoryTests, RenderThenDropAllMemoriesThenSample) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (auto memories : GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        // Render to the texture.
        {
            memories[0].BeginAccess(textures[0], &beginDesc);
            devices[0].GetQueue().Submit(1, &commandBuffer0);
            memories[0].EndAccess(textures[0], &endState);
        }

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        // Begin access, then drop all memories.
        memories[1].BeginAccess(textures[1], &beginDesc);
        memories.clear();

        // Sample from the texture and check the contents.
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Destroy or destroy the first device after submitting the commands, but before performing
// EndAccess. The second device should still be able to wait on the first device and see the
// results.
// This tests both cases where the device is destroyed, and where the device is lost.
TEST_P(SharedTextureMemoryTests, RenderThenLoseOrDestroyDeviceBeforeEndAccessThenSample) {
    // Not supported if using the same device. Not possible to lose one without losing the other.
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mBackend->UseSameDevice());

    // This test expects a fence returned from EndAccess, which is not possible if fences are
    // disabled in D3D11.
    DAWN_TEST_UNSUPPORTED_IF(IsD3D11() && HasToggleEnabled("d3d11_disable_fence"));

    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    auto DoTest = [&](auto DestroyOrLoseDevice) {
        std::vector<wgpu::Device> devices = {CreateDevice(), CreateDevice()};
        auto perDeviceMemories =
            GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
                devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
                GetParam().mLayerCount);
        DAWN_TEST_UNSUPPORTED_IF(perDeviceMemories.empty());

        const auto& memories = perDeviceMemories[0];

        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(textures[0], &beginDesc);

        devices[0].GetQueue().Submit(1, &commandBuffer0);

        // Destroy or lose the device before performing EndAccess.
        DestroyOrLoseDevice(devices[0]);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(textures[0], &endState);
        EXPECT_GT(endState.fenceCount, 0u);

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        memories[1].BeginAccess(textures[1], &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(textures[1], &endState);

        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    };

    DoTest([](wgpu::Device d) { d.Destroy(); });

    DoTest([this](wgpu::Device d) { LoseDeviceForTesting(d); });
}

// Test a shared texture memory created on separate devices but wrapping the same underyling data.
// Write to the texture, then read from two separate devices concurrently, then write again.
// Reads should happen strictly after the writes. The final write should wait for the reads.
TEST_P(SharedTextureMemoryTests, SeparateDevicesWriteThenConcurrentReadThenWrite) {
    DAWN_TEST_UNSUPPORTED_IF(!GetParam().mBackend->SupportsConcurrentRead());

    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice(), CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memories[0].GetProperties(&properties);

        wgpu::TextureDescriptor writeTextureDesc = {};
        writeTextureDesc.format = properties.format;
        writeTextureDesc.size = properties.size;
        writeTextureDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        writeTextureDesc.label = "write texture";

        wgpu::TextureDescriptor readTextureDesc = {};
        readTextureDesc.format = properties.format;
        readTextureDesc.size = properties.size;
        readTextureDesc.usage = wgpu::TextureUsage::TextureBinding;
        readTextureDesc.label = "read texture";

        // Create three textures from each memory.
        // The first one will be written to.
        // The second two will be concurrently read after the write.
        // Then the first one will be written to again.
        wgpu::Texture textures[] = {memories[0].CreateTexture(&writeTextureDesc),
                                    memories[1].CreateTexture(&readTextureDesc),
                                    memories[2].CreateTexture(&readTextureDesc)};

        // Build command buffers for the test.
        wgpu::CommandBuffer writeCommandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);

        auto [checkCommandBuffer1, colorTarget1] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        auto [checkCommandBuffer2, colorTarget2] =
            MakeCheckBySamplingCommandBuffer(devices[2], textures[2]);

        wgpu::CommandBuffer clearToGrayCommandBuffer0;
        {
            wgpu::CommandEncoder encoder = devices[0].CreateCommandEncoder();
            wgpu::TextureViewDescriptor desc;
            desc.dimension = wgpu::TextureViewDimension::e2D;
            desc.baseArrayLayer = 0;
            desc.arrayLayerCount = 1;
            utils::ComboRenderPassDescriptor passDescriptor({textures[0].CreateView(&desc)});
            passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
            passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            passDescriptor.cColorAttachments[0].clearValue = {0.5, 0.5, 0.5, 1.0};

            encoder.BeginRenderPass(&passDescriptor).End();
            clearToGrayCommandBuffer0 = encoder.Finish();
        }

        // Begin access on texture 0
        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memories[0].BeginAccess(textures[0], &beginDesc);

        // Write
        devices[0].GetQueue().Submit(1, &writeCommandBuffer0);

        // End access on texture 0
        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memories[0].EndAccess(textures[0], &endState);
        EXPECT_TRUE(endState.initialized);

        // Import fences to devices[1] and begin access.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = sharedFences.size();
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = true;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);
        memories[1].BeginAccess(textures[1], &beginDesc);

        // Import fences to devices[2] and begin access.
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[2], endState.fences[i]);
        }
        memories[2].BeginAccess(textures[2], &beginDesc);

        // Check contents
        devices[1].GetQueue().Submit(1, &checkCommandBuffer1);
        devices[2].GetQueue().Submit(1, &checkCommandBuffer2);
        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget1);
        CheckFourColors(devices[2], textures[2].GetFormat(), colorTarget2);

        // End access on texture 1
        wgpu::SharedTextureMemoryEndAccessState endState1;
        auto backendEndState1 = GetParam().mBackend->ChainEndState(&endState1);
        memories[1].EndAccess(textures[1], &endState1);
        EXPECT_TRUE(endState1.initialized);

        // End access on texture 2
        wgpu::SharedTextureMemoryEndAccessState endState2;
        auto backendEndState2 = GetParam().mBackend->ChainEndState(&endState2);
        memories[2].EndAccess(textures[2], &endState2);
        EXPECT_TRUE(endState2.initialized);

        // Import fences back to devices[0]
        sharedFences.resize(endState1.fenceCount + endState2.fenceCount);
        std::vector<uint64_t> signaledValues(sharedFences.size());

        for (size_t i = 0; i < endState1.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[0], endState1.fences[i]);
            signaledValues[i] = endState1.signaledValues[i];
        }
        for (size_t i = 0; i < endState2.fenceCount; ++i) {
            sharedFences[i + endState1.fenceCount] =
                GetParam().mBackend->ImportFenceTo(devices[0], endState2.fences[i]);
            signaledValues[i + endState1.fenceCount] = endState2.signaledValues[i];
        }

        beginDesc.fenceCount = sharedFences.size();
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = signaledValues.data();
        beginDesc.concurrentRead = false;
        beginDesc.initialized = true;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState2);

        // Begin access on texture 0
        memories[0].BeginAccess(textures[0], &beginDesc);

        // Submit a clear to gray.
        devices[0].GetQueue().Submit(1, &clearToGrayCommandBuffer0);
    }
}

// Test a shared texture memory created on one device. Create three textures from the memory,
// Write to one texture, then read from two separate textures `concurrently`, then write again.
// Reads should happen strictly after the writes. The final write should wait for the reads.
TEST_P(SharedTextureMemoryTests, SameDeviceWriteThenConcurrentReadThenWrite) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_TEST_UNSUPPORTED_IF(IsVulkan());

    DAWN_TEST_UNSUPPORTED_IF(!GetParam().mBackend->SupportsConcurrentRead());

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             {device}, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        auto memory = memories[0];
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor writeTextureDesc = {};
        writeTextureDesc.format = properties.format;
        writeTextureDesc.size = properties.size;
        writeTextureDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        writeTextureDesc.label = "write texture";

        wgpu::TextureDescriptor readTextureDesc = {};
        readTextureDesc.format = properties.format;
        readTextureDesc.size = properties.size;
        readTextureDesc.usage = wgpu::TextureUsage::TextureBinding;
        readTextureDesc.label = "read texture";

        // Create three textures from each memory.
        // The first one will be written to.
        // The second two will be concurrently read after the write.
        // Then the first one will be written to again.
        wgpu::Texture textures[] = {memory.CreateTexture(&writeTextureDesc),
                                    memory.CreateTexture(&readTextureDesc),
                                    memory.CreateTexture(&readTextureDesc)};

        // Build command buffers for the test.
        wgpu::CommandBuffer writeCommandBuffer0 =
            MakeFourColorsClearCommandBuffer(device, textures[0]);

        auto [checkCommandBuffer, colorTarget] =
            MakeCheckBySamplingTwoTexturesCommandBuffer(textures[1], textures[2]);

        wgpu::CommandBuffer clearToGrayCommandBuffer0;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TextureViewDescriptor desc;
            desc.dimension = wgpu::TextureViewDimension::e2D;
            desc.baseArrayLayer = 0;
            desc.arrayLayerCount = 1;
            utils::ComboRenderPassDescriptor passDescriptor({textures[0].CreateView(&desc)});
            passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
            passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            passDescriptor.cColorAttachments[0].clearValue = {0.5, 0.5, 0.5, 1.0};

            encoder.BeginRenderPass(&passDescriptor).End();
            clearToGrayCommandBuffer0 = encoder.Finish();
        }

        // Begin access on texture 0
        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);
        memory.BeginAccess(textures[0], &beginDesc);

        // Write
        device.GetQueue().Submit(1, &writeCommandBuffer0);

        // End access on texture 0
        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);
        memory.EndAccess(textures[0], &endState);
        EXPECT_TRUE(endState.initialized);

        // Import fences to device and begin access.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(device, endState.fences[i]);
        }
        beginDesc.fenceCount = sharedFences.size();
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = true;
        beginDesc.initialized = true;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);
        memory.BeginAccess(textures[1], &beginDesc);

        // Import fences to device and begin access.
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(device, endState.fences[i]);
        }
        memory.BeginAccess(textures[2], &beginDesc);

        // Check contents
        device.GetQueue().Submit(1, &checkCommandBuffer);
        CheckFourColors(device, textures[1].GetFormat(), colorTarget);

        // End access on texture 1
        wgpu::SharedTextureMemoryEndAccessState endState1;
        auto backendEndState1 = GetParam().mBackend->ChainEndState(&endState1);
        memory.EndAccess(textures[1], &endState1);
        EXPECT_TRUE(endState1.initialized);

        // End access on texture 2
        wgpu::SharedTextureMemoryEndAccessState endState2;
        auto backendEndState2 = GetParam().mBackend->ChainEndState(&endState2);
        memory.EndAccess(textures[2], &endState2);
        EXPECT_TRUE(endState2.initialized);

        // Import fences back to devices[0]
        sharedFences.resize(endState1.fenceCount + endState2.fenceCount);
        std::vector<uint64_t> signaledValues(sharedFences.size());

        for (size_t i = 0; i < endState1.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(device, endState1.fences[i]);
            signaledValues[i] = endState1.signaledValues[i];
        }
        for (size_t i = 0; i < endState2.fenceCount; ++i) {
            sharedFences[i + endState1.fenceCount] =
                GetParam().mBackend->ImportFenceTo(device, endState2.fences[i]);
            signaledValues[i + endState1.fenceCount] = endState2.signaledValues[i];
        }

        beginDesc.fenceCount = sharedFences.size();
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = signaledValues.data();
        beginDesc.concurrentRead = false;
        beginDesc.initialized = true;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState2);

        // Begin access on texture 0
        memory.BeginAccess(textures[0], &beginDesc);

        // Submit a clear to gray.
        device.GetQueue().Submit(1, &clearToGrayCommandBuffer0);
    }
}

// Test that textures created from SharedTextureMemory may perform sRGB reinterpretation.
TEST_P(SharedTextureMemoryTests, SRGBReinterpretation) {
    // Format reinterpretation is not available in compatibility mode.
    DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode());

    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    // TODO(crbug.com/dawn/2304): Investigate if the VVL is wrong here.
    DAWN_SUPPRESS_TEST_IF(GetParam().mBackend->Name().find("dma buf") != std::string::npos &&
                          IsBackendValidationEnabled());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
             GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memories[1].GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};

        textureDesc.format = properties.format;
        textureDesc.size = properties.size;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment;

        wgpu::TextureViewDescriptor viewDesc = {};
        if (properties.format == wgpu::TextureFormat::RGBA8Unorm) {
            viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        } else if (properties.format == wgpu::TextureFormat::BGRA8Unorm) {
            viewDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
        } else {
            continue;
        }

        textureDesc.viewFormatCount = 1;
        textureDesc.viewFormats = &viewDesc.format;

        // Create the texture on device 1.
        wgpu::Texture texture = memories[1].CreateTexture(&textureDesc);

        wgpu::CommandEncoder encoder = devices[1].CreateCommandEncoder();
        for (uint32_t layer = 0; layer < texture.GetDepthOrArrayLayers(); ++layer) {
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseArrayLayer = layer;
            viewDesc.arrayLayerCount = 1;
            // Submit a clear operation to sRGB value rgb(234, 51, 35).
            utils::ComboRenderPassDescriptor renderPassDescriptor({texture.CreateView(&viewDesc)},
                                                                  {});
            renderPassDescriptor.cColorAttachments[0].clearValue = {234.0 / 255.0, 51.0 / 255.0,
                                                                    35.0 / 255.0, 1.0};
            encoder.BeginRenderPass(&renderPassDescriptor).End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);

        memories[1].BeginAccess(texture, &beginDesc);
        devices[1].GetQueue().Submit(1, &commands);
        memories[1].EndAccess(texture, &endState);

        // Create the texture on device 0.
        texture = memories[0].CreateTexture();

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[0], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        memories[0].BeginAccess(texture, &beginDesc);

        // Expect the contents to be approximately rgb(246 124 104)
        if (properties.format == wgpu::TextureFormat::RGBA8Unorm) {
            EXPECT_PIXEL_RGBA8_BETWEEN(            //
                utils::RGBA8(245, 123, 103, 255),  //
                utils::RGBA8(247, 125, 105, 255), texture, 0, 0);
        } else {
            EXPECT_PIXEL_RGBA8_BETWEEN(            //
                utils::RGBA8(103, 123, 245, 255),  //
                utils::RGBA8(105, 125, 247, 255), texture, 0, 0);
        }
    }
}

// Test writing to texture memory in compute pass on one device, then sampling it using another
// device.
TEST_P(SharedTextureMemoryTests, WriteStorageThenReadSample) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        // Create the textures on each SharedTextureMemory.
        wgpu::Texture texture0 = memories[0].CreateTexture();
        wgpu::Texture texture1 = memories[1].CreateTexture();

        // Make a command buffer to populate the texture contents in a compute shader.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsComputeCommandBuffer(devices[0], texture0);

        // Make a command buffer to sample and check the texture contents.
        wgpu::Texture resultTarget;
        wgpu::CommandBuffer commandBuffer1;
        std::tie(commandBuffer1, resultTarget) =
            MakeCheckBySamplingCommandBuffer(devices[1], texture1);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.concurrentRead = false;
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);

        // Begin access on memory 0, submit the compute pass, end access.
        memories[0].BeginAccess(texture0, &beginDesc);
        devices[0].GetQueue().Submit(1, &commandBuffer0);
        memories[0].EndAccess(texture0, &endState);

        // Import fences to device 1.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.concurrentRead = false;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        // Begin access on memory 1, check the contents, end access.
        memories[1].BeginAccess(texture1, &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(texture1, &endState);

        // Check all the sampled colors are correct.
        CheckFourColors(devices[1], texture1.GetFormat(), resultTarget);
    }
}

// Test writing to texture memory using queue.writeTexture, then sampling it using another device.
TEST_P(SharedTextureMemoryTests, WriteTextureThenReadSample) {
    // crbug.com/358166479
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding,
             GetParam().mLayerCount)) {
        wgpu::SharedTextureMemoryProperties properties;
        memories[0].GetProperties(&properties);

        if (properties.format != wgpu::TextureFormat::RGBA8Unorm) {
            continue;
        }

        // Create the textures on each SharedTextureMemory.
        wgpu::Texture texture0 = memories[0].CreateTexture();
        wgpu::Texture texture1 = memories[1].CreateTexture();

        // Make a command buffer to sample and check the texture contents.
        wgpu::Texture resultTarget;
        wgpu::CommandBuffer commandBuffer1;
        std::tie(commandBuffer1, resultTarget) =
            MakeCheckBySamplingCommandBuffer(devices[1], texture1);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        auto backendBeginState = GetParam().mBackend->ChainInitialBeginState(&beginDesc);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        auto backendEndState = GetParam().mBackend->ChainEndState(&endState);

        // Begin access on memory 0, use queue.writeTexture to populate the contents, end access.
        memories[0].BeginAccess(texture0, &beginDesc);
        WriteFourColorsToRGBA8Texture(devices[0], texture0);
        memories[0].EndAccess(texture0, &endState);

        // Import fences to device 1.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = endState.initialized;
        backendBeginState = GetParam().mBackend->ChainBeginState(&beginDesc, endState);

        // Begin access on memory 1, check the contents, end access.
        memories[1].BeginAccess(texture1, &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(texture1, &endState);

        // Check all the sampled colors are correct.
        CheckFourColors(devices[1], texture1.GetFormat(), resultTarget);
    }
}

class SharedTextureMemoryVulkanTests : public DawnTest {};

// Test that only a single Vulkan fence feature may be enabled at once.
TEST_P(SharedTextureMemoryVulkanTests, SingleFenceFeature) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    std::vector<wgpu::FeatureName> fenceFeatures;

    wgpu::Adapter adapter(GetAdapter().Get());
    for (wgpu::FeatureName f : {
             wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD,
             wgpu::FeatureName::SharedFenceSyncFD,
             wgpu::FeatureName::SharedFenceVkSemaphoreZirconHandle,
         }) {
        if (adapter.HasFeature(f)) {
            fenceFeatures.push_back(f);
        }
    }

    // Test that creating a device with each feature is valid.
    for (wgpu::FeatureName f : fenceFeatures) {
        wgpu::DeviceDescriptor deviceDesc;
        deviceDesc.requiredFeatureCount = 1;
        deviceDesc.requiredFeatures = &f;
        EXPECT_NE(adapter.CreateDevice(&deviceDesc), nullptr);
    }

    // Test that any combination of two features is invalid.
    for (size_t i = 0; i < fenceFeatures.size(); ++i) {
        for (size_t j = i + 1; j < fenceFeatures.size(); ++j) {
            wgpu::FeatureName features[] = {fenceFeatures[i], fenceFeatures[j]};
            wgpu::DeviceDescriptor deviceDesc;
            deviceDesc.requiredFeatureCount = 2;
            deviceDesc.requiredFeatures = features;
            EXPECT_EQ(adapter.CreateDevice(&deviceDesc), nullptr);
        }
    }
}

DAWN_INSTANTIATE_TEST(SharedTextureMemoryVulkanTests, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
