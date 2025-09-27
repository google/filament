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

#include <algorithm>
#include <climits>
#include <utility>
#include <type_traits>

using namespace utils;

TEST(CString, EmptyString) {
    CString const emptyString("");
    EXPECT_STREQ("", emptyString.c_str_safe());
    EXPECT_EQ(0, emptyString.length());
    EXPECT_TRUE(emptyString.empty());
}

TEST(CString, Constructors) {
    // CString()
    {
        CString str;
        EXPECT_EQ(nullptr, str.c_str());
        EXPECT_STREQ("", str.c_str_safe());
        EXPECT_EQ(0, str.length());
        EXPECT_TRUE(str.empty());
    }
    // CString(const char* cstr, size_t length)
    {
        CString str("foobar", 3);
        EXPECT_STREQ("foo", str.c_str());
        EXPECT_EQ(3, str.length());
    }
    // CString(size_t length)
    {
        CString str(5);
        EXPECT_EQ(5, str.length());
        // The memory is zero-initialized
        EXPECT_EQ(0, str.c_str()[0]);
        EXPECT_EQ(0, str.c_str()[4]);
    }
    // CString(const char* cstr)
    {
        const char* hello_cstr = "hello";
        CString str(hello_cstr);
        EXPECT_STREQ("hello", str.c_str());
        EXPECT_EQ(5, str.length());
    }
    // CString(StringLiteral<N>)
    {
        CString str("literal"); // this uses the template constructor
        EXPECT_STREQ("literal", str.c_str());
        EXPECT_EQ(7, str.length());
    }
    // Copy constructor
    {
        CString s1("copy me");
        CString s2(s1);
        EXPECT_STREQ("copy me", s2.c_str());
        EXPECT_NE(s1.c_str(), s2.c_str()); // should be a deep copy
    }
    // Move constructor
    {
        CString s1("move me");
        const char* original_cstr = s1.c_str();
        CString s2(std::move(s1));
        EXPECT_STREQ("move me", s2.c_str());
        EXPECT_EQ(original_cstr, s2.c_str()); // pointer should be moved
        EXPECT_EQ(nullptr, s1.c_str()); // original should be empty
    }
}

TEST(CString, LiteralConstructor) {
    // This test verifies that CString can be constructed from a string literal,
    // and that this uses the desired template constructor, not the explicit
    // (and costly) `const char*` constructor.

    // Test copy-initialization. CString should be convertible from a string literal.
    // This works because "literal" has type `const char[N]` which matches
    // `StringLiteral<N>` and calls the non-explicit template constructor.
    static_assert(std::is_convertible_v<decltype("literal"), CString>,
            "CString should be convertible from a string literal.");
    CString s_copy = "literal";
    EXPECT_STREQ("literal", s_copy.c_str());
    EXPECT_EQ(7, s_copy.length());

    // Test direct-initialization. This should also use the same efficient constructor.
    static_assert(std::is_constructible_v<CString, decltype("literal")>,
            "CString should be constructible from a string literal.");
    CString s_direct("literal");
    EXPECT_STREQ("literal", s_direct.c_str());
    EXPECT_EQ(7, s_direct.length());

    // Verify that direct-initialization uses the sized constructor by using an embedded null.
    // If the strlen() constructor were used, the length would be 3.
    // The size of "abc\0def" is 8 (including the terminating null). The constructor
    // calculates the length as N-1 = 7.
    CString const s_direct_null("abc\0def");
    EXPECT_EQ(7, s_direct_null.length());

    // CString should NOT be convertible from a `const char*`.
    // This is because the `const char*` constructor is explicit.
    static_assert(!std::is_convertible_v<const char*, CString>,
            "CString should not be convertible from a const char*.");

    // This demonstrates the non-convertibility. The following line would fail to compile:
    // const char* cstr = "hello";
    // CString s2 = cstr;

    // Explicit construction from a const char* should still work (via direct-initialization).
    const char* cstr = "explicit";
    CString s2(cstr);
    EXPECT_STREQ("explicit", s2.c_str());
    EXPECT_EQ(8, s2.length());
}

TEST(CString, Assignment) {
    // Copy assignment
    {
        CString s1("copy");
        CString s2;
        s2 = s1;
        EXPECT_STREQ("copy", s2.c_str());
        EXPECT_NE(s1.c_str(), s2.c_str());
    }
    // Move assignment
    {
        CString s1("move");
        const char* original_cstr = s1.c_str();
        CString s2;
        s2 = std::move(s1);
        EXPECT_STREQ("move", s2.c_str());
        EXPECT_EQ(original_cstr, s2.c_str());
        EXPECT_EQ(nullptr, s1.c_str());
    }
    // self-assignment
    {
        CString s1("self");
        const char* original_cstr = s1.c_str();
        s1 = s1;
        EXPECT_STREQ("self", s1.c_str());
        EXPECT_EQ(original_cstr, s1.c_str());
    }
}

TEST(CString, Swap) {
    CString s1("first");
    CString s2("second");

    const char* p1 = s1.c_str();
    const char* p2 = s2.c_str();
    size_t l1 = s1.length();
    size_t l2 = s2.length();

    s1.swap(s2);

    EXPECT_STREQ("second", s1.c_str());
    EXPECT_STREQ("first", s2.c_str());
    EXPECT_EQ(p2, s1.c_str());
    EXPECT_EQ(p1, s2.c_str());
    EXPECT_EQ(l2, s1.length());
    EXPECT_EQ(l1, s2.length());
}

TEST(CString, Concatenation) {
    // operator+=
    {
        CString s("hello");
        s += CString(" world");
        EXPECT_STREQ("hello world", s.c_str());
    }
    {
        CString s("hello");
        s += " world";
        EXPECT_STREQ("hello world", s.c_str());
    }
    {
        CString s("hello");
        const char* world = " world";
        s += world;
        EXPECT_STREQ("hello world", s.c_str());
    }
    {
        CString s("hello");
        s += "";
        EXPECT_STREQ("hello", s.c_str());
    }
    {
        CString s;
        s += "world";
        EXPECT_STREQ("world", s.c_str());
    }

    // operator+
    {
        CString s1("hello");
        CString s2(" world");
        CString s3 = s1 + s2;
        EXPECT_STREQ("hello world", s3.c_str());
    }
    {
        CString s1("hello");
        CString s2 = s1 + " world";
        EXPECT_STREQ("hello world", s2.c_str());
    }
    {
        CString s1(" world");
        CString s2 = "hello" + s1;
        EXPECT_STREQ("hello world", s2.c_str());
    }
    {
        CString s1("hello");
        const char* world = " world";
        CString s2 = s1 + world;
        EXPECT_STREQ("hello world", s2.c_str());
    }
    {
        CString s1(" world");
        const char* hello = "hello";
        CString s2 = hello + s1;
        EXPECT_STREQ("hello world", s2.c_str());
    }
    {
        CString s = CString("a") + CString("b") + "c" + "d";
        EXPECT_STREQ("abcd", s.c_str());
    }
}

TEST(CString, Comparison) {
    CString s1("abc");
    CString s2("abc");
    CString s3("def");
    CString s4("ab");

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

TEST(CString, ElementAccess) {
    CString str("01234");
    const CString cstr("const");

    EXPECT_EQ('0', str.front());
    EXPECT_EQ('4', str.back());
    EXPECT_EQ('c', cstr.front());
    EXPECT_EQ('t', cstr.back());

    EXPECT_EQ('2', str[2]);
    EXPECT_EQ('n', cstr[2]);

    str[0] = 'A';
    EXPECT_STREQ("A1234", str.c_str());

    EXPECT_EQ('3', str.at(3));
    EXPECT_EQ('t', cstr.at(4));

    // iterators
    std::string s(str.begin(), str.end());
    EXPECT_EQ("A1234", s);

    EXPECT_TRUE(std::equal(cstr.begin(), cstr.end(), "const"));
}

TEST(CString, Append) {
    // Append CString
    {
        CString str("foo");
        str.append(CString("bar"));
        EXPECT_STREQ("foobar", str.c_str());
    }
    // Append string literal
    {
        CString str("foo");
        str.append("bar");
        EXPECT_STREQ("foobar", str.c_str());
    }
    // Append const char*
    {
        CString str("foo");
        const char* bar = "bar";
        str.append(bar);
        EXPECT_STREQ("foobar", str.c_str());
    }
    // Append nullptr
    {
        CString str("foo");
        const char* bar = nullptr;
        str.append(bar);
        EXPECT_STREQ("foo", str.c_str());
    }
    // Append to empty
    {
        CString str;
        str.append("foo");
        EXPECT_STREQ("foo", str.c_str());
    }
    {
        CString str;
        str.append(CString("foo"));
        EXPECT_STREQ("foo", str.c_str());
    }
    {
        CString str;
        const char* foo = "foo";
        str.append(foo);
        EXPECT_STREQ("foo", str.c_str());
    }
    // Append empty
    {
        CString str("foo");
        str.append("");
        EXPECT_STREQ("foo", str.c_str());
    }
    {
        CString str("foo");
        str.append(CString(""));
        EXPECT_STREQ("foo", str.c_str());
    }
    {
        CString str("foo");
        const char* empty = "";
        str.append(empty);
        EXPECT_STREQ("foo", str.c_str());
    }
    // Chaining
    {
        CString str;
        str.append("foo").append("bar").append(CString("baz"));
        EXPECT_STREQ("foobarbaz", str.c_str());
    }
    // r-value appends
    {
        CString str = CString("foo").append("bar");
        EXPECT_STREQ("foobar", str.c_str());
    }
}

TEST(CString, Insert) {
    // with CString
    {
        CString str("foobaz");
        str.insert(3, CString("bar"));
        EXPECT_STREQ("foobarbaz", str.c_str());
    }
    // with string literal
    {
        CString str("world");
        str.insert(0, "hello ");
        EXPECT_STREQ("hello world", str.c_str());
    }
    // with const char*
    {
        CString str("foo");
        const char* bar = "bar";
        str.insert(3, bar);
        EXPECT_STREQ("foobar", str.c_str());
    }
    // with nullptr
    {
        CString str("foo");
        const char* bar = nullptr;
        str.insert(1, bar);
        EXPECT_STREQ("foo", str.c_str());
    }
    // r-value insert
    {
        const char* bar = "bar";
        CString str = CString("foo").insert(3, bar);
        EXPECT_STREQ("foobar", str.c_str());
    }
}

TEST(CString, Replace) {
    // with CString
    {
        CString str("foo bar baz");
        str.replace(4, 3, CString("dpa"));
        EXPECT_STREQ("foo dpa baz", str.c_str());
    }
    // with string literal
    {
        CString str("foo bar baz");
        str.replace(4, 3, "dpa");
        EXPECT_STREQ("foo dpa baz", str.c_str());
    }
    // with const char*
    {
        CString str("foo bar baz");
        const char* dpa = "dpa";
        str.replace(4, 3, dpa);
        EXPECT_STREQ("foo dpa baz", str.c_str());
    }
    // with nullptr
    {
        CString str("foo bar baz");
        const char* dpa = nullptr;
        str.replace(4, 3, dpa);
        EXPECT_STREQ("foo  baz", str.c_str());
    }
    // with empty string
    {
        CString str("foo bar baz");
        str.replace(4, 3, "");
        EXPECT_STREQ("foo  baz", str.c_str());
    }
    // replace that grows the string
    {
        CString str("foo bar baz");
        str.replace(4, 3, "abcdef");
        EXPECT_STREQ("foo abcdef baz", str.c_str());
    }
    // replace that shrinks the string
    {
        CString str("foo bar baz");
        str.replace(4, 3, "a");
        EXPECT_STREQ("foo a baz", str.c_str());
    }
    // replace at the beginning
    {
        CString str("foo bar baz");
        str.replace(0, 3, "lkj");
        EXPECT_STREQ("lkj bar baz", str.c_str());
    }
    // replace at the end
    {
        CString str("foo bar baz");
        str.replace(8, 3, "lkj");
        EXPECT_STREQ("foo bar lkj", str.c_str());
    }
    // replace all
    {
        CString str("foo bar baz");
        str.replace(0, 11, "lkj");
        EXPECT_STREQ("lkj", str.c_str());
    }
    // r-value replace
    {
        const char* dpa = "dpa";
        CString str = CString("foo bar baz").replace(4, 3, dpa);
        EXPECT_STREQ("foo dpa baz", str.c_str());
    }
}

TEST(CString, ReplaceInPlace) {
    // Shrinking replacement
    {
        CString str("0123456789");
        const char* const original_cstr = str.c_str();
        str.replace(3, 3, "ab");
        EXPECT_STREQ("012ab6789", str.c_str());
        EXPECT_EQ(9, str.length());
        EXPECT_EQ(original_cstr, str.c_str());
    }
    {
        CString str("0123456789");
        const char* const original_cstr = str.c_str();
        str.replace(0, 3, "ab");
        EXPECT_STREQ("ab3456789", str.c_str());
        EXPECT_EQ(9, str.length());
        EXPECT_EQ(original_cstr, str.c_str());
    }
    {
        CString str("0123456789");
        const char* const original_cstr = str.c_str();
        str.replace(7, 3, "ab");
        EXPECT_STREQ("0123456ab", str.c_str());
        EXPECT_EQ(9, str.length());
        EXPECT_EQ(original_cstr, str.c_str());
    }
    {
        CString str("0123456789");
        const char* const original_cstr = str.c_str();
        str.replace(0, 10, "ab");
        EXPECT_STREQ("ab", str.c_str());
        EXPECT_EQ(2, str.length());
        EXPECT_EQ(original_cstr, str.c_str());
    }

    // Same size replacement
    {
        CString str("0123456789");
        const char* const original_cstr = str.c_str();
        str.replace(0, 10, "abcdefghij");
        EXPECT_STREQ("abcdefghij", str.c_str());
        EXPECT_EQ(10, str.length());
        EXPECT_EQ(original_cstr, str.c_str());
    }

    // Shrink to empty
    {
        CString str("0123456789");
        const char* const original_cstr = str.c_str();
        str.replace(3, 3, "");
        EXPECT_STREQ("0126789", str.c_str());
        EXPECT_EQ(7, str.length());
        EXPECT_EQ(original_cstr, str.c_str());
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

TEST(CString, ToString) {
    EXPECT_STREQ("0", to_string(0).c_str());
    EXPECT_STREQ("123", to_string(123).c_str());
    EXPECT_STREQ("-456", to_string(-456).c_str());
    EXPECT_STREQ("2147483647", to_string(INT_MAX).c_str());
    EXPECT_STREQ("-2147483648", to_string(INT_MIN).c_str());

    EXPECT_STREQ("0", to_string(0u).c_str());
    EXPECT_STREQ("4294967295", to_string(UINT_MAX).c_str());

    EXPECT_STREQ("0", to_string((short)0).c_str());
    EXPECT_STREQ("32767", to_string(SHRT_MAX).c_str());
    EXPECT_STREQ("-32768", to_string(SHRT_MIN).c_str());

    EXPECT_STREQ("0", to_string((unsigned short)0).c_str());
    EXPECT_STREQ("65535", to_string(USHRT_MAX).c_str());

#if LONG_MAX == 2147483647
    EXPECT_STREQ("2147483647", to_string(LONG_MAX).c_str());
    EXPECT_STREQ("-2147483648", to_string(LONG_MIN).c_str());
    EXPECT_STREQ("4294967295", to_string(ULONG_MAX).c_str());
#else
    EXPECT_STREQ("9223372036854775807", to_string(LONG_MAX).c_str());
    EXPECT_STREQ("-9223372036854775808", to_string(LONG_MIN).c_str());
    EXPECT_STREQ("18446744073709551615", to_string(ULONG_MAX).c_str());
#endif

    EXPECT_STREQ("9223372036854775807", to_string(LLONG_MAX).c_str());
    EXPECT_STREQ("-9223372036854775808", to_string(LLONG_MIN).c_str());
    EXPECT_STREQ("18446744073709551615", to_string(ULLONG_MAX).c_str());

    EXPECT_STREQ("0.000000", to_string(0.0f).c_str());
    EXPECT_STREQ("1.500000", to_string(1.5f).c_str());
    EXPECT_STREQ("-3.140000", to_string(-3.14f).c_str());
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
