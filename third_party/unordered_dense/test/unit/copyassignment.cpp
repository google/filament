#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <string>

TEST_CASE_MAP("copyassignment", std::string, std::string) {
    auto map = map_t();
    auto tmp = map_t();

    map.emplace("a", "b");
    map = tmp;
    map.emplace("c", "d");

    REQUIRE(map.size() == 1);
    REQUIRE(map["c"] == "d");
    REQUIRE(map.size() == 1);

    REQUIRE(tmp.size() == 0);

    map["e"] = "f";
    REQUIRE(map.size() == 2);
    REQUIRE(tmp.size() == 0);

    tmp["g"] = "h";
    REQUIRE(map.size() == 2);
    REQUIRE(tmp.size() == 1);
}
