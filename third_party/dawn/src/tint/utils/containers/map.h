// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_CONTAINERS_MAP_H_
#define SRC_TINT_UTILS_CONTAINERS_MAP_H_

#include <unordered_map>

namespace tint {

/// Lookup is a utility function for fetching a value from an unordered map if
/// it exists, otherwise returning the `if_missing` argument.
/// @param map the unordered_map
/// @param key the map key of the item to query
/// @param if_missing the value to return if the map does not contain the given
/// key. Defaults to the zero-initializer for the value type.
/// @return the map item value, or `if_missing` if the map does not contain the
/// given key
template <typename K, typename V, typename H, typename C, typename KV = K>
V Lookup(const std::unordered_map<K, V, H, C>& map, const KV& key, const V& if_missing = {}) {
    auto it = map.find(key);
    return it != map.end() ? it->second : if_missing;
}

/// GetOrAdd is a utility function for lazily adding to an unordered map.
/// If the map already contains the key `key` then this is returned, otherwise
/// `create()` is called and the result is added to the map and is returned.
/// @param map the unordered_map
/// @param key the map key of the item to query or add
/// @param create a callable function-like object with the signature `V()`
/// @return the value of the item with the given key, or the newly created item
template <typename K, typename V, typename H, typename C, typename CREATE>
V GetOrAdd(std::unordered_map<K, V, H, C>& map, const K& key, CREATE&& create) {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    V value = create();
    map.emplace(key, value);
    return value;
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_MAP_H_
