//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/WrapMain.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/AstHelpers.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Wrapper : public TIntermTraverser
{
  private:
    IdGen &mIdGen;

  public:
    Wrapper(TSymbolTable &symbolTable, IdGen &idGen)
        : TIntermTraverser(false, false, true, &symbolTable), mIdGen(idGen)
    {}

    bool visitBlock(Visit, TIntermBlock *blockNode) override
    {
        if (blockNode != getRootNode())
        {
            return true;
        }

        for (TIntermNode *node : *blockNode->getSequence())
        {
            if (TIntermFunctionDefinition *funcDefNode = node->getAsFunctionDefinition())
            {
                const TFunction &func = *funcDefNode->getFunction();
                if (func.isMain())
                {
                    visitMain(*blockNode, funcDefNode);
                    break;
                }
            }
        }

        return true;
    }

  private:
    void visitMain(TIntermBlock &root, TIntermFunctionDefinition *funcDefNode)
    {
        const TFunction &func = *funcDefNode->getFunction();
        ASSERT(func.isMain());
        ASSERT(func.getReturnType().getBasicType() == TBasicType::EbtVoid);
        ASSERT(func.getParamCount() == 0);

        const TFunction &externalMainFunc = *funcDefNode->getFunction();
        const TFunction &internalMainFunc = CloneFunction(*mSymbolTable, mIdGen, externalMainFunc);

        TIntermFunctionPrototype *externalMainProto = funcDefNode->getFunctionPrototype();
        TIntermFunctionPrototype *internalMainProto =
            new TIntermFunctionPrototype(&internalMainFunc);

        TIntermBlock *externalMainBody = new TIntermBlock();
        externalMainBody->appendStatement(
            TIntermAggregate::CreateFunctionCall(internalMainFunc, new TIntermSequence()));

        TIntermBlock *internalMainBody = funcDefNode->getBody();

        TIntermFunctionDefinition *externalMainDef =
            new TIntermFunctionDefinition(externalMainProto, externalMainBody);
        TIntermFunctionDefinition *internalMainDef =
            new TIntermFunctionDefinition(internalMainProto, internalMainBody);

        mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(
            &root, funcDefNode, TIntermSequence{internalMainDef, externalMainDef}));
    }
};

}  // namespace

bool sh::WrapMain(TCompiler &compiler, IdGen &idGen, TIntermBlock &root)
{
    TSymbolTable &symbolTable = compiler.getSymbolTable();
    Wrapper wrapper(symbolTable, idGen);
    root.traverse(&wrapper);
    if (!wrapper.updateTree(&compiler, &root))
    {
        return false;
    }
    return true;
}
