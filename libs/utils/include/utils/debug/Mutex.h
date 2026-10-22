/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_UTILS_DEBUG_MUTEX_H
#define TNT_UTILS_DEBUG_MUTEX_H

#include <utils/CallStack.h>
#include <utils/compiler.h>

#include <atomic>

#if defined(__ANDROID__)
#include <utils/linux/Mutex.h>
#else
#include <utils/generic/Mutex.h>
#endif

namespace utils::debug {

class UTILS_CAPABILITY("mutex") Mutex {
public:
    constexpr Mutex() noexcept = default;
    ~Mutex() noexcept;

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock() UTILS_ACQUIRE();
    bool try_lock() UTILS_TRY_ACQUIRE(true);
    void unlock() UTILS_RELEASE();

    CallStack const& getCreationStack() const noexcept;
    void ensureCreationStackCaptured() const noexcept;

#if defined(__ANDROID__)
    linuxutil::Mutex& getUnderlyingMutex() noexcept { return mUnderlying; }
    linuxutil::Mutex const& getUnderlyingMutex() const noexcept { return mUnderlying; }
#else
    generic::Mutex& getUnderlyingMutex() noexcept { return mUnderlying; }
    generic::Mutex const& getUnderlyingMutex() const noexcept { return mUnderlying; }
#endif

private:
    friend class Condition;

#if defined(__ANDROID__)
    linuxutil::Mutex mUnderlying;
#else
    generic::Mutex mUnderlying;
#endif

    mutable CallStack mCreationStack;
    mutable std::atomic<bool> mCreationCaptured = { false };
};

}

#endif // TNT_UTILS_DEBUG_MUTEX_H
