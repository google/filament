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

#include "src/tint/utils/file/tmpfile.h"

#include <fstream>

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(TmpFileTest, WriteReadAppendDelete) {
    std::string path;
    {
        TmpFile tmp;
        if (!tmp) {
            GTEST_SKIP() << "Unable to create a temporary file";
        }

        path = tmp.Path();

        // Write a string to the temporary file
        tmp << "hello world\n";

        // Check the content of the file
        {
            std::ifstream file(path);
            ASSERT_TRUE(file);
            std::string line;
            EXPECT_TRUE(std::getline(file, line));
            EXPECT_EQ(line, "hello world");
            EXPECT_FALSE(std::getline(file, line));
        }

        // Write some more content to the file
        tmp << 42;

        // Check the content of the file again
        {
            std::ifstream file(path);
            ASSERT_TRUE(file);
            std::string line;
            EXPECT_TRUE(std::getline(file, line));
            EXPECT_EQ(line, "hello world");
            EXPECT_TRUE(std::getline(file, line));
            EXPECT_EQ(line, "42");
            EXPECT_FALSE(std::getline(file, line));
        }
    }

    // Check the file has been deleted when it fell out of scope
    std::ifstream file(path);
    ASSERT_FALSE(file);
}

TEST(TmpFileTest, FileExtension) {
    const std::string kExt = ".foo";
    std::string path;
    {
        TmpFile tmp(kExt);
        if (!tmp) {
            GTEST_SKIP() << "Unable create a temporary file";
        }
        path = tmp.Path();
    }

    ASSERT_GT(path.length(), kExt.length());
    EXPECT_EQ(kExt, path.substr(path.length() - kExt.length()));

    // Check the file has been deleted when it fell out of scope
    std::ifstream file(path);
    ASSERT_FALSE(file);
}

}  // namespace
}  // namespace tint
