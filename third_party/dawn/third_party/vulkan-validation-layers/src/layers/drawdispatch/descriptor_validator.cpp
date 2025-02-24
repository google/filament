/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include "descriptor_validator.h"
#include "generated/spirv_grammar_helper.h"
#include "state_tracker/shader_stage_state.h"
#include "error_message/error_strings.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/shader_module.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "utils/vk_layer_utils.h"

namespace vvl {

// This seems like it could be useful elsewhere, but until find another spot, just keep here.
static const char *GetActionType(Func command) {
    switch (command) {
        case Func::vkCmdDispatch:
        case Func::vkCmdDispatchIndirect:
        case Func::vkCmdDispatchBase:
        case Func::vkCmdDispatchBaseKHR:
        case Func::vkCmdDispatchGraphAMDX:
        case Func::vkCmdDispatchGraphIndirectAMDX:
        case Func::vkCmdDispatchGraphIndirectCountAMDX:
            return "dispatch";
        case Func::vkCmdTraceRaysNV:
        case Func::vkCmdTraceRaysKHR:
        case Func::vkCmdTraceRaysIndirectKHR:
        case Func::vkCmdTraceRaysIndirect2KHR:
            return "trace rays";
        default:
            return "draw";
    }
}

std::string DescriptorValidator::DescribeDescriptor(const spirv::ResourceInterfaceVariable &resource_variable,
                                                    uint32_t index) const {
    std::stringstream ss;
    ss << dev_state.FormatHandle(descriptor_set.Handle()) << " [Set " << set_index << ", Binding "
       << resource_variable.decorations.binding << ", Index " << index;

    // If multiple variables tied to a binding, don't attempt to detect which one
    if (!resource_variable.debug_name.empty()) {
        ss << ", variable \"" << resource_variable.debug_name << "\"";
    }
    ss << "]";
    return ss.str();
}

DescriptorValidator::DescriptorValidator(vvl::Device &dev, CommandBuffer &cb_state, DescriptorSet &descriptor_set,
                                         uint32_t set_index, VkFramebuffer framebuffer, const Location &loc)
    : dev_state(dev),
      cb_state(cb_state),
      descriptor_set(descriptor_set),
      set_index(set_index),
      framebuffer(framebuffer),
      loc(loc),
      vuids(GetDrawDispatchVuid(loc.function)) {}

template <typename T>
bool DescriptorValidator::ValidateDescriptorsStatic(const spirv::ResourceInterfaceVariable &resource_variable,
                                                    const T &binding) const {
    bool skip = false;
    for (uint32_t index = 0; !skip && index < binding.count; index++) {
        const auto &descriptor = binding.descriptors[index];

        if (!binding.updated[index]) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(
                vuids.descriptor_buffer_bit_set_08114, set, loc,
                "the descriptor %s is being used in %s but has never been updated via vkUpdateDescriptorSets() or a similar call.",
                DescribeDescriptor(resource_variable, index).c_str(), GetActionType(loc.function));
        }
        skip |= ValidateDescriptor(resource_variable, index, binding.type, descriptor);
    }
    return skip;
}

bool DescriptorValidator::ValidateBindingStatic(const spirv::ResourceInterfaceVariable &resource_variable,
                                                const DescriptorBinding &binding) const {
    bool skip = false;
    switch (binding.descriptor_class) {
        case DescriptorClass::GeneralBuffer:
            skip |= ValidateDescriptorsStatic(resource_variable, static_cast<const BufferBinding &>(binding));
            break;
        case DescriptorClass::ImageSampler:
            skip |= ValidateDescriptorsStatic(resource_variable, static_cast<const ImageSamplerBinding &>(binding));
            break;
        case DescriptorClass::Image:
            skip |= ValidateDescriptorsStatic(resource_variable, static_cast<const ImageBinding &>(binding));
            break;
        case DescriptorClass::PlainSampler:
            skip |= ValidateDescriptorsStatic(resource_variable, static_cast<const SamplerBinding &>(binding));
            break;
        case DescriptorClass::TexelBuffer:
            skip |= ValidateDescriptorsStatic(resource_variable, static_cast<const TexelBinding &>(binding));
            break;
        case DescriptorClass::AccelerationStructure:
            skip |= ValidateDescriptorsStatic(resource_variable, static_cast<const AccelerationStructureBinding &>(binding));
            break;
        case DescriptorClass::InlineUniform:
            break;  // Can't validate the descriptor because it may not have been updated.
        case DescriptorClass::Mutable:
            break;  // TODO - is there anything that can be checked here?
        case DescriptorClass::Invalid:
            assert(false);
            break;
    }
    return skip;
}

template <typename T>
bool DescriptorValidator::ValidateDescriptorsDynamic(const spirv::ResourceInterfaceVariable &resource_variable, const T &binding,
                                                     const uint32_t index) {
    bool skip = false;
    const auto &descriptor = binding.descriptors[index];

    if (!binding.updated[index]) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(
            vuids.descriptor_buffer_bit_set_08114, set, loc,
            "the descriptor %s is being used in %s but has never been updated via vkUpdateDescriptorSets() or a similar call.",
            DescribeDescriptor(resource_variable, index).c_str(), GetActionType(loc.function));
    }
    skip |= ValidateDescriptor(resource_variable, index, binding.type, descriptor);
    return skip;
}

bool DescriptorValidator::ValidateBindingDynamic(const spirv::ResourceInterfaceVariable &resource_variable,
                                                 DescriptorBinding &binding, const uint32_t index) {
    bool skip = false;

    switch (binding.descriptor_class) {
        case DescriptorClass::GeneralBuffer:
            skip |= ValidateDescriptorsDynamic(resource_variable, static_cast<const BufferBinding &>(binding), index);
            break;
        case DescriptorClass::ImageSampler: {
            auto &imgs_binding = static_cast<ImageSamplerBinding &>(binding);
            auto &descriptor = imgs_binding.descriptors[index];
            descriptor.UpdateDrawState(cb_state);
            skip |= ValidateDescriptorsDynamic(resource_variable, imgs_binding, index);
            break;
        }
        case DescriptorClass::Image: {
            auto &img_binding = static_cast<ImageBinding &>(binding);
            auto &descriptor = img_binding.descriptors[index];
            descriptor.UpdateDrawState(cb_state);
            skip |= ValidateDescriptorsDynamic(resource_variable, img_binding, index);
            break;
        }
        case DescriptorClass::PlainSampler:
            skip |= ValidateDescriptorsDynamic(resource_variable, static_cast<const SamplerBinding &>(binding), index);
            break;
        case DescriptorClass::TexelBuffer:
            skip |= ValidateDescriptorsDynamic(resource_variable, static_cast<const TexelBinding &>(binding), index);
            break;
        case DescriptorClass::AccelerationStructure:
            skip |=
                ValidateDescriptorsDynamic(resource_variable, static_cast<const AccelerationStructureBinding &>(binding), index);
            break;
        case DescriptorClass::InlineUniform:
            break;  // TODO - Can give warning if reading uninitialized data
        case DescriptorClass::Mutable:
            break;  // TODO - is there anything that can be checked here?
        case DescriptorClass::Invalid:
            assert(false);
            break;
    }
    return skip;
}

bool DescriptorValidator::ValidateDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, const uint32_t index,
                                             VkDescriptorType descriptor_type, const BufferDescriptor &descriptor) const {
    // Verify that buffers are valid
    const VkBuffer buffer = descriptor.GetBuffer();
    auto buffer_node = descriptor.GetBufferState();
    if ((!buffer_node && !dev_state.enabled_features.nullDescriptor) || (buffer_node && buffer_node->Destroyed())) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                  "the descriptor %s is using buffer %s that is invalid or has been destroyed.",
                                  DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(buffer).c_str());
    }

    // Buffer could be null via nullDescriptor and accessing it is legal
    if (buffer == VK_NULL_HANDLE) {
        return false;
    }
    if (buffer_node /* && !buffer_node->sparse*/) {
        for (const auto &binding : buffer_node->GetInvalidMemory()) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                      "the descriptor %s is using buffer %s that references invalid memory %s.",
                                      DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(buffer).c_str(),
                                      dev_state.FormatHandle(binding->Handle()).c_str());
        }
    }
    if (dev_state.enabled_features.protectedMemory == VK_TRUE) {
        if (dev_state.ValidateProtectedBuffer(cb_state, *buffer_node, loc, vuids.unprotected_command_buffer_02707,
                                              " (Buffer is in a descriptorSet)")) {
            return true;
        }
        if (resource_variable.IsWrittenTo() &&
            dev_state.ValidateUnprotectedBuffer(cb_state, *buffer_node, loc, vuids.protected_command_buffer_02712,
                                                " (Buffer is in a descriptorSet)")) {
            return true;
        }
    }
    return false;
}

// 'index' is the index into the descriptor
bool DescriptorValidator::ValidateDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, const uint32_t index,
                                             VkDescriptorType descriptor_type, const ImageDescriptor &image_descriptor) const {
    // We skip various parts of checks for core check to prevent false positive when we don't know the index
    bool skip = false;
    const bool is_gpu_av = dev_state.container_type == LayerObjectTypeGpuAssisted;
    std::vector<const Sampler *> sampler_states;
    const VkImageView image_view = image_descriptor.GetImageView();
    const ImageView *image_view_state = image_descriptor.GetImageViewState();

    if (image_descriptor.GetClass() == DescriptorClass::ImageSampler) {
        sampler_states.emplace_back(static_cast<const ImageSamplerDescriptor &>(image_descriptor).GetSamplerState());
    } else if (is_gpu_av) {
        // TODO - This will skip for GPU-AV because we don't currently capture array of samplers with array of sampled images
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8922
        // To not give false positve, we will skip all Sampler related checks since sampler_states will be empty
    } else {
        if (index < resource_variable.samplers_used_by_image.size()) {
            for (const auto &sampler_used : resource_variable.samplers_used_by_image[index]) {
                const auto *descriptor =
                    descriptor_set.GetDescriptorFromBinding(sampler_used.sampler_slot.binding, sampler_used.sampler_index);
                // TODO: This check _shouldn't_ be necessary due to the checks made in ResourceInterfaceVariable() in
                //       shader_validation.cpp. However, without this check some traces still crash.
                if (descriptor && (descriptor->GetClass() == DescriptorClass::PlainSampler)) {
                    const auto *sampler_state = static_cast<const SamplerDescriptor *>(descriptor)->GetSamplerState();
                    if (sampler_state) sampler_states.emplace_back(sampler_state);
                }
            }
        }
    }

    if ((!image_view_state && !dev_state.enabled_features.nullDescriptor) || (image_view_state && image_view_state->Destroyed())) {
        // Image view must have been destroyed since initial update. Could potentially flag the descriptor
        //  as "invalid" (updated = false) at DestroyImageView() time and detect this error at bind time

        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                  "the descriptor %s is using imageView %s that is invalid or has been destroyed.",
                                  DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str());
    }

    // ImageView could be null via nullDescriptor and accessing it is legal
    if (image_view == VK_NULL_HANDLE) {
        return skip;
    }
    if (!resource_variable.IsAccessed()) return skip;

    // If there is an non-null imageView, the image inside should be valid
    const auto image_state = image_view_state->image_state.get();
    ASSERT_AND_RETURN_SKIP(image_state);

    const auto &image_view_ci = image_view_state->create_info;

    const spv::Dim dim = resource_variable.info.image_dim;
    const bool is_image_array = resource_variable.info.is_image_array;

    // if combined sampler, this variable might not be a OpTypeImage
    // SubpassData gets validated elsewhere
    if (resource_variable.IsImage() && dim != spv::DimSubpassData) {
        bool valid_dim = true;
        // From vkspec.html#textures-operation-validation
        switch (image_view_ci.viewType) {
            case VK_IMAGE_VIEW_TYPE_1D:
                valid_dim = (dim == spv::Dim1D) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_2D:
                valid_dim = (dim == spv::Dim2D) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_3D:
                valid_dim = (dim == spv::Dim3D) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_CUBE:
                valid_dim = (dim == spv::DimCube) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
                valid_dim = (dim == spv::Dim1D) && is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
                valid_dim = (dim == spv::Dim2D) && is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
                valid_dim = (dim == spv::DimCube) && is_image_array;
                break;
            default:
                break;  // incase a new VkImageViewType is added, let it be valid by default
        }
        if (!valid_dim) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(
                vuids.image_view_dim_07752, objlist, loc,
                "the descriptor %s ImageView type is %s but the OpTypeImage has (Dim = %s) and (Arrayed = %" PRIu32 ").",
                DescribeDescriptor(resource_variable, index).c_str(), string_VkImageViewType(image_view_ci.viewType),
                string_SpvDim(dim), is_image_array);
        }

        if ((resource_variable.info.image_format_type & image_view_state->descriptor_format_bits) == 0) {
            const bool signed_override =
                ((resource_variable.info.image_format_type & spirv::NumericTypeUint) && resource_variable.info.is_sign_extended);
            const bool unsigned_override =
                ((resource_variable.info.image_format_type & spirv::NumericTypeSint) && resource_variable.info.is_zero_extended);
            if (!signed_override && !unsigned_override) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.image_view_numeric_format_07753, objlist, loc,
                                          "the descriptor %s requires %s component type, but bound descriptor format is %s.",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          spirv::string_NumericType(resource_variable.info.image_format_type),
                                          string_VkFormat(image_view_ci.format));
            }
        }

        const bool image_format_width_64 = vkuFormatHasComponentSize(image_view_ci.format, 64);
        if (image_format_width_64) {
            if (resource_variable.image_sampled_type_width != 64) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.image_view_access_64_04470, objlist, loc,
                                          "the descriptor %s has a 64-bit component ImageView format (%s) but the OpTypeImage's "
                                          "Sampled Type has a width of %" PRIu32 ".",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          string_VkFormat(image_view_ci.format), resource_variable.image_sampled_type_width);
            } else if (!dev_state.enabled_features.sparseImageInt64Atomics && image_state->sparse_residency) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, image_state->Handle());
                return dev_state.LogError(
                    vuids.image_view_sparse_64_04474, objlist, loc,
                    "the descriptor %s has a OpTypeImage's Sampled Type has a width of 64 backed by a sparse Image, but "
                    "sparseImageInt64Atomics is not enabled.",
                    DescribeDescriptor(resource_variable, index).c_str());
            }
        } else if (!image_format_width_64 && resource_variable.image_sampled_type_width != 32) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.image_view_access_32_04471, objlist, loc,
                                      "the descriptor %s has a 32-bit component ImageView format (%s) but the OpTypeImage's "
                                      "Sampled Type has a width of %" PRIu32 ".",
                                      DescribeDescriptor(resource_variable, index).c_str(), string_VkFormat(image_view_ci.format),
                                      resource_variable.image_sampled_type_width);
        }
    }

    if (!dev_state.disabled[image_layout_validation]) {
        VkImageLayout image_layout = image_descriptor.GetImageLayout();
        // Verify Image Layout
        // No "invalid layout" VUID required for this call, since the optimal_layout parameter is UNDEFINED.
        bool hit_error = false;
        dev_state.VerifyImageLayout(cb_state, *image_view_state, image_layout, loc,
                          "VUID-VkDescriptorImageInfo-imageLayout-00344", &hit_error);
        if (hit_error) {
            auto set = descriptor_set.Handle();
            std::stringstream msg;
            if (!descriptor_set.IsPushDescriptor()) {
                msg << "Descriptor set " << dev_state.FormatHandle(set)
                    << " Image layout specified by vkCmdBindDescriptorSets doesn't match actual image layout at time "
                       "descriptor is used";
            } else {
                msg << "Image layout specified by vkCmdPushDescriptorSet doesn't match actual image layout at time "
                       "descriptor is used";
            }
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                            "%s. See previous error callback for specific details.", msg.str().c_str());
        }
    }

    // Verify Sample counts
    if (resource_variable.IsImage()) {
        if (!resource_variable.info.is_multisampled && image_view_state->samples != VK_SAMPLE_COUNT_1_BIT) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError("VUID-RuntimeSpirv-samples-08725", objlist, loc, "the descriptor %s has %s created with %s.",
                                      DescribeDescriptor(resource_variable, index).c_str(),
                                      dev_state.FormatHandle(image_state->Handle()).c_str(),
                                      string_VkSampleCountFlagBits(image_view_state->samples));
        }
        if (resource_variable.info.is_multisampled && image_view_state->samples == VK_SAMPLE_COUNT_1_BIT) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(
                "VUID-RuntimeSpirv-samples-08726", objlist, loc, "the descriptor %s has %s created with VK_SAMPLE_COUNT_1_BIT.",
                DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_state->Handle()).c_str());
        }
    }

    if (image_view_state->samplerConversion) {
        if (resource_variable.info.is_not_sampler_sampled) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.image_ycbcr_sampled_06550, objlist, loc,
                                      "the image descriptor %s was created with a sampler Ycbcr conversion, but was accessed with "
                                      "a non OpImage*Sample* command.",
                                      DescribeDescriptor(resource_variable, index).c_str());
        }
        if (resource_variable.info.is_sampler_offset) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.image_ycbcr_offset_06551, objlist, loc,
                                      "the image descriptor %s was created with a sampler Ycbcr conversion, but was accessed with "
                                      "ConstOffset/Offset image operands.",
                                      DescribeDescriptor(resource_variable, index).c_str());
        }
    }

    // Verify VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
    if (resource_variable.IsAtomic() && (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) &&
        !(image_view_state->format_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, image_view);
        return dev_state.LogError(
            vuids.imageview_atomic_02691, objlist, loc,
            "the descriptor %s has %s with format of %s which is missing VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT.\n"
            "(supported features: %s).",
            DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
            string_VkFormat(image_view_ci.format), string_VkFormatFeatureFlags2(image_view_state->format_features).c_str());
    }

    // When KHR_format_feature_flags2 is supported, the read/write without
    // format support is reported per format rather than a single physical
    // device feature.
    if (dev_state.has_format_feature2) {
        const VkFormatFeatureFlags2 format_features = image_view_state->format_features;

        if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            if ((resource_variable.info.is_read_without_format) &&
                !(format_features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.storage_image_read_without_format_07028, objlist, loc,
                                          "the descriptor %s has %s with format of %s which doesn't support "
                                          "VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT.\n"
                                          "(supported features: %s).",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          dev_state.FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
                                          string_VkFormatFeatureFlags2(format_features).c_str());
            }

            if ((resource_variable.info.is_write_without_format) &&
                !(format_features & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.storage_image_write_without_format_07027, objlist, loc,
                                          "the descriptor %s has %s with format of %s which doesn't support "
                                          "VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT.\n"
                                          "(supported features: %s).",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          dev_state.FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
                                          string_VkFormatFeatureFlags2(format_features).c_str());
            }
        }

        if ((resource_variable.info.is_dref) && !(format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT)) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.depth_compare_sample_06479, objlist, loc,
                                      "the descriptor %s has %s with format of %s which doesn't support "
                                      "VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT.\n"
                                      "(supported features: %s).",
                                      DescribeDescriptor(resource_variable, index).c_str(),
                                      dev_state.FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
                                      string_VkFormatFeatureFlags2(format_features).c_str());
        }
    }

    const uint32_t binding_index = resource_variable.decorations.binding;
    // Verify if attachments are used in DescriptorSet
    if (!cb_state.active_attachments.empty() && !cb_state.active_subpasses.empty() &&
        (descriptor_type != VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        for (uint32_t att_index = 0; att_index < cb_state.active_attachments.size(); ++att_index) {
            const auto *view_state = cb_state.active_attachments[att_index].image_view;
            const SubpassInfo &subpass = cb_state.active_subpasses[att_index];
            if (!view_state || view_state->Destroyed()) {
                continue;
            }
            const bool same_view = view_state->VkHandle() == image_view;
            const bool overlapping_view = image_view_state->OverlapSubresource(*view_state);
            if (!same_view && !overlapping_view) {
                continue;
            }

            bool descriptor_written_to = false;
            const auto pipeline = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
            for (const auto &stage : pipeline->stage_states) {
                if (!stage.entrypoint) continue;
                for (const auto &interface_variable : stage.entrypoint->resource_interface_variables) {
                    if (interface_variable.decorations.set == set_index &&
                        interface_variable.decorations.binding == binding_index) {
                        descriptor_written_to |= interface_variable.IsWrittenTo();
                        break;  // only one set/binding will match
                    }
                }
            }

            const bool layout_read_only = IsImageLayoutReadOnly(subpass.layout);
            const bool read_attachment = (subpass.usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) != 0;
            if (read_attachment && descriptor_written_to) {
                if (same_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer);
                    return dev_state.LogError(vuids.image_subresources_subpass_write_06539, objlist, loc,
                                              "the descriptor %s has %s which will be read from as %s attachment %" PRIu32 ".",
                                              DescribeDescriptor(resource_variable, index).c_str(),
                                              dev_state.FormatHandle(image_view).c_str(),
                                              dev_state.FormatHandle(framebuffer).c_str(), att_index);
                } else if (overlapping_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer, view_state->Handle());
                    return dev_state.LogError(
                        vuids.image_subresources_subpass_write_06539, objlist, loc,
                        "the descriptor %s has %s which will be overlap read from as %s in %s attachment %" PRIu32 " overlap.",
                        DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
                        dev_state.FormatHandle(view_state->Handle()).c_str(), dev_state.FormatHandle(framebuffer).c_str(),
                        att_index);
                }
            }

            if (descriptor_written_to && !layout_read_only) {
                if (same_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer);
                    return dev_state.LogError(vuids.image_subresources_render_pass_write_06537, objlist, loc,
                                              "the descriptor %s has %s which is written to but is also %s attachment %" PRIu32 ".",
                                              DescribeDescriptor(resource_variable, index).c_str(),
                                              dev_state.FormatHandle(image_view).c_str(),
                                              dev_state.FormatHandle(framebuffer).c_str(), att_index);
                } else if (overlapping_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer, view_state->Handle());
                    return dev_state.LogError(
                        vuids.image_subresources_render_pass_write_06537, objlist, loc,
                        "the descriptor %s has %s which overlaps writes to %s but is also %s attachment %" PRIu32 ".",
                        DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
                        dev_state.FormatHandle(view_state->Handle()).c_str(), dev_state.FormatHandle(framebuffer).c_str(),
                        att_index);
                }
            }
        }
    }

    if (dev_state.enabled_features.protectedMemory == VK_TRUE) {
        if (dev_state.ValidateProtectedImage(cb_state, *image_state, loc, vuids.unprotected_command_buffer_02707,
                                             " (Image is in a descriptorSet)")) {
            return true;
        }
        if (resource_variable.IsWrittenTo() &&
            dev_state.ValidateUnprotectedImage(cb_state, *image_state, loc, vuids.protected_command_buffer_02712,
                                               " (Image is in a descriptorSet)")) {
            return true;
        }
    }

    const VkFormat image_view_format = image_view_state->create_info.format;
    for (const auto *sampler_state : sampler_states) {
        if (!sampler_state || sampler_state->Destroyed()) {
            continue;
        }

        // TODO: Validate 04015 for DescriptorClass::PlainSampler
        if ((sampler_state->create_info.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
             sampler_state->create_info.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) &&
            (sampler_state->customCreateInfo.format == VK_FORMAT_UNDEFINED)) {
            if (image_view_format == VK_FORMAT_B4G4R4A4_UNORM_PACK16 || image_view_format == VK_FORMAT_B5G6R5_UNORM_PACK16 ||
                image_view_format == VK_FORMAT_B5G5R5A1_UNORM_PACK16 || image_view_format == VK_FORMAT_A1B5G5R5_UNORM_PACK16) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                return dev_state.LogError(
                    "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015", objlist, loc,
                    "the descriptor %s has %s which has a custom border color with format = "
                    "VK_FORMAT_UNDEFINED and is used to sample an image "
                    "view %s with format %s",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                    dev_state.FormatHandle(image_view_state->Handle()).c_str(), string_VkFormat(image_view_format));
            }
        }
        const VkFilter sampler_mag_filter = sampler_state->create_info.magFilter;
        const VkFilter sampler_min_filter = sampler_state->create_info.minFilter;
        const bool sampler_compare_enable = sampler_state->create_info.compareEnable == VK_TRUE;
        const auto sampler_reduction =
            vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(sampler_state->create_info.pNext);
        // The VU is wording is a bit misleading, if there is no VkSamplerReductionModeCreateInfo we still need to check for linear
        // tiling feature
        const bool is_weighted_average =
            !sampler_reduction || sampler_reduction->reductionMode == VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
        if (!sampler_compare_enable && is_weighted_average &&
            !(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            if (sampler_mag_filter == VK_FILTER_LINEAR || sampler_min_filter == VK_FILTER_LINEAR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                return dev_state.LogError(
                    vuids.linear_filter_sampler_04553, objlist, loc,
                    "the descriptor %s has %s which is set to use VK_FILTER_LINEAR with compareEnable is set "
                    "to VK_FALSE, but image view's (%s) format (%s) does not contain "
                    "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT in its format features.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                    dev_state.FormatHandle(image_view_state->Handle()).c_str(), string_VkFormat(image_view_format));
            }
            if (sampler_state->create_info.mipmapMode == VK_SAMPLER_MIPMAP_MODE_LINEAR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                return dev_state.LogError(
                    vuids.linear_mipmap_sampler_04770, objlist, loc,
                    "the descriptor %s has %s which is set to use VK_SAMPLER_MIPMAP_MODE_LINEAR with "
                    "compareEnable is set to VK_FALSE, but image view's (%s) format (%s) does not contain "
                    "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT in its format features.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                    dev_state.FormatHandle(image_view_state->Handle()).c_str(), string_VkFormat(image_view_format));
            }
        }

        const bool is_minmax = sampler_reduction && (sampler_reduction->reductionMode == VK_SAMPLER_REDUCTION_MODE_MIN ||
                                                     sampler_reduction->reductionMode == VK_SAMPLER_REDUCTION_MODE_MAX);
        if (is_minmax && !(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT)) {
            if (sampler_mag_filter == VK_FILTER_LINEAR || sampler_min_filter == VK_FILTER_LINEAR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                return dev_state.LogError(
                    vuids.linear_filter_sampler_09598, objlist, loc,
                    "the descriptor %s has %s which is set to use VK_FILTER_LINEAR with reductionMode is set "
                    "to %s, but image view's (%s) format (%s) does not contain "
                    "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT in its format features.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                    string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                    dev_state.FormatHandle(image_view_state->Handle()).c_str(), string_VkFormat(image_view_format));
            }
            if (sampler_state->create_info.mipmapMode == VK_SAMPLER_MIPMAP_MODE_LINEAR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                return dev_state.LogError(
                    vuids.linear_mipmap_sampler_09599, objlist, loc,
                    "the descriptor %s has %s which is set to use VK_SAMPLER_MIPMAP_MODE_LINEAR with "
                    "reductionMode is set to %s, but image view's (%s) format (%s) does not contain "
                    "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT in its format features.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                    string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                    dev_state.FormatHandle(image_view_state->Handle()).c_str(), string_VkFormat(image_view_format));
            }
        }

        if (sampler_mag_filter == VK_FILTER_CUBIC_EXT || sampler_min_filter == VK_FILTER_CUBIC_EXT) {
            if (!(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                return dev_state.LogError(
                    vuids.cubic_sampler_02692, objlist, loc,
                    "the descriptor %s has %s which is set to use VK_FILTER_CUBIC_EXT, then image view's (%s) format (%s) "
                    "MUST contain VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT in its format features.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                    dev_state.FormatHandle(image_view_state->Handle()).c_str(),
                    string_VkFormat(image_view_state->create_info.format));
            }

            if (IsExtEnabled(dev_state.extensions.vk_ext_filter_cubic)) {
                const auto reduction_mode_info =
                    vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(sampler_state->create_info.pNext);
                if (reduction_mode_info &&
                    (reduction_mode_info->reductionMode == VK_SAMPLER_REDUCTION_MODE_MIN ||
                     reduction_mode_info->reductionMode == VK_SAMPLER_REDUCTION_MODE_MAX) &&
                    !image_view_state->filter_cubic_props.filterCubicMinmax) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                    return dev_state.LogError(
                        vuids.filter_cubic_min_max_02695, objlist, loc,
                        "the descriptor %s has %s which is set to use VK_FILTER_CUBIC_EXT & %s, but image view "
                        "(%s) doesn't support filterCubicMinmax.",
                        DescribeDescriptor(resource_variable, index).c_str(),
                        dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                        string_VkSamplerReductionMode(reduction_mode_info->reductionMode),
                        dev_state.FormatHandle(image_view_state->Handle()).c_str());
                }

                if (!image_view_state->filter_cubic_props.filterCubic) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                    return dev_state.LogError(
                        vuids.filter_cubic_02694, objlist, loc,
                        "the descriptor %s has %s which is set to use VK_FILTER_CUBIC_EXT, but image view (%s) "
                        "doesn't support filterCubic.",
                        DescribeDescriptor(resource_variable, index).c_str(),
                        dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                        dev_state.FormatHandle(image_view_state->Handle()).c_str());
                }
            }

            if (IsExtEnabled(dev_state.extensions.vk_img_filter_cubic)) {
                if (image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_3D ||
                    image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                    image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, sampler_state->Handle(), image_view_state->Handle());
                    return dev_state.LogError(
                        vuids.img_filter_cubic_02693, objlist, loc,
                        "the descriptor %s has %s which is set to use VK_FILTER_CUBIC_EXT while the VK_IMG_filter_cubic "
                        "extension is enabled, but image view (%s) has an invalid imageViewType (%s).",
                        DescribeDescriptor(resource_variable, index).c_str(),
                        dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                        dev_state.FormatHandle(image_view_state->Handle()).c_str(),
                        string_VkImageViewType(image_view_state->create_info.viewType));
                }
            }
        }
        if ((image_state->create_info.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) &&
            (sampler_state->create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
             sampler_state->create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
             sampler_state->create_info.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)) {
            std::string address_mode_letter =
                (sampler_state->create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)   ? "U"
                : (sampler_state->create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ? "V"
                                                                                                     : "W";
            VkSamplerAddressMode address_mode = (sampler_state->create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                                                    ? sampler_state->create_info.addressModeU
                                                : (sampler_state->create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                                                    ? sampler_state->create_info.addressModeV
                                                    : sampler_state->create_info.addressModeW;
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, sampler_state->Handle(), image_state->Handle(), image_view_state->Handle());
            return dev_state.LogError(
                vuids.corner_sampled_address_mode_02696, objlist, loc,
                "the descriptor %s image (%s) in image view (%s) is created with flag "
                "VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and can only be sampled using "
                "VK_SAMPLER_ADDRESS_MODE_CLAMP_EDGE, but sampler (%s) has "
                "pCreateInfo->addressMode%s set to %s.",
                DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_state->Handle()).c_str(),
                dev_state.FormatHandle(image_view_state->Handle()).c_str(), dev_state.FormatHandle(sampler_state->Handle()).c_str(),
                address_mode_letter.c_str(), string_VkSamplerAddressMode(address_mode));
        }

        // UnnormalizedCoordinates sampler validations
        // only check if sampled as could have a texelFetch on a combined image sampler
        if (sampler_state->create_info.unnormalizedCoordinates && resource_variable.info.is_sampler_sampled) {
            // If ImageView is used by a unnormalizedCoordinates sampler, it needs to check ImageView type
            if (image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_3D || image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY || image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
                image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(
                    vuids.sampler_imageview_type_08609, objlist, loc, "the descriptor %s Image View %s, type %s, is used by %s.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
                    string_VkImageViewType(image_view_ci.viewType), dev_state.FormatHandle(sampler_state->Handle()).c_str());
            }

            const auto &subresource_range = image_view_state->normalized_subresource_range;
            if (subresource_range.levelCount != 1) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(
                    vuids.unnormalized_coordinates_09635, objlist, loc,
                    "the descriptor %s Image View %s was created with levelCount of %s, but the sampler (%s) was created with "
                    "unnormalizedCoordinates.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
                    string_LevelCount(image_state->create_info, image_view_ci.subresourceRange).c_str(),
                    dev_state.FormatHandle(sampler_state->Handle()).c_str());
            }

            if (subresource_range.layerCount != 1) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(
                    vuids.unnormalized_coordinates_09635, objlist, loc,
                    "the descriptor %s Image View %s was created with layerCount of %s, but the sampler (%s) was created with "
                    "unnormalizedCoordinates.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
                    string_LayerCount(image_state->create_info, image_view_ci.subresourceRange).c_str(),
                    dev_state.FormatHandle(sampler_state->Handle()).c_str());
            }

            // sampler must not be used with any of the SPIR-V OpImageSample* or OpImageSparseSample*
            // instructions with ImplicitLod, Dref or Proj in their name
            if (resource_variable.info.is_sampler_implicitLod_dref_proj) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(vuids.sampler_implicitLod_dref_proj_08610, objlist, loc,
                                          "the descriptor %s Image View %s is used by %s that uses invalid operator.",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          dev_state.FormatHandle(image_view).c_str(),
                                          dev_state.FormatHandle(sampler_state->Handle()).c_str());
            }

            // sampler must not be used with any of the SPIR-V OpImageSample* or OpImageSparseSample*
            // instructions that includes a LOD bias or any offset values
            if (resource_variable.info.is_sampler_bias_offset) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(
                    vuids.sampler_bias_offset_08611, objlist, loc,
                    "the descriptor %s Image View %s is used by %s that uses invalid bias or offset operator.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(image_view).c_str(),
                    dev_state.FormatHandle(sampler_state->Handle()).c_str());
            }
        }

        if (sampler_state->samplerConversion) {
            if (resource_variable.info.is_not_sampler_sampled) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(vuids.image_ycbcr_sampled_06550, set, loc,
                                          "the sampler descriptor %s was created with a sampler Ycbcr conversion, but was accessed "
                                          "with a non OpImage*Sample* command.",
                                          DescribeDescriptor(resource_variable, index).c_str());
            }
            if (resource_variable.info.is_sampler_offset) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->Handle());
                return dev_state.LogError(vuids.image_ycbcr_offset_06551, set, loc,
                                          "the sampler descriptor %s was created with a sampler Ycbcr conversion, but was accessed "
                                          "with ConstOffset/Offset image operands.",
                                          DescribeDescriptor(resource_variable, index).c_str());
            }
        }
    }

    for (const uint32_t texel_component_count : resource_variable.write_without_formats_component_count_list) {
        const uint32_t format_component_count = vkuFormatComponentCount(image_view_format);
        if (image_view_format == VK_FORMAT_A8_UNORM) {
            if (texel_component_count != 4) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.storage_image_write_texel_count_08796, objlist, loc,
                                          "the descriptor %s VkImageView is mapped to a OpImage format of VK_FORMAT_A8_UNORM, "
                                          "but the OpImageWrite Texel "
                                          "operand only contains %" PRIu32 " components.",
                                          DescribeDescriptor(resource_variable, index).c_str(), texel_component_count);
            }
        } else if (texel_component_count < format_component_count) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.storage_image_write_texel_count_08795, objlist, loc,
                                      "the descriptor %s VkImageView is mapped to a OpImage format of %s which has %" PRIu32
                                      " components, but the OpImageWrite Texel operand only contains %" PRIu32 " components.",
                                      DescribeDescriptor(resource_variable, index).c_str(), string_VkFormat(image_view_format),
                                      format_component_count, texel_component_count);
        }
    }

    return false;
}

bool DescriptorValidator::ValidateDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, const uint32_t index,
                                             VkDescriptorType descriptor_type, const ImageSamplerDescriptor &descriptor) const {
    bool skip = false;
    skip |= ValidateDescriptor(resource_variable, index, descriptor_type, static_cast<const ImageDescriptor &>(descriptor));
    if (skip) {
        return skip;
    }
    skip |= ValidateSamplerDescriptor(resource_variable, index, descriptor.GetSampler(), descriptor.IsImmutableSampler(),
                                      descriptor.GetSamplerState());
    return skip;
}

bool DescriptorValidator::ValidateDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, const uint32_t index,
                                             VkDescriptorType descriptor_type, const TexelDescriptor &texel_descriptor) const {
    const VkBufferView buffer_view = texel_descriptor.GetBufferView();
    auto buffer_view_state = texel_descriptor.GetBufferViewState();
    if ((!buffer_view_state && !dev_state.enabled_features.nullDescriptor) || (buffer_view_state && buffer_view_state->Destroyed())) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                  "the descriptor %s is using bufferView %s that is invalid or has been destroyed.",
                                  DescribeDescriptor(resource_variable, index).c_str(),
                                  dev_state.FormatHandle(buffer_view).c_str());
    }

    // BufferView could be null via nullDescriptor and accessing it is legal
    if (buffer_view == VK_NULL_HANDLE) {
        return false;
    }
    if (!resource_variable.IsAccessed()) return false;

    auto buffer = buffer_view_state->create_info.buffer;
    const auto *buffer_state = buffer_view_state->buffer_state.get();
    if (!buffer_state || buffer_state->Destroyed()) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                  "the descriptor %s is using buffer %s that has been destroyed.",
                                  DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(buffer).c_str());
    }
    const VkFormat buffer_view_format = buffer_view_state->create_info.format;
    const uint32_t format_bits = spirv::GetFormatType(buffer_view_format);

    if ((resource_variable.info.image_format_type & format_bits) == 0) {
        const bool signed_override =
            ((resource_variable.info.image_format_type & spirv::NumericTypeUint) && resource_variable.info.is_sign_extended);
        const bool unsigned_override =
            ((resource_variable.info.image_format_type & spirv::NumericTypeSint) && resource_variable.info.is_zero_extended);
        if (!signed_override && !unsigned_override) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                      "the descriptor %s requires %s component type, but bound descriptor format is %s.",
                                      DescribeDescriptor(resource_variable, index).c_str(),
                                      spirv::string_NumericType(resource_variable.info.image_format_type),
                                      string_VkFormat(buffer_view_format));
        }
    }

    const bool buffer_format_width_64 = vkuFormatHasComponentSize(buffer_view_format, 64);
    if (buffer_format_width_64 && resource_variable.image_sampled_type_width != 64) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, buffer_view);
        return dev_state.LogError(vuids.buffer_view_access_64_04472, objlist, loc,
                                  "the descriptor %s has a 64-bit component BufferView format (%s) but the OpTypeImage's Sampled "
                                  "Type has a width of %" PRIu32 ".",
                                  DescribeDescriptor(resource_variable, index).c_str(), string_VkFormat(buffer_view_format),
                                  resource_variable.image_sampled_type_width);
    } else if (!buffer_format_width_64 && resource_variable.image_sampled_type_width != 32) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, buffer_view);
        return dev_state.LogError(vuids.buffer_view_access_32_04473, objlist, loc,
                                  "the descripto %s has a 32-bit component BufferView format (%s) but the OpTypeImage's Sampled "
                                  "Type has a width of %" PRIu32 ".",
                                  DescribeDescriptor(resource_variable, index).c_str(), string_VkFormat(buffer_view_format),
                                  resource_variable.image_sampled_type_width);
    }

    const VkFormatFeatureFlags2 buffer_format_features = buffer_view_state->buffer_format_features;

    // Verify VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
    if ((resource_variable.IsAtomic()) && (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) &&
        !(buffer_format_features & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, buffer_view);
        return dev_state.LogError(
            vuids.bufferview_atomic_07888, objlist, loc,
            "the descriptor %s has %s with format of %s which is missing VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT.\n"
            "(supported features: %s).",
            DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(buffer_view).c_str(),
            string_VkFormat(buffer_view_format), string_VkFormatFeatureFlags2(buffer_format_features).c_str());
    }

    // When KHR_format_feature_flags2 is supported, the read/write without
    // format support is reported per format rather than a single physical
    // device feature.
    if (dev_state.has_format_feature2) {
        if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
            if ((resource_variable.info.is_read_without_format) &&
                !(buffer_format_features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, buffer_view);
                return dev_state.LogError(vuids.storage_texel_buffer_read_without_format_07030, objlist, loc,
                                          "the descriptor %s has %s with format of %s which is missing "
                                          "VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR.\n"
                                          "(supported features: %s).",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          dev_state.FormatHandle(buffer_view).c_str(), string_VkFormat(buffer_view_format),
                                          string_VkFormatFeatureFlags2(buffer_format_features).c_str());
            }

            if ((resource_variable.info.is_write_without_format) &&
                !(buffer_format_features & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, buffer_view);
                return dev_state.LogError(vuids.storage_texel_buffer_write_without_format_07029, objlist, loc,
                                          "the descriptor %s has %s with format of %s which is missing "
                                          "VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT.\n"
                                          "(supported features: %s).",
                                          DescribeDescriptor(resource_variable, index).c_str(),
                                          dev_state.FormatHandle(buffer_view).c_str(), string_VkFormat(buffer_view_format),
                                          string_VkFormatFeatureFlags2(buffer_format_features).c_str());
            }
        }
    }

    if (dev_state.enabled_features.protectedMemory == VK_TRUE && buffer_view_state->buffer_state) {
        if (dev_state.ValidateProtectedBuffer(cb_state, *buffer_view_state->buffer_state, loc,
                                              vuids.unprotected_command_buffer_02707, " (Buffer is in a descriptorSet)")) {
            return true;
        }
        if (resource_variable.IsWrittenTo() &&
            dev_state.ValidateUnprotectedBuffer(cb_state, *buffer_view_state->buffer_state, loc,
                                                vuids.protected_command_buffer_02712, " (Buffer is in a descriptorSet)")) {
            return true;
        }
    }

    for (const uint32_t texel_component_count : resource_variable.write_without_formats_component_count_list) {
        const uint32_t format_component_count = vkuFormatComponentCount(buffer_view_format);
        if (texel_component_count < format_component_count) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, buffer_view);
            return dev_state.LogError(vuids.storage_texel_buffer_write_texel_count_04469, objlist, loc,
                                      "the descriptor %s VkImageView is mapped to a OpImage format of %s which has %" PRIu32
                                      " components, but the OpImageWrite Texel operand only contains %" PRIu32 " components.",
                                      DescribeDescriptor(resource_variable, index).c_str(), string_VkFormat(buffer_view_format),
                                      format_component_count, texel_component_count);
        }
    }

    return false;
}

bool DescriptorValidator::ValidateDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, const uint32_t index,
                                             VkDescriptorType descriptor_type,
                                             const AccelerationStructureDescriptor &descriptor) const {
    // Verify that acceleration structures are valid
    if (descriptor.IsKHR()) {
        auto acc = descriptor.GetAccelerationStructure();
        auto acc_node = descriptor.GetAccelerationStructureStateKHR();
        if (!acc_node || acc_node->Destroyed()) {
            // the AccelerationStructure could be null via nullDescriptor and accessing it is legal
            if (acc != VK_NULL_HANDLE || !dev_state.enabled_features.nullDescriptor) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(
                    vuids.descriptor_buffer_bit_set_08114, set, loc,
                    "the descriptor %s is using acceleration structure %s that is invalid or has been destroyed.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(acc).c_str());
            }
        } else if (acc_node->buffer_state) {
            for (const auto &mem_binding : acc_node->buffer_state->GetInvalidMemory()) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                          "the descriptor %s is using acceleration structure %s that references invalid memory %s.",
                                          DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(acc).c_str(),
                                          dev_state.FormatHandle(mem_binding->Handle()).c_str());
            }
        }
    } else {
        auto acc = descriptor.GetAccelerationStructureNV();
        auto acc_node = descriptor.GetAccelerationStructureStateNV();
        if (!acc_node || acc_node->Destroyed()) {
            // the AccelerationStructure could be null via nullDescriptor and accessing it is legal
            if (acc != VK_NULL_HANDLE || !dev_state.enabled_features.nullDescriptor) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(
                    vuids.descriptor_buffer_bit_set_08114, set, loc,
                    "the descriptor %s is using acceleration structure %s that is invalid or has been destroyed.",
                    DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(acc).c_str());
            }
        } else {
            for (const auto &mem_binding : acc_node->GetInvalidMemory()) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                          "the descriptor %s is using acceleration structure %s that references invalid memory %s.",
                                          DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(acc).c_str(),
                                          dev_state.FormatHandle(mem_binding->Handle()).c_str());
            }
        }
    }
    return false;
}

// If the validation is related to both of image and sampler,
// please leave it in (descriptor_class == DescriptorClass::ImageSampler || descriptor_class ==
// DescriptorClass::Image) Here is to validate for only sampler.
bool DescriptorValidator::ValidateSamplerDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, uint32_t index,
                                                    VkSampler sampler, bool is_immutable, const Sampler *sampler_state) const {
    // Verify Sampler still valid
    if (!sampler_state || sampler_state->Destroyed()) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                  "the descriptor %s is using sampler %s that is invalid or has been destroyed.",
                                  DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler).c_str());
    } else {
        if (sampler_state->samplerConversion && !is_immutable) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                      "the descriptor %s sampler (%s) contains a YCBCR conversion (%s), but the sampler is not an "
                                      "immutable sampler.",
                                      DescribeDescriptor(resource_variable, index).c_str(), dev_state.FormatHandle(sampler).c_str(),
                                      dev_state.FormatHandle(sampler_state->samplerConversion).c_str());
        }
    }
    return false;
}

bool DescriptorValidator::ValidateDescriptor(const spirv::ResourceInterfaceVariable &resource_variable, const uint32_t index,
                                             VkDescriptorType descriptor_type, const SamplerDescriptor &descriptor) const {
    return ValidateSamplerDescriptor(resource_variable, index, descriptor.GetSampler(), descriptor.IsImmutableSampler(),
                                     descriptor.GetSamplerState());
}

}  // namespace vvl
