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

#include <utility>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ResourceUsageTrackingTest : public ValidationTest {
  protected:
    wgpu::Buffer CreateBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateTexture(wgpu::TextureUsage usage,
                                wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size = {1, 1, 1};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = usage;
        descriptor.format = format;

        return device.CreateTexture(&descriptor);
    }

    // Note that it is valid to bind any bind groups for indices that the pipeline doesn't use.
    // We create a no-op render or compute pipeline without any bindings, and set bind groups
    // in the caller, so it is always correct for binding validation between bind groups and
    // pipeline. But those bind groups in caller can be used for validation for other purposes.
    wgpu::RenderPipeline CreateNoOpRenderPipeline() {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                @fragment fn main() {
                })");
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, nullptr);
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateNoOpComputePipeline(std::vector<wgpu::BindGroupLayout> bgls) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
                @compute @workgroup_size(1) fn main() {
                })");
        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.layout = utils::MakePipelineLayout(device, std::move(bgls));
        pipelineDescriptor.compute.module = csModule;
        return device.CreateComputePipeline(&pipelineDescriptor);
    }

    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;
};

// Test that using a single buffer in multiple read usages in the same pass is allowed.
TEST_F(ResourceUsageTrackingTest, BufferWithMultipleReadUsage) {
    // Test render pass
    {
        // Create a buffer, and use the buffer as both vertex and index buffer.
        wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32);
        pass.SetVertexBuffer(0, buffer);
        pass.End();
        encoder.Finish();
    }

    // Test compute pass
    {
        // Create buffer and bind group
        wgpu::Buffer buffer =
            CreateBuffer(4, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);

        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                     {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}, {1, buffer}});

        // Use the buffer as both uniform and readonly storage buffer in compute pass.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.End();
        encoder.Finish();
    }
}

// Test that it is invalid to use the same buffer as both readable and writable in the same
// render pass. It is invalid in the same dispatch in compute pass.
TEST_F(ResourceUsageTrackingTest, BufferWithReadAndWriteUsage) {
    // test render pass
    {
        // Create buffer and bind group
        wgpu::Buffer buffer =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);

        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}});

        // It is invalid to use the buffer as both index and storage in render pass
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // test compute pass
    {
        // Create buffer and bind group
        wgpu::Buffer buffer = CreateBuffer(512, wgpu::BufferUsage::Storage);

        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                     {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroup bg =
            utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}, {1, buffer, 256, 4}});

        // Create a no-op compute pipeline
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // It is valid to use the buffer as both storage and readonly storage in a single
        // compute pass if dispatch command is not called.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, bg);
            pass.End();
            encoder.Finish();
        }

        // It is invalid to use the buffer as both storage and readonly storage in a single
        // dispatch.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(cp);
            pass.SetBindGroup(0, bg);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Test the use of a buffer as a storage buffer multiple times in the same synchronization
// scope.
TEST_F(ResourceUsageTrackingTest, BufferUsedAsStorageMultipleTimes) {
    // Create buffer and bind group
    wgpu::Buffer buffer = CreateBuffer(512, wgpu::BufferUsage::Storage);

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute,
                  wgpu::BufferBindingType::Storage},
                 {1, wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute,
                  wgpu::BufferBindingType::Storage}});
    wgpu::BindGroup bg =
        utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}, {1, buffer, 256, 4}});

    // test render pass
    {
        // It is valid to use multiple storage usages on the same buffer in render pass
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetBindGroup(0, bg);
        pass.End();
        encoder.Finish();
    }

    // test compute pass
    {
        // It is valid to use multiple storage usages on the same buffer in a dispatch
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bg);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }
}

// Test that using the same buffer as both readable and writable in different passes is allowed
TEST_F(ResourceUsageTrackingTest, BufferWithReadAndWriteUsageInDifferentPasses) {
    // Test render pass
    {
        // Create buffers that will be used as index and storage buffers
        wgpu::Buffer buffer0 =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
        wgpu::Buffer buffer1 =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);

        // Create bind groups to use the buffer as storage
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, buffer0}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, buffer1}});

        // Use these two buffers as both index and storage in different render passes
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);

        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass0.SetIndexBuffer(buffer0, wgpu::IndexFormat::Uint32);
        pass0.SetBindGroup(0, bg1);
        pass0.End();

        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass1.SetIndexBuffer(buffer1, wgpu::IndexFormat::Uint32);
        pass1.SetBindGroup(0, bg0);
        pass1.End();

        encoder.Finish();
    }

    // Test compute pass
    {
        // Create buffer and bind groups that will be used as storage and uniform bindings
        wgpu::Buffer buffer =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform);

        wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, buffer}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, buffer}});

        // Use the buffer as both storage and uniform in different compute passes
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
        pass0.SetBindGroup(0, bg0);
        pass0.End();

        wgpu::ComputePassEncoder pass1 = encoder.BeginComputePass();
        pass1.SetBindGroup(1, bg1);
        pass1.End();

        encoder.Finish();
    }

    // Test render pass and compute pass mixed together with resource dependency.
    {
        // Create buffer and bind groups that will be used as storage and uniform bindings
        wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Storage);

        wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, buffer}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, buffer}});

        // Use the buffer as storage and uniform in render pass and compute pass respectively
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
        pass0.SetBindGroup(0, bg0);
        pass0.End();

        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass1.SetBindGroup(1, bg1);
        pass1.End();

        encoder.Finish();
    }
}

// Test that it is invalid to use the same buffer as both readable and writable in different
// draws in a single render pass. But it is valid in different dispatches in a single compute
// pass.
TEST_F(ResourceUsageTrackingTest, BufferWithReadAndWriteUsageInDifferentDrawsOrDispatches) {
    // Test render pass
    {
        // Create a buffer and a bind group
        wgpu::Buffer buffer =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}});

        // Create a no-op render pipeline.
        wgpu::RenderPipeline rp = CreateNoOpRenderPipeline();

        // It is not allowed to use the same buffer as both readable and writable in different
        // draws within the same render pass.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetPipeline(rp);

        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32);
        pass.Draw(3);

        pass.SetBindGroup(0, bg);
        pass.Draw(3);

        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // test compute pass
    {
        // Create a buffer and bind groups
        wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Storage);

        wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, buffer}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, buffer}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp0 = CreateNoOpComputePipeline({bgl0});
        wgpu::ComputePipeline cp1 = CreateNoOpComputePipeline({bgl1});

        // It is valid to use the same buffer as both readable and writable in different
        // dispatches within the same compute pass.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        pass.SetPipeline(cp0);
        pass.SetBindGroup(0, bg0);
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(cp1);
        pass.SetBindGroup(0, bg1);
        pass.DispatchWorkgroups(1);

        pass.End();
        encoder.Finish();
    }
}

// Test that it is invalid to use the same buffer as both readable and writable in a single
// draw or dispatch.
TEST_F(ResourceUsageTrackingTest, BufferWithReadAndWriteUsageInSingleDrawOrDispatch) {
    // Test render pass
    {
        // Create a buffer and a bind group
        wgpu::Buffer buffer =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, buffer}});

        // Create a no-op render pipeline.
        wgpu::RenderPipeline rp = CreateNoOpRenderPipeline();

        // It is invalid to use the same buffer as both readable and writable usages in a single
        // draw
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetPipeline(rp);

        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32);
        pass.SetBindGroup(0, writeBG);
        pass.Draw(3);

        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // test compute pass
    {
        // Create a buffer and bind groups
        wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Storage);

        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, buffer}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, buffer}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({readBGL, writeBGL});

        // It is invalid to use the same buffer as both readable and writable usages in a single
        // dispatch
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);

        pass.SetBindGroup(0, readBG);
        pass.SetBindGroup(1, writeBG);
        pass.DispatchWorkgroups(1);

        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that using the same buffer as copy src/dst and writable/readable usage is allowed.
TEST_F(ResourceUsageTrackingTest, BufferCopyAndBufferUsageInPass) {
    // Create buffers that will be used as both a copy src/dst buffer and a storage buffer
    wgpu::Buffer bufferSrc =
        CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
    wgpu::Buffer bufferDst =
        CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    // Create the bind group to use the buffer as storage
    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, bufferSrc}});
    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
    wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, bufferDst}});

    // Use the buffer as both copy src and storage in render pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufferSrc, 0, bufferDst, 0, 4);
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetBindGroup(0, bg0);
        pass.End();
        encoder.Finish();
    }

    // Use the buffer as both copy dst and readonly storage in compute pass
    {
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl1});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufferSrc, 0, bufferDst, 0, 4);

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg1);
        pass.SetPipeline(cp);
        pass.DispatchWorkgroups(1);
        pass.End();

        encoder.Finish();
    }
}

// Test that all index buffers and vertex buffers take effect even though some buffers are
// not used because they are overwritten by another consecutive call.
TEST_F(ResourceUsageTrackingTest, BufferWithMultipleSetIndexOrVertexBuffer) {
    // Create buffers that will be used as both vertex and index buffer.
    wgpu::Buffer buffer0 = CreateBuffer(
        4, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index | wgpu::BufferUsage::Storage);
    wgpu::Buffer buffer1 = CreateBuffer(4, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index);

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer0}});

    PlaceholderRenderPass PlaceholderRenderPass(device);

    // Set index buffer twice. The second one overwrites the first one. No buffer is used as
    // both read and write in the same pass. But the overwritten index buffer (buffer0) still
    // take effect during resource tracking.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetIndexBuffer(buffer0, wgpu::IndexFormat::Uint32);
        pass.SetIndexBuffer(buffer1, wgpu::IndexFormat::Uint32);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Set index buffer twice. The second one overwrites the first one. buffer0 is used as both
    // read and write in the same pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetIndexBuffer(buffer1, wgpu::IndexFormat::Uint32);
        pass.SetIndexBuffer(buffer0, wgpu::IndexFormat::Uint32);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Set vertex buffer on the same index twice. The second one overwrites the first one. No
    // buffer is used as both read and write in the same pass. But the overwritten vertex buffer
    // (buffer0) still take effect during resource tracking.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetVertexBuffer(0, buffer0);
        pass.SetVertexBuffer(0, buffer1);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Set vertex buffer on the same index twice. The second one overwrites the first one.
    // buffer0 is used as both read and write in the same pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetVertexBuffer(0, buffer1);
        pass.SetVertexBuffer(0, buffer0);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that all consecutive SetBindGroup()s take effect even though some bind groups are not
// used because they are overwritten by a consecutive call.
TEST_F(ResourceUsageTrackingTest, BufferWithMultipleSetBindGroupsOnSameIndex) {
    // test render pass
    {
        // Create buffers that will be used as index and storage buffers
        wgpu::Buffer buffer0 =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
        wgpu::Buffer buffer1 =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);

        // Create the bind group to use the buffer as storage
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, buffer0}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, buffer1}});

        PlaceholderRenderPass PlaceholderRenderPass(device);

        // Set bind group on the same index twice. The second one overwrites the first one.
        // No buffer is used as both read and write in the same pass. But the overwritten
        // bind group still take effect during resource tracking.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
            pass.SetIndexBuffer(buffer0, wgpu::IndexFormat::Uint32);
            pass.SetBindGroup(0, bg0);
            pass.SetBindGroup(0, bg1);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }

        // Set bind group on the same index twice. The second one overwrites the first one.
        // buffer0 is used as both read and write in the same pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
            pass.SetIndexBuffer(buffer0, wgpu::IndexFormat::Uint32);
            pass.SetBindGroup(0, bg1);
            pass.SetBindGroup(0, bg0);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    // test compute pass
    {
        // Create buffers that will be used as readonly and writable storage buffers
        wgpu::Buffer buffer0 = CreateBuffer(512, wgpu::BufferUsage::Storage);
        wgpu::Buffer buffer1 = CreateBuffer(4, wgpu::BufferUsage::Storage);

        // Create the bind group to use the buffer as storage
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroup writeBG0 = utils::MakeBindGroup(device, writeBGL, {{0, buffer0, 0, 4}});
        wgpu::BindGroup readBG0 = utils::MakeBindGroup(device, readBGL, {{0, buffer0, 256, 4}});
        wgpu::BindGroup readBG1 = utils::MakeBindGroup(device, readBGL, {{0, buffer1, 0, 4}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({writeBGL, readBGL});

        // Set bind group against the same index twice. The second one overwrites the first one.
        // Then no buffer is used as both read and write in the same dispatch. But the
        // overwritten bind group still take effect.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, writeBG0);
            pass.SetBindGroup(1, readBG0);
            pass.SetBindGroup(1, readBG1);
            pass.SetPipeline(cp);
            pass.DispatchWorkgroups(1);
            pass.End();
            encoder.Finish();
        }

        // Set bind group against the same index twice. The second one overwrites the first one.
        // Then buffer0 is used as both read and write in the same dispatch
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, writeBG0);
            pass.SetBindGroup(1, readBG1);
            pass.SetBindGroup(1, readBG0);
            pass.SetPipeline(cp);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Test that it is invalid to have resource usage conflicts even when all bindings are not
// visible to the programmable pass where it is used.
TEST_F(ResourceUsageTrackingTest, BufferUsageConflictBetweenInvisibleStagesInBindGroup) {
    wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Storage);

    // Test render pass for bind group. The conflict of readonly storage and storage usage
    // doesn't reside in render related stages at all
    {
        // Create a bind group whose bindings are not visible in render pass
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                     {1, wgpu::ShaderStage::None, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}, {1, buffer}});

        // These two bindings are invisible in render pass. But we still track these bindings.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass for bind group. The conflict of readonly storage and storage usage
    // doesn't reside in compute related stage at all
    {
        // Create a bind group whose bindings are not visible in compute pass
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage},
                     {1, wgpu::ShaderStage::None, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}, {1, buffer}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // These two bindings are invisible in the dispatch. But we still track these bindings.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bg);
        pass.DispatchWorkgroups(1);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that it is invalid to have resource usage conflicts even when one of the bindings is not
// visible to the programmable pass where it is used.
TEST_F(ResourceUsageTrackingTest, BufferUsageConflictWithInvisibleStageInBindGroup) {
    // Test render pass for bind group and index buffer. The conflict of storage and index
    // buffer usage resides between fragment stage and compute stage. But the compute stage
    // binding is not visible in render pass.
    {
        wgpu::Buffer buffer =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}});

        // Buffer usage in compute stage in bind group conflicts with index buffer. And binding
        // for compute stage is not visible in render pass. But we still track this binding.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass for bind group. The conflict of readonly storage and storage buffer
    // usage resides between compute stage and fragment stage. But the fragment stage binding is
    // not visible in the dispatch.
    {
        wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Storage);
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage},
                     {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer}, {1, buffer}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // Buffer usage in compute stage conflicts with buffer usage in fragment stage. And
        // binding for fragment stage is not visible in the dispatch. But we still track this
        // invisible binding.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bg);
        pass.DispatchWorkgroups(1);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that it is invalid to have resource usage conflicts even when one of the bindings is not
// used in the pipeline.
TEST_F(ResourceUsageTrackingTest, BufferUsageConflictWithUnusedPipelineBindings) {
    wgpu::Buffer buffer = CreateBuffer(4, wgpu::BufferUsage::Storage);

    // Test render pass for bind groups with unused bindings. The conflict of readonly storage
    // and storage usages resides in different bind groups, although some bindings may not be
    // used because its bind group layout is not designated in pipeline layout.
    {
        // Create bind groups. The bindings are visible for render pass.
        wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, buffer}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, buffer}});

        // Create a passthrough render pipeline with a readonly buffer
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                struct RBuffer {
                    value : f32
                }
                @group(0) @binding(0) var<storage, read> rBuffer : RBuffer;
                @fragment fn main() {
                })");
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl0);
        wgpu::RenderPipeline rp = device.CreateRenderPipeline(&pipelineDescriptor);

        // Resource in bg1 conflicts with resources used in bg0. However, bindings in bg1 is
        // not used in pipeline. But we still track this binding.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetPipeline(rp);
        pass.Draw(3);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test that an unused bind group is not used to detect conflicts between bindings in
    // compute passes.
    {
        // Create bind groups. The bindings are visible for compute pass.
        wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, buffer}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, buffer}});

        // Create a compute pipeline with only one of the two BGLs.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl0});

        // Resource in bg1 conflicts with resources used in bg0. However, the binding in bg1 is
        // not used in pipeline so no error is produced in the dispatch.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetPipeline(cp);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }
}

// Test that it is invalid to use the same texture as both readable and writable in the same
// render pass. It is invalid in the same dispatch in compute pass.
TEST_F(ResourceUsageTrackingTest, TextureWithReadAndWriteUsage) {
    // Test render pass
    {
        // Create a texture
        wgpu::Texture texture = CreateTexture(wgpu::TextureUsage::TextureBinding |
                                              wgpu::TextureUsage::RenderAttachment);
        wgpu::TextureView view = texture.CreateView();

        // Create a bind group to use the texture as sampled binding
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::TextureSampleType::Float}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}});

        // Create a render pass to use the texture as a render target
        utils::ComboRenderPassDescriptor renderPass({view});

        // It is invalid to use the texture as both sampled and render target in the same pass
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass
    {
        // Create a texture
        wgpu::Texture texture =
            CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
        wgpu::TextureView view = texture.CreateView();

        // Create a bind group to use the texture as sampled and writeonly bindings
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float},
             {1, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}, {1, view}});

        // Create a no-op compute pipeline
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // It is valid to use the texture as both sampled and writeonly storage in a single
        // compute pass if dispatch command is not called.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, bg);
            pass.End();
            encoder.Finish();
        }

        // It is invalid to use the texture as both sampled and writeonly storage in a single
        // dispatch
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(cp);
            pass.SetBindGroup(0, bg);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Test that it is invalid to use the same texture as both readable and writable depth/stencil
// attachment in the same render pass. But it is valid to use it as both readable and readonly
// depth/stencil attachment in the same render pass.
// Note that depth/stencil attachment is a special render attachment, it can be readonly.
TEST_F(ResourceUsageTrackingTest, TextureWithSamplingAndDepthStencilAttachment) {
    // Create a texture
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
                      wgpu::TextureFormat::Depth32Float);
    wgpu::TextureView view = texture.CreateView();

    // Create a bind group to use the texture as sampled binding
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth}});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}});

    // Create a render pass to use the texture as a render target
    utils::ComboRenderPassDescriptor passDescriptor({}, view);
    passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
    passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
    passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // It is invalid to use the texture as both sampled and writeable depth/stencil attachment
    // in the same pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // It is valid to use the texture as both sampled and readonly depth/stencil attachment in
    // the same pass
    {
        passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = true;
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetBindGroup(0, bg);
        pass.End();
        encoder.Finish();
    }
}

// Test that it is valid to use a depth-stencil texture in mixed readonly and writable attachment
TEST_F(ResourceUsageTrackingTest, MixedReadOnlyAndNotAttachment) {
    // Create the depth stencil texture and views.
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
                      wgpu::TextureFormat::Depth24PlusStencil8);

    wgpu::TextureViewDescriptor viewDesc = {};

    viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    wgpu::TextureView depthView = texture.CreateView(&viewDesc);
    viewDesc.aspect = wgpu::TextureAspect::StencilOnly;
    wgpu::TextureView stencilView = texture.CreateView(&viewDesc);
    viewDesc.aspect = wgpu::TextureAspect::All;
    wgpu::TextureView depthStencilView = texture.CreateView(&viewDesc);

    // Create a bind group.
    wgpu::BindGroupLayout depthBgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth}});
    wgpu::BindGroup depthBg = utils::MakeBindGroup(device, depthBgl, {{0, depthView}});

    wgpu::BindGroupLayout stencilBgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint}});
    wgpu::BindGroup stencilBg = utils::MakeBindGroup(device, stencilBgl, {{0, stencilView}});

    // It is valid to use attachments with depth readonly+sampled and stencil written.
    {
        utils::ComboRenderPassDescriptor passDesc({}, depthStencilView);
        passDesc.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        passDesc.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        passDesc.cDepthStencilAttachmentInfo.depthReadOnly = true;

        passDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        passDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        passDesc.cDepthStencilAttachmentInfo.stencilReadOnly = false;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
        pass.SetBindGroup(0, depthBg);
        pass.End();
        encoder.Finish();
    }

    // It is valid to use attachments with depth written and stencil readonly+sampled.
    {
        utils::ComboRenderPassDescriptor passDesc({}, depthStencilView);
        passDesc.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        passDesc.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        passDesc.cDepthStencilAttachmentInfo.depthReadOnly = false;

        passDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        passDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        passDesc.cDepthStencilAttachmentInfo.stencilReadOnly = true;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
        pass.SetBindGroup(0, stencilBg);
        pass.End();
        encoder.Finish();
    }
}

// Test using multiple writable usages on the same texture in a single pass/dispatch
TEST_F(ResourceUsageTrackingTest, TextureWithMultipleWriteUsage) {
    // Test render pass
    {
        // Create a texture
        wgpu::Texture texture = CreateTexture(wgpu::TextureUsage::StorageBinding |
                                              wgpu::TextureUsage::RenderAttachment);
        wgpu::TextureView view = texture.CreateView();

        // Create a bind group to use the texture as writeonly storage binding
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}});

        // It is invalid to use the texture as both writeonly storage and render target in
        // the same pass
        {
            utils::ComboRenderPassDescriptor renderPass({view});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetBindGroup(0, bg);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }

        // It is valid to use multiple writeonly storage usages on the same texture in render
        // pass
        {
            wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, view}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            PlaceholderRenderPass PlaceholderRenderPass(device);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
            pass.SetBindGroup(0, bg);
            pass.SetBindGroup(1, bg1);
            pass.End();
            encoder.Finish();
        }
    }

    // Test compute pass
    {
        // Create a texture
        wgpu::Texture texture = CreateTexture(wgpu::TextureUsage::StorageBinding);
        wgpu::TextureView view = texture.CreateView();

        // Create a bind group to use the texture as sampled and writeonly bindings
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        // Create 2 bind groups with same texture subresources and dispatch twice to avoid
        // storage texture binding aliasing
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, view}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, view}});

        // Create a no-op compute pipeline
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // It is valid to use the texture as multiple writeonly storage usages in a single
        // dispatch
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bg0);
        pass.DispatchWorkgroups(1);
        pass.SetBindGroup(0, bg1);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }
}

// Test that a single subresource of a texture cannot be used as a render attachment more than
// once in the same pass.
TEST_F(ResourceUsageTrackingTest, TextureWithMultipleRenderAttachmentUsage) {
    // Create a texture with two array layers
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {1, 1, 2};
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    descriptor.format = kFormat;

    wgpu::Texture texture = device.CreateTexture(&descriptor);

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.arrayLayerCount = 1;

    wgpu::TextureView viewLayer0 = texture.CreateView(&viewDesc);

    viewDesc.baseArrayLayer = 1;
    wgpu::TextureView viewLayer1 = texture.CreateView(&viewDesc);

    // Control: It is valid to use layer0 as a render target for one attachment, and
    // layer1 as the second attachment in the same pass
    {
        utils::ComboRenderPassDescriptor renderPass({viewLayer0, viewLayer1});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        encoder.Finish();
    }

    // Control: It is valid to use layer0 as a render target in separate passes.
    {
        utils::ComboRenderPassDescriptor renderPass({viewLayer0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass);
        pass0.End();
        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass);
        pass1.End();
        encoder.Finish();
    }

    // It is invalid to use layer0 as a render target for both attachments in the same pass
    {
        utils::ComboRenderPassDescriptor renderPass({viewLayer0, viewLayer0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // It is invalid to use layer1 as a render target for both attachments in the same pass
    {
        utils::ComboRenderPassDescriptor renderPass({viewLayer1, viewLayer1});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that using the same texture as both readable and writable in different passes is
// allowed
TEST_F(ResourceUsageTrackingTest, TextureWithReadAndWriteUsageInDifferentPasses) {
    // Test render pass
    {
        // Create textures that will be used as both a sampled texture and a render target
        wgpu::Texture t0 = CreateTexture(wgpu::TextureUsage::TextureBinding |
                                         wgpu::TextureUsage::RenderAttachment);
        wgpu::TextureView v0 = t0.CreateView();
        wgpu::Texture t1 = CreateTexture(wgpu::TextureUsage::TextureBinding |
                                         wgpu::TextureUsage::RenderAttachment);
        wgpu::TextureView v1 = t1.CreateView();

        // Create bind groups to use the texture as sampled
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::TextureSampleType::Float}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, v0}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, v1}});

        // Create render passes that will use the textures as render attachments
        utils::ComboRenderPassDescriptor renderPass0({v1});
        utils::ComboRenderPassDescriptor renderPass1({v0});

        // Use the textures as both sampled and render attachments in different passes
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass0);
        pass0.SetBindGroup(0, bg0);
        pass0.End();

        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass1);
        pass1.SetBindGroup(0, bg1);
        pass1.End();

        encoder.Finish();
    }

    // Test compute pass
    {
        // Create a texture that will be used storage texture
        wgpu::Texture texture =
            CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
        wgpu::TextureView view = texture.CreateView();

        // Create bind groups to use the texture as sampled and writeonly bindings
        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, view}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});

        // Use the textures as both sampled and writeonly storages in different passes
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
        pass0.SetBindGroup(0, readBG);
        pass0.End();

        wgpu::ComputePassEncoder pass1 = encoder.BeginComputePass();
        pass1.SetBindGroup(0, writeBG);
        pass1.End();

        encoder.Finish();
    }

    // Test compute pass and render pass mixed together with resource dependency
    {
        // Create a texture that will be used a storage texture
        wgpu::Texture texture =
            CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
        wgpu::TextureView view = texture.CreateView();

        // Create bind groups to use the texture as sampled and writeonly bindings
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});
        wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, view}});

        // Use the texture as writeonly and sampled storage in compute pass and render
        // pass respectively
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
        pass0.SetBindGroup(0, writeBG);
        pass0.End();

        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass1.SetBindGroup(0, readBG);
        pass1.End();

        encoder.Finish();
    }
}

// Test that it is invalid to use the same texture as both readable and writable in different
// draws in a single render pass. But it is valid in different dispatches in a single compute
// pass.
TEST_F(ResourceUsageTrackingTest, TextureWithReadAndWriteUsageOnDifferentDrawsOrDispatches) {
    // Create a texture that will be used both as a sampled texture and a storage texture
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
    wgpu::TextureView view = texture.CreateView();

    // Test render pass
    {
        // Create bind groups to use the texture as sampled and writeonly storage bindings
        wgpu::BindGroupLayout sampledBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup sampledBG = utils::MakeBindGroup(device, sampledBGL, {{0, view}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});

        // Create a no-op render pipeline.
        wgpu::RenderPipeline rp = CreateNoOpRenderPipeline();

        // It is not allowed to use the same texture as both readable and writable in different
        // draws within the same render pass.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetPipeline(rp);

        pass.SetBindGroup(0, sampledBG);
        pass.Draw(3);

        pass.SetBindGroup(0, writeBG);
        pass.Draw(3);

        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass
    {
        // Create bind groups to use the texture as sampled and writeonly storage bindings
        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, view}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline readCp = CreateNoOpComputePipeline({readBGL});
        wgpu::ComputePipeline writeCp = CreateNoOpComputePipeline({writeBGL});

        // It is valid to use the same texture as both readable and writable in different
        // dispatches within the same compute pass.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        pass.SetPipeline(readCp);
        pass.SetBindGroup(0, readBG);
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(writeCp);
        pass.SetBindGroup(0, writeBG);
        pass.DispatchWorkgroups(1);

        pass.End();
        encoder.Finish();
    }
}

// Test that it is invalid to use the same texture as both readable and writable in a single
// draw or dispatch.
TEST_F(ResourceUsageTrackingTest, TextureWithReadAndWriteUsageInSingleDrawOrDispatch) {
    // Create a texture that will be used both as a sampled texture and a storage texture
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
    wgpu::TextureView view = texture.CreateView();

    // Test render pass
    {
        // Create the bind group to use the texture as sampled and writeonly storage bindings
        wgpu::BindGroupLayout sampledBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup sampledBG = utils::MakeBindGroup(device, sampledBGL, {{0, view}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});

        // Create a no-op render pipeline.
        wgpu::RenderPipeline rp = CreateNoOpRenderPipeline();

        // It is invalid to use the same texture as both readable and writable usages in a
        // single draw
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetPipeline(rp);

        pass.SetBindGroup(0, sampledBG);
        pass.SetBindGroup(1, writeBG);
        pass.Draw(3);

        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass
    {
        // Create the bind group to use the texture as sampled and writeonly storage bindings
        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, view}});
        wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({readBGL, writeBGL});

        // It is invalid to use the same texture as both readable and writable usages in a
        // single dispatch
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);

        pass.SetBindGroup(0, readBG);
        pass.SetBindGroup(1, writeBG);
        pass.DispatchWorkgroups(1);

        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that using a single texture as copy src/dst and writable/readable usage in pass is
// allowed.
TEST_F(ResourceUsageTrackingTest, TextureCopyAndTextureUsageInPass) {
    // Create textures that will be used as both a sampled texture and a render target
    wgpu::Texture texture0 = CreateTexture(wgpu::TextureUsage::CopySrc);
    wgpu::Texture texture1 =
        CreateTexture(wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
                      wgpu::TextureUsage::RenderAttachment);
    wgpu::TextureView view0 = texture0.CreateView();
    wgpu::TextureView view1 = texture1.CreateView();

    wgpu::TexelCopyTextureInfo srcView = utils::CreateTexelCopyTextureInfo(texture0, 0, {0, 0, 0});
    wgpu::TexelCopyTextureInfo dstView = utils::CreateTexelCopyTextureInfo(texture1, 0, {0, 0, 0});
    wgpu::Extent3D copySize = {1, 1, 1};

    // Use the texture as both copy dst and render attachment in render pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcView, &dstView, &copySize);
        utils::ComboRenderPassDescriptor renderPass({view1});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        encoder.Finish();
    }

    // Use the texture as both copy dst and readable usage in compute pass
    {
        // Create the bind group to use the texture as sampled
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view1}});

        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcView, &dstView, &copySize);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(cp);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }
}

// Test that all consecutive SetBindGroup()s take effect even though some bind groups are not
// used because they are overwritten by a consecutive call.
TEST_F(ResourceUsageTrackingTest, TextureWithMultipleSetBindGroupsOnSameIndex) {
    // Test render pass
    {
        // Create textures that will be used as both a sampled texture and a render target
        wgpu::Texture texture0 = CreateTexture(wgpu::TextureUsage::TextureBinding |
                                               wgpu::TextureUsage::RenderAttachment);
        wgpu::TextureView view0 = texture0.CreateView();
        wgpu::Texture texture1 = CreateTexture(wgpu::TextureUsage::TextureBinding |
                                               wgpu::TextureUsage::RenderAttachment);
        wgpu::TextureView view1 = texture1.CreateView();

        // Create the bind group to use the texture as sampled
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::TextureSampleType::Float}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, view0}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, view1}});

        // Create the render pass that will use the texture as an render attachment
        utils::ComboRenderPassDescriptor renderPass({view0});

        // Set bind group on the same index twice. The second one overwrites the first one.
        // No texture is used as both sampled and render attachment in the same pass. But the
        // overwritten texture still take effect during resource tracking.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetBindGroup(0, bg0);
            pass.SetBindGroup(0, bg1);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }

        // Set bind group on the same index twice. The second one overwrites the first one.
        // texture0 is used as both sampled and render attachment in the same pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetBindGroup(0, bg1);
            pass.SetBindGroup(0, bg0);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    // Test compute pass
    {
        // Create a texture that will be used both as storage texture
        wgpu::Texture texture0 =
            CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
        wgpu::TextureView view0 = texture0.CreateView();
        wgpu::Texture texture1 = CreateTexture(wgpu::TextureUsage::TextureBinding);
        wgpu::TextureView view1 = texture1.CreateView();

        // Create the bind group to use the texture as sampled and writeonly bindings
        wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});

        wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});

        wgpu::BindGroup writeBG0 = utils::MakeBindGroup(device, writeBGL, {{0, view0}});
        wgpu::BindGroup readBG0 = utils::MakeBindGroup(device, readBGL, {{0, view0}});
        wgpu::BindGroup readBG1 = utils::MakeBindGroup(device, readBGL, {{0, view1}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({writeBGL, readBGL});

        // Set bind group on the same index twice. The second one overwrites the first one.
        // No texture is used as both sampled and writeonly storage in the same dispatch so
        // there are no errors.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, writeBG0);
            pass.SetBindGroup(1, readBG0);
            pass.SetBindGroup(1, readBG1);
            pass.SetPipeline(cp);
            pass.DispatchWorkgroups(1);
            pass.End();
            encoder.Finish();
        }

        // Set bind group on the same index twice. The second one overwrites the first one.
        // texture0 is used as both writeonly and sampled storage in the same dispatch, which
        // is an error.
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, writeBG0);
            pass.SetBindGroup(1, readBG1);
            pass.SetBindGroup(1, readBG0);
            pass.SetPipeline(cp);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Test that it is invalid to have resource usage conflicts even when all bindings are not
// visible to the programmable pass where it is used.
TEST_F(ResourceUsageTrackingTest, TextureUsageConflictBetweenInvisibleStagesInBindGroup) {
    // Create texture and texture view
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
    wgpu::TextureView view = texture.CreateView();

    // Test render pass for bind group. The conflict of sampled storage and writeonly storage
    // usage doesn't reside in render related stages at all
    {
        // Create a bind group whose bindings are not visible in render pass
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float},
                     {1, wgpu::ShaderStage::None, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}, {1, view}});

        // These two bindings are invisible in render pass. But we still track these bindings.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass for bind group. The conflict of sampled storage and writeonly storage
    // usage doesn't reside in compute related stage at all
    {
        // Create a bind group whose bindings are not visible in compute pass
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float},
                     {1, wgpu::ShaderStage::None, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}, {1, view}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // These two bindings are invisible in compute pass. But we still track these bindings.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bg);
        pass.DispatchWorkgroups(1);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that it is invalid to have resource usage conflicts even when one of the bindings is not
// visible to the programmable pass where it is used.
TEST_F(ResourceUsageTrackingTest, TextureUsageConflictWithInvisibleStageInBindGroup) {
    // Create texture and texture view
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding |
                      wgpu::TextureUsage::RenderAttachment);
    wgpu::TextureView view = texture.CreateView();

    // Test render pass
    {
        // Create the render pass that will use the texture as an render attachment
        utils::ComboRenderPassDescriptor renderPass({view});

        // Create a bind group which use the texture as sampled storage in compute stage
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}});

        // Texture usage in compute stage in bind group conflicts with render target. And
        // binding for compute stage is not visible in render pass. But we still track this
        // binding.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass
    {
        // Create a bind group which contains both fragment and compute stages
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float},
             {1, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}, {1, view}});

        // Create a no-op compute pipeline.
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({bgl});

        // Texture usage in compute stage conflicts with texture usage in fragment stage. And
        // binding for fragment stage is not visible in compute pass. But we still track this
        // invisible binding.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bg);
        pass.DispatchWorkgroups(1);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that it is invalid to have resource usage conflicts even when one of the bindings is not
// used in the pipeline.
TEST_F(ResourceUsageTrackingTest, TextureUsageConflictWithUnusedPipelineBindings) {
    // Create texture and texture view
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);
    wgpu::TextureView view = texture.CreateView();

    // Create bind groups.
    wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute,
                  wgpu::TextureSampleType::Float}});
    wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute,
                  wgpu::StorageTextureAccess::WriteOnly, kFormat}});
    wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, view}});
    wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, view}});

    // Test render pass
    {
        // Create a passthrough render pipeline with a sampled storage texture
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                @group(0) @binding(0) var tex : texture_2d<f32>;
                @fragment fn main() {
                })");
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &readBGL);
        wgpu::RenderPipeline rp = device.CreateRenderPipeline(&pipelineDescriptor);

        // Texture binding in readBG conflicts with texture binding in writeBG. The binding
        // in writeBG is not used in pipeline. But we still track this binding.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetBindGroup(0, readBG);
        pass.SetBindGroup(1, writeBG);
        pass.SetPipeline(rp);
        pass.Draw(3);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test compute pass
    {
        wgpu::ComputePipeline cp = CreateNoOpComputePipeline({readBGL});

        // Texture binding in readBG conflicts with texture binding in writeBG. The binding
        // in writeBG is not used in pipeline's layout so it isn't an error.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, readBG);
        pass.SetBindGroup(1, writeBG);
        pass.SetPipeline(cp);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }
}

// Test that using an indirect buffer is disallowed with a writable usage (like storage) but
// allowed with a readable usage (like readonly storage).
TEST_F(ResourceUsageTrackingTest, IndirectBufferWithReadOrWriteStorage) {
    wgpu::Buffer buffer =
        CreateBuffer(20, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage);

    wgpu::BindGroupLayout readBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
    wgpu::BindGroupLayout writeBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    wgpu::BindGroup readBG = utils::MakeBindGroup(device, readBGL, {{0, buffer}});
    wgpu::BindGroup writeBG = utils::MakeBindGroup(device, writeBGL, {{0, buffer}});

    // Test pipelines
    wgpu::RenderPipeline rp = CreateNoOpRenderPipeline();
    wgpu::ComputePipeline readCp = CreateNoOpComputePipeline({readBGL});
    wgpu::ComputePipeline writeCp = CreateNoOpComputePipeline({writeBGL});

    // Test that indirect + readonly is allowed in the same render pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetPipeline(rp);
        pass.SetBindGroup(0, readBG);
        pass.DrawIndirect(buffer, 0);
        pass.End();
        encoder.Finish();
    }

    // Test that indirect + writable is disallowed in the same render pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&PlaceholderRenderPass);
        pass.SetPipeline(rp);
        pass.SetBindGroup(0, writeBG);
        pass.DrawIndirect(buffer, 0);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Test that indirect + readonly is allowed in the same dispatch
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(readCp);
        pass.SetBindGroup(0, readBG);
        pass.DispatchWorkgroupsIndirect(buffer, 0);
        pass.End();
        encoder.Finish();
    }

    // Test that indirect + writable is disallowed in the same dispatch
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(writeCp);
        pass.SetBindGroup(0, writeBG);
        pass.DispatchWorkgroupsIndirect(buffer, 0);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

}  // anonymous namespace
}  // namespace dawn
