//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GuardFragDepthWrite: Guards use of frag depth behind the function constant
// ANGLEDepthWriteEnabled to ensure it is only used when a valid depth buffer
// is bound.

#include "compiler/translator/tree_ops/msl/GuardFragDepthWrite.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/tree_util/BuiltIn.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rewriter : public TIntermRebuild
{
  public:
    Rewriter(TCompiler &compiler) : TIntermRebuild(compiler, false, true) {}

    PostResult visitBinaryPost(TIntermBinary &node) override
    {
        if (TIntermSymbol *leftSymbolNode = node.getLeft()->getAsSymbolNode())
        {
            if (leftSymbolNode->getType().getQualifier() == TQualifier::EvqFragDepth)
            {
                // This transformation leaves the tree in an inconsistent state by using a variable
                // that's defined in text, outside of the knowledge of the AST.
                // FIXME(jcunningham): remove once function constants (specconst) are implemented
                // with the metal translator.
                mCompiler.disableValidateVariableReferences();

                TSymbolTable *symbolTable = &mCompiler.getSymbolTable();

                // Create kDepthWriteEnabled variable reference.
                TType *boolType = new TType(EbtBool);
                boolType->setQualifier(EvqConst);
                TVariable *depthWriteEnabledVar = new TVariable(
                    symbolTable, sh::ImmutableString(sh::mtl::kDepthWriteEnabledConstName),
                    boolType, SymbolType::AngleInternal);

                TIntermBlock *innerif = new TIntermBlock;
                innerif->appendStatement(&node);

                TIntermSymbol *depthWriteEnabled = new TIntermSymbol(depthWriteEnabledVar);
                TIntermIfElse *ifCall = new TIntermIfElse(depthWriteEnabled, innerif, nullptr);
                return ifCall;
            }
        }

        return node;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::GuardFragDepthWrite(TCompiler &compiler, TIntermBlock &root)
{
    Rewriter rewriter(compiler);
    if (!rewriter.rebuildRoot(root))
    {
        return false;
    }
    return true;
}
