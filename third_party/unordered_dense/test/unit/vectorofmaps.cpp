#include <ankerl/unordered_dense.h>

#include <app/counter.h>
#include <app/doctest.h>

#include <third-party/nanobench.h>

#include <algorithm> // for max
#include <cstddef>   // for size_t
#include <utility>   // for move
#include <vector>    // for vector

template <typename Map>
void fill(counter& counts, Map& map, ankerl::nanobench::Rng& rng) {
    auto n = rng.bounded(20);
    for (size_t i = 0; i < n; ++i) {
        auto a = rng.bounded(20);
        auto b = rng.bounded(20);
        map.try_emplace({a, counts}, b, counts);
    }
}

TEST_CASE_MAP("vectormap", counter::obj, counter::obj) {
    auto counts = counter();
    INFO(counts);

    auto rng = ankerl::nanobench::Rng(32154);
    {
        counts("begin");
        std::vector<map_t> maps;

        // copies
        for (size_t i = 0; i < 10; ++i) {
            map_t m;
            fill(counts, m, rng);
            maps.push_back(m);
        }
        counts("copies");

        // move
        for (size_t i = 0; i < 10; ++i) {
            map_t m;
            fill(counts, m, rng);
            maps.push_back(std::move(m));
        }
        counts("move");

        // emplace
        for (size_t i = 0; i < 10; ++i) {
            maps.emplace_back();
            fill(counts, maps.back(), rng);
        }
        counts("emplace");
    }
    counts("dtor");
    REQUIRE(counts.dtor() ==
            counts.ctor() + counts.static_default_ctor + counts.copy_ctor() + counts.default_ctor() + counts.move_ctor());
}
