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

#include <utils/EntityManager.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/CallStack.h>
#include <utils/Entity.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>
#include <utils/Slice.h>

#include <tsl/robin_set.h>

#if FILAMENT_UTILS_TRACK_ENTITIES
#include <tsl/robin_map.h>
#endif

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex> // for std::lock_guard
#include <new>
#include <utility>
#include <vector>

namespace utils {

static constexpr size_t MIN_FREE_INDICES = 1024;

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
    }

    ~EntityManagerImpl() noexcept {
        delete [] mGens;
    }

    bool isAlive(Entity const e) const noexcept {
        assert(getIndex(e) < RAW_INDEX_COUNT);
        if (e.isNull()) {
            return false;
        }

        std::lock_guard const lock(mFreeListLock);
        bool const alive = (getGeneration(e) == mGens[getIndex(e)]);
        return alive;
    }

    UTILS_NOINLINE
    size_t getEntityCount() const noexcept {
        std::lock_guard const lock(mFreeListLock);
        if (mCurrentIndex < RAW_INDEX_COUNT) {
            return (mCurrentIndex - 1) - mFreeList.size();
        } else {
            return getMaxEntityCount() - mFreeList.size();
        }
    }

    UTILS_NOINLINE
    void create(size_t const n, Entity* entities) {
        Entity::Type index{};
        auto& freeList = mFreeList;
        uint8_t const* const gens = mGens;

        // this must be thread-safe, acquire the free-list mutex
        std::lock_guard const lock(mFreeListLock);
        Entity::Type currentIndex = mCurrentIndex;
        for (size_t i = 0; i < n; i++) {
            if (UTILS_UNLIKELY(currentIndex >= RAW_INDEX_COUNT || freeList.size() >= MIN_FREE_INDICES)) {

                // this could only happen if we had gone through all the indices at least once
                if (UTILS_UNLIKELY(freeList.empty())) {
                    // return the null entity
                    entities[i] = {};
                    continue;
                }

                index = freeList.front();
                freeList.pop_front();
            } else {
                index = currentIndex++;
            }
            entities[i] = Entity{ makeIdentity(gens[index], index) };
#if FILAMENT_UTILS_TRACK_ENTITIES
            mDebugActiveEntities.emplace(entities[i], CallStack::unwind(5));
#endif
        }
        mCurrentIndex = currentIndex;
    }

    UTILS_NOINLINE
    void destroy(size_t const n, Entity* entities) noexcept {
        auto& freeList = mFreeList;
        uint8_t* const gens = mGens;

        std::unique_lock lock(mFreeListLock);
        for (size_t i = 0; i < n; i++) {
            if (!entities[i]) {
                // behave like free(), ok to free null Entity.
                continue;
            }

            // it's an error to delete an Entity twice...
            bool const isAlive = getGeneration(entities[i]) == mGens[getIndex(entities[i])];
            assert(isAlive);

            // ... deleting a dead Entity will corrupt the internal state, so we protect ourselves
            // against it. We don't guarantee anything about external state -- e.g. the listeners
            // will be called.
            if (UTILS_LIKELY(isAlive)) {
                Entity::Type const index = getIndex(entities[i]);
                freeList.push_back(index);
                gens[index]++;

#if FILAMENT_UTILS_TRACK_ENTITIES
                mDebugActiveEntities.erase(entities[i]);
#endif
                mDirtyEntities[mDirtyCount++] = entities[i];
                if (mDirtyCount == MAX_DIRTY_COUNT) {
                    Entity localBuffer[MAX_DIRTY_COUNT];
                    std::copy(mDirtyEntities, mDirtyEntities + MAX_DIRTY_COUNT, localBuffer);
                    mDirtyCount = 0;
                    lock.unlock();

                    triggerChangeCallbacks(localBuffer, MAX_DIRTY_COUNT);

                    lock.lock();
                }
            }
        }
        lock.unlock();

        // notify our listeners that some entities are being destroyed
        auto listeners = getListeners();
        for (auto const& l : listeners) {
            l->onEntitiesDestroyed(n, entities);
        }
    }

    void registerListener(Listener* l) noexcept {
        std::lock_guard const lock(mListenerLock);
        mListeners.insert(l);
    }

    void unregisterListener(Listener* l) noexcept {
        std::lock_guard const lock(mListenerLock);
        mListeners.erase(l);
    }

    void registerChangeCallback(void const* token, ChangeCallback callback) noexcept {
        std::lock_guard const lock(mListenerLock);
        mChangeCallbacks.push_back({token, std::move(callback)});
    }

    void unregisterChangeCallback(void const* token) noexcept {
        std::lock_guard const lock(mListenerLock);
        mChangeCallbacks.erase(
                std::remove_if(mChangeCallbacks.begin(), mChangeCallbacks.end(),
                        [token](auto const& info) { return info.token == token; }),
                mChangeCallbacks.end());
    }

    void flushNotifications() noexcept {
        std::unique_lock lock(mFreeListLock);
        if (mDirtyCount > 0) {
            Entity localBuffer[MAX_DIRTY_COUNT];
            assert_invariant(mDirtyCount <= MAX_DIRTY_COUNT);
            std::copy(mDirtyEntities, mDirtyEntities + mDirtyCount, localBuffer);
            size_t const count = mDirtyCount;
            mDirtyCount = 0;
            lock.unlock();

            triggerChangeCallbacks(localBuffer, count);
        }
    }

#if FILAMENT_UTILS_TRACK_ENTITIES
    std::vector<Entity> getActiveEntities() const {
        std::vector<Entity> result(mDebugActiveEntities.size());
        auto p = result.begin();
        for (auto i : mDebugActiveEntities) {
            *p++ = i.first;
        }
        return result;
    }

    void dumpActiveEntities(utils::io::ostream& out) const {
        for (auto i : mDebugActiveEntities) {
            out << "*** Entity " << i.first.getId() << " was allocated at:\n";
            out << i.second;
            out << io::endl;
        }
    }
#endif

private:
    std::vector<ChangeCallback> getChangeCallbacks() const noexcept {
        std::lock_guard const lock(mListenerLock);
        std::vector<ChangeCallback> result;
        result.reserve(mChangeCallbacks.size());
        for (auto const& info : mChangeCallbacks) {
            result.push_back(info.callback);
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

    FixedCapacityVector<Listener*> getListeners() const noexcept {
        std::lock_guard const lock(mListenerLock);
        tsl::robin_set<Listener*> const& listeners = mListeners;
        FixedCapacityVector<Listener*> result(listeners.size());
        result.resize(result.capacity()); // unfortunately this memset()
        std::copy(listeners.begin(), listeners.end(), result.begin());
        return result; // the c++ standard guarantees a move
    }

    static constexpr size_t MAX_DIRTY_COUNT = 16;
    struct CallbackInfo {
        void const* token;
        ChangeCallback callback;
    };

    mutable Mutex mFreeListLock;
    uint32_t mCurrentIndex = 1;
    std::deque<Entity::Type> mFreeList;     // stores indices that got freed
    uint8_t* const mGens;                   // stores the generation of each index.

    mutable Mutex mListenerLock;
    tsl::robin_set<Listener*> mListeners;
    std::vector<CallbackInfo> mChangeCallbacks;
    Entity mDirtyEntities[MAX_DIRTY_COUNT];
    size_t mDirtyCount = 0;

#if FILAMENT_UTILS_TRACK_ENTITIES
    tsl::robin_map<Entity, CallStack, Entity::Hasher> mDebugActiveEntities;
#endif
};

} // namespace utils



#endif // TNT_UTILS_ENTITYMANAGERIMPL_H
