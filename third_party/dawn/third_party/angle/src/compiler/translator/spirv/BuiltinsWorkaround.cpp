//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/spirv/BuiltinsWorkaround.h"

#include "angle_gl.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn.h"

namespace sh
{

namespace
{
constexpr const ImmutableString kGlInstanceIDString("gl_InstanceID");
constexpr const ImmutableString kGlVertexIDString("gl_VertexID");

class TBuiltinsWorkaround : public TIntermTraverser
{
  public:
    TBuiltinsWorkaround(TSymbolTable *symbolTable, const ShCompileOptions &options);

    void visitSymbol(TIntermSymbol *node) override;
    bool visitDeclaration(Visit, TIntermDeclaration *node) override;

  private:
    void ensureVersionIsAtLeast(int version);

    const ShCompileOptions &mCompileOptions;

    bool isBaseInstanceDeclared = false;
};

TBuiltinsWorkaround::TBuiltinsWorkaround(TSymbolTable *symbolTable, const ShCompileOptions &options)
    : TIntermTraverser(true, false, false, symbolTable), mCompileOptions(options)
{}

void TBuiltinsWorkaround::visitSymbol(TIntermSymbol *node)
{
    if (node->variable().symbolType() == SymbolType::BuiltIn)
    {
        if (node->getName() == kGlInstanceIDString)
        {
            TIntermSymbol *instanceIndexRef =
                new TIntermSymbol(BuiltInVariable::gl_InstanceIndex());

            if (isBaseInstanceDeclared)
            {
                TIntermSymbol *baseInstanceRef =
                    new TIntermSymbol(BuiltInVariable::angle_BaseInstance());

                TIntermBinary *subBaseInstance =
                    new TIntermBinary(EOpSub, instanceIndexRef, baseInstanceRef);
                queueReplacement(subBaseInstance, OriginalNode::IS_DROPPED);
            }
            else
            {
                queueReplacement(instanceIndexRef, OriginalNode::IS_DROPPED);
            }
        }
        else if (node->getName() == kGlVertexIDString)
        {
            TIntermSymbol *vertexIndexRef = new TIntermSymbol(BuiltInVariable::gl_VertexIndex());
            queueReplacement(vertexIndexRef, OriginalNode::IS_DROPPED);
        }
    }
}

bool TBuiltinsWorkaround::visitDeclaration(Visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());
    ASSERT(!sequence.empty());

    for (TIntermNode *variableNode : sequence)
    {
        TIntermSymbol *variable = variableNode->getAsSymbolNode();
        if (variable && variable->variable().symbolType() == SymbolType::BuiltIn)
        {
            if (variable->getName() == "angle_BaseInstance")
            {
                isBaseInstanceDeclared = true;
            }
        }
    }
    return true;
}

}  // anonymous namespace

[[nodiscard]] bool ShaderBuiltinsWorkaround(TCompiler *compiler,
                                            TIntermBlock *root,
                                            TSymbolTable *symbolTable,
                                            const ShCompileOptions &compileOptions)
{
    TBuiltinsWorkaround builtins(symbolTable, compileOptions);
    root->traverse(&builtins);
    if (!builtins.updateTree(compiler, root))
    {
        return false;
    }
    return true;
}

}  // namespace sh
