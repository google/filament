/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <sstream>
#include <valarray>

#include "containers/custom_containers.h"
#include "core_validation.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/sampler_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/shader_module.h"
#include "cc_buffer_address.h"
#include "drawdispatch/descriptor_validator.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "utils/vk_layer_utils.h"
#include "utils/vk_struct_compare.h"
#include "error_message/error_strings.h"

using DescriptorSetLayoutDef = vvl::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = vvl::DescriptorSetLayoutId;

bool CoreChecks::ImmutableSamplersAreEqual(const VkDescriptorSetLayoutBinding &b1, const VkDescriptorSetLayoutBinding &b2,
                                           bool &out_exception) const {
    if (b1.pImmutableSamplers == b2.pImmutableSamplers) {
        return true;
    } else if (b1.pImmutableSamplers && b2.pImmutableSamplers) {
        if ((b1.descriptorType == b2.descriptorType) &&
            ((b1.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
             (b1.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) &&
            (b1.descriptorCount == b2.descriptorCount)) {
            out_exception = true;  // If here, we might have two VkSampler that are defined the same
            for (uint32_t i = 0; i < b1.descriptorCount; ++i) {
                if (b1.pImmutableSamplers[i] == b2.pImmutableSamplers[i]) {
                    continue;  // both null or same pointer
                }
                auto sampler_state_1 = Get<vvl::Sampler>(b1.pImmutableSamplers[i]);
                auto sampler_state_2 = Get<vvl::Sampler>(b2.pImmutableSamplers[i]);
                if (!sampler_state_1 || !sampler_state_2) {
                    if (!CompareSamplerCreateInfo(sampler_state_1->create_info, sampler_state_2->create_info)) {
                        return false;
                    }
                }
            }
            return true;
        } else {
            return false;
        }
    } else {
        // One pointer is null, the other is not
        return false;
    }
}

// If our layout is compatible with bound_dsl, return true,
//  else return false and fill in error_msg will description of what causes incompatibility
bool CoreChecks::VerifySetLayoutCompatibility(const vvl::DescriptorSetLayout &layout_dsl, const vvl::DescriptorSetLayout &bound_dsl,
                                              std::string &error_msg) const {
    // Short circuit the detailed check.
    if (layout_dsl.IsCompatible(&bound_dsl)) return true;

    // Do a detailed compatibility check of this lhs def (referenced by layout_dsl), vs. the rhs (layout and def)
    // Should only be run if trivial accept has failed, and in that context should return false.
    VkDescriptorSetLayout layout_dsl_handle = layout_dsl.VkHandle();
    VkDescriptorSetLayout bound_dsl_handle = bound_dsl.VkHandle();
    DescriptorSetLayoutDef const *layout_ds_layout_def = layout_dsl.GetLayoutDef();
    DescriptorSetLayoutDef const *bound_ds_layout_def = bound_dsl.GetLayoutDef();

    // Check descriptor counts
    const auto bound_total_count = bound_ds_layout_def->GetTotalDescriptorCount();
    if (layout_ds_layout_def->GetTotalDescriptorCount() != bound_ds_layout_def->GetTotalDescriptorCount()) {
        std::stringstream error_str;
        error_str << FormatHandle(layout_dsl_handle) << " from pipeline layout has "
                  << layout_ds_layout_def->GetTotalDescriptorCount() << " total descriptors, but " << FormatHandle(bound_dsl_handle)
                  << ", which is bound, has " << bound_total_count << " total descriptors.";
        error_msg = error_str.str();
        return false;  // trivial fail case
    }

    bool exception = false;
    // Descriptor counts match so need to go through bindings one-by-one
    //  and verify that type and stageFlags match
    for (const auto &layout_binding : layout_ds_layout_def->GetBindings()) {
        const auto bound_binding = bound_ds_layout_def->GetBindingInfoFromBinding(layout_binding.binding);
        if (layout_binding.descriptorCount != bound_binding->descriptorCount) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << FormatHandle(layout_dsl_handle)
                      << " from pipeline layout has a descriptorCount of " << layout_binding.descriptorCount << " but binding "
                      << layout_binding.binding << " for " << FormatHandle(bound_dsl_handle)
                      << ", which is bound, has a descriptorCount of " << bound_binding->descriptorCount;
            error_msg = error_str.str();
            return false;
        } else if (layout_binding.descriptorType != bound_binding->descriptorType) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << FormatHandle(layout_dsl_handle)
                      << " from pipeline layout is type " << string_VkDescriptorType(layout_binding.descriptorType)
                      << " but binding " << layout_binding.binding << " for " << FormatHandle(bound_dsl_handle)
                      << ", which is bound, is type " << string_VkDescriptorType(bound_binding->descriptorType) << "";
            error_msg = error_str.str();
            return false;
        } else if (layout_binding.stageFlags != bound_binding->stageFlags) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << FormatHandle(layout_dsl_handle)
                      << " from pipeline layout has stageFlags " << string_VkShaderStageFlags(layout_binding.stageFlags)
                      << " but binding " << layout_binding.binding << " for " << FormatHandle(bound_dsl_handle)
                      << ", which is bound, has stageFlags " << string_VkShaderStageFlags(bound_binding->stageFlags);
            error_msg = error_str.str();
            return false;
        } else if (!ImmutableSamplersAreEqual(*layout_binding.ptr(), *bound_binding, exception)) {
            error_msg = "Immutable samplers from binding " + std::to_string(layout_binding.binding) + " in pipeline layout " +
                        FormatHandle(layout_dsl_handle) + " do not match the immutable samplers in the layout currently bound (" +
                        FormatHandle(bound_dsl_handle) + ")";
            return false;
        }
    }

    const auto &ds_layout_flags = layout_ds_layout_def->GetBindingFlags();
    const auto &bound_layout_flags = bound_ds_layout_def->GetBindingFlags();
    if (bound_layout_flags != ds_layout_flags) {
        std::stringstream error_str;
        assert(ds_layout_flags.size() == bound_layout_flags.size());
        size_t i;
        for (i = 0; i < ds_layout_flags.size(); i++) {
            if (ds_layout_flags[i] != bound_layout_flags[i]) break;
        }
        error_str << FormatHandle(layout_dsl_handle) << " from pipeline layout does not have the same binding flags at binding "
                  << i << " ( " << string_VkDescriptorBindingFlags(ds_layout_flags[i]) << " ) as " << FormatHandle(bound_dsl_handle)
                  << " ( " << string_VkDescriptorBindingFlags(bound_layout_flags[i]) << " ), which is bound";
        error_msg = error_str.str();
        return false;
    }

    // If we got here, we failed IsCompatible() but didn't find what was different, likely missing a case
    //
    // There are exceptions where this is valid, example is the pImmutableSamplers pointing to 2 different handles are not hashed.
    // It would be ugly to pass in the state object when hashing with hash_utils, so we just defer until here.
    assert(exception);
    return true;
}

// For given vvl::DescriptorSet, verify that its Set is compatible w/ the setLayout corresponding to
// pipelineLayout[layoutIndex]
bool CoreChecks::VerifySetLayoutCompatibility(const vvl::DescriptorSet &descriptor_set,
                                              const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts,
                                              const VulkanTypedHandle &handle, const uint32_t layoutIndex,
                                              std::string &error_msg) const {
    size_t num_sets = set_layouts.size();
    if (layoutIndex >= num_sets) {
        std::stringstream error_str;
        error_str << FormatHandle(handle) << ") only contains ";
        if (num_sets == 1) {
            error_str << "1 setLayout, corresponding to index 0";
        } else {
            error_str << num_sets << " setLayouts, corresponding to index from 0 to " << num_sets - 1;
        }
        error_str << ", but you're attempting to bind set to index " << layoutIndex;
        error_msg = error_str.str();
        return false;
    }
    if (descriptor_set.IsPushDescriptor()) return true;
    const auto *layout_node = set_layouts[layoutIndex].get();
    if (layout_node) {
        return VerifySetLayoutCompatibility(*layout_node, *descriptor_set.GetLayout(), error_msg);
    } else {
        // It's possible the DSL is null when creating a graphics pipeline library, in which case we can't verify compatibility
        // here.
        return true;
    }
}

bool CoreChecks::VerifySetLayoutCompatibility(const vvl::PipelineLayout &layout_a, const vvl::PipelineLayout &layout_b,
                                              std::string &error_msg) const {
    const uint32_t num_sets = static_cast<uint32_t>(std::min(layout_a.set_layouts.size(), layout_b.set_layouts.size()));
    for (uint32_t i = 0; i < num_sets; ++i) {
        const auto ds_a = layout_a.set_layouts[i];
        const auto ds_b = layout_b.set_layouts[i];
        if (ds_a && ds_b) {
            if (!VerifySetLayoutCompatibility(*ds_a, *ds_b, error_msg)) {
                return false;
            }
        }
    }
    return true;
}

bool CoreChecks::VerifySetLayoutCompatibilityUnion(const vvl::PipelineLayout &layout, const vvl::PipelineLayout &pre_raster_layout,
                                                   const vvl::PipelineLayout &fs_layout, std::string &error_msg) const {
    // When dealing with Graphics Pipeline Library, we need to get the union of pipeline states.
    // Currently this just means the VkDescriptorSetLayout may be VK_NULL_HANDLE.
    uint32_t num_sets = static_cast<uint32_t>(std::min(pre_raster_layout.set_layouts.size(), fs_layout.set_layouts.size()));
    num_sets = std::min(static_cast<uint32_t>(layout.set_layouts.size()), num_sets);
    for (uint32_t i = 0; i < num_sets; ++i) {
        const auto ds_a = layout.set_layouts[i];
        // If Pre-Rasterization is not null, should be good to use
        auto ds_b = pre_raster_layout.set_layouts[i];
        if (!ds_b) {
            ds_b = fs_layout.set_layouts[i];
        }
        if (ds_a && ds_b) {
            if (!VerifySetLayoutCompatibility(*ds_a, *ds_b, error_msg)) {
                return false;
            }
        }
    }
    return true;
}

bool CoreChecks::ValidateCmdBindDescriptorSets(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout, uint32_t firstSet,
                                               uint32_t setCount, const VkDescriptorSet *pDescriptorSets,
                                               uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets,
                                               const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdBindDescriptorSets;

    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) return skip;  // dynamicPipelineLayout feature

    // Track total count of dynamic descriptor types to make sure we have an offset for each one
    uint32_t total_dynamic_descriptors = 0;

    for (uint32_t set_idx = 0; set_idx < setCount; set_idx++) {
        const Location set_loc = loc.dot(Field::pDescriptorSets, set_idx);
        if (auto descriptor_set = Get<vvl::DescriptorSet>(pDescriptorSets[set_idx])) {
            // Verify that set being bound is compatible with overlapping setLayout of pipelineLayout
            std::string error_string = "";
            if (!VerifySetLayoutCompatibility(*descriptor_set, pipeline_layout->set_layouts, pipeline_layout->Handle(),
                                              set_idx + firstSet, error_string)) {
                const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx]);
                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDescriptorSets-00358"
                                        : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358";
                skip |= LogError(vuid, objlist, set_loc,
                                 "(%s) being bound is not compatible with overlapping "
                                 "descriptorSetLayout at index %" PRIu32
                                 " of "
                                 "%s due to: %s.",
                                 FormatHandle(pDescriptorSets[set_idx]).c_str(), set_idx + firstSet, FormatHandle(layout).c_str(),
                                 error_string.c_str());
            }

            const auto &dsl = descriptor_set->GetLayout();
            if (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
                const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx], dsl->VkHandle());
                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDescriptorSets-08010"
                                        : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-08010";
                skip |= LogError(vuid, objlist, set_loc, "was allocated from VkDescriptorSetLayout with %s flags.",
                                 string_VkDescriptorSetLayoutCreateFlags(dsl->GetCreateFlags()).c_str());
            }

            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            if (set_dynamic_descriptor_count) {
                // First make sure we won't overstep bounds of pDynamicOffsets array
                if ((total_dynamic_descriptors + set_dynamic_descriptor_count) > dynamicOffsetCount) {
                    // Test/report this here, such that we don't run past the end of pDynamicOffsets in the else clause
                    const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx]);
                    const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-dynamicOffsetCount-00359"
                                            : "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359";
                    skip |=
                        LogError(vuid, objlist, set_loc,
                                 "(%s) requires %" PRIu32 " dynamicOffsets, but only %" PRIu32
                                 " "
                                 "dynamicOffsets are left in "
                                 "pDynamicOffsets array. There must be one dynamic offset for each dynamic descriptor being bound.",
                                 FormatHandle(pDescriptorSets[set_idx]).c_str(), descriptor_set->GetDynamicDescriptorCount(),
                                 (dynamicOffsetCount - total_dynamic_descriptors));
                    // Set the number found to the maximum to prevent duplicate messages, or subsquent descriptor sets from
                    // testing against the "short tail" we're skipping below.
                    total_dynamic_descriptors = dynamicOffsetCount;
                } else {  // Validate dynamic offsets and Dynamic Offset Minimums
                    // offset for all sets (pDynamicOffsets)
                    uint32_t cur_dyn_offset = total_dynamic_descriptors;
                    // offset into this descriptor set
                    uint32_t set_dyn_offset = 0;
                    const auto binding_count = dsl->GetBindingCount();
                    const auto &limits = phys_dev_props.limits;
                    for (uint32_t i = 0; i < binding_count; i++) {
                        const auto *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(i);
                        // skip checking binding if not needed
                        if (vvl::IsDynamicDescriptor(binding->descriptorType) == false) {
                            continue;
                        }

                        // If a descriptor set has only binding 0 and 2 the binding_index will be 0 and 2
                        const uint32_t binding_index = binding->binding;
                        const uint32_t descriptorCount = binding->descriptorCount;

                        // Need to loop through each descriptor count inside the binding
                        // if descriptorCount is zero the binding with a dynamic descriptor type does not count
                        for (uint32_t j = 0; j < descriptorCount; j++) {
                            const uint32_t offset = pDynamicOffsets[cur_dyn_offset];
                            if (offset == 0) {
                                // offset of zero is equivalent of not having the dynamic offset
                                cur_dyn_offset++;
                                set_dyn_offset++;
                                continue;
                            }

                            // Validate alignment with limit
                            if ((binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) &&
                                (SafeModulo(offset, limits.minUniformBufferOffsetAlignment) != 0)) {
                                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDynamicOffsets-01971"
                                                        : "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01971";
                                skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                 "is %" PRIu32
                                                 ", but must be a multiple of "
                                                 "device limit minUniformBufferOffsetAlignment %" PRIu64 ".",
                                                 offset, limits.minUniformBufferOffsetAlignment);
                            }
                            if ((binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
                                (SafeModulo(offset, limits.minStorageBufferOffsetAlignment) != 0)) {
                                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDynamicOffsets-01972"
                                                        : "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01972";
                                skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                 "is %" PRIu32
                                                 ", but must be a multiple of "
                                                 "device limit minStorageBufferOffsetAlignment %" PRIu64 ".",
                                                 offset, limits.minStorageBufferOffsetAlignment);
                            }

                            auto *descriptor = descriptor_set->GetDescriptorFromDynamicOffsetIndex(set_dyn_offset);
                            ASSERT_AND_CONTINUE(descriptor);
                            // Currently only GeneralBuffer are dynamic and need to be checked
                            if (descriptor->GetClass() == vvl::DescriptorClass::GeneralBuffer) {
                                const auto *buffer_descriptor = static_cast<const vvl::BufferDescriptor *>(descriptor);
                                const VkDeviceSize bound_range = buffer_descriptor->GetRange();
                                const VkDeviceSize bound_offset = buffer_descriptor->GetOffset();
                                // NOTE: null / invalid buffers may show up here, errors are raised elsewhere for this.
                                auto buffer_state = buffer_descriptor->GetBufferState();

                                // Validate offset didn't go over buffer
                                if ((bound_range == VK_WHOLE_SIZE) && (offset > 0)) {
                                    const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx],
                                                                buffer_descriptor->GetBuffer());
                                    const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDescriptorSets-06715"
                                                            : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06715";
                                    skip |= LogError(vuid, objlist, loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                     "is %" PRIu32
                                                     ", but must be zero since "
                                                     "the buffer descriptor's range is VK_WHOLE_SIZE in descriptorSet #%" PRIu32
                                                     " binding #%" PRIu32
                                                     " "
                                                     "descriptor[%" PRIu32 "].",
                                                     offset, set_idx, binding_index, j);

                                } else if (buffer_state && (bound_range != VK_WHOLE_SIZE) &&
                                           ((offset + bound_range + bound_offset) > buffer_state->create_info.size)) {
                                    const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx],
                                                                buffer_descriptor->GetBuffer());
                                    const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDescriptorSets-01979"
                                                            : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979";
                                    skip |=
                                        LogError(vuid, objlist, loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                 "is %" PRIu32 ", which when added to the buffer descriptor's range (%" PRIu64
                                                 ") and offset (%" PRIu64 ") is greater than the size of the buffer (%" PRIu64
                                                 ") in descriptorSet #%" PRIu32 " binding #%" PRIu32 " descriptor[%" PRIu32 "].",
                                                 offset, bound_range, bound_offset, buffer_state->create_info.size, set_idx,
                                                 binding_index, j);
                                }
                            }
                            cur_dyn_offset++;
                            set_dyn_offset++;
                        }  // descriptorCount loop
                    }  // bindingCount loop
                    // Keep running total of dynamic descriptor count to verify at the end
                    total_dynamic_descriptors += set_dynamic_descriptor_count;
                }
            }
            auto ds_pool_state = descriptor_set->GetPoolState();
            if (ds_pool_state && ds_pool_state->create_info.flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT) {
                const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx], ds_pool_state->Handle());
                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-pDescriptorSets-04616"
                                        : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-04616";
                skip |= LogError(vuid, objlist, set_loc,
                                 "was allocated from a pool that was created with VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT.");
            }
        } else if (!enabled_features.graphicsPipelineLibrary) {
            const LogObjectList objlist(cb_state.Handle(), pDescriptorSets[set_idx]);
            const char *vuid =
                is_2 ? "VUID-VkBindDescriptorSetsInfo-pDescriptorSets-06563" : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06563";
            skip |= LogError(vuid, objlist, set_loc,
                             "(%s) does not exist, and the pipeline layout was not created "
                             "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT.",
                             FormatHandle(pDescriptorSets[set_idx]).c_str());
        }
    }
    //  dynamicOffsetCount must equal the total number of dynamic descriptors in the sets being bound
    if (total_dynamic_descriptors != dynamicOffsetCount) {
        const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-dynamicOffsetCount-00359"
                                : "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359";
        skip |= LogError(vuid, cb_state.Handle(), loc,
                         "Attempting to bind %" PRIu32 " descriptorSets with %" PRIu32
                         " dynamic descriptors, but "
                         "dynamicOffsetCount is %" PRIu32
                         ". It should "
                         "exactly match the number of dynamic descriptors.",
                         setCount, total_dynamic_descriptors, dynamicOffsetCount);
    }
    // firstSet and descriptorSetCount sum must be less than setLayoutCount
    if ((firstSet + setCount) > static_cast<uint32_t>(pipeline_layout->set_layouts.size())) {
        const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfo-firstSet-00360" : "VUID-vkCmdBindDescriptorSets-firstSet-00360";
        skip |= LogError(vuid, cb_state.Handle(), loc,
                         "Sum of firstSet (%" PRIu32 ") and descriptorSetCount (%" PRIu32
                         ") is greater than "
                         "VkPipelineLayoutCreateInfo::setLayoutCount "
                         "(%zu) when pipeline layout was created",
                         firstSet, setCount, pipeline_layout->set_layouts.size());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                      const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                      const uint32_t *pDynamicOffsets, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateCmdBindDescriptorSets(*cb_state, layout, firstSet, setCount, pDescriptorSets, dynamicOffsetCount,
                                          pDynamicOffsets, error_obj.location);
    skip |= ValidatePipelineBindPoint(*cb_state, pipelineBindPoint, error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                       const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo,
                                                       const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateCmdBindDescriptorSets(*cb_state, pBindDescriptorSetsInfo->layout, pBindDescriptorSetsInfo->firstSet,
                                          pBindDescriptorSetsInfo->descriptorSetCount, pBindDescriptorSetsInfo->pDescriptorSets,
                                          pBindDescriptorSetsInfo->dynamicOffsetCount, pBindDescriptorSetsInfo->pDynamicOffsets,
                                          error_obj.location.dot(Field::pBindDescriptorSetsInfo));

    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                          const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo, error_obj);
}

bool CoreChecks::ValidateDescriptorSetLayoutBindingFlags(const VkDescriptorSetLayoutCreateInfo &create_info, uint32_t max_binding,
                                                         uint32_t *update_after_bind, const Location &create_info_loc) const {
    bool skip = false;
    const auto *flags_info = vku::FindStructInPNextChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(create_info.pNext);
    if (!flags_info) {
        return skip;
    }
    if (flags_info->bindingCount != 0 && flags_info->bindingCount != create_info.bindingCount) {
        skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-bindingCount-03002", device,
                         create_info_loc.pNext(Struct::VkDescriptorSetLayoutBindingFlagsCreateInfo, Field::bindingCount),
                         "(%" PRIu32 ") is different from pCreateInfo->bindingCount (%" PRIu32 ").", flags_info->bindingCount,
                         create_info.bindingCount);
    }

    if (flags_info->bindingCount != create_info.bindingCount) {
        return skip;  // nothing left to validate
    }
    for (uint32_t i = 0; i < create_info.bindingCount; ++i) {
        const auto &binding_info = create_info.pBindings[i];
        const Location binding_flags_loc =
            create_info_loc.pNext(Struct::VkDescriptorSetLayoutBindingFlagsCreateInfo, Field::pBindingFlags, i);

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) {
            *update_after_bind = i;
            if ((create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) == 0) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-03000", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pCreateInfo->flags is %s.",
                                 string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
            }

            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                !enabled_features.descriptorBindingUniformBufferUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformBufferUpdateAfterBind-03005",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                             "but descriptorBindingUniformBufferUpdateAfterBind was not enabled.",
                             i);
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) &&
                !enabled_features.descriptorBindingSampledImageUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingSampledImageUpdateAfterBind-03006",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is %s "
                             "but descriptorBindingSampledImageUpdateAfterBind was not enabled.",
                             i, string_VkDescriptorType(binding_info.descriptorType));
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
                !enabled_features.descriptorBindingStorageImageUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageImageUpdateAfterBind-03007",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE "
                             "but descriptorBindingStorageImageUpdateAfterBind was not enabled.",
                             i);
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
                !enabled_features.descriptorBindingStorageBufferUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageBufferUpdateAfterBind-03008",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER "
                             "but descriptorBindingStorageBufferUpdateAfterBind was not enabled.",
                             i);
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER &&
                !enabled_features.descriptorBindingUniformTexelBufferUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformTexelBufferUpdateAfterBind-03009",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER "
                    "but descriptorBindingUniformTexelBufferUpdateAfterBind was not enabled.",
                    i);
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER &&
                !enabled_features.descriptorBindingStorageTexelBufferUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageTexelBufferUpdateAfterBind-03010",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER "
                    "but descriptorBindingStorageTexelBufferUpdateAfterBind was not enabled.",
                    i);
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-None-03011", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }

            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK &&
                !enabled_features.descriptorBindingInlineUniformBlockUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingInlineUniformBlockUpdateAfterBind-02211",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK "
                    "but descriptorBindingInlineUniformBlockUpdateAfterBind was not enabled.",
                    i);
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) &&
                !enabled_features.descriptorBindingAccelerationStructureUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is %s, but the descriptorBindingAccelerationStructureUpdateAfterBind was not enabled.",
                    i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT) {
            if (!enabled_features.descriptorBindingUpdateUnusedWhilePending) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUpdateUnusedWhilePending-03012",
                                 device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s, but the descriptorBindingUpdateUnusedWhilePending was not enabled.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) {
            if (!enabled_features.descriptorBindingPartiallyBound) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingPartiallyBound-03013", device,
                                 binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s, but the descriptorBindingPartiallyBound was not enabled.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            if (binding_info.binding != max_binding) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03004", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT "
                                 "but %" PRIu32 " is the largest value of all the bindings.",
                                 binding_info.binding);
            }

            if (!enabled_features.descriptorBindingVariableDescriptorCount) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingVariableDescriptorCount-03014",
                                 device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, but the "
                                 "descriptorBindingVariableDescriptorCount feature was not enabled.");
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03015", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        const bool push_descriptor_set = (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT) != 0;
        if (push_descriptor_set && (flags_info->pBindingFlags[i] & (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                                                    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                                                                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT))) {
            skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-flags-03003", device, binding_flags_loc,
                             "is %s (which includes CREATE_PUSH_DESCRIPTOR_BIT).",
                             string_VkDescriptorBindingFlags(flags_info->pBindingFlags[i]).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                       const Location &create_info_loc) const {
    bool skip = false;
    vvl::unordered_set<uint32_t> bindings;
    uint64_t total_descriptors = 0;

    const bool push_descriptor_set = (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT) != 0;

    uint32_t max_binding = 0;

    uint32_t update_after_bind = create_info.bindingCount;
    uint32_t uniform_buffer_dynamic = create_info.bindingCount;
    uint32_t storage_buffer_dynamic = create_info.bindingCount;

    for (uint32_t i = 0; i < create_info.bindingCount; ++i) {
        const Location binding_loc = create_info_loc.dot(Field::pBindings, i);
        const auto &binding_info = create_info.pBindings[i];
        max_binding = std::max(max_binding, binding_info.binding);

        if (!bindings.insert(binding_info.binding).second) {
            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-binding-00279", device, binding_loc.dot(Field::binding),
                             "is duplicated at pBindings[%" PRIu32 "].binding.", binding_info.binding);
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            if (!enabled_features.inlineUniformBlock) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-04604", device,
                                 binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but the inlineUniformBlock feature was not enabled.");
            } else if (push_descriptor_set) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-02208", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK but "
                                 "pCreateInfo->flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT.");
            } else {
                if ((binding_info.descriptorCount % 4) != 0) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-02209", device,
                                     binding_loc.dot(Field::descriptorCount), "(%" PRIu32 ") (must be a multiple of 4).",
                                     binding_info.descriptorCount);
                }
                if ((binding_info.descriptorCount > phys_dev_props_core13.maxInlineUniformBlockSize) &&
                    !(create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-08004", device,
                                     binding_loc.dot(Field::descriptorCount),
                                     "(%" PRIu32 ") but must be less than or equal to maxInlineUniformBlockSize (%" PRIu32
                                     "), but "
                                     "pCreateInfo->flags is %s.",
                                     binding_info.descriptorCount, phys_dev_props_core13.maxInlineUniformBlockSize,
                                     string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
                }
            }
        } else if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            uniform_buffer_dynamic = i;
            if (push_descriptor_set) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, but pCreateInfo->flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT.");
            }
        } else if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            storage_buffer_dynamic = i;
            if (push_descriptor_set) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, but pCreateInfo->flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT.");
            }
        }

        if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
             binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
            binding_info.pImmutableSamplers) {
            for (uint32_t j = 0; j < binding_info.descriptorCount; j++) {
                auto sampler_state = Get<vvl::Sampler>(binding_info.pImmutableSamplers[j]);
                if (sampler_state && (sampler_state->create_info.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
                                      sampler_state->create_info.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT)) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-pImmutableSamplers-04009", device,
                                     binding_loc.dot(Field::pImmutableSamplers, j),
                                     "(%s) presented as immutable has a custom border color.",
                                     FormatHandle(binding_info.pImmutableSamplers[j]).c_str());
                }
            }
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT && binding_info.pImmutableSamplers != nullptr) {
            skip |=
                LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-04605", device, binding_loc.dot(Field::descriptorType),
                         "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but pImmutableSamplers is not NULL.");
        }

        if (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) {
            if (binding_info.descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBinding-flags-08005", device, binding_loc.dot(Field::descriptorType),
                    "is %s but pCreateInfo->flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT.",
                    string_VkDescriptorType(binding_info.descriptorType));
            }

            if (binding_info.descriptorCount > 1) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBinding-flags-08006", device, binding_loc.dot(Field::descriptorCount),
                    "is %" PRIu32
                    " but pCreateInfo->flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT.",
                    binding_info.descriptorCount);
            }

            if ((binding_info.descriptorCount == 1) && (binding_info.pImmutableSamplers == nullptr)) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBinding-flags-08007", device, binding_loc.dot(Field::descriptorCount),
                                 "is 1 and pImmutableSamplers is NULL, but pCreateInfo->flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT.");
            }
        }

        total_descriptors += binding_info.descriptorCount;
    }

    skip |= ValidateDescriptorSetLayoutBindingFlags(create_info, max_binding, &update_after_bind, create_info_loc);

    if (update_after_bind < create_info.bindingCount) {
        if (uniform_buffer_dynamic < create_info.bindingCount) {
            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001", device,
                             create_info_loc.dot(Field::pBindings, update_after_bind),
                             "has VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                             "flag, but pBindings[%" PRIu32 "] has descriptor type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.",
                             uniform_buffer_dynamic);
        }
        if (storage_buffer_dynamic < create_info.bindingCount) {
            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001", device,
                             create_info_loc.dot(Field::pBindings, update_after_bind),
                             "has VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                             "flag, but pBindings[%" PRIu32 "] has descriptor type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.",
                             storage_buffer_dynamic);
        }
    }

    if ((push_descriptor_set) && (total_descriptors > phys_dev_props_core14.maxPushDescriptors)) {
        skip |= LogError(
            "VUID-VkDescriptorSetLayoutCreateInfo-flags-00281", device, create_info_loc.dot(Field::flags),
            "contains VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT, but the total descriptor count in layout (%" PRIu64
            ") must not be greater than maxPushDescriptors (%" PRIu32 ").",
            total_descriptors, phys_dev_props_core14.maxPushDescriptors);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDescriptorSetLayout *pSetLayout, const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDescriptorSetLayoutCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                              VkDescriptorSetLayoutSupport *pSupport,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDescriptorSetLayoutCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                 const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                 VkDescriptorSetLayoutSupport *pSupport,
                                                                 const ErrorObject &error_obj) const {
    return PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport, error_obj);
}

// Validate that the state of this set is appropriate for the given bindings and dynamic_offsets at Draw time
//  This includes validating that all descriptors in the given bindings are updated,
//  that any update buffers are valid, and that any dynamic offsets are within the bounds of their buffers.
// Return true if state is acceptable, or false and write an error message into error string
bool CoreChecks::ValidateDrawState(const vvl::DescriptorSet &descriptor_set, uint32_t set_index,
                                   const BindingVariableMap &binding_req_map, const vvl::CommandBuffer &cb_state,
                                   const Location &loc, const vvl::DrawDispatchVuid &vuids) const {
    bool result = false;
    const VkFramebuffer framebuffer = cb_state.activeFramebuffer ? cb_state.activeFramebuffer->VkHandle() : VK_NULL_HANDLE;
    // NOTE: GPU-AV needs non-const state objects to do lazy updates of descriptor state of only the dynamically used
    // descriptors, via the non-const version of ValidateBindingDynamic(), this code uses the const path only even it gives up
    // non-const versions of its state objects here.
    const vvl::DescriptorValidator desc_val(const_cast<CoreChecks &>(*this), const_cast<vvl::CommandBuffer &>(cb_state),
                                            const_cast<vvl::DescriptorSet &>(descriptor_set), set_index, framebuffer, loc);

    for (const auto &[binding_index, desc_set_reqs] : binding_req_map) {
        ASSERT_AND_CONTINUE(desc_set_reqs.variable);
        const auto &resource_variable = *desc_set_reqs.variable;

        const vvl::DescriptorBinding *binding = descriptor_set.GetBinding(binding_index);
        if (!binding) {  //  End at construction is the condition for an invalid binding.
            auto set = descriptor_set.Handle();
            result |= LogError(vuids.descriptor_buffer_bit_set_08114, set, loc, "%s %s is invalid.", FormatHandle(set).c_str(),
                               resource_variable.DescribeDescriptor().c_str());
            return result;
        }

        if (descriptor_set.ValidateBindingOnGPU(*binding, resource_variable.is_runtime_descriptor_array)) {
            continue;
        }

        result |= desc_val.ValidateBindingStatic(resource_variable, *binding);
    }
    return result;
}

// Starting at offset descriptor of given binding, parse over update_count
//  descriptor updates and verify that for any binding boundaries that are crossed, the next binding(s) are all consistent
//  Consistency means that their type, stage flags, and whether or not they use immutable samplers matches
bool CoreChecks::VerifyUpdateConsistency(const vvl::DescriptorSet &set, uint32_t binding, uint32_t offset, uint32_t update_count,
                                         bool is_copy, const Location &set_loc) const {
    bool skip = false;
    auto current_iter = set.FindBinding(binding);
    // Verify consecutive bindings match (if needed)
    auto &orig_binding = **current_iter;
    while (!skip && update_count) {
        // First, it's legal to offset beyond your own binding so handle that case
        if (offset > 0) {
            // index_range.start + offset is which descriptor is needed to update. If it > index_range.end, it means the descriptor
            // isn't in this binding, maybe in next binding.
            if (offset > (*current_iter)->count) {
                // Advance to next binding, decrement offset by binding size
                offset -= (*current_iter)->count;
                ++current_iter;
                // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
                if (current_iter == set.end() || !orig_binding.IsConsistent(**current_iter)) {
                    skip = true;
                }
                continue;
            }
        }

        update_count -= std::min(update_count, (*current_iter)->count - offset);
        if (update_count) {
            // Starting offset is beyond the current binding. Check consistency, update counters and advance to the next binding,
            // looking for the start point. All bindings (even those skipped) must be consistent with the update and with the
            // original binding.
            offset = 0;
            ++current_iter;
            // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
            if (current_iter == set.end() || !orig_binding.IsConsistent(**current_iter)) {
                skip = true;
            }
        }
    }

    if (skip) {
        std::stringstream error_str;
        if (set.IsPushDescriptor()) {
            error_str << "(push descriptors)";
        } else {
            error_str << FormatHandle(set);
        }
        error_str << " binding #" << orig_binding.binding << " with #" << update_count
                  << " descriptors being updated but this update oversteps the bounds of this binding and the next binding is "
                     "not consistent with current binding";
        if (current_iter == set.end()) {
            error_str << " (update past the end of the descriptor set)";
        } else {
            auto current_binding = current_iter->get();
            // Get what was not consistent in IsConsistent() as a more detailed error message
            if (current_binding->type != orig_binding.type) {
                error_str << " (" << string_VkDescriptorType(current_binding->type)
                          << " != " << string_VkDescriptorType(orig_binding.type) << ")";
            } else if (current_binding->stage_flags != orig_binding.stage_flags) {
                error_str << " (" << string_VkShaderStageFlags(current_binding->stage_flags)
                          << " != " << string_VkShaderStageFlags(orig_binding.stage_flags) << ")";
            } else if (current_binding->has_immutable_samplers != orig_binding.has_immutable_samplers) {
                error_str << " (pImmutableSamplers don't match)";
            } else if (current_binding->binding_flags != orig_binding.binding_flags) {
                error_str << " (" << string_VkDescriptorBindingFlags(current_binding->binding_flags)
                          << " != " << string_VkDescriptorBindingFlags(orig_binding.binding_flags) << ")";
            }
        }

        error_str << " so this update is invalid";
        const char *vuid = is_copy ? "VUID-VkCopyDescriptorSet-srcSet-00349" : "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
        skip |= LogError(vuid, set.Handle(), set_loc, "%s", error_str.str().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateCopyUpdate(const VkCopyDescriptorSet &update, const Location &copy_loc) const {
    bool skip = false;
    const auto src_set = Get<vvl::DescriptorSet>(update.srcSet);
    const auto dst_set = Get<vvl::DescriptorSet>(update.dstSet);
    ASSERT_AND_RETURN_SKIP(src_set && dst_set);

    const auto *src_layout = src_set->GetLayout().get();
    uint32_t src_start_idx = 0;
    {
        if (src_layout->Destroyed()) {
            const LogObjectList objlist(update.srcSet, src_layout->Handle());
            return LogError("VUID-VkCopyDescriptorSet-srcSet-parameter", objlist, copy_loc.dot(Field::srcSet),
                            "(%s) has been destroyed.", FormatHandle(src_layout->Handle()).c_str());
        }
        if (!src_set->HasBinding(update.srcBinding)) {
            const LogObjectList objlist(update.srcSet, src_layout->Handle());
            return LogError("VUID-VkCopyDescriptorSet-srcBinding-00345", objlist, copy_loc.dot(Field::srcBinding),
                            "(%" PRIu32 ") does not exist in %s.", update.srcBinding, FormatHandle(src_set->Handle()).c_str());
        }

        src_start_idx = src_set->GetGlobalIndexRangeFromBinding(update.srcBinding).start + update.srcArrayElement;
        if ((src_start_idx + update.descriptorCount) > src_set->GetTotalDescriptorCount()) {
            const LogObjectList objlist(update.srcSet, src_layout->Handle());
            skip |= LogError(
                "VUID-VkCopyDescriptorSet-srcArrayElement-00346", objlist, copy_loc.dot(Field::srcArrayElement),
                "(%" PRIu32 ") + descriptorCount (%" PRIu32 ") + offset index (%" PRIu32
                ") is larger than the total descriptors count (%" PRIu32 ") for the binding at srcBinding (%" PRIu32 ").",
                update.srcArrayElement, update.descriptorCount, src_set->GetGlobalIndexRangeFromBinding(update.srcBinding).start,
                src_set->GetTotalDescriptorCount(), update.srcBinding);
        }
    }

    const auto *dst_layout = dst_set->GetLayout().get();
    uint32_t dst_start_idx = 0;
    {
        if (dst_layout->Destroyed()) {
            const LogObjectList objlist(update.dstSet, dst_layout->Handle());
            return LogError("VUID-VkCopyDescriptorSet-dstSet-parameter", objlist, copy_loc.dot(Field::dstSet),
                            "(%s) has been destroyed.", FormatHandle(dst_layout->Handle()).c_str());
        }
        if (!dst_layout->HasBinding(update.dstBinding)) {
            const LogObjectList objlist(update.dstSet, dst_layout->Handle());
            return LogError("VUID-VkCopyDescriptorSet-dstBinding-00347", objlist, copy_loc.dot(Field::dstBinding),
                            "(%" PRIu32 ") does not exist in %s.", update.dstBinding, FormatHandle(dst_set->Handle()).c_str());
        }

        dst_start_idx = dst_layout->GetGlobalIndexRangeFromBinding(update.dstBinding).start + update.dstArrayElement;
        if ((dst_start_idx + update.descriptorCount) > dst_layout->GetTotalDescriptorCount()) {
            const LogObjectList objlist(update.dstSet, dst_layout->Handle());
            skip |= LogError(
                "VUID-VkCopyDescriptorSet-dstArrayElement-00348", objlist, copy_loc.dot(Field::dstArrayElement),
                "(%" PRIu32 ") + descriptorCount (%" PRIu32 ") + offset index (%" PRIu32
                ") is larger than the total descriptors count (%" PRIu32 ") for the binding at dstBinding (%" PRIu32 ").",
                update.dstArrayElement, update.descriptorCount, dst_set->GetGlobalIndexRangeFromBinding(update.dstBinding).start,
                dst_set->GetTotalDescriptorCount(), update.dstBinding);
        }
    }

    skip |= ValidateCopyUpdateDescriptorSetLayoutFlags(update, *src_layout, *dst_layout, copy_loc);
    skip |= ValidateCopyUpdateDescriptorPoolFlags(update, *src_set, *dst_set, copy_loc);
    skip |= ValidateCopyUpdateDescriptorTypes(update, *src_set, *dst_set, *src_layout, *dst_layout, copy_loc);

    if (skip) {
        return skip;  // consistency will likley be wrong if already bad
    }

    // Verify consistency of src & dst bindings if update crosses binding boundaries
    skip |= VerifyUpdateConsistency(*src_set, update.srcBinding, update.srcArrayElement, update.descriptorCount, true,
                                    copy_loc.dot(Field::dstSet));
    skip |= VerifyUpdateConsistency(*dst_set, update.dstBinding, update.dstArrayElement, update.descriptorCount, true,
                                    copy_loc.dot(Field::srcSet));

    return skip;
}

bool CoreChecks::ValidateCopyUpdateDescriptorSetLayoutFlags(const VkCopyDescriptorSet &update,
                                                            const vvl::DescriptorSetLayout &src_layout,
                                                            const vvl::DescriptorSetLayout &dst_layout,
                                                            const Location &copy_loc) const {
    bool skip = false;
    const VkDescriptorSetLayoutCreateFlags src_flags = src_layout.GetCreateFlags();
    const VkDescriptorSetLayoutCreateFlags dst_flags = dst_layout.GetCreateFlags();

    if ((src_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) &&
        !(dst_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-01918", objlist, copy_loc.dot(Field::srcSet),
                         "layout was created with %s, but dstSet layout was created with %s.",
                         string_VkDescriptorSetLayoutCreateFlags(src_flags).c_str(),
                         string_VkDescriptorSetLayoutCreateFlags(dst_flags).c_str());
    }

    if (!(src_flags &
          (VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) &&
        (dst_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-04885", objlist, copy_loc.dot(Field::srcSet),
                         "layout was created with %s, but dstSet layout was created with %s.",
                         string_VkDescriptorSetLayoutCreateFlags(src_flags).c_str(),
                         string_VkDescriptorSetLayoutCreateFlags(dst_flags).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateCopyUpdateDescriptorPoolFlags(const VkCopyDescriptorSet &update, const vvl::DescriptorSet &src_set,
                                                       const vvl::DescriptorSet &dst_set, const Location &copy_loc) const {
    bool skip = false;

    const auto src_pool = src_set.GetPoolState();
    const auto dst_pool = dst_set.GetPoolState();
    if (!src_pool || !dst_pool) return skip;

    const VkDescriptorPoolResetFlags src_flags = src_pool->create_info.flags;
    const VkDescriptorPoolResetFlags dst_flags = dst_pool->create_info.flags;
    if ((src_flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) &&
        !(dst_flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_pool->Handle(), dst_pool->Handle());
        skip |=
            LogError("VUID-VkCopyDescriptorSet-srcSet-01920", objlist, copy_loc.dot(Field::srcSet),
                     "descriptor pool was created with %s, but dstSet descriptor pool was created with %s.",
                     string_VkDescriptorPoolCreateFlags(src_flags).c_str(), string_VkDescriptorPoolCreateFlags(dst_flags).c_str());
    }

    if (!(src_flags & (VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) &&
        (dst_flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_pool->Handle(), dst_pool->Handle());
        skip |=
            LogError("VUID-VkCopyDescriptorSet-srcSet-04887", objlist, copy_loc.dot(Field::srcSet),
                     "descriptor pool was created with %s, but dstSet descriptor pool was created with %s.",
                     string_VkDescriptorPoolCreateFlags(src_flags).c_str(), string_VkDescriptorPoolCreateFlags(dst_flags).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateCopyUpdateDescriptorTypes(const VkCopyDescriptorSet &update, const vvl::DescriptorSet &src_set,
                                                   const vvl::DescriptorSet &dst_set, const vvl::DescriptorSetLayout &src_layout,
                                                   const vvl::DescriptorSetLayout &dst_layout, const Location &copy_loc) const {
    bool skip = false;

    // Note - the VkDescriptorType are prior to resolving the Mutable Descriptor Types
    VkDescriptorType src_type = src_layout.GetTypeFromBinding(update.srcBinding);
    VkDescriptorType dst_type = dst_layout.GetTypeFromBinding(update.dstBinding);

    if (src_type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT && dst_type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT && src_type != dst_type) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-dstBinding-02632", objlist, copy_loc.dot(Field::dstBinding),
                         "(%" PRIu32 ") (from %s) is %s, but srcBinding (%" PRIu32 ") (from %s) is %s.", update.dstBinding,
                         FormatHandle(dst_set.Handle()).c_str(), string_VkDescriptorType(dst_type), update.srcBinding,
                         FormatHandle(src_set.Handle()).c_str(), string_VkDescriptorType(src_type));
    }

    if (src_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && ((update.srcArrayElement % 4) != 0)) {
        const LogObjectList objlist(update.srcSet, src_layout.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcBinding-02223", objlist, copy_loc.dot(Field::srcArrayElement),
                         "is %" PRIu32 " (not a multiple of 4), but srcBinding (%" PRIu32
                         ") type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK.",
                         update.srcArrayElement, update.srcBinding);
    }
    if (dst_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && ((update.dstArrayElement % 4) != 0)) {
        const LogObjectList objlist(update.dstSet, dst_layout.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-dstBinding-02224", objlist, copy_loc.dot(Field::dstArrayElement),
                         "is %" PRIu32 " (not a multiple of 4), but dstBinding (%" PRIu32
                         ") type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK.",
                         update.dstArrayElement, update.dstBinding);
    }
    if (src_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK || dst_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
        if ((update.descriptorCount % 4) != 0) {
            const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
            skip |= LogError("VUID-VkCopyDescriptorSet-srcBinding-02225", objlist, copy_loc.dot(Field::descriptorCount),
                             "is %" PRIu32 " (not a multiple of 4), but srcBinding (%" PRIu32
                             ") type is %s and dstBinding (%" PRIu32 ") type is %s.",
                             update.descriptorCount, update.srcBinding, string_VkDescriptorType(src_type), update.dstBinding,
                             string_VkDescriptorType(dst_type));
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        if (src_type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            if (!dst_layout.IsTypeMutable(src_type, update.dstBinding)) {
                const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
                skip |= LogError("VUID-VkCopyDescriptorSet-dstSet-04612", objlist, copy_loc.dot(Field::dstBinding),
                                 "(%" PRIu32 ") descriptor type is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but the srcBinding (%" PRIu32
                                 ") descriptor type (%s) is "
                                 "not found in pMutableDescriptorTypeLists list [%s].",
                                 update.dstBinding, update.srcBinding, string_VkDescriptorType(src_type),
                                 dst_layout.PrintMutableTypes(update.dstBinding).c_str());
            }
        }
    } else if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
        for (uint32_t i = 0; i < update.descriptorCount; i++, ++src_iter) {
            const auto &mutable_src = static_cast<const vvl::MutableDescriptor &>(*src_iter);
            if (mutable_src.ActiveType() != dst_type) {
                const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
                skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-04613", objlist, copy_loc.dot(Field::srcBinding),
                                 "(%" PRIu32
                                 ") descriptor type is VK_DESCRIPTOR_TYPE_MUTABLE_EXT and is being updated as descriptor type "
                                 "(%s), but it doesn't "
                                 "match the dstBinding (%" PRIu32 ") descriptor type %s.",
                                 update.srcBinding, string_VkDescriptorType(mutable_src.ActiveType()), update.dstBinding,
                                 string_VkDescriptorType(dst_type));
            }
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT && src_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        // These have been sorted already so can direct compare
        const auto &mutable_src_types = src_layout.GetMutableTypes(update.srcBinding);
        const auto &mutable_dst_types = dst_layout.GetMutableTypes(update.dstBinding);
        if (mutable_src_types != mutable_dst_types) {
            const LogObjectList objlist(update.srcSet, update.dstSet, src_layout.Handle(), dst_layout.Handle());
            skip |= LogError("VUID-VkCopyDescriptorSet-dstSet-04614", objlist, copy_loc.dot(Field::srcBinding),
                             "(%" PRIu32 ") and dstBinding (%" PRIu32
                             ") are both VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but their corresponding pMutableDescriptorTypeLists do "
                             "not match.\nsrc [%s]\ndst [%s]\n",
                             update.srcBinding, update.dstBinding, src_layout.PrintMutableTypes(update.srcBinding).c_str(),
                             dst_layout.PrintMutableTypes(update.dstBinding).c_str());
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        dst_type =
            static_cast<const vvl::MutableDescriptor *>(dst_set.GetDescriptorFromBinding(update.dstBinding, update.dstArrayElement))
                ->ActiveType();
    }
    if (dst_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        auto dst_iter = dst_set.FindDescriptor(update.dstBinding, update.dstArrayElement);
        for (uint32_t di = 0; di < update.descriptorCount; ++di, ++dst_iter) {
            if (dst_iter.updated() && dst_iter->IsImmutableSampler()) {
                const LogObjectList objlist(update.srcSet, update.dstSet);
                skip |=
                    LogError("VUID-VkCopyDescriptorSet-dstBinding-02753", objlist, copy_loc.dot(Field::dstBinding),
                             "(%" PRIu32
                             ") is type VK_DESCRIPTOR_TYPE_SAMPLER, but the dstSet was created with a non-null pImmutableSamplers.",
                             update.dstBinding);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateImageUpdate(const vvl::ImageView &view_state, VkImageLayout image_layout, VkDescriptorType type,
                                     const Location &image_info_loc) const {
    bool skip = false;

    // Note that when an imageview is created, we validated that memory is bound so no need to re-check here
    // Validate that imageLayout is compatible with aspect_mask and image format
    //  and validate that image usage bits are correct for given usage
    VkImageAspectFlags aspect_mask = view_state.normalized_subresource_range.aspectMask;
    VkImage image = view_state.create_info.image;
    VkImageUsageFlags usage = 0;
    auto *image_node = view_state.image_state.get();
    ASSERT_AND_RETURN_SKIP(image_node);

    const auto image_view_usage_info = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(view_state.create_info.pNext);
    const auto stencil_usage_info = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_node->create_info.pNext);
    if (image_view_usage_info) {
        usage = image_view_usage_info->usage;
    } else {
        usage = image_node->create_info.usage;
    }
    if (stencil_usage_info) {
        const bool stencil_aspect = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) > 0;
        const bool depth_aspect = (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) > 0;
        if (stencil_aspect && !depth_aspect) {
            usage = stencil_usage_info->stencilUsage;
        } else if (stencil_aspect && depth_aspect) {
            usage &= stencil_usage_info->stencilUsage;
        }
    }

    // Validate that memory is bound to image
    // VU being worked on https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5598
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(image), *image_node, image_info_loc.dot(Field::image),
                                         "UNASSIGNED-VkDescriptorImageInfo-BoundResourceFreedMemoryAccess");

    const LogObjectList objlist(view_state.Handle(), image_node->Handle());
    // KHR_maintenance1 allows rendering into 2D or 2DArray views which slice a 3D image,
    // but not binding them to descriptor sets.
    if (view_state.IsDepthSliced() && image_node->create_info.imageType == VK_IMAGE_TYPE_3D) {
        // VK_EXT_image_2d_view_of_3d allows use of VIEW_TYPE_2D in descriptor
        if (view_state.create_info.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
            skip |= LogError("VUID-VkDescriptorImageInfo-imageView-06712", objlist, image_info_loc.dot(Field::imageView),
                             "is VK_IMAGE_VIEW_TYPE_2D_ARRAY but the image is VK_IMAGE_TYPE_3D.");
        } else if (view_state.create_info.viewType == VK_IMAGE_VIEW_TYPE_2D) {
            // Check 06713/06714 first to alert apps without VK_EXT_image_2d_view_of_3d that the features are needed
            if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE && !enabled_features.image2DViewOf3D) {
                skip |= LogError("VUID-VkDescriptorImageInfo-descriptorType-06713", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, and the descriptorType is "
                                 "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, but the image2DViewOf3D feature was not enabled.");
            }

            if ((type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
                !enabled_features.sampler2DViewOf3D) {
                skip |= LogError("VUID-VkDescriptorImageInfo-descriptorType-06714", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, and the descriptorType is %s, but "
                                 "the sampler2DViewOf3D feature was not enabled.",
                                 string_VkDescriptorType(type));
            }

            if ((type != VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) && (type != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) &&
                (type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) {
                skip |= LogError("VUID-VkDescriptorImageInfo-imageView-07795", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, and the descriptorType is %s.",
                                 string_VkDescriptorType(type));
            }

            if (!(image_node->create_info.flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT)) {
                skip |= LogError("VUID-VkDescriptorImageInfo-imageView-07796", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, but the image was created with %s.",
                                 string_VkImageCreateFlags(image_node->create_info.flags).c_str());
            }
        }
    }

    if (image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        if (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            skip |= LogError("VUID-VkDescriptorImageInfo-imageLayout-09425", objlist, image_info_loc.dot(Field::imageView),
                             "uses layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but aspectMask (%s) include depth/stencil bit.",
                             string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }

    if (IsValueIn(
            image_layout,
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
             VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
             VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL})) {
        if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= LogError("VUID-VkDescriptorImageInfo-imageLayout-09426", objlist, image_info_loc.dot(Field::imageView),
                             "uses layout %s but aspectMask (%s) includes color bit.", string_VkImageLayout(image_layout),
                             string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }

    if (vkuFormatIsDepthOrStencil(image_node->create_info.format)) {
        if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
            if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
                skip |= LogError("VUID-VkDescriptorImageInfo-imageView-01976", objlist, image_info_loc.dot(Field::imageView),
                                 "use layout %s and the image format (%s), but it has both STENCIL and DEPTH aspects set",
                                 string_VkImageLayout(image_layout), string_VkFormat(image_node->create_info.format));
            }
        }
    }

    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            if (view_state.samplerConversion != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-01946", objlist, image_info_loc.dot(Field::imageView),
                                 "is used as VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but was created with %s",
                                 FormatHandle(view_state.samplerConversion).c_str());
            }
            [[fallthrough]];
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00337", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is %s.", string_VkImageUsageFlags(usage).c_str(),
                                 string_VkDescriptorType(type));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            if (!(usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00339", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.",
                                 string_VkImageUsageFlags(usage).c_str());

            } else if ((VK_IMAGE_LAYOUT_GENERAL != image_layout) && (!IsExtEnabled(extensions.vk_khr_shared_presentable_image) ||
                                                                     (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != image_layout))) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-04152", objlist, image_info_loc.dot(Field::imageView),
                                 "image layout is %s, but descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE. (allowed layouts are "
                                 "VK_IMAGE_LAYOUT_GENERAL or VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR).",
                                 string_VkImageLayout(image_layout));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            if (!(usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00338", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT.",
                                 string_VkImageUsageFlags(usage).c_str());
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-06942", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM.",
                                 string_VkImageUsageFlags(usage).c_str());
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-06943", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM.",
                                 string_VkImageUsageFlags(usage).c_str());
            }
            break;
        }
        default:
            break;
    }

    // All the following types share the same image layouts
    // checkf or Storage Images above
    if ((type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
        (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        // Test that the layout is compatible with the descriptorType for the two sampled image types
        const static std::array<VkImageLayout, 3> valid_layouts = {
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL}};

        struct ExtensionLayout {
            VkImageLayout layout;
            ExtEnabled DeviceExtensions::*extension;
        };
        const static std::array<ExtensionLayout, 9> extended_layouts{{
            //  Note double brace req'd for aggregate initialization
            {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, &DeviceExtensions::vk_khr_shared_presentable_image},
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
            {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
            {VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_synchronization2},
            {VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, &DeviceExtensions::vk_khr_synchronization2},
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_separate_depth_stencil_layouts},
            {VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_separate_depth_stencil_layouts},
            {VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, &DeviceExtensions::vk_ext_attachment_feedback_loop_layout},
            {VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ, &DeviceExtensions::vk_khr_dynamic_rendering_local_read},
        }};
        auto is_layout = [image_layout, this](const ExtensionLayout &ext_layout) {
            return IsExtEnabled(extensions.*(ext_layout.extension)) && (ext_layout.layout == image_layout);
        };

        const bool valid_layout = (std::find(valid_layouts.cbegin(), valid_layouts.cend(), image_layout) != valid_layouts.cend()) ||
                                  std::any_of(extended_layouts.cbegin(), extended_layouts.cend(), is_layout);

        if (!valid_layout) {
            // The following works as currently all 3 descriptor types share the same set of valid layouts
            const char *vuid = kVUIDUndefined;
            switch (type) {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    vuid = "VUID-VkWriteDescriptorSet-descriptorType-04149";
                    break;
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    vuid = "VUID-VkWriteDescriptorSet-descriptorType-04150";
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    vuid = "VUID-VkWriteDescriptorSet-descriptorType-04151";
                    break;
                default:
                    break;
            }
            std::stringstream error_str;
            error_str << "Descriptor update with descriptorType " << string_VkDescriptorType(type)
                      << " is being updated with invalid imageLayout " << string_VkImageLayout(image_layout) << " for image "
                      << FormatHandle(image) << " in imageView " << FormatHandle(view_state.Handle())
                      << ". Allowed layouts are: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                      << "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL";
            for (auto &ext_layout : extended_layouts) {
                if (IsExtEnabled(extensions.*(ext_layout.extension))) {
                    error_str << ", " << string_VkImageLayout(ext_layout.layout);
                }
            }
            skip |= LogError(vuid, objlist, image_info_loc, "%s", error_str.str().c_str());
        }
    }

    if ((type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) || (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        const VkComponentMapping components = view_state.create_info.components;
        if (IsIdentitySwizzle(components) == false) {
            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00336", objlist, image_info_loc.dot(Field::imageView),
                             "has a non-identiy swizzle component, here are the actual swizzle values:\n%s",
                             string_VkComponentMapping(components).c_str());
        }
    }

    if ((type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) && (view_state.min_lod != 0.0f)) {
        skip |=
            LogError("VUID-VkWriteDescriptorSet-descriptorType-06450", objlist, image_info_loc.dot(Field::imageView),
                     "was created with minLod %f, but descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT.", view_state.min_lod);
    }

    return skip;
}

// This is a helper function that iterates over a set of Write and Copy updates, pulls the DescriptorSet* for updated
//  sets, and then calls their respective Validate[Write|Copy]Update functions.
// If the update hits an issue for which the callback returns "true", meaning that the call down the chain should
//  be skipped, then true is returned.
// If there is no issue with the update, then false is returned.
bool CoreChecks::ValidateUpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                                              uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies,
                                              const Location &loc) const {
    bool skip = false;
    // Validate Write updates
    for (uint32_t i = 0; i < descriptorWriteCount; i++) {
        if (const auto set_node = Get<vvl::DescriptorSet>(pDescriptorWrites[i].dstSet)) {
            const Location write_loc = loc.dot(Field::pDescriptorWrites, i);
            const Location dst_set_loc = write_loc.dot(Field::dstSet);
            vvl::DslErrorSource dsl_error_source(dst_set_loc, pDescriptorWrites[i].dstSet);
            skip |= ValidateWriteUpdate(*set_node, pDescriptorWrites[i], write_loc, dsl_error_source);
        }
    }

    for (uint32_t i = 0; i < descriptorCopyCount; ++i) {
        const Location copy_loc = loc.dot(Field::pDescriptorCopies, i);
        skip |= ValidateCopyUpdate(pDescriptorCopies[i], copy_loc);
    }

    return skip;
}

vvl::DecodedTemplateUpdate::DecodedTemplateUpdate(const vvl::Device &device_data, VkDescriptorSet descriptorSet,
                                                  const vvl::DescriptorUpdateTemplate *template_state, const void *pData,
                                                  VkDescriptorSetLayout push_layout) {
    auto const &create_info = template_state->create_info;
    inline_infos.resize(create_info.descriptorUpdateEntryCount);  // Make sure we have one if we need it
    inline_infos_khr.resize(create_info.descriptorUpdateEntryCount);
    inline_infos_nv.resize(create_info.descriptorUpdateEntryCount);
    desc_writes.reserve(create_info.descriptorUpdateEntryCount);  // emplaced, so reserved without initialization
    VkDescriptorSetLayout effective_dsl = create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET
                                              ? create_info.descriptorSetLayout
                                              : push_layout;
    auto ds_layout_state = device_data.Get<vvl::DescriptorSetLayout>(effective_dsl);
    if (!ds_layout_state) return;

    // Create a WriteDescriptorSet struct for each template update entry
    for (uint32_t i = 0; i < create_info.descriptorUpdateEntryCount; i++) {
        auto binding_count = ds_layout_state->GetDescriptorCountFromBinding(create_info.pDescriptorUpdateEntries[i].dstBinding);
        auto binding_being_updated = create_info.pDescriptorUpdateEntries[i].dstBinding;
        auto dst_array_element = create_info.pDescriptorUpdateEntries[i].dstArrayElement;

        desc_writes.reserve(desc_writes.size() + create_info.pDescriptorUpdateEntries[i].descriptorCount);
        for (uint32_t j = 0; j < create_info.pDescriptorUpdateEntries[i].descriptorCount; j++) {
            desc_writes.emplace_back();
            auto &write_entry = desc_writes.back();

            size_t offset = create_info.pDescriptorUpdateEntries[i].offset + j * create_info.pDescriptorUpdateEntries[i].stride;
            char *update_entry = (char *)(pData) + offset;

            if (dst_array_element >= binding_count) {
                dst_array_element = 0;
                binding_being_updated = ds_layout_state->GetNextValidBinding(binding_being_updated);
            }

            write_entry.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_entry.pNext = NULL;
            write_entry.dstSet = descriptorSet;
            write_entry.dstBinding = binding_being_updated;
            write_entry.dstArrayElement = dst_array_element;
            write_entry.descriptorCount = 1;
            write_entry.descriptorType = create_info.pDescriptorUpdateEntries[i].descriptorType;

            switch (create_info.pDescriptorUpdateEntries[i].descriptorType) {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    write_entry.pImageInfo = reinterpret_cast<VkDescriptorImageInfo *>(update_entry);
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    write_entry.pBufferInfo = reinterpret_cast<VkDescriptorBufferInfo *>(update_entry);
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    write_entry.pTexelBufferView = reinterpret_cast<VkBufferView *>(update_entry);
                    break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
                    VkWriteDescriptorSetInlineUniformBlock *inline_info = &inline_infos[i];
                    inline_info->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT;
                    inline_info->pNext = nullptr;
                    inline_info->dataSize = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info->pData = update_entry;
                    write_entry.pNext = inline_info;
                    // descriptorCount must match the dataSize member of the VkWriteDescriptorSetInlineUniformBlock structure
                    write_entry.descriptorCount = inline_info->dataSize;
                    // skip the rest of the array, they just represent bytes in the update
                    j = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    break;
                }
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
                    VkWriteDescriptorSetAccelerationStructureKHR *inline_info_khr = &inline_infos_khr[i];
                    inline_info_khr->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                    inline_info_khr->pNext = nullptr;
                    inline_info_khr->accelerationStructureCount = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info_khr->pAccelerationStructures = reinterpret_cast<VkAccelerationStructureKHR *>(update_entry);
                    write_entry.pNext = inline_info_khr;
                    break;
                }
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
                    VkWriteDescriptorSetAccelerationStructureNV *inline_info_nv = &inline_infos_nv[i];
                    inline_info_nv->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
                    inline_info_nv->pNext = nullptr;
                    inline_info_nv->accelerationStructureCount = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info_nv->pAccelerationStructures = reinterpret_cast<VkAccelerationStructureNV *>(update_entry);
                    write_entry.pNext = inline_info_nv;
                    break;
                }
                default:
                    assert(false);
                    break;
            }
            dst_array_element++;
        }
    }
}

// Loop through the write updates to validate for a push descriptor set, ignoring dstSet
bool CoreChecks::ValidatePushDescriptorsUpdate(const vvl::DescriptorSet &push_set, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet *pDescriptorWrites,
                                               const vvl::DslErrorSource &dsl_error_source, const Location &loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < descriptorWriteCount; i++) {
        skip |= ValidateWriteUpdate(push_set, pDescriptorWrites[i], loc.dot(Field::pDescriptorWrites, i), dsl_error_source);
    }
    return skip;
}

// For the given buffer, verify that its creation parameters are appropriate for the given type
//  If there's an error, update the error_msg string with details and return false, else return true
bool CoreChecks::ValidateBufferUsage(const vvl::Buffer &buffer_state, VkDescriptorType type, const Location &buffer_loc) const {
    bool skip = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-08765", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER.",
                                 string_VkBufferUsageFlags2(buffer_state.usage).c_str());
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-08766", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER.",
                                 string_VkBufferUsageFlags2(buffer_state.usage).c_str());
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00330", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is %s.",
                                 string_VkBufferUsageFlags2(buffer_state.usage).c_str(), string_VkDescriptorType(type));
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00331", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is %s.",
                                 string_VkBufferUsageFlags2(buffer_state.usage).c_str(), string_VkDescriptorType(type));
            }
            break;
        default:
            break;
    }
    return skip;
}

bool CoreChecks::ValidateBufferUpdate(const VkDescriptorBufferInfo &buffer_info, VkDescriptorType type,
                                      const Location &buffer_info_loc) const {
    bool skip = false;
    // Invalid handles should be caught by the object tracker, but lets make sure not to crash anyways.
    if (buffer_info.buffer == VK_NULL_HANDLE) return skip;
    const auto buffer_state = Get<vvl::Buffer>(buffer_info.buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);

    skip |= ValidateMemoryIsBoundToBuffer(device, *buffer_state, buffer_info_loc.dot(Field::buffer),
                                          "VUID-VkWriteDescriptorSet-descriptorType-00329");
    skip |= ValidateBufferUsage(*buffer_state, type, buffer_info_loc.dot(Field::buffer));

    if (buffer_info.offset >= buffer_state->create_info.size) {
        skip |= LogError("VUID-VkDescriptorBufferInfo-offset-00340", buffer_info.buffer, buffer_info_loc.dot(Field::offset),
                         "(%" PRIu64 ") is greater than or equal to buffer size (%" PRIu64 ").", buffer_info.offset,
                         buffer_state->create_info.size);
    }
    if (buffer_info.range != VK_WHOLE_SIZE) {
        if (buffer_info.range == 0) {
            skip |= LogError("VUID-VkDescriptorBufferInfo-range-00341", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                             "is not VK_WHOLE_SIZE and is zero.");
        }
        if (buffer_info.range > (buffer_state->create_info.size - buffer_info.offset)) {
            skip |= LogError("VUID-VkDescriptorBufferInfo-range-00342", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                             "(%" PRIu64 ") is larger than buffer size (%" PRIu64 ") - offset (%" PRIu64 ").", buffer_info.range,
                             buffer_state->create_info.size, buffer_info.offset);
        }
    }

    if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type) {
        const uint32_t max_ub_range = phys_dev_props.limits.maxUniformBufferRange;
        if (buffer_info.range != VK_WHOLE_SIZE && buffer_info.range > max_ub_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00332", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "(%" PRIu64 ") is greater than maxUniformBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_info.range, max_ub_range, string_VkDescriptorType(type));
        } else if (buffer_info.range == VK_WHOLE_SIZE && (buffer_state->create_info.size - buffer_info.offset) > max_ub_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00332", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "is VK_WHOLE_SIZE, but the effective range [size (%" PRIu64 ") - offset (%" PRIu64 ") = %" PRIu64
                         "] is greater than maxUniformBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_state->create_info.size, buffer_info.offset, buffer_state->create_info.size - buffer_info.offset,
                         max_ub_range, string_VkDescriptorType(type));
        }
    } else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER == type || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type) {
        const uint32_t max_sb_range = phys_dev_props.limits.maxStorageBufferRange;
        if (buffer_info.range != VK_WHOLE_SIZE && buffer_info.range > max_sb_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00333", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "(%" PRIu64 ") is greater than maxStorageBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_info.range, max_sb_range, string_VkDescriptorType(type));
        } else if (buffer_info.range == VK_WHOLE_SIZE && (buffer_state->create_info.size - buffer_info.offset) > max_sb_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00333", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "is VK_WHOLE_SIZE, but the effective range [size (%" PRIu64 ") - offset (%" PRIu64 ") = %" PRIu64
                         "] is greater than maxStorageBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_state->create_info.size, buffer_info.offset, buffer_state->create_info.size - buffer_info.offset,
                         max_sb_range, string_VkDescriptorType(type));
        }
    }
    return skip;
}

bool CoreChecks::ValidateWriteUpdate(const vvl::DescriptorSet &dst_set, const VkWriteDescriptorSet &update,
                                     const Location &write_loc, const vvl::DslErrorSource &dsl_error_source) const {
    bool skip = false;
    const vvl::DescriptorSetLayout *dst_layout = dst_set.GetLayout().get();

    // Verify dst layout still valid (ObjectLifetimes only checks if null, we check if valid dstSet here)
    if (dst_layout->Destroyed()) {
        return LogError("VUID-VkWriteDescriptorSet-dstSet-00320", dst_layout->Handle(), dsl_error_source.ds_loc_,
                        "%s has been destroyed.%s", FormatHandle(dst_layout->Handle()).c_str(),
                        dsl_error_source.PrintMessage(*this).c_str());
    }

    const Location dst_binding_loc = write_loc.dot(Field::dstBinding);
    if (dst_layout->GetBindingCount() == 0) {
        return LogError("VUID-VkWriteDescriptorSet-dstBinding-10009", dst_layout->Handle(), dsl_error_source.ds_loc_,
                        "%s was created with bindingCount of zero.%s", FormatHandle(dst_layout->Handle()).c_str(),
                        dsl_error_source.PrintMessage(*this).c_str());
    } else if (update.dstBinding > dst_layout->GetMaxBinding()) {
        return LogError("VUID-VkWriteDescriptorSet-dstBinding-00315", dst_layout->Handle(), dst_binding_loc,
                        "(%" PRIu32 ") is larger than bindingCount (%" PRIu32 ") used to create %s.%s", update.dstBinding,
                        dst_layout->GetBindingCount(), FormatHandle(dst_layout->Handle()).c_str(),
                        dsl_error_source.PrintMessage(*this).c_str());
    }

    const vvl::DescriptorBinding *dst_binding = dst_set.GetBinding(update.dstBinding);
    if (!dst_binding) {
        // Spec: "Bindings that are not specified have a descriptorCount and stageFlags of zero"
        // If we can't find the binding, it means it was not in the layout and is same of having a zero descriptorCount
        skip |=
            LogError("VUID-VkWriteDescriptorSet-dstBinding-00316", dst_layout->Handle(), dst_binding_loc,
                     "(%" PRIu32
                     ") was never set in any VkDescriptorSetLayoutBinding::binding for %s, therefore the descriptorCount value "
                     "is considered zero.%s",
                     update.dstBinding, FormatHandle(dst_layout->Handle()).c_str(), dsl_error_source.PrintMessage(*this).c_str());
        return skip;  // the rest of checks assume a valid DescriptorBinding state
    } else if (dst_binding->count == 0) {
        skip |= LogError("VUID-VkWriteDescriptorSet-dstBinding-00316", dst_layout->Handle(), dst_binding_loc,
                         "(%" PRIu32 ") has VkDescriptorSetLayoutBinding::descriptorCount of zero in %s.%s", update.dstBinding,
                         FormatHandle(dst_layout->Handle()).c_str(), dsl_error_source.PrintMessage(*this).c_str());
    }

    if (!vvl::IsBindless(dst_binding->binding_flags)) {
        if (const auto *used_handle = dst_set.InUse()) {
            const LogObjectList objlist(update.dstSet, dst_layout->Handle());
            skip |= LogError("VUID-vkUpdateDescriptorSets-None-03047", objlist, dst_binding_loc,
                             "(%" PRIu32 ") was created with %s, but %s is in use by %s.", update.dstBinding,
                             string_VkDescriptorBindingFlags(dst_binding->binding_flags).c_str(),
                             FormatHandle(update.dstSet).c_str(), FormatHandle(*used_handle).c_str());
        }
    }

    if (dst_binding->type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        // Check if the new descriptor descriptor type is in the list of allowed mutable types for this binding
        if (!dst_layout->IsTypeMutable(update.descriptorType, update.dstBinding)) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-dstSet-04611", dst_layout->Handle(), write_loc.dot(Field::dstBinding),
                         "(%" PRIu32
                         ") is of type VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but the new descriptorType (%s) was not in "
                         "VkMutableDescriptorTypeListEXT::pDescriptorTypes (%s).%s",
                         update.dstBinding, string_VkDescriptorType(update.descriptorType),
                         dst_layout->PrintMutableTypes(update.dstBinding).c_str(), dsl_error_source.PrintMessage(*this).c_str());
        }
    } else if (dst_binding->type != update.descriptorType) {
        skip |=
            LogError("VUID-VkWriteDescriptorSet-descriptorType-00319", dst_layout->Handle(), write_loc.dot(Field::descriptorType),
                     "(%s) is different from pBindings[%" PRIu32 "].descriptorType (%s) of %s.%s",
                     string_VkDescriptorType(update.descriptorType), update.dstBinding, string_VkDescriptorType(dst_binding->type),
                     FormatHandle(dst_layout->Handle()).c_str(), dsl_error_source.PrintMessage(*this).c_str());
    }

    skip |= ValidateWriteUpdateDescriptorType(update, write_loc);

    // descriptorCount must be greater than 0
    if (update.descriptorCount == 0) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorCount-arraylength", device, write_loc.dot(Field::descriptorCount),
                         "is zero.");
    } else {
        // Save first binding information and error if something different is found
        auto current_iter = dst_set.FindBinding(update.dstBinding);
        VkShaderStageFlags stage_flags = (*current_iter)->stage_flags;
        VkDescriptorType descriptor_type = (*current_iter)->type;
        const bool immutable_samplers = (*current_iter)->has_immutable_samplers;
        uint32_t dst_array_element = update.dstArrayElement;

        for (uint32_t i = 0; i < update.descriptorCount;) {
            if (current_iter == dst_set.end()) {
                break;  // prevents setting error here if bindings don't exist
            }
            auto current_binding = current_iter->get();

            // All consecutive bindings updated, except those with a descriptorCount of zero, must have identical descType and
            // stageFlags
            if (current_binding->count > 0) {
                // Check for consistent stageFlags and descriptorType
                if ((current_binding->stage_flags != stage_flags) || (current_binding->type != descriptor_type)) {
                    std::stringstream extra;
                    // If using inline, easy to go outside of its range and not realize you are in the next descriptor
                    if (descriptor_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
                        extra << " For inline uniforms blocks, you might have your VkWriteDescriptorSet::dstArrayElement ("
                              << update.dstArrayElement << ") + VkWriteDescriptorSet::descriptorCount (" << update.descriptorCount
                              << ") larger than your VkDescriptorSetLayoutBinding::descriptorCount so this is trying to update the "
                                 "next binding.";
                    }

                    const LogObjectList objlist(update.dstSet, dst_layout->Handle());
                    skip |= LogError(
                        "VUID-VkWriteDescriptorSet-descriptorCount-00317", objlist, write_loc,
                        "binding #%" PRIu32 " (started on dstBinding [%" PRIu32 "] + %" PRIu32
                        " descriptors offset) has stageFlags of %s and descriptorType of %s, but previous binding was %s and %s.%s",
                        current_binding->binding, update.dstBinding, i,
                        string_VkShaderStageFlags(current_binding->stage_flags).c_str(),
                        string_VkDescriptorType(current_binding->type), string_VkShaderStageFlags(stage_flags).c_str(),
                        string_VkDescriptorType(descriptor_type), extra.str().c_str());
                }
                // Check if all immutableSamplers or not
                if (current_binding->has_immutable_samplers != immutable_samplers) {
                    const LogObjectList objlist(update.dstSet, dst_layout->Handle());
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorCount-00318", objlist, write_loc,
                                     "binding #%" PRIu32 " (started on dstBinding [%" PRIu32 "] + %" PRIu32
                                     " descriptors offset) %s Immutable Samplers, which is different from the previous binding.",
                                     current_binding->binding, update.dstBinding, i,
                                     current_binding->has_immutable_samplers ? "has" : "doesn't have");
                }
            }

            // Skip the remaining descriptors for this binding, and move to the next binding
            i += (current_binding->count - dst_array_element);
            dst_array_element = 0;
            ++current_iter;
        }
    }

    if (skip) {
        return skip;  // consistency will likley be wrong if already bad
    }

    if (dst_binding->IsVariableCount()) {
        if ((update.dstArrayElement + update.descriptorCount) > dst_set.GetVariableDescriptorCount()) {
            // Can't use Variable Count with PushDescriptors
            const LogObjectList objlist(update.dstSet, dst_layout->Handle());
            skip |= LogError("VUID-VkWriteDescriptorSet-dstArrayElement-00321", objlist, write_loc.dot(Field::dstArrayElement),
                             "(%" PRIu32 ") + descriptorCount (%" PRIu32 ") is larger than (%" PRIu32 ") for dstBinding (%" PRIu32
                             ") in %s (allocated with %s).",
                             update.dstArrayElement, update.descriptorCount, dst_set.GetVariableDescriptorCount(),
                             update.dstBinding, FormatHandle(dst_set.Handle()).c_str(), FormatHandle(dst_layout->Handle()).c_str());
        }
    } else {
        skip |= VerifyUpdateConsistency(dst_set, update.dstBinding, update.dstArrayElement, update.descriptorCount, false,
                                        dsl_error_source.ds_loc_);
    }

    // The pipeline_layout is there for PushDescriptors as the proxy VkDescriptorSet comes from there and not update.dstSet
    const bool is_push_descriptor = dsl_error_source.pipeline_layout_handle_ != VK_NULL_HANDLE;
    // Update is within bounds and consistent so last step is to validate update contents
    skip |= VerifyWriteUpdateContents(dst_set, update, write_loc, is_push_descriptor);

    return skip;
}

bool CoreChecks::ValidateWriteUpdateDescriptorType(const VkWriteDescriptorSet &update, const Location &write_loc) const {
    bool skip = false;
    // Always used unless dealing with Mutable where the type is found in the vvl::DescriptorBinding
    const VkDescriptorType descriptor_type = update.descriptorType;

    // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER and VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    // Valid bufferView handles are checked in ObjectLifetimes::ValidateDescriptorWrite.
    if (descriptor_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
        skip |= ValidateWriteUpdateInlineUniformBlock(update, write_loc);
    } else if (descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        skip |= ValidateWriteUpdateAccelerationStructureKHR(update, write_loc);
    } else if (descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
        skip |= ValidateWriteUpdateAccelerationStructureNV(update, write_loc);
    } else if (IsValueIn(descriptor_type, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                           VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC})) {
        skip |= ValidateWriteUpdateBufferInfo(update, write_loc);
    } else if (IsValueIn(descriptor_type,
                         {VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT})) {
        if (update.pImageInfo == nullptr) {
            const char *vuid =
                (write_loc.function == Func::vkCmdPushDescriptorSet || write_loc.function == Func::vkCmdPushDescriptorSetKHR)
                    ? "VUID-vkCmdPushDescriptorSet-pDescriptorWrites-06494"
                : (write_loc.function == Func::vkCmdPushDescriptorSet2 || write_loc.function == Func::vkCmdPushDescriptorSet2KHR)
                    ? "VUID-VkPushDescriptorSetInfo-pDescriptorWrites-06494"
                    : "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06493";
            skip |= LogError(vuid, device, write_loc.dot(Field::descriptorType), "is %s but pImageInfo is NULL.",
                             string_VkDescriptorType(descriptor_type));
        }
    }

    return skip;
}

bool CoreChecks::ValidateWriteUpdateBufferInfo(const VkWriteDescriptorSet &update, const Location &write_loc) const {
    bool skip = false;
    const VkDescriptorType descriptor_type = update.descriptorType;
    if (!update.pBufferInfo) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00324", device, write_loc.dot(Field::descriptorType),
                         "is %s but pBufferInfo is NULL.", string_VkDescriptorType(descriptor_type));
        return skip;
    }

    const VkDeviceSize uniform_alignment = phys_dev_props.limits.minUniformBufferOffsetAlignment;
    const VkDeviceSize storage_alignment = phys_dev_props.limits.minStorageBufferOffsetAlignment;

    for (uint32_t i = 0; i < update.descriptorCount; ++i) {
        const auto &buffer_info = update.pBufferInfo[i];

        if (enabled_features.nullDescriptor) {
            if (buffer_info.buffer == VK_NULL_HANDLE && (buffer_info.offset != 0 || buffer_info.range != VK_WHOLE_SIZE)) {
                skip |= LogError("VUID-VkDescriptorBufferInfo-buffer-02999", device,
                                 write_loc.dot(Field::pBufferInfo, i).dot(Field::buffer),
                                 "is VK_NULL_HANDLE, but offset (%" PRIu64 ") is not zero and range (%" PRIu64
                                 ") is not VK_WHOLE_SIZE when descriptorType is %s.",
                                 buffer_info.offset, buffer_info.range, string_VkDescriptorType(descriptor_type));
            }
        }

        if (IsValueIn(descriptor_type, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC})) {
            if (SafeModulo(buffer_info.offset, uniform_alignment) != 0) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00327", device,
                                 write_loc.dot(Field::pBufferInfo, i).dot(Field::offset),
                                 "(%" PRIu64 ") must be a multiple of device limit minUniformBufferOffsetAlignment (%" PRIu64
                                 ") when descriptorType is %s.",
                                 buffer_info.offset, uniform_alignment, string_VkDescriptorType(descriptor_type));
            }
        } else if (IsValueIn(descriptor_type, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC})) {
            if (SafeModulo(buffer_info.offset, storage_alignment) != 0) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00328", device,
                                 write_loc.dot(Field::pBufferInfo, i).dot(Field::offset),
                                 "(%" PRIu64 ") must be a multiple of device limit minStorageBufferOffsetAlignment (%" PRIu64
                                 ") when descriptorType is %s.",
                                 buffer_info.offset, storage_alignment, string_VkDescriptorType(descriptor_type));
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateWriteUpdateInlineUniformBlock(const VkWriteDescriptorSet &update, const Location &write_loc) const {
    bool skip = false;
    if ((update.dstArrayElement % 4) != 0) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02219", device, write_loc.dot(Field::dstBinding),
                         "(%" PRIu32 ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but dstArrayElement (%" PRIu32
                         ") is not a multiple of 4.",
                         update.dstBinding, update.dstArrayElement);
    }
    if ((update.descriptorCount % 4) != 0) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02220", device, write_loc.dot(Field::dstBinding),
                         "(%" PRIu32 ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but descriptorCount (%" PRIu32
                         ") is not a multiple of 4.",
                         update.dstBinding, update.descriptorCount);
    }
    const auto *write_inline_info = vku::FindStructInPNextChain<VkWriteDescriptorSetInlineUniformBlock>(update.pNext);
    if (!write_inline_info) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02221", device, write_loc.dot(Field::dstBinding),
                         "(%" PRIu32
                         ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but there is no "
                         "VkWriteDescriptorSetInlineUniformBlock in the pNext chain.",
                         update.dstBinding);
    } else if (write_inline_info->dataSize != update.descriptorCount) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02221", device,
                         write_loc.pNext(Struct::VkWriteDescriptorSetInlineUniformBlock, Field::dataSize),
                         "(%" PRIu32 ") is different then descriptorCount (%" PRIu32 "), but dstBinding (%" PRIu32
                         ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK.",
                         write_inline_info->dataSize, update.descriptorCount, update.dstBinding);
    } else if ((write_inline_info->dataSize % 4) != 0) {
        skip |= LogError("VUID-VkWriteDescriptorSetInlineUniformBlock-dataSize-02222", device,
                         write_loc.pNext(Struct::VkWriteDescriptorSetInlineUniformBlock, Field::dataSize), "is %" PRIu32 ".",
                         write_inline_info->dataSize);
    }
    return skip;
}

bool CoreChecks::ValidateWriteUpdateAccelerationStructureKHR(const VkWriteDescriptorSet &update, const Location &write_loc) const {
    bool skip = false;

    const auto *pnext_struct = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(update.pNext);
    if (!pnext_struct) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02382", device, write_loc.dot(Field::descriptorType),
                         "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, but the pNext chain doesn't include "
                         "VkWriteDescriptorSetAccelerationStructureKHR.");
        return skip;
    }

    if (pnext_struct->accelerationStructureCount != update.descriptorCount) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02382", device,
                         write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureKHR, Field::accelerationStructureCount),
                         "(%" PRIu32 ") is not equal to descriptorCount (%" PRIu32 ").", pnext_struct->accelerationStructureCount,
                         update.descriptorCount);
    }

    for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
        if (pnext_struct->pAccelerationStructures[j] == VK_NULL_HANDLE && !enabled_features.nullDescriptor) {
            skip |=
                LogError("VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03580", device,
                         write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureKHR, Field::pAccelerationStructures, j),
                         "is VK_NULL_HANDLE but the nullDescriptor feature was not enabled.");
        }

        auto as_state = Get<vvl::AccelerationStructureKHR>(pnext_struct->pAccelerationStructures[j]);
        if (!as_state) continue;
        if (as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
            as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
            skip |=
                LogError("VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03579", as_state->Handle(),
                         write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureKHR, Field::pAccelerationStructures, j),
                         "was created with %s.", string_VkAccelerationStructureTypeKHR(as_state->create_info.type));
        }
    }

    return skip;
}

bool CoreChecks::ValidateWriteUpdateAccelerationStructureNV(const VkWriteDescriptorSet &update, const Location &write_loc) const {
    bool skip = false;

    const auto *pnext_struct = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
    if (!pnext_struct || (pnext_struct->accelerationStructureCount != update.descriptorCount)) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-03817", device, write_loc,
                         "If descriptorType is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, the pNext"
                         "chain must include a VkWriteDescriptorSetAccelerationStructureNV structure whose "
                         "accelerationStructureCount %" PRIu32 " member equals descriptorCount %" PRIu32 ".",
                         pnext_struct ? pnext_struct->accelerationStructureCount : -1, update.descriptorCount);
        return skip;
    }

    for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
        if (pnext_struct->pAccelerationStructures[j] == VK_NULL_HANDLE && !enabled_features.nullDescriptor) {
            skip |=
                LogError("VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-03749", device,
                         write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureNV, Field::pAccelerationStructures, j),
                         "is VK_NULL_HANDLE, but the nullDescriptor feature is not enabled.");
        }
        auto as_state = Get<vvl::AccelerationStructureNV>(pnext_struct->pAccelerationStructures[j]);
        if (!as_state) continue;
        if (as_state->create_info.info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV) {
            skip |=
                LogError("VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-03748", as_state->Handle(),
                         write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureNV, Field::pAccelerationStructures, j),
                         "was created with %s.", string_VkAccelerationStructureTypeKHR(as_state->create_info.info.type));
        }
    }

    return skip;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool CoreChecks::VerifyWriteUpdateContents(const vvl::DescriptorSet &dst_set, const VkWriteDescriptorSet &update,
                                           const Location &write_loc, bool is_push_descriptor) const {
    bool skip = false;

    switch (update.descriptorType) {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            if (!update.pImageInfo) break;
            auto iter = dst_set.FindDescriptor(update.dstBinding, update.dstArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount && !iter.AtEnd(); ++di, ++iter) {
                const vvl::ImageSamplerDescriptor &desc = (const vvl::ImageSamplerDescriptor &)*iter;
                const Location image_info_loc = write_loc.dot(Field::pImageInfo, di);
                // Validate image
                const VkImageView image_view = update.pImageInfo[di].imageView;
                if (image_view == VK_NULL_HANDLE) {
                    if (desc.IsImmutableSampler()) {
                        // Only hit if using nullDescriptor
                        const LogObjectList objlist(update.dstSet, desc.GetSampler());
                        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-09506", objlist,
                                         image_info_loc.dot(Field::imageView), "is VK_NULL_HANDLE.");
                    }
                    continue;
                }
                const VkImageLayout image_layout = update.pImageInfo[di].imageLayout;
                const VkSampler sampler = update.pImageInfo[di].sampler;
                auto iv_state = Get<vvl::ImageView>(image_view);
                ASSERT_AND_CONTINUE(iv_state);

                const auto *image_state = iv_state->image_state.get();
                skip |= ValidateImageUpdate(*iv_state, image_layout, update.descriptorType, image_info_loc);

                if (desc.IsImmutableSampler()) {
                    auto sampler_state = Get<vvl::Sampler>(desc.GetSampler());
                    if (iv_state && sampler_state) {
                        if (iv_state->samplerConversion != sampler_state->samplerConversion) {
                            const LogObjectList objlist(update.dstSet, desc.GetSampler(), iv_state->Handle());
                            skip |= LogError(
                                "VUID-VkWriteDescriptorSet-descriptorType-01948", objlist, image_info_loc.dot(Field::sampler),
                                "was created with %s which is not identical to %s that was used to create the imageView.",
                                FormatHandle(iv_state->samplerConversion).c_str(),
                                FormatHandle(sampler_state->samplerConversion).c_str());
                        }
                    }
                } else if (iv_state && (iv_state->samplerConversion != VK_NULL_HANDLE)) {
                    const LogObjectList objlist(update.dstSet, iv_state->Handle());
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02738", objlist, write_loc.dot(Field::dstSet),
                                     "is bound to image view that includes a YCbCr conversion, it must have been allocated "
                                     "with a layout that includes an immutable sampler.");
                }

                // If there is an immutable sampler then |sampler| isn't used, so the following VU does not apply.
                if (sampler && !desc.IsImmutableSampler() && vkuFormatIsMultiplane(image_state->create_info.format)) {
                    // multiplane formats must be created with mutable format bit
                    const VkFormat image_format = image_state->create_info.format;
                    if (0 == (image_state->create_info.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                        const LogObjectList objlist(update.dstSet, image_state->Handle());
                        skip |= LogError("VUID-VkDescriptorImageInfo-sampler-01564", objlist, write_loc,
                                         "combined image sampler is a multi-planar format %s and was created with %s.",
                                         string_VkFormat(image_format),
                                         string_VkImageCreateFlags(image_state->create_info.flags).c_str());
                    }
                    const VkImageAspectFlags image_aspect = iv_state->create_info.subresourceRange.aspectMask;
                    if (!IsValidPlaneAspect(image_format, image_aspect)) {
                        const LogObjectList objlist(update.dstSet, image_state->Handle(), iv_state->Handle());
                        skip |= LogError("VUID-VkDescriptorImageInfo-sampler-01564", objlist, write_loc,
                                         "combined image sampler is a multi-planar format %s and imageView aspectMask is %s.",
                                         string_VkFormat(image_format), string_VkImageAspectFlags(image_aspect).c_str());
                    }
                }

                // Verify portability
                if (auto sampler_state = Get<vvl::Sampler>(sampler)) {
                    if (IsExtEnabled(extensions.vk_khr_portability_subset)) {
                        if ((VK_FALSE == enabled_features.mutableComparisonSamplers) &&
                            (VK_FALSE != sampler_state->create_info.compareEnable)) {
                            skip |= LogError("VUID-VkDescriptorImageInfo-mutableComparisonSamplers-04450", device, write_loc,
                                             "(portability error): sampler comparison not available.");
                        }
                    }
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            auto iter = dst_set.FindDescriptor(update.dstBinding, update.dstArrayElement);
            const vvl::SamplerDescriptor &desc = (const vvl::SamplerDescriptor &)*iter;
            if (desc.IsImmutableSampler() && !is_push_descriptor) {
                skip |=
                    LogError("VUID-VkWriteDescriptorSet-descriptorType-02752", update.dstSet, write_loc.dot(Field::descriptorType),
                             "is VK_DESCRIPTOR_TYPE_SAMPLER but can't update the immutable sampler from %s.",
                             FormatHandle(dst_set.GetLayout().get()->Handle()).c_str());
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM: {
            if (!update.pImageInfo) break;
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                const VkImageView image_view = update.pImageInfo[di].imageView;
                auto image_layout = update.pImageInfo[di].imageLayout;
                if (auto iv_state = Get<vvl::ImageView>(image_view)) {
                    skip |=
                        ValidateImageUpdate(*iv_state, image_layout, update.descriptorType, write_loc.dot(Field::pImageInfo, di));
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                const VkBufferView buffer_view_handle = update.pTexelBufferView[di];
                if (buffer_view_handle == VK_NULL_HANDLE) continue;

                auto bv_state = Get<vvl::BufferView>(buffer_view_handle);
                if (!bv_state) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02994", device,
                                     write_loc.dot(Field::pTexelBufferView, di),
                                     "(%s) is an invalid VkBufferView (while trying to update a descriptorType of %s).",
                                     FormatHandle(buffer_view_handle).c_str(), string_VkDescriptorType(update.descriptorType));
                    break;
                }
                const VkBuffer buffer_handle = bv_state->create_info.buffer;
                auto buffer_state = Get<vvl::Buffer>(buffer_handle);
                // Verify that buffer underlying the view hasn't been destroyed prematurely
                if (!buffer_state) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02994", buffer_view_handle,
                                     write_loc.dot(Field::pTexelBufferView, di),
                                     "was craated with an invalid buffer %s (while trying to update a descriptorType of %s).",
                                     FormatHandle(buffer_handle).c_str(), string_VkDescriptorType(update.descriptorType));
                    break;
                }
                skip |= ValidateBufferUsage(*buffer_state, update.descriptorType, write_loc.dot(Field::pTexelBufferView, di));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            if (!update.pBufferInfo) break;
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                skip |= ValidateBufferUpdate(update.pBufferInfo[di], update.descriptorType, write_loc.dot(Field::pBufferInfo, di));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
            const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
            if (!acc_info) break;
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                VkAccelerationStructureNV as = acc_info->pAccelerationStructures[di];
                // nullDescriptor feature allows this to be VK_NULL_HANDLE
                if (auto as_state = Get<vvl::AccelerationStructureNV>(as)) {
                    skip |= VerifyBoundMemoryIsValid(
                        as_state->MemoryState(), LogObjectList(as), as_state->Handle(),
                        write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureNV, Field::pAccelerationStructures, di),
                        kVUIDUndefined);
                }
            }

        } break;
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            // TODO
            break;
        // KHR acceleration structures don't require memory to be bound manually to them.
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        case VK_DESCRIPTOR_TYPE_MAX_ENUM:
            break;
    }
    return skip;
}

bool CoreChecks::ValidateCmdSetDescriptorBufferOffsets(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout,
                                                       uint32_t firstSet, uint32_t setCount, const uint32_t *pBufferIndices,
                                                       const VkDeviceSize *pOffsets, const Location &loc) const {
    bool skip = false;
    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) return skip;  // dynamicPipelineLayout

    const bool is_2 = loc.function != Func::vkCmdSetDescriptorBufferOffsetsEXT;

    if (!enabled_features.descriptorBuffer) {
        const char *vuid = is_2 ? "VUID-vkCmdSetDescriptorBufferOffsets2EXT-descriptorBuffer-09470"
                                : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-None-08060";
        skip |= LogError(vuid, cb_state.Handle(), loc, "descriptorBuffer feature was not enabled.");
    }

    if ((firstSet + setCount) > pipeline_layout->set_layouts.size()) {
        const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-firstSet-08066"
                                : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-08066";
        skip |= LogError(vuid, cb_state.Handle(), loc,
                         "The sum of firstSet (%" PRIu32 ") and setCount (%" PRIu32
                         ") is greater than VkPipelineLayoutCreateInfo::setLayoutCount (%" PRIuLEAST64 ") when layout was created.",
                         firstSet, setCount, (uint64_t)pipeline_layout->set_layouts.size());

        // Clamp so that we don't attempt to access invalid stuff
        setCount = std::min(setCount, static_cast<uint32_t>(pipeline_layout->set_layouts.size()));
    }

    for (uint32_t i = 0; i < setCount; i++) {
        const auto set_layout = pipeline_layout->set_layouts[firstSet + i];
        if ((set_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) == 0) {
            const LogObjectList objlist(cb_state.Handle(), set_layout->Handle(), pipeline_layout->Handle());
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-firstSet-09006"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-09006";
            skip |= LogError(vuid, objlist, loc,
                             "Descriptor set layout (%s) for set %" PRIu32
                             " was created without VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT flag set.",
                             FormatHandle(set_layout->Handle()).c_str(), firstSet + i);
            continue;
        }

        bool valid_buffer = false;
        const VkDeviceAddress offset = pOffsets[i];
        const uint32_t buffer_index = pBufferIndices[i];
        if (buffer_index < cb_state.descriptor_buffer_binding_info.size()) {
            bool valid_binding = false;
            const VkDeviceAddress start = cb_state.descriptor_buffer_binding_info[buffer_index].address;
            const auto buffer_states = GetBuffersByAddress(start);

            if (!buffer_states.empty()) {
                const auto buffer_state_starts = GetBuffersByAddress(start + offset);

                if (!buffer_state_starts.empty()) {
                    const auto bindings = set_layout->GetBindings();
                    const auto pSetLayoutSize = set_layout->GetLayoutSizeInBytes();
                    VkDeviceSize setLayoutSize = 0;

                    if (pSetLayoutSize == nullptr) {
                        const auto pool = cb_state.command_pool;
                        DispatchGetDescriptorSetLayoutSizeEXT(pool->dev_data.device, set_layout->VkHandle(), &setLayoutSize);
                    } else {
                        setLayoutSize = *pSetLayoutSize;
                    }

                    if (setLayoutSize > 0) {
                        // It looks like enough to check last binding in set
                        for (uint32_t j = 0; j < set_layout->GetBindingCount(); j++) {
                            const VkDescriptorBindingFlags flags = set_layout->GetDescriptorBindingFlagsFromIndex(j);
                            const bool vdc = (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) != 0;

                            if (vdc) {
                                // If a binding is VARIABLE_DESCRIPTOR_COUNT, the effective setLayoutSize we
                                // must validate is just the offset of the last binding.
                                const auto pool = cb_state.command_pool;
                                uint32_t binding = set_layout->GetDescriptorSetLayoutBindingPtrFromIndex(j)->binding;
                                DispatchGetDescriptorSetLayoutBindingOffsetEXT(pool->dev_data.device, set_layout->VkHandle(),
                                                                               binding, &setLayoutSize);

                                // If the descriptor set only consists of VARIABLE_DESCRIPTOR_COUNT bindings, the
                                // offset may be 0. In this case, treat the descriptor set layout as size 1,
                                // so we validate that the offset is sensible.
                                if (set_layout->GetBindingCount() == 1) {
                                    setLayoutSize = 1;
                                }

                                // There can only be one binding with VARIABLE_COUNT.
                                break;
                            }
                        }
                    }

                    if (setLayoutSize > 0) {
                        const auto buffer_state_ends = GetBuffersByAddress(start + offset + setLayoutSize - 1);
                        if (!buffer_state_ends.empty()) {
                            valid_binding = true;
                        }
                    }
                }

                valid_buffer = true;
            }

            if (!valid_binding) {
                const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pOffsets-08063"
                                        : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08063";
                skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pOffsets, i),
                                 "%" PRIuLEAST64
                                 " must be small enough such that any descriptor binding"
                                 " referenced by layout without the VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT"
                                 " flag computes a valid address inside the underlying VkBuffer",
                                 pOffsets[i]);
            }
        }

        if (!valid_buffer) {
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pBufferIndices-08065"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08065";
            skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pBufferIndices, i),
                             "(%" PRIu32
                             ") Each element of pBufferIndices must reference a valid descriptor buffer binding "
                             "set by a previous call to vkCmdBindDescriptorBuffersEXT in commandBuffer",
                             pBufferIndices[i]);
        }

        if (pBufferIndices[i] >= phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings) {
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pBufferIndices-08064"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08064";
            skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pBufferIndices, i),
                             "(%" PRIu32
                             ") "
                             "is greater than maxDescriptorBufferBindings (%" PRIu32 ") ",
                             pBufferIndices[i], phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings);
        }

        if (SafeModulo(offset, phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment) != 0) {
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pOffsets-08061"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08061";
            skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pOffsets, i),
                             "(%" PRIuLEAST64
                             ") is not aligned to descriptorBufferOffsetAlignment"
                             " (%" PRIuLEAST64 ")",
                             offset, phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t firstSet, uint32_t setCount,
                                                                 const uint32_t *pBufferIndices, const VkDeviceSize *pOffsets,
                                                                 const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    bool skip = false;
    skip |= ValidatePipelineBindPoint(*cb_state, pipelineBindPoint, error_obj.location);
    skip |=
        ValidateCmdSetDescriptorBufferOffsets(*cb_state, layout, firstSet, setCount, pBufferIndices, pOffsets, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
    const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmdSetDescriptorBufferOffsets(
        *cb_state, pSetDescriptorBufferOffsetsInfo->layout, pSetDescriptorBufferOffsetsInfo->firstSet,
        pSetDescriptorBufferOffsetsInfo->setCount, pSetDescriptorBufferOffsetsInfo->pBufferIndices,
        pSetDescriptorBufferOffsetsInfo->pOffsets, error_obj.location);

    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::ValidateCmdBindDescriptorBufferEmbeddedSamplers(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout,
                                                                 uint32_t set, const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT;

    if (!enabled_features.descriptorBuffer) {
        const char *vuid = is_2 ? "VUID-vkCmdBindDescriptorBufferEmbeddedSamplers2EXT-descriptorBuffer-09472"
                                : "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-None-08068";
        skip |= LogError(vuid, cb_state.Handle(), loc, "descriptorBuffer feature was not enabled.");
    }

    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) return skip;  // dynamicPipelineLayout

    if (set >= pipeline_layout->set_layouts.size()) {
        const char *vuid = is_2 ? "VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-set-08071"
                                : "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08071";
        skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::set),
                         "(%" PRIu32
                         ") is greater than "
                         "VkPipelineLayoutCreateInfo::setLayoutCount (%" PRIuLEAST64 ") when layout was created.",
                         set, (uint64_t)pipeline_layout->set_layouts.size());
    } else {
        auto set_layout = pipeline_layout->set_layouts[set];
        if (!(set_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT)) {
            const char *vuid = is_2 ? "VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-set-08070"
                                    : "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08070";
            skip |= LogError(vuid, cb_state.Handle(), loc,
                             "layout must have been created with the "
                             "VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT flag set.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                           VkPipelineBindPoint pipelineBindPoint,
                                                                           VkPipelineLayout layout, uint32_t set,
                                                                           const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidatePipelineBindPoint(*cb_state, pipelineBindPoint, error_obj.location);
    skip |= ValidateCmdBindDescriptorBufferEmbeddedSamplers(*cb_state, layout, set, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
    const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmdBindDescriptorBufferEmbeddedSamplers(*cb_state, pBindDescriptorBufferEmbeddedSamplersInfo->layout,
                                                            pBindDescriptorBufferEmbeddedSamplersInfo->set, error_obj.location);

    if (IsStageInPipelineBindPoint(pBindDescriptorBufferEmbeddedSamplersInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorBufferEmbeddedSamplersInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorBufferEmbeddedSamplersInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                            const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                            const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    bool skip = false;

    // A "descriptor buffer" binding is seperate from a "VkBuffer" so you can have the same address to the same VkBuffer and it will
    // count as 2, not 1, towards the limit. (more info at https://gitlab.khronos.org/vulkan/vulkan/-/issues/4086)
    std::vector<VkBuffer> sampler_buffers;
    std::vector<VkBuffer> resource_buffers;
    std::vector<VkBuffer> push_descriptor_buffers;

    for (uint32_t i = 0; i < bufferCount; i++) {
        const Location binding_loc = error_obj.location.dot(Field::pBindingInfos, i);
        const VkDescriptorBufferBindingInfoEXT &binding_info = pBindingInfos[i];
        VkBufferUsageFlags2 buffer_usage = binding_info.usage;
        if (const auto usage_flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(binding_info.pNext)) {
            buffer_usage = usage_flags2->usage;
        }
        const auto buffer_states = GetBuffersByAddress(binding_info.address);
        // Try to find a valid buffer in buffer_states.
        // If none if found, output each violated VUIDs, with the list of buffers that violate it.
        {
            BufferAddressValidation<5> buffer_address_validator = {{{
                {"VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08052",
                 [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                     return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
                 },
                 []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }},

                {"VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055",
                 [buffer_usage](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                     if ((buffer_state->usage &
                          (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                           VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)) !=
                         (buffer_usage &
                          (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                           VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT))) {
                         if (out_error_msg) {
                             *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                         }
                         return false;
                     }
                     return true;
                 },
                 [buffer_usage, i]() {
                     return "The following buffers have a usage that does not match pBindingInfos[" + std::to_string(i) +
                            "].usage (" + string_VkBufferUsageFlags2(buffer_usage) + "):";
                 }},

                {"VUID-VkDescriptorBufferBindingInfoEXT-usage-08122",
                 [buffer_usage, &sampler_buffers](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                     if (buffer_usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) {
                         sampler_buffers.push_back(buffer_state->VkHandle());
                         if (!(buffer_state->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT)) {
                             if (out_error_msg) {
                                 *out_error_msg += "has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                             }
                             return false;
                         }
                     }
                     return true;
                 },
                 []() { return "The following buffers were not created with VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT:"; }},

                {"VUID-VkDescriptorBufferBindingInfoEXT-usage-08123",
                 [buffer_usage, &resource_buffers](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                     if (buffer_usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) {
                         resource_buffers.push_back(buffer_state->VkHandle());
                         if (!(buffer_state->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)) {
                             if (out_error_msg) {
                                 *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                             }
                             return false;
                         }
                     }
                     return true;
                 },
                 []() {
                     return "The following buffers were not created with VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT:";
                 }},

                {"VUID-VkDescriptorBufferBindingInfoEXT-usage-08124",
                 [buffer_usage, &push_descriptor_buffers](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                     if (buffer_usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) {
                         push_descriptor_buffers.push_back(buffer_state->VkHandle());
                         if (!(buffer_state->usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)) {
                             if (out_error_msg) {
                                 *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                             }
                             return false;
                         }
                     }
                     return true;
                 },
                 []() {
                     return "The following buffers were not created with "
                            "VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT:";
                 }},
            }}};

            skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states, binding_loc.dot(Field::address),
                                                                      LogObjectList(device), binding_info.address);
        }

        const auto *buffer_handle =
            vku::FindStructInPNextChain<VkDescriptorBufferBindingPushDescriptorBufferHandleEXT>(pBindingInfos[i].pNext);
        if (!phys_dev_ext_props.descriptor_buffer_props.bufferlessPushDescriptors &&
            (pBindingInfos[i].usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) && !buffer_handle) {
            skip |= LogError("VUID-VkDescriptorBufferBindingInfoEXT-bufferlessPushDescriptors-08056", commandBuffer,
                             binding_loc.dot(Field::pNext),
                             "does not contain a VkDescriptorBufferBindingPushDescriptorBufferHandleEXT structure, but "
                             "bufferlessPushDescriptors is VK_FALSE and usage "
                             "contains VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT");
        }

        if (SafeModulo(pBindingInfos[i].address, phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment) != 0) {
            skip |= LogError("VUID-VkDescriptorBufferBindingInfoEXT-address-08057", commandBuffer, binding_loc.dot(Field::address),
                             "(%" PRIuLEAST64
                             ") is not aligned "
                             "to descriptorBufferOffsetAlignment (%" PRIuLEAST64 ")",
                             pBindingInfos[i].address, phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment);
        }

        if (buffer_handle && phys_dev_ext_props.descriptor_buffer_props.bufferlessPushDescriptors) {
            skip |= LogError("VUID-VkDescriptorBufferBindingPushDescriptorBufferHandleEXT-bufferlessPushDescriptors-08059",
                             commandBuffer, binding_loc.dot(Field::pNext),
                             "contains a VkDescriptorBufferBindingPushDescriptorBufferHandleEXT structure, "
                             "but bufferlessPushDescriptors is VK_TRUE");
        }
    }

    auto list_buffers = [this](std::vector<VkBuffer> &buffer_list) {
        vvl::unordered_set<VkBuffer> unique_buffers;
        std::stringstream msg;
        for (const VkBuffer &buffer : buffer_list) {
            msg << FormatHandle(buffer) << '\n';
            unique_buffers.insert(buffer);
        }
        if (unique_buffers.size() != buffer_list.size()) {
            msg << "Addresses pointing to the same VkBuffer still count as multiple 'descriptor buffer bindings' towards the "
                   "limits.\n";
        }
        return msg.str();
    };

    if (sampler_buffers.size() > phys_dev_ext_props.descriptor_buffer_props.maxSamplerDescriptorBufferBindings) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-maxSamplerDescriptorBufferBindings-08048", commandBuffer,
                         error_obj.location,
                         "Number of sampler buffers is %zu. There must be no more than "
                         "maxSamplerDescriptorBufferBindings (%" PRIu32
                         ") descriptor buffers containing sampler descriptor data bound. List of sampler buffers:\n%s",
                         sampler_buffers.size(), phys_dev_ext_props.descriptor_buffer_props.maxSamplerDescriptorBufferBindings,
                         list_buffers(sampler_buffers).c_str());
    }

    if (resource_buffers.size() > phys_dev_ext_props.descriptor_buffer_props.maxResourceDescriptorBufferBindings) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-maxResourceDescriptorBufferBindings-08049", commandBuffer,
                         error_obj.location,
                         "Number of resource buffers is %zu. There must be no more than "
                         "maxResourceDescriptorBufferBindings (%" PRIu32
                         ") descriptor buffers containing resource descriptor data bound. List of resource buffers:\n%s",
                         resource_buffers.size(), phys_dev_ext_props.descriptor_buffer_props.maxResourceDescriptorBufferBindings,
                         list_buffers(resource_buffers).c_str());
    }

    if (push_descriptor_buffers.size() > 1) {
        skip |= LogError(
            "VUID-vkCmdBindDescriptorBuffersEXT-None-08050", commandBuffer, error_obj.location,
            "Number of push descriptor buffers is %zu. "
            "There must be no more than 1 push descriptor buffer bound that was created "
            "with the VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT bit set. List of push descriptor buffers:\n%s",
            push_descriptor_buffers.size(), list_buffers(push_descriptor_buffers).c_str());
    }

    if (bufferCount > phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-bufferCount-08051", commandBuffer,
                         error_obj.location.dot(Field::bufferCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceDescriptorBufferPropertiesEXT::maxDescriptorBufferBindings (%" PRIu32 ").",
                         bufferCount, phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                              VkDeviceSize *pLayoutSizeInBytes,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBuffer) {
        skip |= LogError("VUID-vkGetDescriptorSetLayoutSizeEXT-None-08011", device, error_obj.location,
                         "descriptorBuffer feature was not enabled.");
    }

    if (auto ds_layout_state = Get<vvl::DescriptorSetLayout>(layout)) {
        const auto create_flags = ds_layout_state->GetCreateFlags();
        if (!(create_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
            skip |= LogError("VUID-vkGetDescriptorSetLayoutSizeEXT-layout-08012", layout, error_obj.location.dot(Field::layout),
                             "was created with %s.", string_VkDescriptorSetLayoutCreateFlags(create_flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                       uint32_t binding, VkDeviceSize *pOffset,
                                                                       const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBuffer) {
        skip |= LogError("VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-None-08013", device, error_obj.location,
                         "descriptorBuffer feature was not enabled.");
    }

    if (auto ds_layout_state = Get<vvl::DescriptorSetLayout>(layout)) {
        const auto create_flags = ds_layout_state->GetCreateFlags();
        if (!(ds_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
            skip |= LogError("VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-layout-08014", layout,
                             error_obj.location.dot(Field::layout), "was created with %s.",
                             string_VkDescriptorSetLayoutCreateFlags(create_flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkBufferCaptureDescriptorDataInfoEXT *pInfo,
                                                                        void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-None-08072", pInfo->buffer, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-device-08074", pInfo->buffer, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    if (auto buffer_state = Get<vvl::Buffer>(pInfo->buffer)) {
        if (!(buffer_state->create_info.flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkBufferCaptureDescriptorDataInfoEXT-buffer-08075", pInfo->buffer,
                             error_obj.location.dot(Field::pInfo).dot(Field::buffer), "was created with %s.",
                             string_VkBufferCreateFlags(buffer_state->create_info.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                       const VkImageCaptureDescriptorDataInfoEXT *pInfo,
                                                                       void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-None-08076", pInfo->image, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-device-08078", pInfo->image, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    if (auto image_state = Get<vvl::Image>(pInfo->image)) {
        if (!(image_state->create_info.flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkImageCaptureDescriptorDataInfoEXT-image-08079", pInfo->image,
                             error_obj.location.dot(Field::pInfo).dot(Field::image), "is %s.",
                             string_VkImageCreateFlags(image_state->create_info.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                           const VkImageViewCaptureDescriptorDataInfoEXT *pInfo,
                                                                           void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-None-08080", pInfo->imageView, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-device-08082", pInfo->imageView, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    if (auto image_view_state = Get<vvl::ImageView>(pInfo->imageView)) {
        if (!(image_view_state->create_info.flags & VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkImageViewCaptureDescriptorDataInfoEXT-imageView-08083", pInfo->imageView,
                             error_obj.location.dot(Field::pInfo).dot(Field::imageView), "is %s.",
                             string_VkImageViewCreateFlags(image_view_state->create_info.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkSamplerCaptureDescriptorDataInfoEXT *pInfo,
                                                                         void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-None-08084", pInfo->sampler, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-device-08086", pInfo->sampler, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    if (auto sampler_state = Get<vvl::Sampler>(pInfo->sampler)) {
        if (!(sampler_state->create_info.flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkSamplerCaptureDescriptorDataInfoEXT-sampler-08087", pInfo->sampler,
                             error_obj.location.dot(Field::pInfo).dot(Field::sampler), "is %s.",
                             string_VkSamplerCreateFlags(sampler_state->create_info.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT *pInfo, void *pData,
    const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-None-08088", device, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-device-08090", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    if (pInfo->accelerationStructure != VK_NULL_HANDLE) {
        if (auto acceleration_structure_state = Get<vvl::AccelerationStructureKHR>(pInfo->accelerationStructure)) {
            if (!(acceleration_structure_state->create_info.createFlags &
                  VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                skip |= LogError(
                    "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-08091",
                    pInfo->accelerationStructure, error_obj.location, "pInfo->accelerationStructure was %s.",
                    string_VkAccelerationStructureCreateFlagsKHR(acceleration_structure_state->create_info.createFlags).c_str());
            }
        }

        if (pInfo->accelerationStructureNV != VK_NULL_HANDLE) {
            LogError("VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-08093", device,
                     error_obj.location,
                     "If accelerationStructure is not VK_NULL_HANDLE, accelerationStructureNV must be VK_NULL_HANDLE. ");
        }
    }

    if (pInfo->accelerationStructureNV != VK_NULL_HANDLE) {
        if (auto acceleration_structure_state = Get<vvl::AccelerationStructureNV>(pInfo->accelerationStructureNV)) {
            if (!(acceleration_structure_state->create_info.info.flags &
                  VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                skip |= LogError(
                    "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructureNV-08092",
                    pInfo->accelerationStructureNV, error_obj.location, "pInfo->accelerationStructure was %s.",
                    string_VkAccelerationStructureCreateFlagsKHR(acceleration_structure_state->create_info.info.flags).c_str());
            }
        }

        if (pInfo->accelerationStructure != VK_NULL_HANDLE) {
            LogError("VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructureNV-08094", device,
                     error_obj.location,
                     "If accelerationStructureNV is not VK_NULL_HANDLE, accelerationStructure must be VK_NULL_HANDLE. ");
        }
    }

    return skip;
}

bool CoreChecks::ValidateDescriptorAddressInfoEXT(const VkDescriptorAddressInfoEXT *address_info,
                                                  const Location &address_loc) const {
    bool skip = false;

    if (address_info->range == 0) {
        skip |= LogError("VUID-VkDescriptorAddressInfoEXT-range-08940", device, address_loc.dot(Field::range), "is zero.");
    }

    if (address_info->address == 0) {
        if (!enabled_features.nullDescriptor) {
            skip |= LogError("VUID-VkDescriptorAddressInfoEXT-address-08043", device, address_loc.dot(Field::address),
                             "is zero, but the nullDescriptor feature was not enabled.");
        } else if (address_info->range != VK_WHOLE_SIZE) {
            skip |= LogError("VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08938", device, address_loc.dot(Field::range),
                             "(%" PRIu64 ") is not VK_WHOLE_SIZE, but address is zero.", address_info->range);
        }
    } else {
        if (address_info->range == VK_WHOLE_SIZE) {
            skip |= LogError("VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08939", device, address_loc.dot(Field::range),
                             "is VK_WHOLE_SIZE.");
        }
    }

    const auto buffer_states = GetBuffersByAddress(address_info->address);
    if ((address_info->address != 0) && buffer_states.empty()) {
        skip |= LogError("VUID-VkDescriptorAddressInfoEXT-None-08044", device, address_loc.dot(Field::address),
                         "(0x%" PRIx64 ") is not a valid buffer address.", address_info->address);
    } else {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-VkDescriptorAddressInfoEXT-range-08045",
               [&address_info](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                   if (address_info->range >
                       buffer_state->create_info.size - (address_info->address - buffer_state->deviceAddress)) {
                       if (out_error_msg) {
                           const sparse_container::range<VkDeviceAddress> buffer_address_range{
                               buffer_state->deviceAddress, buffer_state->deviceAddress + buffer_state->create_info.size};
                           *out_error_msg += "buffer has range " + string_range_hex(buffer_address_range);
                       }
                       return false;
                   }
                   return true;
               },
               [&address_info]() {
                   const sparse_container::range<VkDeviceAddress> address_range{address_info->address,
                                                                                address_info->address + address_info->range};
                   return "The following buffers do not contain address range " + string_range_hex(address_range) + ":";
               }}}}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states, address_loc.dot(Field::address),
                                                                  LogObjectList(device), address_info->address);
    }

    return skip;
}

bool CoreChecks::ValidateGetDescriptorDataSize(const VkDescriptorGetInfoEXT &descriptor_info, const size_t data_size,
                                               const Location &descriptor_info_loc) const {
    bool skip = false;

    size_t size = 0u;
    Struct struct_name = Struct::VkPhysicalDeviceDescriptorBufferPropertiesEXT;
    Field field_name = Field::Empty;

    switch (descriptor_info.type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            size = phys_dev_ext_props.descriptor_buffer_props.samplerDescriptorSize;
            field_name = Field::samplerDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            size = phys_dev_ext_props.descriptor_buffer_props.combinedImageSamplerDescriptorSize;
            field_name = Field::combinedImageSamplerDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            size = phys_dev_ext_props.descriptor_buffer_props.sampledImageDescriptorSize;
            field_name = Field::sampledImageDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            size = phys_dev_ext_props.descriptor_buffer_props.storageImageDescriptorSize;
            field_name = Field::storageImageDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustUniformTexelBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.uniformTexelBufferDescriptorSize;
            field_name = enabled_features.robustBufferAccess ? Field::robustUniformTexelBufferDescriptorSize
                                                             : Field::uniformTexelBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustStorageTexelBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.storageTexelBufferDescriptorSize;
            field_name = enabled_features.robustBufferAccess ? Field::robustStorageTexelBufferDescriptorSize
                                                             : Field::storageTexelBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustUniformBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.uniformBufferDescriptorSize;
            field_name =
                enabled_features.robustBufferAccess ? Field::robustUniformBufferDescriptorSize : Field::uniformBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustStorageBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.storageBufferDescriptorSize;
            field_name =
                enabled_features.robustBufferAccess ? Field::robustStorageBufferDescriptorSize : Field::storageBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            size = phys_dev_ext_props.descriptor_buffer_props.inputAttachmentDescriptorSize;
            field_name = Field::inputAttachmentDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            size = phys_dev_ext_props.descriptor_buffer_props.accelerationStructureDescriptorSize;
            field_name = Field::accelerationStructureDescriptorSize;
            break;
        default:
            return skip;  // nothing to check, unknown descriptor ttype
            break;
    }

    const VkDescriptorImageInfo *combined_image_sampler = descriptor_info.data.pCombinedImageSampler;
    if (descriptor_info.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && combined_image_sampler) {
        if (combined_image_sampler->imageView == VK_NULL_HANDLE) {
            // Only hit if using nullDescriptor
            if (size != data_size) {
                skip |= LogError("VUID-vkGetDescriptorEXT-pDescriptorInfo-09507", device, descriptor_info_loc.dot(Field::type),
                                 "(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) and combinedImageSamplerDescriptorSize (%zu) not "
                                 "equal to dataSize %zu",
                                 phys_dev_ext_props.descriptor_buffer_props.combinedImageSamplerDescriptorSize, data_size);
            }
        } else {
            const auto image_view_state = Get<vvl::ImageView>(combined_image_sampler->imageView);
            if (image_view_state && image_view_state->samplerConversion != VK_NULL_HANDLE) {
                auto image_info = image_view_state->image_state->create_info;
                VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
                image_format_info.type = image_info.imageType;
                image_format_info.format = image_info.format;
                image_format_info.tiling = image_info.tiling;
                image_format_info.usage = image_view_state->inherited_usage;
                image_format_info.flags = image_info.flags;
                VkSamplerYcbcrConversionImageFormatProperties sampler_ycbcr_image_format_info = vku::InitStructHelper();
                VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&sampler_ycbcr_image_format_info);
                DispatchGetPhysicalDeviceImageFormatProperties2Helper(api_version, physical_device, &image_format_info,
                                                                      &image_format_properties);
                size *= static_cast<size_t>(sampler_ycbcr_image_format_info.combinedImageSamplerDescriptorCount);
                if (size != data_size) {
                    skip |= LogError("VUID-vkGetDescriptorEXT-descriptorType-09469", device, descriptor_info_loc.dot(Field::type),
                                     "(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) has %s and descriptor size is %zu "
                                     "[combinedImageSamplerDescriptorCount (%" PRIu32
                                     ") times combinedImageSamplerDescriptorSize (%zu)], but dataSize is %zu",
                                     FormatHandle(image_view_state->samplerConversion).c_str(), size,
                                     sampler_ycbcr_image_format_info.combinedImageSamplerDescriptorCount,
                                     phys_dev_ext_props.descriptor_buffer_props.combinedImageSamplerDescriptorSize, data_size);
                }
                return skip;  // the 08125 VU doesn't apply if we are using a SamplerYcbcrConversion
            }
        }

        if (combined_image_sampler->sampler != VK_NULL_HANDLE) {
            const auto sampler_state = Get<vvl::Sampler>(combined_image_sampler->sampler);
            if (sampler_state && (0 != (sampler_state->create_info.flags & VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT))) {
                size = phys_dev_ext_props.descriptor_buffer_density_props.combinedImageSamplerDensityMapDescriptorSize;
                struct_name = Struct::VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT;
                field_name = Field::combinedImageSamplerDensityMapDescriptorSize;
            }
        }
    }

    if (size != data_size) {
        skip |= LogError("VUID-vkGetDescriptorEXT-dataSize-08125", device, descriptor_info_loc.dot(Field::type),
                         "(%s) has a size of %zu (determined by %s::%s), but dataSize is %zu",
                         string_VkDescriptorType(descriptor_info.type), size, String(struct_name), String(field_name), data_size);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize,
                                                 void *pDescriptor, const ErrorObject &error_obj) const {
    bool skip = false;

    // update on first pass of switch case
    const VkDescriptorAddressInfoEXT *address_info = nullptr;
    Field data_field = Field::Empty;

    const Location descriptor_info_loc = error_obj.location.dot(Field::pDescriptorInfo);
    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            data_field = Field::pCombinedImageSampler;
            if (Get<vvl::Sampler>(pDescriptorInfo->data.pCombinedImageSampler->sampler).get() == nullptr) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08019", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but "
                                 "pCombinedImageSampler->sampler is not a valid sampler.");
            }
            if ((pDescriptorInfo->data.pCombinedImageSampler->imageView != VK_NULL_HANDLE) &&
                (Get<vvl::ImageView>(pDescriptorInfo->data.pCombinedImageSampler->imageView).get() == nullptr)) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08020", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but "
                                 "pCombinedImageSampler->imageView is not a valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            data_field = Field::pInputAttachmentImage;
            if (Get<vvl::ImageView>(pDescriptorInfo->data.pInputAttachmentImage->imageView).get() == nullptr) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08021", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, but "
                                 "pInputAttachmentImage->imageView is not valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            data_field = Field::pSampledImage;
            if (pDescriptorInfo->data.pSampledImage && (pDescriptorInfo->data.pSampledImage->imageView != VK_NULL_HANDLE) &&
                (Get<vvl::ImageView>(pDescriptorInfo->data.pSampledImage->imageView).get() == nullptr)) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08022", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but "
                                 "pSampledImage->imageView is not a valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            data_field = Field::pStorageImage;
            if (pDescriptorInfo->data.pStorageImage && (pDescriptorInfo->data.pStorageImage->imageView != VK_NULL_HANDLE) &&
                (Get<vvl::ImageView>(pDescriptorInfo->data.pStorageImage->imageView).get() == nullptr)) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08023", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but "
                                 "pStorageImage->imageView is not a valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            data_field = Field::pUniformTexelBuffer;
            address_info = pDescriptorInfo->data.pUniformTexelBuffer;
            if (address_info && (address_info->address != 0) && (GetBuffersByAddress(address_info->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08024", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, but "
                                 "pUniformTexelBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 address_info->address);
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            data_field = Field::pStorageTexelBuffer;
            address_info = pDescriptorInfo->data.pStorageTexelBuffer;
            if (address_info && (address_info->address != 0) && (GetBuffersByAddress(address_info->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08025", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, but "
                                 "pStorageTexelBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 address_info->address);
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            data_field = Field::pUniformBuffer;
            address_info = pDescriptorInfo->data.pUniformBuffer;
            if (address_info && (address_info->address != 0) && (GetBuffersByAddress(address_info->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08026", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, but "
                                 "pUniformBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 address_info->address);
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            data_field = Field::pStorageBuffer;
            address_info = pDescriptorInfo->data.pStorageBuffer;
            if (address_info && (address_info->address != 0) && (GetBuffersByAddress(address_info->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08027", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, but "
                                 "pStorageBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 address_info->address);
            }
            break;

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            data_field = Field::accelerationStructure;
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            data_field = Field::accelerationStructure;
            if (pDescriptorInfo->data.accelerationStructure) {
                const VkAccelerationStructureNV as = (VkAccelerationStructureNV)pDescriptorInfo->data.accelerationStructure;
                auto as_state = Get<vvl::AccelerationStructureNV>(as);

                if (!as_state) {
                    skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08029", device, descriptor_info_loc.dot(Field::type),
                                     "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV and accelerationStructure is not 0, "
                                     "accelerationStructure must contain the handle of a VkAccelerationStructureNV created on "
                                     "device, returned by vkGetAccelerationStructureHandleNV");
                }
            }
            break;

        default:
            break;
    }

    const Location data_loc = descriptor_info_loc.dot(Field::data);
    if (address_info && address_info->range != VK_WHOLE_SIZE &&
        (pDescriptorInfo->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
         pDescriptorInfo->type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
        const VkDeviceSize texels_per_block = static_cast<VkDeviceSize>(vkuFormatTexelsPerBlock(address_info->format));
        const VkDeviceSize texel_block_size = static_cast<VkDeviceSize>(GetTexelBufferFormatSize(address_info->format));
        const VkDeviceSize texels =
            SafeDivision(address_info->range, texel_block_size) * static_cast<VkDeviceSize>(texels_per_block);
        if (texels > static_cast<VkDeviceSize>(phys_dev_props.limits.maxTexelBufferElements)) {
            const char *vuid = pDescriptorInfo->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                                   ? "VUID-VkDescriptorGetInfoEXT-type-09427"
                                   : "VUID-VkDescriptorGetInfoEXT-type-09428";
            skip |= LogError(vuid, device, data_loc.dot(data_field).dot(Field::range),
                             "(%" PRIuLEAST64 "), %s texel block size (%" PRIuLEAST64 "), and texels per block (%" PRIuLEAST64
                             ") is a total of (%" PRIuLEAST64
                             ") texels which is more than VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                             address_info->range, string_VkFormat(address_info->format), texel_block_size, texels_per_block, texels,
                             phys_dev_props.limits.maxTexelBufferElements);
        }
    }

    const char *buffer_address_vuid = kVUIDUndefined;

    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            buffer_address_vuid = "VUID-VkDescriptorDataEXT-type-08030";
            if (!address_info && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08039", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, but "
                                 "pUniformBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            buffer_address_vuid = "VUID-VkDescriptorDataEXT-type-08031";
            if (!address_info && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08040", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, but "
                                 "pStorageBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            buffer_address_vuid = "VUID-VkDescriptorDataEXT-type-08032";
            if (!address_info && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08037", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, but "
                                 "pUniformTexelBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            buffer_address_vuid = "VUID-VkDescriptorDataEXT-type-08033";
            if (!address_info && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08038", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, but "
                                 "pStorageTexelBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            if ((pDescriptorInfo->data.accelerationStructure == 0) && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08041", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, but "
                                 "accelerationStructure is 0 and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            if ((pDescriptorInfo->data.accelerationStructure == 0) && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08042", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, but "
                                 "accelerationStructure is 0 and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            if ((pDescriptorInfo->data.pCombinedImageSampler->imageView == VK_NULL_HANDLE) && !enabled_features.nullDescriptor) {
                skip |=
                    LogError("VUID-VkDescriptorDataEXT-type-08034", device, descriptor_info_loc.dot(Field::type),
                             "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but "
                             "pCombinedImageSampler->imageView is VK_NULL_HANDLE and the nullDescriptor feature is not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            if (!enabled_features.nullDescriptor &&
                (!pDescriptorInfo->data.pSampledImage || (pDescriptorInfo->data.pSampledImage->imageView == VK_NULL_HANDLE))) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08035", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but "
                                 "pSampledImage is NULL, or pSampledImage->imageView is VK_NULL_HANDLE, and the nullDescriptor "
                                 "feature is not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            if (!enabled_features.nullDescriptor &&
                (!pDescriptorInfo->data.pStorageImage || (pDescriptorInfo->data.pStorageImage->imageView == VK_NULL_HANDLE))) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08036", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, but "
                                 "pStorageImage is NULL, or pStorageImage->imageView is VK_NULL_HANDLE, and the nullDescriptor "
                                 "feature is not enabled.");
            }
            break;
        default:
            break;
    }

    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (address_info) {
                skip |= ValidateDescriptorAddressInfoEXT(address_info, data_loc.dot(data_field));

                const auto buffer_states = GetBuffersByAddress(address_info->address);
                if (!buffer_states.empty()) {
                    BufferAddressValidation<1> buffer_address_validator = {
                        {{{buffer_address_vuid,
                           [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                               return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
                           },
                           []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};
                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states,
                                                                              data_loc.dot(data_field).dot(Field::address),
                                                                              LogObjectList(device), address_info->address);
                }
            }
            break;
        default:
            break;
    }

    skip |= ValidateGetDescriptorDataSize(*pDescriptorInfo, dataSize, descriptor_info_loc);

    return skip;
}

bool CoreChecks::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                    VkDescriptorPoolResetFlags flags, const ErrorObject &error_obj) const {
    // Make sure sets being destroyed are not currently in-use
    if (disabled[object_in_use]) return false;
    bool skip = false;
    if (auto ds_pool_state = Get<vvl::DescriptorPool>(descriptorPool)) {
        skip |= ValidateObjectNotInUse(ds_pool_state.get(), error_obj.location.dot(Field::descriptorPool),
                                       "VUID-vkResetDescriptorPool-descriptorPool-00313");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                      const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto ds_pool_state = Get<vvl::DescriptorPool>(descriptorPool)) {
        skip |=
            ValidateObjectNotInUse(ds_pool_state.get(), error_obj.location, "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    }
    return skip;
}

// Ensure the pool contains enough descriptors and descriptor sets to satisfy
// an allocation request. Fills common_data with the total number of descriptors of each type required,
// as well as DescriptorSetLayout ptrs used for later update.
bool CoreChecks::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                       VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj,
                                                       vvl::AllocateDescriptorSetsData &ds_data) const {
    BaseClass::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, error_obj, ds_data);

    bool skip = false;
    auto ds_pool_state = Get<vvl::DescriptorPool>(pAllocateInfo->descriptorPool);
    ASSERT_AND_RETURN_SKIP(ds_pool_state);

    const Location allocate_info_loc = error_obj.location.dot(Field::pAllocateInfo);

    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        const Location set_layout_loc = allocate_info_loc.dot(Field::pSetLayouts, i);
        auto ds_layout_state = Get<vvl::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
        // nullptr layout indicates no valid layout handle for this device, validated/logged in object_tracker
        if (!ds_layout_state) continue;

        if (ds_layout_state->IsPushDescriptor()) {
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308", pAllocateInfo->pSetLayouts[i], set_layout_loc,
                             "(%s) was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT. (For Push Descriptors, "
                             "you don't allocate a VkDescriptorSet and VkWriteDescriptorSet::dstSet is ignored)",
                             FormatHandle(pAllocateInfo->pSetLayouts[i]).c_str());
        }
        if (ds_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-08009", pAllocateInfo->pSetLayouts[i], set_layout_loc,
                             "(%s) was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.",
                             FormatHandle(pAllocateInfo->pSetLayouts[i]).c_str());
        }
        if (ds_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT &&
            !(ds_pool_state->create_info.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
            const LogObjectList objlist(pAllocateInfo->descriptorPool, pAllocateInfo->pSetLayouts[i]);
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044", objlist, set_layout_loc,
                             "was created with %s but the descriptorPool was created with %s",
                             string_VkDescriptorSetLayoutCreateFlags(ds_layout_state->GetCreateFlags()).c_str(),
                             string_VkDescriptorPoolCreateFlags(ds_pool_state->create_info.flags).c_str());
        }
        if (ds_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT &&
            !(ds_pool_state->create_info.flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) {
            const LogObjectList objlist(pAllocateInfo->descriptorPool, pAllocateInfo->pSetLayouts[i]);
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-04610", objlist, set_layout_loc,
                             "was created with %s but the descriptorPool was created with %s",
                             string_VkDescriptorSetLayoutCreateFlags(ds_layout_state->GetCreateFlags()).c_str(),
                             string_VkDescriptorPoolCreateFlags(ds_pool_state->create_info.flags).c_str());
        }
    }
    if (!IsExtEnabled(extensions.vk_khr_maintenance1)) {
        // Track number of descriptorSets allowable in this pool
        if (ds_pool_state->GetAvailableSets() < pAllocateInfo->descriptorSetCount) {
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-apiVersion-07895", ds_pool_state->Handle(), error_obj.location,
                             "Unable to allocate %" PRIu32
                             " descriptorSets from %s"
                             ". This pool only has %" PRIu32 " descriptorSets remaining.",
                             pAllocateInfo->descriptorSetCount, FormatHandle(*ds_pool_state).c_str(),
                             ds_pool_state->GetAvailableSets());
        }
        // Determine whether descriptor counts are satisfiable
        for (auto it = ds_data.required_descriptors_by_type.begin(); it != ds_data.required_descriptors_by_type.end(); ++it) {
            auto available_count = ds_pool_state->GetAvailableCount(it->first);

            if (ds_data.required_descriptors_by_type.at(it->first) > available_count) {
                skip |= LogError("VUID-VkDescriptorSetAllocateInfo-apiVersion-07896", ds_pool_state->Handle(), error_obj.location,
                                 "Unable to allocate %" PRIu32
                                 " descriptors of type %s from %s"
                                 ". This pool only has %" PRIu32 " descriptors of this type remaining.",
                                 ds_data.required_descriptors_by_type.at(it->first),
                                 string_VkDescriptorType(VkDescriptorType(it->first)), FormatHandle(*ds_pool_state).c_str(),
                                 available_count);
            }
        }
    }

    const auto *count_allocate_info =
        vku::FindStructInPNextChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(pAllocateInfo->pNext);
    if (count_allocate_info) {
        if (count_allocate_info->descriptorSetCount != 0 &&
            count_allocate_info->descriptorSetCount != pAllocateInfo->descriptorSetCount) {
            skip |= LogError(
                "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-descriptorSetCount-03045", device,
                allocate_info_loc.pNext(Struct::VkDescriptorSetVariableDescriptorCountAllocateInfo, Field::descriptorSetCount),
                "(%" PRIu32 ") != pAllocateInfo->descriptorSetCount (%" PRIu32 ").", count_allocate_info->descriptorSetCount,
                pAllocateInfo->descriptorSetCount);
        }
        if (count_allocate_info->descriptorSetCount == pAllocateInfo->descriptorSetCount) {
            for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
                auto ds_layout_state = Get<vvl::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
                ASSERT_AND_CONTINUE(ds_layout_state);
                if (count_allocate_info->pDescriptorCounts[i] >
                    ds_layout_state->GetDescriptorCountFromBinding(ds_layout_state->GetMaxBinding())) {
                    skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-09380", device,
                                     allocate_info_loc.pNext(Struct::VkDescriptorSetVariableDescriptorCountAllocateInfo,
                                                             Field::pDescriptorCounts, i),
                                     "is %" PRIu32 ", but pAllocateInfo->pSetLayouts[%" PRIu32
                                     "] binding's descriptorCount = (%" PRIu32 ")",
                                     count_allocate_info->pDescriptorCounts[i], i,
                                     ds_layout_state->GetDescriptorCountFromBinding(ds_layout_state->GetMaxBinding()));
                }
            }
        }
    }

    return skip;
}

void CoreChecks::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                      VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj,
                                                      vvl::AllocateDescriptorSetsData &ads_state) {
    // Discussed in https://gitlab.khronos.org/vulkan/vulkan/-/issues/3347
    // The issue if users see VK_ERROR_OUT_OF_POOL_MEMORY they think they over-allocated, but if they instead allocated type not
    // avaiable (so the pool size is zero), they will just keep getting this error mistakenly thinking they ran out. It was decided
    // that this deserves to be a Core Validation check
    if (record_obj.result == VK_ERROR_OUT_OF_POOL_MEMORY && pAllocateInfo) {
        // result type added in VK_KHR_maintenance1
        auto ds_pool_state = Get<vvl::DescriptorPool>(pAllocateInfo->descriptorPool);
        ASSERT_AND_RETURN(ds_pool_state);

        for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
            auto ds_layout_state = Get<vvl::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
            ASSERT_AND_CONTINUE(ds_layout_state);

            const uint32_t binding_count = ds_layout_state->GetBindingCount();
            for (uint32_t j = 0; j < binding_count; ++j) {
                const VkDescriptorType type = ds_layout_state->GetTypeFromIndex(j);
                if (!ds_pool_state->IsAvailableType(type)) {
                    // This check would be caught by validation if VK_KHR_maintenance1 was not enabled
                    LogWarning("WARNING-CoreValidation-AllocateDescriptorSets-WrongType", ds_pool_state->Handle(),
                               record_obj.location.dot(Field::pAllocateInfo).dot(Field::pSetLayouts, i),
                               "binding %" PRIu32
                               " was created with %s but the "
                               "Descriptor Pool was not created with this type and returned VK_ERROR_OUT_OF_POOL_MEMORY",
                               j, string_VkDescriptorType(type));
                }
            }
        }
    }

    BaseClass::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj, ads_state);
}

// Validate that given set is valid and that it's not being used by an in-flight CmdBuffer
// func_str is the name of the calling function
// Return false if no errors occur
// Return true if validation error occurs and callback returns true (to skip upcoming API call down the chain)
bool CoreChecks::ValidateIdleDescriptorSet(VkDescriptorSet set, const Location &loc) const {
    if (disabled[object_in_use]) return false;
    bool skip = false;
    if (auto set_node = Get<vvl::DescriptorSet>(set)) {
        skip |= ValidateObjectNotInUse(set_node.get(), loc, "VUID-vkFreeDescriptorSets-pDescriptorSets-00309");
    }
    return skip;
}

bool CoreChecks::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                   const VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj) const {
    // Make sure that no sets being destroyed are in-flight
    bool skip = false;
    // First make sure sets being destroyed are not currently in-use
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateIdleDescriptorSet(pDescriptorSets[i], error_obj.location.dot(Field::pDescriptorSets, i));
        }
    }
    auto ds_pool_state = Get<vvl::DescriptorPool>(descriptorPool);
    ASSERT_AND_RETURN_SKIP(ds_pool_state);
    if (!(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT & ds_pool_state->create_info.flags)) {
        // Can't Free from a NON_FREE pool
        skip |= LogError("VUID-vkFreeDescriptorSets-descriptorPool-00312", descriptorPool,
                         error_obj.location.dot(Field::descriptorPool), "with a pool created with %s.",
                         string_VkDescriptorPoolCreateFlags(ds_pool_state->create_info.flags).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                     const VkCopyDescriptorSet *pDescriptorCopies,
                                                     const ErrorObject &error_obj) const {
    // First thing to do is perform map look-ups.
    // NOTE : UpdateDescriptorSets is somewhat unique in that it's operating on a number of DescriptorSets
    //  so we can't just do a single map look-up up-front, but do them individually in functions below

    // Now make call(s) that validate state, but don't perform state updates in this function
    // Note, here DescriptorSets is unique in that we don't yet have an instance. Using a helper function in the
    //  namespace which will parse params and make calls into specific class instances
    return ValidateUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies,
                                        error_obj.location);
}

bool CoreChecks::ValidateCmdPushDescriptorSet(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout, uint32_t set,
                                              uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                                              const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdPushDescriptorSetKHR && loc.function != Func::vkCmdPushDescriptorSet;

    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) return skip;  // dynamicPipelineLayout

    // Validate the set index points to a push descriptor set and is in range
    const auto &set_layouts = pipeline_layout->set_layouts;
    if (set >= set_layouts.size()) {
        const char *vuid = is_2 ? "VUID-VkPushDescriptorSetInfo-set-00364" : "VUID-vkCmdPushDescriptorSet-set-00364";
        const LogObjectList objlist(cb_state.Handle(), layout);
        skip |= LogError(vuid, objlist, loc.dot(Field::set),
                         "(%" PRIu32 ") is indexing outside the range for %s (which had a setLayoutCount of only %" PRIu32 ").",
                         set, FormatHandle(layout).c_str(), static_cast<uint32_t>(set_layouts.size()));
        return skip;
    }

    const auto &dsl = set_layouts[set];
    ASSERT_AND_RETURN_SKIP(dsl);

    if (!dsl->IsPushDescriptor()) {
        const char *vuid = is_2 ? "VUID-VkPushDescriptorSetInfo-set-00365" : "VUID-vkCmdPushDescriptorSet-set-00365";
        const LogObjectList objlist(cb_state.Handle(), layout);
        skip |= LogError(vuid, objlist, loc.dot(Field::set),
                         "(%" PRIu32
                         ") points to %s inside %s which is not a push descriptor set layout (it was not created with "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT).",
                         set, FormatHandle(dsl->Handle()).c_str(), FormatHandle(layout).c_str());
    } else {
        // Create an empty proxy in order to use the existing descriptor set update validation
        // TODO move the validation (like this) that doesn't need descriptor set state to the DSL object so we
        // don't have to do this. Note we need to const_cast<>(this) because GPU-AV needs a non-const version of
        // the state tracker. The proxy here could get away with const.
        vvl::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, const_cast<CoreChecks *>(this));
        vvl::DslErrorSource dsl_error_source(loc, layout, set);
        skip |= ValidatePushDescriptorsUpdate(proxy_ds, descriptorWriteCount, pDescriptorWrites, dsl_error_source, loc);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                     VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites,
                                                     const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineBindPoint(*cb_state, pipelineBindPoint, error_obj.location);
    skip |= ValidateCmdPushDescriptorSet(*cb_state, layout, set, descriptorWriteCount, pDescriptorWrites, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet *pDescriptorWrites,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                               pDescriptorWrites, error_obj);
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                      const VkPushDescriptorSetInfo *pPushDescriptorSetInfo,
                                                      const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);

    skip |= ValidateCmdPushDescriptorSet(*cb_state, pPushDescriptorSetInfo->layout, pPushDescriptorSetInfo->set,
                                         pPushDescriptorSetInfo->descriptorWriteCount, pPushDescriptorSetInfo->pDescriptorWrites,
                                         error_obj.location.dot(Field::pPushDescriptorSetInfo));

    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                         const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo, error_obj);
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
                                                               const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    auto ds_layout_state = Get<vvl::DescriptorSetLayout>(pCreateInfo->descriptorSetLayout);
    if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType && !ds_layout_state) {
        skip |= LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350", pCreateInfo->descriptorSetLayout,
                         create_info_loc.dot(Field::descriptorSetLayout), "(%s) is invalid.",
                         FormatHandle(pCreateInfo->descriptorSetLayout).c_str());
    } else if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS == pCreateInfo->templateType) {
        auto bind_point = pCreateInfo->pipelineBindPoint;
        const bool valid_bp = (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) || (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) ||
                              (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        if (!valid_bp) {
            skip |= LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351", device,
                             create_info_loc.dot(Field::pipelineBindPoint), "is %s.", string_VkPipelineBindPoint(bind_point));
        }
        auto pipeline_layout = Get<vvl::PipelineLayout>(pCreateInfo->pipelineLayout);
        if (!pipeline_layout) {
            skip |= LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352", pCreateInfo->pipelineLayout,
                             create_info_loc.dot(Field::pipelineLayout), "(%s) is invalid.",
                             FormatHandle(pCreateInfo->pipelineLayout).c_str());
        } else {
            const uint32_t pd_set = pCreateInfo->set;
            if ((pd_set >= pipeline_layout->set_layouts.size()) || !pipeline_layout->set_layouts[pd_set] ||
                !pipeline_layout->set_layouts[pd_set]->IsPushDescriptor()) {
                skip |=
                    LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353", pCreateInfo->pipelineLayout,
                             create_info_loc.dot(Field::set),
                             "(%" PRIu32 ") does not refer to the push descriptor set layout for pCreateInfo->pipelineLayout (%s).",
                             pd_set, FormatHandle(pCreateInfo->pipelineLayout).c_str());
            }
        }
    } else if (ds_layout_state && (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType)) {
        for (const auto &binding : ds_layout_state->GetBindings()) {
            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                skip |= LogError(
                    "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-04615", device,
                    create_info_loc.dot(Field::templateType),
                    "is VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET, but "
                    "pCreateInfo->descriptorSetLayout contains a binding with descriptor type VK_DESCRIPTOR_TYPE_MUTABLE_EXT.");
            }
        }
    }
    for (uint32_t i = 0; i < pCreateInfo->descriptorUpdateEntryCount; ++i) {
        const auto &descriptor_update = pCreateInfo->pDescriptorUpdateEntries[i];
        if (descriptor_update.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            if (descriptor_update.dstArrayElement & 3) {
                skip |= LogError("VUID-VkDescriptorUpdateTemplateEntry-descriptor-02226", pCreateInfo->pipelineLayout,
                                 create_info_loc.dot(Field::pDescriptorUpdateEntries, i),
                                 "has descriptorType VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but dstArrayElement (%" PRIu32
                                 ") is not a "
                                 "multiple of 4).",
                                 descriptor_update.dstArrayElement);
            }
            if (descriptor_update.descriptorCount & 3) {
                skip |= LogError("VUID-VkDescriptorUpdateTemplateEntry-descriptor-02227", pCreateInfo->pipelineLayout,
                                 create_info_loc.dot(Field::pDescriptorUpdateEntries, i),
                                 "has descriptorType VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but descriptorCount (%" PRIu32
                                 ") is not a "
                                 "multiple of 4).",
                                 descriptor_update.descriptorCount);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                                  const ErrorObject &error_obj) const {
    return PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, error_obj);
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    auto template_state = Get<vvl::DescriptorUpdateTemplate>(descriptorUpdateTemplate);
    // Object tracker will report errors for invalid descriptorUpdateTemplate values, avoiding a crash in release builds
    // but retaining the assert as template support is new enough to want to investigate these in debug builds.
    if (!template_state) return skip;

    // TODO: Validate template push descriptor updates
    if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
        // decode the templatized data and leverage the non-template UpdateDescriptor helper functions.
        // Translate the templated update into a normal update for validation...
        vvl::DecodedTemplateUpdate decoded_update(*this, descriptorSet, template_state.get(), pData);
        return ValidateUpdateDescriptorSets(static_cast<uint32_t>(decoded_update.desc_writes.size()),
                                            decoded_update.desc_writes.data(), 0, nullptr, error_obj.location);
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const void *pData, const ErrorObject &error_obj) const {
    return PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData, error_obj);
}

bool CoreChecks::ValidateCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          VkPipelineLayout layout, uint32_t set, const void *pData,
                                                          const Location &loc) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, loc);

    const bool is_2 =
        loc.function != Func::vkCmdPushDescriptorSetWithTemplateKHR && loc.function != Func::vkCmdPushDescriptorSetWithTemplate;
    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) return skip;  // dynamicPipelineLayout
    const auto &set_layouts = pipeline_layout->set_layouts;
    if (set >= set_layouts.size()) {
        const char *vuid =
            is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfo-set-07304" : "VUID-vkCmdPushDescriptorSetWithTemplate-set-07304";
        const LogObjectList objlist(commandBuffer, layout);
        skip |= LogError(vuid, objlist, loc.dot(Field::set),
                         "(%" PRIu32 ") is indexing outside the range for %s (which had a setLayoutCount of only %" PRIu32 ").",
                         set, FormatHandle(layout).c_str(), static_cast<uint32_t>(set_layouts.size()));
        return skip;
    }

    const auto &dsl = set_layouts[set];
    ASSERT_AND_RETURN_SKIP(dsl);

    if (!dsl->IsPushDescriptor()) {
        const char *vuid =
            is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfo-set-07305" : "VUID-vkCmdPushDescriptorSetWithTemplate-set-07305";
        const LogObjectList objlist(commandBuffer, layout);
        skip |= LogError(vuid, objlist, loc.dot(Field::set),
                         "(%" PRIu32
                         ") points to %s inside %s which is not a push descriptor set layout (it was not created with "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT).",
                         set, FormatHandle(dsl->Handle()).c_str(), FormatHandle(layout).c_str());
    }

    auto template_state = Get<vvl::DescriptorUpdateTemplate>(descriptorUpdateTemplate);
    if (!template_state) return skip;
    const auto &template_ci = template_state->create_info;

    skip |= ValidatePipelineBindPoint(*cb_state, template_ci.pipelineBindPoint, loc);

    if (template_ci.templateType != VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS) {
        const char *vuid = is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfo-descriptorUpdateTemplate-07994"
                                : "VUID-vkCmdPushDescriptorSetWithTemplate-descriptorUpdateTemplate-07994";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::descriptorUpdateTemplate),
                         "(%s) was not created with VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS.",
                         FormatHandle(descriptorUpdateTemplate).c_str());
    }
    if (template_ci.set != set) {
        const char *vuid =
            is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfo-set-07995" : "VUID-vkCmdPushDescriptorSetWithTemplate-set-07995";
        skip |=
            LogError(vuid, commandBuffer, loc.dot(Field::set),
                     "(%" PRIu32 ") does not match the set (%" PRIu32 ") that the descriptorUpdateTemplate (%s) was created with.",
                     set, template_ci.set, FormatHandle(descriptorUpdateTemplate).c_str());
    }
    auto template_layout = Get<vvl::PipelineLayout>(template_ci.pipelineLayout);
    if (!IsPipelineLayoutSetCompatible(set, pipeline_layout.get(), template_layout.get())) {
        const LogObjectList objlist(commandBuffer, descriptorUpdateTemplate, template_ci.pipelineLayout, layout);
        const char *vuid =
            is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfo-layout-07993" : "VUID-vkCmdPushDescriptorSetWithTemplate-layout-07993";
        skip |= LogError(vuid, objlist, loc.dot(Field::descriptorUpdateTemplate),
                         "%s created with %s is incompatible "
                         "with command parameter "
                         "%s for set %" PRIu32 ".\n%s",
                         FormatHandle(descriptorUpdateTemplate).c_str(), FormatHandle(template_ci.pipelineLayout).c_str(),
                         FormatHandle(layout).c_str(), set,
                         DescribePipelineLayoutSetNonCompatible(set, pipeline_layout.get(), template_layout.get()).c_str());
    }

    if (!Get<vvl::DescriptorSetLayout>(dsl->VkHandle())) {
        const LogObjectList objlist(commandBuffer, descriptorUpdateTemplate, layout);
        const char *vuid =
            is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfo-pData-01686" : "VUID-vkCmdPushDescriptorSetWithTemplate-pData-01686";
        skip |= LogError(vuid, objlist, loc.dot(Field::pData),
                         "does not point to a valid layout, it possible the "
                         "VkDescriptorUpdateTemplateCreateInfo::descriptorSetLayout was accidentally destroy.");
    } else {
        // Create an empty proxy in order to use the existing descriptor set update validation
        vvl::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, const_cast<CoreChecks *>(this));
        // Decode the template into a set of write updates
        vvl::DecodedTemplateUpdate decoded_template(*this, VK_NULL_HANDLE, template_state.get(), pData, dsl->VkHandle());
        // Validate the decoded update against the proxy_ds
        vvl::DslErrorSource dsl_error_source(loc, layout, set);
        skip |= ValidatePushDescriptorsUpdate(proxy_ds, static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                              decoded_template.desc_writes.data(), dsl_error_source, loc);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                                 VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                 VkPipelineLayout layout, uint32_t set, const void *pData,
                                                                 const ErrorObject &error_obj) const {
    return ValidateCmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData,
                                                    error_obj.location);
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set, const void *pData,
                                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData, error_obj);
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCmdPushDescriptorSetWithTemplate(
        commandBuffer, pPushDescriptorSetWithTemplateInfo->descriptorUpdateTemplate, pPushDescriptorSetWithTemplateInfo->layout,
        pPushDescriptorSetWithTemplateInfo->set, pPushDescriptorSetWithTemplateInfo->pData,
        error_obj.location.dot(Field::pPushDescriptorSetWithTemplateInfo));
    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR *pPushDescriptorSetWithTemplateInfo,
    const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushDescriptorSetWithTemplate2(commandBuffer, pPushDescriptorSetWithTemplateInfo, error_obj);
}

enum DSL_DESCRIPTOR_GROUPS {
    DSL_TYPE_SAMPLERS = 0,
    DSL_TYPE_UNIFORM_BUFFERS,
    DSL_TYPE_STORAGE_BUFFERS,
    DSL_TYPE_SAMPLED_IMAGES,
    DSL_TYPE_STORAGE_IMAGES,
    DSL_TYPE_INPUT_ATTACHMENTS,
    DSL_TYPE_INLINE_UNIFORM_BLOCK,
    DSL_TYPE_ACCELERATION_STRUCTURE,
    DSL_TYPE_ACCELERATION_STRUCTURE_NV,
    DSL_NUM_DESCRIPTOR_GROUPS
};

// Used by PreCallValidateCreatePipelineLayout.
// Returns an array of size DSL_NUM_DESCRIPTOR_GROUPS of the maximum number of descriptors used in any single pipeline stage
// Use 64-bit because lots of limts are really just UINT32_MAX and need to catch them
static std::valarray<uint64_t> GetDescriptorCountMaxPerStage(
    const DeviceFeatures *enabled_features, const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts,
    bool skip_update_after_bind) {
    // Identify active pipeline stages
    std::vector<VkShaderStageFlags> stage_flags = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   VK_SHADER_STAGE_COMPUTE_BIT};
    if (enabled_features->geometryShader) {
        stage_flags.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    if (enabled_features->tessellationShader) {
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }
    if (enabled_features->rayTracingPipeline) {
        stage_flags.push_back(VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_MISS_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_CALLABLE_BIT_KHR);
    }

    // Allow iteration over enum values
    std::vector<DSL_DESCRIPTOR_GROUPS> dsl_groups = {
        DSL_TYPE_SAMPLERS,
        DSL_TYPE_UNIFORM_BUFFERS,
        DSL_TYPE_STORAGE_BUFFERS,
        DSL_TYPE_SAMPLED_IMAGES,
        DSL_TYPE_STORAGE_IMAGES,
        DSL_TYPE_INPUT_ATTACHMENTS,
        DSL_TYPE_INLINE_UNIFORM_BLOCK,
        DSL_TYPE_ACCELERATION_STRUCTURE,
        DSL_TYPE_ACCELERATION_STRUCTURE_NV,
    };

    // Sum by layouts per stage, then pick max of stages per type
    const uint64_t init_value = 0;
    const uint32_t val_array_size = DSL_NUM_DESCRIPTOR_GROUPS;
    std::valarray<uint64_t> max_sum(init_value, val_array_size);  // max descriptor sum among all pipeline stages
    for (auto stage : stage_flags) {
        std::valarray<uint64_t> stage_sum(init_value, val_array_size);  // per-stage sums
        for (const auto &dsl : set_layouts) {
            if (!dsl) {
                continue;
            }
            if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                continue;
            }

            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (0 != (stage & binding->stageFlags) && binding->descriptorCount > 0) {
                    switch (binding->descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_UNIFORM_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_STORAGE_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                            stage_sum[DSL_TYPE_STORAGE_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            stage_sum[DSL_TYPE_INPUT_ATTACHMENTS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                            // count one block per binding. descriptorCount is number of bytes
                            stage_sum[DSL_TYPE_INLINE_UNIFORM_BLOCK]++;
                            break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                            stage_sum[DSL_TYPE_ACCELERATION_STRUCTURE] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                            stage_sum[DSL_TYPE_ACCELERATION_STRUCTURE_NV] += binding->descriptorCount;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        for (auto type : dsl_groups) {
            max_sum[type] = std::max(stage_sum[type], max_sum[type]);
        }
    }
    return max_sum;
}

// Used by PreCallValidateCreatePipelineLayout.
// Returns a map indexed by VK_DESCRIPTOR_TYPE_* enum of the summed descriptors by type.
// Note: descriptors only count against the limit once even if used by multiple stages.
// Use 64-bit because lots of limts are really just UINT32_MAX and need to catch them
static std::map<uint32_t, uint64_t> GetDescriptorSum(
    const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    std::map<uint32_t, uint64_t> sum_by_type;
    for (const auto &dsl : set_layouts) {
        if (!dsl) {
            continue;
        }
        if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (binding->descriptorCount > 0) {
                sum_by_type[binding->descriptorType] += binding->descriptorCount;
            }
        }
    }
    return sum_by_type;
}

uint64_t GetInlineUniformBlockBindingCount(const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts,
                                           bool skip_update_after_bind) {
    uint64_t sum = 0;
    for (const auto &dsl : set_layouts) {
        if (!dsl) {
            continue;
        }
        if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && binding->descriptorCount > 0) {
                ++sum;
            }
        }
    }
    return sum;
}

bool CoreChecks::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> set_layouts(pCreateInfo->setLayoutCount, nullptr);
    uint32_t descriptor_buffer_set_count = 0;
    uint32_t valid_set_count = 0;
    uint32_t push_descriptor_set_found = pCreateInfo->setLayoutCount;
    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        set_layouts[i] = Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
        if (!set_layouts[i]) continue;

        if (set_layouts[i]->IsPushDescriptor()) {
            if (push_descriptor_set_found < pCreateInfo->setLayoutCount) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00293", set_layouts[i]->VkHandle(),
                                 create_info_loc.dot(Field::pSetLayouts, i),
                                 "and pSetLayouts[%" PRIu32 "] both have push descriptor sets.", push_descriptor_set_found);
            }
            push_descriptor_set_found = i;
        }
        if (set_layouts[i]->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-04606", set_layouts[i]->VkHandle(),
                             create_info_loc.dot(Field::pSetLayouts, i),
                             "was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT bit.");
        }
        ++valid_set_count;
        if (set_layouts[i]->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
            ++descriptor_buffer_set_count;
        }
    }

    if ((descriptor_buffer_set_count != 0) && (valid_set_count != descriptor_buffer_set_count)) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008", device, error_obj.location,
                         "All sets must be created with "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT or none of them.");
    }

    // Max descriptors by type, within a single pipeline stage
    std::valarray<uint64_t> max_descriptors_per_stage = GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, true);
    // Samplers
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLERS] > phys_dev_props.limits.maxPerStageDescriptorSamplers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03016", device, error_obj.location,
                         "max per-stage sampler bindings count (%" PRIu64
                         ") exceeds device "
                         "maxPerStageDescriptorSamplers limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_SAMPLERS], phys_dev_props.limits.maxPerStageDescriptorSamplers);
    }

    // Uniform buffers
    if (max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorUniformBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03017", device, error_obj.location,
                         "max per-stage uniform buffer bindings count (%" PRIu64
                         ") exceeds device "
                         "maxPerStageDescriptorUniformBuffers limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS],
                         phys_dev_props.limits.maxPerStageDescriptorUniformBuffers);
    }

    // Storage buffers
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorStorageBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03018", device, error_obj.location,
                         "max per-stage storage buffer bindings count (%" PRIu64
                         ") exceeds device "
                         "maxPerStageDescriptorStorageBuffers limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS],
                         phys_dev_props.limits.maxPerStageDescriptorStorageBuffers);
    }

    // Sampled images
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorSampledImages) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-06939", device, error_obj.location,
                     "max per-stage sampled image bindings count (%" PRIu64
                     ") exceeds device "
                     "maxPerStageDescriptorSampledImages limit (%" PRIu32 ").",
                     max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES], phys_dev_props.limits.maxPerStageDescriptorSampledImages);
    }

    // Storage images
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorStorageImages) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03020", device, error_obj.location,
                     "max per-stage storage image bindings count (%" PRIu64
                     ") exceeds device "
                     "maxPerStageDescriptorStorageImages limit (%" PRIu32 ").",
                     max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES], phys_dev_props.limits.maxPerStageDescriptorStorageImages);
    }

    // Input attachments
    if (max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS] > phys_dev_props.limits.maxPerStageDescriptorInputAttachments) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03021", device, error_obj.location,
                         "max per-stage input attachment bindings count (%" PRIu64
                         ") exceeds device "
                         "maxPerStageDescriptorInputAttachments limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS],
                         phys_dev_props.limits.maxPerStageDescriptorInputAttachments);
    }

    // Inline uniform blocks
    if (max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK] > phys_dev_props_core13.maxPerStageDescriptorInlineUniformBlocks) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02214", device, error_obj.location,
                         "max per-stage inline uniform block bindings count (%" PRIu64
                         ") exceeds device "
                         "maxPerStageDescriptorInlineUniformBlocks limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                         phys_dev_props_core13.maxPerStageDescriptorInlineUniformBlocks);
    }

    // Acceleration structures
    if (max_descriptors_per_stage[DSL_TYPE_ACCELERATION_STRUCTURE] >
        phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorAccelerationStructures) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03571", device, error_obj.location,
                         "max per-stage acceleration structure bindings count (%" PRIu64
                         ") exceeds device "
                         "maxPerStageDescriptorAccelerationStructures limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_ACCELERATION_STRUCTURE],
                         phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorAccelerationStructures);
    }

    // Total descriptors by type
    std::map<uint32_t, uint64_t> sum_all_stages = GetDescriptorSum(set_layouts, true);

    // Samplers
    uint64_t sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLER] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSamplers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03028", device, error_obj.location,
                         "sum of sampler bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetSamplers limit (%" PRIu32 ").",
                         sum, phys_dev_props.limits.maxDescriptorSetSamplers);
    }

    // Uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] > phys_dev_props.limits.maxDescriptorSetUniformBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03029", device, error_obj.location,
                         "sum of uniform buffer bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetUniformBuffers limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER], phys_dev_props.limits.maxDescriptorSetUniformBuffers);
    }

    // Storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] > phys_dev_props.limits.maxDescriptorSetStorageBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03031", device, error_obj.location,
                         "sum of storage buffer bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetStorageBuffers limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER], phys_dev_props.limits.maxDescriptorSetStorageBuffers);
    }

    if (enabled_features.maintenance7) {
        // Dynamic uniform buffers
        if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
            phys_dev_ext_props.maintenance7_props.maxDescriptorSetTotalUniformBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-maintenance7-10003", device, error_obj.location,
                             "sum of dynamic uniform buffer bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetTotalUniformBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                             phys_dev_ext_props.maintenance7_props.maxDescriptorSetTotalUniformBuffersDynamic);
        }

        // Dynamic storage buffers
        if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
            phys_dev_ext_props.maintenance7_props.maxDescriptorSetTotalStorageBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-maintenance7-10004", device, error_obj.location,
                             "sum of dynamic storage buffer bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetTotalStorageBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                             phys_dev_ext_props.maintenance7_props.maxDescriptorSetTotalStorageBuffersDynamic);
        }

        sum = sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] + sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC];
        if (sum > phys_dev_ext_props.maintenance7_props.maxDescriptorSetTotalBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-None-10005", device, error_obj.location,
                             "sum of both dynamic storage buffer bindings (%" PRIu64
                             ") and dynamic uniform buffer bindings (%" PRIu64 ") among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetTotalBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                             sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC], sum,
                             phys_dev_ext_props.maintenance7_props.maxDescriptorSetTotalBuffersDynamic);
        }
    } else {
        // Dynamic uniform buffers
        if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
            phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03030", device, error_obj.location,
                             "sum of dynamic uniform buffer bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUniformBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                             phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic);
        }

        // Dynamic storage buffers
        if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
            phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03032", device, error_obj.location,
                             "sum of dynamic storage buffer bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetStorageBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                             phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic);
        }
    }

    // Sampled images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
          sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSampledImages) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03033", device, error_obj.location,
                         "sum of sampled image bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetSampledImages limit (%" PRIu32 ").",
                         sum, phys_dev_props.limits.maxDescriptorSetSampledImages);
    }

    // Storage images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetStorageImages) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03034", device, error_obj.location,
                         "sum of storage image bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetStorageImages limit (%" PRIu32 ").",
                         sum, phys_dev_props.limits.maxDescriptorSetStorageImages);
    }

    // Input attachments
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] > phys_dev_props.limits.maxDescriptorSetInputAttachments) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03035", device, error_obj.location,
                     "sum of input attachment bindings among all stages (%" PRIu64
                     ") exceeds device "
                     "maxDescriptorSetInputAttachments limit (%" PRIu32 ").",
                     sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT], phys_dev_props.limits.maxDescriptorSetInputAttachments);
    }

    // Inline uniform blocks
    const uint64_t inline_uniform_block_bindings_all_stages = GetInlineUniformBlockBindingCount(set_layouts, true);
    if (inline_uniform_block_bindings_all_stages > phys_dev_props_core13.maxDescriptorSetInlineUniformBlocks) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02216", device, error_obj.location,
                         "sum of inline uniform block bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetInlineUniformBlocks limit (%" PRIu32 ").",
                         inline_uniform_block_bindings_all_stages, phys_dev_props_core13.maxDescriptorSetInlineUniformBlocks);
    }
    if (api_version >= VK_API_VERSION_1_3 &&
        sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK] > phys_dev_props_core13.maxInlineUniformTotalSize) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-06531", device, error_obj.location,
                         "sum of inline uniform block bytes among all stages (%" PRIu64
                         ") exceeds device "
                         "maxInlineUniformTotalSize limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK], phys_dev_props_core13.maxInlineUniformTotalSize);
    }

    // Acceleration structures
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR] >
        phys_dev_ext_props.acc_structure_props.maxDescriptorSetAccelerationStructures) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03573", device, error_obj.location,
                         "sum of acceleration structures bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetAccelerationStructures limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR],
                         phys_dev_ext_props.acc_structure_props.maxDescriptorSetAccelerationStructures);
    }

    // Acceleration structures NV
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV] >
        phys_dev_ext_props.ray_tracing_props_nv.maxDescriptorSetAccelerationStructures) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02381", device, error_obj.location,
                         "sum of acceleration structures NV bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxDescriptorSetAccelerationStructures limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV],
                         phys_dev_ext_props.ray_tracing_props_nv.maxDescriptorSetAccelerationStructures);
    }

    // Extension exposes new properties limits
    if (IsExtEnabled(extensions.vk_ext_descriptor_indexing)) {
        // Max descriptors by type, within a single pipeline stage
        std::valarray<uint64_t> max_descriptors_per_stage_update_after_bind =
            GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, false);
        // Samplers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03022", device, error_obj.location,
                             "max per-stage sampler bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindSamplers limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03023", device, error_obj.location,
                             "max per-stage uniform buffer bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindUniformBuffers limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
        }

        // Storage buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03024", device, error_obj.location,
                             "max per-stage storage buffer bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindStorageBuffers limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
        }

        // Sampled images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03025", device, error_obj.location,
                             "max per-stage sampled image bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindSampledImages limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages);
        }

        // Storage images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03026", device, error_obj.location,
                             "max per-stage storage image bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindStorageImages limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03027", device, error_obj.location,
                             "max per-stage input attachment bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindInputAttachments limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
            phys_dev_props_core13.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02215", device, error_obj.location,
                             "max per-stage inline uniform block bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                             phys_dev_props_core13.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks);
        }

        // Acceleration structures
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_ACCELERATION_STRUCTURE] >
            phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572", device, error_obj.location,
                             "max per-stage acceleration structure bindings count (%" PRIu64
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindAccelerationStructures limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_ACCELERATION_STRUCTURE],
                             phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures);
        }

        // Total descriptors by type, summed across all pipeline stages
        std::map<uint32_t, uint64_t> sum_all_stages_update_after_bind = GetDescriptorSum(set_layouts, false);

        // Samplers
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036", device, error_obj.location,
                             "sum of sampler bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindSamplers limit (%" PRIu32 ").",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03037", device, error_obj.location,
                             "sum of uniform buffer bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindUniformBuffers limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers);
        }

        // Storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03039", device, error_obj.location,
                             "sum of storage buffer bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageBuffers limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers);
        }

        if (enabled_features.maintenance7) {
            // Dynamic uniform buffers
            if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
                phys_dev_ext_props.maintenance7_props.maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-maintenance7-10007", device, error_obj.location,
                                 "sum of dynamic uniform buffer bindings among all stages (%" PRIu64
                                 ") exceeds device "
                                 "maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic limit (%" PRIu32 ").",
                                 sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                                 phys_dev_ext_props.maintenance7_props.maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic);
            }

            // Dynamic storage buffers
            if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
                phys_dev_ext_props.maintenance7_props.maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-maintenance7-10008", device, error_obj.location,
                                 "sum of dynamic storage buffer bindings among all stages (%" PRIu64
                                 ") exceeds device "
                                 "maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic limit (%" PRIu32 ").",
                                 sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                                 phys_dev_ext_props.maintenance7_props.maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic);
            }

            sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] +
                  sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC];
            if (sum > phys_dev_ext_props.maintenance7_props.maxDescriptorSetUpdateAfterBindTotalBuffersDynamic) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-10006", device, error_obj.location,
                                 "sum of both dynamic storage buffer bindings (%" PRIu64
                                 ") and dynamic uniform buffer bindings (%" PRIu64 ") among all stages (%" PRIu64
                                 ") exceeds device "
                                 "maxDescriptorSetUpdateAfterBindTotalBuffersDynamic limit (%" PRIu32 ").",
                                 sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                                 sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC], sum,
                                 phys_dev_ext_props.maintenance7_props.maxDescriptorSetUpdateAfterBindTotalBuffersDynamic);
            }
        } else {
            // Dynamic uniform buffers
            if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
                phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03038", device, error_obj.location,
                                 "sum of dynamic uniform buffer bindings among all stages (%" PRIu64
                                 ") exceeds device "
                                 "maxDescriptorSetUpdateAfterBindUniformBuffersDynamic limit (%" PRIu32 ").",
                                 sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                                 phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic);
            }

            // Dynamic storage buffers
            if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
                phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03040", device, error_obj.location,
                                 "sum of dynamic storage buffer bindings among all stages (%" PRIu64
                                 ") exceeds device "
                                 "maxDescriptorSetUpdateAfterBindStorageBuffersDynamic limit (%" PRIu32 ").",
                                 sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                                 phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic);
            }
        }

        // Sampled images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041", device, error_obj.location,
                             "sum of sampled image bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindSampledImages limit (%" PRIu32 ").",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages);
        }

        // Storage images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03042", device, error_obj.location,
                             "sum of storage image bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageImages limit (%" PRIu32 ").",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03043", device, error_obj.location,
                             "sum of input attachment bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindInputAttachments limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        const uint64_t inline_uniform_block_bindings = GetInlineUniformBlockBindingCount(set_layouts, false);
        if (inline_uniform_block_bindings > phys_dev_props_core13.maxDescriptorSetUpdateAfterBindInlineUniformBlocks) {
            skip |=
                LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02217", device, error_obj.location,
                         "sum of inline uniform block bindings among all stages (%" PRIu64
                         ") exceeds device "
                         "maxDescriptorSetUpdateAfterBindInlineUniformBlocks limit (%" PRIu32 ").",
                         inline_uniform_block_bindings, phys_dev_props_core13.maxDescriptorSetUpdateAfterBindInlineUniformBlocks);
        }

        // Acceleration structures
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR] >
            phys_dev_ext_props.acc_structure_props.maxDescriptorSetUpdateAfterBindAccelerationStructures) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574", device, error_obj.location,
                             "sum of acceleration structures bindings among all stages (%" PRIu64
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindAccelerationStructures limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR],
                             phys_dev_ext_props.acc_structure_props.maxDescriptorSetUpdateAfterBindAccelerationStructures);
        }
    }

    // Extension exposes new properties limits
    if (IsExtEnabled(extensions.vk_ext_fragment_density_map2)) {
        uint32_t sum_subsampled_samplers = 0;
        for (const auto &dsl : set_layouts) {
            // find the number of subsampled samplers across all stages
            // NOTE: this does not use the GetDescriptorSum patter because it needs the Get<vvl::Sampler> method
            if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                continue;
            }
            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);

                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (binding->descriptorCount == 0) {
                    continue;
                }

                if (((binding->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                     (binding->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)) &&
                    (binding->pImmutableSamplers != nullptr)) {
                    for (uint32_t sampler_idx = 0; sampler_idx < binding->descriptorCount; sampler_idx++) {
                        auto state = Get<vvl::Sampler>(binding->pImmutableSamplers[sampler_idx]);
                        if (state && (state->create_info.flags & (VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT |
                                                                  VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT))) {
                            sum_subsampled_samplers++;
                        }
                    }
                }
            }
        }
        if (sum_subsampled_samplers > phys_dev_ext_props.fragment_density_map2_props.maxDescriptorSetSubsampledSamplers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pImmutableSamplers-03566", device, error_obj.location,
                             "sum of sampler bindings with flags containing "
                             "VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT or "
                             "VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT among all stages(% d) "
                             "exceeds device maxDescriptorSetSubsampledSamplers limit (%" PRIu32 ").",
                             sum_subsampled_samplers,
                             phys_dev_ext_props.fragment_density_map2_props.maxDescriptorSetSubsampledSamplers);
        }
    }

    if (!enabled_features.graphicsPipelineLibrary) {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            if (!pCreateInfo->pSetLayouts[i]) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753", device,
                                 create_info_loc.dot(Field::pSetLayouts, i),
                                 "is VK_NULL_HANDLE, but the graphicsPipelineLibrary feature is not enabled.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                          uint32_t offset, uint32_t size, const Location &loc) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, loc);

    // Check if pipeline_layout VkPushConstantRange(s) overlapping offset, size have stageFlags set for each stage in the command
    // stageFlags argument, *and* that the command stageFlags argument has bits set for the stageFlags in each overlapping range.
    if (skip) {
        return skip;
    }
    auto layout_state = Get<vvl::PipelineLayout>(layout);
    if (!layout_state) return skip;  // dynamicPipelineLayout feature

    const bool is_2 = loc.function != Func::vkCmdPushConstants;
    const auto &ranges = *layout_state->push_constant_ranges_layout;
    VkShaderStageFlags found_stages = 0;
    for (const auto &range : ranges) {
        if ((offset >= range.offset) && (offset + size <= range.offset + range.size)) {
            VkShaderStageFlags matching_stages = range.stageFlags & stageFlags;
            if (matching_stages != range.stageFlags) {
                const char *vuid = is_2 ? "VUID-VkPushConstantsInfo-offset-01796" : "VUID-vkCmdPushConstants-offset-01796";
                skip |= LogError(vuid, commandBuffer, loc,
                                 "is called with\nstageFlags (%s), offset (%" PRIu32 "), size (%" PRIu32
                                 ")\nwhich is missing stageFlags from the overlapping VkPushConstantRange in %s\nstageFlags (%s), "
                                 "offset (%" PRIu32 "), size (%" PRIu32 ")",
                                 string_VkShaderStageFlags(stageFlags).c_str(), offset, size, FormatHandle(layout).c_str(),
                                 string_VkShaderStageFlags(range.stageFlags).c_str(), range.offset, range.size);
            }

            // Accumulate all stages we've found
            found_stages = matching_stages | found_stages;
        }
    }
    if (found_stages != stageFlags) {
        const uint32_t missing_stages = ~found_stages & stageFlags;
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfo-offset-01795" : "VUID-vkCmdPushConstants-offset-01795";
        skip |= LogError(vuid, commandBuffer, loc,
                         "is called with\nstageFlags (%s), offset (%" PRIu32 "), size (%" PRIu32
                         ")\nbut the %s doesn't have a VkPushConstantRange with %s",
                         string_VkShaderStageFlags(stageFlags).c_str(), offset, size, FormatHandle(layout).c_str(),
                         string_VkShaderStageFlags(missing_stages).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCmdPushConstants(commandBuffer, pPushConstantsInfo->layout, pPushConstantsInfo->stageFlags,
                                     pPushConstantsInfo->offset, pPushConstantsInfo->size,
                                     error_obj.location.dot(Field::pPushConstantsInfo));

    return skip;
}

bool CoreChecks::PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer,
                                                     const VkPushConstantsInfoKHR *pPushConstantsInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushConstants2(commandBuffer, pPushConstantsInfo, error_obj);
}

bool CoreChecks::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                              const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateDeviceQueueSupport(error_obj.location);
    auto num_samplers = Count<vvl::Sampler>();
    if (num_samplers >= phys_dev_props.limits.maxSamplerAllocationCount) {
        skip |= LogError("VUID-vkCreateSampler-maxSamplerAllocationCount-04110", device, error_obj.location,
                         "Number of currently valid sampler objects (%zu) is not less than the maximum allowed (%" PRIu32 ").",
                         num_samplers, phys_dev_props.limits.maxSamplerAllocationCount);
    }

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    if (enabled_features.samplerYcbcrConversion == VK_TRUE) {
        if (const auto *conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext)) {
            const VkSamplerYcbcrConversion sampler_ycbcr_conversion = conversion_info->conversion;
            auto ycbcr_state = Get<vvl::SamplerYcbcrConversion>(sampler_ycbcr_conversion);
            if (ycbcr_state && (ycbcr_state->format_features &
                                VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT) == 0) {
                const VkFilter chroma_filter = ycbcr_state->chromaFilter;
                if (pCreateInfo->minFilter != chroma_filter) {
                    skip |= LogError(
                        "VUID-VkSamplerCreateInfo-minFilter-01645", device,
                        create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                        "(%s) does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT "
                        "for format %s and minFilter (%s) is different from "
                        "chromaFilter (%s)",
                        FormatHandle(sampler_ycbcr_conversion).c_str(), string_VkFormat(ycbcr_state->format),
                        string_VkFilter(pCreateInfo->minFilter), string_VkFilter(chroma_filter));
                }
                if (pCreateInfo->magFilter != chroma_filter) {
                    skip |= LogError(
                        "VUID-VkSamplerCreateInfo-minFilter-01645", device,
                        create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                        "(%s) does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT "
                        "for format %s and magFilter (%s) is different from "
                        "chromaFilter (%s)",
                        FormatHandle(sampler_ycbcr_conversion).c_str(), string_VkFormat(ycbcr_state->format),
                        string_VkFilter(pCreateInfo->magFilter), string_VkFilter(chroma_filter));
                }
            }
        }
    }

    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        if (custom_border_color_sampler_count >= phys_dev_ext_props.custom_border_color_props.maxCustomBorderColorSamplers) {
            skip |= LogError("VUID-VkSamplerCreateInfo-None-04012", device, error_obj.location,
                             "Creating a sampler with a custom border color will exceed the "
                             "maxCustomBorderColorSamplers limit of %" PRIu32 ".",
                             phys_dev_ext_props.custom_border_color_props.maxCustomBorderColorSamplers);
        }
    }

    if (IsExtEnabled(extensions.vk_khr_portability_subset)) {
        if ((VK_FALSE == enabled_features.samplerMipLodBias) && pCreateInfo->mipLodBias != 0) {
            skip |= LogError("VUID-VkSamplerCreateInfo-samplerMipLodBias-04467", device, error_obj.location,
                             "(portability error) mipLodBias is %f, but samplerMipLodBias not supported.", pCreateInfo->mipLodBias);
        }
    }

    return skip;
}
