//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RunAtTheBeginningOfShader.cpp: Add code to be run at the beginning of the shader.
// void main() { body }
// =>
// void main()
// {
//     codeToRun
//     body
// }
//

#include "compiler/translator/tree_util/RunAtTheBeginningOfShader.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

bool RunAtTheBeginningOfShader(TCompiler *compiler, TIntermBlock *root, TIntermNode *codeToRun)
{
    TIntermFunctionDefinition *main = FindMain(root);
    main->getBody()->insertStatement(0, codeToRun);
    return compiler->validateAST(root);
}

}  // namespace sh
