// Copyright 2024 The langsvr Authors
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

#ifndef SRC_LANGSVR_UTILS_BLOCK_ALLOCATOR_H_
#define SRC_LANGSVR_UTILS_BLOCK_ALLOCATOR_H_

#include <stdint.h>
#include <array>
#include <cstring>
#include <utility>

namespace langsvr {

/// @param alignment the next multiple to round `value` to
/// @param value the value to round to the next multiple of `alignment`
/// @return `value` rounded to the next multiple of `alignment`
/// @note `alignment` must be positive. An alignment of zero will cause a DBZ.
template <typename T>
inline constexpr T RoundUp(T alignment, T value) {
    return ((value + alignment - 1) / alignment) * alignment;
}

/// Bitcast performs a cast of `from` to the `TO` type using a memcpy.
/// This unsafe cast avoids triggering Clang's Control Flow Integrity checks.
/// See: https://clang.llvm.org/docs/ControlFlowIntegrity.html#bad-cast-checking
/// @param from the value to cast
/// @tparam TO the value to cast to
/// @returns the cast value
template <typename TO, typename FROM>
inline TO Bitcast(FROM&& from) {
    static_assert(sizeof(FROM) == sizeof(TO));
    // gcc warns in cases where either TO or FROM are classes, even if they are trivially
    // copyable. We avoid this by asserting that both types are indeed trivially copyable, and
    // casting both args to std::byte*.
    static_assert(std::is_trivially_copyable_v<std::decay_t<FROM>>);
    static_assert(std::is_trivially_copyable_v<std::decay_t<TO>>);
    TO to;
    memcpy(reinterpret_cast<std::byte*>(&to), reinterpret_cast<const std::byte*>(&from),
           sizeof(TO));
    return to;
}

/// A container and allocator of objects of (or deriving from) the template type `T`.
/// Objects are allocated by calling Create(), and are owned by the BlockAllocator.
/// When the BlockAllocator is destructed, all constructed objects are automatically destructed and
/// freed.
///
/// Objects held by the BlockAllocator can be iterated over using a View.
template <typename T, size_t BLOCK_SIZE = 64 * 1024, size_t BLOCK_ALIGNMENT = 16>
class BlockAllocator {
    /// Pointers is a chunk of T* pointers, forming a linked list.
    /// The list of Pointers are used to maintain the list of allocated objects.
    /// Pointers are allocated out of the block memory.
    struct Pointers {
        static constexpr size_t kMax = 32;
        std::array<T*, kMax> ptrs;
        Pointers* next;
        Pointers* prev;
        size_t count;
    };

    /// Block is linked list of memory blocks.
    /// Blocks are allocated out of heap memory.
    ///
    /// Note: We're not using std::aligned_storage here as this warns / errors on MSVC.
    struct alignas(BLOCK_ALIGNMENT) Block {
        uint8_t data[BLOCK_SIZE];
        Block* next = nullptr;
    };

    // Forward declaration
    template <bool IS_CONST>
    class TView;

    /// An iterator for the objects owned by the BlockAllocator.
    template <bool IS_CONST>
    class TIterator {
        using PointerTy = std::conditional_t<IS_CONST, const T*, T*>;

      public:
        /// Equality operator
        /// @param other the iterator to compare this iterator to
        /// @returns true if this iterator is equal to other
        bool operator==(const TIterator& other) const {
            return ptrs == other.ptrs && idx == other.idx;
        }

        /// Inequality operator
        /// @param other the iterator to compare this iterator to
        /// @returns true if this iterator is not equal to other
        bool operator!=(const TIterator& other) const { return !(*this == other); }

        /// Progress the iterator forward one element
        /// @returns this iterator
        TIterator& operator++() {
            if (ptrs != nullptr) {
                ++idx;
                if (idx >= ptrs->count) {
                    idx = 0;
                    ptrs = ptrs->next;
                }
            }
            return *this;
        }

        /// Progress the iterator backwards one element
        /// @returns this iterator
        TIterator& operator--() {
            if (ptrs != nullptr) {
                if (idx == 0) {
                    ptrs = ptrs->prev;
                    idx = ptrs->count - 1;
                }
                --idx;
            }
            return *this;
        }

        /// @returns the pointer to the object at the current iterator position
        PointerTy operator*() const { return ptrs->ptrs[idx]; }

      private:
        friend TView<IS_CONST>;  // Keep internal iterator impl private.
        explicit TIterator(const Pointers* p, size_t i) : ptrs(p), idx(i) {}

        /// The current Pointers
        const Pointers* ptrs = nullptr;
        /// The current index within #ptrs
        size_t idx = 0;
    };

    /// View provides begin() and end() methods for looping over the objects owned by the
    /// BlockAllocator.
    template <bool IS_CONST>
    class TView {
      public:
        /// @returns an iterator to the beginning of the view
        TIterator<IS_CONST> begin() const {
            return TIterator<IS_CONST>{allocator_->data.pointers.root, 0};
        }

        /// @returns an iterator to the end of the view
        TIterator<IS_CONST> end() const { return TIterator<IS_CONST>{nullptr, 0}; }

      private:
        friend BlockAllocator;  // For BlockAllocator::operator View()
        explicit TView(BlockAllocator const* allocator) : allocator_(allocator) {}
        BlockAllocator const* const allocator_;
    };

  public:
    /// A forward-iterator type over the objects of the BlockAllocator
    using Iterator = TIterator</* const */ false>;

    /// An immutable forward-iterator type over the objects of the BlockAllocator
    using ConstIterator = TIterator</* const */ true>;

    /// View provides begin() and end() methods for looping over the objects owned by the
    /// BlockAllocator.
    using View = TView<false>;

    /// ConstView provides begin() and end() methods for looping over the objects owned by the
    /// BlockAllocator.
    using ConstView = TView<true>;

    /// Constructor
    BlockAllocator() = default;

    /// Move constructor
    /// @param rhs the BlockAllocator to move
    BlockAllocator(BlockAllocator&& rhs) { std::swap(data, rhs.data); }

    /// Move assignment operator
    /// @param rhs the BlockAllocator to move
    /// @return this BlockAllocator
    BlockAllocator& operator=(BlockAllocator&& rhs) {
        if (this != &rhs) {
            Reset();
            std::swap(data, rhs.data);
        }
        return *this;
    }

    /// Destructor
    ~BlockAllocator() { Reset(); }

    /// @return a View of all objects owned by this BlockAllocator
    View Objects() { return View(this); }

    /// @return a ConstView of all objects owned by this BlockAllocator
    ConstView Objects() const { return ConstView(this); }

    /// Creates a new `TYPE` owned by the BlockAllocator.
    /// When the BlockAllocator is destructed the object will be destructed and freed.
    /// @param args the arguments to pass to the constructor
    /// @returns the pointer to the constructed object
    template <typename TYPE = T, typename... ARGS>
    TYPE* Create(ARGS&&... args) {
        static_assert(std::is_same<T, TYPE>::value || std::is_base_of<T, TYPE>::value,
                      "TYPE does not derive from T");
        static_assert(std::is_same<T, TYPE>::value || std::has_virtual_destructor<T>::value,
                      "TYPE requires a virtual destructor when calling Create() for a type "
                      "that is not T");

        auto* ptr = Allocate<TYPE>();
        new (ptr) TYPE(std::forward<ARGS>(args)...);
        AddObjectPointer(ptr);
        data.count++;

        return ptr;
    }

    /// Frees all allocations from the allocator.
    void Reset() {
        for (auto ptr : Objects()) {
            ptr->~T();
        }
        auto* block = data.block.root;
        while (block != nullptr) {
            auto* next = block->next;
            delete block;
            block = next;
        }
        data = {};
    }

    /// @returns the total number of allocated objects.
    size_t Count() const { return data.count; }

  private:
    BlockAllocator(const BlockAllocator&) = delete;
    BlockAllocator& operator=(const BlockAllocator&) = delete;

    /// Allocates an instance of TYPE from the current block, or from a newly allocated block if the
    /// current block is full.
    template <typename TYPE>
    TYPE* Allocate() {
        static_assert(sizeof(TYPE) <= BLOCK_SIZE,
                      "Cannot construct TYPE with size greater than BLOCK_SIZE");
        static_assert(alignof(TYPE) <= BLOCK_ALIGNMENT, "alignof(TYPE) is greater than ALIGNMENT");

        auto& block = data.block;

        block.current_offset = RoundUp(alignof(TYPE), block.current_offset);
        if (block.current_offset + sizeof(TYPE) > BLOCK_SIZE) {
            // Allocate a new block from the heap
            auto* prev_block = block.current;
            block.current = new Block;
            if (!block.current) {
                return nullptr;  // out of memory
            }
            block.current->next = nullptr;
            block.current_offset = 0;
            if (prev_block) {
                prev_block->next = block.current;
            } else {
                block.root = block.current;
            }
        }

        auto* base = &block.current->data[0];
        auto* ptr = Bitcast<TYPE*>(base + block.current_offset);
        block.current_offset += sizeof(TYPE);
        return ptr;
    }

    /// Adds `ptr` to the linked list of objects owned by this BlockAllocator.
    /// Once added, `ptr` will be tracked for destruction when the BlockAllocator is destructed.
    void AddObjectPointer(T* ptr) {
        auto& pointers = data.pointers;

        if (!pointers.current || pointers.current->count == Pointers::kMax) {
            auto* prev_pointers = pointers.current;
            pointers.current = Allocate<Pointers>();
            if (!pointers.current) {
                return;  // out of memory
            }
            pointers.current->next = nullptr;
            pointers.current->prev = prev_pointers;
            pointers.current->count = 0;

            if (prev_pointers) {
                prev_pointers->next = pointers.current;
            } else {
                pointers.root = pointers.current;
            }
        }

        pointers.current->ptrs[pointers.current->count++] = ptr;
    }

    struct {
        struct {
            /// The root block of the block linked list
            Block* root = nullptr;
            /// The current (end) block of the blocked linked list.
            /// New allocations come from this block
            Block* current = nullptr;
            /// The byte offset in #current for the next allocation.
            /// Initialized with BLOCK_SIZE so that the first allocation triggers a block
            /// allocation.
            size_t current_offset = BLOCK_SIZE;
        } block;

        struct {
            /// The root Pointers structure of the pointers linked list
            Pointers* root = nullptr;
            /// The current (end) Pointers structure of the pointers linked list.
            /// AddObjectPointer() adds to this structure.
            Pointers* current = nullptr;
        } pointers;

        size_t count = 0;
    } data;
};

}  // namespace langsvr

#endif  // SRC_LANGSVR_UTILS_BLOCK_ALLOCATOR_H_
