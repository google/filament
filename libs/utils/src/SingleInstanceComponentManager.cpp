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
#include <utils/PagedArenaBitset.h>
#include <utils/Panic.h>
#include <utils/SingleInstanceComponentManager.h>
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
            std::ranges::remove_if(mChangeCallbacks,
                    [token](auto const& info) { return info.token == token; }).begin(),
            mChangeCallbacks.end());
}

void SingleInstanceComponentManagerBase::notifyChange(Entity const e) noexcept {
    for (auto* bitset : mBitsets) {
        bitset->add(e.getId());
    }

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

void SingleInstanceComponentManagerBase::registerBitset(PagedArenaBitset* bitset) {
    mBitsets.push_back(bitset);
}

void SingleInstanceComponentManagerBase::unregisterBitset(PagedArenaBitset const* bitset) {
    mBitsets.erase(std::ranges::remove(mBitsets, bitset).begin(), mBitsets.end());
}

void SingleInstanceComponentManagerBase::catchupGarbage() noexcept {
    uint64_t const currentWatermark = mWatermark.load(std::memory_order_relaxed);

    // Transact with the source of truth
    uint64_t const syncedFence = mEntityManager.getMissedGarbage(mMissedGarbage, currentWatermark);

    // O(1) Fast-Path Early Exit: Ceiling hasn't moved
    if (syncedFence == currentWatermark) {
        return;
    }

    // Process bitsets if any were returned (Clean frames will simply skip this loop)
    if (UTILS_LIKELY(!mMissedGarbage.empty())) {
        mCollapsedGarbage.clear();
        for (auto const* garbageBits : mMissedGarbage) {
            PagedArenaBitset::intersect(&mCollapsedGarbage, mEntities, *garbageBits);
            if (!mCollapsedGarbage.empty()) {
                mPendingDestruction.merge(mCollapsedGarbage);
                mEntities.difference(mCollapsedGarbage);
            }
        }
    }

    // Commit our watermark. It will match currentEpoch - 1,
    mWatermark.store(syncedFence, std::memory_order_release);
}

bool SingleInstanceComponentManagerBase::popPendingZombie(Entity const newEntity, Entity& outZombie) noexcept {
    uint32_t const index = EntityManager::getIndex(newEntity);
    // TODO: this loop will disappear once we only store the indices
    constexpr int32_t kGenerationCount = 1 << EntityManager::GENERATION_BITS;
    for (int32_t g = 0; g < kGenerationCount; ++g) {
        int32_t const potentialZombie = EntityManager::makeIdentity(g, index);
        if (mPendingDestruction[potentialZombie]) {
            mPendingDestruction.remove(potentialZombie);
            outZombie = Entity::import(potentialZombie);
            return true;
        }
    }
    return false;
}

void SingleInstanceComponentManagerBase::checkZombieCollisionPanic(Entity const newEntity) noexcept {
    Entity zombie;
    if (UTILS_UNLIKELY(popPendingZombie(newEntity, zombie))) {
        FILAMENT_CHECK_PRECONDITION(false)
                << "In " << mName.c_str_safe() << ", new component with entity " << newEntity.getId()
                << " collides with a logically-dead component with entity = " << zombie.getId()
                << " (component manager implementation bug)";
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
    size_t count = 0;

    auto flush = [&]() noexcept {
        if (count > 0) {
            dispatcher(context, extraArg, buffer, count);
            destroyedCount += count;
            count = 0;
        }
    };

    mPendingDestruction.popSetBits([&](uint32_t const bit) noexcept {
        if (destroyedCount + count < actualLimit) {
            buffer[count++] = Entity::import(bit);
            if (count == BATCH_SIZE) {
                flush();
            }
            return true;
        }
        return false;
    });

    flush();
}

SingleInstanceComponentManagerBase::Instance
SingleInstanceComponentManagerBase::addComponentImpl(Entity const e, void* context,
        SoaAllocator const allocator) noexcept {
    Instance ci = 0;
    if (!e.isNull()) {
        if (isAmortizationSupported()) {
            checkZombieCollisionPanic(e);
        }

        if (!hasComponent(e)) {
            ci = allocator(context, e);
            mInstanceMap[e] = ci;
            mEntities.add(e.getId());
            notifyChange(e);
        } else {
            ci = mInstanceMap[e];
        }
    }
    assert(ci != 0);
    return ci;
}



void SingleInstanceComponentManagerBase::removeComponentsImpl(Entity const* entities, size_t const count,
        void* context, SoaDeallocator deallocator) noexcept {
    auto& map = mInstanceMap;
    for (size_t k = 0; k < count; ++k) {
        Entity const e = entities[k];
        auto const pos = map.find(e);
        if (UTILS_LIKELY(pos != map.end())) {
            size_t const index = pos->second;
            assert(index != 0);

            auto const [lastIndex, swappedEntity] = deallocator(context, index);

            if (!swappedEntity.isNull()) {
                map[swappedEntity] = index;
            }

            map.erase(pos);
            mEntities.remove(e.getId());
            notifyChange(e);
        }
    }
}

SingleInstanceComponentManagerBase::SingleInstanceComponentManagerBase(EntityManager& em, ImmutableCString name,
        bool const amortizationSupported) noexcept
        : mEntityManager(em), mName(std::move(name)), mAmortizationSupported(amortizationSupported) {
    mEntityManager.registerWatermark(&mWatermark, mName);
}

SingleInstanceComponentManagerBase::~SingleInstanceComponentManagerBase() noexcept {
    mEntityManager.unregisterWatermark(&mWatermark);
}

SingleInstanceComponentManagerBase::SingleInstanceComponentManagerBase(SingleInstanceComponentManagerBase&& rhs) noexcept
        : mInstanceMap(std::move(rhs.mInstanceMap)),
          mEntities(std::move(rhs.mEntities)),
          mPendingDestruction(std::move(rhs.mPendingDestruction)),
          mEntityManager(rhs.mEntityManager),
          mName(std::move(rhs.mName)),
          mAmortizationSupported(rhs.mAmortizationSupported),
          mCollapsedGarbage(std::move(rhs.mCollapsedGarbage)),
          mMissedGarbage(std::move(rhs.mMissedGarbage)),
          mDirtyCount(rhs.mDirtyCount),
          mChangeCallbacks(std::move(rhs.mChangeCallbacks)),
          mBitsets(std::move(rhs.mBitsets)) {
    std::copy_n(rhs.mDirtyEntities, rhs.mDirtyCount, mDirtyEntities);
    mWatermark.store(rhs.mWatermark.load(std::memory_order_relaxed), std::memory_order_relaxed);
    mEntityManager.rebindWatermark(&rhs.mWatermark, &mWatermark, mName);
}

SingleInstanceComponentManagerBase& SingleInstanceComponentManagerBase::operator=(SingleInstanceComponentManagerBase&& rhs) noexcept {
    if (UTILS_LIKELY(this != &rhs)) {
        mEntityManager.unregisterWatermark(&mWatermark);

        mEntities = std::move(rhs.mEntities);
        mPendingDestruction = std::move(rhs.mPendingDestruction);
        mName = std::move(rhs.mName);
        mAmortizationSupported = rhs.mAmortizationSupported;
        mInstanceMap = std::move(rhs.mInstanceMap);
        mCollapsedGarbage = std::move(rhs.mCollapsedGarbage);
        mMissedGarbage = std::move(rhs.mMissedGarbage);
        mDirtyCount = rhs.mDirtyCount;
        mChangeCallbacks = std::move(rhs.mChangeCallbacks);
        mBitsets = std::move(rhs.mBitsets);
        std::copy_n(rhs.mDirtyEntities, rhs.mDirtyCount, mDirtyEntities);
        mWatermark.store(rhs.mWatermark.load(std::memory_order_relaxed), std::memory_order_relaxed);
        mEntityManager.rebindWatermark(&rhs.mWatermark, &mWatermark, mName);
    }
    return *this;
}

} // namespace utils
