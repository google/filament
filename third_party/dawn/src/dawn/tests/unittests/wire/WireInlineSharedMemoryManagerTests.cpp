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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "gtest/gtest.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/wire/InlineSharedMemoryManager.h"
#include "src/utils/platform.h"

namespace dawn::wire {
namespace {

class InlineSharedMemoryManagerTest : public testing::Test {
  protected:
    void SetUp() override {
#if !DAWN_PLATFORM_IS(WINDOWS)
        GTEST_SKIP() << "InlineSharedMemoryManager is only implemented on Windows";
#endif
        mManager = CreateInlineSharedMemoryManager();
    }

    std::shared_ptr<InlineSharedMemoryManager> mManager;
};

TEST_F(InlineSharedMemoryManagerTest, CreateSharedMemory_ReturnsNonNull) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(1024);
    EXPECT_NE(nullptr, memory.Get());
}

TEST_F(InlineSharedMemoryManagerTest, CreateSharedMemory_MultipleBuffersAreDistinct) {
    Ref<SharedMemory> memory1 = mManager->CreateSharedMemory(256);
    Ref<SharedMemory> memory2 = mManager->CreateSharedMemory(256);
    EXPECT_NE(nullptr, memory1.Get());
    EXPECT_NE(nullptr, memory2.Get());
    EXPECT_NE(memory1.Get(), memory2.Get());
}

TEST_F(InlineSharedMemoryManagerTest, GetMappedSpan_ReturnsNonEmptySpanWithCorrectSize) {
    constexpr size_t kSize = 1024;
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(kSize);
    std::span<std::byte> span = memory->GetMappedSpan();
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(kSize, span.size());
    EXPECT_NE(nullptr, span.data());
}

TEST_F(InlineSharedMemoryManagerTest, GetHandle_ReturnsNonNullHandle) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    EXPECT_TRUE(memory->GetSystemHandle().IsValid());
}

TEST_F(InlineSharedMemoryManagerTest, PutOnWire_ReturnsUniqueIds) {
    Ref<SharedMemory> memory1 = mManager->CreateSharedMemory(256);
    Ref<SharedMemory> memory2 = mManager->CreateSharedMemory(256);
    SharedMemoryID id1 = mManager->PutOnWireAndGetID(memory1.Get());
    SharedMemoryID id2 = mManager->PutOnWireAndGetID(memory2.Get());
    EXPECT_NE(id1, id2);
}

TEST_F(InlineSharedMemoryManagerTest, AcquireFromWire_ReturnsSameMemory) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    SharedMemoryID id = mManager->PutOnWireAndGetID(memory.Get());
    Ref<SharedMemory> acquired = mManager->AcquireFromWire(id);
    EXPECT_EQ(memory.Get(), acquired.Get());
}

TEST_F(InlineSharedMemoryManagerTest, AcquireFromWire_ReturnsNullForUnknownId) {
    EXPECT_EQ(nullptr, mManager->AcquireFromWire(SharedMemoryID(0u)).Get());
    EXPECT_EQ(nullptr, mManager->AcquireFromWire(SharedMemoryID(99999u)).Get());
}

TEST_F(InlineSharedMemoryManagerTest, AcquireFromWire_RemovesFromWire) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    SharedMemoryID id = mManager->PutOnWireAndGetID(memory.Get());
    EXPECT_NE(nullptr, mManager->AcquireFromWire(id).Get());

    // A second acquire finds nothing since the reference was already transferred off the wire.
    EXPECT_EQ(nullptr, mManager->AcquireFromWire(id).Get());
}

TEST_F(InlineSharedMemoryManagerTest, PutOnWire_KeepsMemoryAliveAfterLocalRefDropped) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    SharedMemory* rawSharedMemoryPtr = memory.Get();
    SharedMemoryID id = mManager->PutOnWireAndGetID(rawSharedMemoryPtr);

    // Drop the local reference; the wire still holds one so the memory stays alive.
    memory = nullptr;

    Ref<SharedMemory> acquired = mManager->AcquireFromWire(id);
    EXPECT_EQ(rawSharedMemoryPtr, acquired.Get());
    EXPECT_FALSE(acquired->GetMappedSpan().empty());
}

TEST_F(InlineSharedMemoryManagerTest, DataRoundtrip_WriteAndReadBack) {
    constexpr size_t kSize = 256;
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(kSize);

    std::span<std::byte> span = memory->GetMappedSpan();
    ASSERT_EQ(kSize, span.size());

    // Write a known pattern.
    for (size_t i = 0; i < kSize; ++i) {
        span[i] = static_cast<std::byte>(i);
    }

    // Put the memory on the wire and retrieve it back.
    SharedMemoryID id = mManager->PutOnWireAndGetID(memory.Get());
    Ref<SharedMemory> acquired = mManager->AcquireFromWire(id);
    ASSERT_NE(nullptr, acquired.Get());

    // Read back through the acquired reference and compare the data with the expected values.
    std::span<std::byte> readSpan = acquired->GetMappedSpan();
    ASSERT_EQ(kSize, readSpan.size());
    for (size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(static_cast<std::byte>(i), readSpan[i]) << " at index " << i;
    }
}

TEST_F(InlineSharedMemoryManagerTest, MultipleBuffers_DataIsIsolated) {
    constexpr size_t kSize = 128;
    Ref<SharedMemory> memory1 = mManager->CreateSharedMemory(kSize);
    Ref<SharedMemory> memory2 = mManager->CreateSharedMemory(kSize);

    std::span<std::byte> span1 = memory1->GetMappedSpan();
    std::span<std::byte> span2 = memory2->GetMappedSpan();

    constexpr std::byte kData1 = std::byte{0xAA};
    constexpr std::byte kData2 = std::byte{0xBB};
    std::fill(span1.begin(), span1.end(), kData1);
    std::fill(span2.begin(), span2.end(), kData2);

    for (std::byte b : memory1->GetMappedSpan()) {
        EXPECT_EQ(kData1, b);
    }
    for (std::byte b : memory2->GetMappedSpan()) {
        EXPECT_EQ(kData2, b);
    }
}

}  // namespace
}  // namespace dawn::wire
