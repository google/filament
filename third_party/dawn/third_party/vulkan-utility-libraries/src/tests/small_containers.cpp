// Copyright 2024 The Khronos Group Inc.
// Copyright 2024 Valve Corporation
// Copyright 2024 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>
#include <array>
#include <limits>
#include <vector>
#include <vulkan/utility/vk_small_containers.hpp>

template <typename T, typename U>
bool HaveSameElementsUpTo(const T& l1, const U& l2, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (l1[static_cast<typename T::size_type>(i)] != l2[static_cast<typename U::size_type>(i)]) {
            return false;
        }
    }
    return true;
}

template <typename T, typename U>
bool HaveSameElements(const T& l1, const U& l2) {
    return static_cast<size_t>(l1.size()) == static_cast<size_t>(l2.size()) && HaveSameElementsUpTo(l1, l2, l1.size());
}

TEST(small_vector, int_resize) {
    // Resize int small vector, moving to small store
    // ---
    {
        // resize to current size
        vku::small::vector<int, 2, size_t> v1 = {1, 2, 3, 4};
        v1.resize(v1.size());
        std::array<int, 4> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v1, ref));
    }

    {
        // growing resize
        vku::small::vector<int, 2, size_t> v2 = {1, 2, 3, 4};
        v2.resize(5);
        std::array<int, 5> ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v2, ref));
    }

    {
        // shrinking resize
        vku::small::vector<int, 2, size_t> v3 = {1, 2, 3, 4};
        const auto v3_cap = v3.capacity();
        v3.resize(3);
        ASSERT_TRUE(v3.capacity() == v3_cap);  // Resize doesn't shrink capacity
        v3.shrink_to_fit();
        ASSERT_TRUE(v3.capacity() == v3.size());
        std::array<int, 3> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v3, ref));
    }

    {
        // shrink to 0
        vku::small::vector<int, 2, size_t> v4 = {1, 2, 3, 4};
        v4.resize(0);
        ASSERT_TRUE(v4.capacity() == 4);  // Resize doesn't shrink capacity
        v4.shrink_to_fit();
        ASSERT_TRUE(v4.capacity() == 2);  // Small capacity is in the minimal
        std::array<int, 0> ref = {};
        ASSERT_TRUE(HaveSameElements(v4, ref));
    }

    {
        // resize to size limit
        vku::small::vector<int, 2, uint8_t> v5 = {1, 2, 3, 4};
        v5.resize(std::numeric_limits<uint8_t>::max());
        std::vector<int> vec = {1, 2, 3, 4};
        vec.resize(std::numeric_limits<uint8_t>::max());
        ASSERT_TRUE(HaveSameElements(v5, vec));
    }

    // Resize int small vector, not moving to small store
    // ---
    {
        // resize to current size
        vku::small::vector<int, 2, size_t> v6 = {1, 2, 3, 4};
        v6.resize(v6.size());
        std::array<int, 4> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v6, ref));
    }

    {
        // growing resize
        vku::small::vector<int, 2, size_t> v7 = {1, 2, 3, 4};
        v7.resize(5);
        std::array<int, 5> ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v7, ref));
    }

    {
        // shrinking resize
        vku::small::vector<int, 2, size_t> v8 = {1, 2, 3, 4};
        v8.resize(3);
        std::array<int, 3> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v8, ref));
    }

    {
        // shrink to 0
        vku::small::vector<int, 2, size_t> v9 = {1, 2, 3, 4};
        v9.resize(0);
        std::array<int, 0> ref = {};
        ASSERT_TRUE(HaveSameElements(v9, ref));
    }

    {
        // resize to size limit
        vku::small::vector<int, 2, uint8_t> v10 = {1, 2, 3, 4};
        v10.resize(std::numeric_limits<uint8_t>::max());
        std::vector<int> vec = {1, 2, 3, 4};
        vec.resize(std::numeric_limits<uint8_t>::max());
        ASSERT_TRUE(HaveSameElements(v10, vec));
    }
}

struct NoDefaultCons {
    NoDefaultCons(int x) : x(x) {}
    int x;
};

bool operator!=(const NoDefaultCons& lhs, const NoDefaultCons& rhs) { return lhs.x != rhs.x; }

TEST(small_vector, not_default_insertable) {
    // Resize NoDefault small vector, moving to small store
    // ---
    {
        // resize to current size
        vku::small::vector<NoDefaultCons, 2, size_t> v1 = {1, 2, 3, 4};
        v1.resize(v1.size());
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v1, ref));
    }

    {
        // growing resize
        vku::small::vector<NoDefaultCons, 2, size_t> v2 = {1, 2, 3, 4};
        v2.resize(5);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElementsUpTo(v2, ref, ref.size()));
    }

    {
        // shrinking resize
        vku::small::vector<NoDefaultCons, 2, size_t> v3 = {1, 2, 3, 4};
        const auto v3_cap = v3.capacity();
        v3.resize(3);
        ASSERT_TRUE(v3.capacity() == v3_cap);  // Resize doesn't shrink capacity
        v3.shrink_to_fit();
        ASSERT_TRUE(v3.capacity() == v3.size());

        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v3, ref));
    }

    {
        // shrink to 0
        vku::small::vector<NoDefaultCons, 2, size_t> v4 = {1, 2, 3, 4};
        v4.resize(0);
        ASSERT_TRUE(v4.capacity() == 4);  // Resize doesn't shrink capacity
        v4.shrink_to_fit();
        ASSERT_TRUE(v4.capacity() == 2);  // Small capacity is in the minimal
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v4, ref));
    }

    // Resize NoDefault small vector, not moving to small store
    // ---
    {
        // resize to current size
        vku::small::vector<NoDefaultCons, 2, size_t> v6 = {1, 2, 3, 4};
        v6.resize(v6.size());
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v6, ref));
    }

    {
        // growing resize
        vku::small::vector<NoDefaultCons, 2, size_t> v7 = {1, 2, 3, 4};
        v7.resize(5);
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElementsUpTo(v7, ref, ref.size()));
    }

    {
        // shrinking resize
        vku::small::vector<NoDefaultCons, 2, size_t> v8 = {1, 2, 3, 4};
        v8.resize(3);
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v8, ref));
    }

    {
        // shrink to 0
        vku::small::vector<NoDefaultCons, 2, size_t> v9 = {1, 2, 3, 4};
        v9.resize(0);
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v9, ref));
    }
}

TEST(small_vector, not_default_insertable_default_value) {
    // Resize NoDefault small vector, moving to small store
    // ---
    {
        // resize to current size
        vku::small::vector<NoDefaultCons, 2, size_t> v1 = {1, 2, 3, 4};
        v1.resize(v1.size(), NoDefaultCons(0));
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v1, ref));
    }

    {
        // growing resize
        vku::small::vector<NoDefaultCons, 2, size_t> v2 = {1, 2, 3, 4};
        v2.resize(5, NoDefaultCons(0));
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v2, ref));
    }

    {
        // shrinking resize
        vku::small::vector<NoDefaultCons, 2, size_t> v3 = {1, 2, 3, 4};
        v3.resize(3, NoDefaultCons(0));
        v3.shrink_to_fit();
        ASSERT_TRUE(v3.capacity() == v3.size());
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v3, ref));
    }

    {
        // shrink to 0
        vku::small::vector<NoDefaultCons, 2, size_t> v4 = {1, 2, 3, 4};
        v4.resize(0, NoDefaultCons(0));
        ASSERT_TRUE(v4.capacity() == 4);  // Resize doesn't shrink capacity
        v4.shrink_to_fit();
        ASSERT_TRUE(v4.capacity() == 2);  // Small capacity is in the minimal
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v4, ref));
    }

    // Resize NoDefault small vector, not moving to small store
    // ---
    {
        // resize to current size
        vku::small::vector<NoDefaultCons, 2, size_t> v6 = {1, 2, 3, 4};
        v6.resize(v6.size());
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4};
        ASSERT_TRUE(HaveSameElements(v6, ref));
    }

    {
        // growing resize
        vku::small::vector<NoDefaultCons, 2, size_t> v7 = {1, 2, 3, 4};
        v7.resize(5, NoDefaultCons(0));
        std::vector<NoDefaultCons> ref = {1, 2, 3, 4, 0};
        ASSERT_TRUE(HaveSameElements(v7, ref));
    }

    {
        // shrinking resize
        vku::small::vector<NoDefaultCons, 2, size_t> v8 = {1, 2, 3, 4};
        v8.resize(3, NoDefaultCons(0));
        ASSERT_TRUE(v8.capacity() == 4);  // Resize doesn't shrink capacity
        std::vector<NoDefaultCons> ref = {1, 2, 3};
        ASSERT_TRUE(HaveSameElements(v8, ref));
    }

    {
        // shrink to 0
        vku::small::vector<NoDefaultCons, 2, size_t> v9 = {1, 2, 3, 4};
        v9.resize(0, NoDefaultCons(0));
        ASSERT_TRUE(v9.capacity() == 4);  // Resize doesn't shrink capacity
        std::vector<NoDefaultCons> ref = {};
        ASSERT_TRUE(HaveSameElements(v9, ref));
    }
}
TEST(small_vector, construct) {
    using SmallVector = vku::small::vector<std::string, 5, size_t>;
    const SmallVector ref_small = {"one", "two", "three", "four"};
    SmallVector ref_large = {"one", "two", "three", "four", "five", "six"};

    // Small construct and emplace vs. list (tests list contruction, really)
    SmallVector v_small_emplace;
    v_small_emplace.emplace_back("one");
    v_small_emplace.emplace_back("two");
    v_small_emplace.emplace_back("three");
    v_small_emplace.emplace_back("four");
    ASSERT_TRUE(HaveSameElements(ref_small, v_small_emplace));

    // Copy construct from small_store
    SmallVector v_small_copy(ref_small);
    ASSERT_TRUE(HaveSameElements(ref_small, v_small_copy));

    // Move construct from small_store
    SmallVector v_small_move_src(ref_small);
    SmallVector v_small_move_dst(std::move(v_small_move_src));
    ASSERT_TRUE(HaveSameElements(ref_small, v_small_move_dst));

    // Small construct and emplace vs. list (tests list contruction, really)
    SmallVector v_large_emplace;
    v_large_emplace.emplace_back("one");
    v_large_emplace.emplace_back("two");
    v_large_emplace.emplace_back("three");
    v_large_emplace.emplace_back("four");
    v_large_emplace.emplace_back("five");
    v_large_emplace.emplace_back("six");
    ASSERT_TRUE(HaveSameElements(ref_large, v_large_emplace));

    // Copy construct from large_store
    SmallVector v_large_copy(ref_large);
    ASSERT_TRUE(HaveSameElements(ref_large, v_large_copy));

    // Move construct from large_store
    SmallVector v_large_move_src(ref_large);
    SmallVector v_large_move_dst(std::move(v_large_move_src));
    ASSERT_TRUE(HaveSameElements(ref_large, v_large_move_dst));
}

TEST(small_vector, assign) {
    using SmallVector = vku::small::vector<std::string, 5, size_t>;
    const SmallVector ref_xxs = {"one", "two"};
    const SmallVector ref_xs = {"one", "two", "three"};
    const SmallVector ref_small = {"one", "two", "three", "four"};

    const SmallVector ref_large = {"one", "two", "three", "four", "five", "six"};
    const SmallVector ref_xl = {"one", "two", "three", "four", "five", "six", "seven"};
    const SmallVector ref_xxl = {"one", "two", "three", "four", "five", "six", "seven", "eight"};

    SmallVector v_src(ref_large);
    SmallVector v_dst(ref_small);

    // Copy from large store to small store
    v_dst = v_src;
    ASSERT_TRUE(HaveSameElements(ref_large, v_src));
    ASSERT_TRUE(HaveSameElements(ref_large, v_dst));

    // Quick small to large check to reset...
    v_dst = ref_small;
    ASSERT_TRUE(HaveSameElements(ref_small, v_dst));

    // Copy from large store to small store
    v_dst = std::move(v_src);
    // Spec doesn't require src to be empty after move *assignment*
    ASSERT_TRUE(HaveSameElements(ref_large, v_dst));

    // Same store type copy/move

    // Small
    //
    // Copy small to small reducing
    v_src = ref_xs;
    v_dst = ref_small;
    v_dst = v_src;
    ASSERT_TRUE(HaveSameElements(ref_xs, v_src));
    ASSERT_TRUE(HaveSameElements(ref_xs, v_dst));

    // Move small to small reducing
    v_src = ref_xs;
    v_dst = ref_small;
    v_dst = std::move(v_src);
    // Small move operators don't empty source
    ASSERT_TRUE(HaveSameElements(ref_xs, v_dst));

    // Copy small to small increasing
    v_src = ref_small;
    v_dst = ref_xs;
    v_dst = v_src;
    ASSERT_TRUE(HaveSameElements(ref_small, v_src));
    ASSERT_TRUE(HaveSameElements(ref_small, v_dst));

    // Move small to small increasing
    v_src = ref_small;
    v_dst = ref_xs;
    v_dst = std::move(v_src);
    // Small move operators don't empty source
    ASSERT_TRUE(HaveSameElements(ref_small, v_dst));

    // Large
    //
    // Copy large to large reducing
    v_src = ref_large;
    v_dst = ref_xl;
    v_dst = v_src;
    ASSERT_TRUE(HaveSameElements(ref_large, v_src));
    ASSERT_TRUE(HaveSameElements(ref_large, v_dst));

    // Move large to large reducing
    v_src = ref_large;
    v_dst = ref_xl;
    v_dst = std::move(v_src);
    ASSERT_TRUE(v_src.empty());  // Since large moves move the large store, the source is empty, but not required by spec of vector
    ASSERT_TRUE(HaveSameElements(ref_large, v_dst));

    // Copy large to large increasing
    v_src = ref_xxl;
    v_dst = ref_xl;
    v_dst = v_src;
    ASSERT_TRUE(HaveSameElements(ref_xxl, v_src));
    ASSERT_TRUE(HaveSameElements(ref_xxl, v_dst));

    // Move large to large increasing
    v_src = ref_xxl;
    v_dst = ref_xl;
    v_dst = std::move(v_src);
    ASSERT_TRUE(v_src.empty());
    ASSERT_TRUE(HaveSameElements(ref_xxl, v_dst));
}
