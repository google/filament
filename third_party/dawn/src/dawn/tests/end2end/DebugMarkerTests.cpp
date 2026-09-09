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

#include <string>

#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

using testing::HasSubstr;

namespace dawn {
namespace {

class DebugMarkerTests : public DawnTest {};

// Make sure that calling a marker API without a debugging tool attached doesn't cause a failure.
TEST_P(DebugMarkerTests, NoFailureWithoutDebugToolAttached) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4, 4);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

TEST_P(DebugMarkerTests, StringDeletedAfterPushDebugGroup) {
    // Check that the error message still contains the marker after deleting the string passed in
    // argument. Also overwrite the string in case small-string optimization makes the test happen
    // to pass otherwise.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    // CommandEncoder::PushDebugGroup.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            std::string marker = "foo";
            encoder.PushDebugGroup(std::string_view(marker));
            marker[0] = 'b';
        }
        encoder.InjectValidationError("Whatever");
        encoder.PopDebugGroup();

        ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), HasSubstr("foo"));
    }

    // ComputePassEncoder::PushDebugGroup.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        {
            std::string marker = "foo";
            pass.PushDebugGroup(std::string_view(marker));
            marker[0] = 'b';
        }
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.PopDebugGroup();

        ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), HasSubstr("foo"));
    }

    // RenderPassEncoder::PushDebugGroup.
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4, 4);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        {
            std::string marker = "foo";
            pass.PushDebugGroup(std::string_view(marker));
            marker[0] = 'b';
        }
        pass.Draw(1);
        pass.End();
        encoder.PopDebugGroup();

        ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), HasSubstr("foo"));
    }
}

DAWN_INSTANTIATE_TEST(DebugMarkerTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
