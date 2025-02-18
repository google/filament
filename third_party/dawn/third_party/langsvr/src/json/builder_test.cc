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

#include "gtest/gtest.h"

#include "src/utils/replace_all.h"

namespace langsvr::json {
namespace {

TEST(JsonBuilder, ParseNull) {
    auto b = Builder::Create();
    auto null_res = b->Parse("null");
    ASSERT_EQ(null_res, Success);

    auto& null = null_res.Get();
    EXPECT_EQ(null->Kind(), json::Kind::kNull);
    EXPECT_EQ(null->Json(), "null");
}

TEST(JsonBuilder, ParseBool) {
    auto b = Builder::Create();
    auto true_res = b->Parse("true");
    ASSERT_EQ(true_res, Success);

    auto& true_ = true_res.Get();
    EXPECT_EQ(true_->Bool(), true);
    EXPECT_EQ(true_->Kind(), json::Kind::kBool);
    EXPECT_EQ(true_->Json(), "true");
}

TEST(JsonBuilder, ParseI64) {
    auto b = Builder::Create();
    auto i64_res = b->Parse("9223372036854775807");
    ASSERT_EQ(i64_res, Success);

    auto& i64 = i64_res.Get();
    EXPECT_EQ(i64->I64(), static_cast<json::I64>(9223372036854775807));
    EXPECT_EQ(i64->Kind(), json::Kind::kI64);
    EXPECT_EQ(i64->Json(), "9223372036854775807");
}

TEST(JsonBuilder, ParseU64) {
    auto b = Builder::Create();
    auto u64_res = b->Parse("9223372036854775808");
    ASSERT_EQ(u64_res, Success);

    auto& u64 = u64_res.Get();
    EXPECT_EQ(u64->U64(), static_cast<json::U64>(9223372036854775808u));
    EXPECT_EQ(u64->Kind(), json::Kind::kU64);
    EXPECT_EQ(u64->Json(), "9223372036854775808");
}

TEST(JsonBuilder, ParseF64) {
    auto b = Builder::Create();
    auto f64_res = b->Parse("42.0");
    ASSERT_EQ(f64_res, Success);

    auto& f64 = f64_res.Get();
    EXPECT_EQ(f64->F64().Get(), 42.0);
    EXPECT_EQ(f64->Kind(), json::Kind::kF64);
    EXPECT_EQ(f64->Json(), "42.0");
}

TEST(JsonBuilder, ParseString) {
    auto b = Builder::Create();
    auto string_res = b->Parse("\"hello world\"");
    ASSERT_EQ(string_res, Success);

    auto& string_ = string_res.Get();
    EXPECT_EQ(string_->String(), "hello world");
    EXPECT_EQ(string_->Kind(), json::Kind::kString);
    EXPECT_EQ(string_->Json(), "\"hello world\"");
}

TEST(JsonBuilder, ParseArray) {
    auto b = Builder::Create();
    auto arr_res = b->Parse("[10, false, \"fish\" ]");
    ASSERT_EQ(arr_res, Success);

    auto& arr = arr_res.Get();
    EXPECT_EQ(arr->Kind(), json::Kind::kArray);
    EXPECT_EQ(ReplaceAll(arr->Json(), " ", ""), "[10,false,\"fish\"]");
    EXPECT_EQ(arr->Count(), 3u);

    EXPECT_EQ(arr->Get<json::I64>(0u), static_cast<json::I64>(10));
    EXPECT_EQ(arr->Get<json::Bool>(1u), false);
    EXPECT_EQ(arr->Get<json::String>(2u), "fish");

    auto oob = arr->Get(3);
    EXPECT_NE(oob, Success);
}

TEST(JsonBuilder, ParseObject) {
    auto b = Builder::Create();
    auto root_res = b->Parse(R"({"cat": "meow", "ten": 10, "yes": true})");
    ASSERT_EQ(root_res, Success);

    auto& root = root_res.Get();
    EXPECT_EQ(root->Kind(), json::Kind::kObject);
    EXPECT_EQ(ReplaceAll(root->Json(), " ", ""), R"({"cat":"meow","ten":10,"yes":true})");
    EXPECT_EQ(root->Count(), 3u);

    EXPECT_EQ(root->Get<json::String>("cat"), "meow");
    EXPECT_EQ(root->Get<json::I64>("ten"), static_cast<json::I64>(10));
    EXPECT_EQ(root->Get<json::Bool>("yes"), true);

    auto missing = root->Get("missing");
    EXPECT_NE(missing, Success);
}

TEST(JsonBuilder, CreateNull) {
    auto b = Builder::Create();
    auto v = b->Null();
    EXPECT_EQ(v->Kind(), json::Kind::kNull);
    EXPECT_EQ(v->Json(), "null");
}

TEST(JsonBuilder, CreateBool) {
    auto b = Builder::Create();
    auto v = b->Bool(true);
    EXPECT_EQ(v->Kind(), json::Kind::kBool);
    EXPECT_EQ(v->Json(), "true");
}

TEST(JsonBuilder, CreateI64) {
    auto b = Builder::Create();
    auto v = b->I64(static_cast<json::I64>(9223372036854775807));
    EXPECT_EQ(v->Kind(), json::Kind::kI64);
    EXPECT_EQ(v->Json(), "9223372036854775807");
}

TEST(JsonBuilder, CreateU64) {
    auto b = Builder::Create();
    auto v = b->U64(static_cast<json::U64>(9223372036854775808ul));
    EXPECT_EQ(v->Kind(), json::Kind::kU64);
    EXPECT_EQ(v->Json(), "9223372036854775808");
}

TEST(JsonBuilder, CreateF64) {
    auto b = Builder::Create();
    auto v = b->F64(static_cast<json::F64>(42.0));
    EXPECT_EQ(v->Kind(), json::Kind::kF64);
    EXPECT_EQ(v->Json(), "42.0");
}

TEST(JsonBuilder, CreateString) {
    auto b = Builder::Create();
    auto v = b->String("hello world");
    EXPECT_EQ(v->Kind(), json::Kind::kString);
    EXPECT_EQ(v->Json(), "\"hello world\"");
}

TEST(JsonBuilder, CreateArray) {
    auto b = Builder::Create();
    std::vector elements{
        b->I64(10),
        b->Bool(false),
        b->String("fish"),
    };
    auto v = b->Array(elements);
    EXPECT_EQ(v->Kind(), json::Kind::kArray);
    EXPECT_EQ(ReplaceAll(v->Json(), " ", ""), R"([10,false,"fish"])");
}

TEST(JsonBuilder, CreateObject) {
    auto b = Builder::Create();
    std::vector members{
        Builder::Member{"cat", b->String("meow")},
        Builder::Member{"ten", b->I64(10)},
        Builder::Member{"yes", b->Bool(true)},
    };
    auto v = b->Object(members);
    EXPECT_EQ(v->Kind(), json::Kind::kObject);
    EXPECT_EQ(ReplaceAll(v->Json(), " ", ""), R"({"cat":"meow","ten":10,"yes":true})");
}

}  // namespace
}  // namespace langsvr::json
