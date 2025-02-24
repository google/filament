//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteAtomicCounters: Emulate atomic counter buffers with storage buffers.
//

#include "compiler/translator/tree_ops/RewriteArrayOfArrayOfOpaqueUniforms.h"

#include "common/span.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
struct UniformData
{
    // Corresponding to an array of array of opaque uniform variable, this is the flattened variable
    // that is replacing it.
    const TVariable *flattened;
    // Assume a general case of array declaration with N dimensions:
    //
    //     uniform type u[Dn]..[D2][D1];
    //
    // Let's define
    //
    //     Pn = D(n-1)*...*D2*D1
    //
    // In that case, we have:
    //
    //     u[In]         = ac + In*Pn
    //     u[In][I(n-1)] = ac + In*Pn + I(n-1)*P(n-1)
    //     u[In]...[Ii]  = ac + In*Pn + ... + Ii*Pi
    //
    // This array contains Pi.  Note that the like TType::mArraySizes, the last element is the
    // outermost dimension.  Element 0 is necessarily 1.
    TVector<unsigned int> mSubArraySizes;
};

using UniformMap = angle::HashMap<const TVariable *, UniformData>;

TIntermTyped *RewriteArrayOfArraySubscriptExpression(TCompiler *compiler,
                                                     TIntermBinary *node,
                                                     const UniformMap &uniformMap);

// Given an expression, this traverser calculates a new expression where array of array of opaque
// uniforms are replaced with their flattened ones.  In particular, this is run on the right node of
// EOpIndexIndirect binary nodes, so that the expression in the index gets a chance to go through
// this transformation.
class RewriteExpressionTraverser final : public TIntermTraverser
{
  public:
    explicit RewriteExpressionTraverser(TCompiler *compiler, const UniformMap &uniformMap)
        : TIntermTraverser(true, false, false), mCompiler(compiler), mUniformMap(uniformMap)
    {}

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TIntermTyped *rewritten =
            RewriteArrayOfArraySubscriptExpression(mCompiler, node, mUniformMap);
        if (rewritten == nullptr)
        {
            return true;
        }

        queueReplacement(rewritten, OriginalNode::IS_DROPPED);

        // Don't iterate as the expression is rewritten.
        return false;
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        // We cannot reach here for an opaque uniform that is being replaced.  visitBinary should
        // have taken care of it.
        ASSERT(!IsOpaqueType(node->getType().getBasicType()) ||
               mUniformMap.find(&node->variable()) == mUniformMap.end());
    }

  private:
    TCompiler *mCompiler;

    const UniformMap &mUniformMap;
};

// Rewrite the index of an EOpIndexIndirect expression.  The root can never need replacing, because
// it cannot be an opaque uniform itself.
void RewriteIndexExpression(TCompiler *compiler,
                            TIntermTyped *expression,
                            const UniformMap &uniformMap)
{
    RewriteExpressionTraverser traverser(compiler, uniformMap);
    expression->traverse(&traverser);
    bool valid = traverser.updateTree(compiler, expression);
    ASSERT(valid);
}

// Given an expression such as the following:
//
//                                              EOpIndex(In)Direct (opaque uniform)
//                                                    /           \
//                                            EOpIndex(In)Direct   I1
//                                                  /           \
//                                                ...            I2
//                                            /
//                                    EOpIndex(In)Direct
//                                          /           \
//                                      uniform          In
//
// produces:
//
//          EOpIndex(In)Direct
//            /        \
//        uniform    In*Pn + ... + I2*P2 + I1*P1
//
TIntermTyped *RewriteArrayOfArraySubscriptExpression(TCompiler *compiler,
                                                     TIntermBinary *node,
                                                     const UniformMap &uniformMap)
{
    // Only interested in opaque uniforms.
    if (!IsOpaqueType(node->getType().getBasicType()) || node->getOp() == EOpComma)
    {
        return nullptr;
    }

    TIntermSymbol *opaqueUniform = nullptr;

    // Iterate once and find the opaque uniform that's being indexed.
    TIntermBinary *iter = node;
    while (opaqueUniform == nullptr)
    {
        ASSERT(iter->getOp() == EOpIndexDirect || iter->getOp() == EOpIndexIndirect);

        opaqueUniform = iter->getLeft()->getAsSymbolNode();
        iter          = iter->getLeft()->getAsBinaryNode();
    }

    // If not being replaced, there's nothing to do.
    auto flattenedIter = uniformMap.find(&opaqueUniform->variable());
    if (flattenedIter == uniformMap.end())
    {
        return nullptr;
    }

    const UniformData &data = flattenedIter->second;

    // Iterate again and build the index expression.  The index expression constitutes the sum of
    // the variable indices plus a constant offset calculated from the constant indices.  For
    // example, smplr[1][x][2][y] will have an index of x*P3 + y*P1 + c, where c = (1*P4 + 2*P2).
    unsigned int constantOffset = 0;
    TIntermTyped *variableIndex = nullptr;

    // Since the opaque uniforms are fully subscripted, we know exactly how many EOpIndex* nodes
    // there should be.
    for (size_t dimIndex = 0; dimIndex < data.mSubArraySizes.size(); ++dimIndex)
    {
        ASSERT(node);

        unsigned int subArraySize = data.mSubArraySizes[dimIndex];

        switch (node->getOp())
        {
            case EOpIndexDirect:
                // Accumulate the constant index.
                constantOffset +=
                    node->getRight()->getAsConstantUnion()->getIConst(0) * subArraySize;
                break;
            case EOpIndexIndirect:
            {
                // Run RewriteExpressionTraverser on the right node.  It may itself be an expression
                // with an array of array of opaque uniform inside that needs to be rewritten.
                TIntermTyped *indexExpression = node->getRight();
                RewriteIndexExpression(compiler, indexExpression, uniformMap);

                // Scale and accumulate.
                if (subArraySize != 1)
                {
                    indexExpression =
                        new TIntermBinary(EOpMul, indexExpression, CreateIndexNode(subArraySize));
                }

                if (variableIndex == nullptr)
                {
                    variableIndex = indexExpression;
                }
                else
                {
                    variableIndex = new TIntermBinary(EOpAdd, variableIndex, indexExpression);
                }
                break;
            }
            default:
                UNREACHABLE();
                break;
        }

        node = node->getLeft()->getAsBinaryNode();
    }

    // Add the two accumulated indices together.
    TIntermTyped *index = nullptr;
    if (constantOffset == 0 && variableIndex != nullptr)
    {
        // No constant offset, but there's variable offset.  Take that as offset.
        index = variableIndex;
    }
    else
    {
        // Either the constant offset is non zero, or there's no variable offset (so constant 0
        // should be used).
        index = CreateIndexNode(constantOffset);

        if (variableIndex)
        {
            index = new TIntermBinary(EOpAdd, index, variableIndex);
        }
    }

    // Create an index into the flattened uniform.
    TOperator op = variableIndex ? EOpIndexIndirect : EOpIndexDirect;
    return new TIntermBinary(op, new TIntermSymbol(data.flattened), index);
}

// Traverser that takes:
//
//     uniform sampler/image/atomic_uint u[N][M]..
//
// and transforms it to:
//
//     uniform sampler/image/atomic_uint u[N * M * ..]
//
// MonomorphizeUnsupportedFunctions makes it impossible for this array to be partially
// subscripted, or passed as argument to a function unsubscripted.  This means that every encounter
// of this uniform can be expected to be fully subscripted.
//
class RewriteArrayOfArrayOfOpaqueUniformsTraverser : public TIntermTraverser
{
  public:
    RewriteArrayOfArrayOfOpaqueUniformsTraverser(TCompiler *compiler, TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable), mCompiler(compiler)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (!mInGlobalScope)
        {
            return true;
        }

        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        bool isOpaqueUniform =
            type.getQualifier() == EvqUniform && IsOpaqueType(type.getBasicType());

        // Only interested in array of array of opaque uniforms.
        if (!isOpaqueUniform || !type.isArrayOfArrays())
        {
            return false;
        }

        // Opaque uniforms cannot have initializers, so the declaration must necessarily be a
        // symbol.
        TIntermSymbol *symbol = variable->getAsSymbolNode();
        ASSERT(symbol != nullptr);

        const TVariable *uniformVariable = &symbol->variable();

        // Create an entry in the map.
        ASSERT(mUniformMap.find(uniformVariable) == mUniformMap.end());
        UniformData &data = mUniformMap[uniformVariable];

        // Calculate the accumulated dimension products.  See UniformData::mSubArraySizes.
        const angle::Span<const unsigned int> &arraySizes = type.getArraySizes();
        mUniformMap[uniformVariable].mSubArraySizes.resize(arraySizes.size());
        unsigned int runningProduct = 1;
        for (size_t dimension = 0; dimension < arraySizes.size(); ++dimension)
        {
            data.mSubArraySizes[dimension] = runningProduct;
            runningProduct *= arraySizes[dimension];
        }

        // Create a replacement variable with the array flattened.
        TType *newType = new TType(type);
        newType->toArrayBaseType();
        newType->makeArray(runningProduct);

        data.flattened = new TVariable(mSymbolTable, uniformVariable->name(), newType,
                                       uniformVariable->symbolType());

        TIntermDeclaration *decl = new TIntermDeclaration;
        decl->appendDeclarator(new TIntermSymbol(data.flattened));

        queueReplacement(decl, OriginalNode::IS_DROPPED);
        return false;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        // As an optimization, don't bother inspecting functions if there aren't any opaque uniforms
        // to replace.
        return !mUniformMap.empty();
    }

    // Same implementation as in RewriteExpressionTraverser.  That traverser cannot replace root.
    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TIntermTyped *rewritten =
            RewriteArrayOfArraySubscriptExpression(mCompiler, node, mUniformMap);
        if (rewritten == nullptr)
        {
            return true;
        }

        queueReplacement(rewritten, OriginalNode::IS_DROPPED);

        // Don't iterate as the expression is rewritten.
        return false;
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        ASSERT(!IsOpaqueType(node->getType().getBasicType()) ||
               mUniformMap.find(&node->variable()) == mUniformMap.end());
    }

  private:
    TCompiler *mCompiler;
    UniformMap mUniformMap;
};
}  // anonymous namespace

bool RewriteArrayOfArrayOfOpaqueUniforms(TCompiler *compiler,
                                         TIntermBlock *root,
                                         TSymbolTable *symbolTable)
{
    RewriteArrayOfArrayOfOpaqueUniformsTraverser traverser(compiler, symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}
}  // namespace sh
