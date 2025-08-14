// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using testing::HasSubstr;

class UnsafeAPIValidationTest : public ValidationTest {
  protected:
    // UnsafeAPIValidationTest create the device with the AllowUnsafeAPIs toggle explicitly
    // disabled, which overrides the inheritance.
    std::vector<const char*> GetDisabledToggles() override {
        // Disable the AllowUnsafeAPIs toggles in device toggles descriptor to override the
        // inheritance and create a device disallowing unsafe apis.
        return {"allow_unsafe_apis"};
    }
};

// Check chromium_disable_uniformity_analysis is an unsafe API.
TEST_F(UnsafeAPIValidationTest, chromium_disable_uniformity_analysis) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_disable_uniformity_analysis;

        @compute @workgroup_size(8) fn uniformity_error(
            @builtin(local_invocation_id) local_invocation_id : vec3u
        ) {
            if (local_invocation_id.x == 0u) {
                workgroupBarrier();
            }
        }
    )"));
}

// Check that using bindingArraySize > 1 is an unsafe API.
TEST_F(UnsafeAPIValidationTest, BindGroupLayoutEntryArraySize) {
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;

    entry.bindingArraySize = 0;
    device.CreateBindGroupLayout(&desc);

    entry.bindingArraySize = 1;
    device.CreateBindGroupLayout(&desc);

    entry.bindingArraySize = 2;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that using a binding_array statically in an entry point is an unsafe API.
TEST_F(UnsafeAPIValidationTest, BindingArrayInWGSL) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var textures : binding_array<texture_2d<f32>, 3>;
        @fragment fn fs() -> @location(0) u32 {
            let _ = textures[0];
            return 0;
        }
    )"));

    // Even an array of size 1 is an error.
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var textures : binding_array<texture_2d<f32>, 1>;
        @fragment fn fs() -> @location(0) u32 {
            let _ = textures[0];
            return 0;
        }
    )"));
}

class TimestampQueryUnsafeAPIValidationTest : public ValidationTest {
  protected:
    std::vector<const char*> GetDisabledToggles() override { return {"allow_unsafe_apis"}; }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::TimestampQuery};
    }
};

// Check write timestamp on command encoder is an unsafe API.
TEST_F(TimestampQueryUnsafeAPIValidationTest, WriteTimestampOnCommandEncoder) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.type = wgpu::QueryType::Timestamp;
    descriptor.count = 2;

    wgpu::QuerySet timestampQuerySet = device.CreateQuerySet(&descriptor);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.WriteTimestamp(timestampQuerySet, 0);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

}  // anonymous namespace
}  // namespace dawn
