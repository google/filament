// Copyright 2022 The Dawn & Tint Authors
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

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/get_insertion_point.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast::transform {
namespace {

using GetInsertionPointTest = ::testing::Test;

TEST_F(GetInsertionPointTest, Block) {
    // fn f() {
    //     var a = 1i;
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    auto* block = b.Block(var);
    b.Func("f", tint::Empty, b.ty.void_(), Vector{block});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    // Can insert in block containing the variable, above or below the input statement.
    auto ip = utils::GetInsertionPoint(ctx, var);
    ASSERT_EQ(ip.first->Declaration(), block);
    ASSERT_EQ(ip.second, var);
}

TEST_F(GetInsertionPointTest, ForLoopInit) {
    // fn f() {
    //     for(var a = 1i; true; ) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    auto* fl = b.For(var, b.Expr(true), nullptr, b.Block());
    auto* func_block = b.Block(fl);
    b.Func("f", tint::Empty, b.ty.void_(), Vector{func_block});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    // Can insert in block containing for-loop above the for-loop itself.
    auto ip = utils::GetInsertionPoint(ctx, var);
    ASSERT_EQ(ip.first->Declaration(), func_block);
    ASSERT_EQ(ip.second, fl);
}

TEST_F(GetInsertionPointTest, ForLoopCont_Invalid) {
    // fn f() {
    //     for(; true; var a = 1i) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    auto* s = b.For({}, b.Expr(true), var, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    // Can't insert before/after for loop continue statement (would ned to be converted to loop).
    auto ip = utils::GetInsertionPoint(ctx, var);
    ASSERT_EQ(ip.first, nullptr);
    ASSERT_EQ(ip.second, nullptr);
}

}  // namespace
}  // namespace tint::ast::transform
