#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t
#include <string>  // for allocator, string, operator==
#include <utility> // for pair, move
#include <vector>  // for vector

struct regular_type {
    // cppcheck-suppress passedByValue
    regular_type(std::size_t i, std::string s) noexcept
        : m_s(std::move(s))
        , m_i(i) {}

    friend auto operator==(const regular_type& r1, const regular_type& r2) -> bool {
        return r1.m_i == r2.m_i && r1.m_s == r2.m_s;
    }

private:
    std::string m_s;
    std::size_t m_i;
};

TYPE_TO_STRING_MAP(std::string, regular_type);

TEST_CASE_MAP("try_emplace", std::string, regular_type) {
    map_t map;
    auto ret = map.try_emplace("a", 1U, "b");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(ret.first->second == regular_type(1U, "b"));
    REQUIRE(map.size() == 1);

    ret = map.try_emplace("c", 2U, "d");
    REQUIRE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(ret.first->second == regular_type(2U, "d"));
    REQUIRE(map.size() == 2U);

    std::string key = "c";
    ret = map.try_emplace(key, 3U, "dd");
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("c"));
    REQUIRE(ret.first->second == regular_type(2U, "d"));
    REQUIRE(map.size() == 2U);

    key = "a";
    regular_type value(3U, "dd");
    ret = map.try_emplace(key, value);
    REQUIRE_FALSE(ret.second);
    REQUIRE(ret.first == map.find("a"));
    REQUIRE(ret.first->second == regular_type(1U, "b"));
    REQUIRE(map.size() == 2U);

    auto pos = map.try_emplace(map.end(), "e", 67U, "f");
    REQUIRE(pos == map.find("e"));
    REQUIRE(pos->second == regular_type(67U, "f"));
    REQUIRE(map.size() == 3U);

    key = "e";
    regular_type value2(66U, "ff");
    pos = map.try_emplace(map.begin(), key, value2);
    REQUIRE(pos == map.find("e"));
    REQUIRE(pos->second == regular_type(67U, "f"));
    REQUIRE(map.size() == 3U);
}
