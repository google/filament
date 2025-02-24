/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include <bitset>

// Generic function to handle validation for all CmdDraw* type functions
bool BestPractices::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const Location& loc) const {
    bool skip = false;
    const auto cb_state = GetRead<bp_state::CommandBuffer>(cmd_buffer);
    if (const auto* pipe = cb_state->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        if (const auto rp_state = cb_state->active_render_pass.get()) {
            for (uint32_t i = 0; i < rp_state->create_info.subpassCount; ++i) {
                const auto& subpass = rp_state->create_info.pSubpasses[i];
                const auto* ds_state = pipe->DepthStencilState();
                const uint32_t depth_stencil_attachment =
                    GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);
                const auto* raster_state = pipe->RasterizationState();
                if ((depth_stencil_attachment == VK_ATTACHMENT_UNUSED) && raster_state &&
                    raster_state->depthBiasEnable == VK_TRUE) {
                    skip |= LogWarning("BestPractices-vkCmdDraw-DepthBiasNoAttachment", cb_state->Handle(), loc,
                                       "depthBiasEnable == VK_TRUE without a depth-stencil attachment.");
                }
            }
        }
    }
    skip |= ValidatePushConstants(cmd_buffer, loc);
    return skip;
}

bool BestPractices::ValidateCmdDispatchType(VkCommandBuffer cmd_buffer, const Location& loc) const {
    bool skip = false;
    skip |= ValidatePushConstants(cmd_buffer, loc);
    return skip;
}

bool BestPractices::ValidatePushConstants(VkCommandBuffer cmd_buffer, const Location& loc) const {
    using Range = sparse_container::range<uint32_t>;

    bool skip = false;

    const auto cb_state = GetRead<bp_state::CommandBuffer>(cmd_buffer);

    if (!cb_state->push_constant_ranges_layout) {
        return skip;
    }

    for (const VkPushConstantRange& push_constant_range : *cb_state->push_constant_ranges_layout) {
        Range layout_range(push_constant_range.offset, push_constant_range.offset + push_constant_range.size);
        uint32_t size_not_set = push_constant_range.size;
        for (const vvl::CommandBuffer::PushConstantData& filled_pcr : cb_state->push_constant_data_chunks) {
            Range filled_range(filled_pcr.offset, filled_pcr.offset + static_cast<uint32_t>(filled_pcr.values.size()));
            Range intersection = layout_range & filled_range;
            if (intersection.valid()) {
                size_not_set -= std::min(intersection.distance(), size_not_set);
            }
            if (size_not_set == 0) {
                break;
            }
        }
        if (size_not_set > 0) {
            skip |= LogWarning("BestPractices-PushConstants", cmd_buffer, loc,
                               "Pipeline uses a push constant range with offset %" PRIu32 " and size %" PRIu32 ", but %" PRIu32
                               " bytes were never set with vkCmdPushConstants.",
                               push_constant_range.offset, push_constant_range.size, size_not_set);
            break;
        }
    }

    return skip;
}

void BestPractices::RecordCmdDrawType(bp_state::CommandBuffer& cb_state, uint32_t draw_count) {
    if (VendorCheckEnabled(kBPVendorArm)) {
        RecordCmdDrawTypeArm(cb_state, draw_count);
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordCmdDrawTypeNVIDIA(cb_state);
    }

    if (cb_state.render_pass_state.drawTouchAttachments) {
        for (auto& touch : cb_state.render_pass_state.nextDrawTouchesAttachments) {
            RecordAttachmentAccess(cb_state, touch.framebufferAttachment, touch.aspects);
        }
        // No need to touch the same attachments over and over.
        cb_state.render_pass_state.drawTouchAttachments = false;
    }

    const auto* pipeline_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (pipeline_state && !pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        if (pipeline_state->vertex_input_state && !pipeline_state->vertex_input_state->bindings.empty()) {
            cb_state.uses_vertex_buffer = true;
        }
    } else {
        if (!cb_state.dynamic_state_value.vertex_bindings.empty()) {
            cb_state.uses_vertex_buffer = true;
        }
    }
}

void BestPractices::RecordCmdDrawTypeArm(bp_state::CommandBuffer& cb_state, uint32_t draw_count) {
    auto& render_pass_state = cb_state.render_pass_state;
    // Each TBDR vendor requires a depth pre-pass draw call to have a minimum number of vertices/indices before it counts towards
    // depth prepass warnings First find the lowest enabled draw count
    uint32_t lowest_enabled_min_draw_count = 0;
    lowest_enabled_min_draw_count = VendorCheckEnabled(kBPVendorArm) * kDepthPrePassMinDrawCountArm;
    if (VendorCheckEnabled(kBPVendorIMG) && kDepthPrePassMinDrawCountIMG < lowest_enabled_min_draw_count) {
        lowest_enabled_min_draw_count = kDepthPrePassMinDrawCountIMG;
    }

    if (draw_count >= lowest_enabled_min_draw_count) {
        if (render_pass_state.depthOnly) render_pass_state.numDrawCallsDepthOnly++;
        if (render_pass_state.depthEqualComparison) render_pass_state.numDrawCallsDepthEqualCompare++;
    }
}

void BestPractices::RecordCmdDrawTypeNVIDIA(bp_state::CommandBuffer& cb_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (cb_state.nv.depth_test_enable && cb_state.nv.zcull_direction != ZcullDirection::Unknown) {
        RecordSetScopeZcullDirection(cb_state, cb_state.nv.zcull_direction);
        RecordZcullDraw(cb_state);
    }
}

bool BestPractices::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance, const ErrorObject& error_obj) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= LogWarning("BestPractices-vkCmdDraw-instance-count-zero", device, error_obj.location.dot(Field::instanceCount),
                           "is zero.");
    }
    skip |= ValidateCmdDrawType(commandBuffer, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= LogWarning("BestPractices-vkCmdDrawIndexed-instance-count-zero", device,
                           error_obj.location.dot(Field::instanceCount), "is zero.");
    }
    skip |= ValidateCmdDrawType(commandBuffer, error_obj.location);

    // Check if we reached the limit for small indexed draw calls.
    // Note that we cannot update the draw call count here, so we do it in PostCallRecordCmdDrawIndexed.
    const auto cb_state = GetRead<bp_state::CommandBuffer>(commandBuffer);
    if ((indexCount * instanceCount) <= kSmallIndexedDrawcallIndices &&
        (cb_state->small_indexed_draw_call_count == kMaxSmallIndexedDrawcalls - 1) &&
        (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG))) {
        skip |= LogPerformanceWarning("BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls", device, error_obj.location,
                                      "%s %s: The command buffer contains many small indexed drawcalls "
                                      "(at least %u drawcalls with less than %u indices each). This may cause pipeline bubbles. "
                                      "You can try batching drawcalls or instancing when applicable.",
                                      VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), kMaxSmallIndexedDrawcalls,
                                      kSmallIndexedDrawcallIndices);
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        skip |= ValidateIndexBufferArm(*cb_state, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                       error_obj.location);
    }

    return skip;
}

void BestPractices::PostTransformLRUCacheModel::resize(size_t size) { _entries.resize(size); }

bool BestPractices::PostTransformLRUCacheModel::query_cache(uint32_t value) {
    // look for a cache hit
    auto hit = std::find_if(_entries.begin(), _entries.end(), [value](const CacheEntry& entry) { return entry.value == value; });
    if (hit != _entries.end()) {
        // mark the cache hit as being most recently used
        hit->age = iteration++;
        return true;
    }

    // if there's no cache hit, we need to model the entry being inserted into the cache
    CacheEntry new_entry = {value, iteration};
    if (iteration < static_cast<uint32_t>(std::distance(_entries.begin(), _entries.end()))) {
        // if there is still space left in the cache, use the next available slot
        *(_entries.begin() + iteration) = new_entry;
    } else {
        // otherwise replace the least recently used cache entry
        auto lru = std::min_element(_entries.begin(), hit, [](const CacheEntry& a, const CacheEntry& b) { return a.age < b.age; });
        *lru = new_entry;
    }
    iteration++;
    return false;
}

bool BestPractices::ValidateIndexBufferArm(const bp_state::CommandBuffer& cb_state, uint32_t indexCount, uint32_t instanceCount,
                                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                           const Location& loc) const {
    bool skip = false;

    // check for sparse/underutilised index buffer, and post-transform cache thrashing
    const auto ib_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
    // If the maintenance6 feature is enabled, buffer can be VK_NULL_HANDLE. If buffer is VK_NULL_HANDLE and the nullDescriptor
    // feature is enabled, every index fetched results in a value of zero.
    if (!ib_state) {
        return skip;
    }

    const VkIndexType ib_type = cb_state.index_buffer_binding.index_type;
    const auto ib_memory_state = ib_state->MemoryState();
    if (!ib_memory_state) return skip;

    const void* ib_mem = ib_memory_state->p_driver_data;

    const auto& last_bound_state = cb_state.lastBound[ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)];
    const bool primitive_restart_enable = last_bound_state.IsPrimitiveRestartEnable();

    // no point checking index buffer if the memory is nonexistant/unmapped, or if there is no graphics pipeline bound to this CB
    if (ib_mem) {
        const uint32_t scan_stride = GetIndexAlignment(ib_type);
        // Check if all indices are within the memory allocation size, if robustness is enabled they might not be
        if ((firstIndex + indexCount) * scan_stride > ib_memory_state->allocate_info.allocationSize) {
            return skip;
        }
        const uint8_t* scan_begin = static_cast<const uint8_t*>(ib_mem) + firstIndex * scan_stride;
        const uint8_t* scan_end = scan_begin + indexCount * scan_stride;

        // Min and max are important to track for some Mali architectures. In older Mali devices without IDVS, all
        // vertices corresponding to indices between the minimum and maximum may be loaded, and possibly shaded,
        // irrespective of whether or not they're part of the draw call.

        // start with minimum as 0xFFFFFFFF and adjust to indices in the buffer
        uint32_t min_index = ~0u;
        // start with maximum as 0 and adjust to indices in the buffer
        uint32_t max_index = 0u;

        // first scan-through, we're looking to simulate a model LRU post-transform cache, estimating the number of vertices shaded
        // for the given index buffer
        uint32_t vertex_shade_count = 0;

        PostTransformLRUCacheModel post_transform_cache;

        // The size of the cache being modelled positively correlates with how much behaviour it can capture about
        // arbitrary ground-truth hardware/architecture cache behaviour. I.e. it's a good solution when we don't know the
        // target architecture.
        // However, modelling a post-transform cache with more than 32 elements gives diminishing returns in practice.
        // http://eelpi.gotdns.org/papers/fast_vert_cache_opt.html
        post_transform_cache.resize(32);

        for (const uint8_t* scan_ptr = scan_begin; scan_ptr < scan_end; scan_ptr += scan_stride) {
            uint32_t scan_index;
            uint32_t primitive_restart_value;
            if (ib_type == VK_INDEX_TYPE_UINT8) {
                scan_index = *reinterpret_cast<const uint8_t*>(scan_ptr);
                primitive_restart_value = 0xFF;
            } else if (ib_type == VK_INDEX_TYPE_UINT16) {
                scan_index = *reinterpret_cast<const uint16_t*>(scan_ptr);
                primitive_restart_value = 0xFFFF;
            } else {
                scan_index = *reinterpret_cast<const uint32_t*>(scan_ptr);
                primitive_restart_value = 0xFFFFFFFF;
            }

            max_index = std::max(max_index, scan_index);
            min_index = std::min(min_index, scan_index);

            if (!primitive_restart_enable || scan_index != primitive_restart_value) {
                const bool in_cache = post_transform_cache.query_cache(scan_index);
                // if the shaded vertex corresponding to the index is not in the PT-cache, we need to shade again
                if (!in_cache) vertex_shade_count++;
            }
        }

        // if the max and min values were not set, then we either have no indices, or all primitive restarts, exit...
        // if the max and min are the same, then it implies all the indices are the same, then we don't need to do anything
        if (max_index < min_index || max_index == min_index) return skip;

        if (max_index - min_index >= indexCount) {
            skip |=
                LogPerformanceWarning("BestPractices-Arm-vkCmdDrawIndexed-sparse-index-buffer", device, loc,
                                      "%s The indices which were specified for the draw call only utilise approximately %.02f%% of "
                                      "index buffer value range. Arm Mali architectures before G71 do not have IDVS (Index-Driven "
                                      "Vertex Shading), meaning all vertices corresponding to indices between the minimum and "
                                      "maximum would be loaded, and possibly shaded, whether or not they are used.",
                                      VendorSpecificTag(kBPVendorArm),
                                      (static_cast<float>(indexCount) / static_cast<float>(max_index - min_index)) * 100.0f);
            return skip;
        }

        // use a dynamic vector of bitsets as a memory-compact representation of which indices are included in the draw call
        // each bit of the n-th bucket contains the inclusion information for indices (n*n_buckets) to ((n+1)*n_buckets)
        const size_t refs_per_bucket = 64;
        std::vector<std::bitset<refs_per_bucket>> vertex_reference_buckets;

        const uint32_t n_indices = max_index - min_index + 1;
        const uint32_t n_buckets = (n_indices / static_cast<uint32_t>(refs_per_bucket)) +
                                   ((n_indices % static_cast<uint32_t>(refs_per_bucket)) != 0 ? 1 : 0);

        // there needs to be at least one bitset to store a set of indices smaller than n_buckets
        vertex_reference_buckets.resize(std::max(1u, n_buckets));

        // To avoid using too much memory, we run over the indices again.
        // Knowing the size from the last scan allows us to record index usage with bitsets
        for (const uint8_t* scan_ptr = scan_begin; scan_ptr < scan_end; scan_ptr += scan_stride) {
            uint32_t scan_index;
            if (ib_type == VK_INDEX_TYPE_UINT8) {
                scan_index = *reinterpret_cast<const uint8_t*>(scan_ptr);
            } else if (ib_type == VK_INDEX_TYPE_UINT16) {
                scan_index = *reinterpret_cast<const uint16_t*>(scan_ptr);
            } else {
                scan_index = *reinterpret_cast<const uint32_t*>(scan_ptr);
            }
            // keep track of the set of all indices used to reference vertices in the draw call
            size_t index_offset = scan_index - min_index;
            size_t bitset_bucket_index = index_offset / refs_per_bucket;
            uint64_t used_indices = 1ull << ((index_offset % refs_per_bucket) & 0xFFFFFFFFu);
            vertex_reference_buckets[bitset_bucket_index] |= used_indices;
        }

        uint32_t vertex_reference_count = 0;
        for (const auto& bitset : vertex_reference_buckets) {
            vertex_reference_count += static_cast<uint32_t>(bitset.count());
        }

        // low index buffer utilization implies that: of the vertices available to the draw call, not all are utilized
        float utilization = static_cast<float>(vertex_reference_count) / static_cast<float>(max_index - min_index + 1);
        // low hit rate (high miss rate) implies the order of indices in the draw call may be possible to improve
        float cache_hit_rate = static_cast<float>(vertex_reference_count) / static_cast<float>(vertex_shade_count);

        if (utilization < 0.5f) {
            skip |= LogPerformanceWarning("BestPractices-Arm-vkCmdDrawIndexed-sparse-index-buffer", device, loc,
                                          "%s The indices which were specified for the draw call only utilise approximately "
                                          "%.02f%% of the bound vertex buffer.",
                                          VendorSpecificTag(kBPVendorArm), utilization);
        }

        if (cache_hit_rate <= 0.5f) {
            skip |=
                LogPerformanceWarning("BestPractices-Arm-vkCmdDrawIndexed-post-transform-cache-thrashing", device, loc,
                                      "%s The indices which were specified for the draw call are estimated to cause thrashing of "
                                      "the post-transform vertex cache, with a hit-rate of %.02f%%. "
                                      "I.e. the ordering of the index buffer may not make optimal use of indices associated with "
                                      "recently shaded vertices.",
                                      VendorSpecificTag(kBPVendorArm), cache_hit_rate * 100.0f);
        }
    }

    return skip;
}

void BestPractices::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                 uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                 const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                               record_obj);
    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, indexCount * instanceCount);

    if ((indexCount * instanceCount) <= kSmallIndexedDrawcallIndices) {
        cb_state->small_indexed_draw_call_count++;
    }

    ValidateBoundDescriptorSets(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
}

bool BestPractices::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= LogWarning("BestPractices-vkCmdDrawIndirect-draw-count-zero", device, error_obj.location.dot(Field::drawCount),
                           "is zero.");
    }

    skip |= ValidateCmdDrawType(commandBuffer, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= LogWarning("BestPractices-vkCmdDrawIndexedIndirect-draw-count-zero", device,
                           error_obj.location.dot(Field::drawCount), "is zero.");
    }

    skip |= ValidateCmdDrawType(commandBuffer, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride,
                                                              const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                            maxDrawCount, stride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, 0);
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject& error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject& error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                              record_obj);
}

bool BestPractices::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                               uint32_t firstInstance, VkBuffer counterBuffer,
                                                               VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                               uint32_t vertexStride, const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                              uint32_t firstInstance, VkBuffer counterBuffer,
                                                              VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                              uint32_t vertexStride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                            counterBufferOffset, counterOffset, vertexStride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, 0);
}

bool BestPractices::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                     stride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, 0);
}

bool BestPractices::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject& error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void BestPractices::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

bool BestPractices::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject& error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void BestPractices::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
    PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                       record_obj);
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkBuffer countBuffer,
                                                                   VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                   uint32_t stride, const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                maxDrawCount, stride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, 0);
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              uint32_t drawCount, uint32_t stride,
                                                              const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, 0);
}

bool BestPractices::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                      const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                     const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, 0);
}

bool BestPractices::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                          const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                          uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                          const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                         const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                         uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                         const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride,
                                                       pVertexOffset, record_obj);
    uint32_t count = 0;
    for (uint32_t i = 0; i < drawCount; ++i) {
        count += pIndexInfo[i].indexCount;
    }
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, count);
}

bool BestPractices::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                   const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                   uint32_t firstInstance, uint32_t stride, const ErrorObject& error_obj) const {
    return ValidateCmdDrawType(commandBuffer, error_obj.location);
}

void BestPractices::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                  const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount,
                                                  uint32_t firstInstance, uint32_t stride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride,
                                                record_obj);
    uint32_t count = 0;
    for (uint32_t i = 0; i < drawCount; ++i) {
        count += pVertexInfo[i].vertexCount;
    }
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdDrawType(*cb_state, count);
}

void BestPractices::ValidateBoundDescriptorSets(bp_state::CommandBuffer& cb_state, VkPipelineBindPoint bind_point, Func command) {
    auto lvl_bind_point = ConvertToLvlBindPoint(bind_point);
    auto& last_bound = cb_state.lastBound[lvl_bind_point];

    for (const auto& ds_slot : last_bound.ds_slots) {
        if (!ds_slot.ds_state) continue;
        for (const auto& binding : *ds_slot.ds_state) {
            // For bindless scenarios, we should not attempt to track descriptor set state.
            // It is highly uncertain which resources are actually bound.
            // Resources which are written to such a descriptor should be marked as indeterminate w.r.t. state.
            if (binding->binding_flags & (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                          VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT)) {
                continue;
            }

            for (uint32_t i = 0; i < binding->count; ++i) {
                VkImageView image_view{VK_NULL_HANDLE};

                auto descriptor = binding->GetDescriptor(i);
                if (!descriptor) {
                    continue;
                }
                switch (descriptor->GetClass()) {
                    case vvl::DescriptorClass::Image: {
                        if (const auto image_descriptor = static_cast<const vvl::ImageDescriptor*>(descriptor)) {
                            image_view = image_descriptor->GetImageView();
                        }
                        break;
                    }
                    case vvl::DescriptorClass::ImageSampler: {
                        if (const auto image_sampler_descriptor =
                                static_cast<const vvl::ImageSamplerDescriptor*>(descriptor)) {
                            image_view = image_sampler_descriptor->GetImageView();
                        }
                        break;
                    }
                    default:
                        break;
                }

                if (image_view) {
                    auto image_view_state = Get<vvl::ImageView>(image_view);
                    QueueValidateImageView(cb_state.queue_submit_functions, command, image_view_state.get(),
                                           IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS);
                }
            }
        }
    }
}

void BestPractices::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                          uint32_t firstVertex, uint32_t firstInstance, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    RecordCmdDrawType(*cb_state, vertexCount * instanceCount);
}

void BestPractices::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    RecordCmdDrawType(*cb_state, drawCount);
}

void BestPractices::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location.function);
    RecordCmdDrawType(*cb_state, drawCount);
}

bool BestPractices::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ, const ErrorObject& error_obj) const {
    bool skip = false;

    if ((groupCountX == 0) || (groupCountY == 0) || (groupCountZ == 0)) {
        skip |= LogWarning("BestPractices-vkCmdDispatch-group-count-zero", device, error_obj.location,
                           "one or more groupCounts are zero (groupCountX = %" PRIu32 ", groupCountY = %" PRIu32
                           ", groupCountZ = %" PRIu32 ").",
                           groupCountX, groupCountY, groupCountZ);
    }
    skip |= ValidateCmdDispatchType(commandBuffer, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= ValidateCmdDispatchType(commandBuffer, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ, const ErrorObject& error_obj) const {
    bool skip = false;

    if ((groupCountX == 0) || (groupCountY == 0) || (groupCountZ == 0)) {
        skip |= LogWarning("BestPractices-vkCmdDispatchBase-group-count-zero", device, error_obj.location,
                           "one or more groupCounts are zero (groupCountX = %" PRIu32 ", groupCountY = %" PRIu32
                           ", groupCountZ = %" PRIu32 ").",
                           groupCountX, groupCountY, groupCountZ);
    }
    skip |= ValidateCmdDispatchType(commandBuffer, error_obj.location);

    return skip;
}

bool BestPractices::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                      uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                      uint32_t groupCountZ, const ErrorObject& error_obj) const {
    return PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                          error_obj);
}

void BestPractices::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                              const RecordObject& record_obj) {
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location.function);
}

void BestPractices::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      const RecordObject& record_obj) {
    const auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    ValidateBoundDescriptorSets(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location.function);
}

bool BestPractices::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    bool skip = false;
    const auto cb_state = GetRead<bp_state::CommandBuffer>(commandBuffer);

    if (!cb_state->current_vertex_buffer_binding_info.empty() && !cb_state->uses_vertex_buffer) {
        skip |=
            LogPerformanceWarning("BestPractices-vkEndCommandBuffer-VtxIndexOutOfBounds", cb_state->Handle(), error_obj.location,
                                  "Vertex buffers was bound to %s but no draws had a pipeline that used the vertex buffer.",
                                  FormatHandle(cb_state->Handle()).c_str());
    }
    return skip;
}
