// Copyright 2023 The Dawn & Tint Authors
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

#include <utility>
#include <vector>

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class InternalResolveAttachmentSampleTypeTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // vertex shader module.
        vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");
    }

    wgpu::PipelineLayout CreatePipelineLayout(bool withSampler) {
        // Create binding group layout with internal resolve attachment sample type.
        std::vector<native::BindGroupLayoutEntry> bglEntries(2);
        bglEntries[0].binding = 0;
        bglEntries[0].texture.sampleType = native::kInternalResolveAttachmentSampleType;
        bglEntries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
        bglEntries[0].visibility = wgpu::ShaderStage::Fragment;

        native::BindGroupLayoutDescriptor bglDesc;

        if (withSampler) {
            bglEntries[1].binding = 1;
            bglEntries[1].sampler.type = wgpu::SamplerBindingType::Filtering;
            bglEntries[1].visibility = wgpu::ShaderStage::Fragment;

            bglDesc.entryCount = bglEntries.size();
        } else {
            bglDesc.entryCount = 1;
        }
        bglDesc.entries = bglEntries.data();

        native::DeviceBase* nativeDevice = native::FromAPI(device.Get());

        Ref<native::BindGroupLayoutBase> bglRef =
            nativeDevice->CreateBindGroupLayout(&bglDesc, true).AcquireSuccess();

        auto bindGroupLayout =
            wgpu::BindGroupLayout::Acquire(native::ToAPI(ReturnToAPI(std::move(bglRef))));

        // Create pipeline layout from the bind group layout.
        wgpu::PipelineLayoutDescriptor descriptor;
        std::vector<wgpu::BindGroupLayout> bindgroupLayouts = {bindGroupLayout};
        descriptor.bindGroupLayoutCount = bindgroupLayouts.size();
        descriptor.bindGroupLayouts = bindgroupLayouts.data();
        return device.CreatePipelineLayout(&descriptor);
    }

    wgpu::ShaderModule vsModule;
};

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is compatible with
// a shader using textureLoad(texture_2d<f32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureLoadF32Compatible) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<f32>;

        @fragment fn main() -> @location(0) vec4f {
            return textureLoad(src_tex, vec2u(0, 0), 0);
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/false);

    device.CreateRenderPipeline(&pipelineDescriptor);
}

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is compatible
// with a shader using textureSample(texture_2d<f32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureSampleF32Compatible) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<f32>;
        @group(0) @binding(1) var src_sampler : sampler;

        @fragment fn main() -> @location(0) vec4f {
            return textureSample(src_tex, src_sampler, vec2f(0.0, 0.0));
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/true);

    device.CreateRenderPipeline(&pipelineDescriptor);
}

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is incompatible
// with a shader using textureLoad(texture_2d<i32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureLoadI32Incompatible) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<i32>;

        @fragment fn main() -> @location(0) vec4i {
            return textureLoad(src_tex, vec2u(0, 0), 0);
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/false);

    ASSERT_DEVICE_ERROR_MSG(device.CreateRenderPipeline(&pipelineDescriptor),
                            testing::HasSubstr("isn't compatible"));
}

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is incompatible
// with a shader using textureLoad(texture_2d<u32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureLoadU32Incompatible) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<u32>;

        @fragment fn main() -> @location(0) vec4u {
            return textureLoad(src_tex, vec2u(0, 0), 0);
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/false);

    ASSERT_DEVICE_ERROR_MSG(device.CreateRenderPipeline(&pipelineDescriptor),
                            testing::HasSubstr("isn't compatible"));
}

DAWN_INSTANTIATE_TEST(InternalResolveAttachmentSampleTypeTests, NullBackend());

}  // anonymous namespace
}  // namespace dawn
