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

#include <initializer_list>
#include <limits>
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ComputeIndirectValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        wgpu::ShaderModule computeModule = utils::CreateShaderModule(device, R"(
            @compute @workgroup_size(1) fn main() {
            })");

        // Set up compute pipeline
        wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, nullptr);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pl;
        csDesc.compute.module = computeModule;
        pipeline = device.CreateComputePipeline(&csDesc);
    }

    void ValidateExpectation(wgpu::CommandEncoder encoder, utils::Expectation expectation) {
        if (expectation == utils::Expectation::Success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    void TestIndirectOffset(utils::Expectation expectation,
                            std::initializer_list<uint32_t> bufferList,
                            uint64_t indirectOffset,
                            wgpu::BufferUsage usage = wgpu::BufferUsage::Indirect) {
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, usage, bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
        pass.End();

        ValidateExpectation(encoder, expectation);
    }

    wgpu::ComputePipeline pipeline;
};

// Verify out of bounds indirect dispatch calls are caught early
TEST_F(ComputeIndirectValidationTest, IndirectOffsetBounds) {
    // In bounds
    TestIndirectOffset(utils::Expectation::Success, {1, 2, 3}, 0);
    // In bounds, bigger buffer
    TestIndirectOffset(utils::Expectation::Success, {1, 2, 3, 4, 5, 6}, 0);
    // In bounds, bigger buffer, positive offset
    TestIndirectOffset(utils::Expectation::Success, {1, 2, 3, 4, 5, 6}, 3 * sizeof(uint32_t));

    // In bounds, non-multiple of 4 offsets
    TestIndirectOffset(utils::Expectation::Failure, {1, 2, 3, 4}, 1);
    TestIndirectOffset(utils::Expectation::Failure, {1, 2, 3, 4}, 2);

    // Out of bounds, buffer too small
    TestIndirectOffset(utils::Expectation::Failure, {1, 2}, 0);
    // Out of bounds, index too big
    TestIndirectOffset(utils::Expectation::Failure, {1, 2, 3}, 1 * sizeof(uint32_t));
    // Out of bounds, index past buffer
    TestIndirectOffset(utils::Expectation::Failure, {1, 2, 3}, 4 * sizeof(uint32_t));
    // Out of bounds, index + size of command overflows
    uint64_t offset = std::numeric_limits<uint64_t>::max();
    TestIndirectOffset(utils::Expectation::Failure, {1, 2, 3, 4, 5, 6}, offset);
}

// Check that the buffer must have the indirect usage
TEST_F(ComputeIndirectValidationTest, IndirectUsage) {
    // Control case: using a buffer with the indirect usage is valid.
    TestIndirectOffset(utils::Expectation::Success, {1, 2, 3}, 0, wgpu::BufferUsage::Indirect);

    // Error case: using a buffer with the vertex usage is an error.
    TestIndirectOffset(utils::Expectation::Failure, {1, 2, 3}, 0, wgpu::BufferUsage::Vertex);
}

}  // anonymous namespace
}  // namespace dawn
