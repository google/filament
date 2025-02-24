//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/RemoveInvariantDeclaration.h"

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// An AST traverser that removes invariant declaration for input in fragment shader
// when GLSL >= 4.20 and for output in vertex shader when GLSL < 4.2.
class RemoveInvariantDeclarationTraverser : public TIntermTraverser
{
  public:
    RemoveInvariantDeclarationTraverser() : TIntermTraverser(true, false, false) {}

  private:
    bool visitGlobalQualifierDeclaration(Visit visit,
                                         TIntermGlobalQualifierDeclaration *node) override
    {
        if (node->isInvariant())
        {
            TIntermSequence emptyReplacement;
            mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                            std::move(emptyReplacement));
        }
        return false;
    }
};

}  // anonymous namespace

bool RemoveInvariantDeclaration(TCompiler *compiler, TIntermNode *root)
{
    RemoveInvariantDeclarationTraverser traverser;
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
