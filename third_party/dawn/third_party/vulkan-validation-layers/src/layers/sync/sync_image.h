/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include "sync/sync_submit.h"
#include "state_tracker/image_state.h"

namespace syncval_state {

class ImageState : public vvl::Image {
  public:
    ImageState(const vvl::Device &dev_data, VkImage handle, const VkImageCreateInfo *pCreateInfo, VkFormatFeatureFlags2 features)
        : vvl::Image(dev_data, handle, pCreateInfo, features), opaque_base_address_(0U) {}

    ImageState(const vvl::Device &dev_data, VkImage handle, const VkImageCreateInfo *pCreateInfo, VkSwapchainKHR swapchain,
               uint32_t swapchain_index, VkFormatFeatureFlags2 features)
        : vvl::Image(dev_data, handle, pCreateInfo, swapchain, swapchain_index, features), opaque_base_address_(0U) {}
    bool IsLinear() const { return fragment_encoder->IsLinearImage(); }
    bool IsTiled() const { return !IsLinear(); }
    bool IsSimplyBound() const;

    void SetOpaqueBaseAddress(vvl::Device &dev_data);

    VkDeviceSize GetOpaqueBaseAddress() const { return opaque_base_address_; }
    bool HasOpaqueMapping() const { return 0U != opaque_base_address_; }
    VkDeviceSize GetResourceBaseAddress() const;
    ImageRangeGen MakeImageRangeGen(const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const;
    ImageRangeGen MakeImageRangeGen(const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                    const VkExtent3D &extent, bool is_depth_sliced) const;

  protected:
    VkDeviceSize opaque_base_address_ = 0U;
};

class ImageViewState : public vvl::ImageView {
  public:
    ImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle, const VkImageViewCreateInfo *ci,
                   VkFormatFeatureFlags2 ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props);
    const ImageState *GetImageState() const { return static_cast<const syncval_state::ImageState *>(image_state.get()); }
    ImageRangeGen MakeImageRangeGen(const VkOffset3D &offset, const VkExtent3D &extent,
                                    VkImageAspectFlags override_depth_stencil_aspect_mask = 0) const;
    const ImageRangeGen &GetFullViewImageRangeGen() const { return view_range_gen; }

  protected:
    ImageRangeGen MakeImageRangeGen() const;
    // All data members needs for MakeImageRangeGen() must be set before initializing view_range_gen... i.e. above this line.
    const ImageRangeGen view_range_gen;
};

class Swapchain : public vvl::Swapchain {
  public:
    Swapchain(vvl::Device &dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR handle);
    ~Swapchain() { Destroy(); }
    void RecordPresentedImage(PresentedImage &&presented_images);
    PresentedImage MovePresentedImage(uint32_t image_index);
    void GetPresentBatches(std::vector<QueueBatchContext::Ptr> &batches) const;
    std::shared_ptr<const Swapchain> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<Swapchain> shared_from_this() { return SharedFromThisImpl(this); }

  private:
    PresentedImages presented;  // Build this on demand
};
}  // namespace syncval_state
