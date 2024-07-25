#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <string> // for operator==, basic_string, operat...
#include <vector> // for vector

TEST_CASE_SET("set", std::string) {
    auto set = set_t();

    set.insert("asdf");
    REQUIRE(set.size() == 1);
    auto it = set.find("asdf");
    REQUIRE(it != set.end());
    REQUIRE(*it == "asdf");
}
