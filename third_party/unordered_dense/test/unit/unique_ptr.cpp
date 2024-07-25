#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t
#include <memory>  // for operator!=, unique_ptr, make_unique
#include <utility> // for move, pair
#include <vector>  // for vector

TYPE_TO_STRING_MAP(size_t, std::unique_ptr<int>);

TEST_CASE_MAP("unique_ptr", size_t, std::unique_ptr<int>) {
    map_t m;
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.begin());
    m[static_cast<size_t>(32)] = std::make_unique<int>(123);
    REQUIRE(m.end() != m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() != m.find(32));

    for (auto const& kv : m) {
        REQUIRE(kv.first == 32);
        REQUIRE(kv.second != nullptr);
        REQUIRE(*kv.second == 123);
    }

    m = map_t();
    REQUIRE(m.end() == m.begin());
    REQUIRE(m.end() == m.find(123));
    REQUIRE(m.end() == m.find(32));

    map_t empty;
    map_t m3(std::move(empty));
    REQUIRE(m3.end() == m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() == m3.find(32));
    m3[static_cast<size_t>(32)];
    REQUIRE(m3.end() != m3.begin());
    REQUIRE(m3.end() == m3.find(123));
    REQUIRE(m3.end() != m3.find(32));

    empty = map_t{};
    map_t m4(std::move(empty));
    REQUIRE(m4.count(123) == 0);
    REQUIRE(m4.end() == m4.begin());
    REQUIRE(m4.end() == m4.find(123));
    REQUIRE(m4.end() == m4.find(32));
}

TEST_CASE_MAP("unique_ptr_fill", size_t, std::unique_ptr<int>) {
    map_t m;
    for (int i = 0; i < 1000; ++i) {
        // m.emplace(i % 500, std::make_unique<int>(i));
        m.emplace(static_cast<size_t>(i), new int(i)); // NOLINT(cppcoreguidelines-owning-memory)
        // element is still constructed, so there's no memory leak here.
        // Boost 1.80 behaves differently
        m.emplace(static_cast<size_t>(i), new int(i)); // NOLINT(cppcoreguidelines-owning-memory)
    }
}
