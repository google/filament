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

#include <limits>

#include "dawn/tests/DawnTest.h"

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
TEST_P(AllocatorMemoryInstrumentationTest, GetAllocatorMemoryInfoVulkan) {
    // Create a buffer with size 32.
    constexpr uint64_t kBufferSize = 32;
    constexpr wgpu::BufferDescriptor kBufferDesc = {
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = kBufferSize,
    };

    // Creating the buffer should allocate memory with Vulkan ResourceMemoryAllocator.
    wgpu::Buffer uniformBuffer = device.CreateBuffer(&kBufferDesc);
    EXPECT_TRUE(uniformBuffer);

    native::AllocatorMemoryInfo memInfo = native::GetAllocatorMemoryInfo(device.Get());
    EXPECT_GT(memInfo.totalAllocatedMemory, 0u);
    EXPECT_GT(memInfo.totalUsedMemory, 0u);
    EXPECT_GE(memInfo.totalAllocatedMemory, memInfo.totalUsedMemory);
    auto prevAllocatedMemory = memInfo.totalAllocatedMemory;

    uniformBuffer.Destroy();
    // Process the destroy so that the pending command serials are updated.

    // Need an empty Submit in order for Future to wait for queue serial.
    device.GetQueue().Submit(0, nullptr);
    // Use Futures WaitAny to wait for the queue to update serial.
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = device.GetQueue().OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                                                            [](wgpu::QueueWorkDoneStatus) {});
    const auto& instance = device.GetAdapter().GetInstance();
    auto status =
        instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    ASSERT_EQ(status, wgpu::WaitStatus::Success);

    // TODO(chromium:404568017): This submit should not really be needed to free the memory but is
    // needed for now. Investigate this.
    device.GetQueue().Submit(0, nullptr);
    // Tick needed in order to call ResourceMemoryAllocator::Tick.
    device.Tick();

    memInfo = native::GetAllocatorMemoryInfo(device.Get());
    // Vulkan used memory should be 0 now.
    EXPECT_EQ(memInfo.totalUsedMemory, 0u);
    EXPECT_LE(memInfo.totalAllocatedMemory, prevAllocatedMemory);
}

DAWN_INSTANTIATE_TEST(AllocatorMemoryInstrumentationTest, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
