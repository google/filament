/* Copyright (c) 2015-2017, 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2024 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2024 LunarG, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace vku {
namespace small {

// A vector class with "small string optimization" -- meaning that the class contains a fixed working store for N elements.
// Useful in in situations where the needed size is unknown, but the typical size is known  If size increases beyond the
// fixed capacity, a dynamically allocated working store is created.
//
// NOTE: Unlike std::vector which only requires T to be CopyAssignable and CopyConstructable, small::vector requires T to be
//       MoveAssignable and MoveConstructable
// NOTE: Unlike std::vector, iterators are invalidated by move assignment between small::vector objects effectively the
//       "small string" allocation functions as an incompatible allocator.
template <typename T, size_t N, typename SizeType = uint32_t>
class vector {
  public:
    using value_type = T;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = SizeType;
    static const size_type kSmallCapacity = N;
    static const size_type kMaxCapacity = std::numeric_limits<size_type>::max();
    static_assert(N <= kMaxCapacity, "size must be less than size_type::max");

    vector() : size_(0), capacity_(N), working_store_(GetSmallStore()) {}

    vector(std::initializer_list<T> list) : size_(0), capacity_(N), working_store_(GetSmallStore()) { PushBackFrom(list); }

    vector(const vector &other) : size_(0), capacity_(N), working_store_(GetSmallStore()) { PushBackFrom(other); }

    vector(vector &&other) : size_(0), capacity_(N), working_store_(GetSmallStore()) {
        if (other.large_store_) {
            MoveLargeStore(other);
        } else {
            PushBackFrom(std::move(other));
        }
        // Per the spec, when constructing from other, other is guaranteed to be empty after the constructor runs
        other.clear();
    }

    vector(size_type size, const value_type &value = value_type()) : size_(0), capacity_(N), working_store_(GetSmallStore()) {
        reserve(size);
        auto dest = GetWorkingStore();
        for (size_type i = 0; i < size; i++) {
            new (dest) value_type(value);
            ++dest;
        }
        size_ = size;
    }

    ~vector() { clear(); }

    bool operator==(const vector &rhs) const {
        if (size_ != rhs.size_) return false;
        auto value = begin();
        for (const auto &rh_value : rhs) {
            if (!(*value == rh_value)) {
                return false;
            }
            ++value;
        }
        return true;
    }

    bool operator!=(const vector &rhs) const { return !(*this == rhs); }

    vector &operator=(const vector &other) {
        if (this != &other) {
            if (other.size_ > capacity_) {
                // Calling reserve would move construct and destroy all current contents, so just clear them before calling
                // PushBackFrom (which does a reserve vs. the now empty this)
                clear();
                PushBackFrom(other);
            } else {
                // The copy will fit into the current allocation
                auto dest = GetWorkingStore();
                auto source = other.GetWorkingStore();

                const auto overlap = std::min(size_, other.size_);
                // Copy assign anywhere we have objects in this
                // Note: usually cheaper than destruct/construct
                for (size_type i = 0; i < overlap; i++) {
                    dest[i] = source[i];
                }

                // Copy construct anywhere we *don't* have objects in this
                for (size_type i = overlap; i < other.size_; i++) {
                    new (dest + i) value_type(source[i]);
                }

                // Any entries in this past other_size_ must be cleaned up...
                for (size_type i = other.size_; i < size_; i++) {
                    dest[i].~value_type();
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    vector &operator=(vector &&other) {
        if (this != &other) {
            // Note: move assign doesn't require other to become empty (as does move construction)
            //       so we'll leave other alone except in the large store case, while moving the object
            //       *in* the vector from other
            if (other.large_store_) {
                // Moving the other large store intact is probably best, even if we have to destroy everything in this.
                clear();
                MoveLargeStore(other);
            } else if (other.size_ > capacity_) {
                // If we'd have to reallocate, just clean up minimally and copy normally
                clear();
                PushBackFrom(std::move(other));
            } else {
                // The copy will fit into the current allocation
                auto dest = GetWorkingStore();
                auto source = other.GetWorkingStore();

                const auto overlap = std::min(size_, other.size_);

                // Move assign where we have objects in this
                // Note: usually cheaper than destruct/construct
                for (size_type i = 0; i < overlap; i++) {
                    dest[i] = std::move(source[i]);
                }

                // Move construct where we *don't* have objects in this
                for (size_type i = overlap; i < other.size_; i++) {
                    new (dest + i) value_type(std::move(source[i]));
                }

                // Any entries in this past other_size_ must be cleaned up...
                for (size_type i = other.size_; i < size_; i++) {
                    dest[i].~value_type();
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    reference operator[](size_type pos) {
        assert(pos < size_);
        return GetWorkingStore()[pos];
    }
    const_reference operator[](size_type pos) const {
        assert(pos < size_);
        return GetWorkingStore()[pos];
    }

    // Like std::vector:: calling front or back on an empty container causes undefined behavior
    reference front() {
        assert(size_ > 0);
        return GetWorkingStore()[0];
    }
    const_reference front() const {
        assert(size_ > 0);
        return GetWorkingStore()[0];
    }
    reference back() {
        assert(size_ > 0);
        return GetWorkingStore()[size_ - 1];
    }
    const_reference back() const {
        assert(size_ > 0);
        return GetWorkingStore()[size_ - 1];
    }

    bool empty() const { return size_ == 0; }

    template <class... Args>
    void emplace_back(Args &&...args) {
        assert(size_ < kMaxCapacity);
        reserve(size_ + 1);
        new (GetWorkingStore() + size_) value_type(args...);
        size_++;
    }

    // Note: probably should update this to reflect C++23 ranges
    template <typename Container>
    void PushBackFrom(const Container &from) {
        assert(from.size() <= kMaxCapacity);
        assert(size_ <= kMaxCapacity - from.size());
        const size_type new_size = size_ + static_cast<size_type>(from.size());
        reserve(new_size);

        auto dest = GetWorkingStore() + size_;
        for (const auto &element : from) {
            new (dest) value_type(element);
            ++dest;
        }
        size_ = new_size;
    }

    template <typename Container>
    void PushBackFrom(Container &&from) {
        assert(from.size() < kMaxCapacity);
        const size_type new_size = size_ + static_cast<size_type>(from.size());
        reserve(new_size);

        auto dest = GetWorkingStore() + size_;
        for (auto &element : from) {
            new (dest) value_type(std::move(element));
            ++dest;
        }
        size_ = new_size;
    }

    void reserve(size_type new_cap) {
        // Since this can't shrink, if we're growing we're newing
        if (new_cap > capacity_) {
            assert(capacity_ >= kSmallCapacity);
            auto new_store = std::unique_ptr<BackingStore[]>(new BackingStore[new_cap]);
            auto working_store = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                new (new_store[i].data) value_type(std::move(working_store[i]));
                working_store[i].~value_type();
            }
            large_store_ = std::move(new_store);
            assert(new_cap > kSmallCapacity);
            capacity_ = new_cap;
        }
        UpdateWorkingStore();
        // No shrink here.
    }

    void clear() {
        // Keep clear minimal to optimize reset functions for enduring objects
        // more work is deferred until destruction (freeing of large_store for example)
        // and we intentionally *aren't* shrinking.  Callers that desire shrink semantics
        // can call shrink_to_fit.
        auto working_store = GetWorkingStore();
        for (size_type i = 0; i < size_; i++) {
            working_store[i].~value_type();
        }
        size_ = 0;
    }

    void resize(size_type count) {
        struct ValueInitTag {  // tag to request value-initialization
            explicit ValueInitTag() = default;
        };
        Resize(count, ValueInitTag{});
    }

    void resize(size_type count, const value_type &value) { Resize(count, value); }

    void shrink_to_fit() {
        if (size_ == 0) {
            // shrink resets to small when empty
            capacity_ = kSmallCapacity;
            large_store_.reset();
            UpdateWorkingStore();
        } else if ((capacity_ > kSmallCapacity) && (capacity_ > size_)) {
            auto source = GetWorkingStore();
            // Keep the source from disappearing until the end of the function
            auto old_store = std::unique_ptr<BackingStore[]>(std::move(large_store_));
            assert(!large_store_);
            if (size_ < kSmallCapacity) {
                capacity_ = kSmallCapacity;
            } else {
                large_store_ = std::unique_ptr<BackingStore[]>(new BackingStore[size_]);
                capacity_ = size_;
            }
            UpdateWorkingStore();
            auto dest = GetWorkingStore();
            for (size_type i = 0; i < size_; i++) {
                dest[i] = std::move(source[i]);
                source[i].~value_type();
            }
        }
    }

    inline iterator begin() { return GetWorkingStore(); }
    inline const_iterator cbegin() const { return GetWorkingStore(); }
    inline const_iterator begin() const { return GetWorkingStore(); }

    inline iterator end() { return GetWorkingStore() + size_; }
    inline const_iterator cend() const { return GetWorkingStore() + size_; }
    inline const_iterator end() const { return GetWorkingStore() + size_; }
    inline size_type size() const { return size_; }
    auto capacity() const { return capacity_; }

    inline pointer data() { return GetWorkingStore(); }
    inline const_pointer data() const { return GetWorkingStore(); }

  protected:
    inline const_pointer ComputeWorkingStore() const {
        assert(large_store_ || (capacity_ == kSmallCapacity));

        const BackingStore *store = large_store_ ? large_store_.get() : small_store_;
        return &store->object;
    }
    inline pointer ComputeWorkingStore() {
        assert(large_store_ || (capacity_ == kSmallCapacity));

        BackingStore *store = large_store_ ? large_store_.get() : small_store_;
        return &store->object;
    }

    void UpdateWorkingStore() { working_store_ = ComputeWorkingStore(); }

    inline const_pointer GetWorkingStore() const {
        DbgWorkingStoreCheck();
        return working_store_;
    }
    inline pointer GetWorkingStore() {
        DbgWorkingStoreCheck();
        return working_store_;
    }

    inline pointer GetSmallStore() { return &small_store_->object; }

    union BackingStore {
        BackingStore() {}
        ~BackingStore() {}

        uint8_t data[sizeof(value_type)];
        value_type object;
    };
    size_type size_;
    size_type capacity_;
    BackingStore small_store_[N];
    std::unique_ptr<BackingStore[]> large_store_;
    value_type *working_store_;

#ifndef NDEBUG
    void DbgWorkingStoreCheck() const { assert(ComputeWorkingStore() == working_store_); }
#else
    void DbgWorkingStoreCheck() const {}
#endif

  private:
    void MoveLargeStore(vector &other) {
        assert(other.large_store_);
        assert(other.capacity_ > kSmallCapacity);
        // In move operations, from a small vector with a large store, we can move from it
        large_store_ = std::move(other.large_store_);
        capacity_ = other.capacity_;
        size_ = other.size_;
        UpdateWorkingStore();

        // We've stolen other's large store, must leave it in a valid state
        other.size_ = 0;
        other.capacity_ = kSmallCapacity;
        other.UpdateWorkingStore();
    }

    template <typename T2>
    void Resize(size_type new_size, const T2 &value) {
        if (new_size < size_) {
            auto working_store = GetWorkingStore();
            for (size_type i = new_size; i < size_; i++) {
                working_store[i].~value_type();
            }
            size_ = new_size;
        } else if (new_size > size_) {
            reserve(new_size);
            // if T2 != T and T is not DefaultInsertable, new values will be undefined
            if constexpr (std::is_same_v<T2, T> || std::is_default_constructible_v<T>) {
                for (size_type i = size_; i < new_size; ++i) {
                    if constexpr (std::is_same_v<T2, T>) {
                        emplace_back(value_type(value));
                    } else if constexpr (std::is_default_constructible_v<T>) {
                        emplace_back(value_type());
                    }
                }
                assert(size() == new_size);
            } else {
                size_ = new_size;
            }
        }
    }
};

// This is a wrapper around unordered_map that optimizes for the common case
// of only containing a small number of elements. The first N elements are stored
// inline in the object and don't require hashing or memory (de)allocation.

template <typename Key, typename value_type, typename inner_container_type, typename value_type_helper, int N>
class container_base {
  protected:
    bool small_data_allocated[N];
    value_type small_data[N];

    inner_container_type inner_cont;

    value_type_helper helper;

  public:
    container_base() {
        for (int i = 0; i < N; ++i) {
            small_data_allocated[i] = false;
        }
    }

    class iterator {
        typedef typename inner_container_type::iterator inner_iterator;
        friend class container_base<Key, value_type, inner_container_type, value_type_helper, N>;

        container_base<Key, value_type, inner_container_type, value_type_helper, N> *parent;
        int index;
        inner_iterator it;

      public:
        iterator() {}

        iterator operator++() {
            if (index < N) {
                index++;
                while (index < N && !parent->small_data_allocated[index]) {
                    index++;
                }
                if (index < N) {
                    return *this;
                }
                it = parent->inner_cont.begin();
                return *this;
            }
            ++it;
            return *this;
        }

        bool operator==(const iterator &other) const {
            if ((index < N) != (other.index < N)) {
                return false;
            }
            if (index < N) {
                return (index == other.index);
            }
            return it == other.it;
        }

        bool operator!=(const iterator &other) const { return !(*this == other); }

        value_type &operator*() const {
            if (index < N) {
                return parent->small_data[index];
            }
            return *it;
        }
        value_type *operator->() const {
            if (index < N) {
                return &parent->small_data[index];
            }
            return &*it;
        }
    };

    class const_iterator {
        typedef typename inner_container_type::const_iterator inner_iterator;
        friend class container_base<Key, value_type, inner_container_type, value_type_helper, N>;

        const container_base<Key, value_type, inner_container_type, value_type_helper, N> *parent;
        int index;
        inner_iterator it;

      public:
        const_iterator() {}

        const_iterator operator++() {
            if (index < N) {
                index++;
                while (index < N && !parent->small_data_allocated[index]) {
                    index++;
                }
                if (index < N) {
                    return *this;
                }
                it = parent->inner_cont.begin();
                return *this;
            }
            ++it;
            return *this;
        }

        bool operator==(const const_iterator &other) const {
            if ((index < N) != (other.index < N)) {
                return false;
            }
            if (index < N) {
                return (index == other.index);
            }
            return it == other.it;
        }

        bool operator!=(const const_iterator &other) const { return !(*this == other); }

        const value_type &operator*() const {
            if (index < N) {
                return parent->small_data[index];
            }
            return *it;
        }
        const value_type *operator->() const {
            if (index < N) {
                return &parent->small_data[index];
            }
            return &*it;
        }
    };

    iterator begin() {
        iterator it;
        it.parent = this;
        // If index 0 is allocated, return it, otherwise use operator++ to find the first
        // allocated element.
        it.index = 0;
        if (small_data_allocated[0]) {
            return it;
        }
        ++it;
        return it;
    }

    iterator end() {
        iterator it;
        it.parent = this;
        it.index = N;
        it.it = inner_cont.end();
        return it;
    }

    const_iterator begin() const {
        const_iterator it;
        it.parent = this;
        // If index 0 is allocated, return it, otherwise use operator++ to find the first
        // allocated element.
        it.index = 0;
        if (small_data_allocated[0]) {
            return it;
        }
        ++it;
        return it;
    }

    const_iterator end() const {
        const_iterator it;
        it.parent = this;
        it.index = N;
        it.it = inner_cont.end();
        return it;
    }

    bool contains(const Key &key) const {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i] && helper.compare_equal(small_data[i], key)) {
                return true;
            }
        }
        // check size() first to avoid hashing key unnecessarily.
        if (inner_cont.size() == 0) {
            return false;
        }
        return inner_cont.find(key) != inner_cont.end();
    }

    typename inner_container_type::size_type count(const Key &key) const { return contains(key) ? 1 : 0; }

    std::pair<iterator, bool> insert(const value_type &value) {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i] && helper.compare_equal(small_data[i], value)) {
                iterator it;
                it.parent = this;
                it.index = i;
                return std::make_pair(it, false);
            }
        }
        // check size() first to avoid hashing key unnecessarily.
        auto iter = inner_cont.size() > 0 ? inner_cont.find(helper.get_key(value)) : inner_cont.end();
        if (iter != inner_cont.end()) {
            iterator it;
            it.parent = this;
            it.index = N;
            it.it = iter;
            return std::make_pair(it, false);
        } else {
            for (int i = 0; i < N; ++i) {
                if (!small_data_allocated[i]) {
                    small_data_allocated[i] = true;
                    helper.assign(small_data[i], value);
                    iterator it;
                    it.parent = this;
                    it.index = i;
                    return std::make_pair(it, true);
                }
            }
            iter = inner_cont.insert(value).first;
            iterator it;
            it.parent = this;
            it.index = N;
            it.it = iter;
            return std::make_pair(it, true);
        }
    }

    typename inner_container_type::size_type erase(const Key &key) {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i] && helper.compare_equal(small_data[i], key)) {
                small_data_allocated[i] = false;
                return 1;
            }
        }
        return inner_cont.erase(key);
    }

    typename inner_container_type::size_type size() const {
        auto size = inner_cont.size();
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i]) {
                size++;
            }
        }
        return size;
    }

    bool empty() const {
        for (int i = 0; i < N; ++i) {
            if (small_data_allocated[i]) {
                return false;
            }
        }
        return inner_cont.size() == 0;
    }

    void clear() {
        for (int i = 0; i < N; ++i) {
            small_data_allocated[i] = false;
        }
        inner_cont.clear();
    }
};

// Helper function objects to compare/assign/get keys in small_unordered_set/map.
// This helps to abstract away whether value_type is a Key or a pair<Key, T>.
template <typename MapType>
class value_type_helper_map {
    using PairType = typename MapType::value_type;
    using Key = typename std::remove_const<typename PairType::first_type>::type;

  public:
    bool compare_equal(const PairType &lhs, const Key &rhs) const { return lhs.first == rhs; }
    bool compare_equal(const PairType &lhs, const PairType &rhs) const { return lhs.first == rhs.first; }

    void assign(PairType &lhs, const PairType &rhs) const {
        // While the const_cast may be unsatisfactory, we are using small_data as
        // stand-in for placement new and a small-block allocator, so the const_cast
        // is minimal, contained, valid, and allows operators * and -> to avoid copies
        const_cast<Key &>(lhs.first) = rhs.first;
        lhs.second = rhs.second;
    }

    Key get_key(const PairType &value) const { return value.first; }
};

template <typename Key>
class value_type_helper_set {
  public:
    bool compare_equal(const Key &lhs, const Key &rhs) const { return lhs == rhs; }

    void assign(Key &lhs, const Key &rhs) const { lhs = rhs; }

    Key get_key(const Key &value) const { return value; }
};

template <typename Key, typename T, int N = 1, typename Map = std::unordered_map<Key, T>>
class unordered_map : public container_base<Key, typename Map::value_type, Map, value_type_helper_map<Map>, N> {
  public:
    T &operator[](const Key &key) {
        for (int i = 0; i < N; ++i) {
            if (this->small_data_allocated[i] && this->helper.compare_equal(this->small_data[i], key)) {
                return this->small_data[i].second;
            }
        }
        auto iter = this->inner_cont.find(key);
        if (iter != this->inner_cont.end()) {
            return iter->second;
        } else {
            for (int i = 0; i < N; ++i) {
                if (!this->small_data_allocated[i]) {
                    this->small_data_allocated[i] = true;
                    this->helper.assign(this->small_data[i], {key, T()});

                    return this->small_data[i].second;
                }
            }
            return this->inner_cont[key];
        }
    }
};

template <typename Key, int N = 1, typename Set = std::unordered_set<Key>>
class unordered_set : public container_base<Key, Key, Set, value_type_helper_set<Key>, N> {};

}  // namespace small
}  // namespace vku
