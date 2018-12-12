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

#include <gtest/gtest.h>

#include <utils/CString.h>

using namespace utils;

TEST(CString, EmptyString) {
    CString emptyString("");
    EXPECT_STREQ("", emptyString.c_str_safe());
}

TEST(StaticString, hash) {
    StaticString a("Hello World!");
    StaticString b = StaticString::make("Hello World!");
    StaticString c("Hello World");
    StaticString d("Hello World!");

    EXPECT_EQ(a.getHash(), b.getHash());
    EXPECT_EQ(a.getHash(), d.getHash());
    EXPECT_NE(a.getHash(), c.getHash());
    EXPECT_NE(b.getHash(), c.getHash());

    std::hash<StaticString> ha;
    EXPECT_EQ(ha(a), a.getHash());
}
