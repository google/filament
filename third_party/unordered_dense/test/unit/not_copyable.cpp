#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t
#include <tuple>   // for forward_as_tuple
#include <utility> // for move, piecewise_construct

// not copyable, but movable.
class no_copy {
public:
    no_copy() noexcept = default;
    explicit no_copy(size_t d) noexcept
        : m_mData(d) {}

    ~no_copy() = default;
    no_copy(no_copy const&) = delete;
    auto operator=(no_copy const&) -> no_copy& = delete;

    no_copy(no_copy&&) = default;
    auto operator=(no_copy&&) -> no_copy& = default;

    [[nodiscard]] auto data() const -> size_t {
        return m_mData;
    }

private:
    size_t m_mData{};
};

TYPE_TO_STRING_MAP(size_t, no_copy);

TEST_CASE_MAP("not_copyable", size_t, no_copy) {
    // it's ok because it is movable.
    map_t m;
    for (size_t i = 0; i < 100; ++i) {
        m[i];
        m.emplace(std::piecewise_construct, std::forward_as_tuple(i * 100), std::forward_as_tuple(i));
    }
    REQUIRE(m.size() == 199);

    // not copyable, because m is not copyable!
    // map_t m2 = m;

    // movable works
    map_t m2 = std::move(m);
    REQUIRE(m2.size() == 199);
    m = map_t{};
    REQUIRE(m.size() == 0);
}
