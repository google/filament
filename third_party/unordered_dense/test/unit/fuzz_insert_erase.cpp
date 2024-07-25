#include <ankerl/unordered_dense.h>
#include <fuzz/run.h>

#include <app/doctest.h>

#include <unordered_map>

namespace {

// Using DummyHash to make it easier for the fuzzer
struct dummy_hash {
    using is_avalanching = void;

    auto operator()(uint64_t x) const noexcept -> size_t {
        return static_cast<size_t>(x);
    }
};

template <typename Map>
void insert_erase(fuzz::provider p) {
    auto ank = Map();
    auto ref = std::unordered_map<uint64_t, uint64_t>();

    auto c = uint64_t();
    while (p.has_remaining_bytes()) {
        auto key = p.integral<uint64_t>();
        ank[key] = c;
        ref[key] = c;
        ++c;

        key = p.integral<uint64_t>();
        REQUIRE(ank.erase(key) == ref.erase(key));
        REQUIRE(ank.size() == ref.size());
    }
    auto cpy = std::unordered_map(ank.begin(), ank.end());
    REQUIRE(cpy == ref);
}

} // namespace

TEST_CASE("fuzz_insert_erase" * doctest::test_suite("fuzz")) {
    fuzz::run([](fuzz::provider p) {
        // try all 3 different map styles with the same input
        insert_erase<ankerl::unordered_dense::map<uint64_t, uint64_t, dummy_hash>>(p.copy());
        insert_erase<ankerl::unordered_dense::segmented_map<uint64_t, uint64_t, dummy_hash>>(p.copy());
        insert_erase<ankerl::unordered_dense::map<uint64_t,
                                                  uint64_t,
                                                  ankerl::unordered_dense::hash<uint64_t>,
                                                  std::equal_to<uint64_t>,
                                                  std::deque<std::pair<uint64_t, uint64_t>>>>(p.copy());
    });
}
