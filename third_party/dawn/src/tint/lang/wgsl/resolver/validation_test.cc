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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using ::testing::HasSubstr;

using ResolverValidationTest = ResolverTest;
using ResolverValidationDeathTest = ResolverValidationTest;

class FakeStmt final : public Castable<FakeStmt, ast::Statement> {
  public:
    FakeStmt(ast::NodeID nid, Source src) : Base(nid, src) {}
};

class FakeExpr final : public Castable<FakeExpr, ast::Expression> {
  public:
    FakeExpr(ast::NodeID nid, Source src) : Base(nid, src) {}
};

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInVertexStage) {
    ExpectError(
        R"(
var<workgroup> wg: vec4<f32>;

@vertex
fn f0() -> @builtin(position) vec4f {
  return wg;
}
)",
        R"(
input.wgsl:6:10 error: var with 'workgroup' address space cannot be used by vertex pipeline stage
  return wg;
         ^^

input.wgsl:2:1 note: variable is declared here
var<workgroup> wg: vec4<f32>;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInFragmentStage) {
    ExpectError(
        R"(
var<workgroup> wg: vec4<f32>;
var<workgroup> dst: vec4<f32>;
fn f2() { dst = wg; }
fn f1() { f2(); }
@fragment
fn f0() {
 f1();
}
)",
        R"(
input.wgsl:4:11 error: var with 'workgroup' address space cannot be used by fragment pipeline stage
fn f2() { dst = wg; }
          ^^^

input.wgsl:3:1 note: variable is declared here
var<workgroup> dst: vec4<f32>;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

input.wgsl:4:1 note: called by function 'f2'
fn f2() { dst = wg; }
^^^^^^^^^^^^^^^^^^^^^

input.wgsl:5:1 note: called by function 'f1'
fn f1() { f2(); }
^^^^^^^^^^^^^^^^^

input.wgsl:7:1 note: called by entry point 'f0'
fn f0() {
^^^^^^^^^
 f1();
^^^^^^
}
^
)");
}

TEST_F(ResolverValidationTest, RWStorageBufferUsedInVertexStage) {
    ExpectError(
        R"(
@group(0) @binding(0) var<storage, read_write> v : vec4<f32>;
@vertex
fn main() -> @builtin(position) vec4f {
    return v;
}
 )",
        R"(
input.wgsl:5:12 error: var with 'storage' address space and 'read_write' access mode cannot be used by vertex pipeline stage
    return v;
           ^

input.wgsl:2:23 note: variable is declared here
@group(0) @binding(0) var<storage, read_write> v : vec4<f32>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, RWStorageTextureUsedInVertexStage) {
    ExpectError(
        R"(
@group(0) @binding(0) var v : texture_storage_2d<r32uint, read_write>;

@vertex
fn main() -> @builtin(position) vec4f {
  _ = v;
  return vec4f();
}
)",
        R"(
input.wgsl:6:7 error: storage texture with 'read_write' access mode cannot be used by vertex pipeline stage
  _ = v;
      ^

input.wgsl:2:23 note: variable is declared here
@group(0) @binding(0) var v : texture_storage_2d<r32uint, read_write>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationDeathTest, UnhandledStmt) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.WrapInFunction(b.create<FakeStmt>());
            resolver::Resolve(b);
        },
        testing::HasSubstr(
            "internal compiler error: Switch() matched no cases. Type: tint::resolver::FakeStmt"));
}

TEST_F(ResolverValidationTest, Stmt_If_NonBool) {
    ExpectError(
        R"(
fn f() {
  if (1.23f) {}
}
)",
        R"(
input.wgsl:3:7 error: if statement condition must be bool, got f32
  if (1.23f) {}
      ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_ElseIf_NonBool) {
    ExpectError(
        R"(
fn f() {
  if (true) {} else if (1.23f) {}
}
)",
        R"(
input.wgsl:3:25 error: if statement condition must be bool, got f32
  if (true) {} else if (1.23f) {}
                        ^^^^^
)");
}

TEST_F(ResolverValidationDeathTest, Expr_ErrUnknownExprType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.WrapInFunction(b.create<FakeExpr>());
            Resolver(&b, {}).Resolve();
        },
        testing::HasSubstr(
            "internal compiler error: Switch() matched no cases. Type: tint::resolver::FakeExpr"));
}

TEST_F(ResolverValidationTest, UsingUndefinedVariable_Fail) {
    ExpectError(
        R"(
fn f() {
  b = 2;
}
)",
        R"(
input.wgsl:3:3 error: unresolved value 'b'
  b = 2;
  ^
)");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableInBlockStatement_Fail) {
    ExpectError(
        R"(
fn f() {
  {
    b = 2;
  }
}
)",
        R"(
input.wgsl:4:5 error: unresolved value 'b'
    b = 2;
    ^
)");
}

TEST_F(ResolverValidationTest, UsingDefinedGlobalVariable_Pass) {
    ExpectSuccess(
        R"(
var<private> global_var: f32 = 2.1;
fn my_func() {
  global_var = 3.14;
}
    )");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableInnerScope_Fail) {
    ExpectError(
        R"(
fn f() {
  if (true) { var a : f32 = 2.0; }
  a = 3.14;
}
)",
        R"(
input.wgsl:4:3 error: unresolved value 'a'
  a = 3.14;
  ^
)");
}

TEST_F(ResolverValidationTest, UsingDefinedVariableOuterScope_Pass) {
    ExpectSuccess(
        R"(
fn f() {
  var a : f32 = 2.0;
  if (true) { a = 3.14; }
}
)");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableDifferentScope_Fail) {
    ExpectError(
        R"(
fn f() {
  { var a : f32 = 2.0; }
  { a = 3.14; }
}
)",
        R"(
input.wgsl:4:5 error: unresolved value 'a'
  { a = 3.14; }
    ^
)");
}

TEST_F(ResolverValidationTest, AddressSpace_FunctionVariableWorkgroupClass) {
    ExpectError(
        R"(
fn func() {
  var<workgroup> var_name : i32;
}
)",
        R"(
input.wgsl:3:3 error: function-scope 'var' declaration must use 'function' address space
  var<workgroup> var_name : i32;
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, AddressSpace_FunctionVariablePrivateClass) {
    ExpectError(
        R"(
fn func() {
  var<private> s : i32;
}
)",
        R"(
input.wgsl:3:3 error: function-scope 'var' declaration must use 'function' address space
  var<private> s : i32;
  ^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, AddressSpace_SamplerExplicitAddressSpace) {
    ExpectError(
        R"(
@group(0) @binding(0) var<private> var_name : sampler;
)",
        R"(
input.wgsl:2:23 error: variables of type 'sampler' must not specify an address space
@group(0) @binding(0) var<private> var_name : sampler;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, AddressSpace_TextureExplicitAddressSpace) {
    ExpectError(
        R"(
@group(0) @binding(0) var<function> var_name : texture_1d<f32>;
)",
        R"(
input.wgsl:2:23 error: variables of type 'texture_1d<f32>' must not specify an address space
@group(0) @binding(0) var<function> var_name : texture_1d<f32>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
    ExpectError(
        R"(
var<private> my_vec : vec3<f32>;
fn f() {
  _ = my_vec.xyqz;
}
)",
        R"(
input.wgsl:4:16 error: invalid vector swizzle character
  _ = my_vec.xyqz;
               ^
)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
    ExpectError(
        R"(
var<private> my_vec : vec4<f32>;
fn f() {
  _ = my_vec.rgyw;
}
)",
        R"(
input.wgsl:4:14 error: invalid mixing of vector swizzle characters rgba with xyzw
  _ = my_vec.rgyw;
             ^^^^
)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadLength) {
    ExpectError(
        R"(
var<private> my_vec : vec3<f32>;
fn f() {
  _ = my_vec.zzzzz;
}
)",
        R"(
input.wgsl:4:14 error: invalid vector swizzle size
  _ = my_vec.zzzzz;
             ^^^^^
)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadIndex) {
    ExpectError(
        R"(
var<private> my_vec : vec2<f32>;
fn f() {
  _ = my_vec.z;
}
)",
        R"(
input.wgsl:4:14 error: invalid vector swizzle member
  _ = my_vec.z;
             ^
)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDeclAndAfterDecl_UsageInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop  {
      continue;
      var z : i32;
      continue;
      continuing {
          z = 2;
      }
  }
}
)",
        R"(
input.wgsl:5:7 warning: code is unreachable
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:4:7 error: continue statement bypasses declaration of 'z'
      continue;
      ^^^^^^^^

input.wgsl:5:7 note: identifier 'z' declared here
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:8:11 note: identifier 'z' referenced in continuing block here
          z = 2;
          ^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing_InBlocks) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      if (false) { break; }
      var z : i32;
      {{{continue;}}}
      continue;

      continuing {
          z = 2i;
      }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop  {
      if (true) {
          continue;
      }
      var z : i32;
      continuing {
          z = 2i;
      }
  }
}
)",
        R"(
input.wgsl:5:11 error: continue statement bypasses declaration of 'z'
          continue;
          ^^^^^^^^

input.wgsl:7:7 note: identifier 'z' declared here
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:9:11 note: identifier 'z' referenced in continuing block here
          z = 2i;
          ^
)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuingSubscope) {
    ExpectError(
        R"(
fn f() {
  loop  {
      if (true) {
          continue;
      }
      var z : i32;
      continuing {
          if (true) {
              z = 2i;
          }
      }
  }
}
)",
        R"(
input.wgsl:5:11 error: continue statement bypasses declaration of 'z'
          continue;
          ^^^^^^^^

input.wgsl:7:7 note: identifier 'z' declared here
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:10:15 note: identifier 'z' referenced in continuing block here
              z = 2i;
              ^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageOutsideBlock) {
    ExpectError(
        R"(
fn f() {
  loop  {
      if (true) {
          continue;
      }
      var z : i32;
      continuing {
          if (z < 2i) {
          }
      }
  }
}
)",
        R"(
input.wgsl:5:11 error: continue statement bypasses declaration of 'z'
          continue;
          ^^^^^^^^

input.wgsl:7:7 note: identifier 'z' declared here
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:9:15 note: identifier 'z' referenced in continuing block here
          if (z < 2i) {
              ^
)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuingLoop) {
    ExpectError(
        R"(
fn f() {
  loop  {
      if (true) {
          continue;
      }
      var z : i32;
      continuing {
          loop {
              z = 2i;
          }
      }
  }
}
)",
        R"(
input.wgsl:5:11 error: continue statement bypasses declaration of 'z'
          continue;
          ^^^^^^^^

input.wgsl:7:7 note: identifier 'z' declared here
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:10:15 note: identifier 'z' referenced in continuing block here
              z = 2i;
              ^
)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDecl_UsageInNestedContinuingInBody) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      continue;
      var z : i32;
      loop {
          continue;
          continuing {
              z = 2i;
              break if true;
          }
      }
      continuing {
          break if true;
      }
  }
}
)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDecl_UsageInNestedContinuingInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop  {
      continue;
      var z : i32;
      continuing {
          loop {
              continuing {
                z = 2i;
                break if true;
              }
          }
          break if true;
      }
  }
}
)",
        R"(
input.wgsl:5:7 warning: code is unreachable
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:4:7 error: continue statement bypasses declaration of 'z'
      continue;
      ^^^^^^^^

input.wgsl:5:7 note: identifier 'z' declared here
      var z : i32;
      ^^^^^^^^^^^

input.wgsl:9:17 note: identifier 'z' referenced in continuing block here
                z = 2i;
                ^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuing) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      loop {
          if (true) { continue; } // OK: not part of the outer loop
          break;
      }
      var z : i32;
      break;
      continuing {
          z = 2i;
      }
  }
}
)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingSubscope) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      loop {
          if (true) { continue; } // OK: not part of the outer loop
          break;
      }
      var z : i32;
      break;
      continuing {
          if (true) {
              z = 2i;
          }
      }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingLoop) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      loop {
          if (true) { continue; } // OK: not part of the outer loop
          break;
      }
      var z : i32;
      break;
      continuing {
          loop {
              z = 2i;
              break;
          }
      }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      var z : i32;
      if (true) { continue; }
      break;
      continuing {
          z = 2i;
      }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ReturnInContinuing_Direct) {
    ExpectError(
        R"(
fn f() {
  loop  {
    continuing {
      return;
    }
  }
}
)",
        R"(
input.wgsl:5:7 error: continuing blocks must not contain a return statement
      return;
      ^^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ReturnInContinuing_Indirect) {
    ExpectError(
        R"(
fn f() {
  loop {
    if (false) { break; }
    continuing {
      loop {
        return;
      }
    }
  }
}
)",
        R"(
input.wgsl:7:9 error: continuing blocks must not contain a return statement
        return;
        ^^^^^^

input.wgsl:5:16 note: see continuing block here
    continuing {
               ^
      loop {
^^^^^^^^^^^^
        return;
^^^^^^^^^^^^^^^
      }
^^^^^^^
    }
^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_DiscardInContinuing_Direct) {
    ExpectSuccess(
        R"(
fn my_func() {
  loop  {
    continuing {
      discard;
      break if true;
    }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInContinuing_Direct) {
    ExpectError(
        R"(
fn f() {
  loop  {
      continuing {
          continue;
      }
  }
}
)",
        R"(
input.wgsl:5:11 error: continuing blocks must not contain a continue statement
          continue;
          ^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInContinuing_Indirect) {
    ExpectSuccess(
        R"(
fn f() {
  loop {
    if (false) { break; }
    continuing {
      loop {
        if (false) { break; }
        continue;
      }
    }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf) {
    ExpectSuccess(
        R"(
fn f() {
  loop  {
      continuing {
          break if true;
      }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf_Not_Last) {
    ExpectError(
        R"(
fn f() {
  loop  {
      var z : i32;
      continuing {
          break if true;
          z = 2i;
      }
  }
}
)",
        R"(
input.wgsl:6:11 error: break-if must be the last statement in a continuing block
          break if true;
          ^^^^^

input.wgsl:5:18 note: see continuing block here
      continuing {
                 ^
          break if true;
^^^^^^^^^^^^^^^^^^^^^^^^
          z = 2i;
^^^^^^^^^^^^^^^^^
      }
^^^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf_Duplicate) {
    ExpectError(
        R"(
fn f() {
  loop  {
      continuing {
          break if true;
          break if false;
      }
  }
}
)",
        R"(
input.wgsl:5:11 error: break-if must be the last statement in a continuing block
          break if true;
          ^^^^^

input.wgsl:4:18 note: see continuing block here
      continuing {
                 ^
          break if true;
^^^^^^^^^^^^^^^^^^^^^^^^
          break if false;
^^^^^^^^^^^^^^^^^^^^^^^^^
      }
^^^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf_NonBool) {
    ExpectError(
        R"(
fn f() {
  loop  {
      continuing {
          break if 1i;
      }
  }
}
)",
        R"(
input.wgsl:5:20 error: break-if statement condition must be bool, got i32
          break if 1i;
                   ^^
)");
}

TEST_F(ResolverValidationTest, Stmt_ForLoop_CondIsBoolRef) {
    ExpectSuccess(
        R"(
fn f() {
  var cond : bool = true;
  for (; cond; ) {}
}
)");
}

TEST_F(ResolverValidationTest, Stmt_ForLoop_CondIsNotBool) {
    ExpectError(
        R"(
fn f() {
  for (; 1.0f; ) {}
}
)",
        R"(
input.wgsl:3:10 error: for-loop condition must be bool, got f32
  for (; 1.0f; ) {}
         ^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_While_CondIsBoolRef) {
    ExpectSuccess(
        R"(
fn f() {
  var cond : bool = false;
  while (cond) {}
}
)");
}

TEST_F(ResolverValidationTest, Stmt_While_CondIsNotBool) {
    ExpectError(
        R"(
fn f() {
  while (1.0f) {}
}
)",
        R"(
input.wgsl:3:10 error: while condition must be bool, got f32
  while (1.0f) {}
         ^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_ContinueInLoop) {
    ExpectSuccess(
        R"(
fn f() {
  loop {
    if (false) { break; }
    continue;
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_ContinueNotInLoop) {
    ExpectError(
        R"(
fn f() {
  continue;
}
)",
        R"(
input.wgsl:3:3 error: continue statement must be in a loop
  continue;
  ^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInLoop) {
    ExpectSuccess(
        R"(
fn f() {
  loop {
    break;
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInSwitch) {
    ExpectSuccess(
        R"(
fn f() {
  loop {
    switch(1) {
      case 1: { break; }
      default: {}
    }
    break;
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInSwitchInContinuing) {
    ExpectSuccess(
        R"(
fn f() {
  loop {
    break;
    continuing {
      switch(1) {
        default: {
          break;
        }
      }
    }
  }
}
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfTrueInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {
        break;
      }
    }
  }
}
)",
        R"(
input.wgsl:6:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {} else { break; }
    }
  }
}
)",
        R"(
input.wgsl:5:26 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
      if(true) {} else { break; }
                         ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      break;
    }
  }
}
)",
        R"(
input.wgsl:5:7 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
      break;
      ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfInIfInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {
        if(true) {
          break;
        }
      }
    }
  }
}
)",
        R"(
input.wgsl:7:11 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
          break;
          ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfTrueMultipleStmtsInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {
        _ = 1i;
        break;
      }
    }
  }
}
)",
        R"(
input.wgsl:7:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseMultipleStmtsInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {} else {
        _ = 1i;
        break;
      }
    }
  }
}
)",
        R"(
input.wgsl:7:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseIfInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {} else if (true) {
        break;
      }
    }
  }
}
)",
        R"(
input.wgsl:6:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfNonEmptyElseInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {
        break;
      } else {
        _ = 1i;
      }
    }
  }
}
)",
        R"(
input.wgsl:6:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseNonEmptyTrueInContinuing) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {
        _ = 1i;
      } else {
        break;
      }
    }
  }
}
)",
        R"(
input.wgsl:8:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfInContinuingNotLast) {
    ExpectError(
        R"(
fn f() {
  loop {
    continuing {
      if(true) {
        break;
      }
      _ = 1i;
    }
  }
}
)",
        R"(
input.wgsl:6:9 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.
        break;
        ^^^^^
)");
}

TEST_F(ResolverValidationTest, Stmt_BreakNotInLoopOrSwitch) {
    ExpectError(
        R"(
fn f() {
  break;
}
)",
        R"(
input.wgsl:3:3 error: break statement must be in a loop or switch case
  break;
  ^^^^^
)");
}

TEST_F(ResolverValidationTest, StructMemberDuplicateName) {
    ExpectError(
        R"(
struct S {
  a : i32,
  a : i32,
}
)",
        R"(
input.wgsl:4:3 error: redefinition of 'a'
  a : i32,
  ^

input.wgsl:3:3 note: previous definition is here
  a : i32,
  ^
)");
}
TEST_F(ResolverValidationTest, StructMemberDuplicateNameDifferentTypes) {
    ExpectError(
        R"(
struct S {
  a : bool,
  a : vec3<f32>,
}
)",
        R"(
input.wgsl:4:3 error: redefinition of 'a'
  a : vec3<f32>,
  ^

input.wgsl:3:3 note: previous definition is here
  a : bool,
  ^
)");
}
TEST_F(ResolverValidationTest, StructMemberDuplicateNamePass) {
    ExpectSuccess(
        R"(
struct S {
  a : i32,
  b : f32,
}
struct S1 {
  a : i32,
  b : f32,
}
)");
}

TEST_F(ResolverValidationTest, NegativeStructMemberAlignAttribute) {
    ExpectError(
        R"(
struct S {
  @align(-2) a : f32,
}
)",
        R"(
input.wgsl:3:4 error: '@align' value must be a positive, power-of-two integer
  @align(-2) a : f32,
   ^^^^^
)");
}

TEST_F(ResolverValidationTest, NonPOTStructMemberAlignAttribute) {
    ExpectError(
        R"(
struct S {
  @align(3) a : f32,
}
)",
        R"(
input.wgsl:3:4 error: '@align' value must be a positive, power-of-two integer
  @align(3) a : f32,
   ^^^^^
)");
}

TEST_F(ResolverValidationTest, ZeroStructMemberAlignAttribute) {
    ExpectError(
        R"(
struct S {
  @align(0) a : f32,
}
)",
        R"(
input.wgsl:3:4 error: '@align' value must be a positive, power-of-two integer
  @align(0) a : f32,
   ^^^^^
)");
}

TEST_F(ResolverValidationTest, StructMemberSizeAttributeTooSmall) {
    ExpectError(
        R"(
struct S {
  @size(1) a : f32,
}
)",
        R"(
input.wgsl:3:4 error: '@size' must be at least as big as the type's size (4)
  @size(1) a : f32,
   ^^^^
)");
}

TEST_F(ResolverValidationTest, Expr_Initializer_Cast_Pointer) {
    ExpectError(
        R"(
fn f() {
  var vf : f32;
  let ip : ptr<function, i32> = ptr<function, i32>(vf);
}
)",
        R"(
input.wgsl:4:33 error: type is not constructible
  let ip : ptr<function, i32> = ptr<function, i32>(vf);
                                ^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, I32_Overflow) {
    ExpectError(
        R"(
var<private> v : i32 = 2147483648;
)",
        R"(
input.wgsl:2:24 error: value 2147483648 cannot be represented as 'i32'
var<private> v : i32 = 2147483648;
                       ^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, I32_Underflow) {
    ExpectError(
        R"(
var<private> v : i32 = -2147483649;
)",
        R"(
input.wgsl:2:24 error: value -2147483649 cannot be represented as 'i32'
var<private> v : i32 = -2147483649;
                       ^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, U32_Overflow) {
    ExpectError(
        R"(
var<private> v : u32 = 4294967296;
)",
        R"(
input.wgsl:2:24 error: value 4294967296 cannot be represented as 'u32'
var<private> v : u32 = 4294967296;
                       ^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_PartialEval_Valid) {
    ExpectSuccess(
        R"(
var<private> v : i32;
fn f() {
  let res = v << 31u;
}
)");
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_PartialEval_Invalid) {
    ExpectError(
        R"(
var<private> v : i32;
fn f() {
  let res = v << 32u;
}
)",
        R"(
input.wgsl:4:13 error: shift left value must be less than the bit width of the lhs, which is 32
  let res = v << 32u;
            ^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, ShiftRight_U32_PartialEval_Invalid) {
    ExpectError(
        R"(
var<private> v : u32;
fn f() {
  let res = v >> 64u;
}
)",
        R"(
input.wgsl:4:13 error: shift right value must be less than the bit width of the lhs, which is 32
  let res = v >> 64u;
            ^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, ShiftLeft_VecI32_PartialEval_Valid) {
    ExpectSuccess(
        R"(
var<private> v : vec3<i32>;
fn f() {
  let res = v << vec3<u32>(0u, 1u, 2u);
}
)");
}

TEST_F(ResolverValidationTest, ShiftLeft_VecI32_PartialEval_Invalid) {
    ExpectError(
        R"(
var<private> v : vec3<i32>;
fn f() {
  let res = v << vec3<u32>(31u, 32u, 33u);
}
)",
        R"(
input.wgsl:4:13 error: shift left value must be less than the bit width of the lhs, which is 32
  let res = v << vec3<u32>(31u, 32u, 33u);
            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_CompoundAssign_Valid) {
    ExpectSuccess(
        R"(
var<private> v : i32;
fn f() {
  v <<= 1u;
}
)");
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_CompoundAssign_Invalid) {
    ExpectError(
        R"(
var<private> v : i32;
fn f() {
  v <<= 64u;
}
)",
        R"(
input.wgsl:4:5 error: shift left value must be less than the bit width of the lhs, which is 32
  v <<= 64u;
    ^^^
)");
}

TEST_F(ResolverValidationTest, WorkgroupUniformLoad_ArraySize_NamedOverride) {
    ExpectError(
        R"(
override size = 10u;
var<workgroup> a : array<u32, size>;
fn f() {
  _ = workgroupUniformLoad(&a);
}
)",
        R"(
input.wgsl:5:7 error: no matching call to 'workgroupUniformLoad(ptr<workgroup, array<u32, size>, read_write>)'

2 candidate functions:
 • 'workgroupUniformLoad(ptr<workgroup, T, read_write>  ✗ ) -> T' where:
      ✗  'T' is 'any concrete constructible type'
 • 'workgroupUniformLoad(ptr<workgroup, atomic<T>, read_write>  ✗ ) -> T' where:
      ✗  'T' is 'i32' or 'u32'

  _ = workgroupUniformLoad(&a);
      ^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverValidationTest, WorkgroupUniformLoad_ArraySize_NamedConstant) {
    ExpectSuccess(
        R"(
const size = 10u;
var<workgroup> a : array<u32, size>;
fn f() {
  _ = workgroupUniformLoad(&a);
}
)");
}

}  // namespace
}  // namespace tint::resolver

TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeStmt);
TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeExpr);
