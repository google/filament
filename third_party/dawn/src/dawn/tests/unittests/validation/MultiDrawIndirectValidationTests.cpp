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

#include <initializer_list>
#include <limits>
#include <vector>
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class MultiDrawIndirectValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiDrawIndirect};
    }

    void SetUp() override {
        ValidationTest::SetUp();

        wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 0.0);
            }
            @fragment fn fs() -> @location(0) vec4f{
                return vec4f(0.0, 0.0, 0.0, 0.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = shaderModule;
        descriptor.cFragment.module = shaderModule;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    void ValidateExpectation(wgpu::CommandEncoder encoder, utils::Expectation expectation) {
        if (expectation == utils::Expectation::Success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    void TestIndirectOffsetDrawIndexed(utils::Expectation expectation,
                                       std::initializer_list<uint32_t> bufferList,
                                       uint64_t indirectOffset,
                                       uint64_t drawCountOffset = 0,
                                       uint32_t maxDrawCount = 1,
                                       bool useDrawCountBuffer = false) {
        TestIndirectOffset(expectation, bufferList, indirectOffset, drawCountOffset, maxDrawCount,
                           useDrawCountBuffer, true);
    }

    void TestIndirectOffsetDraw(utils::Expectation expectation,
                                std::initializer_list<uint32_t> bufferList,
                                uint64_t indirectOffset,
                                uint64_t drawCountOffset = 0,
                                uint32_t maxDrawCount = 1,
                                bool useDrawCountBuffer = false) {
        TestIndirectOffset(expectation, bufferList, indirectOffset, drawCountOffset, maxDrawCount,
                           useDrawCountBuffer, false);
    }

    void TestIndirectOffset(utils::Expectation expectation,
                            std::initializer_list<uint32_t> bufferList,
                            uint64_t indirectOffset,
                            // Offset to the drawCount field in bufferList
                            uint64_t drawCountOffset,
                            uint32_t maxDrawCount,
                            bool useDrawCountBuffer,
                            bool indexed,
                            wgpu::BufferUsage usage = wgpu::BufferUsage::Indirect) {
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, usage, bufferList);

        PlaceholderRenderPass renderPass(device);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        if (indexed) {
            uint32_t zeros[100] = {};
            wgpu::Buffer indexBuffer =
                utils::CreateBufferFromData(device, zeros, sizeof(zeros), wgpu::BufferUsage::Index);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
            pass.MultiDrawIndexedIndirect(indirectBuffer, indirectOffset, maxDrawCount,
                                          useDrawCountBuffer ? indirectBuffer : nullptr,
                                          drawCountOffset);
        } else {
            pass.MultiDrawIndirect(indirectBuffer, indirectOffset, maxDrawCount,
                                   useDrawCountBuffer ? indirectBuffer : nullptr, drawCountOffset);
        }
        pass.End();

        ValidateExpectation(encoder, expectation);
    }

    void TestBufferUsage(utils::Expectation expectation,
                         bool indexed,
                         wgpu::BufferUsage indirectUsage,
                         wgpu::BufferUsage drawCountUsage) {
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, indirectUsage, {1, 2, 3, 4, 5});

        wgpu::Buffer drawCountBuffer =
            utils::CreateBufferFromData<uint32_t>(device, drawCountUsage, {1});

        PlaceholderRenderPass renderPass(device);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        if (indexed) {
            uint32_t zeros[100] = {};
            wgpu::Buffer indexBuffer =
                utils::CreateBufferFromData(device, zeros, sizeof(zeros), wgpu::BufferUsage::Index);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
            pass.MultiDrawIndexedIndirect(indirectBuffer, 0, 1, drawCountBuffer, 0);
        } else {
            pass.MultiDrawIndirect(indirectBuffer, 0, 1, drawCountBuffer, 0);
        }
        pass.End();

        ValidateExpectation(encoder, expectation);
    }

    wgpu::RenderPipeline pipeline;
};

// Verify out of bounds indirect draw calls are caught early
TEST_F(MultiDrawIndirectValidationTest, DrawIndirectOffsetBounds) {
    // In bounds
    TestIndirectOffsetDraw(utils::Expectation::Success, {1, 2, 3, 4}, 0);
    // In bounds, bigger buffer
    TestIndirectOffsetDraw(utils::Expectation::Success, {1, 2, 3, 4, 5, 6, 7}, 0);
    // In bounds, bigger buffer, positive offset
    TestIndirectOffsetDraw(utils::Expectation::Success, {1, 2, 3, 4, 5, 6, 7, 8},
                           4 * sizeof(uint32_t));
    // In bounds for maxDrawCount
    TestIndirectOffsetDraw(utils::Expectation::Success, {1, 2, 3, 4, 5, 6, 7, 8}, 0, 0, 2);
    // In bounds with drawCountBuffer
    TestIndirectOffsetDraw(utils::Expectation::Success, {1, 2, 3, 4}, 0, 0, 1, true);
    // In bounds with drawCountBuffer, bigger buffer
    TestIndirectOffsetDraw(utils::Expectation::Success, {1, 2, 3, 4, 5, 6, 7}, 0,
                           sizeof(uint32_t) * 6, 1, true);

    // In bounds, non-multiple of 4 offsets
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 1);
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 2);
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 0, 1, 1, true);
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 0, 2, 1, true);

    // Out of bounds, buffer too small
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3}, 0);
    // Out of bounds, index too big
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4}, 1 * sizeof(uint32_t));
    // Out of bounds, index past buffer
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4}, 5 * sizeof(uint32_t));
    // Out of bounds, too small for maxDrawCount
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6, 7}, 0, 0, 2);
    // Out of bounds, offset too big for drawCountBuffer
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4}, 0, sizeof(uint32_t) * 4, 1,
                           true);

    // Out of bounds, index + size of command overflows
    uint64_t offset = std::numeric_limits<uint64_t>::max();
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6, 7}, offset);

    // Out of bounds, index + size of command overflows with drawCountBuffer
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6, 7}, 0, offset, 1, true);

    // Out of bounds, maxDrawCount = uint32_t::max()
    uint32_t maxDrawCount = std::numeric_limits<uint32_t>::max();
    TestIndirectOffsetDraw(utils::Expectation::Failure, {1, 2, 3, 4}, 0, 0, maxDrawCount);
}

// Verify out of bounds indirect draw calls are caught early
TEST_F(MultiDrawIndirectValidationTest, DrawIndexedIndirectOffsetBounds) {
    // In bounds
    TestIndirectOffsetDrawIndexed(utils::Expectation::Success, {1, 2, 3, 4, 5}, 0);
    // In bounds, bigger buffer
    TestIndirectOffsetDrawIndexed(utils::Expectation::Success, {1, 2, 3, 4, 5, 6, 7, 8, 9}, 0);
    // In bounds, bigger buffer, positive offset
    TestIndirectOffsetDrawIndexed(utils::Expectation::Success, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                  5 * sizeof(uint32_t));
    // In bounds with drawCountBuffer
    TestIndirectOffsetDrawIndexed(utils::Expectation::Success, {1, 2, 3, 4, 5}, 0, 0, 1, true);
    // In bounds with drawCountBuffer, bigger buffer
    TestIndirectOffsetDrawIndexed(utils::Expectation::Success, {1, 2, 3, 4, 5, 6}, 0,
                                  sizeof(uint32_t) * 5, 1, true);

    // In bounds, non-multiple of 4 offsets
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6}, 1);
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6}, 2);
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6}, 0, 1, 1, true);
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6}, 0, 2, 1, true);

    // Out of bounds, buffer too small
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4}, 0);
    // Out of bounds, index too big
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5},
                                  1 * sizeof(uint32_t));
    // Out of bounds, index past buffer
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5},
                                  5 * sizeof(uint32_t));
    // Out of bounds, too small for maxDrawCount
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 0, 0, 2);
    // Out of bounds, offset too big for drawCountBuffer
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 0,
                                  sizeof(uint32_t) * 5, 1, true);

    // Out of bounds, index + size of command overflows
    uint64_t offset = std::numeric_limits<uint64_t>::max();
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                  offset);

    // Out of bounds, index + size of command overflows with drawCountBuffer
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 0,
                                  offset, 1, true);

    // Out of bounds, maxDrawCount = uint32_t::max()
    uint32_t maxDrawCount = std::numeric_limits<uint32_t>::max();
    TestIndirectOffsetDrawIndexed(utils::Expectation::Failure, {1, 2, 3, 4, 5}, 0, 0, maxDrawCount);
}

// Check that the buffer must have the indirect usage
TEST_F(MultiDrawIndirectValidationTest, IndirectUsage) {
    // Control cases: using a buffer with the indirect usage is valid.
    TestBufferUsage(utils::Expectation::Success, false, wgpu::BufferUsage::Indirect,
                    wgpu::BufferUsage::Indirect);
    TestBufferUsage(utils::Expectation::Success, true, wgpu::BufferUsage::Indirect,
                    wgpu::BufferUsage::Indirect);

    // Error cases: using a buffer with the vertex usage is an error.
    TestBufferUsage(utils::Expectation::Failure, false, wgpu::BufferUsage::Vertex,
                    wgpu::BufferUsage::Indirect);
    TestBufferUsage(utils::Expectation::Failure, true, wgpu::BufferUsage::Vertex,
                    wgpu::BufferUsage::Indirect);
    TestBufferUsage(utils::Expectation::Failure, false, wgpu::BufferUsage::Indirect,
                    wgpu::BufferUsage::Vertex);
    TestBufferUsage(utils::Expectation::Failure, true, wgpu::BufferUsage::Indirect,
                    wgpu::BufferUsage::Vertex);
}

}  // anonymous namespace
}  // namespace dawn
