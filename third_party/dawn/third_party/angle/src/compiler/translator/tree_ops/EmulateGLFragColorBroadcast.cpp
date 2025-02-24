//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl_FragColor needs to broadcast to all color buffers in ES2 if
// GL_EXT_draw_buffers is explicitly enabled in a fragment shader.
//
// We emulate this by replacing all gl_FragColor with gl_FragData[0], and in the end
// of main() function, assigning gl_FragData[1], ..., gl_FragData[maxDrawBuffers-1]
// with gl_FragData[0].
//
// Similar replacement applies to gl_SecondaryFragColorEXT if it is used.
//

#include "compiler/translator/tree_ops/EmulateGLFragColorBroadcast.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{

namespace
{

constexpr const ImmutableString kGlFragDataString("gl_FragData");
constexpr const ImmutableString kGlSecondaryFragDataString("gl_SecondaryFragDataEXT");

class GLFragColorBroadcastTraverser : public TIntermTraverser
{
  public:
    GLFragColorBroadcastTraverser(int maxDrawBuffers,
                                  int maxDualSourceDrawBuffers,
                                  TSymbolTable *symbolTable,
                                  int shaderVersion)
        : TIntermTraverser(true, false, false, symbolTable),
          mGLFragColorUsed(false),
          mGLSecondaryFragColorUsed(false),
          mMaxDrawBuffers(maxDrawBuffers),
          mMaxDualSourceDrawBuffers(maxDualSourceDrawBuffers),
          mShaderVersion(shaderVersion)
    {}

    [[nodiscard]] bool broadcastGLFragColor(TCompiler *compiler, TIntermBlock *root);

    bool isGLFragColorUsed() const { return mGLFragColorUsed; }
    bool isGLSecondaryFragColorUsed() const { return mGLSecondaryFragColorUsed; }

  protected:
    void visitSymbol(TIntermSymbol *node) override;

    TIntermBinary *constructGLFragDataNode(int index, bool secondary) const;
    TIntermBinary *constructGLFragDataAssignNode(int index, bool secondary) const;

  private:
    bool mGLFragColorUsed;
    bool mGLSecondaryFragColorUsed;
    int mMaxDrawBuffers;
    int mMaxDualSourceDrawBuffers;
    const int mShaderVersion;
};

TIntermBinary *GLFragColorBroadcastTraverser::constructGLFragDataNode(int index,
                                                                      bool secondary) const
{
    TIntermSymbol *symbol = ReferenceBuiltInVariable(
        secondary ? kGlSecondaryFragDataString : kGlFragDataString, *mSymbolTable, mShaderVersion);
    TIntermTyped *indexNode = CreateIndexNode(index);

    TIntermBinary *binary = new TIntermBinary(EOpIndexDirect, symbol, indexNode);
    return binary;
}

TIntermBinary *GLFragColorBroadcastTraverser::constructGLFragDataAssignNode(int index,
                                                                            bool secondary) const
{
    TIntermTyped *fragDataIndex = constructGLFragDataNode(index, secondary);
    TIntermTyped *fragDataZero  = constructGLFragDataNode(0, secondary);

    return new TIntermBinary(EOpAssign, fragDataIndex, fragDataZero);
}

void GLFragColorBroadcastTraverser::visitSymbol(TIntermSymbol *node)
{
    if (node->variable().symbolType() == SymbolType::BuiltIn)
    {
        if (node->getName() == "gl_FragColor")
        {
            queueReplacement(constructGLFragDataNode(0, false), OriginalNode::IS_DROPPED);
            mGLFragColorUsed = true;
        }
        else if (node->getName() == "gl_SecondaryFragColorEXT")
        {
            queueReplacement(constructGLFragDataNode(0, true), OriginalNode::IS_DROPPED);
            mGLSecondaryFragColorUsed = true;
        }
    }
}

bool GLFragColorBroadcastTraverser::broadcastGLFragColor(TCompiler *compiler, TIntermBlock *root)
{
    ASSERT(mMaxDrawBuffers > 1);
    ASSERT(mMaxDualSourceDrawBuffers > 0 || !mGLSecondaryFragColorUsed);
    if (!mGLFragColorUsed && !mGLSecondaryFragColorUsed)
    {
        return true;
    }

    TIntermBlock *broadcastBlock = new TIntermBlock();
    // Now insert statements
    // maxDrawBuffers is replaced with maxDualSourceDrawBuffers
    // if gl_SecondaryFragColorEXT was statically used.
    //   gl_FragData[1] = gl_FragData[0];
    //   ...
    //   gl_FragData[maxDrawBuffers - 1] = gl_FragData[0];
    if (mGLFragColorUsed)
    {
        const int buffers = mGLSecondaryFragColorUsed ? mMaxDualSourceDrawBuffers : mMaxDrawBuffers;
        for (int colorIndex = 1; colorIndex < buffers; ++colorIndex)
        {
            broadcastBlock->appendStatement(constructGLFragDataAssignNode(colorIndex, false));
        }
    }
    if (mGLSecondaryFragColorUsed)
    {
        for (int colorIndex = 1; colorIndex < mMaxDualSourceDrawBuffers; ++colorIndex)
        {
            broadcastBlock->appendStatement(constructGLFragDataAssignNode(colorIndex, true));
        }
    }
    if (broadcastBlock->getChildCount() == 0)
    {
        return true;
    }
    return RunAtTheEndOfShader(compiler, root, broadcastBlock, mSymbolTable);
}

}  // namespace

bool EmulateGLFragColorBroadcast(TCompiler *compiler,
                                 TIntermBlock *root,
                                 int maxDrawBuffers,
                                 int maxDualSourceDrawBuffers,
                                 std::vector<sh::ShaderVariable> *outputVariables,
                                 TSymbolTable *symbolTable,
                                 int shaderVersion)
{
    ASSERT(maxDrawBuffers > 1);
    GLFragColorBroadcastTraverser traverser(maxDrawBuffers, maxDualSourceDrawBuffers, symbolTable,
                                            shaderVersion);
    root->traverse(&traverser);
    if (traverser.isGLFragColorUsed() || traverser.isGLSecondaryFragColorUsed())
    {
        if (!traverser.updateTree(compiler, root))
        {
            return false;
        }
        if (!traverser.broadcastGLFragColor(compiler, root))
        {
            return false;
        }

        for (auto &var : *outputVariables)
        {
            if (var.name == "gl_FragColor")
            {
                // TODO(zmo): Find a way to keep the original variable information.
                var.name       = "gl_FragData";
                var.mappedName = "gl_FragData";
                var.arraySizes.push_back(traverser.isGLSecondaryFragColorUsed()
                                             ? maxDualSourceDrawBuffers
                                             : maxDrawBuffers);
                ASSERT(var.arraySizes.size() == 1u);
            }
            else if (var.name == "gl_SecondaryFragColorEXT")
            {
                var.name       = "gl_SecondaryFragDataEXT";
                var.mappedName = "gl_SecondaryFragDataEXT";
                var.arraySizes.push_back(maxDualSourceDrawBuffers);
                ASSERT(var.arraySizes.size() == 1u);
            }
        }
    }

    return true;
}

}  // namespace sh
