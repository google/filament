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

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 4;
constexpr uint32_t kFloat32x2Stride = 2 * sizeof(float);
constexpr uint32_t kFloat32x4Stride = 4 * sizeof(float);

class DrawVertexAndIndexBufferOOBValidationTests : public ValidationTest {
  public:
    // Parameters for testing index buffer
    struct IndexBufferParams {
        wgpu::IndexFormat indexFormat;
        uint64_t indexBufferSize;              // Size for creating index buffer
        uint64_t indexBufferOffsetForEncoder;  // Offset for SetIndexBuffer in encoder
        uint64_t indexBufferSizeForEncoder;    // Size for SetIndexBuffer in encoder
        uint32_t maxValidIndexNumber;  // max number of {indexCount + firstIndex} for this set
                                       // of parameters
    };

    // Parameters for testing vertex-step-mode and instance-step-mode vertex buffer
    struct VertexBufferParams {
        uint32_t bufferStride;
        uint64_t bufferSize;              // Size for creating vertex buffer
        uint64_t bufferOffsetForEncoder;  // Offset for SetVertexBuffer in encoder
        uint64_t bufferSizeForEncoder;    // Size for SetVertexBuffer in encoder
        uint32_t maxValidAccessNumber;    // max number of valid access time for this set of
                                          // parameters, i.e. {vertexCount + firstVertex} for
        // vertex-step-mode, and {instanceCount + firstInstance}
        // for instance-step-mode
    };

    // Parameters for setIndexBuffer
    struct IndexBufferDesc {
        const wgpu::Buffer buffer;
        wgpu::IndexFormat indexFormat;
        uint64_t offset = 0;
        uint64_t size = wgpu::kWholeSize;
    };

    // Parameters for setVertexBuffer
    struct VertexBufferSpec {
        uint32_t slot;
        const wgpu::Buffer buffer;
        uint64_t offset = 0;
        uint64_t size = wgpu::kWholeSize;
    };
    using VertexBufferList = std::vector<VertexBufferSpec>;

    // Buffer layout parameters for creating pipeline
    struct PipelineVertexBufferAttributeDesc {
        uint32_t shaderLocation;
        wgpu::VertexFormat format;
        uint64_t offset = 0;
    };
    struct PipelineVertexBufferDesc {
        uint64_t arrayStride;
        wgpu::VertexStepMode stepMode;
        std::vector<PipelineVertexBufferAttributeDesc> attributes = {};
    };

    void SetUp() override {
        ValidationTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");
    }

    const wgpu::RenderPassDescriptor* GetBasicRenderPassDescriptor() const {
        return &renderPass.renderPassInfo;
    }

    wgpu::Buffer CreateBuffer(uint64_t size, wgpu::BufferUsage usage = wgpu::BufferUsage::Vertex) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::ShaderModule CreateVertexShaderModuleWithBuffer(
        std::vector<PipelineVertexBufferDesc> bufferDescList) {
        uint32_t attributeCount = 0;
        std::stringstream inputStringStream;

        for (auto buffer : bufferDescList) {
            for (auto attr : buffer.attributes) {
                // @location({shaderLocation}) var_{id} : {typeString},
                inputStringStream << "@location(" << attr.shaderLocation << ") var_"
                                  << attributeCount << " : vec4f,";
                attributeCount++;
            }
        }

        std::stringstream shaderStringStream;

        shaderStringStream << R"(
            @vertex
            fn main()" << inputStringStream.str()
                           << R"() -> @builtin(position) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })";

        return utils::CreateShaderModule(device, shaderStringStream.str().c_str());
    }

    // Create a render pipeline with given buffer layout description, using a vertex shader
    // module automatically generated from the buffer description.
    wgpu::RenderPipeline CreateRenderPipelineWithBufferDesc(
        std::vector<PipelineVertexBufferDesc> bufferDescList) {
        utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = CreateVertexShaderModuleWithBuffer(bufferDescList);
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

        descriptor.vertex.bufferCount = bufferDescList.size();

        size_t attributeCount = 0;

        for (size_t bufferCount = 0; bufferCount < bufferDescList.size(); bufferCount++) {
            auto bufferDesc = bufferDescList[bufferCount];
            descriptor.cBuffers[bufferCount].arrayStride = bufferDesc.arrayStride;
            descriptor.cBuffers[bufferCount].stepMode = bufferDesc.stepMode;
            if (bufferDesc.attributes.size() > 0) {
                descriptor.cBuffers[bufferCount].attributeCount = bufferDesc.attributes.size();
                descriptor.cBuffers[bufferCount].attributes =
                    &descriptor.cAttributes[attributeCount];
                for (auto attribute : bufferDesc.attributes) {
                    descriptor.cAttributes[attributeCount].shaderLocation =
                        attribute.shaderLocation;
                    descriptor.cAttributes[attributeCount].format = attribute.format;
                    descriptor.cAttributes[attributeCount].offset = attribute.offset;
                    attributeCount++;
                }
            } else {
                descriptor.cBuffers[bufferCount].attributeCount = 0;
                descriptor.cBuffers[bufferCount].attributes = nullptr;
            }
        }

        descriptor.cTargets[0].format = renderPass.colorFormat;

        return device.CreateRenderPipeline(&descriptor);
    }

    // Create a render pipeline using only one vertex-step-mode Float32x4 buffer
    wgpu::RenderPipeline CreateBasicRenderPipeline(uint32_t bufferStride = kFloat32x4Stride) {
        DAWN_ASSERT(bufferStride >= kFloat32x4Stride);

        std::vector<PipelineVertexBufferDesc> bufferDescList = {
            {bufferStride, wgpu::VertexStepMode::Vertex, {{0, wgpu::VertexFormat::Float32x4}}},
        };

        return CreateRenderPipelineWithBufferDesc(bufferDescList);
    }

    // Create a render pipeline using one vertex-step-mode Float32x4 buffer and one
    // instance-step-mode Float32x2 buffer
    wgpu::RenderPipeline CreateBasicRenderPipelineWithInstance(
        uint32_t bufferStride1 = kFloat32x4Stride,
        uint32_t bufferStride2 = kFloat32x2Stride) {
        DAWN_ASSERT(bufferStride1 >= kFloat32x4Stride);
        DAWN_ASSERT(bufferStride2 >= kFloat32x2Stride);

        std::vector<PipelineVertexBufferDesc> bufferDescList = {
            {bufferStride1, wgpu::VertexStepMode::Vertex, {{0, wgpu::VertexFormat::Float32x4}}},
            {bufferStride2, wgpu::VertexStepMode::Instance, {{3, wgpu::VertexFormat::Float32x2}}},
        };

        return CreateRenderPipelineWithBufferDesc(bufferDescList);
    }

    // Create a render pipeline using one vertex-step-mode and one instance-step-mode buffer,
    // both with a zero array stride. The minimal size of vertex step mode buffer should be 28,
    // and the minimal size of instance step mode buffer should be 20.
    wgpu::RenderPipeline CreateBasicRenderPipelineWithZeroArrayStride() {
        std::vector<PipelineVertexBufferDesc> bufferDescList = {
            {0,
             wgpu::VertexStepMode::Vertex,
             {{0, wgpu::VertexFormat::Float32x4, 0}, {1, wgpu::VertexFormat::Float32x2, 20}}},
            {0,
             wgpu::VertexStepMode::Instance,
             // Two attributes are overlapped within this instance step mode vertex buffer
             {{3, wgpu::VertexFormat::Float32x4, 4}, {7, wgpu::VertexFormat::Float32x3, 0}}},
        };

        return CreateRenderPipelineWithBufferDesc(bufferDescList);
    }

    void TestRenderPassDraw(const wgpu::RenderPipeline& pipeline,
                            VertexBufferList vertexBufferList,
                            uint32_t vertexCount,
                            uint32_t instanceCount,
                            uint32_t firstVertex,
                            uint32_t firstInstance,
                            bool isSuccess) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            encoder.BeginRenderPass(GetBasicRenderPassDescriptor());
        renderPassEncoder.SetPipeline(pipeline);

        for (auto vertexBufferParam : vertexBufferList) {
            renderPassEncoder.SetVertexBuffer(vertexBufferParam.slot, vertexBufferParam.buffer,
                                              vertexBufferParam.offset, vertexBufferParam.size);
        }
        renderPassEncoder.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
        renderPassEncoder.End();

        if (isSuccess) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    void TestRenderPassDrawIndexed(const wgpu::RenderPipeline& pipeline,
                                   IndexBufferDesc indexBuffer,
                                   VertexBufferList vertexBufferList,
                                   uint32_t indexCount,
                                   uint32_t instanceCount,
                                   uint32_t firstIndex,
                                   int32_t baseVertex,
                                   uint32_t firstInstance,
                                   bool isSuccess) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            encoder.BeginRenderPass(GetBasicRenderPassDescriptor());
        renderPassEncoder.SetPipeline(pipeline);

        renderPassEncoder.SetIndexBuffer(indexBuffer.buffer, indexBuffer.indexFormat,
                                         indexBuffer.offset, indexBuffer.size);

        for (auto vertexBufferParam : vertexBufferList) {
            renderPassEncoder.SetVertexBuffer(vertexBufferParam.slot, vertexBufferParam.buffer,
                                              vertexBufferParam.offset, vertexBufferParam.size);
        }
        renderPassEncoder.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex,
                                      firstInstance);
        renderPassEncoder.End();

        if (isSuccess) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    // Parameters list for index buffer. Should cover all IndexFormat, and the zero/non-zero
    // offset and size case in SetIndexBuffer
    const std::vector<IndexBufferParams> kIndexParamsList = {
        {wgpu::IndexFormat::Uint32, 12 * sizeof(uint32_t), 0, wgpu::kWholeSize, 12},
        {wgpu::IndexFormat::Uint32, 13 * sizeof(uint32_t), sizeof(uint32_t), wgpu::kWholeSize, 12},
        {wgpu::IndexFormat::Uint32, 13 * sizeof(uint32_t), 0, 12 * sizeof(uint32_t), 12},
        {wgpu::IndexFormat::Uint32, 14 * sizeof(uint32_t), sizeof(uint32_t), 12 * sizeof(uint32_t),
         12},

        {wgpu::IndexFormat::Uint16, 12 * sizeof(uint16_t), 0, wgpu::kWholeSize, 12},
        {wgpu::IndexFormat::Uint16, 13 * sizeof(uint16_t), sizeof(uint16_t), wgpu::kWholeSize, 12},
        {wgpu::IndexFormat::Uint16, 13 * sizeof(uint16_t), 0, 12 * sizeof(uint16_t), 12},
        {wgpu::IndexFormat::Uint16, 14 * sizeof(uint16_t), sizeof(uint16_t), 12 * sizeof(uint16_t),
         12},
    };
    // Parameters list for vertex-step-mode buffer. These parameters should cover different
    // stride, buffer size, SetVertexBuffer size and offset.
    const std::vector<VertexBufferParams> kVertexParamsList = {
        // For stride = kFloat32x4Stride
        {kFloat32x4Stride, 3 * kFloat32x4Stride, 0, wgpu::kWholeSize, 3},
        // Non-zero offset
        {kFloat32x4Stride, 4 * kFloat32x4Stride, kFloat32x4Stride, wgpu::kWholeSize, 3},
        // Non-default size
        {kFloat32x4Stride, 4 * kFloat32x4Stride, 0, 3 * kFloat32x4Stride, 3},
        // Non-zero offset and size
        {kFloat32x4Stride, 5 * kFloat32x4Stride, kFloat32x4Stride, 3 * kFloat32x4Stride, 3},
        // For stride = 2 * kFloat32x4Stride
        {(2 * kFloat32x4Stride), 3 * (2 * kFloat32x4Stride), 0, wgpu::kWholeSize, 3},
        // Non-zero offset
        {(2 * kFloat32x4Stride), 4 * (2 * kFloat32x4Stride), (2 * kFloat32x4Stride),
         wgpu::kWholeSize, 3},
        // Non-default size
        {(2 * kFloat32x4Stride), 4 * (2 * kFloat32x4Stride), 0, 3 * (2 * kFloat32x4Stride), 3},
        // Non-zero offset and size
        {(2 * kFloat32x4Stride), 5 * (2 * kFloat32x4Stride), (2 * kFloat32x4Stride),
         3 * (2 * kFloat32x4Stride), 3},
        // For (strideCount - 1) * arrayStride + lastStride <= bound buffer size < strideCount *
        // arrayStride
        {(kFloat32x4Stride + 4), 2 * (kFloat32x4Stride + 4) + kFloat32x4Stride, 0, wgpu::kWholeSize,
         3},
        {(kFloat32x4Stride + 4), 2 * (kFloat32x4Stride + 4) + kFloat32x4Stride - 1, 0,
         wgpu::kWholeSize, 2},
    };
    // Parameters list for instance-step-mode buffer.
    const std::vector<VertexBufferParams> kInstanceParamsList = {
        // For stride = kFloat32x2Stride
        {kFloat32x2Stride, 5 * kFloat32x2Stride, 0, wgpu::kWholeSize, 5},
        // Non-zero offset
        {kFloat32x2Stride, 6 * kFloat32x2Stride, kFloat32x2Stride, wgpu::kWholeSize, 5},
        // Non-default size
        {kFloat32x2Stride, 6 * kFloat32x2Stride, 0, 5 * kFloat32x2Stride, 5},
        // Non-zero offset and size
        {kFloat32x2Stride, 7 * kFloat32x2Stride, kFloat32x2Stride, 5 * kFloat32x2Stride, 5},
        // For stride = 3 * kFloat32x2Stride
        {(3 * kFloat32x2Stride), 5 * (3 * kFloat32x2Stride), 0, wgpu::kWholeSize, 5},
        // Non-zero offset
        {(3 * kFloat32x2Stride), 6 * (3 * kFloat32x2Stride), (3 * kFloat32x2Stride),
         wgpu::kWholeSize, 5},
        // Non-default size
        {(3 * kFloat32x2Stride), 6 * (3 * kFloat32x2Stride), 0, 5 * (3 * kFloat32x2Stride), 5},
        // Non-zero offset and size
        {(3 * kFloat32x2Stride), 7 * (3 * kFloat32x2Stride), (3 * kFloat32x2Stride),
         5 * (3 * kFloat32x2Stride), 5},
        // For (strideCount - 1) * arrayStride + lastStride <= bound buffer size < strideCount *
        // arrayStride
        {(kFloat32x2Stride + 4), 2 * (kFloat32x2Stride + 4) + kFloat32x2Stride, 0, wgpu::kWholeSize,
         3},
        {(kFloat32x2Stride + 4), 2 * (kFloat32x2Stride + 4) + kFloat32x2Stride - 1, 0,
         wgpu::kWholeSize, 2},
    };

  private:
    wgpu::ShaderModule fsModule;
    utils::BasicRenderPass renderPass;
};

// Control case for Draw
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawBasic) {
    wgpu::RenderPipeline pipeline = CreateBasicRenderPipeline();

    wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);

    {
        // Implicit size
        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, wgpu::kWholeSize}};
        TestRenderPassDraw(pipeline, vertexBufferList, /* vertexCount */ 3,
                           /* instanceCount */ 1, /* firstVertex */ 0,
                           /* firstInstance */ 0, /* isSuccess */ true);
    }

    {
        // Explicit zero size
        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, 0}};
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 1, 0, 0, false);
    }
}

// Verify vertex buffer OOB for non-instanced Draw are caught in command encoder
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawVertexBufferOutOfBoundWithoutInstance) {
    for (VertexBufferParams params : kVertexParamsList) {
        // Create a render pipeline without instance step mode buffer
        wgpu::RenderPipeline pipeline = CreateBasicRenderPipeline(params.bufferStride);

        // Build vertex buffer for 3 vertices
        wgpu::Buffer vertexBuffer = CreateBuffer(params.bufferSize);
        VertexBufferList vertexBufferList = {
            {0, vertexBuffer, params.bufferOffsetForEncoder, params.bufferSizeForEncoder}};

        DAWN_ASSERT(params.maxValidAccessNumber > 0);
        uint32_t n = params.maxValidAccessNumber;
        // It is ok to draw n vertices with vertex buffer
        TestRenderPassDraw(pipeline, vertexBufferList, /* vertexCount */ n,
                           /* instanceCount */ 1, /* firstVertex */ 0,
                           /* firstInstance */ 0, /* isSuccess */ true);
        // It is ok to draw n-1 vertices with offset 1
        TestRenderPassDraw(pipeline, vertexBufferList, n - 1, 1, 1, 0, true);
        // Drawing more vertices will cause OOB, even if not enough for another primitive
        TestRenderPassDraw(pipeline, vertexBufferList, n + 1, 1, 0, 0, false);
        // Drawing n vertices will non-zero offset will cause OOB
        TestRenderPassDraw(pipeline, vertexBufferList, n, 1, 1, 0, false);
        // It is ok to draw any number of instances, as we have no instance-mode buffer
        TestRenderPassDraw(pipeline, vertexBufferList, n, 5, 0, 0, true);
        TestRenderPassDraw(pipeline, vertexBufferList, n, 5, 0, 5, true);
    }
}

// Verify vertex buffer OOB for instanced Draw are caught in command encoder
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawVertexBufferOutOfBoundWithInstance) {
    for (VertexBufferParams vertexParams : kVertexParamsList) {
        for (VertexBufferParams instanceParams : kInstanceParamsList) {
            // Create pipeline with given buffer stride
            wgpu::RenderPipeline pipeline = CreateBasicRenderPipelineWithInstance(
                vertexParams.bufferStride, instanceParams.bufferStride);

            // Build vertex buffer
            wgpu::Buffer vertexBuffer = CreateBuffer(vertexParams.bufferSize);
            wgpu::Buffer instanceBuffer = CreateBuffer(instanceParams.bufferSize);

            VertexBufferList vertexBufferList = {
                {0, vertexBuffer, vertexParams.bufferOffsetForEncoder,
                 vertexParams.bufferSizeForEncoder},
                {1, instanceBuffer, instanceParams.bufferOffsetForEncoder,
                 instanceParams.bufferSizeForEncoder},
            };

            DAWN_ASSERT(vertexParams.maxValidAccessNumber > 0);
            DAWN_ASSERT(instanceParams.maxValidAccessNumber > 0);
            uint32_t vert = vertexParams.maxValidAccessNumber;
            uint32_t inst = instanceParams.maxValidAccessNumber;
            // It is ok to draw vert vertices
            TestRenderPassDraw(pipeline, vertexBufferList, /* vertexCount */ vert,
                               /* instanceCount */ 1, /* firstVertex */ 0,
                               /* firstInstance */ 0, /* isSuccess */ true);
            TestRenderPassDraw(pipeline, vertexBufferList, vert - 1, 1, 1, 0, true);
            // It is ok to draw vert vertices and inst instences
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst, 0, 0, true);
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst - 1, 0, 1, true);
            // more vertices causing OOB
            TestRenderPassDraw(pipeline, vertexBufferList, vert + 1, 1, 0, 0, false);
            TestRenderPassDraw(pipeline, vertexBufferList, vert, 1, 1, 0, false);
            TestRenderPassDraw(pipeline, vertexBufferList, vert + 1, inst, 0, 0, false);
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst, 1, 0, false);
            // more instances causing OOB
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst + 1, 0, 0, false);
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst, 0, 1, false);
            // Both OOB
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst + 1, 0, 0, false);
            TestRenderPassDraw(pipeline, vertexBufferList, vert, inst, 1, 1, false);
        }
    }
}

// Verify zero-attribute vertex buffer OOB for non-instanced Draw are caught in command encoder
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, ZeroAttribute) {
    // Create a render pipeline with zero-attribute vertex buffer description
    wgpu::RenderPipeline pipeline =
        CreateRenderPipelineWithBufferDesc({{kFloat32x4Stride, wgpu::VertexStepMode::Vertex, {}}});

    wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);

    {
        // Valid
        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, wgpu::kWholeSize}};
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 1, 0, 0, true);
    }

    {
        // OOB
        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, 0}};
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 1, 0, 0, false);
    }
}

// Verify vertex buffers don't need to be set to unused (hole) slots for Draw
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, UnusedSlots) {
    // Set vertex buffer only to the second slot
    wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);
    VertexBufferList vertexBufferList = {{1, vertexBuffer, 0, wgpu::kWholeSize}};

    {
        // The first slot it unused so valid even if vertex buffer is not set to it.
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithBufferDesc(
            {{0, wgpu::VertexStepMode::Undefined, {}},
             {kFloat32x4Stride, wgpu::VertexStepMode::Vertex, {}}});
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 1, 0, 0, true);
    }

    {
        // The first slot it used so invalid if vertex buffer is not set to it.
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithBufferDesc(
            {{0, wgpu::VertexStepMode::Vertex, {}},
             {kFloat32x4Stride, wgpu::VertexStepMode::Vertex, {}}});
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 1, 0, 0, false);
    }
}

// Control case for DrawIndexed
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawIndexedBasic) {
    wgpu::RenderPipeline pipeline = CreateBasicRenderPipeline();

    // Build index buffer for 12 indexes
    wgpu::Buffer indexBuffer = CreateBuffer(12 * sizeof(uint32_t), wgpu::BufferUsage::Index);

    // Build vertex buffer for 3 vertices
    wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);
    VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, wgpu::kWholeSize}};

    IndexBufferDesc indexBufferDesc = {indexBuffer, wgpu::IndexFormat::Uint32};

    TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, /* indexCount */ 12,
                              /* instanceCount */ 1, /* firstIndex */ 0, /* baseVertex */ 0,
                              /* firstInstance */ 0, /* isSuccess */ true);
}

// Verify index buffer OOB for DrawIndexed are caught in command encoder
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawIndexedIndexBufferOOB) {
    wgpu::RenderPipeline pipeline = CreateBasicRenderPipelineWithInstance();

    for (IndexBufferParams params : kIndexParamsList) {
        // Build index buffer use given params
        wgpu::Buffer indexBuffer = CreateBuffer(params.indexBufferSize, wgpu::BufferUsage::Index);
        // Build vertex buffer for 3 vertices
        wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);
        // Build vertex buffer for 5 instances
        wgpu::Buffer instanceBuffer = CreateBuffer(5 * kFloat32x2Stride);

        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, wgpu::kWholeSize},
                                             {1, instanceBuffer, 0, wgpu::kWholeSize}};

        IndexBufferDesc indexBufferDesc = {indexBuffer, params.indexFormat,
                                           params.indexBufferOffsetForEncoder,
                                           params.indexBufferSizeForEncoder};

        DAWN_ASSERT(params.maxValidIndexNumber > 0);
        uint32_t n = params.maxValidIndexNumber;

        // Control case
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, /* indexCount */ n,
                                  /* instanceCount */ 5, /* firstIndex */ 0, /* baseVertex */ 0,
                                  /* firstInstance */ 0, /* isSuccess */ true);
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, n - 1, 5, 1, 0, 0,
                                  true);
        // Index buffer OOB, indexCount too large
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, n + 1, 5, 0, 0, 0,
                                  false);
        // Index buffer OOB, indexCount + firstIndex too large
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, n, 5, 1, 0, 0,
                                  false);

        if (!HasToggleEnabled("disable_base_vertex")) {
            // baseVertex is not considered in CPU validation and has no effect on validation
            // Although baseVertex is too large, it will still pass
            TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, n, 5, 0, 100, 0,
                                      true);
            // Index buffer OOB, indexCount too large
            TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, n + 1, 5, 0, 100,
                                      0, false);
        }
    }
}

// Verify instance mode vertex buffer OOB for DrawIndexed are caught in command encoder
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawIndexedVertexBufferOOB) {
    for (VertexBufferParams vertexParams : kVertexParamsList) {
        for (VertexBufferParams instanceParams : kInstanceParamsList) {
            // Create pipeline with given buffer stride
            wgpu::RenderPipeline pipeline = CreateBasicRenderPipelineWithInstance(
                vertexParams.bufferStride, instanceParams.bufferStride);

            constexpr wgpu::IndexFormat indexFormat = wgpu::IndexFormat::Uint32;
            constexpr uint64_t indexStride = sizeof(uint32_t);

            // Build index buffer for 12 indexes
            wgpu::Buffer indexBuffer = CreateBuffer(12 * indexStride, wgpu::BufferUsage::Index);
            // Build vertex buffer for vertices
            wgpu::Buffer vertexBuffer = CreateBuffer(vertexParams.bufferSize);
            // Build vertex buffer for instances
            wgpu::Buffer instanceBuffer = CreateBuffer(instanceParams.bufferSize);

            VertexBufferList vertexBufferList = {
                {0, vertexBuffer, vertexParams.bufferOffsetForEncoder,
                 vertexParams.bufferSizeForEncoder},
                {1, instanceBuffer, instanceParams.bufferOffsetForEncoder,
                 instanceParams.bufferSizeForEncoder}};

            IndexBufferDesc indexBufferDesc = {indexBuffer, indexFormat};

            DAWN_ASSERT(instanceParams.maxValidAccessNumber > 0);
            uint32_t inst = instanceParams.maxValidAccessNumber;
            // Control case
            TestRenderPassDrawIndexed(
                pipeline, indexBufferDesc, vertexBufferList, /* indexCount */ 12,
                /* instanceCount */ inst, /* firstIndex */ 0, /* baseVertex */ 0,
                /* firstInstance */ 0, /* isSuccess */ true);
            // Vertex buffer (stepMode = instance) OOB, instanceCount too large
            TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, inst + 1, 0,
                                      0, 0, false);

            if (!HasToggleEnabled("disable_base_instance")) {
                // firstInstance is considered in CPU validation
                // Vertex buffer (stepMode = instance) in bound
                TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, inst - 1,
                                          0, 0, 1, true);
                // Vertex buffer (stepMode = instance) OOB, instanceCount + firstInstance too
                // large
                TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, inst, 0,
                                          0, 1, false);
            }
        }
    }
}

// Verify zero-attribute instance step mode vertex buffer OOB for instanced DrawIndex
// are caught in command encoder
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawIndexedZeroAttribute) {
    // Create a render pipeline with a vertex step mode and a zero-attribute
    // instance step mode vertex buffer description
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithBufferDesc(
        {{kFloat32x4Stride, wgpu::VertexStepMode::Vertex, {{0, wgpu::VertexFormat::Float32x4}}},
         {kFloat32x2Stride, wgpu::VertexStepMode::Instance, {}}});

    // Build index buffer for 12 indexes
    wgpu::Buffer indexBuffer = CreateBuffer(12 * sizeof(uint32_t), wgpu::BufferUsage::Index);
    IndexBufferDesc indexBufferDesc = {indexBuffer, wgpu::IndexFormat::Uint32};

    // Build vertex buffer for 3 vertices
    wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);

    // Build vertex buffer for 3 instances
    wgpu::Buffer instanceBuffer = CreateBuffer(3 * kFloat32x2Stride);

    {
        // Valid
        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, wgpu::kWholeSize},
                                             {1, vertexBuffer, 0, wgpu::kWholeSize}};
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, 3, 0, 0, 0,
                                  true);
    }

    {
        // OOB
        VertexBufferList vertexBufferList = {{0, vertexBuffer, 0, wgpu::kWholeSize},
                                             {1, vertexBuffer, 0, 0}};
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, 3, 0, 0, 0,
                                  false);
    }
}

// Verify vertex buffers don't need to be set to unused (hole) slots for DrawIndexed
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, DrawIndexedUnusedSlots) {
    // Build index buffer
    wgpu::Buffer indexBuffer = CreateBuffer(12 * sizeof(uint32_t), wgpu::BufferUsage::Index);
    IndexBufferDesc indexBufferDesc = {indexBuffer, wgpu::IndexFormat::Uint32};

    // Set vertex buffers only to the second and third slots
    wgpu::Buffer vertexBuffer = CreateBuffer(3 * kFloat32x4Stride);
    wgpu::Buffer instanceBuffer = CreateBuffer(3 * kFloat32x2Stride);
    VertexBufferList vertexBufferList = {{1, vertexBuffer, 0, wgpu::kWholeSize},
                                         {2, instanceBuffer, 0, wgpu::kWholeSize}};

    {
        // The first slot it unused so valid even if vertex buffer is not set to it.
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithBufferDesc(
            {{0, wgpu::VertexStepMode::Undefined, {}},
             {kFloat32x4Stride, wgpu::VertexStepMode::Vertex, {}},
             {kFloat32x2Stride, wgpu::VertexStepMode::Instance, {}}});
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, 3, 0, 0, 0,
                                  true);
    }

    {
        // The first slot it used so invalid if vertex buffer is not set to it.
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithBufferDesc(
            {{0, wgpu::VertexStepMode::Vertex, {}},
             {kFloat32x4Stride, wgpu::VertexStepMode::Vertex, {}},
             {kFloat32x2Stride, wgpu::VertexStepMode::Instance, {}}});
        TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, 3, 0, 0, 0,
                                  false);
    }
}

// Verify zero array stride vertex buffer OOB for Draw and DrawIndexed are caught in command encoder
// This test only test cases that strideCount > 0. Cases of strideCount == 0 are tested in
// ZeroStrideCountVertexBufferNeverOOB.
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, ZeroArrayStrideVertexBuffer) {
    // In this test, we use VertexBufferParams.maxValidAccessNumber > 0 to indicate that such
    // buffer parameter meet the requirement of pipeline, and maxValidAccessNumber == 0 to
    // indicate that such buffer parameter will cause OOB.
    const std::vector<VertexBufferParams> kVertexParamsListForZeroStride = {
        // Control case
        {0, 28, 0, wgpu::kWholeSize, 1},
        // Non-zero offset
        {0, 28, 4, wgpu::kWholeSize, 0},
        {0, 28, 28, wgpu::kWholeSize, 0},
        // Non-default size
        {0, 28, 0, 28, 1},
        {0, 28, 0, 27, 0},
        // Non-zero offset and size
        {0, 32, 4, 28, 1},
        {0, 31, 4, 27, 0},
        {0, 31, 4, wgpu::kWholeSize, 0},
    };

    const std::vector<VertexBufferParams> kInstanceParamsListForZeroStride = {
        // Control case
        {0, 20, 0, wgpu::kWholeSize, 1},
        // Non-zero offset
        {0, 24, 4, wgpu::kWholeSize, 1},
        {0, 23, 4, wgpu::kWholeSize, 0},
        {0, 20, 4, wgpu::kWholeSize, 0},
        {0, 20, 20, wgpu::kWholeSize, 0},
        // Non-default size
        {0, 21, 0, 20, 1},
        {0, 20, 0, 19, 0},
        // Non-zero offset and size
        {0, 30, 4, 20, 1},
        {0, 30, 4, 19, 0},
    };

    // Build a pipeline that require a vertex step mode vertex buffer no smaller than 28 bytes
    // and an instance step mode buffer no smaller than 20 bytes
    wgpu::RenderPipeline pipeline = CreateBasicRenderPipelineWithZeroArrayStride();

    for (VertexBufferParams vertexParams : kVertexParamsListForZeroStride) {
        for (VertexBufferParams instanceParams : kInstanceParamsListForZeroStride) {
            constexpr wgpu::IndexFormat indexFormat = wgpu::IndexFormat::Uint32;
            constexpr uint64_t indexStride = sizeof(uint32_t);

            // Build index buffer for 12 indexes
            wgpu::Buffer indexBuffer = CreateBuffer(12 * indexStride, wgpu::BufferUsage::Index);
            // Build vertex buffer for vertices
            wgpu::Buffer vertexBuffer = CreateBuffer(vertexParams.bufferSize);
            // Build vertex buffer for instances
            wgpu::Buffer instanceBuffer = CreateBuffer(instanceParams.bufferSize);

            VertexBufferList vertexBufferList = {
                {0, vertexBuffer, vertexParams.bufferOffsetForEncoder,
                 vertexParams.bufferSizeForEncoder},
                {1, instanceBuffer, instanceParams.bufferOffsetForEncoder,
                 instanceParams.bufferSizeForEncoder}};

            IndexBufferDesc indexBufferDesc = {indexBuffer, indexFormat};

            const bool vertexModeBufferOOB = vertexParams.maxValidAccessNumber == 0;
            const bool instanceModeBufferOOB = instanceParams.maxValidAccessNumber == 0;

            // Draw validate both vertex and instance step mode buffer OOB.
            // vertexCount and instanceCount doesn't matter, as array stride is zero and all
            // vertex/instance access the same space of buffer, as long as both (vertexCount +
            // firstVertex) and (instanceCount + firstInstance) are larger than zero.
            TestRenderPassDraw(pipeline, vertexBufferList, /* vertexCount */ 100,
                               /* instanceCount */ 100, /* firstVertex */ 0,
                               /* firstInstance */ 0,
                               /* isSuccess */ !vertexModeBufferOOB && !instanceModeBufferOOB);
            TestRenderPassDraw(pipeline, vertexBufferList, 100, 100, 100, 100,
                               !vertexModeBufferOOB && !instanceModeBufferOOB);
            TestRenderPassDraw(pipeline, vertexBufferList, 0, 0, 100, 100,
                               !vertexModeBufferOOB && !instanceModeBufferOOB);

            // DrawIndexed only validate instance step mode buffer OOB.
            // indexCount doesn't matter as long as no index buffer OOB happened, and instanceCount
            // doesn't matter as long as (instanceCount + firstInstance) are larger than zero.
            TestRenderPassDrawIndexed(
                pipeline, indexBufferDesc, vertexBufferList, /* indexCount */ 12,
                /* instanceCount */ 100, /* firstIndex */ 0, /* baseVertex */ 0,
                /* firstInstance */ 0, /* isSuccess */ !instanceModeBufferOOB);
            TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 12, 0, 0, 0, 100,
                                      !instanceModeBufferOOB);
        }
    }
}

// Verify all vertex buffer never OOB for Draw and DrawIndexed with zero stride count
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, ZeroStrideCountVertexBufferNeverOOB) {
    // In this test we use a render pipeline with non-zero array stride and a render pipeline with
    // zero array stride, both use a vertex step mode vertex buffer with lastStride = 16 and a
    // instance step mode vertex buffer with lastStride = 8. Binding a buffer with size less than
    // lastStride to corresponding buffer slot will result in a OOB validation error is strideCount
    // is larger than 0, but shall not result in validation error if strideCount == 0.

    // Create the render pipeline with non-zero array stride (larger than lastStride)
    std::vector<PipelineVertexBufferDesc> bufferDescListNonZeroArrayStride = {
        {20u, wgpu::VertexStepMode::Vertex, {{0, wgpu::VertexFormat::Float32x4}}},
        {12u, wgpu::VertexStepMode::Instance, {{3, wgpu::VertexFormat::Float32x2}}},
    };
    wgpu::RenderPipeline pipelineWithNonZeroArrayStride =
        CreateRenderPipelineWithBufferDesc(bufferDescListNonZeroArrayStride);

    // Create the render pipeline with zero array stride
    std::vector<PipelineVertexBufferDesc> bufferDescListZeroArrayStride = {
        {0u, wgpu::VertexStepMode::Vertex, {{0, wgpu::VertexFormat::Float32x4}}},
        {0u, wgpu::VertexStepMode::Instance, {{3, wgpu::VertexFormat::Float32x2}}},
    };
    wgpu::RenderPipeline pipelineWithZeroArrayStride =
        CreateRenderPipelineWithBufferDesc(bufferDescListZeroArrayStride);

    const std::vector<VertexBufferParams> kVertexParamsListForZeroStride = {
        // Size enough for 1 vertex
        {kFloat32x4Stride, 16, 0, wgpu::kWholeSize, 1},
        // No enough size for 1 vertex
        {kFloat32x4Stride, 19, 4, wgpu::kWholeSize, 0},
        {kFloat32x4Stride, 16, 16, wgpu::kWholeSize, 0},
    };

    const std::vector<VertexBufferParams> kInstanceParamsListForZeroStride = {
        // Size enough for 1 instance
        {kFloat32x2Stride, 8, 0, wgpu::kWholeSize, 1},
        // No enough size for 1 instance
        {kFloat32x2Stride, 11, 4, wgpu::kWholeSize, 0},
        {kFloat32x2Stride, 8, 8, wgpu::kWholeSize, 0},
    };

    for (VertexBufferParams vertexParams : kVertexParamsListForZeroStride) {
        for (VertexBufferParams instanceParams : kInstanceParamsListForZeroStride) {
            for (wgpu::RenderPipeline pipeline :
                 {pipelineWithNonZeroArrayStride, pipelineWithZeroArrayStride}) {
                constexpr wgpu::IndexFormat indexFormat = wgpu::IndexFormat::Uint32;
                constexpr uint64_t indexStride = sizeof(uint32_t);

                // Build index buffer for 1 index
                wgpu::Buffer indexBuffer = CreateBuffer(indexStride, wgpu::BufferUsage::Index);
                // Build vertex buffer for vertices
                wgpu::Buffer vertexBuffer = CreateBuffer(vertexParams.bufferSize);
                // Build vertex buffer for instances
                wgpu::Buffer instanceBuffer = CreateBuffer(instanceParams.bufferSize);

                VertexBufferList vertexBufferList = {
                    {0, vertexBuffer, vertexParams.bufferOffsetForEncoder,
                     vertexParams.bufferSizeForEncoder},
                    {1, instanceBuffer, instanceParams.bufferOffsetForEncoder,
                     instanceParams.bufferSizeForEncoder}};

                IndexBufferDesc indexBufferDesc = {indexBuffer, indexFormat};

                const bool vertexModeBufferOOB = vertexParams.maxValidAccessNumber == 0;
                const bool instanceModeBufferOOB = instanceParams.maxValidAccessNumber == 0;

                // Draw validate both vertex and instance step mode buffer OOB.
                // Control case, non-zero stride for both step mode buffer
                TestRenderPassDraw(pipeline, vertexBufferList, /* vertexCount */ 1,
                                   /* instanceCount */ 1, /* firstVertex */ 0,
                                   /* firstInstance */ 0,
                                   /* isSuccess */ !vertexModeBufferOOB && !instanceModeBufferOOB);
                TestRenderPassDraw(pipeline, vertexBufferList, 1, 0, 0, 1,
                                   !vertexModeBufferOOB && !instanceModeBufferOOB);
                TestRenderPassDraw(pipeline, vertexBufferList, 0, 1, 1, 0,
                                   !vertexModeBufferOOB && !instanceModeBufferOOB);
                TestRenderPassDraw(pipeline, vertexBufferList, 0, 0, 1, 1,
                                   !vertexModeBufferOOB && !instanceModeBufferOOB);
                // Vertex step mode buffer will never OOB if (vertexCount + firstVertex) is zero,
                // and only instance step mode buffer OOB will fail validation
                TestRenderPassDraw(pipeline, vertexBufferList, 0, 1, 0, 0, !instanceModeBufferOOB);
                TestRenderPassDraw(pipeline, vertexBufferList, 0, 0, 0, 1, !instanceModeBufferOOB);
                // Instance step mode buffer will never OOB if (instanceCount + firstInstance) is
                // zero, and only vertex step mode buffer OOB will fail validation
                TestRenderPassDraw(pipeline, vertexBufferList, 1, 0, 0, 0, !vertexModeBufferOOB);
                TestRenderPassDraw(pipeline, vertexBufferList, 0, 0, 1, 0, !vertexModeBufferOOB);
                // If both (vertexCount + firstVertex) and (instanceCount + firstInstance) are zero,
                // all vertex buffer will never OOB
                TestRenderPassDraw(pipeline, vertexBufferList, 0, 0, 0, 0, true);

                // DrawIndexed only validate instance step mode buffer OOB.
                // Control case, non-zero stride for instance step mode buffer
                TestRenderPassDrawIndexed(
                    pipeline, indexBufferDesc, vertexBufferList, /* indexCount */ 1,
                    /* instanceCount */ 1, /* firstIndex */ 0, /* baseVertex */ 0,
                    /* firstInstance */ 0, /* isSuccess */ !instanceModeBufferOOB);
                TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 1, 0, 0, 0,
                                          1, !instanceModeBufferOOB);
                // Instance step mode buffer will never OOB if (instanceCount + firstInstance) is
                // zero, validation shall always succeed
                TestRenderPassDrawIndexed(pipeline, indexBufferDesc, vertexBufferList, 1, 0, 0, 0,
                                          0, true);
            }
        }
    }
}

// Verify that if setVertexBuffer and/or setIndexBuffer for multiple times, only the last one is
// taken into account
TEST_F(DrawVertexAndIndexBufferOOBValidationTests, SetBufferMultipleTime) {
    wgpu::IndexFormat indexFormat = wgpu::IndexFormat::Uint32;
    uint32_t indexStride = sizeof(uint32_t);

    // Build index buffer for 11 indexes
    wgpu::Buffer indexBuffer11 = CreateBuffer(11 * indexStride, wgpu::BufferUsage::Index);
    // Build index buffer for 12 indexes
    wgpu::Buffer indexBuffer12 = CreateBuffer(12 * indexStride, wgpu::BufferUsage::Index);
    // Build vertex buffer for 2 vertices
    wgpu::Buffer vertexBuffer2 = CreateBuffer(2 * kFloat32x4Stride);
    // Build vertex buffer for 3 vertices
    wgpu::Buffer vertexBuffer3 = CreateBuffer(3 * kFloat32x4Stride);
    // Build vertex buffer for 4 instances
    wgpu::Buffer instanceBuffer4 = CreateBuffer(4 * kFloat32x2Stride);
    // Build vertex buffer for 5 instances
    wgpu::Buffer instanceBuffer5 = CreateBuffer(5 * kFloat32x2Stride);

    // Test for setting vertex buffer for multiple times
    {
        wgpu::RenderPipeline pipeline = CreateBasicRenderPipelineWithInstance();

        // Set to vertexBuffer3 and instanceBuffer5 at last
        VertexBufferList vertexBufferList = {{0, vertexBuffer2, 0, wgpu::kWholeSize},
                                             {1, instanceBuffer4, 0, wgpu::kWholeSize},
                                             {1, instanceBuffer5, 0, wgpu::kWholeSize},
                                             {0, vertexBuffer3, 0, wgpu::kWholeSize}};

        // For Draw, the max vertexCount is 3 and the max instanceCount is 5
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 5, 0, 0, true);
        TestRenderPassDraw(pipeline, vertexBufferList, 4, 5, 0, 0, false);
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 6, 0, 0, false);
        // For DrawIndex, the max instanceCount is 5
        TestRenderPassDrawIndexed(pipeline, {indexBuffer12, indexFormat}, vertexBufferList, 12, 5,
                                  0, 0, 0, true);
        TestRenderPassDrawIndexed(pipeline, {indexBuffer12, indexFormat}, vertexBufferList, 12, 6,
                                  0, 0, 0, false);

        // Set to vertexBuffer2 and instanceBuffer4 at last
        vertexBufferList = VertexBufferList{{0, vertexBuffer3, 0, wgpu::kWholeSize},
                                            {1, instanceBuffer5, 0, wgpu::kWholeSize},
                                            {0, vertexBuffer2, 0, wgpu::kWholeSize},
                                            {1, instanceBuffer4, 0, wgpu::kWholeSize}};

        // For Draw, the max vertexCount is 2 and the max instanceCount is 4
        TestRenderPassDraw(pipeline, vertexBufferList, 2, 4, 0, 0, true);
        TestRenderPassDraw(pipeline, vertexBufferList, 3, 4, 0, 0, false);
        TestRenderPassDraw(pipeline, vertexBufferList, 2, 5, 0, 0, false);
        // For DrawIndex, the max instanceCount is 4
        TestRenderPassDrawIndexed(pipeline, {indexBuffer12, indexFormat}, vertexBufferList, 12, 4,
                                  0, 0, 0, true);
        TestRenderPassDrawIndexed(pipeline, {indexBuffer12, indexFormat}, vertexBufferList, 12, 5,
                                  0, 0, 0, false);
    }

    // Test for setIndexBuffer multiple times
    {
        wgpu::RenderPipeline pipeline = CreateBasicRenderPipeline();

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(GetBasicRenderPassDescriptor());
            renderPassEncoder.SetPipeline(pipeline);

            // Index buffer is set to indexBuffer12 at last
            renderPassEncoder.SetIndexBuffer(indexBuffer11, indexFormat);
            renderPassEncoder.SetIndexBuffer(indexBuffer12, indexFormat);

            renderPassEncoder.SetVertexBuffer(0, vertexBuffer3);
            // It should be ok to draw 12 index
            renderPassEncoder.DrawIndexed(12, 1, 0, 0, 0);
            renderPassEncoder.End();

            // Expect success
            encoder.Finish();
        }

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(GetBasicRenderPassDescriptor());
            renderPassEncoder.SetPipeline(pipeline);

            // Index buffer is set to indexBuffer12 at last
            renderPassEncoder.SetIndexBuffer(indexBuffer11, indexFormat);
            renderPassEncoder.SetIndexBuffer(indexBuffer12, indexFormat);

            renderPassEncoder.SetVertexBuffer(0, vertexBuffer3);
            // It should be index buffer OOB to draw 13 index
            renderPassEncoder.DrawIndexed(13, 1, 0, 0, 0);
            renderPassEncoder.End();

            // Expect failure
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(GetBasicRenderPassDescriptor());
            renderPassEncoder.SetPipeline(pipeline);

            // Index buffer is set to indexBuffer11 at last
            renderPassEncoder.SetIndexBuffer(indexBuffer12, indexFormat);
            renderPassEncoder.SetIndexBuffer(indexBuffer11, indexFormat);

            renderPassEncoder.SetVertexBuffer(0, vertexBuffer3);
            // It should be ok to draw 11 index
            renderPassEncoder.DrawIndexed(11, 1, 0, 0, 0);
            renderPassEncoder.End();

            // Expect success
            encoder.Finish();
        }

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(GetBasicRenderPassDescriptor());
            renderPassEncoder.SetPipeline(pipeline);

            // Index buffer is set to indexBuffer11 at last
            renderPassEncoder.SetIndexBuffer(indexBuffer12, indexFormat);
            renderPassEncoder.SetIndexBuffer(indexBuffer11, indexFormat);

            renderPassEncoder.SetVertexBuffer(0, vertexBuffer3);
            // It should be index buffer OOB to draw 12 index
            renderPassEncoder.DrawIndexed(12, 1, 0, 0, 0);
            renderPassEncoder.End();

            // Expect failure
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
