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
#include <stdint.h>

namespace filament {

// Type-erased handle wrapper used as Bimap's Value type so that different Handle<T> types can share
// the same BimapBackward instantiation.
struct BimapHandleValue {
    uint32_t id;
    friend bool operator==(BimapHandleValue const lhs, BimapHandleValue const rhs) noexcept {
        return lhs.id == rhs.id;
    }
};

struct BimapHandleValueHasher {
    size_t operator()(BimapHandleValue const v) const noexcept {
        return std::hash<uint32_t>()(v.id);
    }
};

namespace details {

// Base class holding the backward map (Value -> Key const*) using a type-erased void const* pointer
// for the key. Since the backward map only hashes and compares Value and never dereferences Key,
// factoring it out of Bimap<Key, ...> allows all Bimap instantiations with the same (Value, ValueHash)
// to share a single tsl::robin_map instantiation, reducing template code bloat.
template<typename Value, typename ValueHash>
class BimapBackward {
protected:
    struct VoidKeyDelegate {
        void const* pKey = nullptr;
    };

    using RawBackwardMap = tsl::robin_map<Value, VoidKeyDelegate, ValueHash>;
    RawBackwardMap mBackwardMap;

    void backwardClear() noexcept { mBackwardMap.clear(); }
    void backwardReserve(size_t capacity) { mBackwardMap.reserve(capacity); }
    bool backwardEmpty() const noexcept { return mBackwardMap.empty(); }
    void backwardInsert(Value const& value, void const* pKey) {
        mBackwardMap.insert({ value, { pKey } });
    }
    typename RawBackwardMap::const_iterator backwardFind(Value const& value) const {
        return mBackwardMap.find(value);
    }
    void backwardErase(typename RawBackwardMap::const_iterator it) {
        mBackwardMap.erase(it);
    }
};

} // namespace details

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
class Bimap : private details::BimapBackward<Value, ValueHash> {
    using BackwardBase = details::BimapBackward<Value, ValueHash>;
    using RawBackwardIterator = typename BackwardBase::RawBackwardMap::const_iterator;

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

    Allocator mAllocator;
    ForwardMap mForwardMap;

public:
    class BackwardConstIterator {
        RawBackwardIterator mIt{};
        friend class Bimap;
    public:
        BackwardConstIterator() = default;
        explicit BackwardConstIterator(RawBackwardIterator it) noexcept : mIt(it) {}

        struct ArrowProxy {
            Value const& first;
            KeyDelegate second;
            ArrowProxy const* operator->() const noexcept { return this; }
        };

        ArrowProxy operator->() const noexcept {
            return { mIt->first, { static_cast<Key const*>(mIt->second.pKey) } };
        }

        friend bool operator==(BackwardConstIterator const& lhs,
                BackwardConstIterator const& rhs) noexcept {
            return lhs.mIt == rhs.mIt;
        }
        friend bool operator!=(BackwardConstIterator const& lhs,
                BackwardConstIterator const& rhs) noexcept {
            return lhs.mIt != rhs.mIt;
        }
    };

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
        this->backwardClear();
    }

    void reserve(size_t capacity) {
        mForwardMap.reserve(capacity);
        this->backwardReserve(capacity);
    }

    bool empty() const noexcept {
        return mForwardMap.empty() && this->backwardEmpty();
    }

    // insert a new key/value pair. duplicate are not allowed.
    void insert(Key const& key, Value const& value) noexcept {
        assert_invariant(find(key) == end() && findValue(value) == endValue());
        Key* pKey = mAllocator.allocate(1); // allocate storage for the key
        new(static_cast<void*>(pKey)) Key{ key }; // copy-construct the key
        // TODO: we can leak the Key if the calls below throw
        mForwardMap.insert({{ pKey }, value });
        this->backwardInsert(value, pKey);
    }

    typename ForwardMap::iterator begin() { return mForwardMap.begin(); }
    typename ForwardMap::const_iterator begin() const { return mForwardMap.begin(); }
    typename ForwardMap::const_iterator cbegin() const { return mForwardMap.cbegin(); }

    typename ForwardMap::iterator end() { return mForwardMap.end(); }
    typename ForwardMap::const_iterator end() const { return mForwardMap.end(); }
    typename ForwardMap::const_iterator cend() const { return mForwardMap.cend(); }

    BackwardConstIterator endValue() const noexcept {
        return BackwardConstIterator{ this->mBackwardMap.end() };
    }

    // Find the value iterator from the key in O(1)
    typename ForwardMap::const_iterator find(Key const& key) const {
        return mForwardMap.find(KeyDelegate{ .pKey = &key });
    }
    typename ForwardMap::iterator find(Key const& key) {
        return mForwardMap.find(KeyDelegate{ .pKey = &key });
    }

    // Find the key iterator from the value in O(1). precondition, the value must exist.
    BackwardConstIterator findValue(Value const& value) const {
        return BackwardConstIterator{ this->backwardFind(value) };
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
        assert_invariant(backward_it != endValue());

        // Erase from both maps while the key is still valid.
        this->backwardErase(backward_it.mIt);
        mForwardMap.erase(it);

        // Now that no map refers to the key, safely destroy and deallocate it.
        pKey->~Key();
        mAllocator.deallocate(const_cast<Key*>(pKey), 1);
    }

    // erase by backward map iterator
    void erase(BackwardConstIterator it) {
        // Get a stable pointer to the key object.
        Key const* const pKey = it->second.pKey;

        // Find the corresponding iterator in the forward map before erasing.
        auto forward_it = find(*pKey);
        assert_invariant(forward_it != end());

        // Erase from both maps while the key object is still valid.
        mForwardMap.erase(forward_it);
        this->backwardErase(it.mIt);

        // Now that no map refers to the key, we can safely destroy and deallocate it.
        pKey->~Key();
        mAllocator.deallocate(const_cast<Key*>(pKey), 1);
    }
};

} // namespace filament


#endif // TNT_FILAMENT_BIMAP_H
