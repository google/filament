#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t
#include <vector>  // for vector

// struct that provides both hash and equals operator
struct hash_with_equal {
    auto operator()(int x) const -> size_t {
        return static_cast<size_t>(x);
    }

    auto operator()(int a, int b) const -> bool {
        return a == b;
    }
};

TYPE_TO_STRING_MAP(int, int, hash_with_equal, hash_with_equal);

// make sure the map works with the same type (check that it handles the diamond problem)
TEST_CASE_MAP("diamond_problem", int, int, hash_with_equal, hash_with_equal) {
    auto map = map_t();
    map[1] = 2;
    REQUIRE(map.size() == 1);
    REQUIRE(map.find(1) != map.end());
}
