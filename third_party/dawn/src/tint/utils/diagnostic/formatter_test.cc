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

#include "src/tint/utils/diagnostic/formatter.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::diag {
namespace {

Diagnostic Diag(Severity severity, Source source, std::string message) {
    Diagnostic d;
    d.severity = severity;
    d.source = source;
    d.message = std::move(message);
    return d;
}

constexpr const char* ascii_content =  // Note: words are tab-delimited
    R"(the	cat	says	meow
the	dog	says	woof
the	snake	says	quack
the	snail	says	???
)";

constexpr const char* utf8_content =                      // Note: words are tab-delimited
    "the	\xf0\x9f\x90\xb1	says	meow\n"   // NOLINT: tabs
    "the	\xf0\x9f\x90\x95	says	woof\n"   // NOLINT: tabs
    "the	\xf0\x9f\x90\x8d	says	quack\n"  // NOLINT: tabs
    "the	\xf0\x9f\x90\x8c	says	???\n";   // NOLINT: tabs

class DiagFormatterTest : public testing::Test {
  public:
    Source::File ascii_file{"file.name", ascii_content};
    Source::File utf8_file{"file.name", utf8_content};
    Diagnostic ascii_diag_note =
        Diag(Severity::Note, Source{Source::Range{Source::Location{1, 14}}, &ascii_file}, "purr");
    Diagnostic ascii_diag_warn =
        Diag(Severity::Warning, Source{Source::Range{{2, 14}, {2, 18}}, &ascii_file}, "grrr");
    Diagnostic ascii_diag_err =
        Diag(Severity::Error, Source{Source::Range{{3, 16}, {3, 21}}, &ascii_file}, "hiss");

    Diagnostic utf8_diag_note =
        Diag(Severity::Note, Source{Source::Range{Source::Location{1, 15}}, &utf8_file}, "purr");
    Diagnostic utf8_diag_warn =
        Diag(Severity::Warning, Source{Source::Range{{2, 15}, {2, 19}}, &utf8_file}, "grrr");
    Diagnostic utf8_diag_err =
        Diag(Severity::Error, Source{Source::Range{{3, 15}, {3, 20}}, &utf8_file}, "hiss");
};

TEST_F(DiagFormatterTest, Simple) {
    Formatter fmt{{false, false, false, false}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(1:14: purr
2:14: grrr
3:16: hiss)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, SimpleNewlineAtEnd) {
    Formatter fmt{{false, false, false, true}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(1:14: purr
2:14: grrr
3:16: hiss
)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, SimpleNoSource) {
    Formatter fmt{{false, false, false, false}};
    auto diag = Diag(Severity::Note, Source{}, "no source!");
    auto got = fmt.Format(List{diag}).Plain();
    auto* expect = "no source!";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithFile) {
    Formatter fmt{{true, false, false, false}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(file.name:1:14: purr
file.name:2:14: grrr
file.name:3:16: hiss)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithSeverity) {
    Formatter fmt{{false, true, false, false}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(1:14 note: purr
2:14 warning: grrr
3:16 error: hiss)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithLine) {
    Formatter fmt{{false, false, true, false}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(1:14: purr
the  cat  says  meow
                ^

2:14: grrr
the  dog  says  woof
                ^^^^

3:16: hiss
the  snake  says  quack
                  ^^^^^
)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, UnicodeWithLine) {
    Formatter fmt{{false, false, true, false}};
    auto got = fmt.Format(List{utf8_diag_note, utf8_diag_warn, utf8_diag_err}).Plain();
    auto* expect =
        "1:15: purr\n"
        "the  \xf0\x9f\x90\xb1  says  meow\n"
        "\n"
        "2:15: grrr\n"
        "the  \xf0\x9f\x90\x95  says  woof\n"
        "\n"
        "3:15: hiss\n"
        "the  \xf0\x9f\x90\x8d  says  quack\n";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithFileSeverityLine) {
    Formatter fmt{{true, true, true, false}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(file.name:1:14 note: purr
the  cat  says  meow
                ^

file.name:2:14 warning: grrr
the  dog  says  woof
                ^^^^

file.name:3:16 error: hiss
the  snake  says  quack
                  ^^^^^
)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithMultiLine) {
    auto multiline =
        Diag(Severity::Warning, Source{Source::Range{{2, 9}, {4, 15}}, &ascii_file}, "multiline");
    Formatter fmt{{false, false, true, false}};
    auto got = fmt.Format(List{multiline}).Plain();
    auto* expect = R"(2:9: multiline
the  dog  says  woof
          ^^^^^^^^^^
the  snake  says  quack
^^^^^^^^^^^^^^^^^^^^^^^
the  snail  says  ???
^^^^^^^^^^^^^^^^
)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, UnicodeWithMultiLine) {
    auto multiline =
        Diag(Severity::Warning, Source{Source::Range{{2, 9}, {4, 15}}, &utf8_file}, "multiline");
    Formatter fmt{{false, false, true, false}};
    auto got = fmt.Format(List{multiline}).Plain();
    auto* expect =
        "2:9: multiline\n"
        "the  \xf0\x9f\x90\x95  says  woof\n"
        "the  \xf0\x9f\x90\x8d  says  quack\n"
        "the  \xf0\x9f\x90\x8c  says  ???\n";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithFileSeverityLineTab4) {
    Formatter fmt{{true, true, true, false, 4u}};
    auto got = fmt.Format(List{ascii_diag_note, ascii_diag_warn, ascii_diag_err}).Plain();
    auto* expect = R"(file.name:1:14 note: purr
the    cat    says    meow
                      ^

file.name:2:14 warning: grrr
the    dog    says    woof
                      ^^^^

file.name:3:16 error: hiss
the    snake    says    quack
                        ^^^^^
)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithMultiLineTab4) {
    auto multiline =
        Diag(Severity::Warning, Source{Source::Range{{2, 9}, {4, 15}}, &ascii_file}, "multiline");
    Formatter fmt{{false, false, true, false, 4u}};
    auto got = fmt.Format(List{multiline}).Plain();
    auto* expect = R"(2:9: multiline
the    dog    says    woof
              ^^^^^^^^^^^^
the    snake    says    quack
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
the    snail    says    ???
^^^^^^^^^^^^^^^^^^^^
)";
    ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, RangeOOB) {
    Formatter fmt{{true, true, true, true}};
    diag::List list;
    list.AddError(Source{{{10, 20}, {30, 20}}, &ascii_file}) << "oob";
    auto got = fmt.Format(list).Plain();
    auto* expect = R"(file.name:10:20 error: oob

)";
    ASSERT_EQ(expect, got);
}

}  // namespace
}  // namespace tint::diag
