#include <ankerl/unordered_dense.h>
#include <app/doctest.h>
#include <fuzz/provider.h>
#include <fuzz/run.h>

#include <unordered_map>

template <typename Map>
void do_fuzz_api(fuzz::provider p) {
    auto counts = counter();
    auto map = Map();
    p.repeat_oneof(
        [&] {
            auto key = p.integral<size_t>();
            auto it = map.try_emplace(counter::obj(key, counts), counter::obj(key, counts)).first;
            REQUIRE(it != map.end());
            REQUIRE(it->first.get() == key);
        },
        [&] {
            auto key = p.integral<size_t>();
            map.emplace(std::piecewise_construct, std::forward_as_tuple(key, counts), std::forward_as_tuple(key + 77, counts));
        },
        [&] {
            auto key = p.integral<size_t>();
            map[counter::obj(key, counts)] = counter::obj(key + 123, counts);
        },
        [&] {
            auto key = p.integral<size_t>();
            map.insert(std::pair<counter::obj, counter::obj>(counter::obj(key, counts), counter::obj(key, counts)));
        },
        [&] {
            auto key = p.integral<size_t>();
            map.insert_or_assign(counter::obj(key, counts), counter::obj(key + 1, counts));
        },
        [&] {
            auto key = p.integral<size_t>();
            map.erase(counter::obj(key, counts));
        },
        [&] {
            map = Map{};
        },
        [&] {
            auto m = Map{};
            m.swap(map);
        },
        [&] {
            map.clear();
        },
        [&] {
            auto s = p.bounded<size_t>(1025);
            map.rehash(s);
        },
        [&] {
            auto s = p.bounded<size_t>(1025);
            map.reserve(s);
        },
        [&] {
            auto key = p.integral<size_t>();
            auto it = map.find(counter::obj(key, counts));
            auto d = std::distance(map.begin(), it);
            REQUIRE(0 <= d);
            REQUIRE(d <= static_cast<std::ptrdiff_t>(map.size()));
        },
        [&] {
            if (!map.empty()) {
                auto idx = p.bounded(static_cast<int>(map.size()));
                auto it = map.cbegin() + idx;
                auto const& key = it->first;
                auto found_it = map.find(key);
                REQUIRE(it == found_it);
            }
        },
        [&] {
            if (!map.empty()) {
                auto it = map.begin() + p.bounded(static_cast<int>(map.size()));
                map.erase(it);
            }
        },
        [&] {
            auto tmp = Map();
            std::swap(tmp, map);
        },
        [&] {
            map = std::initializer_list<std::pair<counter::obj, counter::obj>>{
                {{1, counts}, {2, counts}},
                {{3, counts}, {4, counts}},
                {{5, counts}, {6, counts}},
            };
            REQUIRE(map.size() == 3);
        },
        [&] {
            auto first_idx = 0;
            auto last_idx = 0;
            if (!map.empty()) {
                first_idx = p.bounded(static_cast<int>(map.size()));
                last_idx = p.bounded(static_cast<int>(map.size()));
                if (first_idx > last_idx) {
                    std::swap(first_idx, last_idx);
                }
            }
            map.erase(map.cbegin() + first_idx, map.cbegin() + last_idx);
        },
        [&] {
            map.~Map();
            counts.check_all_done();
            new (&map) Map();
        },
        [&] {
            std::erase_if(map, [&](typename Map::value_type const& /*v*/) {
                return p.integral<bool>();
            });
        });
}

TEST_CASE("fuzz_api" * doctest::test_suite("fuzz")) {
    fuzz::run([](fuzz::provider p) {
        do_fuzz_api<ankerl::unordered_dense::map<counter::obj, counter::obj>>(p.copy());
        do_fuzz_api<ankerl::unordered_dense::segmented_map<counter::obj, counter::obj>>(p.copy());
        do_fuzz_api<deque_map<counter::obj, counter::obj>>(p.copy());
    });
}
