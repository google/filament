/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_UTILS_ENTITYINSTANCEMAP_H
#define TNT_UTILS_ENTITYINSTANCEMAP_H

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/EntityInstance.h>
#include <utils/SparsePagedArray.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace utils {

/**
 * A generation-safe, 2-level sparse paged map from `utils::Entity` to
 * `utils::EntityInstanceBase::Type` (`Instance`).
 *
 * Each 32-bit slot in the underlying `SparsePagedArray` packs the entity's upper generation bits
 * (`GENERATION_MASK = ~INDEX_MASK`) at the exact same bit position as `Entity::getId()`, and the
 * 17-bit non-zero `Instance` into the lower 17 bits (`INDEX_MASK`). An empty slot has value `0`.
 *
 * Because unallocated pages in `SparsePagedArray` point to a shared static zero page, `getInstance`
 * performs a completely branchless 2-level lookup and generation comparison.
 *
 * @tparam FILAMENT_PAGE_SHIFT Log2 of the number of entries per page (default 10 = 1024 entries = 4 KB/page).
 * @tparam Allocator           STL-compatible allocator used to allocate and deallocate pages.
 */
template <
        size_t FILAMENT_PAGE_SHIFT = 10,
        typename Allocator = std::allocator<uint32_t>>
class UTILS_PUBLIC EntityInstanceMap {
public:
    using Instance = EntityInstanceBase::Type;
    using ArrayType = SparsePagedArray<uint32_t, FILAMENT_PAGE_SHIFT, FILAMENT_GENERATION_SHIFT, Allocator>;

    static constexpr size_t DOMAIN_BITS = FILAMENT_GENERATION_SHIFT;
    static constexpr uint32_t INDEX_MASK = (1u << DOMAIN_BITS) - 1u;
    static constexpr uint32_t GENERATION_MASK = ~INDEX_MASK;

    struct InsertResult {
        uint32_t* slot;
        Instance existing;
        bool inserted;
    };

    EntityInstanceMap() noexcept = default;

    explicit EntityInstanceMap(Allocator const& alloc) noexcept
            : mArray(alloc) {
    }

    ~EntityInstanceMap() noexcept = default;

    EntityInstanceMap(EntityInstanceMap const&) = delete;
    EntityInstanceMap& operator=(EntityInstanceMap const&) = delete;

    EntityInstanceMap(EntityInstanceMap&&) noexcept = default;
    EntityInstanceMap& operator=(EntityInstanceMap&&) noexcept = default;

    /**
     * Branchless `O(1)` lookup of the `Instance` associated with `e`, or `0` if `e` has no entry
     * (including when the slot is empty, its page is unallocated, `e` is null, or the slot is
     * occupied by a different generation of the same 17-bit index).
     */
    Instance getInstance(Entity const e) const noexcept {
        uint32_t const id = e.getId();
        uint32_t const entry = mArray.get(id & INDEX_MASK);
        return ((entry ^ id) & GENERATION_MASK) == 0 ? Instance(entry & INDEX_MASK) : 0;
    }

    /**
     * Looks up the mutable slot for `e` if and only if `e` is currently present in the map with a
     * matching generation. Returns `nullptr` otherwise.
     */
    uint32_t* findSlot(Entity const e) noexcept {
        uint32_t const id = e.getId();
        uint32_t* const slot = mArray.find(id & INDEX_MASK);
        if (UTILS_LIKELY(slot != nullptr)) {
            uint32_t const entry = *slot;
            if (UTILS_LIKELY(entry != 0 && ((entry ^ id) & GENERATION_MASK) == 0)) {
                return slot;
            }
        }
        return nullptr;
    }

    /**
     * Probes or allocates the slot for `e` in a single lookup.
     * - If the slot is empty (`0`), returns `{ &slot, 0, true }`; the caller must then populate
     *   `*slot` via `commitSlot(slot, e, ci)`.
     * - If `e` is already present (matching generation), returns `{ &slot, existingInstance, false }`.
     * - Precondition: any prior generation occupying the same 17-bit index must have been erased
     *   before inserting a new generation at that index.
     */
    InsertResult tryInsertSlot(Entity const e) {
        assert(!e.isNull());
        uint32_t const id = e.getId();
        uint32_t& slot = mArray.getOrAllocate(id & INDEX_MASK);
        uint32_t const entry = slot;
        assert(entry == 0 || ((entry ^ id) & GENERATION_MASK) == 0);
        if (UTILS_LIKELY(entry == 0)) {
            return { &slot, 0, true };
        }
        return { &slot, Instance(entry & INDEX_MASK), false };
    }

    /**
     * Commits a newly allocated `(e, ci)` pair into a slot returned by `tryInsertSlot(e)`.
     */
    static void commitSlot(uint32_t* const slot, Entity const e, Instance const ci) noexcept {
        assert(slot != nullptr);
        *slot = pack(e, ci);
    }

    /**
     * Clears a slot previously returned by `findSlot(e)`.
     */
    static void clearSlot(uint32_t* const slot) noexcept {
        assert(slot != nullptr);
        *slot = 0;
    }

    /**
     * Extracts the `Instance` from a non-null slot pointer returned by `findSlot(e)`.
     */
    static Instance getSlotInstance(uint32_t const* const slot) noexcept {
        assert(slot != nullptr);
        return Instance(*slot & INDEX_MASK);
    }

    /**
     * Updates the `Instance` for an entity `e` that is already known to be present in the map
     * (e.g. during swap-and-pop compaction or `swap(i, j)`).
     */
    void update(Entity const e, Instance const ci) noexcept {
        assert(!e.isNull());
        uint32_t& slot = mArray.getMutableAssumingAllocated(e.getId() & INDEX_MASK);
        assert(slot != 0 && ((slot ^ e.getId()) & GENERATION_MASK) == 0);
        slot = pack(e, ci);
    }

    /**
     * Inserts or overwrites the mapping for `e` with `ci`.
     */
    void insert(Entity const e, Instance const ci) {
        assert(!e.isNull());
        mArray.set(e.getId() & INDEX_MASK, pack(e, ci));
    }

    /**
     * Removes `e` from the map if present with a matching generation. Returns true if removed.
     */
    bool erase(Entity const e) noexcept {
        if (uint32_t* const slot = findSlot(e)) {
            *slot = 0;
            return true;
        }
        return false;
    }

    /**
     * Deallocates all allocated pages and resets all directory entries to the shared zero page.
     */
    void clear() noexcept {
        mArray.clear();
    }

    size_t allocatedPageCount() const noexcept { return mArray.allocatedPageCount(); }
    size_t allocatedBytes() const noexcept { return mArray.allocatedBytes(); }

    ArrayType const& getArray() const noexcept { return mArray; }

    static uint32_t pack(Entity const e, Instance const ci) noexcept {
        assert(!e.isNull());
        assert(ci > 0 && ci <= INDEX_MASK);
        return (e.getId() & GENERATION_MASK) | (ci & INDEX_MASK);
    }

private:
    ArrayType mArray;
};

} // namespace utils

#endif // TNT_UTILS_ENTITYINSTANCEMAP_H
