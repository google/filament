/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
#include "../framework/ray_tracing_objects.h"

struct NegativeSyncValRayTracing : public VkSyncValTest {};

TEST_F(NegativeSyncValRayTracing, ScratchBufferHazard) {
    TEST_DESCRIPTION("Write to scratch buffer during acceleration structure build");
    RETURN_IF_SKIP(InitRayTracing());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD: buffer device address consistent support across different functions";
    }

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetDeviceScratchAdditionalFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    const vkt::Buffer& scratch_buffer = *blas.GetScratchBuffer();

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer);

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    vk::CmdFillBuffer(m_command_buffer, scratch_buffer, 0, sizeof(uint32_t), 0x42);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
