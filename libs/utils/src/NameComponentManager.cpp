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

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>

namespace utils {

static constexpr size_t NAME = 0;

NameComponentManager::NameComponentManager(EntityManager& em)
        : SingleInstanceComponentManager<CString>(em, "NameComponentManager") {
}

NameComponentManager::~NameComponentManager() = default;

void NameComponentManager::setName(Instance const instance, const char* name) noexcept {
    if (instance) {
        elementAt<NAME>(instance) = CString{ name };
    }
}

const char* NameComponentManager::getName(Instance const instance) const noexcept {
    return elementAt<NAME>(instance).c_str();
}

void NameComponentManager::addComponent(Entity const e) {
    Entity zombie;
    if (UTILS_UNLIKELY(popPendingZombie(e, zombie))) {
        destroy(zombie);
    }
    SingleInstanceComponentManager::addComponent(e);
}

void NameComponentManager::removeComponent(Entity const e) {
    SingleInstanceComponentManager::removeComponent(e);
}

void NameComponentManager::destroyComponents(Entity const* entities, size_t const count) noexcept {
    SingleInstanceComponentManager::removeComponents(entities, count);
}

} // namespace utils
