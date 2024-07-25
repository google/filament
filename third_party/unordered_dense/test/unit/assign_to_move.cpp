#include <ankerl/unordered_dense.h>

#define ENABLE_LOG_LINE
#include <app/print.h>

#include <app/doctest.h>

#include <type_traits> // for remove_reference, remove_referen...
#include <utility>     // for move
#include <vector>      // for vector

TEST_CASE_MAP("assign_to_moved", int, int) {
    auto a = map_t();
    a[1] = 2;
    auto moved = std::move(a);
    REQUIRE(moved.size() == 1U);

    auto c = map_t();
    c[3] = 4;

    // assign to a moved map
    a = c;
}

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

TEST_CASE_MAP("swap", int, int) {
    {
        auto b = map_t();
        {
            auto a = map_t();
            b[1] = 2;

            a.swap(b);
            REQUIRE(a.end() != a.find(1));
            REQUIRE(b.end() == b.find(1));
        }
        REQUIRE(b.end() == b.find(1));
        b[2] = 3;
        REQUIRE(b.end() != b.find(2));
        REQUIRE(b.size() == 1);
    }

    {
        auto a = map_t();
        {
            auto b = map_t();
            b[1] = 2;

            a.swap(b);
            REQUIRE(a.end() != a.find(1));
            REQUIRE(b.end() == b.find(1));
        }
        REQUIRE(a.end() != a.find(1));
        a[2] = 3;
        REQUIRE(a.end() != a.find(2));
        REQUIRE(a.size() == 2);
    }

    {
        auto a = map_t();
        {
            auto b = map_t();
            a.swap(b);
            REQUIRE(a.end() == a.find(1));
            REQUIRE(b.end() == b.find(1));
        }
        REQUIRE(a.end() == a.find(1));
    }
}
