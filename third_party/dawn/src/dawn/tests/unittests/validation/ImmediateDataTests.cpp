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

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "dawn/common/NonMovable.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

enum class FeatureMode {
    Enabled,
    DisabledViaNotAllowUnsafeAPIs,
    DisabledViaBlocklistedFeatures,
};

// Test that the feature only works when enabled
struct ImmediateDataDisableTest : ValidationTestWithParam<FeatureMode> {
    std::vector<const char*> GetWGSLBlocklistedFeatures() override {
        switch (GetParam()) {
            case FeatureMode::Enabled:
                return {};
            case FeatureMode::DisabledViaNotAllowUnsafeAPIs:
                return {};
            case FeatureMode::DisabledViaBlocklistedFeatures:
                return {"immediate_address_space"};
        }
        DAWN_UNREACHABLE();
        return {};
    }

    bool AllowUnsafeAPIs() override {
        switch (GetParam()) {
            case FeatureMode::Enabled:
                // Currently the only way to enable ImmediateAddressSpace is via AllowUnsafeAPIs.
                // See GetLanguageFeatureStatus.
                return true;
            case FeatureMode::DisabledViaNotAllowUnsafeAPIs:
                return false;
            case FeatureMode::DisabledViaBlocklistedFeatures:
                // Enabling AllowUnsafeAPIs while disabling via blocklist should still fail.
                return true;
        }
        DAWN_UNREACHABLE();
        return false;
    }
};

// Check that creating a PipelineLayout with non-zero immediateSize is disallowed
// without the feature enabled.
TEST_P(ImmediateDataDisableTest, ImmediateSizeNotAllowed) {
    wgpu::PipelineLayoutDescriptor desc{};
    desc.immediateSize = 4;

    if (GetParam() == FeatureMode::Enabled) {
        device.CreatePipelineLayout(&desc);
    } else {
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
}

// Check that SetImmediates doesn't work (even with size=0) without the feature enabled.
TEST_P(ImmediateDataDisableTest, SetImmediates) {
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetImmediates(0, nullptr, 0);
        pass.End();
        if (GetParam() == FeatureMode::Enabled) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
    {
        const uint32_t data = 0;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetImmediates(0, &data, 4);
        pass.End();
        if (GetParam() == FeatureMode::Enabled) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Check that limits.maxImmediateSize is 0 when the feature is disabled, and kMaxImmediateDataBytes
// otherwise.
TEST_P(ImmediateDataDisableTest, MaxImmediateSizeIsZero) {
    if (GetParam() == FeatureMode::Enabled) {
        ASSERT_EQ(GetSupportedLimits().maxImmediateSize, kMaxImmediateDataBytes);
    } else {
        ASSERT_EQ(GetSupportedLimits().maxImmediateSize, 0u);
    }
}

INSTANTIATE_TEST_SUITE_P(,
                         ImmediateDataDisableTest,
                         ::testing::ValuesIn({FeatureMode::Enabled,
                                              FeatureMode::DisabledViaNotAllowUnsafeAPIs,
                                              FeatureMode::DisabledViaBlocklistedFeatures}));

class ImmediateDataTest : public ValidationTest {
  protected:
    wgpu::BindGroupLayout CreateBindGroupLayout() {
        wgpu::BindGroupLayoutEntry entries[1];
        entries[0].binding = 0;
        entries[0].visibility = wgpu::ShaderStage::Compute;
        entries[0].buffer.type = wgpu::BufferBindingType::Storage;

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
        bindGroupLayoutDesc.entryCount = 1;
        bindGroupLayoutDesc.entries = entries;

        return device.CreateBindGroupLayout(&bindGroupLayoutDesc);
    }

    wgpu::PipelineLayout CreatePipelineLayout(uint32_t requiredImmediateSize) {
        wgpu::BindGroupLayout bindGroupLayout = CreateBindGroupLayout();

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
        pipelineLayoutDesc.immediateSize = requiredImmediateSize;
        return device.CreatePipelineLayout(&pipelineLayoutDesc);
    }

    wgpu::ShaderModule mShaderModule;
};

// Check that non-zero immediateSize is possible with feature enabled and size must
// below max size limits.
TEST_F(ImmediateDataTest, ValidateImmediateSize) {
    wgpu::PipelineLayoutDescriptor desc{};

    // Success case with valid immediateSize.
    {
        desc.immediateSize = kMaxImmediateDataBytes;
        device.CreatePipelineLayout(&desc);
    }

    // Failed case with invalid immediateSize that exceed limits.
    {
        desc.immediateSize = kMaxImmediateDataBytes + 4;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
}

// Check that immediateSize must be aligned to kImmediateConstantElementByteSize (4 bytes).
TEST_F(ImmediateDataTest, ValidateImmediateSizeAlignment) {
    wgpu::PipelineLayoutDescriptor desc{};

    // Success case: aligned to 4 bytes.
    {
        desc.immediateSize = 4;
        device.CreatePipelineLayout(&desc);
    }
    {
        desc.immediateSize = 8;
        device.CreatePipelineLayout(&desc);
    }
    {
        desc.immediateSize = kMaxImmediateDataBytes;
        device.CreatePipelineLayout(&desc);
    }

    // Failed case: not aligned to 4 bytes.
    {
        desc.immediateSize = 1;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
    {
        desc.immediateSize = 2;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
    {
        desc.immediateSize = 3;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
    {
        desc.immediateSize = 5;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
}

// Check that SetImmediates offset and length must be aligned to 4 bytes.
TEST_F(ImmediateDataTest, ValidateSetImmediatesAlignment) {
    // Success cases
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t data = 0;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(0, &data, 4);
        computePass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(4, nullptr, 0);
        computePass.End();
        encoder.Finish();
    }

    // Failed case with non-aligned offset bytes
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(2, nullptr, 0);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed cases with non-aligned size
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint8_t data = 0;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(0, &data, 2);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that SetImmediates offset + length must be in bound.
TEST_F(ImmediateDataTest, ValidateSetImmediatesOOB) {
    // Success cases
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        std::vector<uint32_t> data(kMaxImmediateDataBytes / 4, 0);
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(0, data.data(), kMaxImmediateDataBytes);
        computePass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(kMaxImmediateDataBytes, nullptr, 0);
        computePass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t data = 0;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(kMaxImmediateDataBytes - 4, &data, 4);
        computePass.End();
        encoder.Finish();
    }

    // Failed case with offset oob
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t offset = kMaxImmediateDataBytes + 4;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(offset, nullptr, 0);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed cases with size oob
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t size = kMaxImmediateDataBytes + 4;
        std::vector<uint32_t> data(size / 4, 0);
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(0, data.data(), size);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed cases with offset + size oob
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t offset = kMaxImmediateDataBytes;
        uint32_t data[] = {0};
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(offset, data, 4);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed case with super large offset + size oob but looping back to zero
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t offset = std::numeric_limits<uint32_t>::max() - 3;
        uint32_t data[] = {0};
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediates(offset, data, 4);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that pipelineLayout immediate data bytes compatible with shaders.
TEST_F(ImmediateDataTest, ValidatePipelineLayoutImmediateDataBytesAndShaders) {
    constexpr uint32_t kShaderImmediateDataBytes = 12u;
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        var<immediate> fragmentConstants: vec3f;
        var<immediate> computeConstants: vec3u;
        @vertex fn vsMain(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            const pos = array(
                vec2( 1.0, -1.0),
                vec2(-1.0, -1.0),
                vec2( 0.0,  1.0),
            );
            return vec4(pos[VertexIndex], 0.0, 1.0);
        }

        // to reuse the same pipeline layout
        @fragment fn fsMain() -> @location(0) vec4f {
            return vec4f(fragmentConstants, 1.0);
        }


        @group(0) @binding(0) var<storage, read_write> output : vec3u;

        @compute @workgroup_size(1, 1, 1)
        fn csMain() {
            output = computeConstants;
        })");

    // Success cases
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = shaderModule;
        pipelineDescriptor.cFragment.module = shaderModule;
        pipelineDescriptor.layout = CreatePipelineLayout(kShaderImmediateDataBytes);
        device.CreateRenderPipeline(&pipelineDescriptor);
    }

    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = shaderModule;
        csDesc.layout = CreatePipelineLayout(kShaderImmediateDataBytes);

        device.CreateComputePipeline(&csDesc);
    }

    // Default layout
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = shaderModule;
        pipelineDescriptor.cFragment.module = shaderModule;
        device.CreateRenderPipeline(&pipelineDescriptor);
    }

    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = shaderModule;

        device.CreateComputePipeline(&csDesc);
    }

    // Failed case with fragment shader requires more immediate data.
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = shaderModule;
        pipelineDescriptor.cFragment.module = shaderModule;
        pipelineDescriptor.layout = CreatePipelineLayout(kShaderImmediateDataBytes - 4);
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pipelineDescriptor));
    }

    // Failed cases with compute shader requires more immediate data.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = shaderModule;
        csDesc.layout = CreatePipelineLayout(kShaderImmediateDataBytes - 4);

        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }
}

// Check that default pipelineLayout has too many immediate data bytes .
TEST_F(ImmediateDataTest, ValidateDefaultPipelineLayout) {
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
            var<immediate> fragmentConstants: vec4f;

            var<immediate> computeConstants: vec4u;

            @vertex fn vsMain(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                const pos = array(
                    vec2( 1.0, -1.0),
                    vec2(-1.0, -1.0),
                    vec2( 0.0,  1.0),
                );
                return vec4(pos[VertexIndex], 0.0, 1.0);
            }

            // to reuse the same pipeline layout
            @fragment fn fsMain() -> @location(0) vec4f {
                return vec4f(fragmentConstants.x, fragmentConstants.yzw);
            }

            @group(0) @binding(0) var<storage, read_write> output : vec4u;

            @compute @workgroup_size(1, 1, 1)
            fn csMain() {
                output = vec4u(computeConstants.x, computeConstants.yzw);
            })");

    // Shader module creation should succeed even with too much immediate data.
    // The validation error should occur at pipeline creation time.
    wgpu::ShaderModule oobShaderModule = utils::CreateShaderModule(device, R"(
            struct FragmentConstants {
                c0: vec4f,
                c1: vec4f,
                c2: vec4f,
                c3: vec4f,
                constantsOOB: f32,
            };

            struct ComputeConstants {
                c0: vec4u,
                c1: vec4u,
                c2: vec4u,
                c3: vec4u,
                constantsOOB: u32,
            };
            var<immediate> fragmentConstants: FragmentConstants;
            var<immediate> computeConstants: ComputeConstants;
            @vertex fn vsMain(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                const pos = array(
                    vec2( 1.0, -1.0),
                    vec2(-1.0, -1.0),
                    vec2( 0.0,  1.0),
                );
                return vec4(pos[VertexIndex], 0.0, 1.0);
            }

            // to reuse the same pipeline layout
            @fragment fn fsMain() -> @location(0) vec4f {
                return vec4f(fragmentConstants.c0.x + fragmentConstants.constantsOOB,
                             fragmentConstants.c0.yzw);
            }

            @group(0) @binding(0) var<storage, read_write> output : vec4u;

            @compute @workgroup_size(1, 1, 1)
            fn csMain() {
                output = vec4u(computeConstants.c0.x + computeConstants.constantsOOB,
                               computeConstants.c0.yzw);
            })");

    // Failed case: too much immediate data in shader should fail at pipeline creation.
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = oobShaderModule;
        pipelineDescriptor.cFragment.module = oobShaderModule;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pipelineDescriptor));
    }

    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = oobShaderModule;

        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }

    // Success cases
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = shaderModule;
        pipelineDescriptor.cFragment.module = shaderModule;
        device.CreateRenderPipeline(&pipelineDescriptor);
    }

    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = shaderModule;

        device.CreateComputePipeline(&csDesc);
    }
}

// Check that executing multiple bundles with different immediate data requirements works.
TEST_F(ImmediateDataTest, ExecuteBundlesWithDifferentImmediateData) {
    // Pipeline 4: requires 4 bytes
    wgpu::ShaderModule module4 = utils::CreateShaderModule(device, R"(
        var<immediate> constants: u32;
        @vertex fn vs() -> @builtin(position) vec4f {
            _ = constants;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");
    utils::ComboRenderPipelineDescriptor desc4;
    desc4.vertex.module = module4;
    desc4.cFragment.module = module4;
    wgpu::RenderPipeline pipeline4 = device.CreateRenderPipeline(&desc4);

    // Pipeline 8: requires 8 bytes
    wgpu::ShaderModule module8 = utils::CreateShaderModule(device, R"(
        struct Constants {
            a: u32,
            b: u32,
        }
        var<immediate> constants: Constants;
        @vertex fn vs() -> @builtin(position) vec4f {
            _ = constants.b;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");
    utils::ComboRenderPipelineDescriptor desc8;
    desc8.vertex.module = module8;
    desc8.cFragment.module = module8;
    wgpu::RenderPipeline pipeline8 = device.CreateRenderPipeline(&desc8);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Bundle 4
    wgpu::RenderBundle bundle4;
    {
        wgpu::RenderBundleEncoderDescriptor bundleDesc;
        bundleDesc.colorFormatCount = 1;
        bundleDesc.colorFormats = &renderPass.colorFormat;
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&bundleDesc);
        encoder.SetPipeline(pipeline4);
        uint32_t data = 0;
        encoder.SetImmediates(0, &data, 4);
        encoder.Draw(3);
        bundle4 = encoder.Finish();
    }

    // Bundle 8
    wgpu::RenderBundle bundle8;
    {
        wgpu::RenderBundleEncoderDescriptor bundleDesc;
        bundleDesc.colorFormatCount = 1;
        bundleDesc.colorFormats = &renderPass.colorFormat;
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&bundleDesc);
        encoder.SetPipeline(pipeline8);
        uint32_t data[] = {0, 0};
        encoder.SetImmediates(0, data, 8);
        encoder.Draw(3);
        bundle8 = encoder.Finish();
    }

    // Execute both
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    wgpu::RenderBundle bundles[] = {bundle4, bundle8};
    pass.ExecuteBundles(2, bundles);
    pass.End();
    encoder.Finish();
}

// Check that ExecuteBundles resets the immediate data state in the RenderPass.
TEST_F(ImmediateDataTest, ExecuteBundlesResetsImmediateDataState) {
    // Pipeline 4: requires 4 bytes
    wgpu::ShaderModule module4 = utils::CreateShaderModule(device, R"(
        var<immediate> constants: u32;
        @vertex fn vs() -> @builtin(position) vec4f {
            _ = constants;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");
    utils::ComboRenderPipelineDescriptor desc4;
    desc4.vertex.module = module4;
    desc4.cFragment.module = module4;
    wgpu::RenderPipeline pipeline4 = device.CreateRenderPipeline(&desc4);

    // Bundle (placeholder, just to execute)
    wgpu::RenderBundle bundle;
    {
        wgpu::RenderBundleEncoderDescriptor bundleDesc;
        bundleDesc.colorFormatCount = 1;
        wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm;
        bundleDesc.colorFormats = &format;
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&bundleDesc);
        bundle = encoder.Finish();
    }

    // Case 1: Immediate -> ExecuteBundles -> SetPipeline -> Draw (Fail: No immediate data)
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(pipeline4);
        uint32_t data = 0;
        pass.SetImmediates(0, &data, 4);

        pass.ExecuteBundles(1, &bundle);

        pass.SetPipeline(pipeline4);  // Restore pipeline
        pass.Draw(3);                 // Should fail (Immediate data lost)
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Case 2: ExecuteBundles -> SetImmediates -> SetPipeline -> Draw (Success)
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(1, &bundle);
        uint32_t data = 0;
        pass.SetImmediates(0, &data, 4);
        pass.SetPipeline(pipeline4);
        pass.Draw(3);
        pass.End();
        encoder.Finish();
    }
}

enum class EncoderType {
    Compute,
    RenderPass,
    RenderBundle,
};

struct ImmediateDataRange {
    uint32_t offset;
    uint32_t size;
};

class ImmediateDataRequiredTest : public ImmediateDataTest,
                                  public testing::WithParamInterface<EncoderType> {
  protected:
    void TestImmediateDataValidation(std::string entryPoint,
                                     std::vector<ImmediateDataRange> ranges,
                                     bool success,
                                     EncoderType encoderType) {
        // Structs with padding:
        // PadMiddle:
        //   a: u32 (0..4)
        //   padding (4..16) -> 12 bytes
        //   b: vec4<f32> (16..32)
        //   Size: 32
        //
        // PadTail:
        //   a: vec3<f32> (0..12)
        //   padding (12..16) -> 4 bytes
        //   Size: 16
        //
        // Layout:
        //   padMiddle: offset 0, size 32
        //   padTail: offset 32, size 16
        const char* kShader = R"(
        struct PadMiddle {
            a : u32,
            b : vec4<f32>,
        }
        struct PadTail {
            a : vec3<f32>,
        }
        var<immediate> padMiddle : PadMiddle;
        var<immediate> padTail : PadTail;

        @compute @workgroup_size(1)
        fn mainMiddle() {
            _ = padMiddle.b;
        }

        @compute @workgroup_size(1)
        fn mainTail() {
            _ = padTail.a;
        }

        @vertex fn vsMiddle() -> @builtin(position) vec4f {
            _ = padMiddle.b;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fsMiddle() -> @location(0) vec4f {
            _ = padMiddle.b;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @vertex fn vsTail() -> @builtin(position) vec4f {
            _ = padTail.a;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fsTail() -> @location(0) vec4f {
            _ = padTail.a;
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )";

        wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, kShader);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        auto SetImmediates = [&](auto& pass) {
            for (const auto& range : ranges) {
                if (range.size > 0) {
                    std::vector<uint32_t> data((range.size + 3) / 4, 0);
                    pass.SetImmediates(range.offset, data.data(), range.size);
                }
            }
        };

        auto CreateRenderPipeline = [&]() {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = shaderModule;
            descriptor.cFragment.module = shaderModule;

            if (entryPoint == "mainMiddle") {
                descriptor.vertex.entryPoint = "vsMiddle";
                descriptor.cFragment.entryPoint = "fsMiddle";
            } else {
                descriptor.vertex.entryPoint = "vsTail";
                descriptor.cFragment.entryPoint = "fsTail";
            }

            return device.CreateRenderPipeline(&descriptor);
        };

        switch (encoderType) {
            case EncoderType::Compute: {
                wgpu::ComputePipelineDescriptor descriptor;
                descriptor.compute.module = shaderModule;
                descriptor.compute.entryPoint = entryPoint.c_str();
                wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);

                wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
                pass.SetPipeline(pipeline);
                SetImmediates(pass);
                pass.DispatchWorkgroups(1);
                pass.End();
                break;
            }
            case EncoderType::RenderPass: {
                wgpu::RenderPipeline pipeline = CreateRenderPipeline();
                utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
                pass.SetPipeline(pipeline);
                SetImmediates(pass);
                pass.Draw(3);
                pass.End();
                break;
            }
            case EncoderType::RenderBundle: {
                wgpu::RenderPipeline pipeline = CreateRenderPipeline();
                wgpu::RenderBundleEncoderDescriptor bundleDesc;
                bundleDesc.colorFormatCount = 1;
                wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm;
                bundleDesc.colorFormats = &format;

                wgpu::RenderBundleEncoder bundleEncoder =
                    device.CreateRenderBundleEncoder(&bundleDesc);
                bundleEncoder.SetPipeline(pipeline);
                SetImmediates(bundleEncoder);
                bundleEncoder.Draw(3);

                if (success) {
                    bundleEncoder.Finish();
                } else {
                    ASSERT_DEVICE_ERROR(bundleEncoder.Finish());
                }
                return;
            }
        }

        if (success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    void RunTests(std::string entryPoint, std::vector<ImmediateDataRange> ranges, bool success) {
        TestImmediateDataValidation(entryPoint, ranges, success, GetParam());
    }
};

TEST_P(ImmediateDataRequiredTest, PadMiddleMissesA) {
    RunTests("mainMiddle", {{16, 16}}, false);
}

TEST_P(ImmediateDataRequiredTest, PadMiddleCoversAll) {
    RunTests("mainMiddle", {{0, 32}}, true);
}

TEST_P(ImmediateDataRequiredTest, PadMiddleMissesB) {
    RunTests("mainMiddle", {{0, 16}}, false);
}

TEST_P(ImmediateDataRequiredTest, PadMiddlePartialB) {
    RunTests("mainMiddle", {{16, 12}}, false);
}

TEST_P(ImmediateDataRequiredTest, PadMiddleSplitCoverage) {
    RunTests("mainMiddle", {{0, 4}, {16, 16}}, true);
}

TEST_P(ImmediateDataRequiredTest, PadTailCoversA) {
    RunTests("mainTail", {{0, 12}}, true);
}

TEST_P(ImmediateDataRequiredTest, PadTailCoversAll) {
    RunTests("mainTail", {{0, 16}}, true);
}

TEST_P(ImmediateDataRequiredTest, PadTailPartialA) {
    RunTests("mainTail", {{0, 8}}, false);
}

INSTANTIATE_TEST_SUITE_P(
    ,
    ImmediateDataRequiredTest,
    ::testing::Values(EncoderType::Compute, EncoderType::RenderPass, EncoderType::RenderBundle),
    [](const testing::TestParamInfo<ImmediateDataRequiredTest::ParamType>& info) {
        switch (info.param) {
            case EncoderType::Compute:
                return "Compute";
            case EncoderType::RenderPass:
                return "RenderPass";
            case EncoderType::RenderBundle:
                return "RenderBundle";
        }
        return "Unknown";
    });

}  // anonymous namespace
}  // namespace dawn
