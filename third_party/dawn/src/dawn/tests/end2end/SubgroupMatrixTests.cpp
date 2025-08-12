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

#include <string>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gtest/gtest.h"

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
        case wgpu::SubgroupMatrixComponentType::U8:
            return "u8";
        case wgpu::SubgroupMatrixComponentType::I8:
            return "i8";
    }
    return "<invalid>";
}

const char* ComponentTypeToScalarShaderType(wgpu::SubgroupMatrixComponentType c) {
    switch (c) {
        case wgpu::SubgroupMatrixComponentType::F32:
            return "f32";
        case wgpu::SubgroupMatrixComponentType::F16:
            return "f16";
        case wgpu::SubgroupMatrixComponentType::U32:
        case wgpu::SubgroupMatrixComponentType::U8:
            return "u32";
        case wgpu::SubgroupMatrixComponentType::I32:
        case wgpu::SubgroupMatrixComponentType::I8:
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
        case wgpu::SubgroupMatrixComponentType::U8:
        case wgpu::SubgroupMatrixComponentType::I8:
            return 1;
    }
    return 0;
}

/// A Matrix object holds the data and layout of a single matrix.
/// Provides helper functions to get and set values in different formats and to fill the matrix with
/// interesting values.
struct Matrix {
    Matrix(uint32_t c, uint32_t r, wgpu::SubgroupMatrixComponentType ct, bool colmajor)
        : cols(c),
          rows(r),
          component_type(ct),
          column_major(colmajor),
          data(new uint8_t[TotalByteSize()]) {}
    ~Matrix() { delete[] data; }

    Matrix(const Matrix&) = delete;
    Matrix& operator=(const Matrix&) = delete;

    uint32_t TotalByteSize() const { return cols * rows * ComponentTypeToByteSize(component_type); }

    void Fill(uint32_t value_offset = 0) {
        // Pick values that should not cause precision issues for small matrix multiplies.
        // Rotate through an odd number of values to catch bugs with majorness and strides.
        constexpr auto kNumValues = 9;
        constexpr float kFloatValues[kNumValues] = {
            -1.0, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1.0,
        };
        constexpr int32_t kSIntValues[kNumValues] = {
            -43, -32, -21, -10, 0, 10, 21, 32, 43,
        };
        constexpr uint32_t kUIntValues[kNumValues] = {
            0, 1, 2, 3, 11, 23, 37, 71, 101,
        };
        for (uint32_t r = 0; r < rows; r++) {
            for (uint32_t c = 0; c < cols; c++) {
                uint32_t index = (value_offset + (c + r * cols)) % kNumValues;
                switch (component_type) {
                    case wgpu::SubgroupMatrixComponentType::F16:
                    case wgpu::SubgroupMatrixComponentType::F32:
                        SetFloat(kFloatValues[index], c, r);
                        break;
                    case wgpu::SubgroupMatrixComponentType::I32:
                    case wgpu::SubgroupMatrixComponentType::I8:
                        SetInt(kSIntValues[index], c, r);
                        break;
                    case wgpu::SubgroupMatrixComponentType::U32:
                    case wgpu::SubgroupMatrixComponentType::U8:
                        SetInt(kUIntValues[index], c, r);
                        break;
                }
            }
        }
    }

    void FillWithZero() { memset(data, 0, TotalByteSize()); }

    int64_t GetInt(uint32_t c, uint32_t r) const {
        switch (component_type) {
            case wgpu::SubgroupMatrixComponentType::U32:
                return GetValue<uint32_t>(c, r);
            case wgpu::SubgroupMatrixComponentType::I32:
                return GetValue<int32_t>(c, r);
            case wgpu::SubgroupMatrixComponentType::U8:
                return GetValue<uint8_t>(c, r);
            case wgpu::SubgroupMatrixComponentType::I8:
                return GetValue<int8_t>(c, r);
            case wgpu::SubgroupMatrixComponentType::F32:
            case wgpu::SubgroupMatrixComponentType::F16:
                break;
        }
        abort();
    }

    float GetFloat(uint32_t c, uint32_t r) const {
        switch (component_type) {
            case wgpu::SubgroupMatrixComponentType::F32:
                return GetValue<float>(c, r);
            case wgpu::SubgroupMatrixComponentType::F16:
                return Float16ToFloat32(GetValue<uint16_t>(c, r));
            case wgpu::SubgroupMatrixComponentType::U32:
            case wgpu::SubgroupMatrixComponentType::I32:
            case wgpu::SubgroupMatrixComponentType::U8:
            case wgpu::SubgroupMatrixComponentType::I8:
                break;
        }
        abort();
    }

    void SetInt(int64_t value, uint32_t c, uint32_t r) {
        switch (component_type) {
            case wgpu::SubgroupMatrixComponentType::U32:
                SetValue(static_cast<uint32_t>(value), c, r);
                return;
            case wgpu::SubgroupMatrixComponentType::I32:
                SetValue(static_cast<int32_t>(value), c, r);
                return;
            case wgpu::SubgroupMatrixComponentType::U8:
                SetValue(static_cast<uint8_t>(value), c, r);
                return;
            case wgpu::SubgroupMatrixComponentType::I8:
                SetValue(static_cast<int8_t>(value), c, r);
                return;
            case wgpu::SubgroupMatrixComponentType::F32:
            case wgpu::SubgroupMatrixComponentType::F16:
                break;
        }
        abort();
    }

    void SetFloat(float value, uint32_t c, uint32_t r) {
        switch (component_type) {
            case wgpu::SubgroupMatrixComponentType::F32:
                SetValue(value, c, r);
                return;
            case wgpu::SubgroupMatrixComponentType::F16:
                SetValue(Float32ToFloat16(value), c, r);
                return;
            case wgpu::SubgroupMatrixComponentType::U32:
            case wgpu::SubgroupMatrixComponentType::I32:
            case wgpu::SubgroupMatrixComponentType::U8:
            case wgpu::SubgroupMatrixComponentType::I8:
                break;
        }
        abort();
    }

    const uint32_t cols;
    const uint32_t rows;
    const wgpu::SubgroupMatrixComponentType component_type;
    const bool column_major;
    uint8_t* const data = nullptr;

  private:
    template <typename T>
    T GetValue(uint32_t c, uint32_t r) const {
        T value;
        uint32_t index = 0;
        if (column_major) {
            index = c * rows + r;
        } else {
            index = c + r * cols;
        }
        memcpy(&value, data + index * sizeof(T), sizeof(T));
        return value;
    }

    template <typename T>
    void SetValue(T value, uint32_t c, uint32_t r) {
        uint32_t index = 0;
        if (column_major) {
            index = c * rows + r;
        } else {
            index = c + r * cols;
        }
        memcpy(data + index * sizeof(T), &value, sizeof(T));
    }
};

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
using ColumnMajor = bool;
DAWN_TEST_PARAM_STRUCT(MatrixMatrixArithmeticParams, MatrixOp, ColumnMajor);

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

    void GenerateReferenceResult(Matrix& expected,
                                 const Matrix& lhs,
                                 const Matrix& rhs,
                                 const Matrix& acc) {
        const bool is_float = expected.component_type == wgpu::SubgroupMatrixComponentType::F16 ||
                              expected.component_type == wgpu::SubgroupMatrixComponentType::F32;
        for (uint32_t r = 0; r < expected.rows; r++) {
            for (uint32_t c = 0; c < expected.cols; c++) {
                if (is_float) {
                    float ref = acc.GetFloat(c, r);
                    for (uint32_t k = 0; k < lhs.cols; k++) {
                        ref += lhs.GetFloat(k, r) * rhs.GetFloat(c, k);
                    }
                    expected.SetFloat(ref, c, r);
                } else {
                    int64_t ref = acc.GetInt(c, r);
                    for (uint32_t k = 0; k < lhs.cols; k++) {
                        ref += lhs.GetInt(k, r) * rhs.GetInt(c, k);
                    }
                    expected.SetInt(ref, c, r);
                }
            }
        }
    }
};

class SubgroupMatrix_MatrixMatrixArithmeticTest : public SubgroupMatrixArithmeticTest {
  public:
    wgpu::ComputePipeline GetComputePipelineFromSubgroupMatrixConfig(
        const wgpu::SubgroupMatrixConfig& config,
        MatrixOp op,
        uint32_t subgroupMaxSize,
        bool columnMajor) {
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
        shader << "alias InputArrayType = " << ComponentTypeToScalarShaderType(config.componentType)
               << ";\n";
        shader << "alias ResultComponentType = "
               << ComponentTypeToWgslType(config.resultComponentType) << ";\n";
        shader << "\n";
        shader << "alias LeftType = subgroup_matrix_left<ComponentType, K, M>;\n";
        shader << "alias RightType = subgroup_matrix_right<ComponentType, N, K>;\n";
        shader << "alias ResultType = subgroup_matrix_result<ResultComponentType, N, M>;\n";
        shader << "const M = " << config.M << ";\n";
        shader << "const N = " << config.N << ";\n";
        shader << "const K = " << config.K << ";\n";

        shader << "const kInputArraySize = (K*M + N*K)";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const kLoadOffset = K * M;\n";
        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(0) var<storage, read>       inputs : array<InputArrayType, kInputArraySize>;
@group(0) @binding(1) var<storage, read_write> output : array<ResultComponentType, M*N>;

@compute @workgroup_size(SubgroupMaxSize)
fn main() {
)";

        std::string loadLHS;
        std::string loadRHS;
        std::string loadAcc;
        std::string storeResult;
        if (columnMajor) {
            // When the matrix is stored in column major, the stride should be the total number of
            // rows.
            loadLHS = "let lhs = subgroupMatrixLoad<LeftType>(&inputs,  0, true, M);";
            loadRHS = "let rhs = subgroupMatrixLoad<RightType>(&inputs, kLoadOffset, true, K);";
            loadAcc = "var result = subgroupMatrixLoad<ResultType>(&output,  0, true, M);";
            storeResult = "subgroupMatrixStore(&output, 0, result, true, M);";
        } else {
            // When the matrix is stored in row major, the stride should be the total number of
            // columns.
            loadLHS = "let lhs = subgroupMatrixLoad<LeftType>(&inputs,  0, false, K);";
            loadRHS = "let rhs = subgroupMatrixLoad<RightType>(&inputs, kLoadOffset, false, N);";
            loadAcc = "var result = subgroupMatrixLoad<ResultType>(&output, 0, false, N);";
            storeResult = "subgroupMatrixStore(&output, 0, result, false, N);";
        }

        shader << loadLHS << "\n" << loadRHS << "\n";

        switch (op) {
            case MatrixMultiply:
                shader << "let result = subgroupMatrixMultiply<ResultComponentType>(lhs, rhs);"
                       << "\n";
                break;
            case MatrixMultiplyAccumulate:
                shader << loadAcc << "\n"
                       << "result = subgroupMatrixMultiplyAccumulate(lhs, rhs, result);" << "\n";
                break;
        }

        shader << storeResult << "\n}";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        return device.CreateComputePipeline(&csDesc);
    }

    void TestSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config,
                                  MatrixOp op,
                                  uint32_t subgroupMaxSize,
                                  bool columnMajor) {
        uint32_t resultComponentByteSize = ComponentTypeToByteSize(config.resultComponentType);

        // Generate a compute pipeline that performs a matrix multiplication that matches the
        // config.
        wgpu::ComputePipeline pipeline =
            GetComputePipelineFromSubgroupMatrixConfig(config, op, subgroupMaxSize, columnMajor);

        // Create the input matrices and fill them with values.
        Matrix inputLHS(config.K, config.M, config.componentType, columnMajor);
        Matrix inputRHS(config.N, config.K, config.componentType, columnMajor);
        Matrix acc(config.N, config.M, config.resultComponentType, columnMajor);
        // Offset the values for each matrix so that they are all different.
        inputLHS.Fill(0);
        inputRHS.Fill(1);
        if (op == MatrixMultiplyAccumulate) {
            acc.Fill(3);
        } else {
            // If we are not accumulating then treat it as if the accumulator is zero.
            acc.FillWithZero();
        }

        // Create the input buffer and copy the input matrices to it.
        wgpu::BufferDescriptor inputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = inputLHS.TotalByteSize() + inputRHS.TotalByteSize(),
            .mappedAtCreation = true,
        };
        wgpu::Buffer inputs = device.CreateBuffer(&inputDescriptor);
        memcpy(inputs.GetMappedRange(), inputLHS.data, inputLHS.TotalByteSize());
        memcpy(static_cast<uint8_t*>(inputs.GetMappedRange()) + inputLHS.TotalByteSize(),
               inputRHS.data, inputRHS.TotalByteSize());
        inputs.Unmap();

        // Create the output buffer and copy the accumulator to it.
        wgpu::BufferDescriptor outputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = config.M * config.N * resultComponentByteSize,
            .mappedAtCreation = true,
        };
        wgpu::Buffer output = device.CreateBuffer(&outputDescriptor);
        memcpy(output.GetMappedRange(), acc.data, acc.TotalByteSize());
        output.Unmap();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, inputs}, {1, output}});
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Verify the result against a reference implementation.
        Matrix expected(config.N, config.M, config.resultComponentType, columnMajor);
        GenerateReferenceResult(expected, inputLHS, inputRHS, acc);
        EXPECT_BUFFER_U8_RANGE_EQ(expected.data, output, 0, expected.TotalByteSize());
    }
};

TEST_P(SubgroupMatrix_MatrixMatrixArithmeticTest, MatrixMultiply) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    MatrixOp op = GetParam().mMatrixOp;
    bool columnMajor = GetParam().mColumnMajor;

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    // Test each supported config.
    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        auto& config = subgroupMatrixConfigs.configs[i];

        std::stringstream configInfo;
        configInfo << "Testing " << config.M << "x" << config.N << "x" << config.K << " "
                   << ComponentTypeToWgslType(config.componentType) << " -> "
                   << ComponentTypeToWgslType(config.resultComponentType);
        SCOPED_TRACE(configInfo.str());

        TestSubgroupMatrixConfig(config, op, info.subgroupMaxSize, columnMajor);
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixMatrixArithmeticTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend({"use_vulkan_memory_model"}),
                        },
                        {
                            // MatrixOp
                            MatrixOp::MatrixMultiply,
                            MatrixOp::MatrixMultiplyAccumulate,
                        },
                        {
                            // In column-major or not
                            true,
                            false,
                        });

using InputColumnMajor = bool;
DAWN_TEST_PARAM_STRUCT(MatrixStoreParams, InputColumnMajor);
class SubgroupMatrix_MatrixStoreTest : public DawnTestWithParams<MatrixStoreParams> {
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

    wgpu::ComputePipeline GetComputePipelineFromSubgroupMatrixConfig(
        const wgpu::SubgroupMatrixConfig& config,
        uint32_t subgroupMaxSize,
        bool inputColumnMajor) {
        // Generate a shader that stores a subgroup matrix into a storage buffer.
        std::ostringstream shader;
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::F16 ||
            config.resultComponentType == wgpu::SubgroupMatrixComponentType::F16) {
            shader << "enable f16;\n";
        }
        shader << "\n";
        shader << "alias ComponentType = " << ComponentTypeToWgslType(config.componentType)
               << ";\n";
        shader << "alias ArrayType = " << ComponentTypeToScalarShaderType(config.componentType)
               << ";\n\n";
        shader << "alias InputType = subgroup_matrix_left<ComponentType, K, M>;\n";
        shader << "const K = " << config.K << ";\n";
        shader << "const M = " << config.M << ";\n";

        shader << "const kStoreOffset = K * M;\n";

        shader << "const kInputArraySize = kStoreOffset";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(0) var<storage, read>       input : array<ArrayType, kInputArraySize>;
@group(0) @binding(1) var<storage, read_write> output : array<ArrayType, kInputArraySize * 2>;

@compute @workgroup_size(SubgroupMaxSize)
fn main() {
)";

        std::string loadInput;
        std::string storeResult;
        if (inputColumnMajor) {
            // When the matrix is stored in column major, the stride should be the total number of
            // rows.
            loadInput = "let input_matrix = subgroupMatrixLoad<InputType>(&input, 0, true, M);";
            storeResult = "subgroupMatrixStore(&output, kStoreOffset, input_matrix, true, M);";
        } else {
            // When the matrix is stored in row major, the stride should be the total number of
            // columns.
            loadInput = "let input_matrix = subgroupMatrixLoad<InputType>(&input,  0, false, K);";
            storeResult = "subgroupMatrixStore(&output, kStoreOffset, input_matrix, false, K);";
        }

        shader << loadInput << "\n" << storeResult << "\n\n}";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        return device.CreateComputePipeline(&csDesc);
    }

    void TestSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config,
                                  uint32_t subgroupMaxSize,
                                  bool inputColumnMajor) {
        // In the tests we use a compute pipeline to store a subgroup matrix into a storage buffer
        // and check if the data in the buffer matches the expectation.
        wgpu::ComputePipeline pipeline =
            GetComputePipelineFromSubgroupMatrixConfig(config, subgroupMaxSize, inputColumnMajor);

        // Create the input matrix and fill it with values.
        Matrix inputMatrix(config.K, config.M, config.componentType, inputColumnMajor);
        inputMatrix.Fill(0);

        // Create the input buffer and copy the input matrix to it.
        wgpu::BufferDescriptor inputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = inputMatrix.TotalByteSize(),
            .mappedAtCreation = true,
        };
        wgpu::Buffer inputBuffer = device.CreateBuffer(&inputDescriptor);
        memcpy(inputBuffer.GetMappedRange(), inputMatrix.data, inputMatrix.TotalByteSize());
        inputBuffer.Unmap();

        uint64_t storeOffset = inputMatrix.TotalByteSize();

        // Create the output buffer.
        wgpu::BufferDescriptor outputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = inputMatrix.TotalByteSize() + storeOffset,
        };
        wgpu::Buffer output = device.CreateBuffer(&outputDescriptor);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, inputBuffer}, {1, output}});
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Verify the result in the output buffer.
        std::vector<uint8_t> zeroBuffer(storeOffset, static_cast<uint8_t>(0));
        EXPECT_BUFFER_U8_RANGE_EQ(zeroBuffer.data(), output, 0, storeOffset);
        EXPECT_BUFFER_U8_RANGE_EQ(inputMatrix.data, output, storeOffset,
                                  inputMatrix.TotalByteSize());
    }
};

TEST_P(SubgroupMatrix_MatrixStoreTest, MatrixStoreWithOffset) {
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

        std::stringstream configInfo;
        configInfo << "Testing " << config.M << "x" << config.N << "x" << config.K << " "
                   << ComponentTypeToWgslType(config.componentType) << " -> "
                   << ComponentTypeToWgslType(config.resultComponentType);
        SCOPED_TRACE(configInfo.str());

        TestSubgroupMatrixConfig(config, info.subgroupMaxSize, GetParam().mInputColumnMajor);
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixStoreTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend({"use_vulkan_memory_model"}),
                        },
                        {
                            // Input matrix is in column-major or not
                            true,
                            false,
                        });

}  // anonymous namespace
}  // namespace dawn
