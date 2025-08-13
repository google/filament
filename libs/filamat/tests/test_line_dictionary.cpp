/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "eiff/LineDictionary.h"

#include <string>

using namespace filamat;

TEST(LineDictionary, splitString) {
    LineDictionary dictionary;
    const std::string text = "first line hp_copy_123456 second line";
    dictionary.addText(text);
    EXPECT_EQ(dictionary.size(), 3);
    EXPECT_EQ(dictionary[0], "first line ");
    EXPECT_EQ(dictionary[1], "hp_copy_123456");
    EXPECT_EQ(dictionary[2], " second line");
}

TEST(LineDictionary, Empty) {
    LineDictionary const dictionary;
    EXPECT_TRUE(dictionary.empty());
    EXPECT_EQ(dictionary.size(), 0);
}

TEST(LineDictionary, AddTextSimple) {
    LineDictionary dictionary;
    dictionary.addText("Hello world\n");
    EXPECT_FALSE(dictionary.empty());
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "Hello world\n");
}

TEST(LineDictionary, AddTextMultipleLines) {
    LineDictionary dictionary;
    dictionary.addText("First line\nSecond line\n");
    EXPECT_EQ(dictionary.size(), 2);
    EXPECT_EQ(dictionary[0], "First line\n");
    EXPECT_EQ(dictionary[1], "Second line\n");
}

TEST(LineDictionary, AddTextDuplicateLines) {
    LineDictionary dictionary;
    dictionary.addText("Same line\nSame line\n");
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "Same line\n");
}

TEST(LineDictionary, GetIndices) {
    LineDictionary dictionary;
    dictionary.addText("Line one\nLine two\nLine one\n");
    auto const indicesOne = dictionary.getIndices("Line one\n");
    ASSERT_EQ(indicesOne.size(), 1);
    EXPECT_EQ(indicesOne[0], 0);

    auto const indicesTwo = dictionary.getIndices("Line two\n");
    ASSERT_EQ(indicesTwo.size(), 1);
    EXPECT_EQ(indicesTwo[0], 1);
}

TEST(LineDictionary, SplitLogicNoPattern) {
    LineDictionary dictionary;
    dictionary.addText("A simple line with no patterns.");
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "A simple line with no patterns.");
}

TEST(LineDictionary, SplitLogicHpPattern) {
    LineDictionary dictionary;
    dictionary.addText("some_var = hp_copy_123;");
    EXPECT_EQ(dictionary.size(), 3);
    EXPECT_EQ(dictionary[0], "some_var = ");
    EXPECT_EQ(dictionary[1], "hp_copy_123");
    EXPECT_EQ(dictionary[2], ";");
}

TEST(LineDictionary, SplitLogicMpPattern) {
    LineDictionary dictionary;
    dictionary.addText("another_var = mp_copy_4567;");
    EXPECT_EQ(dictionary.size(), 3);
    EXPECT_EQ(dictionary[0], "another_var = ");
    EXPECT_EQ(dictionary[1], "mp_copy_4567");
    EXPECT_EQ(dictionary[2], ";");
}

TEST(LineDictionary, SplitLogicUnderscorePattern) {
    LineDictionary dictionary;
    dictionary.addText("var_1 = 0;");
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "var_1 = 0;");
}

TEST(LineDictionary, SplitLogicMultiplePatterns) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_1 mp_copy_2 _3");
    EXPECT_EQ(dictionary.size(), 4);
    EXPECT_EQ(dictionary[0], "hp_copy_1");
    EXPECT_EQ(dictionary[1], " ");
    EXPECT_EQ(dictionary[2], "mp_copy_2");
    EXPECT_EQ(dictionary[3], "_3");
}

TEST(LineDictionary, SplitLogicInvalidPattern) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_ a_b_c");
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "hp_copy_ a_b_c");
}

TEST(LineDictionary, SplitLogicPatternFollowedByWordChar) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_99rest");
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "hp_copy_99rest");
}

TEST(LineDictionary, SplitLogicPatternPrecededByWordChar) {
    LineDictionary dictionary;
    dictionary.addText("rest_of_it_hp_copy_99");
    EXPECT_EQ(dictionary.size(), 1);
    EXPECT_EQ(dictionary[0], "rest_of_it_hp_copy_99");
}

TEST(LineDictionary, SplitLogicPatternNotFollowedByWordChar) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_99;");
    EXPECT_EQ(dictionary.size(), 2);
    EXPECT_EQ(dictionary[0], "hp_copy_99");
    EXPECT_EQ(dictionary[1], ";");
}

TEST(LineDictionary, AddEmptyText) {
    LineDictionary dictionary;
    dictionary.addText("");
    EXPECT_TRUE(dictionary.empty());
}

TEST(LineDictionary, GetIndicesMultiple) {
    LineDictionary dictionary;
    dictionary.addText("A _1 B _2");
    auto const indices = dictionary.getIndices("A _1");
    ASSERT_EQ(indices.size(), 2);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 1);
}

TEST(LineDictionary, GetIndicesMultiplePatternsInARow) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_1 hp_copy_2");
    auto const indices = dictionary.getIndices("hp_copy_1 hp_copy_2");
    ASSERT_EQ(indices.size(), 3);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 1);
    EXPECT_EQ(indices[2], 2);
}

TEST(LineDictionary, GetIndicesSamePatternMultipleTimes) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_1 hp_copy_1");
    auto const indices = dictionary.getIndices("hp_copy_1 hp_copy_1");
    ASSERT_EQ(indices.size(), 3);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 1);
    EXPECT_EQ(indices[2], 0);
}

TEST(LineDictionary, GetIndicesWithExistingDictionary) {
    LineDictionary dictionary;
    dictionary.addText("unrelated_string");
    dictionary.addText("hp_copy_1");
    dictionary.addText("another_string");
    dictionary.addText(" ");
    auto const indices = dictionary.getIndices("hp_copy_1 hp_copy_1");
    ASSERT_EQ(indices.size(), 3);
    EXPECT_EQ(indices[0], 1);
    EXPECT_EQ(indices[1], 3);
    EXPECT_EQ(indices[2], 1);
}

TEST(LineDictionary, GetIndicesWithAdjacentPatterns) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_1hp_copy_2");
    auto const indices = dictionary.getIndices("hp_copy_1hp_copy_2");
    ASSERT_EQ(indices.size(), 1);
    EXPECT_EQ(indices[0], 0);
}

TEST(LineDictionary, GetIndicesWithAdjacentPatternsNotInDictionary) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_1");
    dictionary.addText("hp_copy_2");
    auto const indices = dictionary.getIndices("hp_copy_1hp_copy_2");
    ASSERT_EQ(indices.size(), 0);
}

TEST(LineDictionary, GetIndicesWithMixedContent) {
    LineDictionary dictionary;
    dictionary.addText("hp_copy_1");
    dictionary.addText(" ");
    dictionary.addText("mp_copy_2");

    // The query string contains patterns that are in the dictionary,
    // but also content that is not.
    auto const indices = dictionary.getIndices("prefix hp_copy_1 mp_copy_2 suffix");

    // Since not all substrings of the query string are in the dictionary,
    // getIndices should return an empty vector.
    ASSERT_EQ(indices.size(), 0);
}
