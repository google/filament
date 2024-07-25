#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef>     // for size_t
#include <cstdint>     // for UINT64_C, uint64_t
#include <string>      // for basic_string, allocator, operator==
#include <type_traits> // for integral_constant<>::value, is_same
#include <vector>      // for vector

TEST_CASE("unordered_set_asserts") {
    using set1_t = ankerl::unordered_dense::set<uint64_t>;
    static_assert(std::is_same<typename set1_t::key_type, uint64_t>::value, "key_type same");
    static_assert(std::is_same<typename set1_t::value_type, uint64_t>::value, "value_type same");

    using set2_t = ankerl::unordered_dense::segmented_set<uint64_t>;
    static_assert(std::is_same<typename set2_t::key_type, uint64_t>::value, "key_type same");
    static_assert(std::is_same<typename set2_t::value_type, uint64_t>::value, "value_type same");
}

TEST_CASE_SET("unordered_set", uint64_t) {
    set_t set;
    set.emplace(UINT64_C(123));
    REQUIRE(set.size() == 1U);

    set.insert(UINT64_C(333));
    REQUIRE(set.size() == 2U);

    set.erase(UINT64_C(222));
    REQUIRE(set.size() == 2U);

    set.erase(UINT64_C(123));
    REQUIRE(set.size() == 1U);
}

TEST_CASE_SET("unordered_set_string", std::string) {
    set_t set;
    REQUIRE(set.begin() == set.end());

    set.emplace(static_cast<size_t>(2000), 'a');
    REQUIRE(set.size() == 1);

    REQUIRE(set.begin() != set.end());
    std::string const& str = *set.begin();
    REQUIRE(str == std::string(static_cast<size_t>(2000), 'a'));

    auto it = set.begin();
    REQUIRE(++it == set.end());
}

TEST_CASE_SET("unordered_set_eq", std::string) {
    set_t set1;
    set_t set2;
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);

    set1.emplace("asdf");
    // (asdf) == ()
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.emplace("huh");
    // (asdf) == (huh)
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set1.emplace("huh");
    // (asdf, huh) == (huh)
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.emplace("asdf");
    // (asdf, huh) == (asdf, huh)
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);

    set1.erase("asdf");
    // (huh) == (asdf, huh)
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.erase("asdf");
    // (huh) == (huh)
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);

    set1.clear();
    // () == (huh)
    REQUIRE(set1.size() != set2.size());
    REQUIRE(set1 != set2);
    REQUIRE(set2 != set1);

    set2.erase("huh");
    // () == ()
    REQUIRE(set1.size() == set2.size());
    REQUIRE(set1 == set2);
    REQUIRE(set2 == set1);
}
