//
// Copyright 2020 The ANGLE Project Authors. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in the
// LICENSE file.
//
// ReplaceArrayOfMatrixVarying: Find any references to array of matrices varying
// and replace it with array of vectors.
//

#include "compiler/translator/tree_util/ReplaceArrayOfMatrixVarying.h"

#include <vector>

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/util.h"

namespace sh
{

// We create two variables to replace the given varying:
// - The new varying which is an array of vectors to be used at input/ouput only.
// - The new global variable which is a same type as given variable, to temporarily be used
// as replacements for assignments, arithmetic ops and so on. During input/ouput phrase, this temp
// variable will be copied from/to the array of vectors variable above.
// NOTE(hqle): Consider eliminating the need for using temp variable.

namespace
{
class CollectVaryingTraverser : public TIntermTraverser
{
  public:
    CollectVaryingTraverser(std::vector<const TVariable *> *varyingsOut)
        : TIntermTraverser(true, false, false), mVaryingsOut(varyingsOut)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        if (sequence.size() != 1)
        {
            return false;
        }

        TIntermTyped *variableType = sequence.front()->getAsTyped();
        if (!variableType || !IsVarying(variableType->getQualifier()) ||
            !variableType->isMatrix() || !variableType->isArray())
        {
            return false;
        }

        TIntermSymbol *variableSymbol = variableType->getAsSymbolNode();
        if (!variableSymbol)
        {
            return false;
        }

        mVaryingsOut->push_back(&variableSymbol->variable());

        return false;
    }

  private:
    std::vector<const TVariable *> *mVaryingsOut;
};
}  // namespace

[[nodiscard]] bool ReplaceArrayOfMatrixVarying(TCompiler *compiler,
                                               TIntermBlock *root,
                                               TSymbolTable *symbolTable,
                                               const TVariable *varying)
{
    const TType &type = varying->getType();

    // Create global variable to temporarily acts as the given variable in places such as
    // arithmetic, assignments an so on.
    TType *tmpReplacementType = new TType(type);
    tmpReplacementType->setQualifier(EvqGlobal);

    TVariable *tempReplaceVar = new TVariable(
        symbolTable, ImmutableString(std::string("ANGLE_AOM_Temp_") + varying->name().data()),
        tmpReplacementType, SymbolType::AngleInternal);

    if (!ReplaceVariable(compiler, root, varying, tempReplaceVar))
    {
        return false;
    }

    // Create array of vectors type
    TType *varyingReplaceType = new TType(type);
    varyingReplaceType->toMatrixColumnType();
    varyingReplaceType->toArrayElementType();
    varyingReplaceType->makeArray(type.getCols() * type.getOutermostArraySize());

    TVariable *varyingReplaceVar =
        new TVariable(symbolTable, varying->name(), varyingReplaceType, SymbolType::UserDefined);

    TIntermSymbol *varyingReplaceDeclarator = new TIntermSymbol(varyingReplaceVar);
    TIntermDeclaration *varyingReplaceDecl  = new TIntermDeclaration;
    varyingReplaceDecl->appendDeclarator(varyingReplaceDeclarator);
    root->insertStatement(0, varyingReplaceDecl);

    // Copy from/to the temp variable
    TIntermBlock *reassignBlock         = new TIntermBlock;
    TIntermSymbol *tempReplaceSymbol    = new TIntermSymbol(tempReplaceVar);
    TIntermSymbol *varyingReplaceSymbol = new TIntermSymbol(varyingReplaceVar);
    bool isInput                        = IsVaryingIn(type.getQualifier());

    for (unsigned int i = 0; i < type.getOutermostArraySize(); ++i)
    {
        TIntermBinary *tempMatrixIndexed =
            new TIntermBinary(EOpIndexDirect, tempReplaceSymbol->deepCopy(), CreateIndexNode(i));
        for (uint8_t col = 0; col < type.getCols(); ++col)
        {

            TIntermBinary *tempMatrixColIndexed = new TIntermBinary(
                EOpIndexDirect, tempMatrixIndexed->deepCopy(), CreateIndexNode(col));
            TIntermBinary *vectorIndexed =
                new TIntermBinary(EOpIndexDirect, varyingReplaceSymbol->deepCopy(),
                                  CreateIndexNode(i * type.getCols() + col));
            TIntermBinary *assignment;
            if (isInput)
            {
                assignment = new TIntermBinary(EOpAssign, tempMatrixColIndexed, vectorIndexed);
            }
            else
            {
                assignment = new TIntermBinary(EOpAssign, vectorIndexed, tempMatrixColIndexed);
            }
            reassignBlock->appendStatement(assignment);
        }
    }

    if (isInput)
    {
        TIntermFunctionDefinition *main = FindMain(root);
        main->getBody()->insertStatement(0, reassignBlock);
        return compiler->validateAST(root);
    }
    else
    {
        return RunAtTheEndOfShader(compiler, root, reassignBlock, symbolTable);
    }
}

[[nodiscard]] bool ReplaceArrayOfMatrixVaryings(TCompiler *compiler,
                                                TIntermBlock *root,
                                                TSymbolTable *symbolTable)
{
    std::vector<const TVariable *> arrayOfMatrixVars;
    CollectVaryingTraverser varCollector(&arrayOfMatrixVars);
    root->traverse(&varCollector);

    for (const TVariable *var : arrayOfMatrixVars)
    {
        if (!ReplaceArrayOfMatrixVarying(compiler, root, symbolTable, var))
        {
            return false;
        }
    }

    return true;
}

}  // namespace sh
