//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/ConvertUnsupportedConstructorsToFunctionCalls.h"

#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/FindFunction.h"
#include "compiler/translator/tree_util/IntermNode_util.h"

using namespace sh;

namespace
{

void AppendMatrixElementArgument(TIntermSymbol *parameter,
                                 int colIndex,
                                 int rowIndex,
                                 TIntermSequence *returnCtorArgs)
{
    TIntermBinary *matColN =
        new TIntermBinary(EOpIndexDirect, parameter->deepCopy(), CreateIndexNode(colIndex));
    TIntermSwizzle *matElem = new TIntermSwizzle(matColN, {rowIndex});
    returnCtorArgs->push_back(matElem);
}

// Adds the argument to sequence for a scalar constructor.
// Given scalar(scalarA) appends scalarA
// Given scalar(vecA) appends vecA.x
// Given scalar(matA) appends matA[0].x
void AppendScalarFromNonScalarArguments(TFunction &function, TIntermSequence *returnCtorArgs)
{
    const TVariable *var = function.getParam(0);
    TIntermSymbol *arg0  = new TIntermSymbol(var);

    const TType &type = arg0->getType();

    if (type.isScalar())
    {
        returnCtorArgs->push_back(arg0);
    }
    else if (type.isVector())
    {
        TIntermSwizzle *vecX = new TIntermSwizzle(arg0, {0});
        returnCtorArgs->push_back(vecX);
    }
    else if (type.isMatrix())
    {
        AppendMatrixElementArgument(arg0, 0, 0, returnCtorArgs);
    }
}

// Adds the arguments to sequence for a vector constructor from a scalar.
// Given vecN(scalarA) appends scalarA, scalarA, ... n times
void AppendVectorFromScalarArgument(const TType &type,
                                    TFunction &function,
                                    TIntermSequence *returnCtorArgs)
{
    const uint8_t vectorSize = type.getNominalSize();
    const TVariable *var     = function.getParam(0);
    TIntermSymbol *v         = new TIntermSymbol(var);
    for (uint8_t i = 0; i < vectorSize; ++i)
    {
        returnCtorArgs->push_back(v->deepCopy());
    }
}

// Adds the arguments to sequence for a vector or matrix constructor from the available arguments
// applying arguments in order until the requested number of values have been extracted from the
// given arguments or until there are no more arguments.
void AppendValuesFromMultipleArguments(int numValuesNeeded,
                                       TFunction &function,
                                       TIntermSequence *returnCtorArgs)
{
    size_t numParameters = function.getParamCount();
    size_t paramIndex    = 0;
    uint8_t colIndex     = 0;
    uint8_t rowIndex     = 0;

    for (int i = 0; i < numValuesNeeded && paramIndex < numParameters; ++i)
    {
        const TVariable *p       = function.getParam(paramIndex);
        TIntermSymbol *parameter = new TIntermSymbol(p);
        if (parameter->isScalar())
        {
            returnCtorArgs->push_back(parameter);
            ++paramIndex;
        }
        else if (parameter->isVector())
        {
            TIntermSwizzle *vecS = new TIntermSwizzle(parameter->deepCopy(), {rowIndex++});
            returnCtorArgs->push_back(vecS);
            if (rowIndex == parameter->getNominalSize())
            {
                ++paramIndex;
                rowIndex = 0;
            }
        }
        else if (parameter->isMatrix())
        {
            AppendMatrixElementArgument(parameter, colIndex, rowIndex++, returnCtorArgs);
            if (rowIndex == parameter->getSecondarySize())
            {
                rowIndex = 0;
                ++colIndex;
                if (colIndex == parameter->getNominalSize())
                {
                    colIndex = 0;
                    ++paramIndex;
                }
            }
        }
    }
}

// Adds the arguments for a matrix constructor from a scalar
// putting the scalar along the diagonal and 0 everywhere else.
void AppendMatrixFromScalarArgument(const TType &type,
                                    TFunction &function,
                                    TIntermSequence *returnCtorArgs)
{
    const TVariable *var  = function.getParam(0);
    TIntermSymbol *v      = new TIntermSymbol(var);
    const uint8_t numCols = type.getNominalSize();
    const uint8_t numRows = type.getSecondarySize();
    for (uint8_t col = 0; col < numCols; ++col)
    {
        for (uint8_t row = 0; row < numRows; ++row)
        {
            if (col == row)
            {
                returnCtorArgs->push_back(v->deepCopy());
            }
            else
            {
                returnCtorArgs->push_back(CreateFloatNode(0.0f, sh::EbpUndefined));
            }
        }
    }
}

// Add the argument for a matrix constructor from a matrix
// copying elements from the same column/row and otherwise
// initialize to the identity matrix.
void AppendMatrixFromMatrixArgument(const TType &type,
                                    TFunction &function,
                                    TIntermSequence *returnCtorArgs)
{
    const TVariable *var  = function.getParam(0);
    TIntermSymbol *v      = new TIntermSymbol(var);
    const uint8_t dstCols = type.getNominalSize();
    const uint8_t dstRows = type.getSecondarySize();
    const uint8_t srcCols = v->getNominalSize();
    const uint8_t srcRows = v->getSecondarySize();
    for (uint8_t dstCol = 0; dstCol < dstCols; ++dstCol)
    {
        for (uint8_t dstRow = 0; dstRow < dstRows; ++dstRow)
        {
            if (dstRow < srcRows && dstCol < srcCols)
            {
                AppendMatrixElementArgument(v, dstCol, dstRow, returnCtorArgs);
            }
            else
            {
                returnCtorArgs->push_back(
                    CreateFloatNode(dstRow == dstCol ? 1.0f : 0.0f, sh::EbpUndefined));
            }
        }
    }
}

class Rebuild : public TIntermRebuild
{
  public:
    explicit Rebuild(TCompiler &compiler) : TIntermRebuild(compiler, false, true) {}
    PostResult visitAggregatePost(TIntermAggregate &node) override
    {
        if (!node.isConstructor())
        {
            return node;
        }

        TIntermSequence &arguments = *node.getSequence();
        if (arguments.empty())
        {
            return node;
        }

        const TType &type     = node.getType();
        const TType &arg0Type = arguments[0]->getAsTyped()->getType();

        if (!type.isScalar() && !type.isVector() && !type.isMatrix())
        {
            return node;
        }

        if (type.isArray())
        {
            return node;
        }

        // check for type_ctor(sameType)
        // scalar(scalar) -> passthrough
        // vecN(vecN) -> passthrough
        // matN(matN) -> passthrough
        if (arguments.size() == 1 && arg0Type == type)
        {
            return node;
        }

        // The following are simple casts:
        //
        // - basic(s) (where basic is int, uint, float or bool, and s is scalar).
        // - gvecN(vN) (where the argument is a single vector with the same number of components).
        // - matNxM(mNxM) (where the argument is a single matrix with the same dimensions).  Note
        // that
        //   matrices are always float, so there's no actual cast and this would be a no-op.
        //
        const bool isSingleScalarCast =
            arguments.size() == 1 && type.isScalar() && arg0Type.isScalar();
        const bool isSingleVectorCast = arguments.size() == 1 && type.isVector() &&
                                        arg0Type.isVector() &&
                                        type.getNominalSize() == arg0Type.getNominalSize();
        const bool isSingleMatrixCast =
            arguments.size() == 1 && type.isMatrix() && arg0Type.isMatrix() &&
            type.getCols() == arg0Type.getCols() && type.getRows() == arg0Type.getRows();
        if (isSingleScalarCast || isSingleVectorCast || isSingleMatrixCast)
        {
            return node;
        }

        // Cases we need to handle:
        // scalar(vec)
        // scalar(mat)
        // vecN(scalar)
        // vecN(vecM)
        // vecN(a,...)
        // matN(scalar) -> diag
        // matN(vec) -> fail!
        // manN(matM) -> corner + ident
        // matN(a, ...)

        // Build a function and pass all the constructor's arguments to it.
        TIntermBlock *body  = new TIntermBlock;
        TFunction *function = new TFunction(&mSymbolTable, ImmutableString(""),
                                            SymbolType::AngleInternal, &type, true);

        for (size_t i = 0; i < arguments.size(); ++i)
        {
            TIntermTyped &arg = *arguments[i]->getAsTyped();
            TType *argType    = new TType(arg.getBasicType(), arg.getPrecision(), EvqParamIn,
                                          arg.getNominalSize(), arg.getSecondarySize());
            TVariable *var    = CreateTempVariable(&mSymbolTable, argType);
            function->addParameter(var);
        }

        // Build a return statement for the function that
        // converts the arguments into the required type.
        TIntermSequence *returnCtorArgs = new TIntermSequence();

        if (type.isScalar())
        {
            AppendScalarFromNonScalarArguments(*function, returnCtorArgs);
        }
        else if (type.isVector())
        {
            if (arguments.size() == 1 && arg0Type.isScalar())
            {
                AppendVectorFromScalarArgument(type, *function, returnCtorArgs);
            }
            else
            {
                AppendValuesFromMultipleArguments(type.getNominalSize(), *function, returnCtorArgs);
            }
        }
        else if (type.isMatrix())
        {
            if (arguments.size() == 1 && arg0Type.isScalar())
            {
                // MSL already handles this case
                AppendMatrixFromScalarArgument(type, *function, returnCtorArgs);
            }
            else if (arg0Type.isMatrix())
            {
                AppendMatrixFromMatrixArgument(type, *function, returnCtorArgs);
            }
            else
            {
                AppendValuesFromMultipleArguments(type.getNominalSize() * type.getSecondarySize(),
                                                  *function, returnCtorArgs);
            }
        }

        TIntermBranch *returnStatement =
            new TIntermBranch(EOpReturn, TIntermAggregate::CreateConstructor(type, returnCtorArgs));
        body->appendStatement(returnStatement);

        TIntermFunctionDefinition *functionDefinition =
            CreateInternalFunctionDefinitionNode(*function, body);
        mFunctionDefs.push_back(functionDefinition);

        TIntermTyped *functionCall = TIntermAggregate::CreateFunctionCall(*function, &arguments);

        return *functionCall;
    }

    bool rewrite(TIntermBlock &root)
    {
        if (!rebuildInPlace(root))
        {
            return true;
        }

        size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(&root);
        for (TIntermFunctionDefinition *functionDefinition : mFunctionDefs)
        {
            root.insertChildNodes(firstFunctionIndex, TIntermSequence({functionDefinition}));
        }

        return mCompiler.validateAST(&root);
    }

  private:
    TVector<TIntermFunctionDefinition *> mFunctionDefs;
};

}  // anonymous namespace

bool sh::ConvertUnsupportedConstructorsToFunctionCalls(TCompiler &compiler, TIntermBlock &root)
{
    return Rebuild(compiler).rewrite(root);
}
