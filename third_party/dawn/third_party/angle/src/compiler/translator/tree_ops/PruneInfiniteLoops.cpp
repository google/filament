//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneInfiniteLoops.cpp: The PruneInfiniteLoops function prunes:
//
// 1. while (true) { ... }
//
// 2. bool variable = true; /* variable is never accessed */
//    while (variable) { ... }
//
// In all cases, the loop must not have EOpBreak or EOpReturn inside to be allowed to prune.
//
// In all cases, for (...; condition; ...) is treated the same as while (condition).
//
// It quickly gets error-prone when trying to detect more complicated cases.  For example, it's
// temping to reject any |while (expression involving variable with no side effects)| because that's
// either while(true) or while(false), which is prune-able either way.  That detects loops like
// while(variable == false), while(variable + 2 != 4).  But for example
// while(coherent_buffer[variable]) may indeed not result in an infinite loop.  For now, we stick to
// the basic case.

#include "compiler/translator/tree_ops/PruneInfiniteLoops.h"

#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

#include <stack>

namespace sh
{

namespace
{
using VariableSet = TUnorderedSet<const TVariable *>;

class FindConstantVariablesTraverser : public TIntermTraverser
{
  public:
    FindConstantVariablesTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    const VariableSet &getConstVariables() const { return mConstVariables; }

  private:
    bool visitDeclaration(Visit, TIntermDeclaration *decl) override
    {
        // Initially, assume every variable is a constant
        TIntermSequence *sequence = decl->getSequence();
        for (TIntermNode *node : *sequence)
        {
            TIntermSymbol *symbol = node->getAsSymbolNode();
            if (symbol == nullptr)
            {
                TIntermBinary *assign = node->getAsBinaryNode();
                ASSERT(assign != nullptr && assign->getOp() == EOpInitialize);

                symbol = assign->getLeft()->getAsSymbolNode();
                ASSERT(symbol != nullptr);
            }

            ASSERT(mConstVariables.find(&symbol->variable()) == mConstVariables.end());
            mConstVariables.insert(&symbol->variable());
        }

        return false;
    }

    bool visitLoop(Visit visit, TIntermLoop *loop) override
    {
        ASSERT(visit == PreVisit);

        // For simplicity, for now only consider conditions that are just |variable|.  In that case,
        // the condition is not visited, so that `visitSymbol` doesn't consider this a write.
        if (loop->getInit() != nullptr)
        {
            loop->getInit()->traverse(this);
        }
        if (loop->getExpression() != nullptr)
        {
            loop->getExpression()->traverse(this);
        }
        loop->getBody()->traverse(this);

        TIntermTyped *condition = loop->getCondition();
        if (condition != nullptr &&
            (condition->getAsSymbolNode() == nullptr || loop->getType() == ELoopDoWhile))
        {
            condition->traverse(this);
        }

        return false;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        // Assume write for simplicity.  AST makes it difficult to tell if this is read or write.
        mConstVariables.erase(&symbol->variable());
    }

    VariableSet mConstVariables;
};

struct LoopStats
{
    bool hasBreak  = false;
    bool hasReturn = false;
};

class PruneInfiniteLoopsTraverser : public TIntermTraverser
{
  public:
    PruneInfiniteLoopsTraverser(TSymbolTable *symbolTable, const VariableSet &constVariables)
        : TIntermTraverser(true, false, false, symbolTable),
          mConstVariables(constVariables),
          mAnyLoopsPruned(false)
    {}

    bool anyLoopsPruned() const { return mAnyLoopsPruned; }

  private:
    bool visitLoop(Visit visit, TIntermLoop *loop) override;
    bool visitSwitch(Visit visit, TIntermSwitch *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;

    void onScopeBegin() { mLoopStats.push({}); }

    void onScopeEnd()
    {
        // Propagate |hasReturn| up the stack, it escapes every loop.
        ASSERT(!mLoopStats.empty());
        bool hasReturn = mLoopStats.top().hasReturn;
        mLoopStats.pop();

        if (!mLoopStats.empty())
        {
            mLoopStats.top().hasReturn = mLoopStats.top().hasReturn || hasReturn;
        }
    }

    bool hasLoopEscape()
    {
        ASSERT(!mLoopStats.empty());
        return mLoopStats.top().hasBreak || mLoopStats.top().hasReturn;
    }

    const VariableSet &mConstVariables;
    std::stack<LoopStats> mLoopStats;
    bool mAnyLoopsPruned;
};

bool PruneInfiniteLoopsTraverser::visitLoop(Visit visit, TIntermLoop *loop)
{
    onScopeBegin();

    // Nothing in the init, condition or expression of loops can alter the control flow, just visit
    // the body.
    loop->getBody()->traverse(this);

    // Prune the loop if it has no breaks or returns, it's not do-while, and the condition is a
    // constant variable.
    TIntermTyped *condition              = loop->getCondition();
    TIntermConstantUnion *constCondition = condition ? condition->getAsConstantUnion() : nullptr;
    TIntermSymbol *conditionSymbol = condition ? loop->getCondition()->getAsSymbolNode() : nullptr;

    const bool isConditionConstant =
        condition == nullptr || constCondition != nullptr ||
        (conditionSymbol != nullptr &&
         mConstVariables.find(&conditionSymbol->variable()) != mConstVariables.end());

    if (isConditionConstant && loop->getType() != ELoopDoWhile && !hasLoopEscape())
    {
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), loop, TIntermSequence{});
        mAnyLoopsPruned = true;
    }

    onScopeEnd();

    return false;
}

bool PruneInfiniteLoopsTraverser::visitSwitch(Visit visit, TIntermSwitch *node)
{
    // Insert a LoopStats node for switch, just so that breaks inside the switch are not considered
    // loop breaks.
    onScopeBegin();

    // Nothing in the switch expression that can alter the control flow, just visit the body.
    node->getStatementList()->traverse(this);

    onScopeEnd();

    return false;
}

bool PruneInfiniteLoopsTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    if (!mLoopStats.empty())
    {
        switch (node->getFlowOp())
        {
            case EOpReturn:
                mLoopStats.top().hasReturn = true;
                break;
            case EOpBreak:
                mLoopStats.top().hasBreak = true;
                break;
            case EOpContinue:
            case EOpKill:
                // Kill and continue don't let control flow escape from the loop
                break;
            default:
                UNREACHABLE();
        }
    }

    // Only possible child is the value of a return statement, which has no significance.
    return false;
}
}  // namespace

bool PruneInfiniteLoops(TCompiler *compiler,
                        TIntermBlock *root,
                        TSymbolTable *symbolTable,
                        bool *anyLoopsPruned)
{
    *anyLoopsPruned = false;

    FindConstantVariablesTraverser constVarTransverser(symbolTable);
    root->traverse(&constVarTransverser);

    PruneInfiniteLoopsTraverser pruneTraverser(symbolTable,
                                               constVarTransverser.getConstVariables());
    root->traverse(&pruneTraverser);
    *anyLoopsPruned = pruneTraverser.anyLoopsPruned();
    return pruneTraverser.updateTree(compiler, root);
}

}  // namespace sh
