//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_MAPFUNCTIONSTODEFINITIONS_H_
#define COMPILER_TRANSLATOR_MSL_MAPFUNCTIONSTODEFINITIONS_H_

#include <unordered_map>

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// A map from functions to their corresponding definitions.
using FunctionToDefinition = std::unordered_map<const TFunction *, TIntermFunctionDefinition *>;

// Maps functions to their corresponding definitions.
[[nodiscard]] FunctionToDefinition MapFunctionsToDefinitions(TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_MAPFUNCTIONSTODEFINITIONS_H_
