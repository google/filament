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

#include <sstream>
#include <string>
#include <vector>

#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr unsigned int kNumDisptaches = 100;

enum class DataType {
    F32,
    F16,
    U8,
};

std::ostream& operator<<(std::ostream& ostream, const DataType& v) {
    switch (v) {
        case DataType::F32:
            ostream << "F32";
            break;
        case DataType::F16:
            ostream << "F16";
            break;
        case DataType::U8:
            ostream << "U8";
            break;
    }
    return ostream;
}

enum class KernelImplementation {
    Naive,
    Subgroup,
    WorkgroupShared,
};

std::ostream& operator<<(std::ostream& ostream, const KernelImplementation& v) {
    switch (v) {
        case KernelImplementation::Naive:
            ostream << "Naive";
            break;
        case KernelImplementation::Subgroup:
            ostream << "Subgroup";
            break;
        case KernelImplementation::WorkgroupShared:
            ostream << "WorkgroupShared";
            break;
    }
    return ostream;
}

using Rows = uint32_t;
using Cols = uint32_t;
using StoreType = DataType;
using AccType = DataType;
using Impl = KernelImplementation;
using Swizzle = bool;
DAWN_TEST_PARAM_STRUCT(MatrixVectorMultiplyParams, Rows, Cols, StoreType, AccType, Impl, Swizzle);

class MatrixVectorMultiplyPerf : public DawnPerfTestWithParams<MatrixVectorMultiplyParams> {
  public:
    MatrixVectorMultiplyPerf()
        : DawnPerfTestWithParams(kNumDisptaches, /* allow many steps in flight */ 100) {}
    ~MatrixVectorMultiplyPerf() override = default;

    void SetUp() override;

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mUsingF16 = false;
        mUsingSubgroups = false;
        mAllFeaturesSupported = true;

        auto requirements =
            DawnPerfTestWithParams<MatrixVectorMultiplyParams>::GetRequiredFeatures();
        auto requireFeature = [&](wgpu::FeatureName feature) {
            if (SupportsFeatures({feature})) {
                requirements.push_back(feature);
            } else {
                mAllFeaturesSupported = false;
            }
        };
        if (GetParam().mStoreType == StoreType::F16 || GetParam().mAccType == AccType::F16) {
            mUsingF16 = true;
            requireFeature(wgpu::FeatureName::ShaderF16);
        }
        if (GetParam().mImpl == KernelImplementation::Subgroup) {
            mUsingSubgroups = true;
            requireFeature(wgpu::FeatureName::Subgroups);
        }
        return requirements;
    }

    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        required.maxStorageBufferBindingSize =
            BytesPerElement() * GetParam().mRows * GetParam().mCols;
    }

  private:
    void Step() override;

    uint32_t BytesPerElement() const {
        switch (GetParam().mStoreType) {
            case StoreType::F32:
                return 4;
            case StoreType::F16:
                return 2;
            case StoreType::U8:
                return 1;
        }
    }

    std::string GenerateShader() const;

    wgpu::BindGroup mBindGroup;
    wgpu::ComputePipeline mPipeline;

    bool mUsingF16;
    bool mUsingSubgroups;
    bool mAllFeaturesSupported;
};

void MatrixVectorMultiplyPerf::SetUp() {
    // TODO(crbug.com/dawn/2508): Fails due to an OS/driver upgrade on Linux/llvmpipe.
    // This must also be checked before SetUp() since the crash happens in SetUp() itself.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsMesa("23.2.1") && IsMesaSoftware() &&
                          GetParam().mStoreType == StoreType::F32);

    DawnPerfTestWithParams<MatrixVectorMultiplyParams>::SetUp();

    // Unoptimized variant too slow for bots.
    // Unskip locally with flag --run-suppressed-tests.
    DAWN_SUPPRESS_TEST_IF(IsMacOS() && IsAMD() && !GetParam().mSwizzle);

    if (GetParam().mStoreType != StoreType::U8) {
        // Don't care about testing this case.
        DAWN_TEST_UNSUPPORTED_IF(GetParam().mStoreType != GetParam().mAccType);
    }

    DAWN_TEST_UNSUPPORTED_IF(!mAllFeaturesSupported);

    // D3D12 device must be using DXC to support subgroups feature.
    DAWN_ASSERT(!mUsingSubgroups || !IsD3D12() || IsDXC());

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    bufferDesc.size = BytesPerElement() * GetParam().mRows * GetParam().mCols;
    wgpu::Buffer matrix = device.CreateBuffer(&bufferDesc);

    bufferDesc.size = BytesPerElement() * GetParam().mCols;
    wgpu::Buffer vector = device.CreateBuffer(&bufferDesc);

    bufferDesc.size = BytesPerElement() * GetParam().mRows;
    wgpu::Buffer result = device.CreateBuffer(&bufferDesc);

    uint32_t uniformData[] = {GetParam().mRows, /* packed cols */ GetParam().mCols / 4};
    wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
        device, uniformData, sizeof(uniformData), wgpu::BufferUsage::Uniform);

    std::string code = GenerateShader();
    wgpu::ShaderModule module = utils::CreateShaderModule(device, code.c_str());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    mPipeline = device.CreateComputePipeline(&csDesc);

    mBindGroup = utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0),
                                      {
                                          {0, matrix},
                                          {1, vector},
                                          {2, result},
                                          {3, uniformBuffer},
                                      });
}

std::string MatrixVectorMultiplyPerf::GenerateShader() const {
    std::stringstream code;
    if (mUsingF16) {
        code << "enable f16;\n";
    }
    if (mUsingSubgroups) {
        code << "enable subgroups;\n";
    }
    switch (GetParam().mStoreType) {
        case StoreType::F32:
            code << "alias StoreType = vec4<f32>;\n";
            break;
        case StoreType::F16:
            code << "alias StoreType = vec4<f16>;\n";
            break;
        case StoreType::U8:
            code << "alias StoreType = u32;\n";
            break;
    }
    switch (GetParam().mAccType) {
        case AccType::F32:
            code << "alias AccType = f32;\n";
            break;
        case AccType::F16:
            code << "alias AccType = f16;\n";
            break;
        case AccType::U8:
            code << "alias AccType = u32;\n";
            break;
    }
    code << R"(struct Uniforms {
        rows : u32,
        packedCols : u32,
    }
    struct Matrix {
        values: array<StoreType>
    }
    struct Vector {
        values: array<StoreType>
    }

    @group(0) @binding(0) var<storage, read> matrix : Matrix;
    @group(0) @binding(1) var<storage, read> vector : Vector;
    @group(0) @binding(2) var<storage, read_write> result : Vector;
    @group(0) @binding(3) var<uniform> uniforms : Uniforms;
    )";

    std::function<std::string(std::string)> valueLoad;
    std::function<std::string(std::string)> loopBody;
    std::string writeResult;

    // Parameter mSwizzle indicates whether the physical layout of the matrix has 4x4 subblocks
    // transposed from what they are in the logical matrix.
    // The global compute grid is 1-dimensional for input matrix MxK and vector Kx1 to output vector
    // Mx1. When the kernel implementation is Workgroup:
    //  - each workgroup output 4 rows of the output column vector.
    //  - each invocation within the workgroup compute a slice of input matrix 4x⌈K/workgroup_size⌉
    //    multiples a slice of input column vector ⌈K/workgroup_size⌉x1, and at the end each
    //    invocation's result get added up to get the final output sum.
    // Otherwise:
    //  - invocation gid.x performs the work of 4 output rows in the matrix starting at 4*gid.x, and
    //    nothing else.
    //  - the whole workgroup computes * workgroup_size rows
    if (GetParam().mStoreType == StoreType::U8 && GetParam().mAccType == AccType::U8) {
        // Data is already 8-bit. Compute 8-bit dot products.
        valueLoad = [](std::string i) { return "vector.values[" + i + "]"; };
        // clang-format off
        loopBody = [](std::string offset) {
            if (GetParam().mSwizzle) {
                return "sum += vec4<AccType>(\n"
                    "dot4U8Packed(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 0u], v),\n"
                    "dot4U8Packed(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 1u], v),\n"
                    "dot4U8Packed(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 2u], v),\n"
                    "dot4U8Packed(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 3u], v),\n"
                ");";
            } else {
                return "sum += vec4<AccType>(\n"
                    "dot4U8Packed(matrix.values[(4u * row_by_4 + 0u) * uniforms.packedCols + col" + offset + "], v),\n"
                    "dot4U8Packed(matrix.values[(4u * row_by_4 + 1u) * uniforms.packedCols + col" + offset + "], v),\n"
                    "dot4U8Packed(matrix.values[(4u * row_by_4 + 2u) * uniforms.packedCols + col" + offset + "], v),\n"
                    "dot4U8Packed(matrix.values[(4u * row_by_4 + 3u) * uniforms.packedCols + col" + offset + "], v),\n"
                ");";
            }
        };
        // clang-format on
        writeResult = "result.values[row_by_4] = pack4xU8(sum);\n";
    } else if (GetParam().mStoreType == StoreType::U8) {
        // Data is 8-bit. Expand out to float, compute dot product, and then pack again.
        valueLoad = [](std::string i) {
            return "vec4<AccType>(unpack4xU8(vector.values[" + i + "]))";
        };
        // clang-format off
        loopBody = [](std::string offset) {
            if (GetParam().mSwizzle) {
                return "sum += vec4<AccType>(\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 0u])), v),\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 1u])), v),\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 2u])), v),\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 3u])), v),\n"
                ");";
            } else {
                return "sum += vec4<AccType>(\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[(4u * row_by_4 + 0u) * uniforms.packedCols + col" + offset + "])), v),\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[(4u * row_by_4 + 1u) * uniforms.packedCols + col" + offset + "])), v),\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[(4u * row_by_4 + 2u) * uniforms.packedCols + col" + offset + "])), v),\n"
                    "dot(vec4<AccType>(unpack4xU8(matrix.values[(4u * row_by_4 + 3u) * uniforms.packedCols + col" + offset + "])), v),\n"
                ");";
            }
        };
        // clang-format on
        writeResult = "result.values[row_by_4] = pack4x8unorm(vec4<f32>(sum));\n";
    } else {
        // Data is in float. Compute dot product in float.
        valueLoad = [](std::string i) { return "vector.values[" + i + "]"; };
        // clang-format off
        loopBody = [](std::string offset) {
            if (GetParam().mSwizzle) {
                return "sum += vec4<AccType>(\n"
                    "dot(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 0u], v),\n"
                    "dot(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 1u], v),\n"
                    "dot(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 2u], v),\n"
                    "dot(matrix.values[4u * (row_by_4 * uniforms.packedCols + col" + offset + ") + 3u], v),\n"
                ");";
            } else {
                return "sum += vec4<AccType>(\n"
                    "dot(matrix.values[(4u * row_by_4 + 0u) * uniforms.packedCols + col" + offset + "], v),\n"
                    "dot(matrix.values[(4u * row_by_4 + 1u) * uniforms.packedCols + col" + offset + "], v),\n"
                    "dot(matrix.values[(4u * row_by_4 + 2u) * uniforms.packedCols + col" + offset + "], v),\n"
                    "dot(matrix.values[(4u * row_by_4 + 3u) * uniforms.packedCols + col" + offset + "], v),\n"
                ");";
            }
        };
        // clang-format on
        writeResult = "result.values[row_by_4] = sum;\n";
    }

    switch (GetParam().mImpl) {
        case (KernelImplementation::Naive): {
            // clang-format off
            code << R"(
@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) global_id : vec3u) {
  let row_by_4 = global_id.x;
  var sum : vec4<AccType>;
  for (var col = 0u; col < uniforms.packedCols; col++) {
    let v = )" << valueLoad("col") << R"(;
    )" << loopBody("") << R"(
  }
  )" << writeResult << R"(
}
)";
            // clang-format on
            break;
        }
        case (KernelImplementation::Subgroup): {
            // Helper function to generate a subgroup case since:
            // - we don't know the subgroup size until runtime
            // - subgroupBroadcast requires a constant lane.
            auto GenerateSubgroupCase = [&valueLoad, &loopBody](uint32_t size) {
                std::stringstream c;
                c << "  if (sg_size == " << size << "u){\n";
                c << "    for (var col = 0u; col < uniforms.packedCols; col = col + " << size
                  << "u) {" << "\n";
                c << "      let shared_v = " << valueLoad("col + sg_id") << ";\n";
                if (GetParam().mAccType == AccType::U8) {
                    c << "      var v : AccType;\n";
                } else {
                    c << "      var v : vec4<AccType>;\n";
                }
                for (uint32_t i = 0; i < size; ++i) {
                    c << "      v = subgroupBroadcast(shared_v, " << i << "u);\n";
                    c << "        " << loopBody(" + " + std::to_string(i) + "u") << "\n";
                }
                c << "    }\n";
                c << "  }";
                return c.str();
            };

            // clang-format off
            code << R"(
@compute @workgroup_size(64)
fn main(
  @builtin(global_invocation_id) global_id : vec3u,
  @builtin(subgroup_size) sg_size : u32,
  @builtin(subgroup_invocation_id) sg_id : u32
) {
  let row_by_4 = global_id.x;
  var sum : vec4<AccType>;
)" << GenerateSubgroupCase(4)
    << " else " << GenerateSubgroupCase(8)
    << " else " << GenerateSubgroupCase(16)
    << " else " << GenerateSubgroupCase(32)
    << " else " << GenerateSubgroupCase(64)
    << "\n  " << writeResult << R"(
}
)";
            // clang-format on
            break;
        }
        case (KernelImplementation::WorkgroupShared): {
            const uint32_t workgroupSize = 64;
            // workgroupSize should be power of 2.
            DAWN_ASSERT((workgroupSize & (workgroupSize - 1)) == 0);

            std::stringstream workgroupSumStream;
            for (uint32_t i = workgroupSize / 2; i >= 1; i /= 2) {
                // clang-format off
                workgroupSumStream << R"(
  workgroupBarrier();
  if (local_id.x < )" << i << R"(u) {
    sum = sum + workgroup_sum[local_id.x + )" << i << R"(u];
    workgroup_sum[local_id.x] = sum;
  })";
                // clang-format on
            }

            // clang-format off
            code << R"(
const wg_size = )" << workgroupSize << R"(u;
var<workgroup> workgroup_sum: array<vec4<AccType>, wg_size>;

@compute @workgroup_size(wg_size)
fn main(
  @builtin(workgroup_id) wg_id : vec3u,
  @builtin(local_invocation_id) local_id : vec3u
) {
  let row_by_4 = wg_id.x;
  let compute_packed_cols_per_invocation = (uniforms.packedCols + wg_size - 1) / wg_size;
  let packed_cols_base = compute_packed_cols_per_invocation * local_id.x;
  var sum : vec4<AccType>;
  for (var col = packed_cols_base; col < packed_cols_base + compute_packed_cols_per_invocation; col++) {
    let v = )" << valueLoad("col") << R"(;
    )" << loopBody("") << R"(
  }

  // Store sum result into workgroup memory.
  workgroup_sum[local_id.x] = sum;

  // Add up results from all invocations and output.
)" << workgroupSumStream.str() << R"(
  // Write the final output
  if (local_id.x == 0) {
    )" << writeResult << R"(
  }
}
)";
            // clang-format on
            break;
        }
    }

    return code.str();
}

void MatrixVectorMultiplyPerf::Step() {
    bool useTimestamps = SupportsTimestampQuery();
    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassDescriptor computePassDesc;
        wgpu::PassTimestampWrites timestampWrites;
        if (useTimestamps) {
            timestampWrites = GetPassTimestampWrites();
            computePassDesc.timestampWrites = &timestampWrites;
        }
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePassDesc);
        pass.SetPipeline(mPipeline);
        pass.SetBindGroup(0, mBindGroup);
        for (unsigned int i = 0; i < kNumDisptaches; ++i) {
            if (GetParam().mImpl == KernelImplementation::WorkgroupShared) {
                // 4 is because each unit of data loaded is 4 packed values;
                // either a 4-element vector, or 4 u8's packed as a u32.
                // Each workgroup will write on 4-element output in the output
                // column vector. We need `mRows/ 4` workgroups total.
                pass.DispatchWorkgroups(GetParam().mRows / 4u);
            } else {
                // 64 is the linear workgroup size.
                // 4 is because each unit of data loaded is 4 packed values;
                // either a 4-element vector, or 4 u8's packed as a u32.
                // Each thread will write on 4-element output in the output
                // column vector. There are 64 threads. We need `mRows/ (64 * 4)`
                // workgroups total.
                pass.DispatchWorkgroups(GetParam().mRows / (64u * 4u));
            }
        }
        pass.End();
        if (useTimestamps) {
            ResolveTimestamps(encoder);
        }
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    if (useTimestamps) {
        ComputeGPUElapsedTime();
    }
}

TEST_P(MatrixVectorMultiplyPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(
    MatrixVectorMultiplyPerf,
    {D3D12Backend({"disable_robustness"}, {}),
     D3D12Backend({"polyfill_packed_4x8_dot_product", "disable_robustness"}, {}),
     MetalBackend({"disable_robustness"}, {}),
     MetalBackend({"polyfill_packed_4x8_dot_product", "disable_robustness"}, {}),
     OpenGLBackend({"disable_robustness"}, {}),
     OpenGLBackend({"polyfill_packed_4x8_dot_product", "disable_robustness"}, {}),
     VulkanBackend({"disable_robustness"}, {}),
     VulkanBackend({"polyfill_packed_4x8_dot_product", "disable_robustness"}, {})},
    {32768u}, /* rows */
    {2048u},  /* cols */
    {StoreType::F32, StoreType::F16, StoreType::U8},
    {AccType::F32, AccType::F16, AccType::U8},
    {KernelImplementation::Naive, KernelImplementation::Subgroup,
     KernelImplementation::WorkgroupShared}, /* Impl */
    {false, true} /* swizzle */);

}  // anonymous namespace
}  // namespace dawn
