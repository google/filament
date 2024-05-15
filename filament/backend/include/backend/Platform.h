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

#include <utils/compiler.h>
#include <utils/Invocable.h>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

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

    /**
     * The type of technique for stereoscopic rendering
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
         * Stereoscopic rendering is performed using the multiview feature from the graphics backend.
         */
        MULTIVIEW,
    };

    struct DriverConfig {
        /**
         * Size of handle arena in bytes. Setting to 0 indicates default value is to be used.
         * Driver clamps to valid values.
         */
        size_t handleArenaSize = 0;

        /**
         * This number of most-recently destroyed textures will be tracked for use-after-free.
         * Throws an exception when a texture is freed but still bound to a SamplerGroup and used in
         * a draw call. 0 disables completely. Currently only respected by the Metal backend.
         */
        size_t textureUseAfterFreePoolSize = 0;

        /**
         * Set to `true` to forcibly disable parallel shader compilation in the backend.
         * Currently only honored by the GL and Metal backends.
         */
        bool disableParallelShaderCompile = false;

        /**
         * Disable backend handles use-after-free checks.
         */
        bool disableHandleUseAfterFreeCheck = false;

        /**
         * Force GLES2 context if supported, or pretend the context is ES2. Only meaningful on
         * GLES 3.x backends.
         */
        bool forceGLES2Context = false;

        /**
         * Sets the technique for stereoscopic rendering.
         */
        StereoscopicType stereoscopicType = StereoscopicType::NONE;
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
    virtual backend::Driver* UTILS_NULLABLE createDriver(void* UTILS_NULLABLE sharedContext,
            const DriverConfig& driverConfig) noexcept = 0;

    /**
     * Processes the platform's event queue when called from its primary event-handling thread.
     *
     * Internally, Filament might need to call this when waiting on a fence. It is only implemented
     * on platforms that need it, such as macOS + OpenGL. Returns false if this is not the main
     * thread, or if the platform does not need to perform any special processing.
     */
    virtual bool pumpEvents() noexcept;

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

    using DebugUpdateStatFunc = utils::Invocable<void(const char* UTILS_NONNULL key, uint64_t value)>;

    /**
     * Sets the callback function that the backend can use to update backend-specific statistics
     * to aid with debugging. This callback is guaranteed to be called on the Filament driver
     * thread.
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
     * @param key          a null-terminated C-string with the key of the debug statistic
     * @param value        the updated value of key
     */
    void debugUpdateStat(const char* UTILS_NONNULL key, uint64_t value);

private:
    InsertBlobFunc mInsertBlob;
    RetrieveBlobFunc mRetrieveBlob;
    DebugUpdateStatFunc mDebugUpdateStat;
};

} // namespace filament

#endif // TNT_FILAMENT_BACKEND_PLATFORM_H
