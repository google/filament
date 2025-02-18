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

#include "src/tint/utils/command/command.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

#ifdef _WIN32

TEST(CommandTest, Echo) {
    auto cmd = Command::LookPath("cmd");
    if (!cmd.Found()) {
        GTEST_SKIP() << "cmd not found on PATH";
    }

    auto res = cmd("/C", "echo", "hello world");
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "hello world\r\n");
    EXPECT_EQ(res.err, "");
}

#else

TEST(CommandTest, Echo) {
    auto cmd = Command::LookPath("echo");
    if (!cmd.Found()) {
        GTEST_SKIP() << "echo not found on PATH";
    }

    auto res = cmd("hello world");
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "hello world\n");
    EXPECT_EQ(res.err, "");
}

TEST(CommandTest, Cat) {
    auto cmd = Command::LookPath("cat");
    if (!cmd.Found()) {
        GTEST_SKIP() << "cat not found on PATH";
    }

    cmd.SetInput("hello world");
    auto res = cmd();
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "hello world");
    EXPECT_EQ(res.err, "");
}

TEST(CommandTest, True) {
    auto cmd = Command::LookPath("true");
    if (!cmd.Found()) {
        GTEST_SKIP() << "true not found on PATH";
    }

    auto res = cmd();
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "");
    EXPECT_EQ(res.err, "");
}

TEST(CommandTest, False) {
    auto cmd = Command::LookPath("false");
    if (!cmd.Found()) {
        GTEST_SKIP() << "false not found on PATH";
    }

    auto res = cmd();
    EXPECT_NE(res.error_code, 0);
    EXPECT_EQ(res.out, "");
    EXPECT_EQ(res.err, "");
}

#endif

}  // namespace
}  // namespace tint
