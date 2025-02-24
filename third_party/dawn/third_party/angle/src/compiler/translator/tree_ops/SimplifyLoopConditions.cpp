//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimplifyLoopConditions is an AST traverser that converts loop conditions and loop expressions
// to regular statements inside the loop. This way further transformations that generate statements
// from loop conditions and loop expressions work correctly.
//

#include "compiler/translator/tree_ops/SimplifyLoopConditions.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/tree_util/IntermNodePatternMatcher.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

struct LoopInfo
{
    const TVariable *conditionVariable = nullptr;
    TIntermTyped *condition            = nullptr;
    TIntermTyped *expression           = nullptr;
};

class SimplifyLoopConditionsTraverser : public TLValueTrackingTraverser
{
  public:
    SimplifyLoopConditionsTraverser(const IntermNodePatternMatcher *conditionsToSimplify,
                                    TSymbolTable *symbolTable);

    void traverseLoop(TIntermLoop *node) override;

    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;

    bool foundLoopToChange() const { return mFoundLoopToChange; }

  protected:
    // Marked to true once an operation that needs to be hoisted out of a loop expression has been
    // found.
    bool mFoundLoopToChange;
    bool mInsideLoopInitConditionOrExpression;
    const IntermNodePatternMatcher *mConditionsToSimplify;

  private:
    LoopInfo mLoop;
};

SimplifyLoopConditionsTraverser::SimplifyLoopConditionsTraverser(
    const IntermNodePatternMatcher *conditionsToSimplify,
    TSymbolTable *symbolTable)
    : TLValueTrackingTraverser(true, false, false, symbolTable),
      mFoundLoopToChange(false),
      mInsideLoopInitConditionOrExpression(false),
      mConditionsToSimplify(conditionsToSimplify)
{}

// If we're inside a loop initialization, condition, or expression, we check for expressions that
// should be moved out of the loop condition or expression. If one is found, the loop is
// transformed.
// If we're not inside loop initialization, condition, or expression, we only need to traverse nodes
// that may contain loops.

bool SimplifyLoopConditionsTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    ASSERT(mConditionsToSimplify);
    mFoundLoopToChange = mConditionsToSimplify->match(node);
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    ASSERT(mConditionsToSimplify);
    mFoundLoopToChange =
        mConditionsToSimplify->match(node, getParentNode(), isLValueRequiredHere());
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    ASSERT(mConditionsToSimplify);
    mFoundLoopToChange = mConditionsToSimplify->match(node, getParentNode());
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitTernary(Visit visit, TIntermTernary *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    ASSERT(mConditionsToSimplify);
    mFoundLoopToChange = mConditionsToSimplify->match(node);
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    ASSERT(mConditionsToSimplify);
    mFoundLoopToChange = mConditionsToSimplify->match(node);
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    if (node->getFlowOp() == EOpContinue && (mLoop.condition || mLoop.expression))
    {
        TIntermBlock *parent = getParentNode()->getAsBlock();
        ASSERT(parent);
        TIntermSequence seq;
        if (mLoop.expression)
        {
            seq.push_back(mLoop.expression->deepCopy());
        }
        if (mLoop.condition)
        {
            ASSERT(mLoop.conditionVariable);
            seq.push_back(
                CreateTempAssignmentNode(mLoop.conditionVariable, mLoop.condition->deepCopy()));
        }
        seq.push_back(node);
        mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parent, node, std::move(seq)));
    }

    return true;
}

static TIntermBlock *CreateFromBody(TIntermLoop *node, bool *bodyEndsInBranchOut)
{
    TIntermBlock *newBody  = new TIntermBlock();
    TIntermBlock *nodeBody = node->getBody();
    newBody->getSequence()->push_back(nodeBody);
    *bodyEndsInBranchOut = EndsInBranch(nodeBody);
    return newBody;
}

void SimplifyLoopConditionsTraverser::traverseLoop(TIntermLoop *node)
{
    // Mark that we're inside a loop condition or expression, and determine if the loop needs to be
    // transformed.

    ScopedNodeInTraversalPath addToPath(this, node);

    mInsideLoopInitConditionOrExpression = true;
    mFoundLoopToChange                   = !mConditionsToSimplify;

    if (!mFoundLoopToChange && node->getInit())
    {
        node->getInit()->traverse(this);
    }

    if (!mFoundLoopToChange && node->getCondition())
    {
        node->getCondition()->traverse(this);
    }

    if (!mFoundLoopToChange && node->getExpression())
    {
        node->getExpression()->traverse(this);
    }

    mInsideLoopInitConditionOrExpression = false;

    const LoopInfo prevLoop = mLoop;

    if (mFoundLoopToChange)
    {
        const TType *boolType   = StaticType::Get<EbtBool, EbpUndefined, EvqTemporary, 1, 1>();
        mLoop.conditionVariable = CreateTempVariable(mSymbolTable, boolType);
        mLoop.condition         = node->getCondition();
        mLoop.expression        = node->getExpression();

        // Replace the loop condition with a boolean variable that's updated on each iteration.
        TLoopType loopType = node->getType();
        if (loopType == ELoopWhile)
        {
            ASSERT(!mLoop.expression);

            if (mLoop.condition->getAsSymbolNode())
            {
                // Mask continue statement condition variable update.
                mLoop.condition = nullptr;
            }
            else if (mLoop.condition->getAsConstantUnion())
            {
                // Transform:
                //   while (expr) { body; }
                // into
                //   bool s0 = expr;
                //   while (s0) { body; }
                TIntermDeclaration *tempInitDeclaration =
                    CreateTempInitDeclarationNode(mLoop.conditionVariable, mLoop.condition);
                insertStatementInParentBlock(tempInitDeclaration);

                node->setCondition(CreateTempSymbolNode(mLoop.conditionVariable));

                // Mask continue statement condition variable update.
                mLoop.condition = nullptr;
            }
            else
            {
                // Transform:
                //   while (expr) { body; }
                // into
                //   bool s0 = expr;
                //   while (s0) { { body; } s0 = expr; }
                //
                // Local case statements are transformed into:
                //   s0 = expr; continue;
                TIntermDeclaration *tempInitDeclaration =
                    CreateTempInitDeclarationNode(mLoop.conditionVariable, mLoop.condition);
                insertStatementInParentBlock(tempInitDeclaration);

                bool bodyEndsInBranch;
                TIntermBlock *newBody = CreateFromBody(node, &bodyEndsInBranch);
                if (!bodyEndsInBranch)
                {
                    newBody->getSequence()->push_back(CreateTempAssignmentNode(
                        mLoop.conditionVariable, mLoop.condition->deepCopy()));
                }

                // Can't use queueReplacement to replace old body, since it may have been nullptr.
                // It's safe to do the replacements in place here - the new body will still be
                // traversed, but that won't create any problems.
                node->setBody(newBody);
                node->setCondition(CreateTempSymbolNode(mLoop.conditionVariable));
            }
        }
        else if (loopType == ELoopDoWhile)
        {
            ASSERT(!mLoop.expression);

            if (mLoop.condition->getAsSymbolNode())
            {
                // Mask continue statement condition variable update.
                mLoop.condition = nullptr;
            }
            else if (mLoop.condition->getAsConstantUnion())
            {
                // Transform:
                //   do {
                //     body;
                //   } while (expr);
                // into
                //   bool s0 = expr;
                //   do {
                //     body;
                //   } while (s0);
                TIntermDeclaration *tempInitDeclaration =
                    CreateTempInitDeclarationNode(mLoop.conditionVariable, mLoop.condition);
                insertStatementInParentBlock(tempInitDeclaration);

                node->setCondition(CreateTempSymbolNode(mLoop.conditionVariable));

                // Mask continue statement condition variable update.
                mLoop.condition = nullptr;
            }
            else
            {
                // Transform:
                //   do {
                //     body;
                //   } while (expr);
                // into
                //   bool s0;
                //   do {
                //     { body; }
                //     s0 = expr;
                //   } while (s0);
                // Local case statements are transformed into:
                //   s0 = expr; continue;
                TIntermDeclaration *tempInitDeclaration =
                    CreateTempDeclarationNode(mLoop.conditionVariable);
                insertStatementInParentBlock(tempInitDeclaration);

                bool bodyEndsInBranch;
                TIntermBlock *newBody = CreateFromBody(node, &bodyEndsInBranch);
                if (!bodyEndsInBranch)
                {
                    newBody->getSequence()->push_back(
                        CreateTempAssignmentNode(mLoop.conditionVariable, mLoop.condition));
                }

                // Can't use queueReplacement to replace old body, since it may have been nullptr.
                // It's safe to do the replacements in place here - the new body will still be
                // traversed, but that won't create any problems.
                node->setBody(newBody);
                node->setCondition(CreateTempSymbolNode(mLoop.conditionVariable));
            }
        }
        else if (loopType == ELoopFor)
        {
            if (!mLoop.condition)
            {
                mLoop.condition = CreateBoolNode(true);
            }

            TIntermLoop *whileLoop;
            TIntermBlock *loopScope            = new TIntermBlock();
            TIntermSequence *loopScopeSequence = loopScope->getSequence();

            // Insert "init;"
            if (node->getInit())
            {
                loopScopeSequence->push_back(node->getInit());
            }

            if (mLoop.condition->getAsSymbolNode())
            {
                // Move the loop condition inside the loop.
                // Transform:
                //   for (init; expr; exprB) { body; }
                // into
                //   {
                //     init;
                //     while (expr) {
                //       { body; }
                //       exprB;
                //     }
                //   }
                //
                // Local case statements are transformed into:
                //   exprB; continue;

                // Insert "{ body; }" in the while loop
                bool bodyEndsInBranch;
                TIntermBlock *whileLoopBody = CreateFromBody(node, &bodyEndsInBranch);
                // Insert "exprB;" in the while loop
                if (!bodyEndsInBranch && node->getExpression())
                {
                    whileLoopBody->getSequence()->push_back(node->getExpression());
                }
                // Create "while(expr) { whileLoopBody }"
                whileLoop =
                    new TIntermLoop(ELoopWhile, nullptr, mLoop.condition, nullptr, whileLoopBody);

                // Mask continue statement condition variable update.
                mLoop.condition = nullptr;
            }
            else if (mLoop.condition->getAsConstantUnion())
            {
                // Move the loop condition inside the loop.
                // Transform:
                //   for (init; expr; exprB) { body; }
                // into
                //   {
                //     init;
                //     bool s0 = expr;
                //     while (s0) {
                //       { body; }
                //       exprB;
                //     }
                //   }
                //
                // Local case statements are transformed into:
                //   exprB; continue;

                // Insert "bool s0 = expr;"
                loopScopeSequence->push_back(
                    CreateTempInitDeclarationNode(mLoop.conditionVariable, mLoop.condition));
                // Insert "{ body; }" in the while loop
                bool bodyEndsInBranch;
                TIntermBlock *whileLoopBody = CreateFromBody(node, &bodyEndsInBranch);
                // Insert "exprB;" in the while loop
                if (!bodyEndsInBranch && node->getExpression())
                {
                    whileLoopBody->getSequence()->push_back(node->getExpression());
                }
                // Create "while(s0) { whileLoopBody }"
                whileLoop = new TIntermLoop(ELoopWhile, nullptr,
                                            CreateTempSymbolNode(mLoop.conditionVariable), nullptr,
                                            whileLoopBody);

                // Mask continue statement condition variable update.
                mLoop.condition = nullptr;
            }
            else
            {
                // Move the loop condition inside the loop.
                // Transform:
                //   for (init; expr; exprB) { body; }
                // into
                //   {
                //     init;
                //     bool s0 = expr;
                //     while (s0) {
                //       { body; }
                //       exprB;
                //       s0 = expr;
                //     }
                //   }
                //
                // Local case statements are transformed into:
                //   exprB; s0 = expr; continue;

                // Insert "bool s0 = expr;"
                loopScopeSequence->push_back(
                    CreateTempInitDeclarationNode(mLoop.conditionVariable, mLoop.condition));
                // Insert "{ body; }" in the while loop
                bool bodyEndsInBranch;
                TIntermBlock *whileLoopBody = CreateFromBody(node, &bodyEndsInBranch);
                // Insert "exprB;" in the while loop
                if (!bodyEndsInBranch && node->getExpression())
                {
                    whileLoopBody->getSequence()->push_back(node->getExpression());
                }
                // Insert "s0 = expr;" in the while loop
                if (!bodyEndsInBranch)
                {
                    whileLoopBody->getSequence()->push_back(CreateTempAssignmentNode(
                        mLoop.conditionVariable, mLoop.condition->deepCopy()));
                }
                // Create "while(s0) { whileLoopBody }"
                whileLoop = new TIntermLoop(ELoopWhile, nullptr,
                                            CreateTempSymbolNode(mLoop.conditionVariable), nullptr,
                                            whileLoopBody);
            }

            loopScope->getSequence()->push_back(whileLoop);
            queueReplacement(loopScope, OriginalNode::IS_DROPPED);

            // After this the old body node will be traversed and loops inside it may be
            // transformed. This is fine, since the old body node will still be in the AST after
            // the transformation that's queued here, and transforming loops inside it doesn't
            // need to know the exact post-transform path to it.
        }
    }

    mFoundLoopToChange = false;

    // We traverse the body of the loop even if the loop is transformed.
    node->getBody()->traverse(this);

    mLoop = prevLoop;
}

}  // namespace

bool SimplifyLoopConditions(TCompiler *compiler, TIntermNode *root, TSymbolTable *symbolTable)
{
    SimplifyLoopConditionsTraverser traverser(nullptr, symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

bool SimplifyLoopConditions(TCompiler *compiler,
                            TIntermNode *root,
                            unsigned int conditionsToSimplifyMask,
                            TSymbolTable *symbolTable)
{
    IntermNodePatternMatcher conditionsToSimplify(conditionsToSimplifyMask);
    SimplifyLoopConditionsTraverser traverser(&conditionsToSimplify, symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
