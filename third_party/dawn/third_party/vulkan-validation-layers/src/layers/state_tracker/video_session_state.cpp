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
#include "state_tracker/video_session_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/state_tracker.h"
#include "generated/dispatch_functions.h"

#include <sstream>

namespace vvl {

VideoProfileDesc::VideoProfileDesc(VkPhysicalDevice physical_device, VkVideoProfileInfoKHR const *profile)
    : std::enable_shared_from_this<VideoProfileDesc>(),
      physical_device_(physical_device),
      profile_(),
      capabilities_(),
      cache_(nullptr) {
    if (InitProfile(profile)) {
        InitCapabilities(physical_device);
        InitQuantizationMapFormats(physical_device);
    }
}

VideoProfileDesc::~VideoProfileDesc() {
    if (cache_) {
        cache_->Release(this);
    }
}

bool VideoProfileDesc::InitProfile(VkVideoProfileInfoKHR const *profile) {
    if (profile) {
        profile_.base = *profile;
        profile_.base.pNext = nullptr;
        if (profile_.base.chromaSubsampling == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR) {
            // If monochrome, then chromaBitDepth is ignored, so let's set it to INVALID
            // to avoid special-casing the comparison and hash functions.
            profile_.base.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_INVALID_KHR;
        }

        switch (profile->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto decode_h264 = vku::FindStructInPNextChain<VkVideoDecodeH264ProfileInfoKHR>(profile->pNext);
                if (decode_h264) {
                    profile_.valid = true;
                    profile_.decode_h264 = *decode_h264;
                    profile_.decode_h264.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.decode_h264 = vku::InitStructHelper();
                }
                profile_.is_decode = true;
                profile_.base.pNext = &profile_.decode_h264;
                break;
            }
            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
                auto decode_h265 = vku::FindStructInPNextChain<VkVideoDecodeH265ProfileInfoKHR>(profile->pNext);
                if (decode_h265) {
                    profile_.valid = true;
                    profile_.decode_h265 = *decode_h265;
                    profile_.decode_h265.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.decode_h265 = vku::InitStructHelper();
                }
                profile_.is_decode = true;
                profile_.base.pNext = &profile_.decode_h265;
                break;
            }
            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
                auto decode_av1 = vku::FindStructInPNextChain<VkVideoDecodeAV1ProfileInfoKHR>(profile->pNext);
                if (decode_av1) {
                    profile_.valid = true;
                    profile_.decode_av1 = *decode_av1;
                    profile_.decode_av1.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.decode_av1 = vku::InitStructHelper();
                }
                profile_.is_decode = true;
                profile_.base.pNext = &profile_.decode_av1;
                break;
            }
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                auto encode_h264 = vku::FindStructInPNextChain<VkVideoEncodeH264ProfileInfoKHR>(profile->pNext);
                if (encode_h264) {
                    profile_.valid = true;
                    profile_.encode_h264 = *encode_h264;
                    profile_.encode_h264.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.encode_h264 = vku::InitStructHelper();
                }
                profile_.is_encode = true;
                profile_.base.pNext = &profile_.encode_h264;
                break;
            }
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                auto encode_h265 = vku::FindStructInPNextChain<VkVideoEncodeH265ProfileInfoKHR>(profile->pNext);
                if (encode_h265) {
                    profile_.valid = true;
                    profile_.encode_h265 = *encode_h265;
                    profile_.encode_h265.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.encode_h265 = vku::InitStructHelper();
                }
                profile_.is_encode = true;
                profile_.base.pNext = &profile_.encode_h265;
                break;
            }
            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                auto encode_av1 = vku::FindStructInPNextChain<VkVideoEncodeAV1ProfileInfoKHR>(profile->pNext);
                if (encode_av1) {
                    profile_.valid = true;
                    profile_.encode_av1 = *encode_av1;
                    profile_.encode_av1.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.encode_av1 = vku::InitStructHelper();
                }
                profile_.is_encode = true;
                profile_.base.pNext = &profile_.encode_av1;
                break;
            }
            default:
                profile_.valid = false;
                break;
        }

        if (profile_.is_decode) {
            auto usage = vku::FindStructInPNextChain<VkVideoDecodeUsageInfoKHR>(profile->pNext);
            if (usage) {
                profile_.decode_usage = *usage;
                profile_.decode_usage.pNext = profile_.base.pNext;
                profile_.base.pNext = &profile_.decode_usage;
            } else {
                profile_.decode_usage = vku::InitStructHelper();
            }
        }
        if (profile_.is_encode) {
            auto usage = vku::FindStructInPNextChain<VkVideoEncodeUsageInfoKHR>(profile->pNext);
            if (usage) {
                profile_.encode_usage = *usage;
                profile_.encode_usage.pNext = profile_.base.pNext;
                profile_.base.pNext = &profile_.encode_usage;
            } else {
                profile_.encode_usage = vku::InitStructHelper();
            }
        }
    } else {
        profile_.valid = false;
        profile_.base = vku::InitStructHelper();
    }

    return profile_.valid;
}

void VideoProfileDesc::InitCapabilities(VkPhysicalDevice physical_device) {
    capabilities_.base = vku::InitStructHelper();
    switch (profile_.base.videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.decode;
            capabilities_.decode = vku::InitStructHelper();
            capabilities_.decode.pNext = &capabilities_.decode_h264;
            capabilities_.decode_h264 = vku::InitStructHelper();
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.decode;
            capabilities_.decode = vku::InitStructHelper();
            capabilities_.decode.pNext = &capabilities_.decode_h265;
            capabilities_.decode_h265 = vku::InitStructHelper();
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.decode;
            capabilities_.decode = vku::InitStructHelper();
            capabilities_.decode.pNext = &capabilities_.decode_av1;
            capabilities_.decode_av1 = vku::InitStructHelper();
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.encode;
            capabilities_.encode = vku::InitStructHelper();
            capabilities_.encode.pNext = &capabilities_.encode_h264;
            capabilities_.encode_h264 = vku::InitStructHelper();
            capabilities_.encode_h264.pNext = &capabilities_.encode_ext.quantization_map;
            capabilities_.encode_ext.quantization_map = vku::InitStructHelper();
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.encode;
            capabilities_.encode = vku::InitStructHelper();
            capabilities_.encode.pNext = &capabilities_.encode_h265;
            capabilities_.encode_h265 = vku::InitStructHelper();
            capabilities_.encode_h265.pNext = &capabilities_.encode_ext.quantization_map;
            capabilities_.encode_ext.quantization_map = vku::InitStructHelper();
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.encode;
            capabilities_.encode = vku::InitStructHelper();
            capabilities_.encode.pNext = &capabilities_.encode_av1;
            capabilities_.encode_av1 = vku::InitStructHelper();
            capabilities_.encode_av1.pNext = &capabilities_.encode_ext.quantization_map;
            capabilities_.encode_ext.quantization_map = vku::InitStructHelper();
            break;

        default:
            return;
    }

    VkResult result = DispatchGetPhysicalDeviceVideoCapabilitiesKHR(physical_device, &profile_.base, &capabilities_.base);
    if (result == VK_SUCCESS) {
        capabilities_.supported = true;
    }
}

void VideoProfileDesc::InitQuantizationMapFormats(VkPhysicalDevice physical_device) {
    VkVideoProfileListInfoKHR list = vku::InitStructHelper();
    list.pProfiles = &profile_.base;
    list.profileCount = 1;

    struct QuantMapTypeInfo {
        VkImageUsageFlagBits usage;
        VkVideoEncodeCapabilityFlagBitsKHR cap;
        SupportedQuantizationMapTexelSizes *supported_texel_sizes;
    };

    const std::vector<QuantMapTypeInfo> map_types{
        {VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR,
         &quant_delta_map_texel_sizes_},
        {VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR, VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR,
         &emphasis_map_texel_sizes_},
    };

    for (const auto &map_type : map_types) {
        if ((capabilities_.encode.flags & map_type.cap) == 0) continue;

        VkPhysicalDeviceVideoFormatInfoKHR info = vku::InitStructHelper();
        info.imageUsage = map_type.usage;
        info.pNext = &list;

        uint32_t count;
        VkResult result = DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &info, &count, nullptr);
        if (result != VK_SUCCESS) {
            continue;
        }

        std::vector<VkVideoFormatPropertiesKHR> formats(count, vku::InitStruct<VkVideoFormatPropertiesKHR>());
        std::vector<VkVideoFormatQuantizationMapPropertiesKHR> map_props(
            count, vku::InitStruct<VkVideoFormatQuantizationMapPropertiesKHR>());

        for (uint32_t i = 0; i < count; i++) {
            formats[i].pNext = &map_props[i];
        }

        result = DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &info, &count, formats.data());
        if (result != VK_SUCCESS) {
            formats.resize(0);
            continue;
        }

        for (const auto &props : map_props) {
            map_type.supported_texel_sizes->insert(props.quantizationMapTexelSize);
        }
    }
}

std::shared_ptr<const VideoProfileDesc> VideoProfileDesc::Cache::GetOrCreate(VkPhysicalDevice physical_device,
                                                                             VkVideoProfileInfoKHR const *profile) {
    VideoProfileDesc desc(physical_device, profile);
    if (desc.GetProfile().valid) {
        auto &set = entries_[physical_device];
        auto it = set.find(&desc);
        if (it != set.end()) {
            return (*it)->shared_from_this();
        } else {
            auto desc_ptr = std::make_shared<VideoProfileDesc>(desc);
            desc_ptr->cache_ = this;
            set.emplace(desc_ptr.get());
            return desc_ptr;
        }
    } else {
        return nullptr;
    }
}

std::shared_ptr<const VideoProfileDesc> VideoProfileDesc::Cache::Get(VkPhysicalDevice physical_device,
                                                                     VkVideoProfileInfoKHR const *profile) {
    if (profile) {
        std::unique_lock<std::mutex> lock(mutex_);
        return GetOrCreate(physical_device, profile);
    } else {
        return nullptr;
    }
}

SupportedVideoProfiles VideoProfileDesc::Cache::Get(VkPhysicalDevice physical_device,
                                                    VkVideoProfileListInfoKHR const *profile_list) {
    SupportedVideoProfiles supported_profiles{};
    if (profile_list) {
        std::unique_lock<std::mutex> lock(mutex_);
        for (uint32_t i = 0; i < profile_list->profileCount; ++i) {
            auto profile_desc = GetOrCreate(physical_device, &profile_list->pProfiles[i]);
            if (profile_desc) {
                supported_profiles.insert(std::move(profile_desc));
            }
        }
    }
    return supported_profiles;
}

void VideoProfileDesc::Cache::Release(VideoProfileDesc const *desc) {
    std::unique_lock<std::mutex> lock(mutex_);
    entries_[desc->physical_device_].erase(desc);
}

VideoPictureResource::VideoPictureResource()
    : image_view_state(nullptr), image_state(nullptr), base_array_layer(0), range(), coded_offset(), coded_extent() {}

VideoPictureResource::VideoPictureResource(const Device &dev_data, VkVideoPictureResourceInfoKHR const &res)
    : image_view_state(dev_data.Get<ImageView>(res.imageViewBinding)),
      image_state(image_view_state ? image_view_state->image_state : nullptr),
      base_array_layer(res.baseArrayLayer),
      range(GetImageSubresourceRange(image_view_state.get(), res.baseArrayLayer)),
      coded_offset(res.codedOffset),
      coded_extent(res.codedExtent) {}

VkImageSubresourceRange VideoPictureResource::GetImageSubresourceRange(ImageView const *image_view_state, uint32_t layer) {
    VkImageSubresourceRange range{};
    if (image_view_state) {
        range = image_view_state->normalized_subresource_range;
        range.baseArrayLayer += layer;
        range.layerCount = 1;
    }
    return range;
}

VkOffset3D VideoPictureResource::GetEffectiveImageOffset(const vvl::VideoSession &vs_state) const {
    VkOffset3D offset{coded_offset.x, coded_offset.y, 0};

    // Round to picture access granularity
    const auto gran = vs_state.profile->GetCapabilities().base.pictureAccessGranularity;
    offset.x = (offset.x / gran.width) * gran.width;
    offset.y = (offset.y / gran.height) * gran.height;

    return offset;
}

VkExtent3D VideoPictureResource::GetEffectiveImageExtent(const vvl::VideoSession &vs_state) const {
    VkExtent3D extent{coded_extent.width, coded_extent.height, 1};

    // H.264 decode interlacing with separate planes only accesses half of the coded height
    if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR &&
        vs_state.GetH264PictureLayout() == VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR) {
        extent.height /= 2;
    }

    // Round to picture access granularity
    const auto gran = vs_state.profile->GetCapabilities().base.pictureAccessGranularity;
    extent.width = ((extent.width + gran.width - 1) / gran.width) * gran.width;
    extent.height = ((extent.height + gran.height - 1) / gran.height) * gran.height;

    // Clamp to mip level dimensions
    extent.width = std::min(extent.width, image_state->create_info.extent.width >> range.baseMipLevel);
    extent.height = std::min(extent.height, image_state->create_info.extent.height >> range.baseMipLevel);

    return extent;
}

VideoPictureID::VideoPictureID(VideoProfileDesc const &profile, VkVideoReferenceSlotInfoKHR const &slot) {
    switch (profile.GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(slot.pNext);
            if (slot_info && slot_info->pStdReferenceInfo) {
                top_field = slot_info->pStdReferenceInfo->flags.top_field_flag;
                bottom_field = slot_info->pStdReferenceInfo->flags.bottom_field_flag;
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
            break;

        default:
            break;
    }
}

void VideoSessionDeviceState::Reset() {
    initialized_ = true;
    for (size_t i = 0; i < is_active_.size(); ++i) {
        is_active_[i] = false;
        all_pictures_[i].clear();
        pictures_per_id_[i].clear();
    }
    encode_.quality_level = 0;
    encode_.rate_control_state = VideoEncodeRateControlState();
}

void VideoSessionDeviceState::Activate(int32_t slot_index, const VideoPictureID &picture_id, const VideoPictureResource &res) {
    assert(!picture_id.IsBothFields());

    if (slot_index < 0 || static_cast<uint32_t>(slot_index) >= is_active_.size()) {
        // Out-of-bounds slot index
        return;
    }

    is_active_[slot_index] = true;

    if (picture_id.IsFrame()) {
        // If slot is activated with a frame then it overrides all previous pictures
        all_pictures_[slot_index].clear();
        pictures_per_id_[slot_index].clear();
    }

    auto prev_res_it = pictures_per_id_[slot_index].find(picture_id);
    if (prev_res_it != pictures_per_id_[slot_index].end()) {
        // If we replace an existing picture then remove it
        all_pictures_[slot_index].erase(prev_res_it->second);
    }

    all_pictures_[slot_index].insert(res);
    pictures_per_id_[slot_index][picture_id] = res;
}

void VideoSessionDeviceState::Invalidate(int32_t slot_index, const VideoPictureID &picture_id) {
    assert(!picture_id.IsBothFields());

    if (slot_index < 0 || static_cast<uint32_t>(slot_index) >= is_active_.size()) {
        // Out-of-bounds slot index
        return;
    }

    bool previous_is_frame = pictures_per_id_[slot_index].find(VideoPictureID::Frame()) != pictures_per_id_[slot_index].end();
    if (picture_id.IsFrame() || previous_is_frame) {
        // If invalidation happens due to a non-reference setup frame then it invalidates all previous pictures
        // Also invalidate all if the previous picture reference was a frame (e.g. a field invalidates a previous frame)
        all_pictures_[slot_index].clear();
        pictures_per_id_[slot_index].clear();
    } else {
        // Invalidate any existing picture reference with the specified id by removing it
        auto prev_res_it = pictures_per_id_[slot_index].find(picture_id);
        if (prev_res_it != pictures_per_id_[slot_index].end()) {
            VideoPictureResource res = prev_res_it->second;
            pictures_per_id_[slot_index].erase(picture_id);
            // Check if there are any more references to the previous resource
            auto other_ref_it = std::find_if(std::begin(pictures_per_id_[slot_index]), std::end(pictures_per_id_[slot_index]),
                                             [&res](const auto &it) { return it.second == res; });
            if (other_ref_it == pictures_per_id_[slot_index].end()) {
                // If there are no remaining references to the resource, remove it
                all_pictures_[slot_index].erase(res);
            }
        }
    }

    // If there are no remaining picture references then deactivate the slot
    if (pictures_per_id_[slot_index].empty()) {
        is_active_[slot_index] = false;
    }
}

void VideoSessionDeviceState::Deactivate(int32_t slot_index) {
    if (slot_index < 0 || static_cast<uint32_t>(slot_index) >= is_active_.size()) {
        // Out-of-bounds slot index
        return;
    }

    is_active_[slot_index] = false;
    all_pictures_[slot_index].clear();
    pictures_per_id_[slot_index].clear();
}

class RateControlStateMismatchRecorder {
  public:
    RateControlStateMismatchRecorder() : has_mismatch_{false}, stream_{}, string_{} {}

    template <typename T>
    void Record(const char *where, T actual, T expected) {
        has_mismatch_ = true;
        stream_ << where << " (" << actual << ") does not match current device state (" << expected << ")." << std::endl;
    }

    template <typename T>
    void RecordDefault(const char *missing_struct, const char *member, T expected) {
        has_mismatch_ = true;
        stream_ << missing_struct << " is not in the pNext chain but the current device state for its " << member
                << " member is set (" << expected << ")." << std::endl;
    }

    template <typename T>
    void RecordLayer(uint32_t layer_idx, const char *where, T actual, T expected) {
        has_mismatch_ = true;
        stream_ << where << " (" << actual << ") in VkVideoEncodeRateControlLayerInfoKHR::pLayers[" << layer_idx
                << "] does not match current device state (" << expected << ")." << std::endl;
    }

    template <typename T>
    void RecordLayerDefault(uint32_t layer_idx, const char *missing_struct, const char *member, T expected) {
        has_mismatch_ = true;
        stream_ << missing_struct << " is not in the pNext chain of VkVideoEncodeRateControlLayerInfoKHR::pLayers[" << layer_idx
                << "] but the current device state for its " << member << " member is set (" << expected << ")." << std::endl;
    }

    bool HasMismatch() const { return has_mismatch_; }

    const char *c_str() const {
        string_ = stream_.str();
        return string_.c_str();
    }

  private:
    bool has_mismatch_{};
    std::stringstream stream_{};
    mutable std::string string_{};
};

bool VideoSessionDeviceState::ValidateRateControlState(const Device &dev_data, const VideoSession *vs_state,
                                                       const vku::safe_VkVideoBeginCodingInfoKHR &begin_info,
                                                       const Location &loc) const {
    bool skip = false;

    assert(vs_state->IsEncode());

    auto rc_base = vku::FindStructInPNextChain<VkVideoEncodeRateControlInfoKHR>(begin_info.pNext);

    if (rc_base != nullptr) {
        const auto &ref_base = encode_.rate_control_state.base;
        RateControlStateMismatchRecorder mismatch_recorder{};

        auto string_bool = [](VkBool32 value) { return value ? "VK_TRUE" : "VK_FALSE"; };

#define CHECK_RC_INFO(SCOPE, STRUCT_NAME, MEMBER, CONVERTER)                                                                   \
    if (rc_##SCOPE != nullptr) {                                                                                               \
        if (rc_##SCOPE->MEMBER != ref_##SCOPE.MEMBER) {                                                                        \
            mismatch_recorder.Record(#STRUCT_NAME "::" #MEMBER, CONVERTER(rc_##SCOPE->MEMBER), CONVERTER(ref_##SCOPE.MEMBER)); \
        }                                                                                                                      \
    } else {                                                                                                                   \
        if (ref_##SCOPE.MEMBER != decltype(ref_##SCOPE.MEMBER)()) {                                                            \
            mismatch_recorder.RecordDefault(#STRUCT_NAME, #MEMBER, CONVERTER(ref_##SCOPE.MEMBER));                             \
        }                                                                                                                      \
    }

#define CHECK_RC_LAYER_INFO(SCOPE, STRUCT_NAME, MEMBER, CONVERTER)                                                       \
    if (rc_layer_##SCOPE != nullptr) {                                                                                   \
        if (rc_layer_##SCOPE->MEMBER != ref_layer_##SCOPE.MEMBER) {                                                      \
            mismatch_recorder.RecordLayer(layer_idx, #STRUCT_NAME "::" #MEMBER, CONVERTER(rc_layer_##SCOPE->MEMBER),     \
                                          CONVERTER(ref_layer_##SCOPE.MEMBER));                                          \
        }                                                                                                                \
    } else {                                                                                                             \
        if (ref_layer_##SCOPE.MEMBER != decltype(ref_layer_##SCOPE.MEMBER)()) {                                          \
            mismatch_recorder.RecordLayerDefault(layer_idx, #STRUCT_NAME, #MEMBER, CONVERTER(ref_layer_##SCOPE.MEMBER)); \
        }                                                                                                                \
    }

        CHECK_RC_INFO(base, VkVideoEncodeRateControlInfoKHR, rateControlMode, string_VkVideoEncodeRateControlModeFlagBitsKHR);
        CHECK_RC_INFO(base, VkVideoEncodeRateControlInfoKHR, layerCount, uint32_t);
        CHECK_RC_INFO(base, VkVideoEncodeRateControlInfoKHR, virtualBufferSizeInMs, uint32_t);
        CHECK_RC_INFO(base, VkVideoEncodeRateControlInfoKHR, initialVirtualBufferSizeInMs, uint32_t);

        switch (vs_state->GetCodecOp()) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                auto rc_h264 = vku::FindStructInPNextChain<VkVideoEncodeH264RateControlInfoKHR>(begin_info.pNext);
                const auto &ref_h264 = encode_.rate_control_state.h264;
                CHECK_RC_INFO(h264, VkVideoEncodeH264RateControlInfoKHR, flags, string_VkVideoEncodeH264RateControlFlagsKHR);
                CHECK_RC_INFO(h264, VkVideoEncodeH264RateControlInfoKHR, gopFrameCount, uint32_t);
                CHECK_RC_INFO(h264, VkVideoEncodeH264RateControlInfoKHR, idrPeriod, uint32_t);
                CHECK_RC_INFO(h264, VkVideoEncodeH264RateControlInfoKHR, consecutiveBFrameCount, uint32_t);
                CHECK_RC_INFO(h264, VkVideoEncodeH264RateControlInfoKHR, temporalLayerCount, uint32_t);
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                auto rc_h265 = vku::FindStructInPNextChain<VkVideoEncodeH265RateControlInfoKHR>(begin_info.pNext);
                const auto &ref_h265 = encode_.rate_control_state.h265;
                CHECK_RC_INFO(h265, VkVideoEncodeH265RateControlInfoKHR, flags, string_VkVideoEncodeH265RateControlFlagsKHR);
                CHECK_RC_INFO(h265, VkVideoEncodeH265RateControlInfoKHR, gopFrameCount, uint32_t);
                CHECK_RC_INFO(h265, VkVideoEncodeH265RateControlInfoKHR, idrPeriod, uint32_t);
                CHECK_RC_INFO(h265, VkVideoEncodeH265RateControlInfoKHR, consecutiveBFrameCount, uint32_t);
                CHECK_RC_INFO(h265, VkVideoEncodeH265RateControlInfoKHR, subLayerCount, uint32_t);
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                auto rc_av1 = vku::FindStructInPNextChain<VkVideoEncodeAV1RateControlInfoKHR>(begin_info.pNext);
                const auto &ref_av1 = encode_.rate_control_state.av1;
                CHECK_RC_INFO(av1, VkVideoEncodeAV1RateControlInfoKHR, flags, string_VkVideoEncodeAV1RateControlFlagsKHR);
                CHECK_RC_INFO(av1, VkVideoEncodeAV1RateControlInfoKHR, gopFrameCount, uint32_t);
                CHECK_RC_INFO(av1, VkVideoEncodeAV1RateControlInfoKHR, keyFramePeriod, uint32_t);
                CHECK_RC_INFO(av1, VkVideoEncodeAV1RateControlInfoKHR, consecutiveBipredictiveFrameCount, uint32_t);
                CHECK_RC_INFO(av1, VkVideoEncodeAV1RateControlInfoKHR, temporalLayerCount, uint32_t);
                break;
            }

            default:
                assert(false);
                break;
        }

        for (uint32_t layer_idx = 0; layer_idx < std::min(rc_base->layerCount, ref_base.layerCount); ++layer_idx) {
            const auto rc_layer_base = &rc_base->pLayers[layer_idx];
            const auto &ref_layer_base = encode_.rate_control_state.layers[layer_idx].base;

            CHECK_RC_LAYER_INFO(base, VkVideoEncodeRateControlLayerInfoKHR, averageBitrate, uint64_t);
            CHECK_RC_LAYER_INFO(base, VkVideoEncodeRateControlLayerInfoKHR, maxBitrate, uint64_t);
            CHECK_RC_LAYER_INFO(base, VkVideoEncodeRateControlLayerInfoKHR, frameRateNumerator, uint32_t);
            CHECK_RC_LAYER_INFO(base, VkVideoEncodeRateControlLayerInfoKHR, frameRateDenominator, uint32_t);

            switch (vs_state->GetCodecOp()) {
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                    auto rc_layer_h264 =
                        vku::FindStructInPNextChain<VkVideoEncodeH264RateControlLayerInfoKHR>(rc_layer_base->pNext);
                    const auto &ref_layer_h264 = encode_.rate_control_state.layers[layer_idx].h264;
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, useMinQp, string_bool);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, minQp.qpI, int32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, minQp.qpP, int32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, minQp.qpB, int32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, useMaxQp, string_bool);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, maxQp.qpI, int32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, maxQp.qpP, int32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, maxQp.qpB, int32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, useMaxFrameSize, string_bool);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, maxFrameSize.frameISize, uint32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, maxFrameSize.framePSize, uint32_t);
                    CHECK_RC_LAYER_INFO(h264, VkVideoEncodeH264RateControlLayerInfoKHR, maxFrameSize.frameBSize, uint32_t);
                    break;
                }

                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                    auto rc_layer_h265 =
                        vku::FindStructInPNextChain<VkVideoEncodeH265RateControlLayerInfoKHR>(rc_layer_base->pNext);
                    const auto &ref_layer_h265 = encode_.rate_control_state.layers[layer_idx].h265;
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, useMinQp, string_bool);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, minQp.qpI, int32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, minQp.qpP, int32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, minQp.qpB, int32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, useMaxQp, string_bool);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, maxQp.qpI, int32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, maxQp.qpP, int32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, maxQp.qpB, int32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, useMaxFrameSize, string_bool);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, maxFrameSize.frameISize, uint32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, maxFrameSize.framePSize, uint32_t);
                    CHECK_RC_LAYER_INFO(h265, VkVideoEncodeH265RateControlLayerInfoKHR, maxFrameSize.frameBSize, uint32_t);
                    break;
                }

                case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                    auto rc_layer_av1 = vku::FindStructInPNextChain<VkVideoEncodeAV1RateControlLayerInfoKHR>(rc_layer_base->pNext);
                    const auto &ref_layer_av1 = encode_.rate_control_state.layers[layer_idx].av1;
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, useMinQIndex, string_bool);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, minQIndex.intraQIndex, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, minQIndex.predictiveQIndex, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, minQIndex.bipredictiveQIndex, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, useMaxQIndex, string_bool);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, maxQIndex.intraQIndex, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, maxQIndex.predictiveQIndex, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, maxQIndex.bipredictiveQIndex, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, useMaxFrameSize, string_bool);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, maxFrameSize.intraFrameSize, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, maxFrameSize.predictiveFrameSize, uint32_t);
                    CHECK_RC_LAYER_INFO(av1, VkVideoEncodeAV1RateControlLayerInfoKHR, maxFrameSize.bipredictiveFrameSize, uint32_t);
                    break;
                }

                default:
                    assert(false);
                    break;
            }
        }

#undef CHECK_RC_INFO
#undef CHECK_RC_LAYER_INFO

        if (mismatch_recorder.HasMismatch()) {
            skip |= dev_data.LogError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08254", vs_state->Handle(), loc,
                                      "The video encode rate control information specified when beginning the video coding scope "
                                      "does not match the currently configured video encode rate control state for %s:\n%s",
                                      dev_data.FormatHandle(vs_state->Handle()).c_str(), mismatch_recorder.c_str());
        }

    } else {
        if (encode_.rate_control_state.base.rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR) {
            skip |=
                dev_data.LogError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08253", vs_state->Handle(), loc,
                                  "No VkVideoEncodeRateControlInfoKHR structure was specified when beginning the "
                                  "video coding scope but the currently set video encode rate control mode for %s is %s.",
                                  dev_data.FormatHandle(vs_state->Handle()).c_str(),
                                  string_VkVideoEncodeRateControlModeFlagBitsKHR(encode_.rate_control_state.base.rateControlMode));
        }
    }

    return skip;
}

VideoSession::VideoSession(const Device &dev_data, VkVideoSessionKHR handle, VkVideoSessionCreateInfoKHR const *pCreateInfo,
                           std::shared_ptr<const VideoProfileDesc> &&profile_desc)
    : StateObject(handle, kVulkanObjectTypeVideoSessionKHR),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      profile(std::move(profile_desc)),
      memory_binding_count_queried(false),
      memory_bindings_queried(0),
      memory_bindings_(GetMemoryBindings(dev_data, handle)),
      unbound_memory_binding_count_(static_cast<uint32_t>(memory_bindings_.size())),
      device_state_mutex_(),
      device_state_(pCreateInfo->maxDpbSlots) {}

VideoSession::MemoryBindingMap VideoSession::GetMemoryBindings(const Device &dev_data, VkVideoSessionKHR vs) {
    uint32_t memory_requirement_count;
    DispatchGetVideoSessionMemoryRequirementsKHR(dev_data.device, vs, &memory_requirement_count, nullptr);

    std::vector<VkVideoSessionMemoryRequirementsKHR> memory_requirements(memory_requirement_count,
                                                                         vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    DispatchGetVideoSessionMemoryRequirementsKHR(dev_data.device, vs, &memory_requirement_count, memory_requirements.data());

    MemoryBindingMap memory_bindings;
    for (uint32_t i = 0; i < memory_requirement_count; ++i) {
        memory_bindings[memory_requirements[i].memoryBindIndex].requirements = memory_requirements[i].memoryRequirements;
    }

    return memory_bindings;
}

bool VideoSession::ReferenceSetupRequested(VkVideoDecodeInfoKHR const &decode_info) const {
    switch (GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto pic_info = vku::FindStructInPNextChain<VkVideoDecodeH264PictureInfoKHR>(decode_info.pNext);
            return pic_info != nullptr && pic_info->pStdPictureInfo != nullptr && pic_info->pStdPictureInfo->flags.is_reference;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto pic_info = vku::FindStructInPNextChain<VkVideoDecodeH265PictureInfoKHR>(decode_info.pNext);
            return pic_info != nullptr && pic_info->pStdPictureInfo != nullptr && pic_info->pStdPictureInfo->flags.IsReference;
        }
        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            auto pic_info = vku::FindStructInPNextChain<VkVideoDecodeAV1PictureInfoKHR>(decode_info.pNext);
            return pic_info != nullptr && pic_info->pStdPictureInfo != nullptr &&
                   pic_info->pStdPictureInfo->refresh_frame_flags != 0;
        }

        default:
            return false;
    }
}

bool VideoSession::ReferenceSetupRequested(VkVideoEncodeInfoKHR const &encode_info) const {
    switch (GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto pic_info = vku::FindStructInPNextChain<VkVideoEncodeH264PictureInfoKHR>(encode_info.pNext);
            return pic_info != nullptr && pic_info->pStdPictureInfo != nullptr && pic_info->pStdPictureInfo->flags.is_reference;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto pic_info = vku::FindStructInPNextChain<VkVideoEncodeH265PictureInfoKHR>(encode_info.pNext);
            return pic_info != nullptr && pic_info->pStdPictureInfo != nullptr && pic_info->pStdPictureInfo->flags.is_reference;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            auto pic_info = vku::FindStructInPNextChain<VkVideoEncodeAV1PictureInfoKHR>(encode_info.pNext);
            return pic_info != nullptr && pic_info->pStdPictureInfo != nullptr &&
                   pic_info->pStdPictureInfo->refresh_frame_flags != 0;
        }

        default:
            return false;
    }
}

VideoSessionParameters::VideoSessionParameters(VkVideoSessionParametersKHR handle,
                                               VkVideoSessionParametersCreateInfoKHR const *pCreateInfo,
                                               std::shared_ptr<VideoSession> &&vsstate,
                                               std::shared_ptr<VideoSessionParameters> &&vsp_template)
    : StateObject(handle, kVulkanObjectTypeVideoSessionParametersKHR),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      vs_state(vsstate),
      mutex_(),
      data_(),
      config_(InitConfig(pCreateInfo)) {
    data_.update_sequence_counter = 0;

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            const auto decode_h264 =
                vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersCreateInfoKHR>(create_info.pNext);
            if (vsp_template) {
                auto template_data = vsp_template->Lock();
                data_.h264.sps = template_data->h264.sps;
                data_.h264.pps = template_data->h264.pps;
            }
            if (decode_h264->pParametersAddInfo) {
                AddDecodeH264(decode_h264->pParametersAddInfo);
            }
            data_.h264.sps_capacity = decode_h264->maxStdSPSCount;
            data_.h264.pps_capacity = decode_h264->maxStdPPSCount;
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            const auto decode_h265 =
                vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersCreateInfoKHR>(create_info.pNext);
            if (vsp_template) {
                auto template_data = vsp_template->Lock();
                data_.h265.vps = template_data->h265.vps;
                data_.h265.sps = template_data->h265.sps;
                data_.h265.pps = template_data->h265.pps;
            }
            if (decode_h265->pParametersAddInfo) {
                AddDecodeH265(decode_h265->pParametersAddInfo);
            }
            data_.h265.vps_capacity = decode_h265->maxStdVPSCount;
            data_.h265.sps_capacity = decode_h265->maxStdSPSCount;
            data_.h265.pps_capacity = decode_h265->maxStdPPSCount;
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            const auto decode_av1 = vku::FindStructInPNextChain<VkVideoDecodeAV1SessionParametersCreateInfoKHR>(create_info.pNext);
            if (decode_av1->pStdSequenceHeader) {
                data_.av1.seq_header = std::make_unique<StdVideoAV1SequenceHeader>(*decode_av1->pStdSequenceHeader);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            const auto encode_h264 =
                vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersCreateInfoKHR>(create_info.pNext);
            if (vsp_template) {
                auto template_data = vsp_template->Lock();
                data_.h264.sps = template_data->h264.sps;
                data_.h264.pps = template_data->h264.pps;
            }
            if (encode_h264->pParametersAddInfo) {
                AddEncodeH264(encode_h264->pParametersAddInfo);
            }
            data_.h264.sps_capacity = encode_h264->maxStdSPSCount;
            data_.h264.pps_capacity = encode_h264->maxStdPPSCount;
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            const auto encode_h265 =
                vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersCreateInfoKHR>(create_info.pNext);
            if (vsp_template) {
                auto template_data = vsp_template->Lock();
                data_.h265.vps = template_data->h265.vps;
                data_.h265.sps = template_data->h265.sps;
                data_.h265.pps = template_data->h265.pps;
            }
            if (encode_h265->pParametersAddInfo) {
                AddEncodeH265(encode_h265->pParametersAddInfo);
            }
            data_.h265.vps_capacity = encode_h265->maxStdVPSCount;
            data_.h265.sps_capacity = encode_h265->maxStdSPSCount;
            data_.h265.pps_capacity = encode_h265->maxStdPPSCount;
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            const auto encode_av1 = vku::FindStructInPNextChain<VkVideoEncodeAV1SessionParametersCreateInfoKHR>(create_info.pNext);
            if (encode_av1->pStdSequenceHeader) {
                data_.av1.seq_header = std::make_unique<StdVideoAV1SequenceHeader>(*encode_av1->pStdSequenceHeader);
            }
            break;
        }

        default:
            break;
    }
}

VideoSessionParameters::Config VideoSessionParameters::InitConfig(VkVideoSessionParametersCreateInfoKHR const *pCreateInfo) {
    Config config{};

    if (vs_state->IsEncode()) {
        auto quality_level_info = vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(pCreateInfo->pNext);
        if (quality_level_info != nullptr) {
            config.encode.quality_level = quality_level_info->qualityLevel;
        }

        auto quantization_map_info =
            vku::FindStructInPNextChain<VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR>(pCreateInfo->pNext);
        if (quantization_map_info != nullptr) {
            config.encode.quantization_map_texel_size = quantization_map_info->quantizationMapTexelSize;
        }
    }

    return config;
}

void VideoSessionParameters::Update(VkVideoSessionParametersUpdateInfoKHR const *info) {
    auto lock = Lock();

    data_.update_sequence_counter = info->updateSequenceCount;

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersAddInfoKHR>(info->pNext);
            if (add_info) {
                AddDecodeH264(add_info);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersAddInfoKHR>(info->pNext);
            if (add_info) {
                AddDecodeH265(add_info);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
            // AV1 decode session parameters cannot be updated
            assert(false);
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersAddInfoKHR>(info->pNext);
            if (add_info) {
                AddEncodeH264(add_info);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersAddInfoKHR>(info->pNext);
            if (add_info) {
                AddEncodeH265(add_info);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
            // AV1 encode session parameters cannot be updated
            assert(false);
            break;
        }

        default:
            break;
    }
}

void VideoSessionParameters::AddDecodeH264(VkVideoDecodeH264SessionParametersAddInfoKHR const *info) {
    for (uint32_t i = 0; i < info->stdSPSCount; ++i) {
        const auto &entry = info->pStdSPSs[i];
        data_.h264.sps[GetH264SPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdPPSCount; ++i) {
        const auto &entry = info->pStdPPSs[i];
        data_.h264.pps[GetH264PPSKey(entry)] = entry;
    }
}

void VideoSessionParameters::AddDecodeH265(VkVideoDecodeH265SessionParametersAddInfoKHR const *info) {
    for (uint32_t i = 0; i < info->stdVPSCount; ++i) {
        const auto &entry = info->pStdVPSs[i];
        data_.h265.vps[GetH265VPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdSPSCount; ++i) {
        const auto &entry = info->pStdSPSs[i];
        data_.h265.sps[GetH265SPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdPPSCount; ++i) {
        const auto &entry = info->pStdPPSs[i];
        data_.h265.pps[GetH265PPSKey(entry)] = entry;
    }
}

void VideoSessionParameters::AddEncodeH264(VkVideoEncodeH264SessionParametersAddInfoKHR const *info) {
    for (uint32_t i = 0; i < info->stdSPSCount; ++i) {
        const auto &entry = info->pStdSPSs[i];
        data_.h264.sps[GetH264SPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdPPSCount; ++i) {
        const auto &entry = info->pStdPPSs[i];
        data_.h264.pps[GetH264PPSKey(entry)] = entry;
    }
}

void VideoSessionParameters::AddEncodeH265(VkVideoEncodeH265SessionParametersAddInfoKHR const *info) {
    for (uint32_t i = 0; i < info->stdVPSCount; ++i) {
        const auto &entry = info->pStdVPSs[i];
        data_.h265.vps[GetH265VPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdSPSCount; ++i) {
        const auto &entry = info->pStdSPSs[i];
        data_.h265.sps[GetH265SPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdPPSCount; ++i) {
        const auto &entry = info->pStdPPSs[i];
        data_.h265.pps[GetH265PPSKey(entry)] = entry;
    }
}

}  // namespace vvl
