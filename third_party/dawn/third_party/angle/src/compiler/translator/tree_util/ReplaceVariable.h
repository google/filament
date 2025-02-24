//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceVariable.h: Replace all references to a specific variable in the AST with references to
// another variable.

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REPLACEVARIABLE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REPLACEVARIABLE_H_

#include "common/angleutils.h"
#include "common/hash_containers.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TIntermTyped;
class TSymbolTable;
class TVariable;

[[nodiscard]] bool ReplaceVariable(TCompiler *compiler,
                                   TIntermBlock *root,
                                   const TVariable *toBeReplaced,
                                   const TVariable *replacement);
[[nodiscard]] bool ReplaceVariableWithTyped(TCompiler *compiler,
                                            TIntermBlock *root,
                                            const TVariable *toBeReplaced,
                                            const TIntermTyped *replacement);

using VariableReplacementMap = angle::HashMap<const TVariable *, const TIntermTyped *>;

// Replace a set of variables with their corresponding expression.
[[nodiscard]] bool ReplaceVariables(TCompiler *compiler,
                                    TIntermBlock *root,
                                    const VariableReplacementMap &variableMap);

// Find all declarators, and replace the TVariable they are declaring with a duplicate.  This is
// used to support deepCopy of TIntermBlock and TIntermLoop nodes that include declarations.
// Replacements already present in variableMap are preserved.
void GetDeclaratorReplacements(TSymbolTable *symbolTable,
                               TIntermBlock *root,
                               VariableReplacementMap *variableMap);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_REPLACEVARIABLE_H_
