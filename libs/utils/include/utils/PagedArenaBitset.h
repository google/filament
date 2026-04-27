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

#ifndef TNT_UTILS_PAGEARENABITSET_H
#define TNT_UTILS_PAGEARENABITSET_H

#include <utils/compiler.h>
#include <utils/Slice.h>
#include <utils/algorithm.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <vector>

namespace utils {

/**
 * @class PagedArenaBitset
 * @brief A highly optimized, cache-coherent sparse bitset tailored for Generational ECS indices.
 *
 * @details
 * This structure implements a 4-level hierarchical bitset designed to handle large,
 * sparse integer domains (e.g., 2^27) where bits are densely clustered in "islands".
 * It is specifically optimized for Entity Component Systems where Entity IDs combine a
 * monotonically increasing generation value with a dense, recyclable base index.
 *
 * ### Hierarchical Memory Architecture
 * @code
 * ENTITY ID [27 bits]
 * [ Master Word | Master Bit | Summary Bit | Word Idx | Bit Index ]
 * [   3 bits    |   6 bits   |   6 bits    |  6 bits  |   6 bits  ]
 * |             |            |             |          |
 * |             |            |             |          +->  [Level 3: Bit position in u64]
 * |             |            |             +------------>  [Level 3: Word position in Page]
 * |             |            +-------------------------->  [Level 1: Page presence in Summary]
 * |             +--------------------------------------->  [Level 0: Summary Word presence]
 * +------------------------------------------------------> [Level 0: Master Word selection]
 *
 * VISUAL TOPOLOGY:
 * +-------------------------------------------------------+
 * | [Level 0] MASTER MASK (8 x u64)                       | <-- High-level Skip (512 bits)
 * | [ 11110000... ]                                       |
 * +-----------|-------------------------------------------+
 *             v
 * +-------------------------------------------------------+
 * | [Level 1] SUMMARY MASK (512 x u64)                    | <-- Mid-level Skip (4 KB)
 * | [ ...00101100... ]                                    |
 * +-----------|-------------------------------------------+
 *             v
 * +-------------------------------------------------------+
 * | [Level 2] DIRECTORY (32,768 x u16)                    | <-- Logical -> Physical (64 KB)
 * | [ 0 | 5 | X | 2 | X | X | 1 | ... ] (X = Gated/Dirty) |
 * +-----------|-------------------------------------------+
 *             v
 * +-------------------------------------------------------+
 * | [Level 3] ARENA (Contiguous Pages)                    | <-- Dense Storage
 * | +---------------------------------------------------+ |
 * | | Page N:                                           | |
 * | | [ activeWordsMask ] (1 x u64)                     | | <-- Word-level Skip
 * | | [ words          ] (64 x u64)                     | |
 * | +---------------------------------------------------+ |
 * +-------------------------------------------------------+
 * @endcode
 *
 * ### Architecture (The "Trust the Mask" Design)
 * - **Level 0 (Master Mask):** A 512-bit mask where each bit maps 1:1 to a single word of the
 * Summary Mask. Enables ultra-fast O(1) skipping of 256k-bit empty generational gaps.
 * - **Level 1 (Summary Mask):** A 4KB mask where each bit flags the allocation of a single page.
 * - **Level 2 (Directory):** A 64KB array mapping logical page indices to physical arena indices.
 * *Note:* This memory is left uninitialized/garbage. It is strictly gated by the masks.
 * - **Level 3 (Arena):** A contiguous pool holding fixed 520-byte pages. Each page contains
 * 4,096 bits (64 words) and a 64-bit `activeWordsMask` to skip empty words during operations.
 *
 * ### Performance Guarantees & Assumptions
 * - **Zero-Cost Clear:** Because the architecture strictly "Trusts the Mask", initializing
 * or clearing the bitset requires no massive memory allocations or 64KB memsets. It drops
 * overhead to <20ns by only zeroing the small bitmasks.
 * - **Strict Move Semantics:** To prevent catastrophic, silent O(N) memory allocations
 * and L1 cache thrashing inside ECS update loops, implicit copy constructors are strictly
 * disabled. Moves are O(1) and noexcept. If a deep snapshot is required, the explicit
 * `clone()` method must be used.
 * - **Ghost Pages:** Single-bit `remove()` operations intentionally leak empty pages
 * ("ghost pages") to keep the hot-path branchless. These are automatically cleaned up
 * during bulk `intersect`/`difference` operations or via an explicit `defragment()` call.
 * - **Iteration:** Designed strictly for bulk processing. Random probing via `operator[]`
 * is supported but incurs L1/L2 cache misses. For maximum performance, use `extractTo()`
 * to materialize the bits into a flat vector using the Master Mask's hardware intrinsics.
 */
class UTILS_PUBLIC PagedArenaBitset {
public:
    // ======================================================================================
    // PagedArenaBitset Core Configuration
    // --------------------------------------------------------------------------------------
    // These three constants are the absolute foundation of the bitset's geometry.
    // Every mask size, directory size, and struct layout is derived mathematically from these.
    // ======================================================================================

    // The total addressable capacity of the bitset (2^27 = ~134.2 million bits).
    // This defines the maximum permissible Entity Index. It directly dictates the
    // maximum size of the L2 Directory and the span of the Mask hierarchies.
    static constexpr uint32_t DOMAIN_BITS = 27;

    // The allocation granularity of the Arena, expressed as a bit-shift (2^12 = 4096 bits).
    // This is the "Page Size". Every active 4096-bit range allocates exactly one 512-byte
    // Page in memory. This balances L2 Directory footprint against sparse memory overhead.
    static constexpr uint32_t PAGE_SHIFT = 12;

    // The hardware execution width, expressed as a bit-shift (2^6 = 64 bits).
    // This binds the bitset to the CPU's native 64-bit architecture (uint64_t), ensuring
    // that inner loops perfectly align with hardware popcount (tzcnt/blsr) intrinsics.
    static constexpr uint32_t WORD_SHIFT = 6;

    // The maximum number of bitsets that can be processed in a single multi-way operation
    // (e.g., maximum components in an ECS query). Bounds the stack size for hot loops.
    static constexpr size_t MAX_MULTI_WAY_INPUTS = 64;


    PagedArenaBitset();
    ~PagedArenaBitset();

    /**
     * @brief Move constructor. Transfers ownership of all internal heap resources.
     * * @details This operation is O(1) and noexcept. It performs a pointer swap of the
     * underlying directory and arena vectors.
     * * @param rhs The bitset to move from.
     * * @note **Moved-from State (The Zombie):** The @p other object is left in a
     * non-functional state. While primitive members (like `mSize`) are copied and
     * retain their values, the internal heap-allocated buffers are nullified.
     * * @warning **Caveat:** This object is **unrecoverable**. Because there is no
     * resurrection logic, calling `clear()` will not re-allocate the storage
     * required for use. `empty()` may return a false negative if the source was
     * not empty. The only safe operations on the moved-from object are destruction
     * or overwriting it via a new move-assignment.
     */
    PagedArenaBitset(PagedArenaBitset&& rhs) noexcept;

    /**
     * @brief Move assignment operator. Replaces current content with resources from @p other.
     * * @details This operation is O(1) and noexcept. It releases current resources
     * and steals the heap pointers from the source object.
     * * @param rhs The bitset to move from.
     * @return A reference to this bitset.
     * * @note **Moved-from State (The Zombie):** Identical to the move constructor; the
     * source object is left with its metadata intact but its physical storage destroyed.
     * * @warning **Caveat:** Do not attempt to reuse @p other. Any call to `add()`,
     * `remove()`, or `operator[]` will result in an immediate segment fault.
     * Re-initialization via `clear()` is not supported for moved-from objects
     * in this implementation.
     */
    PagedArenaBitset& operator=(PagedArenaBitset&& rhs) noexcept;

    // not copy-assignable
    PagedArenaBitset& operator=(PagedArenaBitset const& rhs) = delete;

    /**
     * @brief Returns a copy of this bitset.
     */
    PagedArenaBitset clone() const;

    /**
     * @brief copies the other bitset into this bitset and returns a reference to this.
     */
    PagedArenaBitset& copyFrom(const PagedArenaBitset& other);

    /**
     * @brief Returns the total number of set bits.
     */
    size_t size() const;

    /**
     * @brief Checks if the bitset has zero bits set.
     */
    bool empty() const;

    /**
     * @brief Tests if a specific bit is set.
     * @param index The bit index to test.
     * @return true if the bit is set, false otherwise.
     */
    bool operator[](uint32_t index) const;

    /**
     * @brief Sets a bit and returns its previous state.
     * @param index The bit index to set.
     * @return true if the bit was already set, false if it was newly set.
     */
    bool fetchAdd(uint32_t index);

    /**
     * @brief Clears a bit and returns its previous state.
     * @param index The bit index to clear.
     * @return true if the bit was set before clearing, false if it was already cleared.
     */
    bool fetchRemove(uint32_t index);

    /**
     * @brief Sets a bit.
     */
    void add(uint32_t index);

    /**
     * @brief Clears a bit.
     */
    void remove(uint32_t index);

    /**
     * @brief Clears all bits, resets the arena, and zeroes internal state.
     */
    void clear();

    /**
     * @brief Eliminates ghost pages and tightly packs active pages in physical memory.
     * @details Scans the logical directory, drops any pages whose popcount is 0, and
     * relocates all remaining pages to perfectly match logical directory order. This
     * guarantees maximum hardware prefetcher efficiency for subsequent iterations.
     */
    void defragment();

    // ----------------------------------------------------------------------------------------------------------------
    // intersection
    // ----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Performs an in-place intersection (this &= other).
     * @param other The bitset to intersect with.
     */
    PagedArenaBitset& intersect(const PagedArenaBitset& other);

    /**
     * @brief Variadic intersection (Multi-way Path).
     * @details Handles 3+ arguments by forwarding to the order-independent span implementation.
     */
    template<typename... Rest>
    static PagedArenaBitset& intersect(PagedArenaBitset* out,
            const PagedArenaBitset& first,
            const PagedArenaBitset& second,
            const PagedArenaBitset& third,
            const Rest&... rest) {
        static_assert((std::is_same_v<PagedArenaBitset, Rest> && ...),
                "All arguments to intersect must be PagedArenaBitset");

        std::array<const PagedArenaBitset*, sizeof...(rest) + 3> inputs = {&first, &second, &third, &rest...};
        return intersectSpan(out, {inputs.data(), inputs.size()});
    }

    template<typename... Rest>
    static PagedArenaBitset intersect(const PagedArenaBitset& first,
            const PagedArenaBitset& second,
            const PagedArenaBitset& third,
            const Rest&... rest) {
        PagedArenaBitset out;
        out.reserve(std::min({
            first.mArena.size(), second.mArena.size(), third.mArena.size(),
            rest.mArena.size()...
        }));
        intersect(&out, first, second, third, rest...);
        return out;
    }

    /**
     * @brief Variadic intersection size (Multi-way Path).
     * @details Handles 2+ arguments by forwarding to the order-independent span implementation.
     */
    template<typename... Rest>
    static uint32_t intersectSize(const PagedArenaBitset& first,
            const PagedArenaBitset& second,
            const Rest&... rest) noexcept {
        static_assert((std::is_same_v<PagedArenaBitset, Rest> && ...),
                "All arguments to intersect must be PagedArenaBitset");

        std::array<const PagedArenaBitset*, sizeof...(rest) + 2> inputs = {&first, &second, &rest...};
        return intersectSizeSpan({inputs.data(), inputs.size()});
    }

    static PagedArenaBitset& intersect(PagedArenaBitset* out, const PagedArenaBitset& a, const PagedArenaBitset& b);

    static PagedArenaBitset intersect(const PagedArenaBitset& a, const PagedArenaBitset& b);

    // ----------------------------------------------------------------------------------------------------------------
    // merge
    // ----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Performs an in-place merge (Union) (this |= other).
     * @param other The bitset to merge with.
     */
    PagedArenaBitset& merge(const PagedArenaBitset& other);

    /**
     * @brief Variadic merge (Union) (Multi-way Path).
     * @details Handles 3+ arguments by forwarding to the order-independent span implementation.
     */
    template<typename... Rest>
    static PagedArenaBitset& merge(PagedArenaBitset* out,
            const PagedArenaBitset& first,
            const PagedArenaBitset& second,
            const PagedArenaBitset& third,
            const Rest&... rest) {
        static_assert((std::is_same_v<PagedArenaBitset, Rest> && ...),
                "All arguments to merge must be PagedArenaBitset");

        std::array<const PagedArenaBitset*, sizeof...(rest) + 3> inputs = {&first, &second, &third, &rest...};
        return mergeSpan(out, {inputs.data(), inputs.size()});
    }

    template<typename... Rest>
    static PagedArenaBitset merge(const PagedArenaBitset& first,
            const PagedArenaBitset& second,
            const PagedArenaBitset& third,
            const Rest&... rest) {
        PagedArenaBitset out;
        // TODO: max is probably too small, but that the minimum we need
        out.reserve(std::max({
            first.mArena.size(), second.mArena.size(), third.mArena.size(),
            rest.mArena.size()...
        }));
        merge(&out, first, second, third, rest...);
        return out;
    }

    /**
     * @brief Variadic merge (union) size (Multi-way Path).
     * @details Handles 2+ arguments by forwarding to the order-independent span implementation.
     */
    template<typename... Rest>
    static uint32_t mergeSize(const PagedArenaBitset& first,
            const PagedArenaBitset& second,
            const Rest&... rest) noexcept {
        static_assert((std::is_same_v<PagedArenaBitset, Rest> && ...),
                "All arguments to merge must be PagedArenaBitset");

        std::array<const PagedArenaBitset*, sizeof...(rest) + 2> inputs = {&first, &second, &rest...};
        return mergeSizeSpan({inputs.data(), inputs.size()});
    }

    static PagedArenaBitset& merge(PagedArenaBitset* out, const PagedArenaBitset& a, const PagedArenaBitset& b);

    static PagedArenaBitset merge(const PagedArenaBitset& a, const PagedArenaBitset& b);

    // ----------------------------------------------------------------------------------------------------------------
    // difference
    // ----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Performs an in-place difference (this &= ~other).
     * @param other The bitset to subtract.
     */
    PagedArenaBitset& difference(const PagedArenaBitset& other);

    /**
     * @brief Calculates the number of bits in `a` that are not set in `b`.
     */
    static uint32_t differenceSize(const PagedArenaBitset& a, const PagedArenaBitset& b) noexcept;

    static PagedArenaBitset& difference(PagedArenaBitset* out, const PagedArenaBitset& a, const PagedArenaBitset& b);

    static PagedArenaBitset difference(const PagedArenaBitset& a, const PagedArenaBitset& b);

    // ----------------------------------------------------------------------------------------------------------------
    // iterations
    // ----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Materializes the set bits into a contiguous array for maximum iteration speed.
     */
    void extractTo(std::vector<uint32_t>& outBuffer) const;

    /**
     * @brief Iterates over all set bits in ascending numerical order.
     * @tparam Func A callable type accepting a uint32_t.
     * @param callback The function to execute for each set bit.
     */
    template <typename Func>
    void forEachSetBit(Func&& callback) const {
        for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
            uint64_t masterMask = mMasterMask[m];
            while (masterMask) {
                int const maskBit = ctz(masterMask);
                masterMask &= masterMask - 1;

                uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;
                uint64_t mask = mSummaryMask[maskIdx];

                while (mask) {
                    int const bit = ctz(mask);
                    mask &= mask - 1;

                    uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;
                    uint16_t const physicalIdx = mDirectory[dirIdx];

                    assert(physicalIdx != INVALID_PAGE && "Summary mask desynchronized");
                    auto const& [activeWordsMask, words] = mArena[physicalIdx];
                    uint64_t activeMask = activeWordsMask;

                    while (activeMask != 0) {
                        int const w = ctz(activeMask);
                        uint64_t word = words[w];
                        while (word) {
                            int const wBit = ctz(word);
                            word &= word - 1;

                            // Calculate the unique Entity ID from the hierarchy indices
                            uint32_t const id = (dirIdx << PAGE_SHIFT) | (w << WORD_SHIFT) | wBit;

                            if constexpr (std::is_convertible_v<std::invoke_result_t<Func, uint32_t>, bool>) {
                                if (!callback(id)) return;
                            } else {
                                callback(id);
                            }
                        }
                        activeMask &= (activeMask - 1);
                    }
                }
            }
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // subset / superset queries
    // ----------------------------------------------------------------------------------------------------------------

    // Optimized Subset Check: Returns true if (subset & superset) == subset
    static bool isSubset(const PagedArenaBitset& subset, const PagedArenaBitset& superset) noexcept;

    static bool isStrictSubset(const PagedArenaBitset& subset, const PagedArenaBitset& superset) noexcept {
        // A strict subset MUST be smaller than the superset
        if (subset.size() >= superset.size()) {
            return false;
        }
        return isSubset(subset, superset);
    }

    bool isSubsetOf(PagedArenaBitset const& superset) const noexcept {
        return isSubset(*this, superset);
    }

    bool isStrictSubsetOf(PagedArenaBitset const& superset) const noexcept {
        return isStrictSubset(*this, superset);
    }

    bool isSupersetOf(const PagedArenaBitset& other) const noexcept {
        return other.isSubsetOf(*this);
    }

    bool isStrictSupersetOf(const PagedArenaBitset& other) const noexcept {
        if (size() <= other.size()) {
            return false;
        }
        return other.isSubsetOf(*this);
    }

    void swap(PagedArenaBitset& other) noexcept;

    friend void swap(PagedArenaBitset& a, PagedArenaBitset& b) noexcept {
        a.swap(b);
    }

    friend PagedArenaBitset exchange(PagedArenaBitset& object, PagedArenaBitset&& newValue);
    friend PagedArenaBitset exchangeAndClear(PagedArenaBitset& object);

private:
    // Directory Size
    static constexpr uint32_t DIR_SIZE = 1U << (DOMAIN_BITS - PAGE_SHIFT);

    // Summary Mask Size (1 bit per Directory Entry)
    // Shifting right by WORD_SHIFT is equivalent to dividing by 64.
    static constexpr uint32_t MASK_WORDS = DIR_SIZE >> WORD_SHIFT;

    // Master Mask Size (1 bit per Summary Mask Word)
    // We still use integer ceiling division here to guarantee at least 1 word
    // in case a small domain results in MASK_WORDS < 64.
    static constexpr uint32_t MASTER_WORDS = (MASK_WORDS + ((1U << WORD_SHIFT) - 1)) >> WORD_SHIFT;

    static constexpr uint32_t WORDS_PER_PAGE = 1U << (PAGE_SHIFT - WORD_SHIFT);
    static constexpr uint32_t WORD_MASK = ((1U << WORD_SHIFT) - 1);
    static constexpr uint16_t INVALID_PAGE = 0xFFFF;

    static_assert(WORDS_PER_PAGE == 64, "WORDS_PER_PAGE must be 64 to match the 64-bit activeWordsMask!");

    // Do NOT pad or align this struct to 64 bytes (576) or 512 bytes.
    // Maintaining a non-cache-line-multiple size (520) creates a natural 
    // memory stagger. In Multi-Way operations (Merge6, Intersect6), this stagger 
    // prevents 4K Cache Set Aliasing and L1 Cache Thrashing, resulting in 
    // up to an 8x performance increase.
    struct Page {
        uint64_t activeWordsMask = 0;
        uint64_t words[WORDS_PER_PAGE] = {};
        void clear();
        uint32_t popcount() const;
    };

    std::vector<uint64_t> mSummaryMask;     // always  4 KiB
    std::vector<uint16_t> mDirectory;       // always 64 KiB
    std::vector<Page> mArena;               // each page is 512 bytes
    std::vector<uint16_t> mFreePages;       // list of free pages
    uint32_t mSize = 0;                     // size of the set (how many bits are 1)
    std::array<uint64_t, MASTER_WORDS> mMasterMask = {};

    PagedArenaBitset(PagedArenaBitset const& rhs);

    uint16_t allocatePage();
    void freePage(uint32_t dirIdx);
    void reserve(size_t size);

    template<typename Collection>
    static PagedArenaBitset& intersectInternal(PagedArenaBitset* out, const Collection& inputs);
    static PagedArenaBitset& intersectSpan(PagedArenaBitset* out, Slice<const PagedArenaBitset* const> inputs);

    template<typename Collection>
    static uint32_t intersectSizeInternal(const Collection& inputs);
    static uint32_t intersectSizeSpan(Slice<const PagedArenaBitset* const> inputs);

    template<typename Collection>
    static PagedArenaBitset& mergeInternal(PagedArenaBitset* out, const Collection& inputs);
    static PagedArenaBitset& mergeSpan(PagedArenaBitset* out, Slice<const PagedArenaBitset* const> inputs);

    template<typename Collection>
    static uint32_t mergeSizeInternal(const Collection& inputs);
    static uint32_t mergeSizeSpan(Slice<const PagedArenaBitset* const> inputs);
};

} // namespace utils

#endif // TNT_UTILS_PAGEARENABITSET_H
