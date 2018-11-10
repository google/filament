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

namespace utils {

EntityManager::EntityManager()
        : mGens(new uint8_t[RAW_INDEX_COUNT]) {
    // initialize all the generations to 0
    std::fill_n(mGens, RAW_INDEX_COUNT, 0);
}

EntityManager::~EntityManager() {
    delete [] mGens;
}

EntityManager& EntityManager::get() noexcept {
    // note: we leak the EntityManager because it's more important that it survives everything else
    // the leak is really not a problem because the process is terminating anyways.
    static EntityManagerImpl* instance = new EntityManagerImpl;
    return *instance;
}

void EntityManager::create(size_t n, Entity* entities) {
    static_cast<EntityManagerImpl *>(this)->create(n, entities);
}

void EntityManager::destroy(size_t n, Entity* entities) noexcept {
    static_cast<EntityManagerImpl *>(this)->destroy(n, entities);
}

void EntityManager::registerListener(EntityManager::Listener* l) noexcept {
    static_cast<EntityManagerImpl *>(this)->registerListener(l);
}

void EntityManager::unregisterListener(EntityManager::Listener* l) noexcept {
    static_cast<EntityManagerImpl *>(this)->unregisterListener(l);
}

} // namespace utils
