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

#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class MetalBufferRobustnessTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        required.maxBufferSize = supported.maxBufferSize;
        required.maxStorageBufferBindingSize = supported.maxStorageBufferBindingSize;
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::IndirectFirstInstance};
    }

    wgpu::Buffer CreateBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        return device.CreateBuffer(&descriptor);
    }

    enum class BindType { Vertex, Storage };

    void TestBuffer(BindType bindType,
                    uint64_t bufferSize,
                    uint64_t bindingOffset,
                    uint32_t firstVertex,
                    std::array<uint32_t, 4> expected) {
        DAWN_TEST_UNSUPPORTED_IF(deviceLimits.maxBufferSize < bufferSize);

        constexpr uint32_t kNumChecks = expected.size();

        // Create a vertex buffer containing known data. We expect the out-of-bounds access to be
        // clamped so just populate the very end of the buffer with known data.
        wgpu::Buffer testBuffer =
            CreateBuffer(bufferSize, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage |
                                         wgpu::BufferUsage::CopyDst);
        constexpr uint32_t kKnownData = 0xAAAAAAAA;
        queue.WriteBuffer(testBuffer, bufferSize - sizeof(kKnownData), &kKnownData,
                          sizeof(kKnownData));

        // Draw one point to each output pixel, containing the value we got from the vertex buffer.
        wgpu::ShaderModule shader = utils::CreateShaderModule(device, absl::StrFormat(R"(
            // Common code

            const kNumChecks: u32 = %u;

            struct VOut { @builtin(position) pos: vec4f, @location(0) @interpolate(flat) val: u32 }

            fn vsCommon(instanceIndex: u32, val: u32) -> VOut {
                var o: VOut;
                o.pos = vec4f((f32(instanceIndex) + 0.5) / f32(kNumChecks) * 2 - 1, 0, 0, 1);
                o.val = val;
                return o;
            }

            @fragment fn fs(i: VOut) -> @location(0) u32 {
                return i.val;
            }

            // Vertex buffer test

            struct VIn { @location(0) val: u32 }

            @vertex fn vsVertexBufferTest(v: VIn,
                                          @builtin(instance_index) instanceIndex: u32) -> VOut {
                return vsCommon(instanceIndex, v.val);
            }

            // Storage buffer test

            @group(0) @binding(0) var<storage, read> buf: array<u32>;

            @vertex fn vsStorageBufferTest(@builtin(vertex_index) vertexIndex: u32,
                                           @builtin(instance_index) instanceIndex: u32) -> VOut {
                return vsCommon(instanceIndex, buf[vertexIndex]);
            }
        )",
                                                                                      kNumChecks));

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = shader;
        if (bindType == BindType::Vertex) {
            pipelineDesc.vertex.entryPoint = "vsVertexBufferTest";
            pipelineDesc.vertex.bufferCount = 1;
            pipelineDesc.cAttributes[0].format = wgpu::VertexFormat::Uint32;
            pipelineDesc.cAttributes[0].shaderLocation = 0;
            pipelineDesc.cBuffers[0].arrayStride = sizeof(uint32_t);
            pipelineDesc.cBuffers[0].attributes = pipelineDesc.cAttributes.data();
            pipelineDesc.cBuffers[0].attributeCount = 1;
        } else {
            pipelineDesc.vertex.entryPoint = "vsStorageBufferTest";
        }
        pipelineDesc.cFragment.module = shader;
        pipelineDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::TextureDescriptor textureDesc;
        textureDesc.size = {kNumChecks, 1, 1};
        textureDesc.format = wgpu::TextureFormat::R32Uint;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        // Generate indirect draw data.
        struct Draw {
            uint32_t vertexCount, instanceCount, firstVertex, firstInstance;
        };
        std::array<Draw, kNumChecks> indirectData;
        for (uint32_t i = 0; i < kNumChecks; ++i) {
            // One check at each vertex offset. Uses the instance_index to pass the output position.
            indirectData[i] = {1, 1, firstVertex + i, i};
        }
        wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
            device, indirectData.data(), indirectData.size() * sizeof(Draw),
            wgpu::BufferUsage::Indirect);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            utils::ComboRenderPassDescriptor renderPass({texture.CreateView()});
            renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            // Initial value indicating there's a bug in the test and points aren't drawn correctly
            renderPass.cColorAttachments[0].clearValue = {7, 7, 7, 7};
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            {
                pass.SetPipeline(pipeline);
                if (bindType == BindType::Vertex) {
                    pass.SetVertexBuffer(0, testBuffer, bindingOffset);
                } else {
                    wgpu::BindGroup bg =
                        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {
                                                 {0, testBuffer, bindingOffset},
                                             });
                    pass.SetBindGroup(0, bg);
                }
                // Indirect draw avoids the CPU-side validation of the vertex buffer binding size.
                for (uint32_t i = 0; i < kNumChecks; ++i) {
                    pass.DrawIndirect(indirectBuffer, i * sizeof(Draw));
                }
            }
            pass.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check if any of the vertices failed.
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0}, textureDesc.size);

        testBuffer.Destroy();
    }
};

// Regression test for crbug.com/488400770.
// Test that vertex buffer robustness works even with a vertex buffer around 4GB: buffer sizes are
// passed to MSL as u32, so they risk overflowing to 0. If that happens this test should both fail
// and trigger a Metal Shader Validation layer error. (As of this writing, that itself won't fail
// the test, but it will cause the OOB access to return 0.)
TEST_P(MetalBufferRobustnessTest, VertexBuffer_Under4GB) {
    // Implementation adds an extra 4B at the end, in case the buffer is bound with offset=size, so
    // there will be space at the end to clamp into. This should result in a 4GiB MTLBuffer, but the
    // bound size should still be 4GiB-4 which fits in u32. If the buffer size is not passed to MSL
    // correctly, it can overflow to 0, and clamp the the access to the u32[] vertex buffer to
    // 0 - 1 = UINT32_MAX, which allows access to 12GiB of space past the end of the buffer.
    uint32_t bufferSizeInts = 0x4000'0000 - 1;
    uint64_t bufferSize = static_cast<uint64_t>(bufferSizeInts) * sizeof(uint32_t);
    TestBuffer(BindType::Vertex, bufferSize, 0, bufferSizeInts - 1,
               {0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA});
}

// Regression test for crbug.com/488400770.
// If the actual size is 4GiB, then even passing the correct size to MSL for clamping would
// result in the bug. (As of this writing, this test will skip itself.)
TEST_P(MetalBufferRobustnessTest, VertexBuffer_4GB) {
    uint32_t kBufferSizeInts = 0x4000'0000;
    uint64_t kBufferSize = static_cast<uint64_t>(kBufferSizeInts) * sizeof(uint32_t);
    TestBuffer(BindType::Vertex, kBufferSize, 0, kBufferSizeInts - 1,
               {0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA});
}

// If we bind the buffer with offset=size so that there's no space at the end (the binding size is
// 0), we expect to read the padding (which should be 0). Note unfortunately, in this case, we can't
// tell if the Metal shader validation layer caught an OOB, except by looking at stderr manually.
TEST_P(MetalBufferRobustnessTest, VertexBuffer_ZeroSizeRemaining) {
    uint32_t kBufferSizeInts = 0x4000'0000 - 1;
    uint64_t kBufferSize = static_cast<uint64_t>(kBufferSizeInts) * sizeof(uint32_t);
    TestBuffer(BindType::Vertex, kBufferSize, kBufferSize, 0, {0, 0, 0, 0});
}

// Regression test for crbug.com/488400770.
// The same bug also applies to regular storage buffers, not just the ones we generate for
// vertex-pulling. (As of this writing, this test will skip itself.)
TEST_P(MetalBufferRobustnessTest, StorageBuffer) {
    constexpr uint32_t kBufferSizeInts = 0x4000'0000;
    constexpr uint64_t kBufferSize = kBufferSizeInts * sizeof(uint32_t);
    DAWN_TEST_UNSUPPORTED_IF(deviceLimits.maxStorageBufferBindingSize < kBufferSize);
    TestBuffer(BindType::Storage, kBufferSize, 0, kBufferSizeInts - 1,
               {0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA});
}

DAWN_INSTANTIATE_TEST(MetalBufferRobustnessTest, MetalBackend());

}  // anonymous namespace
}  // namespace dawn
