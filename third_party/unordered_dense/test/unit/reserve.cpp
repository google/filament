#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

TEST_CASE_MAP("reserve", int, int) {
    // there's no capacity() for std::deque
    if constexpr (!std::is_same_v<typename map_t::value_container_type, std::deque<std::pair<int, int>>>) {
        auto map = map_t();
        REQUIRE(map.values().capacity() <= 1000U);
        map.reserve(1000);
        REQUIRE(map.values().capacity() >= 1000U);
        REQUIRE(0U == map.values().size());
    }
}
