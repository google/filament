/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <string>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include "cc_synchronization.h"
#include "core_validation.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/semaphore_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "sync/sync_vuid_maps.h"

// Holds common information between all command buffers being submitted
struct CommandBufferSubmitState {
    const CoreChecks &core;
    const vvl::Queue &queue_state;
    QFOTransferCBScoreboards<QFOImageTransferBarrier> qfo_image_scoreboards;
    QFOTransferCBScoreboards<QFOBufferTransferBarrier> qfo_buffer_scoreboards;
    std::vector<VkCommandBuffer> current_cmds;
    GlobalImageLayoutMap global_image_layout_map;
    std::vector<std::string> cmdbuf_label_stack;
    std::string last_closed_cmdbuf_label;
    bool found_unbalanced_cmdbuf_label;

    // The "local" prefix is about tracking state within a *single* queue submission
    // (accross all command buffers of that submission), as opposed to globally
    // tracking state accross *all* submissions to the same queue.
    QueryMap local_query_to_state_map;
    EventMap local_event_signal_info;
    vvl::unordered_map<VkVideoSessionKHR, vvl::VideoSessionDeviceState> local_video_session_state{};

    CommandBufferSubmitState(const CoreChecks &c, const vvl::Queue &q) : core(c), queue_state(q) {
        // Queue label state is updated during PostRecord phase.
        // Copy state to be able to track labels during validation.
        cmdbuf_label_stack = queue_state.cmdbuf_label_stack;
        last_closed_cmdbuf_label = queue_state.last_closed_cmdbuf_label;
        found_unbalanced_cmdbuf_label = queue_state.found_unbalanced_cmdbuf_label;
    }

    bool Validate(const Location &loc, const vvl::CommandBuffer &cb_state, uint32_t perf_pass) {
        bool skip = false;
        skip |= core.ValidateCmdBufImageLayouts(loc, cb_state, global_image_layout_map);
        const VkCommandBuffer cmd = cb_state.VkHandle();
        current_cmds.push_back(cmd);
        skip |= core.ValidatePrimaryCommandBufferState(
            loc, cb_state, static_cast<uint32_t>(std::count(current_cmds.begin(), current_cmds.end(), cmd)), &qfo_image_scoreboards,
            &qfo_buffer_scoreboards);
        skip |= core.ValidateQueueFamilyIndices(loc, cb_state, queue_state);
        skip |= ValidateCmdBufLabelMatching(loc, cb_state);

        // Potential early exit here as bad object state may crash in delayed function calls
        if (skip) {
            return true;
        }

        // Call submit-time functions to validate or update local mirrors of state (to preserve const-ness at validate time)
        for (auto &function : cb_state.queue_submit_functions) {
            skip |= function(core, queue_state, cb_state);
        }
        for (auto &function : cb_state.event_updates) {
            skip |= function(const_cast<vvl::CommandBuffer &>(cb_state), /*do_validate*/ true, local_event_signal_info,
                             queue_state.VkHandle(), loc);
        }
        VkQueryPool first_perf_query_pool = VK_NULL_HANDLE;
        for (auto &function : cb_state.query_updates) {
            skip |= function(const_cast<vvl::CommandBuffer &>(cb_state), /*do_validate*/ true, first_perf_query_pool, perf_pass,
                             &local_query_to_state_map);
        }

        for (const auto &it : cb_state.video_session_updates) {
            auto video_session_state = core.Get<vvl::VideoSession>(it.first);
            auto local_state_it = local_video_session_state.find(it.first);
            if (video_session_state && (local_state_it == local_video_session_state.end())) {
                local_state_it = local_video_session_state.insert({it.first, video_session_state->DeviceStateCopy()}).first;
            }
            for (const auto &function : it.second) {
                skip |= function(core, video_session_state.get(), local_state_it->second, /*do_validate*/ true);
            }
        }
        return skip;
    }

private:
    bool ValidateCmdBufLabelMatching(const Location &loc, const vvl::CommandBuffer &cb_state) {
        bool skip = false;
        if (found_unbalanced_cmdbuf_label) {
            // We already reported an error. After the first error, do not perform further validation or state tracking.
            // The assumption that after label mismatch the label stack is corrupted and can't be reasoned about.
            return skip;
        }
        for (const auto &command : cb_state.GetLabelCommands()) {
            if (command.begin) {
                cmdbuf_label_stack.emplace_back(command.label_name);
            } else {
                if (cmdbuf_label_stack.empty()) {
                    found_unbalanced_cmdbuf_label = true;
                    break;
                }
                last_closed_cmdbuf_label = cmdbuf_label_stack.back();
                cmdbuf_label_stack.pop_back();
            }
        }
        if (found_unbalanced_cmdbuf_label) {
            std::string previous_debug_region;
            if (last_closed_cmdbuf_label.empty()) {
                previous_debug_region = "There are no previous debug regions before the invalid command.";
            } else {
                previous_debug_region =
                    std::string("The previous debug region before the invalid command is '") + last_closed_cmdbuf_label + "'.";
            }
            skip |= core.LogError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912", cb_state.Handle(), loc,
                                  "(%s) contains vkCmdEndDebugUtilsLabelEXT that does not have a matching "
                                  "vkCmdBeginDebugUtilsLabelEXT in this or one of the previously submitted command buffers. %s",
                                  core.FormatHandle(cb_state).c_str(), previous_debug_region.c_str());
        }
        return skip;
    }
};

bool CoreChecks::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto fence_state = Get<vvl::Fence>(fence)) {
        const LogObjectList objlist(queue, fence);
        skip |= ValidateFenceForSubmit(*fence_state, "VUID-vkQueueSubmit-fence-00064", "VUID-vkQueueSubmit-fence-00063", objlist,
                                       error_obj.location);
    }
    if (skip) return skip;

    auto queue_state = Get<vvl::Queue>(queue);
    ASSERT_AND_RETURN_SKIP(queue_state);
    CommandBufferSubmitState cb_submit_state(*this, *queue_state);
    SemaphoreSubmitState sem_submit_state(*this, queue, queue_state->queue_family_properties.queueFlags);

    // Now verify each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo &submit = pSubmits[submit_idx];
        const Location submit_loc = error_obj.location.dot(Struct::VkSubmitInfo, Field::pSubmits, submit_idx);
        const auto perf_submit = vku::FindStructInPNextChain<VkPerformanceQuerySubmitInfoKHR>(submit.pNext);
        uint32_t perf_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        bool protected_submit = false;
        auto protected_submit_info = vku::FindStructInPNextChain<VkProtectedSubmitInfo>(submit.pNext);
        if (protected_submit_info) {
            protected_submit = protected_submit_info->protectedSubmit == VK_TRUE;
            if ((protected_submit == true) && ((queue_state->create_flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) == 0)) {
                skip |= LogError("VUID-vkQueueSubmit-queue-06448", queue, submit_loc,
                                 "contains a protected submission to %s which was not created with "
                                 "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT",
                                 FormatHandle(queue).c_str());
            }
        }

        bool suspended_render_pass_instance = false;
        for (uint32_t i = 0; i < submit.commandBufferCount; i++) {
            const Location cb_loc = submit_loc.dot(Field::pCommandBuffers, i);
            auto cb_state = GetRead<vvl::CommandBuffer>(submit.pCommandBuffers[i]);
            ASSERT_AND_CONTINUE(cb_state);

            skip |= cb_submit_state.Validate(cb_loc, *cb_state, perf_pass);

            // Make sure command buffers are all protected or unprotected
            if ((cb_state->unprotected == true) && (protected_submit == true)) {
                const LogObjectList objlist(cb_state->Handle(), queue);
                skip |= LogError("VUID-VkSubmitInfo-pNext-04148", objlist, cb_loc,
                                 "(%s) is unprotected while queue %s pSubmits[%u] has "
                                 "VkProtectedSubmitInfo:protectedSubmit set to VK_TRUE",
                                 FormatHandle(cb_state->Handle()).c_str(), FormatHandle(queue).c_str(), submit_idx);
            }
            if ((cb_state->unprotected == false) && (protected_submit == false)) {
                const LogObjectList objlist(cb_state->Handle(), queue);
                skip |= LogError("VUID-VkSubmitInfo-pNext-04120", objlist, cb_loc,
                                 "(%s) is protected while queue %s pSubmits[%u] has %s", FormatHandle(cb_state->Handle()).c_str(),
                                 FormatHandle(queue).c_str(), submit_idx,
                                 protected_submit_info ? "VkProtectedSubmitInfo:protectedSubmit set to VK_FALSE"
                                                       : "no VkProtectedSubmitInfo in the pNext chain");
            }

            // Validate flags for dynamic rendering
            if (suspended_render_pass_instance && cb_state->hasRenderPassInstance && !cb_state->resumesRenderPassInstance) {
                skip |= LogError("VUID-VkSubmitInfo-pCommandBuffers-06016", queue, submit_loc,
                                 "has a suspended render pass instance, but pCommandBuffers[%" PRIu32
                                 "] has its own render pass instance that does not resume it.",
                                 i);
            }
            if (cb_state->resumesRenderPassInstance) {
                if (!suspended_render_pass_instance) {
                    const LogObjectList objlist(cb_state->Handle(), queue);
                    skip |= LogError("VUID-VkSubmitInfo-pCommandBuffers-06193", objlist, cb_loc,
                                     "resumes a render pass instance, but there is no suspended render pass instance.");
                }
                suspended_render_pass_instance = false;
            }
            if (cb_state->suspendsRenderPassInstance) {
                suspended_render_pass_instance = true;
            }
        }
        // Renderpass should not be in suspended state after the final cmdbuf
        if (suspended_render_pass_instance) {
            skip |= LogError("VUID-VkSubmitInfo-pCommandBuffers-06014", queue, submit_loc,
                             "has a suspended render pass instance that was not resumed.");
        }
        skip |= ValidateSemaphoresForSubmit(sem_submit_state, submit, submit_loc);

        auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupSubmitInfo>(submit.pNext);
        if (chained_device_group_struct && chained_device_group_struct->commandBufferCount > 0) {
            for (uint32_t i = 0; i < chained_device_group_struct->commandBufferCount; ++i) {
                const LogObjectList objlist(queue);
                skip |= ValidateDeviceMaskToPhysicalDeviceCount(
                    chained_device_group_struct->pCommandBufferDeviceMasks[i], objlist,
                    submit_loc.pNext(Struct::VkDeviceGroupSubmitInfo, Field::pCommandBufferDeviceMasks, i),
                    "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
            }
            if (chained_device_group_struct->signalSemaphoreCount != submit.signalSemaphoreCount) {
                skip |= LogError("VUID-VkDeviceGroupSubmitInfo-signalSemaphoreCount-00084", queue,
                                 submit_loc.pNext(Struct::VkDeviceGroupSubmitInfo, Field::signalSemaphoreCount),
                                 "(%" PRIu32 ") is different than signalSemaphoreCount (%" PRIu32 ").",
                                 chained_device_group_struct->signalSemaphoreCount, submit.signalSemaphoreCount);
            }
            if (chained_device_group_struct->waitSemaphoreCount != submit.waitSemaphoreCount) {
                skip |= LogError("VUID-VkDeviceGroupSubmitInfo-waitSemaphoreCount-00082", queue,
                                 submit_loc.pNext(Struct::VkDeviceGroupSubmitInfo, Field::waitSemaphoreCount),
                                 "(%" PRIu32 ") is different than waitSemaphoreCount (%" PRIu32 ").",
                                 chained_device_group_struct->waitSemaphoreCount, submit.waitSemaphoreCount);
            }
            if (chained_device_group_struct->commandBufferCount != submit.commandBufferCount) {
                skip |= LogError("VUID-VkDeviceGroupSubmitInfo-commandBufferCount-00083", queue,
                                 submit_loc.pNext(Struct::VkDeviceGroupSubmitInfo, Field::commandBufferCount),
                                 "(%" PRIu32 ") is different than commandBufferCount (%" PRIu32 ").",
                                 chained_device_group_struct->commandBufferCount, submit.commandBufferCount);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateRenderPassStripeSubmitInfo(VkQueue queue, const vvl::CommandBuffer &cb_state, const void *pNext,
                                                    const Location &loc) const {
    bool skip = false;
    LogObjectList objlist(queue, cb_state.Handle());

    const VkRenderPassStripeSubmitInfoARM *rp_submit_info = vku::FindStructInPNextChain<VkRenderPassStripeSubmitInfoARM>(pNext);
    if (!rp_submit_info) {
        if (cb_state.has_render_pass_striped && !cb_state.resumesRenderPassInstance) {
            skip |= LogError("VUID-VkCommandBufferSubmitInfo-commandBuffer-09445", objlist, loc.dot(Field::pNext),
                             "missing VkRenderPassStripeSubmitInfoARM struct because command buffer contain begin info "
                             "with renderpass striped struct");
        }
        return skip;
    }

    if (rp_submit_info->stripeSemaphoreInfoCount != cb_state.striped_count && !cb_state.resumesRenderPassInstance) {
        skip |= LogError("VUID-VkCommandBufferSubmitInfo-pNext-09446", objlist,
                         loc.pNext(Struct::VkRenderPassStripeSubmitInfoARM, Field::stripeSemaphoreInfoCount),
                         "(%" PRIu32 ") must be equal to VkRenderPassStripeBeginInfoARM::stripeInfoCount (%" PRIu32 ").",
                         rp_submit_info->stripeSemaphoreInfoCount, cb_state.striped_count);
    }

    for (uint32_t count = 0; count < rp_submit_info->stripeSemaphoreInfoCount; ++count) {
        auto semaphore = rp_submit_info->pStripeSemaphoreInfos[count].semaphore;
        auto semaphore_state = Get<vvl::Semaphore>(semaphore);
        ASSERT_AND_CONTINUE(semaphore_state);
        if (semaphore_state->type != VK_SEMAPHORE_TYPE_BINARY) {
            objlist.add(semaphore);
            skip |= LogError("VUID-VkRenderPassStripeSubmitInfoARM-semaphore-09447", objlist,
                             loc.pNext(Struct::VkRenderPassStripeSubmitInfoARM, Field::pStripeSemaphoreInfos, count),
                             "is not a VK_SEMAPHORE_TYPE_BINARY.");
            break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                      const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto fence_state = Get<vvl::Fence>(fence)) {
        const LogObjectList objlist(queue, fence);
        skip |= ValidateFenceForSubmit(*fence_state, "VUID-vkQueueSubmit2-fence-04895", "VUID-vkQueueSubmit2-fence-04894", objlist,
                                       error_obj.location);
    }
    if (skip) {
        return skip;
    }

    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkQueueSubmit2-synchronization2-03866", queue, error_obj.location,
                         "synchronization2 feature is not enabled");
    }

    auto queue_state = Get<vvl::Queue>(queue);
    ASSERT_AND_RETURN_SKIP(queue_state);
    CommandBufferSubmitState cb_submit_state(*this, *queue_state);
    SemaphoreSubmitState sem_submit_state(*this, queue, queue_state->queue_family_properties.queueFlags);

    // Now verify each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const Location submit_loc = error_obj.location.dot(Struct::VkSubmitInfo2, Field::pSubmits, submit_idx);
        const VkSubmitInfo2 &submit = pSubmits[submit_idx];
        const auto perf_submit = vku::FindStructInPNextChain<VkPerformanceQuerySubmitInfoKHR>(submit.pNext);
        const uint32_t perf_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        skip |= ValidateSemaphoresForSubmit(sem_submit_state, submit, submit_loc);

        const bool protected_submit = (submit.flags & VK_SUBMIT_PROTECTED_BIT) != 0;
        if ((protected_submit == true) && ((queue_state->create_flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)) == 0) {
            skip |= LogError("VUID-vkQueueSubmit2-queue-06447", queue, submit_loc,
                             "contains a protected submission to %s which was not created with "
                             "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT",
                             FormatHandle(queue).c_str());
        }

        bool suspended_render_pass_instance = false;
        for (uint32_t i = 0; i < submit.commandBufferInfoCount; i++) {
            const Location info_loc = submit_loc.dot(Struct::VkCommandBufferSubmitInfo, Field::pCommandBufferInfos, i);
            auto cb_state = GetRead<vvl::CommandBuffer>(submit.pCommandBufferInfos[i].commandBuffer);
            const Location cb_loc = info_loc.dot(Field::commandBuffer);
            ASSERT_AND_CONTINUE(cb_state);

            skip |= cb_submit_state.Validate(cb_loc, *cb_state, perf_pass);

            {
                const LogObjectList objlist(queue);
                skip |= ValidateDeviceMaskToPhysicalDeviceCount(submit.pCommandBufferInfos[i].deviceMask, queue,
                                                                info_loc.dot(Field::deviceMask),
                                                                "VUID-VkCommandBufferSubmitInfo-deviceMask-03891");
            }

            // Make sure command buffers are all protected or unprotected
            if ((cb_state->unprotected == true) && (protected_submit == true)) {
                const LogObjectList objlist(cb_state->Handle(), queue);
                skip |= LogError("VUID-VkSubmitInfo2-flags-03886", objlist, cb_loc,
                                 "is unprotected while %s.flags (%s) has VK_SUBMIT_PROTECTED_BIT set", submit_loc.Fields().c_str(),
                                 string_VkSubmitFlags(submit.flags).c_str());
            }
            if ((cb_state->unprotected == false) && (protected_submit == false)) {
                const LogObjectList objlist(cb_state->Handle(), queue);
                skip |= LogError("VUID-VkSubmitInfo2-flags-03887", objlist, cb_loc,
                                 "is protected while %s.flags (%s) has VK_SUBMIT_PROTECTED_BIT not set",
                                 submit_loc.Fields().c_str(), string_VkSubmitFlags(submit.flags).c_str());
            }

            if (suspended_render_pass_instance && cb_state->hasRenderPassInstance && !cb_state->resumesRenderPassInstance) {
                skip |= LogError("VUID-VkSubmitInfo2-commandBuffer-06012", queue, submit_loc,
                                 "has a suspended render pass instance, but pCommandBuffers[%" PRIu32
                                 "] has its own render pass instance that does not resume it.",
                                 i);
            }
            if (cb_state->suspendsRenderPassInstance) {
                suspended_render_pass_instance = true;
            }
            if (cb_state->resumesRenderPassInstance) {
                if (!suspended_render_pass_instance) {
                    skip |= LogError("VUID-VkSubmitInfo2-commandBuffer-06192", queue, cb_loc,
                                     "resumes a render pass instance, but there is no suspended render pass instance.");
                }
                suspended_render_pass_instance = false;
            }

            skip |= ValidateRenderPassStripeSubmitInfo(queue, *cb_state, submit.pCommandBufferInfos[i].pNext, info_loc);
        }
        if (suspended_render_pass_instance) {
            skip |= LogError("VUID-VkSubmitInfo2-commandBuffer-06010", queue, submit_loc,
                             "has a suspended render pass instance that was not resumed.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                VkFence fence, const ErrorObject &error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool CoreChecks::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                             const ErrorObject &error_obj) const {
    return ValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

void CoreChecks::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                           const RecordObject &record_obj) {
    BaseClass::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    if (record_obj.result != VK_SUCCESS) return;
    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo &submit = pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit.commandBufferCount; i++) {
            auto cb_state = GetWrite<vvl::CommandBuffer>(submit.pCommandBuffers[i]);
            ASSERT_AND_CONTINUE(cb_state);

            for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                UpdateCmdBufImageLayouts(*secondary_cmd_buffer);
                RecordQueuedQFOTransfers(*secondary_cmd_buffer);
            }
            UpdateCmdBufImageLayouts(*cb_state);
            RecordQueuedQFOTransfers(*cb_state);
        }
    }
}

void CoreChecks::RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                    const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2 &submit = pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit.commandBufferInfoCount; i++) {
            auto cb_state = GetWrite<vvl::CommandBuffer>(submit.pCommandBufferInfos[i].commandBuffer);
            ASSERT_AND_CONTINUE(cb_state);

            for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                UpdateCmdBufImageLayouts(*secondary_cmd_buffer);
                RecordQueuedQFOTransfers(*secondary_cmd_buffer);
            }
            UpdateCmdBufImageLayouts(*cb_state);
            RecordQueuedQFOTransfers(*cb_state);
        }
    }
}

void CoreChecks::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                               const RecordObject &record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void CoreChecks::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                            const RecordObject &record_obj) {
    BaseClass::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

// Check that the queue family index of 'queue' matches one of the entries in pQueueFamilyIndices
bool CoreChecks::ValidImageBufferQueue(const vvl::CommandBuffer &cb_state, const VulkanTypedHandle &object,
                                       uint32_t queueFamilyIndex, uint32_t count, const uint32_t *indices,
                                       const Location &loc) const {
    bool found = false;
    bool skip = false;
    for (uint32_t i = 0; i < count; i++) {
        if (indices[i] == queueFamilyIndex) {
            found = true;
            break;
        }
    }

    if (!found) {
        const LogObjectList objlist(cb_state.Handle(), object);
        skip |= LogError("VUID-vkQueueSubmit-pSubmits-04626", objlist, loc,
                         "%s contains %s which was not created allowing concurrent access to "
                         "this queue family %d.",
                         FormatHandle(cb_state).c_str(), FormatHandle(object).c_str(), queueFamilyIndex);
    }
    return skip;
}

// Validate that queueFamilyIndices of primary command buffers match this queue
// Secondary command buffers were previously validated in vkCmdExecuteCommands().
bool CoreChecks::ValidateQueueFamilyIndices(const Location &loc, const vvl::CommandBuffer &cb_state,
                                            const vvl::Queue &queue_state) const {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;
    auto pool = cb_state.command_pool;
    ASSERT_AND_RETURN_SKIP(pool);

    if (pool->queueFamilyIndex != queue_state.queue_family_index) {
        const LogObjectList objlist(cb_state.Handle(), queue_state.Handle());
        const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kCmdWrongQueueFamily);
        skip |= LogError(vuid, objlist, loc,
                         "Primary command buffer %s created in queue family %d is being submitted on %s "
                         "from queue family %d.",
                         FormatHandle(cb_state).c_str(), pool->queueFamilyIndex, FormatHandle(queue_state.Handle()).c_str(),
                         queue_state.queue_family_index);
    }

    // Ensure that any bound images or buffers created with SHARING_MODE_CONCURRENT have access to the current queue family
    for (const auto &state_object : cb_state.object_bindings) {
        switch (state_object->Type()) {
            case kVulkanObjectTypeImage: {
                auto image_state = static_cast<const vvl::Image *>(state_object.get());
                if (image_state && image_state->create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                    skip |= ValidImageBufferQueue(cb_state, image_state->Handle(), queue_state.queue_family_index,
                                                  image_state->create_info.queueFamilyIndexCount,
                                                  image_state->create_info.pQueueFamilyIndices, loc);
                }
                break;
            }
            case kVulkanObjectTypeBuffer: {
                auto buffer_state = static_cast<const vvl::Buffer *>(state_object.get());
                if (buffer_state && buffer_state->create_info.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                    skip |= ValidImageBufferQueue(cb_state, buffer_state->Handle(), queue_state.queue_family_index,
                                                  buffer_state->create_info.queueFamilyIndexCount,
                                                  buffer_state->create_info.pQueueFamilyIndices, loc);
                }
                break;
            }
            default:
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateCommandBufferState(const vvl::CommandBuffer &cb_state, const Location &loc, uint32_t current_submit_count,
                                            const char *vuid) const {
    bool skip = false;
    if (disabled[command_buffer_state]) {
        return skip;
    }

    // Validate ONE_TIME_SUBMIT_BIT CB is not being submitted more than once
    if (const uint64_t submissions = cb_state.submitCount + current_submit_count;
        (cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) && (submissions > 1)) {
        // VU being worked on https://gitlab.khronos.org/vulkan/vulkan/-/issues/2456
        skip |= LogError("UNASSIGNED-DrawState-CommandBufferSingleSubmitViolation", cb_state.Handle(), loc,
                         "%s recorded with VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT has been submitted %" PRIu64 " times.",
                         FormatHandle(cb_state).c_str(), submissions);
    }

    // Validate that cmd buffers have been updated
    switch (cb_state.state) {
        case CbState::InvalidIncomplete:
        case CbState::InvalidComplete:
            skip |= ReportInvalidCommandBuffer(cb_state, loc, vuid);
            break;

        case CbState::New:
            skip |= LogError(vuid, cb_state.Handle(), loc, "%s is unrecorded and contains no commands.",
                             FormatHandle(cb_state).c_str());
            break;

        case CbState::Recording:
            skip |= LogError(vuid, cb_state.Handle(), loc, "You must call vkEndCommandBuffer() on %s before this call.",
                             FormatHandle(cb_state).c_str());
            break;

        default: /* recorded */
            break;
    }
    return skip;
}

bool CoreChecks::ValidateCommandBufferSimultaneousUse(const Location &loc, const vvl::CommandBuffer &cb_state,
                                                      int current_submit_count) const {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    bool skip = false;
    if ((cb_state.InUse() || current_submit_count > 1) &&
        !(cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, SubmitError::kCmdNotSimultaneous);

        skip |= LogError(vuid, device, loc, "%s is already in use and is not marked for simultaneous use.",
                         FormatHandle(cb_state).c_str());
    }
    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBufferState(
    const Location &loc, const vvl::CommandBuffer &cb_state, uint32_t current_submit_count,
    QFOTransferCBScoreboards<QFOImageTransferBarrier> *qfo_image_scoreboards,
    QFOTransferCBScoreboards<QFOBufferTransferBarrier> *qfo_buffer_scoreboards) const {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    // Track in-use for resources off of primary and any secondary CBs
    bool skip = false;

    if (cb_state.IsSecondary()) {
        const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kSecondaryCmdInSubmit);
        skip |= LogError(vuid, cb_state.Handle(), loc, "Command buffer %s must be allocated with VK_COMMAND_BUFFER_LEVEL_PRIMARY.",
                         FormatHandle(cb_state).c_str());
    } else {
        for (const auto *sub_cb : cb_state.linkedCommandBuffers) {
            skip |= ValidateQueuedQFOTransfers(*sub_cb, qfo_image_scoreboards, qfo_buffer_scoreboards, loc);
            // TODO: replace with InvalidateCommandBuffers() at recording.
            if ((sub_cb->primaryCommandBuffer != cb_state.VkHandle()) &&
                !(sub_cb->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kSecondaryCmdNotSimultaneous);
                const LogObjectList objlist(device, cb_state.Handle(), sub_cb->Handle(), sub_cb->primaryCommandBuffer);
                skip |= LogError(vuid, objlist, loc,
                                 "%s was submitted with secondary %s but that buffer has subsequently been bound to "
                                 "primary %s and it does not have VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 FormatHandle(cb_state).c_str(), FormatHandle(sub_cb->Handle()).c_str(),
                                 FormatHandle(sub_cb->primaryCommandBuffer).c_str());
            }

            if (sub_cb->state != CbState::Recorded) {
                const char *const finished_cb_vuid = (loc.function == Func::vkQueueSubmit)
                                                         ? "VUID-vkQueueSubmit-pCommandBuffers-00072"
                                                         : "VUID-vkQueueSubmit2-commandBuffer-03876";
                const LogObjectList objlist(device, cb_state.Handle(), sub_cb->Handle(), sub_cb->primaryCommandBuffer);
                skip |= LogError(finished_cb_vuid, objlist, loc,
                                 "Secondary command buffer %s is not in a valid (pending or executable) state.",
                                 FormatHandle(sub_cb->Handle()).c_str());
            }
        }
    }

    // If USAGE_SIMULTANEOUS_USE_BIT not set then CB cannot already be executing on device
    skip |= ValidateCommandBufferSimultaneousUse(loc, cb_state, current_submit_count);

    skip |= ValidateQueuedQFOTransfers(cb_state, qfo_image_scoreboards, qfo_buffer_scoreboards, loc);

    const char *const vuid = (loc.function == Func::vkQueueSubmit) ? "VUID-vkQueueSubmit-pCommandBuffers-00070"
                                                                   : "VUID-vkQueueSubmit2-commandBuffer-03874";
    skip |= ValidateCommandBufferState(cb_state, loc, current_submit_count, vuid);
    return skip;
}

bool CoreChecks::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                VkFence fence, const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto fence_state = Get<vvl::Fence>(fence)) {
        const LogObjectList objlist(queue, fence);
        skip |= ValidateFenceForSubmit(*fence_state, "VUID-vkQueueBindSparse-fence-01114", "VUID-vkQueueBindSparse-fence-01113",
                                       objlist, error_obj.location);
    }
    if (skip) return skip;

    auto queue_state = Get<vvl::Queue>(queue);
    const VkQueueFlags queue_flags = queue_state->queue_family_properties.queueFlags;
    if (!(queue_flags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        skip |= LogError("VUID-vkQueueBindSparse-queuetype", queue, error_obj.location,
                         "queueFamilyIndex %" PRIu32 " queueFlags are %s.", queue_state->queue_family_index,
                         string_VkQueueFlags(queue_flags).c_str());
    }

    SemaphoreSubmitState sem_submit_state(*this, queue, queue_flags);
    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; ++bind_idx) {
        const Location bind_info_loc = error_obj.location.dot(Struct::VkBindSparseInfo, Field::pBindInfo, bind_idx);
        const VkBindSparseInfo &bind_info = pBindInfo[bind_idx];

        skip |= ValidateSemaphoresForSubmit(sem_submit_state, bind_info, bind_info_loc);

        if (bind_info.pBufferBinds) {
            for (uint32_t buffer_idx = 0; buffer_idx < bind_info.bufferBindCount; ++buffer_idx) {
                const VkSparseBufferMemoryBindInfo &buffer_bind = bind_info.pBufferBinds[buffer_idx];
                if (buffer_bind.pBinds) {
                    auto buffer_state = Get<vvl::Buffer>(buffer_bind.buffer);
                    ASSERT_AND_CONTINUE(buffer_state);
                    for (uint32_t buffer_bind_idx = 0; buffer_bind_idx < buffer_bind.bindCount; ++buffer_bind_idx) {
                        const VkSparseMemoryBind &memory_bind = buffer_bind.pBinds[buffer_bind_idx];
                        const Location buffer_bind_info_loc = bind_info_loc.dot(Field::pBufferBinds, buffer_idx);
                        const Location bind_loc = buffer_bind_info_loc.dot(Field::pBinds, buffer_bind_idx);
                        skip |=
                            ValidateBufferSparseMemoryBindAlignments(memory_bind, *buffer_state, bind_loc, buffer_bind_info_loc);
                        skip |=
                            ValidateSparseMemoryBind(memory_bind, buffer_state->requirements, buffer_state->requirements.size,
                                                     buffer_state->external_memory_handle_types, buffer_state->Handle(), bind_loc);
                    }
                }
            }
        }

        if (bind_info.pImageOpaqueBinds) {
            for (uint32_t image_opaque_idx = 0; image_opaque_idx < bind_info.imageOpaqueBindCount; ++image_opaque_idx) {
                const VkSparseImageOpaqueMemoryBindInfo &image_opaque_bind = bind_info.pImageOpaqueBinds[image_opaque_idx];
                if (image_opaque_bind.pBinds) {
                    auto image_state = Get<vvl::Image>(image_opaque_bind.image);
                    ASSERT_AND_CONTINUE(image_state);
                    for (uint32_t image_opaque_bind_idx = 0; image_opaque_bind_idx < image_opaque_bind.bindCount;
                         ++image_opaque_bind_idx) {
                        const VkSparseMemoryBind &memory_bind = image_opaque_bind.pBinds[image_opaque_bind_idx];
                        const Location image_bind_info_loc = bind_info_loc.dot(Field::pImageOpaqueBinds, image_opaque_idx);
                        const Location bind_loc = image_bind_info_loc.dot(Field::pBinds, image_opaque_bind_idx);
                        // Assuming that no multiplanar disjointed images are possible with sparse memory binding. Needs
                        // confirmation
                        skip |= ValidateImageSparseMemoryBindAlignments(memory_bind, *image_state, bind_loc, image_bind_info_loc);
                        skip |=
                            ValidateSparseMemoryBind(memory_bind, image_state->requirements[0], image_state->requirements[0].size,
                                                     image_state->external_memory_handle_types, image_state->Handle(), bind_loc);
                    }
                }
            }
        }

        if (bind_info.pImageBinds) {
            for (uint32_t image_idx = 0; image_idx < bind_info.imageBindCount; ++image_idx) {
                const Location bind_loc = bind_info_loc.dot(Field::pImageBinds, image_idx);
                const VkSparseImageMemoryBindInfo &image_bind = bind_info.pImageBinds[image_idx];
                auto image_state = Get<vvl::Image>(image_bind.image);
                ASSERT_AND_CONTINUE(image_state);

                if (!image_state->sparse_residency) {
                    skip |=
                        LogError("VUID-VkSparseImageMemoryBindInfo-image-02901", image_bind.image, bind_loc.dot(Field::image),
                                 "was created with flags %s.", string_VkImageCreateFlags(image_state->create_info.flags).c_str());
                }

                for (uint32_t image_bind_idx = 0; image_bind_idx < image_bind.bindCount; ++image_bind_idx) {
                    const Location image_bind_loc = bind_loc.dot(Field::pBinds, image_bind_idx);
                    const VkSparseImageMemoryBind &memory_bind = image_bind.pBinds[image_bind_idx];
                    skip |= ValidateSparseImageMemoryBind(image_state.get(), memory_bind, bind_loc, image_bind_loc);
                }
            }
        }
    }
    return skip;
}
