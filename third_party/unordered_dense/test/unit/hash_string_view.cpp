#include <ankerl/unordered_dense.h>

#include <doctest.h>

#include <string>
#include <string_view>

TEST_CASE("hash_string_view") {
    auto const* cstr = "The ships hung in the sky in much the same way that bricks don't.";
    REQUIRE(ankerl::unordered_dense::hash<std::string>{}(std::string{cstr}) ==
            ankerl::unordered_dense::hash<std::string_view>{}(std::string_view{cstr}));
}

TEST_CASE("unit_hash_u32string") {
    auto str = std::u32string{};
    str.push_back(1);
    str.push_back(2);
    str.push_back(3);
    str.push_back(4);
    str.push_back(5);

    REQUIRE(ankerl::unordered_dense::hash<std::u32string>{}(str) == ankerl::unordered_dense::hash<std::u32string_view>{}(str));
    REQUIRE(ankerl::unordered_dense::hash<std::u32string>{}(str) != std::hash<std::u32string>{}(str));
}
