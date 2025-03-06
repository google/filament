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

#include <gtest/gtest.h>
#include <sstream>
#include <string_view>

#include "gmock/gmock.h"

#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/lsp/printer.h"
#include "src/tint/lang/wgsl/ls/helpers_test.h"

namespace tint::wgsl::ls {
namespace {

namespace lsp = langsvr::lsp;

std::vector<lsp::SignatureInformation> MaxSignatures() {
    std::vector<lsp::SignatureInformation> out;

    {
        std::vector<lsp::ParameterInformation> parameters;
        parameters.push_back(lsp::ParameterInformation{
            /* label */ lsp::String("param-0"),
            /* documentation */ {},
        });
        parameters.push_back(lsp::ParameterInformation{
            /* label */ lsp::String("param-1"),
            /* documentation */ {},
        });

        lsp::MarkupContent documentation;
        documentation.kind = lsp::MarkupKind::kMarkdown;
        documentation.value =
            R"(
`T` is `abstract-float`, `abstract-int`, `f32`, `i32`, `u32` or `f16`)";

        lsp::SignatureInformation sig{};
        sig.label = "max(T, T) -> T";
        sig.documentation = documentation;
        sig.parameters = std::move(parameters);

        out.push_back(std::move(sig));
    }

    {
        std::vector<lsp::ParameterInformation> parameters;
        parameters.push_back(lsp::ParameterInformation{
            /* label */ lsp::String("param-0"),
            /* documentation */ {},
        });
        parameters.push_back(lsp::ParameterInformation{
            /* label */ lsp::String("param-1"),
            /* documentation */ {},
        });

        lsp::MarkupContent documentation;
        documentation.kind = lsp::MarkupKind::kMarkdown;
        documentation.value =
            R"(
`T` is `abstract-float`, `abstract-int`, `f32`, `i32`, `u32` or `f16`)";

        lsp::SignatureInformation sig{};
        sig.label = R"(max(vecN<T>, vecN<T>) -> vecN<T>)";
        sig.documentation = documentation;
        sig.parameters = std::move(parameters);

        out.push_back(std::move(sig));
    }

    return out;
}

struct Case {
    std::string_view markup;
    std::vector<lsp::SignatureInformation> signatures;
    lsp::Uinteger active_signature = 0;
    lsp::Uinteger active_parameter = 0;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

using LsSignatureHelpTest = LsTestWithParam<Case>;
TEST_P(LsSignatureHelpTest, SignatureHelp) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.ranges.size(), 0u);
    ASSERT_EQ(parsed.positions.size(), 1u);

    lsp::TextDocumentSignatureHelpRequest req{};
    req.text_document.uri = OpenDocument(parsed.clean);
    req.position = parsed.positions[0];

    for (auto& n : diagnostics_) {
        for (auto& d : n.diagnostics) {
            if (d.severity == lsp::DiagnosticSeverity::kError) {
                FAIL() << "Error: " << d.message << "\nWGSL:\n" << parsed.clean;
            }
        }
    }

    auto future = client_session_.Send(req);
    ASSERT_EQ(future, langsvr::Success);
    auto res = future->get();
    if (GetParam().signatures.empty()) {
        ASSERT_TRUE(res.Is<lsp::Null>());
    } else {
        ASSERT_TRUE(res.Is<lsp::SignatureHelp>());
        auto& got = *res.Get<lsp::SignatureHelp>();
        EXPECT_EQ(got.signatures, GetParam().signatures);
        EXPECT_EQ(got.active_signature, GetParam().active_signature);
        EXPECT_EQ(got.active_parameter, GetParam().active_parameter);
    }
}

INSTANTIATE_TEST_SUITE_P(,
                         LsSignatureHelpTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {
                                 R"(
const C = max(⧘1, 2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C : i32 = max(⧘1, 2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(⧘1, 2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(1⧘, 2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(1,⧘ 2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max(1, 2⧘);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max(⧘ vec2(1), vec2(2));
)",
                                 MaxSignatures(),
                                 /* active_signature */ 1,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(⧘ vec3(1), vec3(2));
)",
                                 MaxSignatures(),
                                 /* active_signature */ 1,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(vec4(1) ⧘, vec4(2));
)",
                                 MaxSignatures(),
                                 /* active_signature */ 1,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(vec2(1),⧘ vec2(2));
)",
                                 MaxSignatures(),
                                 /* active_signature */ 1,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max(vec3(1), vec3(2) ⧘);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 1,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = min(max(1, ⧘2), max(3, 4));
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
fn f() {
    let x = max(1, ⧘2);
}
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max( (1 + (2⧘) * 3), 4);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(1, (2 + (3⧘) * 4));
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max( array(1,2,3)[1⧘], 2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(1, array(1,2,3)[⧘2]);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max(1
              ⧘
              ,
              2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 0,
                             },  // =========================================
                             {
                                 R"(
const C = max(1
              ,
              ⧘
              2);
)",
                                 MaxSignatures(),
                                 /* active_signature */ 0,
                                 /* active_parameter */ 1,
                             },  // =========================================
                             {
                                 R"(
const C = max(1, 2) ⧘;
)",
                                 {},
                             },
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
