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

#include <utils/SingleInstanceComponentManager.h>
#include <utils/Entity.h>
#include <utils/Slice.h>

#include <algorithm>
#include <utility>

namespace utils {

void SingleInstanceComponentManagerBase::registerChangeCallback(
        void const* token, ChangeCallback callback) noexcept {
    mChangeCallbacks.push_back({ token, std::move(callback) });
}

void SingleInstanceComponentManagerBase::unregisterChangeCallback(
        void const* token) noexcept {
    mChangeCallbacks.erase(
            std::remove_if(mChangeCallbacks.begin(), mChangeCallbacks.end(),
                    [token](auto const& info) { return info.token == token; }),
            mChangeCallbacks.end());
}

void SingleInstanceComponentManagerBase::notifyChange(Entity const e) noexcept {
    if constexpr (USE_SORTED_DIRTY_ARRAY) {
        auto const it = std::lower_bound(mDirtyEntities, mDirtyEntities + mDirtyCount, e);
        if (it != mDirtyEntities + mDirtyCount && *it == e) {
            return;
        }
        size_t const idx = it - mDirtyEntities;
        for (size_t i = mDirtyCount; i > idx; --i) {
            mDirtyEntities[i] = mDirtyEntities[i - 1];
        }
        mDirtyEntities[idx] = e;
        mDirtyCount++;
    } else {
        for (size_t i = 0; i < mDirtyCount; ++i) {
            if (mDirtyEntities[i] == e) {
                return;
            }
        }
        mDirtyEntities[mDirtyCount++] = e;
    }
    if (mDirtyCount == MAX_DIRTY_COUNT) {
        flushNotifications();
    }
}

void SingleInstanceComponentManagerBase::flushNotifications() noexcept {
    if (mDirtyCount > 0) {
        Slice<const Entity> const slice(mDirtyEntities, mDirtyCount);
        for (auto const& [token, callback] : mChangeCallbacks) {
            callback(slice);
        }
        mDirtyCount = 0;
    }
}

} // namespace utils
