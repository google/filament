// Copyright 2024 The langsvr Authors
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

#include "langsvr/content_stream.h"

#include "gtest/gtest.h"

#include "langsvr/buffer_reader.h"
#include "langsvr/buffer_writer.h"

namespace langsvr {
namespace {

TEST(ReadContent, Empty) {
    BufferReader reader("");
    auto got = ReadContent(reader);
    EXPECT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason, "EOF");
}

TEST(ReadContent, InvalidContentLength) {
    BufferReader reader("Content-Length: apples");
    auto got = ReadContent(reader);
    EXPECT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason, "invalid content length value");
}

TEST(ReadContent, MissingFirstCR) {
    BufferReader reader("Content-Length: 10\r    ");
    auto got = ReadContent(reader);
    EXPECT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason, "expected '␍␊␍␊' got '␍   '");
}

TEST(ReadContent, MissingSecondLF) {
    BufferReader reader("Content-Length: 10\r\n    ");
    auto got = ReadContent(reader);
    EXPECT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason, "expected '␍␊␍␊' got '␍␊  '");
}

TEST(ReadContent, MissingSecondCR) {
    BufferReader reader("Content-Length: 10\r\n\r    ");
    auto got = ReadContent(reader);
    EXPECT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason, "expected '␍␊␍␊' got '␍␊␍ '");
}

TEST(ReadContent, ValidMessage) {
    BufferReader reader("Content-Length: 11\r\n\r\nhello world");
    auto got = ReadContent(reader);
    EXPECT_EQ(got, Success);
    EXPECT_EQ(got.Get(), "hello world");
}

TEST(ReadContent, BufferTooShort) {
    BufferReader reader("Content-Length: 99\r\n\r\nhello world");
    auto got = ReadContent(reader);
    EXPECT_NE(got, Success);
}

TEST(ReadContent, ValidMessages) {
    BufferReader reader("Content-Length: 5\r\n\r\nhelloContent-Length: 5\r\n\r\nworld");
    {
        auto got = ReadContent(reader);
        EXPECT_EQ(got, "hello");
    }
    {
        auto got = ReadContent(reader);
        EXPECT_EQ(got, "world");
    }
}

TEST(WriteContent, Single) {
    BufferWriter writer;
    auto got = WriteContent(writer, "hello world");
    EXPECT_EQ(got, Success);
    EXPECT_EQ(writer.BufferString(), "Content-Length: 11\r\n\r\nhello world");
}

TEST(WriteContent, Multiple) {
    BufferWriter writer;
    {
        auto got = WriteContent(writer, "hello");
        EXPECT_EQ(got, Success);
        EXPECT_EQ(writer.BufferString(), "Content-Length: 5\r\n\r\nhello");
    }
    {
        auto got = WriteContent(writer, "world");
        EXPECT_EQ(got, Success);
        EXPECT_EQ(writer.BufferString(),
                  "Content-Length: 5\r\n\r\nhelloContent-Length: 5\r\n\r\nworld");
    }
}

}  // namespace
}  // namespace langsvr
