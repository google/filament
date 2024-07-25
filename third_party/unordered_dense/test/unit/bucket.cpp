#include <ankerl/unordered_dense.h>

#include <app/counter.h>
#include <app/doctest.h>

#include <fmt/format.h>

#include <limits>
#include <stdexcept> // for out_of_range

using map_default_t = ankerl::unordered_dense::map<std::string, size_t>;

// big bucket type allows 2^64 elements, but has more memory & CPU overhead.
using map_big_t = ankerl::unordered_dense::map<std::string,
                                               size_t,
                                               ankerl::unordered_dense::hash<std::string>,
                                               std::equal_to<std::string>,
                                               std::allocator<std::pair<std::string, size_t>>,
                                               ankerl::unordered_dense::bucket_type::big>;

static_assert(sizeof(map_default_t::bucket_type) == 8U);
static_assert(sizeof(map_big_t::bucket_type) == sizeof(size_t) + 4U);
static_assert(map_default_t::max_size() == map_default_t::max_bucket_count());

#if SIZE_MAX == UINT32_MAX
static_assert(map_default_t::max_size() == uint64_t{1} << 31U);
static_assert(map_big_t::max_size() == uint64_t{1} << 31U);
#else
static_assert(map_default_t::max_size() == uint64_t{1} << 32U);
static_assert(map_big_t::max_size() == uint64_t{1} << 63U);
#endif

struct bucket_micro {
    static constexpr uint8_t dist_inc = 1U << 1U;             // 1 bits for fingerprint
    static constexpr uint8_t fingerprint_mask = dist_inc - 1; // 11 bit = 2048 positions for distance

    uint8_t m_dist_and_fingerprint;
    uint8_t m_value_idx;
};

TYPE_TO_STRING_MAP(counter::obj,
                   counter::obj,
                   ankerl::unordered_dense::hash<counter::obj>,
                   std::equal_to<counter::obj>,
                   std::allocator<std::pair<counter::obj, counter::obj>>,
                   bucket_micro);

TEST_CASE_MAP("bucket_micro",
              counter::obj,
              counter::obj,
              ankerl::unordered_dense::hash<counter::obj>,
              std::equal_to<counter::obj>,
              std::allocator<std::pair<counter::obj, counter::obj>>,
              bucket_micro) {
    counter counts;
    INFO(counts);

    auto map = map_t();
    INFO("map_t::max_size()=" << map_t::max_size());
    for (size_t i = 0; i < map_t::max_size(); ++i) {
        if (i == 255) {
            INFO("i=" << i);
        }
        auto const r = map.try_emplace({i, counts}, i, counts);
        REQUIRE(r.second);

        auto it = map.find({0, counts});
        REQUIRE(it != map.end());
    }
    // NOLINTNEXTLINE(llvm-else-after-return,readability-else-after-return)
    REQUIRE_THROWS_AS(map.try_emplace({map_t::max_size(), counts}, map_t::max_size(), counts), std::overflow_error);

    // check that all elements are there
    REQUIRE(map.size() == map_t::max_size());
    for (size_t i = 0; i < map_t::max_size(); ++i) {
        INFO(i);
        auto it = map.find({i, counts});
        REQUIRE(it != map.end());
        REQUIRE(it->first.get() == i);
        REQUIRE(it->second.get() == i);
    }
}
