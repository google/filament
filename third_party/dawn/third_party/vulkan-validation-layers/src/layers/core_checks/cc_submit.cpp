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

#include "cc_submit.h"
#include "state_tracker/queue_state.h"
#include "sync/sync_vuid_maps.h"
#include "chassis/validation_object.h"

static Location GetSignaledSemaphoreLocation(const Location& submit_loc, uint32_t index) {
    vvl::Field field = vvl::Field::Empty;
    if (submit_loc.function == vvl::Func::vkQueueSubmit || submit_loc.function == vvl::Func::vkQueueBindSparse) {
        field = vvl::Field::pSignalSemaphores;
    } else if (submit_loc.function == vvl::Func::vkQueueSubmit2 || submit_loc.function == vvl::Func::vkQueueSubmit2KHR) {
        field = vvl::Field::pSignalSemaphoreInfos;
    } else {
        assert(false && "Unhandled signaling function");
    }
    return submit_loc.dot(field, index);
}

void QueueSubmissionValidator::Validate(const vvl::QueueSubmission& submission) const {
    for (uint32_t i = 0; i < (uint32_t)submission.signal_semaphores.size(); ++i) {
        const auto& signal = submission.signal_semaphores[i];
        const uint64_t current_payload = signal.semaphore->CurrentPayload();
        if (signal.payload < current_payload) {
            const Location signal_semaphore_loc = GetSignaledSemaphoreLocation(submission.loc.Get(), i);
            const auto& vuid = GetQueueSubmitVUID(signal_semaphore_loc, sync_vuid_maps::SubmitError::kTimelineSemSmallValue);
            error_logger.LogError(vuid, signal.semaphore->Handle(), signal_semaphore_loc,
                                  "(%s) signaled with value %" PRIu64 " which is smaller than the current value %" PRIu64,
                                  error_logger.FormatHandle(signal.semaphore->VkHandle()).c_str(), signal.payload, current_payload);
        }
    }
}
