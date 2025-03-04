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

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/tint/lang/wgsl/ast/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::ast {
namespace {

struct Allocator {
    template <typename T, typename... ARGS>
    T* Create(ARGS&&... args) {
        return alloc.Create<T>(this, std::forward<ARGS>(args)...);
    }

  private:
    BlockAllocator<ast::Node> alloc;
};

struct TestNode : public Castable<TestNode, ast::Node> {
    TestNode(Allocator* alloc,
             Symbol n,
             const TestNode* Testnode_a = nullptr,
             const TestNode* Testnode_b = nullptr,
             const TestNode* Testnode_c = nullptr)
        : Base(GenerationID{}, ast::NodeID{}, Source{}),
          allocator(alloc),
          name(n),
          a(Testnode_a),
          b(Testnode_b),
          c(Testnode_c) {}
    TestNode(TestNode&&) = delete;
    Allocator* const allocator;
    Symbol name;
    const TestNode* a = nullptr;
    const TestNode* b = nullptr;
    const TestNode* c = nullptr;
    Vector<const TestNode*, 8> vec;

    TestNode* Clone(ast::CloneContext& ctx) const override {
        auto* out = allocator->Create<TestNode>(ctx.Clone(name));
        out->a = ctx.Clone(a);
        out->b = ctx.Clone(b);
        out->c = ctx.Clone(c);
        out->vec = ctx.Clone(vec);
        return out;
    }
};

struct Replaceable : public Castable<Replaceable, TestNode> {
    Replaceable(Allocator* alloc,
                Symbol n,
                const TestNode* Testnode_a = nullptr,
                const TestNode* Testnode_b = nullptr,
                const TestNode* Testnode_c = nullptr)
        : Base(alloc, n, Testnode_a, Testnode_b, Testnode_c) {}
};

struct Replacement : public Castable<Replacement, Replaceable> {
    Replacement(Allocator* alloc, Symbol n) : Base(alloc, n) {}
};

struct IDTestNode : public Castable<IDTestNode, ast::Node> {
    IDTestNode(Allocator* alloc, GenerationID program_id, GenerationID cid)
        : Base(program_id, ast::NodeID{}, Source{}), allocator(alloc), cloned_id(cid) {}

    Allocator* const allocator;
    const GenerationID cloned_id;

    IDTestNode* Clone(ast::CloneContext&) const override {
        return allocator->Create<IDTestNode>(cloned_id, cloned_id);
    }
};

using ASTCloneContextTestNodeTest = ::testing::Test;
using ASTCloneContextTestNodeDeathTest = ASTCloneContextTestNodeTest;

TEST_F(ASTCloneContextTestNodeTest, Clone) {
    Allocator alloc;

    ProgramBuilder builder;
    TestNode* original_root;
    {
        auto* a_b = alloc.Create<TestNode>(builder.Symbols().New("a->b"));
        auto* a = alloc.Create<TestNode>(builder.Symbols().New("a"), nullptr, a_b);
        auto* b_a = a;  // Aliased
        auto* b_b = alloc.Create<TestNode>(builder.Symbols().New("b->b"));
        auto* b = alloc.Create<TestNode>(builder.Symbols().New("b"), b_a, b_b);
        auto* c = b;  // Aliased
        original_root = alloc.Create<TestNode>(builder.Symbols().New("root"), a, b, c);
    }
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //        N  <──────┐        N  <───────────────┘
    //   ╭────┼────╮    │   ╭────┼────╮
    //  (a)  (b)  (c)   │  (a)  (b)  (c)
    //        N         └───┘    N
    //
    // N: TestNode

    ProgramBuilder cloned;
    auto* cloned_root = CloneContext(&cloned, original.ID()).Clone(original_root);

    EXPECT_NE(cloned_root->a, nullptr);
    EXPECT_EQ(cloned_root->a->a, nullptr);
    EXPECT_NE(cloned_root->a->b, nullptr);
    EXPECT_EQ(cloned_root->a->c, nullptr);
    EXPECT_NE(cloned_root->b, nullptr);
    EXPECT_NE(cloned_root->b->a, nullptr);
    EXPECT_NE(cloned_root->b->b, nullptr);
    EXPECT_EQ(cloned_root->b->c, nullptr);
    EXPECT_NE(cloned_root->c, nullptr);

    EXPECT_NE(cloned_root->a, original_root->a);
    EXPECT_NE(cloned_root->a->b, original_root->a->b);
    EXPECT_NE(cloned_root->b, original_root->b);
    EXPECT_NE(cloned_root->b->a, original_root->b->a);
    EXPECT_NE(cloned_root->b->b, original_root->b->b);
    EXPECT_NE(cloned_root->c, original_root->c);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("a->b"));
    EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->b->b->name, cloned.Symbols().Get("b->b"));

    EXPECT_NE(cloned_root->b->a, cloned_root->a);  // De-aliased
    EXPECT_NE(cloned_root->c, cloned_root->b);     // De-aliased

    EXPECT_EQ(cloned_root->b->a->name, cloned_root->a->name);
    EXPECT_EQ(cloned_root->c->name, cloned_root->b->name);
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithReplaceAll_TestNode) {
    Allocator alloc;

    ProgramBuilder builder;
    TestNode* original_root;
    {
        auto* a_b = alloc.Create<Replaceable>(builder.Symbols().New("a->b"));
        auto* a = alloc.Create<TestNode>(builder.Symbols().New("a"), nullptr, a_b);
        auto* b_a = a;  // Aliased
        auto* b = alloc.Create<Replaceable>(builder.Symbols().New("b"), b_a, nullptr);
        auto* c = b;  // Aliased
        original_root = alloc.Create<TestNode>(builder.Symbols().New("root"), a, b, c);
    }
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //        N  <──────┐        R  <───────────────┘
    //   ╭────┼────╮    │   ╭────┼────╮
    //  (a)  (b)  (c)   │  (a)  (b)  (c)
    //        R         └───┘
    //
    // N: TestNode
    // R: Replaceable

    ProgramBuilder cloned;

    CloneContext ctx(&cloned, original.ID());
    ctx.ReplaceAll([&](const Replaceable* in) {
        auto out_name = cloned.Symbols().Register("replacement:" + in->name.Name());
        auto b_name = cloned.Symbols().Register("replacement-child:" + in->name.Name());
        auto* out = alloc.Create<Replacement>(out_name);
        out->b = alloc.Create<TestNode>(b_name);
        out->c = ctx.Clone(in->a);
        return out;
    });
    auto* cloned_root = ctx.Clone(original_root);

    //                         root
    //        ╭─────────────────┼──────────────────╮
    //       (a)               (b)                (c)
    //        N  <──────┐       R  <───────────────┘
    //   ╭────┼────╮    │  ╭────┼────╮
    //  (a)  (b)  (c)   │ (a)  (b)  (c)
    //        R         │       N    |
    //   ╭────┼────╮    └────────────┘
    //  (a)  (b)  (c)
    //        N
    //
    // N: TestNode
    // R: Replacement

    EXPECT_NE(cloned_root->a, nullptr);
    EXPECT_EQ(cloned_root->a->a, nullptr);
    EXPECT_NE(cloned_root->a->b, nullptr);     // Replaced
    EXPECT_EQ(cloned_root->a->b->a, nullptr);  // From replacement
    EXPECT_NE(cloned_root->a->b->b, nullptr);  // From replacement
    EXPECT_EQ(cloned_root->a->b->c, nullptr);  // From replacement
    EXPECT_EQ(cloned_root->a->c, nullptr);
    EXPECT_NE(cloned_root->b, nullptr);
    EXPECT_EQ(cloned_root->b->a, nullptr);  // From replacement
    EXPECT_NE(cloned_root->b->b, nullptr);  // From replacement
    EXPECT_NE(cloned_root->b->c, nullptr);  // From replacement
    EXPECT_NE(cloned_root->c, nullptr);

    EXPECT_NE(cloned_root->a, original_root->a);
    EXPECT_NE(cloned_root->a->b, original_root->a->b);
    EXPECT_NE(cloned_root->b, original_root->b);
    EXPECT_NE(cloned_root->b->a, original_root->b->a);
    EXPECT_NE(cloned_root->c, original_root->c);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("replacement:a->b"));
    EXPECT_EQ(cloned_root->a->b->b->name, cloned.Symbols().Get("replacement-child:a->b"));
    EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("replacement:b"));
    EXPECT_EQ(cloned_root->b->b->name, cloned.Symbols().Get("replacement-child:b"));

    EXPECT_NE(cloned_root->b->c, cloned_root->a);  // De-aliased
    EXPECT_NE(cloned_root->c, cloned_root->b);     // De-aliased

    EXPECT_EQ(cloned_root->b->c->name, cloned_root->a->name);
    EXPECT_EQ(cloned_root->c->name, cloned_root->b->name);

    EXPECT_FALSE(Is<Replacement>(cloned_root->a));
    EXPECT_TRUE(Is<Replacement>(cloned_root->a->b));
    EXPECT_FALSE(Is<Replacement>(cloned_root->a->b->b));
    EXPECT_TRUE(Is<Replacement>(cloned_root->b));
    EXPECT_FALSE(Is<Replacement>(cloned_root->b->b));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithReplaceAll_Symbols) {
    Allocator alloc;

    ProgramBuilder builder;
    TestNode* original_root;
    {
        auto* a_b = alloc.Create<TestNode>(builder.Symbols().New("a->b"));
        auto* a = alloc.Create<TestNode>(builder.Symbols().New("a"), nullptr, a_b);
        auto* b_a = a;  // Aliased
        auto* b_b = alloc.Create<TestNode>(builder.Symbols().New("b->b"));
        auto* b = alloc.Create<TestNode>(builder.Symbols().New("b"), b_a, b_b);
        auto* c = b;  // Aliased
        original_root = alloc.Create<TestNode>(builder.Symbols().New("root"), a, b, c);
    }
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //        N  <──────┐        N  <───────────────┘
    //   ╭────┼────╮    │   ╭────┼────╮
    //  (a)  (b)  (c)   │  (a)  (b)  (c)
    //        N         └───┘    N
    //
    // N: TestNode

    ProgramBuilder cloned;
    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .ReplaceAll([&](Symbol sym) {
                                auto in = sym.Name();
                                auto out = "transformed<" + in + ">";
                                return cloned.Symbols().New(out);
                            })
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("transformed<root>"));
    EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("transformed<a>"));
    EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("transformed<a->b>"));
    EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("transformed<b>"));
    EXPECT_EQ(cloned_root->b->b->name, cloned.Symbols().Get("transformed<b->b>"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithoutTransform) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_Testnode = a.Create<TestNode>(builder.Symbols().New("root"));
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    ctx.ReplaceAll([&](const TestNode*) {
        return a.Create<Replacement>(builder.Symbols().New("<unexpected-Testnode>"));
    });

    auto* cloned_Testnode = ctx.CloneWithoutTransform(original_Testnode);
    EXPECT_NE(cloned_Testnode, original_Testnode);
    EXPECT_EQ(cloned_Testnode->name, cloned.Symbols().Get("root"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithReplacePointer) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().New("root"));
    original_root->a = a.Create<TestNode>(builder.Symbols().New("a"));
    original_root->b = a.Create<TestNode>(builder.Symbols().New("b"));
    original_root->c = a.Create<TestNode>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //                        Replaced

    ProgramBuilder cloned;
    auto* replacement = a.Create<TestNode>(cloned.Symbols().New("replacement"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .Replace(original_root->b, replacement)
                            .Clone(original_root);

    EXPECT_NE(cloned_root->a, replacement);
    EXPECT_EQ(cloned_root->b, replacement);
    EXPECT_NE(cloned_root->c, replacement);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("replacement"));
    EXPECT_EQ(cloned_root->c->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithRepeatedImmediateReplacePointer) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().New("root"));
    original_root->a = a.Create<TestNode>(builder.Symbols().New("a"));
    original_root->b = a.Create<TestNode>(builder.Symbols().New("b"));
    original_root->c = a.Create<TestNode>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    CloneContext ctx(&cloned, original.ID());

    // Demonstrate that ctx.Replace() can be called multiple times to update the replacement of a
    // Testnode.

    auto* replacement_x =
        a.Create<TestNode>(cloned.Symbols().New("replacement_x"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, replacement_x);

    auto* replacement_y =
        a.Create<TestNode>(cloned.Symbols().New("replacement_y"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, replacement_y);

    auto* replacement_z =
        a.Create<TestNode>(cloned.Symbols().New("replacement_z"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, replacement_z);

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_NE(cloned_root->a, replacement_z);
    EXPECT_EQ(cloned_root->b, replacement_z);
    EXPECT_NE(cloned_root->c, replacement_z);

    EXPECT_EQ(replacement_z->a, replacement_y);
    EXPECT_EQ(replacement_y->a, replacement_x);
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithReplaceFunction) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().New("root"));
    original_root->a = a.Create<TestNode>(builder.Symbols().New("a"));
    original_root->b = a.Create<TestNode>(builder.Symbols().New("b"));
    original_root->c = a.Create<TestNode>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //                        Replaced

    ProgramBuilder cloned;
    auto* replacement = a.Create<TestNode>(cloned.Symbols().New("replacement"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .Replace(original_root->b, [=] { return replacement; })
                            .Clone(original_root);

    EXPECT_NE(cloned_root->a, replacement);
    EXPECT_EQ(cloned_root->b, replacement);
    EXPECT_NE(cloned_root->c, replacement);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("replacement"));
    EXPECT_EQ(cloned_root->c->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithRepeatedImmediateReplaceFunction) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().New("root"));
    original_root->a = a.Create<TestNode>(builder.Symbols().New("a"));
    original_root->b = a.Create<TestNode>(builder.Symbols().New("b"));
    original_root->c = a.Create<TestNode>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    CloneContext ctx(&cloned, original.ID());

    // Demonstrate that ctx.Replace() can be called multiple times to update the replacement of a
    // Testnode.

    TestNode* replacement_x =
        a.Create<TestNode>(cloned.Symbols().New("replacement_x"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, [&] { return replacement_x; });

    TestNode* replacement_y =
        a.Create<TestNode>(cloned.Symbols().New("replacement_y"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, [&] { return replacement_y; });

    TestNode* replacement_z =
        a.Create<TestNode>(cloned.Symbols().New("replacement_z"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, [&] { return replacement_z; });

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_NE(cloned_root->a, replacement_z);
    EXPECT_EQ(cloned_root->b, replacement_z);
    EXPECT_NE(cloned_root->c, replacement_z);

    EXPECT_EQ(replacement_z->a, replacement_y);
    EXPECT_EQ(replacement_y->a, replacement_x);
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithRemove) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .Remove(original_root->vec, original_root->vec[1])
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 2u);

    EXPECT_NE(cloned_root->vec[0], cloned_root->a);
    EXPECT_NE(cloned_root->vec[1], cloned_root->c);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertFront) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertFront(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_NE(cloned_root->vec[0], cloned_root->a);
    EXPECT_NE(cloned_root->vec[1], cloned_root->b);
    EXPECT_NE(cloned_root->vec[2], cloned_root->c);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertFrontFunction) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertFront(original_root->vec,
                         [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_NE(cloned_root->vec[0], cloned_root->a);
    EXPECT_NE(cloned_root->vec[1], cloned_root->b);
    EXPECT_NE(cloned_root->vec[2], cloned_root->c);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertFront_Empty) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertFront(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertFront_Empty_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertFront(original_root->vec,
                         [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBack) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertBack(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("c"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBack_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertBack(original_root->vec,
                        [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("c"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBack_Empty) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertBack(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBack_Empty_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertBack(original_root->vec,
                        [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertFrontAndBack_Empty) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion_back = a.Create<TestNode>(cloned.Symbols().New("insertion_back"));
    auto* insertion_front = a.Create<TestNode>(cloned.Symbols().New("insertion_front"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertBack(original_root->vec, insertion_back)
                            .InsertFront(original_root->vec, insertion_front)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 2u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion_front"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_back"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertFrontAndBack_Empty_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertBack(original_root->vec,
                        [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion_back")); })
            .InsertFront(
                original_root->vec,
                [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion_front")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 2u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion_front"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_back"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBefore) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertBefore(original_root->vec, original_root->vec[1], insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBefore_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertBefore(original_root->vec, original_root->vec[1],
                          [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertAfter) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, original.ID())
                            .InsertAfter(original_root->vec, original_root->vec[1], insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertAfter_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertAfter(original_root->vec, original_root->vec[1],
                         [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertAfterInVectorNodeClone) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    ctx.ReplaceAll([&](const Replaceable* r) {
        auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));
        ctx.InsertAfter(original_root->vec, r, insertion);
        return nullptr;
    });

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertAfterInVectorNodeClone_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    ctx.ReplaceAll([&](const Replaceable* r) {
        ctx.InsertAfter(original_root->vec, r,
                        [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); });
        return nullptr;
    });

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBackInVectorNodeClone) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    ctx.ReplaceAll([&](const Replaceable* /*r*/) {
        auto* insertion = a.Create<TestNode>(cloned.Symbols().New("insertion"));
        ctx.InsertBack(original_root->vec, insertion);
        return nullptr;
    });

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("c"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBackInVectorNodeClone_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    ctx.ReplaceAll([&](const Replaceable* /*r*/) {
        ctx.InsertBack(original_root->vec,
                       [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion")); });
        return nullptr;
    });

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("c"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBeforeAndAfterRemoved) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion_before = a.Create<TestNode>(cloned.Symbols().New("insertion_before"));
    auto* insertion_after = a.Create<TestNode>(cloned.Symbols().New("insertion_after"));

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertBefore(original_root->vec, original_root->vec[1], insertion_before)
            .InsertAfter(original_root->vec, original_root->vec[1], insertion_after)
            .Remove(original_root->vec, original_root->vec[1])
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_before"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion_after"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeTest, CloneWithInsertBeforeAndAfterRemoved_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<TestNode>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<TestNode>(builder.Symbols().Register("a")),
        a.Create<TestNode>(builder.Symbols().Register("b")),
        a.Create<TestNode>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, original.ID())
            .InsertBefore(
                original_root->vec, original_root->vec[1],
                [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion_before")); })
            .InsertAfter(
                original_root->vec, original_root->vec[1],
                [&] { return a.Create<TestNode>(cloned.Symbols().New("insertion_after")); })
            .Remove(original_root->vec, original_root->vec[1])
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_before"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion_after"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ASTCloneContextTestNodeDeathTest, CloneWithReplaceAll_SameTypeTwice) {
    std::string Testnode_name = TypeInfo::Of<TestNode>().name;

    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, original.ID());
            ctx.ReplaceAll([](const TestNode*) { return nullptr; });
            ctx.ReplaceAll([](const TestNode*) { return nullptr; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll() called with a handler for type " +
                           Testnode_name + " that is already handled by a handler for type " +
                           Testnode_name));
}

TEST_F(ASTCloneContextTestNodeDeathTest, CloneWithReplaceAll_BaseThenDerived) {
    std::string Testnode_name = TypeInfo::Of<TestNode>().name;
    std::string replaceable_name = TypeInfo::Of<Replaceable>().name;

    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, original.ID());
            ctx.ReplaceAll([](const TestNode*) { return nullptr; });
            ctx.ReplaceAll([](const Replaceable*) { return nullptr; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll() called with a handler for type " +
                           replaceable_name + " that is already handled by a handler for type " +
                           Testnode_name));
}

TEST_F(ASTCloneContextTestNodeDeathTest, CloneWithReplaceAll_DerivedThenBase) {
    std::string Testnode_name = TypeInfo::Of<TestNode>().name;
    std::string replaceable_name = TypeInfo::Of<Replaceable>().name;

    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, original.ID());
            ctx.ReplaceAll([](const Replaceable*) { return nullptr; });
            ctx.ReplaceAll([](const TestNode*) { return nullptr; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll() called with a handler for type " +
                           Testnode_name + " that is already handled by a handler for type " +
                           replaceable_name));
}

using ASTCloneContextTest = ::testing::Test;
using ASTCloneContextDeathTest = ASTCloneContextTest;

TEST_F(ASTCloneContextDeathTest, CloneWithReplaceAll_SymbolsTwice) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, original.ID());
            ctx.ReplaceAll([](const Symbol s) { return s; });
            ctx.ReplaceAll([](const Symbol s) { return s; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll(const SymbolTransform&) called "
                           "multiple times on the same CloneContext"));
}

TEST_F(ASTCloneContextTest, CloneNewUnnamedSymbols) {
    ProgramBuilder builder;
    Symbol old_a = builder.Symbols().New();
    Symbol old_b = builder.Symbols().New();
    Symbol old_c = builder.Symbols().New();
    EXPECT_EQ(old_a.Name(), "tint_symbol");
    EXPECT_EQ(old_b.Name(), "tint_symbol_1");
    EXPECT_EQ(old_c.Name(), "tint_symbol_2");

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    Symbol new_x = cloned.Symbols().New();
    Symbol new_a = ctx.Clone(old_a);
    Symbol new_y = cloned.Symbols().New();
    Symbol new_b = ctx.Clone(old_b);
    Symbol new_z = cloned.Symbols().New();
    Symbol new_c = ctx.Clone(old_c);

    EXPECT_EQ(new_x.Name(), "tint_symbol");
    EXPECT_EQ(new_a.Name(), "tint_symbol_1");
    EXPECT_EQ(new_y.Name(), "tint_symbol_2");
    EXPECT_EQ(new_b.Name(), "tint_symbol_1_1");
    EXPECT_EQ(new_z.Name(), "tint_symbol_3");
    EXPECT_EQ(new_c.Name(), "tint_symbol_2_1");
}

TEST_F(ASTCloneContextTest, CloneNewSymbols) {
    ProgramBuilder builder;
    Symbol old_a = builder.Symbols().New("a");
    Symbol old_b = builder.Symbols().New("b");
    Symbol old_c = builder.Symbols().New("c");
    EXPECT_EQ(old_a.Name(), "a");
    EXPECT_EQ(old_b.Name(), "b");
    EXPECT_EQ(old_c.Name(), "c");

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, original.ID());
    Symbol new_x = cloned.Symbols().New("a");
    Symbol new_a = ctx.Clone(old_a);
    Symbol new_y = cloned.Symbols().New("b");
    Symbol new_b = ctx.Clone(old_b);
    Symbol new_z = cloned.Symbols().New("c");
    Symbol new_c = ctx.Clone(old_c);

    EXPECT_EQ(new_x.Name(), "a");
    EXPECT_EQ(new_a.Name(), "a_1");
    EXPECT_EQ(new_y.Name(), "b");
    EXPECT_EQ(new_b.Name(), "b_1");
    EXPECT_EQ(new_z.Name(), "c");
    EXPECT_EQ(new_c.Name(), "c_1");
}

TEST_F(ASTCloneContextTest, GenerationIDs) {
    ProgramBuilder dst;
    Program src(ProgramBuilder{});
    CloneContext ctx(&dst, src.ID());
    Allocator allocator;
    auto* cloned = ctx.Clone(allocator.Create<IDTestNode>(src.ID(), dst.ID()));
    EXPECT_EQ(cloned->generation_id, dst.ID());
}

TEST_F(ASTCloneContextDeathTest, GenerationIDs_Clone_ObjectNotOwnedBySrc) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder dst;
            Program src(ProgramBuilder{});
            CloneContext ctx(&dst, src.ID());
            Allocator allocator;
            ctx.Clone(allocator.Create<IDTestNode>(GenerationID::New(), dst.ID()));
        },
        testing::HasSubstr(
            R"(internal compiler error: TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, object))"));
}

TEST_F(ASTCloneContextDeathTest, GenerationIDs_Clone_ObjectNotOwnedByDst) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder dst;
            Program src(ProgramBuilder{});
            CloneContext ctx(&dst, src.ID());
            Allocator allocator;
            ctx.Clone(allocator.Create<IDTestNode>(src.ID(), GenerationID::New()));
        },
        testing::HasSubstr(
            R"(internal compiler error: TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, out))"));
}

}  // namespace
}  // namespace tint::ast

TINT_INSTANTIATE_TYPEINFO(tint::ast::TestNode);
TINT_INSTANTIATE_TYPEINFO(tint::ast::Replaceable);
TINT_INSTANTIATE_TYPEINFO(tint::ast::Replacement);
TINT_INSTANTIATE_TYPEINFO(tint::ast::IDTestNode);
