// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_MEMORY_BUMP_ALLOCATOR_H_
#define SRC_TINT_UTILS_MEMORY_BUMP_ALLOCATOR_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <new>
#include <utility>

#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/memory/bitcast.h"

// This file implements a custom allocator & iterator using C-style data access. It is not
// unexpected that -Wunsafe-buffer-usage triggers in this code, since the type of dynamic access
// being used cannot be guaranteed to be safe via static analysis. Attempting to change this code in
// simple ways to quiet these errors either a) negatively affects the performance by introducing
// unneeded copes, or b) uses typing shenanigans to work around the warning that other
// linters/analyses are unhappy with.
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint {

/// A allocator for chunks of memory. The memory is owned by the BumpAllocator. When the
/// BumpAllocator is freed all of the allocated memory is freed.
class BumpAllocator {
    /// BlockHeader is linked list of memory blocks.
    /// Blocks are allocated out of heap memory.
    struct BlockHeader {
        BlockHeader* next;
    };

  public:
    /// The default size for a block's data. Allocations can be greater than this, but smaller
    /// allocations will use this size.
    static constexpr size_t kDefaultBlockDataSize = 64 * 1024;

    /// Constructor
    BumpAllocator() = default;

    /// Move constructor
    /// @param rhs the BumpAllocator to move
    BumpAllocator(BumpAllocator&& rhs) { std::swap(data, rhs.data); }

    /// Move assignment operator
    /// @param rhs the BumpAllocator to move
    /// @return this BumpAllocator
    BumpAllocator& operator=(BumpAllocator&& rhs) {
        if (this != &rhs) {
            Reset();
            std::swap(data, rhs.data);
        }
        return *this;
    }

    /// Destructor
    ~BumpAllocator() { Reset(); }

    /// Allocates @p size_in_bytes from the current block, or from a newly allocated block if the
    /// current block is full.
    /// @param size_in_bytes the number of bytes to allocate
    /// @returns the pointer to the allocated memory or `nullptr` if the memory can not be allocated
    std::byte* Allocate(size_t size_in_bytes) {
        if (DAWN_UNLIKELY(data.current_offset + size_in_bytes < size_in_bytes)) {
            return nullptr;  // integer overflow
        }
        if (data.current_offset + size_in_bytes > data.current_data_size) {
            // Allocate a new block from the heap
            auto* prev_block = data.current;
            size_t data_size = std::max(size_in_bytes, kDefaultBlockDataSize);
            data.current = Bitcast<BlockHeader*>(new (std::nothrow)
                                                     std::byte[sizeof(BlockHeader) + data_size]);
            if (DAWN_UNLIKELY(!data.current)) {
                return nullptr;  // out of memory
            }
            data.current->next = nullptr;
            data.current_data_size = data_size;
            data.current_offset = 0;
            if (prev_block) {
                prev_block->next = data.current;
            } else {
                data.root = data.current;
            }
        }

        auto* base = Bitcast<std::byte*>(data.current) + sizeof(BlockHeader);
        auto* ptr = base + data.current_offset;
        data.current_offset += size_in_bytes;
        data.count++;
        return ptr;
    }

    /// Frees all allocations from the allocator.
    void Reset() {
        auto* block = data.root;
        while (block != nullptr) {
            auto* next = block->next;
            delete[] Bitcast<std::byte*>(block);
            block = next;
        }
        data = {};
    }

    /// @returns the total number of allocations
    size_t Count() const { return data.count; }

  private:
    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator& operator=(const BumpAllocator&) = delete;

    struct {
        /// The root block of the block linked list
        BlockHeader* root = nullptr;
        /// The current (end) block of the blocked linked list.
        /// New allocations come from this block
        BlockHeader* current = nullptr;
        /// The byte offset in #current for the next allocation.
        size_t current_offset = 0;
        /// The size of the #current, excluding the header size
        size_t current_data_size = 0;
        /// Total number of allocations
        size_t count = 0;
    } data;
};

}  // namespace tint

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

#endif  // SRC_TINT_UTILS_MEMORY_BUMP_ALLOCATOR_H_
