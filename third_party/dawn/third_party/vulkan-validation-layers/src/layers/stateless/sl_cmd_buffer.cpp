/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "containers/range_vector.h"

namespace stateless {
ReadLockGuard Device::ReadLock() const { return ReadLockGuard(validation_object_mutex, std::defer_lock); }
WriteLockGuard Device::WriteLock() { return WriteLockGuard(validation_object_mutex, std::defer_lock); }

bool Device::ValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                        const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdBindIndexBuffer;
    const char *vuid;

    if (buffer == VK_NULL_HANDLE) {
        if (!enabled_features.maintenance6) {
            vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-None-09493" : "VUID-vkCmdBindIndexBuffer-None-09493";
            skip |= LogError(vuid, commandBuffer, loc.dot(Field::buffer), "is VK_NULL_HANDLE.");
        } else if (offset != 0) {
            vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-buffer-09494" : "VUID-vkCmdBindIndexBuffer-buffer-09494";
            skip |= LogError(vuid, commandBuffer, loc.dot(Field::buffer), "is VK_NULL_HANDLE but offset is (%" PRIu64 ").", offset);
        }
    }

    if (indexType == VK_INDEX_TYPE_NONE_KHR) {
        vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-indexType-08786" : "VUID-vkCmdBindIndexBuffer-indexType-08786";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::indexType), "is VK_INDEX_TYPE_NONE_KHR.");
    }

    if (indexType == VK_INDEX_TYPE_UINT8 && !enabled_features.indexTypeUint8) {
        vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-indexType-08787" : "VUID-vkCmdBindIndexBuffer-indexType-08787";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::indexType),
                         "is VK_INDEX_TYPE_UINT8 but indexTypeUint8 feature was not enabled.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkIndexType indexType, const Context &context) const {
    return ValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType, context.error_obj.location);
}

bool Device::manual_PreCallValidateCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkDeviceSize size, VkIndexType indexType, const Context &context) const {
    return ValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType, context.error_obj.location);
}

bool Device::manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (firstBinding > device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdBindVertexBuffers-firstBinding-00624", commandBuffer, error_obj.location,
                         "firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ").", firstBinding,
                         device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdBindVertexBuffers-firstBinding-00625", commandBuffer, error_obj.location,
                         "sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ").",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers == nullptr) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers-pBuffers-parameter", commandBuffer,
                             error_obj.location.dot(Field::pBuffers), "is NULL.");
            return skip;
        }
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
            if (!enabled_features.nullDescriptor) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers-pBuffers-04001", commandBuffer, buffer_loc, "is VK_NULL_HANDLE.");
            } else {
                if (pOffsets[i] != 0) {
                    skip |= LogError("VUID-vkCmdBindVertexBuffers-pBuffers-04002", commandBuffer, buffer_loc,
                                     "is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0.", i);
                }
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                      uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                      const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                      const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-transformFeedback-02355", commandBuffer, error_obj.location,
                         "transformFeedback feature was not enabled.");
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pOffsets[i] & 3) {
            skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359", commandBuffer,
                             error_obj.location.dot(Field::pOffsets, i), "(%" PRIu64 ") is not a multiple of 4.", pOffsets[i]);
        }
    }

    if (firstBinding >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356", commandBuffer,
                         error_obj.location.dot(Field::firstBinding),
                         "(%" PRIu32
                         ") is greater than or equal to "
                         "maxTransformFeedbackBuffers (%" PRIu32 ").",
                         firstBinding, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstBinding + bindingCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357", commandBuffer,
                         error_obj.location.dot(Field::firstBinding),
                         "(%" PRIu32 ") plus bindCount (%" PRIu32 ") is greater than maxTransformFeedbackBuffers (%" PRIu32 ").",
                         firstBinding, bindingCount, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        // pSizes is optional and may be nullptr.
        if (pSizes != nullptr) {
            if (pSizes[i] != VK_WHOLE_SIZE &&
                pSizes[i] > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferSize) {
                skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361", commandBuffer,
                                 error_obj.location.dot(Field::pSizes, i),
                                 "(%" PRIu64
                                 ") is not VK_WHOLE_SIZE and is greater than "
                                 "maxTransformFeedbackBufferSize (%" PRIu64 ").",
                                 pSizes[i], phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferSize);
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                                const VkDeviceSize *pCounterBufferOffsets,
                                                                const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-transformFeedback-02366", commandBuffer, error_obj.location,
                         "transformFeedback feature was not enabled.");
    }

    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368", commandBuffer,
                         error_obj.location.dot(Field::firstCounterBuffer),
                         "(%" PRIu32 ") is not less than maxTransformFeedbackBuffers (%" PRIu32 ").", firstCounterBuffer,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(
            "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369", commandBuffer,
            error_obj.location.dot(Field::firstCounterBuffer),
            "(%" PRIu32 ") plus counterBufferCount (%" PRIu32 ") is greater than maxTransformFeedbackBuffers (%" PRIu32 ").",
            firstCounterBuffer, counterBufferCount, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                              uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                              const VkDeviceSize *pCounterBufferOffsets,
                                                              const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-transformFeedback-02374", commandBuffer, error_obj.location,
                         "transformFeedback feature was not enabled.");
    }

    // pCounterBuffers and pCounterBufferOffsets are optional and may be nullptr.
    //  Additionaly, pCounterBufferOffsets must be nullptr if pCounterBuffers is nullptr.
    if (pCounterBuffers == nullptr && pCounterBufferOffsets != nullptr) {
        skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffer-02379", commandBuffer,
                         error_obj.location.dot(Field::pCounterBuffers), "is NULL but pCounterBufferOffsets is not NULL.");
    }

    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376", commandBuffer,
                         error_obj.location.dot(Field::firstCounterBuffer),
                         "(%" PRIu32 ") is not less than maxTransformFeedbackBuffers (%" PRIu32 ").", firstCounterBuffer,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(
            "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377", commandBuffer,
            error_obj.location.dot(Field::firstCounterBuffer),
            "(%" PRIu32 ") plus counterBufferCount (%" PRIu32 ") is greater than maxTransformFeedbackBuffers (%" PRIu32 ").",
            firstCounterBuffer, counterBufferCount, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                         const VkDeviceSize *pStrides, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    // Check VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength
    // This is a special case and generator currently skips it
    {
        const bool vuid_condition = (pSizes != nullptr) || (pStrides != nullptr);
        const bool vuid_expectation = bindingCount > 0;
        if (vuid_condition) {
            if (!vuid_expectation) {
                const char *not_null_msg = "";
                if ((pSizes != nullptr) && (pStrides != nullptr)) {
                    not_null_msg = "pSizes and pStrides are not NULL";
                } else if (pSizes != nullptr) {
                    not_null_msg = "pSizes is not NULL";
                } else {
                    not_null_msg = "pStrides is not NULL";
                }
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength", commandBuffer, error_obj.location,
                                 "%s, so bindingCount must be greater than 0.", not_null_msg);
            }
        }

        if (!pOffsets && bindingCount > 0) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers2-pOffsets-parameter", commandBuffer,
                             error_obj.location.dot(Field::pOffsets), "is NULL.");
        }
    }

    if (firstBinding >= device_limits.maxVertexInputBindings) {
        skip |=
            LogError("VUID-vkCmdBindVertexBuffers2-firstBinding-03355", commandBuffer, error_obj.location.dot(Field::firstBinding),
                     "(%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ").", firstBinding,
                     device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |=
            LogError("VUID-vkCmdBindVertexBuffers2-firstBinding-03356", commandBuffer, error_obj.location.dot(Field::firstBinding),
                     "(%" PRIu32 ") + bindingCount (%" PRIu32
                     ") must be less than "
                     "maxVertexInputBindings (%" PRIu32 ").",
                     firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers == nullptr) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-parameter", commandBuffer,
                             error_obj.location.dot(Field::pBuffers), "is NULL.");
            return skip;
        }
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
            if (!enabled_features.nullDescriptor) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-04111", commandBuffer, buffer_loc, "is VK_NULL_HANDLE.");
            } else if (pOffsets && pOffsets[i] != 0) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-04112", commandBuffer, buffer_loc,
                                 "is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0.", i);
            }
        }
        if (pStrides) {
            if (pStrides[i] > device_limits.maxVertexInputBindingStride) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pStrides-03362", commandBuffer,
                                 error_obj.location.dot(Field::pStrides, i),
                                 "(%" PRIu64 ") must be less than maxVertexInputBindingStride (%" PRIu32 ").", pStrides[i],
                                 device_limits.maxVertexInputBindingStride);
            }
        }
    }

    return skip;
}

bool Device::ValidateCmdPushConstants(VkCommandBuffer commandBuffer, uint32_t offset, uint32_t size, const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdPushConstants;
    const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
    // Check that offset + size don't exceed the max.
    // Prevent arithetic overflow here by avoiding addition and testing in this order.
    if (offset >= max_push_constants_size) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfo-offset-00370" : "VUID-vkCmdPushConstants-offset-00370";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::offset),
                         "(%" PRIu32 ") is greater than maxPushConstantSize (%" PRIu32 ").", offset, max_push_constants_size);
    }
    if (size > max_push_constants_size - offset) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfo-size-00371" : "VUID-vkCmdPushConstants-size-00371";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::offset),
                         "(%" PRIu32 ") plus size (%" PRIu32 ") is greater than maxPushConstantSize (%" PRIu32 ").", offset, size,
                         max_push_constants_size);
    }

    if (SafeModulo(size, 4) != 0) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfo-size-00369" : "VUID-vkCmdPushConstants-size-00369";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::size), "(%" PRIu32 ") must be a multiple of 4.", size);
    }

    if (SafeModulo(offset, 4) != 0) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfo-offset-00368" : "VUID-vkCmdPushConstants-offset-00368";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::offset), "(%" PRIu32 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                    VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                    const void *pValues, const Context &context) const {
    return ValidateCmdPushConstants(commandBuffer, offset, size, context.error_obj.location);
}

bool Device::manual_PreCallValidateCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo,
                                                     const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    skip |= ValidateCmdPushConstants(commandBuffer, pPushConstantsInfo->offset, pPushConstantsInfo->size,
                                     error_obj.location.dot(Field::pPushConstantsInfo));
    if (pPushConstantsInfo->layout == VK_NULL_HANDLE) {
        if (!enabled_features.dynamicPipelineLayout) {
            skip |= LogError("VUID-VkPushConstantsInfo-None-09495", commandBuffer,
                             error_obj.location.dot(Field::pPushConstantsInfo).dot(Field::layout), "is VK_NULL_HANDLE.");
        } else if (!vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pPushConstantsInfo->pNext)) {
            skip |= LogError("VUID-VkPushConstantsInfo-layout-09496", commandBuffer,
                             error_obj.location.dot(Field::pPushConstantsInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!pColor) {
        skip |= LogError("VUID-vkCmdClearColorImage-pColor-04961", commandBuffer, error_obj.location, "pColor must not be null");
    }
    return skip;
}

bool Device::manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                       uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride,
                                                       VkQueryResultFlags flags, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkGetQueryPoolResults-flags-09443", device, error_obj.location.dot(Field::flags),
                         "(%s) include both STATUS_BIT and AVAILABILITY_BIT.", string_VkQueryResultFlags(flags).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if ((pConditionalRenderingBegin->offset & 3) != 0) {
        skip |= LogError("VUID-VkConditionalRenderingBeginInfoEXT-offset-01984", commandBuffer,
                         error_obj.location.dot(Field::pConditionalRenderingBegin).dot(Field::offset),
                         "(%" PRIu64 ") is not a multiple of 4.", pConditionalRenderingBegin->offset);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                       const VkClearRect *pRects, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    for (uint32_t rect = 0; rect < rectCount; rect++) {
        const Location rect_loc = error_obj.location.dot(Field::pRects, rect);
        if (pRects[rect].layerCount == 0) {
            skip |=
                LogError("VUID-vkCmdClearAttachments-layerCount-01934", commandBuffer, rect_loc.dot(Field::layerCount), "is zero.");
        }
        if (pRects[rect].rect.extent.width == 0) {
            skip |= LogError("VUID-vkCmdClearAttachments-rect-02682", commandBuffer,
                             rect_loc.dot(Field::rect).dot(Field::extent).dot(Field::width), "is zero.");
        }
        if (pRects[rect].rect.extent.height == 0) {
            skip |= LogError("VUID-vkCmdClearAttachments-rect-02683", commandBuffer,
                             rect_loc.dot(Field::rect).dot(Field::extent).dot(Field::height), "is zero.");
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pRegions != nullptr) {
        for (uint32_t i = 0; i < regionCount; i++) {
            if (pRegions[i].size == 0) {
                skip |= LogError("VUID-VkBufferCopy-size-01988", commandBuffer,
                                 error_obj.location.dot(Field::pRegions, i).dot(Field::size), "is zero");
            }
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pCopyBufferInfo->pRegions != nullptr) {
        for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
            if (pCopyBufferInfo->pRegions[i].size == 0) {
                skip |=
                    LogError("VUID-VkBufferCopy2-size-01988", commandBuffer,
                             error_obj.location.dot(Field::pCopyBufferInfo).dot(Field::pRegions, i).dot(Field::size), "is zero");
            }
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (dstOffset & 3) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdUpdateBuffer-dstOffset-00036", objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00037", objlist, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 "), must be greater than zero and less than or equal to 65536.", dataSize);
    } else if (dataSize & 3) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00038", objlist, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 "), is not a multiple of 4.", dataSize);
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (dstOffset & 3) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdFillBuffer-dstOffset-00025", objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 ") is not a multiple of 4.", dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            skip |= LogError("VUID-vkCmdFillBuffer-size-00026", objlist, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") must be greater than zero.", size);
        } else if (size & 3) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            skip |= LogError("VUID-vkCmdFillBuffer-size-00028", objlist, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") is not a multiple of 4.", size);
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                               const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                               const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.descriptorBuffer) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-None-08047", commandBuffer, error_obj.location,
                         "descriptorBuffer feature was not enabled.");
    }

    for (uint32_t i = 0; i < bufferCount; i++) {
        if (!vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(pBindingInfos[i].pNext)) {
            skip |= context.ValidateFlags(error_obj.location.dot(Field::pBindingInfos, i).dot(Field::usage),
                                          vvl::FlagBitmask::VkBufferUsageFlagBits, AllVkBufferUsageFlagBits, pBindingInfos[i].usage,
                                          kRequiredFlags, "VUID-VkDescriptorBufferBindingInfoEXT-None-09499",
                                          "VUID-VkDescriptorBufferBindingInfoEXT-None-09500");
        }
    }

    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(pExternalBufferInfo->pNext)) {
        skip |= context.ValidateFlags(error_obj.location.dot(Field::pExternalBufferInfo).dot(Field::usage),
                                      vvl::FlagBitmask::VkBufferUsageFlagBits, AllVkBufferUsageFlagBits, pExternalBufferInfo->usage,
                                      kRequiredFlags, "VUID-VkPhysicalDeviceExternalBufferInfo-None-09499",
                                      "VUID-VkPhysicalDeviceExternalBufferInfo-None-09500");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet *pDescriptorWrites,
                                                        const Context &context) const {
    return ValidateWriteDescriptorSet(context, context.error_obj.location, descriptorWriteCount, pDescriptorWrites);
}

bool Device::manual_PreCallValidateCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                         const VkPushDescriptorSetInfo *pPushDescriptorSetInfo,
                                                         const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    skip |= ValidateWriteDescriptorSet(context, error_obj.location, pPushDescriptorSetInfo->descriptorWriteCount,
                                       pPushDescriptorSetInfo->pDescriptorWrites);
    if (pPushDescriptorSetInfo->layout == VK_NULL_HANDLE) {
        if (!enabled_features.dynamicPipelineLayout) {
            skip |= LogError("VUID-VkPushDescriptorSetInfo-None-09495", commandBuffer,
                             error_obj.location.dot(Field::pPushDescriptorSetInfo).dot(Field::layout), "is VK_NULL_HANDLE.");
        } else if (vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pPushDescriptorSetInfo->pNext)) {
            skip |= LogError("VUID-VkPushDescriptorSetInfo-layout-09496", commandBuffer,
                             error_obj.location.dot(Field::pPushDescriptorSetInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
        }
    }
    return skip;
}

bool Device::ValidateViewport(const VkViewport &viewport, VkCommandBuffer object, const Location &loc) const {
    bool skip = false;

    // Note: for numerical correctness
    //       - float comparisons should expect NaN (comparison always false).
    //       - VkPhysicalDeviceLimits::maxViewportDimensions is uint32_t, not float -> careful.

    const auto f_lte_u32_exact = [](const float v1_f, const uint32_t v2_u32) {
        if (std::isnan(v1_f)) return false;
        if (v1_f <= 0.0f) return true;

        float intpart;
        const float fract = modff(v1_f, &intpart);

        assert(std::numeric_limits<float>::radix == 2);
        const float u32_max_plus1 = ldexpf(1.0f, 32);  // hopefully exact
        if (intpart >= u32_max_plus1) return false;

        uint32_t v1_u32 = static_cast<uint32_t>(intpart);
        if (v1_u32 < v2_u32) {
            return true;
        } else if (v1_u32 == v2_u32 && fract == 0.0f) {
            return true;
        } else {
            return false;
        }
    };

    const auto f_lte_u32_direct = [](const float v1_f, const uint32_t v2_u32) {
        const float v2_f = static_cast<float>(v2_u32);  // not accurate for > radix^digits; and undefined rounding mode
        return (v1_f <= v2_f);
    };

    // width
    bool width_healthy = true;
    const auto max_w = device_limits.maxViewportDimensions[0];

    if (!(viewport.width > 0.0f)) {
        width_healthy = false;
        skip |= LogError("VUID-VkViewport-width-01770", object, loc.dot(Field::width), "(%f) is not greater than zero.",
                         viewport.width);
    } else if (!(f_lte_u32_exact(viewport.width, max_w) || f_lte_u32_direct(viewport.width, max_w))) {
        width_healthy = false;
        skip |= LogError("VUID-VkViewport-width-01771", object, loc.dot(Field::width),
                         "(%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (%" PRIu32 ").", viewport.width, max_w);
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled =
        IsExtEnabled(extensions.vk_khr_maintenance1) || IsExtEnabled(extensions.vk_amd_negative_viewport_height);
    const auto max_h = device_limits.maxViewportDimensions[1];

    if (!negative_height_enabled && !(viewport.height > 0.0f)) {
        height_healthy = false;
        skip |= LogError("VUID-VkViewport-apiVersion-07917", object, loc.dot(Field::height), "(%f) is not greater zero.",
                         viewport.height);
    } else if (!(f_lte_u32_exact(fabsf(viewport.height), max_h) || f_lte_u32_direct(fabsf(viewport.height), max_h))) {
        height_healthy = false;

        skip |= LogError("VUID-VkViewport-height-01773", object, loc.dot(Field::height),
                         "absolute value (%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (%" PRIu32 ").",
                         viewport.height, max_h);
    }

    // x
    bool x_healthy = true;
    if (!(viewport.x >= device_limits.viewportBoundsRange[0])) {
        x_healthy = false;
        skip |= LogError("VUID-VkViewport-x-01774", object, loc.dot(Field::x),
                         "(%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (%f).", viewport.x,
                         device_limits.viewportBoundsRange[0]);
    }

    // x + width
    if (x_healthy && width_healthy) {
        const float right_bound = viewport.x + viewport.width;
        if (right_bound > device_limits.viewportBoundsRange[1]) {
            skip |= LogError("VUID-VkViewport-x-01232", object, loc,
                             "x (%f) + width (%f) is %f which is greater than VkPhysicalDeviceLimits::viewportBoundsRange[1] (%f).",
                             viewport.x, viewport.width, right_bound, device_limits.viewportBoundsRange[1]);
        }
    }

    // y
    bool y_healthy = true;
    if (!(viewport.y >= device_limits.viewportBoundsRange[0])) {
        y_healthy = false;
        skip |= LogError("VUID-VkViewport-y-01775", object, loc.dot(Field::y),
                         "(%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (%f).", viewport.y,
                         device_limits.viewportBoundsRange[0]);
    } else if (negative_height_enabled && viewport.y > device_limits.viewportBoundsRange[1]) {
        y_healthy = false;
        skip |= LogError("VUID-VkViewport-y-01776", object, loc.dot(Field::y),
                         "(%f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (%f).", viewport.y,
                         device_limits.viewportBoundsRange[1]);
    }

    // y + height
    if (y_healthy && height_healthy) {
        const float boundary = viewport.y + viewport.height;

        if (boundary > device_limits.viewportBoundsRange[1]) {
            skip |= LogError("VUID-VkViewport-y-01233", object, loc.dot(Field::y),
                             "(%f) + height (%f) is %f which exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (%f).",
                             viewport.y, viewport.height, boundary, device_limits.viewportBoundsRange[1]);
        } else if (negative_height_enabled && boundary < device_limits.viewportBoundsRange[0]) {
            skip |= LogError("VUID-VkViewport-y-01777", object, loc.dot(Field::y),
                             "(%f) + height (%f) is %f which is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (%f).",
                             viewport.y, viewport.height, boundary, device_limits.viewportBoundsRange[0]);
        }
    }

    if (!IsExtEnabled(extensions.vk_ext_depth_range_unrestricted)) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            skip |= LogError("VUID-VkViewport-minDepth-01234", object, loc.dot(Field::minDepth), "is %f.", viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            skip |= LogError("VUID-VkViewport-maxDepth-01235", object, loc.dot(Field::maxDepth), "is %f.", viewport.maxDepth);
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // ValidateArray()
    skip |= context.ValidateArray(error_obj.location.dot(Field::commandBufferCount), error_obj.location.dot(Field::pCommandBuffers),
                                  commandBufferCount, &pCommandBuffers, true, true, kVUIDUndefined,
                                  "VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    return skip;
}

bool Device::manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                      const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    // pBeginInfo->pInheritanceInfo can be a non-null invalid pointer. If not secondary command buffer we need to ignore
    if (!error_obj.handle_data->command_buffer.is_secondary) {
        return skip;
    }

    if (pBeginInfo->pInheritanceInfo) {
        const VkCommandBufferInheritanceInfo &info = *pBeginInfo->pInheritanceInfo;
        const Location begin_info_loc = error_obj.location.dot(Field::pBeginInfo);
        const Location inheritance_loc = begin_info_loc.dot(Field::pInheritanceInfo);

        skip |= ValidateCommandBufferInheritanceInfo(context, info, inheritance_loc);

        // Explicit VUs
        if (!enabled_features.inheritedQueries && info.occlusionQueryEnable == VK_TRUE) {
            skip |= LogError(
                "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056", commandBuffer, error_obj.location,
                "inheritedQueries feature is disabled, but pBeginInfo->pInheritanceInfo->occlusionQueryEnable is VK_TRUE.");
        }

        if (enabled_features.inheritedQueries) {
            skip |= context.ValidateFlags(inheritance_loc.dot(Field::queryFlags), vvl::FlagBitmask::VkQueryControlFlagBits,
                                          AllVkQueryControlFlagBits, info.queryFlags, kOptionalFlags,
                                          "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
        } else {  // !inheritedQueries
            skip |= context.ValidateReservedFlags(inheritance_loc.dot(Field::queryFlags), info.queryFlags,
                                                  "VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
        }

        if (enabled_features.pipelineStatisticsQuery) {
            skip |= context.ValidateFlags(inheritance_loc.dot(Field::pipelineStatistics),
                                          vvl::FlagBitmask::VkQueryPipelineStatisticFlagBits, AllVkQueryPipelineStatisticFlagBits,
                                          info.pipelineStatistics, kOptionalFlags,
                                          "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-02789");
        } else {  // !pipelineStatisticsQuery
            skip |= context.ValidateReservedFlags(inheritance_loc.dot(Field::pipelineStatistics), info.pipelineStatistics,
                                                  "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-00058");
        }

        const auto *conditional_rendering =
            vku::FindStructInPNextChain<VkCommandBufferInheritanceConditionalRenderingInfoEXT>(info.pNext);
        if (conditional_rendering) {
            if (!enabled_features.inheritedConditionalRendering && conditional_rendering->conditionalRenderingEnable == VK_TRUE) {
                skip |= LogError(
                    "VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977", commandBuffer,
                    error_obj.location,
                    "Inherited conditional rendering is disabled, but "
                    "pBeginInfo->pInheritanceInfo->pNext<VkCommandBufferInheritanceConditionalRenderingInfoEXT> is VK_TRUE.");
            }
        }

        auto p_inherited_viewport_scissor_info =
            vku::FindStructInPNextChain<VkCommandBufferInheritanceViewportScissorInfoNV>(info.pNext);
        if (p_inherited_viewport_scissor_info != nullptr && !enabled_features.multiViewport &&
            p_inherited_viewport_scissor_info->viewportScissor2D == VK_TRUE &&
            p_inherited_viewport_scissor_info->viewportDepthCount != 1) {
            skip |= LogError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04783", commandBuffer,
                             error_obj.location,
                             "multiViewport feature was not enabled, but "
                             "VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D in "
                             "pBeginInfo->pInheritanceInfo->pNext is VK_TRUE and viewportDepthCount is not 1.");
        }
    }
    return skip;
}

static size_t ComponentTypeBytesPerElement(VkComponentTypeKHR component_type) {
    switch (component_type) {
        case VK_COMPONENT_TYPE_SINT8_KHR:
        case VK_COMPONENT_TYPE_UINT8_KHR:
        case VK_COMPONENT_TYPE_FLOAT_E4M3_NV:
        case VK_COMPONENT_TYPE_FLOAT_E5M2_NV:
        case VK_COMPONENT_TYPE_SINT8_PACKED_NV:
        case VK_COMPONENT_TYPE_UINT8_PACKED_NV:
            return 1;
        case VK_COMPONENT_TYPE_FLOAT16_KHR:
        case VK_COMPONENT_TYPE_SINT16_KHR:
        case VK_COMPONENT_TYPE_UINT16_KHR:
            return 2;
        case VK_COMPONENT_TYPE_FLOAT32_KHR:
        case VK_COMPONENT_TYPE_SINT32_KHR:
        case VK_COMPONENT_TYPE_UINT32_KHR:
            return 4;
        case VK_COMPONENT_TYPE_FLOAT64_KHR:
        case VK_COMPONENT_TYPE_SINT64_KHR:
        case VK_COMPONENT_TYPE_UINT64_KHR:
            return 8;
        default:
            return 0;
    }
}

static bool IsFloatComponentType(VkComponentTypeKHR component_type) {
    switch (component_type) {
        case VK_COMPONENT_TYPE_FLOAT_E4M3_NV:
        case VK_COMPONENT_TYPE_FLOAT_E5M2_NV:
        case VK_COMPONENT_TYPE_FLOAT16_KHR:
        case VK_COMPONENT_TYPE_FLOAT32_KHR:
        case VK_COMPONENT_TYPE_FLOAT64_KHR:
            return true;
        default:
            return false;
    }
}

bool Device::ValidateVkConvertCooperativeVectorMatrixInfoNV(const LogObjectList &objlist,
                                                            const VkConvertCooperativeVectorMatrixInfoNV &info,
                                                            const Location &info_loc) const {
    bool skip = false;

    size_t src_element_size = ComponentTypeBytesPerElement(info.srcComponentType);
    size_t dst_element_size = ComponentTypeBytesPerElement(info.dstComponentType);

    if (info.srcLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV) {
        if (info.srcStride < info.numColumns * src_element_size) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcLayout-10077", objlist, info_loc.dot(Field::srcStride),
                             "(%zu) must be at least as large as numColumns (%d) times source element size (%zu)", info.srcStride,
                             info.numColumns, src_element_size);
        }
    }
    if (info.srcLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV) {
        if (info.srcStride < info.numRows * src_element_size) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcLayout-10077", objlist, info_loc.dot(Field::srcStride),
                             "(%zu) must be at least as large as numRows (%d) times source element size (%zu)", info.srcStride,
                             info.numRows, src_element_size);
        }
    }
    if (info.srcLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV ||
        info.srcLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV) {
        if ((info.srcStride % src_element_size) != 0) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcLayout-10077", objlist, info_loc.dot(Field::srcStride),
                             "(%zu) must be a multiple of source element size (%zu)", info.srcStride, src_element_size);
        }
    }

    if (info.dstLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV) {
        if (info.dstStride < info.numColumns * dst_element_size) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstLayout-10078", objlist, info_loc.dot(Field::dstStride),
                             "(%zu) must be at least as large as numColumns (%d) times destination element size (%zu)",
                             info.dstStride, info.numColumns, dst_element_size);
        }
    }
    if (info.dstLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV) {
        if (info.dstStride < info.numRows * dst_element_size) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstLayout-10078", objlist, info_loc.dot(Field::dstStride),
                             "(%zu) must be at least as large as numRows (%d) times destination element size (%zu)", info.dstStride,
                             info.numRows, dst_element_size);
        }
    }
    if (info.dstLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV ||
        info.dstLayout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV) {
        if ((info.dstStride % dst_element_size) != 0) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstLayout-10078", objlist, info_loc.dot(Field::dstStride),
                             "(%zu) must be a multiple of destination element size (%zu)", info.dstStride, dst_element_size);
        }
    }

    if (info.srcComponentType != info.dstComponentType) {
        bool ok =
            IsFloatComponentType(info.srcComponentType) && IsFloatComponentType(info.dstComponentType) &&
            (info.srcComponentType == VK_COMPONENT_TYPE_FLOAT16_KHR || info.srcComponentType == VK_COMPONENT_TYPE_FLOAT32_KHR ||
             info.dstComponentType == VK_COMPONENT_TYPE_FLOAT16_KHR || info.dstComponentType == VK_COMPONENT_TYPE_FLOAT32_KHR);
        if (!ok) {
            skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcComponentType-10081", objlist, info_loc,
                             "Unsupported conversion from %s to %s", string_VkComponentTypeKHR(info.srcComponentType),
                             string_VkComponentTypeKHR(info.dstComponentType));
        }
    }
    if ((info.dstComponentType == VK_COMPONENT_TYPE_FLOAT_E4M3_NV || info.dstComponentType == VK_COMPONENT_TYPE_FLOAT_E5M2_NV) &&
        info.dstLayout != VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV &&
        info.dstLayout != VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV) {
        skip |=
            LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstComponentType-10082", objlist,
                     info_loc.dot(Field::srcComponentType), "%s cannot be converted to destination layout %s",
                     string_VkComponentTypeKHR(info.srcComponentType), string_VkCooperativeVectorMatrixLayoutNV(info.dstLayout));
    }

    return skip;
}

static size_t ComputeMinSize(VkComponentTypeKHR component_type, VkCooperativeVectorMatrixLayoutNV layout, uint32_t num_rows,
                             uint32_t num_columns, size_t stride) {
    size_t min_size = 0;
    size_t element_size = ComponentTypeBytesPerElement(component_type);
    if (layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV) {
        min_size = (num_rows - 1) * stride + num_columns * element_size;
    } else if (layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV) {
        min_size = (num_columns - 1) * stride + num_rows * element_size;
    }
    return min_size;
}

bool Device::manual_PreCallValidateConvertCooperativeVectorMatrixNV(VkDevice device,
                                                                    const VkConvertCooperativeVectorMatrixInfoNV *pInfo,
                                                                    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location info_loc = error_obj.location.dot(Field::pInfo);

    if (pInfo->srcData.hostAddress == nullptr && pInfo->dstData.hostAddress != nullptr) {
        skip |= LogError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10073", device,
                         info_loc.dot(Field::dstData).dot(Field::hostAddress), "(%p) must be null", pInfo->dstData.hostAddress);
    }

    if (pInfo->srcData.hostAddress != nullptr) {
        size_t min_src_size =
            ComputeMinSize(pInfo->srcComponentType, pInfo->srcLayout, pInfo->numRows, pInfo->numColumns, pInfo->srcStride);
        if (pInfo->srcSize < min_src_size) {
            skip |= LogError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10074", device, info_loc.dot(Field::srcSize),
                             "(%zu) less than minimum size for row/col-major layout (%zu)", pInfo->srcSize, min_src_size);
        }
    }

    if (pInfo->dstData.hostAddress != nullptr) {
        size_t min_dst_size =
            ComputeMinSize(pInfo->dstComponentType, pInfo->dstLayout, pInfo->numRows, pInfo->numColumns, pInfo->dstStride);
        if (*pInfo->pDstSize < min_dst_size) {
            skip |= LogError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10075", device, info_loc.dot(Field::pDstSize),
                             "(%zu) less than minimum size for row/col-major layout (%zu)", *pInfo->pDstSize, min_dst_size);
        }
    }

    if (pInfo->dstData.hostAddress != nullptr) {
        sparse_container::range<size_t> src_range((uintptr_t)pInfo->srcData.hostAddress,
                                                  (uintptr_t)pInfo->srcData.hostAddress + pInfo->srcSize);
        sparse_container::range<size_t> dst_range((uintptr_t)pInfo->dstData.hostAddress,
                                                  (uintptr_t)pInfo->dstData.hostAddress + *pInfo->pDstSize);
        if (src_range.intersects(dst_range)) {
            skip |= LogError("VUID-vkConvertCooperativeVectorMatrixNV-pInfo-10076", device, info_loc,
                             "Source [%zx,%zx) and destination [%zx,%zx) ranges overlap", src_range.begin, src_range.end,
                             dst_range.begin, dst_range.end);
        }
    }

    skip |= ValidateVkConvertCooperativeVectorMatrixInfoNV(device, *pInfo, info_loc);

    return skip;
}

bool Device::manual_PreCallValidateCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                       const VkConvertCooperativeVectorMatrixInfoNV *pInfos,
                                                                       const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    std::vector<sparse_container::range<VkDeviceAddress>> src_memory_ranges;
    std::vector<sparse_container::range<VkDeviceAddress>> dst_memory_ranges;

    for (uint32_t i = 0; i < infoCount; ++i) {
        auto const &info = pInfos[i];

        const Location info_loc = error_obj.location.dot(Field::pInfos, i);

        if ((info.srcData.deviceAddress & 0x3F) != 0) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10084", commandBuffer,
                             info_loc.dot(Field::srcData).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be 64 byte aligned",
                             info.srcData.deviceAddress);
        }
        if ((info.dstData.deviceAddress & 0x3F) != 0) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10085", commandBuffer,
                             info_loc.dot(Field::dstData).dot(Field::deviceAddress), "(0x%" PRIx64 ") must be 64 byte aligned",
                             info.dstData.deviceAddress);
        }

        size_t min_src_size = ComputeMinSize(info.srcComponentType, info.srcLayout, info.numRows, info.numColumns, info.srcStride);
        if (info.srcSize < min_src_size) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10086", device, info_loc.dot(Field::srcSize),
                             "(%zu) less than minimum size for row/col-major layout (%zu)", info.srcSize, min_src_size);
        }

        size_t min_dst_size = ComputeMinSize(info.dstComponentType, info.dstLayout, info.numRows, info.numColumns, info.dstStride);
        if (*info.pDstSize < min_dst_size) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10087", device, info_loc.dot(Field::pDstSize),
                             "(%zu) less than minimum size for row/col-major layout (%zu)", *info.pDstSize, min_dst_size);
        }

        src_memory_ranges.emplace_back(info.srcData.deviceAddress, info.srcData.deviceAddress + info.srcSize);
        dst_memory_ranges.emplace_back(info.dstData.deviceAddress, info.dstData.deviceAddress + *info.pDstSize);

        skip |= ValidateVkConvertCooperativeVectorMatrixInfoNV(commandBuffer, info, info_loc);
    }

    std::sort(src_memory_ranges.begin(), src_memory_ranges.end());
    std::sort(dst_memory_ranges.begin(), dst_memory_ranges.end());

    // Memory ranges are sorted, so looking for overlaps can be done in linear time
    auto src_ranges_it = src_memory_ranges.cbegin();
    auto dst_ranges_it = dst_memory_ranges.cbegin();

    while (src_ranges_it != src_memory_ranges.cend() && dst_ranges_it != dst_memory_ranges.cend()) {
        if (src_ranges_it->intersects(*dst_ranges_it)) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-None-10088", commandBuffer, error_obj.location,
                             "Source [0x%" PRIx64 ", 0x%" PRIx64 ") and destination [0x%" PRIx64 ", 0x%" PRIx64 ") ranges overlap",
                             src_ranges_it->begin, src_ranges_it->end, dst_ranges_it->begin, dst_ranges_it->end);
        }

        if (*src_ranges_it < *dst_ranges_it)
            ++src_ranges_it;
        else
            ++dst_ranges_it;
    }

    return skip;
}

}  // namespace stateless
