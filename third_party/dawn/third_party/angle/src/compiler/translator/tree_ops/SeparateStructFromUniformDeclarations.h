//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SeparateStructFromUniformDeclarations: Separate struct declarations from uniform declarations.
// It necessarily gives nameless uniform structs internal names.
//
// For example:
//   uniform struct { int a; } uni;
// becomes:
//   struct s1 { int a; };
//   uniform s1 uni;
//
// And:
//   uniform struct S { int a; } uni;
// becomes:
//   struct S { int a; };
//   uniform S uni;
//
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SEPARATESTRUCTFROMUNIFORMDECLARATIONS_H_
#define COMPILER_TRANSLATOR_TREEOPS_SEPARATESTRUCTFROMUNIFORMDECLARATIONS_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool SeparateStructFromUniformDeclarations(TCompiler *compiler,
                                                         TIntermBlock *root,
                                                         TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SEPARATESTRUCTFROMUNIFORMDECLARATIONS_H_
