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

#include "src/dawn/tests/DawnTest.h"

namespace dawn {
namespace {

class AllocatorMemoryInstrumentationTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Native GetAllocatorMemoryInfo method is unsupported on wire.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test the detailed memory usage reported by GetAllocatorMemoryInfo()
TEST_P(AllocatorMemoryInstrumentationTest, GetAllocatorMemoryInfo) {
    native::AllocatorMemoryInfo memInfo = native::GetAllocatorMemoryInfo(device.Get());
    auto usedMemoryInInitialization = memInfo.totalUsedMemory;

    // Create a buffer with size 32.
    constexpr uint64_t kBufferSize = 32;
    constexpr wgpu::BufferDescriptor kBufferDesc = {
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = kBufferSize,
    };

    // Creating the buffer should allocate memory with ResourceMemoryAllocator.
    wgpu::Buffer uniformBuffer = device.CreateBuffer(&kBufferDesc);
    EXPECT_TRUE(uniformBuffer);

    memInfo = native::GetAllocatorMemoryInfo(device.Get());
    EXPECT_GT(memInfo.totalAllocatedMemory, 0u);
    EXPECT_GT(memInfo.totalUsedMemory, 0u);
    EXPECT_GE(memInfo.totalAllocatedMemory, memInfo.totalUsedMemory);
    auto prevAllocatedMemory = memInfo.totalAllocatedMemory;

    uniformBuffer.Destroy();

    // Reclaiming the buffer's memory is handled by ResourceMemoryAllocator, which is registered as
    // a BestEffort (Lowest) priority serial processor. As documented on QueuePriority, Lowest
    // priority work is intentionally *not* processed by user-facing waits such as WaitAny; those
    // only process UserVisible and higher priority work in order to stay responsive. Lowest
    // priority work is only processed by device.Tick(), which in turn only does work while the
    // queue still has scheduled commands. So issue an empty submit to give the queue scheduled
    // work, then tick until the queue is idle to let the Lowest-priority reclamation run.
    device.GetQueue().Submit(0, nullptr);
    WaitForAllOperations();

    memInfo = native::GetAllocatorMemoryInfo(device.Get());
    EXPECT_EQ(memInfo.totalUsedMemory, usedMemoryInInitialization);
    EXPECT_LE(memInfo.totalAllocatedMemory, prevAllocatedMemory);
}

DAWN_INSTANTIATE_TEST(AllocatorMemoryInstrumentationTest, D3D12Backend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
