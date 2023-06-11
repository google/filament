/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H
#define TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H

#include <backend/Platform.h>

#include <bluevk/BlueVK.h>
#include <utils/FixedCapacityVector.h>
#include <utils/PrivateImplementation.h>

#include <tuple>
#include <unordered_set>

namespace filament::backend {

using SwapChain = Platform::SwapChain;

/**
 * Private implementation details for the provided vulkan platform.
 */
struct VulkanPlatformPrivate;

/**
 * A Platform interface that creates a Vulkan backend.
 */
class VulkanPlatform : public Platform, utils::PrivateImplementation<VulkanPlatformPrivate> {
public:

    /**
     * A collection of handles to objects and metadata that comprises a Vulkan context. The client
     * can instantiate this struct and pass to Engine::Builder::sharedContext if they wishes to
     * share their vulkan context. This is specifically necessary if the client wishes to override
     * the swapchain API.
     */
    struct VulkanSharedContext {
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        uint32_t graphicsQueueFamilyIndex = 0xFFFFFFFF;
        // In the usual case, the client needs to allocate at least one more graphics queue
        // for Filament, and this index is the param to pass into vkGetDeviceQueue. In the case
        // where the gpu only has one graphics queue. Then the client needs to ensure that no
        // concurrent access can occur.
        uint32_t graphicsQueueIndex = 0xFFFFFFFF;
    };

    /**
     * Shorthand for the pointer to the Platform SwapChain struct, we use it also as a handle (i.e.
     * identifier for the swapchain).
     */
    using SwapChainPtr = Platform::SwapChain*;

    /**
     * Collection of images, formats, and extent (width/height) that defines the swapchain.
     */
    struct SwapChainBundle {
        utils::FixedCapacityVector<VkImage> colors;
        VkImage depth = VK_NULL_HANDLE;
        VkFormat colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D extent = {0, 0};
    };

    VulkanPlatform();

    ~VulkanPlatform() override;

    Driver* createDriver(void* sharedContext,
            Platform::DriverConfig const& driverConfig) noexcept override;

    int getOSVersion() const noexcept override {
        return 0;
    }

    // ----------------------------------------------------
    // ---------- Platform Customization options ----------
    /**
     * The client preference can be stored within the struct.  We allow for two specification of
     * preference:
     *     1) A substring to match against `VkPhysicalDeviceProperties.deviceName`.
     *     2) Index of the device in the list as returned by vkEnumeratePhysicalDevices.
     */
    struct GPUPreference {
        std::string deviceName;
        int8_t index = -1;
    };

    /**
     * Client can provide a preference over the GPU to use in the vulkan instance
     * @return            `GPUPreference` struct that indicates the client's preference
     */
    virtual GPUPreference getPreferredGPU() noexcept {
        return {};
    }
    // -------- End platform customization options --------
    // ----------------------------------------------------

    /**
     * Returns whether the platform supports sRGB swapchain. This is true by default, and the client
     * needs to override this method to specify otherwise.
     * @return            Whether the platform supports sRGB swapchain.
     */
    virtual bool isSRGBSwapChainSupported() const {
        return true;
    }

    /**
     * Get the images handles and format of the memory backing the swapchain. This should be called
     * after createSwapChain() or after recreateIfResized().
     * @param swapchain   The handle returned by createSwapChain()
     * @return            An array of VkImages
     */
    virtual SwapChainBundle getSwapChainBundle(SwapChainPtr handle);

    /**
     * Acquire the next image for rendering. The `index` will be written with an non-negative
     * integer that the backend can use to index into the `SwapChainBundle.colors` array. The
     * corresponding VkImage will be used as the output color attachment. The client should signal
     * the `clientSignal` semaphore when the image is ready to be used by the backend.
     * @param handle         The handle returned by createSwapChain()
     * @param clientSignal   The semaphore that the client will signal to indicate that the backend
     *                       may render into the image.
     * @param index          Pointer to memory that will be filled with the index that corresponding
     *                       to an image in the `SwapChainBundle.colors` array.
     * @return               Result of acquire
     */
    virtual VkResult acquire(SwapChainPtr handle, VkSemaphore clientSignal, uint32_t* index);

    /**
     * Present the image corresponding to `index` to the display. The client should wait on
     * `finishedDrawing` before presenting.
     * @param handle             The handle returned by createSwapChain()
     * @param index              Index that corresponding to an image in the
     *                           `SwapChainBundle.colors` array.
     * @param finishedDrawing    Backend passes in a semaphore that the client will signal to
     *                           indicate that the client may render into the image.
     * @return                   Result of present
     */
    virtual VkResult present(SwapChainPtr handle, uint32_t index, VkSemaphore finishedDrawing);

    /**
     * Check if the surface size has changed.
     * @param handle             The handle returned by createSwapChain()
     * @return                   Whether the swapchain has been resized
     */
    virtual bool hasResized(SwapChainPtr handle);

    /**
     * Carry out a recreation of the swapchain.
     * @param handle             The handle returned by createSwapChain()
     * @return                   Result of the recreation
     */
    virtual VkResult recreate(SwapChainPtr handle);

    /**
     * Create a swapchain given a platform window, or if given a null `nativeWindow`, then we
     * try to create a headless swapchain with the given `extent`.
     * @param flags     Optional parameters passed to the client as defined in
     *                  Filament::SwapChain.h.
     * @param extent    Optional width and height that indicates the size of the headless swapchain.
     * @return          Result of the operation
     */
    virtual SwapChainPtr createSwapChain(void* nativeWindow, uint64_t flags = 0,
            VkExtent2D extent = {0, 0});

    /**
     * Destroy the swapchain.
     * @param handle    The handle returned by createSwapChain()
     */
    virtual void destroy(SwapChainPtr handle);

    /**
     * Clean up any resources owned by the Platform. For example, if the Vulkan instance handle was
     * generated by the platform, we need to clean it up in this method.
     */
    virtual void terminate();

    /**
     * @return The instance (VkInstance) for the Vulkan backend.
     */
    VkInstance getInstance() const noexcept;

    /**
     * @return The logical device (VkDevice) that was selected as the backend device.
     */
    VkDevice getDevice() const noexcept;

    /**
     * @return The physical device (i.e gpu) that was selected as the backend physical device.
     */
    VkPhysicalDevice getPhysicalDevice() const noexcept;

    /**
     * @return The family index of the graphics queue selected for the Vulkan backend.
     */
    uint32_t getGraphicsQueueFamilyIndex() const noexcept;

    /**
     * @return The index of the graphics queue (if there are multiple graphics queues)
     *         selected for the Vulkan backend.
     */
    uint32_t getGraphicsQueueIndex() const noexcept;

    /**
     * @return The queue that was selected for the Vulkan backend.
     */
    VkQueue getGraphicsQueue() const noexcept;

private:
    // Platform dependent helper methods
    using ExtensionSet = std::unordered_set<std::string_view>;
    static ExtensionSet getRequiredInstanceExtensions();

    using SurfaceBundle = std::tuple<VkSurfaceKHR, VkExtent2D>;
    static SurfaceBundle createVkSurfaceKHR(void* nativeWindow, VkInstance instance,
            uint64_t flags) noexcept;

    friend struct VulkanPlatformPrivate;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H
