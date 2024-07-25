#include <ankerl/unordered_dense.h>

#include <app/counter.h>
#include <app/doctest.h>

#include <cstddef>       // for size_t
#include <cstdint>       // for uint64_t
#include <unordered_set> // for unordered_set
#include <utility>       // for pair
#include <vector>        // for vector

TEST_CASE_MAP("iterators_erase", counter::obj, counter::obj) {
    auto counts = counter();
    INFO(counts);
    {
        counts("begin");
        auto map = map_t();
        for (size_t i = 0; i < 100; ++i) {
            map[counter::obj(i * 101, counts)] = counter::obj(i * 101, counts);
        }

        auto it = map.find(counter::obj(size_t{20} * 101, counts));
        REQUIRE(map.size() == 100);
        REQUIRE(map.end() != map.find(counter::obj(size_t{20} * 101, counts)));
        it = map.erase(it);
        REQUIRE(map.size() == 99);
        REQUIRE(map.end() == map.find(counter::obj(size_t{20} * 101, counts)));

        it = map.begin();
        size_t current_size = map.size();
        std::unordered_set<uint64_t> keys;
        while (it != map.end()) {
            REQUIRE(keys.emplace(it->first.get()).second);
            it = map.erase(it);
            current_size--;
            REQUIRE(map.size() == current_size);
        }
        REQUIRE(map.size() == static_cast<size_t>(0));
        counts("done");
    }
    counts("destructed");
    REQUIRE(counts.dtor() ==
            counts.ctor() + counts.static_default_ctor + counts.copy_ctor() + counts.default_ctor() + counts.move_ctor());
}
