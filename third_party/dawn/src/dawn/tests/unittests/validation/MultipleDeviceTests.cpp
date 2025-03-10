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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/native/DawnNative.h"
#include "dawn/tests/MockCallback.h"

namespace dawn {
namespace {

using testing::_;
using testing::IsNull;
using testing::MockCppCallback;
using testing::NotNull;

class MultipleDeviceTest : public ValidationTest {};

// Test that it is invalid to submit a command buffer created on a different device.
TEST_F(MultipleDeviceTest, ValidatesSameDevice) {
    wgpu::Device device2 = RequestDeviceSync(wgpu::DeviceDescriptor{});
    wgpu::CommandBuffer commandBuffer = device2.CreateCommandEncoder().Finish();

    ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commandBuffer));
}

// Test that CreatePipelineAsync fails creation with an Error status if it uses
// objects from a different device.
TEST_F(MultipleDeviceTest, ValidatesSameDeviceCreatePipelineAsync) {
    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgslDesc.code = R"(
         @compute @workgroup_size(1, 1, 1) fn main() {
        }
    )";

    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    using MockComputePipelineAsyncCallback = MockCppCallback<void (*)(
        wgpu::CreatePipelineAsyncStatus, wgpu::ComputePipeline, wgpu::StringView)>;

    // Base case: CreateComputePipelineAsync succeeds.
    {
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc = {};
        pipelineDesc.compute.module = shaderModule;

        MockComputePipelineAsyncCallback creationCallback;
        EXPECT_CALL(creationCallback, Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), _))
            .Times(1);
        device.CreateComputePipelineAsync(&pipelineDesc,
                                          UsesWire() ? wgpu::CallbackMode::AllowSpontaneous
                                                     : wgpu::CallbackMode::AllowProcessEvents,
                                          creationCallback.Callback());

        WaitForAllOperations();
    }

    // CreateComputePipelineAsync errors if the shader module is created on a different device.
    {
        wgpu::Device device2 = RequestDeviceSync(wgpu::DeviceDescriptor{});
        wgpu::ShaderModule shaderModule = device2.CreateShaderModule(&shaderModuleDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc = {};
        pipelineDesc.compute.module = shaderModule;

        MockComputePipelineAsyncCallback creationCallback;
        EXPECT_CALL(creationCallback,
                    Call(wgpu::CreatePipelineAsyncStatus::ValidationError, IsNull(), _))
            .Times(1);
        device.CreateComputePipelineAsync(&pipelineDesc,
                                          UsesWire() ? wgpu::CallbackMode::AllowSpontaneous
                                                     : wgpu::CallbackMode::AllowProcessEvents,
                                          creationCallback.Callback());

        WaitForAllOperations();
    }
}

}  // anonymous namespace
}  // namespace dawn
