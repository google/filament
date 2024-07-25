#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

TEST_CASE_MAP("empty_map_operations", int, int) {
    map_t m;

    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.begin());
    m[32];
    REQUIRE(m.end() != m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() != m.find(32));

    m = map_t();
    REQUIRE(m.end() == m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.find(32));

    map_t m2(m);
    REQUIRE(m2.end() == m2.begin());
    REQUIRE(m2.end() == m2.find(123));
    REQUIRE(m2.end() == m2.find(32));
    m2[32];
    REQUIRE(m2.end() != m2.begin());
    REQUIRE(m2.end() == m2.find(123));
    REQUIRE(m2.end() != m2.find(32));

    map_t empty;
    map_t m3(empty);
    REQUIRE(m3.end() == m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() == m3.find(32));
    m3[32];
    REQUIRE(m3.end() != m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() != m3.find(32));

    map_t m4(std::move(empty));
    REQUIRE(m4.count(123) == 0);
    REQUIRE(m4.end() == m4.begin());
    REQUIRE(m4.end() == m4.find(123));
    REQUIRE(m4.end() == m4.find(32));
}
