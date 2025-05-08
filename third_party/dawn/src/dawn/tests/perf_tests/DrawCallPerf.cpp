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

#include <tuple>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr unsigned int kNumDraws = 2000;

constexpr uint32_t kTextureSize = 64;
constexpr size_t kUniformSize = 3 * sizeof(float);

constexpr float kVertexData[12] = {
    0.0f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.0f, 1.0f,
};

constexpr char kVertexShader[] = R"(
        @vertex fn main(
            @location(0) pos : vec4f
        ) -> @builtin(position) vec4f {
            return pos;
        })";

constexpr char kFragmentShaderA[] = R"(
        @group(0) @binding(0) var<uniform> color : vec3f;
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(color * (1.0 / 5000.0), 1.0);
        })";

constexpr char kFragmentShaderB[] = R"(
        @group(0) @binding(0) var<uniform> constant_color : vec3f;
        @group(1) @binding(0) var<uniform> uniform_color : vec3f;

        @fragment fn main() -> @location(0) vec4f {
            return vec4f((constant_color + uniform_color) * (1.0 / 5000.0), 1.0);
        })";

enum class Pipeline {
    Static,     // Keep the same pipeline for all draws.
    Redundant,  // Use the same pipeline, but redundantly set it.
    Dynamic,    // Change the pipeline between draws.
};

enum class UniformData {
    Static,   // Don't update per-draw uniform data.
    Dynamic,  // Update the per-draw uniform data once per frame.
};

enum class BindGroup {
    NoChange,   // Use one bind group for all draws.
    Redundant,  // Use the same bind group, but redundantly set it.
    NoReuse,    // Create a new bind group every time.
    Multiple,   // Use multiple static bind groups.
    Dynamic,    // Use bind groups with dynamic offsets.
};

enum class VertexBuffer {
    NoChange,  // Use one vertex buffer for all draws.
    Multiple,  // Use multiple static vertex buffers.
    Dynamic,   // Switch vertex buffers between draws.
};

enum class RenderBundle {
    No,   // Record commands in a render pass
    Yes,  // Record commands in a render bundle
};

struct DrawCallParam {
    Pipeline pipelineType;
    VertexBuffer vertexBufferType;
    BindGroup bindGroupType;
    UniformData uniformDataType;
    RenderBundle withRenderBundle;
};

using DrawCallParamTuple = std::tuple<Pipeline, VertexBuffer, BindGroup, UniformData, RenderBundle>;

template <typename T>
unsigned int AssignParam(T& lhs, T rhs) {
    lhs = rhs;
    return 0u;
}

// This helper function allows creating a DrawCallParam from a list of arguments
// without specifying all of the members. Provided members can be passed once in an arbitrary
// order. Unspecified members default to:
//  - Pipeline::Static
//  - VertexBuffer::NoChange
//  - BindGroup::NoChange
//  - UniformData::Static
//  - RenderBundle::No
template <typename... Ts>
DrawCallParam MakeParam(Ts... args) {
    // Baseline param
    DrawCallParamTuple paramTuple{Pipeline::Static, VertexBuffer::NoChange, BindGroup::NoChange,
                                  UniformData::Static, RenderBundle::No};

    [[maybe_unused]] unsigned int unused[] = {
        0,  // Avoid making a 0-sized array.
        AssignParam(std::get<Ts>(paramTuple), args)...,
    };

    return DrawCallParam{
        std::get<Pipeline>(paramTuple),     std::get<VertexBuffer>(paramTuple),
        std::get<BindGroup>(paramTuple),    std::get<UniformData>(paramTuple),
        std::get<RenderBundle>(paramTuple),
    };
}

struct DrawCallParamForTest : AdapterTestParam {
    DrawCallParamForTest(const AdapterTestParam& backendParam, DrawCallParam param)
        : AdapterTestParam(backendParam), param(param) {}
    DrawCallParam param;
};

std::ostream& operator<<(std::ostream& ostream, const DrawCallParamForTest& testParams) {
    ostream << static_cast<const AdapterTestParam&>(testParams);

    const DrawCallParam& param = testParams.param;

    switch (param.pipelineType) {
        case Pipeline::Static:
            break;
        case Pipeline::Redundant:
            ostream << "_RedundantPipeline";
            break;
        case Pipeline::Dynamic:
            ostream << "_DynamicPipeline";
            break;
    }

    switch (param.vertexBufferType) {
        case VertexBuffer::NoChange:
            break;
        case VertexBuffer::Multiple:
            ostream << "_MultipleVertexBuffers";
            break;
        case VertexBuffer::Dynamic:
            ostream << "_DynamicVertexBuffer";
    }

    switch (param.bindGroupType) {
        case BindGroup::NoChange:
            break;
        case BindGroup::Redundant:
            ostream << "_RedundantBindGroups";
            break;
        case BindGroup::NoReuse:
            ostream << "_NoReuseBindGroups";
            break;
        case BindGroup::Multiple:
            ostream << "_MultipleBindGroups";
            break;
        case BindGroup::Dynamic:
            ostream << "_DynamicBindGroup";
            break;
    }

    switch (param.uniformDataType) {
        case UniformData::Static:
            break;
        case UniformData::Dynamic:
            ostream << "_DynamicData";
            break;
    }

    switch (param.withRenderBundle) {
        case RenderBundle::No:
            break;
        case RenderBundle::Yes:
            ostream << "_RenderBundle";
            break;
    }

    return ostream;
}

// DrawCallPerf is an uber-benchmark with supports many parameterizations.
// The specific parameterizations we care about are explicitly instantiated at the bottom
// of this test file.
// DrawCallPerf tests drawing a simple triangle with many ways of encoding commands,
// binding, and uploading data to the GPU. The rationale for this is the following:
//   - Static/Multiple/Dynamic vertex buffers: Tests switching buffer bindings. This has
//     a state tracking cost as well as a GPU driver cost.
//   - Static/Multiple/Dynamic bind groups: Same rationale as vertex buffers
//   - Static/Dynamic pipelines: In addition to a change to GPU state, changing the pipeline
//     layout incurs additional state tracking costs in Dawn.
//   - With/Without render bundles: All of the above can have lower validation costs if
//     precomputed in a render bundle.
//   - Static/Dynamic data: Updating data for each draw is a common use case. It also tests
//     the efficiency of resource transitions.
class DrawCallPerf : public DawnPerfTestWithParams<DrawCallParamForTest> {
  public:
    DrawCallPerf() : DawnPerfTestWithParams(kNumDraws, 3) {}
    ~DrawCallPerf() override = default;

    void SetUp() override;

  protected:
    DrawCallParam GetParam() const { return DawnPerfTestWithParams::GetParam().param; }

    template <typename Encoder>
    void RecordRenderCommands(Encoder encoder);

  private:
    void Step() override;

    // One large dynamic vertex buffer, or multiple separate vertex buffers.
    wgpu::Buffer mVertexBuffers[kNumDraws];
    size_t mAlignedVertexDataSize;

    std::vector<float> mUniformBufferData;
    // One large dynamic uniform buffer, or multiple separate uniform buffers.
    wgpu::Buffer mUniformBuffers[kNumDraws];

    wgpu::BindGroupLayout mUniformBindGroupLayout;
    // One dynamic bind group or multiple bind groups.
    wgpu::BindGroup mUniformBindGroups[kNumDraws];
    size_t mAlignedUniformSize;
    size_t mNumUniformFloats;

    wgpu::BindGroupLayout mConstantBindGroupLayout;
    wgpu::BindGroup mConstantBindGroup;

    // If the pipeline is static, only the first is used.
    // Otherwise, the test alternates between two pipelines for each draw.
    wgpu::RenderPipeline mPipelines[2];

    wgpu::TextureView mColorAttachment;
    wgpu::TextureView mDepthStencilAttachment;

    wgpu::RenderBundle mRenderBundle;
};

void DrawCallPerf::SetUp() {
    DawnPerfTestWithParams::SetUp();

    // Compute aligned uniform / vertex data sizes.
    mAlignedUniformSize = Align(kUniformSize, GetSupportedLimits().minUniformBufferOffsetAlignment);
    mAlignedVertexDataSize = Align(sizeof(kVertexData), 4);

    // Initialize uniform buffer data.
    mNumUniformFloats = mAlignedUniformSize / sizeof(float);
    mUniformBufferData = std::vector<float>(kNumDraws * mNumUniformFloats, 0.0);

    // Create the color / depth stencil attachments.
    {
        wgpu::TextureDescriptor descriptor = {};
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kTextureSize;
        descriptor.size.height = kTextureSize;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;

        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        mColorAttachment = device.CreateTexture(&descriptor).CreateView();

        descriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
        mDepthStencilAttachment = device.CreateTexture(&descriptor).CreateView();
    }

    // Create vertex buffer(s)
    switch (GetParam().vertexBufferType) {
        case VertexBuffer::NoChange:
            mVertexBuffers[0] = utils::CreateBufferFromData(
                device, kVertexData, sizeof(kVertexData), wgpu::BufferUsage::Vertex);
            break;

        case VertexBuffer::Multiple: {
            for (uint32_t i = 0; i < kNumDraws; ++i) {
                mVertexBuffers[i] = utils::CreateBufferFromData(
                    device, kVertexData, sizeof(kVertexData), wgpu::BufferUsage::Vertex);
            }
            break;
        }

        case VertexBuffer::Dynamic: {
            std::vector<char> data(mAlignedVertexDataSize * kNumDraws);
            for (uint32_t i = 0; i < kNumDraws; ++i) {
                memcpy(data.data() + mAlignedVertexDataSize * i, kVertexData, sizeof(kVertexData));
            }

            mVertexBuffers[0] = utils::CreateBufferFromData(device, data.data(), data.size(),
                                                            wgpu::BufferUsage::Vertex);
            break;
        }
    }

    // Create the bind group layout.
    switch (GetParam().bindGroupType) {
        case BindGroup::NoChange:
        case BindGroup::Redundant:
        case BindGroup::NoReuse:
        case BindGroup::Multiple:
            mUniformBindGroupLayout = utils::MakeBindGroupLayout(
                device,
                {
                    {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, false},
                });
            break;

        case BindGroup::Dynamic:
            mUniformBindGroupLayout = utils::MakeBindGroupLayout(
                device,
                {
                    {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true},
                });
            break;

        default:
            DAWN_UNREACHABLE();
            break;
    }

    // Setup the base render pipeline descriptor.
    utils::ComboRenderPipelineDescriptor renderPipelineDesc;
    renderPipelineDesc.vertex.bufferCount = 1;
    renderPipelineDesc.cBuffers[0].arrayStride = 4 * sizeof(float);
    renderPipelineDesc.cBuffers[0].attributeCount = 1;
    renderPipelineDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    renderPipelineDesc.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
    renderPipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    // Create the pipeline layout for the first pipeline.
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayouts = &mUniformBindGroupLayout;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    // Create the shaders for the first pipeline.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, kVertexShader);
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, kFragmentShaderA);

    // Create the first pipeline.
    renderPipelineDesc.layout = pipelineLayout;
    renderPipelineDesc.vertex.module = vsModule;
    renderPipelineDesc.cFragment.module = fsModule;
    mPipelines[0] = device.CreateRenderPipeline(&renderPipelineDesc);

    // If the test is using a dynamic pipeline, create the second pipeline.
    if (GetParam().pipelineType == Pipeline::Dynamic) {
        // Create another bind group layout. The data for this binding point will be the same for
        // all draws.
        mConstantBindGroupLayout = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, false},
                    });

        // Create the pipeline layout for the second pipeline.
        wgpu::BindGroupLayout bindGroupLayouts[2] = {
            mConstantBindGroupLayout,
            mUniformBindGroupLayout,
        };
        pipelineLayoutDesc.bindGroupLayouts = bindGroupLayouts,
        pipelineLayoutDesc.bindGroupLayoutCount = 2;
        pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

        // Create the fragment shader module. This shader matches the pipeline layout described
        // above.
        fsModule = utils::CreateShaderModule(device, kFragmentShaderB);

        // Create the pipeline.
        renderPipelineDesc.layout = pipelineLayout;
        renderPipelineDesc.cFragment.module = fsModule;
        mPipelines[1] = device.CreateRenderPipeline(&renderPipelineDesc);

        // Create the buffer and bind group to bind to the constant bind group layout slot.
        constexpr float kConstantData[] = {0.01, 0.02, 0.03};
        wgpu::Buffer constantBuffer = utils::CreateBufferFromData(
            device, kConstantData, sizeof(kConstantData), wgpu::BufferUsage::Uniform);
        mConstantBindGroup = utils::MakeBindGroup(device, mConstantBindGroupLayout,
                                                  {{0, constantBuffer, 0, sizeof(kConstantData)}});
    }

    // Create the buffers and bind groups for the per-draw uniform data.
    switch (GetParam().bindGroupType) {
        case BindGroup::NoChange:
        case BindGroup::Redundant:
            mUniformBuffers[0] = utils::CreateBufferFromData(
                device, mUniformBufferData.data(), 3 * sizeof(float), wgpu::BufferUsage::Uniform);

            mUniformBindGroups[0] = utils::MakeBindGroup(
                device, mUniformBindGroupLayout, {{0, mUniformBuffers[0], 0, kUniformSize}});
            break;

        case BindGroup::NoReuse:
            for (uint32_t i = 0; i < kNumDraws; ++i) {
                mUniformBuffers[i] = utils::CreateBufferFromData(
                    device, mUniformBufferData.data() + i * mNumUniformFloats, 3 * sizeof(float),
                    wgpu::BufferUsage::Uniform);
            }
            // Bind groups are created on-the-fly.
            break;

        case BindGroup::Multiple:
            for (uint32_t i = 0; i < kNumDraws; ++i) {
                mUniformBuffers[i] = utils::CreateBufferFromData(
                    device, mUniformBufferData.data() + i * mNumUniformFloats, 3 * sizeof(float),
                    wgpu::BufferUsage::Uniform);

                mUniformBindGroups[i] = utils::MakeBindGroup(
                    device, mUniformBindGroupLayout, {{0, mUniformBuffers[i], 0, kUniformSize}});
            }
            break;

        case BindGroup::Dynamic:
            mUniformBuffers[0] = utils::CreateBufferFromData(
                device, mUniformBufferData.data(), mUniformBufferData.size() * sizeof(float),
                wgpu::BufferUsage::Uniform);

            mUniformBindGroups[0] = utils::MakeBindGroup(
                device, mUniformBindGroupLayout, {{0, mUniformBuffers[0], 0, kUniformSize}});
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    // If using render bundles, record the render commands now.
    if (GetParam().withRenderBundle == RenderBundle::Yes) {
        wgpu::RenderBundleEncoderDescriptor descriptor = {};
        descriptor.colorFormatCount = 1;
        descriptor.colorFormats = &renderPipelineDesc.cTargets[0].format;
        descriptor.depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        RecordRenderCommands(encoder);
        mRenderBundle = encoder.Finish();
    }
}

template <typename Encoder>
void DrawCallPerf::RecordRenderCommands(Encoder pass) {
    uint32_t uniformBindGroupIndex = 0;

    if (GetParam().pipelineType == Pipeline::Static) {
        // Static pipeline can be set now.
        pass.SetPipeline(mPipelines[0]);
    }

    if (GetParam().vertexBufferType == VertexBuffer::NoChange) {
        // Static vertex buffer can be set now.
        pass.SetVertexBuffer(0, mVertexBuffers[0]);
    }

    if (GetParam().bindGroupType == BindGroup::NoChange) {
        // Incompatible. Can't change pipeline without changing bind groups.
        DAWN_ASSERT(GetParam().pipelineType == Pipeline::Static);

        // Static bind group can be set now.
        pass.SetBindGroup(uniformBindGroupIndex, mUniformBindGroups[0]);
    }

    for (unsigned int i = 0; i < kNumDraws; ++i) {
        switch (GetParam().pipelineType) {
            case Pipeline::Static:
                break;
            case Pipeline::Redundant:
                pass.SetPipeline(mPipelines[0]);
                break;
            case Pipeline::Dynamic: {
                // If the pipeline is dynamic, ping pong between two pipelines.
                pass.SetPipeline(mPipelines[i % 2]);

                // The pipelines have different layouts so we change the binding index here.
                uniformBindGroupIndex = i % 2;
                if (uniformBindGroupIndex == 1) {
                    // Because of the pipeline layout change, we need to rebind bind group index 0.
                    pass.SetBindGroup(0, mConstantBindGroup);
                }
                break;
            }
        }

        // Set the vertex buffer, if it changes.
        switch (GetParam().vertexBufferType) {
            case VertexBuffer::NoChange:
                break;

            case VertexBuffer::Multiple:
                pass.SetVertexBuffer(0, mVertexBuffers[i]);
                break;

            case VertexBuffer::Dynamic:
                pass.SetVertexBuffer(0, mVertexBuffers[0], i * mAlignedVertexDataSize);
                break;
        }

        // Set the bind group, if it changes.
        switch (GetParam().bindGroupType) {
            case BindGroup::NoChange:
                break;

            case BindGroup::Redundant:
                pass.SetBindGroup(uniformBindGroupIndex, mUniformBindGroups[0]);
                break;

            case BindGroup::NoReuse: {
                wgpu::BindGroup bindGroup = utils::MakeBindGroup(
                    device, mUniformBindGroupLayout, {{0, mUniformBuffers[i], 0, kUniformSize}});
                pass.SetBindGroup(uniformBindGroupIndex, bindGroup);
                break;
            }

            case BindGroup::Multiple:
                pass.SetBindGroup(uniformBindGroupIndex, mUniformBindGroups[i]);
                break;

            case BindGroup::Dynamic: {
                uint32_t dynamicOffset = static_cast<uint32_t>(i * mAlignedUniformSize);
                pass.SetBindGroup(uniformBindGroupIndex, mUniformBindGroups[0], 1, &dynamicOffset);
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }
        pass.Draw(3);
    }
}

void DrawCallPerf::Step() {
    if (GetParam().uniformDataType == UniformData::Dynamic) {
        // Update uniform data if it's dynamic.
        std::fill(mUniformBufferData.begin(), mUniformBufferData.end(),
                  mUniformBufferData[0] + 1.0);

        switch (GetParam().bindGroupType) {
            case BindGroup::NoChange:
            case BindGroup::Redundant:
                queue.WriteBuffer(mUniformBuffers[0], 0, mUniformBufferData.data(),
                                  3 * sizeof(float));
                break;
            case BindGroup::NoReuse:
            case BindGroup::Multiple:
                for (uint32_t i = 0; i < kNumDraws; ++i) {
                    queue.WriteBuffer(mUniformBuffers[i], 0,
                                      mUniformBufferData.data() + i * mNumUniformFloats,
                                      3 * sizeof(float));
                }
                break;
            case BindGroup::Dynamic:
                queue.WriteBuffer(mUniformBuffers[0], 0, mUniformBufferData.data(),
                                  mUniformBufferData.size() * sizeof(float));
                break;
        }
    }

    wgpu::CommandEncoder commands = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor renderPass({mColorAttachment}, mDepthStencilAttachment);
    wgpu::RenderPassEncoder pass = commands.BeginRenderPass(&renderPass);

    switch (GetParam().withRenderBundle) {
        case RenderBundle::No:
            RecordRenderCommands(pass);
            break;
        case RenderBundle::Yes:
            pass.ExecuteBundles(1, &mRenderBundle);
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    pass.End();
    wgpu::CommandBuffer commandBuffer = commands.Finish();
    queue.Submit(1, &commandBuffer);
}

TEST_P(DrawCallPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(
    DrawCallPerf,
    {D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend(),
     VulkanBackend({"skip_validation"})},
    {
        // Baseline
        MakeParam(),

        // Change vertex buffer binding
        MakeParam(VertexBuffer::Multiple),  // Multiple vertex buffers
        MakeParam(VertexBuffer::Dynamic),   // Dynamic vertex buffer

        // Change bind group binding
        MakeParam(BindGroup::Multiple),  // Multiple bind groups
        MakeParam(BindGroup::Dynamic),   // Dynamic bind groups
        MakeParam(BindGroup::NoReuse),   // New bind group per-draw

        // Redundantly set pipeline / bind groups
        MakeParam(Pipeline::Redundant, BindGroup::Redundant),

        // Switch the pipeline every draw to test state tracking and updates to binding points
        MakeParam(Pipeline::Dynamic,
                  BindGroup::Multiple),  // Multiple bind groups w/ dynamic pipeline
        MakeParam(Pipeline::Dynamic,
                  BindGroup::Dynamic),  // Dynamic bind groups w/ dynamic pipeline

        // ----------- Render Bundles -----------
        // Command validation / state tracking can be futher optimized / precomputed.
        // Use render bundles with varying vertex buffer binding
        MakeParam(VertexBuffer::Multiple,
                  RenderBundle::Yes),  // Multiple vertex buffers w/ render bundle
        MakeParam(VertexBuffer::Dynamic,
                  RenderBundle::Yes),  // Dynamic vertex buffer w/ render bundle

        // Use render bundles with varying bind group binding
        MakeParam(BindGroup::Multiple, RenderBundle::Yes),  // Multiple bind groups w/ render bundle
        MakeParam(BindGroup::Dynamic, RenderBundle::Yes),   // Dynamic bind groups w/ render bundle

        // Use render bundles with dynamic pipeline
        MakeParam(Pipeline::Dynamic,
                  BindGroup::Multiple,
                  RenderBundle::Yes),  // Multiple bind groups w/ dynamic pipeline w/ render bundle
        MakeParam(Pipeline::Dynamic,
                  BindGroup::Dynamic,
                  RenderBundle::Yes),  // Dynamic bind groups w/ dynamic pipeline w/ render bundle

        // ----------- Render Bundles (end)-------

        // Update per-draw data in the bind group(s). This will cause resource transitions between
        // updating and drawing.
        MakeParam(BindGroup::Multiple,
                  UniformData::Dynamic),  // Update per-draw data: Multiple bind groups
        MakeParam(BindGroup::Dynamic,
                  UniformData::Dynamic),  // Update per-draw data: Dynamic bind groups
    });

}  // anonymous namespace
}  // namespace dawn
