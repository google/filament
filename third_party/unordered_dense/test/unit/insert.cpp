#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <tuple>   // for forward_as_tuple
#include <utility> // for piecewise_construct
#include <vector>  // for vector

TEST_CASE_MAP("insert", unsigned int, int) {
    auto map = map_t();
    auto const val = typename map_t::value_type(123U, 321);
    map.insert(val);
    REQUIRE(map.size() == 1);

    REQUIRE(map[123U] == 321);
}

TEST_CASE_MAP("insert_hint", unsigned int, int) {
    auto map = map_t();
    auto it = map.insert(map.begin(), {1, 2});

    auto vt = typename decltype(map)::value_type{3, 4};
    map.insert(it, vt);
    REQUIRE(map.size() == 2);
    REQUIRE(map[1] == 2);
    REQUIRE(map[3] == 4);

    auto const vt2 = typename decltype(map)::value_type{10, 11};
    map.insert(it, vt2);
    REQUIRE(map.size() == 3);
    REQUIRE(map[10] == 11);

    it = map.emplace_hint(it, std::piecewise_construct, std::forward_as_tuple(123), std::forward_as_tuple(321));
    REQUIRE(map.size() == 4);
    REQUIRE(map[123] == 321);
}
