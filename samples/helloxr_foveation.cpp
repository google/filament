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

#include "helloxr_foveation.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <utility>

namespace helloxr {

struct Foveation::Impl {
    struct PendingDensityMap {
        VkImage colorImage = VK_NULL_HANDLE;
        VkImage densityMapImage = VK_NULL_HANDLE;
        VkExtent2D extent = {};
        uint32_t layerCount = 0;
    };

    bool check(XrResult result, char const* what) const {
        if (XR_SUCCEEDED(result)) {
            return true;
        }
        char resultName[XR_MAX_RESULT_STRING_SIZE] = {};
        if (instance == XR_NULL_HANDLE ||
                XR_FAILED(xrResultToString(instance, result, resultName))) {
            snprintf(resultName, sizeof(resultName), "XrResult(%d)", int(result));
        }
        XRLOG("%s failed: %s", what, resultName);
        return false;
    }

    template<typename Fn>
    bool load(char const* name, Fn* function) const {
        return check(xrGetInstanceProcAddr(instance, name,
                             reinterpret_cast<PFN_xrVoidFunction*>(function)),
                name) && *function != nullptr;
    }

        bool enumerateSwapchainImages(XrSwapchain swapchain, bool foveated,
            uint32_t const layerCount, std::vector<VkImage>* images) {
        uint32_t count = 0;
        if (!check(xrEnumerateSwapchainImages(swapchain, 0, &count, nullptr),
                    "xrEnumerateSwapchainImages")) {
            return false;
        }

        std::vector<XrSwapchainImageVulkanKHR> xrImages(count,
                { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
        std::vector<XrSwapchainImageFoveationVulkanFB> xrDensityMaps;
        if (foveated) {
            xrDensityMaps.assign(count, { XR_TYPE_SWAPCHAIN_IMAGE_FOVEATION_VULKAN_FB });
            for (uint32_t index = 0; index < count; ++index) {
                xrImages[index].next = &xrDensityMaps[index];
            }
        }
        if (!check(xrEnumerateSwapchainImages(swapchain, count, &count,
                           reinterpret_cast<XrSwapchainImageBaseHeader*>(xrImages.data())),
                    "xrEnumerateSwapchainImages")) {
            return false;
        }

        images->clear();
        images->reserve(count);
        for (auto const& image: xrImages) {
            images->push_back(image.image);
        }
        if (foveated) {
            pendingDensityMaps.reserve(pendingDensityMaps.size() + xrDensityMaps.size());
            for (uint32_t index = 0; index < count; ++index) {
                auto const& densityMap = xrDensityMaps[index];
                PendingDensityMap pending;
                pending.colorImage = xrImages[index].image;
                pending.densityMapImage = densityMap.image;
                pending.extent.width = densityMap.width;
                pending.extent.height = densityMap.height;
                pending.layerCount = layerCount;
                pendingDensityMaps.push_back(pending);
            }
        }
        return true;
    }

    bool enabled = false;
    XrInstance instance = XR_NULL_HANDLE;
    XrFoveationProfileFB profile = XR_NULL_HANDLE;
    PFN_xrCreateFoveationProfileFB createProfile = nullptr;
    PFN_xrDestroyFoveationProfileFB destroyProfile = nullptr;
    PFN_xrUpdateSwapchainFB updateSwapchain = nullptr;
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT enabledFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT,
    };
    std::vector<PendingDensityMap> pendingDensityMaps;
    std::vector<VkImageView> imageViews;
    std::vector<std::pair<VkImage, DensityMapView>> densityMapViews;
};

Foveation::Foveation() : mImpl(std::make_unique<Impl>()) {}

Foveation::~Foveation() = default;

void Foveation::requestExtensions(bool const requested,
        std::function<bool(char const*)> const& supports,
        std::vector<char const*>* extensions) {
    mImpl->enabled = false;
    if (!requested) {
        return;
    }

    char const* const required[] = {
        XR_FB_SWAPCHAIN_UPDATE_STATE_EXTENSION_NAME,
        XR_FB_FOVEATION_EXTENSION_NAME,
        XR_FB_FOVEATION_CONFIGURATION_EXTENSION_NAME,
        XR_FB_FOVEATION_VULKAN_EXTENSION_NAME,
    };
    mImpl->enabled = std::all_of(std::begin(required), std::end(required), supports);
    if (mImpl->enabled) {
        extensions->insert(extensions->end(), std::begin(required), std::end(required));
    } else {
        XRLOG("foveation disabled: the runtime is missing an FB foveation extension");
    }
}

void Foveation::configureVulkanDevice(VkPhysicalDevice physicalDevice,
        std::vector<VkExtensionProperties> const& supportedExtensions,
        VkPhysicalDeviceFeatures2* enabledFeatures,
        std::vector<char const*>* deviceExtensions) {
    if (!mImpl->enabled) {
        return;
    }

    bool const supportsExtension = std::any_of(supportedExtensions.begin(),
            supportedExtensions.end(), [](VkExtensionProperties const& extension) {
                return strcmp(extension.extensionName,
                               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) == 0;
            });
    if (!supportsExtension) {
        XRLOG("foveation disabled: the GPU does not support %s",
                VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        mImpl->enabled = false;
        return;
    }

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT available = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT,
    };
    VkPhysicalDeviceFeatures2 query = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &available,
    };
    bluevk::vkGetPhysicalDeviceFeatures2(physicalDevice, &query);
    if (available.fragmentDensityMap != VK_TRUE ||
            available.fragmentDensityMapNonSubsampledImages != VK_TRUE) {
        XRLOG("foveation disabled: ordinary Vulkan attachments cannot use density maps");
        mImpl->enabled = false;
        return;
    }

    deviceExtensions->push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    mImpl->enabledFeatures.fragmentDensityMap = VK_TRUE;
    mImpl->enabledFeatures.fragmentDensityMapNonSubsampledImages = VK_TRUE;
    mImpl->enabledFeatures.pNext = enabledFeatures->pNext;
    enabledFeatures->pNext = &mImpl->enabledFeatures;
}

bool Foveation::initialize(XrInstance instance, XrSession session,
        XrFoveationLevelFB const level, XrFoveationDynamicFB const dynamic) {
    if (!mImpl->enabled) {
        return true;
    }
    mImpl->instance = instance;
    if (!mImpl->load("xrCreateFoveationProfileFB", &mImpl->createProfile) ||
            !mImpl->load("xrDestroyFoveationProfileFB", &mImpl->destroyProfile) ||
            !mImpl->load("xrUpdateSwapchainFB", &mImpl->updateSwapchain)) {
        return false;
    }

    XrFoveationLevelProfileCreateInfoFB levelInfo = {
        XR_TYPE_FOVEATION_LEVEL_PROFILE_CREATE_INFO_FB
    };
    levelInfo.level = level;
    levelInfo.verticalOffset = 0.0f;
    levelInfo.dynamic = dynamic;
    XrFoveationProfileCreateInfoFB createInfo = {
        XR_TYPE_FOVEATION_PROFILE_CREATE_INFO_FB, &levelInfo
    };
    if (!mImpl->check(mImpl->createProfile(session, &createInfo, &mImpl->profile),
                "xrCreateFoveationProfileFB")) {
        return false;
    }
    XRLOG("foveation: level %d, dynamic %s", int(level),
            dynamic == XR_FOVEATION_DYNAMIC_LEVEL_ENABLED_FB ? "enabled" : "disabled");
    return true;
}

bool Foveation::createSwapchain(XrSession session, XrSwapchainCreateInfo const& createInfo,
    bool const foveated, XrSwapchain* swapchain, std::vector<VkImage>* images) {
    bool const useFoveation = foveated && mImpl->enabled;
    XrSwapchainCreateInfo chainedCreateInfo = createInfo;
    XrSwapchainCreateInfoFoveationFB foveationInfo = {
        XR_TYPE_SWAPCHAIN_CREATE_INFO_FOVEATION_FB
    };
    if (useFoveation) {
        foveationInfo.next = const_cast<void*>(createInfo.next);
        foveationInfo.flags = XR_SWAPCHAIN_CREATE_FOVEATION_FRAGMENT_DENSITY_MAP_BIT_FB;
        chainedCreateInfo.next = &foveationInfo;
    }
    if (!mImpl->check(xrCreateSwapchain(session, &chainedCreateInfo, swapchain),
                "xrCreateSwapchain(color)")) {
        return false;
    }

    if (useFoveation) {
        XrSwapchainStateFoveationFB const state = {
            XR_TYPE_SWAPCHAIN_STATE_FOVEATION_FB, nullptr, 0, mImpl->profile
        };
        if (!mImpl->check(mImpl->updateSwapchain(*swapchain,
                                  reinterpret_cast<XrSwapchainStateBaseHeaderFB const*>(&state)),
                    "xrUpdateSwapchainFB(foveation)")) {
            return false;
        }
    }
    return mImpl->enumerateSwapchainImages(
            *swapchain, useFoveation, createInfo.arraySize, images);
}

bool Foveation::createDensityMapViews(VkDevice device) {
    if (mImpl->pendingDensityMaps.empty()) {
        return true;
    }
    for (auto const& densityMap: mImpl->pendingDensityMaps) {
        VkImageViewCreateInfo const createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = densityMap.densityMapImage,
            .viewType = densityMap.layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                                   : VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8_UNORM,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, densityMap.layerCount },
        };
        VkImageView view = VK_NULL_HANDLE;
        VkResult const result = bluevk::vkCreateImageView(device, &createInfo, nullptr, &view);
        if (result != VK_SUCCESS) {
            XRLOG("foveation: vkCreateImageView returned %d", int(result));
            return false;
        }
        mImpl->imageViews.push_back(view);
        mImpl->densityMapViews.emplace_back(densityMap.colorImage,
                DensityMapView{ view, VK_FORMAT_R8G8_UNORM });
    }
        auto const& first = mImpl->pendingDensityMaps.front();
        XRLOG("foveation: %zu density maps at %ux%u", mImpl->pendingDensityMaps.size(),
            first.extent.width, first.extent.height);
        mImpl->pendingDensityMaps.clear();
    return true;
}

Foveation::DensityMapView Foveation::getDensityMap(VkImage colorImage) const noexcept {
    auto const entry = std::find_if(mImpl->densityMapViews.begin(),
            mImpl->densityMapViews.end(),
            [colorImage](auto const& candidate) { return candidate.first == colorImage; });
    return entry != mImpl->densityMapViews.end() ? entry->second : DensityMapView{};
}

void Foveation::destroyDensityMapViews(VkDevice device) noexcept {
    mImpl->densityMapViews.clear();
    for (VkImageView const imageView: mImpl->imageViews) {
        bluevk::vkDestroyImageView(device, imageView, nullptr);
    }
    mImpl->imageViews.clear();
}

void Foveation::destroyProfile() noexcept {
    if (mImpl->profile != XR_NULL_HANDLE && mImpl->destroyProfile != nullptr) {
        mImpl->destroyProfile(mImpl->profile);
        mImpl->profile = XR_NULL_HANDLE;
    }
}

} // namespace helloxr