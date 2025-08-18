// Copyright 2019 The Dawn & Tint Authors
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

#include <array>
#include <string>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/wire/WireTest.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::AllOf;
using testing::Eq;
using testing::Field;
using testing::Return;
using testing::Sequence;

MATCHER_P2(EqBytes, bytes, size, "") {
    const char* dataToCheck = arg;
    bool isMatch = (memcmp(dataToCheck, bytes, size) == 0);
    return isMatch;
}

class WireArgumentTests : public WireTest {
  public:
    WireArgumentTests() {}
    ~WireArgumentTests() override = default;
};

// Test that the wire is able to send numerical values
TEST_F(WireArgumentTests, ValueArgument) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.DispatchWorkgroups(1, 2, 3);

    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    WGPUComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandEncoderBeginComputePass(apiEncoder, nullptr)).WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderDispatchWorkgroups(apiPass, 1, 2, 3)).Times(1);

    FlushClient();
}

// Test that the wire is able to send arrays of numerical values
TEST_F(WireArgumentTests, ValueArrayArgument) {
    // Create a bindgroup.
    wgpu::BindGroupLayoutDescriptor bglDescriptor = {};

    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDescriptor);
    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _)).WillOnce(Return(apiBgl));

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = bgl;

    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    WGPUBindGroup apiBindGroup = api.GetNewBindGroup();
    EXPECT_CALL(api, DeviceCreateBindGroup(apiDevice, _)).WillOnce(Return(apiBindGroup));

    // Use the bindgroup in SetBindGroup that takes an array of value offsets.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    std::array<uint32_t, 4> testOffsets = {0, 42, 0xDEAD'BEEFu, 0xFFFF'FFFFu};
    pass.SetBindGroup(0, bindGroup, testOffsets.size(), testOffsets.data());

    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    WGPUComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandEncoderBeginComputePass(apiEncoder, nullptr)).WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderSetBindGroup(
                         apiPass, 0, apiBindGroup, testOffsets.size(),
                         MatchesLambda([testOffsets](const uint32_t* offsets) -> bool {
                             for (size_t i = 0; i < testOffsets.size(); i++) {
                                 if (offsets[i] != testOffsets[i]) {
                                     return false;
                                 }
                             }
                             return true;
                         })));

    FlushClient();
}

// Test that the wire is able to send C strings
TEST_F(WireArgumentTests, CStringArgument) {
    // Create shader module
    wgpu::ShaderModuleDescriptor vertexDescriptor = {};
    wgpu::ShaderModule vsModule = device.CreateShaderModule(&vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    // Create the color state descriptor
    wgpu::BlendComponent blendComponent = {};
    wgpu::BlendState blendState = {};
    blendState.alpha = blendComponent;
    blendState.color = blendComponent;
    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = wgpu::TextureFormat::RGBA8Unorm;
    colorTargetState.blend = &blendState;

    // Create the depth-stencil state
    wgpu::StencilFaceState stencilFace = {};

    wgpu::DepthStencilState depthStencilState = {};
    depthStencilState.format = wgpu::TextureFormat::Depth24PlusStencil8;
    depthStencilState.depthCompare = wgpu::CompareFunction::Always;
    depthStencilState.stencilBack = stencilFace;
    depthStencilState.stencilFront = stencilFace;

    // Create the pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDescriptor = {};
    wgpu::PipelineLayout layout = device.CreatePipelineLayout(&layoutDescriptor);
    WGPUPipelineLayout apiLayout = api.GetNewPipelineLayout();
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, _)).WillOnce(Return(apiLayout));

    // Create pipeline
    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.entryPoint = "main";

    wgpu::FragmentState fragment = {};
    fragment.module = vsModule;
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.layout = layout;
    pipelineDescriptor.depthStencil = &depthStencilState;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    WGPURenderPipeline apiPlaceholderPipeline = api.GetNewRenderPipeline();
    EXPECT_CALL(api,
                DeviceCreateRenderPipeline(
                    apiDevice, MatchesLambda([](const WGPURenderPipelineDescriptor* desc) -> bool {
                        return std::string_view(desc->vertex.entryPoint.data,
                                                desc->vertex.entryPoint.length) == "main";
                    })))
        .WillOnce(Return(apiPlaceholderPipeline));

    FlushClient();
}

// Test that the wire is able to send WGPUStringViews
TEST_F(WireArgumentTests, WGPUStringView) {
    // Create shader module
    wgpu::ShaderModuleDescriptor vertexDescriptor = {};
    wgpu::ShaderModule vsModule = device.CreateShaderModule(&vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    const char* label = "null-terminated label\0more string";
    vsModule.SetLabel(std::string_view(label));
    EXPECT_CALL(api, ShaderModuleSetLabel(apiVsModule,
                                          AllOf(Field(&WGPUStringView::data, EqBytes(label, 21u)),
                                                Field(&WGPUStringView::length, Eq(21u)))));
    FlushClient();

    // Give it a longer, explicit length that contains the null-terminator.
    vsModule.SetLabel(std::string_view(label, 34));
    EXPECT_CALL(api, ShaderModuleSetLabel(apiVsModule,
                                          AllOf(Field(&WGPUStringView::data, EqBytes(label, 34u)),
                                                Field(&WGPUStringView::length, Eq(34u)))));
    FlushClient();

    // Give it a shorder, explicit length.
    vsModule.SetLabel(std::string_view(label, 2));
    EXPECT_CALL(api, ShaderModuleSetLabel(apiVsModule,
                                          AllOf(Field(&WGPUStringView::data, EqBytes(label, 2u)),
                                                Field(&WGPUStringView::length, Eq(2u)))));
    FlushClient();

    // Give it a zero length.
    vsModule.SetLabel(std::string_view(label, 0));
    EXPECT_CALL(
        api, ShaderModuleSetLabel(apiVsModule, AllOf(Field(&WGPUStringView::data, EqBytes("", 1u)),
                                                     Field(&WGPUStringView::length, Eq(0u)))));
    FlushClient();

    // Give it zero length and data.
    vsModule.SetLabel(std::string_view(nullptr, 0));
    EXPECT_CALL(api,
                ShaderModuleSetLabel(apiVsModule, AllOf(Field(&WGPUStringView::data, nullptr),
                                                        Field(&WGPUStringView::length, Eq(0u)))));
    FlushClient();

    // Give it the nil string with nullopt.
    vsModule.SetLabel(std::nullopt);
    EXPECT_CALL(api, ShaderModuleSetLabel(apiVsModule,
                                          AllOf(Field(&WGPUStringView::data, nullptr),
                                                Field(&WGPUStringView::length, Eq(WGPU_STRLEN)))));
    FlushClient();
}

// Test that the wire is able to send objects as value arguments
TEST_F(WireArgumentTests, ObjectAsValueArgument) {
    wgpu::CommandEncoder cmdBufEncoder = device.CreateCommandEncoder();
    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
        .WillOnce(Return(apiBuffer))
        .RetiresOnSaturation();

    cmdBufEncoder.CopyBufferToBuffer(buffer, 0, buffer, 4, 4);
    EXPECT_CALL(api, CommandEncoderCopyBufferToBuffer(apiEncoder, apiBuffer, 0, apiBuffer, 4, 4));

    FlushClient();
}

// Test that the wire is able to send array of objects
TEST_F(WireArgumentTests, ObjectsAsPointerArgument) {
    wgpu::CommandBuffer cmdBufs[2];
    WGPUCommandBuffer apiCmdBufs[2];

    // Create two command buffers we need to use a GMock sequence otherwise the order of the
    // CreateCommandEncoder might be swapped since they are equivalent in term of matchers
    Sequence s;
    for (int i = 0; i < 2; ++i) {
        wgpu::CommandEncoder cmdBufEncoder = device.CreateCommandEncoder();
        cmdBufs[i] = cmdBufEncoder.Finish();

        WGPUCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
        EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
            .InSequence(s)
            .WillOnce(Return(apiCmdBufEncoder));

        apiCmdBufs[i] = api.GetNewCommandBuffer();
        EXPECT_CALL(api, CommandEncoderFinish(apiCmdBufEncoder, nullptr))
            .WillOnce(Return(apiCmdBufs[i]));

        EXPECT_CALL(api, CommandEncoderRelease(apiCmdBufEncoder));
    }

    // Submit command buffer and check we got a call with both API-side command buffers
    queue.Submit(2, cmdBufs);

    EXPECT_CALL(
        api, QueueSubmit(apiQueue, 2, MatchesLambda([=](const WGPUCommandBuffer* cmdBufs) -> bool {
                             return cmdBufs[0] == apiCmdBufs[0] && cmdBufs[1] == apiCmdBufs[1];
                         })));

    FlushClient();
}

// Test that the wire is able to send structures that contain pure values (non-objects)
TEST_F(WireArgumentTests, StructureOfValuesArgument) {
    wgpu::SamplerDescriptor descriptor = {};
    descriptor.magFilter = wgpu::FilterMode::Linear;
    descriptor.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    descriptor.addressModeV = wgpu::AddressMode::Repeat;
    descriptor.addressModeW = wgpu::AddressMode::MirrorRepeat;
    descriptor.lodMaxClamp = kLodMax;
    descriptor.compare = wgpu::CompareFunction::Never;

    wgpu::Sampler sampler = device.CreateSampler(&descriptor);

    WGPUSampler apiPlaceholderSampler = api.GetNewSampler();
    EXPECT_CALL(api, DeviceCreateSampler(
                         apiDevice, MatchesLambda([](const WGPUSamplerDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr &&
                                    desc->magFilter == WGPUFilterMode_Linear &&
                                    desc->minFilter == WGPUFilterMode_Undefined &&
                                    desc->mipmapFilter == WGPUMipmapFilterMode_Linear &&
                                    desc->addressModeU == WGPUAddressMode_Undefined &&
                                    desc->addressModeV == WGPUAddressMode_Repeat &&
                                    desc->addressModeW == WGPUAddressMode_MirrorRepeat &&
                                    desc->compare == WGPUCompareFunction_Never &&
                                    desc->lodMinClamp == kLodMin && desc->lodMaxClamp == kLodMax;
                         })))
        .WillOnce(Return(apiPlaceholderSampler));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireArgumentTests, StructureOfObjectArrayArgument) {
    wgpu::BindGroupLayoutDescriptor bglDescriptor = {};

    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDescriptor);
    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _)).WillOnce(Return(apiBgl));

    wgpu::PipelineLayoutDescriptor descriptor = {};
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = &bgl;

    wgpu::PipelineLayout layout = device.CreatePipelineLayout(&descriptor);

    WGPUPipelineLayout apiPlaceholderLayout = api.GetNewPipelineLayout();
    EXPECT_CALL(api, DeviceCreatePipelineLayout(
                         apiDevice,
                         MatchesLambda([apiBgl](const WGPUPipelineLayoutDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr &&
                                    desc->bindGroupLayoutCount == 1 &&
                                    desc->bindGroupLayouts[0] == apiBgl;
                         })))
        .WillOnce(Return(apiPlaceholderLayout));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireArgumentTests, StructureOfStructureArrayArgument) {
    static constexpr int NUM_BINDINGS = 3;
    wgpu::BindGroupLayoutEntry entries[NUM_BINDINGS]{
        {
            .binding = 0,
            .visibility = wgpu::ShaderStage::Vertex,
            .sampler = {nullptr, wgpu::SamplerBindingType::Filtering},
        },
        {
            .binding = 1,
            .visibility = wgpu::ShaderStage::Vertex,
            .texture = {nullptr, wgpu::TextureSampleType::Float, wgpu::TextureViewDimension::e2D,
                        false},
        },
        {
            .binding = 2,
            .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
            .buffer = {nullptr, wgpu::BufferBindingType::Uniform, false, 0},
        },
    };
    wgpu::BindGroupLayoutDescriptor bglDescriptor = {};
    bglDescriptor.entryCount = NUM_BINDINGS;
    bglDescriptor.entries = entries;

    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDescriptor);
    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(
        api,
        DeviceCreateBindGroupLayout(
            apiDevice, MatchesLambda([entries](const WGPUBindGroupLayoutDescriptor* desc) -> bool {
                for (int i = 0; i < NUM_BINDINGS; ++i) {
                    const auto& a = desc->entries[i];
                    const auto& b = entries[i];
                    if (a.binding != b.binding ||
                        a.visibility != static_cast<WGPUShaderStage>(b.visibility) ||
                        a.buffer.type != static_cast<WGPUBufferBindingType>(b.buffer.type) ||
                        a.sampler.type != static_cast<WGPUSamplerBindingType>(b.sampler.type) ||
                        a.texture.sampleType !=
                            static_cast<WGPUTextureSampleType>(b.texture.sampleType)) {
                        return false;
                    }
                }
                return desc->nextInChain == nullptr && desc->entryCount == 3;
            })))
        .WillOnce(Return(apiBgl));

    FlushClient();
}

// Test passing nullptr instead of objects - array of objects version
TEST_F(WireArgumentTests, DISABLED_NullptrInArray) {
    wgpu::BindGroupLayout nullBGL = nullptr;

    wgpu::PipelineLayoutDescriptor descriptor = {};
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = &nullBGL;

    wgpu::PipelineLayout pl = device.CreatePipelineLayout(&descriptor);
    EXPECT_CALL(api,
                DeviceCreatePipelineLayout(
                    apiDevice, MatchesLambda([](const WGPUPipelineLayoutDescriptor* desc) -> bool {
                        return desc->nextInChain == nullptr && desc->bindGroupLayoutCount == 1 &&
                               desc->bindGroupLayouts[0] == nullptr;
                    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
