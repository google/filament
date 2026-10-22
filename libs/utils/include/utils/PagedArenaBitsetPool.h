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

#ifndef TNT_UTILS_PAGEDARENABITSETPOOL_H
#define TNT_UTILS_PAGEDARENABITSETPOOL_H

#include <utils/PagedArenaBitset.h>

#include <memory>
#include <vector>

namespace utils {

/**
 * @class PagedArenaBitsetPool
 * @brief A zero-allocation, RAII-driven transient resource pool for PagedArenaBitset.
 *
 * @details
 * Frequently allocating and freeing temporary PagedArenaBitsets inside engine hot loops causes
 * heavy L2/L3 cache misses and allocator overhead due to randomized heap placements on Android.
 * This pool acts as a thread-local/thread-isolated high-water mark allocator.
 *
 * Outstanding allocations are checked out as move-only ScopedBitset RAII wrappers, which are
 * automatically cleaned and returned back to the pool's free-list on destruction.
 *
 * ### Usage Example
 * @code
 * PagedArenaBitsetPool pool;
 *
 * void processIntersection(const PagedArenaBitset& inputA, const PagedArenaBitset& inputB) {
 *     // Checkout a temporary bitset
 *     PagedArenaBitsetPool::ScopedBitset temp = pool.get();
 *
 *     // Perform fast boolean math (implicitly converts ScopedBitset to PagedArenaBitset&)
 *     PagedArenaBitset::intersect(&temp, inputA, inputB);
 *
 *     // Iterate the results using ergonomic -> operator
 *     temp->forEachSetBit([](uint32_t index) {
 *         // ...
 *     });
 *
 *     // At the end of scope, 'temp' automatically clears the bitset
 *     // and returns it back to the pool without any heap deallocations!
 * }
 * @endcode
 *
 * @note **Thread Safety:** This pool is intentionally NOT thread-safe to avoid mutex lock contention.
 */
class PagedArenaBitsetPool {
    std::vector<std::unique_ptr<PagedArenaBitset>> mFreeList;

public:
    /**
     * @brief Constructs an empty PagedArenaBitsetPool.
     */
    PagedArenaBitsetPool() noexcept;

    /**
     * @brief Destroys the pool, freeing all cached PagedArenaBitset resources.
     */
    ~PagedArenaBitsetPool() noexcept;

    // Delete copy/move to prevent accidental invalidation of internal pointers
    // (specifically: outstanding ScopedBitsets hold a raw pointer to this pool)
    PagedArenaBitsetPool(const PagedArenaBitsetPool&) = delete;
    PagedArenaBitsetPool& operator=(const PagedArenaBitsetPool&) = delete;
    PagedArenaBitsetPool(PagedArenaBitsetPool&&) = delete;
    PagedArenaBitsetPool& operator=(PagedArenaBitsetPool&&) = delete;

    /**
     * @class ScopedBitset
     * @brief Move-only RAII wrapper that manages checkout/cleanup lifecycle of a pooled PagedArenaBitset.
     */
    class ScopedBitset {
        PagedArenaBitset* mBitset = nullptr;
        PagedArenaBitsetPool* mPool = nullptr;

        /**
         * @brief Constructs a ScopedBitset wrapping a checked-out PagedArenaBitset.
         * @param bitset Reference to the checked-out bitset.
         * @param pool Reference to the parent pool.
         */
        ScopedBitset(PagedArenaBitset& bitset, PagedArenaBitsetPool& pool) noexcept
                : mBitset(&bitset), mPool(&pool) {}

        friend class PagedArenaBitsetPool;

    public:
        /**
         * @brief Constructs an empty, null ScopedBitset wrapper in a moved-from state.
         */
        ScopedBitset() noexcept = default;

        /**
         * @brief Destructs the wrapper, automatically clearing the bitset and returning it to the pool.
         */
        ~ScopedBitset() noexcept;

        /**
         * @brief Move constructor. Transfers ownership of the pooled bitset.
         * @param other The wrapper to move from.
         */
        ScopedBitset(ScopedBitset&& other) noexcept
                : mBitset(other.mBitset), mPool(other.mPool) {
            other.mBitset = nullptr;
            other.mPool = nullptr;
        }

        /**
         * @brief Move assignment operator. Releases any currently owned bitset and transfers ownership from @p other.
         * @param other The wrapper to move from.
         * @return Reference to this wrapper.
         */
        ScopedBitset& operator=(ScopedBitset&& other) noexcept;

        // Deleted Copy Semantics
        ScopedBitset(const ScopedBitset&) = delete;
        ScopedBitset& operator=(const ScopedBitset&) = delete;

        /**
         * @brief Returns a pointer to the underlying PagedArenaBitset.
         */
        PagedArenaBitset* get() const noexcept { return mBitset; }

        /**
         * @brief Dereferences the underlying PagedArenaBitset.
         */
        PagedArenaBitset& operator*() const noexcept { return *mBitset; }

        /**
         * @brief Provides member access to the underlying PagedArenaBitset.
         */
        PagedArenaBitset* operator->() const noexcept { return mBitset; }
        
        /**
         * @brief Transparent conversion to reference for API boundary interoperability.
         */
        operator PagedArenaBitset&() const noexcept { return *mBitset; }

        /**
         * @brief Returns true if this wrapper holds a valid, active PagedArenaBitset checkout.
         */
        explicit operator bool() const noexcept { return mBitset != nullptr; }
    };

    /**
     * @brief Checks out a clean, sterile PagedArenaBitset from the pool.
     * @details If the free-list is empty, a new PagedArenaBitset is allocated and its ownership is retained.
     * @return A ScopedBitset RAII wrapper managing the checked-out bitset.
     */
    [[nodiscard]] ScopedBitset get();

    /**
     * @brief Clears the pool, freeing all internally cached PagedArenaBitset resources back to the OS.
     * @details Call this during low-activity states, epoch transitions, or garbage collection
     * to shrink the high-water mark memory footprint when the pool goes unused.
     *
     * @note **Active Allocations Safety:** Calling clear() while ScopedBitsets are currently checked out
     * is completely safe and defined. Only the idle bitsets currently in the free-list are destroyed.
     * Outstanding ScopedBitsets will continue to function normally and will be cleanly recycled
     * back to the pool upon their destruction.
     */
    void clear() noexcept;

    /**
     * @brief Returns the number of idle PagedArenaBitset resources currently cached in the pool.
     * @return The size of the internal free-list.
     */
    size_t size() const noexcept {
        return mFreeList.size();
    }

private:
    friend class ScopedBitset;
    
    // The Return Function (Called exclusively by ScopedBitset::~ScopedBitset)
    void release(PagedArenaBitset& bitset);
};

} // namespace utils

#endif // TNT_UTILS_PAGEDARENABITSETPOOL_H
