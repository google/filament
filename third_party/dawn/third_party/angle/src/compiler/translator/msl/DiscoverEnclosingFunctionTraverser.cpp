//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/msl/DiscoverEnclosingFunctionTraverser.h"

using namespace sh;

DiscoverEnclosingFunctionTraverser::DiscoverEnclosingFunctionTraverser(bool preVisit_,
                                                                       bool inVisit_,
                                                                       bool postVisit_,
                                                                       TSymbolTable *symbolTable)
    : TIntermTraverser(preVisit_, inVisit_, postVisit_, symbolTable)
{}

const TFunction *DiscoverEnclosingFunctionTraverser::discoverEnclosingFunction(TIntermNode *node)
{
    ASSERT(!node->getAsFunctionDefinition());

    unsigned height = 0;
    while (TIntermNode *ancestor = getAncestorNode(height))
    {
        if (TIntermFunctionDefinition *funcDefNode = ancestor->getAsFunctionDefinition())
        {
            return funcDefNode->getFunction();
        }
        ++height;
    }

    return nullptr;
}
