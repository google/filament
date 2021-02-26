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
#include <utils/Entity.h>
#include <utils/Mutex.h>
#include <utils/CallStack.h>
#include <utils/vector.h>

#include <tsl/robin_set.h>

#if FILAMENT_UTILS_TRACK_ENTITIES
#include <tsl/robin_map.h>
#endif

#include <deque>
#include <mutex> // for std::lock_guard
#include <vector>


namespace utils {

static constexpr const size_t MIN_FREE_INDICES = 1024;

class UTILS_PRIVATE EntityManagerImpl : public EntityManager {
public:
    using EntityManager::getGeneration;
    using EntityManager::getIndex;
    using EntityManager::makeIdentity;
    using EntityManager::create;
    using EntityManager::destroy;

    void create(size_t n, Entity* entities) {
        Entity::Type index{};
        auto& freeList = mFreeList;
        uint8_t* const gens = mGens;

        // this must be thread-safe, acquire the free-list mutex
        std::lock_guard<Mutex> lock(mFreeListLock);
        Entity::Type currentIndex = mCurrentIndex;
        for (size_t i = 0; i < n; i++) {
            // If we have more than a certain number of freed indices, get one from the list.
            // this is a trade-off between how often we recycle indices and how large the free list
            // can grow.
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
                // In the common case, we just grab the next index.
                // This works only until all indices have been used once, at which point
                // we're always in the slower case above. The idea is that we have enough indices
                // that it doesn't happen in practice.
                index = currentIndex++;
            }
            entities[i] = Entity{ makeIdentity(gens[index], index) };
#if FILAMENT_UTILS_TRACK_ENTITIES
            mDebugActiveEntities.emplace(entities[i], CallStack::unwind(5));
#endif
        }
        mCurrentIndex = currentIndex;
    }

    void destroy(size_t n, Entity* entities) noexcept {
        auto& freeList = mFreeList;
        uint8_t* const gens = mGens;

        std::unique_lock<Mutex> lock(mFreeListLock);
        for (size_t i = 0; i < n; i++) {
            if (!entities[i]) {
                // behave like free(), ok to free null Entity.
                continue;
            }

            // it's an error to delete an Entity twice...
            assert(isAlive(entities[i]));

            // ... deleting a dead Entity will corrupt the internal state, so we protect ourselves
            // against it. We don't guarantee anything about external state -- e.g. the listeners
            // will be called.
            if (isAlive(entities[i])) {
                Entity::Type index = getIndex(entities[i]);
                freeList.push_back(index);

                // The generation update doesn't require the lock because it's only used for isAlive()
                // and entities work as weak references -- it just means that isAlive() could return
                // true a little longer than expected in some other threads.
                // We do need a memory fence though, it is provided by the mFreeListLock.unlock() below.
                gens[index]++;

#if FILAMENT_UTILS_TRACK_ENTITIES
                mDebugActiveEntities.erase(entities[i]);
#endif
            }
        }
        lock.unlock();

        // notify our listeners that some entities are being destroyed
        auto listeners = getListeners();
        for (auto const& l : listeners) {
            l->onEntitiesDestroyed(n, entities);
        }
    }

    void registerListener(EntityManager::Listener* l) noexcept {
        std::lock_guard<Mutex> lock(mListenerLock);
        mListeners.insert(l);
    }

    void unregisterListener(EntityManager::Listener* l) noexcept {
        std::lock_guard<Mutex> lock(mListenerLock);
        mListeners.erase(l);
    }

    utils::vector<EntityManager::Listener*> getListeners() const noexcept {
        std::unique_lock<Mutex> lock(mListenerLock);
        tsl::robin_set<Listener*> const& listeners = mListeners;
        utils::vector<EntityManager::Listener*> result(listeners.size()); // unfortunately this memset()
        std::copy(listeners.begin(), listeners.end(), result.begin());
        return result; // the c++ standard guarantees a move
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
    uint32_t mCurrentIndex = 1;

    // stores indices that got freed
    mutable Mutex mFreeListLock;
    std::deque<Entity::Type> mFreeList;

    mutable Mutex mListenerLock;
    tsl::robin_set<Listener*> mListeners;

#if FILAMENT_UTILS_TRACK_ENTITIES
    tsl::robin_map<Entity, CallStack> mDebugActiveEntities;
#endif
};

} // namespace utils



#endif // TNT_UTILS_ENTITYMANAGERIMPL_H
