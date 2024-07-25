#include <ankerl/unordered_dense.h> // for map, operator==

#include <third-party/nanobench.h> // for Rng, Bench

#include <app/doctest.h> // for TestCase, skip, TEST_CASE, test_...
#include <fmt/core.h>    // for format

#include <cstddef>       // for size_t
#include <cstdint>       // for uint64_t
#include <string_view>   // for string_view
#include <unordered_map> // for unordered_map, operator==

template <typename Map>
void bench(std::string_view name) {
    auto a = Map();
    auto rng = ankerl::nanobench::Rng(123);
    for (size_t i = 0; i < 1000000; ++i) {
        a.try_emplace(rng(), rng());
    }

    Map b;
    ankerl::nanobench::Bench().batch(a.size() * 2).run(fmt::format("copy {}", name), [&] {
        b = a;
        a = b;
    });
    REQUIRE(a == b);
}

#if 0

TEST_CASE("bench_copy_rhn" * doctest::test_suite("bench") * doctest::skip()) {
    bench<robin_hood::unordered_node_map<uint64_t, uint64_t>>("robin_hood::unordered_node_map");
}

TEST_CASE("bench_copy_rhf" * doctest::test_suite("bench") * doctest::skip()) {
    bench<robin_hood::unordered_flat_map<uint64_t, uint64_t>>("robin_hood::unordered_flat_map");
}

#endif

TEST_CASE_MAP("bench_copy_udm" * doctest::test_suite("bench") * doctest::skip(), uint64_t, uint64_t) {
    bench<map_t>("ankerl::unordered_dense::map");
}

TEST_CASE("bench_copy_std" * doctest::test_suite("bench") * doctest::skip()) {
    bench<std::unordered_map<uint64_t, uint64_t>>("std::unordered_map");
}
