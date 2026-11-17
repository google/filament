/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <utils/BinaryTreeArray.h>

#include <gtest/gtest.h>

using namespace utils;

TEST(BinaryTreeArray, basic) {
    struct Node {
        size_t index = ~size_t(0);
        size_t next = ~size_t(0);
        size_t visitOrder = 0;
        bool visited = false;
    };
    Node array[256];

    auto runTraversal = [&array](size_t const height) {
        for (auto& n : array) {
            n = Node{};
        }
        size_t visitOrder = 0;
        size_t expectedLeafCol = 0;
        BinaryTreeArray::traverse(height,
                [&](size_t index, size_t col, size_t next) {
                    EXPECT_FALSE(array[index].visited);
                    EXPECT_EQ(expectedLeafCol++, col);
                    array[index] = Node{ index, next, ++visitOrder, true };
                },
                [&](size_t index, size_t l, size_t r, size_t next) {
                    EXPECT_FALSE(array[index].visited);
                    EXPECT_TRUE(array[l].visited);
                    EXPECT_TRUE(array[r].visited);
                    EXPECT_LT(array[l].visitOrder, array[r].visitOrder);
                    EXPECT_EQ(index + 1, l);
                    EXPECT_EQ(array[l].next, r);
                    EXPECT_EQ(array[r].next, next);
                    array[index] = Node{ index, next, ++visitOrder, true };
                });
        EXPECT_EQ(BinaryTreeArray::count(height), visitOrder);
        EXPECT_EQ(size_t(1) << (height - 1), expectedLeafCol);
        EXPECT_EQ(BinaryTreeArray::count(height), array[0].visitOrder);
    };

    runTraversal(1);

    EXPECT_EQ(0, array[0].index);
    EXPECT_EQ(1, array[0].next);


    runTraversal(2);

    EXPECT_EQ(0, array[0].index);
    EXPECT_EQ(3, array[0].next);

        EXPECT_EQ(1, array[1].index);
        EXPECT_EQ(2, array[1].next);

        EXPECT_EQ(2, array[2].index);
        EXPECT_EQ(3, array[2].next);


    runTraversal(3);

    EXPECT_EQ(0, array[0].index);
    EXPECT_EQ(7, array[0].next);

        EXPECT_EQ(1, array[1].index);
        EXPECT_EQ(4, array[1].next);

            EXPECT_EQ(2, array[2].index);
            EXPECT_EQ(3, array[2].next);

            EXPECT_EQ(3, array[3].index);
            EXPECT_EQ(4, array[3].next);

        EXPECT_EQ(4, array[4].index);
        EXPECT_EQ(7, array[4].next);

            EXPECT_EQ(5, array[5].index);
            EXPECT_EQ(6, array[5].next);

            EXPECT_EQ(6, array[6].index);
            EXPECT_EQ(7, array[6].next);


    runTraversal(4);

    EXPECT_EQ(0, array[0].index);
    EXPECT_EQ(15, array[0].next);

        EXPECT_EQ(1, array[1].index);
        EXPECT_EQ(8, array[1].next);

            EXPECT_EQ(2, array[2].index);
            EXPECT_EQ(5, array[2].next);

                EXPECT_EQ(3, array[3].index);
                EXPECT_EQ(4, array[3].next);

                EXPECT_EQ(4, array[4].index);
                EXPECT_EQ(5, array[4].next);

            EXPECT_EQ(5, array[5].index);
            EXPECT_EQ(8, array[5].next);

                EXPECT_EQ(6, array[6].index);
                EXPECT_EQ(7, array[6].next);

                EXPECT_EQ(7, array[7].index);
                EXPECT_EQ(8, array[7].next);

        EXPECT_EQ(8, array[8].index);
        EXPECT_EQ(15, array[8].next);

            EXPECT_EQ(9, array[9].index);
            EXPECT_EQ(12, array[9].next);

                EXPECT_EQ(10, array[10].index);
                EXPECT_EQ(11, array[10].next);

                EXPECT_EQ(11, array[11].index);
                EXPECT_EQ(12, array[11].next);

            EXPECT_EQ(12, array[12].index);
            EXPECT_EQ(15, array[12].next);

                EXPECT_EQ(13, array[13].index);
                EXPECT_EQ(14, array[13].next);

                EXPECT_EQ(14, array[14].index);
                EXPECT_EQ(15, array[14].next);

}

