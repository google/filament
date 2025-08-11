/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_UTILS_HASH_H
#define TNT_UTILS_HASH_H

#include <functional>   // for std::hash
#include <string_view>
#include <utility>
#include <vector>

#include <stdint.h>
#include <stddef.h>

namespace utils::hash {

// The standard CRC-32 polynomial (used in Ethernet, ZIP, PNG, etc.)
static constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320;

// Generates the lookup table for the CRC-32 algorithm.
inline void crc32GenerateTable(std::vector<uint32_t>& table) {
    if (table.size() != 256) {
        table.resize(256);
    }

    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc = crc >> 1;
            }
        }
        table[i] = crc;
    }
}

// Updates the CRC-32 value with a new chunk of data. This function can be called multiple times to
// process a large buffer in smaller, separate parts.
//  @param previous_crc The CRC value returned from the previous call to `crc32Update`.
//  @param data A pointer to the data buffer for this chunk.
//  @param length The length of the data buffer in bytes.
//  @return The new CRC value.
inline uint32_t crc32Update(uint32_t previous_crc, const void* data, size_t length, const std::vector<uint32_t>& table) {
    uint32_t crc = ~previous_crc;
    const uint8_t* current = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < length; ++i) {
        // The core CRC-32 calculation step.
        crc = (crc >> 8) ^ table[(crc ^ current[i]) & 0xFF];
    }
    return ~crc;
}

inline size_t combine(size_t lhs, size_t rhs) noexcept {
    std::pair<size_t, size_t> const p{ lhs, rhs };
    return std::hash<std::string_view>{}({ (char*)&p, sizeof(p) });
}

// Hash function that takes an arbitrary swath of word-aligned data.
inline uint32_t murmur3(const uint32_t* key, size_t wordCount, uint32_t seed) noexcept {
    uint32_t h = seed;
    size_t i = wordCount;
    do {
        uint32_t k = *key++;
        k *= 0xcc9e2d51u;
        k = (k << 15u) | (k >> 17u);
        k *= 0x1b873593u;
        h ^= k;
        h = (h << 13u) | (h >> 19u);
        h = (h * 5u) + 0xe6546b64u;
    } while (--i);
    h ^= wordCount;
    h ^= h >> 16u;
    h *= 0x85ebca6bu;
    h ^= h >> 13u;
    h *= 0xc2b2ae35u;
    h ^= h >> 16u;
    return h;
}

// The hash yields the same result for a given byte sequence regardless of alignment.
inline uint32_t murmurSlow(const uint8_t* key, size_t byteCount, uint32_t seed) noexcept {
    const size_t wordCount = (byteCount + 3) / 4;
    const uint8_t* const last = key + byteCount;

    // The remainder is identical to murmur3() except an inner loop safely "reads" an entire word.
    uint32_t h = seed;
    size_t wc = wordCount;
    do {
        uint32_t k = 0;
        for (int i = 0; i < 4 && key < last; ++i, ++key) {
            k >>= 8;
            k |= uint32_t(*key) << 24;
        }
        k *= 0xcc9e2d51u;
        k = (k << 15u) | (k >> 17u);
        k *= 0x1b873593u;
        h ^= k;
        h = (h << 13u) | (h >> 19u);
        h = (h * 5u) + 0xe6546b64u;
    } while (--wc);
    h ^= wordCount;
    h ^= h >> 16u;
    h *= 0x85ebca6bu;
    h ^= h >> 13u;
    h *= 0xc2b2ae35u;
    h ^= h >> 16u;
    return h;
}

template<typename T>
struct MurmurHashFn {
    uint32_t operator()(const T& key) const noexcept {
        static_assert(0 == (sizeof(key) & 3u), "Hashing requires a size that is a multiple of 4.");
        return murmur3((const uint32_t*) &key, sizeof(key) / 4, 0);
    }
};

// combines two hashes together
template<class T>
inline void combine(size_t& seed, const T& v) noexcept {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
}

// combines two hashes together, faster but less good
template<class T>
inline void combine_fast(size_t& seed, const T& v) noexcept {
    std::hash<T> hasher;
    seed ^= hasher(v) << 1u;
}

} // namespace utils::hash

#endif // TNT_UTILS_HASH_H
