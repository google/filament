//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of dFdy viewport transformation.
// See header for more info.

#include "compiler/translator/tree_ops/RewriteDfdy.h"

#include "common/angleutils.h"
#include "compiler/translator/SymbolTable.h"
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

  private:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    SpecConst *mSpecConst                = nullptr;
    const DriverUniform *mDriverUniforms = nullptr;
};

Traverser::Traverser(TSymbolTable *symbolTable,
                     SpecConst *specConst,
                     const DriverUniform *driverUniforms)
    : TIntermTraverser(true, false, false, symbolTable),
      mSpecConst(specConst),
      mDriverUniforms(driverUniforms)
{}

bool Traverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // Decide if the node represents a call to dFdx() or dFdy()
    if (node->getOp() != EOpDFdx && node->getOp() != EOpDFdy)
    {
        return true;
    }

    const bool isDFdx = node->getOp() == EOpDFdx;

    // Two transformations are done on dFdx and dFdy:
    //
    // - If pre-rotation is applied, dFdx and dFdy may need to swap their axis based on the degree
    //   of rotation.  dFdx becomes dFdy if rotation is 90 or 270 degrees.  Similarly, dFdy becomes
    //   dFdx.
    // - The result is potentially negated.  This could be due to viewport y-flip or pre-rotation.
    //
    // Accordingly, there are two variables controlling the above transformations:
    //
    // - Rotation: A vec2 that is either (0, 1) or (1, 0).  dFdx and dFdy are replaced with:
    //
    //       dFdx * Rotation.x + dFdy * Rotation.y
    //
    // - Scale: A vec2 with -1 or 1 for either x or y components.  The previous result is multiplied
    //   by this.
    //
    // Together, the above operations account for the combinations of 4 possible rotations and
    // y-flip.

    // Get the results of dFdx(operand) and dFdy(operand), and multiply them by the swizzles
    TIntermTyped *operand = node->getChildNode(0)->getAsTyped();

    TIntermTyped *dFdx = CreateBuiltInUnaryFunctionCallNode("dFdx", operand, *mSymbolTable, 300);
    TIntermTyped *dFdy =
        CreateBuiltInUnaryFunctionCallNode("dFdy", operand->deepCopy(), *mSymbolTable, 300);

    // Get rotation multiplier
    TIntermTyped *swapXY = mSpecConst->getSwapXY();
    if (swapXY == nullptr)
    {
        swapXY = mDriverUniforms->getSwapXY();
    }

    TIntermTyped *swapXMultiplier = MakeSwapXMultiplier(swapXY);
    TIntermTyped *swapYMultiplier = MakeSwapYMultiplier(swapXY->deepCopy());

    // Get flip multiplier
    TIntermTyped *flipXY = mDriverUniforms->getFlipXY(mSymbolTable, DriverUniformFlip::Fragment);

    // Multiply the flip and rotation multipliers
    TIntermTyped *xMultiplier =
        new TIntermBinary(EOpMul, isDFdx ? swapXMultiplier : swapYMultiplier,
                          (new TIntermSwizzle(flipXY->deepCopy(), {0}))->fold(nullptr));
    TIntermTyped *yMultiplier =
        new TIntermBinary(EOpMul, isDFdx ? swapYMultiplier : swapXMultiplier,
                          (new TIntermSwizzle(flipXY->deepCopy(), {1}))->fold(nullptr));

    const TOperator mulOp            = dFdx->getType().isVector() ? EOpVectorTimesScalar : EOpMul;
    TIntermTyped *rotatedFlippedDfdx = new TIntermBinary(mulOp, dFdx, xMultiplier);
    TIntermTyped *rotatedFlippedDfdy = new TIntermBinary(mulOp, dFdy, yMultiplier);

    // Sum them together into the result
    TIntermBinary *rotatedFlippedResult =
        new TIntermBinary(EOpAdd, rotatedFlippedDfdx, rotatedFlippedDfdy);

    // Replace the old dFdx() or dFdy() node with the new node that contains the corrected value
    //
    // Note the following bugs (anglebug.com/42265816):
    //
    // - Side effects of operand are duplicated with the above
    // - If the direct child of this node is itself dFdx/y, its queueReplacement will not be
    //   effective as the parent is also replaced.
    queueReplacement(rotatedFlippedResult, OriginalNode::IS_DROPPED);

    return true;
}
}  // anonymous namespace

bool RewriteDfdy(TCompiler *compiler,
                 TIntermBlock *root,
                 TSymbolTable *symbolTable,
                 int shaderVersion,
                 SpecConst *specConst,
                 const DriverUniform *driverUniforms)
{
    // dFdx/dFdy is only valid in GLSL 3.0 and later.
    if (shaderVersion < 300)
    {
        return true;
    }

    Traverser traverser(symbolTable, specConst, driverUniforms);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
