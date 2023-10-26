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

#include <utils/NameComponentManager.h>
#include <utils/EntityManager.h>

namespace utils {

static constexpr size_t NAME = 0;

NameComponentManager::NameComponentManager(EntityManager&) {
}

NameComponentManager::~NameComponentManager() = default;

void NameComponentManager::setName(Instance instance, const char* name) noexcept {
    if (instance) {
        elementAt<NAME>(instance) = CString{ name };
    }
}

const char* NameComponentManager::getName(Instance instance) const noexcept {
    return elementAt<NAME>(instance).c_str();
}

void NameComponentManager::addComponent(Entity e) {
    SingleInstanceComponentManager::addComponent(e);
}

void NameComponentManager::removeComponent(Entity e) {
    SingleInstanceComponentManager::removeComponent(e);
}

} // namespace utils
