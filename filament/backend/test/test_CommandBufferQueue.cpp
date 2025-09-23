/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "private/backend/CommandBufferQueue.h"
#include "private/backend/CommandStream.h" // For NoopCommand
#include "private/backend/CircularBuffer.h"

#include <utils/Panic.h> // For assert_invariant

#include <thread>
#include <vector>
#include <numeric>
#include <atomic>
#include <chrono>
#include <future>
#include <cstring>

using namespace std::chrono_literals;

namespace filament::backend {

class CommandBufferQueueTest : public ::testing::Test {
protected:
    // Helper to write some data into the circular buffer.
    void writeData(CircularBuffer& cb, size_t size, uint8_t value) {
        if (size == 0) return;
        void* p = cb.allocate(size);
        ASSERT_NE(p, nullptr);
        memset(p, value, size);
    }

    // Helper to verify data in a command buffer range.
    void verifyData(const CommandBufferQueue::Range& range, size_t expectedDataSize, uint8_t expectedValue) {
        size_t rangeSize = (char*)range.end - (char*)range.begin;
        ASSERT_GE(rangeSize, sizeof(NoopCommand));

        size_t actualDataSize = rangeSize - sizeof(NoopCommand);
        ASSERT_EQ(actualDataSize, expectedDataSize);

        const uint8_t* data = static_cast<const uint8_t*>(range.begin);
        for (size_t i = 0; i < actualDataSize; ++i) {
            ASSERT_EQ(data[i], expectedValue) << "Mismatch at index " << i;
        }
    }

    // Helper to calculate the expected rounded-up size.
    static size_t roundUp(size_t v, size_t multiple) {
        return (v + multiple - 1) & ~(multiple - 1);
    }
};

TEST_F(CommandBufferQueueTest, BasicProduceConsume) {
    // Test with a standard, valid configuration.
    CommandBufferQueue queue(1024, 4096, false);
    std::atomic<bool> consumerFinished = false;
    const size_t testDataSize = 500;

    // Consumer thread
    std::thread consumer([&]() {
        auto buffers = queue.waitForCommands();
        ASSERT_EQ(buffers.size(), 1);
        verifyData(buffers[0], testDataSize, 0xAA);
        queue.releaseBuffer(buffers[0]);
        consumerFinished = true;
    });

    // Producer (main thread)
    std::this_thread::sleep_for(10ms); // Wait a moment to ensure the consumer is likely waiting.
    writeData(queue.getCircularBuffer(), testDataSize, 0xAA);
    queue.flush();

    consumer.join();
    EXPECT_TRUE(consumerFinished);
}

TEST_F(CommandBufferQueueTest, MultipleFlushes) {
    // Test with a standard, valid configuration.
    CommandBufferQueue queue(1024, 4096, false);
    std::atomic<int> buffersProcessed = 0;

    const size_t sizes[] = { 100, 200, 50 };
    const uint8_t values[] = { 1, 2, 3 };
    constexpr int NUM_BUFFERS = 3;

    std::thread consumer([&]() {
        int buffersSeen = 0;
        while (buffersSeen < NUM_BUFFERS) {
            auto buffers = queue.waitForCommands();
            if (buffers.empty()) {
                // Spurious wakeup or exit requested, just continue waiting.
                continue;
            }
            for (const auto& buffer : buffers) {
                ASSERT_LT(buffersSeen, NUM_BUFFERS);
                // Verify the buffer based on the order it was sent.
                verifyData(buffer, sizes[buffersSeen], values[buffersSeen]);
                queue.releaseBuffer(buffer);
                buffersSeen++;
            }
        }
        buffersProcessed = buffersSeen;
    });

    // Producer flushes multiple times. The consumer may see these in one or more batches.
    writeData(queue.getCircularBuffer(), sizes[0], values[0]);
    queue.flush();
    writeData(queue.getCircularBuffer(), sizes[1], values[1]);
    queue.flush();
    writeData(queue.getCircularBuffer(), sizes[2], values[2]);
    queue.flush();

    consumer.join();
    EXPECT_EQ(buffersProcessed.load(), NUM_BUFFERS);
}

TEST_F(CommandBufferQueueTest, RequestExit) {
    CommandBufferQueue queue(1024, 4096, false);
    std::promise<void> consumerIsWaiting;
    std::atomic<bool> didFinish = false;

    std::thread consumer([&]() {
        consumerIsWaiting.set_value();
        auto buffers = queue.waitForCommands();
        EXPECT_TRUE(buffers.empty());
        EXPECT_TRUE(queue.isExitRequested());
        didFinish = true;
    });

    // Wait until the consumer is definitely blocked.
    consumerIsWaiting.get_future().wait();
    std::this_thread::sleep_for(10ms);

    queue.requestExit();
    consumer.join();
    EXPECT_TRUE(didFinish);
}

TEST_F(CommandBufferQueueTest, Backpressure) {
    const size_t blockSize = CircularBuffer::getBlockSize();

    // To test backpressure, we set a requiredSize that is a large fraction of the total bufferSize.
    const size_t rawRequiredSize = blockSize * 2;
    const size_t bufferSize = blockSize * 3; // Use a buffer that is larger than requiredSize.

    // Let the constructor sanitize the sizes. We query the actual values for our calculations.
    CommandBufferQueue queue(rawRequiredSize, bufferSize, false);
    const size_t requiredSize = queue.getCapacity();
    const size_t actualBufferSize = queue.getCircularBuffer().size();

    // Sanity check the test parameters. We need enough room in the buffer to conduct the test.
    ASSERT_GE(actualBufferSize - requiredSize, sizeof(NoopCommand) + 1)
            << "Test setup invalid: not enough headroom to test backpressure.";

    // 1. Fill the buffer so that after this flush, free space is EXACTLY the required size.
    // This flush should pass without blocking.
    // We want: actualBufferSize - used_in_flush_1 = requiredSize
    // This means: used_in_flush_1 = actualBufferSize - requiredSize
    // Since used = firstAlloc + sizeof(NoopCommand), then:
    const size_t firstAlloc = actualBufferSize - requiredSize - sizeof(NoopCommand);
    writeData(queue.getCircularBuffer(), firstAlloc, 1);
    queue.flush(); // This MUST NOT block.

    // At this point, a command buffer is in the queue, and mFreeSpace == requiredSize.

    std::promise<void> producerIsReadyToFlush;
    std::atomic<bool> producerUnblocked = false;

    // 2. This producer thread will make a small allocation and then flush.
    // This second flush MUST block, because any new allocation makes mFreeSpace < requiredSize.
    std::thread producer([&]() {
        // Make a small allocation. This makes the buffer non-empty so flush() proceeds.
        writeData(queue.getCircularBuffer(), 128, 2);

        producerIsReadyToFlush.set_value();
        queue.flush(); // This call commits its buffer, then blocks.
        producerUnblocked = true;
    });

    // 3. Wait for the producer to be ready to flush, then give it time to enter the blocking call.
    producerIsReadyToFlush.get_future().wait();
    std::this_thread::sleep_for(50ms); // Allow time for producer to hit mCondition.wait()
    EXPECT_FALSE(producerUnblocked.load()) << "Producer should be blocked in flush().";

    // 4. Now, as the main thread (acting as consumer), wait for commands.
    // It should receive *both* buffers: the one from the main thread and the one
    // from the producer thread that is now blocked.
    auto buffers = queue.waitForCommands();
    ASSERT_EQ(buffers.size(), 2);
    verifyData(buffers[0], firstAlloc, 1);
    verifyData(buffers[1], 128, 2);

    // 5. Releasing the first buffer should free up enough space to unblock the producer.
    queue.releaseBuffer(buffers[0]);

    // 6. The producer thread should now unblock and finish.
    producer.join();
    EXPECT_TRUE(producerUnblocked.load()) << "Producer did not unblock after consumer released buffer.";

    // 7. Finally, release the second buffer to clean up the queue.
    queue.releaseBuffer(buffers[1]);
}

TEST_F(CommandBufferQueueTest, Pause) {
    const size_t blockSize = CircularBuffer::getBlockSize();
    CommandBufferQueue queue(blockSize, blockSize*2, false);

    writeData(queue.getCircularBuffer(), 100, 0xCC);
    queue.flush();

    queue.setPaused(true);
    EXPECT_TRUE(queue.isPaused());

    std::atomic<bool> consumerGotBuffers = false;
    std::thread consumer([&]{
        // This should block because the queue is paused.
        auto buffers = queue.waitForCommands();
        // It will only unblock when un-paused.
        if (!buffers.empty()) {
            consumerGotBuffers = true;
            queue.releaseBuffer(buffers[0]);
        }
    });

    // Wait and verify consumer is still blocked.
    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(consumerGotBuffers);

    // Unpause the queue.
    queue.setPaused(false);
    EXPECT_FALSE(queue.isPaused());

    // The consumer should now unblock.
    consumer.join();
    EXPECT_TRUE(consumerGotBuffers);
}

TEST_F(CommandBufferQueueTest, StressTest) {
    const size_t bufferSize = 1 * 1024 * 1024; // 1 MB
    const size_t requiredSize = 256 * 1024;    // 256 KB
    CommandBufferQueue queue(requiredSize, bufferSize, false);

    std::atomic<uint64_t> totalDataProduced = 0;
    std::atomic<uint64_t> totalDataConsumed = 0;
    std::atomic<bool> producerDone = false;

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 500; ++i) {
            size_t size = 1024 + (i * 13) % 8192;
            void* p = queue.getCircularBuffer().allocate(size);
            ASSERT_NE(p, nullptr);
            *((int*)p) = i; // Write a sequence number for verification.
            totalDataProduced += size;
            queue.flush();
        }
        producerDone = true;
        queue.requestExit(); // Final flush to wake consumer if it's waiting
    });

    // Consumer thread
    std::thread consumer([&]() {
        int expectedSequence = 0;
        while (true) {
            auto buffers = queue.waitForCommands();
            if (buffers.empty()) {
                if (producerDone) break; // Normal exit condition.
                continue; // Spurious wakeup.
            }
            for (const auto& range : buffers) {
                size_t rangeSize = (char*)range.end - (char*)range.begin;
                if (rangeSize > sizeof(NoopCommand)) {
                    size_t dataSize = rangeSize - sizeof(NoopCommand);
                    int sequence = *((int*)range.begin);
                    ASSERT_EQ(sequence, expectedSequence);
                    expectedSequence++;
                    totalDataConsumed += dataSize;
                }
                queue.releaseBuffer(range);
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(totalDataProduced, totalDataConsumed);
    EXPECT_GT(totalDataConsumed, 0);
}


} // namespace filament::backend
