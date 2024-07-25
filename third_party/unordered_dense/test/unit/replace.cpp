#include <ankerl/unordered_dense.h>

#include <app/counter.h>
#include <app/doctest.h>

TEST_CASE_MAP("replace", counter::obj, counter::obj) {
    auto counts = counter{};
    INFO(counts);

    auto container = typename map_t::value_container_type{};

    for (size_t i = 0; i < 100; ++i) {
        container.emplace_back(counter::obj{i, counts}, counter::obj{i, counts});
        container.emplace_back(counter::obj{i, counts}, counter::obj{i, counts});
    }

    for (size_t i = 0; i < 10; ++i) {
        container.emplace_back(counter::obj{i, counts}, counter::obj{i, counts});
    }

    // add some elements
    auto map = map_t();
    for (size_t i = 0; i < 10; ++i) {
        map.try_emplace(counter::obj{i, counts}, counter::obj{i, counts});
    }

    map.replace(std::move(container));

    REQUIRE(map.size() == 100U);
    for (size_t i = 0; i < 100; ++i) {
        REQUIRE(map.contains(counter::obj{i, counts}));
    }
}
