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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr bool Is8Bit(wgpu::SubgroupMatrixComponentType c) {
    return c == wgpu::SubgroupMatrixComponentType::U8 || c == wgpu::SubgroupMatrixComponentType::I8;
}

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

std::ostream& operator<<(std::ostream& o, const wgpu::SubgroupMatrixConfig& config) {
    o << config.M << "x" << config.N << "x" << config.K << " "
      << ComponentTypeToWgslType(config.componentType) << " -> "
      << ComponentTypeToWgslType(config.resultComponentType);
    return o;
}

bool IsEqual(const wgpu::SubgroupMatrixConfig& lhs, const wgpu::SubgroupMatrixConfig& rhs) {
    return lhs.componentType == rhs.componentType &&              //
           lhs.resultComponentType == rhs.resultComponentType &&  //
           lhs.M == rhs.M &&                                      //
           lhs.N == rhs.N &&                                      //
           lhs.K == rhs.K;
}

bool CrashesOnRX9060XT(const wgpu::SubgroupMatrixConfig& config, bool include8BitTypes) {
    // TODO(crbug.com/525518027): These configs are crashing in the AMD driver during
    // ID3D12Device::CreateComputePipelineState.
    // AMD driver 32.0.31007.2036, 6/4/2026, AMD Agility SDK installer 26.10.07.02
    // Note that these configs are not exhaustive; for example, these work:
    // 16x16x16 f16 -> f16
    // 16x16x16 f16 -> f32
    // 16x16x16 i8 -> i32
    // 16x16x16 i8 -> u32
    // 16x16x16 u8 -> i32
    // 16x16x16 u8 -> u32
    using enum wgpu::SubgroupMatrixComponentType;
    bool crashes = IsEqual(config, {I32, I32, 16, 16, 16}) ||  //
                   IsEqual(config, {U32, U32, 16, 16, 16}) ||  //
                   IsEqual(config, {I32, U32, 16, 16, 16}) ||  //
                   IsEqual(config, {U32, I32, 16, 16, 16}) ||  //
                   IsEqual(config, {F32, F32, 16, 16, 16});
    if (include8BitTypes) {
        crashes = crashes ||                                //
                  IsEqual(config, {I8, I8, 16, 16, 16}) ||  //
                  IsEqual(config, {I8, U8, 16, 16, 16}) ||  //
                  IsEqual(config, {U8, I8, 16, 16, 16}) ||  //
                  IsEqual(config, {U8, U8, 16, 16, 16});
    }
    return crashes;
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
        constexpr std::array<float, kNumValues> kFloatValues = {
            -1.0, -0.75, -0.5, -0.25, 0, 0.25, 0.5, 0.75, 1.0,
        };
        constexpr std::array<int32_t, kNumValues> kSIntValues = {
            -43, -32, -21, -10, 0, 10, 21, 32, 43,
        };
        constexpr std::array<uint32_t, kNumValues> kUIntValues = {
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
    // TODO(crbug.com/485825675): Investigate why this pointer is dangling.
    const raw_ptr<uint8_t, DanglingUntriaged | AllowPtrArithmetic> data = nullptr;

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

void GenerateReferenceMatrixMultiply(Matrix& expected,
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
        if (SupportsFeatures({wgpu::FeatureName::SubgroupSizeControl})) {
            features.push_back(wgpu::FeatureName::SubgroupSizeControl);
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

// Test that if the feature is enabled, querying returns non-zero configs
TEST_P(SubgroupMatrixTest, QueryConfigsMustReturnNonZeroConfigs) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    ASSERT_NE(subgroupMatrixConfigs.configCount, 0u);
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

DAWN_INSTANTIATE_TEST(SubgroupMatrixTest, D3D12Backend(), MetalBackend(), VulkanBackend());

class SubgroupMatrixSubgroupSizeControlTest : public SubgroupMatrixTest {};

// Test that an explicit subgroup size, rather than the device maximum subgroup size, is used to
// validate the workgroup size of an entry point that uses subgroup matrices.
TEST_P(SubgroupMatrixSubgroupSizeControlTest, WorkgroupSizeUsesExplicitSubgroupSize) {
    DAWN_TEST_UNSUPPORTED_IF(
        !device.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix) ||
        !device.HasFeature(wgpu::FeatureName::SubgroupSizeControl));

    // TODO(crbug.com/492539239): Access violation during test teardown.
    DAWN_SUPPRESS_TEST_IF(IsWindows11() && IsAMD() && IsVulkan());

    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(device.GetAdapterInfo(&info), wgpu::Status::Success);

    // A size smaller than the maximum is needed to distinguish explicit-subgroup-size validation
    // from the default maximum-subgroup-size validation.
    DAWN_TEST_UNSUPPORTED_IF(info.subgroupMinSize == info.subgroupMaxSize);
    const uint32_t subgroupSize = info.subgroupMaxSize / 2;
    DAWN_TEST_UNSUPPORTED_IF(subgroupSize < info.subgroupMinSize);

    // Intel Gen12 cannot use subgroup size 8 on D3D12 despite advertising it as the minimum.
    DAWN_TEST_UNSUPPORTED_IF(IsD3D12() && IsIntelGen12() && subgroupSize == 8);

    bool testedConfig = false;
    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        const auto& config = subgroupMatrixConfigs.configs[i];
        if (IsWindows() && IsAMD() && IsD3D12() && CrashesOnRX9060XT(config, false)) {
            continue;
        }

        std::ostringstream configTrace;
        configTrace << config;
        SCOPED_TRACE(configTrace.str());
        testedConfig = true;

        std::ostringstream shader;
        shader << "enable subgroups;\n";
        shader << "enable subgroup_size_control;\n";
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        if (config.resultComponentType == wgpu::SubgroupMatrixComponentType::F16) {
            shader << "enable f16;\n";
        }
        shader << "alias ResultComponentType = "
               << ComponentTypeToWgslType(config.resultComponentType) << ";\n";
        shader << "const M = " << config.M << ";\n";
        shader << "const N = " << config.N << ";\n";
        shader << "const SubgroupSize = " << subgroupSize << ";\n";
        shader << R"(
@compute @workgroup_size(SubgroupSize) @subgroup_size(SubgroupSize)
fn main() {
    _ = subgroup_matrix_result<ResultComponentType, N, M>();
})";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        device.CreateComputePipeline(&csDesc);
    }
    DAWN_TEST_UNSUPPORTED_IF(!testedConfig);
}

DAWN_INSTANTIATE_TEST(SubgroupMatrixSubgroupSizeControlTest,
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());

enum MatrixOp {
    MatrixMultiply,
    MatrixMultiplyAccumulate,
    MatrixScalarAdd,
    MatrixScalarSubtract,
    MatrixScalarMultiply,
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
        if (SupportsFeatures({wgpu::FeatureName::Subgroups})) {
            features.push_back(wgpu::FeatureName::Subgroups);
        }
        return features;
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
        shader << "enable subgroups;\n";
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
        shader << "alias ResultArrayType = "
               << ComponentTypeToScalarShaderType(config.resultComponentType) << ";\n";
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

        shader << "const kResultArraySize = (M*N)";
        if (config.resultComponentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.resultComponentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const kLoadOffset = K * M";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";
        shader << "const kLeftStride = " << (columnMajor ? "M" : "K");
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";
        shader << "const kRightStride = " << (columnMajor ? "K" : "N");
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";
        shader << "const kResultStride = " << (columnMajor ? "M" : "N");
        if (config.resultComponentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.resultComponentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";
        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(0) var<storage, read>       inputs : array<InputArrayType, kInputArraySize>;
@group(0) @binding(1) var<storage, read_write> output : array<ResultArrayType, kResultArraySize>;

@compute @workgroup_size(SubgroupMaxSize)
fn main(@builtin(subgroup_id) sgid: u32) {
if sgid != 0 {
  return;
}
)";
        std::string major = columnMajor ? "col_major" : "row_major";
        std::string loadLHS =
            "let lhs = subgroupMatrixLoad<LeftType, " + major + ">(&inputs, 0, kLeftStride);";
        std::string loadRHS = "let rhs = subgroupMatrixLoad<RightType, " + major +
                              ">(&inputs, kLoadOffset, kRightStride);";
        std::string loadAcc = "var result = subgroupMatrixLoad<ResultType, " + major +
                              ">(&output, 0, kResultStride);";
        std::string storeResult =
            "subgroupMatrixStore<" + major + ">(&output, 0, result, kResultStride);";

        shader << loadLHS << "\n" << loadRHS << "\n";

        if (op == MatrixMultiply) {
            shader << "let result = subgroupMatrixMultiply<ResultComponentType>(lhs, rhs);"
                   << "\n";
        } else if (op == MatrixMultiplyAccumulate) {
            shader << loadAcc << "\n"
                   << "result = subgroupMatrixMultiplyAccumulate(lhs, rhs, result);" << "\n";
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
            .size = static_cast<uint64_t>(config.M) * config.N * resultComponentByteSize,
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
        GenerateReferenceMatrixMultiply(expected, inputLHS, inputRHS, acc);
        EXPECT_BUFFER_U8_RANGE_EQ(expected.data, output, 0, expected.TotalByteSize()) << config;
    }
};

TEST_P(SubgroupMatrix_MatrixMatrixArithmeticTest, MatrixMultiply) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));
    // TODO(crbug.com/492539239): Access violation during test teardown.
    DAWN_SUPPRESS_TEST_IF(IsWindows11() && IsAMD() && IsVulkan());

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

        if (IsWindows() && IsAMD() && IsD3D12()) {
            if (CrashesOnRX9060XT(config, true)) {
                std::cout << "Skipping config: " << config << "\n";
                continue;
            }
        }

        TestSubgroupMatrixConfig(config, op, info.subgroupMaxSize, columnMajor);
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixMatrixArithmeticTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend(),
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

class SubgroupMatrix_MatrixScalarArithmeticTest : public SubgroupMatrixArithmeticTest {
  public:
    wgpu::ComputePipeline GetComputePipelineFromSubgroupMatrixConfig(
        const wgpu::SubgroupMatrixConfig& config,
        MatrixOp op,
        uint32_t subgroupMaxSize,
        bool columnMajor) {
        // Generate a shader that performs a matrix scalar operation that matches the config.
        std::ostringstream shader;
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        shader << "enable subgroups;\n";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::F16 ||
            config.resultComponentType == wgpu::SubgroupMatrixComponentType::F16) {
            shader << "enable f16;\n";
        }
        shader << "\n";
        shader << "alias ComponentType = " << ComponentTypeToWgslType(config.componentType)
               << ";\n";
        shader << "alias ScalarShaderType = "
               << ComponentTypeToScalarShaderType(config.componentType) << ";\n";
        shader << "\n";
        shader << "const M = " << config.M << ";\n";
        shader << "const K = " << config.K << ";\n";
        shader << "alias LeftType = subgroup_matrix_left<ComponentType, K, M>;\n";

        shader << "const kMatrixDataSize = (K * M)";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const kStride = " << (columnMajor ? "M" : "K");
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(0) var<storage, read>       inputs : array<ScalarShaderType, kMatrixDataSize>;
@group(0) @binding(1) var<storage, read_write> output : array<ScalarShaderType, kMatrixDataSize>;

@compute @workgroup_size(SubgroupMaxSize)
fn main(@builtin(subgroup_id) sgid: u32) {
if sgid != 0 {
  return;
}
)";

        const std::string major = columnMajor ? "col_major" : "row_major";
        std::string loadLHS =
            "let lhs = subgroupMatrixLoad<LeftType, " + major + ">(&inputs, 0, kStride);";
        std::string storeResult =
            "subgroupMatrixStore<" + major + ">(&output, 0, result, kStride);";

        shader << loadLHS << "\n";

        if (op == MatrixScalarAdd) {
            shader << "let result = subgroupMatrixScalarAdd(lhs, 5);\n";
        } else if (op == MatrixScalarSubtract) {
            shader << "let result = subgroupMatrixScalarSubtract(lhs, 5);\n";
        } else if (op == MatrixScalarMultiply) {
            shader << "let result = subgroupMatrixScalarMultiply(lhs, 5);\n";
        }

        shader << storeResult << "\n}";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        return device.CreateComputePipeline(&csDesc);
    }

    void GenerateReferenceResult(Matrix& expected, const Matrix& lhs, MatrixOp op) {
        const bool is_float = expected.component_type == wgpu::SubgroupMatrixComponentType::F16 ||
                              expected.component_type == wgpu::SubgroupMatrixComponentType::F32;
        for (uint32_t r = 0; r < expected.rows; r++) {
            for (uint32_t c = 0; c < expected.cols; c++) {
                if (is_float) {
                    auto lval = lhs.GetFloat(c, r);
                    float ref = lval;

                    if (op == MatrixOp::MatrixScalarAdd) {
                        ref += 5.f;
                    } else if (op == MatrixOp::MatrixScalarSubtract) {
                        ref -= 5.f;
                    } else if (op == MatrixOp::MatrixScalarMultiply) {
                        ref *= 5.f;
                    }

                    expected.SetFloat(ref, c, r);
                } else {
                    int64_t ref = lhs.GetInt(c, r);

                    if (op == MatrixOp::MatrixScalarAdd) {
                        ref += 5;
                    } else if (op == MatrixOp::MatrixScalarSubtract) {
                        ref -= 5;
                    } else if (op == MatrixOp::MatrixScalarMultiply) {
                        ref *= 5;
                    }

                    expected.SetInt(ref, c, r);
                }
            }
        }
    }

    void TestSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config,
                                  MatrixOp op,
                                  uint32_t subgroupMaxSize,
                                  bool columnMajor) {
        // TODO(crbug.com/512455646): Fix shader to support 8-bit component type
        if (Is8Bit(config.componentType)) {
            std::cout << "Skipping componentType: " << ComponentTypeToWgslType(config.componentType)
                      << "\n";
            return;
        }

        uint32_t componentByteSize = ComponentTypeToByteSize(config.componentType);

        // Generate a compute pipeline that performs a matrix scalar operation that matches the
        // config.
        wgpu::ComputePipeline pipeline =
            GetComputePipelineFromSubgroupMatrixConfig(config, op, subgroupMaxSize, columnMajor);

        // Create the input matrices and fill them with values.
        Matrix inputLHS(config.K, config.M, config.componentType, columnMajor);
        inputLHS.Fill();

        // Create the input buffer and copy the input matrices to it.
        wgpu::BufferDescriptor inputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = std::max(inputLHS.TotalByteSize(), 512u),
            .mappedAtCreation = true,
        };
        wgpu::Buffer inputs = device.CreateBuffer(&inputDescriptor);
        memcpy(inputs.GetMappedRange(), inputLHS.data, inputLHS.TotalByteSize());
        inputs.Unmap();

        // Create the output buffer
        wgpu::BufferDescriptor outputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = static_cast<uint64_t>(config.K) * config.M * componentByteSize,
            .mappedAtCreation = false,
        };
        wgpu::Buffer output = device.CreateBuffer(&outputDescriptor);

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
        Matrix expected(config.K, config.M, config.componentType, columnMajor);
        GenerateReferenceResult(expected, inputLHS, op);
        EXPECT_BUFFER_U8_RANGE_EQ(expected.data, output, 0, expected.TotalByteSize()) << config;
    }
};

TEST_P(SubgroupMatrix_MatrixScalarArithmeticTest, MatrixScalar) {
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

        if (IsWindows() && IsAMD() && IsD3D12()) {
            if (CrashesOnRX9060XT(config, true)) {
                std::cout << "Skipping config: " << config << "\n";
                continue;
            }
        }

        TestSubgroupMatrixConfig(config, op, info.subgroupMaxSize, columnMajor);
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixScalarArithmeticTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend(),
                        },
                        {
                            // MatrixOp
                            MatrixOp::MatrixScalarAdd,
                            MatrixOp::MatrixScalarSubtract,
                            MatrixOp::MatrixScalarMultiply,
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

    static bool NeedsF16(const wgpu::SubgroupMatrixConfig& config, uint32_t array_bytes) {
        return config.componentType == wgpu::SubgroupMatrixComponentType::F16 ||
               config.resultComponentType == wgpu::SubgroupMatrixComponentType::F16 ||
               array_bytes == 2;
    }

    wgpu::ComputePipeline GetComputePipelineFromSubgroupMatrixConfig(
        const wgpu::SubgroupMatrixConfig& config,
        uint32_t subgroupMaxSize,
        bool inputColumnMajor,
        uint32_t array_bytes) {
        uint32_t mat_ele_bytes = ComponentTypeToByteSize(config.componentType);
        uint32_t factor =
            std::max(array_bytes, mat_ele_bytes) / std::min(array_bytes, mat_ele_bytes);
        std::string factor_operator = array_bytes < mat_ele_bytes ? "*" : "/";
        std::string array_type;
        switch (array_bytes) {
            case 2:
                array_type = "f16";
                break;
            case 4:
                array_type = "u32";
                break;
            case 8:
                array_type = "vec2u";
                break;
            case 16:
                array_type = "vec4u";
                break;
        }

        // Generate a shader that stores a subgroup matrix into a storage buffer.
        std::ostringstream shader;
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        if (NeedsF16(config, array_bytes)) {
            shader << "enable f16;\n";
        }
        shader << "\n";
        shader << "alias ComponentType = " << ComponentTypeToWgslType(config.componentType)
               << ";\n";
        shader << "alias ArrayType = " << array_type << ";\n\n";
        shader << "alias InputType = subgroup_matrix_left<ComponentType, K, M>;\n";
        shader << "const K = " << config.K << ";\n";
        shader << "const M = " << config.M << ";\n";

        shader << "const kStoreOffset = K * M";
        // Offset in terms of ArrayType
        shader << " " << factor_operator << " " << factor;
        shader << ";\n";

        shader << "const stride = " << (inputColumnMajor ? "M" : "K");
        // Offset in terms of ArrayType
        shader << " " << factor_operator << " " << factor;
        shader << ";\n";

        shader << "const kInputArraySize = kStoreOffset" << ";\n";

        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(0) var<storage, read>       input : array<ArrayType, kInputArraySize>;
@group(0) @binding(1) var<storage, read_write> output : array<ArrayType, kInputArraySize * 2>;

@compute @workgroup_size(SubgroupMaxSize)
fn main() {
)";

        if (inputColumnMajor) {
            shader << "let input_matrix = subgroupMatrixLoad<InputType, col_major>(&input, 0, "
                      "stride);\n";
            shader << "subgroupMatrixStore<col_major>(&output, kStoreOffset, input_matrix, "
                      "stride);\n";
        } else {
            shader << "let input_matrix = subgroupMatrixLoad<InputType, row_major>(&input,  0, "
                      "stride);\n";
            shader << "subgroupMatrixStore<row_major>(&output, kStoreOffset, input_matrix, "
                      "stride);\n";
        }

        shader << "\n}";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        return device.CreateComputePipeline(&csDesc);
    }

    void TestSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config,
                                  uint32_t subgroupMaxSize,
                                  bool inputColumnMajor,
                                  uint32_t array_bytes) {
        // In the tests we use a compute pipeline to store a subgroup matrix into a storage buffer
        // and check if the data in the buffer matches the expectation.
        wgpu::ComputePipeline pipeline = GetComputePipelineFromSubgroupMatrixConfig(
            config, subgroupMaxSize, inputColumnMajor, array_bytes);

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
        EXPECT_BUFFER_U8_RANGE_EQ(zeroBuffer.data(), output, 0, storeOffset)
            << config << "\narray_bytes = " << array_bytes;
        EXPECT_BUFFER_U8_RANGE_EQ(inputMatrix.data, output, storeOffset,
                                  inputMatrix.TotalByteSize())
            << config << "\narray_bytes = " << array_bytes;
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

        if (IsWindows() && IsAMD() && IsD3D12()) {
            if (CrashesOnRX9060XT(config, false)) {
                std::cout << "Skipping config: " << config << "\n";
                continue;
            }
        }

        // For majorness templated variants, test a variety of array element types.
        for (uint32_t j = 2; j <= 16; j <<= 1) {
            if (j < ComponentTypeToByteSize(config.componentType)) {
                continue;
            }

            // TODO(b/542145066): IMG has a problem with mismatched components.
            if (IsImgTec() && j != 2) {
                continue;
            }
            uint32_t stride = GetParam().mInputColumnMajor ? config.M : config.K;
            uint32_t stride_bytes = stride * ComponentTypeToByteSize(config.componentType);
            if (j <= stride_bytes) {
                if (NeedsF16(config, j) && !adapter.HasFeature(wgpu::FeatureName::ShaderF16)) {
                    continue;
                }
                TestSubgroupMatrixConfig(config, info.subgroupMaxSize, GetParam().mInputColumnMajor,
                                         j);
            }
        }
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixStoreTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend(),
                        },
                        {
                            // Input matrix is in column-major or not
                            true,
                            false,
                        });

using WithArgument = bool;
DAWN_TEST_PARAM_STRUCT(MatrixConstructorParams, WithArgument);
class SubgroupMatrix_MatrixConstructorTest : public DawnTestWithParams<MatrixConstructorParams> {
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
        bool withArgument) {
        // Generate a shader that constructs a subgroup matrix and stores it into a storage buffer.
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
        shader << "const K = " << config.K << ";\n";
        shader << "const M = " << config.M << ";\n";

        shader << "const kOutputArraySize = (K * M)";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const kStride = K";
        if (config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
            config.componentType == wgpu::SubgroupMatrixComponentType::I8) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";
        shader << R"(
@group(0) @binding(1) var<storage, read_write> output : array<ArrayType, kOutputArraySize>;

@compute @workgroup_size(SubgroupMaxSize)
fn main() {
)";

        std::string loadInput;
        std::string storeResult;
        loadInput = "let input_matrix = subgroup_matrix_left<ComponentType, K, M>(";
        if (withArgument) {
            loadInput += "5";
        }
        loadInput += ");";
        storeResult = "subgroupMatrixStore<row_major>(&output, 0, input_matrix, kStride);";

        shader << loadInput << "\n" << storeResult << "\n\n}";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        return device.CreateComputePipeline(&csDesc);
    }

    void TestSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config,
                                  uint32_t subgroupMaxSize,
                                  bool withArgument) {
        // In the tests we use a compute pipeline to construct a subgroup matrix and store it into a
        // storage buffer and check if the data in the buffer matches the expectation.
        wgpu::ComputePipeline pipeline =
            GetComputePipelineFromSubgroupMatrixConfig(config, subgroupMaxSize, withArgument);

        Matrix inputMatrix(config.K, config.M, config.componentType, false);

        // Create the output buffer.
        wgpu::BufferDescriptor outputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = inputMatrix.TotalByteSize(),
        };
        wgpu::Buffer output = device.CreateBuffer(&outputDescriptor);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{1, output}});
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Verify the result in the output buffer.
        Matrix expected(config.K, config.M, config.componentType, false);
        GenerateReferenceResult(expected, withArgument);
        EXPECT_BUFFER_U8_RANGE_EQ(expected.data, output, 0, expected.TotalByteSize()) << config;
    }

    void GenerateReferenceResult(Matrix& expected, bool withArgument) {
        const bool is_float = expected.component_type == wgpu::SubgroupMatrixComponentType::F16 ||
                              expected.component_type == wgpu::SubgroupMatrixComponentType::F32;
        for (uint32_t r = 0; r < expected.rows; r++) {
            for (uint32_t c = 0; c < expected.cols; c++) {
                if (is_float) {
                    float ref = 0.f;
                    if (withArgument) {
                        ref += 5.f;
                    }

                    expected.SetFloat(ref, c, r);
                } else {
                    int64_t ref = 0;
                    if (withArgument) {
                        ref = 5;
                    }
                    expected.SetInt(ref, c, r);
                }
            }
        }
    }
};

TEST_P(SubgroupMatrix_MatrixConstructorTest, MatrixConstruct) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    // TODO(crbug.com/492539239): Access violation during test teardown.
    DAWN_SUPPRESS_TEST_IF(IsWindows11() && IsAMD() && IsVulkan());

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    // Test each supported config.
    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        auto& config = subgroupMatrixConfigs.configs[i];

        if (IsWindows() && IsAMD() && IsD3D12()) {
            if (CrashesOnRX9060XT(config, false)) {
                std::cout << "Skipping config: " << config << "\n";
                continue;
            }
        }

        TestSubgroupMatrixConfig(config, info.subgroupMaxSize, GetParam().mWithArgument);
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_MatrixConstructorTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend(),
                        },
                        {
                            // Pass an argument to the constructor
                            true,
                            false,
                        });

using TileDim = uint32_t;
using WorkgroupSize = uint32_t;
DAWN_TEST_PARAM_STRUCT(TiledMatrixMultiplyParams, TileDim, WorkgroupSize);
class SubgroupMatrix_TiledMatrixMultiplyTest
    : public DawnTestWithParams<TiledMatrixMultiplyParams> {
  protected:
    using DawnTestBase::SupportsFeatures;

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix})) {
            features.push_back(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix);
        }
        if (SupportsFeatures({wgpu::FeatureName::ShaderF16})) {
            features.push_back(wgpu::FeatureName::ShaderF16);
        }
        if (SupportsFeatures({wgpu::FeatureName::Subgroups})) {
            features.push_back(wgpu::FeatureName::Subgroups);
        }
        return features;
    }

    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        required.maxComputeWorkgroupSizeX = supported.maxComputeWorkgroupSizeX;
        required.maxComputeWorkgroupSizeY = supported.maxComputeWorkgroupSizeY;
        required.maxComputeWorkgroupSizeZ = supported.maxComputeWorkgroupSizeZ;
        required.maxComputeInvocationsPerWorkgroup = supported.maxComputeInvocationsPerWorkgroup;
    }
};

// This is a slightly more interesting test of subgroup matrix types.
// It is also used to cover an issue with the MSL compiler described in crbug.com/443794633.
TEST_P(SubgroupMatrix_TiledMatrixMultiplyTest, MatrixMultiply) {
    // We will test a single workgroup that processes a matrix with size (N*kTileDim, M*kTileDim).
    const uint32_t kTileDim = GetParam().mTileDim;
    const uint32_t kWorkgroupSize = GetParam().mWorkgroupSize;

    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    // TODO(crbug.com/492539239): Access violation during test teardown.
    DAWN_SUPPRESS_TEST_IF(IsWindows11() && IsAMD() && IsVulkan());

    // TODO(crbug.com/525517826): On WARP 1.65535.20-preview, starts hanging for tile dim 2+
    DAWN_SUPPRESS_TEST_IF(IsWARP() && kTileDim >= 2);

    // TODO(525518027): On AMD Radeon RX 9060 XT, Windows Vulkan, getting invalid results for tile
    // dim 32
    DAWN_SUPPRESS_TEST_IF(IsAMD() && IsVulkan() && IsWindows() && kTileDim == 32);
    // TODO(525518027): On AMD Radeon RX 9060 XT, Windows D3D12, hanging during
    // CreateComputePipeline for tile dim 32
    DAWN_SUPPRESS_TEST_IF(IsAMD() && IsD3D12() && IsWindows() && kTileDim == 32);

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    const auto& supportedLimits = GetSupportedLimits();
    DAWN_TEST_UNSUPPORTED_IF(kWorkgroupSize > supportedLimits.maxComputeWorkgroupSizeX);
    DAWN_TEST_UNSUPPORTED_IF(kWorkgroupSize > supportedLimits.maxComputeInvocationsPerWorkgroup);
    DAWN_TEST_UNSUPPORTED_IF(kWorkgroupSize < info.subgroupMaxSize);
    // Pipeline creation may fail (gracefully) for some large tile and workgroup sizes.
    // The test will not fail in these cases, but we should make sure that the smallest cases always
    // pass so that we know the test isn't completely broken.
    const bool mustPass = kTileDim <= 2 && kWorkgroupSize == info.subgroupMaxSize;

    // Test each supported config.
    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        auto& config = subgroupMatrixConfigs.configs[i];
        uint32_t resultComponentByteSize = ComponentTypeToByteSize(config.resultComponentType);

        if (IsWindows() && IsAMD() && IsD3D12()) {
            if (CrashesOnRX9060XT(config, true)) {
                std::cout << "Skipping config: " << config << "\n";
                continue;
            }
        }

        std::stringstream configInfo;
        configInfo << "Testing " << config;
        SCOPED_TRACE(configInfo.str());

        const uint32_t matrix_cols = config.N * kTileDim;
        const uint32_t matrix_rows = config.M * kTileDim;
        const uint32_t matrix_k = config.K * kTileDim;

        const bool comp_8bit = config.componentType == wgpu::SubgroupMatrixComponentType::U8 ||
                               config.componentType == wgpu::SubgroupMatrixComponentType::I8;
        const bool res_8bit = config.resultComponentType == wgpu::SubgroupMatrixComponentType::U8 ||
                              config.resultComponentType == wgpu::SubgroupMatrixComponentType::I8;

        // Generate a shader that performs a matrix multiplication that matches the config.
        std::ostringstream shader;
        shader << "enable chromium_experimental_subgroup_matrix;\n";
        shader << "enable subgroups;\n";
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
        shader << "alias InputArrayType = " << ComponentTypeToScalarShaderType(config.componentType)
               << ";\n";
        shader << "alias ResultArrayType = "
               << ComponentTypeToScalarShaderType(config.resultComponentType) << ";\n";
        shader << "alias LeftType = subgroup_matrix_left<ComponentType, K, M>;";
        shader << "alias RightType = subgroup_matrix_right<ComponentType, N, K>;";
        shader << "alias ResultType = subgroup_matrix_result<ResultComponentType, N, M>;";
        shader << "const M = " << config.M << ";\n";
        shader << "const N = " << config.N << ";\n";
        shader << "const K = " << config.K << ";\n";
        shader << "const kTileDim = " << kTileDim << ";\n";
        shader << "const kMatrixCols = " << matrix_cols << ";\n";
        shader << "const kMatrixRows = " << matrix_rows << ";\n";
        shader << "const kMatrixK = " << matrix_k << ";\n";
        shader << "const kWorkgroupSize = " << kWorkgroupSize << ";\n";

        shader << "const kInputArraySize = (kMatrixK*kMatrixRows + kMatrixCols*kMatrixK)";
        if (comp_8bit) {
            shader << "/4";
        }
        shader << ";\n";

        shader << "const kResultArraySize = (kMatrixCols*kMatrixRows)";
        if (res_8bit) {
            shader << "/4";
        }
        shader << ";\n";

        shader << R"(
@group(0) @binding(0) var<storage, read>       inputs : array<InputArrayType, kInputArraySize>;
@group(0) @binding(1) var<storage, read_write> output : array<ResultArrayType, kResultArraySize>;

@compute @workgroup_size(kWorkgroupSize)
fn main(@builtin(subgroup_id) sgid: u32,
        @builtin(num_subgroups) num_subgroups: u32) {
  // The LHS matrix is (kMatrixK * kMatrixRows) elements.
  // The RHS matrix is (kMatrixCols * kMatrixK) elements.
  // The LHS and RHS matrices are stored in the same buffer.
  // The RHS matrix starts after the LHS matrix.
  const kRhsBase = kMatrixK * kMatrixRows;

  // The workgroup will process a grid of (kTileDim * kTileDim) subgroup matrices.
  // Each subgroup will process one or more rows of this grid.
  // For example, if the kTileDim = 4 and there are two subgroups, the distribution will be:
  //            ----------- ----------- ----------- -----------
  // sgid=0 -> | tile(0,0) | tile(1,0) | tile(2,0) | tile(3,0) |
  //            ----------- ----------- ----------- -----------
  // sgid=1 -> | tile(0,1) | tile(1,1) | tile(2,1) | tile(3,1) |
  //            ----------- ----------- ----------- -----------
  // sgid=0 -> | tile(0,2) | tile(1,2) | tile(2,2) | tile(3,2) |
  //            ----------- ----------- ----------- -----------
  // sgid=1 -> | tile(0,3) | tile(1,3) | tile(2,3) | tile(3,3) |
  //            ----------- ----------- ----------- -----------
  //
  // Note: This is not a performant algorithm, but gives us a simple approach to test subgroup
  // matrices with multiple subgroups and was sufficient to trigger crbug.com/443794633.

  // Accumulate results for each tile of the output matrix.
  var acc: array<array<ResultType, kTileDim>, kTileDim>;

  for (var k = 0u; k < kMatrixK; k+=K) {
    for (var r = sgid; r < kTileDim; r += num_subgroups) {
      for (var c = 0u; c < kTileDim; c++) {
        let lhs_offset = (k + r*M*kMatrixK))" +
                      std::string(comp_8bit ? "/4" : "") + R"(;
        let lhs_stride = kMatrixK)" +
                      std::string(comp_8bit ? "/4" : "") + R"(;
        let lhs = subgroupMatrixLoad<LeftType, row_major>(&inputs,  lhs_offset, lhs_stride);
        let rhs_offset = (c*N + k*kMatrixCols + kRhsBase))" +
                      std::string(comp_8bit ? "/4" : "") + R"(;
        let rhs_stride = kMatrixCols)" +
                      std::string(comp_8bit ? "/4" : "") + R"(;
        let rhs = subgroupMatrixLoad<RightType, row_major>(&inputs, rhs_offset, rhs_stride);
        acc[r][c] = subgroupMatrixMultiplyAccumulate(lhs, rhs, acc[r][c]);
      }
    }
  }

  // Store the results to the output buffer.
  for (var r = sgid; r < kTileDim; r += num_subgroups) {
    for (var c = 0u; c < kTileDim; c++) {
      let res_offset = (c*N + r*M*kMatrixCols))" +
                      std::string(res_8bit ? "/4" : "") + R"(;
      let res_stride = kMatrixCols)" +
                      std::string(res_8bit ? "/4" : "") + R"(;
      subgroupMatrixStore<row_major>(&output, res_offset, acc[r][c], res_stride);
    }
  }
})";

        // Wrap pipeline creation in an error scope since it may spuriously fail for reasons beyond
        // out control (e.g. exceeding function stack space in the MSL compiler).
        device.PushErrorScope(wgpu::ErrorFilter::Internal);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        bool createFailed = false;
        auto popFuture = device.PopErrorScope(
            wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::PopErrorScopeStatus, wgpu::ErrorType type, wgpu::StringView msg) {
                switch (type) {
                    case wgpu::ErrorType::NoError:
                        return;
                    case wgpu::ErrorType::Internal:
                    case wgpu::ErrorType::OutOfMemory:
                    case wgpu::ErrorType::Validation:
                    case wgpu::ErrorType::Unknown:
                        std::cerr << "creating pipeline failed: " << msg << "\n";
                        createFailed = true;
                        return;
                }
            });
        WaitForAllOperations();
        auto status = instance.WaitAny(popFuture, UINT64_MAX);
        if (status != wgpu::WaitStatus::Success || createFailed) {
            // Allow a spurious pipeline creation failure, unless this is one of the small cases
            // that must always pass.
            ASSERT_FALSE(mustPass) << "unexpected pipeline creation failure";
            continue;
        }

        // Create the input matrices and fill them with values.
        Matrix inputLHS(matrix_k, matrix_rows, config.componentType, false);
        Matrix inputRHS(matrix_cols, matrix_k, config.componentType, false);
        Matrix acc(matrix_cols, matrix_rows, config.resultComponentType, false);
        // Offset the values for each matrix so that they are all different.
        inputLHS.Fill(0);
        inputRHS.Fill(1);
        acc.FillWithZero();

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

        // Create the output buffer.
        wgpu::BufferDescriptor outputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = static_cast<uint64_t>(matrix_cols) * matrix_rows * resultComponentByteSize,
        };
        wgpu::Buffer output = device.CreateBuffer(&outputDescriptor);

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
        Matrix expected(matrix_cols, matrix_rows, config.resultComponentType, false);
        GenerateReferenceMatrixMultiply(expected, inputLHS, inputRHS, acc);
        EXPECT_BUFFER_U8_RANGE_EQ(expected.data, output, 0, expected.TotalByteSize());
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_TiledMatrixMultiplyTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend({"use_vulkan_memory_model"}),
                        },
                        {
                            // TileDim
                            1u,
                            2u,
                            4u,
                            8u,
                            16u,
                            32u,
                        },
                        {
                            // WorkgroupSize
                            128u,
                            256u,
                            512u,
                            1024u,
                        });

// Test loading from workgroup address space and storing to storage address space.
// The spec states that subgroupMatrixLoad/Store can operate on both workgroup and storage.
class SubgroupMatrix_WorkgroupLoadStoreTest : public DawnTestWithParams<MatrixStoreParams> {
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
        bool columnMajor) {
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

        shader << "const kArraySize = K * M";
        if (Is8Bit(config.componentType)) {
            shader << " / 4";
        }
        shader << ";\n";

        shader << "const SubgroupMaxSize = " << subgroupMaxSize << ";\n";

        shader << "const kStride = " << (columnMajor ? "M" : "K");
        if (Is8Bit(config.componentType)) {
            shader << " / 4";
        }
        shader << ";\n";

        shader << R"(
@group(0) @binding(0) var<storage, read>       input : array<ArrayType, kArraySize>;
@group(0) @binding(1) var<storage, read_write> output : array<ArrayType, kArraySize>;

var<workgroup> wg_data : array<ArrayType, kArraySize>;

@compute @workgroup_size(SubgroupMaxSize)
fn main(@builtin(local_invocation_index) lid: u32) {
  // Copy data from storage to workgroup memory.
  for (var i = lid; i < kArraySize; i += SubgroupMaxSize) {
    wg_data[i] = input[i];
  }
  workgroupBarrier();

  // Load the matrix from workgroup memory.
)";

        if (columnMajor) {
            shader << "  let mat = subgroupMatrixLoad<InputType, col_major>(&wg_data, 0, "
                      "kStride);\n";
            shader << "  // Store to storage buffer.\n";
            shader << "  subgroupMatrixStore<col_major>(&output, 0, mat, kStride);\n";
        } else {
            shader << "  let mat = subgroupMatrixLoad<InputType, row_major>(&wg_data, 0, "
                      "kStride);\n";
            shader << "  // Store to storage buffer.\n";
            shader << "  subgroupMatrixStore<row_major>(&output, 0, mat, kStride);\n";
        }

        shader << "}\n";

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.str());
        return device.CreateComputePipeline(&csDesc);
    }

    void TestSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config,
                                  uint32_t subgroupMaxSize,
                                  bool columnMajor) {
        wgpu::ComputePipeline pipeline =
            GetComputePipelineFromSubgroupMatrixConfig(config, subgroupMaxSize, columnMajor);

        Matrix inputMatrix(config.K, config.M, config.componentType, columnMajor);
        inputMatrix.Fill(0);

        // Create the input buffer.
        wgpu::BufferDescriptor inputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = inputMatrix.TotalByteSize(),
            .mappedAtCreation = true,
        };
        wgpu::Buffer inputBuffer = device.CreateBuffer(&inputDescriptor);
        memcpy(inputBuffer.GetMappedRange(), inputMatrix.data, inputMatrix.TotalByteSize());
        inputBuffer.Unmap();

        // Create the output buffer.
        wgpu::BufferDescriptor outputDescriptor{
            .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
            .size = inputMatrix.TotalByteSize(),
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

        // Verify: data loaded from workgroup and stored to storage should match the original input.
        EXPECT_BUFFER_U8_RANGE_EQ(inputMatrix.data, output, 0, inputMatrix.TotalByteSize())
            << config;
    }
};

// Test loading from workgroup address space and storing to storage.
TEST_P(SubgroupMatrix_WorkgroupLoadStoreTest, WorkgroupLoadStore) {
    DAWN_TEST_UNSUPPORTED_IF(
        !adapter.HasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix));

    bool columnMajor = GetParam().mInputColumnMajor;

    // Query the supported subgroup matrix configurations.
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs;
    info.nextInChain = &subgroupMatrixConfigs;
    ASSERT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

    for (size_t i = 0; i < subgroupMatrixConfigs.configCount; i++) {
        auto& config = subgroupMatrixConfigs.configs[i];

        // TODO(crbug.com/512455144): Support 8-bit subgroup matrix loads from workgroup memory in
        // the HLSL writer.
        if (IsD3D12() && Is8Bit(config.componentType)) {
            std::cout << "Skipping config: " << config << "\n";
            continue;
        }

        if ((IsWindows() && IsAMD() && IsD3D12()) && (CrashesOnRX9060XT(config, false))) {
            std::cout << "Skipping config: " << config << "\n";
            continue;
        }

        TestSubgroupMatrixConfig(config, info.subgroupMaxSize, columnMajor);
    }
}

DAWN_INSTANTIATE_TEST_P(SubgroupMatrix_WorkgroupLoadStoreTest,
                        {
                            D3D12Backend(),
                            MetalBackend(),
                            VulkanBackend(),
                        },
                        {
                            // Column-major or row-major
                            true,
                            false,
                        });

}  // anonymous namespace
}  // namespace dawn
