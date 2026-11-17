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

#ifndef TNT_UTILS_SPARSEPAGEDARRAY_H
#define TNT_UTILS_SPARSEPAGEDARRAY_H

#include <utils/compiler.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace utils {

/**
 * A 2-level sparse paged array mapping a bounded integer domain [0, 2^DOMAIN_BITS) to values of
 * a trivial type T.
 *
 * Unallocated pages point to a shared static read-only zero page (`sZeroPage`), making `get()`
 * completely branchless (`O(1)` with no null-pointer checks). Pages are allocated lazily on first
 * write via `getOrAllocate()` / `set()` using the provided STL-compatible `Allocator`, and are
 * released in bulk on `clear()` or destruction.
 *
 * @tparam T                   Trivial element type. `T{}` represents an empty/default entry.
 * @tparam FILAMENT_PAGE_SHIFT Log2 of the number of entries per page (default 10 = 1024 entries = 4 KB for uint32_t).
 * @tparam DOMAIN_BITS         Log2 of the maximum index domain (default 17 = 131,072 entries).
 * @tparam Allocator           STL-compatible allocator used to allocate and deallocate `Page` objects.
 */
template <
        typename T,
        size_t FILAMENT_PAGE_SHIFT = 10,
        size_t DOMAIN_BITS = 17,
        typename Allocator = std::allocator<T>>
class UTILS_PUBLIC SparsePagedArray {
    static_assert(std::is_trivial_v<T>, "SparsePagedArray requires a trivial type T");
    static_assert(DOMAIN_BITS < 32, "DOMAIN_BITS must be < 32 for uint32_t indexing");
    static_assert(FILAMENT_PAGE_SHIFT > 0 && FILAMENT_PAGE_SHIFT <= DOMAIN_BITS,
            "FILAMENT_PAGE_SHIFT must be in (0, DOMAIN_BITS]");

public:
    static constexpr size_t ELEMENTS_PER_PAGE = size_t(1) << FILAMENT_PAGE_SHIFT;
    static constexpr size_t ELEMENT_MASK = ELEMENTS_PER_PAGE - 1u;
    static constexpr size_t DOMAIN_SIZE = size_t(1) << DOMAIN_BITS;
    static constexpr size_t DIR_SIZE = size_t(1) << (DOMAIN_BITS - FILAMENT_PAGE_SHIFT);

    struct Page {
        T entries[ELEMENTS_PER_PAGE];
    };

    using value_type = T;
    using allocator_type = Allocator;
    using PageAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Page>;
    using PageTraits = std::allocator_traits<PageAllocator>;

    SparsePagedArray() noexcept {
        mPages.fill(zeroPage());
    }

    explicit SparsePagedArray(Allocator const& alloc) noexcept
            : mAllocator(alloc) {
        mPages.fill(zeroPage());
    }

    ~SparsePagedArray() noexcept {
        clear();
    }

    SparsePagedArray(SparsePagedArray const&) = delete;
    SparsePagedArray& operator=(SparsePagedArray const&) = delete;

    SparsePagedArray(SparsePagedArray&& rhs) noexcept
            : mPages(rhs.mPages),
              mAllocator(std::move(rhs.mAllocator)) {
        rhs.mPages.fill(zeroPage());
    }

    SparsePagedArray& operator=(SparsePagedArray&& rhs) noexcept {
        if (UTILS_LIKELY(this != &rhs)) {
            clear();
            if constexpr (PageTraits::propagate_on_container_move_assignment::value &&
                    std::is_move_assignable_v<PageAllocator>) {
                mAllocator = std::move(rhs.mAllocator);
                mPages = rhs.mPages;
                rhs.mPages.fill(zeroPage());
            } else if (mAllocator == rhs.mAllocator) {
                mPages = rhs.mPages;
                rhs.mPages.fill(zeroPage());
            } else {
                for (size_t i = 0; i < DIR_SIZE; ++i) {
                    if (rhs.mPages[i] != &sZeroPage) {
                        Page* const page = PageTraits::allocate(mAllocator, 1);
                        std::copy_n(rhs.mPages[i]->entries, ELEMENTS_PER_PAGE, page->entries);
                        mPages[i] = page;
                    }
                }
                rhs.clear();
            }
        }
        return *this;
    }

    /**
     * Branchless read of the entry at `index`. Returns `T{}` if the page is unallocated.
     */
    T get(uint32_t const index) const noexcept {
        assert(index < DOMAIN_SIZE);
        return mPages[index >> FILAMENT_PAGE_SHIFT]->entries[index & ELEMENT_MASK];
    }

    T operator[](uint32_t const index) const noexcept {
        return get(index);
    }

    /**
     * Returns a pointer to the slot at `index` if its page is allocated, or `nullptr` if the page
     * is unallocated. Does not allocate.
     */
    T* find(uint32_t const index) noexcept {
        assert(index < DOMAIN_SIZE);
        Page* const page = mPages[index >> FILAMENT_PAGE_SHIFT];
        return page != &sZeroPage ? &page->entries[index & ELEMENT_MASK] : nullptr;
    }

    T const* find(uint32_t const index) const noexcept {
        assert(index < DOMAIN_SIZE);
        Page const* const page = mPages[index >> FILAMENT_PAGE_SHIFT];
        return page != &sZeroPage ? &page->entries[index & ELEMENT_MASK] : nullptr;
    }

    /**
     * Returns a mutable reference to the slot at `index`, assuming its page is already allocated.
     * Asserts in debug builds that the page is not the shared zero page.
     */
    T& getMutableAssumingAllocated(uint32_t const index) noexcept {
        assert(index < DOMAIN_SIZE);
        Page* const page = mPages[index >> FILAMENT_PAGE_SHIFT];
        assert(page != &sZeroPage);
        return page->entries[index & ELEMENT_MASK];
    }

    /**
     * Returns a mutable reference to the slot at `index`, lazily allocating and zero-initializing
     * the underlying page if it is not yet allocated.
     */
    T& getOrAllocate(uint32_t const index) {
        assert(index < DOMAIN_SIZE);
        uint32_t const dirIdx = index >> FILAMENT_PAGE_SHIFT;
        Page* page = mPages[dirIdx];
        if (UTILS_UNLIKELY(page == &sZeroPage)) {
            page = allocatePage(dirIdx);
        }
        return page->entries[index & ELEMENT_MASK];
    }

    /**
     * Writes `value` into the slot at `index`, lazily allocating the underlying page if needed.
     */
    void set(uint32_t const index, T const value) {
        getOrAllocate(index) = value;
    }

    /**
     * Deallocates all allocated pages and resets all directory entries to the shared zero page.
     */
    void clear() noexcept {
        for (Page*& page : mPages) {
            if (UTILS_UNLIKELY(page != &sZeroPage)) {
                PageTraits::deallocate(mAllocator, page, 1);
                page = zeroPage();
            }
        }
    }

    /**
     * Returns the number of currently allocated pages.
     */
    size_t allocatedPageCount() const noexcept {
        size_t count = 0;
        for (Page const* const page : mPages) {
            if (page != &sZeroPage) {
                ++count;
            }
        }
        return count;
    }

    /**
     * Returns the total heap/arena bytes currently allocated for pages (excluding inline directory).
     */
    size_t allocatedBytes() const noexcept {
        return allocatedPageCount() * sizeof(Page);
    }

    PageAllocator const& getPageAllocator() const noexcept { return mAllocator; }

private:
    static constexpr Page* zeroPage() noexcept {
        return const_cast<Page*>(&sZeroPage);
    }

    UTILS_NOINLINE Page* allocatePage(uint32_t const dirIdx) {
        Page* const page = PageTraits::allocate(mAllocator, 1);
        std::fill_n(page->entries, ELEMENTS_PER_PAGE, T{});
        mPages[dirIdx] = page;
        return page;
    }

    // Shared read-only sentinel page for unallocated directory slots. Pointer comparison against
    // `&sZeroPage` assumes a single definition within the linkage unit (standard in C++17 within a
    // single DSO / static build; do not move-assign instances across Windows DLL boundaries with
    // distinct CRT/module images).
    static inline constexpr Page sZeroPage{};

    std::array<Page*, DIR_SIZE> mPages;
    UTILS_NO_UNIQUE_ADDRESS PageAllocator mAllocator;
};

} // namespace utils

#endif // TNT_UTILS_SPARSEPAGEDARRAY_H
