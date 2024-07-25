#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t

TEST_CASE_MAP("rehash", size_t, int) {
    auto map = map_t();

    for (size_t i = 0; i < 1000; ++i) {
        map[i];
    }
    auto old_bucket_size = map.bucket_count();

    map.rehash(10000);
    REQUIRE(map.bucket_count() >= 10000);
    map.rehash(0);
    REQUIRE(map.bucket_count() == old_bucket_size);

    map.clear();
    map.rehash(0);
    REQUIRE(map.bucket_count() > 0);
    REQUIRE(map.bucket_count() < old_bucket_size);
}
