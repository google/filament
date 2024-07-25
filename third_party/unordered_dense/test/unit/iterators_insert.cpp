#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <algorithm> // for max
#include <utility>   // for pair
#include <vector>    // for vector

TEST_CASE_MAP("iterators_insert", int, int) {
    auto v = std::vector<std::pair<int, int>>();
    v.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        v.emplace_back(i, i);
    }

    auto map = map_t(v.begin(), v.end());
    REQUIRE(map.size() == v.size());
    for (auto const& kv : v) {
        REQUIRE(map.count(kv.first) == 1);
        auto it = map.find(kv.first);
        REQUIRE(it != map.end());
    }
}
