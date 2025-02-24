// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

struct VariableStorageData {
    const char* input;
    core::AddressSpace address_space;
    core::Access access;
};
inline std::ostream& operator<<(std::ostream& out, VariableStorageData data) {
    out << std::string(data.input);
    return out;
}

class VariableQualifierTest : public WGSLParserTestWithParam<VariableStorageData> {};

TEST_P(VariableQualifierTest, ParsesAddressSpace) {
    auto params = GetParam();
    auto p = parser(std::string("var<") + params.input + "> name");

    auto sc = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_TRUE(sc.matched);
    if (params.address_space != core::AddressSpace::kUndefined) {
        ast::CheckIdentifier(sc->address_space, tint::ToString(params.address_space));
    } else {
        EXPECT_EQ(sc->address_space, nullptr);
    }
    if (params.access != core::Access::kUndefined) {
        ast::CheckIdentifier(sc->access, tint::ToString(params.access));
    } else {
        EXPECT_EQ(sc->access, nullptr);
    }

    auto& t = p->next();
    EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    WGSLParserTest,
    VariableQualifierTest,
    testing::Values(
        VariableStorageData{"uniform", core::AddressSpace::kUniform, core::Access::kUndefined},
        VariableStorageData{"workgroup", core::AddressSpace::kWorkgroup, core::Access::kUndefined},
        VariableStorageData{"storage", core::AddressSpace::kStorage, core::Access::kUndefined},
        VariableStorageData{"private", core::AddressSpace::kPrivate, core::Access::kUndefined},
        VariableStorageData{"function", core::AddressSpace::kFunction, core::Access::kUndefined},
        VariableStorageData{"storage, read", core::AddressSpace::kStorage, core::Access::kRead},
        VariableStorageData{"storage, write", core::AddressSpace::kStorage, core::Access::kWrite},
        VariableStorageData{"storage, read_write", core::AddressSpace::kStorage,
                            core::Access::kReadWrite}));

TEST_F(WGSLParserTest, VariableQualifier_Empty) {
    auto p = parser("var<> name");
    auto sc = p->variable_decl();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), R"(1:5: expected expression for 'var' address space)");
}

TEST_F(WGSLParserTest, VariableQualifier_MissingLessThan) {
    auto p = parser("private>");
    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(sc.matched);

    auto& t = p->next();
    ASSERT_TRUE(t.Is(Token::Type::kIdentifier));
}

TEST_F(WGSLParserTest, VariableQualifier_MissingLessThan_AfterSC) {
    auto p = parser("private, >");
    auto sc = p->variable_qualifier();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(sc.errored);
    EXPECT_FALSE(sc.matched);

    auto& t = p->next();
    ASSERT_TRUE(t.Is(Token::Type::kIdentifier));
}

TEST_F(WGSLParserTest, VariableQualifier_MissingGreaterThan) {
    auto p = parser("<private");
    auto sc = p->variable_qualifier();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(sc.errored);
    EXPECT_FALSE(sc.matched);
    EXPECT_EQ(p->error(), "1:1: missing closing '>' for variable declaration");
}

}  // namespace
}  // namespace tint::wgsl::reader
