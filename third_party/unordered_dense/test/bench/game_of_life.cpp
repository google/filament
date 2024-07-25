#include <ankerl/unordered_dense.h> // for map

#include <app/counting_allocator.h>

#include <app/doctest.h> // for TestCase, skip, ResultBuilder
#include <fmt/core.h>    // for format, print

#include <unordered_map>
#include <vector>

#if __has_include("boost/unordered/unordered_flat_map.hpp")
#    if defined(__clang__)
#        pragma clang diagnostic ignored "-Wold-style-cast"
#    endif
#    include "boost/unordered/unordered_flat_map.hpp"
#    define HAS_BOOST_UNORDERED_FLAT_MAP() 1 // NOLINT(cppcoreguidelines-macro-usage)
#else
#    define HAS_BOOST_UNORDERED_FLAT_MAP() 0 // NOLINT(cppcoreguidelines-macro-usage)
#endif

#if 0 && __has_include("absl/container/flat_hash_map.h")
#    if defined(__clang__)
#        pragma clang diagnostic ignored "-Wdeprecated-builtins"
#        pragma clang diagnostic ignored "-Wsign-conversion"
#    endif
#    include <absl/container/flat_hash_map.h>
#    define HAS_ABSL() 1 // NOLINT(cppcoreguidelines-macro-usage)
#else
#    define HAS_ABSL() 0 // NOLINT(cppcoreguidelines-macro-usage)
#endif

class vec2 {
    uint32_t m_xy;

public:
    constexpr vec2(uint16_t x, uint16_t y)
        : m_xy{static_cast<uint32_t>(x) << 16U | y} {}

    constexpr explicit vec2(uint32_t xy)
        : m_xy(xy) {}

    [[nodiscard]] constexpr auto pack() const -> uint32_t {
        return m_xy;
    };

    [[nodiscard]] constexpr auto add_x(uint16_t x) const -> vec2 {
        return vec2{m_xy + (static_cast<uint32_t>(x) << 16U)};
    }

    [[nodiscard]] constexpr auto add_y(uint16_t y) const -> vec2 {
        return vec2{(m_xy & 0xffff0000) | ((m_xy + y) & 0xffff)};
    }

    template <typename Op>
    constexpr void for_each_surrounding(Op&& op) const {
        uint32_t v = m_xy;

        uint32_t upper = (v & 0xffff0000U) - 0x10000;
        uint32_t l1 = (v - 1) & 0xffffU;
        uint32_t l2 = v & 0xffffU;
        uint32_t l3 = (v + 1) & 0xffffU;

        op(upper | l1);
        op(upper | l2);
        op(upper | l3);

        upper += 0x10000;
        op(upper | l1);
        // op(upper | l2);
        op(upper | l3);

        upper += 0x10000;
        op(upper | l1);
        op(upper | l2);
        op(upper | l3);
    }
};

template <typename Map>
auto game_of_life(std::string_view name, size_t nsteps, Map map1, std::vector<vec2> state) -> size_t {
    auto before = std::chrono::steady_clock::now();
    map1.clear();
    auto map2 = map1; // copy the empty map so we get the allocator

    for (auto& v : state) {
        v = v.add_x(UINT16_MAX / 2).add_y(UINT16_MAX / 2);
        map1[v.pack()] = true;
        v.for_each_surrounding([&](uint32_t xy) {
            map1.emplace(xy, false);
        });
    }

    auto* m1 = &map1;
    auto* m2 = &map2;
    for (size_t i = 0; i < nsteps; ++i) {
        for (auto const& kv : *m1) {
            auto const& pos = kv.first;
            auto alive = kv.second;
            int neighbors = 0;
            vec2{pos}.for_each_surrounding([&](uint32_t xy) {
                if (auto x = m1->find(xy); x != m1->end()) {
                    neighbors += x->second;
                }
            });
            if ((alive && (neighbors == 2 || neighbors == 3)) || (!alive && neighbors == 3)) {
                (*m2)[pos] = true;
                vec2{pos}.for_each_surrounding([&](uint32_t xy) {
                    m2->emplace(xy, false);
                });
            }
        }
        m1->clear();
        std::swap(m1, m2);
    }

    size_t final_population = 0;
    for (auto const& kv : *m1) {
        final_population += kv.second;
    }
    auto after = std::chrono::steady_clock::now();
    fmt::print("{}s {}\n", std::chrono::duration<double>(after - before).count(), name);
    return final_population;
}

TEST_CASE("gameoflife_gotts-dots" * doctest::test_suite("bench") * doctest::skip()) {
    // https://conwaylife.com/wiki/Gotts_dots
    auto state = std::vector<vec2>{
        {0, 0},    {0, 1},    {0, 2},                                                                                 // 1
        {4, 11},   {5, 12},   {6, 13},   {7, 12},   {8, 11},                                                          // 2
        {9, 13},   {9, 14},   {9, 15},                                                                                // 3
        {185, 24}, {186, 25}, {186, 26}, {186, 27}, {185, 27}, {184, 27}, {183, 27}, {182, 26},                       // 4
        {179, 28}, {180, 29}, {181, 29}, {179, 30},                                                                   // 5
        {182, 32}, {183, 31}, {184, 31}, {185, 31}, {186, 31}, {186, 32}, {186, 33}, {185, 34},                       // 6
        {175, 35}, {176, 36}, {170, 37}, {176, 37}, {171, 38}, {172, 38}, {173, 38}, {174, 38}, {175, 38}, {176, 38}, // 7
    };

    // size_t nsteps = 200;
    // size_t nsteps = 2000;
    size_t nsteps = 4000;
    // size_t nsteps = 10000;

    auto pop = size_t();
    {
        using map_t = ankerl::unordered_dense::map<uint32_t, bool>;
        pop = game_of_life("ankerl::unordered_dense::map", nsteps, map_t(), state);
    }
    {
        using map_t = ankerl::unordered_dense::segmented_map<uint32_t, bool>;
        auto new_pop = game_of_life("ankerl::unordered_dense::segmented_map", nsteps, map_t(), state);
        REQUIRE(pop == new_pop);
    }

#if HAS_BOOST_UNORDERED_FLAT_MAP
    {
        using map_t = boost::unordered_flat_map<uint32_t, bool>;
        auto new_pop = game_of_life("boost::unordered_flat_map", nsteps, map_t(), state);
        REQUIRE(pop == new_pop);
    }
#endif

#if HAS_ABSL()
    {
        using map_t = absl::flat_hash_map<uint32_t, bool>;
        auto new_pop = game_of_life("absl::flat_hash_map", nsteps, map_t(), state);
        REQUIRE(pop == new_pop);
    }
#endif

    {
        using map_t = std::unordered_map<uint32_t, bool>;
        auto new_pop = game_of_life("std::unordered_map", nsteps, map_t(), state);
        REQUIRE(pop == new_pop);
    }
}
