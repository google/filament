// #include "absl/container/flat_hash_map.h"
#include <ankerl/unordered_dense.h> // for map, operator==

#include <app/counting_allocator.h>

#include <third-party/nanobench.h>

#if __has_include("boost/unordered/unordered_flat_map.hpp")
#    if defined(__clang__)
#        pragma clang diagnostic push
#        pragma clang diagnostic ignored "-Wold-style-cast"
#    endif
#    include "boost/unordered/unordered_flat_map.hpp"
#    define HAS_BOOST_UNORDERED_FLAT_MAP() 1 // NOLINT(cppcoreguidelines-macro-usage)
#else
#    define HAS_BOOST_UNORDERED_FLAT_MAP() 0 // NOLINT(cppcoreguidelines-macro-usage)
#endif

#include <doctest.h>
#include <fmt/ostream.h>

#include <deque>
#include <filesystem>
#include <fstream>
#include <unordered_map>

template <typename Map>
void evaluate_map(Map& map) {
    auto rng = ankerl::nanobench::Rng{1234};

    auto num_elements = size_t{10'000'000};
    for (uint64_t i = 0; i < num_elements; ++i) {
        map[rng()] = i;
    }
    REQUIRE(map.size() == num_elements);
}

using hash_t = ankerl::unordered_dense::hash<uint64_t>;
using eq_t = std::equal_to<uint64_t>;
using pair_t = std::pair<uint64_t, uint64_t>;
using pair_const_t = std::pair<const uint64_t, uint64_t>;
using alloc_t = counting_allocator<pair_t>;
using alloc_const_t = counting_allocator<pair_const_t>;

TEST_CASE("allocated_memory_std_vector" * doctest::skip()) {
    auto counters = counts_for_allocator{};
    {
        using vec_t = std::vector<pair_t, alloc_t>;
        using map_t = ankerl::unordered_dense::map<uint64_t, uint64_t, hash_t, eq_t, vec_t>;
        auto map = map_t(0, hash_t{}, eq_t{}, alloc_t{&counters});
        evaluate_map(map);
    }
    counters.save("allocated_memory_std_vector.txt");
}

#if HAS_BOOST_UNORDERED_FLAT_MAP()

TEST_CASE("allocated_memory_boost_flat_map" * doctest::skip()) {
    auto counters = counts_for_allocator{};
    {
        using map_t = boost::unordered_flat_map<uint64_t, uint64_t, hash_t, eq_t, alloc_t>;
        auto map = map_t(alloc_t{&counters});
        evaluate_map(map);
    }
    counters.save("allocated_memory_unordered_flat_map.txt");
}
#endif

TEST_CASE("allocated_memory_std_deque" * doctest::skip()) {
    auto counters = counts_for_allocator{};
    {
        using vec_t = std::deque<pair_t, alloc_t>;
        using map_t = ankerl::unordered_dense::map<uint64_t, uint64_t, hash_t, eq_t, vec_t>;
        auto map = map_t(0, hash_t{}, eq_t{}, alloc_t{&counters});
        evaluate_map(map);
    }
    counters.save("allocated_memory_std_deque.txt");
}

TEST_CASE("allocated_memory_segmented_vector" * doctest::skip()) {
    auto counters = counts_for_allocator{};
    {
        using vec_t = ankerl::unordered_dense::segmented_vector<pair_t, alloc_t>;
        using map_t = ankerl::unordered_dense::segmented_map<uint64_t, uint64_t, hash_t, eq_t, vec_t>;
        auto map = map_t{0, hash_t{}, eq_t{}, alloc_t{&counters}};
        evaluate_map(map);
    }
    counters.save("allocated_memory_segmented_vector.txt");
}

#if 0

TEST_CASE("allocated_memory_std_unordered_map" * doctest::skip()) {
    auto counters = counts_for_allocator{};
    {
        using map_t = std::unordered_map<uint64_t, uint64_t, hash_t, eq_t, alloc_const_t>;
        auto map = map_t(0, alloc_t{&counters});
        evaluate_map(map);
    }
    counters.save("allocated_memory_std_unordered_map.txt");
}

TEST_CASE("allocated_memory_boost_unordered_flat_map" * doctest::skip()) {
    auto counters = counts_for_allocator{};
    {
        using map_t = absl::
            flat_hash_map<uint64_t, uint64_t, absl::container_internal::hash_default_hash<uint64_t>, eq_t, alloc_const_t>;
        auto map = map_t(0, alloc_t{&counters});
        evaluate_map(map);
    }
    counters.save("allocated_memory_absl_flat_hash_map.txt");
}

#endif