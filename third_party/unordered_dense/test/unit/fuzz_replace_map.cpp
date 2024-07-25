#include <ankerl/unordered_dense.h>
#include <app/doctest.h>
#include <fuzz/run.h>

#include <unordered_map>

namespace {

template <typename Map>
void replace_map(fuzz::provider p) {
    auto counts = counter{};

    using map_t = Map;

    auto initial_size = p.bounded<size_t>(100);
    auto map = map_t{};
    for (size_t i = 0; i < initial_size; ++i) {
        map.try_emplace(counter::obj{i, counts}, counter::obj{i, counts});
    }

    // create a container with data in it provided by fuzzer
    auto container = typename map_t::value_container_type{};
    auto comparison_container = std::vector<std::pair<size_t, size_t>>();
    auto v = size_t{};
    while (p.has_remaining_bytes()) {
        auto key = p.integral<size_t>();
        container.emplace_back(counter::obj{key, counts}, counter::obj{v, counts});
        comparison_container.emplace_back(key, v);
        ++v;
    }

    // create comparison map with the same move-back-forward algorithm
    auto comparison_map = std::unordered_map<size_t, size_t>{};
    size_t idx = 0;
    while (idx != comparison_container.size()) {
        auto [key, val] = comparison_container[idx];
        if (comparison_map.try_emplace(key, val).second) {
            ++idx;
        } else {
            comparison_container[idx] = comparison_container.back();
            comparison_container.pop_back();
        }
    }

    map.replace(std::move(container));

    // now check if the data in the map is exactly what we expect
    REQUIRE(map.size() == comparison_map.size());
    for (auto [key, val] : comparison_map) {
        auto key_obj = counter::obj{key, counts};
        auto val_obj = counter::obj{val, counts};
        auto it = map.find(key_obj);
        REQUIRE(it != map.end());
        REQUIRE(it->first == key_obj);
        REQUIRE(it->second == val_obj);
    }
}

} // namespace

TEST_CASE("fuzz_replace_map" * doctest::test_suite("fuzz")) {
    fuzz::run([](fuzz::provider p) {
        replace_map<ankerl::unordered_dense::map<counter::obj, counter::obj>>(p.copy());
        replace_map<ankerl::unordered_dense::segmented_map<counter::obj, counter::obj>>(p.copy());
        replace_map<ankerl::unordered_dense::map<counter::obj,
                                                 counter::obj,
                                                 ankerl::unordered_dense::hash<counter::obj>,
                                                 std::equal_to<counter::obj>,
                                                 std::deque<std::pair<counter::obj, counter::obj>>>>(p.copy());
    });
}
