//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/glsl/RegenerateStructNames.h"

#include "common/debug.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

#include <set>

namespace sh
{

namespace
{
constexpr const ImmutableString kPrefix("_webgl_struct_");
}  // anonymous namespace

class RegenerateStructNamesTraverser : public TIntermTraverser
{
  public:
    RegenerateStructNamesTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable), mScopeDepth(0)
    {}

  protected:
    void visitSymbol(TIntermSymbol *) override;
    bool visitBlock(Visit, TIntermBlock *block) override;

  private:
    // Indicating the depth of the current scope.
    // The global scope is 1.
    int mScopeDepth;

    // If a struct is declared globally, push its ID in this set.
    std::set<int> mDeclaredGlobalStructs;
};

void RegenerateStructNamesTraverser::visitSymbol(TIntermSymbol *symbol)
{
    ASSERT(symbol);
    const TType &type          = symbol->getType();
    const TStructure *userType = type.getStruct();
    if (!userType)
        return;

    if (userType->symbolType() == SymbolType::BuiltIn ||
        userType->symbolType() == SymbolType::Empty)
    {
        // Built-in struct or nameless struct, do not touch it.
        return;
    }

    int uniqueId = userType->uniqueId().get();

    ASSERT(mScopeDepth > 0);
    if (mScopeDepth == 1)
    {
        // If a struct is defined at global scope, we don't map its name.
        // This is because at global level, the struct might be used to
        // declare a uniform, so the same name needs to stay the same for
        // vertex/fragment shaders. However, our mapping uses internal ID,
        // which will be different for the same struct in vertex/fragment
        // shaders.
        // This is OK because names for any structs defined in other scopes
        // will begin with "_webgl", which is reserved. So there will be
        // no conflicts among unmapped struct names from global scope and
        // mapped struct names from other scopes.
        // However, we need to keep track of these global structs, so if a
        // variable is used in a local scope, we don't try to modify the
        // struct name through that variable.
        mDeclaredGlobalStructs.insert(uniqueId);
        return;
    }
    if (mDeclaredGlobalStructs.count(uniqueId) > 0)
        return;
    // Map {name} to _webgl_struct_{uniqueId}_{name}.
    if (userType->name().beginsWith(kPrefix))
    {
        // The name has already been regenerated.
        return;
    }
    ImmutableStringBuilder tmp(kPrefix.length() + sizeof(uniqueId) * 2u + 1u +
                               userType->name().length());
    tmp << kPrefix;
    tmp.appendHex(uniqueId);
    tmp << '_' << userType->name();

    // TODO(oetuaho): Add another mechanism to change symbol names so that the const_cast is not
    // needed.
    const_cast<TStructure *>(userType)->setName(tmp);
}

bool RegenerateStructNamesTraverser::visitBlock(Visit, TIntermBlock *block)
{
    ++mScopeDepth;
    TIntermSequence &sequence = *(block->getSequence());
    for (TIntermNode *node : sequence)
    {
        node->traverse(this);
    }
    --mScopeDepth;
    return false;
}

bool RegenerateStructNames(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    RegenerateStructNamesTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return compiler->validateAST(root);
}

}  // namespace sh
