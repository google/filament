#include <ankerl/unordered_dense.h>

#define ENABLE_LOG_LINE
#include <app/doctest.h>
#include <app/print.h>

#include <type_traits> // for remove_reference, remove_referen...
#include <utility>     // for move

TEST_CASE_MAP("move_to_moved", int, int) {
    auto a = map_t();
    a[1] = 2;
    auto moved = std::move(a);

    auto c = map_t();
    c[3] = 4;

    // assign to a moved map
    a = std::move(c);

    a[5] = 6;
    moved[6] = 7;
    REQUIRE(moved[6] == 7);
}
