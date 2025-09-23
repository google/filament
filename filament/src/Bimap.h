/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BIMAP_H
#define TNT_FILAMENT_BIMAP_H

#include <utils/debug.h>

#include <tsl/robin_map.h>

#include <functional>
#include <memory>
#include <utility>

#include <stddef.h>

namespace filament {

/*
 * A semi-generic custom bimap. This bimap stores a key/value pair and can retrieve
 * the value from the key and the key from the value.
 * It is optimized for large keys and small values.  The keys are stored out-of-line and are
 * never moved.
 */
template<typename Key, typename Value,
        typename KeyHash = std::hash<Key>,
        typename ValueHash = std::hash<Value>,
        typename Allocator = std::allocator<Key>>
class Bimap {

    struct KeyDelegate {
        Key const* pKey = nullptr;
        bool operator==(KeyDelegate const& rhs) const noexcept {
            return *pKey == *rhs.pKey;
        }
    };

    // KeyWrapperHash delegates the hash computation to KeyHash
    struct KeyHasherDelegate {
        size_t operator()(KeyDelegate const& p) const noexcept {
            KeyHash const h;
            return h(*p.pKey);
        }
    };

    using ForwardMap = tsl::robin_map<
            KeyDelegate, Value,
            KeyHasherDelegate,
            std::equal_to<KeyDelegate>,
            std::allocator<std::pair<KeyDelegate, Value>>,
            true>;
    using BackwardMap = tsl::robin_map<Value, KeyDelegate, ValueHash>;

    Allocator mAllocator;
    ForwardMap mForwardMap;
    BackwardMap mBackwardMap;

public:
    Bimap() = default;

    explicit Bimap(Allocator&& allocator)
            : mAllocator(std::forward<Allocator>(allocator)) {
    }

    ~Bimap() noexcept {
        clear();
    }

    Bimap(Bimap const&) = delete;
    Bimap& operator=(Bimap const&) = delete;
    Bimap(Bimap&&) = delete;
    Bimap& operator=(Bimap&&) = delete;

    void clear() noexcept {
        // We only need to iterate one map, as they both point to the same keys.
        for (auto& pair : mForwardMap) {
            // Manually call the destructor on the key...
            pair.first.pKey->~Key();
            // ...then deallocate the memory.
            mAllocator.deallocate(const_cast<Key*>(pair.first.pKey), 1);
        }
        mForwardMap.clear();
        mBackwardMap.clear();
    }

    void reserve(size_t capacity) {
        mForwardMap.reserve(capacity);
        mBackwardMap.reserve(capacity);
    }

    bool empty() const noexcept {
        return mForwardMap.empty() && mBackwardMap.empty();
    }

    // insert a new key/value pair. duplicate are not allowed.
    void insert(Key const& key, Value const& value) noexcept {
        assert_invariant(find(key) == end() && findValue(value) == mBackwardMap.end());
        Key* pKey = mAllocator.allocate(1); // allocate storage for the key
        new(static_cast<void*>(pKey)) Key{ key }; // copy-construct the key
        // TODO: we can leak the Key if the calls below throw
        mForwardMap.insert({{ pKey }, value });
        mBackwardMap.insert({ value, { pKey }});
    }

    typename ForwardMap::iterator begin() { return mForwardMap.begin(); }
    typename ForwardMap::const_iterator begin() const { return mForwardMap.begin(); }
    typename ForwardMap::const_iterator cbegin() const { return mForwardMap.cbegin(); }

    typename ForwardMap::iterator end() { return mForwardMap.end(); }
    typename ForwardMap::const_iterator end() const { return mForwardMap.end(); }
    typename ForwardMap::const_iterator cend() const { return mForwardMap.cend(); }

    typename BackwardMap::iterator endValue() { return mBackwardMap.end(); }
    typename BackwardMap::const_iterator endValue() const { return mBackwardMap.end(); }

    // Find the value iterator from the key in O(1)
    typename ForwardMap::const_iterator find(Key const& key) const {
        return mForwardMap.find(KeyDelegate{ .pKey = &key });
    }
    typename ForwardMap::iterator find(Key const& key) {
        return mForwardMap.find(KeyDelegate{ .pKey = &key });
    }

    // Find the key iterator from the value in O(1). precondition, the value must exist.
    typename BackwardMap::const_iterator findValue(Value const& value) const {
        return mBackwardMap.find(value);
    }
    typename BackwardMap::iterator findValue(Value& value) {
        return mBackwardMap.find(value);
    }

    // Erase by key
    bool erase(Key const& key) {
        auto forward_it = find(key);
        if (forward_it != end()) {
            erase(forward_it);
            return true;
        }
        return false;
    }

    // Erase by forward map iterator
    void erase(typename ForwardMap::const_iterator it) {
        // Get a stable pointer to the key object before erasing.
        Key const* const pKey = const_cast<Key*>(it->first.pKey);

        // Find the corresponding entry in the backward map.
        auto backward_it = findValue(it->second);
        assert_invariant(backward_it != mBackwardMap.end());

        // Erase from both maps while the key is still valid.
        mBackwardMap.erase(backward_it);
        mForwardMap.erase(it);

        // Now that no map refers to the key, safely destroy and deallocate it.
        pKey->~Key();
        mAllocator.deallocate(const_cast<Key*>(pKey), 1);
    }

    // erase by backward map iterator
    void erase(typename BackwardMap::const_iterator it) {
        // Get a stable pointer to the key object.
        Key const* const pKey = it->second.pKey;

        // Find the corresponding iterator in the forward map before erasing.
        auto forward_it = find(*pKey);
        assert_invariant(forward_it != end());

        // Erase from both maps while the key object is still valid.
        mForwardMap.erase(forward_it);
        mBackwardMap.erase(it);

        // Now that no map refers to the key, we can safely destroy and deallocate it.
        pKey->~Key();
        mAllocator.deallocate(const_cast<Key*>(pKey), 1);
    }
};

} // namespace filament


#endif // TNT_FILAMENT_BIMAP_H
