/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HELLOXR_FOVEATION_H
#define HELLOXR_FOVEATION_H

#include "helloxr_features.h"

#include <functional>
#include <memory>
#include <vector>

#include <stddef.h>

namespace helloxr {

class Foveation {
public:
    struct DensityMapView {
        VkImageView imageView = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    Foveation();
    ~Foveation();

    Foveation(Foveation const&) = delete;
    Foveation& operator=(Foveation const&) = delete;

    void requestExtensions(bool requested,
            std::function<bool(char const*)> const& supports,
            std::vector<char const*>* extensions);

    void configureVulkanDevice(VkPhysicalDevice physicalDevice,
            std::vector<VkExtensionProperties> const& supportedExtensions,
            VkPhysicalDeviceFeatures2* enabledFeatures,
            std::vector<char const*>* deviceExtensions);

    bool initialize(XrInstance instance, XrSession session, XrFoveationLevelFB level,
            XrFoveationDynamicFB dynamic);

    bool createSwapchain(XrSession session, XrSwapchainCreateInfo const& createInfo,
            bool foveated, XrSwapchain* swapchain, std::vector<VkImage>* images);

    bool createDensityMapViews(VkDevice device);

    DensityMapView getDensityMap(VkImage colorImage) const noexcept;

    void destroyDensityMapViews(VkDevice device) noexcept;
    void destroyProfile() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace helloxr

#endif // HELLOXR_FOVEATION_H