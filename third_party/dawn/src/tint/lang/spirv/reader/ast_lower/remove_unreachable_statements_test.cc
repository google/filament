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

#include "src/tint/lang/spirv/reader/ast_lower/remove_unreachable_statements.h"

#include "src/tint/lang/spirv/reader/ast_lower/helper_test.h"

namespace tint::spirv::reader {
namespace {

using RemoveUnreachableStatementsTest = ast::transform::TransformTest;

TEST_F(RemoveUnreachableStatementsTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<RemoveUnreachableStatements>(src));
}

TEST_F(RemoveUnreachableStatementsTest, ShouldRunHasNoUnreachable) {
    auto* src = R"(
fn f() {
  if (true) {
    var x = 1;
  }
}
)";

    EXPECT_FALSE(ShouldRun<RemoveUnreachableStatements>(src));
}

TEST_F(RemoveUnreachableStatementsTest, ShouldRunHasUnreachable) {
    auto* src = R"(
fn f() {
  return;
  if (true) {
    var x = 1;
  }
}
)";

    EXPECT_TRUE(ShouldRun<RemoveUnreachableStatements>(src));
}

TEST_F(RemoveUnreachableStatementsTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, Return) {
    auto* src = R"(
fn f() {
  return;
  var remove_me = 1;
  if (true) {
    var remove_me_too = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  return;
}
)";

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, NestedReturn) {
    auto* src = R"(
fn f() {
  {
    {
      return;
    }
  }
  var remove_me = 1;
  if (true) {
    var remove_me_too = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  {
    {
      return;
    }
  }
}
)";

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

// Discard has "demote-to-helper" semantics, and so code following a discard statement is not
// considered unreachable.
TEST_F(RemoveUnreachableStatementsTest, Discard) {
    auto* src = R"(
fn f() {
  discard;
  var preserve_me = 1;
}
)";

    auto* expect = R"(
fn f() {
  discard;
  var preserve_me = 1;
}
)";

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, IfReturn) {
    auto* src = R"(
fn f() {
  if (true) {
    return;
  }
  var preserve_me = 1;
  if (true) {
    var preserve_me_too = 1;
  }
}
)";

    auto* expect = src;

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, IfElseReturn) {
    auto* src = R"(
fn f() {
  if (true) {
  } else {
    return;
  }
  var preserve_me = 1;
  if (true) {
    var preserve_me_too = 1;
  }
}
)";

    auto* expect = src;

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, LoopWithConditionalBreak) {
    auto* src = R"(
fn f() {
  loop {
    var a = 1;
    if (true) {
      break;
    }

    continuing {
      var b = 2;
    }
  }
  var preserve_me = 1;
  if (true) {
    var preserve_me_too = 1;
  }
}
)";

    auto* expect = src;

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, LoopWithConditionalBreakInContinuing) {
    auto* src = R"(
fn f() {
  loop {

    continuing {
      break if true;
    }
  }
  var preserve_me = 1;
  if (true) {
    var preserve_me_too = 1;
  }
}
)";

    auto* expect = src;

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RemoveUnreachableStatementsTest, SwitchCaseReturnDefaultBreak) {
    auto* src = R"(
fn f() {
  switch(1) {
    case 0: {
      return;
    }
    default: {
      break;
    }
  }
  var preserve_me = 1;
  if (true) {
    var preserve_me_too = 1;
  }
}
)";

    auto* expect = src;

    auto got = Run<RemoveUnreachableStatements>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::spirv::reader
