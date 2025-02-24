// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/text/string.h"

#include "gmock/gmock.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"

#include "src/tint/utils/containers/transform.h"  // Used by ToStringList()

namespace tint {
namespace {

// Workaround for https://github.com/google/googletest/issues/3081
// Remove when using C++20
template <size_t N>
Vector<std::string, N> ToStringList(const Vector<std::string_view, N>& views) {
    return Transform(views, [](std::string_view view) { return std::string(view); });
}

TEST(StringTest, ReplaceAll) {
    EXPECT_EQ("xybbcc", ReplaceAll("aabbcc", "aa", "xy"));
    EXPECT_EQ("aaxycc", ReplaceAll("aabbcc", "bb", "xy"));
    EXPECT_EQ("aabbxy", ReplaceAll("aabbcc", "cc", "xy"));
    EXPECT_EQ("xyxybbcc", ReplaceAll("aabbcc", "a", "xy"));
    EXPECT_EQ("aaxyxycc", ReplaceAll("aabbcc", "b", "xy"));
    EXPECT_EQ("aabbxyxy", ReplaceAll("aabbcc", "c", "xy"));
    // Replacement string includes the searched-for string.
    // This proves that the algorithm needs to advance 'pos'
    // past the replacement.
    EXPECT_EQ("aabxybbxybcc", ReplaceAll("aabbcc", "b", "bxyb"));
}

TEST(StringTest, ToString) {
    EXPECT_EQ("true", ToString(true));
    EXPECT_EQ("false", ToString(false));
    EXPECT_EQ("123", ToString(123));
    EXPECT_EQ("hello", ToString("hello"));
}

TEST(StringTest, HasPrefix) {
    EXPECT_TRUE(HasPrefix("abc", "a"));
    EXPECT_TRUE(HasPrefix("abc", "ab"));
    EXPECT_TRUE(HasPrefix("abc", "abc"));
    EXPECT_FALSE(HasPrefix("abc", "abc1"));
    EXPECT_FALSE(HasPrefix("abc", "ac"));
    EXPECT_FALSE(HasPrefix("abc", "b"));
}

TEST(StringTest, HasSuffix) {
    EXPECT_TRUE(HasSuffix("abc", "c"));
    EXPECT_TRUE(HasSuffix("abc", "bc"));
    EXPECT_TRUE(HasSuffix("abc", "abc"));
    EXPECT_FALSE(HasSuffix("abc", "1abc"));
    EXPECT_FALSE(HasSuffix("abc", "ac"));
    EXPECT_FALSE(HasSuffix("abc", "b"));
}

TEST(StringTest, Distance) {
    EXPECT_EQ(Distance("hello world", "hello world"), 0u);
    EXPECT_EQ(Distance("hello world", "helloworld"), 1u);
    EXPECT_EQ(Distance("helloworld", "hello world"), 1u);
    EXPECT_EQ(Distance("hello world", "hello  world"), 1u);
    EXPECT_EQ(Distance("hello  world", "hello world"), 1u);
    EXPECT_EQ(Distance("Hello World", "hello world"), 2u);
    EXPECT_EQ(Distance("hello world", "Hello World"), 2u);
    EXPECT_EQ(Distance("Hello world", ""), 11u);
    EXPECT_EQ(Distance("", "Hello world"), 11u);
}

TEST(StringTest, SuggestAlternatives) {
    {
        std::string_view alternatives[] = {"hello world", "Hello World"};
        StyledText ss;
        SuggestAlternatives("hello wordl", alternatives, ss);
        EXPECT_EQ(ss.Plain(), R"(Did you mean 'hello world'?
Possible values: 'hello world', 'Hello World')");
    }
    {
        std::string_view alternatives[] = {"foobar", "something else"};
        StyledText ss;
        SuggestAlternatives("hello world", alternatives, ss);
        EXPECT_EQ(ss.Plain(), R"(Possible values: 'foobar', 'something else')");
    }
    {
        std::string_view alternatives[] = {"hello world", "Hello World"};
        StyledText ss;
        SuggestAlternativeOptions opts;
        opts.prefix = "$";
        SuggestAlternatives("hello wordl", alternatives, ss, opts);
        EXPECT_EQ(ss.Plain(), R"(Did you mean '$hello world'?
Possible values: '$hello world', '$Hello World')");
    }
    {
        std::string_view alternatives[] = {"hello world", "Hello World"};
        StyledText ss;
        SuggestAlternativeOptions opts;
        opts.list_possible_values = false;
        SuggestAlternatives("hello world", alternatives, ss, opts);
        EXPECT_EQ(ss.Plain(), R"(Did you mean 'hello world'?)");
    }
}

TEST(StringTest, TrimLeft) {
    EXPECT_EQ(TrimLeft("hello world", [](char) { return false; }), "hello world");
    EXPECT_EQ(TrimLeft("hello world", [](char c) { return c == 'h'; }), "ello world");
    EXPECT_EQ(TrimLeft("hello world", [](char c) { return c == 'h' || c == 'e'; }), "llo world");
    EXPECT_EQ(TrimLeft("hello world", [](char c) { return c == 'e'; }), "hello world");
    EXPECT_EQ(TrimLeft("hello world", [](char) { return true; }), "");
    EXPECT_EQ(TrimLeft("", [](char) { return false; }), "");
    EXPECT_EQ(TrimLeft("", [](char) { return true; }), "");
}

TEST(StringTest, TrimRight) {
    EXPECT_EQ(TrimRight("hello world", [](char) { return false; }), "hello world");
    EXPECT_EQ(TrimRight("hello world", [](char c) { return c == 'd'; }), "hello worl");
    EXPECT_EQ(TrimRight("hello world", [](char c) { return c == 'd' || c == 'l'; }), "hello wor");
    EXPECT_EQ(TrimRight("hello world", [](char c) { return c == 'l'; }), "hello world");
    EXPECT_EQ(TrimRight("hello world", [](char) { return true; }), "");
    EXPECT_EQ(TrimRight("", [](char) { return false; }), "");
    EXPECT_EQ(TrimRight("", [](char) { return true; }), "");
}

TEST(StringTest, TrimPrefix) {
    EXPECT_EQ(TrimPrefix("abc", "a"), "bc");
    EXPECT_EQ(TrimPrefix("abc", "ab"), "c");
    EXPECT_EQ(TrimPrefix("abc", "abc"), "");
    EXPECT_EQ(TrimPrefix("abc", "abc1"), "abc");
    EXPECT_EQ(TrimPrefix("abc", "ac"), "abc");
    EXPECT_EQ(TrimPrefix("abc", "b"), "abc");
    EXPECT_EQ(TrimPrefix("abc", "c"), "abc");
}

TEST(StringTest, TrimSuffix) {
    EXPECT_EQ(TrimSuffix("abc", "c"), "ab");
    EXPECT_EQ(TrimSuffix("abc", "bc"), "a");
    EXPECT_EQ(TrimSuffix("abc", "abc"), "");
    EXPECT_EQ(TrimSuffix("abc", "1abc"), "abc");
    EXPECT_EQ(TrimSuffix("abc", "ac"), "abc");
    EXPECT_EQ(TrimSuffix("abc", "b"), "abc");
    EXPECT_EQ(TrimSuffix("abc", "a"), "abc");
}

TEST(StringTest, Trim) {
    EXPECT_EQ(Trim("hello world", [](char) { return false; }), "hello world");
    EXPECT_EQ(Trim("hello world", [](char c) { return c == 'h'; }), "ello world");
    EXPECT_EQ(Trim("hello world", [](char c) { return c == 'd'; }), "hello worl");
    EXPECT_EQ(Trim("hello world", [](char c) { return c == 'h' || c == 'd'; }), "ello worl");
    EXPECT_EQ(Trim("hello world", [](char) { return true; }), "");
    EXPECT_EQ(Trim("", [](char) { return false; }), "");
    EXPECT_EQ(Trim("", [](char) { return true; }), "");
}

TEST(StringTest, IsSpace) {
    EXPECT_FALSE(IsSpace('a'));
    EXPECT_FALSE(IsSpace('z'));
    EXPECT_FALSE(IsSpace('\0'));
    EXPECT_TRUE(IsSpace(' '));
    EXPECT_TRUE(IsSpace('\f'));
    EXPECT_TRUE(IsSpace('\n'));
    EXPECT_TRUE(IsSpace('\r'));
    EXPECT_TRUE(IsSpace('\t'));
    EXPECT_TRUE(IsSpace('\v'));
}

TEST(StringTest, TrimSpace) {
    EXPECT_EQ(TrimSpace("hello world"), "hello world");
    EXPECT_EQ(TrimSpace(" \t hello world\v\f"), "hello world");
    EXPECT_EQ(TrimSpace("hello \t world"), "hello \t world");
    EXPECT_EQ(TrimSpace(""), "");
}

TEST(StringTest, Quote) {
    EXPECT_EQ("'meow'", Quote("meow"));
}

TEST(StringTest, Split) {
    EXPECT_THAT(ToStringList(Split("", ",")), testing::ElementsAre(""));
    EXPECT_THAT(ToStringList(Split("cat", ",")), testing::ElementsAre("cat"));
    EXPECT_THAT(ToStringList(Split("cat,", ",")), testing::ElementsAre("cat", ""));
    EXPECT_THAT(ToStringList(Split(",cat", ",")), testing::ElementsAre("", "cat"));
    EXPECT_THAT(ToStringList(Split("cat,dog,fish", ",")),
                testing::ElementsAre("cat", "dog", "fish"));
    EXPECT_THAT(ToStringList(Split("catdogfish", "dog")), testing::ElementsAre("cat", "fish"));
}

TEST(StringTest, Join) {
    EXPECT_EQ(Join(Vector<int, 1>{}, ","), "");
    EXPECT_EQ(Join(Vector{1, 2, 3}, ","), "1,2,3");
    EXPECT_EQ(Join(Vector{"cat"}, ","), "cat");
    EXPECT_EQ(Join(Vector{"cat", "dog"}, ","), "cat,dog");
}

}  // namespace
}  // namespace tint
