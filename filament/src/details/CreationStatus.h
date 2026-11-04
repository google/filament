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

#ifndef TNT_FILAMENT_DETAILS_CREATIONSTATUS_H
#define TNT_FILAMENT_DETAILS_CREATIONSTATUS_H

#include <backend/DriverEnums.h>

#include <atomic>

#include <stdint.h>

namespace filament {

/**
 * Where an object is in its creation, for the classes that support asynchronous creation
 * (FTexture, FVertexBuffer, FIndexBuffer).
 *
 * Two distinct questions are asked of this, and cancellation is where they diverge:
 *  - "is the asynchronous pipeline done with this object?" — a *lifetime* question, which
 *    FEngine::destroy uses to decide whether it can free the object now. Both terminal states
 *    answer yes.
 *  - "is the resource usable?" — only CREATED answers yes.
 *
 * Objects created synchronously go straight to CREATED; asynchronously created ones start at
 * CREATING and reach exactly one terminal state, once, when the last of their creation jobs
 * reports in.
 */
enum class CreationStatus : uint8_t {
    CREATING,   //!< Creation is still in flight.
    CANCELED,   //!< Creation finished without ever populating the object.
    CREATED,    //!< Creation finished and populated the object.
};

/**
 * Holds a CreationStatus that one thread settles and others read.
 *
 * The completion callback of an asynchronous creation runs on the ServiceThread. The release
 * store in settle() pairs with the acquire loads, so a thread that sees the new status also sees
 * what the driver wrote before the callback ran.
 */
class CreationState {
public:
    // Synchronous creation. Relaxed is enough because the object is not visible to other
    // threads yet.
    void setCreated() noexcept {
        mStatus.store(CreationStatus::CREATED, std::memory_order_relaxed);
    }

    // Asynchronous creation, called from the completion callback. Leaves CREATING even when
    // canceled, because FEngine::destroy waits for that before freeing the object.
    void settle(backend::AsyncCallStatus const status) noexcept {
        mStatus.store(status == backend::AsyncCallStatus::CANCELED
                        ? CreationStatus::CANCELED
                        : CreationStatus::CREATED,
                std::memory_order_release);
    }

    bool isSettled() const noexcept {
        return mStatus.load(std::memory_order_acquire) != CreationStatus::CREATING;
    }

    bool isSuccessful() const noexcept {
        return mStatus.load(std::memory_order_acquire) == CreationStatus::CREATED;
    }

private:
    std::atomic<CreationStatus> mStatus{ CreationStatus::CREATING };
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_CREATIONSTATUS_H
