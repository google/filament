/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <backend/Platform.h>

namespace filament::backend {

Platform::Platform() noexcept = default;

// this generates the vtable in this translation unit
Platform::~Platform() noexcept = default;

bool Platform::pumpEvents() noexcept {
    return false;
}

void Platform::setBlobFunc(InsertBlobFunc&& insertBlob, RetrieveBlobFunc&& retrieveBlob) noexcept {
    mInsertBlob = std::move(insertBlob);
    mRetrieveBlob = std::move(retrieveBlob);
}

bool Platform::hasInsertBlobFunc() const noexcept {
    return bool(mInsertBlob);
}

bool Platform::hasRetrieveBlobFunc() const noexcept {
    return bool(mRetrieveBlob);
}

void Platform::insertBlob(void const* key, size_t keySize, void const* value, size_t valueSize) {
    if (mInsertBlob) {
        mInsertBlob(key, keySize, value, valueSize);
    }
}

size_t Platform::retrieveBlob(void const* key, size_t keySize, void* value, size_t valueSize) {
    if (mRetrieveBlob) {
        return mRetrieveBlob(key, keySize, value, valueSize);
    }
    return 0;
}

void Platform::setDebugLogFunc(DebugLogFunc&& debugLog) noexcept {
    mDebugLog = std::move(debugLog);
}

bool Platform::hasDebugLogFunc() const noexcept {
    return bool(mDebugLog);
}

void Platform::debugLog(const char* str, size_t len) {
    if (mDebugLog) {
        mDebugLog(str, len);
    }
}

} // namespace filament::backend
