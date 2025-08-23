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

#include "details/Sync.h"

#include "details/Engine.h"

#include <filament/Sync.h>

namespace filament {

using DriverApi = backend::DriverApi;

FSync::FSync(FEngine& engine)
    : mEngine(engine) {
    DriverApi& driverApi = engine.getDriverApi();
    mHwFence = driverApi.createFence();
    driverApi.queueCommand([this]() {
        {
            std::lock_guard lock(mMutex);
            mHasHandle = true;
        }
        processAllCallbacks();
    });
}

void FSync::terminate(FEngine& engine) noexcept { engine.getDriverApi().destroyFence(mHwFence); }

void FSync::convertToExternalSync(SyncConversionCallback callback) noexcept {
    {
        std::lock_guard lock(mMutex);
        if (!mHasHandle) {
            mConversionCallbacks.push_back(callback);
            return;
        }
    }

    processCallback(callback);
}

void FSync::processAllCallbacks() {
    for (const auto& callback: mConversionCallbacks) {
        processCallback(callback);
    }
}

void FSync::processCallback(SyncConversionCallback callback) {
    DriverApi& driverApi = mEngine.getDriverApi();
    int32_t externalHandle;
    SyncConversionResult result = driverApi.getFenceFD(mHwFence, &externalHandle);
    callback(result, externalHandle);
}

} // namespace filament
