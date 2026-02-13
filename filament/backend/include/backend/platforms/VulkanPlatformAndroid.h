/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PLATFORMS_VULKAN_PLATFORM_ANDROID_H
#define TNT_FILAMENT_BACKEND_PLATFORMS_VULKAN_PLATFORM_ANDROID_H

#include "AndroidNdk.h"

#include <backend/DriverEnums.h>
#include <backend/platforms/VulkanPlatform.h>

#include <utils/compiler.h>

#include <android/hardware_buffer.h>

namespace filament::backend {

class VulkanPlatformAndroid : public VulkanPlatform, public AndroidNdk {
public:
    ExternalImageHandle UTILS_PUBLIC createExternalImage(AHardwareBuffer const* buffer,
            bool sRGB) noexcept;

    struct UTILS_PUBLIC ExternalImageDescAndroid {
        uint32_t width;      // Texture width
        uint32_t height;     // Texture height
        TextureFormat format;// Texture format
        TextureUsage usage;  // Texture usage flags
    };

    VulkanPlatformAndroid();

    ~VulkanPlatformAndroid() noexcept override;

    ExternalImageDescAndroid UTILS_PUBLIC getExternalImageDesc(
            ExternalImageHandleRef externalImage) const noexcept;

    ExternalImageMetadata extractExternalImageMetadata(
            ExternalImageHandleRef image) const override;

    ImageData createVkImageFromExternal(ExternalImageHandleRef image) const override;

    /**
     * Converts a sync to an external file descriptor, if possible. Accepts an
     * opaque handle to a sync, as well as a pointer to where the fd should be
     * stored.
     * @param sync The sync to be converted to a file descriptor.
     * @param fd   A pointer to where the file descriptor should be stored.
     * @return `true` on success, `false` on failure. The default implementation
     *         returns `false`.
     */
    bool convertSyncToFd(Sync* sync, int* fd) const noexcept;

    int getOSVersion() const noexcept override;

    void terminate() override;

    Driver* createDriver(void* sharedContext,
        DriverConfig const& driverConfig) override;


    bool isCompositorTimingSupported() const noexcept override;

    bool queryCompositorTiming(SwapChain const* swapchain,
            CompositorTiming* outCompositorTiming) const noexcept override;

    bool setPresentFrameId(SwapChain const* swapchain, uint64_t frameId) noexcept override;

    bool queryFrameTimestamps(SwapChain const* swapchain, uint64_t frameId,
            FrameTimestamps* outFrameTimestamps) const noexcept override;

protected:
    ExtensionSet getSwapchainInstanceExtensions() const override;

    using SurfaceBundle = SurfaceBundle;
    SurfaceBundle createVkSurfaceKHR(void* nativeWindow, VkInstance instance,
            uint64_t flags) const noexcept override;

    VkExternalFenceHandleTypeFlagBits getFenceExportFlags() const noexcept override;

private:
    struct AndroidDetails;

    struct ExternalImageVulkanAndroid : public ExternalImage {
        AHardwareBuffer* aHardwareBuffer = nullptr;
        bool sRGB = false;

    protected:
        ~ExternalImageVulkanAndroid() override;
    };

    AndroidDetails& mAndroidDetails;
    int mOSVersion{};
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_PLATFORMS_VULKAN_PLATFORM_ANDROID_H
