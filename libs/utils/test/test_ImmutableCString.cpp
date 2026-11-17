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

#include <utils/ImmutableCString.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

using namespace utils;

TEST(ImmutableCString, EmptyString) {
    ImmutableCString const emptyString("");
    EXPECT_STREQ("", emptyString.c_str());
    EXPECT_EQ(0, emptyString.length());
    EXPECT_TRUE(emptyString.empty());
    EXPECT_TRUE(emptyString.isStatic());
}

TEST(ImmutableCString, Constructors) {
    // ImmutableCString()
    {
        ImmutableCString const str;
        EXPECT_STREQ("", str.c_str());
        EXPECT_EQ(0, str.length());
        EXPECT_TRUE(str.empty());
        EXPECT_TRUE(str.isStatic());
    }
    // ImmutableCString(const char* cstr, size_t length)
    {
        ImmutableCString const str("foobar", 3);
        EXPECT_STREQ("foo", str.c_str());
        EXPECT_EQ(3, str.length());
        EXPECT_TRUE(str.isDynamic());
    }
    // ImmutableCString(const char* cstr)
    {
        const char* hello_cstr = "hello";
        ImmutableCString const str(hello_cstr);
        EXPECT_STREQ("hello", str.c_str());
        EXPECT_EQ(5, str.length());
        EXPECT_TRUE(str.isDynamic());
    }
    // ImmutableCString(StringLiteral<N>)
    {
        ImmutableCString const str("literal"); // this uses the template constructor
        EXPECT_STREQ("literal", str.c_str());
        EXPECT_EQ(7, str.length());
        EXPECT_TRUE(str.isStatic());
    }
    // Copy constructor (static)
    {
        ImmutableCString const s1("copy me");
        EXPECT_TRUE(s1.isStatic());
        ImmutableCString const s2(s1); // NOLINT(*-unnecessary-copy-initialization)
        EXPECT_STREQ("copy me", s2.c_str());
        EXPECT_TRUE(s2.isStatic());
        EXPECT_EQ(s1.c_str(), s2.c_str());
    }
    // Copy constructor (dynamic)
    {
        const char* dyn_cstr = "dynamic copy me";
        ImmutableCString const s1(dyn_cstr);
        EXPECT_TRUE(s1.isDynamic());
        ImmutableCString const s2(s1); // NOLINT(*-unnecessary-copy-initialization)
        EXPECT_STREQ("dynamic copy me", s2.c_str());
        EXPECT_EQ(s1.length(), s2.length());
        EXPECT_TRUE(s2.isDynamic());
        EXPECT_NE(s1.c_str(), s2.c_str());
    }
    // Move constructor (static)
    {
        ImmutableCString s1("move me");
        EXPECT_TRUE(s1.isStatic());
        ImmutableCString const s2(std::move(s1));
        EXPECT_STREQ("move me", s2.c_str());
        EXPECT_TRUE(s2.isStatic());
    }
    // Move constructor (dynamic)
    {
        const char* dyn_cstr = "dynamic move me";
        ImmutableCString s1(dyn_cstr);
        const char* const original_ptr = s1.c_str();
        EXPECT_TRUE(s1.isDynamic());
        ImmutableCString const s2(std::move(s1));
        EXPECT_STREQ("dynamic move me", s2.c_str());
        EXPECT_TRUE(s2.isDynamic());
        EXPECT_EQ(original_ptr, s2.c_str());
    }
}

TEST(ImmutableCString, Assignment) {
    // Copy assignment (static and dynamic)
    {
        ImmutableCString const s1("copy");
        ImmutableCString s2;
        s2 = s1;
        EXPECT_STREQ("copy", s2.c_str());
        EXPECT_TRUE(s2.isStatic());

        const char* dyn_cstr = "dynamic copy";
        ImmutableCString const s3(dyn_cstr);
        EXPECT_TRUE(s3.isDynamic());
        s2 = s3;
        EXPECT_STREQ("dynamic copy", s2.c_str());
        EXPECT_TRUE(s2.isDynamic());
        EXPECT_NE(s3.c_str(), s2.c_str());
    }
    // Move assignment
    {
        ImmutableCString s1("move");
        ImmutableCString s2("previous");
        s2 = std::move(s1);
        EXPECT_STREQ("move", s2.c_str());
        EXPECT_TRUE(s2.isStatic());
    }
    // self-copy-assignment
    {
        ImmutableCString s1("self");
        // This looks strange, but it's an important edge case to test for assignment operators.
        s1 = s1;
        EXPECT_STREQ("self", s1.c_str());
        EXPECT_TRUE(s1.isStatic());
    }
    // self-move-assignment
    {
        ImmutableCString s1("self-move");
        s1 = std::move(s1);
        // A self-move-assignment via swap leaves the object unchanged.
        EXPECT_STREQ("self-move", s1.c_str());
        EXPECT_TRUE(s1.isStatic());
    }
}

TEST(ImmutableCString, Swap) {
    ImmutableCString s1("first");
    ImmutableCString s2("second");

    size_t const l1 = s1.length();
    size_t const l2 = s2.length();

    s1.swap(s2);

    EXPECT_STREQ("second", s1.c_str());
    EXPECT_STREQ("first", s2.c_str());
    EXPECT_EQ(l2, s1.length());
    EXPECT_EQ(l1, s2.length());
}

TEST(ImmutableCString, Comparison) {
    ImmutableCString const s1("abc");
    ImmutableCString const s2("abc");
    ImmutableCString const s3("def");
    ImmutableCString const s4("ab");

    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);

    EXPECT_TRUE(s1 != s3);
    EXPECT_FALSE(s1 != s2);

    EXPECT_TRUE(s1 < s3);
    EXPECT_FALSE(s3 < s1);

    EXPECT_TRUE(s3 > s1);
    EXPECT_FALSE(s1 > s3);

    EXPECT_TRUE(s1 <= s2);
    EXPECT_TRUE(s1 <= s3);
    EXPECT_FALSE(s3 <= s1);

    EXPECT_TRUE(s2 >= s1);
    EXPECT_TRUE(s3 >= s1);
    EXPECT_FALSE(s1 >= s3);

    EXPECT_TRUE(s4 < s1);
    EXPECT_TRUE(s1 > s4);
}

TEST(ImmutableCString, ElementAccess) {
    // Test with statically-allocated string
    const ImmutableCString cstr("const");
    EXPECT_EQ('c', cstr.front());
    EXPECT_EQ('t', cstr.back());
    EXPECT_EQ('n', cstr[2]);
    EXPECT_EQ('s', cstr.at(3));

    // Test with heap-allocated string
    std::string const a_normal_string = "a normal string";
    const ImmutableCString v(a_normal_string.c_str()); // heap allocation
    EXPECT_EQ('a', v.front());
    EXPECT_EQ('g', v.back());
    EXPECT_EQ(' ', v[1]);
    EXPECT_EQ('n', v.at(2));

    // iterators
    std::string const s(cstr.begin(), cstr.end());
    EXPECT_EQ("const", s);

    EXPECT_TRUE(std::equal(cstr.begin(), cstr.end(), "const"));
}

TEST(ImmutableCString, NoHeapAllocation) {
    ImmutableCString s("hello world"); // no allocation
    EXPECT_TRUE(s.isStatic());
    ImmutableCString t(s); // no allocation
    EXPECT_TRUE(t.isStatic());
    EXPECT_EQ(s.c_str(), t.c_str());

    ImmutableCString u("different literal");
    EXPECT_TRUE(u.isStatic());
    s = u; // no allocation
    EXPECT_TRUE(s.isStatic());
    EXPECT_STREQ("different literal", s.c_str());

    t = std::move(s); // no allocation (swaps t and s)
    EXPECT_TRUE(t.isStatic());
    EXPECT_STREQ("different literal", t.c_str());
    EXPECT_TRUE(s.isStatic());
    EXPECT_STREQ("hello world", s.c_str());
}

TEST(ImmutableCString, HeapAllocation) {
    std::string const a_normal_string = "a normal string";
    ImmutableCString const v(a_normal_string.c_str()); // heap allocation
    EXPECT_TRUE(v.isDynamic());
    EXPECT_STREQ("a normal string", v.c_str());
}
