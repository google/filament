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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

const diag::Formatter::Style formatter_style{/* print_file: */ true, /* print_severity: */ true,
                                             /* print_line: */ true,
                                             /* print_newline_at_end: */ false};

class ParserImplErrorResyncTest : public WGSLParserTest {};

#define EXPECT(SOURCE, EXPECTED)                                                           \
    do {                                                                                   \
        std::string source = SOURCE;                                                       \
        std::string expected = EXPECTED;                                                   \
        auto p = parser(source);                                                           \
        EXPECT_EQ(false, p->Parse());                                                      \
        auto diagnostics = p->builder().Diagnostics();                                     \
        EXPECT_EQ(true, diagnostics.ContainsErrors());                                     \
        EXPECT_EQ(expected, diag::Formatter(formatter_style).Format(diagnostics).Plain()); \
    } while (false)

TEST_F(ParserImplErrorResyncTest, BadFunctionDecls) {
    EXPECT(R"(
fn .() -> . {}
fn x(.) {}
@_ fn -> {}
fn good() {}
)",
           R"(test.wgsl:2:4 error: expected identifier for function declaration
fn .() -> . {}
   ^

test.wgsl:2:11 error: unable to determine function return type
fn .() -> . {}
          ^

test.wgsl:3:6 error: expected ')' for function declaration
fn x(.) {}
     ^

test.wgsl:4:2 error: expected attribute
Possible values: 'align', 'binding', 'blend_src', 'builtin', 'color', 'compute', 'diagnostic', 'fragment', 'group', 'id', 'input_attachment_index', 'interpolate', 'invariant', 'location', 'must_use', 'size', 'vertex', 'workgroup_size'
@_ fn -> {}
 ^
)");
}

TEST_F(ParserImplErrorResyncTest, AssignmentStatement) {
    EXPECT(R"(
fn f() {
  blah blah blah blah;
  good = 1;
  blah blah blah blah;
  x = .;
  good = 1;
}
)",
           R"(test.wgsl:3:8 error: expected '=' for assignment
  blah blah blah blah;
       ^^^^

test.wgsl:5:8 error: expected '=' for assignment
  blah blah blah blah;
       ^^^^

test.wgsl:6:7 error: unable to parse right side of assignment
  x = .;
      ^
)");
}

TEST_F(ParserImplErrorResyncTest, DiscardStatement) {
    EXPECT(R"(
fn f() {
  discard blah blah blah;
  a = 1;
  discard blah blah blah;
}
)",
           R"(test.wgsl:3:11 error: expected ';' for discard statement
  discard blah blah blah;
          ^^^^

test.wgsl:5:11 error: expected ';' for discard statement
  discard blah blah blah;
          ^^^^
)");
}

TEST_F(ParserImplErrorResyncTest, StructMembers) {
    EXPECT(R"(
struct S {
    blah blah blah,
    a : i32,
    blah blah blah,
    b : i32,
    @- x : i32,
    c : i32,
}
)",
           R"(test.wgsl:3:10 error: expected ':' for struct member
    blah blah blah,
         ^^^^

test.wgsl:5:10 error: expected ':' for struct member
    blah blah blah,
         ^^^^

test.wgsl:7:6 error: expected attribute
Possible values: 'align', 'binding', 'blend_src', 'builtin', 'color', 'compute', 'diagnostic', 'fragment', 'group', 'id', 'input_attachment_index', 'interpolate', 'invariant', 'location', 'must_use', 'size', 'vertex', 'workgroup_size'
    @- x : i32,
     ^
)");
}

// Check that the forward scan in resynchronize() stop at nested sync points.
// In this test the inner resynchronize() is looking for a terminating ';', and
// the outer resynchronize() is looking for a terminating '}' for the function
// scope.
TEST_F(ParserImplErrorResyncTest, NestedSyncPoints) {
    EXPECT(R"(
fn f() {
  x = 1;
  discard
}
struct S { blah };
)",
           R"(test.wgsl:5:1 error: expected ';' for discard statement
}
^

test.wgsl:6:17 error: expected ':' for struct member
struct S { blah };
                ^
)");
}

TEST_F(ParserImplErrorResyncTest, BracketCounting) {
    EXPECT(
        R"(
fn f(x(((())))) {
  meow = {{{}}}
}
struct S { blah };
)",
        R"(test.wgsl:2:7 error: expected ':' for parameter
fn f(x(((())))) {
      ^

test.wgsl:3:10 error: unable to parse right side of assignment
  meow = {{{}}}
         ^

test.wgsl:5:17 error: expected ':' for struct member
struct S { blah };
                ^
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
