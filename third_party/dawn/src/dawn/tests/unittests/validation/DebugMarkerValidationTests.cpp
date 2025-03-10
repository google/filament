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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class DebugMarkerValidationTest : public ValidationTest {};

// Correct usage of debug markers should succeed in render pass.
TEST_F(DebugMarkerValidationTest, RenderSuccess) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.PushDebugGroup("Event Start");
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.PopDebugGroup();
        pass.End();
    }

    encoder.Finish();
}

// A PushDebugGroup call without a following PopDebugGroup produces an error in render pass.
TEST_F(DebugMarkerValidationTest, RenderUnbalancedPush) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.PushDebugGroup("Event Start");
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// A PopDebugGroup call without a preceding PushDebugGroup produces an error in render pass.
TEST_F(DebugMarkerValidationTest, RenderUnbalancedPop) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.PopDebugGroup();
        pass.End();
    }

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Correct usage of debug markers should succeed in render bundle.
TEST_F(DebugMarkerValidationTest, RenderBundleSuccess) {
    utils::ComboRenderBundleEncoderDescriptor desc;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    desc.colorFormatCount = 1;

    wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&desc);
    encoder.PushDebugGroup("Event Start");
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    encoder.PopDebugGroup();

    encoder.Finish();
}

// A PushDebugGroup call without a following PopDebugGroup produces an error in render bundle.
TEST_F(DebugMarkerValidationTest, RenderBundleUnbalancedPush) {
    utils::ComboRenderBundleEncoderDescriptor desc;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    desc.colorFormatCount = 1;

    wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&desc);
    encoder.PushDebugGroup("Event Start");
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// A PopDebugGroup call without a preceding PushDebugGroup produces an error in render bundle.
TEST_F(DebugMarkerValidationTest, RenderBundleUnbalancedPop) {
    utils::ComboRenderBundleEncoderDescriptor desc;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    desc.colorFormatCount = 1;

    wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&desc);
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    encoder.PopDebugGroup();

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Correct usage of debug markers should succeed in compute pass.
TEST_F(DebugMarkerValidationTest, ComputeSuccess) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.PushDebugGroup("Event Start");
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.PopDebugGroup();
        pass.End();
    }

    encoder.Finish();
}

// A PushDebugGroup call without a following PopDebugGroup produces an error in compute pass.
TEST_F(DebugMarkerValidationTest, ComputeUnbalancedPush) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.PushDebugGroup("Event Start");
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// A PopDebugGroup call without a preceding PushDebugGroup produces an error in compute pass.
TEST_F(DebugMarkerValidationTest, ComputeUnbalancedPop) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.PopDebugGroup();
        pass.End();
    }

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Correct usage of debug markers should succeed in command encoder.
TEST_F(DebugMarkerValidationTest, CommandEncoderSuccess) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    encoder.PopDebugGroup();
    encoder.Finish();
}

// A PushDebugGroup call without a following PopDebugGroup produces an error in command encoder.
TEST_F(DebugMarkerValidationTest, CommandEncoderUnbalancedPush) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// A PopDebugGroup call without a preceding PushDebugGroup produces an error in command encoder.
TEST_F(DebugMarkerValidationTest, CommandEncoderUnbalancedPop) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    encoder.PopDebugGroup();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// It is possible to nested pushes in a compute pass in a command encoder.
TEST_F(DebugMarkerValidationTest, NestedComputeInCommandEncoder) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }
    encoder.PopDebugGroup();
    encoder.Finish();
}

// Command encoder and compute pass pushes must be balanced independently.
TEST_F(DebugMarkerValidationTest, NestedComputeInCommandEncoderIndependent) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// It is possible to nested pushes in a render pass in a command encoder.
TEST_F(DebugMarkerValidationTest, NestedRenderInCommandEncoder) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }
    encoder.PopDebugGroup();
    encoder.Finish();
}

// Command encoder and render pass pushes must be balanced independently.
TEST_F(DebugMarkerValidationTest, NestedRenderInCommandEncoderIndependent) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

}  // anonymous namespace
}  // namespace dawn
