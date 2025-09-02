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

#include "dawn/common/Assert.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/ImmediateConstantsTracker.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/tests/DawnNativeTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn::native {
namespace {
class ImmediateConstantsTrackerTest : public DawnNativeTest {
  protected:
    wgpu::RenderPipeline MakeTestRenderPipeline() {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 0.0);
            }
        )");
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.1, 0.2, 0.3, 0.4);
            }
        )");
        desc.cFragment.entryPoint = "main";
        return device.CreateRenderPipeline(&desc);
    }

    wgpu::ComputePipeline MakeTestComputePipeline() {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {}
    )");
        desc.compute.entryPoint = "main";
        return device.CreateComputePipeline(&desc);
    }
};

class RenderImmediateConstantsTrackerTest : public ImmediateConstantsTrackerTest {};

class ComputeImmediateConstantsTrackerTest : public ImmediateConstantsTrackerTest {};

// Test pipeline change reset dirty bits and update tracked pipeline constants mask.
TEST_F(ImmediateConstantsTrackerTest, OnPipelineChange) {
    RenderImmediateConstantsTrackerBase tracker;

    // Control Case
    EXPECT_TRUE(tracker.GetDirtyBits() == ImmediateConstantMask(0));

    // Pipeline change should reset dirty bits
    wgpu::RenderPipeline wgpuPipeline = MakeTestRenderPipeline();
    RenderPipelineBase* pipeline = FromAPI(wgpuPipeline.Get());
    pipeline->SetImmediateMaskForTesting({0b01010101});
    tracker.OnSetPipeline(pipeline);
    EXPECT_TRUE(tracker.GetDirtyBits() == ImmediateConstantMask(0b01010101));

    device.Destroy();
}

// Test immediate setting update dirty bits and contents correctly.
TEST_F(ImmediateConstantsTrackerTest, SetImmediateData) {
    static constexpr uint32_t rangeOffset = 1u * kImmediateConstantElementByteSize;
    static constexpr uint32_t dataOffset = 2u;
    static constexpr uint32_t userImmediateDataSize = 2u * kImmediateConstantElementByteSize;
    ImmediateConstantMask expected =
        GetImmediateConstantBlockBits(0u, sizeof(UserImmediateConstants));

    size_t userImmediateDataStartByteOffset = 0u;
    // RenderImmediateConstantsTracker
    {
        RenderImmediateConstantsTrackerBase tracker;
        int32_t userImmediateData[] = {2, 4, -6, 8};
        tracker.SetImmediateData(rangeOffset,
                                 reinterpret_cast<uint8_t*>(&userImmediateData[dataOffset]),
                                 userImmediateDataSize);
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        uint32_t userImmediateDataRangeOffset = userImmediateDataStartByteOffset + rangeOffset;
        EXPECT_TRUE(memcmp(tracker.GetContent().Get<int32_t>(userImmediateDataRangeOffset),
                           &userImmediateData[dataOffset], userImmediateDataSize) == 0);
    }

    // ComputeImmediateConstantsTracker
    {
        ComputeImmediateConstantsTrackerBase tracker;
        int32_t userImmediateData[] = {2, 4, -6, 8};
        tracker.SetImmediateData(rangeOffset,
                                 reinterpret_cast<uint8_t*>(&userImmediateData[dataOffset]),
                                 userImmediateDataSize);
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        uint32_t userImmediateDataRangeOffset = userImmediateDataStartByteOffset + rangeOffset;
        EXPECT_TRUE(memcmp(tracker.GetContent().Get<int32_t>(userImmediateDataRangeOffset),
                           &userImmediateData[dataOffset], userImmediateDataSize) == 0);
    }

    device.Destroy();
}

// Test setting clamp frag depth args with float value updates dirty bits and contents correctly.
TEST_F(RenderImmediateConstantsTrackerTest, SetClampFragDepth) {
    RenderImmediateConstantsTrackerBase tracker;
    float minClampFragDepth = 0.1;
    float maxClampFragDepth = 0.95;
    tracker.SetClampFragDepth(minClampFragDepth, maxClampFragDepth);

    ImmediateConstantMask expected;
    // Hard coded to verify dirty bit.
    expected |= 1u << (offsetof(RenderImmediateConstants, clampFragDepth) /
                       kImmediateConstantElementByteSize);
    expected |= 1u << (offsetof(RenderImmediateConstants, clampFragDepth) /
                           kImmediateConstantElementByteSize +
                       1u);
    EXPECT_TRUE(tracker.GetDirtyBits() == expected);

    // Compare bits instead of values here to ensure bits level equality.
    size_t clampFragDepthStartOffsetBytes = offsetof(RenderImmediateConstants, clampFragDepth);
    size_t minClampFragDepthOffsetBytes =
        clampFragDepthStartOffsetBytes + offsetof(ClampFragDepthArgs, minClampFragDepth);
    size_t maxClampFragDepthOffsetBytes =
        clampFragDepthStartOffsetBytes + offsetof(ClampFragDepthArgs, maxClampFragDepth);
    EXPECT_TRUE(memcmp(tracker.GetContent().Get<float>(minClampFragDepthOffsetBytes),
                       &minClampFragDepth, sizeof(float)) == 0);
    EXPECT_TRUE(memcmp(tracker.GetContent().Get<float>(maxClampFragDepthOffsetBytes),
                       &maxClampFragDepth, sizeof(float)) == 0);

    device.Destroy();
}

// Test setting first index offset args updates dirty bits and contents correctly.
TEST_F(RenderImmediateConstantsTrackerTest, SetFirstIndexOffset) {
    size_t firstVertexByteOffset = offsetof(RenderImmediateConstants, firstVertex);
    size_t firstInstanceByteOffset = offsetof(RenderImmediateConstants, firstInstance);
    // SetFirstIndexOffset()
    {
        RenderImmediateConstantsTrackerBase tracker;
        uint32_t firstVertex = 1;
        uint32_t firstInstance = 2;
        tracker.SetFirstIndexOffset(firstVertex, firstInstance);

        ImmediateConstantMask expected;
        // Hard coded to verify dirty bit.
        expected |= 1u << offsetof(RenderImmediateConstants, firstVertex) /
                              kImmediateConstantElementByteSize;
        expected |= 1u << offsetof(RenderImmediateConstants, firstInstance) /
                              kImmediateConstantElementByteSize;
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstVertexByteOffset), &firstVertex,
                           sizeof(uint32_t)) == 0);
        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstInstanceByteOffset),
                           &firstInstance, sizeof(uint32_t)) == 0);
    }

    // SetFirstVertex()
    {
        RenderImmediateConstantsTrackerBase tracker;
        uint32_t firstVertex = 1;
        tracker.SetFirstVertex(firstVertex);

        ImmediateConstantMask expected;
        // Hard coded to verify dirty bit.
        expected |= 1u << offsetof(RenderImmediateConstants, firstVertex) /
                              kImmediateConstantElementByteSize;
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstVertexByteOffset), &firstVertex,
                           sizeof(uint32_t)) == 0);
    }

    // SetFirstInstance()
    {
        RenderImmediateConstantsTrackerBase tracker;
        uint32_t firstInstance = 2;
        tracker.SetFirstInstance(firstInstance);

        ImmediateConstantMask expected;
        // Hard coded to verify dirty bit.
        expected |= 1u << offsetof(RenderImmediateConstants, firstInstance) /
                              kImmediateConstantElementByteSize;
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstInstanceByteOffset),
                           &firstInstance, sizeof(uint32_t)) == 0);
    }

    device.Destroy();
}

// Test setting num workgroups dimensions update dirty bits and contents correctly.
TEST_F(ComputeImmediateConstantsTrackerTest, SetNumWorkgroupDimensions) {
    ComputeImmediateConstantsTrackerBase tracker;
    uint32_t numWorkgroupsX = 256;
    uint32_t numWorkgroupsY = 128;
    uint32_t numWorkgroupsZ = 64;
    tracker.SetNumWorkgroups(256, 128, 64);

    ImmediateConstantMask expected;
    // Hard coded to verify dirty bit.
    expected |= 1u << offsetof(ComputeImmediateConstants, numWorkgroups) /
                          kImmediateConstantElementByteSize;
    expected |= 1u << (offsetof(ComputeImmediateConstants, numWorkgroups) /
                           kImmediateConstantElementByteSize +
                       1u);
    expected |= 1u << (offsetof(ComputeImmediateConstants, numWorkgroups) /
                           kImmediateConstantElementByteSize +
                       2u);
    EXPECT_TRUE(tracker.GetDirtyBits() == expected);

    size_t numWorkgroupsStartByteOffset = offsetof(ComputeImmediateConstants, numWorkgroups);
    size_t numWorkgroupXByteOffset =
        numWorkgroupsStartByteOffset + offsetof(NumWorkgroupsDimensions, numWorkgroupsX);
    size_t numWorkgroupYByteOffset =
        numWorkgroupsStartByteOffset + offsetof(NumWorkgroupsDimensions, numWorkgroupsY);
    size_t numWorkgroupZByteOffset =
        numWorkgroupsStartByteOffset + offsetof(NumWorkgroupsDimensions, numWorkgroupsZ);
    EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(numWorkgroupXByteOffset), &numWorkgroupsX,
                       sizeof(uint32_t)) == 0);

    EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(numWorkgroupYByteOffset), &numWorkgroupsY,
                       sizeof(uint32_t)) == 0);
    EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(numWorkgroupZByteOffset), &numWorkgroupsZ,
                       sizeof(uint32_t)) == 0);
    device.Destroy();
}

}  // anonymous namespace
}  // namespace dawn::native
