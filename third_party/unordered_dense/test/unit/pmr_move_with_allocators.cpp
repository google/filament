#include <ankerl/unordered_dense.h>

#define ENABLE_LOG_LINE
#include <app/doctest.h>
#include <app/print.h>

#if defined(ANKERL_UNORDERED_DENSE_PMR)
// windows' vector has different allocation behavior, macos has linker errors
#    if __linux__

namespace {

// creates a map and moves it out
template <typename Map>
auto return_hello_world(ANKERL_UNORDERED_DENSE_PMR::memory_resource* resource) -> Map {
    Map map_default_resource(resource);
    map_default_resource[0] = "Hello";
    map_default_resource[1] = "World";
    return map_default_resource;
}

template <typename Map>
auto doTest() {
    ANKERL_UNORDERED_DENSE_PMR::synchronized_pool_resource pool;

    {
        Map m(ANKERL_UNORDERED_DENSE_PMR::new_delete_resource());
        m = return_hello_world<Map>(&pool);
        REQUIRE(m.contains(0));
    }

    {
        Map m(ANKERL_UNORDERED_DENSE_PMR::new_delete_resource());
        m[0] = "foo";
        m = return_hello_world<Map>(&pool);
        REQUIRE(m[0] == "Hello");
    }

    {
        Map m(return_hello_world<Map>(&pool), ANKERL_UNORDERED_DENSE_PMR::new_delete_resource());
        REQUIRE(m.contains(0));
    }

    {
        Map a(ANKERL_UNORDERED_DENSE_PMR::new_delete_resource());
        a[0] = "hello";
        Map b(&pool);
        b[0] = "world";

        // looping here causes lots of memory held up
        // in the resources
        for (int i = 0; i < 100; ++i) {
            std::swap(a, b);
            REQUIRE(b[0] == "hello");
            REQUIRE(a[0] == "world");

            std::swap(a, b);
            REQUIRE(a[0] == "hello");
            REQUIRE(b[0] == "world");
        }
    }

    {
        Map a(ANKERL_UNORDERED_DENSE_PMR::new_delete_resource());
        a[0] = "hello";
        Map b(&pool);
        b[0] = "world";

        // looping here causes lots of memory held up
        // in the resources
        for (int i = 0; i < 100; ++i) {
            std::swap(a, b);
            REQUIRE(b[0] == "hello");
            REQUIRE(a[0] == "world");

            std::swap(a, b);
            REQUIRE(a[0] == "hello");
            REQUIRE(b[0] == "world");
        }
    }

    {
        Map a(&pool);
        a[0] = "world";

        Map tmp(ANKERL_UNORDERED_DENSE_PMR::new_delete_resource());
        tmp[0] = "nope";
        tmp = std::move(a);
        REQUIRE(tmp[0] == "world");
        REQUIRE(a.empty());
        a[0] = "hey";
        REQUIRE(a.size() == 1);
        REQUIRE(a[0] == "hey");
    }
}

TEST_CASE("move_with_allocators") {
    doTest<ankerl::unordered_dense::pmr::map<int, std::string>>();
}

TEST_CASE("move_with_allocators_segmented") {
    doTest<ankerl::unordered_dense::pmr::segmented_map<int, std::string>>();
}

} // namespace

#    endif
#endif