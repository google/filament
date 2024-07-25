#include <ankerl/unordered_dense.h>

#include <app/doctest.h>
#include <app/name_of_type.h>

#include <fmt/format.h>

#include <array>         // for array
#include <cstddef>       // for size_t
#include <functional>    // for equal_to
#include <string>        // for basic_string, string, operator""s
#include <string_view>   // for basic_string_view, operator""sv
#include <type_traits>   // for add_const_t
#include <unordered_map> // for unordered_map
#include <utility>       // for pair, as_const
#include <vector>        // for vector

using namespace std::literals;

class string_eq {
    mutable std::unordered_map<std::string, size_t> m_names_to_counts{};

public:
    using is_transparent = void;

    template <class T, class U>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    auto operator()(T&& lhs, U&& rhs) const -> decltype(std::forward<T>(lhs) == std::forward<U>(rhs)) {
        auto names = fmt::format("{} -> {}", name_of_type<T>(), name_of_type<U>());
        ++m_names_to_counts[names];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
        return std::forward<T>(lhs) == std::forward<U>(rhs);
    }

    auto counts() const -> std::unordered_map<std::string, size_t> const& {
        return m_names_to_counts;
    }

    string_eq() = default;
    ~string_eq() = default;

    string_eq(string_eq const&) = default;
    auto operator=(string_eq const&) -> string_eq& = default;

    string_eq(string_eq&&) = delete;
    auto operator=(string_eq&&) -> string_eq& = delete;
};

// transparent hash, counts number of calls per operator
class string_hash {
    mutable size_t m_num_charstar{};
    mutable size_t m_num_stringview{};
    mutable size_t m_num_string{};

public:
    using hash_type = ankerl::unordered_dense::hash<std::string_view>;
    using is_transparent = void;
    using is_avalanching = void;

    [[nodiscard]] auto operator()(const char* str) const noexcept -> uint64_t {
        ++m_num_charstar;
        return hash_type{}(str);
    }

    [[nodiscard]] auto operator()(std::string_view str) const noexcept -> uint64_t {
        ++m_num_stringview;
        return hash_type{}(str);
    }

    [[nodiscard]] auto operator()(std::string const& str) const noexcept -> uint64_t {
        ++m_num_string;
        return hash_type{}(str);
    }

    auto counts() const -> std::array<size_t, 3> {
        return {m_num_charstar, m_num_stringview, m_num_string};
    }
};

namespace docs {

struct string_hash {
    using is_transparent = void; // enable heterogeneous lookup
    using is_avalanching = void; // mark class as high quality avalanching hash

    [[nodiscard]] auto operator()(const char* str) const noexcept -> uint64_t {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }

    [[nodiscard]] auto operator()(std::string_view str) const noexcept -> uint64_t {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }

    [[nodiscard]] auto operator()(std::string const& str) const noexcept -> uint64_t {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }
};

} // namespace docs

template <typename C>
void check(int line, C const& container, size_t num_charstar, size_t num_stringview, size_t num_string) {
    auto sh = container.hash_function();
    auto counts = sh.counts();
    INFO("check line " << line << ": expect (" << num_charstar << ", " << num_stringview << ", " << num_string << ") got ("
                       << counts[0] << ", " << counts[1] << ", " << counts[2]
                       << ") for (num_charstar, num_stringview, num_string)");
    REQUIRE(counts[0] == num_charstar);
    REQUIRE(counts[1] == num_stringview);
    REQUIRE(counts[2] == num_string);
}

TYPE_TO_STRING_MAP(std::string, size_t, string_hash, std::equal_to<>);

TEST_CASE_MAP("transparent_find", std::string, size_t, string_hash, std::equal_to<>) {
    auto map = map_t();
    map.try_emplace("hello", 1);
    check(__LINE__, map, 1, 0, 0);

    auto it = map.find("huh");
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(it == map.end());
    it = map.find("hello");
    check(__LINE__, map, 3, 0, 0);
    REQUIRE(it != map.end());

    auto cit = std::as_const(map).find("huh");
    check(__LINE__, map, 4, 0, 0);
    REQUIRE(cit == map.end());
    REQUIRE(cit == map.cend());
    cit = std::as_const(map).find("hello");
    check(__LINE__, map, 5, 0, 0);
    REQUIRE(cit != map.end());

    // string_view
    it = map.find("huh"sv);
    REQUIRE(it == map.end());
    check(__LINE__, map, 5, 1, 0);
    it = map.find("hello"sv);
    REQUIRE(it != map.end());
    check(__LINE__, map, 5, 2, 0);

    // string
    it = map.find("huh"s);
    REQUIRE(it == map.end());
    check(__LINE__, map, 5, 2, 1);
    it = map.find("hello"s);
    REQUIRE(it != map.end());
    check(__LINE__, map, 5, 2, 2);
}

TEST_CASE_MAP("transparent_count", std::string, size_t, string_hash, std::equal_to<>) {
    auto map = map_t();
    map.try_emplace("hello", 1);
    check(__LINE__, map, 1, 0, 0);

    REQUIRE(0 == map.count("huh"));
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(1 == map.count("hello"));
    check(__LINE__, map, 3, 0, 0);

    REQUIRE(0 == map.count("huh"sv));
    check(__LINE__, map, 3, 1, 0);
    REQUIRE(1 == map.count("hello"sv));
    check(__LINE__, map, 3, 2, 0);

    REQUIRE(0 == map.count("huh"s));
    check(__LINE__, map, 3, 2, 1);
    REQUIRE(1 == map.count("hello"s));
    check(__LINE__, map, 3, 2, 2);
}

TEST_CASE_MAP("transparent_contains", std::string, size_t, string_hash, std::equal_to<>) {
    auto map = map_t();
    map.try_emplace("hello", 1);
    check(__LINE__, map, 1, 0, 0);

    REQUIRE(!map.contains("huh"));
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(map.contains("hello"));
    check(__LINE__, map, 3, 0, 0);

    REQUIRE(!map.contains("huh"sv));
    check(__LINE__, map, 3, 1, 0);
    REQUIRE(map.contains("hello"sv));
    check(__LINE__, map, 3, 2, 0);

    REQUIRE(!map.contains("huh"s));
    check(__LINE__, map, 3, 2, 1);
    REQUIRE(map.contains("hello"s));
    check(__LINE__, map, 3, 2, 2);
}

TEST_CASE_MAP("transparent_erase", std::string, size_t, string_hash, std::equal_to<>) {
    auto map = map_t();
    map.try_emplace("hello", 1);
    check(__LINE__, map, 1, 0, 0);
    REQUIRE(0 == map.erase("huh"));
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(1 == map.erase("hello"));
    check(__LINE__, map, 3, 0, 0);

    map.try_emplace("hello", 1);
    check(__LINE__, map, 4, 0, 0);
    REQUIRE(0 == map.erase("huh"sv));
    check(__LINE__, map, 4, 1, 0);
    REQUIRE(1 == map.erase("hello"sv));
    check(__LINE__, map, 4, 2, 0);

    map.try_emplace("hello", 1);
    check(__LINE__, map, 5, 2, 0);
    REQUIRE(0 == map.erase("huh"s));
    check(__LINE__, map, 5, 2, 1);
    REQUIRE(1 == map.erase("hello"s));
    check(__LINE__, map, 5, 2, 2);
}

TEST_CASE_MAP("transparent_equal_range", std::string, size_t, string_hash, std::equal_to<>) {
    auto map = map_t();
    map.try_emplace("hello", 1);
    check(__LINE__, map, 1, 0, 0);

    auto range = map.equal_range("hello");
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(range.first != range.second);
    REQUIRE(range.first->first == "hello");
    REQUIRE(range.second == map.end());

    auto crange = std::as_const(map).equal_range("hello"sv);
    check(__LINE__, map, 2, 1, 0);
    REQUIRE(crange.first != range.second);
    REQUIRE(crange.first->first == "hello");
    REQUIRE(crange.second == map.end());
}

TYPE_TO_STRING_MAP(std::string, size_t, string_hash, string_eq);

TEST_CASE_MAP("transparent_string_eq", std::string, size_t, string_hash, string_eq) {
    auto map = map_t();
    map.try_emplace("hello", 1);

    REQUIRE(map.count("hello"));
    REQUIRE(map.count("hello"sv));
    REQUIRE(map.count("hello"s));

    auto ke = map.key_eq();
    REQUIRE(ke.counts().size() == 3);
    for (auto const& kv : ke.counts()) {
        REQUIRE(kv.second == 1U);
    }
}

TEST_CASE_MAP("transparent_at", std::string, size_t, string_hash, string_eq) {
    auto map = map_t();
    map.try_emplace("asdf", 123);
    check(__LINE__, map, 1, 0, 0);

    auto& vt = map.at("asdf");
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(vt == 123);

    // NOLINTNEXTLINE(llvm-else-after-return,readability-else-after-return)
    REQUIRE_THROWS_AS(map.at("nope"sv), std::out_of_range);
    check(__LINE__, map, 2, 1, 0);
}

TYPE_TO_STRING_MAP(std::string, size_t, string_hash);

TEST_CASE_MAP("transparent_at_not", std::string, size_t, string_hash) {
    // no string_eq, so not is_transparent
    auto map = map_t();
    map.try_emplace("asdf", 123);
    check(__LINE__, map, 0, 0, 1);

    auto& vt = map.at("asdf");
    check(__LINE__, map, 0, 0, 2);
    REQUIRE(vt == 123);

    // NOLINTNEXTLINE(llvm-else-after-return,readability-else-after-return)
    REQUIRE_THROWS_AS(map.at("nope"), std::out_of_range);
    check(__LINE__, map, 0, 0, 3);
}

TEST_CASE_MAP("transparent_insert_or_assign", std::string, size_t, string_hash, string_eq) {
    auto map = map_t();
    auto r = map.insert_or_assign("asdf", 123U);
    check(__LINE__, map, 1, 0, 0);
    REQUIRE(r.first->first == "asdf");
    REQUIRE(r.first->second == 123U);
    REQUIRE(r.second);

    r = map.insert_or_assign("asdf", 42U);
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(r.first->first == "asdf");
    REQUIRE(r.first->second == 42U);
    REQUIRE(!r.second);
    REQUIRE(map.size() == 1U);
}

TEST_CASE_MAP("transparent_insert_or_assign_not", std::string, size_t, string_hash) {
    auto map = map_t();
    auto r = map.insert_or_assign("asdf", 123U);
    check(__LINE__, map, 0, 0, 1);
    REQUIRE(r.first->first == "asdf");
    REQUIRE(r.first->second == 123U);
    REQUIRE(r.second);

    r = map.insert_or_assign("asdf", 42U);
    check(__LINE__, map, 0, 0, 2);
    REQUIRE(r.first->first == "asdf");
    REQUIRE(r.first->second == 42U);
    REQUIRE(!r.second);
    REQUIRE(map.size() == 1U);
}

TEST_CASE_MAP("transparent_insert_or_assign_iterator", std::string, size_t, string_hash, string_eq) {
    auto map = map_t();
    auto r = map.insert_or_assign(typename map_t::const_iterator{}, "asdf", 123U);
    check(__LINE__, map, 1, 0, 0);
    REQUIRE(r->first == "asdf");
    REQUIRE(r->second == 123U);

    r = map.insert_or_assign(typename map_t::const_iterator{}, "asdf", 42U);
    check(__LINE__, map, 2, 0, 0);
    REQUIRE(r->first == "asdf");
    REQUIRE(r->second == 42U);
    REQUIRE(map.size() == 1U);
}

TEST_CASE_MAP("transparent_insert_or_assign_iterator_not", std::string, size_t, string_hash) {
    auto map = map_t();
    auto r = map.insert_or_assign(typename map_t::const_iterator{}, "asdf", 123U);
    check(__LINE__, map, 0, 0, 1);
    REQUIRE(r->first == "asdf");
    REQUIRE(r->second == 123U);

    r = map.insert_or_assign(typename map_t::const_iterator{}, "asdf", 42U);
    check(__LINE__, map, 0, 0, 2);
    REQUIRE(r->first == "asdf");
    REQUIRE(r->second == 42U);
    REQUIRE(map.size() == 1U);
}

TYPE_TO_STRING_SET(std::string, string_hash, string_eq);

// insert() transparent is only possible with unordered_set, not with unordered_map
TEST_CASE_SET("transparent_set_insert", std::string, string_hash, string_eq) {
    auto set = set_t();
    set.insert("abcdefg");
    check(__LINE__, set, 1, 0, 0);
    set.insert("abcdefg");
    check(__LINE__, set, 2, 0, 0);
}

TYPE_TO_STRING_SET(std::string, string_hash);

// insert() transparent is only possible with unordered_set, not with unordered_map
TEST_CASE_SET("transparent_set_insert_not", std::string, string_hash) {
    auto set = set_t();
    set.insert("abcdefg");
    check(__LINE__, set, 0, 0, 1);
    set.insert("abcdefg");
    check(__LINE__, set, 0, 0, 2);
}

// emplace() transparent is only possible with unordered_set, not with unordered_map
TEST_CASE_SET("transparent_set_emplace", std::string, string_hash, string_eq) {
    auto set = set_t();
    set.emplace("abcdefg");
    check(__LINE__, set, 1, 0, 0);
    set.emplace("abcdefg");
    check(__LINE__, set, 2, 0, 0);
}

// emplace() transparent is only possible with unordered_set, not with unordered_map
TEST_CASE_SET("transparent_set_emplace_not", std::string, string_hash) {
    auto set = set_t();
    set.emplace("abcdefg");
    check(__LINE__, set, 0, 0, 1);
    set.emplace("abcdefg");
    check(__LINE__, set, 0, 0, 2);
}

struct string_hash_simple {
    using is_transparent = void; // enable heterogeneous lookup
    using is_avalanching = void; // mark class as high quality avalanching hash

    [[nodiscard]] auto operator()(std::string_view str) const noexcept -> uint64_t {
        return ankerl::unordered_dense::hash<std::string_view>{}(str);
    }
};

TYPE_TO_STRING_MAP(std::string, size_t, string_hash_simple, std::equal_to<>);

TEST_CASE_MAP("transparent_find_simple", std::string, size_t, string_hash_simple, std::equal_to<>) {
    auto map = map_t();
    map.try_emplace("hello", 1);
    auto it = map.find("huh");
    REQUIRE(it == map.end());
    it = map.find("hello");
    REQUIRE(it != map.end());

    auto cit = std::as_const(map).find("huh");
    REQUIRE(cit == map.end());
    REQUIRE(cit == map.cend());
    cit = std::as_const(map).find("hello");
    REQUIRE(cit != map.end());

    // string_view
    it = map.find("huh"sv);
    REQUIRE(it == map.end());
    it = map.find("hello"sv);
    REQUIRE(it != map.end());

    // string
    it = map.find("huh"s);
    REQUIRE(it == map.end());
    it = map.find("hello"s);
    REQUIRE(it != map.end());
}
