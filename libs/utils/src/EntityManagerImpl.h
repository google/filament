/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_UTILS_ENTITYMANAGERIMPL_H
#define TNT_UTILS_ENTITYMANAGERIMPL_H

#include <utils/CallStack.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Logger.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>
#include <utils/Slice.h>

#if FILAMENT_UTILS_TRACK_ENTITIES
#include <tsl/robin_map.h>
#endif
#include <tsl/robin_set.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <deque>
#include <limits>
#include <memory>
#include <mutex> // for LockGuard
#include <new>
#include <utility>
#include <vector>

namespace utils {

static constexpr size_t MIN_FREE_INDICES = 1024;

struct GarbageEpoch {
    uint64_t id;
    std::unique_ptr<PagedArenaBitset> garbageBits;
};

struct WatermarkInfo {
    std::atomic<uint64_t>* watermark;
    ImmutableCString name;
};

class UTILS_PRIVATE EntityManagerImpl : public EntityManager {
public:
    friend class EntityManager;

    using EntityManager::getGeneration;
    using EntityManager::getIndex;
    using EntityManager::makeIdentity;
    using EntityManager::create;
    using EntityManager::destroy;

    EntityManagerImpl() noexcept
            : mGens(new(std::nothrow) uint8_t[RAW_INDEX_COUNT]) {
        // initialize all the generations to 0
        std::fill_n(mGens, RAW_INDEX_COUNT, 0);
        mTimeline.push_back({mCurrentEpochID.load(std::memory_order_relaxed), std::make_unique<PagedArenaBitset>()});
    }

    ~EntityManagerImpl() noexcept {
        delete [] mGens;
    }

    bool isAlive(Entity const e) const noexcept {
        assert(getIndex(e) < RAW_INDEX_COUNT);
        if (e.isNull()) {
            return false;
        }

        LockGuard const lock(mDestructionLock);
        uint32_t const index = getIndex(e);

        // Is this entity the current tenant of this memory slot?
        bool const alive = (getGeneration(e) == mGens[index]);
        assert(alive == mAliveEntities[e.getId()]);
        return alive;
    }

    UTILS_NOINLINE
    size_t getEntityCount() const noexcept {
        LockGuard const lock(mDestructionLock);
        return mAliveEntities.size();
    }

    UTILS_NOINLINE
    void create(size_t const n, Entity* entities) {
        auto& freeList = mFreeList;

        size_t processed = 0;

        while (processed < n) {
            constexpr size_t BATCH_SIZE = 128;
            // Determine how many entities to process in this iteration
            size_t const chunk = std::min(n - processed, BATCH_SIZE);

            Entity::Type localCreated[BATCH_SIZE];
            size_t validCount = 0;

            // -------------------------------------------------------------
            // Phase 1: Lock the Free List and allocate IDs
            // -------------------------------------------------------------
            {
                LockGuard const lock(mCreationLock);
                Entity::Type currentIndex = mCurrentIndex;

                for (size_t i = 0; i < chunk; i++) {
                    Entity::Type index{};

                    if (UTILS_UNLIKELY(currentIndex >= RAW_INDEX_COUNT || freeList.size() >= MIN_FREE_INDICES)) {
                        // Out of indices check
                        if (UTILS_UNLIKELY(freeList.empty())) {
                            entities[processed + i] = {}; // Return Null Entity
                            continue; // Do NOT increment validCount
                        }
                        index = freeList.front();
                        freeList.pop_front();
                    } else {
                        index = currentIndex++;
                    }

                    // Read mGens lock-free because this index is mathematically sterile
                    uint8_t const gen = [this, index]() UTILS_NO_THREAD_SAFETY_ANALYSIS noexcept {
                        return mGens[index];
                    }();
                    Entity e = Entity{makeIdentity(gen, index)};
                    entities[processed + i] = e;
                    localCreated[validCount++] = e.getId();
                }
                mCurrentIndex = currentIndex;
            } // mCreationLock dropped!

            // -------------------------------------------------------------
            // Phase 2: Lock the Global State to register active entities
            // -------------------------------------------------------------
            if (UTILS_LIKELY(validCount > 0)) {
                LockGuard const stateLock(mDestructionLock);
                for (size_t i = 0; i < validCount; i++) {
                    mAliveEntities.add(localCreated[i]);
                }
            } // mDestructionLock dropped!

#if FILAMENT_UTILS_TRACK_ENTITIES
            {
                LockGuard const stateLock(mDestructionLock);
                for (size_t i = 0; i < chunk; i++) {
                    if (!entities[processed + i].isNull()) {
                        mDebugActiveEntities.emplace(entities[processed + i], CallStack::unwind(5));
                    }
                }
            }
#endif

            processed += chunk;
        }
    }

    UTILS_NOINLINE
    void destroy(size_t const n, Entity* entities) noexcept {
        UniqueLock stateLock(mDestructionLock);
        uint8_t* const gens = mGens;
        for (size_t i = 0; i < n; i++) {
            if (!entities[i]) {
                continue; // behave like free(), ok to free null Entity.
            }

            bool const isAlive = getGeneration(entities[i]) == mGens[getIndex(entities[i])];
            assert(isAlive);

            if (UTILS_LIKELY(isAlive)) {
                Entity::Type const index = getIndex(entities[i]);

                gens[index]++;
                mAliveEntities.remove(entities[i].getId());

#if FILAMENT_UTILS_TRACK_ENTITIES
                // MUST be protected by mDestructionLock
                mDebugActiveEntities.erase(entities[i]);
#endif

                // Safely write to the shared member buffer
                mDirtyEntities[mDirtyCount++] = entities[i];

                // Batch Process when buffer is full
                if (mDirtyCount == MAX_DIRTY_COUNT) {
                    // 1. Extract the shared data to a private stack buffer
                    Entity localBuffer[MAX_DIRTY_COUNT];
                    std::copy_n(mDirtyEntities, MAX_DIRTY_COUNT, localBuffer);

                    // 2. Reset the global state for other threads
                    mDirtyCount = 0;

                    // 3. Drop the state lock NOW, safely
                    stateLock.unlock();

                    // 4. Process the heavy timeline append + callbacks
                    flushDestroyBuffer(localBuffer, MAX_DIRTY_COUNT);

                    // 5. Re-acquire for the next loop iteration
                    stateLock.lock();
                }
            }
        }

        // Handle any remaining entities in the buffer
        if (mDirtyCount > 0) {
            Entity localBuffer[MAX_DIRTY_COUNT];
            uint32_t const remainder = mDirtyCount;
            std::copy_n(mDirtyEntities, remainder, localBuffer);
            mDirtyCount = 0;

            stateLock.unlock();
            flushDestroyBuffer(localBuffer, remainder);
        } else {
            stateLock.unlock(); // Ensure lock is dropped before listeners
        }

        // notify our listeners that some entities are being destroyed
        auto listeners = getListeners();
        for (auto const& l : listeners) {
            l->onEntitiesDestroyed(n, entities);
        }
    }

    void flushDestroyBuffer(Entity const* localEntities, uint32_t const count) noexcept {
        // Lock the timeline EXACTLY ONCE for the entire batch
        {
            LockGuard timelineLock(mTimelineLock);
            auto& currentGarbage = *mTimeline.back().garbageBits;

            for (size_t i = 0; i < count; ++i) {
                Entity::Type const fullId = localEntities[i].getId();
#ifndef NDEBUG
                assert(!mDebugGlobalPurgatory[fullId] && "EBR: Double-Destroy caught!");
                mDebugGlobalPurgatory.add(fullId);
#endif
                currentGarbage.add(fullId);
            }
        } // mTimelineLock dropped!

        // Trigger Callbacks outside ALL locks
        triggerChangeCallbacks(localEntities, count);
    }

    void registerListener(Listener* l) noexcept {
        LockGuard const lock(mListenerLock);
        mListeners.insert(l);
    }

    void unregisterListener(Listener* l) noexcept {
        LockGuard const lock(mListenerLock);
        mListeners.erase(l);
    }

    void registerWatermark(std::atomic<uint64_t>* const watermark, ImmutableCString name) noexcept {
        LockGuard const lock(mTimelineLock);
        mActiveWatermarks.push_back({watermark, std::move(name)});
    }

    void unregisterWatermark(std::atomic<uint64_t>* const watermark) noexcept {
        constexpr size_t BATCH_SIZE = RECLAMATION_EPOCH_BATCH_SIZE;
        std::array<GarbageEpoch, BATCH_SIZE> batch;
        size_t count = 0;
        {
            LockGuard const lock(mTimelineLock);
            mActiveWatermarks.erase(
                std::ranges::remove_if(mActiveWatermarks,
                        [watermark](auto const& info) noexcept { return info.watermark == watermark; }).begin(),
                mActiveWatermarks.end());
            count = getExpiredEpochsLocked(batch.data(), BATCH_SIZE);
        }
        if (count > 0) {
            recycleGarbageEpochs(batch.data(), count);
        }
    }

    void rebindWatermark(std::atomic<uint64_t> const* oldW, std::atomic<uint64_t>* newW,
            ImmutableCString newName) noexcept {
        LockGuard const lock(mTimelineLock);
        for (auto& [watermark, name] : mActiveWatermarks) {
            if (watermark == oldW) {
                watermark = newW;
                name = std::move(newName);
                return;
            }
        }
    }

    void advanceEpoch() noexcept {
        std::unique_ptr<PagedArenaBitset> newBitset;
        {
            LockGuard const lock(mCreationLock);
            if (UTILS_LIKELY(!mBitsetPool.empty())) {
                newBitset = std::move(mBitsetPool.back());
                mBitsetPool.pop_back();
            } else {
                newBitset = std::make_unique<PagedArenaBitset>();
            }
        }

        constexpr size_t BATCH_SIZE = RECLAMATION_EPOCH_BATCH_SIZE;
        std::array<GarbageEpoch, BATCH_SIZE> batch;
        size_t count = 0;
        {
            LockGuard const lock(mTimelineLock);
            
            // Read the current epoch (relaxed is safe because we hold the exclusive lock)
            uint64_t const nextEpoch = mCurrentEpochID.load(std::memory_order_relaxed) + 1;
            
            // Mutate the heavy data structures
            mTimeline.push_back({nextEpoch, std::move(newBitset)});

            // mCurrentEpochID is not publishing data, the lock is, but we keep it at the end -- even if not required for
            // correctness.
            mCurrentEpochID.store(nextEpoch, std::memory_order_relaxed);

            // this must be done after we write mCurrentEpochID 
            count = getExpiredEpochsLocked(batch.data(), BATCH_SIZE);
        }

        // Do the heavy lifting outside the lock
        if (count > 0) {
            recycleGarbageEpochs(batch.data(), count);
        }
    }

    uint64_t getMissedGarbage(
            std::vector<const PagedArenaBitset*>& out, uint64_t const readerWatermark) noexcept {
        LockGuard const lock(mTimelineLock);
        out.clear();
        out.reserve(mTimeline.size());
        // Relaxed is fine here. The mutex acts as our memory barrier.
        uint64_t const currentEpoch = mCurrentEpochID.load(std::memory_order_relaxed);
        for (auto const& [id, garbageBits] : mTimeline) {
            if (id > readerWatermark && id < currentEpoch) {
                out.push_back(garbageBits.get());
            }
        }
        // Return the logical fence. Because we scanned everything < currentEpoch,
        // the reader is guaranteed to be fully caught up to currentEpoch - 1.
        return currentEpoch - 1;
    }

    void reclaimSafeEpochs() noexcept {
        constexpr size_t BATCH_SIZE = RECLAMATION_EPOCH_BATCH_SIZE;
        std::array<GarbageEpoch, BATCH_SIZE> batch;
        size_t count = 0;
        {
            LockGuard const lock(mTimelineLock);
            count = getExpiredEpochsLocked(batch.data(), BATCH_SIZE);
        }
        // Do the heavy lifting outside the lock
        if (count > 0) {
            recycleGarbageEpochs(batch.data(), count);
        }
    }

    uint64_t getLatestEpochID() const noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
        // this cannot be used to acquire data (mCurrentEpochID is not releasing data)
        return mCurrentEpochID.load(std::memory_order_relaxed);
    }

    void registerChangeCallback(void const* token, ChangeCallback callback) noexcept {
        LockGuard const lock(mListenerLock);
        mChangeCallbacks.push_back({token, std::move(callback)});
    }

    void unregisterChangeCallback(void const* token) noexcept {
        LockGuard const lock(mListenerLock);
        std::erase_if(mChangeCallbacks,
                [token](auto const& info) { return info.token == token; });
    }

    void flushNotifications() noexcept {
        UniqueLock lock(mDestructionLock);
        if (mDirtyCount > 0) {
            Entity localBuffer[MAX_DIRTY_COUNT];
            assert_invariant(mDirtyCount <= MAX_DIRTY_COUNT);
            std::copy_n(mDirtyEntities, mDirtyCount, localBuffer);
            size_t const count = mDirtyCount;
            mDirtyCount = 0;
            lock.unlock();

            // Safely appends to timeline and triggers callbacks
            flushDestroyBuffer(localBuffer, count);
        }
    }

#if FILAMENT_UTILS_TRACK_ENTITIES
    std::vector<Entity> getActiveEntities() const {
        std::vector<Entity> result;
        {
            LockGuard const lock(mDestructionLock);
            result.reserve(mDebugActiveEntities.size());
            for (auto const& i : mDebugActiveEntities) {
                result.push_back(i.first);
            }
        }
        return result;
    }

    void dumpActiveEntities(io::ostream& out) const {
        std::vector<std::pair<Entity, CallStack>> activeEntities;
        {
            LockGuard const lock(mDestructionLock);
            activeEntities.reserve(mDebugActiveEntities.size());
            for (auto const& i : mDebugActiveEntities) {
                activeEntities.push_back(i);
            }
        }

        for (auto const& i : activeEntities) {
            out << "*** Entity " << i.first.getId() << " was allocated at:\n";
            out << i.second;
            out << io::endl;
        }
    }
#endif

private:
    std::vector<ChangeCallback> getChangeCallbacks() const noexcept {
        LockGuard const lock(mListenerLock);
        std::vector<ChangeCallback> result;
        result.reserve(mChangeCallbacks.size());
        for (auto const& [token, callback] : mChangeCallbacks) {
            result.push_back(callback);
        }
        return result;
    }

    void triggerChangeCallbacks(Entity const* entities, size_t const n) const noexcept {
        auto const callbacks = getChangeCallbacks();
        Slice const slice(entities, n);
        for (auto const& callback : callbacks) {
            callback(slice);
        }
    }

    void recycleGarbageEpochs(GarbageEpoch* batch, size_t const count) noexcept {
        // Only lock the free-list. Never blocks destroy().
        LockGuard const lock(mCreationLock);

        for (size_t i = 0; i < count; ++i) {
            auto& garbageBits = batch[i].garbageBits;
            garbageBits->forEachSetBit([this](uint32_t const bit) UTILS_NO_THREAD_SAFETY_ANALYSIS noexcept {
                // STRIP THE GENERATION: Convert the ID back to a pure index
                // before pushing it to the available free-list.
                Entity::Type const index = getIndex(Entity::import(bit));
                mFreeList.push_back(index); // FIXME: this will heap allocate (std::deque)
            });
            garbageBits->clear();
            mBitsetPool.push_back(std::move(garbageBits));
        }
    }

    size_t getExpiredEpochsLocked(GarbageEpoch* out, size_t const capacity) noexcept UTILS_REQUIRES(mTimelineLock) {
        size_t count = 0;
        uint64_t safeThreshold = std::numeric_limits<uint64_t>::max();
        // Relaxed is fine here. The mutex acts as our memory barrier.
        uint64_t const currentEpoch = mCurrentEpochID.load(std::memory_order_relaxed);
        if (UTILS_LIKELY(!mActiveWatermarks.empty())) {
            for (auto const& [watermark, name] : mActiveWatermarks) {
                safeThreshold = std::min(safeThreshold, watermark->load(std::memory_order_acquire));
            }
        } else {
            safeThreshold = currentEpoch;
        }

        // EBR Stall Diagnostics
        // Only print the warning when it crosses a 64-epoch threshold to prevent I/O freezing
        if (UTILS_UNLIKELY(mTimeline.size() > 128 && (mTimeline.size() % 64 == 0))) {
            for (auto const& [watermark, name] : mActiveWatermarks) {
                uint64_t const wm = watermark->load(std::memory_order_relaxed);
                if (wm < currentEpoch - 1) {
                    LOG(WARNING)
                            << "EBR Stall Warning: Component Manager '" << name.c_str()
                            << "' is stalling reclamation! Its watermark is stuck at epoch "
                            << wm << ", blocking the recycling of "
                            << mTimeline.front().garbageBits->size() << " dead entities.";
                }
            }
        }

        auto it = mTimeline.begin();
        while (it != mTimeline.end() && it->id < safeThreshold && count < capacity) {
            out[count++] = std::move(*it);
            ++it;
        }

        if (UTILS_LIKELY(it != mTimeline.begin())) {
            mTimeline.erase(mTimeline.begin(), it);

#ifndef NDEBUG
            // We already hold mTimelineLock here!
            // Validate and update the global purgatory tracker mathematically.
            for (size_t i = 0; i < count; ++i) {
                auto const& garbageBits = out[i].garbageBits;
                // Prove Invariant: The dying epoch MUST be a strict subset of our tracker
                assert(garbageBits->isSubsetOf(mDebugGlobalPurgatory) &&
                        "EBR: Attempted to recycle an untracked ID!");
                // Safely remove these bits from purgatory before they hit the free list
                mDebugGlobalPurgatory.difference(*garbageBits);
            }
#endif
        }
        return count;
    }

    FixedCapacityVector<Listener*> getListeners() const noexcept {
        LockGuard const lock(mListenerLock);
        tsl::robin_set<Listener*> const& listeners = mListeners;
        FixedCapacityVector<Listener*> result(listeners.size());
        result.resize(result.capacity()); // unfortunately this memset()
        std::ranges::copy(listeners, result.begin());
        return result; // the c++ standard guarantees a move
    }

    /**
     * @brief The Bounded Stack-Batch Size for Epoch-Based Reclamation (EBR) Timeline Drains.
     *
     * @details
     * ARCHITECTURAL TRADEOFF ANALYSIS:
     * -------------------------------
     * 1. 8x Drain-to-Input Ratio: Since advanceEpoch() produces exactly ONE new epoch
     *    per rendering frame, a batch size of 8 delivers a mathematically guaranteed
     *    8x output reclamation rate. Stalled timeline backlogs automatically drain and
     *    catch up to steady-state within a handful of frame intervals.
     *
     * 2. CPU Spike Amortization: Hard-bounding the extraction limit to 8 epochs prevents
     *    massive synchronizations during Level/Scene unloads from flushing thousands of
     *    dead components in a single frame, perfectly protecting the 60 FPS frame time from stutter.
     *
     * 3. Forward Progress & Multithread Liveness: Because the extraction loop executes bounded,
     *    single-pass extractions within EXACTLY ONE lock acquisition, the engine eliminates all
     *    livelock vulnerabilities and indefinite thread spinning under heavy concurrent Engine contention.
     *
     * 4. Zero Heap Allocations: Matches the array capacity to exactly fit on the executing thread's
     *    local stack frame inside std::array<GarbageEpoch, N>, guaranteeing zero dynamic heap Footprint.
     */
    static constexpr size_t RECLAMATION_EPOCH_BATCH_SIZE = 8;
    static constexpr size_t MAX_DIRTY_COUNT = 16;
    struct CallbackInfo {
        void const* token;
        ChangeCallback callback;
    };

    mutable Mutex mCreationLock;
    uint32_t mCurrentIndex UTILS_GUARDED_BY(mCreationLock) = 1;
    std::deque<Entity::Type> mFreeList UTILS_GUARDED_BY(mCreationLock);     // stores indices that got freed
    std::vector<std::unique_ptr<PagedArenaBitset>> mBitsetPool UTILS_GUARDED_BY(mCreationLock);

    mutable Mutex mDestructionLock;
    uint8_t* const mGens UTILS_GUARDED_BY(mDestructionLock);                   // stores the generation of each index.
    PagedArenaBitset mAliveEntities UTILS_GUARDED_BY(mDestructionLock);        // stores entities that are alive
    Entity mDirtyEntities[MAX_DIRTY_COUNT] UTILS_GUARDED_BY(mDestructionLock);
    size_t mDirtyCount UTILS_GUARDED_BY(mDestructionLock) = 0;
#if FILAMENT_UTILS_TRACK_ENTITIES
    tsl::robin_map<Entity, CallStack, Entity::Hasher> mDebugActiveEntities UTILS_GUARDED_BY(mDestructionLock);
#endif

    mutable std::mutex mTimelineLock;
    // EBR timeline state
    std::atomic<uint64_t> mCurrentEpochID UTILS_GUARDED_BY(mTimelineLock){1}; // doesn't publish data, the mutex does
    std::vector<GarbageEpoch> mTimeline UTILS_GUARDED_BY(mTimelineLock);
    std::vector<WatermarkInfo> mActiveWatermarks UTILS_GUARDED_BY(mTimelineLock);
#ifndef NDEBUG
    // Tracks every bit that is dead but not yet recycled
    PagedArenaBitset UTILS_GUARDED_BY(mTimelineLock) mDebugGlobalPurgatory;
#endif

    mutable Mutex mListenerLock;
    tsl::robin_set<Listener*> mListeners UTILS_GUARDED_BY(mListenerLock);
    std::vector<CallbackInfo> mChangeCallbacks UTILS_GUARDED_BY(mListenerLock);
};

} // namespace utils



#endif // TNT_UTILS_ENTITYMANAGERIMPL_H
