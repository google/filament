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

#include <utils/QuadTree.h>

#include <gtest/gtest.h>

using namespace utils;

TEST(QuadTreeArrayTest, TraversalDFS) {

    using QuadTree = QuadTreeArray<bool, 4>;
    QuadTree qt{};
    QuadTree::NodeId indices[qt.size()];
    size_t visitOrder[qt.size()] = {};
    size_t order = 0;

    QuadTree::traverse(0, 0,
            [&](auto const& curr) -> QuadTree::TraversalResult {
                size_t const i = QuadTreeUtils::index(curr.l, curr.code);
                EXPECT_FALSE(qt[i]);
                qt[i] = true;
                indices[i] = curr;
                visitOrder[i] = ++order;
                if (curr.l > 0) {
                    size_t const parentIdx = QuadTreeUtils::parent(curr.l, curr.code);
                    EXPECT_TRUE(qt[parentIdx]);
                    EXPECT_LT(visitOrder[parentIdx], visitOrder[i]);
                }
                return QuadTree::TraversalResult::RECURSE;
            });

    EXPECT_EQ(qt.size(), order);

    size_t i = 0;
    for (size_t y = 0; y < QuadTree::height(); y++) {
        for (size_t x = 0; x < (1 << 2 * y); x++, i++) {
            EXPECT_TRUE(qt[i]);
            EXPECT_EQ(indices[i].l, y);
            EXPECT_EQ(indices[i].code, x);
            // Verify DFS pre-order property: for any internal node, its entire child c subtree
            // (including the deepest leaf of child c) is visited before sibling child c + 1.
            if (y + 1 < QuadTree::height()) {
                size_t const shiftToLeaf = 2 * (QuadTree::height() - 1 - (y + 1));
                for (size_t c = 0; c < 3; c++) {
                    size_t const childCode = (x << 2u) | c;
                    size_t const nextSiblingCode = (x << 2u) | (c + 1);
                    size_t const deepestLeafCode =
                            (childCode << shiftToLeaf) | ((1u << shiftToLeaf) - 1u);
                    size_t const deepestLeafIdx =
                            QuadTreeUtils::index(QuadTree::height() - 1, deepestLeafCode);
                    size_t const nextSiblingIdx = QuadTreeUtils::index(y + 1, nextSiblingCode);
                    EXPECT_LT(visitOrder[deepestLeafIdx], visitOrder[nextSiblingIdx]);
                }
            }
        }
    }

    // Verify SKIP_CHILDREN stops descending from root while visiting root itself.
    size_t skipCount = 0;
    QuadTree::traverse(0, 0,
            [&](auto const&) -> QuadTree::TraversalResult {
                ++skipCount;
                return QuadTree::TraversalResult::SKIP_CHILDREN;
            });
    EXPECT_EQ(1u, skipCount);
}
