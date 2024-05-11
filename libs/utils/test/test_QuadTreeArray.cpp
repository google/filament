/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <utils/QuadTree.h>

using namespace utils;

TEST(QuadTreeArrayTest, TraversalDFS) {

    using QuadTree = QuadTreeArray<bool, 4>;
    QuadTree qt;
    QuadTree::NodeId indices[qt.size()];

    QuadTree::traverse(0, 0,
            [&](auto const& curr) -> QuadTree::TraversalResult {
                size_t i = QuadTreeUtils::index(curr.l, curr.code);
                indices[i] = curr;
                return QuadTree::TraversalResult::RECURSE;
            });

    size_t i = 0;
    for (size_t y = 0; y < QuadTree::height(); y++) {
        for (size_t x = 0; x < (1 << 2 * y); x++, i++) {
            EXPECT_EQ(indices[i].l, y);
            EXPECT_EQ(indices[i].code, x);
        }
    }
}
