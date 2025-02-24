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
#include "generated/dispatch_functions.h"

namespace stateless {

bool Device::manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport *pViewports, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.multiViewport) {
        if (viewportCount != 1) {
            skip |= LogError("VUID-vkCmdSetViewportWithCount-viewportCount-03395", commandBuffer,
                             error_obj.location.dot(Field::viewportCount),
                             "(%" PRIu32 ") is not 1, but the multiViewport feature is not enabled.", viewportCount);
        }
    } else {  // multiViewport enabled
        if (viewportCount < 1 || viewportCount > device_limits.maxViewports) {
            skip |= LogError("VUID-vkCmdSetViewportWithCount-viewportCount-03394", commandBuffer,
                             error_obj.location.dot(Field::viewportCount),
                             "(%" PRIu32
                             ") must "
                             "not be greater than VkPhysicalDeviceLimits::maxViewports (%" PRIu32 ").",
                             viewportCount, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            skip |= ValidateViewport(viewport, commandBuffer, error_obj.location.dot(Field::pViewports, viewport_i));
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D *pScissors, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.multiViewport) {
        if (scissorCount != 1) {
            skip |= LogError("VUID-vkCmdSetScissorWithCount-scissorCount-03398", commandBuffer,
                             error_obj.location.dot(Field::scissorCount),
                             "(%" PRIu32
                             ") must "
                             "be 1 when the multiViewport feature is disabled.",
                             scissorCount);
        }
    } else {  // multiViewport enabled
        if (scissorCount == 0) {
            skip |= LogError("VUID-vkCmdSetScissorWithCount-scissorCount-03397", commandBuffer,
                             error_obj.location.dot(Field::scissorCount),
                             "(%" PRIu32
                             ") must "
                             "be great than zero.",
                             scissorCount);
        } else if (scissorCount > device_limits.maxViewports) {
            skip |= LogError("VUID-vkCmdSetScissorWithCount-scissorCount-03397", commandBuffer,
                             error_obj.location.dot(Field::scissorCount),
                             "(%" PRIu32
                             ") must "
                             "not be greater than maxViewports (%" PRIu32 ").",
                             scissorCount, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const Location scissor_loc = error_obj.location.dot(Field::pScissors, scissor_i);
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError("VUID-vkCmdSetScissorWithCount-x-03399", commandBuffer,
                                 scissor_loc.dot(Field::offset).dot(Field::x), "(%" PRId32 ") is negative.", scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError("VUID-vkCmdSetScissorWithCount-x-03399", commandBuffer,
                                 scissor_loc.dot(Field::offset).dot(Field::y), "(%" PRId32 ") is negative.", scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError("VUID-vkCmdSetScissorWithCount-offset-03400", commandBuffer, scissor_loc,
                                 "offset.x (%" PRId32 ") + extent.width (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError("VUID-vkCmdSetScissorWithCount-offset-03401", commandBuffer, scissor_loc,
                                 "offset.y (%" PRId32 ") + extent.height (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum);
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                        const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                                                        uint32_t vertexAttributeDescriptionCount,
                                                        const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions,
                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791", commandBuffer,
                         error_obj.location.dot(Field::vertexBindingDescriptionCount),
                         "(%" PRIu32 ") is greater than the maxVertexInputBindings limit (%" PRIu32 ").",
                         vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
    }

    if (vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
        skip |= LogError("VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792", commandBuffer,
                         error_obj.location.dot(Field::vertexAttributeDescriptionCount),
                         "(%" PRIu32 ") is greater than the maxVertexInputAttributes limit (%" PRIu32 ").",
                         vertexAttributeDescriptionCount, device_limits.maxVertexInputAttributes);
    }

    for (uint32_t attribute = 0; attribute < vertexAttributeDescriptionCount; ++attribute) {
        bool binding_found = false;
        for (uint32_t binding = 0; binding < vertexBindingDescriptionCount; ++binding) {
            if (pVertexAttributeDescriptions[attribute].binding == pVertexBindingDescriptions[binding].binding) {
                binding_found = true;
                break;
            }
        }
        if (!binding_found) {
            skip |= LogError("VUID-vkCmdSetVertexInputEXT-binding-04793", commandBuffer,
                             error_obj.location.dot(Field::pVertexAttributeDescriptions, attribute),
                             "references an unspecified binding.");
        }
    }

    // check for distinct values
    {
        vvl::unordered_set<uint32_t> vertex_bindings(vertexBindingDescriptionCount);
        for (uint32_t i = 0; i < vertexBindingDescriptionCount; ++i) {
            const uint32_t binding = pVertexBindingDescriptions[i].binding;
            auto const &binding_it = vertex_bindings.find(binding);
            if (binding_it != vertex_bindings.cend()) {
                skip |= LogError("VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794", commandBuffer,
                                 error_obj.location.dot(Field::pVertexBindingDescriptions, binding),
                                 "is already in pVertexBindingDescriptions[%" PRIu32 "].", *binding_it);
                break;
            }
            vertex_bindings.insert(binding);
        }

        vvl::unordered_set<uint32_t> vertex_locations(vertexAttributeDescriptionCount);
        for (uint32_t i = 0; i < vertexAttributeDescriptionCount; ++i) {
            const uint32_t location = pVertexAttributeDescriptions[i].location;
            auto const &location_it = vertex_locations.find(location);
            if (location_it != vertex_locations.cend()) {
                skip |= LogError("VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795", commandBuffer,
                                 error_obj.location.dot(Field::pVertexAttributeDescriptions, location),
                                 "is already in pVertexAttributeDescriptions[%" PRIu32 "].", *location_it);
                break;
            }
            vertex_locations.insert(location);
        }
    }

    for (uint32_t binding = 0; binding < vertexBindingDescriptionCount; ++binding) {
        const Location binding_loc = error_obj.location.dot(Field::pVertexBindingDescriptions, binding);
        if (pVertexBindingDescriptions[binding].binding > device_limits.maxVertexInputBindings) {
            skip |= LogError("VUID-VkVertexInputBindingDescription2EXT-binding-04796", commandBuffer,
                             binding_loc.dot(Field::binding), "(%" PRIu32 ") is greater than maxVertexInputBindings (%" PRIu32 ").",
                             pVertexBindingDescriptions[binding].binding, device_limits.maxVertexInputBindings);
        }

        if (pVertexBindingDescriptions[binding].stride > device_limits.maxVertexInputBindingStride) {
            skip |= LogError("VUID-VkVertexInputBindingDescription2EXT-stride-04797", commandBuffer, binding_loc.dot(Field::stride),
                             "(%" PRIu32 ") is greater than maxVertexInputBindingStride (%" PRIu32 ").",
                             pVertexBindingDescriptions[binding].stride, device_limits.maxVertexInputBindingStride);
        }

        if (pVertexBindingDescriptions[binding].divisor == 0 && (!enabled_features.vertexAttributeInstanceRateZeroDivisor)) {
            skip |=
                LogError("VUID-VkVertexInputBindingDescription2EXT-divisor-04798", commandBuffer, binding_loc.dot(Field::divisor),
                         "is zero but "
                         "vertexAttributeInstanceRateZeroDivisor feature was not enabled");
        }

        if (pVertexBindingDescriptions[binding].divisor > 1) {
            if (!enabled_features.vertexAttributeInstanceRateDivisor) {
                skip |= LogError("VUID-VkVertexInputBindingDescription2EXT-divisor-04799", commandBuffer,
                                 binding_loc.dot(Field::divisor),
                                 "is %" PRIu32
                                 " (greater than 1) but "
                                 "vertexAttributeInstanceRateDivisor feature was not enabled",
                                 pVertexBindingDescriptions[binding].divisor);
            } else {
                if (pVertexBindingDescriptions[binding].divisor > phys_dev_props_core14.maxVertexAttribDivisor) {
                    skip |= LogError("VUID-VkVertexInputBindingDescription2EXT-divisor-06226", commandBuffer,
                                     binding_loc.dot(Field::divisor),
                                     "(%" PRIu32 ") is greater than maxVertexAttribDivisor (%" PRIu32 ")",
                                     pVertexBindingDescriptions[binding].divisor, phys_dev_props_core14.maxVertexAttribDivisor);
                }

                if (pVertexBindingDescriptions[binding].inputRate != VK_VERTEX_INPUT_RATE_INSTANCE) {
                    skip |= LogError("VUID-VkVertexInputBindingDescription2EXT-divisor-06227", commandBuffer,
                                     binding_loc.dot(Field::divisor), "is %" PRIu32 " (greater than 1) but inputRate is %s.",
                                     pVertexBindingDescriptions[binding].divisor,
                                     string_VkVertexInputRate(pVertexBindingDescriptions[binding].inputRate));
                }
            }
        }
    }

    for (uint32_t attribute = 0; attribute < vertexAttributeDescriptionCount; ++attribute) {
        const Location attribute_loc = error_obj.location.dot(Field::pVertexAttributeDescriptions, attribute);
        if (pVertexAttributeDescriptions[attribute].location > device_limits.maxVertexInputAttributes) {
            skip |= LogError("VUID-VkVertexInputAttributeDescription2EXT-location-06228", commandBuffer,
                             attribute_loc.dot(Field::location),
                             "(%" PRIu32 ") is greater than maxVertexInputAttributes (%" PRIu32 ").",
                             pVertexAttributeDescriptions[attribute].location, device_limits.maxVertexInputAttributes);
        }

        if (pVertexAttributeDescriptions[attribute].binding > device_limits.maxVertexInputBindings) {
            skip |=
                LogError("VUID-VkVertexInputAttributeDescription2EXT-binding-06229", commandBuffer,
                         attribute_loc.dot(Field::binding), "(%" PRIu32 ") is greater than maxVertexInputBindings (%" PRIu32 ").",
                         pVertexAttributeDescriptions[attribute].binding, device_limits.maxVertexInputBindings);
        }

        if (pVertexAttributeDescriptions[attribute].offset > device_limits.maxVertexInputAttributeOffset) {
            skip |=
                LogError("VUID-VkVertexInputAttributeDescription2EXT-offset-06230", commandBuffer, attribute_loc.dot(Field::offset),
                         "(%" PRIu32 ") is greater than maxVertexInputAttributeOffset (%" PRIu32 ").",
                         pVertexAttributeDescriptions[attribute].offset, device_limits.maxVertexInputAttributeOffset);
        }

        VkFormatProperties properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, pVertexAttributeDescriptions[attribute].format, &properties);
        if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
            skip |=
                LogError("VUID-VkVertexInputAttributeDescription2EXT-format-04805", commandBuffer, attribute_loc.dot(Field::format),
                         "(%s) doesn't support VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT.\n"
                         "(supported bufferFeatures: %s)",
                         string_VkFormat(pVertexAttributeDescriptions[attribute].format),
                         string_VkFormatFeatureFlags2(properties.bufferFeatures).c_str());
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                             uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!pDiscardRectangles) {
        return skip;
    }
    for (uint32_t i = 0; i < discardRectangleCount; ++i) {
        const Location loc = error_obj.location.dot(Field::pDiscardRectangles, i);
        const int64_t x_sum =
            static_cast<int64_t>(pDiscardRectangles[i].offset.x) + static_cast<int64_t>(pDiscardRectangles[i].extent.width);
        if (x_sum > std::numeric_limits<int32_t>::max()) {
            skip |= LogError("VUID-vkCmdSetDiscardRectangleEXT-offset-00588", commandBuffer, loc,
                             "offset.x (%" PRId32 ") + extent.width (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                             pDiscardRectangles[i].offset.x, pDiscardRectangles[i].extent.width, x_sum);
        }

        const int64_t y_sum =
            static_cast<int64_t>(pDiscardRectangles[i].offset.y) + static_cast<int64_t>(pDiscardRectangles[i].extent.height);
        if (y_sum > std::numeric_limits<int32_t>::max()) {
            skip |= LogError("VUID-vkCmdSetDiscardRectangleEXT-offset-00589", commandBuffer, loc,
                             "offset.y (%" PRId32 ") + extent.height (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                             pDiscardRectangles[i].offset.y, pDiscardRectangles[i].extent.height, y_sum);
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (discard_rectangles_extension_version < 2) {
        skip |= LogError("VUID-vkCmdSetDiscardRectangleEnableEXT-specVersion-07851", commandBuffer, error_obj.location,
                         "Requires support for version 2 of VK_EXT_discard_rectangles.");
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                                 VkDiscardRectangleModeEXT discardRectangleMode,
                                                                 const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (discard_rectangles_extension_version < 2) {
        skip |= LogError("VUID-vkCmdSetDiscardRectangleModeEXT-specVersion-07852", commandBuffer, error_obj.location,
                         "Requires support for version 2 of VK_EXT_discard_rectangles.");
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                  uint32_t exclusiveScissorCount,
                                                                  const VkBool32 *pExclusiveScissorEnables,
                                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (scissor_exclusive_extension_version < 2) {
        skip |= LogError("VUID-vkCmdSetExclusiveScissorEnableNV-exclusiveScissor-07853", commandBuffer, error_obj.location,
                         "Requires support for version 2 of VK_NV_scissor_exclusive.");
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                            uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors,
                                                            const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.multiViewport) {
        if (firstExclusiveScissor != 0) {
            skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02035", commandBuffer,
                             error_obj.location.dot(Field::firstExclusiveScissor),
                             "is %" PRIu32 " but the multiViewport feature is not enabled.", firstExclusiveScissor);
        }
        if (exclusiveScissorCount > 1) {
            skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-exclusiveScissorCount-02036", commandBuffer,
                             error_obj.location.dot(Field::exclusiveScissorCount),
                             "is %" PRIu32 " but the multiViewport feature is not enabled.", exclusiveScissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstExclusiveScissor) + static_cast<uint64_t>(exclusiveScissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-firstExclusiveScissor-02034", commandBuffer, error_obj.location,
                             "firstExclusiveScissor (%" PRIu32 ") + exclusiveScissorCount (%" PRIu32 ") is %" PRIu64
                             " which is greater than VkPhysicalDeviceLimits::maxViewports (%" PRIu32 ").",
                             firstExclusiveScissor, exclusiveScissorCount, sum, device_limits.maxViewports);
        }
    }

    if (pExclusiveScissors) {
        for (uint32_t scissor_i = 0; scissor_i < exclusiveScissorCount; ++scissor_i) {
            const Location scissor_loc = error_obj.location.dot(Field::pExclusiveScissors, scissor_i);
            const auto &scissor = pExclusiveScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-x-02037", commandBuffer,
                                 scissor_loc.dot(Field::offset).dot(Field::x), "(%" PRId32 ") is negative.", scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-x-02037", commandBuffer,
                                 scissor_loc.dot(Field::offset).dot(Field::y), "(%" PRId32 ") is negative.", scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-offset-02038", commandBuffer, scissor_loc,
                                 "offset.x (%" PRId32 ") + extent.width (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-offset-02039", commandBuffer, scissor_loc,
                                 "offset.y (%" PRId32 ") + extent.height (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum);
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                            uint32_t viewportCount, const VkViewportWScalingNV *pViewportWScalings,
                                                            const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if ((sum < 1) || (sum > device_limits.maxViewports)) {
        skip |= LogError("VUID-vkCmdSetViewportWScalingNV-firstViewport-01324", commandBuffer, error_obj.location,
                         "firstViewport (%" PRIu32 ") + viewportCount (%" PRIu32 ") is %" PRIu64
                         ", which must be between 1 and VkPhysicalDeviceLimits::maxViewports (%" PRIu32 "), inculsive.",
                         firstViewport, viewportCount, sum, device_limits.maxViewports);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                      uint32_t viewportCount,
                                                                      const VkShadingRatePaletteNV *pShadingRatePalettes,
                                                                      const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.multiViewport) {
        if (firstViewport != 0) {
            skip |= LogError("VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068", commandBuffer,
                             error_obj.location.dot(Field::firstViewport),
                             "is %" PRIu32 " but the multiViewport feature was not enabled.", firstViewport);
        }
        if (viewportCount > 1) {
            skip |= LogError("VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069", commandBuffer,
                             error_obj.location.dot(Field::viewportCount),
                             "is %" PRIu32 " but the multiViewport feature was not enabled.", viewportCount);
        }
    }

    const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
    if (sum > device_limits.maxViewports) {
        skip |= LogError("VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067", commandBuffer, error_obj.location,
                         "firstViewport (%" PRIu32 ") + viewportCount (%" PRIu32 ") is %" PRIu64
                         ") which is greater than VkPhysicalDeviceLimits::maxViewports (%" PRIu32 ").",
                         firstViewport, viewportCount, sum, device_limits.maxViewports);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                                                             VkCoarseSampleOrderTypeNV sampleOrderType,
                                                             uint32_t customSampleOrderCount,
                                                             const VkCoarseSampleOrderCustomNV *pCustomSampleOrders,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV && customSampleOrderCount != 0) {
        skip |= LogError("VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081", commandBuffer, error_obj.location,
                         "If sampleOrderType is not VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, "
                         "customSampleOrderCount must be 0.");
    }

    for (uint32_t order_i = 0; order_i < customSampleOrderCount; ++order_i) {
        skip |= ValidateCoarseSampleOrderCustomNV(pCustomSampleOrders[order_i],
                                                  error_obj.location.dot(Field::pCustomSampleOrders, order_i));
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                  const VkViewport *pViewports, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.multiViewport) {
        if (firstViewport != 0) {
            skip |=
                LogError("VUID-vkCmdSetViewport-firstViewport-01224", commandBuffer, error_obj.location.dot(Field::firstViewport),
                         "is %" PRIu32 " but the multiViewport feature was not enabled.", firstViewport);
        }
        if (viewportCount > 1) {
            skip |=
                LogError("VUID-vkCmdSetViewport-viewportCount-01225", commandBuffer, error_obj.location.dot(Field::viewportCount),
                         "is %" PRIu32 " but the multiViewport feature was not enabled.", viewportCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstViewport) + static_cast<uint64_t>(viewportCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError("VUID-vkCmdSetViewport-firstViewport-01223", commandBuffer, error_obj.location,
                             "firstViewport (%" PRIu32 ") + viewportCount (%" PRIu32 ") is %" PRIu64
                             " which is greater than maxViewports (%" PRIu32 ").",
                             firstViewport, viewportCount, sum, device_limits.maxViewports);
        }
    }

    if (pViewports) {
        for (uint32_t viewport_i = 0; viewport_i < viewportCount; ++viewport_i) {
            const auto &viewport = pViewports[viewport_i];  // will crash on invalid ptr
            skip |= ValidateViewport(viewport, commandBuffer, error_obj.location.dot(Field::pViewports, viewport_i));
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                            const VkDepthClampRangeEXT *pDepthClampRange,
                                                            const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (depthClampMode == VK_DEPTH_CLAMP_MODE_USER_DEFINED_RANGE_EXT) {
        if (!pDepthClampRange) {
            skip |= LogError("VUID-vkCmdSetDepthClampRangeEXT-pDepthClampRange-09647", device,
                             error_obj.location.dot(Field::pDepthClampRange), "is NULL.");
        } else {
            skip |= ValidateDepthClampRange(*pDepthClampRange, error_obj.location.dot(Field::pDepthClampRange));
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                                 const VkRect2D *pScissors, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.multiViewport) {
        if (firstScissor != 0) {
            skip |= LogError("VUID-vkCmdSetScissor-firstScissor-00593", commandBuffer, error_obj.location.dot(Field::firstScissor),
                             "is %" PRIu32 " but the multiViewport feature was not enabled.", firstScissor);
        }
        if (scissorCount > 1) {
            skip |= LogError("VUID-vkCmdSetScissor-scissorCount-00594", commandBuffer, error_obj.location.dot(Field::scissorCount),
                             "is %" PRIu32 " but the multiViewport feature was not enabled.", scissorCount);
        }
    } else {  // multiViewport enabled
        const uint64_t sum = static_cast<uint64_t>(firstScissor) + static_cast<uint64_t>(scissorCount);
        if (sum > device_limits.maxViewports) {
            skip |= LogError("VUID-vkCmdSetScissor-firstScissor-00592", commandBuffer, error_obj.location,
                             "firstScissor (%" PRIu32 ") + scissorCount (%" PRIu32 ") is %" PRIu64
                             " which is greater than maxViewports (%" PRIu32 ").",
                             firstScissor, scissorCount, sum, device_limits.maxViewports);
        }
    }

    if (pScissors) {
        for (uint32_t scissor_i = 0; scissor_i < scissorCount; ++scissor_i) {
            const Location scissor_loc = error_obj.location.dot(Field::pScissors, scissor_i);
            const auto &scissor = pScissors[scissor_i];  // will crash on invalid ptr

            if (scissor.offset.x < 0) {
                skip |= LogError("VUID-vkCmdSetScissor-x-00595", commandBuffer, scissor_loc.dot(Field::offset).dot(Field::x),
                                 "(%" PRId32 ") is negative.", scissor.offset.x);
            }

            if (scissor.offset.y < 0) {
                skip |= LogError("VUID-vkCmdSetScissor-x-00595", commandBuffer, scissor_loc.dot(Field::offset).dot(Field::y),
                                 "(%" PRId32 ") is negative.", scissor.offset.y);
            }

            const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
            if (x_sum > vvl::kI32Max) {
                skip |= LogError("VUID-vkCmdSetScissor-offset-00596", commandBuffer, scissor_loc,
                                 "offset.x (%" PRId32 ") + extent.width (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                                 scissor.offset.x, scissor.extent.width, x_sum);
            }

            const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
            if (y_sum > vvl::kI32Max) {
                skip |= LogError("VUID-vkCmdSetScissor-offset-00597", commandBuffer, scissor_loc,
                                 "offset.y (%" PRId32 ") + extent.height (%" PRIu32 ") is %" PRIi64 " which will overflow int32_t.",
                                 scissor.offset.y, scissor.extent.height, y_sum);
            }
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.wideLines && (lineWidth != 1.0f)) {
        skip |= LogError("VUID-vkCmdSetLineWidth-lineWidth-00788", commandBuffer, error_obj.location.dot(Field::lineWidth),
                         "is %f (not 1.0), but wideLines was not enabled.", lineWidth);
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (lineStippleFactor < 1 || lineStippleFactor > 256) {
        skip |= LogError("VUID-vkCmdSetLineStipple-lineStippleFactor-02776", commandBuffer,
                         error_obj.location.dot(Field::lineStippleFactor), "%" PRIu32 " is not in [1,256].", lineStippleFactor);
    }

    return skip;
}
}  // namespace stateless
