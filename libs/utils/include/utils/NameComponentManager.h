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


/**
 * \class NameComponentManager NameComponentManager.h utils/NameComponentManager.h
 * \brief Allows clients to associate string labels with entities.
 *
 * To access the name of an existing entity, clients should first use NameComponentManager to get a
 * temporary handle called an \em instance. Please note that instances are ephemeral; clients should
 * store entities, not instances.
 *
 * Usage example:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * auto names = new NameComponentManager(EntityManager::get());
 * names->addComponent(myEntity);
 * names->setName(names->getInstance(myEntity), "Jeanne d'Arc");
 * ...
 * printf("%s\n", names->getName(names->getInstance(myEntity));
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class UTILS_PUBLIC NameComponentManager : public SingleInstanceComponentManager<details::SafeString> {
public:
    using Instance = EntityInstance<NameComponentManager>;

    /**
     * Creates a new name manager associated with the given entity manager.
     *
     * Note that multiple string labels could be associated with each entity simply by
     * creating multiple instances of NameComponentManager.
     */
    explicit NameComponentManager(EntityManager& em);
    ~NameComponentManager();

    /**
     * Checks if the given entity already has a name component.
     */
    using SingleInstanceComponentManager::hasComponent;

    /**
     * Gets a temporary handle that can be used to access the name.
     *
     * @return Non-zero handle if the entity has a name component, 0 otherwise.
     */
    Instance getInstance(Entity e) const noexcept {
        return Instance(SingleInstanceComponentManager::getInstance(e));
    }

    /*! \cond PRIVATE */
    // these are implemented in SingleInstanceComponentManager<>, but we need to
    // reimplement them in each manager, to ensure they are generated in an implementation file
    // for backward binary compatibility reasons.
    size_t getComponentCount() const noexcept;
    Entity const* getEntities() const noexcept;
    void gc(const EntityManager& em, size_t ratio = 4) noexcept;
    /*! \endcond */

    /**
     * Adds a name component to the given entity if it doesn't already exist.
     */
    void addComponent(Entity e);

    /**
     * Removes the name component to the given entity if it exists.
     */
    void removeComponent(Entity e);

    /**
     * Stores a copy of the given string and associates it with the given instance.
     */
    void setName(Instance instance, const char* name) noexcept;

    /**
     * Retrieves the string associated with the given instance, or nullptr if none exists.
     *
     * @return pointer to the copy that was made during setName()
     */
    const char* getName(Instance instance) const noexcept;
};

} // namespace utils

#endif // TNT_UTILS_NAMECOMPONENTMANAGER_H
