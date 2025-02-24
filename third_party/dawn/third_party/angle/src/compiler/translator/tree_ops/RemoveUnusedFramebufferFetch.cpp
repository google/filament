//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/RemoveUnusedFramebufferFetch.h"

#include "common/bitset_utils.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
class FindUnusedInoutVariablesTraverser : public TIntermTraverser
{
  public:
    FindUnusedInoutVariablesTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    VariableReplacementMap getReplacementMap() const;

  private:
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;
    void visitSymbol(TIntermSymbol *node) override;

    void markWrite(const TVariable *var, uint8_t channels);
    void markRead(const TVariable *var);
    bool isDirectlyInMain(uint32_t expectedBlockLevel);

    angle::HashMap<const TVariable *, uint8_t> mAssignedChannels;
    angle::HashSet<const TVariable *> mVariablesWithLoadAccess;
    bool mMainHasReturn    = false;
    bool mShaderHasDiscard = false;
    bool mIsInMain         = false;
};

void FindUnusedInoutVariablesTraverser::markWrite(const TVariable *var, uint8_t channels)
{
    auto iter = mAssignedChannels.insert(std::make_pair(var, static_cast<uint8_t>(0)));
    iter.first->second |= channels;
}

void FindUnusedInoutVariablesTraverser::markRead(const TVariable *var)
{
    mVariablesWithLoadAccess.insert(var);
}

bool FindUnusedInoutVariablesTraverser::isDirectlyInMain(uint32_t expectedBlockLevel)
{
    TIntermNode *block        = getAncestorNode(expectedBlockLevel);
    TIntermNode *functionNode = getAncestorNode(expectedBlockLevel + 1);
    TIntermFunctionDefinition *function =
        functionNode ? functionNode->getAsFunctionDefinition() : nullptr;

    return block && block->getAsBlock() && function && function->getFunction()->isMain();
}

bool FindUnusedInoutVariablesTraverser::visitFunctionDefinition(Visit visit,
                                                                TIntermFunctionDefinition *node)
{
    const TFunction *function = node->getFunction();
    mIsInMain                 = function->isMain();
    return true;
}

bool FindUnusedInoutVariablesTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    switch (node->getFlowOp())
    {
        case EOpReturn:
            if (mIsInMain)
            {
                mMainHasReturn = true;
            }
            break;
        case EOpKill:
            mShaderHasDiscard = true;
            break;
        default:
            break;
    }

    return true;
}

void FindUnusedInoutVariablesTraverser::visitSymbol(TIntermSymbol *node)
{
    const TVariable *var = &node->variable();
    const TType &type    = var->getType();
    if (type.getQualifier() != EvqFragmentInOut)
    {
        return;
    }
    if (getParentNode()->getAsDeclarationNode())
    {
        return;
    }

    // Look for the following patterns:
    //
    //   var = ...
    //   var.xy = ...
    //
    // And only directly in main()'s body.  Otherwise it may be an assignment in a conditional that
    // is not executed.  Additionally, main() shouldn't have |return| prior to this access, and the
    // shader shouldn't have any |discard|s anywhere.
    //
    // Every other access is considered a load access for simplicity.
    TIntermNode *parent      = getAncestorNode(0);
    TIntermNode *grandParent = getAncestorNode(1);

    TIntermBinary *parentBinary   = parent ? parent->getAsBinaryNode() : nullptr;
    TIntermSwizzle *parentSwizzle = parent ? parent->getAsSwizzleNode() : nullptr;

    TIntermBinary *grandParentBinary = grandParent ? grandParent->getAsBinaryNode() : nullptr;

    if (parentBinary != nullptr && parentBinary->getOp() == EOpAssign &&
        parentBinary->getLeft() == node && isDirectlyInMain(1) && !mMainHasReturn)
    {
        // All channels are written to
        ASSERT(mIsInMain);
        markWrite(var, 0xF);
    }
    else if (parentSwizzle != nullptr && grandParentBinary != nullptr &&
             grandParentBinary->getOp() == EOpAssign && grandParentBinary->getLeft() == parent &&
             isDirectlyInMain(2) && !mMainHasReturn)
    {
        // Some channels are written to
        ASSERT(mIsInMain);
        uint8_t channels = 0;
        for (int channel : parentSwizzle->getSwizzleOffsets())
        {
            channels |= static_cast<uint8_t>(1 << channel);
        }
        markWrite(var, channels);
    }
    else
    {
        // For simplicity, assume read.  If other write patterns need to be detected, they can be
        // done here.
        markRead(var);
    }
}

VariableReplacementMap FindUnusedInoutVariablesTraverser::getReplacementMap() const
{
    VariableReplacementMap replacementMap;

    if (mShaderHasDiscard)
    {
        return replacementMap;
    }

    for (auto iter : mAssignedChannels)
    {
        const TVariable *var          = iter.first;
        const uint8_t writtenChannels = iter.second;

        // Only replace variables that have only been written to.
        if (mVariablesWithLoadAccess.find(var) != mVariablesWithLoadAccess.end())
        {
            continue;
        }

        // ... and have overwritten all their channels
        const TType &type         = var->getType();
        const uint8_t allChannels = angle::BitMask<uint8_t>(type.getNominalSize());
        if ((writtenChannels & allChannels) != allChannels)
        {
            continue;
        }

        // Create an identical variable, only with EvqFragmentOut instead of EvqFragmentInOut.
        TType *newType = new TType(type);
        newType->setQualifier(EvqFragmentOut);
        const TVariable *replacement =
            new TVariable(mSymbolTable, var->name(), newType, var->symbolType());

        replacementMap[var] = new TIntermSymbol(replacement);
    }

    return replacementMap;
}
}  // namespace

bool RemoveUnusedFramebufferFetch(TCompiler *compiler,
                                  TIntermBlock *root,
                                  TSymbolTable *symbolTable)
{
    FindUnusedInoutVariablesTraverser traverser(symbolTable);
    root->traverse(&traverser);

    VariableReplacementMap replacementMap = traverser.getReplacementMap();
    if (replacementMap.empty())
    {
        return true;
    }

    return ReplaceVariables(compiler, root, replacementMap);
}

}  // namespace sh
