// Copyright 2024 The Khronos Group Inc.
// Copyright 2024 Valve Corporation
// Copyright 2024 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vulkan/utility/vk_sparse_range_map.hpp>

TEST(sparse_range_map, basic) {
    vku::sparse::range_map<uint32_t, std::string> map;

    map.insert(std::make_pair(vku::sparse::range<uint32_t>(0, 100), "first"));
    map.insert(std::make_pair(vku::sparse::range<uint32_t>(500, 501), "second"));

    auto iter = map.find(42);
    ASSERT_NE(iter, map.end());
    ASSERT_EQ(0u, iter->first.begin);
    ASSERT_EQ(100u, iter->first.end);
    ASSERT_EQ("first", iter->second);

    iter = map.find(501);
    ASSERT_EQ(iter, map.end());
}

TEST(sparse_range_map, small) {
    vku::sparse::small_range_map<uint32_t, std::string> map;

    map.insert(std::make_pair(vku::sparse::range<uint32_t>(0, 10), "first"));
    map.insert(std::make_pair(vku::sparse::range<uint32_t>(50, 51), "second"));

    auto iter = map.find(4);
    ASSERT_NE(iter, map.end());
    ASSERT_EQ(0u, iter->first.begin);
    ASSERT_EQ(10u, iter->first.end);
    ASSERT_EQ("first", iter->second);

    iter = map.find(51);
    ASSERT_EQ(iter, map.end());
}
