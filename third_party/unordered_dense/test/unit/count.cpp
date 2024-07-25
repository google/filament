#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

TEST_CASE_MAP("count", int, int) {
    auto map = map_t();
    REQUIRE(map.count(123) == 0);
    REQUIRE(map.count(0) == 0);
    map[123];
    REQUIRE(map.count(123) == 1);
    REQUIRE(map.count(0) == 0);
}
