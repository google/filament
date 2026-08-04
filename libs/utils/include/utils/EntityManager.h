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

#ifndef FILAMENT_UTILS_TRACK_ENTITIES
#define FILAMENT_UTILS_TRACK_ENTITIES false
#endif

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/ImmutableCString.h>
#include <utils/Mutex.h>
#if FILAMENT_UTILS_TRACK_ENTITIES
#include <utils/ostream.h>
#endif
#include <utils/PagedArenaBitset.h>
#include <utils/Slice.h>

#include <atomic>
#include <vector>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

class SingleInstanceComponentManagerBase;

/**
 * @class EntityManager
 * @brief Thread-safe coordinator managing the lifecycle, indices, and generation tracking for all Entity instances.
 *
 * @details The EntityManager is the core source-of-truth for Entity identities in Filament. It coordinates the
 * creation, recycling, and safe reclamation of 32-bit Entity IDs. Due to the highly multithreaded nature of the
 * engine, the EntityManager provides **two distinct groups of APIs**:
 *
 * ---
 *
 * ### 1. Public API (Gameplay & Application Layer)
 * This group is aimed at standard usage for applications, renderers, and game systems.
 * - **Entity Creation**: Methods like @p create() and @p create(size_t, Entity*) allocate new or recycled Entity
 *   identities.
 * - **Entity Destruction**: Methods like @p destroy(Entity) and @p destroy(size_t, Entity*) logically kill entities,
 *   instantly advancing the active timeline.
 * - **Queries & Hooks**: Mechanisms to query logical presence (@p isAlive()) or subscribe to global destruction
 *   notifications via @p registerChangeCallback().
 *
 * ---
 *
 * ### 2. Internal Component API (ECS Integration & Component Managers)
 * This group supports the underlying asynchronous **Epoch-Based Reclamation (EBR)** garbage collector. Designed
 * exclusively for subclasses of @p SingleInstanceComponentManagerBase (e.g. @p FCameraManager, @p FTransformManager),
 * these methods govern thread-consensus and physical payload purging budgets:
 * - **Consensus Registration**: Methods like @p registerWatermark() and @p unregisterWatermark() allow components to
 *   join or vacate the global timeline consensus.
 * - **Timeline Sweeps**: Methods like @p advanceEpoch() and @p reclaimSafeEpochs() seal completed frames and process
 *   physical reclaiming boundaries.
 * - **Missed Garbage Harvests**: APIs like @p getMissedGarbage() enable Component Managers to retrieve logically dead
 *   Entity bitsets precisely synced against their local watermarks in atomic, collision-free transaction blocks.
 */
class UTILS_PUBLIC EntityManager {
public:
    /**
     * @brief Gets a reference to the global EntityManager singleton.
     * @note It is recommended to cache this reference locally to bypass lookups.
     * @return Reference to the thread-safe global EntityManager.
     */
    static EntityManager& get() noexcept;

    /**
     * @class Listener
     * @brief Abstract base class for receiving global entity lifecycle notifications.
     */
    class Listener {
    public:
        /**
         * @brief Invoked asynchronously after a batch of entities has been globally destroyed.
         * @param[in] n The number of entities contained in the destruction batch.
         * @param[in] entities Pointer to the contiguous array of destroyed Entity identities.
         */
        virtual void onEntitiesDestroyed(size_t n, Entity const* entities) noexcept = 0;
    protected:
        virtual ~Listener() noexcept;
    };

    using ChangeCallback = std::function<void(Slice<const Entity>)>;

    /**
     * @brief Registers a callback to be triggered when entities are destroyed.
     * @param[in] token Unique identifier key for the listener (e.g., 'this' pointer).
     * @param[in] callback The invocable target function to execute.
     */
    void registerChangeCallback(void const* token, ChangeCallback callback) noexcept;

    /**
     * @brief Unregisters a pre-existing change notification callback.
     * @param[in] token The unique identifier key used during registration.
     */
    void unregisterChangeCallback(void const* token) noexcept;

    /**
     * @brief Flushes all pending entity lifecycle change notifications to registered callbacks.
     * @details Invoked automatically upon internal queue thresholds, or manually per frame.
     */
    void flushNotifications() noexcept;

    /**
     * @brief Retrieves the maximum theoretical upper bound of entities that can exist concurrently.
     * @details Factored around reserved index slots and 2^GENERATION_SHIFT limits.
     * @return The maximum available 32-bit Entity identity limit.
     */
    static size_t getMaxEntityCount() noexcept {
        return RAW_INDEX_COUNT - 1;
    }

    /**
     * @brief Returns the total number of currently active/alive Entities.
     * @return Count of logical entities currently populated in the system.
     */
    size_t getEntityCount() const noexcept;

    /**
     * @brief Allocates and creates a batch of 'n' new or recycled Entities.
     * @param[in] n The number of Entity identities to generate.
     * @param[out] entities Pointer to the output array receiving the populated Entity IDs.
     */
    void create(size_t n, Entity* entities);

    /**
     * @brief Globally and logically destroys a batch of 'n' Entities.
     * @param[in] n The number of Entity identities to destroy.
     * @param[in] entities Pointer to the contiguous array of Entities to logically kill.
     */
    void destroy(size_t n, Entity* entities) noexcept;

    /**
     * @brief Allocates and creates a single new or recycled Entity.
     * @return The populated Entity ID, or Entity.isNull() upon allocation failure.
     */
    Entity create() {
        Entity e;
        create(1, &e);
        return e;
    }

    /**
     * @brief Globally and logically destroys a single Entity identity.
     * @param[in] e The Entity to logically kill.
     */
    void destroy(Entity e) noexcept {
        destroy(1, &e);
    }

    /**
     * @brief Queries the logical lifecycle state of a given Entity.
     * @param[in] e The Entity to test for presence.
     * @return True if the Entity is active and logically alive, false if dead/destroyed.
     */
    bool isAlive(Entity e) const noexcept;

    /**
     * @brief Subscribes an abstract Listener to global entity destruction notifications.
     * @param[in] l Pointer to the abstract Listener subclass.
     */
    void registerListener(Listener* l) noexcept;

    /**
     * @brief Unregisters a pre-existing abstract Listener from the system.
     * @param[in] l Pointer to the Listener instance to remove.
     */
    void unregisterListener(Listener* l) noexcept;

    /**
     * @brief Extracts the complete tracking bitset representing all logically alive Entities.
     * @return PagedArenaBitset containing bits for every active 32-bit Entity index.
     */
    PagedArenaBitset getAliveEntities() const noexcept;

    /**
     * @brief Registers a reader's watermark with the Epoch-Based Reclamation (EBR) system.
     *
     * @details Registering a watermark prevents the orchestrator from reclaiming/recycling
     * any Entity indices destroyed in epochs greater than or equal to this watermark.
     * Readers must update their watermarks during their GC phase.
     * Thread-safe.
     *
     * @param watermark Pointer to the reader's atomic watermark.
     */
    void registerWatermark(std::atomic<uint64_t>* watermark, utils::ImmutableCString name = "Unknown",
            const PagedArenaBitset* entityBitset = nullptr, Mutex* entityBitsetLock = nullptr) noexcept;

    /**
     * @brief Unregisters a reader's watermark from the EBR system.
     *
     * @details Unregistering a watermark allows the orchestrator to reclaim epochs without
     * being blocked by this reader. Automatically triggers reclaimSafeEpochs() to recycle
     * any pending garbage that was blocked by this reader.
     * Thread-safe.
     *
     * @param watermark Pointer to the reader's atomic watermark.
     */
    void unregisterWatermark(std::atomic<uint64_t>* watermark) noexcept;

    /**
     * @brief Atomically rebinds a component manager's EBR watermark to a new memory address.
     *
     * @details
     * This method is strictly required to maintain Epoch-Based Reclamation (EBR) consensus
     * during Component Manager lifecycle events, specifically Move Construction and Move Assignment.
     *
     * When a Component Manager is moved, the physical memory address of its internal
     * `std::atomic<uint64_t>` watermark changes. If the manager were to simply call
     * `unregisterWatermark()` followed by `registerWatermark()`, it would create a microscopic
     * race condition where the manager temporarily drops out of the global timeline consensus.
     * * If a background thread calls `advanceEpoch()` during that unlocked window, the
     * EntityManager could miscalculate the `safeThreshold` and prematurely recycle Entity IDs
     * that the moving manager still expects to be safely held in purgatory, leading to
     * Stale State Resurrection bugs.
     *
     * `rebindWatermark` prevents this by acquiring the timeline lock and swapping the pointers
     * atomically. This guarantees that the manager's watermark is continuously tracked and
     * its protection over dead entities is never interrupted.
     *
     * @param oldW Pointer to the watermark's previous memory address.
     * @param newW Pointer to the watermark's new memory address.
     * @param newName The diagnostic name of the component manager, used for tracking EBR stalls.
     *
     * @warning Never substitute this method with an unregister/register pair when moving
     * a Component Manager.
     */
    void rebindWatermark(std::atomic<uint64_t> const* oldW, std::atomic<uint64_t>* newW,
            ImmutableCString newName, const PagedArenaBitset* newEntityBitset = nullptr,
            Mutex* newEntityBitsetLock = nullptr) noexcept;

    /**
     * @brief Advances the timeline to the next epoch.
     *
     * @details Increments the current epoch ID and seals the current epoch. All subsequent
     * entity destructions will be recorded in the new epoch. Automatically triggers
     * reclaimSafeEpochs() to recycle safe, completed epochs.
     * Thread-safe.
     */
    void advanceEpoch() noexcept;

    /**
     * @brief Retrieves all completed, sealed garbage epochs missed by a reader.
     *
     * @details Returns pointers to the PagedArenaBitsets of all epochs in the timeline
     * where `epoch.id > readerWatermark && epoch.id < currentEpochID`.
     * Thread-safe.
     *
     * @note **Important Allocation/Memory Contract**:
     * 1. The vector passed as @p out is explicitly cleared at the start of the query.
     * 2. It reserves necessary capacity and appends all harvested missed epochs directly,
     *    completely avoiding dynamic heap allocation overhead inside the culling/rendering hot paths.
     *
     * @param out The pre-allocated target vector to collect the missed garbage epochs.
     * @param readerWatermark The current watermark of the requesting reader.
     * @return Reference to the populated target vector @p out.
     */
    uint64_t getMissedGarbage(
            std::vector<const PagedArenaBitset*>& out, uint64_t readerWatermark) noexcept;

    // Reclaims and recycles all indices from epochs that are safe to reclaim.
    // Note: This method is intended EXCLUSIVELY for testing, unit test harnesses, and micro-benchmarks 
    // to trigger harvesting sweeps in isolation. In production, Filament NEVER needs to call this method, 
    // as advanceEpoch() automatically and synchronously invokes the identical recycling sweep.
    void reclaimSafeEpochs() noexcept;

    /**
     * @brief Returns the current active (unsealed) epoch ID.
     * Thread-safe.
     *
     * @return The current epoch ID.
     */
    uint64_t getLatestEpochID() const noexcept;

    /* no user serviceable parts below */

    // singleton, can't be copied
    EntityManager(const EntityManager& rhs) = delete;
    EntityManager& operator=(const EntityManager& rhs) = delete;

#if FILAMENT_UTILS_TRACK_ENTITIES
    std::vector<Entity> getActiveEntities() const;
    void dumpActiveEntities(utils::io::ostream& out) const;
#endif

    // use carefully, several entities can have the same index.
    static Entity::Type getIndex(Entity const e) noexcept  {
        return e.getId() & INDEX_MASK;
    }

private:
    friend class EntityManagerImpl;
    friend class SingleInstanceComponentManagerBase;
    EntityManager();
    ~EntityManager();

    // GENERATION_SHIFT determines how many simultaneous Entities are available, the
    // minimum memory requirement is 2^GENERATION_SHIFT bytes.
    // **IMPORTANT**
    // These constants must stay consistent with PagedArenaBitset.h
    static constexpr size_t GENERATION_SHIFT    = Entity::GENERATION_SHIFT;
    static constexpr size_t GENERATION_BITS     = Entity::GENERATION_BITS;
    static constexpr size_t RAW_INDEX_COUNT     = Entity::RAW_INDEX_COUNT;
    static constexpr Entity::Type INDEX_MASK    = Entity::INDEX_MASK;
    static constexpr Entity::Type MAX_IDENTITY  = Entity::MAX_IDENTITY;

    static Entity::Type getGeneration(Entity const e) noexcept {
        return e.getId() >> GENERATION_SHIFT;
    }

    static Entity::Type makeIdentity(Entity::Type const g, Entity::Type const i) noexcept {
        return (g << GENERATION_SHIFT) | (i & INDEX_MASK);
    }
};

} // namespace utils

#endif // TNT_UTILS_ENTITYMANAGER_H
