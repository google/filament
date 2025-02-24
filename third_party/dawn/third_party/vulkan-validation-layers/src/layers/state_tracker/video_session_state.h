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
#pragma once

#include "state_tracker/state_object.h"
#include "utils/hash_util.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>

namespace vvl {
class Device;
class Image;
class ImageView;
class VideoSession;

struct QuantizationMapTexelSize {
    struct compare {
        bool operator()(VkExtent2D const &lhs, VkExtent2D const &rhs) const {
            return lhs.width == rhs.width && lhs.height == rhs.height;
        }
    };

    struct hash {
        std::size_t operator()(VkExtent2D const &texel_size) const {
            hash_util::HashCombiner hc;
            hc << texel_size.width << texel_size.height;
            return hc.Value();
        }
    };
};

using SupportedQuantizationMapTexelSizes =
    unordered_set<VkExtent2D, QuantizationMapTexelSize::hash, QuantizationMapTexelSize::compare>;

using SupportedVideoProfiles = unordered_set<std::shared_ptr<const class VideoProfileDesc>>;

// The VideoProfileDesc contains the entire video profile description, which includes all
// parameters specified in VkVideoProfileInfoKHR and its pNext chain. This includes any
// parameters specific to the coding operation type (e.g. decode/encode usage hints/modes)
// and the codec. Accordingly, hashing and comparison takes into consideration all the
// relevant parameters.
class VideoProfileDesc : public std::enable_shared_from_this<VideoProfileDesc> {
  public:
    struct Profile {
        bool valid;
        bool is_decode;
        bool is_encode;
        VkVideoProfileInfoKHR base;
        union {
            VkVideoDecodeUsageInfoKHR decode_usage;
            VkVideoEncodeUsageInfoKHR encode_usage;
        };
        union {
            VkVideoDecodeH264ProfileInfoKHR decode_h264;
            VkVideoDecodeH265ProfileInfoKHR decode_h265;
            VkVideoDecodeAV1ProfileInfoKHR decode_av1;
            VkVideoEncodeH264ProfileInfoKHR encode_h264;
            VkVideoEncodeH265ProfileInfoKHR encode_h265;
            VkVideoEncodeAV1ProfileInfoKHR encode_av1;
        };
    };

    struct Capabilities {
        bool supported;
        VkVideoCapabilitiesKHR base;
        union {
            VkVideoDecodeCapabilitiesKHR decode;
            VkVideoEncodeCapabilitiesKHR encode;
        };
        union {
            VkVideoDecodeH264CapabilitiesKHR decode_h264;
            VkVideoDecodeH265CapabilitiesKHR decode_h265;
            VkVideoDecodeAV1CapabilitiesKHR decode_av1;
            VkVideoEncodeH264CapabilitiesKHR encode_h264;
            VkVideoEncodeH265CapabilitiesKHR encode_h265;
            VkVideoEncodeAV1CapabilitiesKHR encode_av1;
        };
        union {
            struct {
            } decode_ext;
            struct {
                VkVideoEncodeQuantizationMapCapabilitiesKHR quantization_map;
            } encode_ext;
        };
    };

    VideoProfileDesc(VkPhysicalDevice physical_device, VkVideoProfileInfoKHR const *profile);
    ~VideoProfileDesc();

    const Profile &GetProfile() const { return profile_; }
    const Capabilities &GetCapabilities() const { return capabilities_; }

    const SupportedQuantizationMapTexelSizes &GetSupportedQuantDeltaMapTexelSizes() const { return quant_delta_map_texel_sizes_; }
    const SupportedQuantizationMapTexelSizes &GetSupportedEmphasisMapTexelSizes() const { return emphasis_map_texel_sizes_; }

    bool IsDecode() const { return profile_.is_decode; }
    bool IsEncode() const { return profile_.is_encode; }
    VkVideoCodecOperationFlagBitsKHR GetCodecOp() const { return profile_.base.videoCodecOperation; }
    VkVideoDecodeH264PictureLayoutFlagBitsKHR GetH264PictureLayout() const { return profile_.decode_h264.pictureLayout; }
    bool HasAV1FilmGrainSupport() const { return profile_.decode_av1.filmGrainSupport; }

    VkExtent2D GetMaxCodingBlockSize() const {
        switch (profile_.base.videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                const uint32_t mb_size = 16;
                return {mb_size, mb_size};
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                uint32_t max_ctb_size = 16 * static_cast<uint32_t>(log2(capabilities_.encode_h265.ctbSizes));
                return {max_ctb_size, max_ctb_size};
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                uint32_t max_sb_size = 128 * static_cast<uint32_t>(log2(capabilities_.encode_av1.superblockSizes));
                return {max_sb_size, max_sb_size};
            }

            default:
                assert(false);
                return {0, 0};
        }
    }

    struct compare {
      public:
        bool operator()(VideoProfileDesc const *lhs, VideoProfileDesc const *rhs) const {
            bool match = lhs->profile_.base.videoCodecOperation == rhs->profile_.base.videoCodecOperation &&
                         lhs->profile_.base.chromaSubsampling == rhs->profile_.base.chromaSubsampling &&
                         lhs->profile_.base.lumaBitDepth == rhs->profile_.base.lumaBitDepth &&
                         lhs->profile_.base.chromaBitDepth == rhs->profile_.base.chromaBitDepth;

            if (match && lhs->profile_.is_decode) {
                match = match && lhs->profile_.decode_usage.videoUsageHints == rhs->profile_.decode_usage.videoUsageHints;
            }

            if (match && lhs->profile_.is_encode) {
                match = match && lhs->profile_.encode_usage.videoUsageHints == rhs->profile_.encode_usage.videoUsageHints &&
                        lhs->profile_.encode_usage.videoContentHints == rhs->profile_.encode_usage.videoContentHints &&
                        lhs->profile_.encode_usage.tuningMode == rhs->profile_.encode_usage.tuningMode;
            }

            if (match) {
                switch (lhs->profile_.base.videoCodecOperation) {
                    case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                        match = match && lhs->profile_.decode_h264.stdProfileIdc == rhs->profile_.decode_h264.stdProfileIdc &&
                                lhs->profile_.decode_h264.pictureLayout == rhs->profile_.decode_h264.pictureLayout;
                        break;

                    case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                        match = match && lhs->profile_.decode_h265.stdProfileIdc == rhs->profile_.decode_h265.stdProfileIdc;
                        break;

                    case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
                        match = match && lhs->profile_.decode_av1.stdProfile == rhs->profile_.decode_av1.stdProfile &&
                                lhs->profile_.decode_av1.filmGrainSupport == rhs->profile_.decode_av1.filmGrainSupport;
                        break;

                    case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                        match = match && lhs->profile_.encode_h264.stdProfileIdc == rhs->profile_.encode_h264.stdProfileIdc;
                        break;

                    case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                        match = match && lhs->profile_.encode_h265.stdProfileIdc == rhs->profile_.encode_h265.stdProfileIdc;
                        break;

                    case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR:
                        match = match && lhs->profile_.encode_av1.stdProfile == rhs->profile_.encode_av1.stdProfile;
                        break;

                    default:
                        break;
                }
            }

            return match;
        }
    };

    struct hash {
      public:
        std::size_t operator()(VideoProfileDesc const *desc) const {
            hash_util::HashCombiner hc;
            hc << desc->profile_.base.videoCodecOperation << desc->profile_.base.chromaSubsampling
               << desc->profile_.base.lumaBitDepth << desc->profile_.base.chromaBitDepth;

            if (desc->profile_.is_decode) {
                hc << desc->profile_.decode_usage.videoUsageHints;
            }

            if (desc->profile_.is_encode) {
                hc << desc->profile_.encode_usage.videoUsageHints << desc->profile_.encode_usage.videoContentHints
                   << desc->profile_.encode_usage.tuningMode;
            }

            switch (desc->profile_.base.videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                    hc << desc->profile_.decode_h264.stdProfileIdc << desc->profile_.decode_h264.pictureLayout;
                    break;
                }
                case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
                    hc << desc->profile_.decode_h265.stdProfileIdc;
                    break;
                }
                case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
                    hc << desc->profile_.decode_av1.stdProfile << desc->profile_.decode_av1.filmGrainSupport;
                    break;
                }
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                    hc << desc->profile_.encode_h264.stdProfileIdc;
                    break;
                }
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                    hc << desc->profile_.encode_h265.stdProfileIdc;
                    break;
                }
                case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                    hc << desc->profile_.encode_av1.stdProfile;
                    break;
                }
                default:
                    break;
            }
            return hc.Value();
        }
    };

    // The cache maintains non-owning references to all VideoProfileDesc objects that are referred to by shared
    // pointers of VideoProfileDesc owned by any object.
    // This ensured that every VideoProfileDesc object has only a unique instance acquired from the cache using the
    // Get() method and thus enable us to compare video profiles through a single pointer comparison instead of a
    // deep compare of the structure chains.
    // Once all shared pointers to a VideoProfileDesc go away the destructor will call Release() to remove the
    // non-owning reference to the to-be-deleted object.
    class Cache {
      public:
        std::shared_ptr<const VideoProfileDesc> Get(VkPhysicalDevice physical_device, VkVideoProfileInfoKHR const *profile);
        SupportedVideoProfiles Get(VkPhysicalDevice physical_device, VkVideoProfileListInfoKHR const *profile_list);
        void Release(VideoProfileDesc const *desc);

      private:
        using PerDeviceVideoProfileDescSet =
            unordered_set<VideoProfileDesc const *, VideoProfileDesc::hash, VideoProfileDesc::compare>;

        std::mutex mutex_;
        unordered_map<VkPhysicalDevice, PerDeviceVideoProfileDescSet> entries_;

        std::shared_ptr<const VideoProfileDesc> GetOrCreate(VkPhysicalDevice physical_device, VkVideoProfileInfoKHR const *profile);
    };

  private:
    VkPhysicalDevice physical_device_;
    Profile profile_;
    Capabilities capabilities_;
    SupportedQuantizationMapTexelSizes quant_delta_map_texel_sizes_;
    SupportedQuantizationMapTexelSizes emphasis_map_texel_sizes_;
    Cache *cache_;

    bool InitProfile(VkVideoProfileInfoKHR const *profile);
    void InitCapabilities(VkPhysicalDevice physical_device);
    void InitQuantizationMapFormats(VkPhysicalDevice physical_device);
};

class VideoPictureResource {
  public:
    std::shared_ptr<const ImageView> image_view_state;
    std::shared_ptr<const Image> image_state;
    uint32_t base_array_layer;
    VkImageSubresourceRange range;
    VkOffset2D coded_offset;
    VkExtent2D coded_extent;

    VideoPictureResource();
    VideoPictureResource(const Device &dev_data, VkVideoPictureResourceInfoKHR const &res);

    operator bool() const { return image_view_state != nullptr; }

    bool operator==(VideoPictureResource const &rhs) const {
        return image_state == rhs.image_state && range.baseMipLevel == rhs.range.baseMipLevel &&
               range.baseArrayLayer == rhs.range.baseArrayLayer && coded_offset.x == rhs.coded_offset.x &&
               coded_offset.y == rhs.coded_offset.y && coded_extent.width == rhs.coded_extent.width &&
               coded_extent.height == rhs.coded_extent.height;
    }

    bool operator!=(VideoPictureResource const &rhs) const {
        return image_state != rhs.image_state || range.baseMipLevel != rhs.range.baseMipLevel ||
               range.baseArrayLayer != rhs.range.baseArrayLayer || coded_offset.x != rhs.coded_offset.x ||
               coded_offset.y != rhs.coded_offset.y || coded_extent.width != rhs.coded_extent.width ||
               coded_extent.height != rhs.coded_extent.height;
    }

    struct hash {
      public:
        std::size_t operator()(VideoPictureResource const &res) const {
            hash_util::HashCombiner hc;
            hc << res.image_state.get() << res.range.baseMipLevel << res.range.baseArrayLayer << res.coded_offset.x
               << res.coded_offset.y << res.coded_extent.width << res.coded_extent.height;
            return hc.Value();
        }
    };

    VkOffset3D GetEffectiveImageOffset(const vvl::VideoSession &vs_state) const;
    VkExtent3D GetEffectiveImageExtent(const vvl::VideoSession &vs_state) const;

  private:
    VkImageSubresourceRange GetImageSubresourceRange(ImageView const *image_view_state, uint32_t layer);
};

using VideoPictureResources = unordered_set<VideoPictureResource, VideoPictureResource::hash>;
using BoundVideoPictureResources = unordered_map<VideoPictureResource, int32_t, VideoPictureResource::hash>;

struct VideoPictureID {
    // Used by H.264 to indicate it's a top field picture
    bool top_field = false;
    // Used by H.264 to indicate it's a bottom field picture
    bool bottom_field = false;

    VideoPictureID() {}
    VideoPictureID(VideoProfileDesc const &profile, VkVideoReferenceSlotInfoKHR const &slot);

    static VideoPictureID Frame() {
        VideoPictureID id{};
        return id;
    }

    static VideoPictureID TopField() {
        VideoPictureID id{};
        id.top_field = true;
        return id;
    }

    static VideoPictureID BottomField() {
        VideoPictureID id{};
        id.bottom_field = true;
        return id;
    }

    bool IsFrame() const { return !top_field && !bottom_field; }
    bool IsTopField() const { return top_field && !bottom_field; }
    bool IsBottomField() const { return !top_field && bottom_field; }
    bool IsBothFields() const { return top_field && bottom_field; }
    bool ContainsTopField() const { return top_field; }
    bool ContainsBottomField() const { return bottom_field; }

    bool operator==(VideoPictureID const &rhs) const { return top_field == rhs.top_field && bottom_field == rhs.bottom_field; }

    struct hash {
      public:
        std::size_t operator()(VideoPictureID const &id) const {
            hash_util::HashCombiner hc;
            hc << id.top_field << id.bottom_field;
            return hc.Value();
        }
    };
};

struct VideoReferenceSlot {
    int32_t index;
    VideoPictureID picture_id;
    VideoPictureResource resource;

    VideoReferenceSlot() : index(-1), picture_id(), resource() {}

    VideoReferenceSlot(const Device &dev_data, VideoProfileDesc const &profile, VkVideoReferenceSlotInfoKHR const &slot,
                       bool has_picture_id = true)
        : index(slot.slotIndex),
          picture_id(has_picture_id ? VideoPictureID(profile, slot) : VideoPictureID()),
          resource(slot.pPictureResource ? VideoPictureResource(dev_data, *slot.pPictureResource) : VideoPictureResource()) {}

    // The reference is only valid if it refers to a valid DPB index and resource
    operator bool() const { return index >= 0 && resource; }
};

struct VideoEncodeRateControlLayerState {
    VkVideoEncodeRateControlLayerInfoKHR base;
    union {
        VkVideoEncodeH264RateControlLayerInfoKHR h264;
        VkVideoEncodeH265RateControlLayerInfoKHR h265;
        VkVideoEncodeAV1RateControlLayerInfoKHR av1;
    };

    VideoEncodeRateControlLayerState(VkVideoCodecOperationFlagBitsKHR op = VK_VIDEO_CODEC_OPERATION_NONE_KHR,
                                     const VkVideoEncodeRateControlLayerInfoKHR *info = nullptr) {
        base = info ? *info : vku::InitStructHelper();

        switch (op) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                auto h264_info = vku::FindStructInPNextChain<VkVideoEncodeH264RateControlLayerInfoKHR>(info);
                h264 = h264_info ? *h264_info : vku::InitStructHelper();
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                auto h265_info = vku::FindStructInPNextChain<VkVideoEncodeH265RateControlLayerInfoKHR>(info);
                h265 = h265_info ? *h265_info : vku::InitStructHelper();
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                auto av1_info = vku::FindStructInPNextChain<VkVideoEncodeAV1RateControlLayerInfoKHR>(info);
                av1 = av1_info ? *av1_info : vku::InitStructHelper();
                break;
            }

            default:
                assert(false);
                break;
        }
    }
};

struct VideoEncodeRateControlState {
    bool default_state{true};
    VkVideoEncodeRateControlInfoKHR base;
    union {
        VkVideoEncodeH264RateControlInfoKHR h264;
        VkVideoEncodeH265RateControlInfoKHR h265;
        VkVideoEncodeAV1RateControlInfoKHR av1;
    };
    std::vector<VideoEncodeRateControlLayerState> layers;

    VideoEncodeRateControlState(VkVideoCodecOperationFlagBitsKHR op = VK_VIDEO_CODEC_OPERATION_NONE_KHR,
                                const void *info = nullptr) {
        auto base_info = vku::FindStructInPNextChain<VkVideoEncodeRateControlInfoKHR>(info);
        if (base_info != nullptr) {
            default_state = false;
            base = *base_info;
        } else {
            default_state = true;
            base = vku::InitStructHelper();
        }

        switch (op) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                auto h264_info = vku::FindStructInPNextChain<VkVideoEncodeH264RateControlInfoKHR>(info);
                h264 = h264_info ? *h264_info : vku::InitStructHelper();
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                auto h265_info = vku::FindStructInPNextChain<VkVideoEncodeH265RateControlInfoKHR>(info);
                h265 = h265_info ? *h265_info : vku::InitStructHelper();
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR: {
                auto av1_info = vku::FindStructInPNextChain<VkVideoEncodeAV1RateControlInfoKHR>(info);
                av1 = av1_info ? *av1_info : vku::InitStructHelper();
                break;
            }

            default:
                break;
        }

        layers.reserve(base.layerCount);
        for (uint32_t i = 0; i < base.layerCount; ++i) {
            layers.emplace_back(op, &base.pLayers[i]);
        }
    }

    operator bool() const { return !default_state; }
};

class VideoSessionDeviceState {
  public:
    VideoSessionDeviceState(uint32_t reference_slot_count = 0)
        : initialized_(false),
          is_active_(reference_slot_count, false),
          all_pictures_(reference_slot_count),
          pictures_per_id_(reference_slot_count),
          encode_() {}

    bool IsInitialized() const { return initialized_; }
    bool IsSlotActive(int32_t slot_index) const {
        if (slot_index < 0 || static_cast<uint32_t>(slot_index) >= is_active_.size()) {
            // Out-of-bounds slot index
            return false;
        }
        return is_active_[slot_index];
    }

    bool IsSlotPicture(int32_t slot_index, const VideoPictureResource &res) const {
        if (slot_index < 0 || static_cast<uint32_t>(slot_index) >= all_pictures_.size()) {
            // Out-of-bounds slot index
            return false;
        }
        return all_pictures_[slot_index].find(res) != all_pictures_[slot_index].end();
    }

    bool IsSlotPicture(int32_t slot_index, const VideoPictureID &picture_id, const VideoPictureResource &res) const {
        if (slot_index < 0 || static_cast<uint32_t>(slot_index) >= pictures_per_id_.size()) {
            // Out-of-bounds slot index
            return false;
        }
        auto it = pictures_per_id_[slot_index].find(picture_id);
        return it != pictures_per_id_[slot_index].end() && it->second == res;
    }

    uint32_t GetEncodeQualityLevel() const { return encode_.quality_level; }
    const VideoEncodeRateControlState &GetRateControlState() const { return encode_.rate_control_state; }

    void Reset();
    void Activate(int32_t slot_index, const VideoPictureID &picture_id, const VideoPictureResource &res);
    void Invalidate(int32_t slot_index, const VideoPictureID &picture_id);
    void Deactivate(int32_t slot_index);

    void SetEncodeQualityLevel(uint32_t quality_level) { encode_.quality_level = quality_level; }
    void SetRateControlState(const VideoEncodeRateControlState &rate_control_state) {
        encode_.rate_control_state = rate_control_state;
    }

    bool ValidateRateControlState(const Device &dev_data, const VideoSession *vs_state,
                                  const vku::safe_VkVideoBeginCodingInfoKHR &begin_info, const Location &loc) const;

  private:
    bool initialized_;
    std::vector<bool> is_active_;
    std::vector<VideoPictureResources> all_pictures_;
    std::vector<unordered_map<VideoPictureID, VideoPictureResource, VideoPictureID::hash>> pictures_per_id_;

    struct {
        uint32_t quality_level{0};
        VideoEncodeRateControlState rate_control_state{};
    } encode_;
};

class VideoSession : public StateObject {
  public:
    struct MemoryBindingInfo {
        VkMemoryRequirements requirements;
        bool bound;
    };
    using MemoryBindingMap = unordered_map<uint32_t, MemoryBindingInfo>;

    const vku::safe_VkVideoSessionCreateInfoKHR safe_create_info;
    const VkVideoSessionCreateInfoKHR &create_info;
    std::shared_ptr<const VideoProfileDesc> profile;
    bool memory_binding_count_queried;
    uint32_t memory_bindings_queried;

    class DeviceStateWriter {
      public:
        DeviceStateWriter(VideoSession *state) : lock_(state->device_state_mutex_), state_(state->device_state_) {}
        VideoSessionDeviceState &operator*() { return state_; }

      private:
        std::unique_lock<std::mutex> lock_;
        VideoSessionDeviceState &state_;
    };

    VideoSession(const Device &dev_data, VkVideoSessionKHR handle, VkVideoSessionCreateInfoKHR const *pCreateInfo,
                 std::shared_ptr<const VideoProfileDesc> &&profile_desc);

    VkVideoSessionKHR VkHandle() const { return handle_.Cast<VkVideoSessionKHR>(); }
    bool IsDecode() const { return profile->IsDecode(); }
    bool IsEncode() const { return profile->IsEncode(); }
    VkVideoCodecOperationFlagBitsKHR GetCodecOp() const { return profile->GetCodecOp(); }

    VkVideoDecodeH264PictureLayoutFlagBitsKHR GetH264PictureLayout() const { return profile->GetH264PictureLayout(); }

    VideoSessionDeviceState DeviceStateCopy() const {
        std::unique_lock<std::mutex> lock(device_state_mutex_);
        return device_state_;
    }

    DeviceStateWriter DeviceStateWrite() { return DeviceStateWriter(this); }

    const MemoryBindingInfo *GetMemoryBindingInfo(uint32_t index) const {
        auto it = memory_bindings_.find(index);
        if (it != memory_bindings_.end()) {
            return &it->second;
        } else {
            return nullptr;
        }
    }

    uint32_t GetMemoryBindingCount() const { return (uint32_t)memory_bindings_.size(); }
    uint32_t GetUnboundMemoryBindingCount() const { return unbound_memory_binding_count_; }

    void BindMemoryBindingIndex(uint32_t index) {
        auto it = memory_bindings_.find(index);
        if (it != memory_bindings_.end() && !it->second.bound) {
            it->second.bound = true;
            --unbound_memory_binding_count_;
        }
    }

    uint32_t GetVideoDecodeOperationCount(VkVideoDecodeInfoKHR const *) const { return 1; }
    uint32_t GetVideoEncodeOperationCount(VkVideoEncodeInfoKHR const *) const { return 1; }

    bool ReferenceSetupRequested(VkVideoDecodeInfoKHR const &decode_info) const;
    bool ReferenceSetupRequested(VkVideoEncodeInfoKHR const &encode_info) const;

  private:
    MemoryBindingMap GetMemoryBindings(const Device &dev_data, VkVideoSessionKHR vs);

    MemoryBindingMap memory_bindings_;
    uint32_t unbound_memory_binding_count_;

    mutable std::mutex device_state_mutex_;
    VideoSessionDeviceState device_state_;
};

class VideoSessionParameters : public StateObject {
  public:
    using H264SPSKey = uint8_t;
    using H264PPSKey = uint16_t;
    using H265VPSKey = uint8_t;
    using H265SPSKey = uint16_t;
    using H265PPSKey = uint32_t;
    using ParameterKey = uint32_t;

    struct H264Parameters {
        unordered_map<H264SPSKey, StdVideoH264SequenceParameterSet> sps;
        unordered_map<H264PPSKey, StdVideoH264PictureParameterSet> pps;
        uint32_t sps_capacity;
        uint32_t pps_capacity;
    };

    struct H265Parameters {
        unordered_map<H265VPSKey, StdVideoH265VideoParameterSet> vps;
        unordered_map<H265SPSKey, StdVideoH265SequenceParameterSet> sps;
        unordered_map<H265PPSKey, StdVideoH265PictureParameterSet> pps;
        uint32_t vps_capacity;
        uint32_t sps_capacity;
        uint32_t pps_capacity;
    };

    struct AV1Parameters {
        std::unique_ptr<StdVideoAV1SequenceHeader> seq_header;
    };

    struct Data {
        uint32_t update_sequence_counter;
        H264Parameters h264;
        H265Parameters h265;
        AV1Parameters av1;
    };

    struct EncodeConfig {
        uint32_t quality_level{0};
        VkExtent2D quantization_map_texel_size{0, 0};
    };

    struct Config {
        EncodeConfig encode;
    };

    static H264SPSKey GetH264SPSKey(const StdVideoH264SequenceParameterSet &sps) { return sps.seq_parameter_set_id; }

    static H264PPSKey GetH264PPSKey(const StdVideoH264PictureParameterSet &pps) {
        return GetKeyFor2xID8(pps.seq_parameter_set_id, pps.pic_parameter_set_id);
    }

    static H265VPSKey GetH265VPSKey(const StdVideoH265VideoParameterSet &vps) { return vps.vps_video_parameter_set_id; }

    static H265SPSKey GetH265SPSKey(const StdVideoH265SequenceParameterSet &sps) {
        return GetKeyFor2xID8(sps.sps_video_parameter_set_id, sps.sps_seq_parameter_set_id);
    }

    static H265PPSKey GetH265PPSKey(const StdVideoH265PictureParameterSet &pps) {
        return GetKeyFor3xID8(pps.sps_video_parameter_set_id, pps.pps_seq_parameter_set_id, pps.pps_pic_parameter_set_id);
    }

    class ReadOnlyAccessor {
      public:
        ReadOnlyAccessor() : lock_(), data_(nullptr) {}
        ReadOnlyAccessor(const VideoSessionParameters *state) : lock_(state->mutex_), data_(&state->data_) {}
        operator bool() const { return data_ != nullptr; }
        const Data *operator->() const { return data_; }

        const StdVideoH264SequenceParameterSet *GetH264SPS(uint32_t sps_id) const {
            if (sps_id > 0xFF) return nullptr;
            auto it = data_->h264.sps.find(sps_id & 0xFF);
            if (it != data_->h264.sps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH264PictureParameterSet *GetH264PPS(uint32_t sps_id, uint32_t pps_id) const {
            if (sps_id > 0xFF) return nullptr;
            if (pps_id > 0xFF) return nullptr;
            auto it = data_->h264.pps.find(GetKeyFor2xID8(sps_id & 0xFF, pps_id & 0xFF));
            if (it != data_->h264.pps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH265VideoParameterSet *GetH265VPS(uint32_t vps_id) const {
            if (vps_id > 0xFF) return nullptr;
            auto it = data_->h265.vps.find(vps_id & 0xFF);
            if (it != data_->h265.vps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH265SequenceParameterSet *GetH265SPS(uint32_t vps_id, uint32_t sps_id) const {
            if (vps_id > 0xFF) return nullptr;
            if (sps_id > 0xFF) return nullptr;
            auto it = data_->h265.sps.find(GetKeyFor2xID8(vps_id & 0xFF, sps_id & 0xFF));
            if (it != data_->h265.sps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH265PictureParameterSet *GetH265PPS(uint32_t vps_id, uint32_t sps_id, uint32_t pps_id) const {
            if (vps_id > 0xFF) return nullptr;
            if (sps_id > 0xFF) return nullptr;
            if (pps_id > 0xFF) return nullptr;
            auto it = data_->h265.pps.find(GetKeyFor3xID8(vps_id & 0xFF, sps_id & 0xFF, pps_id & 0xFF));
            if (it != data_->h265.pps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoAV1SequenceHeader *GetAV1SequenceHeader() const { return data_->av1.seq_header.get(); }

      private:
        std::unique_lock<std::mutex> lock_;
        const Data *data_;
    };

    const vku::safe_VkVideoSessionParametersCreateInfoKHR safe_create_info;
    const VkVideoSessionParametersCreateInfoKHR &create_info;
    std::shared_ptr<const VideoSession> vs_state;

    VideoSessionParameters(VkVideoSessionParametersKHR handle, VkVideoSessionParametersCreateInfoKHR const *pCreateInfo,
                           std::shared_ptr<VideoSession> &&vsstate, std::shared_ptr<VideoSessionParameters> &&vsp_template);

    VkVideoSessionParametersKHR VkHandle() const { return handle_.Cast<VkVideoSessionParametersKHR>(); }

    bool IsDecode() const { return vs_state->IsDecode(); }
    bool IsEncode() const { return vs_state->IsEncode(); }

    VkVideoCodecOperationFlagBitsKHR GetCodecOp() const { return vs_state->GetCodecOp(); }

    uint32_t GetEncodeQualityLevel() const { return config_.encode.quality_level; }
    VkExtent2D GetEncodeQuantizationMapTexelSize() const { return config_.encode.quantization_map_texel_size; }

    void Update(VkVideoSessionParametersUpdateInfoKHR const *info);

    ReadOnlyAccessor Lock() const { return ReadOnlyAccessor(this); }

  private:
    mutable std::mutex mutex_;
    Data data_;
    const Config config_;

    Config InitConfig(VkVideoSessionParametersCreateInfoKHR const *pCreateInfo);

    static uint16_t GetKeyFor2xID8(uint8_t id1, uint8_t id2) { return (id1 << 8) | id2; }
    static uint32_t GetKeyFor3xID8(uint8_t id1, uint8_t id2, uint8_t id3) { return (id1 << 16) | (id2 << 8) | id3; }

    void AddDecodeH264(VkVideoDecodeH264SessionParametersAddInfoKHR const *info);
    void AddDecodeH265(VkVideoDecodeH265SessionParametersAddInfoKHR const *info);

    void AddEncodeH264(VkVideoEncodeH264SessionParametersAddInfoKHR const *info);
    void AddEncodeH265(VkVideoEncodeH265SessionParametersAddInfoKHR const *info);
};

using VideoSessionUpdateList = std::vector<std::function<bool(const Device &dev_data, const VideoSession *vs_state,
                                                              VideoSessionDeviceState &dev_state, bool do_validate)>>;
using VideoSessionUpdateMap = unordered_map<VkVideoSessionKHR, VideoSessionUpdateList>;

}  // namespace vvl
