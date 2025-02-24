//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_SKIPPINGTRAVERSER_H_
#define COMPILER_TRANSLATOR_MSL_SKIPPINGTRAVERSER_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// A TIntermTraverser that skips traversing childen by default.
class SkippingTraverser : public TIntermTraverser
{
  public:
    SkippingTraverser(bool preVisit_,
                      bool inVisit_,
                      bool postVisit_,
                      TSymbolTable *symbolTable = nullptr)
        : TIntermTraverser(preVisit_, inVisit_, postVisit_, symbolTable)
    {}

    bool visitSwizzle(Visit, TIntermSwizzle *) { return false; }
    bool visitUnary(Visit, TIntermUnary *) { return false; }
    bool visitBinary(Visit, TIntermBinary *) { return false; }
    bool visitTernary(Visit, TIntermTernary *) { return false; }
    bool visitIfElse(Visit, TIntermIfElse *) { return false; }
    bool visitSwitch(Visit, TIntermSwitch *) { return false; }
    bool visitCase(Visit, TIntermCase *) { return false; }
    bool visitFunctionDefinition(Visit, TIntermFunctionDefinition *) { return false; }
    bool visitAggregate(Visit, TIntermAggregate *) { return false; }
    bool visitBlock(Visit, TIntermBlock *) { return false; }
    bool visitDeclaration(Visit, TIntermDeclaration *) { return false; }
    bool visitLoop(Visit, TIntermLoop *) { return false; }
    bool visitBranch(Visit, TIntermBranch *) { return false; }
    bool visitGlobalQualifierDeclaration(Visit, TIntermGlobalQualifierDeclaration *)
    {
        return false;
    }
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_SKIPPINGTRAVERSER_H_
