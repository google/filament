#if defined(__has_include)
#    if __has_include(<Windows.h>)
#        include <Windows.h>
#        pragma message("Windows.h included")
#    endif
#endif

#include <ankerl/unordered_dense.h>
#include <app/counter.h>

#include <doctest.h>

#include <initializer_list>

TEST_CASE("unordered_dense_with_windows_h") {
    auto counts = counter();
    using map_t = ankerl::unordered_dense::map<counter::obj, counter::obj>;
    auto map = map_t();

    {
        auto key = size_t{1};
        auto it = map.try_emplace(counter::obj(key, counts), counter::obj(key, counts)).first;
        REQUIRE(it != map.end());
        REQUIRE(it->first.get() == key);
    }
    {
        auto key = size_t{2};
        map.emplace(std::piecewise_construct, std::forward_as_tuple(key, counts), std::forward_as_tuple(key + 77, counts));
    }
    {
        auto key = size_t{3};
        map[counter::obj(key, counts)] = counter::obj(key + 123, counts);
    }
    {
        auto key = size_t{4};
        map.insert(std::pair<counter::obj, counter::obj>(counter::obj(key, counts), counter::obj(key, counts)));
    }
    {
        auto key = size_t{5};
        map.insert_or_assign(counter::obj(key, counts), counter::obj(key + 1, counts));
    }
    {
        auto key = size_t{6};
        map.erase(counter::obj(key, counts));
    }
    { map = map_t{}; }
    {
        auto m = map_t{};
        m.swap(map);
    }
    { map.clear(); }
    {
        auto s = size_t{7};
        map.rehash(s);
    }
    {
        auto s = size_t{8};
        map.reserve(s);
    }
    {
        auto key = size_t{9};
        auto it = map.find(counter::obj(key, counts));
        auto d = std::distance(map.begin(), it);
        REQUIRE(0 <= d);
        REQUIRE(d <= static_cast<std::ptrdiff_t>(map.size()));
    }
    {
        if (!map.empty()) {
            auto idx = static_cast<int>(map.size() / 2);
            auto it = map.cbegin() + idx;
            auto const& key = it->first;
            auto found_it = map.find(key);
            REQUIRE(it == found_it);
        }
    }
    {
        if (!map.empty()) {
            auto it = map.begin() + static_cast<int>(map.size() / 2);
            map.erase(it);
        }
    }
    {
        auto tmp = map_t();
        std::swap(tmp, map);
    }
    {
        map = std::initializer_list<std::pair<counter::obj, counter::obj>>{
            {{1, counts}, {2, counts}},
            {{3, counts}, {4, counts}},
            {{5, counts}, {6, counts}},
        };
        REQUIRE(map.size() == 3);
    }
    {
        auto first_idx = 0;
        auto last_idx = 0;
        if (!map.empty()) {
            first_idx = static_cast<int>(map.size() / 2);
            last_idx = static_cast<int>(map.size() / 2);
            if (first_idx > last_idx) {
                std::swap(first_idx, last_idx);
            }
        }
        map.erase(map.cbegin() + first_idx, map.cbegin() + last_idx);
    }
    {
        map.~map_t();
        counts.check_all_done();
        new (&map) map_t();
    }
    {
        bool b = true;
        std::erase_if(map, [&](map_t::value_type const& /*v*/) {
            b = !b;
            return b;
        });
    }
}
