//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ValidateBarrierFunctionCalls:
//   Runs compilation checks related to the "barrier built-in function.

#include "compiler/translator/ValidateBarrierFunctionCall.h"

#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
class Traverser : public TIntermTraverser
{
  public:
    Traverser(TDiagnostics *diagnostics)
        : TIntermTraverser(true, false, true), mDiagnostics(diagnostics)
    {}

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        if (!node->getFunction()->isMain())
        {
            return false;
        }

        mInMain = visit == PreVisit;
        return true;
    }

    bool visitBranch(Visit visit, TIntermBranch *branch) override
    {
        if (branch->getFlowOp() == EOpReturn)
        {
            mSeenReturn = true;
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (node->getOp() != EOpBarrierTCS)
        {
            return true;
        }

        if (mSeenReturn)
        {
            mDiagnostics->error(node->getLine(),
                                "barrier() may not be called at any point after a return statement "
                                "in the function main().",
                                "barrier");
            mValid = false;
            return false;
        }

        // TODO(anglebug.com/42264094): Determine if we should check loops as well.
        if (mBranchCount > 0)
        {
            mDiagnostics->error(
                node->getLine(),
                "barrier() may not be called in potentially divergent flow control.", "barrier");
            mValid = false;
            return false;
        }

        return true;
    }

    bool visitIfElse(Visit visit, TIntermIfElse *node) override
    {
        mBranchCount += ((visit == PreVisit) ? 1 : -1);
        return true;
    }

    bool valid() const { return mValid; }

  private:
    TDiagnostics *mDiagnostics = nullptr;
    bool mInMain               = false;
    bool mSeenReturn           = false;
    bool mValid                = true;
    uint32_t mBranchCount      = 0;
};
}  // anonymous namespace

bool ValidateBarrierFunctionCall(TIntermBlock *root, TDiagnostics *diagnostics)
{
    Traverser traverser(diagnostics);
    root->traverse(&traverser);
    return traverser.valid();
}
}  // namespace sh
