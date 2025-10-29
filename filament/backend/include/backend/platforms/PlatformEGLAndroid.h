/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_EGL_ANDROID_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_EGL_ANDROID_H

#include "AndroidSwapChainHelper.h"

#include <backend/AcquiredImage.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>
#include <backend/platforms/OpenGLPlatform.h>
#include <backend/platforms/PlatformEGL.h>

#include <utils/android/PerformanceHintManager.h>
#include <utils/compiler.h>

#include <math/mat3.h>

#include <chrono>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

class ExternalStreamManagerAndroid;

/**
 * A concrete implementation of OpenGLPlatform and subclass of PlatformEGL that supports
 * EGL on Android. It adds Android streaming functionality to PlatformEGL.
 */
class PlatformEGLAndroid : public PlatformEGL {
public:

    PlatformEGLAndroid() noexcept;
    ~PlatformEGLAndroid() noexcept override;

    /**
     * Creates an ExternalImage from a EGLImageKHR
     */
    ExternalImageHandle UTILS_PUBLIC createExternalImage(AHardwareBuffer const* buffer, bool sRGB) noexcept;

    struct UTILS_PUBLIC ExternalImageDescAndroid {
        uint32_t width;      // Texture width
        uint32_t height;     // Texture height
        TextureFormat format;// Texture format
        TextureUsage usage;  // Texture usage flags
    };

    ExternalImageDescAndroid UTILS_PUBLIC getExternalImageDesc(ExternalImageHandle externalImage) noexcept;

    /**
     * Converts a sync to an external file descriptor, if possible. Accepts an
     * opaque handle to a sync, as well as a pointer to where the fd should be
     * stored.
     * @param sync The sync to be converted to a file descriptor.
     * @param fd   A pointer to where the file descriptor should be stored.
     * @return `true` on success, `false` on failure. The default implementation
     *         returns `false`.
     */
    bool convertSyncToFd(Sync* sync, int* fd) noexcept;

protected:
    struct {
        struct {
            bool ANDROID_presentation_time = false;
            bool ANDROID_get_frame_timestamps = false;
            bool ANDROID_native_fence_sync = false;
        } egl;
    } ext;

    // --------------------------------------------------------------------------------------------
    // Platform Interface

    /**
     * Returns the Android SDK version.
     * @return Android SDK version.
     */
    int getOSVersion() const noexcept override;

    Driver* createDriver(void* sharedContext,
            const DriverConfig& driverConfig) override;

    bool isCompositorTimingSupported() const noexcept override;

    bool queryCompositorTiming(SwapChain const* swapchain,
            CompositorTiming* outCompositorTiming) const noexcept override;

    bool setPresentFrameId(SwapChain const* swapchain, uint64_t frameId) noexcept override;

    bool queryFrameTimestamps(SwapChain const* swapchain, uint64_t frameId,
            FrameTimestamps* outFrameTimestamps) const noexcept override;

    // --------------------------------------------------------------------------------------------
    // OpenGLPlatform Interface

    struct SyncEGLAndroid : public Sync {
        EGLSyncKHR sync;
    };

    void terminate() noexcept override;

    void beginFrame(
            int64_t monotonic_clock_ns,
            int64_t refreshIntervalNs,
            uint32_t frameId) noexcept override;

    void preCommit() noexcept override;

    /**
     * Set the presentation time using `eglPresentationTimeANDROID`
     * @param presentationTimeInNanosecond
     */
    void setPresentationTime(int64_t presentationTimeInNanosecond) noexcept override;

    Stream* createStream(void* nativeStream) noexcept override;
    void destroyStream(Stream* stream) noexcept override;
    Sync* createSync() noexcept override;
    void destroySync(Sync* sync) noexcept override;
    void attach(Stream* stream, intptr_t tname) noexcept override;
    void detach(Stream* stream) noexcept override;
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept override;
    math::mat3f getTransformMatrix(Stream* stream) noexcept override;

    /**
     * Converts a AHardwareBuffer to EGLImage
     * @param source source.image is a AHardwareBuffer
     * @return source.image contains an EGLImage
     */
    AcquiredImage transformAcquiredImage(AcquiredImage source) noexcept override;

    ExternalTexture* createExternalImageTexture() noexcept override;
    void destroyExternalImageTexture(ExternalTexture* texture) noexcept override;

    struct ExternalImageEGLAndroid : public ExternalImageEGL {
        AHardwareBuffer* aHardwareBuffer = nullptr;
        uint32_t width;      // Texture width
        uint32_t height;     // Texture height
        TextureFormat format;// Texture format
        TextureUsage usage;  // Texture usage flags
        bool sRGB = false;

    protected:
        ~ExternalImageEGLAndroid() override;
    };

    bool setExternalImage(ExternalImageHandleRef externalImage,
            ExternalTexture* texture) noexcept override;
    bool setImage(ExternalImageEGLAndroid const* eglExternalImage,
            ExternalTexture* texture) noexcept;

    bool makeCurrent(ContextType type,
            SwapChain* drawSwapChain,
            SwapChain* readSwapChain) override;

    struct SwapChainEGLAndroid : public SwapChainEGL {
        SwapChainEGLAndroid(PlatformEGLAndroid const& platform,
                void* nativeWindow, uint64_t flags);
        SwapChainEGLAndroid(PlatformEGLAndroid const& platform,
                uint32_t width, uint32_t height, uint64_t flags);
        void terminate(PlatformEGLAndroid& platform);
        bool setPresentFrameId(uint64_t frameId) const noexcept;
        uint64_t getFrameId(uint64_t frameId) const noexcept;
    private:
        AndroidSwapChainHelper mImpl{};
    };

private:
    // prevent derived classes' implementations to call through
    [[nodiscard]] SwapChain* createSwapChain(void* nativeWindow, uint64_t flags) override;
    [[nodiscard]] SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags) override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;

    struct InitializeJvmForPerformanceManagerIfNeeded {
        InitializeJvmForPerformanceManagerIfNeeded();
    };

    struct ExternalTextureAndroid : public ExternalTexture {
        EGLImageKHR eglImage = EGL_NO_IMAGE;
    };

    int mOSVersion;
    ExternalStreamManagerAndroid& mExternalStreamManager;
    InitializeJvmForPerformanceManagerIfNeeded const mInitializeJvmForPerformanceManagerIfNeeded;
    utils::PerformanceHintManager mPerformanceHintManager;
    utils::PerformanceHintManager::Session mPerformanceHintSession;
    SwapChainEGLAndroid* mCurrentDrawSwapChain{};

    using clock = std::chrono::high_resolution_clock;
    clock::time_point mStartTimeOfActualWork;

    int32_t (*ANativeWindow_setProducerThrottlingEnabled)(ANativeWindow* window, bool enabled) = nullptr;
    int32_t (*ANativeWindow_isProducerThrottlingEnabled)(ANativeWindow* window, bool* outEnabled) = nullptr;
    bool mAssertNativeWindowIsValid = false;
    bool mHasProducerThrottlingControl = false;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_EGL_ANDROID_H
