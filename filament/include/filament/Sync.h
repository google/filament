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

#include <backend/DriverEnums.h>

#include <functional>

namespace filament {

class UTILS_PUBLIC Sync : public FilamentAPI {
public:
    using SyncConversionResult = backend::FenceConversionResult;
    using SyncConversionCallback = std::function<void(SyncConversionResult, int32_t)>;

    /**
     * Converts a sync object to one that can be used externally to
     * wait for some GPU work to be completed within Filament before proceeding
     *
     * @param callback A function that will receive the external sync handle, if
     *                 conversion was successful, along with a conversion result
     *                 code (e.g. SUCCESS, ERROR, NOT_SUPPORTED).
     */
    void convertToExternalSync(SyncConversionCallback callback) noexcept;

protected:
    // prevent heap allocation
    ~Sync() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_SYNC_H
