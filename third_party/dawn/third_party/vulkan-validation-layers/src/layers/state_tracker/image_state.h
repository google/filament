/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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
#pragma once

#include <variant>

#include "state_tracker/device_memory_state.h"
#include "state_tracker/fence_state.h"
#include "state_tracker/image_layout_map.h"
#include "utils/vk_layer_utils.h"

namespace vvl {
class Device;
class Fence;
class Semaphore;
class Surface;
class Swapchain;
class VideoProfileDesc;
}  // namespace vvl

static inline bool operator==(const VkImageSubresource &lhs, const VkImageSubresource &rhs) {
    return (lhs.aspectMask == rhs.aspectMask) && (lhs.mipLevel == rhs.mipLevel) && (lhs.arrayLayer == rhs.arrayLayer);
}

VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info, const VkImageSubresourceRange &range);
VkImageSubresourceRange NormalizeSubresourceRange(const VkImageCreateInfo &image_create_info,
                                                  const VkImageViewCreateInfo &view_create_info);

// Transfer VkImageSubresourceRange into VkImageSubresourceLayers struct
static inline VkImageSubresourceLayers LayersFromRange(const VkImageSubresourceRange &subresource_range) {
    VkImageSubresourceLayers subresource_layers;
    subresource_layers.aspectMask = subresource_range.aspectMask;
    subresource_layers.baseArrayLayer = subresource_range.baseArrayLayer;
    subresource_layers.layerCount = subresource_range.layerCount;
    subresource_layers.mipLevel = subresource_range.baseMipLevel;
    return subresource_layers;
}

// Transfer VkImageSubresourceLayers into VkImageSubresourceRange struct
static inline VkImageSubresourceRange RangeFromLayers(const VkImageSubresourceLayers &subresource_layers) {
    VkImageSubresourceRange subresource_range;
    subresource_range.aspectMask = subresource_layers.aspectMask;
    subresource_range.baseArrayLayer = subresource_layers.baseArrayLayer;
    subresource_range.layerCount = subresource_layers.layerCount;
    subresource_range.baseMipLevel = subresource_layers.mipLevel;
    subresource_range.levelCount = 1;
    return subresource_range;
}

namespace vvl {

// State for VkImage objects.
// Parent -> child relationships in the object usage tree:
// 1. Normal images:
//    vvl::Image [1] -> [1] vvl::DeviceMemory
//
// 2. Sparse images:
//    vvl::Image [1] -> [N] vvl::DeviceMemory
//
// 3. VK_IMAGE_CREATE_ALIAS_BIT images:
//    vvl::Image [N] -> [1] vvl::DeviceMemory
//    All other images using the same device memory are in the aliasing_images set.
//
// 4. Swapchain images
//    vvl::Image [N] -> [1] vvl::Swapchain
//    All other images using the same swapchain and swapchain_image_index are in the aliasing_images set.
//    Note that the images for *every* image_index will show up as parents of the swapchain,
//    so swapchain_image_index values must be compared.
//
class Image : public Bindable {
  public:
    const vku::safe_VkImageCreateInfo safe_create_info;
    const VkImageCreateInfo &create_info;
    bool shared_presentable;                   // True for a front-buffered swapchain image
    bool layout_locked;                        // A front-buffered image that has been presented can never have layout transitioned
    const uint64_t ahb_format;                 // External Android format, if provided
    const VkImageSubresourceRange full_range;  // The normalized ISR for all levels, layers, and aspects
    const VkSwapchainKHR create_from_swapchain;
    const bool owned_by_swapchain;
    std::shared_ptr<vvl::Swapchain> bind_swapchain;
    uint32_t swapchain_image_index;
    const VkFormatFeatureFlags2KHR format_features;
    // Need to memory requirments for each plane if image is disjoint
    const bool disjoint;  // True if image was created with VK_IMAGE_CREATE_DISJOINT_BIT
    static constexpr int kMaxPlanes = 3;
    using MemoryReqs = std::array<VkMemoryRequirements, kMaxPlanes>;
    const MemoryReqs requirements;
    std::array<bool, kMaxPlanes> memory_requirements_checked = {};

    const bool sparse_residency;
    using SparseReqs = std::vector<VkSparseImageMemoryRequirements>;
    const SparseReqs sparse_requirements;
    const bool sparse_metadata_required;  // Track if sparse metadata aspect is required for this image
    bool get_sparse_reqs_called;          // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_bound;           // Track if sparse metadata aspect is bound to this image

    VkImageFormatProperties image_format_properties = {};
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_image_export;
    const bool metal_io_surface_export;
#endif  // VK_USE_PLATFORM_METAL

    const image_layout_map::Encoder subresource_encoder;                             // Subresource resolution encoder
    std::unique_ptr<const subresource_adapter::ImageRangeEncoder> fragment_encoder;  // Fragment resolution encoder
    const VkDevice store_device_as_workaround;                                       // TODO REMOVE WHEN encoder can be const

    std::shared_ptr<GlobalImageLayoutRangeMap> layout_range_map;

    vvl::unordered_set<std::shared_ptr<const vvl::VideoProfileDesc>> supported_video_profiles;

    Image(const Device &dev_data, VkImage handle, const VkImageCreateInfo *pCreateInfo, VkFormatFeatureFlags2KHR features);
    Image(const Device &dev_data, VkImage handle, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
          uint32_t swapchain_index, VkFormatFeatureFlags2KHR features);
    Image(Image const &rh_obj) = delete;
    std::shared_ptr<const Image> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<Image> shared_from_this() { return SharedFromThisImpl(this); }

    VkImage VkHandle() const { return handle_.Cast<VkImage>(); }

    bool HasAHBFormat() const { return ahb_format != 0; }
    bool IsCompatibleAliasing(const Image *other_image_state) const;

    // returns true if this image could be using the same memory as another image
    bool HasAliasFlag() const { return 0 != (create_info.flags & VK_IMAGE_CREATE_ALIAS_BIT); }
    bool CanAlias() const { return HasAliasFlag() || bind_swapchain; }

    bool IsCreateInfoEqual(const VkImageCreateInfo &other_create_info) const;
    bool IsCreateInfoDedicatedAllocationImageAliasingCompatible(const VkImageCreateInfo &other_create_info) const;

    bool IsSwapchainImage() const { return create_from_swapchain != VK_NULL_HANDLE; }

    // TODO - need to understand if VkBindImageMemorySwapchainInfoKHR counts as "bound"
    bool HasBeenBound() const { return (MemoryState() != nullptr) || (bind_swapchain); }

    inline bool IsImageTypeEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.imageType == other_create_info.imageType;
    }
    inline bool IsFormatEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.format == other_create_info.format;
    }
    inline bool IsMipLevelsEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.mipLevels == other_create_info.mipLevels;
    }
    inline bool IsUsageEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.usage == other_create_info.usage;
    }
    inline bool IsSamplesEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.samples == other_create_info.samples;
    }
    inline bool IsTilingEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.tiling == other_create_info.tiling;
    }
    inline bool IsArrayLayersEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.arrayLayers == other_create_info.arrayLayers;
    }
    inline bool IsInitialLayoutEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.initialLayout == other_create_info.initialLayout;
    }
    inline bool IsSharingModeEqual(const VkImageCreateInfo &other_create_info) const {
        return create_info.sharingMode == other_create_info.sharingMode;
    }
    inline bool IsExtentEqual(const VkImageCreateInfo &other_create_info) const {
        return (create_info.extent.width == other_create_info.extent.width) &&
               (create_info.extent.height == other_create_info.extent.height) &&
               (create_info.extent.depth == other_create_info.extent.depth);
    }
    inline bool IsQueueFamilyIndicesEqual(const VkImageCreateInfo &other_create_info) const {
        return (create_info.queueFamilyIndexCount == other_create_info.queueFamilyIndexCount) &&
               (create_info.queueFamilyIndexCount == 0 ||
                memcmp(create_info.pQueueFamilyIndices, other_create_info.pQueueFamilyIndices,
                       create_info.queueFamilyIndexCount * sizeof(create_info.pQueueFamilyIndices[0])) == 0);
    }

    ~Image() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void SetSwapchain(std::shared_ptr<vvl::Swapchain> &swapchain, uint32_t swapchain_index);

    void Destroy() override;

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresourceLayers &sub) const {
        return GetEffectiveExtent(create_info, sub.aspectMask, sub.mipLevel);
    }

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresource &sub) const {
        return GetEffectiveExtent(create_info, sub.aspectMask, sub.mipLevel);
    }

    // Returns the effective extent of the provided subresource, adjusted for mip level and array depth.
    VkExtent3D GetEffectiveSubresourceExtent(const VkImageSubresourceRange &range) const {
        return GetEffectiveExtent(create_info, range.aspectMask, range.baseMipLevel);
    }

    VkImageSubresourceRange NormalizeSubresourceRange(const VkImageSubresourceRange &range) const {
        return ::NormalizeSubresourceRange(create_info, range);
    }

    void SetInitialLayoutMap();
    void SetImageLayout(const VkImageSubresourceRange &range, VkImageLayout layout);

    // This function is only used for comparing Imported External Dedicated Memory
    bool CompareCreateInfo(const Image &other) const;

  protected:
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    template <typename UnaryPredicate>
    bool AnyAliasBindingOf(const StateObject::NodeMap &bindings, const UnaryPredicate &pred) const {
        for (auto &entry : bindings) {
            if (entry.first.type == kVulkanObjectTypeImage) {
                auto state_object = entry.second.lock();
                if (state_object) {
                    auto other_image = static_cast<Image *>(state_object.get());
                    if ((other_image != this) && other_image->IsCompatibleAliasing(this)) {
                        if (pred(*other_image)) return true;
                    }
                }
            }
        }
        return false;
    }

    template <typename UnaryPredicate>
    bool AnyImageAliasOf(const UnaryPredicate &pred) const {
        // Look for another aliasing image and
        // ObjectBindings() is thread safe since returns by value, and once
        // the weak_ptr is successfully locked, the other image state won't
        // be freed out from under us.
        for (auto const &memory_state : GetBoundMemoryStates()) {
            if (AnyAliasBindingOf(memory_state->ObjectBindings(), pred)) return true;
        }
        return false;
    }

  private:
    std::variant<std::monostate, BindableNoMemoryTracker, BindableLinearMemoryTracker, BindableSparseMemoryTracker,
                 BindableMultiplanarMemoryTracker>
        tracker_;
};

// State for VkImageView objects.
// Parent -> child relationships in the object usage tree:
//    ImageView [N] -> [1] vv::Image
class ImageView : public StateObject {
  public:
    const vku::safe_VkImageViewCreateInfo safe_create_info;
    const VkImageViewCreateInfo &create_info;

    const VkImageSubresourceRange normalized_subresource_range;
    const image_layout_map::RangeGenerator range_generator;
    const VkSampleCountFlagBits samples;
    const uint32_t descriptor_format_bits;
    const VkSamplerYcbcrConversion samplerConversion;  // Handle of the ycbcr sampler conversion the image was created with, if any
    const VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props;
    const float min_lod;
    const VkFormatFeatureFlags2KHR format_features;
    const VkImageUsageFlags inherited_usage;  // from spec #resources-image-inherited-usage
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_imageview_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    std::shared_ptr<vvl::Image> image_state;
    const bool is_depth_sliced;

    ImageView(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle, const VkImageViewCreateInfo *ci,
              VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    ImageView(const ImageView &rh_obj) = delete;
    VkImageView VkHandle() const { return handle_.Cast<VkImageView>(); }

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        image_state->AddParent(this);
    }

    virtual ~ImageView() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    bool OverlapSubresource(const ImageView &compare_view) const;

    void Destroy() override;

    bool IsDepthSliced() const { return is_depth_sliced; }

    uint32_t GetAttachmentLayerCount() const;

    bool Invalid() const override { return Destroyed() || !image_state || image_state->Invalid(); }
};

struct SwapchainImage {
    vvl::Image *image_state = nullptr;
    bool acquired = false;
    std::shared_ptr<vvl::Semaphore> acquire_semaphore;
    std::shared_ptr<vvl::Fence> acquire_fence;

    // Each swapchain image keeps information about submissions associated with current present.
    // When the image is re-acquired later this information can be used to synchronize with
    // these submissions by using acquire fence.
    AcquireFenceSync acquire_fence_sync;
};

// State for VkSwapchainKHR objects.
// Parent -> child relationships in the object usage tree:
//    vvl::Swapchain [N] -> [1] vvl::Surface
//    However, only 1 swapchain for each surface can be !retired.
class Swapchain : public StateObject {
  public:
    const vku::safe_VkSwapchainCreateInfoKHR safe_create_info;
    const VkSwapchainCreateInfoKHR &create_info;

    std::vector<VkPresentModeKHR> present_modes;
    std::vector<SwapchainImage> images;
    bool retired = false;
    bool exclusive_full_screen_access;
    const bool shared_presentable;
    uint64_t max_present_id = 0;
    const vku::safe_VkImageCreateInfo image_create_info;

    std::shared_ptr<vvl::Surface> surface;
    Device &dev_data;
    uint32_t acquired_images = 0;

    Swapchain(Device &dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR handle);

    ~Swapchain() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSwapchainKHR VkHandle() const { return handle_.Cast<VkSwapchainKHR>(); }

    void PresentImage(uint32_t image_index, uint64_t present_id, const AcquireFenceSync &acquire_fence_sync);

    void ReleaseImage(uint32_t image_index);

    void AcquireImage(uint32_t image_index, const std::shared_ptr<vvl::Semaphore> &semaphore_state,
                      const std::shared_ptr<vvl::Fence> &fence_state);

    void Destroy() override;

    SwapchainImage GetSwapChainImage(uint32_t index) const;

    std::shared_ptr<const vvl::Image> GetSwapChainImageShared(uint32_t index) const;

  protected:
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;
};

}  // namespace vvl

struct GpuQueue {
    VkPhysicalDevice gpu;
    uint32_t queue_family_index;
};

inline bool operator==(GpuQueue const &lhs, GpuQueue const &rhs) {
    return (lhs.gpu == rhs.gpu && lhs.queue_family_index == rhs.queue_family_index);
}

namespace std {
template <>
struct hash<GpuQueue> {
    size_t operator()(GpuQueue gq) const throw() {
        return hash<uint64_t>()((uint64_t)(gq.gpu)) ^ hash<uint32_t>()(gq.queue_family_index);
    }
};
}  // namespace std

namespace vvl {

// Parent -> child relationships in the object usage tree:
//    vvl::Surface -> nothing
class Surface : public StateObject {
  public:
    Surface(VkSurfaceKHR handle) : StateObject(handle, kVulkanObjectTypeSurfaceKHR) {}

    ~Surface() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkSurfaceKHR VkHandle() const { return handle_.Cast<VkSurfaceKHR>(); }

    void Destroy() override;

    void RemoveParent(StateObject *parent_node) override;

    void SetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi, bool supported);
    bool GetQueueSupport(VkPhysicalDevice phys_dev, uint32_t qfi) const;

    void SetPresentModes(VkPhysicalDevice phys_dev, vvl::span<const VkPresentModeKHR> modes);
    std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice phys_dev) const;

    void SetFormats(VkPhysicalDevice phys_dev, std::vector<vku::safe_VkSurfaceFormat2KHR> &&fmts);
    vvl::span<const vku::safe_VkSurfaceFormat2KHR> GetFormats(bool get_surface_capabilities2, VkPhysicalDevice phys_dev,
                                                              const void *surface_info2_pnext) const;

    // Cache capabilities that do not depend on the present mode
    void UpdateCapabilitiesCache(VkPhysicalDevice phys_dev, const VkSurfaceCapabilitiesKHR &surface_caps);
    // Cache per present mode capabilities
    void UpdateCapabilitiesCache(VkPhysicalDevice phys_dev, const VkSurfaceCapabilities2KHR &surface_caps,
                                 VkPresentModeKHR present_mode);

    bool IsLastCapabilityQueryUsedPresentMode(VkPhysicalDevice phys_dev) const;
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice phys_dev, const void *surface_info_pnext) const;
    VkSurfaceCapabilitiesKHR GetPresentModeSurfaceCapabilities(VkPhysicalDevice phys_dev, VkPresentModeKHR present_mode) const;
    VkSurfacePresentScalingCapabilitiesEXT GetPresentModeScalingCapabilities(VkPhysicalDevice phys_dev,
                                                                             VkPresentModeKHR present_mode) const;
    std::vector<VkPresentModeKHR> GetCompatibleModes(VkPhysicalDevice phys_dev, VkPresentModeKHR present_mode) const;

    vvl::Swapchain *swapchain{nullptr};

  private:
    // Contains per present mode capabilities
    struct PresentModeInfo {
        VkPresentModeKHR present_mode;
        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::optional<VkSurfacePresentScalingCapabilitiesEXT> scaling_capabilities;
        std::optional<std::vector<VkPresentModeKHR>> compatible_present_modes;
    };
    // Cached information per physical device. Optional indicates if element is in the cache.
    //
    // NOTE: One of the reasons to cache surface caps is to prevent a false-positive
    // when the surface change happens (e.g. resize) after the surface caps are queried
    // and before the swapchain is created. The assumption is that with the current API,
    // the app can't do better than this (no atomicity between query and swapchain creation).
    // The caching ensures that validation sees the same surface state as the application.
    //
    // The priority is to avoid false-positives for correctly written application.
    // When the application behaves incorrectly (e.g. forgets to query surface caps after
    // it processed the resize event), then the caching can hide a problem, since validation
    // will think that application respects surface caps values.
    struct PhysDevCache {
        std::optional<std::vector<VkPresentModeKHR>> present_modes;
        std::optional<VkSurfaceCapabilitiesKHR> capabilities;
        std::vector<PresentModeInfo> present_mode_infos;
        bool last_capability_query_used_present_mode = false;

        const PresentModeInfo *GetPresentModeInfo(VkPresentModeKHR present_mode) const;
    };
    const PhysDevCache *GetPhysDevCache(VkPhysicalDevice phys_dev) const;

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }
    // TODO: make mutex shared, so multiple Validate can read simultaneously. Remove remaining mutations in Validate first
    mutable std::mutex lock_;
    mutable vvl::unordered_map<GpuQueue, bool> gpu_queue_support_;
    mutable vvl::unordered_map<VkPhysicalDevice, std::vector<vku::safe_VkSurfaceFormat2KHR>> formats_;

    vvl::unordered_map<VkPhysicalDevice, PhysDevCache> cache_;
};

}  // namespace vvl
