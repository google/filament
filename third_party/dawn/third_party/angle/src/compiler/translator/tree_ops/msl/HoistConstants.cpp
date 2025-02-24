//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/HoistConstants.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/msl/Layout.h"
#include "compiler/translator/tree_util/FindFunction.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/util.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rewriter : private TIntermRebuild
{
  private:
    const size_t mMinRequiredSize;
    TIntermSequence mHoistedDeclNodes;

  public:
    Rewriter(TCompiler &compiler, size_t minRequiredSize)
        : TIntermRebuild(compiler, true, false), mMinRequiredSize(minRequiredSize)
    {}

    PreResult visitDeclarationPre(TIntermDeclaration &declNode) override
    {
        if (getParentFunction())
        {
            Declaration decl  = ViewDeclaration(declNode);
            const TType &type = decl.symbol.getType();
            if (type.getQualifier() == TQualifier::EvqConst)
            {
                if (decl.initExpr && decl.initExpr->hasConstantValue())
                {
                    const size_t size = MetalLayoutOf(type).sizeOf;
                    if (size >= mMinRequiredSize)
                    {
                        mHoistedDeclNodes.push_back(&declNode);
                        return nullptr;
                    }
                }
            }
        }
        return {declNode, VisitBits::Neither};
    }

    bool rewrite(TIntermBlock &root, IdGen &idGen)
    {
        if (!rebuildRoot(root))
        {
            return false;
        }

        if (mHoistedDeclNodes.empty())
        {
            return true;
        }

        root.insertChildNodes(FindFirstFunctionDefinitionIndex(&root), mHoistedDeclNodes);

        for (TIntermNode *opaqueDeclNode : mHoistedDeclNodes)
        {
            TIntermDeclaration *declNode = opaqueDeclNode->getAsDeclarationNode();
            ASSERT(declNode);
            const TVariable &oldVar = ViewDeclaration(*declNode).symbol.variable();
            const Name newName      = idGen.createNewName(oldVar.name());
            auto *newVar = new TVariable(&mSymbolTable, newName.rawName(), &oldVar.getType(),
                                         newName.symbolType());
            if (!ReplaceVariable(&mCompiler, &root, &oldVar, newVar))
            {
                return false;
            }
        }

        return true;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::HoistConstants(TCompiler &compiler,
                        TIntermBlock &root,
                        IdGen &idGen,
                        size_t minRequiredSize)
{
    return Rewriter(compiler, minRequiredSize).rewrite(root, idGen);
}
