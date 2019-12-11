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

#ifndef TNT_FILAMENT_DRIVER_PLATFORM_H
#define TNT_FILAMENT_DRIVER_PLATFORM_H

#include <backend/DriverEnums.h>

#include <utils/compiler.h>

namespace filament {
namespace backend {

class Driver;

class UTILS_PUBLIC Platform {
public:
    struct SwapChain {};
    struct Fence {};
    struct Stream {};
    struct ExternalTexture {
        uintptr_t image = 0;
    };

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
     * @return nullptr on failure, or a pointer to the newly created driver.
     */
    virtual backend::Driver* createDriver(void* sharedContext) noexcept = 0;

    /**
     * Processes the platform's event queue when called from its primary event-handling thread.
     *
     * Internally, Filament might need to call this when waiting on a fence. It is only implemented
     * on platforms that need it, such as macOS + OpenGL. Returns false if this is not the main
     * thread, or if the platform does not need to perform any special processing.
     */
    virtual bool pumpEvents() noexcept { return false; }
};


class UTILS_PUBLIC DefaultPlatform : public Platform {
public:
    ~DefaultPlatform() noexcept override;

    /**
     * Creates a Platform configured for the requested backend if available
     *
     * @param backendHint Preferred backend, if not available the backend most suitable for the
     *                    underlying platform is returned and \p backendHint is updated
     *                    accordingly. Can't be nullptr.
     *
     * @return A pointer to the Plaform object.
     *
     * @see destroy
     */
    static DefaultPlatform* create(backend::Backend* backendHint) noexcept;

    /**
     * Destroys a Platform object returned by create()
     *
     * @param platform a reference (as a pointer) to the DefaultPlatform pointer to destroy.
     *                 \p platform is cleared upon return.
     *
     * @see create
     */
    static void destroy(DefaultPlatform** platform) noexcept;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_PLATFORM_H
