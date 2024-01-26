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

#ifndef TNT_UTILS_ENTITYMANAGER_H
#define TNT_UTILS_ENTITYMANAGER_H

#include <utils/Entity.h>
#include <utils/compiler.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#ifndef FILAMENT_UTILS_TRACK_ENTITIES
#define FILAMENT_UTILS_TRACK_ENTITIES false
#endif

#if FILAMENT_UTILS_TRACK_ENTITIES
#include <utils/ostream.h>
#include <vector>
#endif

namespace utils {

class UTILS_PUBLIC EntityManager {
public:
    // Get the global EntityManager. It is recommended to cache this value.
    // Thread Safe.
    static EntityManager& get() noexcept;

    class Listener {
    public:
        virtual void onEntitiesDestroyed(size_t n, Entity const* entities) noexcept = 0;
    protected:
        virtual ~Listener() noexcept;
    };

    // maximum number of entities that can exist at the same time
    static size_t getMaxEntityCount() noexcept {
        // because index 0 is reserved, we only have 2^GENERATION_SHIFT - 1 valid indices
        return RAW_INDEX_COUNT - 1;
    }

    // number of active Entities
    size_t getEntityCount() const noexcept;

    // Create n entities. Thread safe.
    void create(size_t n, Entity* entities);

    // destroys n entities. Thread safe.
    void destroy(size_t n, Entity* entities) noexcept;

    // Create a new Entity. Thread safe.
    // Return Entity.isNull() if the entity cannot be allocated.
    Entity create() {
        Entity e;
        create(1, &e);
        return e;
    }

    // Destroys an Entity. Thread safe.
    void destroy(Entity e) noexcept {
        destroy(1, &e);
    }

    // Return whether the given Entity has been destroyed (false) or not (true).
    // Thread safe.
    bool isAlive(Entity e) const noexcept {
        assert(getIndex(e) < RAW_INDEX_COUNT);
        return (!e.isNull()) && (getGeneration(e) == mGens[getIndex(e)]);
    }

    // Registers a listener to be called when an entity is destroyed. Thread safe.
    // If the listener is already registered, this method has no effect.
    void registerListener(Listener* l) noexcept;

    // unregisters a listener.
    void unregisterListener(Listener* l) noexcept;


    /* no user serviceable parts below */

    // current generation of the given index. Use for debugging and testing.
    uint8_t getGenerationForIndex(size_t index) const noexcept {
        return mGens[index];
    }

    // singleton, can't be copied
    EntityManager(const EntityManager& rhs) = delete;
    EntityManager& operator=(const EntityManager& rhs) = delete;

#if FILAMENT_UTILS_TRACK_ENTITIES
    std::vector<Entity> getActiveEntities() const;
    void dumpActiveEntities(utils::io::ostream& out) const;
#endif

private:
    friend class EntityManagerImpl;
    EntityManager();
    ~EntityManager();

    // GENERATION_SHIFT determines how many simultaneous Entities are available, the
    // minimum memory requirement is 2^GENERATION_SHIFT bytes.
    static constexpr const int GENERATION_SHIFT = 17;
    static constexpr const size_t RAW_INDEX_COUNT = (1 << GENERATION_SHIFT);
    static constexpr const Entity::Type INDEX_MASK = (1 << GENERATION_SHIFT) - 1u;

    static inline Entity::Type getGeneration(Entity e) noexcept {
        return e.getId() >> GENERATION_SHIFT;
    }
    static inline Entity::Type getIndex(Entity e) noexcept  {
        return e.getId() & INDEX_MASK;
    }
    static inline Entity::Type makeIdentity(Entity::Type g, Entity::Type i) noexcept {
        return (g << GENERATION_SHIFT) | (i & INDEX_MASK);
    }

    // stores the generation of each index.
    uint8_t * const mGens;
};

} // namespace utils

#endif // TNT_UTILS_ENTITYMANAGER_H
