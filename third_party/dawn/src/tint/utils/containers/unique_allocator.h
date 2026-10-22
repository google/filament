// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_CONTAINERS_UNIQUE_ALLOCATOR_H_
#define SRC_TINT_UTILS_CONTAINERS_UNIQUE_ALLOCATOR_H_

#include <functional>
#include <utility>

#include "src/tint/utils/containers/hashmap_base.h"
#include "src/tint/utils/memory/block_allocator.h"

namespace tint {

/// UniqueAllocator is used to allocate unique instances of the template type `T`.
template <typename T, typename HASH = Hasher<T>, typename EQUAL = std::equal_to<T>>
class UniqueAllocator {
  public:
    /// Iterator is the type returned by begin() and end()
    using Iterator = typename BlockAllocator<T>::ConstIterator;

    /// @param args the arguments used to construct the object.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If an existing instance of `T` has been constructed, then the same
    ///         pointer is returned.
    template <typename TYPE = T, typename... ARGS>
    TYPE* Get(ARGS&&... args) {
        // Create a temporary T instance on the stack so that we can hash it, and use it for
        // equality lookup for the Set. If the item is not found in the set, then we create the
        // persisted instance with the allocator.
        TYPE prototype{args...};
        Key& key = items.Add(&prototype);
        if (key.Value() == &prototype) {
            key = allocator.template Create<TYPE>(std::forward<ARGS>(args)...);
        }
        return static_cast<TYPE*>(key.Value());
    }

    /// @param args the arguments used to create the temporary used for the search.
    /// @return a pointer to an instance of `T` with the provided arguments, or nullptr if the item
    ///         was not found.
    template <typename TYPE = T, typename... ARGS>
    TYPE* Find(ARGS&&... args) const {
        // Create a temporary T instance on the stack so that we can hash it, and use it for
        // equality lookup for the Set.
        TYPE prototype{std::forward<ARGS>(args)...};
        return static_cast<TYPE*>(items.Get(&prototype));
    }

    /// Wrap sets this allocator to the objects created with the content of `inner`.
    /// The allocator after Wrap is intended to temporarily extend the objects
    /// of an existing immutable UniqueAllocator.
    /// As the copied objects are owned by `inner`, `inner` must not be destructed
    /// or assigned while using this allocator.
    /// @param o the immutable UniqueAlllocator to extend
    void Wrap(const UniqueAllocator<T, HASH, EQUAL>& o) { items = o.items; }

    /// @returns an iterator to the beginning of the types
    Iterator begin() const { return allocator.Objects().begin(); }
    /// @returns an iterator to the end of the types
    Iterator end() const { return allocator.Objects().end(); }

  private:
    /// Comparator is the hashing function used by the Hashset
    struct Hasher {
        /// Hashing function
        /// @param e the entry
        /// @returns the hash of the entry
        HashCode operator()(T* e) const { return HASH{}(*e); }
    };

    /// Equality is the equality function used by the Hashset
    struct Equality {
        /// Equality function
        /// @param a the first entry to compare
        /// @param b the second entry to compare
        /// @returns true if the two entries are equal
        bool operator()(T* a, T* b) const { return EQUAL{}(*a, *b); }
    };

    /// The maximum capacity of Set before it performs heap allocations.
    static constexpr size_t kFixedSize = 32;

    /// The key type of Set.
    using Key = HashmapKey<T*, Hasher, Equality>;

    /// A custom Hashset implementation that allows keys to be modified.
    class Set : public HashmapBase<Key, kFixedSize> {
      public:
        Key& Add(T* key) {
            auto idx = this->EditAt(key);
            if (!idx.entry) {
                idx.Insert(std::forward<T*>(key));
            }
            return *idx.entry;
        }

        T* Get(T* key) const {
            if (auto* entry = this->GetEntry(key)) {
                return *entry;
            }
            return nullptr;
        }
    };

    /// The block allocator used to allocate the unique objects
    BlockAllocator<T> allocator;
    /// The set of unique item entries
    Set items;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_UNIQUE_ALLOCATOR_H_
