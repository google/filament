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

#include <backend/AcquiredImage.h>
#include <backend/Platform.h>
#include <backend/platforms/OpenGLPlatform.h>
#include <backend/platforms/PlatformEGL.h>

#include <utils/android/PerformanceHintManager.h>

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

protected:

    // --------------------------------------------------------------------------------------------
    // Platform Interface

    /**
     * Returns the Android SDK version.
     * @return Android SDK version.
     */
    int getOSVersion() const noexcept override;

    Driver* createDriver(void* sharedContext,
            const Platform::DriverConfig& driverConfig) noexcept override;

    // --------------------------------------------------------------------------------------------
    // OpenGLPlatform Interface

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
    void attach(Stream* stream, intptr_t tname) noexcept override;
    void detach(Stream* stream) noexcept override;
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept override;

    /**
     * Converts a AHardwareBuffer to EGLImage
     * @param source source.image is a AHardwareBuffer
     * @return source.image contains an EGLImage
     */
    AcquiredImage transformAcquiredImage(AcquiredImage source) noexcept override;

protected:
    bool makeCurrent(ContextType type,
            SwapChain* drawSwapChain,
            SwapChain* readSwapChain) noexcept override;

private:
    struct InitializeJvmForPerformanceManagerIfNeeded {
        InitializeJvmForPerformanceManagerIfNeeded();
    };

    int mOSVersion;
    ExternalStreamManagerAndroid& mExternalStreamManager;
    InitializeJvmForPerformanceManagerIfNeeded const mInitializeJvmForPerformanceManagerIfNeeded;
    utils::PerformanceHintManager mPerformanceHintManager;
    utils::PerformanceHintManager::Session mPerformanceHintSession;

    using clock = std::chrono::high_resolution_clock;
    clock::time_point mStartTimeOfActualWork;

    void* mNativeWindowLib = nullptr;
    int32_t (*ANativeWindow_getBuffersDefaultDataSpace)(ANativeWindow* window) = nullptr;
    bool mAssertNativeWindowIsValid = false;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_EGL_ANDROID_H
