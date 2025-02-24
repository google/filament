//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClampGLLayer: Clamp gl_Layer to 0 if framebuffer is not layered.
//

#include "compiler/translator/tree_ops/spirv/ClampGLLayer.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
// A traverser to check if gl_Layer is used at all.
class HasGLLayerTraverser : public TIntermTraverser
{
  public:
    HasGLLayerTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool referencesGLLayer() const { return mReferencesGLLayer; }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (symbol->getQualifier() == EvqLayerOut)
        {
            mReferencesGLLayer = true;
        }
    }

  private:
    bool mReferencesGLLayer = false;
};

// A traverser that adds `if (!layeredFramebuffer) gl_Layer = 0;` before emitVertex() in geometry
// shaders.
class ClampGLLayerTraverser : public TIntermTraverser
{
  public:
    ClampGLLayerTraverser(TSymbolTable *symbolTable,
                          const DriverUniform *driverUniforms,
                          int shaderVersion)
        : TIntermTraverser(true, false, false, symbolTable),
          mDriverUniforms(driverUniforms),
          mShaderVersion(shaderVersion)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  private:
    const DriverUniform *mDriverUniforms;
    int mShaderVersion;
};

bool ClampGLLayerTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    ASSERT(visit == Visit::PreVisit);

    if (node->getOp() != EOpEmitVertex)
    {
        return false;
    }

    // if (!layeredFramebuffer)
    TIntermTyped *layeredFramebuffer =
        new TIntermUnary(EOpLogicalNot, mDriverUniforms->getLayeredFramebuffer(), nullptr);

    // gl_Layer = 0;
    const TVariable *gl_Layer = static_cast<const TVariable *>(
        mSymbolTable->findBuiltIn(ImmutableString("gl_Layer"), mShaderVersion));
    TIntermBinary *setToZero =
        new TIntermBinary(EOpAssign, new TIntermSymbol(gl_Layer), CreateIndexNode(0));

    TIntermBlock *block = new TIntermBlock;
    block->appendStatement(setToZero);

    TIntermIfElse *ifNotLayered = new TIntermIfElse(layeredFramebuffer, block, nullptr);

    TIntermSequence replacement;
    replacement.push_back(ifNotLayered);
    replacement.push_back(node);
    mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, std::move(replacement));

    return false;
}
}  // anonymous namespace

bool ClampGLLayer(TCompiler *compiler,
                  TIntermBlock *root,
                  TSymbolTable *symbolTable,
                  const DriverUniform *driverUniforms)
{
    // First, check if there is a reference to gl_Layer.  If there isn't, there's nothing to do.
    // Note that if gl_Layer isn't otherwise set, this transformation adds static usage of it
    // without initializaing it in every path, leading to multiple drivers crashing / failing tests.
    HasGLLayerTraverser hasGLLayer(symbolTable);
    root->traverse(&hasGLLayer);
    if (!hasGLLayer.referencesGLLayer())
    {
        return true;
    }

    ClampGLLayerTraverser traverser(symbolTable, driverUniforms, compiler->getShaderVersion());
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}
}  // namespace sh
