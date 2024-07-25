#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstdint> // for uint64_t

TEST_CASE_MAP("contains", uint64_t, uint64_t) {
    static_assert(std::is_same_v<bool, decltype(map_t{}.contains(1))>);

    auto map = map_t();

    REQUIRE(!map.contains(0));
    REQUIRE(!map.contains(123));
    map[123];
    REQUIRE(!map.contains(0));
    REQUIRE(map.contains(123));
    map.clear();
    REQUIRE(!map.contains(0));
    REQUIRE(!map.contains(123));
}
