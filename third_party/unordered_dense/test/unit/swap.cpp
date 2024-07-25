#include <ankerl/unordered_dense.h>

#define ENABLE_LOG_LINE
#include <app/doctest.h>
#include <app/print.h>

#include <vector>

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
