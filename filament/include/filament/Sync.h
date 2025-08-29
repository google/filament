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

#ifndef TNT_FILAMENT_SYNC_H
#define TNT_FILAMENT_SYNC_H

#include <filament/FilamentAPI.h>

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <functional>

namespace filament {

class UTILS_PUBLIC Sync : public FilamentAPI {
public:
    using CallbackHandler = backend::CallbackHandler;
    using Callback = backend::Platform::SyncCallback;

    /**
     * Fetches a handle to the external, platform-specific representation of
     * this sync object.
     *
     * @return The external handle for the Sync. This is valid destroy() is
     *         called on this Sync object.
     */
    void getExternalHandle(CallbackHandler* handler, Callback callback) noexcept;

protected:
    // prevent heap allocation
    ~Sync() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_SYNC_H
