// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ComputePipelineValidationTest : public ValidationTest {
  protected:
    // Helper function that create a shader module with compute entry point named main and
    // workgroup size of (workgroup_size_x, 1, 1).
    wgpu::ShaderModule CreateShaderModule(uint32_t workgroup_size_x = 1) {
        std::stringstream shader;
        shader << R"(
            @compute @workgroup_size()"
               << workgroup_size_x << R"(, 1, 1)
            fn main() {
        })";
        return utils::CreateShaderModule(device, shader.str().c_str());
    }
};

// Test that creating a compute pipeline with basic shader module and pipeline layout succeeds.
TEST_F(ComputePipelineValidationTest, Success) {
    auto computeModule = CreateShaderModule();

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = computeModule;
    device.CreateComputePipeline(&csDesc);
}

// Test that creating a compute pipeline with mismatched entry point name fails.
TEST_F(ComputePipelineValidationTest, EntryPointNameMismatched) {
    auto computeModule = CreateShaderModule();

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = computeModule;
    csDesc.compute.entryPoint = "main0";
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// TODO(cwallez@chromium.org): Add a regression test for Disptach validation trying to access the
// input state.

class ComputeDispatchValidationTest : public ValidationTest {
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

    void TestDispatch(uint32_t x, uint32_t y, uint32_t z) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(x, y, z);
        pass.End();
        encoder.Finish();
    }

    wgpu::ComputePipeline pipeline;
};

// Check that 1x1x1 dispatch is OK.
TEST_F(ComputeDispatchValidationTest, PerDimensionDispatchSizeLimits_SmallestValid) {
    TestDispatch(1, 1, 1);
}

// Check that the largest allowed dispatch is OK.
TEST_F(ComputeDispatchValidationTest, PerDimensionDispatchSizeLimits_LargestValid) {
    const uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;
    TestDispatch(max, max, max);
}

// Check that exceeding the maximum on the X dimension results in validation failure.
TEST_F(ComputeDispatchValidationTest, PerDimensionDispatchSizeLimits_InvalidX) {
    const uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(max + 1, 1, 1));
}

// Check that exceeding the maximum on the Y dimension results in validation failure.
TEST_F(ComputeDispatchValidationTest, PerDimensionDispatchSizeLimits_InvalidY) {
    const uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(1, max + 1, 1));
}

// Check that exceeding the maximum on the Z dimension results in validation failure.
TEST_F(ComputeDispatchValidationTest, PerDimensionDispatchSizeLimits_InvalidZ) {
    const uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(1, 1, max + 1));
}

// Check that exceeding the maximum on all dimensions results in validation failure.
TEST_F(ComputeDispatchValidationTest, PerDimensionDispatchSizeLimits_InvalidAll) {
    const uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(max + 1, max + 1, max + 1));
}

class ComputeValidationEntryPointTest : public ValidationTest {};

// Check that entry points are optional.
TEST_F(ComputeValidationEntryPointTest, EntryPointNameOptional) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {}
    )");

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = nullptr;

    device.CreateComputePipeline(&csDesc);

    csDesc.layout = nullptr;
    device.CreateComputePipeline(&csDesc);
}

// Check that entry points are required if module has multiple entry points.
TEST_F(ComputeValidationEntryPointTest, EntryPointNameRequiredIfMultipleEntryPoints) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main1() {}
        @compute @workgroup_size(1) fn main2() {}
    )");

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main1";

    device.CreateComputePipeline(&csDesc);

    csDesc.compute.entryPoint = "nullptr";
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Check that entry points are required if module has no compatible entry points.
TEST_F(ComputeValidationEntryPointTest, EntryPointNameRequiredIfNoCompatibleEntryPoints) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @fragment fn main() {}
    )");

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = nullptr;

    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

}  // anonymous namespace
}  // namespace dawn
