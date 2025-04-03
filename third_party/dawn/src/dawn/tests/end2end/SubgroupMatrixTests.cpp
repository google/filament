// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

const char* ComponentTypeToWgslType(wgpu::SubgroupMatrixComponentType c) {
    switch (c) {
        case wgpu::SubgroupMatrixComponentType::F32:
            return "f32";
        case wgpu::SubgroupMatrixComponentType::F16:
            return "f16";
        case wgpu::SubgroupMatrixComponentType::U32:
            return "u32";
        case wgpu::SubgroupMatrixComponentType::I32:
            return "i32";
    }
    return "<invalid>";
}

uint32_t ComponentTypeToByteSize(wgpu::SubgroupMatrixComponentType c) {
    switch (c) {
        case wgpu::SubgroupMatrixComponentType::F32:
        case wgpu::SubgroupMatrixComponentType::U32:
        case wgpu::SubgroupMatrixComponentType::I32:
            return 4;
        case wgpu::SubgroupMatrixComponentType::F16:
            return 2;
    }
    return 0;
}

class SubgroupMatrixTest : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix})) {
            features.push_back(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix);
        }
        if (SupportsFeatures({wgpu::FeatureName::ShaderF16})) {
            features.push_back(wgpu::FeatureName::ShaderF16);
        }
        return features;
    }
};

// Test that it is only valid to request the AdapterPropertiesSubgroupMatrixConfigs structure if the
// feature is available.
TEST_P(SubgroupMatrixTest, QueryConfigsOnlyValidWithFeature) {
    auto expected = adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix)
                        ? wgpu::Status::Success
                        : wgpu::Status::Error;
    {
        wgpu::AdapterInfo info;
        wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
        info.nextInChain = &subgroupMatrixConfigs;

        EXPECT_EQ(adapter.GetInfo(&info), expected);
    }
    {
        wgpu::AdapterInfo adapterInfo;
        wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
        adapterInfo.nextInChain = &subgroupMatrixConfigs;

        EXPECT_EQ(device.GetAdapterInfo(&adapterInfo), expected);
    }
}

// Test that Dawn validates the X-dimension of the workgroup size when subgroup matrices are used,
// such that it must be a multiple of the maximum subgroup size.
// The valid edge cases (where it is exactly the same as the maximum subgroup size) are tested in
// the arithmetic tests below.
TEST_P(SubgroupMatrixTest, WorkgroupSizeXMustBeMultipleOfMaxSubgroupSize) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    // Test each supported config.
    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        auto& config = subgroupMatrixConfigs.configs[i];

        std::ostringstream shader;
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        if (config.resultComponentType == wgpu::SubgroupMatrixComponentType::F16) {
            shader << "enable f16;\n";
        }
        shader << "alias ResultComponentType = "
               << ComponentTypeToWgslType(config.resultComponentType) << ";\n";
        shader << "\n";
        shader << "const M = " << config.M << ";\n";
        shader << "const N = " << config.N << ";\n";
        shader << "const SubgroupMaxSize = " << info.subgroupMaxSize << ";\n";
        shader << R"(
@compute @workgroup_size(SubgroupMaxSize / 2, 2)
fn main() {
    _ = subgroup_matrix_result<ResultComponentType, N, M>();
})";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());

        std::stringstream err;
        err << "The x-dimension of workgroup_size (" << (info.subgroupMaxSize / 2)
            << ") must be a multiple of the device maxSubgroupSize";
        ASSERT_DEVICE_ERROR_MSG(device.CreateComputePipeline(&csDesc),
                                testing::HasSubstr(err.str()));
    }
}

DAWN_INSTANTIATE_TEST(SubgroupMatrixTest,
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend({"use_vulkan_memory_model"}));

enum MatrixOp {
    MatrixMultiply,
    MatrixMultiplyAccumulate,
};
DAWN_TEST_PARAM_STRUCT(MatrixMatrixArithmeticParams, MatrixOp);

class SubgroupMatrixArithmeticTest : public DawnTestWithParams<MatrixMatrixArithmeticParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix})) {
            features.push_back(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix);
        }
        if (SupportsFeatures({wgpu::FeatureName::ShaderF16})) {
            features.push_back(wgpu::FeatureName::ShaderF16);
        }
        return features;
    }
};

using SubgroupMatrix_MatrixMatrixArithmeticTest = SubgroupMatrixArithmeticTest;
TEST_P(SubgroupMatrix_MatrixMatrixArithmeticTest, MatrixMultiply) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    MatrixOp op = GetParam().mMatrixOp;

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    // Test each supported config.
    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        auto& config = subgroupMatrixConfigs.configs[i];
        uint32_t componentByteSize = ComponentTypeToByteSize(config.componentType);
        uint32_t resultComponentByteSize = ComponentTypeToByteSize(config.resultComponentType);

        // Generate a shader that performs a matrix multiplication that matches the config.
        std::ostringstream shader;
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::F16 ||
            config.resultComponentType == wgpu::SubgroupMatrixComponentType::F16) {
            shader << "enable f16;\n";
        }
        shader << "\n";
        shader << "alias ComponentType = " << ComponentTypeToWgslType(config.componentType)
               << ";\n";
        shader << "alias ResultComponentType = "
               << ComponentTypeToWgslType(config.resultComponentType) << ";\n";
        shader << "\n";
        shader << "const M = " << config.M << ";\n";
        shader << "const N = " << config.N << ";\n";
        shader << "const K = " << config.K << ";\n";
        shader << "const SubgroupMaxSize = " << info.subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(0) var<storage, read>       inputs : array<ComponentType, K*M + N*K>;
@group(0) @binding(1) var<storage, read_write> output : array<ResultComponentType, M*N>;

@compute @workgroup_size(SubgroupMaxSize)
fn main() {
    let lhs = subgroupMatrixLoad<subgroup_matrix_left<ComponentType, K, M>>(&inputs,  0, true, M);
    let rhs = subgroupMatrixLoad<subgroup_matrix_right<ComponentType, N, K>>(&inputs, K*M, true, K);
)";
        switch (op) {
            case MatrixMultiply:
                shader << "let result = subgroupMatrixMultiply<ResultComponentType>(lhs, rhs);\n";
                break;
            case MatrixMultiplyAccumulate:
                // Perform the multiplication twice, accumulating into a zero matrix the first time.
                shader << "let zero = subgroup_matrix_result<ResultComponentType, N, M>();\n";
                shader << "var result = subgroupMatrixMultiplyAccumulate(lhs, rhs, zero);\n";
                shader << "result = subgroupMatrixMultiplyAccumulate(lhs, rhs, result);\n";
                break;
        }
        shader << R"(
    subgroupMatrixStore(&output, 0, result, true, M);
})";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        // Convert the matrix multiplication result value to the result component type.
        auto toResultType = [&](auto value) -> uint32_t {
            switch (config.resultComponentType) {
                case wgpu::SubgroupMatrixComponentType::F32: {
                    float valueF32 = static_cast<float>(value);
                    return *reinterpret_cast<uint32_t*>(&valueF32);
                }
                case wgpu::SubgroupMatrixComponentType::F16: {
                    uint16_t valueF16 = Float32ToFloat16(static_cast<float>(value));
                    return (uint32_t(valueF16) << 16) | valueF16;
                }
                case wgpu::SubgroupMatrixComponentType::U32:
                    return uint32_t(value);
                case wgpu::SubgroupMatrixComponentType::I32:
                    int32_t valueI32 = static_cast<int32_t>(value);
                    return *reinterpret_cast<uint32_t*>(&valueI32);
            }
            return 0;
        };

        // Generate the value to fill the input matrices with as a 32-bit word, and generate the
        // corresponding output value as well. Pack multiple copies of the value together if the
        // size of the input component type is less than 32 bits.
        uint32_t inputValue;
        uint32_t outputValue;
        switch (config.componentType) {
            case wgpu::SubgroupMatrixComponentType::F32: {
                float in = 0.5;
                float out = in * in * config.K;
                if (op == MatrixMultiplyAccumulate) {
                    out *= 2;
                }
                inputValue = *reinterpret_cast<uint32_t*>(&in);
                outputValue = toResultType(out);
                break;
            }
            case wgpu::SubgroupMatrixComponentType::F16: {
                float inF32 = 0.5;
                uint16_t in = Float32ToFloat16(inF32);
                float out = inF32 * inF32 * config.K;
                if (op == MatrixMultiplyAccumulate) {
                    out *= 2;
                }
                inputValue = (uint32_t(in) << 16) | in;
                outputValue = toResultType(out);
                break;
            }
            case wgpu::SubgroupMatrixComponentType::U32:
            case wgpu::SubgroupMatrixComponentType::I32: {
                uint32_t in = 2;
                uint32_t out = in * in * config.K;
                if (op == MatrixMultiplyAccumulate) {
                    out *= 2;
                }
                inputValue = in;
                outputValue = toResultType(out);
                break;
            }
        }

        uint32_t numInputElements = (config.M + config.N) * config.K;
        std::vector<uint32_t> inValues(numInputElements * componentByteSize / 4, inputValue);
        std::vector<uint32_t> expected(config.M * config.N * resultComponentByteSize / 4,
                                       outputValue);
        wgpu::Buffer inputs = utils::CreateBufferFromData(
            device, inValues.data(), inValues.size() * 4, wgpu::BufferUsage::Storage);

        wgpu::BufferDescriptor outputDescriptor;
        outputDescriptor.size = config.M * config.N * resultComponentByteSize;
        outputDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage;
        wgpu::Buffer output = device.CreateBuffer(&outputDescriptor);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, inputs}, {1, output}});

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroups(1);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixMatrixArithmeticTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend({"use_vulkan_memory_model"}),
                        },
                        {
                            MatrixOp::MatrixMultiply,
                            MatrixOp::MatrixMultiplyAccumulate,
                        });

}  // anonymous namespace
}  // namespace dawn
