#include <ankerl/unordered_dense.h>

#include <app/counter.h>
#include <app/doctest.h>
#include <third-party/nanobench.h>

#include <cstddef> // for size_t
#include <vector>  // for vector

TEST_CASE_MAP("erase_range", counter::obj, counter::obj) {
    int const num_elements = 10;

    for (int first_pos = 0; first_pos <= num_elements; ++first_pos) {
        for (int last_pos = first_pos; last_pos <= num_elements; ++last_pos) {
            auto counts = counter();
            INFO(counts);

            auto map = map_t();

            for (size_t i = 0; i < num_elements; ++i) {
                auto key = i;
                auto val = i * 1000;
                map.try_emplace({key, counts}, val, counts);
            }
            REQUIRE(map.size() == num_elements);

            auto it_ret = map.erase(map.cbegin() + first_pos, map.cbegin() + last_pos);
            REQUIRE(map.size() == static_cast<size_t>(num_elements - (last_pos - first_pos)));
            REQUIRE(it_ret == map.begin() + first_pos);
        }
    }
}
