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

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/PrivateImplementation.h>

#include <tuple>
#include <unordered_set>

#include <stddef.h>
#include <stdint.h>

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

    struct ExtensionHashFn {
        std::size_t operator()(utils::CString const& s) const noexcept {
            return std::hash<std::string>{}(s.data());
        }
    };
    // Utility for managing device or instance extensions during initialization.
    using ExtensionSet = std::unordered_set<utils::CString, ExtensionHashFn>;

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
        bool debugUtilsSupported = false;
        bool debugMarkersSupported = false;
        bool multiviewSupported = false;
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
        uint32_t layerCount = 1;
        bool isProtected = false;
    };

    struct ImageSyncData {
        static constexpr uint32_t INVALID_IMAGE_INDEX = UINT32_MAX;

        // The index of the next image as returned by vkAcquireNextImage or equivalent.
        uint32_t imageIndex = INVALID_IMAGE_INDEX;

        // Semaphore to be signaled once the image is available.
        VkSemaphore imageReadySemaphore = VK_NULL_HANDLE;

        // A function called right before vkQueueSubmit. After this call, the image must be 
        // available. This pointer can be null if imageReadySemaphore is not VK_NULL_HANDLE.
        std::function<void(SwapChainPtr handle)> explicitImageReadyWait = nullptr;
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
    struct Customization {
        /**
         * The client can specify the GPU (i.e. VkDevice) for the platform. We allow the
         * following preferences:
         *     1) A substring to match against `VkPhysicalDeviceProperties.deviceName`. Empty string
         *        by default.
         *     2) Index of the device in the list as returned by
         *        `vkEnumeratePhysicalDevices`. -1 by default to indicate no preference.
         */
        struct GPUPreference {
            utils::CString deviceName;
            int8_t index = -1;
        } gpu;

        /**
         * Whether the platform supports sRGB swapchain. Default is true.
         */
        bool isSRGBSwapChainSupported = true;

        /**
         * When the platform window is resized, we will flush and wait on the command queues
         * before recreating the swapchain. Default is true.
         */
        bool flushAndWaitOnWindowResize = true;

        /**
         * Whether the swapchain image should be transitioned to a layout suitable for
         * presentation. Default is true.
         */
        bool transitionSwapChainImageLayoutForPresent = true;
    };

    /**
     * Client can override to indicate customized behavior or parameter for their platform.
     * @return            `Customization` struct that indicates the client's platform
     *                    customizations.
     */
    virtual Customization getCustomization() const noexcept {
        return {};
    }

    // -------- End platform customization options --------
    // ----------------------------------------------------

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
     * @param outImageSyncData The synchronization data used for image readiness
     * @return               Result of acquire
     */
    virtual VkResult acquire(SwapChainPtr handle, ImageSyncData* outImageSyncData);

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
     * Check if the surface is protected.
     * @param handle             The handle returned by createSwapChain()
     * @return                   Whether the swapchain is protected
     */
    virtual bool isProtected(SwapChainPtr handle);

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
     * Allows implementers to provide instance extensions that they'd like to include in the
     * instance creation.
     * @return          A set of extensions to enable for the instance.
     */
    virtual ExtensionSet getRequiredInstanceExtensions() { return {}; }

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

    /**
    * @return The family index of the protected graphics queue selected for the
    *          Vulkan backend.
    */
    uint32_t getProtectedGraphicsQueueFamilyIndex() const noexcept;

    /**
     * @return The index of the protected graphics queue (if there are multiple
     *          graphics queues) selected for the Vulkan backend.
     */
    uint32_t getProtectedGraphicsQueueIndex() const noexcept;

    /**
     * @return The protected queue that was selected for the Vulkan backend.
     */
    VkQueue getProtectedGraphicsQueue() const noexcept;

    struct ExternalImageMetadata {
        /**
         * The width of the external image
         */
        uint32_t width;

        /**
         * The height of the external image
         */
        uint32_t height;

        /**
         * The layerCount of the external image
         */
        uint32_t layerCount;

        /**
         * The layer count of the external image
         */
        uint32_t layers;

        /**
         * The format of the external image
         */
        VkFormat format;

        /**
         * An external buffer can be protected. This tells you if it is.
         */
        bool isProtected;

        /**
         * The type of external format (opaque int) if used.
         */
        uint64_t externalFormat;

        /**
         * Image usage
         */
        VkImageUsageFlags usage;

        /**
         * Allocation size
         */
        VkDeviceSize allocationSize;

        /**
         * Heap information
         */
        uint32_t memoryTypeBits;
    };
    virtual ExternalImageMetadata getExternalImageMetadata(ExternalImageHandleRef externalImage);

    using ImageData = std::pair<VkImage, VkDeviceMemory>;
    virtual ImageData createExternalImageData(ExternalImageHandleRef externalImage,
            const ExternalImageMetadata& metadata);

private:
    static ExtensionSet getSwapchainInstanceExtensions();

    static ExternalImageMetadata getExternalImageMetadataImpl(ExternalImageHandleRef externalImage,
            VkDevice device);

    static ImageData createExternalImageDataImpl(ExternalImageHandleRef externalImage,
            VkDevice device, const ExternalImageMetadata& metadata);

    // Platform dependent helper methods
    using SurfaceBundle = std::tuple<VkSurfaceKHR, VkExtent2D>;
    static SurfaceBundle createVkSurfaceKHR(void* nativeWindow, VkInstance instance,
            uint64_t flags) noexcept;

    friend struct VulkanPlatformPrivate;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H
