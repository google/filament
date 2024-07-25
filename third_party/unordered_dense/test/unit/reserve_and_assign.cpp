#include <ankerl/unordered_dense.h>

#include <app/checksum.h>
#include <app/doctest.h>

#include <cstdint> // for uint64_t
#include <string>  // for allocator, string
#include <vector>  // for vector

TEST_CASE_MAP("reserve_and_assign", std::string, uint64_t) {
    map_t a = {
        {"button", {}},
        {"selectbox-tr", {}},
        {"slidertrack-t", {}},
        {"sliderarrowinc-hover", {}},
        {"text-l", {}},
        {"title-bar-l", {}},
        {"checkbox-checked-hover", {}},
        {"datagridexpand-active", {}},
        {"icon-waves", {}},
        {"sliderarrowdec-hover", {}},
        {"datagridexpand-collapsed", {}},
        {"sliderarrowinc-active", {}},
        {"radio-active", {}},
        {"radio-checked", {}},
        {"selectvalue-hover", {}},
        {"huditem-l", {}},
        {"datagridexpand-collapsed-active", {}},
        {"slidertrack-b", {}},
        {"selectarrow-hover", {}},
        {"window-r", {}},
        {"selectbox-tl", {}},
        {"icon-score", {}},
        {"datagridheader-r", {}},
        {"icon-game", {}},
        {"sliderbar-c", {}},
        {"window-c", {}},
        {"datagridexpand-hover", {}},
        {"button-hover", {}},
        {"icon-hiscore", {}},
        {"sliderbar-hover-t", {}},
        {"sliderbar-hover-c", {}},
        {"selectarrow-active", {}},
        {"window-tl", {}},
        {"checkbox-active", {}},
        {"sliderarrowdec-active", {}},
        {"sliderbar-active-b", {}},
        {"sliderarrowdec", {}},
        {"window-bl", {}},
        {"datagridheader-l", {}},
        {"sliderbar-t", {}},
        {"sliderbar-active-t", {}},
        {"text-c", {}},
        {"window-br", {}},
        {"huditem-c", {}},
        {"selectbox-l", {}},
        {"icon-flag", {}},
        {"sliderbar-hover-b", {}},
        {"icon-help", {}},
        {"selectvalue", {}},
        {"title-bar-r", {}},
        {"sliderbar-active-c", {}},
        {"huditem-r", {}},
        {"radio-checked-active", {}},
        {"selectbox-c", {}},
        {"selectbox-bl", {}},
        {"icon-invader", {}},
        {"checkbox-checked-active", {}},
        {"slidertrack-c", {}},
        {"sliderarrowinc", {}},
        {"checkbox", {}},
    };

    map_t b;
    b = a;
    REQUIRE(b.find("button") != b.end()); // Passes OK.

    map_t c;
    c.reserve(51);
    c = a;
    REQUIRE(checksum::map(a) == checksum::map(c));
    REQUIRE(c.find("button") != c.end()); // Fails.
}

TEST_CASE_MAP("unit_reserve_only_flat", std::string, uint64_t) {
    auto map = map_t();
    map.reserve(51);
}
