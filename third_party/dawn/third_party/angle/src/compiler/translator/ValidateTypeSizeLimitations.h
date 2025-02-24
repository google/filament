//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATETYPESIZELIMITATIONS_H_
#define COMPILER_TRANSLATOR_VALIDATETYPESIZELIMITATIONS_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{

class TDiagnostics;

// Returns true if the given shader does not violate certain
// implementation-defined limits on the size of variables' types.
bool ValidateTypeSizeLimitations(TIntermNode *root,
                                 TSymbolTable *symbolTable,
                                 TDiagnostics *diagnostics);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATETYPESIZELIMITATIONS_H_
