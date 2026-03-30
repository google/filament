/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <utils/tribool.h>

using namespace utils;

TEST(TriboolTest, ConstructorsAndObservers) {
    tribool t1;
    EXPECT_TRUE(t1.is_indeterminate());
    EXPECT_FALSE(t1.is_true());
    EXPECT_FALSE(t1.is_false());

    tribool t2(true);
    EXPECT_TRUE(t2.is_true());
    EXPECT_FALSE(t2.is_indeterminate());
    EXPECT_FALSE(t2.is_false());

    tribool t3(false);
    EXPECT_TRUE(t3.is_false());
    EXPECT_FALSE(t3.is_true());
    EXPECT_FALSE(t3.is_indeterminate());

    tribool t4(tribool::True);
    EXPECT_TRUE(t4.is_true());

    tribool t5(tribool::False);
    EXPECT_TRUE(t5.is_false());

    tribool t6(tribool::Indeterminate);
    EXPECT_TRUE(t6.is_indeterminate());
}

TEST(TriboolTest, LogicalNot) {
    EXPECT_TRUE((!tribool(true)).is_false());
    EXPECT_TRUE((!tribool(false)).is_true());
    EXPECT_TRUE((!tribool(tribool::Indeterminate)).is_indeterminate());
}

TEST(TriboolTest, LogicalAnd) {
    tribool T(true);
    tribool F(false);
    tribool I(tribool::Indeterminate);

    EXPECT_TRUE((T && T).is_true());
    EXPECT_TRUE((T && F).is_false());
    EXPECT_TRUE((F && T).is_false());
    EXPECT_TRUE((F && F).is_false());

    EXPECT_TRUE((T && I).is_indeterminate());
    EXPECT_TRUE((I && T).is_indeterminate());

    EXPECT_TRUE((F && I).is_false());
    EXPECT_TRUE((I && F).is_false());

    EXPECT_TRUE((I && I).is_indeterminate());
}

TEST(TriboolTest, LogicalOr) {
    tribool T(true);
    tribool F(false);
    tribool I(tribool::Indeterminate);

    EXPECT_TRUE((T || T).is_true());
    EXPECT_TRUE((T || F).is_true());
    EXPECT_TRUE((F || T).is_true());
    EXPECT_TRUE((F || F).is_false());

    EXPECT_TRUE((T || I).is_true());
    EXPECT_TRUE((I || T).is_true());

    EXPECT_TRUE((F || I).is_indeterminate());
    EXPECT_TRUE((I || F).is_indeterminate());

    EXPECT_TRUE((I || I).is_indeterminate());
}

TEST(TriboolTest, Equality) {
    tribool T(true);
    tribool F(false);
    tribool I(tribool::Indeterminate);

    EXPECT_TRUE(T == tribool(true));
    EXPECT_TRUE(F == tribool(false));
    EXPECT_TRUE(I == tribool(tribool::Indeterminate));

    EXPECT_TRUE(T != F);
    EXPECT_TRUE(T != I);
    EXPECT_TRUE(F != I);
}
