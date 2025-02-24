//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IntermTraverse.h : base classes for AST traversers that walk the AST and
//   also have the ability to transform it by replacing nodes.

#ifndef COMPILER_TRANSLATOR_TREEUTIL_INTERMTRAVERSE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_INTERMTRAVERSE_H_

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/tree_util/Visit.h"

namespace sh
{

class TCompiler;
class TSymbolTable;
class TSymbolUniqueId;

// For traversing the tree.  User should derive from this class overriding the visit functions,
// and then pass an object of the subclass to a traverse method of a node.
//
// The traverse*() functions may also be overridden to do other bookkeeping on the tree to provide
// contextual information to the visit functions, such as whether the node is the target of an
// assignment. This is complex to maintain and so should only be done in special cases.
//
// When using this, just fill in the methods for nodes you want visited.
// Return false from a pre-visit to skip visiting that node's subtree.
//
// See also how to write AST transformations documentation:
//   https://github.com/google/angle/blob/master/doc/WritingShaderASTTransformations.md
class TIntermTraverser : angle::NonCopyable
{
  public:
    POOL_ALLOCATOR_NEW_DELETE
    TIntermTraverser(bool preVisitIn,
                     bool inVisitIn,
                     bool postVisitIn,
                     TSymbolTable *symbolTable = nullptr);
    virtual ~TIntermTraverser();

    virtual void visitSymbol(TIntermSymbol *node) {}
    virtual void visitConstantUnion(TIntermConstantUnion *node) {}
    virtual bool visitSwizzle(Visit visit, TIntermSwizzle *node) { return true; }
    virtual bool visitBinary(Visit visit, TIntermBinary *node) { return true; }
    virtual bool visitUnary(Visit visit, TIntermUnary *node) { return true; }
    virtual bool visitTernary(Visit visit, TIntermTernary *node) { return true; }
    virtual bool visitIfElse(Visit visit, TIntermIfElse *node) { return true; }
    virtual bool visitSwitch(Visit visit, TIntermSwitch *node) { return true; }
    virtual bool visitCase(Visit visit, TIntermCase *node) { return true; }
    virtual void visitFunctionPrototype(TIntermFunctionPrototype *node) {}
    virtual bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
    {
        return true;
    }
    virtual bool visitAggregate(Visit visit, TIntermAggregate *node) { return true; }
    virtual bool visitBlock(Visit visit, TIntermBlock *node) { return true; }
    virtual bool visitGlobalQualifierDeclaration(Visit visit,
                                                 TIntermGlobalQualifierDeclaration *node)
    {
        return true;
    }
    virtual bool visitDeclaration(Visit visit, TIntermDeclaration *node) { return true; }
    virtual bool visitLoop(Visit visit, TIntermLoop *node) { return true; }
    virtual bool visitBranch(Visit visit, TIntermBranch *node) { return true; }
    virtual void visitPreprocessorDirective(TIntermPreprocessorDirective *node) {}

    // The traverse functions contain logic for iterating over the children of the node
    // and calling the visit functions in the appropriate places. They also track some
    // context that may be used by the visit functions.

    // The generic traverse() function is used for nodes that don't need special handling.
    // It's templated in order to avoid virtual function calls, this gains around 2% compiler
    // performance.
    template <typename T>
    void traverse(T *node);

    // Specialized traverse functions are implemented for node types where traversal logic may need
    // to be overridden or where some special bookkeeping needs to be done.
    virtual void traverseBinary(TIntermBinary *node);
    virtual void traverseUnary(TIntermUnary *node);
    virtual void traverseFunctionDefinition(TIntermFunctionDefinition *node);
    virtual void traverseAggregate(TIntermAggregate *node);
    virtual void traverseBlock(TIntermBlock *node);
    virtual void traverseLoop(TIntermLoop *node);

    int getMaxDepth() const { return mMaxDepth; }

    // If traversers need to replace nodes, they can add the replacements in
    // mReplacements/mMultiReplacements during traversal and the user of the traverser should call
    // this function after traversal to perform them.
    //
    // Compiler is used to validate the tree.  Node is the same given to traverse().  Returns false
    // if the tree is invalid after update.
    [[nodiscard]] bool updateTree(TCompiler *compiler, TIntermNode *node);

  protected:
    void setMaxAllowedDepth(int depth);

    // Should only be called from traverse*() functions
    bool incrementDepth(TIntermNode *current)
    {
        mMaxDepth = std::max(mMaxDepth, static_cast<int>(mPath.size()));
        mPath.push_back(current);
        return mMaxDepth < mMaxAllowedDepth;
    }

    // Should only be called from traverse*() functions
    void decrementDepth() { mPath.pop_back(); }

    int getCurrentTraversalDepth() const { return static_cast<int>(mPath.size()) - 1; }
    int getCurrentBlockDepth() const { return static_cast<int>(mParentBlockStack.size()) - 1; }

    // RAII helper for incrementDepth/decrementDepth
    class [[nodiscard]] ScopedNodeInTraversalPath
    {
      public:
        ScopedNodeInTraversalPath(TIntermTraverser *traverser, TIntermNode *current)
            : mTraverser(traverser)
        {
            mWithinDepthLimit = mTraverser->incrementDepth(current);
        }
        ~ScopedNodeInTraversalPath() { mTraverser->decrementDepth(); }

        bool isWithinDepthLimit() { return mWithinDepthLimit; }

      private:
        TIntermTraverser *mTraverser;
        bool mWithinDepthLimit;
    };
    // Optimized traversal functions for leaf nodes directly access ScopedNodeInTraversalPath.
    friend void TIntermSymbol::traverse(TIntermTraverser *);
    friend void TIntermConstantUnion::traverse(TIntermTraverser *);
    friend void TIntermFunctionPrototype::traverse(TIntermTraverser *);

    TIntermNode *getParentNode() const
    {
        return mPath.size() <= 1 ? nullptr : mPath[mPath.size() - 2u];
    }

    // Return the nth ancestor of the node being traversed. getAncestorNode(0) == getParentNode()
    TIntermNode *getAncestorNode(unsigned int n) const
    {
        if (mPath.size() > n + 1u)
        {
            return mPath[mPath.size() - n - 2u];
        }
        return nullptr;
    }

    // Returns what child index is currently being visited.  For example when visiting the children
    // of an aggregate, it can be used to find out which argument of the parent (aggregate) node
    // they correspond to.  Only valid in the PreVisit call of the child.
    size_t getParentChildIndex(Visit visit) const
    {
        ASSERT(visit == PreVisit);
        return mCurrentChildIndex;
    }
    // Returns what child index has just been processed.  Only valid in the InVisit and PostVisit
    // calls of the parent node.
    size_t getLastTraversedChildIndex(Visit visit) const
    {
        ASSERT(visit != PreVisit);
        return mCurrentChildIndex;
    }

    const TIntermBlock *getParentBlock() const;

    TIntermNode *getRootNode() const
    {
        ASSERT(!mPath.empty());
        return mPath.front();
    }

    void pushParentBlock(TIntermBlock *node);
    void incrementParentBlockPos();
    void popParentBlock();

    // To replace a single node with multiple nodes in the parent aggregate. May be used with blocks
    // but also with other nodes like declarations.
    struct NodeReplaceWithMultipleEntry
    {
        NodeReplaceWithMultipleEntry(TIntermAggregateBase *parentIn,
                                     TIntermNode *originalIn,
                                     TIntermSequence &&replacementsIn)
            : parent(parentIn), original(originalIn), replacements(std::move(replacementsIn))
        {}

        TIntermAggregateBase *parent;
        TIntermNode *original;
        TIntermSequence replacements;
    };

    // Helper to insert statements in the parent block of the node currently being traversed.
    // The statements will be inserted before the node being traversed once updateTree is called.
    // Should only be called during PreVisit or PostVisit if called from block nodes.
    // Note that two insertions to the same position in the same block are not supported.
    void insertStatementsInParentBlock(const TIntermSequence &insertions);

    // Same as above, but supports simultaneous insertion of statements before and after the node
    // currently being traversed.
    void insertStatementsInParentBlock(const TIntermSequence &insertionsBefore,
                                       const TIntermSequence &insertionsAfter);

    // Helper to insert a single statement.
    void insertStatementInParentBlock(TIntermNode *statement);

    // Explicitly specify where to insert statements. The statements are inserted before and after
    // the specified position. The statements will be inserted once updateTree is called. Note that
    // two insertions to the same position in the same block are not supported.
    void insertStatementsInBlockAtPosition(TIntermBlock *parent,
                                           size_t position,
                                           const TIntermSequence &insertionsBefore,
                                           const TIntermSequence &insertionsAfter);

    enum class OriginalNode
    {
        BECOMES_CHILD,
        IS_DROPPED
    };

    void clearReplacementQueue();

    // Replace the node currently being visited with replacement.
    void queueReplacement(TIntermNode *replacement, OriginalNode originalStatus);
    // Explicitly specify a node to replace with replacement.
    void queueReplacementWithParent(TIntermNode *parent,
                                    TIntermNode *original,
                                    TIntermNode *replacement,
                                    OriginalNode originalStatus);
    // Walk the ancestors and replace the access chain that leads to this symbol.  This fixes up the
    // types of the intermediate nodes, so it should be used when the type of the symbol changes.
    // The AST transformation must still visit the (indirect) index nodes to transform the
    // expression inside those nodes.  Note that due to the way these replacements work, the AST
    // transformation should not attempt to replace the actual index node itself, but only a subnode
    // of that.
    //
    //                    Node 1                                                Node 6
    //                 EOpIndexDirect                                        EOpIndexDirect
    //                /          \                                              /       \
    //           Node 2        Node 3                                   Node 7        Node 3
    //       EOpIndexIndirect     N     --> replaced with -->       EOpIndexIndirect     N
    //         /        \                                            /        \
    //      Node 4    Node 5                                      Node 8      Node 5
    //      symbol   expression                                replacement   expression
    //        ^                                                                 ^
    //        |                                                                 |
    //    This symbol is being replaced,                        This node is directly placed in the
    //    and the replacement is given                          new access chain, and its parent is
    //    to this function.                                     is changed.  This is why a
    //                                                          replacment attempt for this node
    //                                                          itself will not work.
    //
    void queueAccessChainReplacement(TIntermTyped *replacement);

    const bool preVisit;
    const bool inVisit;
    const bool postVisit;

    int mMaxDepth;
    int mMaxAllowedDepth;

    bool mInGlobalScope;

    // During traversing, save all the changes that need to happen into
    // mReplacements/mMultiReplacements, then do them by calling updateTree().
    // Multi replacements are processed after single replacements.
    std::vector<NodeReplaceWithMultipleEntry> mMultiReplacements;

    TSymbolTable *mSymbolTable;

  private:
    // To insert multiple nodes into the parent block.
    struct NodeInsertMultipleEntry
    {
        NodeInsertMultipleEntry(TIntermBlock *_parent,
                                TIntermSequence::size_type _position,
                                TIntermSequence _insertionsBefore,
                                TIntermSequence _insertionsAfter)
            : parent(_parent),
              position(_position),
              insertionsBefore(_insertionsBefore),
              insertionsAfter(_insertionsAfter)
        {}

        TIntermBlock *parent;
        TIntermSequence::size_type position;
        TIntermSequence insertionsBefore;
        TIntermSequence insertionsAfter;
    };

    static bool CompareInsertion(const NodeInsertMultipleEntry &a,
                                 const NodeInsertMultipleEntry &b);

    // To replace a single node with another on the parent node
    struct NodeUpdateEntry
    {
        NodeUpdateEntry(TIntermNode *_parent,
                        TIntermNode *_original,
                        TIntermNode *_replacement,
                        bool _originalBecomesChildOfReplacement)
            : parent(_parent),
              original(_original),
              replacement(_replacement),
              originalBecomesChildOfReplacement(_originalBecomesChildOfReplacement)
        {}

        TIntermNode *parent;
        TIntermNode *original;
        TIntermNode *replacement;
        bool originalBecomesChildOfReplacement;
    };

    struct ParentBlock
    {
        ParentBlock(TIntermBlock *nodeIn, TIntermSequence::size_type posIn)
            : node(nodeIn), pos(posIn)
        {}

        TIntermBlock *node;
        TIntermSequence::size_type pos;
    };

    std::vector<NodeInsertMultipleEntry> mInsertions;
    std::vector<NodeUpdateEntry> mReplacements;

    // All the nodes from root to the current node during traversing.
    TVector<TIntermNode *> mPath;
    // The current child of parent being traversed.
    size_t mCurrentChildIndex;

    // All the code blocks from the root to the current node's parent during traversal.
    std::vector<ParentBlock> mParentBlockStack;
};

// Traverser parent class that tracks where a node is a destination of a write operation and so is
// required to be an l-value.
class TLValueTrackingTraverser : public TIntermTraverser
{
  public:
    TLValueTrackingTraverser(bool preVisit,
                             bool inVisit,
                             bool postVisit,
                             TSymbolTable *symbolTable);
    ~TLValueTrackingTraverser() override {}

    void traverseBinary(TIntermBinary *node) final;
    void traverseUnary(TIntermUnary *node) final;
    void traverseAggregate(TIntermAggregate *node) final;

  protected:
    bool isLValueRequiredHere() const
    {
        return mOperatorRequiresLValue || mInFunctionCallOutParameter;
    }

  private:
    // Track whether an l-value is required in the node that is currently being traversed by the
    // surrounding operator.
    // Use isLValueRequiredHere to check all conditions which require an l-value.
    void setOperatorRequiresLValue(bool lValueRequired)
    {
        mOperatorRequiresLValue = lValueRequired;
    }
    bool operatorRequiresLValue() const { return mOperatorRequiresLValue; }

    // Track whether an l-value is required inside a function call.
    void setInFunctionCallOutParameter(bool inOutParameter);
    bool isInFunctionCallOutParameter() const;

    bool mOperatorRequiresLValue;
    bool mInFunctionCallOutParameter;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_INTERMTRAVERSE_H_
