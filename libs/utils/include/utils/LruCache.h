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

#include <utils/GenerationalArena.h>

#include <tsl/robin_map.h>

#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>

namespace utils {

/**
 * A fixed-capacity Least Recently Used (LRU) cache.
 *
 * This container allows O(1) access, insertion, and eviction.
 * It uses a GenerationalArena to store the nodes of a doubly-linked list
 * representing the LRU order, and a robin_map for fast key lookups.
 *
 * @tparam Key The type of the keys.
 * @tparam T The type of the values.
 * @tparam Hasher The hasher for the keys.
 */
template<typename Key, typename T, typename Hasher = std::hash<Key>>
class LruCache {
public:
    /**
     * Creates an LRU cache with the specified capacity.
     * @param capacity The maximum number of elements the cache can hold.
     */
    explicit LruCache(size_t capacity)
            : mCapacity(capacity),
              mArena(capacity) {
        mMap.reserve(capacity);
    }

    // Move-only because GenerationalArena is move-only.
    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;
    LruCache(LruCache&&) = default;
    LruCache& operator=(LruCache&&) = default;
    ~LruCache() = default;

    /**
     * Returns a reference to the value associated with the key.
     *
     * Additionally moves the key/value pair to the front of the most recently
     * used (MRU) list.
     *
     * The key MUST exist in the cache. Accessing a non-existent key is
     * undefined behavior (will assert in debug builds).
     *
     * @param key The key to look up.
     * @return Reference to the value.
     */
    T& get(const Key& key) {
        auto it = mMap.find(key);
        assert(it != mMap.end());

        GenerationalArenaHandle handle = it->second;
        Node* node = mArena.get(handle);
        assert(node != nullptr);

        moveToFront(handle, node);

        return node->value;
    }

    /**
     * Inserts a new entry or updates an existing one.
     *
     * Prepends it to the front of the most recently used (MRU) list.
     * Potentially evicts the least-recently used (LRU) key/value pair by
     * calling the releaser function.
     *
     * @tparam F Callable type accepting T&&.
     * @param key The key to insert or update.
     * @param t The value to insert or update.
     * @param releaser Function called with the evicted value (T&&) if eviction occurs.
     * @return Reference to the inserted or updated value.
     */
    template<typename F>
    T& put(Key key, T value, F releaser) {
        auto it = mMap.find(key);
        if (it != mMap.end()) {
            GenerationalArenaHandle handle = it->second;
            Node* node = mArena.get(handle);
            assert(node != nullptr);

            moveToFront(handle);

            node->value = std::move(value);
            return node->value;
        }

        if (mMap.size() >= mCapacity) {
            evict(releaser);
        }

        GenerationalArenaHandle handle = mArena.allocate(key, std::move(value));
        Node* node = mArena.get(handle);
        assert(node != nullptr);

        mMap.emplace(std::move(key), handle);
        attach(handle, node);

        return node->value;
    }

    /** Returns the number of elements in the cache. */
    size_t size() const { return mMap.size(); }

    /** Returns the capacity of the cache. */
    size_t capacity() const { return mCapacity; }

private:
    struct Node {
        Key key;
        T value;
        GenerationalArenaHandle prev;
        GenerationalArenaHandle next;

        Node(const Key& k, T&& v)
                : key(k),
                  value(std::move(v)) {}

        // Overload for move-constructed key if needed, or pass by value/move.
        // Ideally we want to support both, but GenerationalArena forwards args.
        // We'll simplify constructor to take by value/move.
        template<typename K, typename V>
        Node(K&& k, V&& v)
                : key(std::forward<K>(k)),
                  value(std::forward<V>(v)) {}
    };

    void moveToFront(GenerationalArenaHandle handle, Node* node) {
        if (handle == mHead) {
            return;
        }
        detach(handle, node);
        attach(handle, node);
    }

    // Detach from the linked list.
    void detach(GenerationalArenaHandle handle, Node* node) {
        if (handle == mHead) {
            mHead = node->next;
        }
        if (handle == mTail) {
            mTail = node->prev;
        }

        if (node->prev) {
            mArena.get(node->prev)->next = node->next;
        }
        if (node->next) {
            mArena.get(node->next)->prev = node->prev;
        }
    }

    // Attach to head.
    void attach(GenerationalArenaHandle handle, Node* node) {
        node->next = mHead;
        node->prev.clear();

        if (mHead) {
            Node* headNode = mArena.get(mHead);
            assert(headNode != nullptr);
            headNode->prev = handle;
        }
        mHead = handle;

        if (!mTail) {
            mTail = handle;
        }
    }

    template<typename F>
    void evict(F releaser) {
        assert(mTail);
        GenerationalArenaHandle handle = mTail;
        Node* node = mArena.get(handle);

        releaser(std::move(node->value));

        mMap.erase(node->key);
        detach(handle);
        mArena.free(handle);
    }

    size_t mCapacity;
    GenerationalArena<Node> mArena;
    tsl::robin_map<Key, GenerationalArenaHandle, Hasher> mMap;
    GenerationalArenaHandle mHead;
    GenerationalArenaHandle mTail;
};

} // namespace utils

#endif // TNT_UTILS_LRUCACHE_H
