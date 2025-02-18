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

#include "langsvr/json/builder.h"
#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/printer.h"

#include "gmock/gmock.h"

namespace langsvr::lsp {
namespace {

TEST(DecodeTest, ShowDocumentParams) {
    auto b = json::Builder::Create();
    auto parse_res = b->Parse(
        R"({"selection":{"end":{"character":4,"line":3},"start":{"character":2,"line":1}},"uri":"file.txt"})");
    EXPECT_EQ(parse_res, Success);

    ShowDocumentParams got;
    auto decode_res = Decode(*parse_res.Get(), got);
    EXPECT_EQ(decode_res, Success);

    ShowDocumentParams expected;
    expected.uri = "file.txt";
    expected.selection = Range{{1, 2}, {3, 4}};
    EXPECT_EQ(got, expected);
}

TEST(DecodeTest, ErrNullStruct) {
    auto b = json::Builder::Create();
    auto parse_res = b->Parse("null");
    EXPECT_EQ(parse_res, Success);

    SemanticTokensFullDelta got;  // Note: all fields are optional
    auto decode_res = Decode(*parse_res.Get(), got);
    EXPECT_NE(decode_res, Success);
}

TEST(DecodeTest, ErrNumberStruct) {
    auto b = json::Builder::Create();
    auto parse_res = b->Parse("42");
    EXPECT_EQ(parse_res, Success);

    SemanticTokensFullDelta got;  // Note: all fields are optional
    auto decode_res = Decode(*parse_res.Get(), got);
    EXPECT_NE(decode_res, Success);
}

}  // namespace
}  // namespace langsvr::lsp
