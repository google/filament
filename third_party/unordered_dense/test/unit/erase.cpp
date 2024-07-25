#include <ankerl/unordered_dense.h>

#include <third-party/nanobench.h>

#include <app/doctest.h>

#include <algorithm>     // for all_of
#include <cstddef>       // for size_t
#include <cstdint>       // for uint32_t
#include <unordered_set> // for unordered_set, operator!=
#include <vector>        // for vector

template <typename A, typename B>
[[nodiscard]] auto is_eq(A const& a, B const& b) -> bool {
    if (a.size() != b.size()) {
        return false;
    }
    return std::all_of(a.begin(), a.end(), [&b](auto const& k) {
        return b.end() != b.find(k);
    });
}

TEST_CASE_SET("insert_erase_random", uint32_t) {
    auto uds = set_t();
    auto us = std::unordered_set<uint32_t>();
    auto rng = ankerl::nanobench::Rng(123);
    for (size_t i = 0; i < 10000; ++i) {
        auto key = rng.bounded(1000);
        uds.insert(key);
        us.insert(key);
        REQUIRE(uds.size() == us.size());

        key = rng.bounded(1000);
        REQUIRE(uds.erase(key) == us.erase(key));
        REQUIRE(uds.size() == us.size());
    }
    REQUIRE(is_eq(uds, us));
    auto k = *uds.begin();
    uds.erase(k);
    REQUIRE(!is_eq(uds, us));
    us.erase(k);
    REQUIRE(is_eq(uds, us));
}

TEST_CASE_MAP("erase", int, int) {
    auto map = ankerl::unordered_dense::map<int, int>();
    REQUIRE(0 == map.erase(123));
    REQUIRE(0 == map.count(0));
}
