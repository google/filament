/*
 * Copyright (C) 2015 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_BACKEND_PLATFORM_H
#define TNT_FILAMENT_BACKEND_PLATFORM_H

#include <utils/CString.h>
#include <utils/compiler.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <memory>
#include <mutex>

namespace filament::backend {

class CallbackHandler;
class Driver;

/**
 * Platform is an interface that abstracts how the backend (also referred to as Driver) is
 * created. The backend provides several common Platform concrete implementations, which are
 * selected automatically. It is possible however to provide a custom Platform when creating
 * the filament Engine.
 */
class UTILS_PUBLIC Platform {
public:
    struct SwapChain {};
    struct Fence {};
    struct Stream {};
    struct Sync {};

    using SyncCallback = void(*)(Sync* UTILS_NONNULL sync, void* UTILS_NULLABLE userData);

    class ExternalImageHandle;

    class ExternalImage {
        friend class ExternalImageHandle;
        std::atomic_uint32_t mRefCount{0};
    protected:
        virtual ~ExternalImage() noexcept;
    };

    class ExternalImageHandle {
        ExternalImage* UTILS_NULLABLE mTarget = nullptr;
        static void incref(ExternalImage* UTILS_NULLABLE p) noexcept;
        static void decref(ExternalImage* UTILS_NULLABLE p) noexcept;

    public:
        ExternalImageHandle() noexcept;
        ~ExternalImageHandle() noexcept;
        explicit ExternalImageHandle(ExternalImage* UTILS_NULLABLE p) noexcept;
        ExternalImageHandle(ExternalImageHandle const& rhs) noexcept;
        ExternalImageHandle(ExternalImageHandle&& rhs) noexcept;
        ExternalImageHandle& operator=(ExternalImageHandle const& rhs) noexcept;
        ExternalImageHandle& operator=(ExternalImageHandle&& rhs) noexcept;

        bool operator==(const ExternalImageHandle& rhs) const noexcept {
            return mTarget == rhs.mTarget;
        }
        explicit operator bool() const noexcept { return mTarget != nullptr; }

        ExternalImage* UTILS_NULLABLE get() noexcept { return mTarget; }
        ExternalImage const* UTILS_NULLABLE get() const noexcept { return mTarget; }

        ExternalImage* UTILS_NULLABLE operator->() noexcept { return mTarget; }
        ExternalImage const* UTILS_NULLABLE operator->() const noexcept { return mTarget; }

        ExternalImage& operator*() noexcept { return *mTarget; }
        ExternalImage const& operator*() const noexcept { return *mTarget; }

        void clear() noexcept;
        void reset(ExternalImage* UTILS_NULLABLE p) noexcept;

    private:
        friend utils::io::ostream& operator<<(utils::io::ostream& out,
                ExternalImageHandle const& handle);
    };

    using ExternalImageHandleRef = ExternalImageHandle const&;

    struct CompositorTiming {
        /** duration in nanosecond since epoch of std::steady_clock */
        using time_point_ns = int64_t;
        /** duration in nanosecond on the std::steady_clock */
        using duration_ns = int64_t;
        static constexpr time_point_ns INVALID = -1;    //!< value not supported
        /**
         * The timestamp [ns] since epoch of the next time the compositor will begin composition.
         * This is effectively the deadline for when the compositor must receive a newly queued
         * frame.
         */
        time_point_ns compositeDeadline;

        /**
         * The time delta [ns] between subsequent composition events.
         */
        duration_ns compositeInterval;

        /**
         * The time delta [ns] between the start of composition and the expected present time of
         * that composition. This can be used to estimate the latency of the actual present time.
         */
        duration_ns compositeToPresentLatency;

        /**
         * The timestamp [ns] since epoch of the system's expected presentation time.
         * INVALID if not supported.
         */
        time_point_ns expectedPresentTime;

        /**
         * The timestamp [ns] since epoch of the current frame's start (i.e. vsync)
         * INVALID if not supported.
         */
        time_point_ns frameTime;

        /**
         * The timestamp [ns] since epoch of the current frame's deadline
         * INVALID if not supported.
         */
        time_point_ns frameTimelineDeadline;
    };

    struct FrameTimestamps {
        /** duration in nanosecond since epoch of std::steady_clock */
        using time_point_ns = int64_t;
        static constexpr time_point_ns INVALID = -1;    //!< value not supported
        static constexpr time_point_ns PENDING = -2;    //!< value not yet available

        /**
         * The time the application requested this frame be presented.
         * If the application does not request a presentation time explicitly,
         * this will correspond to buffer's queue time.
         */
        time_point_ns requestedPresentTime;

        /**
         * The time when all the application's rendering to the surface was completed.
         */
        time_point_ns acquireTime;

        /**
         * The time when the compositor selected this frame as the one to use for the next
         * composition. This is the earliest indication that the frame was submitted in time.
         */
        time_point_ns latchTime;

        /**
         * The first time at which the compositor began preparing composition for this frame.
         * Zero if composition was handled by the display and the compositor didn't do any
         * rendering.
         */
        time_point_ns firstCompositionStartTime;

        /**
         * The last time at which the compositor began preparing composition for this frame, for
         * frames composited more than once. Zero if composition was handled by the display and the
         * compositor didn't do any rendering.
         */
        time_point_ns lastCompositionStartTime;

        /**
         * The time at which the compositor's rendering work for this frame finished. This will be
         * INVALID if composition was handled by the display and the compositor didn't do any
         * rendering.
         */
        time_point_ns gpuCompositionDoneTime;

        /**
         * The time at which this frame started to scan out to the physical display.
         */
        time_point_ns displayPresentTime;

        /**
         * The time when the buffer became available for reuse as a buffer the client can target
         * without blocking. This is generally the point when all read commands of the buffer have
         * been submitted, but not necessarily completed.
         */
        time_point_ns dequeueReadyTime;

        /**
         * The time at which all reads for the purpose of display/composition were completed for
         * this frame.
         */
        time_point_ns releaseTime;
    };

    /**
     * The type of technique for stereoscopic rendering. (Note that the materials used will need to
     * be compatible with the chosen technique.)
     */
    enum class StereoscopicType : uint8_t {
        /**
         * No stereoscopic rendering
         */
        NONE,
        /**
         * Stereoscopic rendering is performed using instanced rendering technique.
         */
        INSTANCED,
        /**
         * Stereoscopic rendering is performed using the multiview feature from the graphics
         * backend.
         */
        MULTIVIEW,
    };

    /**
     * This controls the priority level for GPU work scheduling, which helps prioritize the
     * submitted GPU work and enables preemption.
     */
    enum class GpuContextPriority : uint8_t {
        /**
         * Backend default GPU context priority (typically MEDIUM)
         */
        DEFAULT,
        /**
         * For non-interactive, deferrable workloads. This should not interfere with standard
         * applications.
         */
        LOW,
        /**
         * The default priority level for standard applications.
         */
        MEDIUM,
        /**
         * For high-priority, latency-sensitive workloads that are more important than standard
         * applications.
         */
        HIGH,
        /**
         * The highest priority, intended for system-critical, real-time applications where missing
         * deadlines is unacceptable (e.g., VR/AR compositors or other system-critical tasks).
         */
        REALTIME,
    };

    /**
     * Defines how asynchronous operations are handled by the engine.
     */
    enum class AsynchronousMode : uint8_t {
        /**
         * Asynchronous operations are disabled. This is the default.
         */
        NONE,

        /**
         * Attempts to use a dedicated worker thread for asynchronous tasks. If threading is not
         * supported by the platform, it automatically falls back to using an amortization strategy.
         */
        THREAD_PREFERRED,

        /**
         * Uses an amortization strategy, processing a small number of asynchronous tasks during
         * each engine update cycle.
         */
        AMORTIZATION,
    };

    struct DriverConfig {
        /**
         * Size of handle arena in bytes. Setting to 0 indicates default value is to be used.
         * Driver clamps to valid values.
         */
        size_t handleArenaSize = 0;

        size_t metalUploadBufferSizeBytes = 512 * 1024;

        /**
         * Set to `true` to forcibly disable parallel shader compilation in the backend.
         * Currently only honored by the GL and Metal backends, and the Vulkan backend
         * when some experimental features are enabled.
         */
        bool disableParallelShaderCompile = false;

        /**
         * Set to `true` to forcibly disable amortized shader compilation in the backend.
         * Currently only honored by the GL backend.
         */
        bool disableAmortizedShaderCompile = true;

        /**
         * Disable backend handles use-after-free checks.
         */
        bool disableHandleUseAfterFreeCheck = false;

        /**
         * Disable backend handles tags for heap allocated (fallback) handles
         */
        bool disableHeapHandleTags = false;

        /**
         * Force GLES2 context if supported, or pretend the context is ES2. Only meaningful on
         * GLES 3.x backends.
         */
        bool forceGLES2Context = false;

        /**
         * Sets the technique for stereoscopic rendering.
         */
        StereoscopicType stereoscopicType = StereoscopicType::NONE;

        /*
         * The number of eyes to render when stereoscopic rendering is enabled. Supported values are
         * between 1 and Engine::getMaxStereoscopicEyes() (inclusive).
         */
        uint8_t stereoscopicEyeCount = 2;

        /**
         * Assert the native window associated to a SwapChain is valid when calling makeCurrent().
         * This is only supported for:
         *      - PlatformEGLAndroid
         */
        bool assertNativeWindowIsValid = false;

        /**
         * The action to take if a Drawable cannot be acquired. If true, the
         * frame is aborted instead of panic. This is only supported for:
         *      - PlatformMetal
         */
        bool metalDisablePanicOnDrawableFailure = false;

        /**
         * GPU context priority level. Controls GPU work scheduling and preemption.
         * This is only supported for:
         *      - PlatformEGL
         */
        GpuContextPriority gpuContextPriority = GpuContextPriority::DEFAULT;

        /**
         * Enables asynchronous pipeline cache preloading, if supported on this device.
         * This is only supported for:
         *      - VulkanPlatform
         * When the following device extensions are available:
         *      - VK_KHR_dynamic_rendering
         *      - VK_EXT_vertex_input_dynamic_state
         * Should be enabled only for devices where it has been shown this is effective.
         */
        bool vulkanEnableAsyncPipelineCachePrewarming = false;

        /**
         * Bypass the staging buffer because the device is of Unified Memory Architecture.
         * This is only supported for:
         *      - VulkanPlatform
         */
        bool vulkanEnableStagingBufferBypass = false;

        /**
         * Asynchronous mode for the engine. Defines how asynchronous operations are handled.
         */
        AsynchronousMode asynchronousMode = AsynchronousMode::NONE;
    };

    Platform() noexcept;

    virtual ~Platform() noexcept;

    /**
     * Queries the underlying OS version.
     * @return The OS version.
     */
    virtual int getOSVersion() const noexcept = 0;

    /**
     * Creates and initializes the low-level API (e.g. an OpenGL context or Vulkan instance),
     * then creates the concrete Driver.
     * The caller takes ownership of the returned Driver* and must destroy it with delete.
     *
     * @param sharedContext an optional shared context. This is not meaningful with all graphic
     *                      APIs and platforms.
     *                      For EGL platforms, this is an EGLContext.
     *
     * @param driverConfig  specifies driver initialization parameters
     *
     * @return nullptr on failure, or a pointer to the newly created driver.
     */
    virtual Driver* UTILS_NULLABLE createDriver(void* UTILS_NULLABLE sharedContext,
            const DriverConfig& driverConfig) = 0;

    /**
     * Processes the platform's event queue when called from its primary event-handling thread.
     *
     * Internally, Filament might need to call this when waiting on a fence. It is only implemented
     * on platforms that need it, such as macOS + OpenGL. Returns false if this is not the main
     * thread, or if the platform does not need to perform any special processing.
     */
    virtual bool pumpEvents() noexcept;

    // --------------------------------------------------------------------------------------------
    // Swapchain timing APIs

    /**
     * Whether this platform supports compositor timing querying.
     *
     * @return true if this Platform supports compositor timings, false otherwise [default]
     * @see queryCompositorTiming()
     * @see setPresentFrameId()
     * @see queryFrameTimestamps()
     */
    virtual bool isCompositorTimingSupported() const noexcept;

    /**
     * If compositor timing is supported, fills the provided CompositorTiming structure
     * with timing information form the compositor the swapchain's native window is using.
     * The swapchain'snative window must be valid (i.e. not a headless swapchain).
     * @param swapchain to query the compositor timing from
     * @return true on success, false otherwise (e.g. if not supported)
     * @see isCompositorTimingSupported()
     */
    virtual bool queryCompositorTiming(SwapChain const* UTILS_NONNULL swapchain,
            CompositorTiming* UTILS_NONNULL outCompositorTiming) const noexcept;

    /**
     * Associate a generic frameId which must be monotonically increasing (albeit not strictly) with
     * the next frame to be presented on the specified swapchain.
     *
     * This must be called from the backend thread.
     *
     * @param swapchain
     * @param frameId
     * @return true on success, false otherwise
     * @see isCompositorTimingSupported()
     * @see queryFrameTimestamps()
     */
    virtual bool setPresentFrameId(SwapChain const* UTILS_NONNULL swapchain,
            uint64_t frameId) noexcept;

    /**
     * If compositor timing is supported, fills the provided FrameTimestamps structure
     * with timing information of a given frame, identified by the frame id, of the specified
     * swapchain. The system only keeps a limited history of frames timings.
     *
     * This API is thread safe and can be called from any thread.
     *
     * @param swapchain swapchain to query the timestamps of
     * @param frameId frame we're interested it
     * @param outFrameTimestamps output structure receiving the timestamps
     * @return true if successful, false otherwise
     * @see isCompositorTimingSupported()
     * @see setPresentFrameId()
     */
    virtual bool queryFrameTimestamps(SwapChain const* UTILS_NONNULL swapchain,
            uint64_t frameId, FrameTimestamps* UTILS_NONNULL outFrameTimestamps) const noexcept;

    // --------------------------------------------------------------------------------------------
    // Caching APIs

    /**
     * InsertBlobFunc is an Invocable to an application-provided function that a
     * backend implementation may use to insert a key/value pair into the
     * cache.
     */
    using InsertBlobFunc = utils::Invocable<
            void(const void* UTILS_NONNULL key, size_t keySize,
                    const void* UTILS_NONNULL value, size_t valueSize)>;

    /*
     * RetrieveBlobFunc is an Invocable to an application-provided function that a
     * backend implementation may use to retrieve a cached value from the
     * cache.
     */
    using RetrieveBlobFunc = utils::Invocable<
            size_t(const void* UTILS_NONNULL key, size_t keySize,
                    void* UTILS_NONNULL value, size_t valueSize)>;

    /**
     * Sets the callback functions that the backend can use to interact with caching functionality
     * provided by the application.
     *
     * Cache functions may only be specified once during the lifetime of a
     * Platform.  The <insert> and <retrieve> Invocables may be called at any time and
     * from any thread from the time at which setBlobFunc is called until the time that Platform
     * is destroyed. Concurrent calls to these functions from different threads is also allowed.
     * Either function can be null.
     *
     * @param insertBlob    an Invocable that inserts a new value into the cache and associates
     *                      it with the given key
     * @param retrieveBlob  an Invocable that retrieves from the cache the value associated with a
     *                      given key
     */
    void setBlobFunc(InsertBlobFunc&& insertBlob, RetrieveBlobFunc&& retrieveBlob) noexcept;

    /**
     * @return true if insertBlob is valid.
     */
    bool hasInsertBlobFunc() const noexcept;

    /**
     * @return true if retrieveBlob is valid.
     */
    bool hasRetrieveBlobFunc() const noexcept;

    /**
     * @return true if either of insertBlob or retrieveBlob are valid.
     */
    bool hasBlobFunc() const noexcept {
        return hasInsertBlobFunc() || hasRetrieveBlobFunc();
    }

    /**
     * To insert a new binary value into the cache and associate it with a given
     * key, the backend implementation can call the application-provided callback
     * function insertBlob.
     *
     * No guarantees are made as to whether a given key/value pair is present in
     * the cache after the set call.  If a different value has been associated
     * with the given key in the past then it is undefined which value, if any, is
     * associated with the key after the set call.  Note that while there are no
     * guarantees, the cache implementation should attempt to cache the most
     * recently set value for a given key.
     *
     * @param key           pointer to the beginning of the key data that is to be inserted
     * @param keySize       specifies the size in byte of the data pointed to by <key>
     * @param value         pointer to the beginning of the value data that is to be inserted
     * @param valueSize     specifies the size in byte of the data pointed to by <value>
     */
    void insertBlob(const void* UTILS_NONNULL key, size_t keySize,
            const void* UTILS_NONNULL value, size_t valueSize);

    /**
     * To retrieve the binary value associated with a given key from the cache, a
     * the backend implementation can call the application-provided callback
     * function retrieveBlob.
     *
     * If the cache contains a value for the given key and its size in bytes is
     * less than or equal to <valueSize> then the value is written to the memory
     * pointed to by <value>.  Otherwise nothing is written to the memory pointed
     * to by <value>.
     *
     * @param key          pointer to the beginning of the key
     * @param keySize      specifies the size in bytes of the binary key pointed to by <key>
     * @param value        pointer to a buffer to receive the cached binary data, if it exists
     * @param valueSize    specifies the size in bytes of the memory pointed to by <value>
     * @return             If the cache contains a value associated with the given key then the
     *                     size of that binary value in bytes is returned. Otherwise 0 is returned.
     */
    size_t retrieveBlob(const void* UTILS_NONNULL key, size_t keySize,
            void* UTILS_NONNULL value, size_t valueSize);

    // --------------------------------------------------------------------------------------------
    // Debugging APIs

    using DebugUpdateStatFunc = utils::Invocable<void(const char* UTILS_NONNULL key,
            uint64_t intValue, utils::CString stringValue)>;

    /**
     * Sets the callback function that the backend can use to update backend-specific statistics
     * to aid with debugging. This callback is guaranteed to be called on the Filament driver
     * thread.
     *
     * The callback signature is (key, intValue, stringValue). Note that for any given call,
     * only one of the value parameters (intValue or stringValue) will be meaningful, depending on
     * the specific key.
     *
     * IMPORTANT_NOTE: because the callback is called on the driver thread, only quick, non-blocking
     * work should be done inside it. Furthermore, no graphics API calls (such as GL calls) should
     * be made, which could interfere with Filament's driver state.
     *
     * @param debugUpdateStat   an Invocable that updates debug statistics
     */
    void setDebugUpdateStatFunc(DebugUpdateStatFunc&& debugUpdateStat) noexcept;

    /**
     * @return true if debugUpdateStat is valid.
     */
    bool hasDebugUpdateStatFunc() const noexcept;

    /**
     * To track backend-specific statistics, the backend implementation can call the
     * application-provided callback function debugUpdateStatFunc to associate or update a value
     * with a given key. It is possible for this function to be called multiple times with the
     * same key, in which case newer values should overwrite older values.
     *
     * This function is guaranteed to be called only on a single thread, the Filament driver
     * thread.
     *
     * @param key           a null-terminated C-string with the key of the debug statistic
     * @param intValue      the updated integer value of key (the string value passed to the
     *                      callback will be empty)
     */
    void debugUpdateStat(const char* UTILS_NONNULL key, uint64_t intValue);

    /**
     * To track backend-specific statistics, the backend implementation can call the
     * application-provided callback function debugUpdateStatFunc to associate or update a value
     * with a given key. It is possible for this function to be called multiple times with the
     * same key, in which case newer values should overwrite older values.
     *
     * This function is guaranteed to be called only on a single thread, the Filament driver
     * thread.
     *
     * @param key           a null-terminated C-string with the key of the debug statistic
     * @param stringValue   the updated string value of key (the integer value passed to the
     *                      callback will be 0)
     */
    void debugUpdateStat(const char* UTILS_NONNULL key, utils::CString stringValue);

private:
    std::shared_ptr<InsertBlobFunc> mInsertBlob;
    std::shared_ptr<RetrieveBlobFunc> mRetrieveBlob;
    std::shared_ptr<DebugUpdateStatFunc> mDebugUpdateStat;
    mutable utils::Mutex mMutex;
};

} // namespace filament

#endif // TNT_FILAMENT_BACKEND_PLATFORM_H
