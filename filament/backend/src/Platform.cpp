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

#include <utils/compiler.h>
#include <utils/ostream.h>

#include <atomic>
#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

void Platform::ExternalImageHandle::incref(ExternalImage* p) noexcept {
    if (p) {
        // incrementing the ref-count doesn't acquire or release anything
        p->mRefCount.fetch_add(1, std::memory_order_relaxed);
    }
}

void Platform::ExternalImageHandle::decref(ExternalImage* p) noexcept {
    if (p) {
        // When decrementing the ref-count, unless it reaches zero, there is no need to acquire
        // data; we need to release all previous writes though so they can be visible to the thread
        // that will actually delete the object.
        if (p->mRefCount.fetch_sub(1, std::memory_order_release) == 1) {
            // if we reach zero, we're about to delete the object, we need to acquire all previous
            // writes from other threads (i.e.: the memory from other threads prior to the decref()
            // need to be visible now.
            std::atomic_thread_fence(std::memory_order_acquire);
            delete p;
        }
    }
}

Platform::ExternalImageHandle::ExternalImageHandle() noexcept = default;

Platform::ExternalImageHandle::~ExternalImageHandle() noexcept {
    decref(mTarget);
}

Platform::ExternalImageHandle::ExternalImageHandle(ExternalImage* p) noexcept
        : mTarget(p) {
    incref(mTarget);
}

Platform::ExternalImageHandle::ExternalImageHandle(ExternalImageHandle const& rhs) noexcept
        : mTarget(rhs.mTarget) {
    incref(mTarget);
}

Platform::ExternalImageHandle::ExternalImageHandle(ExternalImageHandle&& rhs) noexcept
        : mTarget(rhs.mTarget) {
    rhs.mTarget = nullptr;
}

Platform::ExternalImageHandle& Platform::ExternalImageHandle::operator=(
        ExternalImageHandle const& rhs) noexcept {
    if (UTILS_LIKELY(this != &rhs)) {
        incref(rhs.mTarget);
        decref(mTarget);
        mTarget = rhs.mTarget;
    }
    return *this;
}

Platform::ExternalImageHandle& Platform::ExternalImageHandle::operator=(
        ExternalImageHandle&& rhs) noexcept {
    if (UTILS_LIKELY(this != &rhs)) {
        decref(mTarget);
        mTarget = rhs.mTarget;
        rhs.mTarget = nullptr;
    }
    return *this;
}

void Platform::ExternalImageHandle::clear() noexcept {
    decref(mTarget);
    mTarget = nullptr;
}

void Platform::ExternalImageHandle::reset(ExternalImage* p) noexcept {
    incref(p);
    decref(mTarget);
    mTarget = p;
}

utils::io::ostream& operator<<(utils::io::ostream& out,
        Platform::ExternalImageHandle const& handle) {
    out << "ExternalImageHandle{" << handle.mTarget << "}";
    return out;
}

// --------------------------------------------------------------------------------------------------------------------

Platform::ExternalImage::~ExternalImage() noexcept = default;

// --------------------------------------------------------------------------------------------------------------------

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

void Platform::setDebugUpdateStatFunc(DebugUpdateStatFunc&& debugUpdateStat) noexcept {
    mDebugUpdateStat = std::move(debugUpdateStat);
}

bool Platform::hasDebugUpdateStatFunc() const noexcept {
    return bool(mDebugUpdateStat);
}

void Platform::debugUpdateStat(const char* key, uint64_t value) {
    if (mDebugUpdateStat) {
        mDebugUpdateStat(key, value);
    }
}

} // namespace filament::backend
