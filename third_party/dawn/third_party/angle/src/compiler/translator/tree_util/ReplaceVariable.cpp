//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceVariable.cpp: Replace all references to a specific variable in the AST with references to
// another variable.

#include "compiler/translator/tree_util/ReplaceVariable.h"

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class ReplaceVariableTraverser : public TIntermTraverser
{
  public:
    ReplaceVariableTraverser(const TVariable *toBeReplaced, const TIntermTyped *replacement)
        : TIntermTraverser(true, false, false),
          mToBeReplaced(toBeReplaced),
          mReplacement(replacement)
    {}

    void visitSymbol(TIntermSymbol *node) override
    {
        if (&node->variable() == mToBeReplaced)
        {
            queueReplacement(mReplacement->deepCopy(), OriginalNode::IS_DROPPED);
        }
    }

  private:
    const TVariable *const mToBeReplaced;
    const TIntermTyped *const mReplacement;
};

class ReplaceVariablesTraverser : public TIntermTraverser
{
  public:
    ReplaceVariablesTraverser(const VariableReplacementMap &variableMap)
        : TIntermTraverser(true, false, false), mVariableMap(variableMap)
    {}

    void visitSymbol(TIntermSymbol *node) override
    {
        auto iter = mVariableMap.find(&node->variable());
        if (iter != mVariableMap.end())
        {
            queueReplacement(iter->second->deepCopy(), OriginalNode::IS_DROPPED);
        }
    }

  private:
    const VariableReplacementMap &mVariableMap;
};

class GetDeclaratorReplacementsTraverser : public TIntermTraverser
{
  public:
    GetDeclaratorReplacementsTraverser(TSymbolTable *symbolTable,
                                       VariableReplacementMap *variableMap)
        : TIntermTraverser(true, false, false, symbolTable), mVariableMap(variableMap)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        for (TIntermNode *decl : sequence)
        {
            TIntermSymbol *asSymbol = decl->getAsSymbolNode();
            TIntermBinary *asBinary = decl->getAsBinaryNode();

            if (asBinary != nullptr)
            {
                ASSERT(asBinary->getOp() == EOpInitialize);
                asSymbol = asBinary->getLeft()->getAsSymbolNode();
            }

            ASSERT(asSymbol);
            const TVariable &variable = asSymbol->variable();

            ASSERT(mVariableMap->find(&variable) == mVariableMap->end());

            const TVariable *replacementVariable = new TVariable(
                mSymbolTable, variable.name(), &variable.getType(), variable.symbolType());

            (*mVariableMap)[&variable] = new TIntermSymbol(replacementVariable);
        }

        return false;
    }

  private:
    VariableReplacementMap *mVariableMap;
};

}  // anonymous namespace

// Replaces every occurrence of a variable with another variable.
[[nodiscard]] bool ReplaceVariable(TCompiler *compiler,
                                   TIntermBlock *root,
                                   const TVariable *toBeReplaced,
                                   const TVariable *replacement)
{
    ReplaceVariableTraverser traverser(toBeReplaced, new TIntermSymbol(replacement));
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

[[nodiscard]] bool ReplaceVariables(TCompiler *compiler,
                                    TIntermBlock *root,
                                    const VariableReplacementMap &variableMap)
{
    ReplaceVariablesTraverser traverser(variableMap);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

void GetDeclaratorReplacements(TSymbolTable *symbolTable,
                               TIntermBlock *root,
                               VariableReplacementMap *variableMap)
{
    GetDeclaratorReplacementsTraverser traverser(symbolTable, variableMap);
    root->traverse(&traverser);
}

// Replaces every occurrence of a variable with a TIntermNode.
[[nodiscard]] bool ReplaceVariableWithTyped(TCompiler *compiler,
                                            TIntermBlock *root,
                                            const TVariable *toBeReplaced,
                                            const TIntermTyped *replacement)
{
    ReplaceVariableTraverser traverser(toBeReplaced, replacement);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
