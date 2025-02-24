//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "compiler/translator/tree_ops/SeparateDeclarations.h"

#include "common/hash_containers.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/util.h"

namespace sh
{
namespace
{

class Separator final : private TIntermRebuild
{
  public:
    Separator(TCompiler &compiler, bool separateCompoundStructDeclarations)
        : TIntermRebuild(compiler, true, true),
          mSeparateCompoundStructDeclarations(separateCompoundStructDeclarations)
    {}
    using TIntermRebuild::rebuildRoot;

  private:
    void recordModifiedStructVariables(TIntermDeclaration &node)
    {
        ASSERT(!mNewStructure);  // No nested struct declarations.
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() <= 1 && !mSeparateCompoundStructDeclarations)
        {
            return;
        }
        TIntermTyped *declarator    = sequence.at(0)->getAsTyped();
        const TType &declaratorType = declarator->getType();
        const TStructure *structure = declaratorType.getStruct();
        // Rewrite variable declarations that specify structs AND variable(s) at the same
        // time.
        if (!structure || !declaratorType.isStructSpecifier())
        {
            return;
        }
        if (mSeparateCompoundStructDeclarations && sequence.size() == 1)
        {
            if (TIntermSymbol *symbol = declarator->getAsSymbolNode(); symbol != nullptr)
            {
                if (symbol->variable().symbolType() == SymbolType::Empty)
                {
                    return;
                }
            }
        }
        // By default, struct specifier changes for all variables except the first one.
        uint32_t index = 1;
        if (structure->symbolType() == SymbolType::Empty)
        {
            TStructure *newStructure =
                new TStructure(&mSymbolTable, kEmptyImmutableString, &structure->fields(),
                               SymbolType::AngleInternal);
            newStructure->setAtGlobalScope(structure->atGlobalScope());
            structure     = newStructure;
            // Adding name causes the struct type to change, so also the first variable variable
            // needs rewriting.
            index = 0;
        }
        if (mSeparateCompoundStructDeclarations)
        {
            mNewStructure = structure;
            // Separating struct and variable declaration causes the variable type to change
            // from specifying to not-specifying, so also the first variable needs rewriting.
            index = 0;
        }

        for (; index < sequence.size(); ++index)
        {
            Declaration decl              = ViewDeclaration(node, index);
            const TVariable &var          = decl.symbol.variable();
            const TType &varType          = var.getType();
            const bool newTypeIsSpecifier = index == 0 && !mSeparateCompoundStructDeclarations;
            TType *newType                = new TType(structure, newTypeIsSpecifier);
            newType->setQualifier(varType.getQualifier());
            newType->makeArrays(varType.getArraySizes());
            TVariable *newVar = new TVariable(&mSymbolTable, var.name(), newType, var.symbolType());
            mStructVariables.insert(std::make_pair(&var, newVar));
        }
    }

    PreResult visitDeclarationPre(TIntermDeclaration &node) override
    {
        recordModifiedStructVariables(node);
        return node;
    }

    PostResult visitDeclarationPost(TIntermDeclaration &node) override
    {
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() <= 1 && !mNewStructure)
        {
            return node;
        }
        std::vector<TIntermNode *> replacements;
        if (mNewStructure)
        {
            TType *newType = new TType(mNewStructure, true);
            if (mNewStructure->atGlobalScope())
            {
                newType->setQualifier(EvqGlobal);
            }
            TVariable *structVar =
                new TVariable(&mSymbolTable, kEmptyImmutableString, newType, SymbolType::Empty);
            TIntermDeclaration *replacement = new TIntermDeclaration({structVar});
            replacement->setLine(node.getLine());
            replacements.push_back(replacement);
            mNewStructure = nullptr;
        }
        for (uint32_t index = 0; index < sequence.size(); ++index)
        {
            TIntermTyped *declarator        = sequence.at(index)->getAsTyped();
            TIntermDeclaration *replacement = new TIntermDeclaration({declarator});
            replacement->setLine(declarator->getLine());
            replacements.push_back(replacement);
        }
        return PostResult::Multi(std::move(replacements));
    }

    PreResult visitSymbolPre(TIntermSymbol &symbolNode) override
    {
        auto it = mStructVariables.find(&symbolNode.variable());
        if (it == mStructVariables.end())
        {
            return symbolNode;
        }
        return *new TIntermSymbol(it->second);
    }

    PreResult visitFunctionPrototypePre(TIntermFunctionPrototype &node) override
    {
        const TFunction *function = node.getFunction();
        auto it                   = mFunctionsToReplace.find(function);
        if (it != mFunctionsToReplace.end())
        {
            TIntermFunctionPrototype *newFuncProto = new TIntermFunctionPrototype(it->second);
            return newFuncProto;
        }
        else if (node.getType().isStructSpecifier())
        {
            const TType &oldType        = node.getType();
            const TStructure *structure = oldType.getStruct();
            // Name unnamed inline structs
            if (structure->symbolType() == SymbolType::Empty)
            {
                TStructure *newStructure =
                    new TStructure(&mSymbolTable, kEmptyImmutableString, &structure->fields(),
                                   SymbolType::AngleInternal);
                newStructure->setAtGlobalScope(structure->atGlobalScope());
                structure = newStructure;
            }
            TType *newType = new TType(structure, true);
            if (structure->atGlobalScope())
            {
                newType->setQualifier(EvqGlobal);
            }
            TVariable *structVar =
                new TVariable(&mSymbolTable, ImmutableString(""), newType, SymbolType::Empty);
            TType *returnType = new TType(structure, false);
            if (oldType.isArray())
            {
                returnType->makeArrays(oldType.getArraySizes());
            }
            returnType->setQualifier(oldType.getQualifier());

            const TFunction *oldFunc = function;
            ASSERT(oldFunc->symbolType() == SymbolType::UserDefined);

            const TFunction *newFunc     = cloneFunctionAndChangeReturnType(oldFunc, returnType);
            mFunctionsToReplace[oldFunc] = newFunc;
            if (getParentNode()->getAsFunctionDefinition() != nullptr)
            {
                mNewFunctionReturnStructDeclaration = new TIntermDeclaration({structVar});
                return new TIntermFunctionPrototype(newFunc);
            }
            return PreResult::Multi(
                {new TIntermDeclaration({structVar}), new TIntermFunctionPrototype(newFunc)});
        }

        return node;
    }

    PostResult visitFunctionDefinitionPost(TIntermFunctionDefinition &node) override
    {
        if (mNewFunctionReturnStructDeclaration)
        {
            return PostResult::Multi(
                {std::exchange(mNewFunctionReturnStructDeclaration, nullptr), &node});
        }
        return node;
    }

    PreResult visitAggregatePre(TIntermAggregate &node) override
    {
        const TFunction *function = node.getFunction();
        auto it                   = mFunctionsToReplace.find(function);
        if (it != mFunctionsToReplace.end())
        {
            TIntermAggregate *replacementNode =
                TIntermAggregate::CreateFunctionCall(*it->second, node.getSequence());

            return PreResult(replacementNode, VisitBits::Children);
        }

        return node;
    }

  private:
    const TFunction *cloneFunctionAndChangeReturnType(const TFunction *oldFunc,
                                                      const TType *newReturnType)

    {
        ASSERT(oldFunc->symbolType() == SymbolType::UserDefined);

        TFunction *newFunc = new TFunction(&mSymbolTable, oldFunc->name(), oldFunc->symbolType(),
                                           newReturnType, oldFunc->isKnownToNotHaveSideEffects());

        if (oldFunc->isDefined())
        {
            newFunc->setDefined();
        }

        if (oldFunc->hasPrototypeDeclaration())
        {
            newFunc->setHasPrototypeDeclaration();
        }

        const size_t paramCount = oldFunc->getParamCount();
        for (size_t i = 0; i < paramCount; ++i)
        {
            const TVariable *var = oldFunc->getParam(i);
            newFunc->addParameter(var);
        }

        return newFunc;
    }

    angle::HashMap<const TFunction *, const TFunction *> mFunctionsToReplace;
    // New structure separated from function declaration.
    TIntermDeclaration *mNewFunctionReturnStructDeclaration = nullptr;

    // New structure from compound declaration.
    const TStructure *mNewStructure = nullptr;
    // Old struct variable to new struct variable mapping.
    angle::HashMap<const TVariable *, TVariable *> mStructVariables;
    const bool mSeparateCompoundStructDeclarations;
};

}  // namespace

bool SeparateDeclarations(TCompiler &compiler,
                          TIntermBlock &root,
                          bool separateCompoundStructDeclarations)
{
    Separator separator(compiler, separateCompoundStructDeclarations);
    return separator.rebuildRoot(root);
}

}  // namespace sh
