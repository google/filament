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

class ImmediateDataDisableTest : public ValidationTest {};

// Check that creating a PipelineLayout with non-zero immediateSize is disallowed
// without the feature enabled.
TEST_F(ImmediateDataDisableTest, ImmediateSizeNotAllowed) {
    wgpu::PipelineLayoutDescriptor desc;
    desc.bindGroupLayoutCount = 0;
    desc.immediateSize = 1;

    ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
}

// Check that SetImmediateData doesn't work (even with size=0) without maxImmediateSize>0.
TEST_F(ImmediateDataDisableTest, SetImmediateData) {
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetImmediateData(0, nullptr, 0);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
    {
        const uint32_t data = 0;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetImmediateData(0, &data, 4);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

class ImmediateDataTest : public ValidationTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        required.maxImmediateSize = kDefaultMaxImmediateDataBytes;
    }

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
    wgpu::PipelineLayoutDescriptor desc;
    desc.bindGroupLayoutCount = 0;

    // Success case with valid immediateSize.
    {
        desc.immediateSize = kDefaultMaxImmediateDataBytes;
        device.CreatePipelineLayout(&desc);
    }

    // Failed case with invalid immediateSize that exceed limits.
    {
        desc.immediateSize = kDefaultMaxImmediateDataBytes + 1;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
}

// Check that SetImmediateData offset and length must be aligned to 4 bytes.
TEST_F(ImmediateDataTest, ValidateSetImmediateDataAlignment) {
    // Success cases
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t data = 0;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(0, &data, 4);
        computePass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(4, nullptr, 0);
        computePass.End();
        encoder.Finish();
    }

    // Failed case with non-aligned offset bytes
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(2, nullptr, 0);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed cases with non-aligned size
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint8_t data = 0;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(0, &data, 2);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that SetImmediateData offset + length must be in bound.
TEST_F(ImmediateDataTest, ValidateSetImmediateDataOOB) {
    // Success cases
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        std::vector<uint32_t> data(kDefaultMaxImmediateDataBytes / 4, 0);
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(0, data.data(), kDefaultMaxImmediateDataBytes);
        computePass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(kDefaultMaxImmediateDataBytes, nullptr, 0);
        computePass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t data = 0;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(kDefaultMaxImmediateDataBytes - 4, &data, 4);
        computePass.End();
        encoder.Finish();
    }

    // Failed case with offset oob
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t offset = kDefaultMaxImmediateDataBytes + 4;
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(offset, nullptr, 0);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed cases with size oob
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t size = kDefaultMaxImmediateDataBytes + 4;
        std::vector<uint32_t> data(size / 4, 0);
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(0, data.data(), size);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed cases with offset + size oob
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t offset = kDefaultMaxImmediateDataBytes;
        uint32_t data[] = {0};
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(offset, data, 4);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failed case with super large offset + size oob but looping back to zero
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        uint32_t offset = std::numeric_limits<uint32_t>::max() - 3;
        uint32_t data[] = {0};
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.SetImmediateData(offset, data, 4);
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that pipelineLayout immediate data bytes compatible with shaders.
TEST_F(ImmediateDataTest, ValidatePipelineLayoutImmediateDataBytesAndShaders) {
    constexpr uint32_t kShaderImmediateDataBytes = 12u;
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_immediate;
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
        pipelineDescriptor.cFragment.targetCount = 1;
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
        pipelineDescriptor.cFragment.targetCount = 1;
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
        pipelineDescriptor.cFragment.targetCount = 1;
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
        enable chromium_experimental_immediate;
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
            return fragmentConstants;
        }

        @group(0) @binding(0) var<storage, read_write> output : vec4u;

        @compute @workgroup_size(1, 1, 1)
        fn csMain() {
            output = computeConstants;
        })");

    wgpu::ShaderModule oobShaderModule = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_immediate;
            struct FragmentConstants {
                constants: vec4f,
                constantsOOB: f32,
            };

            struct ComputeConstants {
                constants: vec4u,
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
                return vec4f(fragmentConstants.constants.x + fragmentConstants.constantsOOB,
                             fragmentConstants.constants.yzw);
            }

            @group(0) @binding(0) var<storage, read_write> output : vec4u;

            @compute @workgroup_size(1, 1, 1)
            fn csMain() {
                output = vec4u(computeConstants.constants.x + computeConstants.constantsOOB,
                               computeConstants.constants.yzw);
            })");

    // Success cases
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = shaderModule;
        pipelineDescriptor.cFragment.module = shaderModule;
        pipelineDescriptor.cFragment.targetCount = 1;
        device.CreateRenderPipeline(&pipelineDescriptor);
    }

    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = shaderModule;

        device.CreateComputePipeline(&csDesc);
    }

    // Using too many immediate data cases
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = oobShaderModule;
        pipelineDescriptor.cFragment.module = oobShaderModule;
        pipelineDescriptor.cFragment.targetCount = 1;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pipelineDescriptor));
    }

    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = oobShaderModule;

        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }
}

}  // anonymous namespace
}  // namespace dawn
