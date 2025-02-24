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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct ResolverAliasAnalysisTest : public resolver::TestHelper, public testing::Test {};

// Base test harness for tests that pass two pointers to a function.
//
// fn target(p1 : ptr<function, i32>, p2 : ptr<function, i32>) {
//   <test statements>
// }
// fn caller() {
//   var v1 : i32;
//   var v2 : i32;
//   target(&v1, aliased ? &v1 : &v2);
// }
struct TwoPointerConfig {
    const core::AddressSpace address_space;  // The address space for the pointers.
    const bool aliased;                      // Whether the pointers alias or not.
};
class TwoPointers : public ResolverTestWithParam<TwoPointerConfig> {
  protected:
    void SetUp() override {
        Vector<const ast::Statement*, 4> body;
        if (GetParam().address_space == core::AddressSpace::kFunction) {
            body.Push(Decl(Var("v1", ty.i32())));
            body.Push(Decl(Var("v2", ty.i32())));
        } else {
            GlobalVar("v1", core::AddressSpace::kPrivate, ty.i32());
            GlobalVar("v2", core::AddressSpace::kPrivate, ty.i32());
        }
        body.Push(CallStmt(Call("target", AddressOf(Source{{12, 34}}, "v1"),
                                AddressOf(Source{{56, 78}}, GetParam().aliased ? "v1" : "v2"))));
        Func("caller", tint::Empty, ty.void_(), body);
    }

    void Run(Vector<const ast::Statement*, 4>&& body, const char* err = nullptr) {
        auto addrspace = GetParam().address_space;
        Func("target",
             Vector{
                 Param("p1", ty.ptr<i32>(addrspace)),
                 Param("p2", ty.ptr<i32>(addrspace)),
             },
             ty.void_(), std::move(body));
        if (GetParam().aliased && err) {
            EXPECT_FALSE(r()->Resolve());
            EXPECT_EQ(r()->error(), err);
        } else {
            EXPECT_TRUE(r()->Resolve()) << r()->error();
        }
    }
};

TEST_P(TwoPointers, ReadRead) {
    // _ = *p1;
    // _ = *p2;
    Run({
        Assign(Phony(), Deref("p1")),
        Assign(Phony(), Deref("p2")),
    });
}

TEST_P(TwoPointers, ReadWrite) {
    // _ = *p1;
    // *p2 = 42;
    Run(
        {
            Assign(Phony(), Deref("p1")),
            Assign(Deref("p2"), 42_a),
        },
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(TwoPointers, WriteRead) {
    // *p1 = 42;
    // _ = *p2;
    Run(
        {
            Assign(Deref("p1"), 42_a),
            Assign(Phony(), Deref("p2")),
        },
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(TwoPointers, WriteWrite) {
    // *p1 = 42;
    // *p2 = 42;
    Run(
        {
            Assign(Deref("p1"), 42_a),
            Assign(Deref("p2"), 42_a),
        },
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(TwoPointers, ReadWriteThroughChain) {
    // fn f2(p1 : ptr<function, i32>, p2 : ptr<function, i32>) {
    //   _ = *p1;
    //   *p2 = 42;
    // }
    // fn f1(p1 : ptr<function, i32>, p2 : ptr<function, i32>) {
    //   f2(p1, p2);
    // }
    //
    // f1(p1, p2);
    Func("f2",
         Vector{
             Param("p1", ty.ptr<i32>(GetParam().address_space)),
             Param("p2", ty.ptr<i32>(GetParam().address_space)),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p1")),
             Assign(Deref("p2"), 42_a),
         });
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(GetParam().address_space)),
             Param("p2", ty.ptr<i32>(GetParam().address_space)),
         },
         ty.void_(),
         Vector{
             CallStmt(Call("f2", "p1", "p2")),
         });
    Run(
        {
            CallStmt(Call("f1", "p1", "p2")),
        },
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(TwoPointers, ReadWriteAcrossDifferentFunctions) {
    // fn f1(p1 : ptr<function, i32>) {
    //   _ = *p1;
    // }
    // fn f2(p2 : ptr<function, i32>) {
    //   *p2 = 42;
    // }
    //
    // f1(p1);
    // f2(p2);
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(GetParam().address_space)),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p1")),
         });
    Func("f2",
         Vector{
             Param("p2", ty.ptr<i32>(GetParam().address_space)),
         },
         ty.void_(),
         Vector{
             Assign(Deref("p2"), 42_a),
         });
    Run(
        {
            CallStmt(Call("f1", "p1")),
            CallStmt(Call("f2", "p2")),
        },
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

INSTANTIATE_TEST_SUITE_P(ResolverAliasAnalysisTest,
                         TwoPointers,
                         ::testing::Values(TwoPointerConfig{core::AddressSpace::kFunction, false},
                                           TwoPointerConfig{core::AddressSpace::kFunction, true},
                                           TwoPointerConfig{core::AddressSpace::kPrivate, false},
                                           TwoPointerConfig{core::AddressSpace::kPrivate, true}),
                         [](const ::testing::TestParamInfo<TwoPointers::ParamType>& p) {
                             StringStream ss;
                             ss << (p.param.aliased ? "Aliased" : "Unaliased") << "_"
                                << p.param.address_space;
                             return ss.str();
                         });

// Base test harness for tests that pass a pointer to a function that references a module-scope var.
//
// var<private> global_1 : i32;
// var<private> global_2 : i32;
// fn target(p1 : ptr<private, i32>) {
//   <test statements>
// }
// fn caller() {
//   target(aliased ? &global_1 : &global_2);
// }
class OnePointerOneModuleScope : public ResolverTestWithParam<bool> {
  protected:
    void SetUp() override {
        GlobalVar("global_1", core::AddressSpace::kPrivate, ty.i32());
        GlobalVar("global_2", core::AddressSpace::kPrivate, ty.i32());
        Func("caller", tint::Empty, ty.void_(),
             Vector{
                 CallStmt(Call("target",
                               AddressOf(Source{{12, 34}}, GetParam() ? "global_1" : "global_2"))),
             });
    }

    void Run(Vector<const ast::Statement*, 4>&& body, const char* err = nullptr) {
        Func("target",
             Vector{
                 Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
             },
             ty.void_(), std::move(body));
        if (GetParam() && err) {
            EXPECT_FALSE(r()->Resolve());
            EXPECT_EQ(r()->error(), err);
        } else {
            EXPECT_TRUE(r()->Resolve()) << r()->error();
        }
    }
};

TEST_P(OnePointerOneModuleScope, ReadRead) {
    // _ = *p1;
    // _ = global_1;
    Run({
        Assign(Phony(), Deref("p1")),
        Assign(Phony(), "global_1"),
    });
}

TEST_P(OnePointerOneModuleScope, ReadWrite) {
    // _ = *p1;
    // global_1 = 42;
    Run(
        {
            Assign(Phony(), Deref("p1")),
            Assign(Expr(Source{{56, 78}}, "global_1"), 42_a),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable write in 'target')");
}

TEST_P(OnePointerOneModuleScope, WriteRead) {
    // *p1 = 42;
    // _ = global_1;
    Run(
        {
            Assign(Deref("p1"), 42_a),
            Assign(Phony(), Expr(Source{{56, 78}}, "global_1")),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable read in 'target')");
}

TEST_P(OnePointerOneModuleScope, WriteWrite) {
    // *p1 = 42;
    // global_1 = 42;
    Run(
        {
            Assign(Deref("p1"), 42_a),
            Assign(Expr(Source{{56, 78}}, "global_1"), 42_a),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable write in 'target')");
}

TEST_P(OnePointerOneModuleScope, ReadWriteThroughChain_GlobalViaArg) {
    // fn f2(p1 : ptr<private, i32>) {
    //   *p1 = 42;
    // }
    // fn f1(p1 : ptr<private, i32>) {
    //   _ = *p1;
    //   f2(&global_1);
    // }
    //
    // f1(p1);
    Func("f2",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Deref("p1"), 42_a),
         });
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p1")),
             CallStmt(Call("f2", AddressOf(Source{{56, 78}}, "global_1"))),
         });
    Run(
        {
            CallStmt(Call("f1", "p1")),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable write in 'f1')");
}

TEST_P(OnePointerOneModuleScope, ReadWriteThroughChain_Both) {
    // fn f2(p1 : ptr<private, i32>) {
    //   _ = *p1;
    //   global_1 = 42;
    // }
    // fn f1(p1 : ptr<private, i32>) {
    //   f2(p1);
    // }
    //
    // f1(p1);
    Func("f2",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p1")),
             Assign(Expr(Source{{56, 78}}, "global_1"), 42_a),
         });
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             CallStmt(Call("f2", "p1")),
         });
    Run(
        {
            CallStmt(Call("f1", "p1")),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable write in 'f2')");
}

TEST_P(OnePointerOneModuleScope, WriteReadThroughChain_GlobalViaArg) {
    // fn f2(p1 : ptr<private, i32>) {
    //   _ = *p1;
    // }
    // fn f1(p1 : ptr<private, i32>) {
    //   *p1 = 42;
    //   f2(&global_1);
    // }
    //
    // f1(p1);
    Func("f2",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p1")),
         });
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Deref("p1"), 42_a),
             CallStmt(Call("f2", AddressOf(Source{{56, 78}}, "global_1"))),
         });
    Run(
        {
            CallStmt(Call("f1", "p1")),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable read in 'f1')");
}

TEST_P(OnePointerOneModuleScope, WriteReadThroughChain_Both) {
    // fn f2(p1 : ptr<private, i32>) {
    //   *p1 = 42;
    //   _ = global_1;
    // }
    // fn f1(p1 : ptr<private, i32>) {
    //   f2(p1);
    // }
    //
    // f1(p1);
    Func("f2",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Deref("p1"), 42_a),
             Assign(Phony(), Expr(Source{{56, 78}}, "global_1")),
         });
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             CallStmt(Call("f2", "p1")),
         });
    Run(
        {
            CallStmt(Call("f1", "p1")),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable read in 'f2')");
}

TEST_P(OnePointerOneModuleScope, ReadWriteAcrossDifferentFunctions) {
    // fn f1(p1 : ptr<private, i32>) {
    //   _ = *p1;
    // }
    // fn f2() {
    //   global_1 = 42;
    // }
    //
    // f1(p1);
    // f2();
    Func("f1",
         Vector{
             Param("p1", ty.ptr<i32>(core::AddressSpace::kPrivate)),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p1")),
         });
    Func("f2", tint::Empty, ty.void_(),
         Vector{
             Assign(Expr(Source{{56, 78}}, "global_1"), 42_a),
         });
    Run(
        {
            CallStmt(Call("f1", "p1")),
            CallStmt(Call("f2")),
        },
        R"(12:34 error: invalid aliased pointer argument
56:78 note: aliases with module-scope variable write in 'f2')");
}

INSTANTIATE_TEST_SUITE_P(ResolverAliasAnalysisTest,
                         OnePointerOneModuleScope,
                         ::testing::Values(false, true),
                         [](const ::testing::TestParamInfo<bool>& p) {
                             return p.param ? "Aliased" : "Unaliased";
                         });

// Base test harness for tests that use a potentially aliased pointer in a variety of expressions.
//
// fn target(p1 : ptr<function, i32>, p2 : ptr<function, i32>) {
//   *p1 = 42;
//   <test statements>
// }
// fn caller() {
//   var v1 : i32;
//   var v2 : i32;
//   target(&v1, aliased ? &v1 : &v2);
// }
class Use : public ResolverTestWithParam<bool> {
  protected:
    void SetUp() override {
        Func("caller", tint::Empty, ty.void_(),
             Vector{
                 Decl(Var("v1", ty.i32())),
                 Decl(Var("v2", ty.i32())),
                 CallStmt(Call("target", AddressOf(Source{{12, 34}}, "v1"),
                               AddressOf(Source{{56, 78}}, GetParam() ? "v1" : "v2"))),
             });
    }

    void Run(const ast::Statement* stmt, const char* err = nullptr) {
        Func("target",
             Vector{
                 Param("p1", ty.ptr<i32>(core::AddressSpace::kFunction)),
                 Param("p2", ty.ptr<i32>(core::AddressSpace::kFunction)),
             },
             ty.void_(),
             Vector{
                 Assign(Deref("p1"), 42_a),
                 stmt,
             });
        if (GetParam() && err) {
            EXPECT_FALSE(r()->Resolve());
            EXPECT_EQ(r()->error(), err);
        } else {
            EXPECT_TRUE(r()->Resolve()) << r()->error();
        }
    }
};

TEST_P(Use, NoAccess) {
    // Expect no errors even when aliasing occurs.
    Run(Assign(Phony(), 42_a));
}

TEST_P(Use, Write_Increment) {
    // (*p2)++;
    Run(Increment(Deref("p2")), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Write_Decrement) {
    // (*p2)--;
    Run(Decrement(Deref("p2")), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Write_CompoundAssignment_LHS) {
    // *p2 += 42;
    Run(CompoundAssign(Deref("p2"), 42_a, core::BinaryOp::kAdd),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_CompoundAssignment_RHS) {
    // var<private> global : i32;
    // global += *p2;
    GlobalVar("global", core::AddressSpace::kPrivate, ty.i32());
    Run(CompoundAssign("global", Deref("p2"), core::BinaryOp::kAdd),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_BinaryOp_LHS) {
    // _ = (*p2) + 1;
    Run(Assign(Phony(), Add(Deref("p2"), 1_a)), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_BinaryOp_RHS) {
    // _ = 1 + (*p2);
    Run(Assign(Phony(), Add(1_a, Deref("p2"))), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_UnaryMinus) {
    // _ = -(*p2);
    Run(Assign(Phony(), Negation(Deref("p2"))), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_FunctionCallArg) {
    // abs(*p2);
    Run(Assign(Phony(), Call("abs", Deref("p2"))), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_Bitcast) {
    // _ = bitcast<f32>(*p2);
    Run(Assign(Phony(), Bitcast<f32>(Deref("p2"))),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_Convert) {
    // _ = f32(*p2);
    Run(Assign(Phony(), Call<f32>(Deref("p2"))),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_IndexAccessor) {
    // var<private> data : array<f32, 4>;
    // _ = data[*p2];
    GlobalVar("data", core::AddressSpace::kPrivate, ty.array<f32, 4>());
    Run(Assign(Phony(), IndexAccessor("data", Deref("p2"))),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_LetInitializer) {
    // let x = *p2;
    Run(Decl(Let("x", Deref("p2"))), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_VarInitializer) {
    // var x = *p2;
    Run(Decl(Var("x", core::AddressSpace::kFunction, Deref("p2"))),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_ReturnValue) {
    // fn foo(p : ptr<function, i32>) -> i32 { return *p; }
    // foo(p2);
    Func("foo",
         Vector{
             Param("p", ty.ptr<i32>(core::AddressSpace::kFunction)),
         },
         ty.i32(),
         Vector{
             Return(Deref("p")),
         });
    Run(Assign(Phony(), Call("foo", "p2")), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, Read_Switch) {
    // Switch (*p2) { default {} }
    Run(Switch(Deref("p2"), Vector{DefaultCase(Block())}),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(Use, NoAccess_AddressOf_Deref) {
    // Should not invoke the load-rule, and therefore expect no errors even when aliasing occurs.
    // let newp = &(*p2);
    Run(Decl(Let("newp", AddressOf(Deref("p2")))));
}

INSTANTIATE_TEST_SUITE_P(ResolverAliasAnalysisTest,
                         Use,
                         ::testing::Values(false, true),
                         [](const ::testing::TestParamInfo<bool>& p) {
                             return p.param ? "Aliased" : "Unaliased";
                         });

// Base test harness for tests that use a potentially aliased pointer in a variety of expressions.
// As above, but using the bool type to test expressions that invoke that load-rule for booleans.
//
// fn target(p1 : ptr<function, bool>, p2 : ptr<function, bool>) {
//   *p1 = true;
//   <test statements>
// }
// fn caller() {
//   var v1 : bool;
//   var v2 : bool;
//   target(&v1, aliased ? &v1 : &v2);
// }
class UseBool : public ResolverTestWithParam<bool> {
  protected:
    void SetUp() override {
        Func("caller", tint::Empty, ty.void_(),
             Vector{
                 Decl(Var("v1", ty.bool_())),
                 Decl(Var("v2", ty.bool_())),
                 CallStmt(Call("target", AddressOf(Source{{12, 34}}, "v1"),
                               AddressOf(Source{{56, 78}}, GetParam() ? "v1" : "v2"))),
             });
    }

    void Run(const ast::Statement* stmt, const char* err = nullptr) {
        Func("target",
             Vector{
                 Param("p1", ty.ptr<bool>(core::AddressSpace::kFunction)),
                 Param("p2", ty.ptr<bool>(core::AddressSpace::kFunction)),
             },
             ty.void_(),
             Vector{
                 Assign(Deref("p1"), true),
                 stmt,
             });
        if (GetParam() && err) {
            EXPECT_FALSE(r()->Resolve());
            EXPECT_EQ(r()->error(), err);
        } else {
            EXPECT_TRUE(r()->Resolve()) << r()->error();
        }
    }
};

TEST_P(UseBool, Read_IfCond) {
    // if (*p2) {}
    Run(If(Deref("p2"), Block()), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(UseBool, Read_WhileCond) {
    // while (*p2) {}
    Run(While(Deref("p2"), Block()), R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(UseBool, Read_ForCond) {
    // for (; *p2; ) {}
    Run(For(nullptr, Deref("p2"), nullptr, Block()),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(UseBool, Read_BreakIf) {
    // loop { continuing { break if (*p2); } }
    Run(Loop(Block(), Block(BreakIf(Deref("p2")))),
        R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

INSTANTIATE_TEST_SUITE_P(ResolverAliasAnalysisTest,
                         UseBool,
                         ::testing::Values(false, true),
                         [](const ::testing::TestParamInfo<bool>& p) {
                             return p.param ? "Aliased" : "Unaliased";
                         });

TEST_F(ResolverAliasAnalysisTest, NoAccess_MemberAccessor) {
    // Should not invoke the load-rule, and therefore expect no errors even when aliasing occurs.
    //
    // struct S { a : i32 }
    // fn f2(p1 : ptr<function, S>, p2 : ptr<function, S>) {
    //   let newp = &((*p2).a);
    //   (*p1).a = 42;
    // }
    // fn f1() {
    //   var v : S;
    //   f2(&v, &v);
    // }
    Structure("S", Vector{Member("a", ty.i32())});
    Func("f2",
         Vector{
             Param("p1", ty.ptr<function>(ty("S"))),
             Param("p2", ty.ptr<function>(ty("S"))),
         },
         ty.void_(),
         Vector{
             Decl(Let("newp", AddressOf(MemberAccessor(Deref("p2"), "a")))),
             Assign(MemberAccessor(Deref("p1"), "a"), 42_a),
         });
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty("S"))),
             CallStmt(Call("f2", AddressOf("v"), AddressOf("v"))),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAliasAnalysisTest, Read_MemberAccessor) {
    // struct S { a : i32 }
    // fn f2(p1 : ptr<function, S>, p2 : ptr<function, S>) {
    //   _ = (*p2).a;
    //   *p1 = S();
    // }
    // fn f1() {
    //   var v : S;
    //   f2(&v, &v);
    // }
    Structure("S", Vector{Member("a", ty.i32())});
    Func("f2",
         Vector{
             Param("p1", ty.ptr<function>(ty("S"))),
             Param("p2", ty.ptr<function>(ty("S"))),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), MemberAccessor(Deref("p2"), "a")),
             Assign(Deref("p1"), Call("S")),
         });
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty("S"))),
             CallStmt(
                 Call("f2", AddressOf(Source{{12, 34}}, "v"), AddressOf(Source{{56, 76}}, "v"))),
         });
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(56:76 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_F(ResolverAliasAnalysisTest, Write_MemberAccessor) {
    // struct S { a : i32 }
    // fn f2(p1 : ptr<function, S>, p2 : ptr<function, S>) {
    //   _ = *p2;
    //   (*p1).a = 42;
    // }
    // fn f1() {
    //   var v : S;
    //   f2(&v, &v);
    // }
    Structure("S", Vector{Member("a", ty.i32())});
    Func("f2",
         Vector{
             Param("p1", ty.ptr<function>(ty("S"))),
             Param("p2", ty.ptr<function>(ty("S"))),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), Deref("p2")),
             Assign(MemberAccessor(Deref("p1"), "a"), 42_a),
         });
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty("S"))),
             CallStmt(
                 Call("f2", AddressOf(Source{{12, 34}}, "v"), AddressOf(Source{{56, 76}}, "v"))),
         });
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(56:76 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_F(ResolverAliasAnalysisTest, Read_MultiComponentSwizzle) {
    // fn f2(p1 : ptr<function, vec4<f32>, p2 : ptr<function, vec4<f32>) {
    //   _ = (*p2).zy;
    //   *p1 = vec4<f32>();
    // }
    // fn f1() {
    //   var v : vec4<f32>;
    //   f2(&v, &v);
    // }
    Structure("S", Vector{Member("a", ty.i32())});
    Func("f2",
         Vector{
             Param("p1", ty.ptr<function, vec4<f32>>()),
             Param("p2", ty.ptr<function, vec4<f32>>()),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), MemberAccessor(Deref("p2"), "zy")),
             Assign(Deref("p1"), Call<vec4<f32>>()),
         });
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.vec4<f32>())),
             CallStmt(
                 Call("f2", AddressOf(Source{{12, 34}}, "v"), AddressOf(Source{{56, 76}}, "v"))),
         });
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(56:76 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_F(ResolverAliasAnalysisTest, Read_MultiComponentSwizzle_FromPointer) {
    // fn f2(p1 : ptr<function, vec4<f32>, p2 : ptr<function, vec4<f32>) {
    //   _ = p2.zy;
    //   *p1 = vec4<f32>();
    // }
    // fn f1() {
    //   var v : vec4<f32>;
    //   f2(&v, &v);
    // }
    Structure("S", Vector{Member("a", ty.i32())});
    Func("f2",
         Vector{
             Param("p1", ty.ptr<function, vec4<f32>>()),
             Param("p2", ty.ptr<function, vec4<f32>>()),
         },
         ty.void_(),
         Vector{
             Assign(Phony(), MemberAccessor("p2", "zy")),
             Assign(Deref("p1"), Call<vec4<f32>>()),
         });
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.vec4<f32>())),
             CallStmt(
                 Call("f2", AddressOf(Source{{12, 34}}, "v"), AddressOf(Source{{56, 76}}, "v"))),
         });
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(56:76 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_F(ResolverAliasAnalysisTest, SinglePointerReadWrite) {
    // Test that we can both read and write from a single pointer parameter.
    //
    // fn f1(p : ptr<function, i32>) {
    //   _ = *p;
    //   *p = 42;
    // }
    // fn f2() {
    //   var v : i32;
    //   f1(&v);
    // }
    Func("f1",
         Vector{
             Param("p", ty.ptr<i32>(core::AddressSpace::kFunction)),
         },
         ty.void_(),
         Vector{
             Decl(Var("v", ty.i32())),
             Assign(Phony(), Deref("p")),
             Assign(Deref("p"), 42_a),
         });
    Func("f2", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.i32())),
             CallStmt(Call("f1", AddressOf("v"))),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAliasAnalysisTest, AliasingInsideFunction) {
    // Test that we can use two aliased pointers inside the same function they are created in.
    //
    // fn f1() {
    //   var v : i32;
    //   let p1 = &v;
    //   let p2 = &v;
    //   *p1 = 42;
    //   *p2 = 42;
    // }
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.i32())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", AddressOf("v"))),
             Assign(Deref("p1"), 42_a),
             Assign(Deref("p2"), 42_a),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAliasAnalysisTest, NonOverlappingCalls) {
    // Test that we pass the same pointer to multiple non-overlapping function calls.
    //
    // fn f2(p : ptr<function, i32>) {
    //   *p = 42;
    // }
    // fn f3(p : ptr<function, i32>) {
    //   *p = 42;
    // }
    // fn f1() {
    //   var v : i32;
    //   f2(&v);
    //   f3(&v);
    // }
    Func("f2",
         Vector{
             Param("p", ty.ptr<i32>(core::AddressSpace::kFunction)),
         },
         ty.void_(),
         Vector{
             Assign(Deref("p"), 42_a),
         });
    Func("f3",
         Vector{
             Param("p", ty.ptr<i32>(core::AddressSpace::kFunction)),
         },
         ty.void_(),
         Vector{
             Assign(Deref("p"), 42_a),
         });
    Func("f1", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.i32())),
             CallStmt(Call("f2", AddressOf("v"))),
             CallStmt(Call("f3", AddressOf("v"))),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

////////////////////////////////////////////////////////////////////////////////
// Atomics
////////////////////////////////////////////////////////////////////////////////
class AtomicPointers
    : public ResolverTestWithParam<
          std::tuple<wgsl::BuiltinFn, wgsl::BuiltinFn, core::AddressSpace, bool /* aliased */>> {
  protected:
    static constexpr std::string_view kPass = "<PASS>";

    ast::Type Ptr() {
        auto address_space = std::get<2>(GetParam());
        if (address_space == storage) {
            return ty.ptr<storage, atomic<i32>, read_write>();
        } else {
            return ty.ptr<atomic<i32>>(address_space);
        }
    }

    void SetUp() override {
        auto address_space = std::get<2>(GetParam());
        if (address_space == storage) {
            GlobalVar("v1", address_space, read_write, ty.Of<atomic<i32>>(),  //
                      Binding(0_a), Group(0_a));
            GlobalVar("v2", address_space, read_write, ty.Of<atomic<i32>>(),  //
                      Binding(1_a), Group(0_a));
        } else {
            GlobalVar("v1", address_space, ty.Of<atomic<i32>>());
            GlobalVar("v2", address_space, ty.Of<atomic<i32>>());
        }
    }

    bool IsWrite(wgsl::BuiltinFn fn) const {
        switch (fn) {
            case wgsl::BuiltinFn::kAtomicStore:
            case wgsl::BuiltinFn::kAtomicAdd:
            case wgsl::BuiltinFn::kAtomicSub:
            case wgsl::BuiltinFn::kAtomicMax:
            case wgsl::BuiltinFn::kAtomicMin:
            case wgsl::BuiltinFn::kAtomicAnd:
            case wgsl::BuiltinFn::kAtomicOr:
            case wgsl::BuiltinFn::kAtomicXor:
            case wgsl::BuiltinFn::kAtomicExchange:
            case wgsl::BuiltinFn::kAtomicCompareExchangeWeak:
                return true;
            default:
                return false;
        }
    }

    bool ShouldPass() const {
        auto [builtin_a, builtin_b, space, aliased] = GetParam();
        bool fail = aliased && (IsWrite(builtin_a) || IsWrite(builtin_b));
        return !fail;
    }

    const ast::Statement* CallBuiltin(wgsl::BuiltinFn fn, std::string_view ptr) {
        switch (fn) {
            case wgsl::BuiltinFn::kAtomicLoad:
                return CallStmt(Call(fn, ptr));
            case wgsl::BuiltinFn::kAtomicStore:
            case wgsl::BuiltinFn::kAtomicAdd:
            case wgsl::BuiltinFn::kAtomicSub:
            case wgsl::BuiltinFn::kAtomicMax:
            case wgsl::BuiltinFn::kAtomicMin:
            case wgsl::BuiltinFn::kAtomicAnd:
            case wgsl::BuiltinFn::kAtomicOr:
            case wgsl::BuiltinFn::kAtomicXor:
            case wgsl::BuiltinFn::kAtomicExchange:
                return CallStmt(Call(fn, ptr, 42_a));
            case wgsl::BuiltinFn::kAtomicCompareExchangeWeak:
                return CallStmt(Call(fn, ptr, 10_a, 42_a));
            default:
                TINT_UNIMPLEMENTED() << fn;
        }
    }

    std::string Run() {
        if (r()->Resolve()) {
            return std::string(kPass);
        }
        return r()->error();
    }
};

TEST_P(AtomicPointers, CallDirect) {
    // var<ADDRESS_SPACE> v1 : atomic<i32>;
    // var<ADDRESS_SPACE> v2 : atomic<i32>;
    //
    // fn caller() {
    //    callee(&v1, aliased ? &v1 : &v2);
    // }
    //
    // fn callee(p1 : PTR, p2 : PTR) {
    //   <builtin-a>(p1);
    //   <builtin-b>(p2);
    // }
    auto [builtin_a, builtin_b, space, aliased] = GetParam();

    Func("caller", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("callee",  //
                           AddressOf(Source{{12, 34}}, "v1"),
                           AddressOf(Source{{56, 78}}, aliased ? "v1" : "v2"))),
         });

    Func("callee", Vector{Param("p1", Ptr()), Param("p2", Ptr())}, ty.void_(),
         Vector{
             CallBuiltin(builtin_a, "p1"),
             CallBuiltin(builtin_b, "p2"),
         });

    EXPECT_EQ(Run(), ShouldPass() ? kPass : R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(AtomicPointers, CallThroughChain) {
    // var<ADDRESS_SPACE> v1 : atomic<i32>;
    // var<ADDRESS_SPACE> v2 : atomic<i32>;
    //
    // fn caller() {
    //    callee(&v1, aliased ? &v1 : &v2);
    // }
    //
    // fn f2(p1 : PTR, p2 : PTR) {
    //    f1(p1, p2);
    // }
    //
    // fn f1(p1 : PTR, p2 : PTR) {
    //    callee(p1, p2);
    // }
    //
    // fn callee(p1 : PTR, p2 : PTR) {
    //   <builtin-a>(p1);
    //   <builtin-b>(p2);
    // }
    auto [builtin_a, builtin_b, space, aliased] = GetParam();

    Func("caller", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("callee",  //
                           AddressOf(Source{{12, 34}}, "v1"),
                           AddressOf(Source{{56, 78}}, aliased ? "v1" : "v2"))),
         });

    Func("f2", Vector{Param("p1", Ptr()), Param("p2", Ptr())}, ty.void_(),
         Vector{
             CallStmt(Call("f1", "p1", "p2")),
         });

    Func("f1", Vector{Param("p1", Ptr()), Param("p2", Ptr())}, ty.void_(),
         Vector{
             CallStmt(Call("callee", "p1", "p2")),
         });

    Func("callee", Vector{Param("p1", Ptr()), Param("p2", Ptr())}, ty.void_(),
         Vector{
             CallBuiltin(builtin_a, "p1"),
             CallBuiltin(builtin_b, "p2"),
         });

    EXPECT_EQ(Run(), ShouldPass() ? kPass : R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(AtomicPointers, ReadWriteAcrossDifferentFunctions) {
    // var<ADDRESS_SPACE> v1 : atomic<i32>;
    // var<ADDRESS_SPACE> v2 : atomic<i32>;
    //
    // fn caller() {
    //   f(&v1, aliased ? &v1 : &v2);
    // }
    //
    // fn f(p1 : PTR, p2 : PTR) {
    //    f1(p1);
    //    f2(p2);
    // }
    //
    // fn f1(p : PTR) {
    //   <builtin-a>(p);
    // }
    //
    // fn f2(p : PTR) {
    //   <builtin-b>(p);
    // }
    auto [builtin_a, builtin_b, space, aliased] = GetParam();

    Func("caller", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("f",  //
                           AddressOf(Source{{12, 34}}, "v1"),
                           AddressOf(Source{{56, 78}}, aliased ? "v1" : "v2"))),
         });

    Func("f", Vector{Param("p1", Ptr()), Param("p2", Ptr())}, ty.void_(),
         Vector{
             CallStmt(Call("f1", "p1")),
             CallStmt(Call("f2", "p2")),
         });

    Func("f1", Vector{Param("p", Ptr())}, ty.void_(),
         Vector{
             CallBuiltin(builtin_a, "p"),
         });

    Func("f2", Vector{Param("p", Ptr())}, ty.void_(),
         Vector{
             CallBuiltin(builtin_b, "p"),
         });

    EXPECT_EQ(Run(), ShouldPass() ? kPass : R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

std::array kAtomicFns{
    wgsl::BuiltinFn::kAtomicLoad,
    wgsl::BuiltinFn::kAtomicStore,
    wgsl::BuiltinFn::kAtomicAdd,
    wgsl::BuiltinFn::kAtomicSub,
    wgsl::BuiltinFn::kAtomicMax,
    wgsl::BuiltinFn::kAtomicMin,
    wgsl::BuiltinFn::kAtomicAnd,
    wgsl::BuiltinFn::kAtomicOr,
    wgsl::BuiltinFn::kAtomicXor,
    wgsl::BuiltinFn::kAtomicExchange,
    wgsl::BuiltinFn::kAtomicCompareExchangeWeak,
};

INSTANTIATE_TEST_SUITE_P(ResolverAliasAnalysisTest,
                         AtomicPointers,
                         ::testing::Combine(::testing::ValuesIn(kAtomicFns),
                                            ::testing::ValuesIn(kAtomicFns),
                                            ::testing::Values(core::AddressSpace::kWorkgroup,
                                                              core::AddressSpace::kStorage),
                                            ::testing::Values(true, false)));

////////////////////////////////////////////////////////////////////////////////
// WorkgroupUniformLoad
////////////////////////////////////////////////////////////////////////////////
enum class WorkgroupUniformLoadAction {
    kRead,
    kWrite,
    kWorkgroupUniformLoad,
};

std::array kWorkgroupUniformLoadActions{
    WorkgroupUniformLoadAction::kRead,
    WorkgroupUniformLoadAction::kWrite,
    WorkgroupUniformLoadAction::kWorkgroupUniformLoad,
};

class WorkgroupUniformLoad
    : public ResolverTestWithParam<
          std::tuple<WorkgroupUniformLoadAction, WorkgroupUniformLoadAction, bool>> {
  protected:
    static constexpr std::string_view kPass = "<PASS>";

    void SetUp() override {
        GlobalVar("v1", workgroup, ty.i32());
        GlobalVar("v2", workgroup, ty.i32());
    }

    const ast::Statement* Do(WorkgroupUniformLoadAction action, std::string_view ptr) {
        switch (action) {
            case WorkgroupUniformLoadAction::kRead:
                return Assign(Phony(), Deref(ptr));
            case WorkgroupUniformLoadAction::kWrite:
                return Assign(Deref(ptr), 42_a);
            case WorkgroupUniformLoadAction::kWorkgroupUniformLoad:
                return Assign(Phony(), Call(wgsl::BuiltinFn::kWorkgroupUniformLoad, ptr));
        }
        return nullptr;
    }

    bool IsWrite(WorkgroupUniformLoadAction action) const {
        return action == WorkgroupUniformLoadAction::kWrite;
    }

    bool ShouldPass() const {
        auto [action_a, action_b, aliased] = GetParam();
        bool fail = aliased && (IsWrite(action_a) || IsWrite(action_b));
        return !fail;
    }

    std::string Run() {
        if (r()->Resolve()) {
            return std::string(kPass);
        }
        return r()->error();
    }
};

TEST_P(WorkgroupUniformLoad, CallDirect) {
    // var<workgroup> v1 : i32;
    // var<workgroup> v2 : i32;
    //
    // fn caller() {
    //    callee(&v1, aliased ? &v1 : &v2);
    // }
    //
    // fn callee(p1 : PTR, p2 : PTR) {
    //   <action-a>(p1);
    //   <action-b>(p2);
    // }
    auto [action_a, action_b, aliased] = GetParam();

    Func("caller", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("callee",  //
                           AddressOf(Source{{12, 34}}, "v1"),
                           AddressOf(Source{{56, 78}}, aliased ? "v1" : "v2"))),
         });

    Func("callee",
         Vector{Param("p1", ty.ptr<workgroup, i32>()), Param("p2", ty.ptr<workgroup, i32>())},
         ty.void_(),
         Vector{
             Do(action_a, "p1"),
             Do(action_b, "p2"),
         });

    EXPECT_EQ(Run(), ShouldPass() ? kPass : R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(WorkgroupUniformLoad, CallThroughChain) {
    // var<workgroup> v1 : i32;
    // var<workgroup> v2 : i32;
    //
    // fn caller() {
    //    callee(&v1, aliased ? &v1 : &v2);
    // }
    //
    // fn f2(p1 : PTR, p2 : PTR) {
    //    f1(p1, p2);
    // }
    //
    // fn f1(p1 : PTR, p2 : PTR) {
    //    callee(p1, p2);
    // }
    //
    // fn callee(p1 : PTR, p2 : PTR) {
    //   <action-a>(p1);
    //   <action-b>(p2);
    // }
    auto [action_a, action_b, aliased] = GetParam();

    Func("caller", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("callee",  //
                           AddressOf(Source{{12, 34}}, "v1"),
                           AddressOf(Source{{56, 78}}, aliased ? "v1" : "v2"))),
         });

    Func("f2", Vector{Param("p1", ty.ptr<workgroup, i32>()), Param("p2", ty.ptr<workgroup, i32>())},
         ty.void_(),
         Vector{
             CallStmt(Call("f1", "p1", "p2")),
         });

    Func("f1", Vector{Param("p1", ty.ptr<workgroup, i32>()), Param("p2", ty.ptr<workgroup, i32>())},
         ty.void_(),
         Vector{
             CallStmt(Call("callee", "p1", "p2")),
         });

    Func("callee",
         Vector{Param("p1", ty.ptr<workgroup, i32>()), Param("p2", ty.ptr<workgroup, i32>())},
         ty.void_(),
         Vector{
             Do(action_a, "p1"),
             Do(action_b, "p2"),
         });

    EXPECT_EQ(Run(), ShouldPass() ? kPass : R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

TEST_P(WorkgroupUniformLoad, ReadWriteAcrossDifferentFunctions) {
    // var<workgroup> v1 : i32;
    // var<workgroup> v2 : i32;
    //
    // fn caller() {
    //   f(&v1, aliased ? &v1 : &v2);
    // }
    //
    // fn f(p1 : PTR, p2 : PTR) {
    //    f1(p1);
    //    f2(p2);
    // }
    //
    // fn f1(p : PTR) {
    //   <action-a>(p);
    // }
    //
    // fn f2(p : PTR) {
    //   <action-b>(p);
    // }
    auto [action_a, action_b, aliased] = GetParam();

    Func("caller", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("f",  //
                           AddressOf(Source{{12, 34}}, "v1"),
                           AddressOf(Source{{56, 78}}, aliased ? "v1" : "v2"))),
         });

    Func("f", Vector{Param("p1", ty.ptr<workgroup, i32>()), Param("p2", ty.ptr<workgroup, i32>())},
         ty.void_(),
         Vector{
             CallStmt(Call("f1", "p1")),
             CallStmt(Call("f2", "p2")),
         });

    Func("f1", Vector{Param("p", ty.ptr<workgroup, i32>())}, ty.void_(), Vector{Do(action_a, "p")});

    Func("f2", Vector{Param("p", ty.ptr<workgroup, i32>())}, ty.void_(), Vector{Do(action_b, "p")});

    EXPECT_EQ(Run(), ShouldPass() ? kPass : R"(56:78 error: invalid aliased pointer argument
12:34 note: aliases with another argument passed here)");
}

INSTANTIATE_TEST_SUITE_P(ResolverAliasAnalysisTest,
                         WorkgroupUniformLoad,
                         ::testing::Combine(::testing::ValuesIn(kWorkgroupUniformLoadActions),
                                            ::testing::ValuesIn(kWorkgroupUniformLoadActions),
                                            ::testing::Values(true, false)));

}  // namespace
}  // namespace tint::resolver
