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

#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

enum class LoopKind {
    kLoop,            // loop { if breakCond {break;}}
    kLoopContinuing,  // loop { continuing { break if breakCond; } }}
    kFor,             // for ( ; !breakCond ; ) { }
    kWhile,           // while ( !breakCond ) { }
};
std::ostream& operator<<(std::ostream& os, const LoopKind lk) {
    switch (lk) {
        case LoopKind::kLoop:
            os << "loop";
            break;
        case LoopKind::kLoopContinuing:
            os << "loopContinuing";
            break;
        case LoopKind::kFor:
            os << "for";
            break;
        case LoopKind::kWhile:
            os << "while";
            break;
    }
    return os;
}

// Conditions that are always false, and cause the loop to iterate forever.
enum class BreakCond {
    kFalse,
    kClampedIndexEqualsArrayLen,
    kClampedIndexExceedsArrayLen,
};
std::ostream& operator<<(std::ostream& os, const BreakCond bc) {
    switch (bc) {
        case BreakCond::kFalse:
            os << "false";
            break;
        case BreakCond::kClampedIndexEqualsArrayLen:
            os << "clampIndexEqualsArrayLen";
            break;
        case BreakCond::kClampedIndexExceedsArrayLen:
            os << "clampIndexExceedsArrayLen";
            break;
    }
    return os;
}

// What kind of expression provides the index?
enum class IndexKind {
    kConst,
    kFromBuffer,  // Use the first entry in the input buffer.
};
std::ostream& operator<<(std::ostream& os, const IndexKind ik) {
    switch (ik) {
        case IndexKind::kConst:
            os << "const";
            break;
        case IndexKind::kFromBuffer:
            os << "fromBuffer";
            break;
    }
    return os;
}

DAWN_TEST_PARAM_STRUCT(InfiniteLoopTestParams, LoopKind, BreakCond, IndexKind);

// Tests somewhat safe results from dynamically infinite loops.
// It's hard to write a conformnace test because the specified result is a dynamic
// error. But we can check for platform-specific results.

struct InfiniteLoopTests : public DawnTestWithParams<InfiniteLoopTestParams> {
    using DawnTestWithParams<InfiniteLoopTestParams>::GetParam;
    const uint32_t kArraySize = 50;
    const uint32_t kOOBIndex = 100;

    // Runs a test on an input array, yielding a buffer of outputs.
    // Checks the given expectation on the output buffer. Takes ownership
    // of the expectation object.
    void RunTest(const char* file,
                 size_t line,
                 const char* shader,
                 uint32_t OOBIndex,
                 size_t numOutputs,
                 detail::Expectation* expectation);

#define RUN_TEST(shader, inputs, numOutputs, expectation) \
    this->RunTest(__FILE__, __LINE__, shader, inputs, numOutputs, expectation)

    // Returns expression for the break condition.
    // It should always evaluate to false.
    std::string IndexStr() {
        switch (GetParam().mIndexKind) {
            case IndexKind::kConst:
                return std::to_string(kOOBIndex);
            case IndexKind::kFromBuffer:
                return "inputs[0]";
        }
        DAWN_UNREACHABLE();
    }
    // Returns expression for the break condition.
    // It should always evaluate to false.
    std::string BreakCondStr() {
        switch (GetParam().mBreakCond) {
            case BreakCond::kFalse:
                return "false";
            case BreakCond::kClampedIndexEqualsArrayLen:
                return "(min(" + IndexStr() +
                       ", arrayLength(&outputs)-1) == arrayLength(&outputs))";
            case BreakCond::kClampedIndexExceedsArrayLen:
                return "(min(" + IndexStr() + ", arrayLength(&outputs)-1) > arrayLength(&outputs))";
        }
        DAWN_UNREACHABLE();
    }
    // Returns code for the the infinite loop.
    std::string LoopStr() {
        switch (GetParam().mLoopKind) {
            case LoopKind::kLoop:
                return "loop { if " + BreakCondStr() + "{break;} }\n";
            case LoopKind::kLoopContinuing:
                return "loop { continuing { break if " + BreakCondStr() + "; } }\n";
            case LoopKind::kFor:
                return "for (; !" + BreakCondStr() + "; ) { }\n";
            case LoopKind::kWhile:
                return "while (!" + BreakCondStr() + ") { }\n";
        }
        DAWN_UNREACHABLE();
    }

    // Returns the shader string for the current parameterization.
    std::string Shader(uint32_t sentinelValue) {
        return R"(
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : array<u32>;

@compute @workgroup_size(1)
fn main() {
  _ = &outputs[0];
  _ = &inputs[0];
  )" + LoopStr() +
               R"(
  outputs[)" + IndexStr() +
               "] = " + std::to_string(sentinelValue) + R"(;
}
)";
    }
};

void InfiniteLoopTests::RunTest(const char* file,
                                size_t line,
                                const char* shader,
                                uint32_t OOBIndex,
                                size_t numOutputs,
                                detail::Expectation* expectation) {
    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up src storage buffer
    wgpu::Buffer src = utils::CreateBufferFromData(
        device, &OOBIndex, sizeof(OOBIndex),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Set up dst storage buffer
    std::vector<uint32_t> dstInitValues(numOutputs, 0);

    const auto outSize = numOutputs * sizeof(uint32_t);
    wgpu::Buffer dst = utils::CreateBufferFromData(
        device, dstInitValues.data(), outSize,
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, src},
                                                         {1, dst},
                                                     });

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

    AddBufferExpectation(file, line, dst, 0, outSize, expectation);
}

// A gtest assertion checking that the given array is filled with zeros, except for one entry which
// contains the given value.
template <typename T>
class ExpectOneNonZero : public detail::Expectation {
  public:
    explicit ExpectOneNonZero(T value) : mValue(value) { DAWN_ASSERT(mValue != T(0)); }

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size % sizeof(T) == 0 && size > 0);
        const T* actual = static_cast<const T*>(data);

        std::optional<size_t> whereFound;
        for (size_t i = 0; i < size / sizeof(T); i++) {
            if (actual[i] == mValue) {
                if (whereFound.has_value()) {
                    return testing::AssertionFailure()
                           << "Found value " << mValue << " at data[" << whereFound.value()
                           << "] and data[" << i << "]\n";
                }
                whereFound = i;
            } else if (actual[i] != 0) {
                return testing::AssertionFailure()
                       << "Found unexpected value data[" << i << "] = " << actual[i]
                       << " instead of " << mValue << "\n";
            }
        }
        if (!whereFound.has_value()) {
            return testing::AssertionFailure() << "Sentinel value " << mValue << " was not found\n";
        }
        return testing::AssertionSuccess();
    }

  private:
    const T mValue;
};

TEST_P(InfiniteLoopTests, LoopDeletedThenBoundedWrite) {
    DAWN_SKIP_TEST_IF_BASE(!IsMetal(), "infinite-loops", "only test on Metal");
    DAWN_SKIP_TEST_IF_BASE(
        IsMetal(), "infinite-loops",
        "Metal loops run forever: TODO(crbug.com/371840056) rewrite as death tests with watchdog?");

    const uint32_t sentinelValue = 777;
    std::string shader = Shader(sentinelValue);
    RUN_TEST(shader.c_str(), kOOBIndex, kArraySize, new ExpectOneNonZero<uint32_t>(sentinelValue));
}

DAWN_INSTANTIATE_TEST_P(InfiniteLoopTests,
                        {MetalBackend()},
                        {
                            LoopKind::kLoop,
                            LoopKind::kLoopContinuing,
                            LoopKind::kFor,
                            LoopKind::kWhile,
                        },
                        {
                            BreakCond::kFalse,
                            BreakCond::kClampedIndexEqualsArrayLen,
                            BreakCond::kClampedIndexExceedsArrayLen,
                        },
                        {IndexKind::kConst, IndexKind::kFromBuffer});

}  // anonymous namespace
}  // namespace dawn
