// Copyright 2020 The Dawn & Tint Authors
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

#include <algorithm>
#include <list>
#include <set>
#include <vector>

#include "dawn/native/ResourceTableDefaultResources.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d12 {
namespace {

constexpr uint32_t kRTSize = 4;

// Pooling tests are required to advance the GPU completed serial to reuse heaps.
// This requires Tick() to be called at-least |kFrameDepth| times. This constant
// should be updated if the internals of Tick() change.
constexpr uint32_t kFrameDepth = 2;

class D3D12DescriptorHeapTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Tests reach directly in d3d12::Device and friends so skip them on wire and with implicit
        // device sync.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(IsImplicitDeviceSyncEnabled());

        mD3DDevice = ToBackend(FromAPI(device.Get()));
        mD3DQueue = ToBackend(mD3DDevice->GetQueue());

        mSimpleVSModule = utils::CreateShaderModule(device, R"(

            @vertex fn main(
                @builtin(vertex_index) VertexIndex : u32
            ) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0, -1.0)
                );
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        mSimpleFSModule = utils::CreateShaderModule(device, R"(
            struct U {
                color : vec4f
            }
            @group(0) @binding(0) var<uniform> colorBuffer : U;

            @fragment fn main() -> @location(0) vec4f {
                return colorBuffer.color;
            })");
    }

    utils::BasicRenderPass MakeRenderPass(uint32_t width,
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

        return utils::BasicRenderPass(width, height, color);
    }

    std::array<float, 4> GetSolidColor(uint32_t n) const {
        DAWN_ASSERT(n >> 24 == 0);
        float b = (n & 0xFF) / 255.0f;
        float g = ((n >> 8) & 0xFF) / 255.0f;
        float r = ((n >> 16) & 0xFF) / 255.0f;
        return {r, g, b, 1};
    }

    raw_ptr<Device> mD3DDevice = nullptr;
    raw_ptr<Queue> mD3DQueue = nullptr;

    wgpu::ShaderModule mSimpleVSModule;
    wgpu::ShaderModule mSimpleFSModule;
};

class PlaceholderStagingDescriptorAllocator {
  public:
    PlaceholderStagingDescriptorAllocator(Device* device,
                                          uint32_t descriptorCount,
                                          uint32_t allocationsPerHeap)
        : mAllocator(device,
                     descriptorCount,
                     allocationsPerHeap * descriptorCount,
                     D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}

    CPUDescriptorHeapAllocation AllocateCPUDescriptors() {
        native::ResultOrError<CPUDescriptorHeapAllocation> result =
            mAllocator.AllocateCPUDescriptors();
        return (result.IsSuccess()) ? result.AcquireSuccess() : CPUDescriptorHeapAllocation{};
    }

    void Deallocate(CPUDescriptorHeapAllocation& allocation) { mAllocator.Deallocate(&allocation); }

  private:
    StagingDescriptorAllocator mAllocator;
};

// Verify the shader visible view heaps switch over within a single submit.
TEST_P(D3D12DescriptorHeapTests, SwitchOverViewHeap) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;

    // Fill in a view heap with "view only" bindgroups (1x view per group) by creating a
    // view bindgroup each draw. After HEAP_SIZE + 1 draws, the heaps must switch over.
    renderPipelineDescriptor.vertex.module = mSimpleVSModule;
    renderPipelineDescriptor.cFragment.module = mSimpleFSModule;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetViewShaderVisibleDescriptorAllocator();
    const uint64_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(renderPipeline);

        std::array<float, 4> redColor = {1, 0, 0, 1};
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, &redColor, sizeof(redColor), wgpu::BufferUsage::Uniform);

        for (uint32_t i = 0; i < heapSize + 1; ++i) {
            // Allocates one descriptor
            pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                      {{0, uniformBuffer, 0, sizeof(redColor)}}));
            pass.Draw(3);
        }

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(1));
}

// Verify the shader visible view heap switches over within a single submit because bind group 0
// requires more descriptors than available in the heap, while there's still enough for group 1.
TEST_P(D3D12DescriptorHeapTests, SwitchOverViewHeapBecauseOfBindingGroup0) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;

    // Fill in a view heap with "view only" bindgroups (1x view per group) by creating a
    // view bindgroup each draw. After HEAP_SIZE + 1 draws, the heaps must switch over.
    renderPipelineDescriptor.vertex.module = mSimpleVSModule;
    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            struct U {
                color : vec4f
            }
            @group(0) @binding(0) var<uniform> cb1 : U;
            @group(0) @binding(1) var<uniform> cb2 : U;
            @group(0) @binding(2) var<uniform> cb3 : U;
            @group(0) @binding(3) var<uniform> cb4 : U;

            @group(1) @binding(0) var<uniform> cb5: U;

            @fragment fn main() -> @location(0) vec4f {
                return cb1.color + cb2.color + cb3.color + cb4.color + cb5.color;
            }
    )");

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetViewShaderVisibleDescriptorAllocator();
    const uint64_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    // This test is written assuming a certain heap size to trigger
    // AllocateAndSwitchShaderVisibleHeap when binding group 0 (see below). If this value is
    // changed, we likely need to update this test.
    DAWN_ASSERT(heapSize == 32);

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        wgpu::RenderPipeline renderPipeline =
            device.CreateRenderPipeline(&renderPipelineDescriptor);
        pass.SetPipeline(renderPipeline);

        uint32_t numDescriptorsPerIter = 5;
        uint32_t allocOnIter = (heapSize / numDescriptorsPerIter + 1);

        // "small heap" size is 32 for CBV_SRV_UAV, so 32 / 5 = 6.4; on the 7th iteration,
        // there will only be room enough for 32 - (5*6) = 2 descriptors, so the
        // group->PopulateViews() will fail for group 0 as it needs 4 descriptors, but would succeed
        // for group 1 as it only needs 1. This should trigger an
        // AllocateAndSwitchShaderVisibleHeap, but previously didn't because the failure of the
        // allocation of group 0 was overwritten by the success of group 1.

        std::array<float, 4> redColor = {1, 0, 0, 1};
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, &redColor, sizeof(redColor), wgpu::BufferUsage::Uniform);

        for (uint32_t i = 0; i < allocOnIter; ++i) {
            // Allocates descriptors
            pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, uniformBuffer, 0, sizeof(redColor)},
                                                          {1, uniformBuffer, 0, sizeof(redColor)},
                                                          {2, uniformBuffer, 0, sizeof(redColor)},
                                                          {3, uniformBuffer, 0, sizeof(redColor)},
                                                      }));
            pass.SetBindGroup(1, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(1),
                                                      {
                                                          {0, uniformBuffer, 0, sizeof(redColor)},
                                                      }));
            pass.Draw(3);
        }
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(1));
}

// Verify the shader visible sampler heaps does not switch over within a single submit when samplers
// are cached.
TEST_P(D3D12DescriptorHeapTests, NoSwitchOverSamplerHeapBecauseOfCache) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;

    // Fill in a sampler heap with "sampler only" bindgroups (1x sampler per group) by creating
    // a sampler bindgroup each draw. After HEAP_SIZE + 1 draws, the heaps WILL NOT switch over
    // because the sampler heap allocations are de-duplicated.
    renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @fragment fn main() -> @location(0) vec4f {
                _ = sampler0;
                return vec4f(0.0, 0.0, 0.0, 0.0);
            })");

    wgpu::Sampler sampler = device.CreateSampler();

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetSamplerShaderVisibleDescriptorAllocator();
    const uint64_t samplerHeapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPipeline renderPipeline =
            device.CreateRenderPipeline(&renderPipelineDescriptor);
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(renderPipeline);

        for (uint32_t i = 0; i < samplerHeapSize + 1; ++i) {
            pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                      {{0, sampler}}));
            pass.Draw(3);
        }

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial);
}

// Verify the shader visible sampler heap switches over within a single submit
TEST_P(D3D12DescriptorHeapTests, SwitchOverSamplerHeap) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    // "small heap" size is 16 for SAMPLERS, so we want to allocate more than 16 samplers in one
    // submit. Since the max number of samplers per stage is also 16, we have to do this in two draw
    // calls: the first allocates 16 samplers, and the second allocates 1, which should result in an
    // heap change. Note that we cannot issue two draw calls with the same pipeline because samplers
    // are cached by bind group, so the second draw would reuse the samplers from the first.

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetSamplerShaderVisibleDescriptorAllocator();
    [[maybe_unused]] const uint64_t samplerHeapSize =
        allocator->GetShaderVisibleHeapSizeForTesting();

    // This test is written assuming a certain heap size. If this value is changed, we likely need
    // to update this test.
    DAWN_ASSERT(samplerHeapSize == 16);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;

    renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

    // Allocates 16 sampler descriptors, will succeed on current heap, leaving 0 slots
    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;
            @group(0) @binding(2) var sampler2 : sampler;
            @group(0) @binding(3) var sampler3 : sampler;
            @group(0) @binding(4) var sampler4 : sampler;
            @group(0) @binding(5) var sampler5 : sampler;
            @group(0) @binding(6) var sampler6 : sampler;
            @group(0) @binding(7) var sampler7 : sampler;
            @group(0) @binding(8) var sampler8 : sampler;
            @group(0) @binding(9) var sampler9 : sampler;
            @group(0) @binding(10) var sampler10 : sampler;
            @group(0) @binding(11) var sampler11 : sampler;
            @group(0) @binding(12) var sampler12 : sampler;
            @group(0) @binding(13) var sampler13 : sampler;
            @group(0) @binding(14) var sampler14 : sampler;
            @group(0) @binding(15) var sampler15 : sampler;

            @fragment fn main() -> @location(0) vec4f {
                _ = sampler0;
                _ = sampler1;
                _ = sampler2;
                _ = sampler3;
                _ = sampler4;
                _ = sampler5;
                _ = sampler6;
                _ = sampler7;
                _ = sampler8;
                _ = sampler9;
                _ = sampler10;
                _ = sampler11;
                _ = sampler12;
                _ = sampler13;
                _ = sampler14;
                _ = sampler15;
                return vec4f(0.0, 0.0, 0.0, 0.0);
            })");

    wgpu::RenderPipeline pipeline16Samplers =
        device.CreateRenderPipeline(&renderPipelineDescriptor);

    // Allocates 1 sampler descriptor, will fail because no slots left, resulting in a heap change.
    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;

            @fragment fn main() -> @location(0) vec4f {
                _ = sampler0;
                return vec4f(0.0, 0.0, 0.0, 0.0);
            })");
    wgpu::RenderPipeline pipeline1Sampler = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(pipeline16Samplers);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline16Samplers.GetBindGroupLayout(0),
                                                  {
                                                      {0, sampler},
                                                      {1, sampler},
                                                      {2, sampler},
                                                      {3, sampler},
                                                      {4, sampler},
                                                      {5, sampler},
                                                      {6, sampler},
                                                      {7, sampler},
                                                      {8, sampler},
                                                      {9, sampler},
                                                      {10, sampler},
                                                      {11, sampler},
                                                      {12, sampler},
                                                      {13, sampler},
                                                      {14, sampler},
                                                      {15, sampler},
                                                  }));
        pass.Draw(3);

        wgpu::RenderPipeline pipeline1Sampler =
            device.CreateRenderPipeline(&renderPipelineDescriptor);
        pass.SetPipeline(pipeline1Sampler);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline1Sampler.GetBindGroupLayout(0),
                                                  {
                                                      {0, sampler},
                                                  }));
        pass.Draw(3);

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(1));
}

// Verify the shader visible sampler heap switches over within a single submit because bind group 0
// requires more descriptors than available in the heap, while there's still enough for group 1.
TEST_P(D3D12DescriptorHeapTests, SwitchOverSamplerHeapBecauseOfBindingGroup0) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    // "small heap" size is 16 for SAMPLERS, so we want to allocate sampler descriptors such
    // that group 0 fails, but group 1 succeeds. Because the max number of samplers per stage is
    // also 16, we cannot induce this situation on the first draw, but can on the second. Also,
    // since samplers are cached per bind group layout, we create two pipelines, each with different
    // bind group layouts. The first pipeline uses 15 samplers for bind group 0. The second pipeline
    // uses 2 samplers for bind group 0, and 1 for bind group 1; this way, when processing the
    // second draw, attempting to allocate the 2 sampler descriptors for bind group 0 will fail, as
    // there's only one slot left, but allocating the 1 sampler descriptor for bind group 1 would
    // succeed. We expect the allocation failure on bind group 0 to result in
    // AllocateAndSwitchShaderVisibleHeap being called, but previously didn't because the failure of
    // the allocation of group 0 was overwritten by the success of group 1.

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetSamplerShaderVisibleDescriptorAllocator();
    [[maybe_unused]] const uint64_t samplerHeapSize =
        allocator->GetShaderVisibleHeapSizeForTesting();

    // This test is written assuming a certain heap size to trigger. If this value is changed, we
    // likely need to update this test.
    DAWN_ASSERT(samplerHeapSize == 16);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;

    renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

    // Allocates 15 sampler descriptors, will succeed on current heap, leaving 1 slot
    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;
            @group(0) @binding(2) var sampler2 : sampler;
            @group(0) @binding(3) var sampler3 : sampler;
            @group(0) @binding(4) var sampler4 : sampler;
            @group(0) @binding(5) var sampler5 : sampler;
            @group(0) @binding(6) var sampler6 : sampler;
            @group(0) @binding(7) var sampler7 : sampler;
            @group(0) @binding(8) var sampler8 : sampler;
            @group(0) @binding(9) var sampler9 : sampler;
            @group(0) @binding(10) var sampler10 : sampler;
            @group(0) @binding(11) var sampler11 : sampler;
            @group(0) @binding(12) var sampler12 : sampler;
            @group(0) @binding(13) var sampler13 : sampler;
            @group(0) @binding(14) var sampler14 : sampler;

            @fragment fn main() -> @location(0) vec4f {
                _ = sampler0;
                _ = sampler1;
                _ = sampler2;
                _ = sampler3;
                _ = sampler4;
                _ = sampler5;
                _ = sampler6;
                _ = sampler7;
                _ = sampler8;
                _ = sampler9;
                _ = sampler10;
                _ = sampler11;
                _ = sampler12;
                _ = sampler13;
                _ = sampler14;
                return vec4f(0.0, 0.0, 0.0, 0.0);
            })");

    wgpu::RenderPipeline pipeline15Samplers =
        device.CreateRenderPipeline(&renderPipelineDescriptor);

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;

            @group(1) @binding(0) var sampler2 : sampler;

            @fragment fn main() -> @location(0) vec4f {
                _ = sampler0;
                _ = sampler1;
                _ = sampler2;
                return vec4f(0.0, 0.0, 0.0, 0.0);
            })");
    wgpu::RenderPipeline pipeline3Samplers = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(pipeline15Samplers);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline15Samplers.GetBindGroupLayout(0),
                                                  {
                                                      {0, sampler},
                                                      {1, sampler},
                                                      {2, sampler},
                                                      {3, sampler},
                                                      {4, sampler},
                                                      {5, sampler},
                                                      {6, sampler},
                                                      {7, sampler},
                                                      {8, sampler},
                                                      {9, sampler},
                                                      {10, sampler},
                                                      {11, sampler},
                                                      {12, sampler},
                                                      {13, sampler},
                                                      {14, sampler},
                                                  }));
        pass.Draw(3);

        pass.SetPipeline(pipeline3Samplers);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline3Samplers.GetBindGroupLayout(0),
                                                  {
                                                      {0, sampler},
                                                      {1, sampler},
                                                  }));
        pass.SetBindGroup(1, utils::MakeBindGroup(device, pipeline3Samplers.GetBindGroupLayout(1),
                                                  {
                                                      {0, sampler},
                                                  }));
        pass.Draw(3);

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(1));
}

// Verify shader-visible heaps can be recycled for multiple submits.
TEST_P(D3D12DescriptorHeapTests, PoolHeapsInMultipleSubmits) {
    // Use small heaps to count only pool-allocated switches.
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    auto* allocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();
    const uint32_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    std::list<ComPtr<ID3D12DescriptorHeap>> heaps = {allocator->GetShaderVisibleHeap()};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 0u);

    // Allocate + increment internal serials up to |kFrameDepth| and ensure heaps are always
    // unique.
    for (uint32_t i = 0; i < kFrameDepth; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.push_back(heap);
        // CheckPassedSerials() will update the last internally completed serial.
        EXPECT_TRUE(mD3DQueue->CheckPassedSerials().IsSuccess());
        // NextSerial() will increment the last internally submitted serial.
        EXPECT_TRUE(mD3DQueue->NextSerial().IsSuccess());
    }

    // Repeat up to |kFrameDepth| again but ensure heaps are the same in the expected order
    // (oldest heaps are recycled first). The "+ 1" is so we also include the very first heap in
    // the check.
    for (uint32_t i = 0; i < kFrameDepth + 1; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(heaps.front() == heap);
        heaps.pop_front();
        EXPECT_TRUE(mD3DQueue->CheckPassedSerials().IsSuccess());
        EXPECT_TRUE(mD3DQueue->NextSerial().IsSuccess());
    }

    EXPECT_TRUE(heaps.empty());
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), kFrameDepth);
}

// Verify shader-visible heaps do not recycle in a pending submit.
TEST_P(D3D12DescriptorHeapTests, PoolHeapsInPendingSubmit) {
    // Use small heaps to count only pool-allocated switches.
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    constexpr uint32_t kNumOfSwitches = 5;

    auto* allocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();
    const uint32_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {allocator->GetShaderVisibleHeap()};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 0u);

    // Switch-over |kNumOfSwitches| and ensure heaps are always unique.
    for (uint32_t i = 0; i < kNumOfSwitches; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
    }

    // After |kNumOfSwitches|, no heaps are recycled.
    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(),
              heapSerial + HeapVersionID(kNumOfSwitches));
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), kNumOfSwitches);
}

// Verify switching shader-visible heaps do not recycle in a pending submit but do so
// once no longer pending.
TEST_P(D3D12DescriptorHeapTests, PoolHeapsInPendingAndMultipleSubmits) {
    // Use small heaps to count only pool-allocated switches.
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    constexpr uint32_t kNumOfSwitches = 5;

    auto* allocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();
    const uint32_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();
    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {allocator->GetShaderVisibleHeap()};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 0u);

    // Switch-over |kNumOfSwitches| to create a pool of unique heaps.
    for (uint32_t i = 0; i < kNumOfSwitches; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
    }

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(),
              heapSerial + HeapVersionID(kNumOfSwitches));
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), kNumOfSwitches);

    // Ensure switched-over heaps can be recycled by advancing the GPU.
    mD3DQueue->AssumeCommandsComplete();

    // Switch-over |kNumOfSwitches| again reusing the same heaps.
    for (uint32_t i = 0; i < kNumOfSwitches; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) != heaps.end());
        heaps.erase(heap);
    }

    // After switching-over |kNumOfSwitches| x 2, ensure no additional heaps exist.
    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(),
              heapSerial + HeapVersionID(kNumOfSwitches * 2));
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), kNumOfSwitches);
}

// Verify shader-visible heaps do not recycle in multiple submits.
TEST_P(D3D12DescriptorHeapTests, GrowHeapsInMultipleSubmits) {
    auto* allocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();
    uint32_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {allocator->GetShaderVisibleHeap()};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 0u);

    // Growth: Allocate + Tick() and ensure heaps are always unique.
    while (allocator->GetShaderVisiblePoolSizeForTesting() == 0) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        heapSize *= 2;
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
        mD3DDevice->APITick();
    }

    // Verify the number of switches equals the size of heaps allocated (minus the initial).
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 1u);
    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(),
              heapSerial + HeapVersionID(heaps.size() - 1));
}

// Verify shader-visible heaps do not recycle in a pending submit.
TEST_P(D3D12DescriptorHeapTests, GrowHeapsInPendingSubmit) {
    auto* allocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();
    uint32_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {allocator->GetShaderVisibleHeap()};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 0u);

    // Growth: Allocate new heaps.
    while (allocator->GetShaderVisiblePoolSizeForTesting() == 0) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        heapSize *= 2;
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
    }

    // Verify the number of switches equals the size of heaps allocated (minus the initial).
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 1u);
    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(),
              heapSerial + HeapVersionID(heaps.size() - 1));
}

// Verify switching shader-visible heaps do not recycle in a pending submit but do so
// once no longer pending.
// Switches over many times until |kNumOfPooledHeaps| heaps are pool-allocated.
TEST_P(D3D12DescriptorHeapTests, GrowAndPoolHeapsInPendingAndMultipleSubmits) {
    // TODO(crbug.com/463661448): Flaky on Snapdragon X Elite SoCs.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm());

    auto* allocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();
    uint32_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {allocator->GetShaderVisibleHeap()};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), 0u);

    uint32_t kNumOfPooledHeaps = 5;
    while (allocator->GetShaderVisiblePoolSizeForTesting() < kNumOfPooledHeaps) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        heapSize = std::min(heapSize * 2, allocator->GetShaderVisibleHeapMaxSize());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
    }

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), kNumOfPooledHeaps);

    // Ensure switched-over heaps can be recycled by advancing the GPU.
    mD3DQueue->AssumeCommandsComplete();

    // Switch-over the pool-allocated heaps.
    for (uint32_t i = 0; i < kNumOfPooledHeaps; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeap(heapSize).IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeap();
        EXPECT_FALSE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
    }

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(), kNumOfPooledHeaps);
}

// Verify encoding multiple heaps worth of bindgroups.
// Shader-visible heaps will switch out |kNumOfHeaps| times.
TEST_P(D3D12DescriptorHeapTests, EncodeManyUBO) {
    // This test draws a solid color triangle |heapSize| times. Each draw uses a new bindgroup
    // that has its own UBO with a "color value" in the range [1... heapSize]. After |heapSize|
    // draws, the result is the arithmetic sum of the sequence after the framebuffer is blended
    // by accumulation. By checking for this sum, we ensure each bindgroup was encoded
    // correctly.
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    utils::BasicRenderPass renderPass =
        MakeRenderPass(kRTSize, kRTSize, wgpu::TextureFormat::R16Float);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = mSimpleVSModule;

    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        struct U {
            heapSize : f32
        }
        @group(0) @binding(0) var<uniform> buffer0 : U;

        @fragment fn main() -> @location(0) vec4f {
            return vec4f(buffer0.heapSize, 0.0, 0.0, 1.0);
        })");

    wgpu::BlendState blend;
    blend.color.operation = wgpu::BlendOperation::Add;
    blend.color.srcFactor = wgpu::BlendFactor::One;
    blend.color.dstFactor = wgpu::BlendFactor::One;
    blend.alpha.operation = wgpu::BlendOperation::Add;
    blend.alpha.srcFactor = wgpu::BlendFactor::One;
    blend.alpha.dstFactor = wgpu::BlendFactor::One;

    pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::R16Float;
    pipelineDescriptor.cTargets[0].blend = &blend;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    const uint32_t heapSize =
        mD3DDevice->GetViewShaderVisibleDescriptorAllocator()->GetShaderVisibleHeapSizeForTesting();

    constexpr uint32_t kNumOfHeaps = 2;

    const uint32_t numOfEncodedBindGroups = kNumOfHeaps * heapSize;

    std::vector<wgpu::BindGroup> bindGroups;
    for (uint32_t i = 0; i < numOfEncodedBindGroups; i++) {
        const float color = i + 1;
        wgpu::Buffer uniformBuffer =
            utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
        bindGroups.push_back(utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, uniformBuffer}}));
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(renderPipeline);

        for (uint32_t i = 0; i < numOfEncodedBindGroups; ++i) {
            pass.SetBindGroup(0, bindGroups[i]);
            pass.Draw(3);
        }

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    float colorSum = numOfEncodedBindGroups * (numOfEncodedBindGroups + 1) / 2;
    EXPECT_PIXEL_FLOAT16_EQ(colorSum, renderPass.color, 0, 0);
}

// Verify encoding one bindgroup then a heaps worth in different submits.
// Shader-visible heaps should switch out once upon encoding 1 + |heapSize| descriptors.
// The first descriptor's memory will be reused when the second submit encodes |heapSize|
// descriptors.
TEST_P(D3D12DescriptorHeapTests, EncodeUBOOverflowMultipleSubmit) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = mSimpleVSModule;
    pipelineDescriptor.cFragment.module = mSimpleFSModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Encode the first descriptor and submit.
    {
        std::array<float, 4> greenColor = {0, 1, 0, 1};
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, &greenColor, sizeof(greenColor), wgpu::BufferUsage::Uniform);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, renderPipeline.GetBindGroupLayout(0), {{0, uniformBuffer}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

            pass.SetPipeline(renderPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 0);

    // Encode a heap worth of descriptors.
    {
        const uint32_t heapSize = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator()
                                      ->GetShaderVisibleHeapSizeForTesting();

        std::vector<wgpu::BindGroup> bindGroups;
        for (uint32_t i = 0; i < heapSize - 1; i++) {
            std::array<float, 4> fillColor = GetSolidColor(i + 1);  // Avoid black
            wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
                device, &fillColor, sizeof(fillColor), wgpu::BufferUsage::Uniform);

            bindGroups.push_back(utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                      {{0, uniformBuffer}}));
        }

        std::array<float, 4> redColor = {1, 0, 0, 1};
        wgpu::Buffer lastUniformBuffer = utils::CreateBufferFromData(
            device, &redColor, sizeof(redColor), wgpu::BufferUsage::Uniform);

        bindGroups.push_back(utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, lastUniformBuffer, 0, sizeof(redColor)}}));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

            pass.SetPipeline(renderPipeline);

            for (uint32_t i = 0; i < heapSize; ++i) {
                pass.SetBindGroup(0, bindGroups[i]);
                pass.Draw(3);
            }

            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

// Verify encoding a heaps worth of bindgroups plus one more then reuse the first
// bindgroup in the same submit.
// Shader-visible heaps should switch out once then re-encode the first descriptor at a new
// offset in the heap.
TEST_P(D3D12DescriptorHeapTests, EncodeReuseUBOOverflow) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = mSimpleVSModule;
    pipelineDescriptor.cFragment.module = mSimpleFSModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    std::array<float, 4> redColor = {1, 0, 0, 1};
    wgpu::Buffer firstUniformBuffer = utils::CreateBufferFromData(
        device, &redColor, sizeof(redColor), wgpu::BufferUsage::Uniform);

    std::vector<wgpu::BindGroup> bindGroups = {utils::MakeBindGroup(
        device, pipeline.GetBindGroupLayout(0), {{0, firstUniformBuffer, 0, sizeof(redColor)}})};

    const uint32_t heapSize =
        mD3DDevice->GetViewShaderVisibleDescriptorAllocator()->GetShaderVisibleHeapSizeForTesting();

    for (uint32_t i = 0; i < heapSize; i++) {
        const std::array<float, 4>& fillColor = GetSolidColor(i + 1);  // Avoid black
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, &fillColor, sizeof(fillColor), wgpu::BufferUsage::Uniform);
        bindGroups.push_back(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, uniformBuffer, 0, sizeof(fillColor)}}));
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(pipeline);

        // Encode a heap worth of descriptors plus one more.
        for (uint32_t i = 0; i < heapSize + 1; ++i) {
            pass.SetBindGroup(0, bindGroups[i]);
            pass.Draw(3);
        }

        // Re-encode the first bindgroup again.
        pass.SetBindGroup(0, bindGroups[0]);
        pass.Draw(3);

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Make sure the first bindgroup was encoded correctly.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

// Verify encoding a heaps worth of bindgroups plus one more in the first submit then reuse the
// first bindgroup again in the second submit.
// Shader-visible heaps should switch out once then re-encode the
// first descriptor at the same offset in the heap.
TEST_P(D3D12DescriptorHeapTests, EncodeReuseUBOMultipleSubmits) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = mSimpleVSModule;
    pipelineDescriptor.cFragment.module = mSimpleFSModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Encode heap worth of descriptors plus one more.
    std::array<float, 4> redColor = {1, 0, 0, 1};

    wgpu::Buffer firstUniformBuffer = utils::CreateBufferFromData(
        device, &redColor, sizeof(redColor), wgpu::BufferUsage::Uniform);

    std::vector<wgpu::BindGroup> bindGroups = {utils::MakeBindGroup(
        device, pipeline.GetBindGroupLayout(0), {{0, firstUniformBuffer, 0, sizeof(redColor)}})};

    const uint32_t heapSize =
        mD3DDevice->GetViewShaderVisibleDescriptorAllocator()->GetShaderVisibleHeapSizeForTesting();

    for (uint32_t i = 0; i < heapSize; i++) {
        std::array<float, 4> fillColor = GetSolidColor(i + 1);  // Avoid black
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, &fillColor, sizeof(fillColor), wgpu::BufferUsage::Uniform);

        bindGroups.push_back(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, uniformBuffer, 0, sizeof(fillColor)}}));
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

            pass.SetPipeline(pipeline);

            for (uint32_t i = 0; i < heapSize + 1; ++i) {
                pass.SetBindGroup(0, bindGroups[i]);
                pass.Draw(3);
            }

            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Re-encode the first bindgroup again.
    {
        std::array<float, 4> greenColor = {0, 1, 0, 1};
        queue.WriteBuffer(firstUniformBuffer, 0, &greenColor, sizeof(greenColor));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

            pass.SetPipeline(pipeline);

            pass.SetBindGroup(0, bindGroups[0]);
            pass.Draw(3);

            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Make sure the first bindgroup was re-encoded correctly.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 0);
}

// Verify encoding many sampler and ubo worth of bindgroups.
// Shader-visible heaps should switch out |kNumOfViewHeaps| times.
TEST_P(D3D12DescriptorHeapTests, EncodeManyUBOAndSamplers) {
    DAWN_TEST_UNSUPPORTED_IF(
        !mD3DDevice->IsToggleEnabled(native::Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

    // Create a solid filled texture.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kRTSize;
    descriptor.size.height = kRTSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                       wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    {
        utils::BasicRenderPass renderPass = utils::BasicRenderPass(kRTSize, kRTSize, texture);

        utils::ComboRenderPassDescriptor renderPassDesc({textureView});
        renderPassDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDesc.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};
        renderPass.renderPassInfo.cColorAttachments[0].view = textureView;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        auto pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.End();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        utils::RGBA8 filled(0, 255, 0, 255);
        EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 0, 0);
    }

    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            struct U {
                transform : mat2x2<f32>
            }
            @group(0) @binding(0) var<uniform> buffer0 : U;

            @vertex fn main(
                @builtin(vertex_index) VertexIndex : u32
            ) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0, -1.0)
                );
                return vec4f(buffer0.transform * (pos[VertexIndex]), 0.0, 1.0);
            })");
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            struct U {
                color : vec4f
            }
            @group(0) @binding(1) var sampler0 : sampler;
            @group(0) @binding(2) var texture0 : texture_2d<f32>;
            @group(0) @binding(3) var<uniform> buffer0 : U;

            @fragment fn main(
                @builtin(position) FragCoord : vec4f
            ) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, FragCoord.xy) + buffer0.color;
            })");

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        // Encode a heap worth of descriptors |kNumOfHeaps| times.
        constexpr float transform[] = {1.f, 0.f, 0.f, 1.f};
        wgpu::Buffer transformBuffer = utils::CreateBufferFromData(
            device, &transform, sizeof(transform), wgpu::BufferUsage::Uniform);

        wgpu::SamplerDescriptor samplerDescriptor;
        wgpu::Sampler sampler = device.CreateSampler(&samplerDescriptor);

        auto* viewAllocator = mD3DDevice->GetViewShaderVisibleDescriptorAllocator();
        auto* samplerAllocator = mD3DDevice->GetSamplerShaderVisibleDescriptorAllocator();

        const HeapVersionID viewHeapSerial = viewAllocator->GetShaderVisibleHeapSerialForTesting();
        const HeapVersionID samplerHeapSerial =
            samplerAllocator->GetShaderVisibleHeapSerialForTesting();

        const uint32_t viewHeapSize = viewAllocator->GetShaderVisibleHeapSizeForTesting();

        // "Small" view heap is always 2 x sampler heap size and encodes 3x the descriptors per
        // group. This means the count of heaps switches is determined by the total number of
        // views to encode. Compute the number of bindgroups to encode by counting the required
        // views for |kNumOfViewHeaps| heaps worth.
        constexpr uint32_t kViewsPerBindGroup = 3;
        constexpr uint32_t kNumOfViewHeaps = 5;

        const uint32_t numOfEncodedBindGroups =
            (viewHeapSize * kNumOfViewHeaps) / kViewsPerBindGroup;

        std::vector<wgpu::BindGroup> bindGroups;
        for (uint32_t i = 0; i < numOfEncodedBindGroups - 1; i++) {
            std::array<float, 4> fillColor = GetSolidColor(i + 1);  // Avoid black
            wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
                device, &fillColor, sizeof(fillColor), wgpu::BufferUsage::Uniform);

            bindGroups.push_back(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {{0, transformBuffer, 0, sizeof(transform)},
                                                       {1, sampler},
                                                       {2, textureView},
                                                       {3, uniformBuffer, 0, sizeof(fillColor)}}));
        }

        std::array<float, 4> redColor = {1, 0, 0, 1};
        wgpu::Buffer lastUniformBuffer = utils::CreateBufferFromData(
            device, &redColor, sizeof(redColor), wgpu::BufferUsage::Uniform);

        bindGroups.push_back(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, transformBuffer, 0, sizeof(transform)},
                                                   {1, sampler},
                                                   {2, textureView},
                                                   {3, lastUniformBuffer, 0, sizeof(redColor)}}));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(pipeline);

        for (uint32_t i = 0; i < numOfEncodedBindGroups; ++i) {
            pass.SetBindGroup(0, bindGroups[i]);
            pass.Draw(3);
        }

        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Final accumulated color is result of sampled + UBO color.
        utils::RGBA8 filled(255, 255, 0, 255);
        utils::RGBA8 notFilled(0, 0, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, kRTSize - 1, 0);

        EXPECT_EQ(viewAllocator->GetShaderVisiblePoolSizeForTesting(), kNumOfViewHeaps);
        EXPECT_EQ(viewAllocator->GetShaderVisibleHeapSerialForTesting(),
                  viewHeapSerial + HeapVersionID(kNumOfViewHeaps));

        EXPECT_EQ(samplerAllocator->GetShaderVisiblePoolSizeForTesting(), 0u);
        EXPECT_EQ(samplerAllocator->GetShaderVisibleHeapSerialForTesting(), samplerHeapSerial);
    }
}

// Verify a single allocate/deallocate.
// One non-shader visible heap will be created.
TEST_P(D3D12DescriptorHeapTests, Single) {
    constexpr uint32_t kDescriptorCount = 4;
    constexpr uint32_t kAllocationsPerHeap = 3;
    PlaceholderStagingDescriptorAllocator allocator(mD3DDevice, kDescriptorCount,
                                                    kAllocationsPerHeap);

    CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
    EXPECT_EQ(allocation.GetHeapIndex(), 0u);
    EXPECT_NE(allocation.OffsetFrom(0, 0).ptr, 0u);

    allocator.Deallocate(allocation);
    EXPECT_FALSE(allocation.IsValid());
}

// Verify allocating many times causes the pool to increase in size.
// Creates |kNumOfHeaps| non-shader visible heaps.
TEST_P(D3D12DescriptorHeapTests, Sequential) {
    constexpr uint32_t kDescriptorCount = 4;
    constexpr uint32_t kAllocationsPerHeap = 3;
    PlaceholderStagingDescriptorAllocator allocator(mD3DDevice, kDescriptorCount,
                                                    kAllocationsPerHeap);

    // Allocate |kNumOfHeaps| worth.
    constexpr uint32_t kNumOfHeaps = 2;

    std::set<uint32_t> allocatedHeaps;

    std::vector<CPUDescriptorHeapAllocation> allocations;
    for (uint32_t i = 0; i < kAllocationsPerHeap * kNumOfHeaps; i++) {
        CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
        EXPECT_EQ(allocation.GetHeapIndex(), i / kAllocationsPerHeap);
        EXPECT_NE(allocation.OffsetFrom(0, 0).ptr, 0u);
        allocations.push_back(allocation);
        allocatedHeaps.insert(allocation.GetHeapIndex());
    }

    EXPECT_EQ(allocatedHeaps.size(), kNumOfHeaps);

    // Deallocate all.
    for (CPUDescriptorHeapAllocation& allocation : allocations) {
        allocator.Deallocate(allocation);
        EXPECT_FALSE(allocation.IsValid());
    }
}

// Verify that re-allocating a number of allocations < pool size, all heaps are reused.
// Creates and reuses |kNumofHeaps| non-shader visible heaps.
TEST_P(D3D12DescriptorHeapTests, ReuseFreedHeaps) {
    constexpr uint32_t kDescriptorCount = 4;
    constexpr uint32_t kAllocationsPerHeap = 25;
    PlaceholderStagingDescriptorAllocator allocator(mD3DDevice, kDescriptorCount,
                                                    kAllocationsPerHeap);

    constexpr uint32_t kNumofHeaps = 10;

    std::list<CPUDescriptorHeapAllocation> allocations;
    std::set<size_t> allocationPtrs;

    // Allocate |kNumofHeaps| heaps worth.
    for (uint32_t i = 0; i < kAllocationsPerHeap * kNumofHeaps; i++) {
        CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
        allocations.push_back(allocation);
        EXPECT_TRUE(allocationPtrs.insert(allocation.OffsetFrom(0, 0).ptr).second);
    }

    // Deallocate all.
    for (CPUDescriptorHeapAllocation& allocation : allocations) {
        allocator.Deallocate(allocation);
        EXPECT_FALSE(allocation.IsValid());
    }

    allocations.clear();

    // Re-allocate all again.
    std::set<size_t> reallocatedPtrs;
    for (uint32_t i = 0; i < kAllocationsPerHeap * kNumofHeaps; i++) {
        CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
        allocations.push_back(allocation);
        EXPECT_TRUE(reallocatedPtrs.insert(allocation.OffsetFrom(0, 0).ptr).second);
        EXPECT_TRUE(std::find(allocationPtrs.begin(), allocationPtrs.end(),
                              allocation.OffsetFrom(0, 0).ptr) != allocationPtrs.end());
    }

    // Deallocate all again.
    for (CPUDescriptorHeapAllocation& allocation : allocations) {
        allocator.Deallocate(allocation);
        EXPECT_FALSE(allocation.IsValid());
    }
}

// Verify allocating then deallocating many times.
TEST_P(D3D12DescriptorHeapTests, AllocateDeallocateMany) {
    constexpr uint32_t kDescriptorCount = 4;
    constexpr uint32_t kAllocationsPerHeap = 25;
    PlaceholderStagingDescriptorAllocator allocator(mD3DDevice, kDescriptorCount,
                                                    kAllocationsPerHeap);

    std::list<CPUDescriptorHeapAllocation> list3;
    std::list<CPUDescriptorHeapAllocation> list5;
    std::list<CPUDescriptorHeapAllocation> allocations;

    constexpr uint32_t kNumofHeaps = 2;

    // Allocate |kNumofHeaps| heaps worth.
    for (uint32_t i = 0; i < kAllocationsPerHeap * kNumofHeaps; i++) {
        CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
        EXPECT_NE(allocation.OffsetFrom(0, 0).ptr, 0u);
        if (i % 3 == 0) {
            list3.push_back(allocation);
        } else {
            allocations.push_back(allocation);
        }
    }

    // Deallocate every 3rd allocation.
    for (auto it = list3.begin(); it != list3.end(); it = list3.erase(it)) {
        allocator.Deallocate(*it);
    }

    // Allocate again.
    for (uint32_t i = 0; i < kAllocationsPerHeap * kNumofHeaps; i++) {
        CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
        EXPECT_NE(allocation.OffsetFrom(0, 0).ptr, 0u);
        if (i % 5 == 0) {
            list5.push_back(allocation);
        } else {
            allocations.push_back(allocation);
        }
    }

    // Deallocate every 5th allocation.
    for (auto it = list5.begin(); it != list5.end(); it = list5.erase(it)) {
        allocator.Deallocate(*it);
    }

    // Allocate again.
    for (uint32_t i = 0; i < kAllocationsPerHeap * kNumofHeaps; i++) {
        CPUDescriptorHeapAllocation allocation = allocator.AllocateCPUDescriptors();
        EXPECT_NE(allocation.OffsetFrom(0, 0).ptr, 0u);
        allocations.push_back(allocation);
    }

    // Deallocate remaining.
    for (CPUDescriptorHeapAllocation& allocation : allocations) {
        allocator.Deallocate(allocation);
        EXPECT_FALSE(allocation.IsValid());
    }
}

// Verifies that gpu descriptor heap allocations are only valid during the serial they were created
// on.
TEST_P(D3D12DescriptorHeapTests, InvalidateAllocationAfterSerial) {
    auto* gpuAllocator = mD3DDevice->GetViewShaderVisibleDescriptorAllocator();

    GPUDescriptorHeapAllocation gpuHeapDescAllocation;

    D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor;
    gpuAllocator->AllocateGPUDescriptors(1, mD3DDevice->GetQueue()->GetPendingCommandSerial(),
                                         &baseCPUDescriptor, &gpuHeapDescAllocation);

    EXPECT_TRUE(gpuAllocator->IsAllocationStillValid(gpuHeapDescAllocation));

    EXPECT_TRUE(mD3DQueue->NextSerial().IsSuccess());

    EXPECT_FALSE(gpuAllocator->IsAllocationStillValid(gpuHeapDescAllocation));
}

DAWN_INSTANTIATE_TEST(D3D12DescriptorHeapTests,
                      D3D12Backend(),
                      D3D12Backend({"use_d3d12_small_shader_visible_heap"}));

class D3D12ResourceTableDescriptorHeapTests : public D3D12DescriptorHeapTests {
  protected:
    // Number of descriptors implicitly allocated by a ResourceTable, which are
    // for the default resources plus one for the metadata buffer.
    uint32_t mImplicitDescriptorCount;

    void SetUp() override {
        D3D12DescriptorHeapTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable}));

        // Override the fragment shader from base
        mSimpleFSModule = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @fragment fn main() -> @location(0) vec4f {
                _ = hasResource<texture_2d<u32>>(0);
                return vec4f(0);
            })");

        mImplicitDescriptorCount = uint32_t{ResourceTableDefaultResources::GetCount()} + 1;
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable})) {
            return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
        }
        return {};
    }

    wgpu::ResourceTable MakeResourceTable(uint32_t size) {
        wgpu::ResourceTableDescriptor desc;
        desc.size = size;
        wgpu::ResourceTable table = device.CreateResourceTable(&desc);
        return table;
    }

    wgpu::PipelineLayout MakePipelineLayoutWithTable(std::vector<wgpu::BindGroupLayout> bgls = {},
                                                     uint32_t immediateSize = 0) {
        wgpu::PipelineLayoutResourceTable plTable;
        plTable.usesResourceTable = true;

        wgpu::PipelineLayoutDescriptor desc{
            .nextInChain = &plTable,
            .bindGroupLayoutCount = bgls.size(),
            .bindGroupLayouts = bgls.data(),
            .immediateSize = immediateSize,
        };

        return device.CreatePipelineLayout(&desc);
    }
};

// Verify the shader visible view heaps switch over every submit when binding a resource table that
// doubles in size
TEST_P(D3D12ResourceTableDescriptorHeapTests, SwitchOverViewHeapGradually) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.layout = MakePipelineLayoutWithTable();
    renderPipelineDescriptor.vertex.module = mSimpleVSModule;
    renderPipelineDescriptor.cFragment.module = mSimpleFSModule;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetViewShaderVisibleDescriptorAllocator();
    const uint64_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();
    DAWN_ASSERT(
        heapSize >
        mImplicitDescriptorCount);  // Don't test with UseD3D12SmallShaderVisibleHeapForTesting

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    uint32_t tableSize = heapSize;
    for (int i = 0;; ++i) {
        wgpu::ResourceTable table = MakeResourceTable(tableSize - mImplicitDescriptorCount);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetResourceTable(table);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(i));

        // Make the table grow so that each iteration is double the previous heap size
        // to force the allocator to grow by double its current size.
        tableSize *= 2;
        if ((tableSize - mImplicitDescriptorCount) >= kMaxResourceTableSize) {
            break;
        }
    }
}

// Verify the shader visible view heaps switch over every submit when binding a resource table that
// quadruples in size
TEST_P(D3D12ResourceTableDescriptorHeapTests, SwitchOverViewHeapLargeJumps) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.layout = MakePipelineLayoutWithTable();
    renderPipelineDescriptor.vertex.module = mSimpleVSModule;
    renderPipelineDescriptor.cFragment.module = mSimpleFSModule;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetViewShaderVisibleDescriptorAllocator();
    const uint64_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();
    DAWN_ASSERT(
        heapSize >
        mImplicitDescriptorCount);  // Don't test with UseD3D12SmallShaderVisibleHeapForTesting

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    uint32_t tableSize = heapSize;
    for (int i = 0;; ++i) {
        wgpu::ResourceTable table = MakeResourceTable(tableSize - mImplicitDescriptorCount);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetResourceTable(table);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(i));

        // Make the table grow so that each iteration is quadruple the previous heap size
        // to force the allocator to grow by quadruple its current size.
        tableSize *= 4;
        if ((tableSize - mImplicitDescriptorCount) >= kMaxResourceTableSize) {
            break;
        }
    }
}

// Verify the shader visible view heaps switch over every submit when binding a resource table that
// goes from smallest to largest size
TEST_P(D3D12ResourceTableDescriptorHeapTests, SwitchOverViewHeapLargestJump) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.layout = MakePipelineLayoutWithTable();
    renderPipelineDescriptor.vertex.module = mSimpleVSModule;
    renderPipelineDescriptor.cFragment.module = mSimpleFSModule;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetViewShaderVisibleDescriptorAllocator();
    const uint64_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();
    DAWN_ASSERT(
        heapSize >
        mImplicitDescriptorCount);  // Don't test with UseD3D12SmallShaderVisibleHeapForTesting

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    auto draw = [&](wgpu::ResourceTable table) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetResourceTable(table);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    // Make min size table
    {
        wgpu::ResourceTable table = MakeResourceTable(heapSize - mImplicitDescriptorCount);
        draw(table);
        EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(0));
    }
    // Make largest table
    {
        wgpu::ResourceTable table =
            MakeResourceTable(kMaxResourceTableSize - mImplicitDescriptorCount);
        draw(table);
        EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(1));
    }
}

// Verify the shader visible view heaps switch over every submit when binding a resource table that
// goes from smallest to largest size
TEST_P(D3D12ResourceTableDescriptorHeapTests, SwitchOverViewHeapTableAndBindGroups) {
    auto fsModule = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> buffer : array<u32>;

        @fragment fn main() -> @location(0) vec4f {
            _ = hasResource<texture_2d<u32>>(0);
            return vec4f(0);
        })");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.layout = MakePipelineLayoutWithTable({{bgl}});
    renderPipelineDescriptor.vertex.module = mSimpleVSModule;
    renderPipelineDescriptor.cFragment.module = fsModule;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    auto* allocator = d3dDevice->GetViewShaderVisibleDescriptorAllocator();
    const uint64_t heapSize = allocator->GetShaderVisibleHeapSizeForTesting();
    DAWN_ASSERT(
        heapSize >
        mImplicitDescriptorCount);  // Don't test with UseD3D12SmallShaderVisibleHeapForTesting

    const HeapVersionID heapSerial = allocator->GetShaderVisibleHeapSerialForTesting();

    // Create a bind group with a buffer
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer buffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}});

    // Make a table that's exactly double the heap size (e.g. 8192), which should cause
    // a realloc. With the bind group of 1 resource, the realloc min size will be heapSize * 2 + 1
    // (e.g. 8193), which should result in a realloc of heapSize * 4 (e.g. 16384).
    wgpu::ResourceTable table = MakeResourceTable((heapSize * 2) - mImplicitDescriptorCount);

    // Draw with both the table and bind group. This should result in a heap realloc.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(renderPipeline);
    pass.SetBindGroup(0, bg);
    pass.SetResourceTable(table);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapSerialForTesting(), heapSerial + HeapVersionID(1));
    EXPECT_EQ(allocator->GetShaderVisibleHeapSizeForTesting(), heapSize * 4);
}

DAWN_INSTANTIATE_TEST(D3D12ResourceTableDescriptorHeapTests, D3D12Backend(), );

}  // anonymous namespace
}  // namespace dawn::native::d3d12
