//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <set>

#include "compiler/translator/tree_ops/RescopeGlobalVariables.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/util.h"

namespace sh
{

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rescoper : public TIntermTraverser
{
  public:
    struct VariableInfo
    {
        std::set<TIntermFunctionDefinition *> functions;
        TIntermDeclaration *declaration;
    };

    Rescoper(TSymbolTable *symbolTable) : TIntermTraverser(true, false, true, symbolTable) {}

    bool rescope(TCompiler *compiler, TIntermBlock &root)
    {
        if (mGlobalVarsNeedRescope.empty())
        {
            return true;
        }

        // Insert the declarations into the first block in the function body and keep track of which
        // ones have been moved, as well as the variables to replace.
        VariableReplacementMap replacementMap;
        std::set<TIntermDeclaration *> movedDeclarations;
        for (auto &pair : mGlobalVarsNeedRescope)
        {
            if (pair.second.functions.size() == 1)
            {
                TIntermFunctionDefinition *func = *pair.second.functions.begin();
                // Function* may be a nullptr if the variable was used in a
                // global initializer. Don't rescope, http://anglebug.com/42266827
                if (func != nullptr)
                {
                    TIntermSequence *funcSequence = func->getBody()->getSequence();
                    funcSequence->insert(funcSequence->begin(), pair.second.declaration);

                    TType *newType = new TType(pair.first->getType());
                    newType->setQualifier(TQualifier::EvqTemporary);
                    const TVariable *newVar =
                        new TVariable(&compiler->getSymbolTable(), pair.first->name(), newType,
                                      pair.first->symbolType(), pair.first->extensions());
                    replacementMap[pair.first] = new TIntermSymbol(newVar);

                    movedDeclarations.insert(pair.second.declaration);
                }
            }
        }

        // Remove the global declarations that have been moved from the root block.
        TIntermSequence *rootOriginal = root.getSequence();
        TIntermSequence rootReplacement;
        for (TIntermNode *node : *rootOriginal)
        {
            if (movedDeclarations.find(node->getAsDeclarationNode()) == movedDeclarations.end())
            {
                rootReplacement.push_back(node);
            }
        }
        *rootOriginal = std::move(rootReplacement);
        return ReplaceVariables(compiler, &root, replacementMap);
    }

  protected:
    void visitSymbol(TIntermSymbol *node) override
    {
        const TVariable &var = node->variable();
        // Check that the symbol is in the globals list, but is not LHS of
        // the current global initialiser
        if (&var != mCurrentGlobal &&
            mGlobalVarsNeedRescope.find(&var) != mGlobalVarsNeedRescope.end())
        {
            std::set<TIntermFunctionDefinition *> &set = mGlobalVarsNeedRescope.at(&var).functions;
            if (set.find(mCurrentFunction) == set.end())
            {
                set.emplace(mCurrentFunction);
            }
        }
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit == Visit::PreVisit)
        {
            Declaration decl           = ViewDeclaration(*node);
            const TVariable &var       = decl.symbol.variable();
            const TType &nodeType      = var.getType();
            const TQualifier qualifier = nodeType.getQualifier();
            if (qualifier == TQualifier::EvqGlobal && !nodeType.isStructSpecifier())
            {
                mGlobalVarsNeedRescope.emplace(&var, VariableInfo());
                mGlobalVarsNeedRescope.at(&var).declaration = node;
            }

            // A declaration outside function definition context would be a
            // global variable, set the flag to avoid rescoping any variables
            // used in initializers.
            if (!mCurrentFunction)
            {
                mCurrentGlobal = &var;
            }
        }
        else if (visit == Visit::PostVisit)
        {
            if (!mCurrentFunction)
            {
                mCurrentGlobal = nullptr;
            }
        }
        return true;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        if (visit == Visit::PreVisit)
        {
            TIntermFunctionDefinition *func = node->getAsFunctionDefinition();
            if (func)
            {
                mCurrentFunction = func;
            }
        }
        else if (visit == Visit::PostVisit)
        {
            if (mCurrentFunction && mCurrentFunction == node->getAsFunctionDefinition())
            {
                mCurrentFunction = nullptr;
            }
        }
        return true;
    }

  private:
    TUnorderedMap<const TVariable *, VariableInfo> mGlobalVarsNeedRescope;
    TIntermFunctionDefinition *mCurrentFunction = nullptr;
    const TVariable *mCurrentGlobal             = nullptr;
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool RescopeGlobalVariables(TCompiler &compiler, TIntermBlock &root)
{
    TSymbolTable &symbolTable = compiler.getSymbolTable();
    Rescoper rescoper(&symbolTable);
    root.traverse(&rescoper);
    return rescoper.rescope(&compiler, root);
}

}  // namespace sh
