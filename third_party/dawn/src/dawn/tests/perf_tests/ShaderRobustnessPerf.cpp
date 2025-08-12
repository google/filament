// Copyright 2021 The Dawn & Tint Authors
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

constexpr uint32_t kTileSize = 32u;
constexpr uint32_t kWorkgroupSizeX = 8u;
constexpr uint32_t kWorkgroupSizeY = 8u;
static_assert(kTileSize % kWorkgroupSizeX == 0,
              "workgroup width must evenly divide tile dimension");
static_assert(kTileSize % kWorkgroupSizeY == 0,
              "workgroup height must evenly divide tile dimension");

std::string GenMatMulFloatHeader() {
    std::stringstream ss;
    ss << "const kTileSize = " << kTileSize << ";  // 32\n";
    ss << "const kWorkgroupSizeX = " << kWorkgroupSizeX << "u; // 8;\n";
    ss << "const kWorkgroupSizeY = " << kWorkgroupSizeY << "u; // 8;\n";
    ss << R"(
        struct Uniforms {
            dimAOuter : u32,
            dimInner : u32,
            dimBOuter : u32,
        }
        struct Matrix {
            numbers: array<ElemT>
        }

        @group(0) @binding(0) var<storage, read> firstMatrix : Matrix;
        @group(0) @binding(1) var<storage, read> secondMatrix : Matrix;
        @group(0) @binding(2) var<storage, read_write> resultMatrix : Matrix;
        @group(0) @binding(3) var<uniform> uniforms : Uniforms;

        fn mm_readA(row : u32, col : u32) -> ElemT  {
            if (row < uniforms.dimAOuter && col < uniforms.dimInner)
            {
                let result : ElemT = firstMatrix.numbers[row * uniforms.dimInner + col];
                return result;
            }
            return 0.;
        }

        fn mm_readB(row : u32, col : u32) -> ElemT {
            if (row < uniforms.dimInner && col < uniforms.dimBOuter)
            {
                let result : ElemT = secondMatrix.numbers[row * uniforms.dimBOuter + col];
                return result;
            }
            return 0.;
        }

        fn mm_write(row : u32, col : u32, value : ElemT) {
            if (row < uniforms.dimAOuter && col < uniforms.dimBOuter)
            {
                let index : u32 = col + row * uniforms.dimBOuter;
                resultMatrix.numbers[index] = value;
            }
        }

        const RowPerThread : u32 = kTileSize / kWorkgroupSizeY; // 4
        const ColPerThread : u32 = kTileSize / kWorkgroupSizeX; // 4
        const TileBOuter : u32 = kTileSize;
        const TileInner : u32 = kTileSize;
        )";
    return ss.str();
}

const std::string& kMatMulFloatSharedArray1D = R"(
        var<workgroup> mm_Asub : array<ElemT, kTileSize * kTileSize>;
        var<workgroup> mm_Bsub : array<ElemT, kTileSize * kTileSize>;)";
const std::string& kMatMulFloatSharedArray2D = R"(
        var<workgroup> mm_Asub : array<array<ElemT, kTileSize>, kTileSize>;
        var<workgroup> mm_Bsub : array<array<ElemT, kTileSize>, kTileSize>;)";
const std::string& kMatMulFloatBodyPart1 = R"(

        @compute @workgroup_size(kWorkgroupSizeX, kWorkgroupSizeY, 1)
        fn main(@builtin(local_invocation_id) local_id : vec3u,
                @builtin(global_invocation_id) global_id  : vec3u) {
            // This invocation is responsible the region in the current tile with
            //   rows in [ tileRow, tileRow + RowPerThread -1 ]
            //   cols in [ tileCol, tileCol + ColPerThread -1 ]
            let tileRow : u32 = local_id.y * RowPerThread;
            let tileCol : u32 = local_id.x * ColPerThread;

            // This invocation is responsible the region in the output matrix with
            //   rows in [ globalRow, globalRow + RowPerThread -1 ]
            //   cols in [ globalCol, globalCol + ColPerThread -1 ]
            let globalRow : u32 = global_id.y * RowPerThread;
            let globalCol : u32 = global_id.x * ColPerThread;

            let numTiles : u32 = (uniforms.dimInner - 1u) / TileInner + 1u;

            var acc: array<ElemT, RowPerThread * ColPerThread>;
            var ACached : ElemT;
            var BCached : array<ElemT, ColPerThread>;

            // Define the region within the current tile that this thread
            // is responsible for loading into the cache.
            const ColPerThreadA : u32 = TileInner / kWorkgroupSizeX;
            let tileColA : u32 = local_id.x * ColPerThreadA;
            const RowPerThreadB : u32 = TileInner / kWorkgroupSizeY;
            let tileRowB : u32 = local_id.y * RowPerThreadB;

            // Loop over shared dimension.
            for (var t : u32 = 0u; t < numTiles; t++) {
                // Load one tile of A into local memory.
                for (var innerRow : u32 = 0u; innerRow < RowPerThread; innerRow++) {
                for (var innerCol : u32 = 0u; innerCol < ColPerThreadA; innerCol++) {
                    let inputRow : u32 = tileRow + innerRow;
                    let inputCol : u32 = tileColA + innerCol;)";
const std::string& kMatMulFloatBodyPart2Array1D = R"(
                    let index : u32 = inputRow * TileInner + inputCol;
                    mm_Asub[index] = mm_readA(globalRow + innerRow, t * TileInner + inputCol);
                }
                }
                // Load one tile of B into local memory.
                for (var innerRow : u32 = 0u; innerRow < RowPerThreadB; innerRow++) {
                for (var innerCol : u32 = 0u; innerCol < ColPerThread; innerCol++) {
                    let inputRow : u32 = tileRowB + innerRow;
                    let inputCol : u32 = tileCol + innerCol;
                    let index : u32 = inputRow * TileBOuter + inputCol;

                    mm_Bsub[index] = mm_readB(t * TileInner + inputRow, globalCol + innerCol);;
                }
                }

                workgroupBarrier();

                // Compute acc values for a single thread.
                for (var k : u32 = 0u; k < TileInner; k++) {
                    for (var inner : u32 = 0u; inner < ColPerThread; inner++) {
                        BCached[inner] = mm_Bsub[k * TileBOuter + tileCol + inner];
                    }

                    for (var innerRow : u32 = 0u; innerRow < RowPerThread; innerRow++) {
                        ACached = mm_Asub[(tileRow + innerRow) * TileInner + k];)";
const std::string& kMatMulFloatBodyPart2Array2D = R"(
                    mm_Asub[inputRow][inputCol] = mm_readA(globalRow + innerRow, t * TileInner + inputCol);
                }
                }
                // Load one tile of B into local memory.
                for (var innerRow : u32 = 0u; innerRow < RowPerThreadB; innerRow++) {
                for (var innerCol : u32 = 0u; innerCol < ColPerThread; innerCol++) {
                    let inputRow : u32 = tileRowB + innerRow;
                    let inputCol : u32 = tileCol + innerCol;

                    mm_Bsub[innerCol][inputCol] = mm_readB(t * TileInner + inputRow, globalCol + innerCol);;
                }
                }

                workgroupBarrier();

                // Compute acc values for a single thread.
                for (var k : u32 = 0u; k < TileInner; k++) {
                    for (var inner : u32 = 0u; inner < ColPerThread; inner++) {
                        BCached[inner] = mm_Bsub[k][tileCol + inner];
                    }

                    for (var innerRow : u32 = 0u; innerRow < RowPerThread; innerRow++) {
                        ACached = mm_Asub[tileRow + innerRow][k];)";
const std::string& kMatMulFloatBodyPart3 = R"(
                        for (var innerCol : u32 = 0u; innerCol < ColPerThread; innerCol++) {
                            let index : u32 = innerRow * ColPerThread + innerCol;
                            acc[index] = acc[index] + ACached * BCached[innerCol];
                        }
                    }
                }

                workgroupBarrier();
            }

            for (var innerRow : u32 = 0u; innerRow < RowPerThread; innerRow++) {
            for (var innerCol : u32 = 0u; innerCol < ColPerThread; innerCol++) {
                let index : u32 = innerRow * ColPerThread + innerCol;
                mm_write(globalRow + innerRow,
                         globalCol + innerCol,
                         acc[index]);
            }
            }
        })";
std::string GenMatMulFloatOneDimensionalSharedArray() {
    return GenMatMulFloatHeader() + kMatMulFloatSharedArray1D + kMatMulFloatBodyPart1 +
           kMatMulFloatBodyPart2Array1D + kMatMulFloatBodyPart3;
}

std::string GenMatMulFloatTwoDimensionalSharedArray() {
    return GenMatMulFloatHeader() + kMatMulFloatSharedArray2D + kMatMulFloatBodyPart1 +
           kMatMulFloatBodyPart2Array2D + kMatMulFloatBodyPart3;
}

// The vec4 version requires that dimInner and dimBOuter are divisible by 4.
std::string GenMatMulVec4Header() {
    std::stringstream ss;
    ss << "const kTileSize = " << kTileSize << ";  // 32\n";
    ss << "const kWorkgroupSizeX = " << kWorkgroupSizeX << "u; // 8;\n";
    ss << "const kWorkgroupSizeY = " << kWorkgroupSizeY << "u; // 8;\n";
    ss << R"(
        alias VecT = vec4<ElemT>;
        const VecLen = 4;
        struct Uniforms {
            dimAOuter : u32,
            dimInner : u32,
            dimBOuter : u32,
        }
        struct Matrix {
            numbers: array<VecT>
        }

        @group(0) @binding(0) var<storage, read> firstMatrix : Matrix;
        @group(0) @binding(1) var<storage, read> secondMatrix : Matrix;
        @group(0) @binding(2) var<storage, read_write> resultMatrix : Matrix;
        @group(0) @binding(3) var<uniform> uniforms : Uniforms;

        fn mm_readA(row : u32, col : u32) -> VecT  {
            if (row < uniforms.dimAOuter && col < uniforms.dimInner)
            {
                let result : VecT = firstMatrix.numbers[row * uniforms.dimInner / 4u + col];
                return result;
            }
            return VecT(0.0, 0.0, 0.0, 0.0);
        }

        fn mm_readB(row : u32, col : u32) -> VecT {
            if (row < uniforms.dimInner && col < uniforms.dimBOuter)
            {
                let result : VecT = secondMatrix.numbers[row * uniforms.dimBOuter / 4u + col];
                return result;
            }
            return VecT(0.0, 0.0, 0.0, 0.0);
        }

        fn mm_write(row : u32, col : u32, value : VecT) {
            if (row < uniforms.dimAOuter && col < uniforms.dimBOuter)
            {
                let index : u32 = col + row * uniforms.dimBOuter / 4u;
                resultMatrix.numbers[index] = value;
            }
        }

        const RowPerThread : u32 = kTileSize / kWorkgroupSizeY; // 4
        const ColPerThread : u32 = kTileSize / kWorkgroupSizeX; // 4
        const TileOuter : u32 = kTileSize;
        const TileInner : u32 = kTileSize;

        // The code below uses an unrolled loop to fill BCached.
        // If this count changes, then you also have to modify that code.
        const_assert ColPerThread == 4;
        )";
    return ss.str();
}
const std::string& kMatMulVec4SharedArray1D = R"(
        var<workgroup> mm_Asub : array<VecT, 256>;
        var<workgroup> mm_Bsub : array<VecT, 256>;)";
const std::string& kMatMulVec4SharedArray2D = R"(
        var<workgroup> mm_Asub : array<array<VecT, 8>, kTileSize>;
        var<workgroup> mm_Bsub : array<array<VecT, 8>, kTileSize>;)";
const std::string& kMatMulVec4BodyPart1 = R"(
        @compute @workgroup_size(8, 8, 1)
        fn main(@builtin(local_invocation_id) local_id : vec3u,
                @builtin(global_invocation_id) global_id  : vec3u) {
            let tileRow : u32 = local_id.y * RowPerThread;
            let tileCol : u32 = local_id.x;

            let globalRow : u32 = global_id.y * RowPerThread;
            let globalCol : u32 = global_id.x;

            let numTiles : u32 = (uniforms.dimInner - 1u) / TileInner + 1u;

            var acc: array<VecT, ColPerThread * RowPerThread / VecLen >;
            var ACached : VecT;
            var BCached : array<VecT, ColPerThread>;

            var globalColA : u32 = tileCol;
            const RowPerThreadB : u32 = TileInner / kWorkgroupSizeY;
            let tileRowB : u32 = local_id.y * RowPerThreadB;

            // Loop over shared dimension.
            for (var t : u32 = 0u; t < numTiles; t++) {
                // Load one tile of A into local memory.
                for (var innerRow : u32 = 0u; innerRow < RowPerThread; innerRow++) {
                    let inputRow : u32 = tileRow + innerRow;
                    let inputCol : u32 = tileCol;)";
const std::string& kMatMulVec4BodyPart2Array1D = R"(
                    let index : u32 = inputRow * TileInner / ColPerThread + inputCol;
                    mm_Asub[index] = mm_readA(globalRow + innerRow, globalColA);
                }
                globalColA = globalColA + TileInner / ColPerThread;

                // Load one tile of B into local memory.
                for (var innerRow : u32 = 0u; innerRow < RowPerThreadB; innerRow++) {
                    let inputRow : u32 = tileRowB + innerRow;
                    let inputCol : u32 = tileCol;
                    let index : u32 = inputRow * TileOuter / ColPerThread + inputCol;
                    mm_Bsub[index] = mm_readB(t * TileInner + inputRow, globalCol);;
                }

                workgroupBarrier();

                // Compute acc values for a single thread.
                for (var k : u32 = 0u; k < TileInner / ColPerThread; k++) {
                    BCached[0] = mm_Bsub[(k * ColPerThread) * (TileOuter / ColPerThread) + tileCol];
                    BCached[1] = mm_Bsub[(k * ColPerThread + 1u) * (TileOuter / ColPerThread) + tileCol];
                    BCached[2] = mm_Bsub[(k * ColPerThread + 2u) * (TileOuter / ColPerThread) + tileCol];
                    BCached[3] = mm_Bsub[(k * ColPerThread + 3u) * (TileOuter / ColPerThread) + tileCol];

                    for (var i : u32 = 0u; i < RowPerThread; i = i + 1u) {
                        ACached = mm_Asub[(tileRow + i) * (TileInner / ColPerThread) + k];)";
const std::string& kMatMulVec4BodyPart2Array2D = R"(
                    mm_Asub[inputRow][inputCol] = mm_readA(globalRow + innerRow, globalColA);
                }
                globalColA = globalColA + TileInner / ColPerThread;

                // Load one tile of B into local memory.
                for (var innerRow : u32 = 0u; innerRow < RowPerThreadB; innerRow++) {
                    let inputRow : u32 = tileRowB + innerRow;
                    let inputCol : u32 = tileCol;
                    mm_Bsub[inputRow][inputCol] = mm_readB(t * TileInner + inputRow, globalCol);;
                }

                workgroupBarrier();

                // Compute acc values for a single thread.
                for (var k : u32 = 0u; k < TileInner / ColPerThread; k++) {
                    BCached[0] = mm_Bsub[k * ColPerThread][tileCol];
                    BCached[1] = mm_Bsub[k * ColPerThread + 1u][tileCol];
                    BCached[2] = mm_Bsub[k * ColPerThread + 2u][tileCol];
                    BCached[3] = mm_Bsub[k * ColPerThread + 3u][tileCol];

                    for (var i : u32 = 0u; i < RowPerThread; i = i + 1u) {
                        ACached = mm_Asub[tileRow + i][k];)";
const std::string& kMatMulVec4BodyPart3 = R"(
                        acc[i] = BCached[0] * ACached.x + acc[i];
                        acc[i] = BCached[1] * ACached.y + acc[i];
                        acc[i] = BCached[2] * ACached.z + acc[i];
                        acc[i] = BCached[3] * ACached.w + acc[i];
                    }
                }

                workgroupBarrier();
            }

            for (var innerRow : u32 = 0u; innerRow < RowPerThread; innerRow++) {
                mm_write(globalRow + innerRow,
                         globalCol,
                         acc[innerRow]);
            }
        })";

std::string GenMatMulVec4OneDimensionalSharedArray() {
    return GenMatMulVec4Header() + kMatMulVec4SharedArray1D + kMatMulVec4BodyPart1 +
           kMatMulVec4BodyPart2Array1D + kMatMulVec4BodyPart3;
}

std::string GenMatMulVec4TwoDimensionalSharedArray() {
    return GenMatMulVec4Header() + kMatMulVec4SharedArray2D + kMatMulVec4BodyPart1 +
           kMatMulVec4BodyPart2Array2D + kMatMulVec4BodyPart3;
}

constexpr unsigned int kNumIterations = 50;

enum class MatMulMethod {
    MatMulFloatOneDimSharedArray,
    MatMulFloatTwoDimSharedArray,
    MatMulVec4OneDimSharedArray,
    MatMulVec4TwoDimSharedArray
};

std::ostream& operator<<(std::ostream& ostream, const MatMulMethod& matMulMethod) {
    switch (matMulMethod) {
        case MatMulMethod::MatMulFloatOneDimSharedArray:
            ostream << "MatMulFloatOneDimSharedArray";
            break;
        case MatMulMethod::MatMulFloatTwoDimSharedArray:
            ostream << "MatMulFloatTwoDimSharedArray";
            break;
        case MatMulMethod::MatMulVec4OneDimSharedArray:
            ostream << "MatMulVec4OneDimSharedArray";
            break;
        case MatMulMethod::MatMulVec4TwoDimSharedArray:
            ostream << "MatMulVec4TwoDimSharedArray";
            break;
    }
    return ostream;
}

enum class ElemType {
    F32,
    F16,
};

std::ostream& operator<<(std::ostream& ostream, const ElemType& elemType) {
    switch (elemType) {
        case ElemType::F32:
            ostream << "f32";
            break;
        case ElemType::F16:
            ostream << "f16";
            break;
    }
    return ostream;
}

using DimAOuter = uint32_t;
using DimInner = uint32_t;
using DimBOuter = uint32_t;
DAWN_TEST_PARAM_STRUCT(ShaderRobustnessParams,
                       MatMulMethod,
                       ElemType,
                       DimAOuter,
                       DimInner,
                       DimBOuter);

// Test the execution time of matrix multiplication (A [dimAOuter, dimInner] * B [dimInner,
// dimBOuter]) on the GPU and see the difference between robustness on and off.
class ShaderRobustnessPerf : public DawnPerfTestWithParams<ShaderRobustnessParams> {
  public:
    ShaderRobustnessPerf()
        : DawnPerfTestWithParams(kNumIterations, 1),
          mElemType(GetParam().mElemType),
          mDimAOuter(GetParam().mDimAOuter),
          mDimInner(GetParam().mDimInner),
          mDimBOuter(GetParam().mDimBOuter) {}
    ~ShaderRobustnessPerf() override = default;

    void SetUp() override;

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        auto requirements = DawnPerfTestWithParams<ShaderRobustnessParams>::GetRequiredFeatures();
        if ((GetParam().mElemType == ElemType::F16) &&
            SupportsFeatures({wgpu::FeatureName::ShaderF16})) {
            requirements.push_back(wgpu::FeatureName::ShaderF16);
        }
        return requirements;
    }

  private:
    void Step() override;

    // Returns the shader prefix required for parameters.
    std::string GetShaderPreamble();
    // Returns the shader body.
    std::string GetShaderBody();
    // Returns the shader source.
    std::string GetShader();

    wgpu::BindGroup mBindGroup;
    wgpu::ComputePipeline mPipeline;
    ElemType mElemType;
    uint32_t mDimAOuter;
    uint32_t mDimInner;
    uint32_t mDimBOuter;
};

void ShaderRobustnessPerf::SetUp() {
    DawnPerfTestWithParams<ShaderRobustnessParams>::SetUp();

    DAWN_TEST_UNSUPPORTED_IF((GetParam().mElemType == ElemType::F16) &&
                             !SupportsFeatures({wgpu::FeatureName::ShaderF16}));

    const size_t dataASize = mDimAOuter * mDimInner;
    std::vector<float> dataA(dataASize);
    uint64_t byteASize = sizeof(float) * dataA.size();
    // It's ok to use all zeros to do the matrix multiplication for performance test.
    wgpu::Buffer bufA =
        utils::CreateBufferFromData(device, dataA.data(), byteASize, wgpu::BufferUsage::Storage);

    const size_t dataBSize = mDimInner * mDimBOuter;
    std::vector<float> dataB(dataBSize);
    uint64_t byteBSize = sizeof(float) * dataB.size();
    wgpu::Buffer bufB =
        utils::CreateBufferFromData(device, dataB.data(), byteBSize, wgpu::BufferUsage::Storage);

    uint64_t byteDstSize = sizeof(float) * mDimAOuter * mDimBOuter;
    wgpu::BufferDescriptor desc = {};
    desc.usage = wgpu::BufferUsage::Storage;
    desc.size = byteDstSize;
    wgpu::Buffer dst = device.CreateBuffer(&desc);

    uint32_t uniformData[] = {mDimAOuter, mDimInner, mDimBOuter};
    wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
        device, uniformData, sizeof(uniformData), wgpu::BufferUsage::Uniform);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, GetShader().c_str());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    mPipeline = device.CreateComputePipeline(&csDesc);

    mBindGroup = utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0),
                                      {
                                          {0, bufA, 0, byteASize},
                                          {1, bufB, 0, byteBSize},
                                          {2, dst, 0, byteDstSize},
                                          {3, uniformBuffer, 0, sizeof(uniformData)},
                                      });
}

std::string ShaderRobustnessPerf::GetShaderPreamble() {
    switch (mElemType) {
        case ElemType::F32:
            return "alias ElemT = f32;\n";
        case ElemType::F16:
            return "enable f16;\nalias ElemT = f16;\n";
    }
    DAWN_UNREACHABLE();
}

std::string ShaderRobustnessPerf::GetShaderBody() {
    switch (GetParam().mMatMulMethod) {
        case MatMulMethod::MatMulFloatOneDimSharedArray:
            return GenMatMulFloatOneDimensionalSharedArray();
        case MatMulMethod::MatMulFloatTwoDimSharedArray:
            return GenMatMulFloatTwoDimensionalSharedArray();
        case MatMulMethod::MatMulVec4OneDimSharedArray:
            return GenMatMulVec4OneDimensionalSharedArray();
        case MatMulMethod::MatMulVec4TwoDimSharedArray:
            return GenMatMulVec4TwoDimensionalSharedArray();
    }
    DAWN_UNREACHABLE();
}

std::string ShaderRobustnessPerf::GetShader() {
    return GetShaderPreamble() + GetShaderBody();
}

void ShaderRobustnessPerf::Step() {
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
        for (unsigned int i = 0; i < kNumIterations; ++i) {
            pass.DispatchWorkgroups(ceil(static_cast<float>(mDimBOuter) / float{kTileSize}),
                                    ceil(static_cast<float>(mDimAOuter) / float{kTileSize}), 1);
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

TEST_P(ShaderRobustnessPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(ShaderRobustnessPerf,
                        {D3D12Backend({}, {"enable_integer_range_analysis_in_robustness"}),
                         D3D12Backend({"enable_integer_range_analysis_in_robustness"}, {}),
                         D3D12Backend({"disable_robustness"}, {}),
                         MetalBackend({}, {"enable_integer_range_analysis_in_robustness"}),
                         MetalBackend({"enable_integer_range_analysis_in_robustness"}, {}),
                         MetalBackend({"disable_robustness"}, {}),
                         OpenGLBackend({}, {"enable_integer_range_analysis_in_robustness"}),
                         OpenGLBackend({"enable_integer_range_analysis_in_robustness"}, {}),
                         OpenGLBackend({"disable_robustness"}, {}),
                         VulkanBackend({}, {"enable_integer_range_analysis_in_robustness"}),
                         VulkanBackend({"enable_integer_range_analysis_in_robustness"}, {}),
                         VulkanBackend({"disable_robustness"}, {})},
                        {MatMulMethod::MatMulFloatOneDimSharedArray,
                         MatMulMethod::MatMulFloatTwoDimSharedArray,
                         MatMulMethod::MatMulVec4OneDimSharedArray,
                         MatMulMethod::MatMulVec4TwoDimSharedArray},
                        {ElemType::F32, ElemType::F16},
                        {512u},
                        {512u},
                        {512u});

}  // anonymous namespace
}  // namespace dawn
