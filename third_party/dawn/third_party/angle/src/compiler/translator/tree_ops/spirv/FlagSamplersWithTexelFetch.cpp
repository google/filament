//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FlagSamplersForTexelFetch.cpp: finds all instances of texelFetch used with a static reference to
// a sampler uniform, and flag that uniform as having been used with texelFetch
//

#include "compiler/translator/tree_ops/spirv/FlagSamplersWithTexelFetch.h"

#include "angle_gl.h"
#include "common/utilities.h"

#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{

class FlagSamplersWithTexelFetchTraverser : public TIntermTraverser
{
  public:
    FlagSamplersWithTexelFetchTraverser(TSymbolTable *symbolTable,
                                        std::vector<ShaderVariable> *uniforms)
        : TIntermTraverser(true, true, true, symbolTable), mUniforms(uniforms)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        // Decide if the node is a call to texelFetch[Offset]
        if (!BuiltInGroup::IsBuiltIn(node->getOp()))
        {
            return true;
        }

        ASSERT(node->getFunction()->symbolType() == SymbolType::BuiltIn);
        if (node->getFunction()->name() != "texelFetch" &&
            node->getFunction()->name() != "texelFetchOffset")
        {
            return true;
        }

        const TIntermSequence *sequence = node->getSequence();

        ASSERT(sequence->size() > 0);

        TIntermSymbol *samplerSymbol = sequence->at(0)->getAsSymbolNode();
        ASSERT(samplerSymbol != nullptr);

        const TVariable &samplerVariable = samplerSymbol->variable();

        for (ShaderVariable &uniform : *mUniforms)
        {
            if (samplerVariable.name() == uniform.name)
            {
                ASSERT(gl::IsSamplerType(uniform.type));
                uniform.texelFetchStaticUse = true;
                break;
            }
        }

        return true;
    }

  private:
    std::vector<ShaderVariable> *mUniforms;
};

}  // anonymous namespace

bool FlagSamplersForTexelFetch(TCompiler *compiler,
                               TIntermBlock *root,
                               TSymbolTable *symbolTable,
                               std::vector<ShaderVariable> *uniforms)
{
    ASSERT(uniforms != nullptr);
    if (uniforms->size() > 0)
    {
        FlagSamplersWithTexelFetchTraverser traverser(symbolTable, uniforms);
        root->traverse(&traverser);
    }

    return true;
}

}  // namespace sh
