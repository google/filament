//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of InterpolateAtOffset viewport transformation.
// See header for more info.

#include "compiler/translator/tree_ops/spirv/RewriteInterpolateAtOffset.h"

#include "common/angleutils.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/spirv/TranslatorSPIRV.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"

namespace sh
{

namespace
{

class Traverser : public TIntermTraverser
{
  public:
    Traverser(TSymbolTable *symbolTable, SpecConst *specConst, const DriverUniform *driverUniforms);

    bool update(TCompiler *compiler, TIntermBlock *root);

  private:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    const TFunction *getRotateFunc();

    SpecConst *mSpecConst                = nullptr;
    const DriverUniform *mDriverUniforms = nullptr;

    TIntermFunctionDefinition *mRotateFunc = nullptr;
};

Traverser::Traverser(TSymbolTable *symbolTable,
                     SpecConst *specConst,
                     const DriverUniform *driverUniforms)
    : TIntermTraverser(true, false, false, symbolTable),
      mSpecConst(specConst),
      mDriverUniforms(driverUniforms)
{}

bool Traverser::update(TCompiler *compiler, TIntermBlock *root)
{
    if (mRotateFunc != nullptr)
    {
        const size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
        root->insertStatement(firstFunctionIndex, mRotateFunc);
    }

    return updateTree(compiler, root);
}

bool Traverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // Decide if the node represents the call of texelFetchOffset.
    if (!BuiltInGroup::IsBuiltIn(node->getOp()))
    {
        return true;
    }

    ASSERT(node->getFunction()->symbolType() == SymbolType::BuiltIn);
    if (node->getFunction()->name() != "interpolateAtOffset")
    {
        return true;
    }

    const TIntermSequence *sequence = node->getSequence();
    ASSERT(sequence->size() == 2u);

    // offset
    TIntermTyped *offsetNode = sequence->at(1)->getAsTyped();
    ASSERT(offsetNode->getType().getBasicType() == EbtFloat &&
           offsetNode->getType().getNominalSize() == 2);

    // Rotate the offset as necessary.
    const TFunction *rotateFunc = getRotateFunc();

    TIntermSequence args = {
        offsetNode,
    };
    TIntermTyped *correctedOffset = TIntermAggregate::CreateFunctionCall(*rotateFunc, &args);
    correctedOffset->setLine(offsetNode->getLine());

    // Replace the offset by the rotated one.
    queueReplacementWithParent(node, offsetNode, correctedOffset, OriginalNode::IS_DROPPED);

    return true;
}

const TFunction *Traverser::getRotateFunc()
{
    if (mRotateFunc != nullptr)
    {
        return mRotateFunc->getFunction();
    }

    // The function prototype is vec2 ANGLERotateInterpolateOffset(vec2 offset)
    const TType *vec2Type = StaticType::GetBasic<EbtFloat, EbpMedium, 2>();

    TType *offsetType = new TType(*vec2Type);
    offsetType->setQualifier(EvqParamIn);

    TVariable *offsetParam = new TVariable(mSymbolTable, ImmutableString("offset"), offsetType,
                                           SymbolType::AngleInternal);

    TFunction *function =
        new TFunction(mSymbolTable, ImmutableString("ANGLERotateInterpolateOffset"),
                      SymbolType::AngleInternal, vec2Type, true);
    function->addParameter(offsetParam);

    // The function body is as such:
    //
    //     return (swap ? offset.yx : offset) * flip;

    TIntermTyped *swapXY = mSpecConst->getSwapXY();
    if (swapXY == nullptr)
    {
        swapXY = mDriverUniforms->getSwapXY();
    }

    TIntermTyped *flipXY = mDriverUniforms->getFlipXY(mSymbolTable, DriverUniformFlip::Fragment);

    TIntermSwizzle *offsetYX = new TIntermSwizzle(new TIntermSymbol(offsetParam), {1, 0});

    TIntermTyped *swapped = new TIntermTernary(swapXY, offsetYX, new TIntermSymbol(offsetParam));
    TIntermTyped *flipped = new TIntermBinary(EOpMul, swapped, flipXY);
    TIntermBranch *returnStatement = new TIntermBranch(EOpReturn, flipped);

    TIntermBlock *body = new TIntermBlock;
    body->appendStatement(returnStatement);

    mRotateFunc = new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);
    return function;
}

}  // anonymous namespace

bool RewriteInterpolateAtOffset(TCompiler *compiler,
                                TIntermBlock *root,
                                TSymbolTable *symbolTable,
                                int shaderVersion,
                                SpecConst *specConst,
                                const DriverUniform *driverUniforms)
{
    // interpolateAtOffset is only valid in GLSL 3.0 and later.
    if (shaderVersion < 300)
    {
        return true;
    }

    Traverser traverser(symbolTable, specConst, driverUniforms);
    root->traverse(&traverser);
    return traverser.update(compiler, root);
}

}  // namespace sh
