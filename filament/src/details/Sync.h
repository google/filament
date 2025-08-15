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

#ifndef TNT_FILAMENT_DETAILS_SYNC_H
#define TNT_FILAMENT_DETAILS_SYNC_H

#include "downcast.h"

#include <filament/Sync.h>

#include <backend/Handle.h>

#include <functional>
#include <mutex>
#include <vector>

namespace filament {

class FEngine;

class FSync : public Sync {
public:
    FSync(FEngine& engine);

    void terminate(FEngine& engine) noexcept;

    backend::FenceHandle getHwHandle() const noexcept { return mHwFence; }

    /**
     * Converts a sync object to one that can be used externally to
     * wait for some GPU work to be completed within Filament before proceeding
     *
     * @param callback A function that will receive the external sync handle, if
     *                 conversion was successful, along with a conversion result
     *                 code (e.g. SUCCESS, ERROR, NOT_SUPPORTED).
     */
    void convertToExternalSync(SyncConversionCallback callback) noexcept;

private:
    FEngine& mEngine;
    backend::FenceHandle mHwFence;

    std::mutex mMutex;
    bool mHasHandle = false;
    std::vector<SyncConversionCallback> mConversionCallbacks;

    void processAllCallbacks();

    void processCallback(SyncConversionCallback callback);
};

FILAMENT_DOWNCAST(Sync)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SYNC_H
