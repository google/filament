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

#include "src/tint/lang/core/ir/module.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/var.h"

using ::testing::ElementsAre;

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ModuleTest = IRTestHelper;

TEST_F(IR_ModuleTest, NameOfUnnamed) {
    auto* v = b.Var(ty.ptr<function, i32>());
    EXPECT_FALSE(mod.NameOf(v).IsValid());
}

TEST_F(IR_ModuleTest, SetName) {
    auto* v = b.Var(ty.ptr<function, i32>());
    mod.SetName(v, "a");
    EXPECT_EQ(mod.NameOf(v).Name(), "a");
}

TEST_F(IR_ModuleTest, SetNameRename) {
    auto* v = b.Var(ty.ptr<function, i32>());
    mod.SetName(v, "a");
    mod.SetName(v, "b");
    EXPECT_EQ(mod.NameOf(v).Name(), "b");
}

TEST_F(IR_ModuleTest, DependencyOrderedFunctions) {
    auto* fa = b.Function("a", ty.void_());
    auto* fb = b.Function("b", ty.void_());
    auto* fc = b.Function("c", ty.void_());
    auto* fd = b.Function("d", ty.void_());
    b.Append(fa->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {
            b.Call(fc);
            b.Call(fc);
            b.Call(fc);
            b.ExitIf(ifelse);
        });
        b.Append(ifelse->False(), [&] {
            b.Call(fb);
            b.ExitIf(ifelse);
        });
        b.Return(fa);
    });
    b.Append(fb->Block(), [&] {  //
        b.Call(fc);
        b.Call(fd);
        b.Return(fb);
    });
    b.Append(fc->Block(), [&] {  //
        b.Call(fd);
        b.Return(fc);
    });
    b.Append(fd->Block(), [&] {  //
        b.Return(fd);
    });
    mod.functions.Clear();
    mod.functions.Push(fa);
    mod.functions.Push(fd);
    mod.functions.Push(fb);
    mod.functions.Push(fc);
    EXPECT_THAT(mod.DependencyOrderedFunctions(), ElementsAre(fd, fc, fb, fa));
}

}  // namespace
}  // namespace tint::core::ir
