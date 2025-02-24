//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INTERMREBUILD_H_
#define COMPILER_TRANSLATOR_INTERMREBUILD_H_

#include "compiler/translator/NodeType.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// Walks the tree to rebuild nodes.
// This class is intended to be derived with overridden visitXXX functions.
//
// Each visitXXX function that does not have a Visit parameter simply has the visitor called
// exactly once, regardless of (preVisit) or (postVisit) values.

// Each visitXXX function that has a Visit parameter behaves as follows:
//    * If (preVisit):
//      - The node is visited before children are traversed.
//      - The returned value is used to replace the visited node. The returned value may be the same
//        as the original node.
//      - If multiple nodes are returned, children and post visits of the returned nodes are not
//        preformed, even if it is a singleton collection.
//    * If (childVisit)
//      - If any new children are returned, the node is automatically rebuilt with the new children
//        before post visit.
//      - Depending on the type of the node, null children may be discarded.
//      - Ill-typed children cause rebuild errors. Ill-typed means the node to automatically rebuild
//        cannot accept a child of a certain type as input to its constructor.
//      - Only instances of TIntermAggregateBase can accept Multi results for any of its children.
//        If supplied, the nodes are spliced children at the spot of the original child.
//    * If (postVisit)
//      - The node is visited after any children are traversed.
//      - Only after such a rebuild (or lack thereof), the post-visit is performed.
//
// Nodes in visit functions are allowed to be modified in place, including TIntermAggregateBase
// child sequences.
//
// The default implementations of all the visitXXX functions support full pre and post traversal
// without modifying the visited nodes.
//
class TIntermRebuild : angle::NonCopyable
{

    enum class Action
    {
        ReplaceSingle,
        ReplaceMulti,
        Drop,
        Fail,
    };

  public:
    struct Fail
    {};

    enum VisitBits : size_t
    {
        // No bits are set.
        Empty = 0u,

        // Allow visit of returned node's children.
        Children = 1u << 0u,

        // Allow post visit of returned node.
        Post = 1u << 1u,

        // If (Children) bit, only visit if the returned node is the same as the original node.
        ChildrenRequiresSame = 1u << 2u,

        // If (Post) bit, only visit if the returned node is the same as the original node.
        PostRequiresSame = 1u << 3u,

        RequireSame  = ChildrenRequiresSame | PostRequiresSame,
        Neither      = Empty,
        Both         = Children | Post,
        BothWhenSame = Both | RequireSame,
    };

  private:
    struct NodeStackGuard;

    template <typename T>
    struct ConsList
    {
        T value;
        ConsList<T> *tail;
    };

    class BaseResult
    {
        BaseResult(const BaseResult &)            = delete;
        BaseResult &operator=(const BaseResult &) = delete;

      public:
        BaseResult(BaseResult &&other) = default;
        BaseResult(BaseResult &other);  // For subclass move constructor impls
        BaseResult(TIntermNode &node, VisitBits visit);
        BaseResult(TIntermNode *node, VisitBits visit);
        BaseResult(std::nullptr_t);
        BaseResult(Fail);
        BaseResult(std::vector<TIntermNode *> &&nodes);

        void moveAssignImpl(BaseResult &other);  // For subclass move assign impls

        static BaseResult Multi(std::vector<TIntermNode *> &&nodes);

        template <typename Iter>
        static BaseResult Multi(Iter nodesBegin, Iter nodesEnd)
        {
            std::vector<TIntermNode *> nodes;
            for (Iter nodesCurr = nodesBegin; nodesCurr != nodesEnd; ++nodesCurr)
            {
                nodes.push_back(*nodesCurr);
            }
            return std::move(nodes);
        }

        bool isFail() const;
        bool isDrop() const;
        TIntermNode *single() const;
        const std::vector<TIntermNode *> *multi() const;

      public:
        Action mAction;
        VisitBits mVisit;
        TIntermNode *mSingle;
        std::vector<TIntermNode *> mMulti;
    };

  public:
    class PreResult : private BaseResult
    {
        friend class TIntermRebuild;

      public:
        PreResult(PreResult &&other);
        PreResult(TIntermNode &node, VisitBits visit = VisitBits::BothWhenSame);
        PreResult(TIntermNode *node, VisitBits visit = VisitBits::BothWhenSame);
        PreResult(std::nullptr_t);  // Used to drop a node.
        PreResult(Fail);            // Used to signal failure.

        void operator=(PreResult &&other);

        static PreResult Multi(std::vector<TIntermNode *> &&nodes)
        {
            return BaseResult::Multi(std::move(nodes));
        }

        template <typename Iter>
        static PreResult Multi(Iter nodesBegin, Iter nodesEnd)
        {
            return BaseResult::Multi(nodesBegin, nodesEnd);
        }

        using BaseResult::isDrop;
        using BaseResult::isFail;
        using BaseResult::multi;
        using BaseResult::single;

      private:
        PreResult(BaseResult &&other);
    };

    class PostResult : private BaseResult
    {
        friend class TIntermRebuild;

      public:
        PostResult(PostResult &&other);
        PostResult(TIntermNode &node);
        PostResult(TIntermNode *node);
        PostResult(std::nullptr_t);  // Used to drop a node
        PostResult(Fail);            // Used to signal failure.

        void operator=(PostResult &&other);

        static PostResult Multi(std::vector<TIntermNode *> &&nodes)
        {
            return BaseResult::Multi(std::move(nodes));
        }

        template <typename Iter>
        static PostResult Multi(Iter nodesBegin, Iter nodesEnd)
        {
            return BaseResult::Multi(nodesBegin, nodesEnd);
        }

        using BaseResult::isDrop;
        using BaseResult::isFail;
        using BaseResult::multi;
        using BaseResult::single;

      private:
        PostResult(BaseResult &&other);
    };

  public:
    TIntermRebuild(TCompiler &compiler, bool preVisit, bool postVisit);

    virtual ~TIntermRebuild();

    // Rebuilds the tree starting at the provided root. If a new node would be returned for the
    // root, the root node's children become that of the new node instead. Returns false if failure
    // occurred.
    [[nodiscard]] bool rebuildRoot(TIntermBlock &root);

  protected:
    virtual PreResult visitSymbolPre(TIntermSymbol &node);
    virtual PreResult visitConstantUnionPre(TIntermConstantUnion &node);
    virtual PreResult visitFunctionPrototypePre(TIntermFunctionPrototype &node);
    virtual PreResult visitPreprocessorDirectivePre(TIntermPreprocessorDirective &node);
    virtual PreResult visitUnaryPre(TIntermUnary &node);
    virtual PreResult visitBinaryPre(TIntermBinary &node);
    virtual PreResult visitTernaryPre(TIntermTernary &node);
    virtual PreResult visitSwizzlePre(TIntermSwizzle &node);
    virtual PreResult visitIfElsePre(TIntermIfElse &node);
    virtual PreResult visitSwitchPre(TIntermSwitch &node);
    virtual PreResult visitCasePre(TIntermCase &node);
    virtual PreResult visitLoopPre(TIntermLoop &node);
    virtual PreResult visitBranchPre(TIntermBranch &node);
    virtual PreResult visitDeclarationPre(TIntermDeclaration &node);
    virtual PreResult visitBlockPre(TIntermBlock &node);
    virtual PreResult visitAggregatePre(TIntermAggregate &node);
    virtual PreResult visitFunctionDefinitionPre(TIntermFunctionDefinition &node);
    virtual PreResult visitGlobalQualifierDeclarationPre(TIntermGlobalQualifierDeclaration &node);

    virtual PostResult visitSymbolPost(TIntermSymbol &node);
    virtual PostResult visitConstantUnionPost(TIntermConstantUnion &node);
    virtual PostResult visitFunctionPrototypePost(TIntermFunctionPrototype &node);
    virtual PostResult visitPreprocessorDirectivePost(TIntermPreprocessorDirective &node);
    virtual PostResult visitUnaryPost(TIntermUnary &node);
    virtual PostResult visitBinaryPost(TIntermBinary &node);
    virtual PostResult visitTernaryPost(TIntermTernary &node);
    virtual PostResult visitSwizzlePost(TIntermSwizzle &node);
    virtual PostResult visitIfElsePost(TIntermIfElse &node);
    virtual PostResult visitSwitchPost(TIntermSwitch &node);
    virtual PostResult visitCasePost(TIntermCase &node);
    virtual PostResult visitLoopPost(TIntermLoop &node);
    virtual PostResult visitBranchPost(TIntermBranch &node);
    virtual PostResult visitDeclarationPost(TIntermDeclaration &node);
    virtual PostResult visitBlockPost(TIntermBlock &node);
    virtual PostResult visitAggregatePost(TIntermAggregate &node);
    virtual PostResult visitFunctionDefinitionPost(TIntermFunctionDefinition &node);
    virtual PostResult visitGlobalQualifierDeclarationPost(TIntermGlobalQualifierDeclaration &node);

    // Can be used to rebuild a specific node during a traversal. Useful for fine control of
    // rebuilding a node's children.
    [[nodiscard]] PostResult rebuild(TIntermNode &node);

    // Rebuilds the provided node in place. If a new node would be returned, the old node's children
    // become that of the new node instead. Returns false if failure occurred.
    [[nodiscard]] bool rebuildInPlace(TIntermAggregate &node);

    // Rebuilds the provided node in place. If a new node would be returned, the old node's children
    // become that of the new node instead. Returns false if failure occurred.
    [[nodiscard]] bool rebuildInPlace(TIntermBlock &node);

    // Rebuilds the provided node in place. If a new node would be returned, the old node's children
    // become that of the new node instead. Returns false if failure occurred.
    [[nodiscard]] bool rebuildInPlace(TIntermDeclaration &node);

    // If currently at or below a function declaration body, this returns the function that encloses
    // the currently visited node. (This returns null if at a function declaration node.)
    const TFunction *getParentFunction() const;

    TIntermNode *getParentNode(size_t offset = 0) const;

  private:
    template <typename Node>
    [[nodiscard]] bool rebuildInPlaceImpl(Node &node);

    PostResult traverseAny(TIntermNode &node);

    template <typename Node>
    Node *traverseAnyAs(TIntermNode &node);

    template <typename Node>
    bool traverseAnyAs(TIntermNode &node, Node *&out);

    PreResult traversePre(TIntermNode &originalNode);
    TIntermNode *traverseChildren(NodeType currNodeType,
                                  const TIntermNode &originalNode,
                                  TIntermNode &currNode,
                                  VisitBits visit);
    PostResult traversePost(NodeType nodeType,
                            const TIntermNode &originalNode,
                            TIntermNode &currNode,
                            VisitBits visit);

    bool traverseAggregateBaseChildren(TIntermAggregateBase &node);

    TIntermNode *traverseUnaryChildren(TIntermUnary &node);
    TIntermNode *traverseBinaryChildren(TIntermBinary &node);
    TIntermNode *traverseTernaryChildren(TIntermTernary &node);
    TIntermNode *traverseSwizzleChildren(TIntermSwizzle &node);
    TIntermNode *traverseIfElseChildren(TIntermIfElse &node);
    TIntermNode *traverseSwitchChildren(TIntermSwitch &node);
    TIntermNode *traverseCaseChildren(TIntermCase &node);
    TIntermNode *traverseLoopChildren(TIntermLoop &node);
    TIntermNode *traverseBranchChildren(TIntermBranch &node);
    TIntermNode *traverseDeclarationChildren(TIntermDeclaration &node);
    TIntermNode *traverseBlockChildren(TIntermBlock &node);
    TIntermNode *traverseAggregateChildren(TIntermAggregate &node);
    TIntermNode *traverseFunctionDefinitionChildren(TIntermFunctionDefinition &node);
    TIntermNode *traverseGlobalQualifierDeclarationChildren(
        TIntermGlobalQualifierDeclaration &node);

  protected:
    TCompiler &mCompiler;
    TSymbolTable &mSymbolTable;
    const TFunction *mParentFunc = nullptr;
    GetNodeType getNodeType;

  private:
    ConsList<TIntermNode *> mNodeStack{nullptr, nullptr};
    bool mPreVisit;
    bool mPostVisit;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_INTERMREBUILD_H_
