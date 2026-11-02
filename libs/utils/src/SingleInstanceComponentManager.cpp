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

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>

#include <algorithm>
#include <cassert>
#include <utility>

namespace utils {

void SingleInstanceComponentManagerBase::registerChangeCallback(
        void const* token, ChangeCallback callback) noexcept {
    flushNotifications();
    mChangeCallbacks.push_back({ token, std::move(callback) });
}

void SingleInstanceComponentManagerBase::unregisterChangeCallback(
        void const* token) noexcept {
    mChangeCallbacks.erase(
            std::ranges::remove_if(mChangeCallbacks,
                    [token](auto const& info) { return info.token == token; }).begin(),
            mChangeCallbacks.end());
}

void SingleInstanceComponentManagerBase::recordDirtyEntity(Entity const e) noexcept {
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

void SingleInstanceComponentManagerBase::notifyChange(Entity const e) noexcept {
    uint32_t const index = EntityManager::getIndex(e);
    for (auto const [bitset, filter] : mBitsets) {
        if (!filter || (*filter)[index]) {
            bitset->add(index);
        }
    }

    if (UTILS_UNLIKELY(!mChangeCallbacks.empty())) {
        recordDirtyEntity(e);
    }
}

void SingleInstanceComponentManagerBase::notifyChange(Slice<const Entity> const entities) noexcept {
    static_assert(sizeof(Entity) == sizeof(uint32_t) && std::is_standard_layout_v<Entity>,
            "Entity must be layout-compatible with uint32_t for zero-copy Slice forwarding");

    Slice const ids{ reinterpret_cast<uint32_t const*>(entities.data()), entities.size() };
    for (auto const [bitset, filter] : mBitsets) {
        bitset->add(ids, filter, EntityManager::INDEX_MASK);
    }

    if (UTILS_UNLIKELY(!mChangeCallbacks.empty())) {
        for (Entity const e : entities) {
            recordDirtyEntity(e);
        }
    }
}

void SingleInstanceComponentManagerBase::flushNotifications() noexcept {
    if (mDirtyCount > 0) {
        size_t const count = mDirtyCount;
        mDirtyCount = 0;
        Entity dirty[MAX_DIRTY_COUNT];
        std::copy_n(mDirtyEntities, count, dirty);
        Slice<const Entity> const slice(dirty, count);
        for (auto const& [token, callback] : mChangeCallbacks) {
            callback(slice);
        }
    }
}

void SingleInstanceComponentManagerBase::registerBitset(
        PagedArenaBitset* bitset, PagedArenaBitset const* filter) {
    mBitsets.push_back({ bitset, filter });
}

void SingleInstanceComponentManagerBase::unregisterBitset(PagedArenaBitset const* bitset) {
    mBitsets.erase(
            std::ranges::remove_if(mBitsets,
                    [bitset](auto const& reg) { return reg.bitset == bitset; }).begin(),
            mBitsets.end());
}

void SingleInstanceComponentManagerBase::catchupGarbage() noexcept {
    uint64_t const currentWatermark = mWatermark.load(std::memory_order_relaxed);
    uint64_t const currentFence = std::max(mHarvestedEpoch, currentWatermark);

    // Transact with the source of truth
    uint64_t const syncedFence = mEntityManager.getMissedGarbage(mMissedGarbage, currentFence);

    // O(1) Fast-Path Early Exit: Ceiling hasn't moved
    if (syncedFence == currentFence) {
        if (!mInGc && mPendingDestruction.empty() && currentWatermark < currentFence) {
            mWatermark.store(currentFence, std::memory_order_release);
        }
        return;
    }

    mHarvestedEpoch = syncedFence;

    // Process bitsets if any were returned (Clean frames will simply skip this loop)
    if (UTILS_LIKELY(!mMissedGarbage.empty())) {
        LockGuard const lock(mEbrEntitiesLock);
        mCollapsedGarbage.clear();
        for (auto const* garbageBits : mMissedGarbage) {
            PagedArenaBitset::intersect(&mCollapsedGarbage, mEntities, *garbageBits);
            if (!mCollapsedGarbage.empty()) {
                mPendingDestruction.merge(mCollapsedGarbage);
            }
        }
    }

    // Only commit our watermark immediately if no pending destructions remain;
    // otherwise, the watermark is held until gcImpl() physically evicts them.
    if (!mInGc && mPendingDestruction.empty()) {
        mWatermark.store(syncedFence, std::memory_order_release);
    }
}

void SingleInstanceComponentManagerBase::catchupCurrentGarbage() noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
    if (mEntities.empty()) {
        return;
    }
    mEntityManager.intersectCurrentGarbage(mCollapsedGarbage, mEntities);
    if (UTILS_UNLIKELY(!mCollapsedGarbage.empty())) {
        mPendingDestruction.merge(mCollapsedGarbage);
    }
}

void SingleInstanceComponentManagerBase::gcImpl(uint32_t const maxDestructions,
        void* context, void* extraArg, EvictionDispatcher const dispatcher) noexcept {

    catchupGarbage();

    if (UTILS_UNLIKELY(mPendingDestruction.empty())) {
        return;
    }

    uint32_t const actualLimit = isAmortizationSupported() ?
            maxDestructions : std::numeric_limits<uint32_t>::max();

    uint32_t destroyedCount = 0;

    constexpr size_t BATCH_SIZE = 64;
    Entity buffer[BATCH_SIZE];

    bool const prevInGc = mInGc;
    mInGc = true;
    while (destroyedCount < actualLimit && !mPendingDestruction.empty()) {
        size_t const batchLimit = std::min<size_t>(BATCH_SIZE, actualLimit - destroyedCount);
        size_t count = 0;
        mPendingDestruction.popSetBits([&](uint32_t const bit) noexcept {
            if (count < batchLimit) {
                buffer[count++] = mEntityManager.getEntity(bit);
                return true;
            }
            return false;
        });
        if (UTILS_UNLIKELY(count == 0)) {
            break;
        }
        dispatcher(context, extraArg, buffer, count);
        destroyedCount += count;
    }
    mInGc = prevInGc;

    if (!mInGc && mPendingDestruction.empty()) {
        uint64_t const latestFence = std::max(mHarvestedEpoch, mWatermark.load(std::memory_order_relaxed));
        mWatermark.store(latestFence, std::memory_order_release);
    }
}

SingleInstanceComponentManagerBase::AddResult
SingleInstanceComponentManagerBase::addComponentImpl(Entity const e, void* context,
        SoaAllocator const allocator) noexcept {
    Instance ci = 0;
    bool added = false;
    if (!e.isNull()) {
        assert(mEntityManager.isAlive(e));
        resume();

        auto [it, inserted] = mInstanceMap.try_emplace(e, 0);
        if (UTILS_LIKELY(inserted)) {
            ci = allocator(context, e);
            it.value() = ci;
            added = true;
            {
                LockGuard const lock(mEbrEntitiesLock);
                mEntities.add(EntityManager::getIndex(e));
            }
            notifyChange(e);
        } else {
            ci = it.value();
        }
    }
    assert(ci != 0);
    return { ci, added };
}



void SingleInstanceComponentManagerBase::eraseComponentFromMap(
        InstanceMap::iterator const pos, Entity const e,
        Entity const swappedEntity, Instance const index) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
    auto& map = mInstanceMap;
    map.erase_fast(pos);

    if (!swappedEntity.isNull()) {
        auto const swappedPos = map.find(swappedEntity);
        assert(swappedPos != map.end());
        swappedPos.value() = index;
    }

    uint32_t const entityIndex = EntityManager::getIndex(e);
    bool const wasPending = mPendingDestruction.fetchRemove(entityIndex);
    if (wasPending && !mInGc && mPendingDestruction.empty()) {
        uint64_t const currentWatermark = mWatermark.load(std::memory_order_relaxed);
        if (currentWatermark < mHarvestedEpoch) {
            mWatermark.store(mHarvestedEpoch, std::memory_order_release);
        }
    }

    // We only need to lock mEbrEntitiesLock when writing (mutating) mEntities because the
    // global timeline thread (EntityManager) can read it concurrently. Since component
    // managers only run on the main thread, read-only accesses to mEntities from this
    // class do not need synchronization.
    {
        LockGuard const lock(mEbrEntitiesLock);
        mEntities.remove(entityIndex);
    }

    notifyChange(e);

    // Lock-free read check: safe because no other thread mutates mEntities.
    if (mEntities.empty()) {
        suspend();
    }
}

SingleInstanceComponentManagerBase::SingleInstanceComponentManagerBase(EntityManager& em, ImmutableCString name,
        bool const amortizationSupported) noexcept
        : mEntityManager(em), mName(std::move(name)), mAmortizationSupported(amortizationSupported) {
}

SingleInstanceComponentManagerBase::~SingleInstanceComponentManagerBase() noexcept {
    suspend();
}

SingleInstanceComponentManagerBase::SingleInstanceComponentManagerBase(SingleInstanceComponentManagerBase&& rhs) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS
        : mInstanceMap(std::move(rhs.mInstanceMap)),
          mEntities(std::move(rhs.mEntities)),
          mPendingDestruction(std::move(rhs.mPendingDestruction)),
          mEntityManager(rhs.mEntityManager),
          mName(std::move(rhs.mName)),
          mAmortizationSupported(rhs.mAmortizationSupported),
          mCollapsedGarbage(std::move(rhs.mCollapsedGarbage)),
          mHarvestedEpoch(rhs.mHarvestedEpoch),
          mMissedGarbage(std::move(rhs.mMissedGarbage)),
          mDirtyCount(rhs.mDirtyCount),
          mChangeCallbacks(std::move(rhs.mChangeCallbacks)),
          mBitsets(std::move(rhs.mBitsets)) {
    std::copy_n(rhs.mDirtyEntities, rhs.mDirtyCount, mDirtyEntities);
    mSuspended = rhs.mSuspended;
    mWatermark.store(rhs.mWatermark.load(std::memory_order_relaxed), std::memory_order_relaxed);
    if (!mSuspended) {
        mEntityManager.rebindWatermark(&rhs.mWatermark, &mWatermark, mName, &mEntities, &mEbrEntitiesLock);
    }
}

SingleInstanceComponentManagerBase& SingleInstanceComponentManagerBase::operator=(SingleInstanceComponentManagerBase&& rhs) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
    if (UTILS_LIKELY(this != &rhs)) {
        suspend();

        mEntities = std::move(rhs.mEntities);
        mPendingDestruction = std::move(rhs.mPendingDestruction);
        mName = std::move(rhs.mName);
        mAmortizationSupported = rhs.mAmortizationSupported;
        mInstanceMap = std::move(rhs.mInstanceMap);
        mCollapsedGarbage = std::move(rhs.mCollapsedGarbage);
        mHarvestedEpoch = rhs.mHarvestedEpoch;
        mMissedGarbage = std::move(rhs.mMissedGarbage);
        mDirtyCount = rhs.mDirtyCount;
        mChangeCallbacks = std::move(rhs.mChangeCallbacks);
        mBitsets = std::move(rhs.mBitsets);
        std::copy_n(rhs.mDirtyEntities, rhs.mDirtyCount, mDirtyEntities);
        mSuspended = rhs.mSuspended;
        mWatermark.store(rhs.mWatermark.load(std::memory_order_relaxed), std::memory_order_relaxed);
        if (!mSuspended) {
            mEntityManager.rebindWatermark(&rhs.mWatermark, &mWatermark, mName, &mEntities, &mEbrEntitiesLock);
        }
    }
    return *this;
}

} // namespace utils
