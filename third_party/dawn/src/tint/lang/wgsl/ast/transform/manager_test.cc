// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/manager.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::ast::transform {
namespace {

using TransformManagerTest = testing::Test;

class AST_NoOp final : public ast::transform::Transform {
    ApplyResult Apply(const Program&, const DataMap&, DataMap&) const override {
        return SkipTransform;
    }
};

class AST_AddFunction final : public ast::transform::Transform {
    ApplyResult Apply(const Program& src, const DataMap&, DataMap&) const override {
        ProgramBuilder b;
        program::CloneContext ctx{&b, &src};
        b.Func(b.Sym("ast_func"), {}, b.ty.void_(), {});
        ctx.Clone();
        return resolver::Resolve(b);
    }
};

Program MakeAST() {
    ProgramBuilder b;
    b.Func(b.Sym("main"), {}, b.ty.void_(), {});
    return resolver::Resolve(b);
}

// Test that an AST program is always cloned, even if all transforms are skipped.
TEST_F(TransformManagerTest, AST_AlwaysClone) {
    Program ast = MakeAST();

    Manager manager;
    DataMap outputs;
    manager.Add<AST_NoOp>();

    auto result = manager.Run(ast, {}, outputs);
    EXPECT_TRUE(result.IsValid()) << result.Diagnostics();
    EXPECT_NE(result.ID(), ast.ID());
    ASSERT_EQ(result.AST().Functions().Length(), 1u);
    EXPECT_EQ(result.AST().Functions()[0]->name->symbol.Name(), "main");
}

}  // namespace
}  // namespace tint::ast::transform
