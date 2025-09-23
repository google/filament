// Copyright 2021 The Dawn & Tint Authors
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
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class LabelTest : public ValidationTest {
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

TEST_F(LabelTest, BindGroup) {
    std::string label = "test";
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 0;
    descriptor.entries = nullptr;

    // The label should be empty if one was not set.
    {
        wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(bindGroup.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);
        bindGroup.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(bindGroup.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(bindGroup.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, BindGroupLayout) {
    std::string label = "test";

    wgpu::BindGroupLayoutDescriptor descriptor = {};
    descriptor.entryCount = 0;
    descriptor.entries = nullptr;

    // The label should be empty if one was not set.
    {
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(bindGroupLayout.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&descriptor);
        bindGroupLayout.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(bindGroupLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(bindGroupLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, Buffer) {
    std::string label = "test";
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    // The label should be empty if one was not set.
    {
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, CommandBuffer) {
    std::string label = "test";
    wgpu::CommandBufferDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::CommandBuffer commandBuffer = encoder.Finish(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(commandBuffer.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::CommandBuffer commandBuffer = encoder.Finish(&descriptor);
        commandBuffer.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(commandBuffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::CommandBuffer commandBuffer = encoder.Finish(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(commandBuffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, CommandEncoder) {
    std::string label = "test";
    wgpu::CommandEncoderDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, ComputePassEncoder) {
    std::string label = "test";
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    wgpu::ComputePassDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::ComputePassEncoder encoder = commandEncoder.BeginComputePass(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
        encoder.End();
    }

    // Test setting a label through API
    {
        wgpu::ComputePassEncoder encoder = commandEncoder.BeginComputePass(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.End();
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ComputePassEncoder encoder = commandEncoder.BeginComputePass(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.End();
    }
}

TEST_F(LabelTest, ExternalTexture) {
    std::string label = "test";
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size.width = 1;
    textureDescriptor.size.height = 1;
    textureDescriptor.size.depthOrArrayLayers = 1;
    textureDescriptor.mipLevelCount = 1;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.dimension = wgpu::TextureDimension::e2D;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage =
        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor descriptor;
    descriptor.plane0 = texture.CreateView();
    std::array<float, 12> mPlaceholderConstantArray;
    descriptor.yuvToRgbConversionMatrix = mPlaceholderConstantArray.data();
    descriptor.gamutConversionMatrix = mPlaceholderConstantArray.data();
    descriptor.srcTransferFunctionParameters = mPlaceholderConstantArray.data();
    descriptor.dstTransferFunctionParameters = mPlaceholderConstantArray.data();
    descriptor.cropSize = {1, 1};
    descriptor.apparentSize = {1, 1};

    // The label should be empty if one was not set.
    {
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(externalTexture.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&descriptor);
        externalTexture.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(externalTexture.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(externalTexture.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, PipelineLayout) {
    std::string label = "test";
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = &layout;

    // The label should be empty if one was not set.
    {
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(pipelineLayout.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        pipelineLayout.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(pipelineLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(pipelineLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, QuerySet) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::QuerySetDescriptor descriptor;
    descriptor.type = wgpu::QueryType::Occlusion;
    descriptor.count = 1;

    // The label should be empty if one was not set.
    {
        wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(querySet.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
        querySet.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(querySet.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(querySet.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, Queue) {
    std::string label = "test";

    // The label should be empty if one was not set.
    {
        wgpu::DeviceDescriptor descriptor;
        wgpu::Device labelDevice =
            wgpu::Device::Acquire(GetBackendAdapter().CreateDevice(&descriptor));
        std::string readbackLabel = native::GetObjectLabelForTesting(labelDevice.GetQueue().Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::DeviceDescriptor descriptor;
        wgpu::Device labelDevice =
            wgpu::Device::Acquire(GetBackendAdapter().CreateDevice(&descriptor));
        labelDevice.GetQueue().SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(labelDevice.GetQueue().Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        wgpu::DeviceDescriptor descriptor;
        descriptor.defaultQueue.label = label.c_str();
        wgpu::Device labelDevice =
            wgpu::Device::Acquire(GetBackendAdapter().CreateDevice(&descriptor));
        std::string readbackLabel = native::GetObjectLabelForTesting(labelDevice.GetQueue().Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, RenderBundleEncoder) {
    std::string label = "test";

    utils::ComboRenderBundleEncoderDescriptor descriptor = {};
    descriptor.colorFormatCount = 1;
    descriptor.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;

    // The label should be empty if one was not set.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, RenderPassEncoder) {
    std::string label = "test";
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    utils::ComboRenderPassDescriptor descriptor({texture.CreateView()});

    // The label should be empty if one was not set.
    {
        wgpu::RenderPassEncoder encoder = commandEncoder.BeginRenderPass(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
        encoder.End();
    }

    // Test setting a label through API
    {
        wgpu::RenderPassEncoder encoder = commandEncoder.BeginRenderPass(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.End();
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::RenderPassEncoder encoder = commandEncoder.BeginRenderPass(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.End();
    }
}

TEST_F(LabelTest, Sampler) {
    std::string label = "test";
    wgpu::SamplerDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::Sampler sampler = device.CreateSampler(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(sampler.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Sampler sampler = device.CreateSampler(&descriptor);
        sampler.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(sampler.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Sampler sampler = device.CreateSampler(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(sampler.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, Texture) {
    std::string label = "test";
    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.sampleCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    // The label should be empty if one was not set.
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(texture.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        texture.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(texture.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(texture.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, TextureView) {
    std::string label = "test";
    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.sampleCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    wgpu::Texture texture = device.CreateTexture(&descriptor);

    // The label should be generated if no one was not set.
    {
        wgpu::TextureView textureView = texture.CreateView();
        std::string readbackLabel = native::GetObjectLabelForTesting(textureView.Get());
        ASSERT_EQ(readbackLabel.rfind("defaulted from [Texture (unlabeled", 0), 0U);
    }

    // Test setting a label through API
    {
        wgpu::TextureView textureView = texture.CreateView();
        textureView.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(textureView.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        wgpu::TextureViewDescriptor viewDescriptor;
        viewDescriptor.label = label.c_str();
        wgpu::TextureView textureView = texture.CreateView(&viewDescriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(textureView.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, RenderPipeline) {
    std::string label = "test";

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    // The label should be empty if one was not set.
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        pipeline.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, ComputePipeline) {
    std::string label = "test";

    wgpu::ShaderModule computeModule = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, nullptr);
    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = pl;
    descriptor.compute.module = computeModule;

    // The label should be empty if one was not set.
    {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
        pipeline.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, ShaderModule) {
    std::string label = "test";

    const char* source = R"(
    @compute @workgroup_size(1) fn main() {
    })";

    wgpu::ShaderSourceWGSL wgslDesc;
    wgslDesc.code = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;

    // The label should be empty if one was not set.
    {
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(shaderModule.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
        shaderModule.SetLabel(label.c_str());
        std::string readbackLabel = native::GetObjectLabelForTesting(shaderModule.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
        std::string readbackLabel = native::GetObjectLabelForTesting(shaderModule.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

}  // anonymous namespace
}  // namespace dawn
