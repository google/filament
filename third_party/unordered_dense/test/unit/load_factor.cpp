#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

TEST_CASE_MAP("load_factor", int, int) {
    auto m = map_t();

    REQUIRE(static_cast<double>(m.load_factor()) == doctest::Approx(0.0));

    for (int i = 0; i < 10000; ++i) {
        m.emplace(i, i);
        REQUIRE(m.load_factor() > 0.0F);
        REQUIRE(m.load_factor() <= 0.8F);
    }
}
