#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <type_traits> // for add_const_t
#include <utility>     // for pair, as_const
#include <vector>      // for vector

TEST_CASE_MAP("equal_range", int, int) {
    auto map = map_t();
    // auto map = std::unordered_map<int, int>();

    auto range = map.equal_range(123);
    REQUIRE(range.first == map.end());
    REQUIRE(range.second == map.end());

    map.try_emplace(1, 1);
    range = map.equal_range(123);
    REQUIRE(range.first == map.end());
    REQUIRE(range.second == map.end());

    int const x = 1;
    auto const_range = std::as_const(map).equal_range(x);
    REQUIRE(const_range.first == map.begin());
    REQUIRE(const_range.second == map.end());

    for (int i = 0; i < 100; ++i) {
        map.try_emplace(i, i);
    }
    range = map.equal_range(50);
    auto after_first = ++range.first;
    REQUIRE(range.second == after_first);
    REQUIRE(range.second != map.end());
}
