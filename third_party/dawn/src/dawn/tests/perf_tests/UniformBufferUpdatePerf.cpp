// Copyright 2024 The Dawn & Tint Authors
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

#include <queue>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

// This is for developers only to ensure the triangle color drawn is as expected.
// #define PIXEL_CHECK 1

namespace dawn {
namespace {

constexpr unsigned int kNumIterations = 100;

constexpr uint32_t kTextureSize = 128;
constexpr size_t kUniformDataSize = 4 * sizeof(float);
constexpr size_t kUniformBufferSize = 256;

constexpr float kVertexData[12] = {
    0.0f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.0f, 1.0f,
};

constexpr char kVertexShader[] = R"(
        @vertex fn main(
            @location(0) pos : vec4f
        ) -> @builtin(position) vec4f {
            return pos;
        })";

constexpr char kFragmentShader[] = R"(
        @group(0) @binding(0) var<uniform> color : vec3f;
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(color * (1.0 / %d), 1.0);
        })";

enum class UploadMethod {
    // Use Queue.WriteBuffer() to update the uniform buffers.
    WriteBuffer,
    // Map and copy to a common staging buffer first, then copy it to all uniform buffers.
    SingleStagingBuffer,
    // Map and copy to a specific staging buffer first for each uniform buffer to then copy from.
    MultipleStagingBuffer,
    // Map uniform buffer directly
    MapWithExtendedUsages,
};

enum class UploadSize {
    Partial,
    Full,
};

enum class UniformBuffer {
    Single,    // Use one same uniform buffer for all draws.
    Multiple,  // Switch uniform buffers between draws.
};

struct UniformBufferUpdateParams : AdapterTestParam {
    UniformBufferUpdateParams(const AdapterTestParam& param,
                              UploadMethod uploadMethod,
                              UploadSize uploadSize,
                              UniformBuffer uniformBuffer)
        : AdapterTestParam(param),
          uploadMethod(uploadMethod),
          uploadSize(uploadSize),
          uniformBuffer(uniformBuffer) {}

    UploadMethod uploadMethod;
    UploadSize uploadSize;
    UniformBuffer uniformBuffer;
};

std::ostream& operator<<(std::ostream& ostream, const UniformBufferUpdateParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);

    switch (param.uploadMethod) {
        case UploadMethod::WriteBuffer:
            ostream << "_WriteBuffer";
            break;
        case UploadMethod::SingleStagingBuffer:
            ostream << "_SingleStagingBuffer";
            break;
        case UploadMethod::MultipleStagingBuffer:
            ostream << "_MultipleStagingBuffer";
            break;
        case UploadMethod::MapWithExtendedUsages:
            ostream << "_MapWithExtendedUsages";
            break;
    }

    switch (param.uploadSize) {
        case UploadSize::Partial:
            ostream << "_PartialSize";
            break;
        case UploadSize::Full:
            ostream << "_FullSize";
            break;
    }

    switch (param.uniformBuffer) {
        case UniformBuffer::Single:
            ostream << "_SingleUniformBuffer";
            break;
        case UniformBuffer::Multiple:
            ostream << "_MultipleUniformBuffer";
            break;
    }

    return ostream;
}

// Test updating a uniform buffer |kNumIterations| times.
class UniformBufferUpdatePerf : public DawnPerfTestWithParams<UniformBufferUpdateParams> {
  public:
    UniformBufferUpdatePerf() : DawnPerfTestWithParams(kNumIterations, 1) {}
    ~UniformBufferUpdatePerf() override = default;

    void SetUp() override;

  private:
    // Data needed for buffer returning.
    struct CallbackData {
        UniformBufferUpdatePerf* self;
        wgpu::Buffer buffer;
    };
    void Step() override;
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;

    size_t GetBufferSize();
    wgpu::Buffer FindOrCreateUniformBuffer();
    void ReturnUniformBuffer(wgpu::Buffer buffer);
    wgpu::Buffer FindOrCreateStagingBuffer();
    void ReturnStagingBuffer(wgpu::Buffer buffer);

    wgpu::Texture mColorAttachmentTexture;
    wgpu::TextureView mColorAttachmentTextureView;
    wgpu::TextureView mDepthStencilAttachment;
    wgpu::Buffer mVertexBuffer;
    wgpu::BindGroupLayout mUniformBindGroupLayout;
    wgpu::RenderPipeline mPipeline;

    // Free uniform buffers to be re-used.
    MutexProtected<std::queue<wgpu::Buffer>> mUniformBuffers;

    wgpu::Buffer mSingleStagingBuffer;
    // Free staging buffers to be re-used. All buffers are mapped already.
    MutexProtected<std::queue<wgpu::Buffer>> mMultipleStagingBuffers;
};

std::vector<wgpu::FeatureName> UniformBufferUpdatePerf::GetRequiredFeatures() {
    std::vector<wgpu::FeatureName> requiredFeatures = DawnPerfTestWithParams::GetRequiredFeatures();
    if (!UsesWire() && GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages &&
        SupportsFeatures({wgpu::FeatureName::BufferMapExtendedUsages})) {
        requiredFeatures.push_back(wgpu::FeatureName::BufferMapExtendedUsages);
    }
    return requiredFeatures;
}

size_t UniformBufferUpdatePerf::GetBufferSize() {
    // The actual data size, and buffer create size should be same for full upload size.
    return GetParam().uploadSize == UploadSize::Full ? kUniformDataSize : kUniformBufferSize;
}

// Try to grab a free uniform buffer. If unavailable, create a new one on-the-fly.
wgpu::Buffer UniformBufferUpdatePerf::FindOrCreateUniformBuffer() {
    if (!mUniformBuffers->empty()) {
        wgpu::Buffer buffer = mUniformBuffers->front();
        mUniformBuffers->pop();
        return buffer;
    }
    wgpu::BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    if (GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages) {
        descriptor.usage |= wgpu::BufferUsage::MapWrite;
        descriptor.mappedAtCreation = true;
    } else {
        descriptor.usage |= wgpu::BufferUsage::CopyDst;
    }

    descriptor.size = GetBufferSize();
    return device.CreateBuffer(&descriptor);
}

// Return a uniform buffer, so that it's free to be re-used.
void UniformBufferUpdatePerf::ReturnUniformBuffer(wgpu::Buffer buffer) {
    mUniformBuffers->push(buffer);
}

// Try to grab a free staging buffer. If unavailable, create a new one on-the-fly.
wgpu::Buffer UniformBufferUpdatePerf::FindOrCreateStagingBuffer() {
    if (!mMultipleStagingBuffers->empty()) {
        wgpu::Buffer buffer = mMultipleStagingBuffers->front();
        mMultipleStagingBuffers->pop();
        return buffer;
    }
    wgpu::BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    descriptor.size = GetBufferSize();
    descriptor.mappedAtCreation = true;
    return device.CreateBuffer(&descriptor);
}

// Return a staging buffer, so that it's free to be re-used.
void UniformBufferUpdatePerf::ReturnStagingBuffer(wgpu::Buffer buffer) {
    mMultipleStagingBuffers->push(buffer);
}

void UniformBufferUpdatePerf::SetUp() {
    DawnPerfTestWithParams<UniformBufferUpdateParams>::SetUp();

    // Skip all tests if the BufferMapExtendedUsages feature is not supported.
    DAWN_TEST_UNSUPPORTED_IF(GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages &&
                             !device.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages));

    // Create the color / depth stencil attachments.
    wgpu::TextureDescriptor descriptor = {};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kTextureSize;
    descriptor.size.height = kTextureSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
#ifdef PIXEL_CHECK
    descriptor.usage |= wgpu::TextureUsage::CopySrc;
#endif
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    mColorAttachmentTexture = device.CreateTexture(&descriptor);
    mColorAttachmentTextureView = mColorAttachmentTexture.CreateView();

    descriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    mDepthStencilAttachment = device.CreateTexture(&descriptor).CreateView();

    // Create the vertex buffer
    mVertexBuffer = utils::CreateBufferFromData(device, kVertexData, sizeof(kVertexData),
                                                wgpu::BufferUsage::Vertex);

    // Create the bind group layout.
    mUniformBindGroupLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, false},
                });

    // Setup the base render pipeline descriptor.
    utils::ComboRenderPipelineDescriptor renderPipelineDesc;
    renderPipelineDesc.vertex.bufferCount = 1;
    renderPipelineDesc.cBuffers[0].arrayStride = 4 * sizeof(float);
    renderPipelineDesc.cBuffers[0].attributeCount = 1;
    renderPipelineDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    renderPipelineDesc.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
    renderPipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    // Create the pipeline layout for the pipeline.
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayouts = &mUniformBindGroupLayout;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    // Create the shaders for the pipeline.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, kVertexShader);
    // Inject kNumIterations into the fragment shader.
    char fragmentShader[sizeof(kFragmentShader) + 16];
    snprintf(fragmentShader, sizeof(fragmentShader), kFragmentShader, kNumIterations);
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader);

    // Create the pipeline.
    renderPipelineDesc.layout = pipelineLayout;
    renderPipelineDesc.vertex.module = vsModule;
    renderPipelineDesc.cFragment.module = fsModule;
    mPipeline = device.CreateRenderPipeline(&renderPipelineDesc);

    std::vector<float> data(kUniformDataSize, 1.0f * (kNumIterations / 2));
    mSingleStagingBuffer = FindOrCreateStagingBuffer();
    memcpy(mSingleStagingBuffer.GetMappedRange(0, data.size()), data.data(), data.size());
    mSingleStagingBuffer.Unmap();

    if (GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages &&
        GetParam().uniformBuffer == UniformBuffer::Single) {
        auto buffer = FindOrCreateUniformBuffer();
        memcpy(buffer.GetMappedRange(0, data.size()), data.data(), data.size());
        buffer.Unmap();
        ReturnUniformBuffer(buffer);
    }
}

void UniformBufferUpdatePerf::Step() {
    for (unsigned int i = 0; i < kNumIterations; ++i) {
        std::vector<float> data(kUniformDataSize, 1.0f * i);
        wgpu::CommandEncoder commands = device.CreateCommandEncoder();
        wgpu::Buffer uniformBuffer = FindOrCreateUniformBuffer();
        wgpu::Buffer stagingBuffer = nullptr;
        switch (GetParam().uploadMethod) {
            case UploadMethod::WriteBuffer:
                queue.WriteBuffer(uniformBuffer, 0, data.data(), data.size());
                break;
            case UploadMethod::SingleStagingBuffer:
                commands.CopyBufferToBuffer(mSingleStagingBuffer, 0, uniformBuffer, 0, data.size());
                break;
            case UploadMethod::MultipleStagingBuffer:
                stagingBuffer = FindOrCreateStagingBuffer();
                memcpy(stagingBuffer.GetMappedRange(0, data.size()), data.data(), data.size());
                stagingBuffer.Unmap();
                commands.CopyBufferToBuffer(stagingBuffer, 0, uniformBuffer, 0, data.size());
                break;
            case UploadMethod::MapWithExtendedUsages:
                if (GetParam().uniformBuffer == UniformBuffer::Multiple) {
                    memcpy(uniformBuffer.GetMappedRange(0, data.size()), data.data(), data.size());
                    uniformBuffer.Unmap();
                }
                break;
        }
        utils::ComboRenderPassDescriptor renderPass({mColorAttachmentTextureView},
                                                    mDepthStencilAttachment);
        wgpu::RenderPassEncoder pass = commands.BeginRenderPass(&renderPass);
        pass.SetPipeline(mPipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mUniformBindGroupLayout,
                                                         {{0, uniformBuffer, 0, GetBufferSize()}});
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commandBuffer = commands.Finish();
        queue.Submit(1, &commandBuffer);

        // Return the staging buffer once it's done with the last usage and re-mapped.
        if (GetParam().uploadMethod == UploadMethod::MultipleStagingBuffer) {
            stagingBuffer.MapAsync(
                wgpu::MapMode::Write, 0, GetBufferSize(), wgpu::CallbackMode::AllowProcessEvents,
                [this, stagingBuffer](wgpu::MapAsyncStatus status, wgpu::StringView) {
                    if (status == wgpu::MapAsyncStatus::Success) {
                        this->ReturnStagingBuffer(stagingBuffer);
                    }
                });
        }

        switch (GetParam().uniformBuffer) {
            case UniformBuffer::Single:
                // Return the uniform buffer immediately so that we always use the same one.
                ReturnUniformBuffer(uniformBuffer);
                break;
            case UniformBuffer::Multiple:
                // Return the uniform buffer once it's done with the last submit.
                if (GetParam().uploadMethod == UploadMethod::MapWithExtendedUsages) {
                    uniformBuffer.MapAsync(
                        wgpu::MapMode::Write, 0, GetBufferSize(),
                        wgpu::CallbackMode::AllowProcessEvents,
                        [this, uniformBuffer](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                this->ReturnUniformBuffer(uniformBuffer);
                            }
                        });
                } else {
                    queue.OnSubmittedWorkDone(
                        wgpu::CallbackMode::AllowProcessEvents,
                        [this, uniformBuffer](wgpu::QueueWorkDoneStatus status) {
                            if (status == wgpu::QueueWorkDoneStatus::Success) {
                                this->ReturnUniformBuffer(uniformBuffer);
                            }
                        });
                }
                break;
        }

#ifdef PIXEL_CHECK
        auto value = i;
        if (GetParam().uploadMethod == UploadMethod::SingleStagingBuffer) {
            value = kNumIterations / 2;
        }
        uint8_t u8 = std::floor(value * 255.0 / kNumIterations);
        utils::RGBA8 color0(u8, u8, u8, 255);
        utils::RGBA8 color1(u8 + 1, u8 + 1, u8 + 1, 255);
        EXPECT_PIXEL_RGBA8_BETWEEN(color0, color1, mColorAttachmentTexture, kTextureSize / 2,
                                   kTextureSize / 2);
#endif
    }
}

TEST_P(UniformBufferUpdatePerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(UniformBufferUpdatePerf,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {UploadMethod::WriteBuffer, UploadMethod::SingleStagingBuffer,
                         UploadMethod::MultipleStagingBuffer, UploadMethod::MapWithExtendedUsages},
                        {UploadSize::Partial, UploadSize::Full},
                        {UniformBuffer::Single, UniformBuffer::Multiple});

}  // anonymous namespace
}  // namespace dawn
