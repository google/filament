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

#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static uint32_t kRTSize = 8;

class BindGroupTests : public DawnTest {
  protected:
    wgpu::RequiredLimits GetRequiredLimits(const wgpu::SupportedLimits& supported) override {
        // TODO(crbug.com/383593270): Enable all the limits.
        wgpu::RequiredLimits required = {};
        required.limits.maxStorageBuffersInVertexStage =
            supported.limits.maxStorageBuffersInVertexStage;
        required.limits.maxStorageBuffersInFragmentStage =
            supported.limits.maxStorageBuffersInFragmentStage;
        required.limits.maxStorageBuffersPerShaderStage =
            supported.limits.maxStorageBuffersPerShaderStage;
        required.limits.maxStorageTexturesInFragmentStage =
            supported.limits.maxStorageTexturesInFragmentStage;
        required.limits.maxStorageTexturesPerShaderStage =
            supported.limits.maxStorageTexturesPerShaderStage;
        return required;
    }

    void SetUp() override {
        DawnTest::SetUp();
        mMinUniformBufferOffsetAlignment =
            GetSupportedLimits().limits.minUniformBufferOffsetAlignment;
    }
    wgpu::CommandBuffer CreateSimpleComputeCommandBuffer(const wgpu::ComputePipeline& pipeline,
                                                         const wgpu::BindGroup& bindGroup) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        return encoder.Finish();
    }

    wgpu::PipelineLayout MakeBasicPipelineLayout(
        std::vector<wgpu::BindGroupLayout> bindingInitializer) const {
        wgpu::PipelineLayoutDescriptor descriptor;

        descriptor.bindGroupLayoutCount = bindingInitializer.size();
        descriptor.bindGroupLayouts = bindingInitializer.data();

        return device.CreatePipelineLayout(&descriptor);
    }

    wgpu::ShaderModule MakeSimpleVSModule() const {
        return utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
             var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( 1.0, 1.0),
                vec2f(-1.0, -1.0));

            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })");
    }

    wgpu::ShaderModule MakeFSModule(std::vector<wgpu::BufferBindingType> bindingTypes) const {
        DAWN_ASSERT(bindingTypes.size() <= kMaxBindGroups);

        std::ostringstream fs;
        for (size_t i = 0; i < bindingTypes.size(); ++i) {
            fs << "struct Buffer" << i << R"( {
                color : vec4f
            })";

            switch (bindingTypes[i]) {
                case wgpu::BufferBindingType::Uniform:
                    fs << "\n@group(" << i << ") @binding(0) var<uniform> buffer" << i
                       << " : Buffer" << i << ";";
                    break;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    fs << "\n@group(" << i << ") @binding(0) var<storage, read> buffer" << i
                       << " : Buffer" << i << ";";
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
        }

        fs << "\n@fragment fn main() -> @location(0) vec4f{\n";
        fs << "var fragColor : vec4f = vec4f();\n";
        for (size_t i = 0; i < bindingTypes.size(); ++i) {
            fs << "fragColor = fragColor + buffer" << i << ".color;\n";
        }
        fs << "return fragColor;\n";
        fs << "}\n";
        return utils::CreateShaderModule(device, fs.str().c_str());
    }

    wgpu::RenderPipeline MakeTestPipeline(const utils::BasicRenderPass& renderPass,
                                          std::vector<wgpu::BufferBindingType> bindingTypes,
                                          std::vector<wgpu::BindGroupLayout> bindGroupLayouts) {
        wgpu::ShaderModule vsModule = MakeSimpleVSModule();
        wgpu::ShaderModule fsModule = MakeFSModule(bindingTypes);

        wgpu::PipelineLayout pipelineLayout = MakeBasicPipelineLayout(bindGroupLayouts);

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.layout = pipelineLayout;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

        wgpu::BlendState blend;
        blend.color.operation = wgpu::BlendOperation::Add;
        blend.color.srcFactor = wgpu::BlendFactor::One;
        blend.color.dstFactor = wgpu::BlendFactor::One;
        blend.alpha.operation = wgpu::BlendOperation::Add;
        blend.alpha.srcFactor = wgpu::BlendFactor::One;
        blend.alpha.dstFactor = wgpu::BlendFactor::One;

        pipelineDescriptor.cTargets[0].blend = &blend;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    uint32_t mMinUniformBufferOffsetAlignment;
};

// Test a bindgroup reused in two command buffers in the same call to queue.Submit().
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, ReusedBindGroupSingleSubmit) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Contents {
            f : f32
        }
        @group(0) @binding(0) var <uniform> contents: Contents;

        @compute @workgroup_size(1) fn main() {
          var f : f32 = contents.f;
        })");

    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.compute.module = module;
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer cb[2];
    cb[0] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    cb[1] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    queue.Submit(2, cb);
}

// Test a bindgroup containing a UBO which is used in both the vertex and fragment shader.
// It contains a transformation matrix for the VS and the fragment color for the FS.
// These must result in different register offsets in the native APIs.
TEST_P(BindGroupTests, ReusedUBO) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct VertexUniformBuffer {
            transform : mat2x2f
        }

        @group(0) @binding(0) var <uniform> vertexUbo : VertexUniformBuffer;

        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( 1.0, 1.0),
                vec2f(-1.0, -1.0));

            return vec4f(vertexUbo.transform * pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct FragmentUniformBuffer {
            color : vec4f
        }
        @group(0) @binding(1) var <uniform> fragmentUbo : FragmentUniformBuffer;

        @fragment fn main() -> @location(0) vec4f {
            return fragmentUbo.color;
        })");

    utils::ComboRenderPipelineDescriptor textureDescriptor;
    textureDescriptor.vertex.module = vsModule;
    textureDescriptor.cFragment.module = fsModule;
    textureDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&textureDescriptor);

    struct Data {
        float transform[8];
        char padding[256 - 8 * sizeof(float)];
        float color[4];
    };
    DAWN_ASSERT(offsetof(Data, color) == 256);
    Data data{
        {1.f, 0.f, 0.f, 1.0f},
        {0},
        {0.f, 1.f, 0.f, 1.f},
    };
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, &data, sizeof(data), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(
        device, pipeline.GetBindGroupLayout(0),
        {{0, buffer, 0, sizeof(Data::transform)}, {1, buffer, 256, sizeof(Data::color)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test a bindgroup containing a UBO in the vertex shader and a sampler and texture in the fragment
// shader. In D3D12 for example, these different types of bindings end up in different namespaces,
// but the register offsets used must match between the shader module and descriptor range.
TEST_P(BindGroupTests, UBOSamplerAndTexture) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var <uniform> transform : mat2x2f;

        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( 1.0, 1.0),
                vec2f(-1.0, -1.0));

            return vec4f(transform * pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(1) var samp : sampler;
        @group(0) @binding(2) var tex : texture_2d<f32>;

        @fragment
        fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
            return textureSample(tex, samp, FragCoord.xy);
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    constexpr float transform[] = {1.f, 0.f, 0.f, 1.f};
    wgpu::Buffer buffer = utils::CreateBufferFromData(device, &transform, sizeof(transform),
                                                      wgpu::BufferUsage::Uniform);

    wgpu::SamplerDescriptor samplerDescriptor = {};
    samplerDescriptor.minFilter = wgpu::FilterMode::Nearest;
    samplerDescriptor.magFilter = wgpu::FilterMode::Nearest;
    samplerDescriptor.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
    samplerDescriptor.addressModeU = wgpu::AddressMode::ClampToEdge;
    samplerDescriptor.addressModeV = wgpu::AddressMode::ClampToEdge;
    samplerDescriptor.addressModeW = wgpu::AddressMode::ClampToEdge;

    wgpu::Sampler sampler = device.CreateSampler(&samplerDescriptor);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kRTSize;
    descriptor.size.height = kRTSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    uint32_t width = kRTSize, height = kRTSize;
    uint32_t widthInBytes = width * sizeof(utils::RGBA8);
    widthInBytes = (widthInBytes + 255) & ~255;
    uint32_t sizeInBytes = widthInBytes * height;
    uint32_t size = sizeInBytes / sizeof(utils::RGBA8);
    std::vector<utils::RGBA8> data = std::vector<utils::RGBA8>(size);
    for (uint32_t i = 0; i < size; i++) {
        data[i] = utils::RGBA8(0, 255, 0, 255);
    }
    wgpu::Buffer stagingBuffer =
        utils::CreateBufferFromData(device, data.data(), sizeInBytes, wgpu::BufferUsage::CopySrc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, buffer, 0, sizeof(transform)}, {1, sampler}, {2, textureView}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(stagingBuffer, 0, widthInBytes);
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    wgpu::Extent3D copySize = {width, height, 1};
    encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

TEST_P(BindGroupTests, MultipleBindLayouts) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
         struct VertexUniformBuffer {
             transform : mat2x2f
         }

        @group(0) @binding(0) var <uniform> vertexUbo1 : VertexUniformBuffer;
        @group(1) @binding(0) var <uniform> vertexUbo2 : VertexUniformBuffer;

        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( 1.0, 1.0),
                vec2f(-1.0, -1.0));

            return vec4f(
                (vertexUbo1.transform + vertexUbo2.transform) * pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct FragmentUniformBuffer {
            color : vec4f
        }

        @group(0) @binding(1) var <uniform> fragmentUbo1 : FragmentUniformBuffer;
        @group(1) @binding(1) var <uniform> fragmentUbo2 : FragmentUniformBuffer;

        @fragment fn main() -> @location(0) vec4f {
            return fragmentUbo1.color + fragmentUbo2.color;
        })");

    utils::ComboRenderPipelineDescriptor textureDescriptor;
    textureDescriptor.vertex.module = vsModule;
    textureDescriptor.cFragment.module = fsModule;
    textureDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&textureDescriptor);

    struct Data {
        float transform[4];
        char padding[256 - 4 * sizeof(float)];
        float color[4];
    };
    DAWN_ASSERT(offsetof(Data, color) == 256);

    std::vector<Data> data;
    std::vector<wgpu::Buffer> buffers;
    std::vector<wgpu::BindGroup> bindGroups;

    data.push_back({{1.0f, 0.0f, 0.0f, 0.0f}, {0}, {0.0f, 1.0f, 0.0f, 1.0f}});

    data.push_back({{0.0f, 0.0f, 0.0f, 1.0f}, {0}, {1.0f, 0.0f, 0.0f, 1.0f}});

    for (int i = 0; i < 2; i++) {
        wgpu::Buffer buffer =
            utils::CreateBufferFromData(device, &data[i], sizeof(Data), wgpu::BufferUsage::Uniform);
        buffers.push_back(buffer);
        bindGroups.push_back(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, buffers[i], 0, sizeof(Data::transform)},
                                                   {1, buffers[i], 256, sizeof(Data::color)}}));
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroups[0]);
    pass.SetBindGroup(1, bindGroups[1]);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(255, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// This is a regression test for crbug.com/dawn/1170 that tests a module that contains multiple
// entry points, using non-zero binding groups. This has the potential to cause problems when we
// only remap bindings for one entry point, as the remaining unmapped binding numbers may be invalid
// for certain backends.
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, MultipleEntryPointsWithMultipleNonZeroGroups) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Contents {
            f : f32
        }
        @group(0) @binding(0) var <uniform> contents0: Contents;
        @group(1) @binding(0) var <uniform> contents1: Contents;
        @group(2) @binding(0) var <uniform> contents2: Contents;

        @compute @workgroup_size(1) fn main0() {
          var a : f32 = contents0.f;
        }

        @compute @workgroup_size(1) fn main1() {
          var a : f32 = contents1.f;
          var b : f32 = contents2.f;
        }

        @compute @workgroup_size(1) fn main2() {
          var a : f32 = contents0.f;
          var b : f32 = contents1.f;
          var c : f32 = contents2.f;
        })");

    // main0: bind (0,0)
    {
        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.compute.module = module;
        cpDesc.compute.entryPoint = "main0";
        wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        wgpu::Buffer buffer0 = device.CreateBuffer(&bufferDesc);
        wgpu::BindGroup bindGroup0 =
            utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {{0, buffer0}});

        wgpu::CommandBuffer cb;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bindGroup0);
        pass.DispatchWorkgroups(1);
        pass.End();
        cb = encoder.Finish();
        queue.Submit(1, &cb);
    }

    // main1: bind (1,0) and (2,0)
    {
        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.compute.module = module;
        cpDesc.compute.entryPoint = "main1";
        wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        wgpu::Buffer buffer1 = device.CreateBuffer(&bufferDesc);
        wgpu::Buffer buffer2 = device.CreateBuffer(&bufferDesc);
        wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {});
        wgpu::BindGroup bindGroup1 =
            utils::MakeBindGroup(device, cp.GetBindGroupLayout(1), {{0, buffer1}});
        wgpu::BindGroup bindGroup2 =
            utils::MakeBindGroup(device, cp.GetBindGroupLayout(2), {{0, buffer2}});

        wgpu::CommandBuffer cb;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bindGroup0);
        pass.SetBindGroup(1, bindGroup1);
        pass.SetBindGroup(2, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();
        cb = encoder.Finish();
        queue.Submit(1, &cb);
    }

    // main2: bind (0,0), (1,0), and (2,0)
    {
        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.compute.module = module;
        cpDesc.compute.entryPoint = "main2";
        wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        wgpu::Buffer buffer0 = device.CreateBuffer(&bufferDesc);
        wgpu::Buffer buffer1 = device.CreateBuffer(&bufferDesc);
        wgpu::Buffer buffer2 = device.CreateBuffer(&bufferDesc);
        wgpu::BindGroup bindGroup0 =
            utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {{0, buffer0}});
        wgpu::BindGroup bindGroup1 =
            utils::MakeBindGroup(device, cp.GetBindGroupLayout(1), {{0, buffer1}});
        wgpu::BindGroup bindGroup2 =
            utils::MakeBindGroup(device, cp.GetBindGroupLayout(2), {{0, buffer2}});

        wgpu::CommandBuffer cb;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cp);
        pass.SetBindGroup(0, bindGroup0);
        pass.SetBindGroup(1, bindGroup1);
        pass.SetBindGroup(2, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();
        cb = encoder.Finish();
        queue.Submit(1, &cb);
    }
}

// This test reproduces an out-of-bound bug on D3D12 backends when calling draw command twice with
// one pipeline that has 4 bind group sets in one render pass.
TEST_P(BindGroupTests, DrawTwiceInSamePipelineWithFourBindGroupSets) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(renderPass,
                         {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform,
                          wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
                         {layout, layout, layout, layout});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    pass.SetPipeline(pipeline);

    // The color will be added 8 times, so the value should be 0.125. But we choose 0.126
    // because of precision issues on some devices (for example NVIDIA bots).
    std::array<float, 4> color = {0.126, 0, 0, 0.126};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    pass.SetBindGroup(0, bindGroup);
    pass.SetBindGroup(1, bindGroup);
    pass.SetBindGroup(2, bindGroup);
    pass.SetBindGroup(3, bindGroup);
    pass.Draw(3);

    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(255, 0, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that bind groups can be set before the pipeline.
TEST_P(BindGroupTests, SetBindGroupBeforePipeline) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single uniform buffer.
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    // Create a pipeline that uses the uniform bind group layout.
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(renderPass, {wgpu::BufferBindingType::Uniform}, {layout});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Create a bind group with a uniform buffer and fill it with RGBAunorm(1, 0, 0, 1).
    std::array<float, 4> color = {1, 0, 0, 1};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    // Set the bind group, then the pipeline, and draw.
    pass.SetBindGroup(0, bindGroup);
    pass.SetPipeline(pipeline);
    pass.Draw(3);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The result should be red.
    utils::RGBA8 filled(255, 0, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that dynamic bind groups can be set before the pipeline.
TEST_P(BindGroupTests, SetDynamicBindGroupBeforePipeline) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a pipeline that uses the dynamic uniform bind group layout for two bind groups.
    wgpu::RenderPipeline pipeline = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
        {layout, layout});

    // Prepare data RGBAunorm(1, 0, 0, 0.5) and RGBAunorm(0, 1, 0, 0.5). They will be added in the
    // shader.
    std::array<float, 4> color0 = {1, 0, 0, 0.501};
    std::array<float, 4> color1 = {0, 1, 0, 0.501};

    size_t color1Offset = Align(sizeof(color0), mMinUniformBufferOffsetAlignment);

    std::vector<uint8_t> data(color1Offset + sizeof(color1));
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));

    // Create a bind group and uniform buffer with the color data. It will be bound at the offset
    // to each color.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the first dynamic bind group.
    uint32_t dynamicOffset = 0;
    pass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);

    // Set the second dynamic bind group.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, bindGroup, 1, &dynamicOffset);

    // Set the pipeline and draw.
    pass.SetPipeline(pipeline);
    pass.Draw(3);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The result should be RGBAunorm(1, 0, 0, 0.5) + RGBAunorm(0, 1, 0, 0.5)
    utils::RGBA8 filled(255, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that the same renderpass can use 3 more pipelines
TEST_P(BindGroupTests, ThreePipelinesInSameRenderpass) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Pipeline0 uses 2 bind groups, 'bindGroup00' and 'bindGroup01', respectively accommodated with
    // 'color00' and 'color01'.
    wgpu::RenderPipeline pipeline0 = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
        {uniformLayout, uniformLayout});

    // Pipeline1 uses 'bindGroup1', accommodated with 'color1'.
    wgpu::RenderPipeline pipeline1 =
        MakeTestPipeline(renderPass, {wgpu::BufferBindingType::Uniform}, {uniformLayout});

    // Pipeline2 uses 'bindGroup2', accommodated with 'color2'.
    wgpu::RenderPipeline pipeline2 =
        MakeTestPipeline(renderPass, {wgpu::BufferBindingType::Uniform}, {uniformLayout});

    // Sum up to (1, 1, 1, 1)
    std::array<float, 4> color00 = {1, 0, 0, 0.5};
    std::array<float, 4> color01 = {0, 1, 0, 0.5};
    std::array<float, 4> color1 = {0, 0, 0.2, 0};
    std::array<float, 4> color2 = {0, 0, 0.8, 0};

    std::vector<uint8_t> data(sizeof(color00));
    memcpy(data.data(), color00.data(), sizeof(color00));
    wgpu::Buffer uniformBuffer00 =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup00 =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer00, 0, 4 * sizeof(float)}});

    memcpy(data.data(), color01.data(), sizeof(color01));
    wgpu::Buffer uniformBuffer01 =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup01 =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer01, 0, 4 * sizeof(float)}});

    memcpy(data.data(), color1.data(), sizeof(color1));
    wgpu::Buffer uniformBuffer1 =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer1, 0, 4 * sizeof(float)}});

    memcpy(data.data(), color2.data(), sizeof(color2));
    wgpu::Buffer uniformBuffer2 =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup2 =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer2, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    uint32_t dynamicOffset = 0;

    pass.SetPipeline(pipeline0);
    pass.SetBindGroup(0, bindGroup00, 1, &dynamicOffset);
    pass.SetBindGroup(1, bindGroup01, 1, &dynamicOffset);
    pass.Draw(3);

    pass.SetBindGroup(0, bindGroup1, 1, &dynamicOffset);
    pass.SetPipeline(pipeline1);
    pass.Draw(3);

    pass.SetBindGroup(0, bindGroup2, 1, &dynamicOffset);
    pass.SetPipeline(pipeline2);
    pass.Draw(3);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(255, 255, 255, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that bind groups set for one pipeline are still set when the pipeline changes.
TEST_P(BindGroupTests, BindGroupsPersistAfterPipelineChange) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().limits.maxStorageBuffersInFragmentStage < 1);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a bind group layout which uses a single dynamic storage buffer.
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage, true}});

    // Create a pipeline which uses the uniform buffer and storage buffer bind groups.
    wgpu::RenderPipeline pipeline0 = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::ReadOnlyStorage},
        {uniformLayout, storageLayout});

    // Create a pipeline which uses the uniform buffer bind group twice.
    wgpu::RenderPipeline pipeline1 = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
        {uniformLayout, uniformLayout});

    // Prepare data RGBAunorm(1, 0, 0, 0.5) and RGBAunorm(0, 1, 0, 0.5). They will be added in the
    // shader.
    std::array<float, 4> color0 = {1, 0, 0, 0.5};
    std::array<float, 4> color1 = {0, 1, 0, 0.5};

    size_t color1Offset = Align(sizeof(color0), mMinUniformBufferOffsetAlignment);

    std::vector<uint8_t> data(color1Offset + sizeof(color1));
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));

    // Create a bind group and uniform buffer with the color data. It will be bound at the offset
    // to each color.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the first pipeline (uniform, storage).
    pass.SetPipeline(pipeline0);

    // Set the first bind group at a dynamic offset.
    // This bind group matches the slot in the pipeline layout.
    uint32_t dynamicOffset = 0;
    pass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);

    // Set the second bind group at a dynamic offset.
    // This bind group does not match the slot in the pipeline layout.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, bindGroup, 1, &dynamicOffset);

    // Set the second pipeline (uniform, uniform).
    // Both bind groups match the pipeline.
    // They should persist and not need to be bound again.
    pass.SetPipeline(pipeline1);
    pass.Draw(3);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The result should be RGBAunorm(1, 0, 0, 0.5) + RGBAunorm(0, 1, 0, 0.5)
    utils::RGBA8 filled(255, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Do a successful draw. Then, change the pipeline and one bind group.
// Draw to check that the all bind groups are set.
TEST_P(BindGroupTests, DrawThenChangePipelineAndBindGroup) {
    // TODO(anglebug.com/3032): fix failure in ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().limits.maxStorageBuffersInFragmentStage < 1);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a bind group layout which uses a single dynamic storage buffer.
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage, true}});

    // Create a pipeline with pipeline layout (uniform, uniform, storage).
    wgpu::RenderPipeline pipeline0 =
        MakeTestPipeline(renderPass,
                         {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform,
                          wgpu::BufferBindingType::ReadOnlyStorage},
                         {uniformLayout, uniformLayout, storageLayout});

    // Create a pipeline with pipeline layout (uniform, storage, storage).
    wgpu::RenderPipeline pipeline1 = MakeTestPipeline(
        renderPass,
        {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::ReadOnlyStorage,
         wgpu::BufferBindingType::ReadOnlyStorage},
        {uniformLayout, storageLayout, storageLayout});

    // Prepare color data.
    // The first draw will use { color0, color1, color2 }.
    // The second draw will use { color0, color3, color2 }.
    // The pipeline uses additive color and alpha blending so the result of two draws should be
    // { 2 * color0 + color1 + 2 * color2 + color3} = RGBAunorm(1, 1, 1, 1)
    std::array<float, 4> color0 = {0.501, 0, 0, 0};
    std::array<float, 4> color1 = {0, 1, 0, 0};
    std::array<float, 4> color2 = {0, 0, 0, 0.501};
    std::array<float, 4> color3 = {0, 0, 1, 0};

    size_t color1Offset = Align(sizeof(color0), mMinUniformBufferOffsetAlignment);
    size_t color2Offset = Align(color1Offset + sizeof(color1), mMinUniformBufferOffsetAlignment);
    size_t color3Offset = Align(color2Offset + sizeof(color2), mMinUniformBufferOffsetAlignment);

    std::vector<uint8_t> data(color3Offset + sizeof(color3), 0);
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));
    memcpy(data.data() + color2Offset, color2.data(), sizeof(color2));
    memcpy(data.data() + color3Offset, color3.data(), sizeof(color3));

    // Create a uniform and storage buffer bind groups to bind the color data.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);

    wgpu::Buffer storageBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Storage);

    wgpu::BindGroup uniformBindGroup =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});
    wgpu::BindGroup storageBindGroup =
        utils::MakeBindGroup(device, storageLayout, {{0, storageBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the pipeline to (uniform, uniform, storage)
    pass.SetPipeline(pipeline0);

    // Set the first bind group to color0 in the dynamic uniform buffer.
    uint32_t dynamicOffset = 0;
    pass.SetBindGroup(0, uniformBindGroup, 1, &dynamicOffset);

    // Set the first bind group to color1 in the dynamic uniform buffer.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, uniformBindGroup, 1, &dynamicOffset);

    // Set the first bind group to color2 in the dynamic storage buffer.
    dynamicOffset = color2Offset;
    pass.SetBindGroup(2, storageBindGroup, 1, &dynamicOffset);

    pass.Draw(3);

    // Set the pipeline to (uniform, storage, storage)
    //  - The first bind group should persist (inherited on some backends)
    //  - The second bind group needs to be set again to pass validation.
    //    It changed from uniform to storage.
    //  - The third bind group should persist. It should be set again by the backend internally.
    pass.SetPipeline(pipeline1);

    // Set the second bind group to color3 in the dynamic storage buffer.
    dynamicOffset = color3Offset;
    pass.SetBindGroup(1, storageBindGroup, 1, &dynamicOffset);

    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(255, 255, 255, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test for crbug.com/dawn/1049, where setting a pipeline without drawing can prevent
// bind groups from being applied later
TEST_P(BindGroupTests, DrawThenChangePipelineTwiceAndBindGroup) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a pipeline with pipeline layout (uniform, uniform, uniform).
    wgpu::RenderPipeline pipeline0 =
        MakeTestPipeline(renderPass,
                         {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform,
                          wgpu::BufferBindingType::Uniform},
                         {uniformLayout, uniformLayout, uniformLayout});

    // Create a pipeline with pipeline layout (uniform).
    wgpu::RenderPipeline pipeline1 = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
        {uniformLayout, uniformLayout});

    // Prepare color data.
    // The first draw will use { color0, color1, color2 }.
    // The second draw will use { color0, color1, color3 }.
    // The pipeline uses additive color and alpha so the result of two draws should be
    // { 2 * color0 + 2 * color1 + color2 + color3} = RGBAunorm(1, 1, 1, 1)
    std::array<float, 4> color0 = {0.501, 0, 0, 0};
    std::array<float, 4> color1 = {0, 0.501, 0, 0};
    std::array<float, 4> color2 = {0, 0, 1, 0};
    std::array<float, 4> color3 = {0, 0, 0, 1};

    size_t color0Offset = 0;
    size_t color1Offset = Align(color0Offset + sizeof(color0), mMinUniformBufferOffsetAlignment);
    size_t color2Offset = Align(color1Offset + sizeof(color1), mMinUniformBufferOffsetAlignment);
    size_t color3Offset = Align(color2Offset + sizeof(color2), mMinUniformBufferOffsetAlignment);

    std::vector<uint8_t> data(color3Offset + sizeof(color3), 0);
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));
    memcpy(data.data() + color2Offset, color2.data(), sizeof(color2));
    memcpy(data.data() + color3Offset, color3.data(), sizeof(color3));

    // Create a uniform and storage buffer bind groups to bind the color data.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);

    wgpu::BindGroup uniformBindGroup =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the pipeline to (uniform, uniform, uniform)
    pass.SetPipeline(pipeline0);

    // Set the first bind group to color0 in the dynamic uniform buffer.
    uint32_t dynamicOffset = color0Offset;
    pass.SetBindGroup(0, uniformBindGroup, 1, &dynamicOffset);

    // Set the first bind group to color1 in the dynamic uniform buffer.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, uniformBindGroup, 1, &dynamicOffset);

    // Set the first bind group to color2 in the dynamic uniform buffer.
    dynamicOffset = color2Offset;
    pass.SetBindGroup(2, uniformBindGroup, 1, &dynamicOffset);

    // This draw will internally apply bind groups for pipeline 0.
    pass.Draw(3);

    // When we set pipeline 1, which has no bind group at index 2 in its layout, it
    // should not prevent bind group 2 from being used after reverting to pipeline 0.
    // More specifically, internally the pipeline 1 layout should not be saved,
    // because we never applied the bind groups via a Draw or Dispatch.
    pass.SetPipeline(pipeline1);

    // Set the second bind group to color3 in the dynamic uniform buffer.
    dynamicOffset = color3Offset;
    pass.SetBindGroup(2, uniformBindGroup, 1, &dynamicOffset);

    // Revert to pipeline 0
    pass.SetPipeline(pipeline0);

    // Internally this should re-apply bind group 2. Because we already
    // drew with this pipeline, and setting pipeline 1 did not dirty the bind groups,
    // bind groups 0 and 1 should still be valid.
    pass.Draw(3);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 filled(255, 255, 255, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Regression test for crbug.com/dawn/408 where dynamic offsets were applied in the wrong order.
// Dynamic offsets should be applied in increasing order of binding number.
TEST_P(BindGroupTests, DynamicOffsetOrder) {
    // We will put the following values and the respective offsets into a buffer.
    // The test will ensure that the correct dynamic offset is applied to each buffer by reading the
    // value from an offset binding.
    std::array<uint32_t, 3> offsets = {3 * mMinUniformBufferOffsetAlignment,
                                       1 * mMinUniformBufferOffsetAlignment,
                                       2 * mMinUniformBufferOffsetAlignment};
    std::array<uint32_t, 3> values = {21, 67, 32};

    // Create three buffers large enough to by offset by the largest offset.
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 3 * mMinUniformBufferOffsetAlignment + sizeof(uint32_t);
    bufferDescriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer0 = device.CreateBuffer(&bufferDescriptor);
    wgpu::Buffer buffer3 = device.CreateBuffer(&bufferDescriptor);

    // This test uses both storage and uniform buffers to ensure buffer bindings are sorted first by
    // binding number before type.
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer2 = device.CreateBuffer(&bufferDescriptor);

    // Populate the values
    queue.WriteBuffer(buffer0, offsets[0], &values[0], sizeof(uint32_t));
    queue.WriteBuffer(buffer2, offsets[1], &values[1], sizeof(uint32_t));
    queue.WriteBuffer(buffer3, offsets[2], &values[2], sizeof(uint32_t));

    wgpu::Buffer outputBuffer = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage, {0, 0, 0});

    // Create the bind group and bind group layout.
    // Note: The order of the binding numbers are intentionally different and not in increasing
    // order.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {
                    {3, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                    {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, true},
                    {4, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                });
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl,
                                                     {
                                                         {0, buffer0, 0, sizeof(uint32_t)},
                                                         {3, buffer3, 0, sizeof(uint32_t)},
                                                         {2, buffer2, 0, sizeof(uint32_t)},
                                                         {4, outputBuffer, 0, 3 * sizeof(uint32_t)},
                                                     });

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
        struct Buffer {
            value : u32
        }

        @group(0) @binding(2) var<uniform> buffer2 : Buffer;
        @group(0) @binding(3) var<storage, read> buffer3 : Buffer;
        @group(0) @binding(0) var<storage, read> buffer0 : Buffer;
        @group(0) @binding(4) var<storage, read_write> outputBuffer : vec3u;

        @compute @workgroup_size(1) fn main() {
            outputBuffer = vec3u(buffer0.value, buffer2.value, buffer3.value);
        })");
    pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(values.data(), outputBuffer, 0, values.size());
}

// Test that ensures that backends do not remap bindings such that dynamic and non-dynamic bindings
// conflict. This can happen if the backend treats dynamic bindings separately from non-dynamic
// bindings.
TEST_P(BindGroupTests, DynamicAndNonDynamicBindingsDoNotConflictAfterRemapping) {
    auto RunTestWith = [&](bool dynamicBufferFirst) {
        uint32_t dynamicBufferBindingNumber = dynamicBufferFirst ? 0 : 1;
        uint32_t bufferBindingNumber = dynamicBufferFirst ? 1 : 0;

        std::array<uint32_t, 1> offsets{mMinUniformBufferOffsetAlignment};
        std::array<uint32_t, 2> values = {21, 67};

        // Create three buffers large enough to by offset by the largest offset.
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = 2 * mMinUniformBufferOffsetAlignment + sizeof(uint32_t);
        bufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;

        wgpu::Buffer dynamicBuffer = device.CreateBuffer(&bufferDescriptor);
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

        // Populate the values
        queue.WriteBuffer(dynamicBuffer, mMinUniformBufferOffsetAlignment,
                          &values[dynamicBufferBindingNumber], sizeof(uint32_t));
        queue.WriteBuffer(buffer, 0, &values[bufferBindingNumber], sizeof(uint32_t));

        wgpu::Buffer outputBuffer = utils::CreateBufferFromData(
            device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage, {0, 0});

        // Create a bind group layout which uses a single dynamic uniform buffer.
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device,
            {
                {dynamicBufferBindingNumber, wgpu::ShaderStage::Compute,
                 wgpu::BufferBindingType::Uniform, true},
                {bufferBindingNumber, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
            });

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, bgl,
            {
                {dynamicBufferBindingNumber, dynamicBuffer, 0, sizeof(uint32_t)},
                {bufferBindingNumber, buffer, 0, sizeof(uint32_t)},
                {2, outputBuffer, 0, 2 * sizeof(uint32_t)},
            });

        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
        struct Buffer {
            value : u32
        }

        struct OutputBuffer {
            value : vec2u
        }

        @group(0) @binding(0) var<uniform> buffer0 : Buffer;
        @group(0) @binding(1) var<uniform> buffer1 : Buffer;
        @group(0) @binding(2) var<storage, read_write> outputBuffer : OutputBuffer;

        @compute @workgroup_size(1) fn main() {
            outputBuffer.value = vec2u(buffer0.value, buffer1.value);
        })");
        pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_RANGE_EQ(values.data(), outputBuffer, 0, values.size());
    };

    // Run the test with the dynamic buffer in index 0 and with the non-dynamic buffer in index 1,
    // and vice versa. This should cause a conflict at index 0, if the binding remapping is too
    // aggressive.
    RunTestWith(true);
    RunTestWith(false);
}

// Test that visibility of bindings in BindGroupLayout can be none
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, BindGroupLayoutVisibilityCanBeNone) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::BindGroupLayoutEntry entries[2];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::None;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::None;
    entries[1].texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 2;
    descriptor.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&descriptor);

    wgpu::RenderPipeline pipeline = MakeTestPipeline(renderPass, {}, {layout});

    std::array<float, 4> color = {1, 0, 0, 1};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {kRTSize, kRTSize};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView textureView = texture.CreateView();

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(
        device, layout, {{0, uniformBuffer, 0, sizeof(color)}, {1, textureView}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Regression test for crbug.com/dawn/448 that dynamic buffer bindings can have None visibility.
TEST_P(BindGroupTests, DynamicBindingNoneVisibility) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::None;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry.buffer.hasDynamicOffset = true;
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&descriptor);

    wgpu::RenderPipeline pipeline = MakeTestPipeline(renderPass, {}, {layout});

    std::array<float, 4> color = {1, 0, 0, 1};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    uint32_t dynamicOffset = 0;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test that bind group bindings may have unbounded and arbitrary binding numbers
TEST_P(BindGroupTests, ArbitraryBindingNumbers) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( 1.0, 1.0),
                vec2f(-1.0, -1.0));

            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct Ubo {
            color : vec4f
        }

        @group(0) @binding(553) var <uniform> ubo1 : Ubo;
        @group(0) @binding(47) var <uniform> ubo2 : Ubo;
        @group(0) @binding(111) var <uniform> ubo3 : Ubo;

        @fragment fn main() -> @location(0) vec4f {
            return ubo1.color + 2.0 * ubo2.color + 4.0 * ubo3.color;
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::Buffer black =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.f, 0.f, 0.f, 0.f});
    wgpu::Buffer red =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.251f, 0.0f, 0.0f, 0.0f});
    wgpu::Buffer green =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.0f, 0.251f, 0.0f, 0.0f});
    wgpu::Buffer blue =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.0f, 0.0f, 0.251f, 0.0f});

    auto DoTest = [&](wgpu::Buffer color1, wgpu::Buffer color2, wgpu::Buffer color3,
                      utils::RGBA8 filled) {
        auto DoTestInner = [&](wgpu::BindGroup bindGroup) {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 1);
        };

        utils::BindingInitializationHelper bindings[] = {
            {553, color1, 0, 4 * sizeof(float)},  //
            {47, color2, 0, 4 * sizeof(float)},   //
            {111, color3, 0, 4 * sizeof(float)},  //
        };

        // Should work regardless of what order the bindings are specified in.
        DoTestInner(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                         {bindings[0], bindings[1], bindings[2]}));
        DoTestInner(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                         {bindings[1], bindings[0], bindings[2]}));
        DoTestInner(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                         {bindings[2], bindings[0], bindings[1]}));
    };

    // first color is normal, second is 2x, third is 3x.
    DoTest(black, black, black, utils::RGBA8(0, 0, 0, 0));

    // Check the first binding maps to the first slot. We know this because the colors are
    // multiplied 1x.
    DoTest(red, black, black, utils::RGBA8(64, 0, 0, 0));
    DoTest(green, black, black, utils::RGBA8(0, 64, 0, 0));
    DoTest(blue, black, black, utils::RGBA8(0, 0, 64, 0));

    // Use multiple bindings and check the second color maps to the second slot.
    // We know this because the second slot is multiplied 2x.
    DoTest(green, blue, black, utils::RGBA8(0, 64, 128, 0));
    DoTest(blue, green, black, utils::RGBA8(0, 128, 64, 0));
    DoTest(red, green, black, utils::RGBA8(64, 128, 0, 0));

    // Use multiple bindings and check the third color maps to the third slot.
    // We know this because the third slot is multiplied 4x.
    DoTest(black, blue, red, utils::RGBA8(255, 0, 128, 0));
    DoTest(blue, black, green, utils::RGBA8(0, 255, 64, 0));
    DoTest(red, black, blue, utils::RGBA8(64, 0, 255, 0));
}

// This is a regression test for crbug.com/dawn/355 which tests that destruction of a bind group
// that holds the last reference to its bind group layout does not result in a use-after-free. In
// the bug, the destructor of BindGroupBase, when destroying member mLayout,
// Ref<BindGroupLayoutBase> assigns to Ref::mPointee, AFTER calling Release(). After the BGL
// is destroyed, the storage for |mPointee| has been freed.
TEST_P(BindGroupTests, LastReferenceToBindGroupLayout) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup bg;
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
        bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, sizeof(float)}});
    }
}

// Test that bind groups with an empty bind group layout may be created and used.
TEST_P(BindGroupTests, EmptyLayout) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(device, {});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        })");

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bg);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test creating a BGL with a storage buffer binding but declared readonly in the shader works.
// This is a regression test for crbug.com/dawn/410 which tests that it can successfully compile and
// execute the shader.
TEST_P(BindGroupTests, ReadonlyStorage) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().limits.maxStorageBuffersInFragmentStage < 1);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;

    pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( 1.0, 1.0),
                vec2f(-1.0, -1.0));

            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })");

    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        struct Buffer0 {
            color : vec4f
        }
        @group(0) @binding(0) var<storage, read> buffer0 : Buffer0;

        @fragment fn main() -> @location(0) vec4f {
            return buffer0.color;
        })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});

    pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    std::array<float, 4> greenColor = {0, 1, 0, 1};
    wgpu::Buffer storageBuffer = utils::CreateBufferFromData(
        device, &greenColor, sizeof(greenColor), wgpu::BufferUsage::Storage);

    pass.SetPipeline(renderPipeline);
    pass.SetBindGroup(0, utils::MakeBindGroup(device, bgl, {{0, storageBuffer}}));
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 0);
}

// This is a regression test for crbug.com/dawn/319 where creating a bind group with a
// destroyed resource would crash the backend.
TEST_P(BindGroupTests, CreateWithDestroyedResource) {
    auto doBufferTest = [&](wgpu::BufferBindingType bindingType, wgpu::BufferUsage usage) {
        wgpu::BindGroupLayout bgl =
            utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Fragment, bindingType}});

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = usage;
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
        buffer.Destroy();

        wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, sizeof(float)}});
    };

    // Test various usages and binding types since they take different backend code paths.
    doBufferTest(wgpu::BufferBindingType::Uniform, wgpu::BufferUsage::Uniform);
    if (GetSupportedLimits().limits.maxStorageBuffersInFragmentStage > 0) {
        doBufferTest(wgpu::BufferBindingType::Storage, wgpu::BufferUsage::Storage);
        doBufferTest(wgpu::BufferBindingType::ReadOnlyStorage, wgpu::BufferUsage::Storage);
    }

    // Test a sampled texture.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::TextureBinding;
        textureDesc.size = {1, 1, 1};
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

        // Create view, then destroy.
        {
            wgpu::Texture texture = device.CreateTexture(&textureDesc);
            wgpu::TextureView textureView = texture.CreateView();

            texture.Destroy();
            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, textureView}});
        }
        // Destroy, then create view.
        {
            wgpu::Texture texture = device.CreateTexture(&textureDesc);
            texture.Destroy();
            wgpu::TextureView textureView = texture.CreateView();

            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, textureView}});
        }
    }

    // Test a storage texture.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly,
                      wgpu::TextureFormat::R32Uint}});

        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::StorageBinding;
        textureDesc.size = {1, 1, 1};
        textureDesc.format = wgpu::TextureFormat::R32Uint;

        // Create view, then destroy.
        {
            wgpu::Texture texture = device.CreateTexture(&textureDesc);
            wgpu::TextureView textureView = texture.CreateView();

            texture.Destroy();
            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, textureView}});
        }
        // Destroy, then create view.
        {
            wgpu::Texture texture = device.CreateTexture(&textureDesc);
            texture.Destroy();
            wgpu::TextureView textureView = texture.CreateView();

            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, textureView}});
        }
    }
}

DAWN_INSTANTIATE_TEST(BindGroupTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // namespace
}  // namespace dawn
