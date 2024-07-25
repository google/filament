#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <string>  // for string, operator==, allocator
#include <utility> // for pair
#include <vector>  // for vector

TEST_CASE_MAP("insert_or_assign", std::string, std::string) {
    auto map = map_t();

    auto ret = map.insert_or_assign("a", "b");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(map.size() == 1);
    REQUIRE(map["a"] == "b");

    ret = map.insert_or_assign("c", "d");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(map.size() == 2);
    REQUIRE(map["c"] == "d");

    ret = map.insert_or_assign("c", "dd");
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(map.size() == 2);
    REQUIRE(map["c"] == "dd");

    std::string key = "a";
    std::string value = "bb";
    ret = map.insert_or_assign(key, value);
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(map.size() == 2);
    REQUIRE(map["a"] == "bb");

    key = "e";
    value = "f";
    auto pos = map.insert_or_assign(map.end(), key, value);
    REQUIRE(pos == map.find("e"));
    REQUIRE(map.size() == 3);
    REQUIRE(map["e"] == "f");

    pos = map.insert_or_assign(map.begin(), "e", "ff");
    REQUIRE(pos == map.find("e"));
    REQUIRE(map.size() == 3);
    REQUIRE(map["e"] == "ff");
}
