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

#ifndef TNT_UTILS_NAMECOMPONENTMANAGER_H
#define TNT_UTILS_NAMECOMPONENTMANAGER_H

#include <stddef.h>
#include <stdint.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/EntityInstance.h>
#include <utils/SingleInstanceComponentManager.h>

namespace utils {

class EntityManager;

namespace details {
class SafeString {
public:
    SafeString() noexcept = default;
    explicit SafeString(const char* str) noexcept : mCStr(strdup(str)) { }
    SafeString(SafeString&& rhs) noexcept : mCStr(rhs.mCStr) { rhs.mCStr = nullptr; }
    SafeString& operator=(SafeString&& rhs) noexcept {
        mCStr = rhs.mCStr;
        rhs.mCStr = nullptr;
        return *this;
    }
    ~SafeString() { free((void*)mCStr); }
    const char* c_str() const noexcept { return mCStr; }

private:
    char const* mCStr = nullptr;
};
} // namespace details


class NameComponentManager : public SingleInstanceComponentManager<details::SafeString> {
public:
    using Instance = EntityInstance<NameComponentManager>;

    explicit NameComponentManager(EntityManager& em);
    ~NameComponentManager();

    using SingleInstanceComponentManager::hasComponent;

    Instance getInstance(Entity e) const noexcept {
        return Instance(SingleInstanceComponentManager::getInstance(e));
    }

    // these are implemented in SingleInstanceComponentManager<>, but we need to
    // reimplement them in each manager, to ensure they are generated in an implementation file
    // for backward binary compatibility reasons.
    size_t getComponentCount() const noexcept;
    Entity const* getEntities() const noexcept;
    void addComponent(Entity e);
    void removeComponent(Entity e);
    void gc(const EntityManager& em, size_t ratio = 4) noexcept;

    void setName(Instance instance, const char* name) noexcept;
    const char* getName(Instance instance) const noexcept;
};

} // namespace utils

#endif // TNT_UTILS_NAMECOMPONENTMANAGER_H
