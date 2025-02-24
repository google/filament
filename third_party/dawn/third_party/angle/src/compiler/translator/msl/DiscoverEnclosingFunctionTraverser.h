//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_DISCOVERENCLOSINGFUNCTIONTRAVERSER_H_
#define COMPILER_TRANSLATOR_MSL_DISCOVERENCLOSINGFUNCTIONTRAVERSER_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// A TIntermTraverser that supports discovery of the function a node belongs to.
class DiscoverEnclosingFunctionTraverser : public TIntermTraverser
{
  public:
    DiscoverEnclosingFunctionTraverser(bool preVisit,
                                       bool inVisit,
                                       bool postVisit,
                                       TSymbolTable *symbolTable = nullptr);

    // Returns the function a node belongs inside.
    // Returns null if the node does not belong inside a function.
    const TFunction *discoverEnclosingFunction(TIntermNode *node);
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_DISCOVERENCLOSINGFUNCTIONTRAVERSER_H_
