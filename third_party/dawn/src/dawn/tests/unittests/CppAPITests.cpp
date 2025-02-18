// Copyright 2024 The Dawn & Tint Authors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dawn/native/dawn_platform.h"

namespace dawn::native {
namespace {

// Test that default construction or assignment to wgpu::StringView produces the nil string.
TEST(CppAPITests, StringViewDefault) {
    {
        wgpu::StringView s;
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s{};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = {};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = wgpu::StringView();
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that resetting the string, clears both data and length.
    std::string_view sv("hello world!");
    {
        wgpu::StringView s(sv);
        s = {};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s(sv);
        s = wgpu::StringView();
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that construction or assignment to wgpu::StringView from const char*.
TEST(CppAPITests, StringViewFromCstr) {
    {
        wgpu::StringView s("hello world!");
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s{"hello world!"};
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = {"hello world!"};
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = wgpu::StringView("hello world!");
        EXPECT_STREQ(s.data, "hello world!");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that setting to a cstr clears the length.
    std::string_view sv("hello world!");
    {
        wgpu::StringView s(sv);
        s = "other str";
        EXPECT_STREQ(s.data, "other str");
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that construction or assignment to wgpu::StringView from std::string_view
TEST(CppAPITests, StringViewFromStdStringView) {
    std::string_view sv("hello\x00world!");
    {
        wgpu::StringView s(sv);
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::StringView s{sv};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::StringView s = {sv};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::StringView s = wgpu::StringView(sv);
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
}

// Test that construction or assignment to wgpu::StringView from pointer and length
TEST(CppAPITests, StringViewFromPtrAndLength) {
    std::string_view sv("hello\x00world!");
    {
        wgpu::StringView s(sv.data(), sv.length());
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::StringView s{sv.data(), sv.length()};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::StringView s = {sv.data(), sv.length()};
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
    {
        wgpu::StringView s = wgpu::StringView(sv.data(), sv.length());
        EXPECT_EQ(s.data, sv.data());
        EXPECT_EQ(s.length, sv.length());
    }
}

// Test that construction or assignment to wgpu::StringView from nullptr
TEST(CppAPITests, StringViewFromNullptr) {
    {
        wgpu::StringView s(nullptr);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s{nullptr};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = {nullptr};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = wgpu::StringView(nullptr);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that setting to nullptr, clears both data and length.
    std::string_view sv("hello world!");
    {
        wgpu::StringView s(sv);
        s = nullptr;
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that construction or assignment to wgpu::StringView from std::nullopt
TEST(CppAPITests, StringViewFromNullopt) {
    {
        wgpu::StringView s(std::nullopt);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s{std::nullopt};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = {std::nullopt};
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
    {
        wgpu::StringView s = wgpu::StringView(std::nullopt);
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }

    // Test that setting to std::nullopt, clears both data and length.
    std::string_view sv("hello world!");
    {
        wgpu::StringView s(sv);
        s = std::nullopt;
        EXPECT_EQ(s.data, nullptr);
        EXPECT_EQ(s.length, WGPU_STRLEN);
    }
}

// Test that .IsUndefined() is only for {nullptr, WGPU_STRLEN}.
TEST(CppAPITests, StringViewIsUndefined) {
    {
        wgpu::StringView s(nullptr, wgpu::kStrlen);
        ASSERT_TRUE(s.IsUndefined());
    }

    // Nothing else is "undefined".
    {
        wgpu::StringView s("woot", wgpu::kStrlen);
        ASSERT_FALSE(s.IsUndefined());
    }
    {
        wgpu::StringView s(nullptr, 0);
        ASSERT_FALSE(s.IsUndefined());
    }
    {
        wgpu::StringView s("woot", 0);
        ASSERT_FALSE(s.IsUndefined());
    }
    {
        wgpu::StringView s("woot", 3);
        ASSERT_FALSE(s.IsUndefined());
    }
}

// Test implicit conversion to std::string_view
TEST(CppAPITests, StringViewStdStringViewConversion) {
    {
        wgpu::StringView s(nullptr, wgpu::kStrlen);
        ASSERT_EQ("", std::string_view(s));
    }
    {
        wgpu::StringView s("woot", wgpu::kStrlen);
        ASSERT_EQ("woot", std::string_view(s));
    }
    {
        wgpu::StringView s(nullptr, 0);
        ASSERT_EQ("", std::string_view(s));
    }
    {
        wgpu::StringView s("woot", 0);
        ASSERT_EQ("", std::string_view(s));
    }
    {
        wgpu::StringView s("woot", 3);
        ASSERT_EQ("woo", std::string_view(s));
    }
}

}  // anonymous namespace
}  // namespace dawn::native
