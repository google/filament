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

#ifndef SRC_TINT_UTILS_CONTAINERS_HASHMAP_BASE_H_
#define SRC_TINT_UTILS_CONTAINERS_HASHMAP_BASE_H_

#include <algorithm>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/memory/aligned_storage.h"
#include "src/tint/utils/rtti/traits.h"

// This file implements a custom STL style container & iterator in a performant manner, using
// C-style data access. It is not unexpected that -Wunsafe-buffer-usage triggers in this code, since
// the type of dynamic access being used cannot be guaranteed to be safe via static analysis.
// Attempting to change this code in simple ways to quiet these errors either a) negatively affects
// the performance by introducing unneeded copes, or b) uses typing shenanigans to work around the
// warning that other linters/analyses are unhappy with.
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint {

/// HashmapKey wraps the comparator type for a Hashmap and Hashset.
/// HashmapKey acts like a read-only `T`, but can be reassigned so long as the value is equivalent.
/// @tparam T the key comparator type.
/// @tparam HASH the hash function for the key type.
/// @tparam EQUAL the equality function for the key type.
template <typename T, typename HASH = Hasher<T>, typename EQUAL = std::equal_to<T>>
class HashmapKey {
    T value_;

  public:
    /// Key is an alias to this templated class.
    using Key = HashmapKey<T, HASH, EQUAL>;
    /// Hash is an alias to the hash function for the key type.
    using Hash = HASH;
    /// Equal is an alias to the equality function for the key type.
    using Equal = EQUAL;

    /// KeyOf() returns @p key, so a HashmapKey can be used as the entry type for a Hashset.
    /// @param key the HashmapKey
    /// @return @p key
    static const Key& KeyOf(const Key& key) { return key; }

    /// Constructor using copied value.
    /// @param value the key value.
    HashmapKey(const T& value) : value_(value), hash(HASH{}(value_)) {}  // NOLINT

    /// Constructor using moved value.
    /// @param value the key value.
    HashmapKey(T&& value) : value_(std::forward<T>(value)), hash(HASH{}(value_)) {}  // NOLINT

    /// Constructor using pre-computed hash and copied value.
    /// @param hash_ the precomputed hash of @p value
    /// @param value the key value
    HashmapKey(HashCode hash_, const T& value) : value_(value), hash(hash_) {}

    /// Constructor using pre-computed hash and moved value.
    /// @param hash_ the precomputed hash of @p value
    /// @param value the key value
    HashmapKey(HashCode hash_, T&& value) : value_(std::forward<T>(value)), hash(hash_) {}

    /// Copy constructor
    HashmapKey(const HashmapKey&) = default;

    /// Move constructor
    HashmapKey(HashmapKey&&) = default;

    /// Destructor
    ~HashmapKey() = default;

    /// Copy-assignment operator.
    /// @note As a hashmap uses the HashmapKey for indexing, the new value *must* have the same hash
    /// value and be equal to this key.
    /// @param other the key to copy to this key.
    /// @return this HashmapKey.
    HashmapKey& operator=(const HashmapKey& other) {
        TINT_ASSERT(*this == other);
        value_ = other.Value();
        return *this;
    }

    /// Move-assignment operator.
    /// @note As a hashmap uses the HashmapKey for indexing, the new value *must* have the same hash
    /// value and be equal to this key.
    /// @param other the key to move to this key.
    /// @return this HashmapKey.
    HashmapKey& operator=(HashmapKey&& other) {
        TINT_ASSERT(*this == other);
        value_ = std::move(other.Value());
        return *this;
    }

    /// Equality operator
    /// @param other the other key.
    /// @return true if the hash and value of @p other are equal to this key.
    bool operator==(const HashmapKey& other) const {
        return hash == other.hash && EQUAL{}(value_, other.Value());
    }

    /// Equality operator
    /// @param other the other key.
    /// @return true if the hash of other and value of @p other are equal to this key.
    template <typename RHS>
    bool operator==(const RHS& other) const {
        return hash == HASH{}(other) && EQUAL{}(value_, other);
    }

    /// @returns the value of the key
    const T& Value() const { return value_; }

    /// @returns the value of the key
    operator const T&() const { return value_; }

    /// @returns the pointer to the value, or the value itself if T is a pointer.
    auto operator->() const {
        if constexpr (std::is_pointer_v<T>) {
            // operator-> is useless if the T is a pointer, so automatically unwrap a pointer.
            return value_;
        } else {
            return &value_;
        }
    }

    /// The hash of value
    const HashCode hash;
};

/// Writes the HashmapKey to the stream.
/// @param out the stream to write to
/// @param key the HashmapKey to write
/// @returns out so calls can be chained
template <typename STREAM, typename T>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, const HashmapKey<T>& key) {
    if constexpr (traits::HasOperatorShiftLeft<STREAM, T>) {
        return out << key.Value();
    } else {
        return out << "<hashmap-key>";
    }
}

/// HashmapBase is the base class for Hashmap and Hashset.
/// @tparam ENTRY is the single record in the map. The entry type must alias 'Key' to the HashmapKey
/// type, and implement the method `static HashmapKey<...> KeyOf(ENTRY)` to return the key for the
/// entry.
template <typename ENTRY, size_t N>
class HashmapBase {
  protected:
    struct Node;
    struct Slot;

  public:
    /// Entry is the type of a single record in the hashmap.
    using Entry = ENTRY;
    /// Key is the HashmapKey type used to find entries.
    using Key = typename Entry::Key;
    /// Hash is the
    using Hash = typename Key::Hash;
    /// Equal is the
    using Equal = typename Key::Equal;

    /// The minimum capacity of the map.
    static constexpr size_t kMinCapacity = std::max<size_t>(N, 8);

    /// The target number of slots, expressed as a fractional percentage of the map capacity.
    /// e.g. a kLoadFactor of 75, would mean a target slots count of (0.75 * capacity).
    static constexpr size_t kLoadFactor = 75;

    /// @param capacity the capacity of the map, as total number of entries.
    /// @returns the target slot vector size to hold @p capacity map entries.
    static constexpr size_t NumSlots(size_t capacity) {
        return (std::max<size_t>(capacity, kMinCapacity) * kLoadFactor) / 100;
    }

    /// Constructor.
    /// Constructs an empty map.
    HashmapBase() {
        slots_.Resize(slots_.Capacity());
        for (auto& node : fixed_) {
            free_.Add(&node);
        }
    }

    /// Copy constructor.
    /// Constructs a map with a copy of @p other.
    /// @param other the map to copy.
    HashmapBase(const HashmapBase& other) : HashmapBase() {
        if (&other != this) {
            Copy(other);
        }
    }

    /// Move constructor.
    /// Constructs a map with the moved entries of @p other.
    /// @param other the map to move.
    HashmapBase(HashmapBase&& other) : HashmapBase() {
        if (&other != this) {
            Move(std::move(other));
        }
    }

    /// Destructor.
    ~HashmapBase() {
        // Call the destructor on all entries in the map.
        for (size_t slot_idx = 0; slot_idx < slots_.Length(); slot_idx++) {
            auto* node = slots_[slot_idx].nodes;
            while (node) {
                auto next = node->next;
                node->Destroy();
                node = next;
            }
        }
    }

    /// Assignment operator.
    /// Clears this map, and populates this map with a copy of @p other.
    /// @param other the map to copy.
    /// @returns this HashmapBase
    HashmapBase& operator=(const HashmapBase& other) {
        if (&other != this) {
            Clear();
            Copy(other);
        }
        return *this;
    }

    /// Move-assignment operator.
    /// Clears this map, and populates this map with the moved entries of @p other.
    /// @param other the map to move.
    /// @returns this HashmapBase
    HashmapBase& operator=(HashmapBase&& other) {
        if (&other != this) {
            Clear();
            Move(std::move(other));
        }
        return *this;
    }

    /// @returns the number of entries in the map.
    size_t Count() const { return count_; }

    /// @returns true if the map holds no entries.
    bool IsEmpty() const { return count_ == 0; }

    /// Removes all the entries from the map.
    /// @note the map's capacity is not reduced, as it is assumed that a reused map will likely fill
    /// to a similar size as before.
    void Clear() {
        for (size_t slot_idx = 0; slot_idx < slots_.Length(); slot_idx++) {
            auto* node = slots_[slot_idx].nodes;
            while (node) {
                auto next = node->next;
                node->Destroy();
                free_.Add(node);
                node = next;
            }
            slots_[slot_idx].nodes = nullptr;
        }
        count_ = 0;
    }

    /// Ensures that the map can hold @p n entries without heap reallocation or rehashing.
    /// @param n the number of entries to ensure can fit in the map without reallocation or
    /// rehashing.
    void Reserve(size_t n) {
        if (n > capacity_) {
            size_t count = n - capacity_;
            free_.Allocate(count);
            capacity_ += count;
        }
    }

    /// Looks up an entry with the given key.
    /// @param key the entry's key to search for.
    /// @returns a pointer to the matching entry, or null if no entry was found.
    /// @note The returned pointer is guaranteed to be valid until the owning entry is removed,
    /// the map is cleared, or the map is destructed.
    template <typename K>
    Entry* GetEntry(K&& key) {
        HashCode hash = Hash{}(key);
        auto& slot = slots_[hash % slots_.Length()];
        return slot.Find(hash, key);
    }

    /// Looks up an entry with the given key.
    /// @param key the entry's key to search for.
    /// @returns a pointer to the matching entry, or null if no entry was found.
    /// @note The returned pointer is guaranteed to be valid until the owning entry is removed,
    /// the map is cleared, or the map is destructed.
    template <typename K>
    const Entry* GetEntry(K&& key) const {
        HashCode hash = Hash{}(key);
        auto& slot = slots_[hash % slots_.Length()];
        return slot.Find(hash, key);
    }

    /// @returns true if the map contains an entry with a key that matches @p key.
    /// @param key the key to look for.
    template <typename K = Key>
    bool Contains(K&& key) const {
        return GetEntry(key) != nullptr;
    }

    /// Removes an entry from the map that has a key which matches @p key.
    /// @returns true if the entry was found and removed, otherwise false.
    /// @param key the key to look for.
    template <typename K = Key>
    bool Remove(K&& key) {
        HashCode hash = Hash{}(key);
        auto& slot = slots_[hash % slots_.Length()];
        Node** edge = &slot.nodes;
        for (auto* node = *edge; node; node = node->next) {
            if (node->Equals(hash, key)) {
                *edge = node->next;
                node->Destroy();
                free_.Add(node);
                count_--;
                return true;
            }
            edge = &node->next;
        }
        return false;
    }

    /// Iterator for entries in the map.
    template <bool IS_CONST>
    class IteratorT {
      private:
        using MAP = std::conditional_t<IS_CONST, const HashmapBase, HashmapBase>;
        using NODE = std::conditional_t<IS_CONST, const Node, Node>;

      public:
        /// @returns the entry pointed to by this iterator
        auto& operator->() { return node_->Entry(); }

        /// @returns a reference to the entry at the iterator
        auto& operator*() { return node_->Entry(); }

        /// Increments the iterator
        /// @returns this iterator
        IteratorT& operator++() {
            node_ = node_->next;
            SkipEmptySlots();
            return *this;
        }

        /// Equality operator
        /// @param other the other iterator to compare this iterator to
        /// @returns true if this iterator is equal to other
        bool operator==(const IteratorT& other) const { return node_ == other.node_; }

        /// Inequality operator
        /// @param other the other iterator to compare this iterator to
        /// @returns true if this iterator is not equal to other
        bool operator!=(const IteratorT& other) const { return node_ != other.node_; }

      private:
        /// Friend class
        friend class HashmapBase;

        IteratorT(MAP& map, size_t slot, NODE* node) : map_(map), slot_(slot), node_(node) {
            SkipEmptySlots();
        }

        void SkipEmptySlots() {
            while (!node_ && slot_ + 1 < map_.slots_.Length()) {
                node_ = map_.slots_[++slot_].nodes;
            }
        }

        MAP& map_;
        size_t slot_ = 0;
        NODE* node_ = nullptr;
    };

    /// An immutable key and mutable value iterator
    using Iterator = IteratorT</*IS_CONST*/ false>;

    /// An immutable key and value iterator
    using ConstIterator = IteratorT</*IS_CONST*/ true>;

    /// @returns an immutable iterator to the start of the map.
    ConstIterator begin() const { return ConstIterator{*this, 0, slots_.Front().nodes}; }

    /// @returns an immutable iterator to the end of the map.
    ConstIterator end() const { return ConstIterator{*this, slots_.Length(), nullptr}; }

    /// @returns an iterator to the start of the map.
    Iterator begin() { return Iterator{*this, 0, slots_.Front().nodes}; }

    /// @returns an iterator to the end of the map.
    Iterator end() { return Iterator{*this, slots_.Length(), nullptr}; }

    /// STL-friendly alias to Entry. Used by gmock.
    using value_type = const Entry&;

  protected:
    /// Node holds an Entry in a linked list.
    struct Node {
        /// Destructs the entry.
        void Destroy() { Entry().~ENTRY(); }

        /// @returns the storage reinterpreted as an `Entry&`
        ENTRY& Entry() { return storage.Get(); }

        /// @returns the storage reinterpreted as a `const Entry&`
        const ENTRY& Entry() const { return storage.Get(); }

        /// @returns a reference to the Entry's HashmapKey
        const HashmapBase::Key& Key() const { return HashmapBase::Entry::KeyOf(Entry()); }

        /// @param hash the hash value to compare against the Entry's key hash value
        /// @param value the value to compare against the Entry's key
        /// @returns true if the Entry's hash is equal to @p hash, and the Entry's key is equal to
        /// @p value.
        template <typename T>
        bool Equals(HashCode hash, T&& value) const {
            auto& key = Key();
            return key.hash == hash && HashmapBase::Equal{}(key.Value(), value);
        }

        /// storage is a buffer that has the same size and alignment as Entry.
        /// The storage holds a constructed Entry when linked in the slots, and is destructed when
        /// removed from slots.
        AlignedStorage<ENTRY> storage;

        /// next is the next Node in the slot, or in the free list.
        Node* next;
    };

    /// Copies the hashmap @p other into this empty hashmap.
    /// @note This hashmap must be empty before calling
    /// @param other the hashmap to copy
    void Copy(const HashmapBase& other) {
        Reserve(other.capacity_);
        slots_.Resize(other.slots_.Length());
        for (size_t slot_idx = 0; slot_idx < slots_.Length(); slot_idx++) {
            for (auto* o = other.slots_[slot_idx].nodes; o; o = o->next) {
                auto* node = free_.Take();
                new (&node->Entry()) Entry{o->Entry()};
                slots_[slot_idx].Add(node);
            }
        }
        count_ = other.count_;
    }

    /// Moves the hashmap @p other into this empty hashmap.
    /// @note This hashmap must be empty before calling
    /// @param other the hashmap to move
    void Move(HashmapBase&& other) {
        Reserve(other.capacity_);
        slots_.Resize(other.slots_.Length());
        for (size_t slot_idx = 0; slot_idx < slots_.Length(); slot_idx++) {
            for (auto* o = other.slots_[slot_idx].nodes; o; o = o->next) {
                auto* node = free_.Take();
                new (&node->Entry()) Entry{std::move(o->Entry())};
                slots_[slot_idx].Add(node);
            }
        }
        count_ = other.count_;
        other.Clear();
    }

    /// EditIndex is the structure returned by EditAt(), used to simplify entry replacement and
    /// insertion.
    struct EditIndex {
        /// The HashmapBase that created this EditIndex
        HashmapBase& map;
        /// The slot that will hold the edit.
        Slot& slot;
        /// The hash of the key, passed to EditAt().
        HashCode hash;
        /// The resolved node entry, or nullptr if EditAt() did not resolve to an existing entry.
        Entry* entry = nullptr;

        /// Replace will replace the entry with a new Entry built from @p key and @p values.
        /// @note #entry must not be null before calling.
        /// @note the new key must have equality to the old key.
        /// @param key the key value (inner value of a HashmapKey).
        /// @param values optional additional values to pass to the Entry constructor.
        template <typename K, typename... V>
        void Replace(K&& key, V&&... values) {
            *entry = Entry{Key{hash, std::forward<K>(key)}, std::forward<V>(values)...};
        }

        /// Insert will create a new entry using @p key and @p values and insert it into the slot.
        /// The created entry will be assigned to #entry before returning.
        /// @note #entry must be null before calling.
        /// @note the key must not already exist in the map.
        /// @param key the key value (inner value of a HashmapKey).
        /// @param values optional additional values to pass to the Entry constructor.
        template <typename K, typename... V>
        void Insert(K&& key, V&&... values) {
            auto* node = map.free_.Take();
            slot.Add(node);
            map.count_++;
            entry = &node->Entry();
            new (entry) Entry{Key{hash, std::forward<K>(key)}, std::forward<V>(values)...};
        }
    };

    /// EditAt is a helper for map entry replacement and entry insertion.
    /// Before indexing, EditAt will ensure there's at least one free node available, potentially
    /// allocating and rehashing if there's no free nodes available.
    /// @param key the key used to compute the hash, look up the slot and search for the existing
    /// node.
    /// @returns a EditIndex used to modify or insert a new entry into the map with the given key.
    template <typename K>
    EditIndex EditAt(K&& key) {
        if (!free_.nodes_) {
            free_.Allocate(capacity_);
            capacity_ += capacity_;
            Rehash();
        }
        HashCode hash = Hash{}(key);
        auto& slot = slots_[hash % slots_.Length()];
        auto* entry = slot.Find(hash, key);
        return {*this, slot, hash, entry};
    }

    /// Rehash resizes the slots vector proportionally to the map capacity, and then reinserts the
    /// nodes so they're linked in the correct slots linked lists.
    void Rehash() {
        size_t num_slots = NumSlots(capacity_);
        decltype(slots_) old_slots;
        std::swap(slots_, old_slots);
        slots_.Resize(num_slots);
        for (size_t old_slot_idx = 0; old_slot_idx < old_slots.Length(); old_slot_idx++) {
            auto* node = old_slots[old_slot_idx].nodes;
            while (node) {
                auto next = node->next;
                size_t new_slot_idx = node->Key().hash % num_slots;
                slots_[new_slot_idx].Add(node);
                node = next;
            }
        }
    }

    /// Slot holds a linked list of nodes. Nodes are assigned to the slot list by calculating the
    /// modulo of the entry's hash with the slot_ vector length.
    struct Slot {
        /// The linked list of nodes in this slot.
        Node* nodes = nullptr;

        /// Add adds the node @p node to this slot.
        /// @note The node must be unlinked from any existing list before calling.
        /// @param node the node to add.
        void Add(Node* node) {
            node->next = nodes;
            nodes = node;
        }

        /// @returns the node in the slot with the given hash and key.
        /// @param hash the key hash to search for.
        /// @param key the key value to search for.
        template <typename K>
        const Entry* Find(HashCode hash, K&& key) const {
            for (auto* node = nodes; node; node = node->next) {
                if (node->Equals(hash, key)) {
                    return &node->Entry();
                }
            }
            return nullptr;
        }

        /// @returns the node in the slot with the given hash and key.
        /// @param hash the key hash to search for.
        /// @param key the key value to search for.
        template <typename K>
        Entry* Find(HashCode hash, K&& key) {
            for (auto* node = nodes; node; node = node->next) {
                if (node->Equals(hash, key)) {
                    return &node->Entry();
                }
            }
            return nullptr;
        }
    };

    /// Free holds a linked list of nodes which are currently not used by entries in the map, and a
    /// linked list of node allocations.
    struct FreeNodes {
        /// Allocation is the header of a block of memory that holds Nodes.
        struct Allocation {
            /// The linked list of allocations.
            Allocation* next = nullptr;
            // Node[] array follows this structure.
        };

        /// The linked list of free nodes.
        Node* nodes_ = nullptr;

        /// The linked list of allocations.
        Allocation* allocations_ = nullptr;

        /// Destructor.
        /// Frees all the allocations made.
        ~FreeNodes() {
            auto* allocation = allocations_;
            while (allocation) {
                auto* next = allocation->next;
                free(allocation);
                allocation = next;
            }
        }

        /// @returns the next free node in the list
        Node* Take() {
            auto* node = nodes_;
            nodes_ = node->next;
            node->next = nullptr;
            return node;
        }

        /// Add adds the node @p node to the list of free nodes.
        /// @note The node must be unlinked from any existing list before calling.
        /// @param node the node to add.
        void Add(Node* node) {
            node->next = nodes_;
            nodes_ = node;
        }

        /// Allocate allocates an additional @p count nodes and adds them to the free node list.
        /// @param count the number of new nodes to allocate.
        /// @note callers must remember to increment HashmapBase::capacity_ by the same amount.
        void Allocate(size_t count) {
            static_assert(std::is_trivial_v<Node>,
                          "Node is not trivial, and will require construction / destruction");
            constexpr size_t kAllocationSize = RoundUp(alignof(Node), sizeof(Allocation));
            auto* memory =
                reinterpret_cast<std::byte*>(malloc(kAllocationSize + sizeof(Node) * count));
            if (DAWN_UNLIKELY(!memory)) {
                TINT_ICE() << "out of memory";
                return;
            }
            auto* nodes_allocation = Bitcast<Allocation*>(memory);
            nodes_allocation->next = allocations_;
            allocations_ = nodes_allocation;

            auto* nodes = Bitcast<Node*>(memory + kAllocationSize);
            for (size_t i = 0; i < count; i++) {
                Add(&nodes[i]);
            }
        }
    };

    /// The fixed-size array of nodes, used for the first kMinCapacity entries of the map, before
    /// allocating from the heap.
    std::array<Node, kMinCapacity> fixed_;
    /// The vector of slots. Each slot holds a linked list of nodes which hold entries in the map.
    Vector<Slot, NumSlots(N)> slots_;
    /// The linked list of free nodes, and node allocations from the heap.
    FreeNodes free_;
    /// The total number of nodes, including free nodes (kMinCapacity + heap-allocated)
    size_t capacity_ = kMinCapacity;
    /// The total number of nodes that currently hold map entries.
    size_t count_ = 0;
};

}  // namespace tint

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

#endif  // SRC_TINT_UTILS_CONTAINERS_HASHMAP_BASE_H_
