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

TEST(CString, Replace) {
    {
        CString str("foo bar baz");
        str.replace(0, 0, CString("lkj"));
        EXPECT_STREQ("lkjfoo bar baz", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(4, 3, CString("dpa"));
        EXPECT_STREQ("foo dpa baz", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(4, 3, CString(""));
        EXPECT_STREQ("foo  baz", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(4, 3, CString("a"));
        EXPECT_STREQ("foo a baz", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(4, 3, CString("abcdef"));
        EXPECT_STREQ("foo abcdef baz", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(0, 3, CString("abcdef"));
        EXPECT_STREQ("abcdef bar baz", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(8, 3, CString("abcdef"));
        EXPECT_STREQ("foo bar abcdef", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(0, 11, CString("abcdef"));
        EXPECT_STREQ("abcdef", str.c_str());
    }
}

TEST(CString, ReplaceZeroLength) {
    {
        std::string str("foobar");
        str.replace(6, 0, "abc");
        EXPECT_STREQ("foobarabc", str.c_str());
    }
    {
        CString str;
        str.replace(0, 0, CString("far"));
        EXPECT_STREQ("far", str.c_str());
    }
}

TEST(CString, ReplacePastEndOfString) {
    {
        CString str("foo bar baz");
        str.replace(0, 100, CString("bat"));
        EXPECT_STREQ("bat", str.c_str());
    }
    {
        CString str("foo bar baz");
        str.replace(8, 100, CString("bat"));
        EXPECT_STREQ("foo bar bat", str.c_str());
    }
}

TEST(FixedSizeString, EmptyString) {
    {
        FixedSizeString<32> str;
        EXPECT_STREQ("", str.c_str());
    }
    {
        FixedSizeString<32> str("");
        EXPECT_STREQ("", str.c_str());
    }
}

TEST(FixedSizeString, Constructors) {
    {
        FixedSizeString<32> str("short string");
        EXPECT_STREQ("short string", str.c_str());
    }
    {
        FixedSizeString<16> str("a long string abcdefghijklmnopqrst");
        EXPECT_STREQ("a long string a", str.c_str());
    }
}
