#include <ankerl/unordered_dense.h>

#define ENABLE_LOG_LINE
#include <app/doctest.h>
#include <app/print.h>

#include <cstdint> // for uint64_t
#include <utility> // for pair
#include <vector>  // for vector

TEST_CASE_MAP("assignment_combinations_1", uint64_t, uint64_t) {
    map_t a;
    map_t b;
    b = a;
    REQUIRE(b == a);
}

TEST_CASE_MAP("assignment_combinations_2", uint64_t, uint64_t) {
    map_t a;
    map_t const& a_const = a;
    map_t b;
    a[123] = 321;
    b = a;

    REQUIRE(a.find(123)->second == 321);
    REQUIRE(a_const.find(123)->second == 321);

    REQUIRE(b.find(123)->second == 321);
    a[123] = 111;
    REQUIRE(a.find(123)->second == 111);
    REQUIRE(a_const.find(123)->second == 111);
    REQUIRE(b.find(123)->second == 321);
    b[123] = 222;
    REQUIRE(a.find(123)->second == 111);
    REQUIRE(a_const.find(123)->second == 111);
    REQUIRE(b.find(123)->second == 222);
}

TEST_CASE_MAP("assignment_combinations_3", uint64_t, uint64_t) {
    map_t a;
    map_t b;
    a[123] = 321;
    a.clear();
    b = a;

    REQUIRE(a.size() == 0);
    REQUIRE(b.size() == 0);
}

TEST_CASE_MAP("assignment_combinations_4", uint64_t, uint64_t) {
    map_t a;
    map_t b;
    b[123] = 321;
    b = a;

    REQUIRE(a.size() == 0);
    REQUIRE(b.size() == 0);
}

TEST_CASE_MAP("assignment_combinations_5", uint64_t, uint64_t) {
    map_t a;
    map_t b;
    b[123] = 321;
    b.clear();
    b = a;

    REQUIRE(a.size() == 0);
    REQUIRE(b.size() == 0);
}

TEST_CASE_MAP("assignment_combinations_6", uint64_t, uint64_t) {
    map_t a;
    a[1] = 2;
    map_t b;
    b[3] = 4;
    b = a;

    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    REQUIRE(b.find(1)->second == 2);
    a[1] = 123;
    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    REQUIRE(b.find(1)->second == 2);
}

TEST_CASE_MAP("assignment_combinations_7", uint64_t, uint64_t) {
    map_t a;
    a[1] = 2;
    a.clear();
    map_t b;
    REQUIRE(a == b);
    b[3] = 4;
    REQUIRE(a != b);
    b = a;
    REQUIRE(a == b);
}

TEST_CASE_MAP("assignment_combinations_7", uint64_t, uint64_t) {
    map_t a;
    a[1] = 2;
    map_t b;
    REQUIRE(a != b);
    b[3] = 4;
    b.clear();
    REQUIRE(a != b);
    b = a;
    REQUIRE(a == b);
}

TEST_CASE_MAP("assignment_combinations_8", uint64_t, uint64_t) {
    map_t a;
    a[1] = 2;
    a.clear();
    map_t b;
    b[3] = 4;
    REQUIRE(a != b);
    b.clear();
    REQUIRE(a == b);
    b = a;
    REQUIRE(a == b);
}

TEST_CASE_MAP("assignment_combinations_9", uint64_t, uint64_t) {
    map_t a;
    a[1] = 2;

    // self assignment should work too
    map_t* b = &a;
    a = *b;
    REQUIRE(a == a);
    REQUIRE(a.size() == 1);
    REQUIRE(a.find(1) != a.end());
}

TEST_CASE_MAP("assignment_combinations_10", uint64_t, uint64_t) {
    map_t a;
    a[1] = 2;
    map_t b;
    b[2] = 1;

    // maps have the same number of elements, but are not equal.
    REQUIRE(!(a == b));
    REQUIRE(a != b);
    REQUIRE(b != a);
    REQUIRE(!(a == b));
    REQUIRE(!(b == a));

    map_t c;
    c[1] = 3;
    REQUIRE(a != c);
    REQUIRE(c != a);
    REQUIRE(!(a == c));
    REQUIRE(!(c == a));

    b.clear();
    REQUIRE(a != b);
    REQUIRE(b != a);
    REQUIRE(!(a == b));
    REQUIRE(!(b == a));

    map_t empty;
    REQUIRE(b == empty);
    REQUIRE(empty == b);
    REQUIRE(!(b != empty));
    REQUIRE(!(empty != b));
}
