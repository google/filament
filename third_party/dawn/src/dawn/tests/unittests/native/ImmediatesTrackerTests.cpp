// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/ComputePipeline.h"
#include "src/dawn/native/ImmediatesLayout.h"
#include "src/dawn/native/ImmediatesTracker.h"
#include "src/dawn/native/RenderPipeline.h"
#include "src/dawn/tests/DawnNativeTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/assert.h"

namespace dawn::native {
namespace {
DAWN_ENABLE_STRUCT_PADDING_WARNINGS
// Define render pipeline immediate data layout for test.
struct RenderImmediateTestConstants {
    UserImmediates userImmediates;

    ClampFragDepthArgs clampFragDepth;

    // first index offset
    uint32_t firstVertex;
    uint32_t firstInstance;
};

// Define compute pipeline immediate data layout for test.
struct ComputeImmediateTestConstants {
    UserImmediates userImmediates;

    NumWorkgroupsDimensions numWorkgroups;
};
DAWN_DISABLE_STRUCT_PADDING_WARNINGS

class RenderImmediatesTestTracker
    : public UserImmediatesTrackerBase<RenderImmediateTestConstants, RenderPipelineBase> {
  public:
    RenderImmediatesTestTracker() = default;
    void SetClampFragDepth(float minClampFragDepth, float maxClampFragDepth) {
        // Put the data in the right layout to match the RenderImmediates struct
        ClampFragDepthArgs fragDepthArgs;
        fragDepthArgs.minClampFragDepth = minClampFragDepth;
        fragDepthArgs.maxClampFragDepth = maxClampFragDepth;

        UpdateImmediates(offsetof(RenderImmediateTestConstants, clampFragDepth), fragDepthArgs);
    }

    void SetFirstIndexOffset(uint32_t firstVertex, uint32_t firstInstance) {
        this->SetFirstVertex(firstVertex);
        this->SetFirstInstance(firstInstance);
    }

    void SetFirstVertex(uint32_t firstVertex) {
        UpdateImmediates(offsetof(RenderImmediateTestConstants, firstVertex), firstVertex);
    }

    void SetFirstInstance(uint32_t firstInstance) {
        UpdateImmediates(offsetof(RenderImmediateTestConstants, firstInstance), firstInstance);
    }
};

class ComputeImmediatesTestTracker
    : public UserImmediatesTrackerBase<ComputeImmediateTestConstants, ComputePipelineBase> {
  public:
    ComputeImmediatesTestTracker() = default;
    void SetNumWorkgroups(uint32_t numWorkgroupX, uint32_t numWorkgroupY, uint32_t numWorkgroupZ) {
        // Put the data in the right layout to match the ComputeImmediates struct
        NumWorkgroupsDimensions numWorkgroupsDimensions;
        numWorkgroupsDimensions.numWorkgroupsX = numWorkgroupX;
        numWorkgroupsDimensions.numWorkgroupsY = numWorkgroupY;
        numWorkgroupsDimensions.numWorkgroupsZ = numWorkgroupZ;

        UpdateImmediates(offsetof(ComputeImmediateTestConstants, numWorkgroups),
                         numWorkgroupsDimensions);
    }
};

class ImmediatesTrackerTest : public DawnNativeTest {
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

class RenderImmediatesTrackerTest : public ImmediatesTrackerTest {};

class ComputeImmediatesTrackerTest : public ImmediatesTrackerTest {};

// Test pipeline change reset dirty bits and update tracked pipeline constants mask.
TEST_F(ImmediatesTrackerTest, OnPipelineChange) {
    RenderImmediatesTestTracker tracker;

    // Control Case
    EXPECT_TRUE(tracker.GetDirtyBits() == ImmediateMask(0));

    // Pipeline change should reset dirty bits
    wgpu::RenderPipeline wgpuPipeline = MakeTestRenderPipeline();
    RenderPipelineBase* pipeline = FromAPI(wgpuPipeline.Get());
    pipeline->SetImmediateMaskForTesting({0b01010101});
    tracker.OnSetPipeline(pipeline);
    EXPECT_TRUE(tracker.GetDirtyBits() == ImmediateMask(0b01010101));

    device.Destroy();
}

// Test immediate setting update dirty bits and contents correctly.
TEST_F(ImmediatesTrackerTest, SetImmediates) {
    constexpr size_t kUserImmediateDataStartByteOffset = 0u;
    constexpr uint32_t offset = 1u * kImmediateElementByteSize;

    // Since userImmediates are the first members, we know the mask will have exactly one hole
    // (because offset is 1 element).
    ImmediateMask expectedDirtyMask = ImmediateMask(0b110);

    std::array<int32_t, 2u> newData = {-6, 8};
    // RenderImmediatesTracker
    {
        RenderImmediatesTestTracker tracker;
        tracker.SetImmediates(offset, ByteSpanFromRef(newData));

        EXPECT_TRUE(tracker.GetDirtyBits() == expectedDirtyMask);
        size_t offsetInTracker = kUserImmediateDataStartByteOffset + offset;
        EXPECT_TRUE(std::ranges::equal(
            ByteSpanFromRef(newData),
            tracker.GetContent().GetDataBytes(offsetInTracker, sizeof(newData))));
    }

    // ComputeImmediatesTracker
    {
        ComputeImmediatesTestTracker tracker;
        tracker.SetImmediates(offset, ByteSpanFromRef(newData));

        EXPECT_TRUE(tracker.GetDirtyBits() == expectedDirtyMask);
        size_t offsetInTracker = kUserImmediateDataStartByteOffset + offset;
        EXPECT_TRUE(std::ranges::equal(
            ByteSpanFromRef(newData),
            tracker.GetContent().GetDataBytes(offsetInTracker, sizeof(newData))));
    }

    device.Destroy();
}

// Test setting clamp frag depth args with float value updates dirty bits and contents correctly.
TEST_F(RenderImmediatesTrackerTest, SetClampFragDepth) {
    RenderImmediatesTestTracker tracker;
    float minClampFragDepth = 0.1;
    float maxClampFragDepth = 0.95;
    tracker.SetClampFragDepth(minClampFragDepth, maxClampFragDepth);

    ImmediateMask expected;
    // Hard coded to verify dirty bit.
    expected |=
        1u << (offsetof(RenderImmediateTestConstants, clampFragDepth) / kImmediateElementByteSize);
    expected |=
        1u << (offsetof(RenderImmediateTestConstants, clampFragDepth) / kImmediateElementByteSize +
               1u);
    EXPECT_TRUE(tracker.GetDirtyBits() == expected);

    // Compare bits instead of values here to ensure bits level equality.
    size_t clampFragDepthStartOffsetBytes = offsetof(RenderImmediateTestConstants, clampFragDepth);
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
TEST_F(RenderImmediatesTrackerTest, SetFirstIndexOffset) {
    size_t firstVertexByteOffset = offsetof(RenderImmediateTestConstants, firstVertex);
    size_t firstInstanceByteOffset = offsetof(RenderImmediateTestConstants, firstInstance);
    // SetFirstIndexOffset()
    {
        RenderImmediatesTestTracker tracker;
        uint32_t firstVertex = 1;
        uint32_t firstInstance = 2;
        tracker.SetFirstIndexOffset(firstVertex, firstInstance);

        ImmediateMask expected;
        // Hard coded to verify dirty bit.
        expected |=
            1u << offsetof(RenderImmediateTestConstants, firstVertex) / kImmediateElementByteSize;
        expected |=
            1u << offsetof(RenderImmediateTestConstants, firstInstance) / kImmediateElementByteSize;
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstVertexByteOffset), &firstVertex,
                           sizeof(uint32_t)) == 0);
        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstInstanceByteOffset),
                           &firstInstance, sizeof(uint32_t)) == 0);
    }

    // SetFirstVertex()
    {
        RenderImmediatesTestTracker tracker;
        uint32_t firstVertex = 1;
        tracker.SetFirstVertex(firstVertex);

        ImmediateMask expected;
        // Hard coded to verify dirty bit.
        expected |=
            1u << offsetof(RenderImmediateTestConstants, firstVertex) / kImmediateElementByteSize;
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstVertexByteOffset), &firstVertex,
                           sizeof(uint32_t)) == 0);
    }

    // SetFirstInstance()
    {
        RenderImmediatesTestTracker tracker;
        uint32_t firstInstance = 2;
        tracker.SetFirstInstance(firstInstance);

        ImmediateMask expected;
        // Hard coded to verify dirty bit.
        expected |=
            1u << offsetof(RenderImmediateTestConstants, firstInstance) / kImmediateElementByteSize;
        EXPECT_TRUE(tracker.GetDirtyBits() == expected);

        EXPECT_TRUE(memcmp(tracker.GetContent().Get<uint32_t>(firstInstanceByteOffset),
                           &firstInstance, sizeof(uint32_t)) == 0);
    }

    device.Destroy();
}

// Test setting num workgroups dimensions update dirty bits and contents correctly.
TEST_F(ComputeImmediatesTrackerTest, SetNumWorkgroupDimensions) {
    ComputeImmediatesTestTracker tracker;
    uint32_t numWorkgroupsX = 256;
    uint32_t numWorkgroupsY = 128;
    uint32_t numWorkgroupsZ = 64;
    tracker.SetNumWorkgroups(256, 128, 64);

    ImmediateMask expected;
    // Hard coded to verify dirty bit.
    expected |=
        1u << offsetof(ComputeImmediateTestConstants, numWorkgroups) / kImmediateElementByteSize;
    expected |=
        1u << (offsetof(ComputeImmediateTestConstants, numWorkgroups) / kImmediateElementByteSize +
               1u);
    expected |=
        1u << (offsetof(ComputeImmediateTestConstants, numWorkgroups) / kImmediateElementByteSize +
               2u);
    EXPECT_TRUE(tracker.GetDirtyBits() == expected);

    size_t numWorkgroupsStartByteOffset = offsetof(ComputeImmediateTestConstants, numWorkgroups);
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

// Regression test: GetImmediateIndexInPipeline must count preceding set bits correctly even for
// layout offsets >= 32. A 32-bit shift base overflows (UB) for offsets like
// storageBufferDynamicOffsets (bit 42/43), producing a wrong compacted index. On x86 the shift
// count is masked mod 32, so `1u << 42` silently becomes `1u << 10`; bit 20 falls strictly between
// that wrapped boundary and the true boundary at 42, so it is the bit that exposes the bug (it must
// be counted, but a wrapped prefix mask of only 10 bits would miss it).
TEST(ImmediatesLayoutTest, IndexInPipelineHandlesHighBitOffsets) {
    // Set 5 bits below bit 42, one bit between the wrapped (10) and true (42) boundary, plus bits
    // at/above 42 that must NOT be counted.
    ImmediateMask mask;
    for (uint32_t i = 0; i < 5; ++i) {
        mask.set(ImmediateIndex(i));
    }
    mask.set(ImmediateIndex(20u));
    mask.set(ImmediateIndex(42u));
    mask.set(ImmediateIndex(50u));

    // Compacted index of bit 42 = count of set bits strictly below 42 = 6.
    EXPECT_EQ(GetImmediateIndexInPipeline(42u, mask), 6u);

    // A low offset still works (guards against a regression the other direction).
    EXPECT_EQ(GetImmediateIndexInPipeline(3u, mask), 3u);
}

// GetImmediateBlockBits must set the correct high bits for members past bit 32. A 32-bit shift base
// overflows for blocks starting at offsets like storageBufferDynamicOffsets (bit 42/43), producing
// wrong bits and a corrupt immediate mask.
TEST(ImmediatesLayoutTest, BlockBitsHandlesHighBitOffsets) {
    // A 4-slot block starting at byte offset 168 (bit 42) - matches storageBufferDynamicOffsets.
    ImmediateMask bits = GetImmediateBlockBits(/*byteOffset=*/168u, /*byteSize=*/16u);
    EXPECT_EQ(bits.count(), 4u);
    EXPECT_TRUE(bits.test(ImmediateIndex(42u)));
    EXPECT_TRUE(bits.test(ImmediateIndex(45u)));
    EXPECT_FALSE(bits.test(ImmediateIndex(41u)));
    EXPECT_FALSE(bits.test(ImmediateIndex(46u)));
}

}  // anonymous namespace
}  // namespace dawn::native
