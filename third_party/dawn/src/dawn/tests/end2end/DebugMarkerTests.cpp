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

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/WGPUHelpers.h"

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

DAWN_INSTANTIATE_TEST(DebugMarkerTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
