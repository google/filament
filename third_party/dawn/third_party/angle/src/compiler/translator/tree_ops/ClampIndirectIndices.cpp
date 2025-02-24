//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClampIndirectIndices.h: Add clamp to the indirect indices used on arrays.
//

#include "compiler/translator/tree_ops/ClampIndirectIndices.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
// Traverser that finds EOpIndexIndirect nodes and applies a clamp to their right-hand side
// expression.
class ClampIndirectIndicesTraverser : public TIntermTraverser
{
  public:
    ClampIndirectIndicesTraverser(TCompiler *compiler, TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable), mCompiler(compiler)
    {}

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        ASSERT(visit == PreVisit);

        // Only interested in EOpIndexIndirect nodes.
        if (node->getOp() != EOpIndexIndirect)
        {
            return true;
        }

        // Apply the transformation to the left and right nodes
        bool valid = ClampIndirectIndices(mCompiler, node->getLeft(), mSymbolTable);
        ASSERT(valid);
        valid = ClampIndirectIndices(mCompiler, node->getRight(), mSymbolTable);
        ASSERT(valid);

        // Generate clamp(right, 0, N), where N is the size of the array being indexed minus 1.  If
        // the array is runtime-sized, the length() method is called on it.
        const TType &leftType  = node->getLeft()->getType();
        const TType &rightType = node->getRight()->getType();

        // Don't clamp indirect indices on unsized arrays in buffer blocks.  They are covered by the
        // relevant robust access behavior of the backend.
        if (leftType.isUnsizedArray())
        {
            return true;
        }

        // On GLSL es 100, clamp is only defined for float, so float arguments are used.
        //
        // However, float clamp is unconditionally emitted to workaround driver bugs with integer
        // clamp on Qualcomm.  http://crbug.com/1217167
        //
        // const bool useFloatClamp = mCompiler->getShaderVersion() == 100;
        const bool useFloatClamp = true;

        TIntermConstantUnion *zero = createClampValue(0, useFloatClamp);
        TIntermTyped *max;

        if (leftType.isArray())
        {
            max = createClampValue(static_cast<int>(leftType.getOutermostArraySize()) - 1,
                                   useFloatClamp);
        }
        else
        {
            ASSERT(leftType.isVector() || leftType.isMatrix());
            max = createClampValue(leftType.getNominalSize() - 1, useFloatClamp);
        }

        TIntermTyped *index = node->getRight();
        // If the index node is not an int (i.e. it's a uint), or a float (if using float clamp),
        // cast it.
        const TBasicType requiredBasicType = useFloatClamp ? EbtFloat : EbtInt;
        if (rightType.getBasicType() != requiredBasicType)
        {
            const TType *clampType = useFloatClamp ? StaticType::GetBasic<EbtFloat, EbpHigh>()
                                                   : StaticType::GetBasic<EbtInt, EbpHigh>();
            TIntermSequence constructorArgs = {index};
            index = TIntermAggregate::CreateConstructor(*clampType, &constructorArgs);
        }

        // min(gl_PointSize, maxPointSize)
        TIntermSequence args;
        args.push_back(index);
        args.push_back(zero);
        args.push_back(max);
        TIntermTyped *clamped =
            CreateBuiltInFunctionCallNode("clamp", &args, *mSymbolTable, useFloatClamp ? 100 : 300);

        // Cast back to int if float clamp was used.
        if (useFloatClamp)
        {
            TIntermSequence constructorArgs = {clamped};
            clamped = TIntermAggregate::CreateConstructor(*StaticType::GetBasic<EbtInt, EbpHigh>(),
                                                          &constructorArgs);
        }

        // Replace the right node (the index) with the clamped result.
        queueReplacementWithParent(node, node->getRight(), clamped, OriginalNode::IS_DROPPED);

        // Don't recurse as left and right nodes are already processed.
        return false;
    }

  private:
    TIntermConstantUnion *createClampValue(int value, bool useFloat)
    {
        if (useFloat)
        {
            return CreateFloatNode(static_cast<float>(value), EbpHigh);
        }
        return CreateIndexNode(value);
    }

    TCompiler *mCompiler;
};
}  // anonymous namespace

bool ClampIndirectIndices(TCompiler *compiler, TIntermNode *root, TSymbolTable *symbolTable)
{
    ClampIndirectIndicesTraverser traverser(compiler, symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
