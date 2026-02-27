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

#ifndef TNT_UTILS_LRUCACHE_H
#define TNT_UTILS_LRUCACHE_H

#include <utils/Allocator.h>
#include <utils/compiler.h>

#include <tsl/robin_map.h>

#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

namespace utils {

/**
 * A fixed-capacity Least Recently Used (LRU) cache.
 *
 * This container allows O(1) access, insertion, and eviction. It uses an Arena with an
 * ObjectPoolAllocator to store the nodes of a doubly-linked list representing the LRU order,
 * and a robin_map for fast key lookups.
 *
 * @tparam Key The type of the keys.
 * @tparam T The type of the values.
 * @tparam Hash The hasher for the keys.
 */
template<typename Key,
         typename T,
         typename Hash = std::hash<Key>,
         typename KeyEqual = std::equal_to<Key>>
class LruCache {
    // If Key is bigger than a pointer, robin map keys are references to Node::key. Otherwise,
    // they're full copies.
    using MapKey = std::conditional_t<(sizeof(Key) > sizeof(void*)),
            std::reference_wrapper<const Key>, Key>;

public:
    using size_type = uint32_t;

    /**
     * Creates an LRU cache with the specified capacity.
     *
     * @param name Name to identify the backing Arena allocator.
     * @param capacity The maximum number of elements the cache can hold.
     */
    LruCache(const char* UTILS_NONNULL name, size_type capacity)
            : mCapacity(capacity),
              // HACK: FreeList cannot handle a capacity of 0.
              mArena(name, (capacity ? capacity : 1) * sizeof(Node)),
              mHead(nullptr),
              mTail(nullptr) {
        mMap.reserve(capacity);
    }

    ~LruCache() {
        clear();
    }

    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;
    LruCache(LruCache&&) = default;
    LruCache& operator=(LruCache&&) = default;

    /**
     * Returns a reference to the value associated with the key.
     *
     * Additionally moves the key/value pair to the front of the most recently used (MRU) list.
     *
     * @param key The key to look up.
     * @param hash The precomputed hash of the key.
     * @return Pointer to the value, if exists; otherwise, nullptr.
     */
    T* UTILS_NULLABLE get(Key const& key, size_t hash) {
        auto it = mMap.find(key, hash);
        if (UTILS_UNLIKELY(it == mMap.end())) {
            return nullptr;
        }

        Node* node = it->second;
        assert(node != nullptr);

        moveToFront(node);

        return &node->value;
    }

    inline T* UTILS_NULLABLE get(Key const& key) {
        return get(key, Hash{}(key));
    }

    /**
     * Moves the value out of the cache, if it exists.
     *
     * @param key The key to look up.
     * @param hash The precomputed hash of the key.
     */
    std::optional<T> pop(Key const& key, size_t hash) {
        auto it = mMap.find(key, hash);
        if (UTILS_UNLIKELY(it == mMap.end())) {
            return std::nullopt;
        }

        Node* node = it->second;
        assert(node != nullptr);

        if (node == mHead && node == mTail) {
            mHead = nullptr;
            mTail = nullptr;
        } else if (node == mHead) {
            assert(node->next != nullptr);
            mHead = node->next;
            mHead->prev = nullptr;
        } else if (node == mTail) {
            assert(node->prev != nullptr);
            mTail = node->prev;
            mTail->next = nullptr;
        } else {
            assert(node->prev != nullptr);
            assert(node->next != nullptr);
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        T r = std::move(node->value);
        mMap.erase(node->key);
        mArena.destroy(node);
        return r;
    }

    inline std::optional<T> pop(Key const& key) {
        return pop(key, Hash{}(key));
    }

    /**
     * Inserts a new entry or updates an existing one.
     *
     * Prepends it to the front of the most recently used (MRU) list. Potentially evicts the
     * least-recently used (LRU) key/value pair by calling the releaser function.
     *
     * @tparam F Callable type accepting T&&.
     * @param key The key to insert or update.
     * @param t The value to insert or update.
     * @param hash The precomputed hash of the key.
     * @param releaser Function called with the evicted value (T&&) if eviction occurs.
     * @return Reference to the inserted or updated value.
     */
    template<typename F>
    T& put(Key key, T value, size_t hash, F releaser) {
        // Assert that we have capacity to store at least one item
        assert(mCapacity > 0);

        auto it = mMap.find(key, hash);
        if (it != mMap.end()) {
            Node* node = it->second;
            assert(node != nullptr);

            moveToFront(node);

            node->value = std::move(value);
            return node->value;
        }

        if (UTILS_LIKELY(mMap.size() >= mCapacity)) {
            evict(releaser);
        }

        // Create new node at front of list.
        Node* node = mArena.template make<Node>(std::move(key), std::move(value), mHead);
        assert(node != nullptr);
        attach(node);

        mMap.emplace(node->key, node);

        return node->value;
    }

    template<typename F>
    inline T& put(Key key, T value, F releaser) {
        return put(std::move(key), std::move(value), Hash{}(key), std::move(releaser));
    }

    /**
     * Clear the cache, calling releaser on each item evicted.
     *
     * @param releaser Function called with the evicted value (T&&)
     */
    template<typename F>
    void clear(F releaser) {
        // Clear map first, just in case it tries to dereference any keys.
        mMap.clear();
        // Destroy everything in the arena.
        Node* node = mHead;
        while (UTILS_LIKELY(node)) {
            Node* next = node->next;
            releaser(std::move(node->value));
            mArena.destroy(node);
            node = next;
        }
        mHead = nullptr;
        mTail = nullptr;
    }

    void clear() {
        clear([](T&&){});
    }

    /** Returns the number of elements in the cache. */
    size_type size() const noexcept { return mMap.size(); }

    /** Returns the capacity of the cache. */
    size_type capacity() const noexcept { return mCapacity; }

private:
    struct Node {
        Key key;
        T value;
        Node* UTILS_NULLABLE prev;
        Node* UTILS_NULLABLE next;

        Node(Key&& k, T&& v, Node* UTILS_NULLABLE next)
                : key(std::move(k)),
                  value(std::move(v)),
                  prev(nullptr),
                  next(next) {}
    };

    void moveToFront(Node* UTILS_NONNULL node) {
        if (node == mHead) {
            return;
        }

        // First, detach from list.
        if (UTILS_UNLIKELY(node == mTail)) {
            assert(node->prev != nullptr);
            mTail = node->prev;
            mTail->next = nullptr;
        } else {
            assert(node->prev != nullptr);
            assert(node->next != nullptr);
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        // Then attach to head.
        node->prev = nullptr;
        node->next = mHead;
        attach(node);
    }

    // Attach node to front.
    void attach(Node* UTILS_NONNULL node) {
        if (UTILS_LIKELY(mHead != nullptr)) {
            mHead->prev = node;
        } else {
            assert(mTail == nullptr);
            mTail = node;
        }
        mHead = node;
    }

    // Evict the least recently used node.
    template<typename F>
    void evict(F releaser) {
        Node* node = mTail;
        assert(node != nullptr);

        if (UTILS_UNLIKELY(node == mHead)) {
            mTail = nullptr;
            mHead = nullptr;
        } else {
            // Move new tail node to mTail.
            assert(node->prev != nullptr);
            mTail = node->prev;
            mTail->next = nullptr;
        }

        // Finally free the node.
        releaser(std::move(node->value));
        mMap.erase(node->key);
        mArena.destroy(node);
    }

    size_type mCapacity;
    utils::Arena<utils::ObjectPoolAllocator<Node>, utils::LockingPolicy::NoLock> mArena;
    tsl::robin_map<MapKey, Node* UTILS_NONNULL, Hash, KeyEqual> mMap;
    Node* UTILS_NULLABLE mHead;
    Node* UTILS_NULLABLE mTail;
};

} // namespace utils

#endif // TNT_UTILS_LRUCACHE_H
