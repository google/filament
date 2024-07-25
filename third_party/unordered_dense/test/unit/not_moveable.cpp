#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <cstddef> // for size_t
#include <tuple>   // for forward_as_tuple
#include <utility> // for piecewise_construct

class no_move {
public:
    no_move() noexcept = default;
    explicit no_move(size_t d) noexcept
        : m_mData(d) {}
    ~no_move() = default;

    no_move(no_move const&) = default;
    auto operator=(no_move const&) -> no_move& = default;

    no_move(no_move&&) = delete;
    auto operator=(no_move&&) -> no_move& = delete;

    [[nodiscard]] auto data() const -> size_t {
        return m_mData;
    }

private:
    size_t m_mData{};
};

TYPE_TO_STRING_MAP(size_t, no_move);

// doesn't work with robin_hood::unordered_flat_map<size_t, NoMove> because not movable and not
// copyable
TEST_CASE_MAP("not_moveable", size_t, no_move) {
    // it's ok because it is movable.
    auto m = map_t();
    for (size_t i = 0; i < 100; ++i) {
        m[i];
        m.emplace(std::piecewise_construct, std::forward_as_tuple(i * 100), std::forward_as_tuple(i));
    }
    REQUIRE(m.size() == 199);

    // not copyable, because m is not copyable!
    // map_t m2 = m;

    // not movable
    // map_t m2 = std::move(m);
    // REQUIRE(m2.size() == 199);
    m.clear();
    REQUIRE(m.size() == 0);
}
