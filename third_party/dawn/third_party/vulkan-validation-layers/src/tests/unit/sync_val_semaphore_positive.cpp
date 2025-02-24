/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>
#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"

struct PositiveSyncValTimelineSemaphore : public VkSyncValTest {};

TEST_F(PositiveSyncValTimelineSemaphore, WaitInitialValue) {
    TEST_DESCRIPTION("Wait on the initial value");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE, 1);
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignal) {
    TEST_DESCRIPTION("Signal then wait for signaled value");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalSync1) {
    TEST_DESCRIPTION("Signal then wait for signaled value");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(m_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_default_queue->Submit(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalTwoQueues) {
    TEST_DESCRIPTION("Signal then wait for signaled value");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalSmallerValue) {
    TEST_DESCRIPTION("Signal a value then wait for a smaller value");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 2));
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalSmallerValueTwoQueues) {
    TEST_DESCRIPTION("Signal a value then wait for a smaller value");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 2));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalNonDefaultStage) {
    TEST_DESCRIPTION("Signal and wait with specific synchronization scopes");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 3, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalNonDefaultStage2) {
    TEST_DESCRIPTION("Signal and wait with specific synchronization scopes");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 3, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitAfterSignalNonDefaultStageTwoQueues) {
    TEST_DESCRIPTION("Signal and wait with specific synchronization scopes");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 3, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitBeforeSignal) {
    TEST_DESCRIPTION("Wait on the first queue then signal from a different queue");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitBeforeSignalSync1) {
    TEST_DESCRIPTION("Wait on the first queue then signal from a different queue");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit(m_second_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitSmallerValueBeforeSignal) {
    TEST_DESCRIPTION("Wait for a value on the first queue then signal a larger value from a different queue");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 2));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitBeforeSignalNonDefaultStage) {
    TEST_DESCRIPTION("Signal and wait with specific synchronization scopes");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 3, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitBeforeSignalNonDefaultStage2) {
    TEST_DESCRIPTION("Signal and wait with specific synchronization scopes");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 3, VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitLatestSignal) {
    TEST_DESCRIPTION("Check that resolving signal is determined correctly");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 1, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT));
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));  // includes all stages

    // If due to regression signal=1 resolves this wait then it should generate a WAW hazard due to stage mask mismatch
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 2));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitLatestSignalTwoQueues) {
    TEST_DESCRIPTION("Check that resolving signal is determined correctly");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 1, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT));
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));  // includes all stages

    // If due to regression signal=1 resolves this wait then it should generate a WAW hazard due to stage mask mismatch
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 2));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, QueuesCollaborateToResolveEachOthersWait) {
    TEST_DESCRIPTION("Two queues resolve wait-before-signal for one another");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Semaphore first_queue_wait(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore second_queue_wait(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(second_queue_wait, 1));
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(first_queue_wait, 1));
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(second_queue_wait, 1));
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(first_queue_wait, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, SignalResolvesTwoWaits) {
    TEST_DESCRIPTION("Signal resolves two wait-before-signal waits");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_third_queue) {
        GTEST_SKIP() << "Three queues are needed";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_third_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, SignalResolvesTwoWaits2) {
    TEST_DESCRIPTION("Signal resolves two waits");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    semaphore.Signal(1);
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, SyncSubmitsWithSingleSemaphore) {
    // TODO: existing implementation releases QBC objects and removes signals properly but vvl::Semaphore state
    // objects accumulate timepoints. Investigate if it's possible to safely release timepoint in vvl::Semaphore
    // or if should be special customization of vvl::Semaphore for syncval purposes, or maybe some other option.
    TEST_DESCRIPTION("This test helps to check that implementation releases SignalInfo and QueueBatchContexts objects");

    RETURN_IF_SKIP(InitTimelineSemaphore());

    // This number can be increased to monitor memory usage.
    // NOTE: currently memory usage increases due to vvl::Semaphore behavior mentioned above.
    const int N = 100;

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    for (int i = 1; i <= N; i++) {
        m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, i - 1), vkt::TimelineSignal(semaphore, i));
    }
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, FrameSynchronization) {
    TEST_DESCRIPTION("The host waits for frame completion by waiting on timeline semaphore");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    const int N = 100;

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    // Due to synchronization the copies from different frames do not collide
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    for (int i = 1; i <= N; i++) {
        m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, i));
        semaphore.Wait(i, vvl::kU64Max);
    }
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, FrameSynchronization2) {
    TEST_DESCRIPTION("The host waits for frame completion by polling semaphore counter value");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (requires counter from vkGetSemaphoreCounterValue)";
    }
    const int N = 100;

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    // Due to synchronization the copies from different frames do not collide
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    for (int i = 1; i <= N; i++) {
        m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, i));
        while (semaphore.GetCounterValue() != i)
            ;
    }
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, HostSignalAndWait) {
    TEST_DESCRIPTION("Signal on the host and then wait on the host");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    semaphore.Signal(1);
    semaphore.Wait(1, kWaitTimeout);
}

TEST_F(PositiveSyncValTimelineSemaphore, HostSignal) {
    TEST_DESCRIPTION("Host signal finishes device wait");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    semaphore.Signal(1);
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, HostWaitWait) {
    TEST_DESCRIPTION("Wait for semaphore one more time on the host");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    semaphore.Wait(1, kWaitTimeout);
    // Test that the second wait does not cause any issues.
    semaphore.Wait(1, kWaitTimeout);
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, HostSignalSignal) {
    TEST_DESCRIPTION("Signal semaphore two times in a row from the host");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    semaphore.Signal(1);
    // Test that the second signal does not cause any issues.
    semaphore.Signal(2);

    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, HostWaitEmptyBatch) {
    TEST_DESCRIPTION("Check that host wait for an empty batch correclty synchronizes with the previous batches");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_a, buffer_b);
    m_second_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    VkCommandBufferSubmitInfo cbuf_info = vku::InitStructHelper();
    cbuf_info.commandBuffer = m_command_buffer;

    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = semaphore;
    semaphore_info.value = 1;
    semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submits[2];
    submits[0] = vku::InitStructHelper();
    submits[0].commandBufferInfoCount = 1;
    submits[0].pCommandBufferInfos = &cbuf_info;

    // The second batch does not have command buffers and just signals the semaphore.
    // The test checks that waiting on the semaphore from such an empty batch works as expected.
    submits[1] = vku::InitStructHelper();
    submits[1].signalSemaphoreInfoCount = 1;
    submits[1].pSignalSemaphoreInfos = &semaphore_info;
    vk::QueueSubmit2(*m_default_queue, 2, submits, VK_NULL_HANDLE);

    semaphore.Wait(1, kWaitTimeout);

    // The wait should synchronize writes to buffer_b, so it's safe to copy again
    m_second_queue->Submit2(m_second_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, KhronosTimelineSemaphoreExample) {
    TEST_DESCRIPTION("https://www.khronos.org/blog/vulkan-timeline-semaphores");
    RETURN_IF_SKIP(InitTimelineSemaphore());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD (synchronization dependencies)";
    }
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    if (!buffer_b.Memory().initialized()) {
        GTEST_SKIP() << "Can't allocate host visible/coherent memory";
    }
    uint8_t* bytes = static_cast<uint8_t*>(buffer_b.Memory().Map());

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();
    m_second_command_buffer.Begin();
    m_second_command_buffer.Copy(buffer_b, buffer_a);
    m_second_command_buffer.End();

    vkt::Semaphore timeline(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    auto thread1 = [this, &timeline]() {
        const uint64_t wait_value_1 = 0;    // No-op wait. Value is always >= 0.
        const uint64_t signal_value_1 = 5;  // Unblock thread2's CPU work.
        m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(timeline, wait_value_1),
                                 vkt::TimelineSignal(timeline, signal_value_1));
    };
    auto thread2 = [&timeline, &bytes]() {
        // Wait for thread1's device work to complete.
        const uint64_t wait_value_2 = 4;
        timeline.Wait(wait_value_2, kWaitTimeout);
        // Increment everything
        for (int i = 0; i < 256; i++, bytes++) {
            ++(*bytes);
        }
        // Unblock thread3's device work.
        timeline.Signal(7);
    };
    auto thread3 = [this, &timeline]() {
        const uint64_t wait_value_3 = 7;    // Wait for thread2's CPU work to complete.
        const uint64_t signal_value_3 = 8;  // Signal completion of all work.
        m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(timeline, wait_value_3),
                                vkt::TimelineSignal(timeline, signal_value_3));
    };

    std::thread t1(thread1);
    std::thread t2(thread2);
    std::thread t3(thread3);

    timeline.Wait(8, kWaitTimeout);
    t3.join();
    t2.join();
    t1.join();
}

TEST_F(PositiveSyncValTimelineSemaphore, ResolveManyWaitBeforeSignals) {
    TEST_DESCRIPTION("Check for regression when resolved wait-before-signal did not cleanup older signals");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    // This test is mostly for manual observation of memory usage.
    // Increase constant value to get longer running time.
    uint32_t N = 1000;
    for (uint32_t i = 0; i < N; i++) {
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, i + 1));
        m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, i + 1));
        m_device->Wait();
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
TEST_F(PositiveSyncValTimelineSemaphore, ExternalSemaphoreWaitBeforeSignal) {
    TEST_DESCRIPTION("Wait-before-signal with external semaphore should not hoard resources");
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    constexpr auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if (!SemaphoreExportImportSupported(Gpu(), handle_type)) {
        GTEST_SKIP() << "Semaphore does not support export and import through Win32 handle";
    }

    VkSemaphoreTypeCreateInfo semaphore_type_ci = vku::InitStructHelper();
    semaphore_type_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_ci.initialValue = 0;
    VkExportSemaphoreCreateInfo export_info = vku::InitStructHelper(&semaphore_type_ci);
    export_info.handleTypes = handle_type;
    const VkSemaphoreCreateInfo export_semaphore_ci = vku::InitStructHelper(&export_info);
    vkt::Semaphore export_semaphore(*m_device, export_semaphore_ci);

    vkt::Semaphore import_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    HANDLE win32_handle = NULL;
    export_semaphore.ExportHandle(win32_handle, handle_type);
    import_semaphore.ImportHandle(win32_handle, handle_type);

    // This test is for manual inspection. Without special handling the wait-before-signal with an
    // external semaphore accumulates unresolved batches and registered signals (it's not possible
    // to track external signal). Check that the list of unresolved batches and signals does not grow.
    const int N = 100;
    for (int i = 1; i <= N; i++) {
        m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(export_semaphore, i));
        m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(import_semaphore, i));
        m_device->Wait();
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST_F(PositiveSyncValTimelineSemaphore, QueueWaitIdleRemovesSignals) {
    TEST_DESCRIPTION("Test for manual inspection of registered signals (VK_SYNCVAL_SHOW_STATS can be used)");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    const uint32_t N = 100;
    for (uint32_t i = 1; i <= N; i++) {
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, i));
        // The maximum number of registered signals will be around 25
        if (i % 25 == 0) {
            m_default_queue->Wait();
        }
    }
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, DeviceWaitIdleRemovesignals) {
    TEST_DESCRIPTION("Test for manual inspection of registered signals (VK_SYNCVAL_SHOW_STATS can be used)");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    const uint32_t N = 100;
    for (uint32_t i = 1; i <= N; i++) {
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, i));
        // The maximum number of registered signals will be around 50
        if (i % 50 == 0) {
            m_device->Wait();
        }
    }
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, ManyOrphanedSignals) {
    TEST_DESCRIPTION("Test limit of maximum number of registered signals per timeline");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    VkCommandBufferSubmitInfo cbuf_info = vku::InitStructHelper();
    cbuf_info.commandBuffer = m_command_buffer;

    VkSemaphoreSubmitInfo wait = vku::InitStructHelper();
    wait.semaphore = binary_semaphore;
    wait.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSemaphoreSubmitInfo signals[2];
    // binary signal
    signals[0] = vku::InitStructHelper();
    signals[0].semaphore = binary_semaphore;
    signals[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    // timeline signal (value is set for each loop iteration)
    signals[1] = vku::InitStructHelper();
    signals[1].semaphore = semaphore;
    signals[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    const uint32_t N = 1'000;
    for (uint32_t i = 1; i <= N; i++) {
        // signal timeline on each iteration but never wait for it
        signals[1].value = i;

        VkSubmitInfo2 submit = vku::InitStructHelper();
        if (i > 1) {
            // wait binary signal from previous iteration
            submit.waitSemaphoreInfoCount = 1;
            submit.pWaitSemaphoreInfos = &wait;
        }
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &cbuf_info;
        submit.signalSemaphoreInfoCount = 2;
        submit.pSignalSemaphoreInfos = signals;

        vk::QueueSubmit2(m_default_queue->handle(), 1, &submit, VK_NULL_HANDLE);
    }
    m_device->Wait();
}

TEST_F(PositiveSyncValTimelineSemaphore, WaitForFencesWithTimelineSignalBatches) {
    TEST_DESCRIPTION("Check that WaitForFences applies tagged waits to timeline signal batches");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Fence fence(*m_device);

    // The first batch context.
    // Specify VERTEX_SHADER signal scope, so waiting for timeline signal does not protect buffer copy
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 1, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT), fence);

    // The second batch context which imports the first one.
    // The timeline signal in the first submit still references the first batch.
    m_default_queue->Submit2(vkt::no_cmd);

    // The original omission was that iteration over all batch context did not take into account
    // batches associated with timeline signals. In the case of regression accesses in the first
    // batch (stored in timeline signal) will survive the fence wait.
    fence.Wait(kWaitTimeout);

    // Waiting for timeline signal imports the first batch stored in that signal.
    // In case of regression the first batch will contain unprotected copy writes and
    // this will cause WRITE-AFTER-WRITE hazard.
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));

    m_default_queue->Wait();
}
