//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteSampleMaskVariable.cpp: Find any references to gl_SampleMask and gl_SampleMaskIn, and
// rewrite it with ANGLESampleMask or ANGLESampleMaskIn.
//

#include "compiler/translator/tree_util/RewriteSampleMaskVariable.h"

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{
namespace
{
constexpr int kMaxIndexForSampleMaskVar = 0;
constexpr int kFullSampleMask           = 0xFFFFFFFF;

// Traverse the tree and collect the redeclaration and replace all non constant index references of
// gl_SampleMask or gl_SampleMaskIn with constant index references
class GLSampleMaskRelatedReferenceTraverser : public TIntermTraverser
{
  public:
    GLSampleMaskRelatedReferenceTraverser(const TIntermSymbol **redeclaredSymOut,
                                          const ImmutableString &targetStr)
        : TIntermTraverser(true, false, false),
          mRedeclaredSym(redeclaredSymOut),
          mTargetStr(targetStr)
    {
        *mRedeclaredSym = nullptr;
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        // If gl_SampleMask is redeclared, we need to collect its information
        const TIntermSequence &sequence = *(node->getSequence());

        if (sequence.size() != 1)
        {
            return true;
        }

        TIntermTyped *variable = sequence.front()->getAsTyped();
        TIntermSymbol *symbol  = variable->getAsSymbolNode();
        if (symbol == nullptr || symbol->getName() != mTargetStr)
        {
            return true;
        }

        *mRedeclaredSym = symbol;

        return true;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TOperator op = node->getOp();
        if (op != EOpIndexDirect && op != EOpIndexIndirect)
        {
            return true;
        }
        TIntermSymbol *left = node->getLeft()->getAsSymbolNode();
        if (!left)
        {
            return true;
        }
        if (left->getName() != mTargetStr)
        {
            return true;
        }
        const TConstantUnion *constIdx = node->getRight()->getConstantValue();
        if (!constIdx)
        {
            if (node->getRight()->hasSideEffects())
            {
                insertStatementInParentBlock(node->getRight());
            }

            queueReplacementWithParent(node, node->getRight(),
                                       CreateIndexNode(kMaxIndexForSampleMaskVar),
                                       OriginalNode::IS_DROPPED);
        }

        return true;
    }

  private:
    const TIntermSymbol **mRedeclaredSym;
    const ImmutableString mTargetStr;
};

}  // anonymous namespace

[[nodiscard]] bool RewriteSampleMask(TCompiler *compiler,
                                     TIntermBlock *root,
                                     TSymbolTable *symbolTable,
                                     const TIntermTyped *numSamplesUniform)
{
    const TIntermSymbol *redeclaredGLSampleMask = nullptr;
    GLSampleMaskRelatedReferenceTraverser indexTraverser(&redeclaredGLSampleMask,
                                                         ImmutableString("gl_SampleMask"));

    root->traverse(&indexTraverser);
    if (!indexTraverser.updateTree(compiler, root))
    {
        return false;
    }

    // Retrieve gl_SampleMask variable reference
    // Search user redeclared it first
    const TVariable *glSampleMaskVar = nullptr;
    if (redeclaredGLSampleMask)
    {
        glSampleMaskVar = &redeclaredGLSampleMask->variable();
    }
    else
    {
        // User defined not found, find in built-in table
        glSampleMaskVar = static_cast<const TVariable *>(symbolTable->findBuiltIn(
            ImmutableString("gl_SampleMask"), compiler->getShaderVersion()));
    }
    if (!glSampleMaskVar)
    {
        return false;
    }

    // Current ANGLE assumes that the maximum number of samples is less than or equal to
    // VK_SAMPLE_COUNT_32_BIT. So, the size of gl_SampleMask array is always one.
    const unsigned int arraySizeOfSampleMask = glSampleMaskVar->getType().getOutermostArraySize();
    ASSERT(arraySizeOfSampleMask == 1);

    TIntermSymbol *glSampleMaskSymbol = new TIntermSymbol(glSampleMaskVar);

    // if (ANGLEUniforms.numSamples == 1)
    // {
    //     gl_SampleMask[0] = int(0xFFFFFFFF);
    // }
    TIntermConstantUnion *singleSampleCount = CreateUIntNode(1);
    TIntermBinary *equalTo =
        new TIntermBinary(EOpEqual, numSamplesUniform->deepCopy(), singleSampleCount);

    TIntermBlock *trueBlock = new TIntermBlock();

    TIntermBinary *sampleMaskVar = new TIntermBinary(EOpIndexDirect, glSampleMaskSymbol->deepCopy(),
                                                     CreateIndexNode(kMaxIndexForSampleMaskVar));
    TIntermConstantUnion *fullSampleMask = CreateIndexNode(kFullSampleMask);
    TIntermBinary *assignment = new TIntermBinary(EOpAssign, sampleMaskVar, fullSampleMask);

    trueBlock->appendStatement(assignment);

    TIntermIfElse *multiSampleOrNot = new TIntermIfElse(equalTo, trueBlock, nullptr);

    return RunAtTheEndOfShader(compiler, root, multiSampleOrNot, symbolTable);
}

[[nodiscard]] bool RewriteSampleMaskIn(TCompiler *compiler,
                                       TIntermBlock *root,
                                       TSymbolTable *symbolTable)
{
    const TIntermSymbol *redeclaredGLSampleMaskIn = nullptr;
    GLSampleMaskRelatedReferenceTraverser indexTraverser(&redeclaredGLSampleMaskIn,
                                                         ImmutableString("gl_SampleMaskIn"));

    root->traverse(&indexTraverser);
    if (!indexTraverser.updateTree(compiler, root))
    {
        return false;
    }

    // Retrieve gl_SampleMaskIn variable reference
    const TVariable *glSampleMaskInVar = nullptr;
    glSampleMaskInVar                  = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_SampleMaskIn"), compiler->getShaderVersion()));
    if (!glSampleMaskInVar)
    {
        return false;
    }

    // Current ANGLE assumes that the maximum number of samples is less than or equal to
    // VK_SAMPLE_COUNT_32_BIT. So, the size of gl_SampleMask array is always one.
    const unsigned int arraySizeOfSampleMaskIn =
        glSampleMaskInVar->getType().getOutermostArraySize();
    ASSERT(arraySizeOfSampleMaskIn == 1);

    return true;
}

}  // namespace sh
