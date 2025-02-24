//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/msl/MapFunctionsToDefinitions.h"
#include "compiler/translator/Symbol.h"

using namespace sh;

class Mapper : public TIntermTraverser
{
  public:
    FunctionToDefinition mFuncToDef;

  public:
    Mapper() : TIntermTraverser(true, false, false) {}

    bool visitFunctionDefinition(Visit, TIntermFunctionDefinition *funcDefNode) override
    {
        const TFunction *func = funcDefNode->getFunction();
        ASSERT(func->getBuiltInOp() == TOperator::EOpNull);
        mFuncToDef[func] = funcDefNode;
        return false;
    }
};

FunctionToDefinition sh::MapFunctionsToDefinitions(TIntermBlock &root)
{
    Mapper mapper;
    root.traverse(&mapper);
    return std::move(mapper.mFuncToDef);
}
