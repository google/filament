#include <ankerl/unordered_dense.h>

#include <doctest.h>

namespace versioned_namespace = ankerl::unordered_dense::v4_4_0;

static_assert(std::is_same_v<versioned_namespace::map<int, int>, ankerl::unordered_dense::map<int, int>>);
static_assert(std::is_same_v<versioned_namespace::hash<int>, ankerl::unordered_dense::hash<int>>);

TEST_CASE("version_namespace") {
    auto map = versioned_namespace::map<int, int>{};
    REQUIRE(map.empty());
}
