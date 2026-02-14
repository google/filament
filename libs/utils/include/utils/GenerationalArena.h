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

#ifndef TNT_UTILS_GENERATIONAL_ARENA_H
#define TNT_UTILS_GENERATIONAL_ARENA_H

#include <utils/FixedCapacityVector.h>
#include <utils/compiler.h>

#include <assert.h>
#include <stdint.h>
#include <utility>

namespace utils {

/**
 * A handle into a GenerationalArena.
 *
 * This class is independent of GenerationalArena proper such that we can create
 * a GenerationalArena whose values themselves are dependent types of
 * GenerationalArenaHandle, for example, in the implementation of a linked list.
 */
struct GenerationalArenaHandle {
    using generation_type = uint32_t;
    using size_type = uint32_t;

    static constexpr const generation_type NULL_GENERATION{UINT32_MAX};

    generation_type generation;
    size_type index;

    GenerationalArenaHandle()
            : generation(NULL_GENERATION),
              index(0) {}

    bool operator==(const GenerationalArenaHandle& other) const {
        return generation == other.generation && index == other.index;
    }

    explicit operator bool() const { return generation != NULL_GENERATION; }

    void clear() {
        generation = NULL_GENERATION;
    }
};

/**
 * A container that provides generational handles to objects.
 *
 * Objects are allocated from a fixed-capacity pool. Each allocation returns a
 * GenerationalArenaHandle that contains an index and a generation ID. When an
 * object is freed, its slot is reused and the generation ID is incremented.
 * This allows GenerationArenaHandles to be validated to ensure they still refer
 * to the original object.
 *
 * @tparam T The type of object to store.
 */
template <typename T, typename A = std::allocator<T>>
class GenerationalArena {
public:
    using allocator_type = A;
    using generation_type = GenerationalArenaHandle::generation_type;
    using size_type = GenerationalArenaHandle::size_type;

private:
    struct Entry;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using EntryAllocator = allocator_traits::template rebind_alloc<Entry>;

public:
    /**
     * Creates an arena with the specified capacity.
     * The arena initially contains no objects.
     */
    explicit GenerationalArena(size_type capacity,
            const allocator_type& allocator = allocator_type())
            : mAllocator(allocator),
              mData(EntryAllocator(mAllocator).allocate(capacity)),
              mCapacity(capacity),
              mFreeHead(0) {
        const size_type last = capacity - 1;
        for (size_type i = 0; i < last; i++) {
            Entry& entry = mData[i];
            entry.generation = FREE_FLAG;
            entry.u.nextFree = i + 1;
        }
        Entry& lastEntry = mData[last];
        lastEntry.generation = FREE_FLAG;
        lastEntry.u.nextFree = INVALID_INDEX;
    }

    /**
     * Destroys the arena and all contained objects.
     */
    ~GenerationalArena() {
        destroy();
    }

    GenerationalArena(GenerationalArena&& other) noexcept
            : mAllocator(other.mAllocator),
              mData(other.mData),
              mCapacity(other.mCapacity),
              mFreeHead(other.mFreeHead) {
        other.mData = nullptr;
        other.mCapacity = 0;
        other.mFreeHead = INVALID_INDEX;
    }

    GenerationalArena& operator=(GenerationalArena&& other) noexcept {
        if (this != &other) {
            destroy();

            mAllocator = other.mAllocator;
            mData = other.mData;
            mFreeHead = other.mFreeHead;

            other.mData = nullptr;
            other.mCapacity = 0;
            other.mFreeHead = INVALID_INDEX;
        }
        return *this;
    }

    GenerationalArena(const GenerationalArena&) = delete;
    GenerationalArena& operator=(const GenerationalArena&) = delete;

    /**
     * Allocates a new object in the arena, constructing it with the given
     * arguments. Returns a GenerationalArenaHandle to the object.
     */
    template<typename... Args>
    GenerationalArenaHandle allocate(Args&&... args) {
        assert(mFreeHead != INVALID_INDEX);

        Entry& entry = mData[mFreeHead];
        assert(!entry);
        const size_type nextFree = entry.u.nextFree;

        try {
            // Construct T.
            allocator_traits::construct(mAllocator, &entry.u.value,
                    std::forward<Args>(args)...);
        } catch (...) {
            // nextFree may have been overwritten by the constructor, so restore
            // the recorded value.
            entry.u.nextFree = nextFree;
            throw;
        }

        // Record the next free head for the next allocate() call.
        const size_type index = mFreeHead;
        mFreeHead = nextFree;

        // Flag entry as allocated and return a GenerationalArenaHandle.
        entry.generation &= GENERATION_MASK;
        return {
            .generation = entry.generation,
            .index = index,
        };
    }

    /**
     * Frees the object referred to by the given GenerationalArenaHandle.
     *
     * The generation of the slot is incremented. The GenerationalArenaHandle
     * must be valid (i.e. get(GenerationalArenaHandle) would return non-null).
     */
    void free(GenerationalArenaHandle handle) {
        assert(handle.index < mCapacity);

        Entry& entry = mData[handle.index];

        if (entry.generation != handle.generation || !entry) {
            // Double free.
            // TODO: warn here?
            return;
        }

        entry.generation = (entry.generation + 1) | FREE_FLAG;

        // Destroy the object and use its slot as the new free head.
        try {
            allocator_traits::destroy(mAllocator, &entry.u.value);
            entry.u.nextFree = mFreeHead;
            mFreeHead = handle.index;
        } catch (...) {
            entry.u.nextFree = mFreeHead;
            mFreeHead = handle.index;
            throw;
        }
    }

    /**
     * Returns a pointer to the object if the GenerationalArenaHandle is valid,
     * otherwise nullptr.
     */
    T* get(GenerationalArenaHandle handle) {
        assert(handle.index < mCapacity);
        Entry& entry = mData[handle.index];
        if (entry.generation != handle.generation) {
            return nullptr;
        }
        return &entry.u.value;
    }

    /**
     * Returns a const pointer to the object if the GenerationalArenaHandle is
     * valid, otherwise nullptr.
     */
    T const* get(GenerationalArenaHandle handle) const {
        assert(handle.index < mCapacity);
        const Entry& entry = mData[handle.index];
        if (entry.generation != handle.generation) {
            return nullptr;
        }
        return &entry.u.value;
    }

private:
    static constexpr generation_type FREE_FLAG = 0x80000000u;
    static constexpr generation_type GENERATION_MASK = ~FREE_FLAG;
    static constexpr generation_type INVALID_INDEX = UINT32_MAX;

    struct Entry {
        // If FREE_FLAG is set, this entry is freed and the union holds
        // nextFree.
        generation_type generation;
        union {
            T value;
            size_type nextFree;
        } u;

        // Return true if non-free.
        explicit operator bool() const {
            return !(generation & FREE_FLAG);
        }
    };

    // Don't use a FixedCapacityVector here because we want to construct this
    // uninitialized. Unfortunately, even though the constructor/destructor of
    // Entry is a no-op due to the use of a union,
    // std::is_trivially_constructible still returns true, which causes
    // FixedCapacityVector to do a lot of unnecessary initialization.
    allocator_type mAllocator{};
    Entry* mData{};
    size_type mCapacity{};
    size_type mFreeHead = INVALID_INDEX;

    // Free any members of mData which have been constructed and frees mData.
    void destroy() {
        for (size_type i = 0; i < mCapacity; i++) {
            Entry& entry = mData[i];
            if (entry) {
                allocator_traits::destroy(mAllocator, &entry.u.value);
            }
        }
        EntryAllocator(mAllocator).deallocate(mData, mCapacity);
    }
};

} // namespace utils

#endif // TNT_UTILS_GENERATIONAL_ARENA_H
