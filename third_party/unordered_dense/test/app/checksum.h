#pragma once

#include <app/counter.h>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace checksum {

// final step from MurmurHash3
[[nodiscard]] static inline auto mix(uint64_t k) -> uint64_t {
    k ^= k >> 33U;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33U;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33U;
    return k;
}

[[maybe_unused]] [[nodiscard]] static inline auto mix(std::string_view data) -> uint64_t {
    static constexpr uint64_t fnv_offset_basis = UINT64_C(14695981039346656037);
    static constexpr uint64_t fnv_prime = UINT64_C(1099511628211);

    uint64_t val = fnv_offset_basis;
    for (auto c : data) {
        val ^= static_cast<uint64_t>(c);
        val *= fnv_prime;
    }
    return val;
}

[[maybe_unused]] [[nodiscard]] static inline auto mix(counter::obj const& cdv) -> uint64_t {
    return mix(cdv.get());
}

// from boost::hash_combine, with additional fmix64 of value
[[maybe_unused]] [[nodiscard]] static inline auto combine(uint64_t seed, uint64_t value) -> uint64_t {
    return seed ^ (value + 0x9e3779b9 + (seed << 6U) + (seed >> 2U));
}

// calculates a hash of any iterable map. Order is irrelevant for the hash's result, as it simply
// xors the elements together.
template <typename M>
[[nodiscard]] auto map(const M& map) -> uint64_t {
    uint64_t combined_hash = 1;

    uint64_t num_elements = 0;
    for (auto const& entry : map) {
        auto entry_hash = combine(mix(entry.first), mix(entry.second));

        combined_hash ^= entry_hash;
        ++num_elements;
    }

    return combine(combined_hash, num_elements);
}

// map of maps
template <typename MM>
[[nodiscard]] auto mapmap(const MM& mapmap) -> uint64_t {
    uint64_t combined_hash = 1;

    uint64_t num_elements = 0;
    for (auto const& entry : mapmap) {
        auto entry_hash = combine(mix(entry.first), map(entry.second));

        combined_hash ^= entry_hash;
        ++num_elements;
    }

    return combine(combined_hash, num_elements);
}

} // namespace checksum
