#include <ankerl/unordered_dense.h>

#include <app/checksum.h>
#include <app/counter.h>
#include <app/doctest.h>

#include <third-party/nanobench.h>

#include <cstddef> // for size_t
#include <cstdint> // for uint64_t
#include <utility> // for move

template <typename Map>
void test() {
    auto counts = counter();
    INFO(counts);

    auto rng = ankerl::nanobench::Rng();
    for (size_t trial = 0; trial < 4; ++trial) {
        {
            counts("start");
            auto maps = Map();
            for (size_t i = 0; i < 100; ++i) {
                auto a = rng.bounded(20);
                auto b = rng.bounded(20);
                auto x = static_cast<size_t>(rng());
                // std::cout << i << ": map[" << a << "][" << b << "] = " << x << std::endl;
                maps[counter::obj(a, counts)][counter::obj(b, counts)] = counter::obj(x, counts);
            }
            counts("filled");

            Map maps_copied;
            maps_copied = maps;
            REQUIRE(checksum::mapmap(maps_copied) == checksum::mapmap(maps));
            REQUIRE(maps_copied == maps);
            counts("copied");

            Map maps_moved;
            maps_moved = std::move(maps_copied);
            counts("moved");

            // move
            REQUIRE(checksum::mapmap(maps_moved) == checksum::mapmap(maps));
            REQUIRE(maps_copied.size() == 0); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
            maps_copied = std::move(maps_moved);
            counts("moved back");

            // move back
            REQUIRE(checksum::mapmap(maps_copied) == checksum::mapmap(maps));
            REQUIRE(maps_moved.size() == 0); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
            counts("done");
        }
        counts("all destructed");
        REQUIRE(counts.dtor() ==
                counts.ctor() + counts.static_default_ctor + counts.copy_ctor() + counts.default_ctor() + counts.move_ctor());
    }
}

TYPE_TO_STRING_MAP(counter::obj, ankerl::unordered_dense::map<counter::obj, counter::obj>);
TYPE_TO_STRING_MAP(counter::obj, ankerl::unordered_dense::segmented_map<counter::obj, counter::obj>);

TEST_CASE_MAP("mapmap_map", counter::obj, ankerl::unordered_dense::map<counter::obj, counter::obj>) {
    test<map_t>();
}

TEST_CASE_MAP("mapmap_segmented_map", counter::obj, ankerl::unordered_dense::segmented_map<counter::obj, counter::obj>) {
    test<map_t>();
}
