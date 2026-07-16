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

#include "EntityManagerImpl.h"

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>

#include <cassert>
#include <cstddef>
#include <mutex>
#include <new>
#include <utility>

namespace utils {

EntityManager::Listener::~Listener() noexcept = default;

EntityManager::EntityManager() = default;

EntityManager::~EntityManager() = default;

EntityManager& EntityManager::get() noexcept {
    // note: we leak the EntityManager because it's more important that it survives everything else
    // the leak is really not a problem because the process is terminating anyway.
    static EntityManagerImpl* instance = new(std::nothrow) EntityManagerImpl;
    return *instance;
}

void EntityManager::create(size_t const n, Entity* entities) {
    static_cast<EntityManagerImpl *>(this)->create(n, entities);
}

void EntityManager::destroy(size_t const n, Entity* entities) noexcept {
    static_cast<EntityManagerImpl *>(this)->destroy(n, entities);
}

void EntityManager::registerListener(Listener* l) noexcept {
    static_cast<EntityManagerImpl *>(this)->registerListener(l);
}

void EntityManager::unregisterListener(Listener* l) noexcept {
    static_cast<EntityManagerImpl *>(this)->unregisterListener(l);
}

void EntityManager::registerChangeCallback(void const* token, ChangeCallback callback) noexcept {
    static_cast<EntityManagerImpl *>(this)->registerChangeCallback(token, std::move(callback));
}

void EntityManager::unregisterChangeCallback(void const* token) noexcept {
    static_cast<EntityManagerImpl *>(this)->unregisterChangeCallback(token);
}

void EntityManager::flushNotifications() noexcept {
    static_cast<EntityManagerImpl *>(this)->flushNotifications();
}

size_t EntityManager::getEntityCount() const noexcept {
    return static_cast<EntityManagerImpl const *>(this)->getEntityCount();
}

#if FILAMENT_UTILS_TRACK_ENTITIES
std::vector<Entity> EntityManager::getActiveEntities() const {
    return static_cast<EntityManagerImpl const *>(this)->getActiveEntities();
}

void EntityManager::dumpActiveEntities(utils::io::ostream& out) const {
    static_cast<EntityManagerImpl const *>(this)->dumpActiveEntities(out);
}

#endif

bool EntityManager::isAlive(Entity const e) const noexcept {
    return static_cast<EntityManagerImpl const *>(this)->isAlive(e);
}

PagedArenaBitset EntityManager::getAliveEntities() const noexcept {
    auto const* impl = static_cast<EntityManagerImpl const*>(this);
    LockGuard const lock(impl->mDestructionLock);
    return impl->mAliveEntities.clone();
}

void EntityManager::registerWatermark(std::atomic<uint64_t>* watermark, ImmutableCString name,
        const PagedArenaBitset* entityBitset, Mutex* entityBitsetLock) noexcept {
    static_cast<EntityManagerImpl *>(this)->registerWatermark(watermark, std::move(name), entityBitset, entityBitsetLock);
}

void EntityManager::unregisterWatermark(std::atomic<uint64_t>* watermark) noexcept {
    static_cast<EntityManagerImpl *>(this)->unregisterWatermark(watermark);
}

void EntityManager::rebindWatermark(std::atomic<uint64_t> const* oldW, std::atomic<uint64_t>* newW,
        ImmutableCString newName, const PagedArenaBitset* newEntityBitset,
        Mutex* newEntityBitsetLock) noexcept {
    static_cast<EntityManagerImpl *>(this)->rebindWatermark(oldW, newW, std::move(newName), newEntityBitset, newEntityBitsetLock);
}

void EntityManager::advanceEpoch() noexcept {
    static_cast<EntityManagerImpl *>(this)->advanceEpoch();
}

uint64_t EntityManager::getMissedGarbage(
        std::vector<const PagedArenaBitset*>& out, uint64_t readerWatermark) noexcept {
    return static_cast<EntityManagerImpl *>(this)->getMissedGarbage(out, readerWatermark);
}

void EntityManager::reclaimSafeEpochs() noexcept {
    static_cast<EntityManagerImpl *>(this)->reclaimSafeEpochs();
}

uint64_t EntityManager::getLatestEpochID() const noexcept {
    return static_cast<EntityManagerImpl const *>(this)->getLatestEpochID();
}

} // namespace utils
