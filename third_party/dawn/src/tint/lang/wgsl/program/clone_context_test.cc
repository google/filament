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
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::program {
namespace {

struct Allocator {
    template <typename T, typename... ARGS>
    T* Create(ARGS&&... args) {
        return alloc.Create<T>(this, std::forward<ARGS>(args)...);
    }

  private:
    BlockAllocator<ast::Node> alloc;
};

struct Node : public Castable<Node, ast::Node> {
    Node(Allocator* alloc,
         Symbol n,
         const Node* node_a = nullptr,
         const Node* node_b = nullptr,
         const Node* node_c = nullptr)
        : Base(GenerationID{}, ast::NodeID{}, Source{}),
          allocator(alloc),
          name(n),
          a(node_a),
          b(node_b),
          c(node_c) {}
    Node(Node&&) = delete;
    Allocator* const allocator;
    Symbol name;
    const Node* a = nullptr;
    const Node* b = nullptr;
    const Node* c = nullptr;
    Vector<const Node*, 8> vec;

    Node* Clone(ast::CloneContext& ctx) const override {
        auto* out = allocator->Create<Node>(ctx.Clone(name));
        out->a = ctx.Clone(a);
        out->b = ctx.Clone(b);
        out->c = ctx.Clone(c);
        out->vec = ctx.Clone(vec);
        return out;
    }
};

struct Replaceable : public Castable<Replaceable, Node> {
    Replaceable(Allocator* alloc,
                Symbol n,
                const Node* node_a = nullptr,
                const Node* node_b = nullptr,
                const Node* node_c = nullptr)
        : Base(alloc, n, node_a, node_b, node_c) {}
};

struct Replacement : public Castable<Replacement, Replaceable> {
    Replacement(Allocator* alloc, Symbol n) : Base(alloc, n) {}
};

struct IDNode : public Castable<IDNode, ast::Node> {
    IDNode(Allocator* alloc, GenerationID program_id, GenerationID cid)
        : Base(program_id, ast::NodeID{}, Source{}), allocator(alloc), cloned_id(cid) {}

    Allocator* const allocator;
    const GenerationID cloned_id;

    IDNode* Clone(ast::CloneContext&) const override {
        return allocator->Create<IDNode>(cloned_id, cloned_id);
    }
};

using ProgramCloneContextNodeTest = ::testing::Test;
using ProgramCloneContextNodeDeathTest = ProgramCloneContextNodeTest;

TEST_F(ProgramCloneContextNodeTest, Clone) {
    Allocator alloc;

    ProgramBuilder builder;
    Node* original_root;
    {
        auto* a_b = alloc.Create<Node>(builder.Symbols().New("a->b"));
        auto* a = alloc.Create<Node>(builder.Symbols().New("a"), nullptr, a_b);
        auto* b_a = a;  // Aliased
        auto* b_b = alloc.Create<Node>(builder.Symbols().New("b->b"));
        auto* b = alloc.Create<Node>(builder.Symbols().New("b"), b_a, b_b);
        auto* c = b;  // Aliased
        original_root = alloc.Create<Node>(builder.Symbols().New("root"), a, b, c);
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
    // N: Node

    ProgramBuilder cloned;
    auto* cloned_root = CloneContext(&cloned, &original).Clone(original_root);

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

TEST_F(ProgramCloneContextNodeTest, CloneWithReplaceAll_Node) {
    Allocator alloc;

    ProgramBuilder builder;
    Node* original_root;
    {
        auto* a_b = alloc.Create<Replaceable>(builder.Symbols().New("a->b"));
        auto* a = alloc.Create<Node>(builder.Symbols().New("a"), nullptr, a_b);
        auto* b_a = a;  // Aliased
        auto* b = alloc.Create<Replaceable>(builder.Symbols().New("b"), b_a, nullptr);
        auto* c = b;  // Aliased
        original_root = alloc.Create<Node>(builder.Symbols().New("root"), a, b, c);
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
    // N: Node
    // R: Replaceable

    ProgramBuilder cloned;

    CloneContext ctx(&cloned, &original);
    ctx.ReplaceAll([&](const Replaceable* in) {
        auto out_name = cloned.Symbols().Register("replacement:" + in->name.Name());
        auto b_name = cloned.Symbols().Register("replacement-child:" + in->name.Name());
        auto* out = alloc.Create<Replacement>(out_name);
        out->b = alloc.Create<Node>(b_name);
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
    // N: Node
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

TEST_F(ProgramCloneContextNodeTest, CloneWithReplaceAll_Symbols) {
    Allocator alloc;

    ProgramBuilder builder;
    Node* original_root;
    {
        auto* a_b = alloc.Create<Node>(builder.Symbols().New("a->b"));
        auto* a = alloc.Create<Node>(builder.Symbols().New("a"), nullptr, a_b);
        auto* b_a = a;  // Aliased
        auto* b_b = alloc.Create<Node>(builder.Symbols().New("b->b"));
        auto* b = alloc.Create<Node>(builder.Symbols().New("b"), b_a, b_b);
        auto* c = b;  // Aliased
        original_root = alloc.Create<Node>(builder.Symbols().New("root"), a, b, c);
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
    // N: Node

    ProgramBuilder cloned;
    auto* cloned_root = CloneContext(&cloned, &original, false)
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

TEST_F(ProgramCloneContextNodeTest, CloneWithoutTransform) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_node = a.Create<Node>(builder.Symbols().New("root"));
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original);
    ctx.ReplaceAll([&](const Node*) {
        return a.Create<Replacement>(builder.Symbols().New("<unexpected-node>"));
    });

    auto* cloned_node = ctx.CloneWithoutTransform(original_node);
    EXPECT_NE(cloned_node, original_node);
    EXPECT_EQ(cloned_node->name, cloned.Symbols().Get("root"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithReplacePointer) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
    original_root->a = a.Create<Node>(builder.Symbols().New("a"));
    original_root->b = a.Create<Node>(builder.Symbols().New("b"));
    original_root->c = a.Create<Node>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //                        Replaced

    ProgramBuilder cloned;
    auto* replacement = a.Create<Node>(cloned.Symbols().New("replacement"));

    auto* cloned_root = CloneContext(&cloned, &original)
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

TEST_F(ProgramCloneContextNodeTest, CloneWithRepeatedImmediateReplacePointer) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
    original_root->a = a.Create<Node>(builder.Symbols().New("a"));
    original_root->b = a.Create<Node>(builder.Symbols().New("b"));
    original_root->c = a.Create<Node>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    CloneContext ctx(&cloned, &original);

    // Demonstrate that ctx.Replace() can be called multiple times to update the replacement of a
    // node.

    auto* replacement_x =
        a.Create<Node>(cloned.Symbols().New("replacement_x"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, replacement_x);

    auto* replacement_y =
        a.Create<Node>(cloned.Symbols().New("replacement_y"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, replacement_y);

    auto* replacement_z =
        a.Create<Node>(cloned.Symbols().New("replacement_z"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, replacement_z);

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_NE(cloned_root->a, replacement_z);
    EXPECT_EQ(cloned_root->b, replacement_z);
    EXPECT_NE(cloned_root->c, replacement_z);

    EXPECT_EQ(replacement_z->a, replacement_y);
    EXPECT_EQ(replacement_y->a, replacement_x);
}

TEST_F(ProgramCloneContextNodeTest, CloneWithReplaceFunction) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
    original_root->a = a.Create<Node>(builder.Symbols().New("a"));
    original_root->b = a.Create<Node>(builder.Symbols().New("b"));
    original_root->c = a.Create<Node>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    //                          root
    //        ╭──────────────────┼──────────────────╮
    //       (a)                (b)                (c)
    //                        Replaced

    ProgramBuilder cloned;
    auto* replacement = a.Create<Node>(cloned.Symbols().New("replacement"));

    auto* cloned_root = CloneContext(&cloned, &original)
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

TEST_F(ProgramCloneContextNodeTest, CloneWithRepeatedImmediateReplaceFunction) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
    original_root->a = a.Create<Node>(builder.Symbols().New("a"));
    original_root->b = a.Create<Node>(builder.Symbols().New("b"));
    original_root->c = a.Create<Node>(builder.Symbols().New("c"));
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    CloneContext ctx(&cloned, &original);

    // Demonstrate that ctx.Replace() can be called multiple times to update the replacement of a
    // node.

    Node* replacement_x =
        a.Create<Node>(cloned.Symbols().New("replacement_x"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, [&] { return replacement_x; });

    Node* replacement_y =
        a.Create<Node>(cloned.Symbols().New("replacement_y"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, [&] { return replacement_y; });

    Node* replacement_z =
        a.Create<Node>(cloned.Symbols().New("replacement_z"), ctx.Clone(original_root->b));
    ctx.Replace(original_root->b, [&] { return replacement_z; });

    auto* cloned_root = ctx.Clone(original_root);

    EXPECT_NE(cloned_root->a, replacement_z);
    EXPECT_EQ(cloned_root->b, replacement_z);
    EXPECT_NE(cloned_root->c, replacement_z);

    EXPECT_EQ(replacement_z->a, replacement_y);
    EXPECT_EQ(replacement_y->a, replacement_x);
}

TEST_F(ProgramCloneContextNodeTest, CloneWithRemove) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* cloned_root = CloneContext(&cloned, &original)
                            .Remove(original_root->vec, original_root->vec[1])
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 2u);

    EXPECT_NE(cloned_root->vec[0], cloned_root->a);
    EXPECT_NE(cloned_root->vec[1], cloned_root->c);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("c"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertFront) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, &original)
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertFrontFunction) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertFront(original_root->vec,
                         [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); })
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertFront_Empty) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, &original)
                            .InsertFront(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertFront_Empty_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertFront(original_root->vec,
                         [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBack) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, &original)
                            .InsertBack(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("c"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBack_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertBack(original_root->vec,
                        [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("c"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBack_Empty) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, &original)
                            .InsertBack(original_root->vec, insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBack_Empty_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertBack(original_root->vec,
                        [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 1u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertFrontAndBack_Empty) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion_back = a.Create<Node>(cloned.Symbols().New("insertion_back"));
    auto* insertion_front = a.Create<Node>(cloned.Symbols().New("insertion_front"));

    auto* cloned_root = CloneContext(&cloned, &original)
                            .InsertBack(original_root->vec, insertion_back)
                            .InsertFront(original_root->vec, insertion_front)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 2u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion_front"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_back"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertFrontAndBack_Empty_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec.Clear();
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertBack(original_root->vec,
                        [&] { return a.Create<Node>(cloned.Symbols().New("insertion_back")); })
            .InsertFront(original_root->vec,
                         [&] { return a.Create<Node>(cloned.Symbols().New("insertion_front")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 2u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("insertion_front"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_back"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBefore) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, &original)
                            .InsertBefore(original_root->vec, original_root->vec[1], insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBefore_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertBefore(original_root->vec, original_root->vec[1],
                          [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertAfter) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

    auto* cloned_root = CloneContext(&cloned, &original)
                            .InsertAfter(original_root->vec, original_root->vec[1], insertion)
                            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertAfter_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertAfter(original_root->vec, original_root->vec[1],
                         [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); })
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertAfterInVectorNodeClone) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original);
    ctx.ReplaceAll([&](const Replaceable* r) {
        auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertAfterInVectorNodeClone_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original);
    ctx.ReplaceAll([&](const Replaceable* r) {
        ctx.InsertAfter(original_root->vec, r,
                        [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); });
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBackInVectorNodeClone) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original);
    ctx.ReplaceAll([&](const Replaceable* /*r*/) {
        auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBackInVectorNodeClone_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Replaceable>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original);
    ctx.ReplaceAll([&](const Replaceable* /*r*/) {
        ctx.InsertBack(original_root->vec,
                       [&] { return a.Create<Node>(cloned.Symbols().New("insertion")); });
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBeforeAndAfterRemoved) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    auto* insertion_before = a.Create<Node>(cloned.Symbols().New("insertion_before"));
    auto* insertion_after = a.Create<Node>(cloned.Symbols().New("insertion_after"));

    auto* cloned_root =
        CloneContext(&cloned, &original)
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

TEST_F(ProgramCloneContextNodeTest, CloneWithInsertBeforeAndAfterRemoved_Function) {
    Allocator a;

    ProgramBuilder builder;
    auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
    original_root->vec = {
        a.Create<Node>(builder.Symbols().Register("a")),
        a.Create<Node>(builder.Symbols().Register("b")),
        a.Create<Node>(builder.Symbols().Register("c")),
    };
    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;

    auto* cloned_root =
        CloneContext(&cloned, &original)
            .InsertBefore(original_root->vec, original_root->vec[1],
                          [&] { return a.Create<Node>(cloned.Symbols().New("insertion_before")); })
            .InsertAfter(original_root->vec, original_root->vec[1],
                         [&] { return a.Create<Node>(cloned.Symbols().New("insertion_after")); })
            .Remove(original_root->vec, original_root->vec[1])
            .Clone(original_root);

    EXPECT_EQ(cloned_root->vec.Length(), 4u);

    EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
    EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
    EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion_before"));
    EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion_after"));
    EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(ProgramCloneContextNodeDeathTest, CloneWithReplaceAll_SameTypeTwice) {
    std::string node_name = TypeInfo::Of<Node>().name;

    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, &original);
            ctx.ReplaceAll([](const Node*) { return nullptr; });
            ctx.ReplaceAll([](const Node*) { return nullptr; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll() called with a handler for type " +
                           node_name + " that is already handled by a handler for type " +
                           node_name));
}

TEST_F(ProgramCloneContextNodeDeathTest, CloneWithReplaceAll_BaseThenDerived) {
    std::string node_name = TypeInfo::Of<Node>().name;
    std::string replaceable_name = TypeInfo::Of<Replaceable>().name;

    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, &original);
            ctx.ReplaceAll([](const Node*) { return nullptr; });
            ctx.ReplaceAll([](const Replaceable*) { return nullptr; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll() called with a handler for type " +
                           replaceable_name + " that is already handled by a handler for type " +
                           node_name));
}

TEST_F(ProgramCloneContextNodeDeathTest, CloneWithReplaceAll_DerivedThenBase) {
    std::string node_name = TypeInfo::Of<Node>().name;
    std::string replaceable_name = TypeInfo::Of<Replaceable>().name;

    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, &original);
            ctx.ReplaceAll([](const Replaceable*) { return nullptr; });
            ctx.ReplaceAll([](const Node*) { return nullptr; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll() called with a handler for type " +
                           node_name + " that is already handled by a handler for type " +
                           replaceable_name));
}

using ProgramCloneContextTest = ::testing::Test;
using ProgramCloneContextDeathTest = ProgramCloneContextTest;

TEST_F(ProgramCloneContextDeathTest, CloneWithReplaceAll_SymbolsTwice) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder cloned;
            Program original;
            CloneContext ctx(&cloned, &original);
            ctx.ReplaceAll([](const Symbol s) { return s; });
            ctx.ReplaceAll([](const Symbol s) { return s; });
        },
        testing::HasSubstr("internal compiler error: ReplaceAll(const SymbolTransform&) called "
                           "multiple times on the same CloneContext"));
}

TEST_F(ProgramCloneContextTest, CloneNewUnnamedSymbols) {
    ProgramBuilder builder;
    Symbol old_a = builder.Symbols().New();
    Symbol old_b = builder.Symbols().New();
    Symbol old_c = builder.Symbols().New();
    EXPECT_EQ(old_a.Name(), "tint_symbol");
    EXPECT_EQ(old_b.Name(), "tint_symbol_1");
    EXPECT_EQ(old_c.Name(), "tint_symbol_2");

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original, false);
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

TEST_F(ProgramCloneContextTest, CloneNewSymbols) {
    ProgramBuilder builder;
    Symbol old_a = builder.Symbols().New("a");
    Symbol old_b = builder.Symbols().New("b");
    Symbol old_c = builder.Symbols().New("c");
    EXPECT_EQ(old_a.Name(), "a");
    EXPECT_EQ(old_b.Name(), "b");
    EXPECT_EQ(old_c.Name(), "c");

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original, false);
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

TEST_F(ProgramCloneContextTest, CloneNewSymbols_AfterCloneSymbols) {
    ProgramBuilder builder;
    Symbol old_a = builder.Symbols().New("a");
    Symbol old_b = builder.Symbols().New("b");
    Symbol old_c = builder.Symbols().New("c");
    EXPECT_EQ(old_a.Name(), "a");
    EXPECT_EQ(old_b.Name(), "b");
    EXPECT_EQ(old_c.Name(), "c");

    Program original(resolver::Resolve(builder));

    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &original);
    Symbol new_x = cloned.Symbols().New("a");
    Symbol new_a = ctx.Clone(old_a);
    Symbol new_y = cloned.Symbols().New("b");
    Symbol new_b = ctx.Clone(old_b);
    Symbol new_z = cloned.Symbols().New("c");
    Symbol new_c = ctx.Clone(old_c);

    EXPECT_EQ(new_x.Name(), "a_1");
    EXPECT_EQ(new_a.Name(), "a");
    EXPECT_EQ(new_y.Name(), "b_1");
    EXPECT_EQ(new_b.Name(), "b");
    EXPECT_EQ(new_z.Name(), "c_1");
    EXPECT_EQ(new_c.Name(), "c");
}

TEST_F(ProgramCloneContextTest, GenerationIDs) {
    ProgramBuilder dst;
    Program src(ProgramBuilder{});
    CloneContext ctx(&dst, &src);
    Allocator allocator;
    auto* cloned = ctx.Clone(allocator.Create<IDNode>(src.ID(), dst.ID()));
    EXPECT_EQ(cloned->generation_id, dst.ID());
}

TEST_F(ProgramCloneContextDeathTest, GenerationIDs_Clone_ObjectNotOwnedBySrc) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder dst;
            Program src(ProgramBuilder{});
            CloneContext ctx(&dst, &src);
            Allocator allocator;
            ctx.Clone(allocator.Create<IDNode>(GenerationID::New(), dst.ID()));
        },
        testing::HasSubstr(
            "internal compiler error: TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, object)"));
}

TEST_F(ProgramCloneContextDeathTest, GenerationIDs_Clone_ObjectNotOwnedByDst) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder dst;
            Program src(ProgramBuilder{});
            CloneContext ctx(&dst, &src);
            Allocator allocator;
            ctx.Clone(allocator.Create<IDNode>(src.ID(), GenerationID::New()));
        },
        testing::HasSubstr(
            "internal compiler error: TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, out)"));
}

}  // namespace
}  // namespace tint::program

TINT_INSTANTIATE_TYPEINFO(tint::program::Node);
TINT_INSTANTIATE_TYPEINFO(tint::program::Replaceable);
TINT_INSTANTIATE_TYPEINFO(tint::program::Replacement);
TINT_INSTANTIATE_TYPEINFO(tint::program::IDNode);
