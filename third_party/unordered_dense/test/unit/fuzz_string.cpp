#include <ankerl/unordered_dense.h>
#include <fuzz/provider.h>
#include <fuzz/run.h>

#include <app/doctest.h>

#include <unordered_map>

namespace {

template <typename Map>
void do_string(fuzz::provider p) {
    auto ank = Map();
    auto ref = std::unordered_map<std::string, std::string>();

    while (p.has_remaining_bytes()) {
        auto str = p.string(32);
        REQUIRE(ank.try_emplace(str, "hello!").second == ref.try_emplace(str, "hello!").second);

        str = p.string(32);
        auto it_ank = ank.find(str);
        auto it_ref = ref.find(str);
        REQUIRE((it_ank == ank.end()) == (it_ref == ref.end()));

        if (it_ank != ank.end()) {
            ank.erase(it_ank);
            ref.erase(it_ref);
        }
        REQUIRE(ank.size() == ref.size());

        str = p.string(32);
        REQUIRE(ank.try_emplace(str, "huh").second == ref.try_emplace(str, "huh").second);

        str = p.string(32);
        REQUIRE(ank.erase(str) == ref.erase(str));
    }

    REQUIRE(std::unordered_map(ank.begin(), ank.end()) == ref);
}

} // namespace

TEST_CASE("fuzz_string" * doctest::test_suite("fuzz")) {
    fuzz::run([](fuzz::provider p) {
        do_string<ankerl::unordered_dense::map<std::string, std::string>>(p.copy());
        do_string<ankerl::unordered_dense::segmented_map<std::string, std::string>>(p.copy());
        do_string<ankerl::unordered_dense::map<std::string,
                                               std::string,
                                               ankerl::unordered_dense::hash<std::string>,
                                               std::equal_to<std::string>,
                                               std::deque<std::pair<std::string, std::string>>>>(p.copy());
    });
}
