#include <ankerl/unordered_dense.h>

#include <doctest.h>

#include <cstddef> // for size_t
#include <string>  // for allocator, string, operator==
#include <utility> // for pair, move
#include <vector>  // for vector

namespace {

struct foo {
    uint64_t m_i;
};

} // namespace

template <>
struct std::hash<foo> {
    auto operator()(foo const& foo) const noexcept {
        return static_cast<size_t>(foo.m_i + 1);
    }
};

TEST_CASE("std_hash") {
    auto f = foo{12345};
    REQUIRE(std::hash<foo>{}(f) == 12346U);
    // unordered_dense::hash blows that up to 64bit!

    // Just wraps std::hash
    REQUIRE(ankerl::unordered_dense::hash<foo>{}(f) == UINT64_C(12346));
    REQUIRE(ankerl::unordered_dense::hash<uint64_t>{}(12346U) == UINT64_C(0x3F645BE4CE24110C));
}
