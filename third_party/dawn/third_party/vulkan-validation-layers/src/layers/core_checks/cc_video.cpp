/* Copyright (c) 2022-2024 The Khronos Group Inc.
 * Copyright (c) 2022-2024 RasterGrid Kft.
 * Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <assert.h>
#include <vector>
#include <type_traits>

#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "error_message/error_strings.h"
#include "error_message/logging.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "generated/dispatch_functions.h"

// Flags validation error if the associated call is made inside a video coding block.
// The apiName routine should ONLY be called outside a video coding block.
bool CoreChecks::InsideVideoCodingScope(const vvl::CommandBuffer &cb_state, const Location &loc, const char *vuid) const {
    bool inside = false;
    if (cb_state.bound_video_session) {
        inside = LogError(vuid, cb_state.Handle(), loc, "It is invalid to issue this call inside a video coding block.");
    }
    return inside;
}

// Flags validation error if the associated call is made outside a video coding block.
// The apiName routine should ONLY be called inside a video coding block.
bool CoreChecks::OutsideVideoCodingScope(const vvl::CommandBuffer &cb_state, const Location &loc, const char *vuid) const {
    bool outside = false;
    if (!cb_state.bound_video_session) {
        outside = LogError(vuid, cb_state.Handle(), loc, "This call must be issued inside a video coding block.");
    }
    return outside;
}

std::vector<VkVideoFormatPropertiesKHR> CoreChecks::GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                             const VkVideoProfileListInfoKHR *profile_list) const {
    VkPhysicalDeviceVideoFormatInfoKHR format_info = vku::InitStructHelper();
    format_info.imageUsage = image_usage;
    format_info.pNext = profile_list;

    uint32_t format_count = 0;
    DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &format_info, &format_count, nullptr);
    std::vector<VkVideoFormatPropertiesKHR> format_props(format_count, vku::InitStruct<VkVideoFormatPropertiesKHR>());
    DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &format_info, &format_count, format_props.data());

    return format_props;
}

std::vector<VkVideoFormatPropertiesKHR> CoreChecks::GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                             const VkVideoProfileInfoKHR *profile) const {
    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = profile;

    return GetVideoFormatProperties(image_usage, &profile_list);
}

bool CoreChecks::IsSupportedVideoFormat(const VkImageCreateInfo &image_ci, const VkVideoProfileListInfoKHR *profile_list) const {
    auto format_props_list = GetVideoFormatProperties(image_ci.usage, profile_list);

    for (auto &format_props : format_props_list) {
        const VkImageCreateFlags allowed_flags = format_props.imageCreateFlags | VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
        const bool compatible_usage = (image_ci.flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT) ||
                                      ((image_ci.usage & format_props.imageUsageFlags) == image_ci.usage);
        if (image_ci.format == format_props.format && (image_ci.flags & allowed_flags) == image_ci.flags &&
            image_ci.imageType == format_props.imageType && image_ci.tiling == format_props.imageTiling && compatible_usage) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::IsSupportedVideoFormat(const VkImageCreateInfo &image_ci, const VkVideoProfileInfoKHR *profile) const {
    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = profile;

    return IsSupportedVideoFormat(image_ci, &profile_list);
}

bool CoreChecks::IsVideoFormatSupported(VkFormat format, VkImageUsageFlags image_usage,
                                        const VkVideoProfileInfoKHR *profile) const {
    auto format_props_list = GetVideoFormatProperties(image_usage, profile);
    for (const auto &format_props : format_props_list) {
        if (format_props.format == format) return true;
    }
    return false;
}

bool CoreChecks::IsBufferCompatibleWithVideoSession(const vvl::Buffer &buffer_state, const vvl::VideoSession &vs_state) const {
    return (buffer_state.create_info.flags & VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR) ||
           buffer_state.supported_video_profiles.find(vs_state.profile) != buffer_state.supported_video_profiles.end();
}

bool CoreChecks::IsImageCompatibleWithVideoSession(const vvl::Image &image_state, const vvl::VideoSession &vs_state) const {
    if (image_state.create_info.flags & VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR) {
        return IsSupportedVideoFormat(image_state.create_info, vs_state.create_info.pVideoProfile);
    } else {
        return image_state.supported_video_profiles.find(vs_state.profile) != image_state.supported_video_profiles.end();
    }
}

void CoreChecks::EnqueueVerifyVideoSessionInitialized(vvl::CommandBuffer &cb_state, vvl::VideoSession &vs_state,
                                                      const Location &loc, const char *vuid) {
    cb_state.video_session_updates[vs_state.VkHandle()].emplace_back(
        [loc, vuid](const vvl::Device &dev_data, const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
                    bool do_validate) {
            bool skip = false;
            if (!dev_state.IsInitialized()) {
                skip |= dev_data.LogError(vuid, vs_state->Handle(), loc, "Bound video session %s is uninitialized.",
                                          dev_data.FormatHandle(*vs_state).c_str());
            }
            return skip;
        });
}

void CoreChecks::EnqueueVerifyVideoInlineQueryUnavailable(vvl::CommandBuffer &cb_state, const VkVideoInlineQueryInfoKHR &query_info,
                                                          Func command) {
    if (disabled[query_validation]) return;
    cb_state.query_updates.emplace_back([query_info, command](vvl::CommandBuffer &cb_state_arg, bool do_validate,
                                                              VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                              QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        for (uint32_t i = 0; i < query_info.queryCount; i++) {
            QueryObject query_obj = {query_info.queryPool, query_info.firstQuery + i, perfPass};
            skip |= VerifyQueryIsReset(cb_state_arg, query_obj, command, firstPerfQueryPool, perfPass, localQueryToStateMap);
        }
        return skip;
    });
}

bool CoreChecks::ValidateVideoInlineQueryInfo(const vvl::QueryPool &query_pool_state, const VkVideoInlineQueryInfoKHR &query_info,
                                              const Location &loc) const {
    bool skip = false;

    if (query_info.firstQuery >= query_pool_state.create_info.queryCount) {
        skip |= LogError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08372", query_pool_state.Handle(), loc.dot(Field::firstQuery),
                         "(%u) is greater than or equal to the number of queries (%u) in %s.", query_info.firstQuery,
                         query_pool_state.create_info.queryCount, FormatHandle(query_pool_state).c_str());
    }

    if (query_info.firstQuery + query_info.queryCount > query_pool_state.create_info.queryCount) {
        skip |= LogError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373", query_pool_state.Handle(), loc.dot(Field::firstQuery),
                         "(%u) plus queryCount (%u) is greater than the number of queries (%u) in %s.", query_info.firstQuery,
                         query_info.queryCount, query_pool_state.create_info.queryCount, FormatHandle(query_pool_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfo(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                    VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                    const Location &loc) const {
    bool skip = false;

    const Location rc_info_loc = loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (rc_info.layerCount > profile_caps.encode.maxRateControlLayers) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245", objlist, rc_info_loc.dot(Field::layerCount),
                         "(%u) is greater than the maxRateControlLayers (%u) "
                         "supported by the video profile %s was created with.",
                         rc_info.layerCount, profile_caps.encode.maxRateControlLayers, FormatHandle(vs_state).c_str());
    }

    if ((rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR ||
         rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) &&
        rc_info.layerCount != 0) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248", cmdbuf, rc_info_loc.dot(Field::rateControlMode),
                     "is %s, but %s (%u) is not zero.", string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_info.rateControlMode),
                     rc_info_loc.dot(Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    if (rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR ||
        rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR) {
        if (rc_info.layerCount != 0) {
            for (uint32_t layer_idx = 0; layer_idx < rc_info.layerCount; ++layer_idx) {
                skip |= ValidateVideoEncodeRateControlLayerInfo(layer_idx, rc_info, pNext, cmdbuf, vs_state, rc_info_loc);
            }
        } else {
            skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275", cmdbuf,
                             rc_info_loc.dot(Field::rateControlMode), "is %s, but %s is zero.",
                             string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_info.rateControlMode),
                             rc_info_loc.dot(Field::layerCount).Fields().c_str());
        }
    }

    if (rc_info.rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR) {
        switch (vs_state.GetCodecOp()) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                skip |= ValidateVideoEncodeRateControlInfoH264(rc_info, pNext, cmdbuf, vs_state, loc);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                skip |= ValidateVideoEncodeRateControlInfoH265(rc_info, pNext, cmdbuf, vs_state, loc);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
                skip |= ValidateVideoEncodeRateControlInfoAV1(rc_info, pNext, cmdbuf, vs_state, loc);
                break;

            default:
                assert(false);
                break;
        }
        if ((profile_caps.encode.rateControlModes & rc_info.rateControlMode) == 0) {
            const LogObjectList objlist(cmdbuf, vs_state.Handle());
            skip |=
                LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244", objlist,
                         rc_info_loc.dot(Field::rateControlMode), "(%s) is not supported by the video profile %s was created with.",
                         string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_info.rateControlMode), FormatHandle(vs_state).c_str());
        }
    }

    if (rc_info.layerCount != 0) {
        if (rc_info.virtualBufferSizeInMs == 0) {
            skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08357", cmdbuf,
                             rc_info_loc.dot(Field::virtualBufferSizeInMs), "must not be zero if %s (%u) is not zero.",
                             rc_info_loc.dot(Field::layerCount).Fields().c_str(), rc_info.layerCount);
        }
        if (rc_info.initialVirtualBufferSizeInMs > rc_info.virtualBufferSizeInMs) {
            skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08358", cmdbuf,
                             rc_info_loc.dot(Field::initialVirtualBufferSizeInMs),
                             "(%u) must be less than or equal to virtualBufferSizeInMs (%u) if %s (%u) is not zero.",
                             rc_info.initialVirtualBufferSizeInMs, rc_info.virtualBufferSizeInMs,
                             rc_info_loc.dot(Field::layerCount).Fields().c_str(), rc_info.layerCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfoH264(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                        VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                        const Location &loc) const {
    bool skip = false;

    const auto rc_info_h264 = vku::FindStructInPNextChain<VkVideoEncodeH264RateControlInfoKHR>(pNext);
    if (rc_info_h264 == nullptr) return false;

    const auto rc_info_h264_loc = loc.pNext(Struct::VkVideoEncodeH264RateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR &&
        (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280", objlist, rc_info_h264_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H264_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR but HRD compliance "
                         "is not supported by the H.264 encode profile %s was created with.",
                         FormatHandle(vs_state).c_str());
    }

    if ((rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR ||
         rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) &&
        (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR) == 0) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08281", cmdbuf, rc_info_h264_loc.dot(Field::flags),
                         "(%s) specifies a reference pattern but does not indicate the use of a regular GOP structure.",
                         string_VkVideoEncodeH264RateControlFlagsKHR(rc_info_h264->flags).c_str());
    }

    if (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR &&
        rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08282", cmdbuf, rc_info_h264_loc.dot(Field::flags),
                         "(%s) indicates conflicting reference patterns.",
                         string_VkVideoEncodeH264RateControlFlagsKHR(rc_info_h264->flags).c_str());
    }

    if (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR && rc_info_h264->gopFrameCount == 0) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08283", cmdbuf, rc_info_h264_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR but the GOP size "
                         "specified in %s is zero.",
                         rc_info_h264_loc.dot(Field::gopFrameCount).Fields().c_str());
    }

    if (rc_info_h264->idrPeriod != 0 && rc_info_h264->idrPeriod < rc_info_h264->gopFrameCount) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-idrPeriod-08284", cmdbuf, rc_info_h264_loc.dot(Field::idrPeriod),
                         "(%u) is not zero and the specified IDR period smaller than the GOP size specified in %s (%u).",
                         rc_info_h264->idrPeriod, rc_info_h264_loc.dot(Field::gopFrameCount).Fields().c_str(),
                         rc_info_h264->gopFrameCount);
    }

    if (rc_info_h264->consecutiveBFrameCount != 0 && rc_info_h264->consecutiveBFrameCount >= rc_info_h264->gopFrameCount) {
        skip |=
            LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-consecutiveBFrameCount-08285", cmdbuf,
                     rc_info_h264_loc.dot(Field::consecutiveBFrameCount),
                     "(%u) is greater than or equal to the GOP size specified in %s (%u).", rc_info_h264->consecutiveBFrameCount,
                     rc_info_h264_loc.dot(Field::gopFrameCount).Fields().c_str(), rc_info_h264->gopFrameCount);
    }

    if (rc_info.layerCount > 1 && rc_info.layerCount != rc_info_h264->temporalLayerCount) {
        skip |= LogError(
            "VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022", cmdbuf,
            rc_info_h264_loc.dot(Field::temporalLayerCount), "(%u) does not match %s (%u).", rc_info_h264->temporalLayerCount,
            loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR, Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfoH265(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                        VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                        const Location &loc) const {
    bool skip = false;

    const auto rc_info_h265 = vku::FindStructInPNextChain<VkVideoEncodeH265RateControlInfoKHR>(pNext);
    if (rc_info_h265 == nullptr) return false;

    const auto rc_info_h265_loc = loc.pNext(Struct::VkVideoEncodeH265RateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR &&
        (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291", objlist, rc_info_h265_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H265_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR but HRD compliance "
                         "is not supported by the H.265 encode profile %s was created with.",
                         FormatHandle(vs_state).c_str());
    }

    if ((rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR ||
         rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) &&
        (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR) == 0) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08292", cmdbuf, rc_info_h265_loc.dot(Field::flags),
                         "(%s) specifies a reference pattern but does not indicate the use of a regular GOP structure.",
                         string_VkVideoEncodeH265RateControlFlagsKHR(rc_info_h265->flags).c_str());
    }

    if (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR &&
        rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08293", cmdbuf, rc_info_h265_loc.dot(Field::flags),
                         "(%s) indicates conflicting reference patterns.",
                         string_VkVideoEncodeH265RateControlFlagsKHR(rc_info_h265->flags).c_str());
    }

    if (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR && rc_info_h265->gopFrameCount == 0) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08294", cmdbuf, rc_info_h265_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR but the GOP size "
                         "specified in %s is zero.",
                         rc_info_h265_loc.dot(Field::gopFrameCount).Fields().c_str());
    }

    if (rc_info_h265->idrPeriod != 0 && rc_info_h265->idrPeriod < rc_info_h265->gopFrameCount) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-idrPeriod-08295", cmdbuf, rc_info_h265_loc.dot(Field::idrPeriod),
                         "(%u) is not zero and the specified IDR period smaller than the GOP size specified in %s (%u).",
                         rc_info_h265->idrPeriod, rc_info_h265_loc.dot(Field::gopFrameCount).Fields().c_str(),
                         rc_info_h265->gopFrameCount);
    }

    if (rc_info_h265->consecutiveBFrameCount != 0 && rc_info_h265->consecutiveBFrameCount >= rc_info_h265->gopFrameCount) {
        skip |=
            LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-consecutiveBFrameCount-08296", cmdbuf,
                     rc_info_h265_loc.dot(Field::consecutiveBFrameCount),
                     "(%u) is greater than or equal to the GOP size specified in %s (%u).", rc_info_h265->consecutiveBFrameCount,
                     rc_info_h265_loc.dot(Field::gopFrameCount).Fields().c_str(), rc_info_h265->gopFrameCount);
    }

    if (rc_info.layerCount > 1 && rc_info.layerCount != rc_info_h265->subLayerCount) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025", cmdbuf,
                     rc_info_h265_loc.dot(Field::subLayerCount), "(%u) does not match %s (%u).", rc_info_h265->subLayerCount,
                     loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR, Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfoAV1(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                       VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                       const Location &loc) const {
    bool skip = false;

    const auto rc_info_av1 = vku::FindStructInPNextChain<VkVideoEncodeAV1RateControlInfoKHR>(pNext);
    if (rc_info_av1 == nullptr) return false;

    const auto rc_info_av1_loc = loc.pNext(Struct::VkVideoEncodeAV1RateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if ((rc_info_av1->flags & VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR ||
         rc_info_av1->flags & VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) &&
        (rc_info_av1->flags & VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR) == 0) {
        skip |= LogError("VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10294", cmdbuf, rc_info_av1_loc.dot(Field::flags),
                         "(%s) specifies a reference pattern but does not indicate the use of a regular GOP structure.",
                         string_VkVideoEncodeAV1RateControlFlagsKHR(rc_info_av1->flags).c_str());
    }

    if (rc_info_av1->flags & VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR &&
        rc_info_av1->flags & VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) {
        skip |= LogError("VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10295", cmdbuf, rc_info_av1_loc.dot(Field::flags),
                         "(%s) indicates conflicting reference patterns.",
                         string_VkVideoEncodeAV1RateControlFlagsKHR(rc_info_av1->flags).c_str());
    }

    if (rc_info_av1->flags & VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR && rc_info_av1->gopFrameCount == 0) {
        skip |= LogError("VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10296", cmdbuf, rc_info_av1_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR but the GOP size "
                         "specified in %s is zero.",
                         rc_info_av1_loc.dot(Field::gopFrameCount).Fields().c_str());
    }

    if (rc_info_av1->keyFramePeriod != 0 && rc_info_av1->keyFramePeriod < rc_info_av1->gopFrameCount) {
        skip |= LogError(
            "VUID-VkVideoEncodeAV1RateControlInfoKHR-keyFramePeriod-10297", cmdbuf, rc_info_av1_loc.dot(Field::keyFramePeriod),
            "(%u) is not zero and the specified key frame period smaller than the GOP size specified in %s (%u).",
            rc_info_av1->keyFramePeriod, rc_info_av1_loc.dot(Field::gopFrameCount).Fields().c_str(), rc_info_av1->gopFrameCount);
    }

    if (rc_info_av1->consecutiveBipredictiveFrameCount != 0 &&
        rc_info_av1->consecutiveBipredictiveFrameCount >= rc_info_av1->gopFrameCount) {
        skip |= LogError("VUID-VkVideoEncodeAV1RateControlInfoKHR-consecutiveBipredictiveFrameCount-10298", cmdbuf,
                         rc_info_av1_loc.dot(Field::consecutiveBipredictiveFrameCount),
                         "(%u) is greater than or equal to the GOP size specified in %s (%u).",
                         rc_info_av1->consecutiveBipredictiveFrameCount, rc_info_av1_loc.dot(Field::gopFrameCount).Fields().c_str(),
                         rc_info_av1->gopFrameCount);
    }

    if (rc_info_av1->temporalLayerCount > profile_caps.encode_av1.maxTemporalLayerCount) {
        skip |= LogError("VUID-VkVideoEncodeAV1RateControlInfoKHR-temporalLayerCount-10299", cmdbuf,
                         rc_info_av1_loc.dot(Field::temporalLayerCount),
                         "(%u) is greater than the VkVideoEncodeAV1Capabilities::maxTemporalLayerCount (%u) "
                         "supported by the AV1 encode profile the bound video session %s was created with.",
                         rc_info_av1->temporalLayerCount, profile_caps.encode_av1.maxTemporalLayerCount,
                         FormatHandle(vs_state).c_str());
    }

    if (rc_info.layerCount > 1 && rc_info.layerCount != rc_info_av1->temporalLayerCount) {
        skip |= LogError(
            "VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351", cmdbuf,
            rc_info_av1_loc.dot(Field::temporalLayerCount), "(%u) does not match %s (%u).", rc_info_av1->temporalLayerCount,
            loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR, Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfo(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                         const void *pNext, VkCommandBuffer cmdbuf,
                                                         const vvl::VideoSession &vs_state, const Location &rc_info_loc) const {
    bool skip = false;

    const auto &rc_layer_info = rc_info.pLayers[layer_index];
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const Location rc_layer_info_loc = rc_info_loc.dot(Field::pLayers, layer_index);

    if (rc_layer_info.averageBitrate < 1 || rc_layer_info.averageBitrate > profile_caps.encode.maxBitrate) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08276", objlist, rc_layer_info_loc.dot(Field::averageBitrate),
                     "(%" PRIu64 ") must be between 1 and VkVideoEncodeCapabilitiesKHR::maxBitrate (%" PRIu64
                     ") limit supported by the video profile %s was created with.",
                     rc_layer_info.averageBitrate, profile_caps.encode.maxBitrate, FormatHandle(vs_state).c_str());
    }

    if (rc_layer_info.maxBitrate < 1 || rc_layer_info.maxBitrate > profile_caps.encode.maxBitrate) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08277", objlist, rc_layer_info_loc.dot(Field::maxBitrate),
                         "(%" PRIu64 ") must be between 1 and VkVideoEncodeCapabilitiesKHR::maxBitrate (%" PRIu64
                         ") limit supported by the video profile %s was created with.",
                         rc_layer_info.maxBitrate, profile_caps.encode.maxBitrate, FormatHandle(vs_state).c_str());
    }

    if (rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR &&
        rc_layer_info.averageBitrate != rc_layer_info.maxBitrate) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08356", cmdbuf, rc_info_loc.dot(Field::rateControlMode),
                     "is VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR but maxBitrate (%" PRIu64
                     ") is not equal to averageBitrate (%" PRIu64 ") in %s.",
                     rc_layer_info.maxBitrate, rc_layer_info.averageBitrate, rc_layer_info_loc.Fields().c_str());
    }

    if (rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR &&
        rc_layer_info.averageBitrate > rc_layer_info.maxBitrate) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278", cmdbuf, rc_info_loc.dot(Field::rateControlMode),
                     "is VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR but averageBitrate (%" PRIu64
                     ") is greater than maxBitrate (%" PRIu64 ") in %s.",
                     rc_layer_info.averageBitrate, rc_layer_info.maxBitrate, rc_layer_info_loc.Fields().c_str());
    }

    if (rc_layer_info.frameRateNumerator == 0) {
        skip |= LogError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateNumerator-08350", cmdbuf,
                         rc_layer_info_loc.dot(Field::frameRateNumerator), "is zero.");
    }

    if (rc_layer_info.frameRateDenominator == 0) {
        skip |= LogError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateDenominator-08351", cmdbuf,
                         rc_layer_info_loc.dot(Field::frameRateDenominator), "is zero.");
    }

    switch (vs_state.GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            skip |= ValidateVideoEncodeRateControlLayerInfoH264(layer_index, rc_info, pNext, cmdbuf, vs_state, rc_layer_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            skip |= ValidateVideoEncodeRateControlLayerInfoH265(layer_index, rc_info, pNext, cmdbuf, vs_state, rc_layer_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            skip |= ValidateVideoEncodeRateControlLayerInfoAV1(layer_index, rc_info, pNext, cmdbuf, vs_state, rc_layer_info_loc);
            break;

        default:
            assert(false);
            break;
    }

    return skip;
}

template <typename RateControlLayerInfo>
bool CoreChecks::ValidateVideoEncodeRateControlH26xQp(VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                      const RateControlLayerInfo &rc_layer_info, const char *min_qp_range_vuid,
                                                      const char *max_qp_range_vuid, int32_t min_qp, int32_t max_qp,
                                                      const char *min_qp_per_pic_type_vuid, const char *max_qp_per_pic_type_vuid,
                                                      bool qp_per_picture_type, const char *min_max_qp_compare_vuid,
                                                      const Location &loc) const {
    bool skip = false;

    auto qp_range_error = [&](const char *vuid, const Location &field_loc, int32_t value) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        return LogError(vuid, objlist, field_loc,
                        "(%d) is outside of the range [%d, %d] supported by the video profile %s was created with.", value, min_qp,
                        max_qp, FormatHandle(vs_state).c_str());
    };

    auto qp_per_pic_type_error = [&](const char *vuid, const Location &struct_loc, int32_t qp_i, int32_t qp_p, int32_t qp_b) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        return LogError(vuid, objlist, struct_loc,
                        "contains non-matching QP values (qpI = %d, qpP = %d, qpB = %d) but different QP values per "
                        "picture type are not supported by the video profile %s was created with.",
                        qp_i, qp_p, qp_b, FormatHandle(vs_state).c_str());
    };

    auto min_max_qp_compare_error = [&](const char *which, int32_t min_value, int32_t max_value) {
        return LogError(min_max_qp_compare_vuid, cmdbuf, loc, "minQp.%s (%d) is greater than maxQp.%s (%d).", which, min_value,
                        which, max_value);
    };

    if (rc_layer_info.useMinQp) {
        if (rc_layer_info.minQp.qpI < min_qp || rc_layer_info.minQp.qpI > max_qp) {
            skip |= qp_range_error(min_qp_range_vuid, loc.dot(Field::minQp).dot(Field::qpI), rc_layer_info.minQp.qpI);
        }

        if (rc_layer_info.minQp.qpP < min_qp || rc_layer_info.minQp.qpP > max_qp) {
            skip |= qp_range_error(min_qp_range_vuid, loc.dot(Field::minQp).dot(Field::qpP), rc_layer_info.minQp.qpP);
        }

        if (rc_layer_info.minQp.qpB < min_qp || rc_layer_info.minQp.qpB > max_qp) {
            skip |= qp_range_error(min_qp_range_vuid, loc.dot(Field::minQp).dot(Field::qpB), rc_layer_info.minQp.qpB);
        }

        if ((rc_layer_info.minQp.qpI != rc_layer_info.minQp.qpP || rc_layer_info.minQp.qpI != rc_layer_info.minQp.qpB) &&
            !qp_per_picture_type) {
            skip |= qp_per_pic_type_error(min_qp_per_pic_type_vuid, loc.dot(Field::minQp), rc_layer_info.minQp.qpI,
                                          rc_layer_info.minQp.qpP, rc_layer_info.minQp.qpB);
        }
    }

    if (rc_layer_info.useMaxQp) {
        if (rc_layer_info.maxQp.qpI < min_qp || rc_layer_info.maxQp.qpI > max_qp) {
            skip |= qp_range_error(max_qp_range_vuid, loc.dot(Field::maxQp).dot(Field::qpI), rc_layer_info.maxQp.qpI);
        }

        if (rc_layer_info.maxQp.qpP < min_qp || rc_layer_info.maxQp.qpP > max_qp) {
            skip |= qp_range_error(max_qp_range_vuid, loc.dot(Field::maxQp).dot(Field::qpP), rc_layer_info.maxQp.qpP);
        }

        if (rc_layer_info.maxQp.qpB < min_qp || rc_layer_info.maxQp.qpB > max_qp) {
            skip |= qp_range_error(max_qp_range_vuid, loc.dot(Field::maxQp).dot(Field::qpB), rc_layer_info.maxQp.qpB);
        }

        if ((rc_layer_info.maxQp.qpI != rc_layer_info.maxQp.qpP || rc_layer_info.maxQp.qpI != rc_layer_info.maxQp.qpB) &&
            !qp_per_picture_type) {
            skip |= qp_per_pic_type_error(max_qp_per_pic_type_vuid, loc.dot(Field::maxQp), rc_layer_info.maxQp.qpI,
                                          rc_layer_info.maxQp.qpP, rc_layer_info.maxQp.qpB);
        }
    }

    if (rc_layer_info.useMinQp && rc_layer_info.useMaxQp) {
        if (rc_layer_info.minQp.qpI > rc_layer_info.maxQp.qpI) {
            skip |= min_max_qp_compare_error(String(Field::qpI), rc_layer_info.minQp.qpI, rc_layer_info.maxQp.qpI);
        }

        if (rc_layer_info.minQp.qpP > rc_layer_info.maxQp.qpP) {
            skip |= min_max_qp_compare_error(String(Field::qpP), rc_layer_info.minQp.qpP, rc_layer_info.maxQp.qpP);
        }

        if (rc_layer_info.minQp.qpB > rc_layer_info.maxQp.qpB) {
            skip |= min_max_qp_compare_error(String(Field::qpB), rc_layer_info.minQp.qpB, rc_layer_info.maxQp.qpB);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlAV1QIndex(VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                         const VkVideoEncodeAV1RateControlLayerInfoKHR &rc_layer_info,
                                                         const char *min_q_index_range_vuid, const char *max_q_index_range_vuid,
                                                         uint32_t min_q_index, uint32_t max_q_index,
                                                         const char *min_q_index_per_rc_group_vuid,
                                                         const char *max_q_index_per_rc_group_vuid, bool q_index_per_rc_group,
                                                         const char *min_max_q_index_compare_vuid, const Location &loc) const {
    bool skip = false;

    auto q_index_range_error = [&](const char *vuid, const Location &field_loc, uint32_t value) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        return LogError(vuid, objlist, field_loc,
                        "(%u) is outside of the range [%u, %u] supported by the video profile %s was created with.", value,
                        min_q_index, max_q_index, FormatHandle(vs_state).c_str());
    };

    auto q_index_per_rc_group_error = [&](const char *vuid, const Location &struct_loc, uint32_t qi_i, uint32_t qi_p,
                                          uint32_t qi_b) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        return LogError(vuid, objlist, struct_loc,
                        "contains non-matching quantizer index values (intraQIndex = %u, predictiveQIndex = %u, "
                        "bipredictiveQIndex = %u) but different quantizer index values per rate control "
                        "group are not supported by the video profile %s was created with.",
                        qi_i, qi_p, qi_b, FormatHandle(vs_state).c_str());
    };

    auto min_max_q_index_compare_error = [&](const char *which, uint32_t min_value, uint32_t max_value) {
        return LogError(min_max_q_index_compare_vuid, cmdbuf, loc, "minQIndex.%s (%u) is greater than maxQIndex.%s (%u).", which,
                        min_value, which, max_value);
    };

    if (rc_layer_info.useMinQIndex) {
        if (rc_layer_info.minQIndex.intraQIndex < min_q_index || rc_layer_info.minQIndex.intraQIndex > max_q_index) {
            skip |= q_index_range_error(min_q_index_range_vuid, loc.dot(Field::minQIndex).dot(Field::intraQIndex),
                                        rc_layer_info.minQIndex.intraQIndex);
        }

        if (rc_layer_info.minQIndex.predictiveQIndex < min_q_index || rc_layer_info.minQIndex.predictiveQIndex > max_q_index) {
            skip |= q_index_range_error(min_q_index_range_vuid, loc.dot(Field::minQIndex).dot(Field::qpP),
                                        rc_layer_info.minQIndex.predictiveQIndex);
        }

        if (rc_layer_info.minQIndex.bipredictiveQIndex < min_q_index || rc_layer_info.minQIndex.bipredictiveQIndex > max_q_index) {
            skip |= q_index_range_error(min_q_index_range_vuid, loc.dot(Field::minQIndex).dot(Field::bipredictiveQIndex),
                                        rc_layer_info.minQIndex.bipredictiveQIndex);
        }

        if ((rc_layer_info.minQIndex.intraQIndex != rc_layer_info.minQIndex.predictiveQIndex ||
             rc_layer_info.minQIndex.intraQIndex != rc_layer_info.minQIndex.bipredictiveQIndex) &&
            !q_index_per_rc_group) {
            skip |= q_index_per_rc_group_error(min_q_index_per_rc_group_vuid, loc.dot(Field::minQIndex),
                                               rc_layer_info.minQIndex.intraQIndex, rc_layer_info.minQIndex.predictiveQIndex,
                                               rc_layer_info.minQIndex.bipredictiveQIndex);
        }
    }

    if (rc_layer_info.useMaxQIndex) {
        if (rc_layer_info.maxQIndex.intraQIndex < min_q_index || rc_layer_info.maxQIndex.intraQIndex > max_q_index) {
            skip |= q_index_range_error(max_q_index_range_vuid, loc.dot(Field::maxQIndex).dot(Field::intraQIndex),
                                        rc_layer_info.maxQIndex.intraQIndex);
        }

        if (rc_layer_info.maxQIndex.predictiveQIndex < min_q_index || rc_layer_info.maxQIndex.predictiveQIndex > max_q_index) {
            skip |= q_index_range_error(max_q_index_range_vuid, loc.dot(Field::maxQIndex).dot(Field::predictiveQIndex),
                                        rc_layer_info.maxQIndex.predictiveQIndex);
        }

        if (rc_layer_info.maxQIndex.bipredictiveQIndex < min_q_index || rc_layer_info.maxQIndex.bipredictiveQIndex > max_q_index) {
            skip |= q_index_range_error(max_q_index_range_vuid, loc.dot(Field::maxQIndex).dot(Field::bipredictiveQIndex),
                                        rc_layer_info.maxQIndex.bipredictiveQIndex);
        }

        if ((rc_layer_info.maxQIndex.intraQIndex != rc_layer_info.maxQIndex.predictiveQIndex ||
             rc_layer_info.maxQIndex.intraQIndex != rc_layer_info.maxQIndex.bipredictiveQIndex) &&
            !q_index_per_rc_group) {
            skip |= q_index_per_rc_group_error(max_q_index_per_rc_group_vuid, loc.dot(Field::maxQIndex),
                                               rc_layer_info.maxQIndex.intraQIndex, rc_layer_info.maxQIndex.predictiveQIndex,
                                               rc_layer_info.maxQIndex.bipredictiveQIndex);
        }
    }

    if (rc_layer_info.useMinQIndex && rc_layer_info.useMaxQIndex) {
        if (rc_layer_info.minQIndex.intraQIndex > rc_layer_info.maxQIndex.intraQIndex) {
            skip |= min_max_q_index_compare_error(String(Field::intraQIndex), rc_layer_info.minQIndex.intraQIndex,
                                                  rc_layer_info.maxQIndex.intraQIndex);
        }

        if (rc_layer_info.minQIndex.predictiveQIndex > rc_layer_info.maxQIndex.predictiveQIndex) {
            skip |= min_max_q_index_compare_error(String(Field::predictiveQIndex), rc_layer_info.minQIndex.predictiveQIndex,
                                                  rc_layer_info.maxQIndex.predictiveQIndex);
        }

        if (rc_layer_info.minQIndex.bipredictiveQIndex > rc_layer_info.maxQIndex.bipredictiveQIndex) {
            skip |= min_max_q_index_compare_error(String(Field::bipredictiveQIndex), rc_layer_info.minQIndex.bipredictiveQIndex,
                                                  rc_layer_info.maxQIndex.bipredictiveQIndex);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfoH264(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                             const void *pNext, VkCommandBuffer cmdbuf,
                                                             const vvl::VideoSession &vs_state,
                                                             const Location &rc_layer_info_loc) const {
    bool skip = false;

    const auto rc_layer_info_h264 =
        vku::FindStructInPNextChain<VkVideoEncodeH264RateControlLayerInfoKHR>(rc_info.pLayers[layer_index].pNext);
    if (rc_layer_info_h264 == nullptr) return false;

    const Location rc_layer_info_h264_loc = rc_layer_info_loc.pNext(Struct::VkVideoEncodeH264RateControlLayerInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    skip |= ValidateVideoEncodeRateControlH26xQp(
        cmdbuf, vs_state, *rc_layer_info_h264, "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287", profile_caps.encode_h264.minQp,
        profile_caps.encode_h264.maxQp, "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289",
        profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR,
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08374", rc_layer_info_h264_loc);

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfoH265(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                             const void *pNext, VkCommandBuffer cmdbuf,
                                                             const vvl::VideoSession &vs_state,
                                                             const Location &rc_layer_info_loc) const {
    bool skip = false;

    const auto rc_layer_info_h265 =
        vku::FindStructInPNextChain<VkVideoEncodeH265RateControlLayerInfoKHR>(rc_info.pLayers[layer_index].pNext);
    if (rc_layer_info_h265 == nullptr) return false;

    const Location rc_layer_info_h265_loc = rc_layer_info_loc.pNext(Struct::VkVideoEncodeH265RateControlLayerInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    skip |= ValidateVideoEncodeRateControlH26xQp(
        cmdbuf, vs_state, *rc_layer_info_h265, "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298", profile_caps.encode_h265.minQp,
        profile_caps.encode_h265.maxQp, "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300",
        profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR,
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08375", rc_layer_info_h265_loc);

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfoAV1(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                            const void *pNext, VkCommandBuffer cmdbuf,
                                                            const vvl::VideoSession &vs_state,
                                                            const Location &rc_layer_info_loc) const {
    bool skip = false;

    const auto rc_layer_info_av1 =
        vku::FindStructInPNextChain<VkVideoEncodeAV1RateControlLayerInfoKHR>(rc_info.pLayers[layer_index].pNext);
    if (rc_layer_info_av1 == nullptr) return false;

    const Location rc_layer_info_av1_loc = rc_layer_info_loc.pNext(Struct::VkVideoEncodeAV1RateControlLayerInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    skip |= ValidateVideoEncodeRateControlAV1QIndex(
        cmdbuf, vs_state, *rc_layer_info_av1, "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10300",
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302", profile_caps.encode_av1.minQIndex,
        profile_caps.encode_av1.maxQIndex, "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301",
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303",
        profile_caps.encode_av1.flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR,
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10304", rc_layer_info_av1_loc);

    return skip;
}

bool CoreChecks::ValidateVideoPictureResource(const vvl::VideoPictureResource &picture_resource, VkCommandBuffer cmdbuf,
                                              const vvl::VideoSession &vs_state, const Location &loc, const char *coded_offset_vuid,
                                              const char *coded_extent_vuid) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (coded_offset_vuid) {
        VkOffset2D offset_granularity{0, 0};
        if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR &&
            vs_state.GetH264PictureLayout() == VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR) {
            offset_granularity = profile_caps.decode_h264.fieldOffsetGranularity;
        }

        if (!IsIntegerMultipleOf(picture_resource.coded_offset, offset_granularity)) {
            const LogObjectList objlist(cmdbuf, vs_state.Handle());
            skip |=
                LogError(coded_offset_vuid, objlist, loc.dot(Field::codedExtent),
                         "(%s) is not an integer multiple of the codedOffsetGranularity (%s).",
                         string_VkOffset2D(picture_resource.coded_offset).c_str(), string_VkOffset2D(offset_granularity).c_str());
        }
    }

    if (coded_extent_vuid &&
        !IsBetweenInclusive(picture_resource.coded_extent, profile_caps.base.minCodedExtent, vs_state.create_info.maxCodedExtent)) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle());
        skip |= LogError(
            coded_extent_vuid, objlist, loc.dot(Field::codedExtent), "(%s) is outside of the range (%s)-(%s) supported by %s.",
            string_VkExtent2D(picture_resource.coded_extent).c_str(), string_VkExtent2D(profile_caps.base.minCodedExtent).c_str(),
            string_VkExtent2D(vs_state.create_info.maxCodedExtent).c_str(), FormatHandle(vs_state).c_str());
    }

    if (picture_resource.base_array_layer >= picture_resource.image_view_state->create_info.subresourceRange.layerCount) {
        const LogObjectList objlist(cmdbuf, vs_state.Handle(), picture_resource.image_view_state->Handle(),
                                    picture_resource.image_state->Handle());
        skip |=
            LogError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175", objlist, loc.dot(Field::baseArrayLayer),
                     "(%u) is greater than or equal to the layerCount (%u) "
                     "the %s specified in imageViewBinding was created with.",
                     picture_resource.base_array_layer, picture_resource.image_view_state->create_info.subresourceRange.layerCount,
                     FormatHandle(picture_resource.image_view_state->Handle()).c_str());
    }

    return skip;
}

template <typename StateObject>
bool core::ValidateVideoProfileInfo(const StateObject &state, const VkVideoProfileInfoKHR *profile, const ErrorObject &error_obj,
                                    const Location &loc) {
    using Field = vvl::Field;

    bool skip = false;

    const char *profile_pnext_msg = "chain does not contain a %s structure.";

    if (GetBitSetCount(profile->chromaSubsampling) != 1) {
        skip |= state.LogError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07013", error_obj.objlist,
                               loc.dot(Field::chromaSubsampling), "must have a single bit set.");
    }

    if (GetBitSetCount(profile->lumaBitDepth) != 1) {
        skip |= state.LogError("VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014", error_obj.objlist, loc.dot(Field::lumaBitDepth),
                               "must have a single bit set.");
    }

    if (profile->chromaSubsampling != VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR) {
        if (GetBitSetCount(profile->chromaBitDepth) != 1) {
            skip |= state.LogError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015", error_obj.objlist,
                                   loc.dot(Field::chromaBitDepth), "must have a single bit set.");
        }
    }

    switch (profile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            const auto decode_h264 = vku::FindStructInPNextChain<VkVideoDecodeH264ProfileInfoKHR>(profile->pNext);
            if (decode_h264 == nullptr) {
                skip |= state.LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07179", error_obj.objlist,
                                       loc.dot(Field::pNext), profile_pnext_msg, "VkVideoDecodeH264ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            const auto decode_h265 = vku::FindStructInPNextChain<VkVideoDecodeH265ProfileInfoKHR>(profile->pNext);
            if (decode_h265 == nullptr) {
                skip |= state.LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180", error_obj.objlist,
                                       loc.dot(Field::pNext), profile_pnext_msg, "VkVideoDecodeH265ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            const auto decode_av1 = vku::FindStructInPNextChain<VkVideoDecodeAV1ProfileInfoKHR>(profile->pNext);
            if (decode_av1 == nullptr) {
                skip |= state.LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-09256", error_obj.objlist,
                                       loc.dot(Field::pNext), profile_pnext_msg, "VkVideoDecodeAV1ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            const auto encode_h264 = vku::FindStructInPNextChain<VkVideoEncodeH264ProfileInfoKHR>(profile->pNext);
            if (encode_h264 == nullptr) {
                skip |= state.LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181", error_obj.objlist,
                                       loc.dot(Field::pNext), profile_pnext_msg, "VkVideoEncodeH264ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            const auto encode_h265 = vku::FindStructInPNextChain<VkVideoEncodeH265ProfileInfoKHR>(profile->pNext);
            if (encode_h265 == nullptr) {
                skip |= state.LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182", error_obj.objlist,
                                       loc.dot(Field::pNext), profile_pnext_msg, "VkVideoEncodeH265ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            if constexpr (std::is_same_v<StateObject, CoreChecks>) {
                using Func = vvl::Func;
                const char *codec_feature_not_enabled_msg = "is %s but the %s device feature is not enabled.";
                if (!state.enabled_features.videoEncodeAV1) {
                    const char *vuid = kVUIDUndefined;
                    switch (loc.function) {
                        case Func::vkCreateVideoSessionKHR:
                            vuid = "VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10269";
                            break;
                        case Func::vkCreateImage:
                            vuid = "VUID-VkImageCreateInfo-pNext-10250";
                            break;
                        case Func::vkCreateBuffer:
                            vuid = "VUID-VkBufferCreateInfo-pNext-10249";
                            break;
                        case Func::vkCreateQueryPool:
                            vuid = "VUID-VkQueryPoolCreateInfo-pNext-10248";
                            break;
                        default:
                            // Unhandled call site
                            assert(false);
                            break;
                    }
                    skip |=
                        state.LogError(vuid, error_obj.objlist, loc.dot(Field::videoCodecOperation), codec_feature_not_enabled_msg,
                                       string_VkVideoCodecOperationFlagBitsKHR(profile->videoCodecOperation), "videoEncodeAV1");
                }
            }

            const auto encode_av1 = vku::FindStructInPNextChain<VkVideoEncodeAV1ProfileInfoKHR>(profile->pNext);
            if (encode_av1 == nullptr) {
                skip |= state.LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-10262", error_obj.objlist,
                                       loc.dot(Field::pNext), profile_pnext_msg, "VkVideoEncodeAV1ProfileInfoKHR");
            }
            break;
        }

        default:
            assert(false);
            skip = true;
            break;
    }

    return skip;
}
template bool core::ValidateVideoProfileInfo<core::Instance>(const core::Instance &state, const VkVideoProfileInfoKHR *profile,
                                                             const ErrorObject &error_obj, const Location &loc);
template bool core::ValidateVideoProfileInfo<CoreChecks>(const CoreChecks &state, const VkVideoProfileInfoKHR *profile,
                                                         const ErrorObject &error_obj, const Location &loc);

template <typename StateObject>
bool core::ValidateVideoProfileListInfo(const StateObject &state, const VkVideoProfileListInfoKHR *profile_list,
                                        const ErrorObject &error_obj, const Location &loc, bool expect_decode_profile,
                                        const char *missing_decode_profile_msg_code, bool expect_encode_profile,
                                        const char *missing_encode_profile_msg_code) {
    bool skip = false;

    bool has_decode_profile = false;
    bool has_encode_profile = false;

    if (profile_list) {
        for (uint32_t i = 0; i < profile_list->profileCount; ++i) {
            skip |= ValidateVideoProfileInfo(state, &profile_list->pProfiles[i], error_obj, loc.dot(vvl::Field::pProfiles, i));

            switch (profile_list->pProfiles[i].videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
                    if (has_decode_profile) {
                        skip |= state.LogError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813", error_obj.objlist, loc,
                                               "contains more than one profile with decode codec operation.");
                    }
                    has_decode_profile = true;
                    break;

                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
                    has_encode_profile = true;
                    break;

                default:
                    assert(false);
                    skip = true;
                    break;
            }
        }
    }

    if (expect_decode_profile && !has_decode_profile) {
        skip |= state.LogError(missing_decode_profile_msg_code, error_obj.objlist, loc.dot(vvl::Field::pProfiles),
                               "contains no video profile specifying a video decode operation.");
    }

    if (expect_encode_profile && !has_encode_profile) {
        skip |= state.LogError(missing_encode_profile_msg_code, error_obj.objlist, loc.dot(vvl::Field::pProfiles),
                               "contains no video profile specifying a video encode operation.");
    }

    return skip;
}
template bool core::ValidateVideoProfileListInfo<core::Instance>(
    const core::Instance &state, const VkVideoProfileListInfoKHR *profile_list, const ErrorObject &error_obj, const Location &loc,
    bool expect_decode_profile, const char *missing_decode_profile_msg_code, bool expect_encode_profile,
    const char *missing_encode_profile_msg_code);
template bool core::ValidateVideoProfileListInfo<CoreChecks>(const CoreChecks &state, const VkVideoProfileListInfoKHR *profile_list,
                                                             const ErrorObject &error_obj, const Location &loc,
                                                             bool expect_decode_profile,
                                                             const char *missing_decode_profile_msg_code,
                                                             bool expect_encode_profile,
                                                             const char *missing_encode_profile_msg_code);

bool CoreChecks::ValidateDecodeH264ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoDecodeH264SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoDecodeH264SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204", device, loc.function,
                             "number of H.264 SPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH264SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205", device, loc.function,
                             "number of H.264 PPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH264SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDecodeH265ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoDecodeH265SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoDecodeH265SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify VPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265VPSKey(add_info->pStdVPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833", device, loc.dot(Field::pStdVPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify VPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.vps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdVPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207", device, loc.function,
                             "number of H.265 VPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdVPSCount (%u).",
                             keys.size(), create_info->maxStdVPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208", device, loc.function,
                             "number of H.265 SPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209", device, loc.function,
                             "number of H.265 PPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateEncodeH264ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoEncodeH264SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoEncodeH264SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839", device, loc.function,
                             "number of H.264 SPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH264SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840", device, loc.function,
                             "number of H.264 PPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH264SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateEncodeH265ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoEncodeH265SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoEncodeH265SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify VPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265VPSKey(add_info->pStdVPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438", device, loc.dot(Field::pStdVPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify VPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.vps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdVPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841", device, loc.function,
                             "number of H.265 VPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH265SessionParametersCreateInfoKHR::maxStdVPSCount (%u).",
                             keys.size(), create_info->maxStdVPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842", device, loc.function,
                             "number of H.265 SPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH265SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843", device, loc.function,
                             "number of H.265 PPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH265SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             keys.size(), create_info->maxStdPPSCount);
        }
    }

    if (add_info) {
        const auto &profile_caps = vs_state.profile->GetCapabilities();

        // Verify PPS contents
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            if (add_info->pStdPPSs[i].num_tile_columns_minus1 >= profile_caps.encode_h265.maxTiles.width) {
                const char *vuid = nullptr;
                if (create_info) {
                    assert(loc.function == Func::vkCreateVideoSessionParametersKHR);
                    vuid = "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319";
                } else {
                    assert(loc.function == Func::vkUpdateVideoSessionParametersKHR);
                    vuid = "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321";
                }
                skip |= LogError(vuid, device, loc.function,
                                 "%s.num_tile_columns_minus1 (%u) exceeds the maxTiles.width (%u) "
                                 "supported by the H.265 encode profile %s was created with.",
                                 loc.dot(Field::pStdPPSs, i).Fields().c_str(), add_info->pStdPPSs[i].num_tile_columns_minus1,
                                 profile_caps.encode_h265.maxTiles.width, FormatHandle(vs_state).c_str());
            }
            if (add_info->pStdPPSs[i].num_tile_rows_minus1 >= profile_caps.encode_h265.maxTiles.height) {
                const char *vuid = nullptr;
                if (create_info) {
                    assert(loc.function == Func::vkCreateVideoSessionParametersKHR);
                    vuid = "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320";
                } else {
                    assert(loc.function == Func::vkUpdateVideoSessionParametersKHR);
                    vuid = "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322";
                }
                skip |= LogError(vuid, device, loc.function,
                                 "%s.num_tile_rows_minus1 (%u) exceeds the maxTiles.height (%u) "
                                 "supported by the H.265 encode profile %s was created with.",
                                 loc.dot(Field::pStdPPSs, i).Fields().c_str(), add_info->pStdPPSs[i].num_tile_rows_minus1,
                                 profile_caps.encode_h265.maxTiles.height, FormatHandle(vs_state).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateEncodeQuantizationMapParametersCreateInfo(
    const vvl::VideoSession &vs_state, const VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR &quantization_map_info,
    VkDevice device, const Location &loc, const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    const char *quant_map_type_name = nullptr;
    const char *texel_size_vuid = nullptr;
    const vvl::SupportedQuantizationMapTexelSizes *supported_texel_sizes = nullptr;

    if (vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) {
        quant_map_type_name = "quantization delta map";
        texel_size_vuid = "VUID-VkVideoSessionParametersCreateInfoKHR-flags-10273";
        supported_texel_sizes = &vs_state.profile->GetSupportedQuantDeltaMapTexelSizes();
    } else if (vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR) {
        quant_map_type_name = "emphasis map";
        texel_size_vuid = "VUID-VkVideoSessionParametersCreateInfoKHR-flags-10274";
        supported_texel_sizes = &vs_state.profile->GetSupportedEmphasisMapTexelSizes();
    }

    VkExtent2D texel_size = quantization_map_info.quantizationMapTexelSize;

    if (supported_texel_sizes != nullptr && supported_texel_sizes->find(texel_size) == supported_texel_sizes->end()) {
        const LogObjectList objlist(device, vs_state.Handle());
        skip |= LogError(texel_size_vuid, objlist, loc.dot(Field::quantizationMapTexelSize),
                         "(%s) does not match any of the %s texel sizes supported for the video profile "
                         "pCreateInfo->videoSession was created with.",
                         string_VkExtent2D(texel_size).c_str(), quant_map_type_name);
    }

    if (template_state != nullptr &&
        template_state->create_info.flags & VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR &&
        (template_state->GetEncodeQuantizationMapTexelSize().width != texel_size.width ||
         template_state->GetEncodeQuantizationMapTexelSize().height != texel_size.height)) {
        const LogObjectList objlist(device, template_state->Handle());
        skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10276", objlist,
                         loc.dot(Field::quantizationMapTexelSize),
                         "(%s) does not match the quantization map texel size (%s) "
                         "pCreateInfo->videoSessionParametersTemplate was created with.",
                         string_VkExtent2D(texel_size).c_str(),
                         string_VkExtent2D(template_state->GetEncodeQuantizationMapTexelSize()).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateDecodeDistinctOutput(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                              const Location &loc) const {
    bool skip = false;
    auto cmd_loc = Location(loc.function);

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if ((profile_caps.decode.flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR) == 0) {
        const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
        switch (vs_state.GetCodecOp()) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
                // In case of AV1 decode distinct output can be used for film grain enabled frames
                auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeAV1PictureInfoKHR>(decode_info.pNext);
                bool film_grain_enabled =
                    (picture_info && picture_info->pStdPictureInfo && picture_info->pStdPictureInfo->flags.apply_grain);
                if (!vs_state.profile->HasAV1FilmGrainSupport()) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141", objlist, cmd_loc,
                                     "the AV1 decode profile %s was created with does not support "
                                     "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR and does not have "
                                     "VkVideoDecodeAV1ProfileInfoKHR::filmGrainSupport set to VK_TRUE but "
                                     "pDecodeInfo->dstPictureResource and pSetupReferenceSlot->pPictureResource "
                                     "do not match.",
                                     FormatHandle(vs_state).c_str());
                } else if (!film_grain_enabled) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141", objlist, cmd_loc,
                                     "the AV1 decode profile %s was created with does not support "
                                     "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR and "
                                     "film grain is not enabled for the decoded picture but "
                                     "pDecodeInfo->dstPictureResource and pSetupReferenceSlot->pPictureResource "
                                     "do not match.",
                                     FormatHandle(vs_state).c_str());
                }
                break;
            }

            default: {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141", objlist, cmd_loc,
                                 "the video profile %s was created with does not support "
                                 "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR but "
                                 "pDecodeInfo->dstPictureResource and pSetupReferenceSlot->pPictureResource "
                                 "do not match.",
                                 FormatHandle(vs_state).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoH264(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;

    const bool inline_session_params_enabled =
        vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;
    const auto inline_session_params =
        vku::FindStructInPNextChain<VkVideoDecodeH264InlineSessionParametersInfoKHR>(decode_info.pNext);

    const bool has_inline_sps =
        inline_session_params_enabled && inline_session_params != nullptr && inline_session_params->pStdSPS != nullptr;
    const bool has_inline_pps =
        inline_session_params_enabled && inline_session_params != nullptr && inline_session_params->pStdPPS != nullptr;
    const bool needs_bound_session_params = !has_inline_sps || !has_inline_pps;
    const bool has_bound_session_params = cb_state.bound_video_session_parameters != nullptr;

    if (needs_bound_session_params && !has_bound_session_params) {
        if (inline_session_params_enabled) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-10400", cb_state.Handle(), loc.function,
                             "not all H.264 decode parameter sets are specified inline through %s "
                             "but there is no bound video session parameters object.",
                             loc.pNext(Struct::VkVideoDecodeH264InlineSessionParametersInfoKHR).Fields().c_str());
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-10400", cb_state.Handle(), loc.function,
                             "%s was created without VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR "
                             "but there is no bound video session parameters object.",
                             FormatHandle(vs_state).c_str());
        }
    }

    bool interlaced_frame_support =
        (vs_state.profile->GetH264PictureLayout() != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR);

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeH264PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        auto std_picture_info_loc = loc.pNext(Struct::VkVideoDecodeH264PictureInfoKHR, Field::pStdPictureInfo);

        if (!interlaced_frame_support && std_picture_info->flags.field_pic_flag) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-07258", objlist, loc.function,
                             "decode output picture is a field but the bound video session "
                             "%s was not created with interlaced frame support.",
                             FormatHandle(vs_state).c_str());
        }

        for (uint32_t i = 0; i < picture_info->sliceCount; ++i) {
            if (picture_info->pSliceOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pSliceOffsets-07153", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeH264PictureInfoKHR, Field::pSliceOffsets, i),
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                                 picture_info->pSliceOffsets[i], decode_info.srcBufferRange);
            }
        }

        if (has_inline_sps) {
            if (inline_session_params->pStdSPS->seq_parameter_set_id != std_picture_info->seq_parameter_set_id) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-10401", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeH264InlineSessionParametersInfoKHR, Field::pStdSPS),
                                 "seq_parameter_set_id (%u) does not match the seq_parameter_set_id (%u) specified in %s.",
                                 inline_session_params->pStdSPS->seq_parameter_set_id, std_picture_info->seq_parameter_set_id,
                                 std_picture_info_loc.Fields().c_str());
            }
        }

        if (has_inline_pps) {
            if (inline_session_params->pStdPPS->seq_parameter_set_id != std_picture_info->seq_parameter_set_id ||
                inline_session_params->pStdPPS->pic_parameter_set_id != std_picture_info->pic_parameter_set_id) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-10402", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeH264InlineSessionParametersInfoKHR, Field::pStdPPS),
                                 "seq_parameter_set_id (%u) and pic_parameter_set_id (%u) does not match the "
                                 "seq_parameter_set_id (%u) and pic_parameter_set_id (%u) specified in %s.",
                                 inline_session_params->pStdPPS->seq_parameter_set_id,
                                 inline_session_params->pStdPPS->pic_parameter_set_id, std_picture_info->seq_parameter_set_id,
                                 std_picture_info->pic_parameter_set_id, std_picture_info_loc.Fields().c_str());
                ;
            }
        }

        if (needs_bound_session_params && has_bound_session_params) {
            const auto &vsp_state = *cb_state.bound_video_session_parameters;
            auto session_params = vsp_state.Lock();
            if (!has_inline_sps && session_params.GetH264SPS(std_picture_info->seq_parameter_set_id) == nullptr) {
                const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
                const char *additional_info =
                    inline_session_params_enabled
                        ? "nor is inline SPS provided by including a VkVideoDecodeH264InlineSessionParametersInfoKHR "
                          "structure in the pNext chain of pDecodeInfo with a non-null pStdSPS"
                        : "and the bound video session was not created with "
                          "VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR";
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-StdVideoH264SequenceParameterSet-07154", objlist, loc.function,
                                 "no H.264 SPS with seq_parameter_set_id = %u "
                                 "exists in the bound video session parameters object %s %s.",
                                 std_picture_info->seq_parameter_set_id, FormatHandle(vsp_state).c_str(), additional_info);
            }

            if (!has_inline_pps && session_params.GetH264PPS(std_picture_info->seq_parameter_set_id,
                                                             std_picture_info->pic_parameter_set_id) == nullptr) {
                const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
                const char *additional_info =
                    inline_session_params_enabled
                        ? "nor is inline PPS provided by including a VkVideoDecodeH264InlineSessionParametersInfoKHR "
                          "structure in the pNext chain of pDecodeInfo with a non-null pStdPPS"
                        : "and the bound video session was not created with "
                          "VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR";
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155", objlist, loc.function,
                                 "no H.264 PPS with seq_parameter_set_id = %u "
                                 "and pic_parameter_set_id = %u exists in the bound video session parameters object %s %s.",
                                 std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id,
                                 FormatHandle(vsp_state).c_str(), additional_info);
            }
        }
    } else {
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07152", cb_state.Handle(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoDecodeH264PictureInfoKHR");
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (dpb_slot_info) {
            vvl::VideoPictureID picture_id(*vs_state.profile, *decode_info.pSetupReferenceSlot);
            if (!interlaced_frame_support && !picture_id.IsFrame()) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259", objlist, loc.function,
                                 "reconstructed picture is a field but the bound "
                                 "video session %s was not created with interlaced frame support.",
                                 FormatHandle(vs_state).c_str());
            }

            if (picture_info) {
                bool dst_is_field = (picture_info->pStdPictureInfo->flags.field_pic_flag != 0);
                bool dst_is_bottom_field = (picture_info->pStdPictureInfo->flags.bottom_field_flag != 0);

                if (!dst_is_field && !picture_id.IsFrame()) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261", cb_state.Handle(), loc.function,
                                     "decode output picture is a frame but the "
                                     "reconstructed picture is not a frame.");
                }

                if (dst_is_field && !dst_is_bottom_field && !picture_id.IsTopField()) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262", cb_state.Handle(), loc.function,
                                     "decode output picture is a top field but the "
                                     "reconstructed picture is not a top field.");
                }

                if (dst_is_field && dst_is_bottom_field && !picture_id.IsBottomField()) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263", cb_state.Handle(), loc.function,
                                     "decode output picture is a bottom field but the "
                                     "reconstructed picture is not a bottom field.");
                }
            }
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07156", cb_state.Handle(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoDecodeH264DpbSlotInfoKHR");
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (dpb_slot_info) {
            vvl::VideoPictureID picture_id(*vs_state.profile, decode_info.pReferenceSlots[i]);
            if (!interlaced_frame_support && !picture_id.IsFrame()) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260", objlist, loc.function,
                                 "reference picture specified in "
                                 "pDecodeInfo->pReferenceSlots[%u] is a field but the bound "
                                 "video session %s was not created with interlaced frame support.",
                                 i, FormatHandle(vs_state).c_str());
            }
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07157", cb_state.Handle(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoDecodeH264DpbSlotInfoKHR");
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoH265(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;

    const bool inline_session_params_enabled =
        vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;
    const auto inline_session_params =
        vku::FindStructInPNextChain<VkVideoDecodeH265InlineSessionParametersInfoKHR>(decode_info.pNext);

    const bool has_inline_vps =
        inline_session_params_enabled && inline_session_params != nullptr && inline_session_params->pStdVPS != nullptr;
    const bool has_inline_sps =
        inline_session_params_enabled && inline_session_params != nullptr && inline_session_params->pStdSPS != nullptr;
    const bool has_inline_pps =
        inline_session_params_enabled && inline_session_params != nullptr && inline_session_params->pStdPPS != nullptr;
    const bool needs_bound_session_params = !has_inline_vps || !has_inline_sps || !has_inline_pps;
    const bool has_bound_session_params = cb_state.bound_video_session_parameters != nullptr;

    if (needs_bound_session_params && !has_bound_session_params) {
        if (inline_session_params_enabled) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-10403", cb_state.Handle(), loc.function,
                             "not all H.265 decode parameter sets are specified inline through %s "
                             "but there is no bound video session parameters object.",
                             loc.pNext(Struct::VkVideoDecodeH265InlineSessionParametersInfoKHR).Fields().c_str());
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-10403", cb_state.Handle(), loc.function,
                             "%s was created without VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR "
                             "but there is no bound video session parameters object.",
                             FormatHandle(vs_state).c_str());
        }
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeH265PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        auto std_picture_info_loc = loc.pNext(Struct::VkVideoDecodeH265PictureInfoKHR, Field::pStdPictureInfo);

        for (uint32_t i = 0; i < picture_info->sliceSegmentCount; ++i) {
            if (picture_info->pSliceSegmentOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pSliceSegmentOffsets-07159", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeH265PictureInfoKHR, Field::pSliceSegmentOffsets, i),
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                                 picture_info->pSliceSegmentOffsets[i], decode_info.srcBufferRange);
            }
        }

        if (has_inline_vps) {
            if (inline_session_params->pStdVPS->vps_video_parameter_set_id != std_picture_info->sps_video_parameter_set_id) {
                skip |=
                    LogError("VUID-vkCmdDecodeVideoKHR-pNext-10404", cb_state.Handle(),
                             loc.pNext(Struct::VkVideoDecodeH265InlineSessionParametersInfoKHR, Field::pStdVPS),
                             "vps_video_parameter_set_id (%u) does not match the sps_video_parameter_set_id (%u) specified in %s.",
                             inline_session_params->pStdVPS->vps_video_parameter_set_id,
                             std_picture_info->sps_video_parameter_set_id, std_picture_info_loc.Fields().c_str());
            }
        }

        if (has_inline_sps) {
            if (inline_session_params->pStdSPS->sps_video_parameter_set_id != std_picture_info->sps_video_parameter_set_id ||
                inline_session_params->pStdSPS->sps_seq_parameter_set_id != std_picture_info->pps_seq_parameter_set_id) {
                skip |=
                    LogError("VUID-vkCmdDecodeVideoKHR-pNext-10405", cb_state.Handle(),
                             loc.pNext(Struct::VkVideoDecodeH265InlineSessionParametersInfoKHR, Field::pStdSPS),
                             "sps_video_parameter_set_id (%u) and sps_seq_parameter_set_id (%u) does not match the "
                             "sps_video_parameter_set_id (%u) and pps_seq_parameter_set_id (%u) specified in %s.",
                             inline_session_params->pStdSPS->sps_video_parameter_set_id,
                             inline_session_params->pStdSPS->sps_seq_parameter_set_id, std_picture_info->sps_video_parameter_set_id,
                             std_picture_info->pps_seq_parameter_set_id, std_picture_info_loc.Fields().c_str());
            }
        }

        if (has_inline_pps) {
            if (inline_session_params->pStdPPS->sps_video_parameter_set_id != std_picture_info->sps_video_parameter_set_id ||
                inline_session_params->pStdPPS->pps_seq_parameter_set_id != std_picture_info->pps_seq_parameter_set_id ||
                inline_session_params->pStdPPS->pps_pic_parameter_set_id != std_picture_info->pps_pic_parameter_set_id) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-10406", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeH265InlineSessionParametersInfoKHR, Field::pStdPPS),
                                 "sps_video_parameter_set_id (%u), pps_seq_parameter_set_id (%u) and pps_pic_parameter_set_id (%u) "
                                 "does not match the "
                                 "sps_video_parameter_set_id (%u), pps_seq_parameter_set_id (%u) and pps_pic_parameter_set_id (%u) "
                                 "specified in %s.",
                                 inline_session_params->pStdPPS->sps_video_parameter_set_id,
                                 inline_session_params->pStdPPS->pps_seq_parameter_set_id,
                                 inline_session_params->pStdPPS->pps_pic_parameter_set_id,
                                 std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                 std_picture_info->pps_pic_parameter_set_id, std_picture_info_loc.Fields().c_str());
            }
        }

        if (needs_bound_session_params && has_bound_session_params) {
            const auto &vsp_state = *cb_state.bound_video_session_parameters;
            const auto session_params = vsp_state.Lock();

            if (!has_inline_vps && session_params.GetH265VPS(std_picture_info->sps_video_parameter_set_id) == nullptr) {
                const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
                const char *additional_info =
                    inline_session_params_enabled
                        ? "nor is inline VPS provided by including a VkVideoDecodeH265InlineSessionParametersInfoKHR "
                          "structure in the pNext chain of pDecodeInfo with a non-null pStdVPS"
                        : "and the bound video session was not created with "
                          "VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR";
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-StdVideoH265VideoParameterSet-07160", objlist, loc.function,
                                 "no H.265 VPS with sps_video_parameter_set_id = %u "
                                 "exists in the bound video session parameters object %s %s.",
                                 std_picture_info->sps_video_parameter_set_id, FormatHandle(vsp_state).c_str(), additional_info);
            }

            if (!has_inline_sps && session_params.GetH265SPS(std_picture_info->sps_video_parameter_set_id,
                                                             std_picture_info->pps_seq_parameter_set_id) == nullptr) {
                const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
                const char *additional_info =
                    inline_session_params_enabled
                        ? "nor is inline SPS provided by including a VkVideoDecodeH265InlineSessionParametersInfoKHR "
                          "structure in the pNext chain of pDecodeInfo with a non-null pStdSPS"
                        : "and the bound video session was not created with "
                          "VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR";
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161", objlist, loc.function,
                                 "no H.265 SPS with sps_video_parameter_set_id = %u "
                                 "and pps_seq_parameter_set_id = %u exists in the bound video session "
                                 "parameters object %s %s.",
                                 std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                 FormatHandle(vsp_state).c_str(), additional_info);
            }

            if (!has_inline_pps &&
                session_params.GetH265PPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                          std_picture_info->pps_pic_parameter_set_id) == nullptr) {
                const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
                const char *additional_info =
                    inline_session_params_enabled
                        ? "nor is inline PPS provided by including a VkVideoDecodeH265InlineSessionParametersInfoKHR "
                          "structure in the pNext chain of pDecodeInfo with a non-null pStdPPS"
                        : "and the bound video session was not created with "
                          "VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR";
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162", objlist, loc.function,
                                 "no H.265 SPS with sps_video_parameter_set_id = %u, "
                                 "pps_seq_parameter_set_id = %u, and pps_pic_parameter_set_id = %u exists in "
                                 "the bound video session parameters object %s %s.",
                                 std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                 std_picture_info->pps_pic_parameter_set_id, FormatHandle(vsp_state).c_str(), additional_info);
            }
        }
    } else {
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07158", cb_state.Handle(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoDecodeH265PictureInfoKHR");
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH265DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07163", cb_state.Handle(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoDecodeH265DpbSlotInfoKHR");
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH265DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07164", cb_state.Handle(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoDecodeH265DpbSlotInfoKHR");
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoAV1(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                            const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;

    const bool inline_session_params_enabled =
        vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;
    const auto inline_session_params =
        vku::FindStructInPNextChain<VkVideoDecodeAV1InlineSessionParametersInfoKHR>(decode_info.pNext);

    const bool has_inline_seq_header =
        inline_session_params_enabled && inline_session_params != nullptr && inline_session_params->pStdSequenceHeader != nullptr;
    const bool needs_bound_session_params = !has_inline_seq_header;
    const bool has_bound_session_params = cb_state.bound_video_session_parameters != nullptr;

    if (needs_bound_session_params && !has_bound_session_params) {
        if (inline_session_params_enabled) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-10407", cb_state.Handle(), loc.function,
                             "no AV1 sequence header was specified through %s "
                             "but there is no bound video session parameters object.",
                             loc.pNext(Struct::VkVideoDecodeAV1InlineSessionParametersInfoKHR).Fields().c_str());
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-10407", cb_state.Handle(), loc.function,
                             "%s was created without VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR "
                             "but there is no bound video session parameters object.",
                             FormatHandle(vs_state).c_str());
        }
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeAV1DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09254", cb_state.Handle(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoDecodeAV1DpbSlotInfoKHR");
        }
    }

    vvl::unordered_set<int32_t> reference_slot_indices{};
    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeAV1DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-09255", cb_state.Handle(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoDecodeAV1DpbSlotInfoKHR");
        }

        reference_slot_indices.insert(decode_info.pReferenceSlots[i].slotIndex);
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeAV1PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;

        if (std_picture_info->flags.apply_grain) {
            if (!vs_state.profile->HasAV1FilmGrainSupport()) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-filmGrainSupport-09248", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeAV1PictureInfoKHR, Field::pStdPictureInfo),
                                 "has flags.apply_grain set but %s was created with an AV1 decode profile "
                                 "with filmGrainSupport disabled.",
                                 FormatHandle(vs_state).c_str());
            }

            if (decode_info.pSetupReferenceSlot != nullptr && decode_info.pSetupReferenceSlot->pPictureResource != nullptr) {
                auto dst_resource = vvl::VideoPictureResource(*this, decode_info.dstPictureResource);
                auto setup_resource = vvl::VideoPictureResource(*this, *decode_info.pSetupReferenceSlot->pPictureResource);
                if (dst_resource == setup_resource) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09249", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoDecodeAV1PictureInfoKHR, Field::pStdPictureInfo),
                                     "has flags.apply_grain set but the decode output picture and the "
                                     "reconstructed picture are not distinct.");
                }
            }
        }

        vvl::unordered_set<int32_t> reference_name_slot_indices{};
        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if (picture_info->referenceNameSlotIndices[i] >= 0) {
                if (reference_slot_indices.find(picture_info->referenceNameSlotIndices[i]) == reference_slot_indices.end()) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-referenceNameSlotIndices-09262", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoDecodeAV1PictureInfoKHR, Field::referenceNameSlotIndices, i),
                                     "(%d) does not match the slotIndex of any of the elements of pDecodeInfo->pReferenceSlots.",
                                     picture_info->referenceNameSlotIndices[i]);
                }

                reference_name_slot_indices.insert(picture_info->referenceNameSlotIndices[i]);
            }
        }

        for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
            if (reference_name_slot_indices.find(decode_info.pReferenceSlots[i].slotIndex) == reference_name_slot_indices.end()) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-slotIndex-09263", cb_state.Handle(),
                                 loc.dot(Field::pReferenceSlots, i),
                                 "(%d) does not match any of the elements of "
                                 "VkVideoDecodeAV1PictureInfoKHR::referenceNameSlotIndices.",
                                 decode_info.pReferenceSlots[i].slotIndex);
            }
        }

        if (picture_info->frameHeaderOffset >= decode_info.srcBufferRange) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-frameHeaderOffset-09251", cb_state.Handle(),
                             loc.pNext(Struct::VkVideoDecodeAV1PictureInfoKHR, Field::frameHeaderOffset),
                             "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                             picture_info->frameHeaderOffset, decode_info.srcBufferRange);
        }

        for (uint32_t i = 0; i < picture_info->tileCount; ++i) {
            if (picture_info->pTileOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09253", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeAV1PictureInfoKHR, Field::pTileOffsets, i),
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                                 picture_info->pTileOffsets[i], decode_info.srcBufferRange);
            }

            if (picture_info->pTileOffsets[i] + picture_info->pTileSizes[i] > decode_info.srcBufferRange) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoDecodeAV1PictureInfoKHR, Field::pTileOffsets, i),
                                 "(%u) plus pTileSizes[%u] (%u) is greater than pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                                 picture_info->pTileOffsets[i], i, picture_info->pTileSizes[i], decode_info.srcBufferRange);
            }
        }
    } else {
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-09250", cb_state.Handle(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoDecodeAV1PictureInfoKHR");
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeH264PicType(const vvl::VideoSession &vs_state, StdVideoH264PictureType pic_type,
                                                const Location &loc, const char *where) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (profile_caps.encode_h264.maxPPictureL0ReferenceCount == 0 && pic_type == STD_VIDEO_H264_PICTURE_TYPE_P) {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340", vs_state.Handle(), loc,
                         "%s is STD_VIDEO_H264_PICTURE_TYPE_P but P pictures "
                         "are not supported by the H.264 encode profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    if (profile_caps.encode_h264.maxBPictureL0ReferenceCount == 0 && profile_caps.encode_h264.maxL1ReferenceCount == 0 &&
        pic_type == STD_VIDEO_H264_PICTURE_TYPE_B) {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341", vs_state.Handle(), loc,
                         "%s is STD_VIDEO_H264_PICTURE_TYPE_B but B pictures "
                         "are not supported by the H.264 encode profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeInfoH264(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const auto &rc_state = cb_state.video_encode_rate_control_state;

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    if (encode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH264DpbSlotInfoKHR>(encode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08228", cb_state.Handle(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoEncodeH264DpbSlotInfoKHR");
        }
    }

    vvl::unordered_map<int32_t, const VkVideoEncodeH264DpbSlotInfoKHR *> reference_slots{};
    for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH264DpbSlotInfoKHR>(encode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08229", cb_state.Handle(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoEncodeH264DpbSlotInfoKHR");
        }

        reference_slots.insert({encode_info.pReferenceSlots[i].slotIndex, dpb_slot_info});
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoEncodeH264PictureInfoKHR>(encode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        auto std_sps = session_params.GetH264SPS(std_picture_info->seq_parameter_set_id);
        auto std_pps = session_params.GetH264PPS(std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id);

        const Location slice_count_loc = loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::naluSliceEntryCount);
        const Location slice_list_loc = loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pNaluSliceEntries);

        if (std_sps == nullptr) {
            const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-StdVideoH264SequenceParameterSet-08226", objlist, loc.function,
                             "no H.264 SPS with seq_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s.",
                             std_picture_info->seq_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (std_pps == nullptr) {
            const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-StdVideoH264PictureParameterSet-08227", objlist, loc.function,
                             "no H.264 PPS with seq_parameter_set_id = %u "
                             "and pic_parameter_set_id = %u exists in the bound video session parameters object %s.",
                             std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }

        if (picture_info->naluSliceEntryCount > profile_caps.encode_h264.maxSliceCount) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |=
                LogError("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301", objlist, slice_count_loc,
                         "(%u) exceeds the maxSliceCount (%u) limit supported by the H.264 encode profile %s was created with.",
                         picture_info->naluSliceEntryCount, profile_caps.encode_h264.maxSliceCount, FormatHandle(vs_state).c_str());
        }

        VkExtent2D max_coding_block_size = vs_state.profile->GetMaxCodingBlockSize();
        VkExtent2D min_coding_block_extent = {
            encode_info.srcPictureResource.codedExtent.width / max_coding_block_size.width,
            encode_info.srcPictureResource.codedExtent.height / max_coding_block_size.height,
        };
        if (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR) {
            if (picture_info->naluSliceEntryCount > min_coding_block_extent.width * min_coding_block_extent.height) {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302", cb_state.Handle(), slice_count_loc,
                                 "(%u) is greater than the number of MBs (minCodingBlockExtent = {%s}) "
                                 "that can be coded for the encode input picture specified in "
                                 "pEncodeInfo->srcPictureResource (codedExtent = {%s}).",
                                 picture_info->naluSliceEntryCount, string_VkExtent2D(min_coding_block_extent).c_str(),
                                 string_VkExtent2D(encode_info.srcPictureResource.codedExtent).c_str());
            }
        } else {
            if (picture_info->naluSliceEntryCount > min_coding_block_extent.height) {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312", cb_state.Handle(), slice_count_loc,
                                 "(%u) is greater than the number of MB rows (minCodingBlockExtent.height = %u) "
                                 "that can be coded for the encode input picture specified in "
                                 "pEncodeInfo->srcPictureResource (codedExtent = {%s}).",
                                 picture_info->naluSliceEntryCount, min_coding_block_extent.height,
                                 string_VkExtent2D(encode_info.srcPictureResource.codedExtent).c_str());
            }
        }

        bool different_slice_types = false;
        bool different_constant_qp_per_slice = false;
        for (uint32_t slice_idx = 0; slice_idx < picture_info->naluSliceEntryCount; ++slice_idx) {
            const auto &slice_info = picture_info->pNaluSliceEntries[slice_idx];
            const auto *std_slice_header = slice_info.pStdSliceHeader;
            const Location slice_info_loc = loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pNaluSliceEntries, slice_idx);

            if (std_slice_header->slice_type != picture_info->pNaluSliceEntries[0].pStdSliceHeader->slice_type) {
                different_slice_types = true;
            }

            if (rc_state.base.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
                if (slice_info.constantQp < profile_caps.encode_h264.minQp ||
                    slice_info.constantQp > profile_caps.encode_h264.maxQp) {
                    const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08270", objlist, slice_info_loc.dot(Field::constantQp),
                                     "(%d) is outside of the range [%d, %d] supported by the video "
                                     "profile %s was created with.",
                                     slice_info.constantQp, profile_caps.encode_h264.minQp, profile_caps.encode_h264.maxQp,
                                     FormatHandle(vs_state).c_str());
                }

                if (slice_info.constantQp != picture_info->pNaluSliceEntries[0].constantQp) {
                    different_constant_qp_per_slice = true;
                }
            } else {
                if (slice_info.constantQp != 0) {
                    const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08269", objlist, slice_info_loc.dot(Field::constantQp),
                                     "(%d) is not zero but the currently set video encode rate control mode for %s "
                                     "was specified to be %s when beginning the video coding scope.",
                                     slice_info.constantQp, FormatHandle(vs_state).c_str(),
                                     string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_state.base.rateControlMode));
                }
            }

            if (std_pps != nullptr &&
                (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0) {
                const char *weighted_pred_error_msg = nullptr;

                if (std_slice_header->slice_type == STD_VIDEO_H264_SLICE_TYPE_P && std_pps->flags.weighted_pred_flag) {
                    weighted_pred_error_msg =
                        "weighted_pred_flag is set in the active H.264 PPS, slice_type is STD_VIDEO_H264_SLICE_TYPE_P";
                } else if (std_slice_header->slice_type == STD_VIDEO_H264_SLICE_TYPE_B &&
                           std_pps->weighted_bipred_idc == STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT) {
                    weighted_pred_error_msg =
                        "weighted_bipred_idc is set to STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT "
                        "in the active H.264 PPS, slice_type is STD_VIDEO_H264_SLICE_TYPE_B";
                }

                if (std_slice_header->pWeightTable == nullptr && weighted_pred_error_msg != nullptr) {
                    const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                    skip |= LogError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314", objlist, loc.function,
                                     "%s, and pWeightTable is NULL in %s but "
                                     "VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR "
                                     "is not supported by the H.264 encode profile %s was created with.",
                                     weighted_pred_error_msg, slice_info_loc.dot(Field::pStdSliceHeader).Fields().c_str(),
                                     FormatHandle(vs_state).c_str());
                }
            }
        }

        if ((profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR) == 0 &&
            different_slice_types) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08315", objlist, loc.function,
                             "pStdSliceHeader->slice_type does not match across the elements "
                             "of %s but different slice types in a picture are not supported by the H.264 encode "
                             "profile %s was created with.",
                             slice_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        if ((profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR) == 0 &&
            different_constant_qp_per_slice) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08271", objlist, loc.function,
                             "constantQp does not match across the elements of %s"
                             "but per-slice constant QP values are not supported by the H.264 encode "
                             "profile %s was created with.",
                             slice_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        skip |= ValidateVideoEncodeH264PicType(vs_state, std_picture_info->primary_pic_type, loc.function,
                                               "VkVideoEncodeH264PictureInfoKHR::pStdPictureInfo->primary_pic_type");

        if (std_picture_info->pRefLists != nullptr) {
            vvl::unordered_set<uint8_t> ref_list_entries{};

            for (uint8_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList0[i];
                if (ref_list_entry == STD_VIDEO_H264_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH264PicType(vs_state, std_reference_info->primary_pic_type, loc.function,
                                                               "primary_pic_type for L0 reference");

                        if (std_reference_info->primary_pic_type == STD_VIDEO_H264_PICTURE_TYPE_B &&
                            (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-08342", objlist, loc.function,
                                             "primary_pic_type for L0 reference is "
                                             "STD_VIDEO_H264_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L0 reference list by the H.264 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08339", cb_state.Handle(), loc.function,
                                     "%s->pRefLists->RefPicList0[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint8_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList1[i];
                if (ref_list_entry == STD_VIDEO_H264_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH264PicType(vs_state, std_reference_info->primary_pic_type, loc.function,
                                                               "primary_pic_type for L1 reference");

                        if (std_reference_info->primary_pic_type == STD_VIDEO_H264_PICTURE_TYPE_B &&
                            (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-08343", objlist, loc.function,
                                             "primary_pic_type for L1 reference is "
                                             "STD_VIDEO_H264_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L1 reference list by the H.264 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08339", cb_state.Handle(), loc.function,
                                     "%s->pRefLists->RefPicList1[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
                int32_t slot_index = encode_info.pReferenceSlots[i].slotIndex;
                if (slot_index >= 0 && (uint32_t)slot_index < vs_state.create_info.maxDpbSlots &&
                    ref_list_entries.find((uint8_t)slot_index) == ref_list_entries.end()) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08353", cb_state.Handle(),
                                     loc.dot(Field::pReferenceSlots, i).dot(Field::slotIndex),
                                     "(%d) does not match "
                                     "any of the elements of RefPicList0 or RefPicList1 in %s->pRefLists.",
                                     slot_index,
                                     loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str());
                }
            }
        } else if (encode_info.referenceSlotCount > 0) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08352", cb_state.Handle(), loc.function,
                             "%s->pRefLists is NULL but pEncodeInfo->referenceSlotCount (%u) is not zero.",
                             loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(),
                             encode_info.referenceSlotCount);
        }

        if (picture_info->generatePrefixNalu &&
            (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR) == 0) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08304", objlist,
                             loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::generatePrefixNalu),
                             "is VK_TRUE but VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR "
                             "is not supported by the H.264 encode profile %s was created with.",
                             FormatHandle(vs_state).c_str());
        }
    } else {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08225", cb_state.Handle(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoEncodeH264PictureInfoKHR");
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeH265PicType(const vvl::VideoSession &vs_state, StdVideoH265PictureType pic_type,
                                                const Location &loc, const char *where) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (profile_caps.encode_h265.maxPPictureL0ReferenceCount == 0 && pic_type == STD_VIDEO_H265_PICTURE_TYPE_P) {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345", vs_state.Handle(), loc,
                         "%s is STD_VIDEO_H265_PICTURE_TYPE_P but P pictures "
                         "are not supported by the H.265 encode profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    if (profile_caps.encode_h265.maxBPictureL0ReferenceCount == 0 && profile_caps.encode_h265.maxL1ReferenceCount == 0 &&
        pic_type == STD_VIDEO_H265_PICTURE_TYPE_B) {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346", vs_state.Handle(), loc,
                         "%s is STD_VIDEO_H265_PICTURE_TYPE_B but B pictures "
                         "are not supported by the H.265 profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeInfoH265(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const auto &rc_state = cb_state.video_encode_rate_control_state;

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    if (encode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH265DpbSlotInfoKHR>(encode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08234", cb_state.Handle(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoEncodeH265DpbSlotInfoKHR");
        }
    }

    vvl::unordered_map<int32_t, const VkVideoEncodeH265DpbSlotInfoKHR *> reference_slots{};
    for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH265DpbSlotInfoKHR>(encode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08235", cb_state.Handle(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoEncodeH265DpbSlotInfoKHR");
        }

        reference_slots.insert({encode_info.pReferenceSlots[i].slotIndex, dpb_slot_info});
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoEncodeH265PictureInfoKHR>(encode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        auto std_vps = session_params.GetH265VPS(std_picture_info->sps_video_parameter_set_id);
        auto std_sps =
            session_params.GetH265SPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id);
        auto std_pps =
            session_params.GetH265PPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                      std_picture_info->pps_pic_parameter_set_id);

        const Location slice_seg_count_loc = loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::naluSliceSegmentEntryCount);
        const Location slice_seg_list_loc = loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pNaluSliceSegmentEntries);

        if (std_vps == nullptr) {
            const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-StdVideoH265VideoParameterSet-08231", objlist, loc.function,
                             "no H.265 VPS with sps_video_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (std_sps == nullptr) {
            const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-StdVideoH265SequenceParameterSet-08232", objlist, loc.function,
                             "no H.265 SPS with sps_video_parameter_set_id = %u "
                             "and pps_seq_parameter_set_id = %u exists in the bound video session "
                             "parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }

        if (std_pps == nullptr) {
            const LogObjectList objlist(cb_state.Handle(), vsp_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233", objlist, loc.function,
                             "no H.265 SPS with sps_video_parameter_set_id = %u, "
                             "pps_seq_parameter_set_id = %u, and pps_pic_parameter_set_id = %u exists in "
                             "the bound video session parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             std_picture_info->pps_pic_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (picture_info->naluSliceSegmentEntryCount > profile_caps.encode_h265.maxSliceSegmentCount) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError(
                "VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306", objlist, slice_seg_count_loc,
                "(%u) exceeds the maxSliceSegmentCount (%u) limit supported by the H.265 encode profile %s was created with.",
                picture_info->naluSliceSegmentEntryCount, profile_caps.encode_h265.maxSliceSegmentCount,
                FormatHandle(vs_state).c_str());
        }

        VkExtent2D max_coding_block_size = vs_state.profile->GetMaxCodingBlockSize();
        VkExtent2D min_coding_block_extent = {
            encode_info.srcPictureResource.codedExtent.width / max_coding_block_size.width,
            encode_info.srcPictureResource.codedExtent.height / max_coding_block_size.height,
        };
        if (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR) {
            if (picture_info->naluSliceSegmentEntryCount > min_coding_block_extent.width * min_coding_block_extent.height) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307", objlist, slice_seg_count_loc,
                                 "(%u) is greater than the number of CTBs (minCodingBlockExtent = {%s}) that can "
                                 "be coded for the encode input picture specified in pEncodeInfo->srcPictureResource "
                                 "(codedExtent = {%s}) assuming the maximum CTB size (%s) supported by the "
                                 "H.265 encode profile %s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, string_VkExtent2D(min_coding_block_extent).c_str(),
                                 string_VkExtent2D(encode_info.srcPictureResource.codedExtent).c_str(),
                                 string_VkExtent2D(max_coding_block_size).c_str(), FormatHandle(vs_state).c_str());
            }
        } else {
            if (picture_info->naluSliceSegmentEntryCount > min_coding_block_extent.height) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313", objlist, slice_seg_count_loc,
                                 "(%u) is greater than the number of CTB rows (minCodingBlockExtent.height = %u) that can "
                                 "be coded for the encode input picture specified in pEncodeInfo->srcPictureResource "
                                 "(codedExtent = {%s}) assuming the maximum CTB size (%s) supported by the "
                                 "H.265 encode profile %s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, min_coding_block_extent.height,
                                 string_VkExtent2D(encode_info.srcPictureResource.codedExtent).c_str(),
                                 string_VkExtent2D(max_coding_block_size).c_str(), FormatHandle(vs_state).c_str());
            }
        }

        if (std_pps != nullptr) {
            uint32_t num_tiles = (std_pps->num_tile_columns_minus1 + 1) * (std_pps->num_tile_rows_minus1 + 1);

            if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR) == 0 &&
                picture_info->naluSliceSegmentEntryCount < num_tiles) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08323", objlist, slice_seg_count_loc,
                                 "(%u) is less than the number of H.265 tiles (%u) in the encoded picture "
                                 "(num_tile_columns_minus1 = %u and num_tile_rows_minus1 = %u in the active H.265 PPS) "
                                 "but multiple tiles per slice segment are not supported by the H.265 encode profile "
                                 "%s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, num_tiles, std_pps->num_tile_columns_minus1,
                                 std_pps->num_tile_rows_minus1, FormatHandle(vs_state).c_str());
            }

            if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR) == 0 &&
                picture_info->naluSliceSegmentEntryCount > num_tiles) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324", objlist, slice_seg_count_loc,
                                 "(%u) is greater than the number of H.265 tiles (%u) in the encoded picture "
                                 "(num_tile_columns_minus1 = %u and num_tile_rows_minus1 = %u in the active H.265 PPS) "
                                 "but multiple slice segments per tile are not supported by the H.265 encode profile "
                                 "%s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, num_tiles, std_pps->num_tile_columns_minus1,
                                 std_pps->num_tile_rows_minus1, FormatHandle(vs_state).c_str());
            }
        }

        bool different_slice_segment_types = false;
        bool different_constant_qp_per_slice_segment = false;
        for (uint32_t slice_seg_idx = 0; slice_seg_idx < picture_info->naluSliceSegmentEntryCount; ++slice_seg_idx) {
            const auto &slice_segment_info = picture_info->pNaluSliceSegmentEntries[slice_seg_idx];
            const auto *std_slice_segment_header = slice_segment_info.pStdSliceSegmentHeader;
            const Location slice_seg_info_loc =
                loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pNaluSliceSegmentEntries, slice_seg_idx);

            if (std_slice_segment_header->slice_type !=
                picture_info->pNaluSliceSegmentEntries[0].pStdSliceSegmentHeader->slice_type) {
                different_slice_segment_types = true;
            }

            if (rc_state.base.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
                if (slice_segment_info.constantQp < profile_caps.encode_h265.minQp ||
                    slice_segment_info.constantQp > profile_caps.encode_h265.maxQp) {
                    const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                    skip |=
                        LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08273", objlist, slice_seg_info_loc.dot(Field::constantQp),
                                 "(%d) is outside of the range [%d, %d] supported by the video "
                                 "profile %s was created with.",
                                 slice_segment_info.constantQp, profile_caps.encode_h265.minQp, profile_caps.encode_h265.maxQp,
                                 FormatHandle(vs_state).c_str());
                }

                if (slice_segment_info.constantQp != picture_info->pNaluSliceSegmentEntries[0].constantQp) {
                    different_constant_qp_per_slice_segment = true;
                }
            } else {
                if (slice_segment_info.constantQp != 0) {
                    const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                    skip |=
                        LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08272", objlist, slice_seg_info_loc.dot(Field::constantQp),
                                 "(%d) is not zero but the currently set video encode rate control mode for %s "
                                 "was specified to be %s when beginning the video coding scope.",
                                 slice_segment_info.constantQp, FormatHandle(vs_state).c_str(),
                                 string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_state.base.rateControlMode));
                }
            }

            if (std_pps != nullptr &&
                (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0) {
                const char *weighted_pred_error_msg = nullptr;

                if (std_slice_segment_header->slice_type == STD_VIDEO_H265_SLICE_TYPE_P && std_pps->flags.weighted_pred_flag) {
                    weighted_pred_error_msg =
                        "weighted_pred_flag is set in the active H.265 PPS, slice_type is STD_VIDEO_H265_SLICE_TYPE_P";
                } else if (std_slice_segment_header->slice_type == STD_VIDEO_H265_SLICE_TYPE_B &&
                           std_pps->flags.weighted_bipred_flag) {
                    weighted_pred_error_msg =
                        "weighted_bipred_flag is set in the active H.265 PPS, slice_type is STD_VIDEO_H265_SLICE_TYPE_B";
                }

                if (std_slice_segment_header->pWeightTable == nullptr && weighted_pred_error_msg != nullptr) {
                    const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                    skip |=
                        LogError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316", objlist, loc.function,
                                 "%s, and pWeightTable is NULL in %s but "
                                 "VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR "
                                 "is not supported by the H.265 encode profile %s was created with.",
                                 weighted_pred_error_msg, slice_seg_info_loc.dot(Field::pStdSliceSegmentHeader).Fields().c_str(),
                                 FormatHandle(vs_state).c_str());
                }
            }
        }

        if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR) == 0 &&
            different_slice_segment_types) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08317", objlist, loc.function,
                             "pStdSliceSegmentHeader->slice_type does not match across the elements "
                             "of %s but different slice segment types in a picture are not supported by the H.265 encode "
                             "profile %s was created with.",
                             slice_seg_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR) == 0 &&
            different_constant_qp_per_slice_segment) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08274", objlist, loc.function,
                             "constantQp does not match across the elements of %s "
                             "but per-slice-segment constant QP values are not supported by the H.265 encode "
                             "profile %s was created with.",
                             slice_seg_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        skip |= ValidateVideoEncodeH265PicType(vs_state, std_picture_info->pic_type, loc.function,
                                               "VkVideoEncodeH265PictureInfoKHR::pStdPictureInfo->pic_type");

        if (std_picture_info->pRefLists != nullptr) {
            vvl::unordered_set<uint8_t> ref_list_entries{};

            for (uint8_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList0[i];
                if (ref_list_entry == STD_VIDEO_H265_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH265PicType(vs_state, std_reference_info->pic_type, loc.function,
                                                               "pic_type for L0 reference");

                        if (std_reference_info->pic_type == STD_VIDEO_H265_PICTURE_TYPE_B &&
                            (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-08347", objlist, loc.function,
                                             "pic_type for L0 reference is "
                                             "STD_VIDEO_H265_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L0 reference list by the H.265 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08344", cb_state.Handle(), loc.function,
                                     "%s->pRefLists->RefPicList0[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint8_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList1[i];
                if (ref_list_entry == STD_VIDEO_H265_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH265PicType(vs_state, std_reference_info->pic_type, loc.function,
                                                               "pic_type for L1 reference");

                        if (std_reference_info->pic_type == STD_VIDEO_H265_PICTURE_TYPE_B &&
                            (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-08348", objlist, loc.function,
                                             "pic_type for L1 reference is "
                                             "STD_VIDEO_H265_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L1 reference list by the H.265 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08344", cb_state.Handle(), loc.function,
                                     "%s->pRefLists->RefPicList1[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
                int32_t slot_index = encode_info.pReferenceSlots[i].slotIndex;
                if (slot_index >= 0 && (uint32_t)slot_index < vs_state.create_info.maxDpbSlots &&
                    ref_list_entries.find((uint8_t)slot_index) == ref_list_entries.end()) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08355", cb_state.Handle(),
                                     loc.dot(Field::pReferenceSlots, i).dot(Field::slotIndex),
                                     "(%d) does not match "
                                     "any of the elements of RefPicList0 or RefPicList1 in %s->pRefLists.",
                                     slot_index,
                                     loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str());
                }
            }
        } else if (encode_info.referenceSlotCount > 0) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08354", cb_state.Handle(), loc.function,
                             "%s->pRefLists is NULL but pEncodeInfo->referenceSlotCount (%u) is not zero.",
                             loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(),
                             encode_info.referenceSlotCount);
        }
    } else {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08230", cb_state.Handle(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoEncodeH265PictureInfoKHR");
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeInfoAV1(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                            const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const auto &rc_state = cb_state.video_encode_rate_control_state;

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();
    const auto std_seq_header = session_params.GetAV1SequenceHeader();

    const VkVideoEncodeAV1DpbSlotInfoKHR *setup_dpb_slot_info = nullptr;
    if (encode_info.pSetupReferenceSlot) {
        setup_dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeAV1DpbSlotInfoKHR>(encode_info.pSetupReferenceSlot->pNext);
        if (!setup_dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10318", cb_state.Handle(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoEncodeAV1DpbSlotInfoKHR");
        }
    }

    vvl::unordered_map<int32_t, const VkVideoEncodeAV1DpbSlotInfoKHR *> reference_slots{};
    for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeAV1DpbSlotInfoKHR>(encode_info.pReferenceSlots[i].pNext);
        if (dpb_slot_info) {
            if (dpb_slot_info->pStdReferenceInfo->pExtensionHeader != nullptr) {
                auto reference_loc = loc.dot(Field::pReferenceSlots, i);
                auto std_reference_info_loc = reference_loc.pNext(Struct::VkVideoEncodeAV1DpbSlotInfoKHR, Field::pStdReferenceInfo);
                auto av1_obu_ext_header = dpb_slot_info->pStdReferenceInfo->pExtensionHeader;

                if (av1_obu_ext_header->temporal_id >= profile_caps.encode_av1.maxTemporalLayerCount) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10341", cb_state.Handle(), std_reference_info_loc,
                                     "pExtensionHeader->temporal_id (%u) is greater than or equal to the maxTemporalLayer (%u) "
                                     "limit supported by the AV1 encode profile the bound video session %s was created with.",
                                     av1_obu_ext_header->temporal_id, profile_caps.encode_av1.maxTemporalLayerCount,
                                     FormatHandle(vs_state).c_str());
                }

                if (av1_obu_ext_header->spatial_id >= profile_caps.encode_av1.maxSpatialLayerCount) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10342", cb_state.Handle(), std_reference_info_loc,
                                     "pExtensionHeader->spatial_id (%u) is greater than or equal to the maxSpatialLayerCount (%u) "
                                     "limit supported by the AV1 encode profile the bound video session %s was created with.",
                                     av1_obu_ext_header->spatial_id, profile_caps.encode_av1.maxSpatialLayerCount,
                                     FormatHandle(vs_state).c_str());
                }
            }
        } else {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-10319", cb_state.Handle(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoEncodeAV1DpbSlotInfoKHR");
        }

        reference_slots.insert({encode_info.pReferenceSlots[i].slotIndex, dpb_slot_info});
    }

    if ((profile_caps.encode_av1.flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR) == 0) {
        if (encode_info.srcPictureResource.codedExtent.width != std_seq_header->max_frame_width_minus_1 + 1u) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-10323", objlist,
                             loc.dot(Field::srcPictureResource).dot(Field::codedExtent).dot(Field::width),
                             "(%u) does not equal max_frame_width_minus_1 (%u) plus one but "
                             "VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR is not supported "
                             "by the AV1 encode profile the bound video session %s was created with.",
                             encode_info.srcPictureResource.codedExtent.width, std_seq_header->max_frame_width_minus_1,
                             FormatHandle(vs_state).c_str());
        }
        if (encode_info.srcPictureResource.codedExtent.height != std_seq_header->max_frame_height_minus_1 + 1u) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-10324", objlist,
                             loc.dot(Field::srcPictureResource).dot(Field::codedExtent).dot(Field::height),
                             "(%u) does not equal max_frame_height_minus_1 (%u) plus one but "
                             "VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR is not supported "
                             "by the AV1 encode profile the bound video session %s was created with.",
                             encode_info.srcPictureResource.codedExtent.height, std_seq_header->max_frame_height_minus_1,
                             FormatHandle(vs_state).c_str());
        }
    }

    if ((profile_caps.encode_av1.flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR) == 0) {
        for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
            auto reference_resource = encode_info.pReferenceSlots[i].pPictureResource;
            if (reference_resource != nullptr &&
                (reference_resource->codedExtent.width != encode_info.srcPictureResource.codedExtent.width ||
                 reference_resource->codedExtent.height != encode_info.srcPictureResource.codedExtent.height)) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-10325", objlist,
                                 loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource).dot(Field::codedExtent),
                                 "(%u, %u) does not match %s (%u, %u) but "
                                 "VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR is not supported "
                                 "by the AV1 encode profile the bound video session %s was created with.",
                                 reference_resource->codedExtent.width, reference_resource->codedExtent.height,
                                 loc.dot(Field::srcPictureResource).dot(Field::codedExtent).Fields().c_str(),
                                 encode_info.srcPictureResource.codedExtent.width,
                                 encode_info.srcPictureResource.codedExtent.height, FormatHandle(vs_state).c_str());
            }
        }
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoEncodeAV1PictureInfoKHR>(encode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        std::optional<uint32_t> cdf_only_ref_index{};

        if (rc_state.base.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
            if (picture_info->constantQIndex < profile_caps.encode_av1.minQIndex ||
                picture_info->constantQIndex > profile_caps.encode_av1.maxQIndex) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321", objlist,
                                 loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::constantQIndex),
                                 "(%u) is outside of the range [%u, %u] supported by the video profile %s was created with.",
                                 picture_info->constantQIndex, profile_caps.encode_av1.minQIndex, profile_caps.encode_av1.maxQIndex,
                                 FormatHandle(vs_state).c_str());
            }
        } else {
            if (picture_info->constantQIndex != 0) {
                const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10320", objlist,
                                 loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::constantQIndex),
                                 "(%u) is not zero but the currently set video encode rate control mode for %s "
                                 "was specified to be %s when beginning the video coding scope.",
                                 picture_info->constantQIndex, FormatHandle(vs_state).c_str(),
                                 string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_state.base.rateControlMode));
            }
        }

        if (std_picture_info->primary_ref_frame < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR &&
            picture_info->referenceNameSlotIndices[std_picture_info->primary_ref_frame] < 0) {
            skip |= LogError("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291", cb_state.Handle(),
                             loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::referenceNameSlotIndices),
                             "contains a negative value (%d) for the primary reference name specified in "
                             "pStdPictureInfo->primary_ref_frame (%u).",
                             picture_info->referenceNameSlotIndices[std_picture_info->primary_ref_frame],
                             std_picture_info->primary_ref_frame);
        }

        if (picture_info->primaryReferenceCdfOnly == VK_TRUE) {
            if ((profile_caps.encode_av1.flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) == 0) {
                skip |= LogError("VUID-VkVideoEncodeAV1PictureInfoKHR-flags-10289", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::primaryReferenceCdfOnly),
                                 "is VK_TRUE but VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR "
                                 "is not supported by the AV1 encode profile the bound video session %s was created with.",
                                 FormatHandle(vs_state).c_str());
            }

            if (std_picture_info->primary_ref_frame < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR) {
                cdf_only_ref_index = std_picture_info->primary_ref_frame;
            } else {
                skip |= LogError("VUID-VkVideoEncodeAV1PictureInfoKHR-primaryReferenceCdfOnly-10290", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::primaryReferenceCdfOnly),
                                 "is VK_TRUE but the primary reference name specified in pStdPictureInfo->primary_ref_frame "
                                 "(%u) is not a valid reference name to use as the CDF only reference.",
                                 std_picture_info->primary_ref_frame);
            }
        }

        if (picture_info->generateObuExtensionHeader == VK_TRUE) {
            if ((profile_caps.encode_av1.flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR) == 0) {
                skip |= LogError("VUID-VkVideoEncodeAV1PictureInfoKHR-flags-10292", cb_state.Handle(),
                                 loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::generateObuExtensionHeader),
                                 "is VK_TRUE but VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR "
                                 "is not supported by the AV1 encode profile the bound video session %s was created with.",
                                 FormatHandle(vs_state).c_str());
            }

            if (std_picture_info->pExtensionHeader == nullptr) {
                skip |= LogError(
                    "VUID-VkVideoEncodeAV1PictureInfoKHR-generateObuExtensionHeader-10293", cb_state.Handle(),
                    loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::generateObuExtensionHeader),
                    "is VK_TRUE but no OBU extension header information is specified in pStdPictureInfo->pExtensionHeader.");
            }
        }

        if (picture_info->predictionMode == VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR &&
            std_picture_info->frame_type != STD_VIDEO_AV1_FRAME_TYPE_KEY &&
            std_picture_info->frame_type != STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-predictionMode-10326", cb_state.Handle(),
                             loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                             "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR but the AV1 frame type "
                             "is not KEY_FRAME or INTRA_ONLY_FRAME.");
        }

        if ((std_picture_info->frame_type == STD_VIDEO_AV1_FRAME_TYPE_KEY ||
             std_picture_info->frame_type == STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY) &&
            picture_info->predictionMode != VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10327", cb_state.Handle(),
                             loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                             "is not VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR (%s) but the AV1 frame type is %s.",
                             string_VkVideoEncodeAV1PredictionModeKHR(picture_info->predictionMode),
                             std_picture_info->frame_type == STD_VIDEO_AV1_FRAME_TYPE_KEY ? "KEY_FRAME" : "INTRA_ONLY_FRAME");
        }

        switch (picture_info->predictionMode) {
            case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR: {
                if (profile_caps.encode_av1.maxSingleReferenceCount == 0) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxSingleReferenceCount-10328", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                     "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR but "
                                     "maxSingleReferenceCount is zero.");
                    break;
                }

                bool has_supported_reference_name = false;
                for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                    if (i != cdf_only_ref_index && picture_info->referenceNameSlotIndices[i] >= 0 &&
                        (profile_caps.encode_av1.singleReferenceNameMask & (1 << i)) != 0) {
                        has_supported_reference_name = true;
                        break;
                    }
                }
                if (!has_supported_reference_name) {
                    if (cdf_only_ref_index.has_value() &&
                        ((profile_caps.encode_av1.singleReferenceNameMask & (1 << *cdf_only_ref_index)) != 0)) {
                        skip |= LogError("VUID-vkCmdEncodeVideoKHR-predictionMode-10329", cb_state.Handle(),
                                         loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                         "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR but "
                                         "referenceNameSlotIndices[] does not contain a non-negative DPB slot index for any of "
                                         "the supported reference names (singleReferenceNameMask = 0x%02X), except at index "
                                         "%u, but that reference is only used for CDF data reference.",
                                         profile_caps.encode_av1.singleReferenceNameMask, *cdf_only_ref_index);
                    } else {
                        skip |= LogError("VUID-vkCmdEncodeVideoKHR-predictionMode-10329", cb_state.Handle(),
                                         loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                         "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR but "
                                         "referenceNameSlotIndices[] does not contain a non-negative DPB slot index for any of "
                                         "the supported reference names (singleReferenceNameMask = 0x%02X).",
                                         profile_caps.encode_av1.singleReferenceNameMask);
                    }
                }
                break;
            }

            case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR: {
                if (profile_caps.encode_av1.maxUnidirectionalCompoundReferenceCount == 0) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxUnidirectionalCompoundReferenceCount-10330", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                     "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR but "
                                     "maxUnidirectionalCompoundReferenceCount is zero.");
                    break;
                }

                bool has_supported_reference_name_pair = false;

                auto ref_name_supported_and_used = [=](StdVideoAV1ReferenceName ref_name) {
                    const uint32_t ref_idx = ref_name - 1;
                    return cdf_only_ref_index != ref_idx && picture_info->referenceNameSlotIndices[ref_idx] > 0 &&
                           ((profile_caps.encode_av1.unidirectionalCompoundReferenceNameMask & (1 << ref_idx)) != 0);
                };

                // Check for LAST_FRAME + (LAST2_FRAME | LAST3_FRAME | GOLDEN_FRAME)
                if (ref_name_supported_and_used(STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME) &&
                    (ref_name_supported_and_used(STD_VIDEO_AV1_REFERENCE_NAME_LAST2_FRAME) ||
                     ref_name_supported_and_used(STD_VIDEO_AV1_REFERENCE_NAME_LAST3_FRAME) ||
                     ref_name_supported_and_used(STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME))) {
                    has_supported_reference_name_pair = true;
                }
                // Check for BWDREF_FRAME + ALTREF_FRAME
                if (ref_name_supported_and_used(STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME) &&
                    ref_name_supported_and_used(STD_VIDEO_AV1_REFERENCE_NAME_ALTREF_FRAME)) {
                    has_supported_reference_name_pair = true;
                }

                if (!has_supported_reference_name_pair) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                     "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR but "
                                     "referenceNameSlotIndices[] does not contain a non-negative DPB slot index for any of "
                                     "the valid pairs of unidirectional compound reference names (allowed options are: "
                                     "LAST_FRAME+LAST2_FRAME, LAST_FRAME+LAST3_FRAME, LAST_FRAME+GOLDEN_FRAME, or "
                                     "BWDREF_FRAME+ALTREF_FRAME) that are supported by the AV1 encode profile "
                                     "(unidirectionalCompoundReferenceNameMask=0x%02X) and are not used only for CDF reference.",
                                     profile_caps.encode_av1.unidirectionalCompoundReferenceNameMask);
                }
                break;
            }

            case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR: {
                if (profile_caps.encode_av1.maxBidirectionalCompoundReferenceCount == 0) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-maxBidirectionalCompoundReferenceCount-10332", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                     "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR but "
                                     "maxBidirectionalCompoundReferenceCount is zero.");
                    break;
                }

                bool has_supported_group1_reference_name = false;
                bool has_supported_group2_reference_name = false;
                for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                    if (i != cdf_only_ref_index && picture_info->referenceNameSlotIndices[i] >= 0 &&
                        (profile_caps.encode_av1.bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                        if (i < STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1) {
                            has_supported_group1_reference_name = true;
                            if (has_supported_group2_reference_name) break;
                        } else {
                            has_supported_group2_reference_name = true;
                            if (has_supported_group1_reference_name) break;
                        }
                    }
                }
                if (!has_supported_group1_reference_name || !has_supported_group2_reference_name) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::predictionMode),
                                     "is VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR but "
                                     "referenceNameSlotIndices[] does not contain a non-negative DPB slot index for at least "
                                     "one reference name from each group that are supported by the AV1 encode profile "
                                     "(bidirectionalCompoundReferenceNameMask = 0x%02X) and are not used only for CDF reference.",
                                     profile_caps.encode_av1.bidirectionalCompoundReferenceNameMask);
                }
                break;
            }

            default:
                break;
        }

        vvl::unordered_set<int32_t> reference_name_slot_indices{};
        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if (picture_info->referenceNameSlotIndices[i] >= 0) {
                if (reference_slots.find(picture_info->referenceNameSlotIndices[i]) == reference_slots.end()) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-referenceNameSlotIndices-10334", cb_state.Handle(),
                                     loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::referenceNameSlotIndices, i),
                                     "(%d) does not match the slotIndex of any of the elements of pEncodeInfo->pReferenceSlots.",
                                     picture_info->referenceNameSlotIndices[i]);
                }

                reference_name_slot_indices.insert(picture_info->referenceNameSlotIndices[i]);
            }
        }

        for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
            if (reference_name_slot_indices.find(encode_info.pReferenceSlots[i].slotIndex) == reference_name_slot_indices.end()) {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-slotIndex-10335", cb_state.Handle(), loc.dot(Field::pReferenceSlots, i),
                                 "(%d) does not match any of the elements of "
                                 "VkVideoEncodeAV1PictureInfoKHR::referenceNameSlotIndices.",
                                 encode_info.pReferenceSlots[i].slotIndex);
            }
        }

        auto std_picture_info_loc = loc.pNext(Struct::VkVideoEncodeAV1PictureInfoKHR, Field::pStdReferenceInfo);
        auto setup_reference_loc = loc.dot(Field::pSetupReferenceSlot);
        auto std_setup_reference_info_loc =
            setup_reference_loc.pNext(Struct::VkVideoEncodeAV1DpbSlotInfoKHR, Field::pStdReferenceInfo);
        if (std_picture_info->pExtensionHeader != nullptr) {
            if (std_picture_info->pExtensionHeader->temporal_id >= profile_caps.encode_av1.maxTemporalLayerCount) {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10336", cb_state.Handle(), std_picture_info_loc,
                                 "pExtensionHeader->temporal_id (%u) is greater than or equal to the maxTemporalLayer (%u) "
                                 "limit supported by the AV1 encode profile the bound video session %s was created with.",
                                 std_picture_info->pExtensionHeader->temporal_id, profile_caps.encode_av1.maxTemporalLayerCount,
                                 FormatHandle(vs_state).c_str());
            }

            if (std_picture_info->pExtensionHeader->spatial_id >= profile_caps.encode_av1.maxSpatialLayerCount) {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10337", cb_state.Handle(), std_picture_info_loc,
                                 "pExtensionHeader->spatial_id (%u) is greater than or equal to the maxSpatialLayerCount (%u) "
                                 "limit supported by the AV1 encode profile the bound video session %s was created with.",
                                 std_picture_info->pExtensionHeader->spatial_id, profile_caps.encode_av1.maxSpatialLayerCount,
                                 FormatHandle(vs_state).c_str());
            }

            if (setup_dpb_slot_info != nullptr) {
                if (setup_dpb_slot_info->pStdReferenceInfo->pExtensionHeader != nullptr) {
                    auto av1_setup_obu_ext_header = setup_dpb_slot_info->pStdReferenceInfo->pExtensionHeader;

                    if (std_picture_info->pExtensionHeader->temporal_id != av1_setup_obu_ext_header->temporal_id) {
                        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10339", cb_state.Handle(), std_picture_info_loc,
                                         "pExtensionHeader->temporal_id (%u) does not match the "
                                         "pExtensionHeader->temporal_id (%u) specified in %s.",
                                         std_picture_info->pExtensionHeader->temporal_id, av1_setup_obu_ext_header->temporal_id,
                                         std_setup_reference_info_loc.Fields().c_str());
                    }

                    if (std_picture_info->pExtensionHeader->spatial_id != av1_setup_obu_ext_header->spatial_id) {
                        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10340", cb_state.Handle(), std_picture_info_loc,
                                         "pExtensionHeader->spatial_id (%u) does not match the "
                                         "pExtensionHeader->spatial_id (%u) specified in %s.",
                                         std_picture_info->pExtensionHeader->spatial_id, av1_setup_obu_ext_header->spatial_id,
                                         std_setup_reference_info_loc.Fields().c_str());
                    }
                } else {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10338", cb_state.Handle(), std_picture_info_loc,
                                     "includes AV1 OBU extension header information (pExtensionHeader != NULL) but "
                                     "the setup reference info does not (pExtensionHeader == NULL in %s).",
                                     std_setup_reference_info_loc.Fields().c_str());
                }
            }
        } else if (setup_dpb_slot_info != nullptr && setup_dpb_slot_info->pStdReferenceInfo->pExtensionHeader != nullptr) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10338", cb_state.Handle(), std_setup_reference_info_loc,
                             "includes AV1 OBU extension header information (pExtensionHeader != NULL) but "
                             "the picture info does not (pExtensionHeader == NULL in %s).",
                             std_picture_info_loc.Fields().c_str());
        }

        if (std_picture_info->pTileInfo != nullptr) {
            auto tile_size = [](uint32_t extent, uint32_t tile_count) { return (extent + tile_count - 1) / tile_count; };

            if (std_picture_info->pTileInfo->TileCols != 0) {
                if (std_picture_info->pTileInfo->TileCols > profile_caps.encode_av1.maxTiles.width) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345", cb_state.Handle(), std_picture_info_loc,
                                     "pTileInfo is not NULL but the requested number of tile columns in pTileInfo->TileCols (%u) "
                                     "exceeds the maxTiles.width (%u) limit supported by the AV1 encode profile the "
                                     "bound video session %s was created with.",
                                     std_picture_info->pTileInfo->TileCols, profile_caps.encode_av1.maxTiles.width,
                                     FormatHandle(vs_state).c_str());
                }

                auto tile_width =
                    tile_size(encode_info.srcPictureResource.codedExtent.width, std_picture_info->pTileInfo->TileCols);
                if (tile_width < profile_caps.encode_av1.minTileSize.width ||
                    tile_width > profile_caps.encode_av1.maxTileSize.width) {
                    skip |= LogError(
                        "VUID-vkCmdEncodeVideoKHR-pTileInfo-10347", cb_state.Handle(), std_picture_info_loc,
                        "effective tile width (%u) calculated as ceil(pEncodeInfo->srcPictureResource.codedExtent.width [%u] / "
                        "pTileInfo->TileCols [%u]) must be between the minTileSize.width (%u) and maxTileSize.width (%u) "
                        "limits supported by the AV1 encode profile the bound video session %s was created with.",
                        tile_width, encode_info.srcPictureResource.codedExtent.width, std_picture_info->pTileInfo->TileCols,
                        profile_caps.encode_av1.minTileSize.width, profile_caps.encode_av1.maxTileSize.width,
                        FormatHandle(vs_state).c_str());
                }
            } else {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10343", cb_state.Handle(), std_picture_info_loc,
                                 "pTileInfo is not NULL but the requested number of tile columns in pTileInfo->TileCols is zero.");
            }

            if (std_picture_info->pTileInfo->TileRows != 0) {
                if (std_picture_info->pTileInfo->TileRows > profile_caps.encode_av1.maxTiles.height) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346", cb_state.Handle(), std_picture_info_loc,
                                     "pTileInfo is not NULL but the requested number of tile rows in pTileInfo->TileRows (%u) "
                                     "exceeds the maxTiles.height (%u) limit supported by the AV1 encode profile the "
                                     "bound video session %s was created with.",
                                     std_picture_info->pTileInfo->TileRows, profile_caps.encode_av1.maxTiles.height,
                                     FormatHandle(vs_state).c_str());
                }

                auto tile_height =
                    tile_size(encode_info.srcPictureResource.codedExtent.height, std_picture_info->pTileInfo->TileRows);
                if (tile_height < profile_caps.encode_av1.minTileSize.height ||
                    tile_height > profile_caps.encode_av1.maxTileSize.height) {
                    skip |= LogError(
                        "VUID-vkCmdEncodeVideoKHR-pTileInfo-10348", cb_state.Handle(), std_picture_info_loc,
                        "effective tile height (%u) calculated as ceil(pEncodeInfo->srcPictureResource.codedExtent.height [%u] / "
                        "pTileInfo->TileRows [%u]) must be between the minTileSize.height (%u) and maxTileSize.height (%u) "
                        "limits supported by the AV1 encode profile the bound video session %s was created with.",
                        tile_height, encode_info.srcPictureResource.codedExtent.height, std_picture_info->pTileInfo->TileRows,
                        profile_caps.encode_av1.minTileSize.height, profile_caps.encode_av1.maxTileSize.height,
                        FormatHandle(vs_state).c_str());
                }
            } else {
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10344", cb_state.Handle(), std_picture_info_loc,
                                 "pTileInfo is not NULL but the requested number of tile rows in pTileInfo->TileRows is zero.");
            }
        }

        if ((profile_caps.encode_av1.flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR) == 0 &&
            std_picture_info->flags.frame_size_override_flag != 0) {
            const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-flags-10322", objlist, std_picture_info_loc,
                             "has flags.frame_size_override_flag set but "
                             "VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR is not supported "
                             "by the AV1 encode profile the bound video session %s was created with.",
                             FormatHandle(vs_state).c_str());
        }

        if (std_picture_info->flags.segmentation_enabled != 0) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10349", cb_state.Handle(), std_picture_info_loc,
                             "AV1 encoding with segmentation is not supported but flags.segmentation_enabled is not zero.");
        }

        if (std_picture_info->pSegmentation != nullptr) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10350", cb_state.Handle(), std_picture_info_loc,
                             "AV1 encoding with segmentation is not supported but pSegmentation is not NULL.");
        }
    } else {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-10317", cb_state.Handle(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoEncodeAV1PictureInfoKHR");
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeQuantizationMapInfo(const vvl::CommandBuffer &cb_state, const VkExtent2D &coded_extent,
                                                        const VkVideoEncodeQuantizationMapInfoKHR &quantization_map_info,
                                                        const Location &loc) const {
    bool skip = false;

    bool hit_error = false;

    const auto vs_state = cb_state.bound_video_session.get();
    const auto vsp_state = cb_state.bound_video_session_parameters.get();

    if (vsp_state->create_info.flags & VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR) {
        const VkExtent2D texel_size = vsp_state->GetEncodeQuantizationMapTexelSize();
        const VkExtent2D expected_extent = {
            (coded_extent.width + texel_size.width - 1) / texel_size.width,
            (coded_extent.height + texel_size.height - 1) / texel_size.height,
        };
        if (quantization_map_info.quantizationMapExtent.width != expected_extent.width ||
            quantization_map_info.quantizationMapExtent.height != expected_extent.height) {
            const LogObjectList objlist(cb_state.Handle(), vsp_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-10316", objlist, loc.dot(Field::quantizationMapExtent),
                             "(%s) is not equal to ceil(pEncodeInfo->srcPictureResource.codedExtent (%s) / "
                             "quantizationMapTexelSize (%s)) (%s).",
                             string_VkExtent2D(quantization_map_info.quantizationMapExtent).c_str(),
                             string_VkExtent2D(coded_extent).c_str(), string_VkExtent2D(texel_size).c_str(),
                             string_VkExtent2D(expected_extent).c_str());
        }
    }

    const auto iv_state = Get<vvl::ImageView>(quantization_map_info.quantizationMap);
    if (iv_state) {
        auto iv_extent = iv_state->image_state->GetEffectiveSubresourceExtent(iv_state->normalized_subresource_range);
        if (iv_extent.width < quantization_map_info.quantizationMapExtent.width) {
            const LogObjectList objlist(cb_state.Handle(), iv_state->Handle(), iv_state->image_state->Handle());
            skip |= LogError("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10352", objlist,
                             loc.dot(Field::quantizationMapExtent).dot(Field::width),
                             "(%u) exceeds the width (%u) of the image view specified in %s (%s created from %s).",
                             quantization_map_info.quantizationMapExtent.width, iv_extent.width,
                             loc.dot(Field::quantizationMap).Fields().c_str(), FormatHandle(iv_state->Handle()).c_str(),
                             FormatHandle(iv_state->image_state->Handle()).c_str());
        }

        if (iv_extent.height < quantization_map_info.quantizationMapExtent.height) {
            const LogObjectList objlist(cb_state.Handle(), iv_state->Handle(), iv_state->image_state->Handle());
            skip |= LogError("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10353", objlist,
                             loc.dot(Field::quantizationMapExtent).dot(Field::height),
                             "(%u) exceeds the height (%u) of the image view specified in %s (%s created from %s).",
                             quantization_map_info.quantizationMapExtent.height, iv_extent.height,
                             loc.dot(Field::quantizationMap).Fields().c_str(), FormatHandle(iv_state->Handle()).c_str(),
                             FormatHandle(iv_state->image_state->Handle()).c_str());
        }

        skip |= ValidateProtectedImage(cb_state, *iv_state->image_state, loc.dot(Field::quantizationMap),
                                       "VUID-vkCmdEncodeVideoKHR-pNext-10313");

        skip |= VerifyImageLayout(cb_state, *iv_state->image_state, iv_state->normalized_subresource_range,
                                  VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, loc.dot(Field::quantizationMap),
                                  "VUID-vkCmdEncodeVideoKHR-pNext-10314", &hit_error);

        if (!IsImageCompatibleWithVideoSession(*iv_state->image_state, *vs_state)) {
            const LogObjectList objlist(cb_state.Handle(), vs_state->Handle(), iv_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10310", objlist, loc.dot(Field::quantizationMap),
                             "(%s created from %s) is not compatible with the video profile the bound "
                             "video session %s was created with.",
                             FormatHandle(iv_state->Handle()).c_str(), FormatHandle(iv_state->image_state->Handle()).c_str(),
                             FormatHandle(vs_state->Handle()).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateActiveReferencePictureCount(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                                     const Location &loc) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    uint32_t active_reference_picture_count = decode_info.referenceSlotCount;

    if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
            auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
            if (!dpb_slot_info) continue;

            auto std_reference_info = dpb_slot_info->pStdReferenceInfo;
            if (!std_reference_info) continue;

            if (std_reference_info->flags.top_field_flag && std_reference_info->flags.bottom_field_flag) {
                ++active_reference_picture_count;
            }
        }
    }

    if (active_reference_picture_count > vs_state.create_info.maxActiveReferencePictures) {
        const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150", objlist, loc,
                         "more active reference pictures (%u) were specified than "
                         "the maxActiveReferencePictures (%u) the bound video session %s was created with.",
                         active_reference_picture_count, vs_state.create_info.maxActiveReferencePictures,
                         FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateActiveReferencePictureCount(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                                     const Location &loc) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    uint32_t active_reference_picture_count = encode_info.referenceSlotCount;

    if (active_reference_picture_count > vs_state.create_info.maxActiveReferencePictures) {
        const LogObjectList objlist(cb_state.Handle(), vs_state.Handle());
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-activeReferencePictureCount-08216", objlist, loc,
                         "more active reference pictures (%u) were specified than "
                         "the maxActiveReferencePictures (%u) the bound video session %s was created with.",
                         active_reference_picture_count, vs_state.create_info.maxActiveReferencePictures,
                         FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateReferencePictureUseCount(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                                  const Location &loc) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    std::vector<uint32_t> dpb_frame_use_count(vs_state.create_info.maxDpbSlots, 0);

    bool interlaced_frame_support = false;
    std::vector<uint32_t> dpb_top_field_use_count;
    std::vector<uint32_t> dpb_bottom_field_use_count;

    if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        if (vs_state.profile->GetH264PictureLayout() != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR) {
            interlaced_frame_support = true;
            dpb_top_field_use_count.resize(vs_state.create_info.maxDpbSlots, 0);
            dpb_bottom_field_use_count.resize(vs_state.create_info.maxDpbSlots, 0);
        }
    }

    // Collect use count for each DPB across the elements pReferenceSlots and pSetupReferenceSlot
    for (uint32_t i = 0; i <= decode_info.referenceSlotCount; ++i) {
        const VkVideoReferenceSlotInfoKHR *slot =
            (i == decode_info.referenceSlotCount) ? decode_info.pSetupReferenceSlot : &decode_info.pReferenceSlots[i];

        if (slot == nullptr) continue;
        if (slot->slotIndex < 0 || (uint32_t)slot->slotIndex >= vs_state.create_info.maxDpbSlots) continue;

        ++dpb_frame_use_count[slot->slotIndex];

        if (!interlaced_frame_support) continue;

        if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
            auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(slot->pNext);
            if (!dpb_slot_info) continue;

            auto std_reference_info = dpb_slot_info->pStdReferenceInfo;
            if (!std_reference_info) continue;

            if (std_reference_info->flags.top_field_flag || std_reference_info->flags.bottom_field_flag) {
                --dpb_frame_use_count[slot->slotIndex];
            }
            if (std_reference_info->flags.top_field_flag) {
                ++dpb_top_field_use_count[slot->slotIndex];
            }
            if (std_reference_info->flags.bottom_field_flag) {
                ++dpb_bottom_field_use_count[slot->slotIndex];
            }
        }
    }

    for (uint32_t i = 0; i < vs_state.create_info.maxDpbSlots; ++i) {
        if (dpb_frame_use_count[i] > 1) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176", cb_state.Handle(), loc,
                             "frame in DPB slot %u is referred to multiple times across "
                             "pDecodeInfo->pSetupReferenceSlot and the elements of pDecodeInfo->pReferenceSlots.",
                             i);
        }
        if (interlaced_frame_support) {
            if (dpb_top_field_use_count[i] > 1) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177", cb_state.Handle(), loc,
                                 "top field in DPB slot %u is referred to multiple "
                                 "times across pDecodeInfo->pSetupReferenceSlot and the elements of "
                                 "pDecodeInfo->pReferenceSlots.",
                                 i);
            }
            if (dpb_bottom_field_use_count[i] > 1) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178", cb_state.Handle(), loc,
                                 "bottom field in DPB slot %u is referred to multiple "
                                 "times across pDecodeInfo->pSetupReferenceSlot and the elements of "
                                 "pDecodeInfo->pReferenceSlots.",
                                 i);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateReferencePictureUseCount(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                                  const Location &loc) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    std::vector<uint32_t> dpb_frame_use_count(vs_state.create_info.maxDpbSlots, 0);

    // Collect use count for each DPB across the elements pReferenceSlots and pSetupReferenceSlot
    for (uint32_t i = 0; i <= encode_info.referenceSlotCount; ++i) {
        const VkVideoReferenceSlotInfoKHR *slot =
            (i == encode_info.referenceSlotCount) ? encode_info.pSetupReferenceSlot : &encode_info.pReferenceSlots[i];

        if (slot == nullptr) continue;
        if (slot->slotIndex < 0 || (uint32_t)slot->slotIndex >= vs_state.create_info.maxDpbSlots) continue;

        ++dpb_frame_use_count[slot->slotIndex];
    }

    for (uint32_t i = 0; i < vs_state.create_info.maxDpbSlots; ++i) {
        if (dpb_frame_use_count[i] > 1) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221", cb_state.Handle(), loc,
                             "frame in DPB slot %u is referred to multiple times across "
                             "pEncodeInfo->pSetupReferenceSlot and the elements of pEncodeInfo->pReferenceSlots.",
                             i);
        }
    }

    return skip;
}

bool core::Instance::PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                          const VkVideoProfileInfoKHR *pVideoProfile,
                                                                          VkVideoCapabilitiesKHR *pCapabilities,
                                                                          const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateVideoProfileInfo(*this, pVideoProfile, error_obj, error_obj.location.dot(Field::pVideoProfile));

    const char *caps_pnext_msg = "chain does not contain a %s structure.";

    const Location caps_loc = error_obj.location.dot(Field::pCapabilities);

    bool is_decode = false;
    bool is_encode = false;

    switch (pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeH264CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoDecodeH264CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeH265CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoDecodeH265CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeAV1CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-09257", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoDecodeAV1CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            is_encode = true;
            if (!vku::FindStructInPNextChain<VkVideoEncodeH264CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07187", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoEncodeH264CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            is_encode = true;
            if (!vku::FindStructInPNextChain<VkVideoEncodeH265CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07188", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoEncodeH265CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            is_encode = true;
            if (!vku::FindStructInPNextChain<VkVideoEncodeAV1CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-10263", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoEncodeAV1CapabilitiesKHR");
            }
            break;

        default:
            break;
    }

    if (is_decode && !vku::FindStructInPNextChain<VkVideoDecodeCapabilitiesKHR>(pCapabilities->pNext)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183", physicalDevice, caps_loc,
                         caps_pnext_msg, "VkVideoDecodeCapabilitiesKHR");
    }

    if (is_encode && !vku::FindStructInPNextChain<VkVideoEncodeCapabilitiesKHR>(pCapabilities->pNext)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186", physicalDevice, caps_loc,
                         caps_pnext_msg, "VkVideoEncodeCapabilitiesKHR");
    }

    return skip;
}

bool core::Instance::PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo,
    uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *video_profiles = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pVideoFormatInfo->pNext);
    if (video_profiles && video_profiles->profileCount != 0) {
        skip |=
            ValidateVideoProfileListInfo(*this, video_profiles, error_obj,
                                         error_obj.location.dot(Field::pVideoFormatInfo).pNext(Struct::VkVideoProfileListInfoKHR),
                                         false, nullptr, false, nullptr);
    } else {
        const char *msg = video_profiles ? "no VkVideoProfileListInfoKHR structure found in the pNext chain of pVideoFormatInfo."
                                         : "profileCount is zero in the VkVideoProfileListInfoKHR structure included in the "
                                           "pNext chain of pVideoFormatInfo.";
        skip |=
            LogError("VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812", physicalDevice, error_obj.location, "%s", msg);
    }

    return skip;
}

bool core::Instance::PreCallValidateGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR *pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR *pQualityLevelProperties, const ErrorObject &error_obj) const {
    bool skip = false;

    const Location quality_level_info_loc = error_obj.location.dot(Field::pQualityLevelInfo);
    const Location quality_level_props_loc = error_obj.location.dot(Field::pQualityLevelProperties);

    const char *props_pnext_msg = "chain does not contain a %s structure.";

    skip |= core::ValidateVideoProfileInfo(*this, pQualityLevelInfo->pVideoProfile, error_obj,
                                           quality_level_info_loc.dot(Field::pVideoProfile));

    vvl::VideoProfileDesc profile_desc(physicalDevice, pQualityLevelInfo->pVideoProfile);
    const auto &profile_caps = profile_desc.GetCapabilities();

    if (!profile_desc.IsEncode()) {
        skip |= LogError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08260", physicalDevice,
                         quality_level_info_loc.dot(Field::pVideoProfile), "does not specify an encode profile.");
    }

    if (profile_caps.supported) {
        if (profile_desc.IsEncode() && pQualityLevelInfo->qualityLevel >= profile_caps.encode.maxQualityLevels) {
            skip |= LogError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-qualityLevel-08261", physicalDevice,
                             quality_level_info_loc.dot(Field::qualityLevel),
                             "(%u) must be smaller than the VkVideoEncodeCapabilitiesKHR::maxQualityLevels (%u) limit "
                             "supported by the specified video profile.",
                             pQualityLevelInfo->qualityLevel, profile_caps.encode.maxQualityLevels);
        }
    } else {
        skip |= LogError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259", physicalDevice,
                         quality_level_info_loc.dot(Field::pVideoProfile), "is not supported.");
    }

    switch (pQualityLevelInfo->pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            if (!vku::FindStructInPNextChain<VkVideoEncodeH264QualityLevelPropertiesKHR>(pQualityLevelProperties->pNext)) {
                skip |=
                    LogError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08257", physicalDevice,
                             quality_level_props_loc, props_pnext_msg, "VkVideoEncodeH264QualityLevelPropertiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            if (!vku::FindStructInPNextChain<VkVideoEncodeH265QualityLevelPropertiesKHR>(pQualityLevelProperties->pNext)) {
                skip |=
                    LogError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08258", physicalDevice,
                             quality_level_props_loc, props_pnext_msg, "VkVideoEncodeH264QualityLevelPropertiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            if (!vku::FindStructInPNextChain<VkVideoEncodeAV1QualityLevelPropertiesKHR>(pQualityLevelProperties->pNext)) {
                skip |=
                    LogError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-10305", physicalDevice,
                             quality_level_props_loc, props_pnext_msg, "VkVideoEncodeAV1QualityLevelPropertiesKHR");
            }
            break;

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    skip |= core::ValidateVideoProfileInfo(*this, pCreateInfo->pVideoProfile, error_obj, create_info_loc.dot(Field::pVideoProfile));

    vvl::VideoProfileDesc profile_desc(physical_device, pCreateInfo->pVideoProfile);
    const auto &profile_caps = profile_desc.GetCapabilities();

    if (profile_caps.supported) {
        if (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) {
            const char *error_msg = nullptr;
            if (enabled_features.protectedMemory == VK_FALSE) {
                error_msg = "the protectedMemory feature is not enabled";
            } else if ((profile_caps.base.flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) == 0) {
                error_msg = "protected content is not supported for the video profile";
            }
            if (error_msg != nullptr) {
                skip |=
                    LogError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR set but %s.", error_msg);
            }
        }

        if (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR && !enabled_features.videoMaintenance1) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-08371", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR set but "
                             "the videoMaintenance1 device feature is not enabled.");
        }

        if (pCreateInfo->flags & (VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR |
                                  VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR)) {
            if (!enabled_features.videoEncodeQuantizationMap) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10264", device, create_info_loc.dot(Field::flags),
                                 "has VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR or "
                                 "VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_MAP_BIT_KHR set but "
                                 "the videoEncodeQuantizationMap device feature is not enabled.");
            }

            if ((pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
                (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR)) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10266", device, create_info_loc.dot(Field::flags),
                                 "has both VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR and "
                                 "VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_MAP_BIT_KHR set.");
            }

            if (profile_desc.IsEncode()) {
                if ((pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
                    (profile_caps.encode.flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) == 0) {
                    skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10267", device, create_info_loc.dot(Field::flags),
                                     "has VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR set but the "
                                     "video profile does not support VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR.");
                }

                if ((pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR) &&
                    (profile_caps.encode.flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) == 0) {
                    skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10268", device, create_info_loc.dot(Field::flags),
                                     "has VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR set but the "
                                     "video profile does not support VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR.");
                }
            } else {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10265", device, create_info_loc.dot(Field::flags),
                                 "has VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR or "
                                 "VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_MAP_BIT_KHR set but "
                                 "the video profile does not specify an encode operation.");
            }
        }

        if (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR && !enabled_features.videoMaintenance2) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10398", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR set but "
                             "the videoMaintenance2 device feature is not enabled.");
        }

        if (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR && !profile_desc.IsDecode()) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-10399", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR set but "
                             "%s does not specify a decode operation.",
                             create_info_loc.dot(Field::pVideoProfile).Fields().c_str());
        }

        if (!IsBetweenInclusive(pCreateInfo->maxCodedExtent, profile_caps.base.minCodedExtent, profile_caps.base.maxCodedExtent)) {
            skip |= LogError(
                "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851", device, create_info_loc.dot(Field::maxCodedExtent),
                "(%s) is outside of the "
                "range (%s)-(%s) supported by the video profile.",
                string_VkExtent2D(pCreateInfo->maxCodedExtent).c_str(), string_VkExtent2D(profile_caps.base.minCodedExtent).c_str(),
                string_VkExtent2D(profile_caps.base.maxCodedExtent).c_str());
        }

        if (pCreateInfo->maxDpbSlots > profile_caps.base.maxDpbSlots) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847", device, create_info_loc.dot(Field::maxDpbSlots),
                             "(%u) is greater than the "
                             "maxDpbSlots (%u) supported by the video profile.",
                             pCreateInfo->maxDpbSlots, profile_caps.base.maxDpbSlots);
        }

        if (pCreateInfo->maxActiveReferencePictures > profile_caps.base.maxActiveReferencePictures) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849", device, error_obj.location,
                             "pCreateInfo->maxActiveReferencePictures (%u) is greater "
                             "than the maxActiveReferencePictures (%u) supported by the video profile.",
                             pCreateInfo->maxActiveReferencePictures, profile_caps.base.maxActiveReferencePictures);
        }

        if ((pCreateInfo->maxDpbSlots == 0 && pCreateInfo->maxActiveReferencePictures != 0) ||
            (pCreateInfo->maxDpbSlots != 0 && pCreateInfo->maxActiveReferencePictures == 0)) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850", device, error_obj.location,
                             "if either pCreateInfo->maxDpbSlots (%u) or "
                             "pCreateInfo->maxActiveReferencePictures (%u) is zero then both must be zero.",
                             pCreateInfo->maxDpbSlots, pCreateInfo->maxActiveReferencePictures);
        }

        if (profile_desc.IsDecode() && pCreateInfo->maxActiveReferencePictures > 0 &&
            !IsVideoFormatSupported(pCreateInfo->referencePictureFormat, VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
                                    pCreateInfo->pVideoProfile)) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852", device,
                             create_info_loc.dot(Field::referencePictureFormat),
                             "(%s) is not a supported "
                             "decode DPB format for the video profile specified in pCreateInfo->pVideoProfile.",
                             string_VkFormat(pCreateInfo->referencePictureFormat));
        }

        if (profile_desc.IsEncode() && pCreateInfo->maxActiveReferencePictures > 0 &&
            !IsVideoFormatSupported(pCreateInfo->referencePictureFormat, VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
                                    pCreateInfo->pVideoProfile)) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-06814", device,
                             create_info_loc.dot(Field::referencePictureFormat),
                             "(%s) is not a supported "
                             "encode DPB format for the video profile specified in pCreateInfo->pVideoProfile.",
                             string_VkFormat(pCreateInfo->referencePictureFormat));
        }

        if (profile_desc.IsDecode() && !IsVideoFormatSupported(pCreateInfo->pictureFormat, VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
                                                               pCreateInfo->pVideoProfile)) {
            skip |=
                LogError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853", device, create_info_loc.dot(Field::pictureFormat),
                         "(%s) is not a supported "
                         "decode output format for the video profile specified in pCreateInfo->pVideoProfile.",
                         string_VkFormat(pCreateInfo->pictureFormat));
        }

        if (profile_desc.IsEncode() && !IsVideoFormatSupported(pCreateInfo->pictureFormat, VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
                                                               pCreateInfo->pVideoProfile)) {
            skip |=
                LogError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04854", device, create_info_loc.dot(Field::pictureFormat),
                         "(%s) is not a supported "
                         "encode input format for the video profile specified in pCreateInfo->pVideoProfile.",
                         string_VkFormat(pCreateInfo->pictureFormat));
        }

        if (strncmp(pCreateInfo->pStdHeaderVersion->extensionName, profile_caps.base.stdHeaderVersion.extensionName,
                    VK_MAX_EXTENSION_NAME_SIZE)) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190", device,
                             create_info_loc.dot(Field::pStdHeaderVersion).dot(Field::extensionName),
                             "'%.*s' is an unsupported Video Std header name, expected '%.*s'.", VK_MAX_EXTENSION_NAME_SIZE,
                             pCreateInfo->pStdHeaderVersion->extensionName, VK_MAX_EXTENSION_NAME_SIZE,
                             profile_caps.base.stdHeaderVersion.extensionName);
        }

        if (pCreateInfo->pStdHeaderVersion->specVersion > profile_caps.base.stdHeaderVersion.specVersion) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191", device,
                             create_info_loc.dot(Field::pStdHeaderVersion).dot(Field::specVersion),
                             "(0x%08x) is larger than the supported version (0x%08x).", pCreateInfo->pStdHeaderVersion->specVersion,
                             profile_caps.base.stdHeaderVersion.specVersion);
        }
    } else {
        skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-04845", device, create_info_loc.dot(Field::pVideoProfile),
                         "is not supported.");
    }

    switch (pCreateInfo->pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto h264_create_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionCreateInfoKHR>(pCreateInfo);
            if (h264_create_info != nullptr && h264_create_info->maxLevelIdc > profile_caps.encode_h264.maxLevelIdc) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08251", device,
                                 create_info_loc.pNext(Struct::VkVideoEncodeH264SessionCreateInfoKHR, Field::maxLevelIdc),
                                 "(%u) exceeds the maxLevelIdc (%u) supported by the specified H.264 encode profile.",
                                 h264_create_info->maxLevelIdc, profile_caps.encode_h264.maxLevelIdc);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto h265_create_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionCreateInfoKHR>(pCreateInfo);
            if (h265_create_info != nullptr && h265_create_info->maxLevelIdc > profile_caps.encode_h265.maxLevelIdc) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08252", device,
                                 create_info_loc.pNext(Struct::VkVideoEncodeH265SessionCreateInfoKHR, Field::maxLevelIdc),
                                 "(%u) exceeds the maxLevelIdc (%u) supported by the specified H.265 encode profile.",
                                 h265_create_info->maxLevelIdc, profile_caps.encode_h265.maxLevelIdc);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            auto av1_create_info = vku::FindStructInPNextChain<VkVideoEncodeAV1SessionCreateInfoKHR>(pCreateInfo);
            if (av1_create_info != nullptr && av1_create_info->maxLevel > profile_caps.encode_av1.maxLevel) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10270", device,
                                 create_info_loc.pNext(Struct::VkVideoEncodeAV1SessionCreateInfoKHR, Field::maxLevel),
                                 "(%u) exceeds the maxLevel (%u) supported by the specified AV1 encode profile.",
                                 av1_create_info->maxLevel, profile_caps.encode_av1.maxLevel);
            }
            break;
        }

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto video_session_state = Get<vvl::VideoSession>(videoSession)) {
        skip |= ValidateObjectNotInUse(video_session_state.get(), error_obj.location,
                                       "VUID-vkDestroyVideoSessionKHR-videoSession-07192");
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                          uint32_t bindSessionMemoryInfoCount,
                                                          const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;

    auto vs_state = Get<vvl::VideoSession>(videoSession);
    if (!vs_state) return skip;

    if (pBindSessionMemoryInfos) {
        {
            vvl::unordered_set<uint32_t> memory_bind_indices;
            for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
                uint32_t mem_bind_index = pBindSessionMemoryInfos[i].memoryBindIndex;
                if (memory_bind_indices.find(mem_bind_index) != memory_bind_indices.end()) {
                    skip |= LogError("VUID-vkBindVideoSessionMemoryKHR-memoryBindIndex-07196", videoSession,
                                     error_obj.location.dot(Field::pBindSessionMemoryInfos, i).dot(Field::memoryBindIndex),
                                     "%u is not unique.", mem_bind_index);
                    break;
                }
                memory_bind_indices.emplace(mem_bind_index);
            }
        }

        for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
            const auto &bind_info = pBindSessionMemoryInfos[i];
            const auto &mem_binding_info = vs_state->GetMemoryBindingInfo(bind_info.memoryBindIndex);
            if (mem_binding_info != nullptr) {
                if (auto memory_state = Get<vvl::DeviceMemory>(bind_info.memory)) {
                    if (((1 << memory_state->allocate_info.memoryTypeIndex) & mem_binding_info->requirements.memoryTypeBits) == 0) {
                        const LogObjectList objlist(videoSession, memory_state->Handle());
                        skip |=
                            LogError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07198", objlist, error_obj.location,
                                     "memoryTypeBits (0x%x) for memory binding "
                                     "with index %u of %s are not compatible with the memory type index (%u) of "
                                     "%s specified in pBindSessionMemoryInfos[%u].memory.",
                                     mem_binding_info->requirements.memoryTypeBits, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str(), memory_state->allocate_info.memoryTypeIndex,
                                     FormatHandle(*memory_state).c_str(), i);
                    }

                    if (bind_info.memoryOffset >= memory_state->allocate_info.allocationSize) {
                        const LogObjectList objlist(videoSession, memory_state->Handle());
                        skip |= LogError("VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201", objlist,
                                         error_obj.location.dot(Field::pBindSessionMemoryInfos, i).dot(Field::memoryOffset),
                                         "(%" PRIuLEAST64 ") must be less than the size (%" PRIuLEAST64 ") of %s.",
                                         bind_info.memoryOffset, memory_state->allocate_info.allocationSize,
                                         FormatHandle(*memory_state).c_str());
                    } else if (bind_info.memoryOffset + bind_info.memorySize > memory_state->allocate_info.allocationSize) {
                        const LogObjectList objlist(videoSession, memory_state->Handle());
                        skip |= LogError("VUID-VkBindVideoSessionMemoryInfoKHR-memorySize-07202", objlist,
                                         error_obj.location.dot(Field::pBindSessionMemoryInfos, i).dot(Field::memoryOffset),
                                         "(%" PRIuLEAST64 ") + memory size (%" PRIuLEAST64
                                         ") must be less than or equal to the size (%" PRIuLEAST64 ") of %s.",
                                         bind_info.memoryOffset, bind_info.memorySize, memory_state->allocate_info.allocationSize,
                                         FormatHandle(*memory_state).c_str());
                    }
                }

                if (SafeModulo(bind_info.memoryOffset, mem_binding_info->requirements.alignment) != 0) {
                    skip |= LogError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199", videoSession,
                                     error_obj.location.dot(Field::pBindSessionMemoryInfos, i).dot(Field::memoryOffset),
                                     "(%" PRIuLEAST64 ") but must be an integer multiple of the alignment value %" PRIuLEAST64
                                     " for the memory binding index %u of %s.",
                                     bind_info.memoryOffset, mem_binding_info->requirements.alignment, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str());
                }

                if (bind_info.memorySize != mem_binding_info->requirements.size) {
                    skip |= LogError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200", videoSession,
                                     error_obj.location.dot(Field::pBindSessionMemoryInfos, i).dot(Field::memorySize),
                                     "(%" PRIuLEAST64 ") does not equal the required size (%" PRIuLEAST64
                                     ") for the memory binding index %u of %s.",
                                     bind_info.memorySize, mem_binding_info->requirements.size, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str());
                }

                if (mem_binding_info->bound) {
                    skip |= LogError("VUID-vkBindVideoSessionMemoryKHR-videoSession-07195", videoSession, error_obj.location,
                                     "memory binding with index %u of %s is already "
                                     "bound but was specified in pBindSessionMemoryInfos[%u].memoryBindIndex.",
                                     bind_info.memoryBindIndex, FormatHandle(videoSession).c_str(), i);
                }
            } else {
                skip |= LogError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07197", videoSession, error_obj.location,
                                 "%s does not have a memory binding corresponding "
                                 "to the memoryBindIndex specified in pBindSessionMemoryInfos[%u].",
                                 FormatHandle(videoSession).c_str(), i);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateVideoSessionParametersKHR(VkDevice device,
                                                                const VkVideoSessionParametersCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkVideoSessionParametersKHR *pVideoSessionParameters,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    std::shared_ptr<const vvl::VideoSessionParameters> template_state;
    if (pCreateInfo->videoSessionParametersTemplate != VK_NULL_HANDLE) {
        template_state = Get<vvl::VideoSessionParameters>(pCreateInfo->videoSessionParametersTemplate);
        if (template_state && (template_state->vs_state->VkHandle() != pCreateInfo->videoSession)) {
            template_state = nullptr;
            const LogObjectList objlist(device, pCreateInfo->videoSessionParametersTemplate, pCreateInfo->videoSession);
            skip |= LogError(
                "VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-04855", objlist,
                create_info_loc.dot(Field::videoSessionParametersTemplate), "(%s) was not created against the same %s.",
                FormatHandle(pCreateInfo->videoSessionParametersTemplate).c_str(), FormatHandle(pCreateInfo->videoSession).c_str());
        }
    }

    auto vs_state = Get<vvl::VideoSession>(pCreateInfo->videoSession);
    if (!vs_state) return skip;

    const char *pnext_chain_msg = "does not contain a %s structure.";
    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateDecodeH264ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoDecodeH264SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07203", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoDecodeH264SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateDecodeH265ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoDecodeH265SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07206", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoDecodeH265SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeAV1SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (!codec_info) {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09259", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoDecodeAV1SessionParametersCreateInfoKHR");
            }
            if (pCreateInfo->videoSessionParametersTemplate != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09258", device,
                                 create_info_loc.dot(Field::videoSessionParametersTemplate),
                                 "must be VK_NULL_HANDLE when using an AV1 decode profile.");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateEncodeH264ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07210", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoEncodeH264SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateEncodeH265ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoEncodeH265SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07211", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoEncodeH265SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoEncodeAV1SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                if (codec_info->stdOperatingPointCount > vs_state->profile->GetCapabilities().encode_av1.maxOperatingPoints) {
                    skip |= LogError(
                        "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10280", device,
                        create_info_loc.pNext(Struct::VkVideoEncodeAV1SessionParametersCreateInfoKHR,
                                              Field::stdOperatingPointCount),
                        "(%u) is greater than the VkVideoEncodeAV1Capabilities::maxOperatingPoints (%u) supported by the "
                        "AV1 encode profile VkVideoSessionParametersCreateInfoKHR::videoSession (%s) was created with.",
                        codec_info->stdOperatingPointCount, vs_state->profile->GetCapabilities().encode_av1.maxOperatingPoints,
                        FormatHandle(*vs_state).c_str());
                }

                if (codec_info->pStdSequenceHeader != nullptr &&
                    codec_info->pStdSequenceHeader->flags.film_grain_params_present != 0) {
                    skip |= LogError(
                        "VUID-VkVideoEncodeAV1SessionParametersCreateInfoKHR-pStdSequenceHeader-10288", device,
                        create_info_loc.pNext(Struct::VkVideoEncodeAV1SessionParametersCreateInfoKHR, Field::pStdSequenceHeader),
                        "AV1 encoding with film grain is not supported but flags.film_grain_params_present is not zero.");
                }
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10279", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoEncodeAV1SessionParametersCreateInfoKHR");
            }
            if (pCreateInfo->videoSessionParametersTemplate != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10278", device,
                                 create_info_loc.dot(Field::videoSessionParametersTemplate),
                                 "must be VK_NULL_HANDLE when using an AV1 encode profile.");
            }
            break;
        }

        default:
            break;
    }

    if (vs_state->IsEncode()) {
        const uint32_t max_quality_levels = vs_state->profile->GetCapabilities().encode.maxQualityLevels;
        auto quality_level_info = vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(pCreateInfo->pNext);
        uint32_t encode_quality_level = 0;

        if (quality_level_info != nullptr) {
            encode_quality_level = quality_level_info->qualityLevel;
            if (encode_quality_level >= max_quality_levels) {
                skip |= LogError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311", pCreateInfo->videoSession,
                                 create_info_loc.pNext(Struct::VkVideoEncodeQualityLevelInfoKHR, Field::qualityLevel),
                                 "(%u) must be smaller than the maxQualityLevels (%u) supported by the video "
                                 "profile %s was created with.",
                                 encode_quality_level, max_quality_levels, FormatHandle(pCreateInfo->videoSession).c_str());
            }
        }

        if (template_state != nullptr && encode_quality_level != template_state->GetEncodeQualityLevel()) {
            const LogObjectList objlist(device, pCreateInfo->videoSessionParametersTemplate, pCreateInfo->videoSession);
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310)", objlist,
                             create_info_loc.pNext(Struct::VkVideoEncodeQualityLevelInfoKHR, Field::qualityLevel),
                             "(%u) does not match the video encode quality level (%u) template %s was created with.",
                             encode_quality_level, template_state->GetEncodeQualityLevel(),
                             FormatHandle(pCreateInfo->videoSessionParametersTemplate).c_str());
        }

        if (pCreateInfo->flags & VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR) {
            if (template_state != nullptr && ((template_state->create_info.flags &
                                               VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR) == 0)) {
                const LogObjectList objlist(device, pCreateInfo->videoSessionParametersTemplate, pCreateInfo->videoSession);
                skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10275", objlist,
                                 create_info_loc.dot(Field::flags),
                                 "has VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR set but the template "
                                 "%s was not created with VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR.",
                                 FormatHandle(pCreateInfo->videoSessionParametersTemplate).c_str());
            }

            if ((vs_state->create_info.flags & (VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR |
                                                VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR)) == 0) {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-flags-10271", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR set but "
                             "the video session specified in videoSession was not created with either "
                             "VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR or "
                             "VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR.");
            } else {
                const auto *quantization_map_info =
                    vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR>(pCreateInfo->pNext);
                if (quantization_map_info) {
                    skip |= ValidateEncodeQuantizationMapParametersCreateInfo(
                        *vs_state, *quantization_map_info, device,
                        create_info_loc.pNext(Struct::VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR),
                        template_state.get());
                } else {
                    skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-flags-10272", device,
                                     create_info_loc.dot(Field::flags),
                                     "has VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR set but "
                                     "missing VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR from the pNext "
                                     "chain of pCreateInfo.");
                }
            }
        } else {
            if (template_state != nullptr &&
                (template_state->create_info.flags & VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR)) {
                const LogObjectList objlist(device, pCreateInfo->videoSessionParametersTemplate, pCreateInfo->videoSession);
                skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10277", objlist,
                                 create_info_loc.dot(Field::videoSessionParametersTemplate),
                                 "was created with VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR but "
                                 "pCreateInfo->flags (%s) does not include "
                                 "VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR.",
                                 string_VkVideoSessionCreateFlagsKHR(pCreateInfo->flags).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                                const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    auto vsp_state = Get<vvl::VideoSessionParameters>(videoSessionParameters);
    if (!vsp_state) return skip;

    const Location update_info_loc = error_obj.location.dot(Field::pUpdateInfo);

    auto vsp_data = vsp_state->Lock();

    if (pUpdateInfo->updateSequenceCount != vsp_data->update_sequence_counter + 1) {
        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215", device,
                         update_info_loc.dot(Field::updateSequenceCount), "(%" PRIu32 ") should be %" PRIu32 ".",
                         pUpdateInfo->updateSequenceCount, vsp_data->update_sequence_counter + 1);
    }

    switch (vsp_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateDecodeH264ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoDecodeH264SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH264SPS(add_info->pStdSPSs[i].seq_parameter_set_id)) {
                        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216",
                                         videoSessionParameters, error_obj.location,
                                         "H.264 SPS with key "
                                         "(SPS ID = %u) already exists in %s.",
                                         add_info->pStdSPSs[i].seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h264.sps.size() > vsp_data->h264.sps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.264 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h264.sps.size(), vsp_data->h264.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH264PPS(add_info->pStdPPSs[i].seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pic_parameter_set_id)) {
                        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218",
                                         videoSessionParameters, error_obj.location,
                                         "H.264 PPS with key "
                                         "(SPS ID = %u, PPS ID = %u) already exists in %s.",
                                         add_info->pStdPPSs[i].seq_parameter_set_id, add_info->pStdPPSs[i].pic_parameter_set_id,
                                         FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h264.pps.size() > vsp_data->h264.pps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.264 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h264.pps.size(), vsp_data->h264.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateDecodeH265ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoDecodeH265SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
                    if (vsp_data.GetH265VPS(add_info->pStdVPSs[i].vps_video_parameter_set_id)) {
                        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07220",
                                         videoSessionParameters, error_obj.location,
                                         "H.265 VPS with key "
                                         "(VPS ID = %u) already exists in %s.",
                                         add_info->pStdVPSs[i].vps_video_parameter_set_id,
                                         FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdVPSCount + vsp_data->h265.vps.size() > vsp_data->h265.vps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07221", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.265 VPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "VPS capacity (%u) the %s was created with.",
                                     add_info->stdVPSCount, vsp_data->h265.vps.size(), vsp_data->h265.vps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH265SPS(add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdSPSs[i].sps_seq_parameter_set_id)) {
                        skip |=
                            LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222", videoSessionParameters,
                                     error_obj.location,
                                     "H.265 SPS with key "
                                     "(VPS ID = %u, SPS ID = %u) already exists in %s.",
                                     add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                     add_info->pStdSPSs[i].sps_seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h265.sps.size() > vsp_data->h265.sps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.265 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h265.sps.size(), vsp_data->h265.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH265PPS(add_info->pStdPPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_pic_parameter_set_id)) {
                        skip |= LogError(
                            "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224", videoSessionParameters,
                            error_obj.location,
                            "H.265 PPS with key "
                            "(VPS ID = %u, SPS ID = %u, PPS ID = %u) already exists in %s.",
                            add_info->pStdPPSs[i].sps_video_parameter_set_id, add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                            add_info->pStdPPSs[i].pps_pic_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h265.pps.size() > vsp_data->h265.pps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.265 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h265.pps.size(), vsp_data->h265.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-09260", videoSessionParameters,
                             error_obj.location, "AV1 decode session parameters cannot be updated.");
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateEncodeH264ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH264SPS(add_info->pStdSPSs[i].seq_parameter_set_id)) {
                        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07226",
                                         videoSessionParameters, error_obj.location,
                                         "H.264 SPS with key "
                                         "(SPS ID = %u) already exists in %s.",
                                         add_info->pStdSPSs[i].seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h264.sps.size() > vsp_data->h264.sps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06441", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.264 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h264.sps.size(), vsp_data->h264.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH264PPS(add_info->pStdPPSs[i].seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pic_parameter_set_id)) {
                        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07227",
                                         videoSessionParameters, error_obj.location,
                                         "H.264 PPS with key "
                                         "(SPS ID = %u, PPS ID = %u) already exists in %s.",
                                         add_info->pStdPPSs[i].seq_parameter_set_id, add_info->pStdPPSs[i].pic_parameter_set_id,
                                         FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h264.pps.size() > vsp_data->h264.pps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06442", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.264 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h264.pps.size(), vsp_data->h264.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateEncodeH265ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoEncodeH265SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
                    if (vsp_data.GetH265VPS(add_info->pStdVPSs[i].vps_video_parameter_set_id)) {
                        skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07228",
                                         videoSessionParameters, error_obj.location,
                                         "H.265 VPS with key "
                                         "(VPS ID = %u) already exists in %s.",
                                         add_info->pStdVPSs[i].vps_video_parameter_set_id,
                                         FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdVPSCount + vsp_data->h265.vps.size() > vsp_data->h265.vps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06443", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.265 VPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "VPS capacity (%u) the %s was created with.",
                                     add_info->stdVPSCount, vsp_data->h265.vps.size(), vsp_data->h265.vps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH265SPS(add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdSPSs[i].sps_seq_parameter_set_id)) {
                        skip |=
                            LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07229", videoSessionParameters,
                                     error_obj.location,
                                     "H.265 SPS with key "
                                     "(VPS ID = %u, SPS ID = %u) already exists in %s.",
                                     add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                     add_info->pStdSPSs[i].sps_seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h265.sps.size() > vsp_data->h265.sps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06444", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.265 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h265.sps.size(), vsp_data->h265.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH265PPS(add_info->pStdPPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_pic_parameter_set_id)) {
                        skip |= LogError(
                            "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07230", videoSessionParameters,
                            error_obj.location,
                            "H.265 PPS with key "
                            "(VPS ID = %u, SPS ID = %u, PPS ID = %u) already exists in %s.",
                            add_info->pStdPPSs[i].sps_video_parameter_set_id, add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                            add_info->pStdPPSs[i].pps_pic_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h265.pps.size() > vsp_data->h265.pps_capacity) {
                    skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06445", videoSessionParameters,
                                     error_obj.location,
                                     "number of H.265 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h265.pps.size(), vsp_data->h265.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            skip |= LogError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-10281", videoSessionParameters,
                             error_obj.location, "AV1 encode session parameters cannot be updated.");
            break;
        }

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device,
                                                                 VkVideoSessionParametersKHR videoSessionParameters,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto video_session_parameters_state = Get<vvl::VideoSessionParameters>(videoSessionParameters)) {
        skip |= ValidateObjectNotInUse(video_session_parameters_state.get(), error_obj.location,
                                       "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR *pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR *pFeedbackInfo, size_t *pDataSize, void *pData,
    const ErrorObject &error_obj) const {
    bool skip = false;

    const auto vsp_state = Get<vvl::VideoSessionParameters>(pVideoSessionParametersInfo->videoSessionParameters);
    if (!vsp_state) return skip;

    const Location params_info_loc = error_obj.location.dot(Field::pVideoSessionParametersInfo);

    auto vsp_data = vsp_state->Lock();

    const char *pnext_msg = "chain does not contain a %s structure.";

    if (vsp_state->IsEncode()) {
        switch (vsp_state->GetCodecOp()) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                auto h264_info =
                    vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersGetInfoKHR>(pVideoSessionParametersInfo->pNext);
                if (h264_info) {
                    const Location h264_info_loc = params_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersGetInfoKHR);

                    if (!h264_info->writeStdSPS && !h264_info->writeStdPPS) {
                        skip |= LogError("VUID-VkVideoEncodeH264SessionParametersGetInfoKHR-writeStdSPS-08279",
                                         pVideoSessionParametersInfo->videoSessionParameters, h264_info_loc,
                                         "must have at least one of writeStdSPS and writeStdPPS set to VK_TRUE.");
                    }

                    if (h264_info->writeStdSPS && vsp_data.GetH264SPS(h264_info->stdSPSId) == nullptr) {
                        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263",
                                         pVideoSessionParametersInfo->videoSessionParameters, error_obj.location,
                                         "%s does not contain an H.264 SPS "
                                         "matching the stdSPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h264_info->stdSPSId, h264_info_loc.Fields().c_str());
                    }

                    if (h264_info->writeStdPPS && vsp_data.GetH264PPS(h264_info->stdSPSId, h264_info->stdPPSId) == nullptr) {
                        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264",
                                         pVideoSessionParametersInfo->videoSessionParameters, error_obj.location,
                                         "%s does not contain an H.264 PPS "
                                         "matching the stdSPSId (%u) and stdPPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h264_info->stdSPSId, h264_info->stdPPSId,
                                         h264_info_loc.Fields().c_str());
                    }
                } else {
                    skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08262",
                                     pVideoSessionParametersInfo->videoSessionParameters, params_info_loc.dot(Field::pNext),
                                     pnext_msg, "VkVideoEncodeH264SessionParametersGetInfoKHR");
                }
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                auto h265_info =
                    vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersGetInfoKHR>(pVideoSessionParametersInfo->pNext);
                if (h265_info) {
                    const Location h265_info_loc = params_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersGetInfoKHR);

                    if (!h265_info->writeStdVPS && !h265_info->writeStdSPS && !h265_info->writeStdPPS) {
                        skip |= LogError("VUID-VkVideoEncodeH265SessionParametersGetInfoKHR-writeStdVPS-08290",
                                         pVideoSessionParametersInfo->videoSessionParameters, h265_info_loc,
                                         "must have at least one of writeStdVPS, writeStdSPS, and writeStdPPS set to VK_TRUE.");
                    }

                    if (h265_info->writeStdVPS && vsp_data.GetH265VPS(h265_info->stdVPSId) == nullptr) {
                        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266",
                                         pVideoSessionParametersInfo->videoSessionParameters, error_obj.location,
                                         "%s does not contain an H.265 VPS "
                                         "matching the stdVPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h265_info->stdVPSId, h265_info_loc.Fields().c_str());
                    }

                    if (h265_info->writeStdSPS && vsp_data.GetH265SPS(h265_info->stdVPSId, h265_info->stdSPSId) == nullptr) {
                        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267",
                                         pVideoSessionParametersInfo->videoSessionParameters, error_obj.location,
                                         "%s does not contain an H.265 SPS "
                                         "matching the stdVPSId (%u) and stdSPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h265_info->stdVPSId, h265_info->stdSPSId,
                                         h265_info_loc.Fields().c_str());
                    }

                    if (h265_info->writeStdPPS &&
                        vsp_data.GetH265PPS(h265_info->stdVPSId, h265_info->stdSPSId, h265_info->stdPPSId) == nullptr) {
                        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268",
                                         pVideoSessionParametersInfo->videoSessionParameters, error_obj.location,
                                         "%s does not contain an H.265 PPS "
                                         "matching the stdVPSId(%u), stdSPSId (%u), and stdPPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h265_info->stdVPSId, h265_info->stdSPSId,
                                         h265_info->stdPPSId, h265_info_loc.Fields().c_str());
                    }
                } else {
                    skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08265",
                                     pVideoSessionParametersInfo->videoSessionParameters, params_info_loc.dot(Field::pNext),
                                     pnext_msg, "VkVideoEncodeH265SessionParametersGetInfoKHR");
                }
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                // Nothing to validate here for AV1 encode
                break;
            }

            default:
                assert(false);
                break;
        }
    } else {
        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08359",
                         pVideoSessionParametersInfo->videoSessionParameters, params_info_loc.dot(Field::videoSessionParameters),
                         "(%s) was not created with an encode operation.",
                         FormatHandle(pVideoSessionParametersInfo->videoSessionParameters).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (cb_state->activeQueries.size() > 0) {
        skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-None-07232", commandBuffer, error_obj.location.dot(Field::commandBuffer),
                         "%s has active queries.", FormatHandle(commandBuffer).c_str());
    }

    auto vs_state = Get<vvl::VideoSession>(pBeginInfo->videoSession);
    if (!vs_state) return skip;

    const Location begin_info_loc = error_obj.location.dot(Field::pBeginInfo);

    auto qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];

    if ((qf_ext_props.video_props.videoCodecOperations & vs_state->GetCodecOp()) == 0) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession, cb_state->command_pool->Handle());
        skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231", objlist, begin_info_loc.dot(Field::videoSession),
                         "%s does not support video codec operation %s "
                         "that %s specified in pBeginInfo->videoSession was created with.",
                         FormatHandle(cb_state->command_pool->Handle()).c_str(),
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()),
                         FormatHandle(pBeginInfo->videoSession).c_str());
    }

    if (vs_state->GetUnboundMemoryBindingCount() > 0) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07237", objlist, begin_info_loc.dot(Field::videoSession),
                         "%s has %u unbound memory binding indices.", FormatHandle(pBeginInfo->videoSession).c_str(),
                         vs_state->GetUnboundMemoryBindingCount());
    }

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state->unprotected == true) &&
        ((vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) != 0)) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233", objlist, error_obj.location.dot(Field::commandBuffer),
                         "%s is unprotected while %s was created with "
                         "VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR.",
                         FormatHandle(commandBuffer).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state->unprotected == false) &&
        ((vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) == 0)) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234", objlist, error_obj.location.dot(Field::commandBuffer),
                         "%s is protected while %s was created without "
                         "VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR.",
                         FormatHandle(commandBuffer).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    if (pBeginInfo->pReferenceSlots) {
        vvl::VideoPictureResources unique_resources{};
        bool resources_unique = true;
        bool has_separate_images = false;
        const vvl::Image *last_dpb_image = nullptr;

        for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
            const auto &slot = pBeginInfo->pReferenceSlots[i];
            const Location reference_slot_loc = begin_info_loc.dot(Field::pReferenceSlots, i);

            if (slot.slotIndex >= 0 && (uint32_t)slot.slotIndex >= vs_state->create_info.maxDpbSlots) {
                const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                skip |=
                    LogError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856", objlist, reference_slot_loc.dot(Field::slotIndex),
                             "(%d) is greater than the maxDpbSlots %s was created with.", slot.slotIndex,
                             FormatHandle(pBeginInfo->videoSession).c_str());
            }

            if (slot.pPictureResource != nullptr) {
                const Location reference_resource_loc = reference_slot_loc.dot(Field::pPictureResource);
                auto reference_resource = vvl::VideoPictureResource(*this, *slot.pPictureResource);
                skip |= ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state, reference_resource_loc,
                                                     "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242",
                                                     "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    if (last_dpb_image != nullptr && last_dpb_image != reference_resource.image_state.get()) {
                        has_separate_images = true;
                    }

                    const Location reference_image_view_loc = reference_resource_loc.dot(Field::imageViewBinding);
                    skip |= ValidateProtectedImage(*cb_state, *reference_resource.image_state, reference_image_view_loc,
                                                   "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235");
                    skip |= ValidateUnprotectedImage(*cb_state, *reference_resource.image_state, reference_image_view_loc,
                                                     "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236");

                    if (!IsImageCompatibleWithVideoSession(*reference_resource.image_state, *vs_state)) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->Handle(),
                                                    reference_resource.image_state->Handle());
                        skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240", objlist, reference_image_view_loc,
                                         "(%s created from %s) is not compatible with the video profile %s was created with.",
                                         FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    if (reference_resource.image_view_state->create_info.format != vs_state->create_info.referencePictureFormat) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->Handle(),
                                                    reference_resource.image_state->Handle());
                        skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241", objlist, reference_image_view_loc,
                                         "(%s created from %s) format (%s) "
                                         "does not match the referencePictureFormat (%s) %s was created with.",
                                         FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         string_VkFormat(reference_resource.image_view_state->create_info.format),
                                         string_VkFormat(vs_state->create_info.referencePictureFormat),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    auto supported_usage = reference_resource.image_view_state->inherited_usage;

                    if (vs_state->IsDecode() && (supported_usage & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) == 0) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->Handle(),
                                                    reference_resource.image_state->Handle());
                        skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245", objlist, reference_image_view_loc,
                                         "(%s created from %s) was not created "
                                         "with VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR thus it cannot be used as "
                                         "a reference picture with %s that was created with a decode operation.",
                                         FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    if (vs_state->IsEncode() && (supported_usage & VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) == 0) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->Handle(),
                                                    reference_resource.image_state->Handle());
                        skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07246", objlist, reference_image_view_loc,
                                         "(%s created from %s) was not created "
                                         "with VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR thus it cannot be used as "
                                         "a reference picture with %s that was created with an encode operation.",
                                         FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    last_dpb_image = reference_resource.image_state.get();
                }
            }
        }

        if (!resources_unique) {
            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
            skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238", objlist, error_obj.location,
                             "more than one element of pBeginInfo->pReferenceSlots "
                             "refers to the same video picture resource.");
        }

        auto supported_cap_flags = vs_state->profile->GetCapabilities().base.flags;
        if ((supported_cap_flags & VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR) == 0 && has_separate_images) {
            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
            skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-flags-07244", objlist, error_obj.location,
                             "not all elements of pBeginInfo->pReferenceSlots refer "
                             "to the same image and the video profile %s was created with does not support "
                             "VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR.",
                             FormatHandle(pBeginInfo->videoSession).c_str());
        }
    }

    auto vsp_state = Get<vvl::VideoSessionParameters>(pBeginInfo->videoSessionParameters);
    if (vsp_state && vsp_state->vs_state->VkHandle() != vs_state->VkHandle()) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSessionParameters, pBeginInfo->videoSession);
        skip |= LogError("VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857", objlist,
                         begin_info_loc.dot(Field::videoSessionParameters), "%s was not created for %s.",
                         FormatHandle(pBeginInfo->videoSessionParameters).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    const char *codec_op_requires_params_vuid = nullptr;
    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            if (!enabled_features.videoMaintenance2) {
                codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07247";
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            if (!enabled_features.videoMaintenance2) {
                codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07248";
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
            if (!enabled_features.videoMaintenance2) {
                codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-09261";
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07249";
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07250";
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-10283";
            break;

        default:
            break;
    }

    if ((codec_op_requires_params_vuid != nullptr) && (vsp_state == nullptr)) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError(codec_op_requires_params_vuid, objlist, begin_info_loc.dot(Field::videoSession),
                         "%s was created with %s but no video session parameters object was "
                         "specified in pBeginInfo->videoSessionParameters.",
                         FormatHandle(pBeginInfo->videoSession).c_str(),
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()));
    }

    if (vs_state->IsEncode()) {
        const auto rc_info = vku::FindStructInPNextChain<VkVideoEncodeRateControlInfoKHR>(pBeginInfo->pNext);
        if (rc_info != nullptr) {
            skip |= ValidateVideoEncodeRateControlInfo(*rc_info, pBeginInfo->pNext, commandBuffer, *vs_state, begin_info_loc);

            if (rc_info->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR &&
                rc_info->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
                switch (vs_state->GetCodecOp()) {
                    case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                        auto gop_info_h264 =
                            vku::FindStructInPNextChain<VkVideoEncodeH264GopRemainingFrameInfoKHR>(pBeginInfo->pNext);
                        if (vs_state->profile->GetCapabilities().encode_h264.requiresGopRemainingFrames &&
                            (gop_info_h264 == nullptr || gop_info_h264->useGopRemainingFrames == VK_FALSE)) {
                            const char *why = gop_info_h264 == nullptr
                                                  ? "there is no VkVideoEncodeH264GopRemainingFrameInfoKHR structure"
                                                  : "VkVideoEncodeH264GopRemainingFrameInfoKHR::useGopRemainingFrames is VK_FALSE";
                            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                            skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08255", objlist, error_obj.location,
                                             "requiresGopRemainingFrames is VK_TRUE for "
                                             "the H.264 encode profile %s was created with, but %s in the pNext "
                                             "chain of pBeginInfo.",
                                             FormatHandle(pBeginInfo->videoSession).c_str(), why);
                        }
                        break;
                    }

                    case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                        auto gop_info_h265 =
                            vku::FindStructInPNextChain<VkVideoEncodeH265GopRemainingFrameInfoKHR>(pBeginInfo->pNext);
                        if (vs_state->profile->GetCapabilities().encode_h265.requiresGopRemainingFrames &&
                            (gop_info_h265 == nullptr || gop_info_h265->useGopRemainingFrames == VK_FALSE)) {
                            const char *why = gop_info_h265 == nullptr
                                                  ? "there is no VkVideoEncodeH265GopRemainingFrameInfoKHR structure"
                                                  : "VkVideoEncodeH265GopRemainingFrameInfoKHR::useGopRemainingFrames is VK_FALSE";
                            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                            skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08256", objlist, error_obj.location,
                                             "requiresGopRemainingFrames is VK_TRUE for "
                                             "the H.265 encode profile %s was created with, but %s in the pNext "
                                             "chain of pBeginInfo.",
                                             FormatHandle(pBeginInfo->videoSession).c_str(), why);
                        }
                        break;
                    }

                    case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                        auto gop_info_av1 =
                            vku::FindStructInPNextChain<VkVideoEncodeAV1GopRemainingFrameInfoKHR>(pBeginInfo->pNext);
                        if (vs_state->profile->GetCapabilities().encode_av1.requiresGopRemainingFrames &&
                            (gop_info_av1 == nullptr || gop_info_av1->useGopRemainingFrames == VK_FALSE)) {
                            const char *why = gop_info_av1 == nullptr
                                                  ? "there is no VkVideoEncodeAV1GopRemainingFrameInfoKHR structure"
                                                  : "VkVideoEncodeAV1GopRemainingFrameInfoKHR::useGopRemainingFrames is VK_FALSE";
                            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                            skip |= LogError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-10282", objlist, error_obj.location,
                                             "requiresGopRemainingFrames is VK_TRUE for "
                                             "the AV1 encode profile %s was created with, but %s in the pNext "
                                             "chain of pBeginInfo.",
                                             FormatHandle(pBeginInfo->videoSession).c_str(), why);
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    auto vs_state = Get<vvl::VideoSession>(pBeginInfo->videoSession);
    if (!vs_state) return;

    const Location loc = record_obj.location;
    if (pBeginInfo->referenceSlotCount > 0) {
        std::vector<vvl::VideoReferenceSlot> expected_slots{};
        expected_slots.reserve(pBeginInfo->referenceSlotCount);

        for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
            if (pBeginInfo->pReferenceSlots[i].slotIndex >= 0) {
                expected_slots.emplace_back(*this, *vs_state->profile, pBeginInfo->pReferenceSlots[i], false);
            }
        }

        // Enqueue submission time validation of DPB slots
        cb_state->video_session_updates[vs_state->VkHandle()].emplace_back(
            [expected_slots, loc](const vvl::Device &dev_data, const vvl::VideoSession *vs_state,
                                  vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                if (!do_validate) return false;
                bool skip = false;
                for (const auto &slot : expected_slots) {
                    if (!dev_state.IsSlotActive(slot.index)) {
                        skip |= dev_data.LogError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239", vs_state->Handle(), loc,
                                                  "DPB slot index %d is not active in %s.", slot.index,
                                                  dev_data.FormatHandle(*vs_state).c_str());
                    } else if (slot.resource && !dev_state.IsSlotPicture(slot.index, slot.resource)) {
                        skip |= dev_data.LogError("VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265", vs_state->Handle(), loc,
                                                  "DPB slot index %d of %s is not currently associated with the specified "
                                                  "video picture resource: %s, layer %u, offset (%s), extent (%s).",
                                                  slot.index, dev_data.FormatHandle(*vs_state).c_str(),
                                                  dev_data.FormatHandle(slot.resource.image_state->Handle()).c_str(),
                                                  slot.resource.range.baseArrayLayer,
                                                  string_VkOffset2D(slot.resource.coded_offset).c_str(),
                                                  string_VkExtent2D(slot.resource.coded_extent).c_str());
                    }
                }
                return skip;
            });
    }

    if (vs_state->IsEncode()) {
        vku::safe_VkVideoBeginCodingInfoKHR begin_info(pBeginInfo);

        // Enqueue submission time validation of rate control state
        cb_state->video_session_updates[vs_state->VkHandle()].emplace_back(
            [begin_info, loc](const vvl::Device &dev_data, const vvl::VideoSession *vs_state,
                              vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                if (!do_validate) return false;
                return dev_state.ValidateRateControlState(dev_data, vs_state, begin_info, loc);
            });
    }
}

bool CoreChecks::PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (cb_state->activeQueries.size() > 0) {
        skip |= LogError("VUID-vkCmdEndVideoCodingKHR-None-07251", commandBuffer, error_obj.location.dot(Field::commandBuffer),
                         "%s has active queries.", FormatHandle(commandBuffer).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                         const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const Location control_info_loc = error_obj.location.dot(Field::pCodingControlInfo);

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    const char *flags_pnext_msg = "has %s set but missing %s from the pNext chain of pCodingControlInfo.";
    const char *flags_require_encode_msg = "has %s set but %s is not a video encode session.";

    if (pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR) {
        if (vs_state->IsEncode()) {
            const auto rc_info = vku::FindStructInPNextChain<VkVideoEncodeRateControlInfoKHR>(pCodingControlInfo->pNext);
            if (rc_info != nullptr) {
                skip |= ValidateVideoEncodeRateControlInfo(*rc_info, pCodingControlInfo->pNext, commandBuffer, *vs_state,
                                                           control_info_loc);
            } else {
                skip |= LogError("VUID-VkVideoCodingControlInfoKHR-flags-07018", commandBuffer, control_info_loc.dot(Field::flags),
                                 flags_pnext_msg, "VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR",
                                 "VkVideoEncodeRateControlInfoKHR");
            }
        } else {
            const LogObjectList objlist(commandBuffer, vs_state->Handle());
            skip |= LogError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243", objlist,
                             control_info_loc.dot(Field::flags), flags_require_encode_msg,
                             "VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR", FormatHandle(*vs_state).c_str());
        }
    }

    if (pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR) {
        if (vs_state->IsEncode()) {
            const auto quality_level_info =
                vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(pCodingControlInfo->pNext);
            if (quality_level_info != nullptr) {
                if (quality_level_info->qualityLevel >= profile_caps.encode.maxQualityLevels) {
                    const LogObjectList objlist(commandBuffer, vs_state->Handle());
                    skip |= LogError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311", objlist,
                                     control_info_loc.pNext(Struct::VkVideoEncodeQualityLevelInfoKHR, Field::qualityLevel),
                                     "(%u) must be smaller than the maxQualityLevels (%u) supported by the video "
                                     "profile %s was created with.",
                                     quality_level_info->qualityLevel, profile_caps.encode.maxQualityLevels,
                                     FormatHandle(*vs_state).c_str());
                }
            } else {
                skip |= LogError("VUID-VkVideoCodingControlInfoKHR-flags-08349", commandBuffer, control_info_loc.dot(Field::flags),
                                 flags_pnext_msg, "VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR",
                                 "VkVideoEncodeQualityLevelInfoKHR");
            }
        } else {
            const LogObjectList objlist(commandBuffer, vs_state->Handle());
            skip |= LogError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243", objlist,
                             control_info_loc.dot(Field::flags), flags_require_encode_msg,
                             "VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR", FormatHandle(*vs_state).c_str());
        }
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                       const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    if ((pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR) == 0) {
        EnqueueVerifyVideoSessionInitialized(*cb_state, *vs_state, record_obj.location,
                                             "VUID-vkCmdControlVideoCodingKHR-flags-07017");
    }
}

bool CoreChecks::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const Location decode_info_loc = error_obj.location.dot(Field::pDecodeInfo);

    if (!vs_state->IsDecode()) {
        const LogObjectList objlist(commandBuffer, vs_state->Handle());
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-None-08249", objlist, error_obj.location,
                         "the video codec operation (%s) the bound video session %s "
                         "was created with is not a decode operation.",
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()), FormatHandle(*vs_state).c_str());
        return skip;
    }

    const auto &bound_resources = cb_state->bound_video_picture_resources;

    bool hit_error = false;

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    if (auto buffer_state = Get<vvl::Buffer>(pDecodeInfo->srcBuffer)) {
        skip |= ValidateProtectedBuffer(*cb_state, *buffer_state, decode_info_loc.dot(Field::srcBuffer),
                                        "VUID-vkCmdDecodeVideoKHR-commandBuffer-07136");

        if ((buffer_state->usage & VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pDecodeInfo->srcBuffer);
            skip |= LogError("VUID-VkVideoDecodeInfoKHR-srcBuffer-07165", objlist, decode_info_loc.dot(Field::srcBuffer),
                             "(%s) was not created with "
                             "VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR.",
                             FormatHandle(pDecodeInfo->srcBuffer).c_str());
        }

        if (!IsBufferCompatibleWithVideoSession(*buffer_state, *vs_state)) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pDecodeInfo->srcBuffer);
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135", objlist, decode_info_loc.dot(Field::srcBuffer),
                             "(%s) is not compatible "
                             "with the video profile %s was created with.",
                             FormatHandle(pDecodeInfo->srcBuffer).c_str(), FormatHandle(*vs_state).c_str());
        }

        if (pDecodeInfo->srcBufferOffset >= buffer_state->create_info.size) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pDecodeInfo->srcBuffer);
            skip |= LogError(
                "VUID-VkVideoDecodeInfoKHR-srcBufferOffset-07166", objlist, decode_info_loc.dot(Field::srcBufferOffset),
                "(%" PRIu64 ") must be less than the size (%" PRIu64 ") of pDecodeInfo->srcBuffer (%s).",
                pDecodeInfo->srcBufferOffset, buffer_state->create_info.size, FormatHandle(pDecodeInfo->srcBuffer).c_str());
        }

        if (!IsIntegerMultipleOf(pDecodeInfo->srcBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment)) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle());
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138", objlist, decode_info_loc.dot(Field::srcBufferOffset),
                             "(%" PRIu64 ") is not an integer multiple of the minBitstreamBufferOffsetAlignment (%" PRIu64
                             ") required by the video profile %s was created with.",
                             pDecodeInfo->srcBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment,
                             FormatHandle(*vs_state).c_str());
        }

        if (pDecodeInfo->srcBufferOffset + pDecodeInfo->srcBufferRange > buffer_state->create_info.size) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pDecodeInfo->srcBuffer);
            skip |= LogError("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167", objlist, decode_info_loc.dot(Field::srcBufferOffset),
                             "(%" PRIu64 ") plus pDecodeInfo->srcBufferRange (%" PRIu64
                             ") must be less than or equal to the size (%" PRIu64 ") of pDecodeInfo->srcBuffer (%s).",
                             pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange, buffer_state->create_info.size,
                             FormatHandle(pDecodeInfo->srcBuffer).c_str());
        }
    }

    if (!IsIntegerMultipleOf(pDecodeInfo->srcBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment)) {
        const LogObjectList objlist(commandBuffer, vs_state->Handle());
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139", objlist, decode_info_loc.dot(Field::srcBufferRange),
                         "(%" PRIu64 ") is not an integer multiple of the minBitstreamBufferSizeAlignment (%" PRIu64
                         ") required by the video profile %s was created with.",
                         pDecodeInfo->srcBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment,
                         FormatHandle(*vs_state).c_str());
    }

    if (vs_state->create_info.maxDpbSlots > 0 && pDecodeInfo->pSetupReferenceSlot == nullptr) {
        const LogObjectList objlist(commandBuffer, vs_state->Handle());
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-08376", objlist, decode_info_loc.dot(Field::pSetupReferenceSlot),
                         "is NULL but the bound video session %s was created with maxDpbSlot %u.", FormatHandle(*vs_state).c_str(),
                         vs_state->create_info.maxDpbSlots);
    }

    vvl::VideoPictureResource setup_resource;
    if (pDecodeInfo->pSetupReferenceSlot) {
        if (pDecodeInfo->pSetupReferenceSlot->slotIndex < 0) {
            skip |= LogError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07168", commandBuffer,
                             decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::slotIndex), "(%d) must not be negative.",
                             pDecodeInfo->pSetupReferenceSlot->slotIndex);
        } else if ((uint32_t)pDecodeInfo->pSetupReferenceSlot->slotIndex >= vs_state->create_info.maxDpbSlots) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07170", commandBuffer,
                             decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::slotIndex),
                             "(%d) must be smaller than the maxDpbSlots (%u) the bound video session %s "
                             "was created with.",
                             pDecodeInfo->pSetupReferenceSlot->slotIndex, vs_state->create_info.maxDpbSlots,
                             FormatHandle(*vs_state).c_str());
        }

        if (pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
            setup_resource = vvl::VideoPictureResource(*this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
            if (setup_resource) {
                skip |= ValidateVideoPictureResource(setup_resource, commandBuffer, *vs_state,
                                                     decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                                     "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");

                if (bound_resources.find(setup_resource) == bound_resources.end()) {
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149", commandBuffer,
                                     decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                     "is not one of the bound video picture resources.");
                }

                skip |= VerifyImageLayout(*cb_state, *setup_resource.image_state, setup_resource.range,
                                          VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, error_obj.location,
                                          "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254", &hit_error);
            }
        } else {
            skip |= LogError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169", commandBuffer,
                             decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource), "is NULL");
        }
    }

    auto dst_resource = vvl::VideoPictureResource(*this, pDecodeInfo->dstPictureResource);
    const Location dst_resource_loc = decode_info_loc.dot(Field::dstPictureResource);
    skip |=
        ValidateVideoPictureResource(dst_resource, commandBuffer, *vs_state, dst_resource_loc,
                                     "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144", "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    if (dst_resource) {
        const Location dst_image_view_loc = dst_resource_loc.dot(Field::imageViewBinding);
        skip |= ValidateProtectedImage(*cb_state, *dst_resource.image_state, dst_image_view_loc,
                                       "VUID-vkCmdDecodeVideoKHR-commandBuffer-07147");
        skip |= ValidateUnprotectedImage(*cb_state, *dst_resource.image_state, dst_image_view_loc,
                                         "VUID-vkCmdDecodeVideoKHR-commandBuffer-07148");

        if (!IsImageCompatibleWithVideoSession(*dst_resource.image_state, *vs_state)) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), dst_resource.image_view_state->Handle(),
                                        dst_resource.image_state->Handle());
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142", objlist, dst_image_view_loc,
                             "(%s created from %s) is not compatible with the video profile the bound "
                             "video session %s was created with.",
                             FormatHandle(pDecodeInfo->dstPictureResource.imageViewBinding).c_str(),
                             FormatHandle(dst_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        if (dst_resource.image_view_state->create_info.format != vs_state->create_info.pictureFormat) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), dst_resource.image_view_state->Handle(),
                                        dst_resource.image_state->Handle());
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143", objlist, dst_image_view_loc,
                             "(%s created from %s) format (%s) does not match the pictureFormat (%s) "
                             "the bound video session %s was created with.",
                             FormatHandle(dst_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(dst_resource.image_state->Handle()).c_str(),
                             string_VkFormat(dst_resource.image_view_state->create_info.format),
                             string_VkFormat(vs_state->create_info.pictureFormat), FormatHandle(vs_state->Handle()).c_str());
        }

        auto supported_usage = dst_resource.image_view_state->inherited_usage;
        if ((supported_usage & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), dst_resource.image_view_state->Handle(),
                                        dst_resource.image_state->Handle());
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146", objlist, dst_image_view_loc,
                             "(%s created from %s) was not created with VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR "
                             "thus it cannot be used as a decode output picture with the bound video session %s "
                             "that was created with a decode operation.",
                             FormatHandle(dst_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(dst_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        bool dst_same_as_setup = (setup_resource == dst_resource);

        VkImageLayout expected_layout =
            dst_same_as_setup ? VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR : VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;

        const char *vuid =
            dst_same_as_setup ? "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253" : "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07252";

        skip |= VerifyImageLayout(*cb_state, *dst_resource.image_state, dst_resource.range, expected_layout, error_obj.location,
                                  vuid, &hit_error);

        if (setup_resource) {
            if ((profile_caps.decode.flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR) == 0 &&
                dst_same_as_setup) {
                const LogObjectList objlist(commandBuffer, vs_state->Handle());
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140", objlist, error_obj.location,
                                 "the video profile %s was created with does not support "
                                 "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR but "
                                 "pDecodeInfo->dstPictureResource and "
                                 "pDecodeInfo->pSetupReferenceSlot->pPictureResource match.",
                                 FormatHandle(*vs_state).c_str());
            }

            if (!dst_same_as_setup) {
                skip |= ValidateDecodeDistinctOutput(*cb_state, *pDecodeInfo, decode_info_loc);
            }
        }
    }

    if (pDecodeInfo->pReferenceSlots) {
        vvl::VideoPictureResources unique_resources{};
        bool resources_unique = true;

        skip |= ValidateActiveReferencePictureCount(*cb_state, *pDecodeInfo, error_obj.location);
        skip |= ValidateReferencePictureUseCount(*cb_state, *pDecodeInfo, error_obj.location);

        for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
            if (pDecodeInfo->pReferenceSlots[i].slotIndex < 0) {
                skip |= LogError("VUID-VkVideoDecodeInfoKHR-slotIndex-07171", commandBuffer,
                                 decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::slotIndex), "(%d) must not be negative.",
                                 pDecodeInfo->pReferenceSlots[i].slotIndex);
            } else if ((uint32_t)pDecodeInfo->pReferenceSlots[i].slotIndex >= vs_state->create_info.maxDpbSlots) {
                const LogObjectList objlist(commandBuffer, vs_state->Handle());
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-slotIndex-07256", objlist,
                                 decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::slotIndex),
                                 "(%d) must be smaller than the maxDpbSlots (%u) the bound video session %s "
                                 "was created with.",
                                 pDecodeInfo->pReferenceSlots[i].slotIndex, vs_state->create_info.maxDpbSlots,
                                 FormatHandle(*vs_state).c_str());
            }

            if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
                auto reference_resource = vvl::VideoPictureResource(*this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    const auto &it = bound_resources.find(reference_resource);
                    if (it == bound_resources.end()) {
                        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151", commandBuffer, error_obj.location,
                                         "the video picture resource specified in "
                                         "pDecodeInfo->pReferenceSlots[%u].pPictureResource is not one of the "
                                         "bound video picture resources.",
                                         i);
                    } else if (pDecodeInfo->pReferenceSlots[i].slotIndex >= 0 &&
                               pDecodeInfo->pReferenceSlots[i].slotIndex != it->second) {
                        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151", commandBuffer, error_obj.location,
                                         "the bound video picture resource specified  in "
                                         "pDecodeInfo->pReferenceSlots[%u].pPictureResource is not currently "
                                         "associated with the DPB slot index specifed in "
                                         "pDecodeInfo->pReferenceSlots[%u].slotIndex (%d).",
                                         i, i, pDecodeInfo->pReferenceSlots[i].slotIndex);
                    }

                    skip |=
                        ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state,
                                                     decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                                     "VUID-vkCmdDecodeVideoKHR-codedOffset-07257");

                    skip |= VerifyImageLayout(*cb_state, *reference_resource.image_state, reference_resource.range,
                                              VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, error_obj.location,
                                              "VUID-vkCmdDecodeVideoKHR-pPictureResource-07255", &hit_error);
                }
            } else {
                skip |= LogError("VUID-VkVideoDecodeInfoKHR-pPictureResource-07172", commandBuffer,
                                 decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource), "is NULL.");
            }
        }

        if (!resources_unique) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264", commandBuffer, error_obj.location,
                             "more than one element of pDecodeInfo->pReferenceSlots "
                             "refers to the same video picture resource.");
        }
    }

    uint32_t op_count = vs_state->GetVideoDecodeOperationCount(pDecodeInfo);
    for (const auto &query : cb_state->activeQueries) {
        if (query.active_query_index + op_count > query.last_activatable_query_index + 1) {
            auto query_pool_state = Get<vvl::QueryPool>(query.pool);
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-opCount-07134", commandBuffer, error_obj.location,
                             "not enough activatable queries for query type %s "
                             "with opCount %u, active query index %u, and last activatable query index %u.",
                             string_VkQueryType(query_pool_state->create_info.queryType), op_count, query.active_query_index,
                             query.last_activatable_query_index);
        }
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pDecodeInfo->pNext);
        if (inline_query_info != nullptr) {
            if (auto query_pool_state = Get<vvl::QueryPool>(inline_query_info->queryPool)) {
                skip |= ValidateVideoInlineQueryInfo(*query_pool_state, *inline_query_info,
                                                     decode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR));

                if (inline_query_info->queryCount != op_count) {
                    const LogObjectList objlist(commandBuffer, vs_state->Handle());
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-08365", objlist,
                                     decode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR, Field::queryCount),
                                     "(%u) is not equal to opCount (%u).", inline_query_info->queryCount, op_count);
                }

                if (query_pool_state->create_info.queryType != VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-queryType-08367", objlist,
                                     decode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR, Field::queryPool),
                                     "(%s) has query type (%s) but must be VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR.",
                                     FormatHandle(*query_pool_state).c_str(),
                                     string_VkQueryType(query_pool_state->create_info.queryType));
                }

                if (vs_state->profile != query_pool_state->supported_video_profile) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool, vs_state->Handle());
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-queryPool-08368", objlist,
                                     decode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR, Field::queryPool),
                                     "video profile %s that was created with does not "
                                     "match the video profile of the bound video session %s.",
                                     FormatHandle(*query_pool_state).c_str(), FormatHandle(*vs_state).c_str());
                }

                const auto &qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];
                if (!qf_ext_props.query_result_status_props.queryResultStatusSupport) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError("VUID-vkCmdDecodeVideoKHR-queryType-08369", objlist, error_obj.location,
                                     "the command pool's queue family (index %u) the command "
                                     "buffer %s was allocated from does not support result status queries.",
                                     cb_state->command_pool->queueFamilyIndex, FormatHandle(*cb_state).c_str());
                }
            }
        }
    }

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            skip |= ValidateVideoDecodeInfoH264(*cb_state, *pDecodeInfo, decode_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            skip |= ValidateVideoDecodeInfoH265(*cb_state, *pDecodeInfo, decode_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
            skip |= ValidateVideoDecodeInfoAV1(*cb_state, *pDecodeInfo, decode_info_loc);
            break;

        default:
            break;
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    const Location loc = record_obj.location;
    EnqueueVerifyVideoSessionInitialized(*cb_state, *vs_state, loc, "VUID-vkCmdDecodeVideoKHR-None-07011");

    if (vs_state->GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        std::vector<vvl::VideoReferenceSlot> reference_slots{};
        reference_slots.reserve(pDecodeInfo->referenceSlotCount);
        for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
            reference_slots.emplace_back(*this, *vs_state->profile, pDecodeInfo->pReferenceSlots[i]);
        }

        // Enqueue submission time validation of picture kind (frame, top field, bottom field) for H.264
        cb_state->video_session_updates[vs_state->VkHandle()].emplace_back(
            [reference_slots, loc](const vvl::Device &dev_data, const vvl::VideoSession *vs_state,
                                   vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                if (!do_validate) return false;
                bool skip = false;
                const auto log_picture_kind_error = [&](const vvl::VideoReferenceSlot &slot, const char *vuid,
                                                        const char *picture_kind) -> bool {
                    return dev_data.LogError(vuid, vs_state->Handle(), loc,
                                             "DPB slot index %d of %s does not currently contain a %s with the specified "
                                             "video picture resource: %s, layer %u, offset (%s), extent (%s).",
                                             slot.index, dev_data.FormatHandle(*vs_state).c_str(), picture_kind,
                                             dev_data.FormatHandle(slot.resource.image_state->Handle()).c_str(),
                                             slot.resource.range.baseArrayLayer,
                                             string_VkOffset2D(slot.resource.coded_offset).c_str(),
                                             string_VkExtent2D(slot.resource.coded_extent).c_str());
                };
                for (const auto &slot : reference_slots) {
                    if (slot.picture_id.IsFrame() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::Frame(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266", "frame");
                    }
                    if (slot.picture_id.ContainsTopField() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::TopField(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267", "top field");
                    }
                    if (slot.picture_id.ContainsBottomField() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::BottomField(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268", "bottom field");
                    }
                }
                return skip;
            });
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pDecodeInfo->pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            EnqueueVerifyVideoInlineQueryUnavailable(*cb_state, *inline_query_info, Func::vkCmdDecodeVideoKHR);
        }
    }
}

bool CoreChecks::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const Location encode_info_loc = error_obj.location.dot(Field::pEncodeInfo);

    if (!vs_state->IsEncode()) {
        const LogObjectList objlist(commandBuffer, vs_state->Handle());
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-None-08250", objlist, error_obj.location,
                         "the video codec operation (%s) the bound video session %s "
                         "was created with is not an encode operation.",
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()), FormatHandle(*vs_state).c_str());
        return skip;
    }

    const auto vsp_state = cb_state->bound_video_session_parameters.get();
    if (vs_state->IsEncode() && vsp_state && cb_state->video_encode_quality_level.has_value()) {
        // If we already know the current encode quality level already at command buffer recording
        // time, because it was set in this command buffer, then we do command buffer recording time
        // validation for matching parameters object encode quality level
        if (vsp_state->GetEncodeQualityLevel() != cb_state->video_encode_quality_level.value()) {
            const LogObjectList objlist(vs_state->Handle(), vsp_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-None-08318", objlist, error_obj.location,
                             "the currently configured encode quality level (%u) for %s "
                             "does not match the encode quality level (%u) %s was created with.",
                             cb_state->video_encode_quality_level.value(), FormatHandle(*vs_state).c_str(),
                             vsp_state->GetEncodeQualityLevel(), FormatHandle(*vsp_state).c_str());
        }
    }

    const auto *quantization_map_info = vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapInfoKHR>(pEncodeInfo->pNext);

    struct QuantizationMapInfoValidUsages {
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_flag;
        VkImageUsageFlagBits image_usage_flag;
        const char *session_mismatch_vuid;
        const char *image_view_mismatch_vuid;
    };
    QuantizationMapInfoValidUsages quantization_map_info_valid_usages[] = {
        {
            VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
            VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
            VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
            "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10306",
            "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10311",
        },
        {
            VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
            VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
            VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
            "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10307",
            "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10312",
        },
    };

    for (auto &valid_usage : quantization_map_info_valid_usages) {
        if ((pEncodeInfo->flags & valid_usage.encode_flag) == 0) continue;

        if ((vs_state->create_info.flags & valid_usage.session_flag) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle());
            skip |= LogError(valid_usage.session_mismatch_vuid, objlist, encode_info_loc.dot(Field::flags),
                             "contains %s but the bound video session %s was not created with %s.",
                             string_VkVideoEncodeFlagBitsKHR(valid_usage.encode_flag), FormatHandle(*vs_state).c_str(),
                             string_VkVideoSessionCreateFlagBitsKHR(valid_usage.session_flag));
        }

        if (quantization_map_info != nullptr && quantization_map_info->quantizationMap != VK_NULL_HANDLE) {
            const auto iv_state = Get<vvl::ImageView>(quantization_map_info->quantizationMap);
            if (!iv_state) return false;

            if ((iv_state->inherited_usage & valid_usage.image_usage_flag) == 0) {
                const LogObjectList objlist(commandBuffer, iv_state->Handle());
                skip |= LogError(
                    valid_usage.image_view_mismatch_vuid, objlist, encode_info_loc.dot(Field::flags),
                    "contains %s but the image view specified in %s (%s created from %s) was not created with %s.",
                    string_VkVideoEncodeFlagBitsKHR(valid_usage.encode_flag),
                    encode_info_loc.pNext(Struct::VkVideoEncodeQuantizationMapInfoKHR, Field::quantizationMap).Fields().c_str(),
                    FormatHandle(*iv_state).c_str(), FormatHandle(*iv_state->image_state).c_str(),
                    string_VkImageUsageFlagBits(valid_usage.image_usage_flag));
            }
        }
    }

    bool hit_error = false;

    if (pEncodeInfo->flags & (VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR)) {
        if (quantization_map_info != nullptr && quantization_map_info->quantizationMap != VK_NULL_HANDLE) {
            if (vsp_state &&
                (vsp_state->create_info.flags & VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR) == 0) {
                const LogObjectList objlist(commandBuffer, vsp_state->Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-10315", objlist,
                                 encode_info_loc.pNext(Struct::VkVideoEncodeQuantizationMapInfoKHR, Field::quantizationMap),
                                 "is not VK_NULL_HANDLE but the bound video session parameters (%s) was not created with "
                                 "VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR.",
                                 FormatHandle(*vsp_state).c_str());
            }

            skip |= ValidateVideoEncodeQuantizationMapInfo(*cb_state, pEncodeInfo->srcPictureResource.codedExtent,
                                                           *quantization_map_info,
                                                           encode_info_loc.pNext(Struct::VkVideoEncodeQuantizationMapInfoKHR));
        } else {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309", commandBuffer, encode_info_loc.dot(Field::flags),
                             "(%s) contains VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR or "
                             "VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR but the pNext chain of pEncodeInfo "
                             "does not contain a VkVideoEncodeQuantizationMapInfoKHR structure specifying a "
                             "valid image view in its quantizationMap member.",
                             string_VkVideoEncodeFlagsKHR(pEncodeInfo->flags).c_str());
        }
    }

    if (pEncodeInfo->flags & VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
        const auto rc_mode = cb_state->video_encode_rate_control_state.base.rateControlMode;
        if (rc_mode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR ||
            rc_mode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308", objlist, encode_info_loc.dot(Field::flags),
                             "contains VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR but the current rate control mode is %s.",
                             string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_mode));
        }
    }

    const auto &bound_resources = cb_state->bound_video_picture_resources;

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    if (auto buffer_state = Get<vvl::Buffer>(pEncodeInfo->dstBuffer)) {
        skip |= ValidateProtectedBuffer(*cb_state, *buffer_state, encode_info_loc.dot(Field::dstBuffer),
                                        "VUID-vkCmdEncodeVideoKHR-commandBuffer-08202");
        skip |= ValidateUnprotectedBuffer(*cb_state, *buffer_state, encode_info_loc.dot(Field::dstBuffer),
                                          "VUID-vkCmdEncodeVideoKHR-commandBuffer-08203");

        if ((buffer_state->usage & VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pEncodeInfo->dstBuffer);
            skip |= LogError("VUID-VkVideoEncodeInfoKHR-dstBuffer-08236", objlist, encode_info_loc.dot(Field::dstBuffer),
                             "(%s) was not created with "
                             "VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR.",
                             FormatHandle(pEncodeInfo->dstBuffer).c_str());
        }

        if (!IsBufferCompatibleWithVideoSession(*buffer_state, *vs_state)) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pEncodeInfo->dstBuffer);
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08201", objlist, encode_info_loc.dot(Field::dstBuffer),
                             "(%s) is not compatible "
                             "with the video profile %s was created with.",
                             FormatHandle(pEncodeInfo->dstBuffer).c_str(), FormatHandle(*vs_state).c_str());
        }

        if (pEncodeInfo->dstBufferOffset >= buffer_state->create_info.size) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pEncodeInfo->dstBuffer);
            skip |= LogError(
                "VUID-VkVideoEncodeInfoKHR-dstBufferOffset-08237", objlist, encode_info_loc.dot(Field::dstBufferOffset),
                "(%" PRIu64 ") must be less than the size (%" PRIu64 ") of pEncodeInfo->dstBuffer (%s).",
                pEncodeInfo->dstBufferOffset, buffer_state->create_info.size, FormatHandle(pEncodeInfo->dstBuffer).c_str());
        }

        if (!IsIntegerMultipleOf(pEncodeInfo->dstBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment)) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08204", objlist, encode_info_loc.dot(Field::dstBufferOffset),
                             "(%" PRIu64 ") is not an integer multiple of the minBitstreamBufferOffsetAlignment (%" PRIu64
                             ") required by the video profile %s was created with.",
                             pEncodeInfo->dstBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment,
                             FormatHandle(*vs_state).c_str());
        }

        if (pEncodeInfo->dstBufferOffset + pEncodeInfo->dstBufferRange > buffer_state->create_info.size) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), pEncodeInfo->dstBuffer);
            skip |= LogError("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238", objlist, encode_info_loc.dot(Field::dstBufferOffset),
                             "(%" PRIu64 ") plus pEncodeInfo->dstBufferRange (%" PRIu64
                             ") must be less than or equal to the size (%" PRIu64 ") of pEncodeInfo->dstBuffer (%s).",
                             pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange, buffer_state->create_info.size,
                             FormatHandle(pEncodeInfo->dstBuffer).c_str());
        }
    }

    if (!IsIntegerMultipleOf(pEncodeInfo->dstBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment)) {
        const LogObjectList objlist(commandBuffer, vs_state->Handle());
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08205", objlist, encode_info_loc.dot(Field::dstBufferRange),
                         "(%" PRIu64 ") is not an integer multiple of the minBitstreamBufferSizeAlignment (%" PRIu64
                         ") required by the video profile %s was created with.",
                         pEncodeInfo->dstBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment,
                         FormatHandle(*vs_state).c_str());
    }

    if (vs_state->create_info.maxDpbSlots > 0 && pEncodeInfo->pSetupReferenceSlot == nullptr) {
        const LogObjectList objlist(commandBuffer, vs_state->Handle());
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08377", objlist, encode_info_loc.dot(Field::pSetupReferenceSlot),
                         "is NULL but the bound video session %s was created with maxDpbSlot %u.", FormatHandle(*vs_state).c_str(),
                         vs_state->create_info.maxDpbSlots);
    }

    vvl::VideoPictureResource setup_resource;
    if (pEncodeInfo->pSetupReferenceSlot) {
        if (pEncodeInfo->pSetupReferenceSlot->slotIndex < 0) {
            skip |= LogError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08239", commandBuffer,
                             encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::slotIndex), "(%d) must not be negative.",
                             pEncodeInfo->pSetupReferenceSlot->slotIndex);
        } else if ((uint32_t)pEncodeInfo->pSetupReferenceSlot->slotIndex >= vs_state->create_info.maxDpbSlots) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08213", commandBuffer,
                             encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::slotIndex),
                             "(%d) must be smaller than the maxDpbSlots (%u) the bound video session %s "
                             "was created with.",
                             pEncodeInfo->pSetupReferenceSlot->slotIndex, vs_state->create_info.maxDpbSlots,
                             FormatHandle(*vs_state).c_str());
        }

        if (pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
            setup_resource = vvl::VideoPictureResource(*this, *pEncodeInfo->pSetupReferenceSlot->pPictureResource);
            if (setup_resource) {
                skip |= ValidateVideoPictureResource(setup_resource, commandBuffer, *vs_state,
                                                     encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                                     "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08214");

                if (bound_resources.find(setup_resource) == bound_resources.end()) {
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215", commandBuffer, error_obj.location,
                                     "the video picture resource specified in "
                                     "pEncodeInfo->pSetupReferenceSlot->pPictureResource is not one of the "
                                     "bound video picture resources.");
                }

                skip |= VerifyImageLayout(*cb_state, *setup_resource.image_state, setup_resource.range,
                                          VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, error_obj.location,
                                          "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08223", &hit_error);
            }
        } else {
            skip |= LogError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08240", commandBuffer,
                             encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource), "is NULL.");
        }
    }

    auto src_resource = vvl::VideoPictureResource(*this, pEncodeInfo->srcPictureResource);
    const Location src_resource_loc = encode_info_loc.dot(Field::srcPictureResource);
    skip |=
        ValidateVideoPictureResource(src_resource, commandBuffer, *vs_state, src_resource_loc,
                                     "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08208", "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08209");
    if (src_resource) {
        const Location src_image_view_loc = src_resource_loc.dot(Field::imageViewBinding);
        skip |= ValidateProtectedImage(*cb_state, *src_resource.image_state, src_image_view_loc,
                                       "VUID-vkCmdEncodeVideoKHR-commandBuffer-08211");

        if (!IsImageCompatibleWithVideoSession(*src_resource.image_state, *vs_state)) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), src_resource.image_view_state->Handle(),
                                        src_resource.image_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08206", objlist, src_image_view_loc,
                             "(%s created from %s) is not compatible with the video profile the bound "
                             "video session %s was created with.",
                             FormatHandle(pEncodeInfo->srcPictureResource.imageViewBinding).c_str(),
                             FormatHandle(src_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        if (src_resource.image_view_state->create_info.format != vs_state->create_info.pictureFormat) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), src_resource.image_view_state->Handle(),
                                        src_resource.image_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08207", objlist, src_image_view_loc,
                             "(%s created from %s) format (%s) does not match the pictureFormat (%s) "
                             "the bound video session %s was created with.",
                             FormatHandle(src_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(src_resource.image_state->Handle()).c_str(),
                             string_VkFormat(src_resource.image_view_state->create_info.format),
                             string_VkFormat(vs_state->create_info.pictureFormat), FormatHandle(*vs_state).c_str());
        }

        auto supported_usage = src_resource.image_view_state->inherited_usage;
        if ((supported_usage & VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->Handle(), src_resource.image_view_state->Handle(),
                                        src_resource.image_state->Handle());
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08210", objlist, src_image_view_loc,
                             "(%s created from %s) was not created with VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR "
                             "thus it cannot be used as an encode input picture with the bound video session %s "
                             "that was created with an encode operation.",
                             FormatHandle(src_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(src_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        skip |= VerifyImageLayout(*cb_state, *src_resource.image_state, src_resource.range, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
                                  error_obj.location, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08222", &hit_error);
    }

    if (pEncodeInfo->pReferenceSlots) {
        vvl::VideoPictureResources unique_resources{};
        bool resources_unique = true;

        skip |= ValidateActiveReferencePictureCount(*cb_state, *pEncodeInfo, error_obj.location);
        skip |= ValidateReferencePictureUseCount(*cb_state, *pEncodeInfo, error_obj.location);

        for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
            if (pEncodeInfo->pReferenceSlots[i].slotIndex < 0) {
                skip |= LogError("VUID-VkVideoEncodeInfoKHR-slotIndex-08241", commandBuffer,
                                 encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::slotIndex),
                                 "(%d) "
                                 "must not be negative.",
                                 pEncodeInfo->pReferenceSlots[i].slotIndex);
            } else if ((uint32_t)pEncodeInfo->pReferenceSlots[i].slotIndex >= vs_state->create_info.maxDpbSlots) {
                const LogObjectList objlist(commandBuffer, vs_state->Handle());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-slotIndex-08217", objlist,
                                 encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::slotIndex),
                                 "(%d) "
                                 "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                                 "was created with.",
                                 pEncodeInfo->pReferenceSlots[i].slotIndex, vs_state->create_info.maxDpbSlots,
                                 FormatHandle(*vs_state).c_str());
            }

            if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
                auto reference_resource = vvl::VideoPictureResource(*this, *pEncodeInfo->pReferenceSlots[i].pPictureResource);
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    const auto &it = bound_resources.find(reference_resource);
                    if (it == bound_resources.end()) {
                        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219", commandBuffer, error_obj.location,
                                         "the video picture resource specified in "
                                         "pEncodeInfo->pReferenceSlots[%u].pPictureResource is not one of the "
                                         "bound video picture resources.",
                                         i);
                    } else if (pEncodeInfo->pReferenceSlots[i].slotIndex >= 0 &&
                               pEncodeInfo->pReferenceSlots[i].slotIndex != it->second) {
                        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219", commandBuffer, error_obj.location,
                                         "the bound video picture resource specified in "
                                         "pEncodeInfo->pReferenceSlots[%u].pPictureResource is not currently "
                                         "associated with the DPB slot index specifed in "
                                         "pEncodeInfo->pReferenceSlots[%u].slotIndex (%d).",
                                         i, i, pEncodeInfo->pReferenceSlots[i].slotIndex);
                    }

                    skip |=
                        ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state,
                                                     encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                                     "VUID-vkCmdEncodeVideoKHR-codedOffset-08218");

                    skip |= VerifyImageLayout(*cb_state, *reference_resource.image_state, reference_resource.range,
                                              VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, error_obj.location,
                                              "VUID-vkCmdEncodeVideoKHR-pPictureResource-08224", &hit_error);
                }
            } else {
                skip |= LogError("VUID-VkVideoEncodeInfoKHR-pPictureResource-08242", commandBuffer,
                                 encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource), "is NULL.");
            }
        }

        if (!resources_unique) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08220", commandBuffer, error_obj.location,
                             "more than one element of pEncodeInfo->pReferenceSlots "
                             "refers to the same video picture resource.");
        }
    }

    uint32_t op_count = vs_state->GetVideoEncodeOperationCount(pEncodeInfo);

    for (const auto &query : cb_state->activeQueries) {
        if (query.active_query_index + op_count > query.last_activatable_query_index + 1) {
            auto query_pool_state = Get<vvl::QueryPool>(query.pool);
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-opCount-07174", commandBuffer, error_obj.location,
                             "not enough activatable queries for query type %s "
                             "with opCount %u, active query index %u, and last activatable query index %u.",
                             string_VkQueryType(query_pool_state->create_info.queryType), op_count, query.active_query_index,
                             query.last_activatable_query_index);
        }
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pEncodeInfo->pNext);
        if (inline_query_info != nullptr) {
            if (auto query_pool_state = Get<vvl::QueryPool>(inline_query_info->queryPool)) {
                skip |= ValidateVideoInlineQueryInfo(*query_pool_state, *inline_query_info,
                                                     encode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR));

                if (inline_query_info->queryCount != op_count) {
                    const LogObjectList objlist(commandBuffer, vs_state->Handle());
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08360", objlist,
                                     encode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR, Field::queryCount),
                                     "(%u) is not equal to opCount (%u).", inline_query_info->queryCount, op_count);
                }

                if (query_pool_state->create_info.queryType != VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
                    query_pool_state->create_info.queryType != VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-queryType-08362", objlist,
                                     encode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR, Field::queryPool),
                                     "(%s) has query type (%s) but must be VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR or "
                                     "VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR.",
                                     FormatHandle(*query_pool_state).c_str(),
                                     string_VkQueryType(query_pool_state->create_info.queryType));
                }

                if (vs_state->profile != query_pool_state->supported_video_profile) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool, vs_state->Handle());
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-queryPool-08363", objlist,
                                     encode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR, Field::queryPool),
                                     "video profile %s that was created with does not "
                                     "match the video profile of the bound video session %s.",
                                     FormatHandle(*query_pool_state).c_str(), FormatHandle(*vs_state).c_str());
                }

                const auto &qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];
                if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
                    !qf_ext_props.query_result_status_props.queryResultStatusSupport) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-queryType-08364", objlist, error_obj.location,
                                     "the command pool's queue family (index %u) the command "
                                     "buffer %s was allocated from does not support result status queries.",
                                     cb_state->command_pool->queueFamilyIndex, FormatHandle(*cb_state).c_str());
                }
            }
        }
    }

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            skip |= ValidateVideoEncodeInfoH264(*cb_state, *pEncodeInfo, encode_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            skip |= ValidateVideoEncodeInfoH265(*cb_state, *pEncodeInfo, encode_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            skip |= ValidateVideoEncodeInfoAV1(*cb_state, *pEncodeInfo, encode_info_loc);
            break;

        default:
            break;
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    const Location loc = record_obj.location;
    EnqueueVerifyVideoSessionInitialized(*cb_state, *vs_state, loc, "VUID-vkCmdEncodeVideoKHR-None-07012");

    // For encode sessions also verify encode quality level match for the bound parameters object
    if (vs_state->IsEncode() && cb_state->bound_video_session_parameters) {
        if (!cb_state->video_encode_quality_level.has_value()) {
            // If we already know the current encode quality level already at command buffer recording
            // time, because it was set in this command buffer, then that was already checked outside
            // so we only have to do submit-time validation if that's not the case
            cb_state->video_session_updates[vs_state->VkHandle()].emplace_back(
                [vsp_state = cb_state->bound_video_session_parameters, loc](
                    const vvl::Device &dev_data, const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
                    bool do_validate) {
                    if (!do_validate) return false;
                    bool skip = false;
                    if (vsp_state->GetEncodeQualityLevel() != dev_state.GetEncodeQualityLevel()) {
                        const LogObjectList objlist(vs_state->Handle(), vsp_state->Handle());
                        skip |= dev_data.LogError("VUID-vkCmdEncodeVideoKHR-None-08318", objlist, loc,
                                                  "The currently configured encode quality level (%u) for %s "
                                                  "does not match the encode quality level (%u) %s was created with.",
                                                  dev_state.GetEncodeQualityLevel(), dev_data.FormatHandle(*vs_state).c_str(),
                                                  vsp_state->GetEncodeQualityLevel(), dev_data.FormatHandle(*vsp_state).c_str());
                    }
                    return skip;
                });
        }
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pEncodeInfo->pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            EnqueueVerifyVideoInlineQueryUnavailable(*cb_state, *inline_query_info, Func::vkCmdEncodeVideoKHR);
        }
    }
}
