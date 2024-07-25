#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t
#include <cstdint> // for uint64_t
#include <vector>  // for vector

TEST_CASE_MAP("iterators_empty", size_t, size_t) {
    for (size_t i = 0; i < 10; ++i) {
        auto m = ankerl::unordered_dense::map<size_t, size_t>();
        REQUIRE(m.begin() == m.end());

        REQUIRE(m.end() == m.find(132));

        m[1];
        REQUIRE(m.begin() != m.end());
        REQUIRE(++m.begin() == m.end());
        m.clear();

        REQUIRE(m.begin() == m.end());
    }
}
