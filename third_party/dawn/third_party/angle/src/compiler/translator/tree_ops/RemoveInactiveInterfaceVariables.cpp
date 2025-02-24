//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveInactiveInterfaceVariables.h:
//  Drop shader interface variable declarations for those that are inactive.
//

#include "compiler/translator/tree_ops/RemoveInactiveInterfaceVariables.h"

#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

// Traverser that removes all declarations that correspond to inactive variables.
class RemoveInactiveInterfaceVariablesTraverser : public TIntermTraverser
{
  public:
    RemoveInactiveInterfaceVariablesTraverser(
        TSymbolTable *symbolTable,
        const std::vector<sh::ShaderVariable> &attributes,
        const std::vector<sh::ShaderVariable> &inputVaryings,
        const std::vector<sh::ShaderVariable> &outputVariables,
        const std::vector<sh::ShaderVariable> &uniforms,
        const std::vector<sh::InterfaceBlock> &interfaceBlocks,
        bool removeFragmentOutputs);

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;

  private:
    const std::vector<sh::ShaderVariable> &mAttributes;
    const std::vector<sh::ShaderVariable> &mInputVaryings;
    const std::vector<sh::ShaderVariable> &mOutputVariables;
    const std::vector<sh::ShaderVariable> &mUniforms;
    const std::vector<sh::InterfaceBlock> &mInterfaceBlocks;
    bool mRemoveFragmentOutputs;
};

RemoveInactiveInterfaceVariablesTraverser::RemoveInactiveInterfaceVariablesTraverser(
    TSymbolTable *symbolTable,
    const std::vector<sh::ShaderVariable> &attributes,
    const std::vector<sh::ShaderVariable> &inputVaryings,
    const std::vector<sh::ShaderVariable> &outputVariables,
    const std::vector<sh::ShaderVariable> &uniforms,
    const std::vector<sh::InterfaceBlock> &interfaceBlocks,
    bool removeFragmentOutputs)
    : TIntermTraverser(true, false, false, symbolTable),
      mAttributes(attributes),
      mInputVaryings(inputVaryings),
      mOutputVariables(outputVariables),
      mUniforms(uniforms),
      mInterfaceBlocks(interfaceBlocks),
      mRemoveFragmentOutputs(removeFragmentOutputs)
{}

template <typename Variable>
bool IsVariableActive(const std::vector<Variable> &mVars, const ImmutableString &name)
{
    for (const Variable &var : mVars)
    {
        if (name == var.name)
        {
            return var.active;
        }
    }
    UNREACHABLE();
    return true;
}

bool RemoveInactiveInterfaceVariablesTraverser::visitDeclaration(Visit visit,
                                                                 TIntermDeclaration *node)
{
    // SeparateDeclarations should have already been run.
    ASSERT(node->getSequence()->size() == 1u);

    TIntermTyped *declarator = node->getSequence()->front()->getAsTyped();
    ASSERT(declarator);

    TIntermSymbol *asSymbol = declarator->getAsSymbolNode();
    if (!asSymbol)
    {
        return false;
    }

    const TType &type = declarator->getType();

    // Remove all shader interface variables except outputs, i.e. uniforms, interface blocks and
    // inputs.
    //
    // Imagine a situation where the VS doesn't write to a varying but the FS reads from it.  This
    // is allowed, though the value of the varying is undefined.  If the varying is removed here,
    // the situation is changed to VS not declaring the varying, but the FS reading from it, which
    // is not allowed.  That's why inactive shader outputs are not removed.
    //
    // Inactive fragment shader outputs can be removed though, as there is no next stage.
    bool removeDeclaration     = false;
    const TQualifier qualifier = type.getQualifier();

    if (type.isInterfaceBlock())
    {
        // When a member has an explicit location, interface block should not be removed.
        // If the member or interface would be removed, GetProgramResource could not return the
        // location.
        if (!IsShaderIoBlock(type.getQualifier()) && type.getQualifier() != EvqPatchIn &&
            type.getQualifier() != EvqPatchOut)
        {
            removeDeclaration =
                !IsVariableActive(mInterfaceBlocks, type.getInterfaceBlock()->name());
        }
    }
    else if (qualifier == EvqUniform)
    {
        removeDeclaration = !IsVariableActive(mUniforms, asSymbol->getName());
    }
    else if (qualifier == EvqAttribute || qualifier == EvqVertexIn)
    {
        removeDeclaration = !IsVariableActive(mAttributes, asSymbol->getName());
    }
    else if (IsShaderIn(qualifier))
    {
        removeDeclaration = !IsVariableActive(mInputVaryings, asSymbol->getName());
    }
    else if (qualifier == EvqFragmentOut)
    {
        removeDeclaration =
            !IsVariableActive(mOutputVariables, asSymbol->getName()) && mRemoveFragmentOutputs;
    }

    if (removeDeclaration)
    {
        TIntermSequence replacement;

        // If the declaration was of a struct, keep the struct declaration itself.
        if (type.isStructSpecifier())
        {
            TType *structSpecifierType      = new TType(type.getStruct(), true);
            TVariable *emptyVariable        = new TVariable(mSymbolTable, kEmptyImmutableString,
                                                            structSpecifierType, SymbolType::Empty);
            TIntermDeclaration *declaration = new TIntermDeclaration();
            declaration->appendDeclarator(new TIntermSymbol(emptyVariable));
            replacement.push_back(declaration);
        }

        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                        std::move(replacement));
    }

    return false;
}

bool RemoveInactiveInterfaceVariablesTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    // Remove any code that initOutputVariables might have added corresponding to inactive
    // output variables.  This code is always in the form of `variable = ...;`.
    if (node->getOp() != EOpAssign)
    {
        // Don't recurse, won't find the initialization nested in another expression.
        return false;
    }

    // Get the symbol being initialized, and check if it's an inactive output.  If it is, this must
    // necessarily be initialization code that ANGLE has added (and wasn't there in the original
    // shader; if it was, the symbol wouldn't have been inactive).
    TIntermSymbol *symbol = node->getLeft()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return false;
    }

    const TQualifier qualifier = symbol->getType().getQualifier();
    if (qualifier != EvqFragmentOut || IsVariableActive(mOutputVariables, symbol->getName()))
    {
        return false;
    }

    // Drop the initialization code.
    TIntermSequence replacement;
    mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, std::move(replacement));
    return false;
}

}  // namespace

bool RemoveInactiveInterfaceVariables(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable *symbolTable,
                                      const std::vector<sh::ShaderVariable> &attributes,
                                      const std::vector<sh::ShaderVariable> &inputVaryings,
                                      const std::vector<sh::ShaderVariable> &outputVariables,
                                      const std::vector<sh::ShaderVariable> &uniforms,
                                      const std::vector<sh::InterfaceBlock> &interfaceBlocks,
                                      bool removeFragmentOutputs)
{
    RemoveInactiveInterfaceVariablesTraverser traverser(symbolTable, attributes, inputVaryings,
                                                        outputVariables, uniforms, interfaceBlocks,
                                                        removeFragmentOutputs);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
