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

#include "../framework/layer_validation_tests.h"

struct NegativeSyncValTimelineSemaphore : public VkSyncValTest {};

TEST_F(NegativeSyncValTimelineSemaphore, WaitInitialValue) {
    TEST_DESCRIPTION("Wait on the initial value");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 1));

    // Wait on the initial value 0 does not synchronize with signal 1
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 0));
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, WaitInitialValueTwoQueues) {
    TEST_DESCRIPTION("Wait on the initial value");
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

    // Wait on the initial value 0 does not synchronize with signal 1
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 0));
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, WaitAfterSignalStageMismatch) {
    TEST_DESCRIPTION("Hazard due to stage mask mismatch");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    // Signal first scope includes only CLEAR commands but not COPY commands
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineSignal(semaphore, 2, VK_PIPELINE_STAGE_2_CLEAR_BIT));

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_2_COPY_BIT));
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, WaitBeforeSignal) {
    TEST_DESCRIPTION("Signal from the second queue resolves wait on the main queue");
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

    // Resolve waint on the main queue
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));

    // Writes on the second queue collide with writes on the main queue
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    m_second_queue->Submit2(m_second_command_buffer);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, WaitBeforeSignalBatchFollowedByOneMoreBatch) {
    TEST_DESCRIPTION("Hazard between wait-before-signal batch and the next batch on the same queue");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit2(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, WaitBeforeSignalEmptyWaitScope) {
    TEST_DESCRIPTION("Hazard due to stage mask mismatch");
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

    // Buffer copy starts immediately because VERTEX_SHADER stage does not include copy operation
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT));

    // TODO: this should be detected as WRITE-RACING-WRITE error
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 1));
    m_errorMonitor->VerifyFound();

    // Unblock the first queue (the previous call due to error did not reach Record phase)
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, WaitBeforeSignalAfterNoDepsBatch) {
    TEST_DESCRIPTION("Batch without dependencies is followed by wait-before-signal batch");
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
    // the first batch is a batch with a copy job
    submits[0] = vku::InitStructHelper();
    submits[0].commandBufferInfoCount = 1;
    submits[0].pCommandBufferInfos = &cbuf_info;
    // the second batch is a wait-before-signal batch
    submits[1] = vku::InitStructHelper();
    submits[1].waitSemaphoreInfoCount = 1;
    submits[1].pWaitSemaphoreInfos = &semaphore_info;
    vk::QueueSubmit2(*m_default_queue, 2, submits, VK_NULL_HANDLE);

    // Submit signal to resolve the wait
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));

    // Submit one more copy to collide with a copy on the default queue
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, HostSignal) {
    TEST_DESCRIPTION("Host signal breaks synchronization between two submits");
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

    // Finish the previous wait, so both submits can run in parallel
    semaphore.Signal(1);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineSignal(semaphore, 2));
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, SignalResolvesTwoWaits) {
    TEST_DESCRIPTION("One signal resolves two wait-before-signal waits");
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
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    semaphore.Signal(1);
    m_errorMonitor->VerifyFound();

    // Unblock queue threads (the previous call did not reach Record phase)
    semaphore.Signal(1);
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, SignalResolvesTwoWaits2) {
    TEST_DESCRIPTION("One signal resolves two wait-before-signal waits");
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
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 2));
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    semaphore.Signal(2);
    m_errorMonitor->VerifyFound();

    // Unblock queue threads (the previous call did not reach Record phase)
    semaphore.Signal(2);
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, SignalResolvesTwoWaits3) {
    TEST_DESCRIPTION("One signal resolves two wait-before-signal waits");
    RETURN_IF_SKIP(InitTimelineSemaphore());

    if (!m_third_queue) {
        GTEST_SKIP() << "Three queues are needed";
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
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    m_third_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    m_errorMonitor->VerifyFound();

    // Unblock queue threads (the previous call did not reach Record phase)
    m_third_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncValTimelineSemaphore, SignalResolvesTwoWaits4) {
    TEST_DESCRIPTION("One signal resolves two waits");
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
    semaphore.Signal(1);
    m_default_queue->Submit2(m_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-RACING-WRITE");
    m_second_queue->Submit2(m_second_command_buffer, vkt::TimelineWait(semaphore, 1));
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}
