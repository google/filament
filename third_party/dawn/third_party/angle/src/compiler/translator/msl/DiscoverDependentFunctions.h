//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_DISCOVERDEPENDENTFUNCTIONS_H_
#define COMPILER_TRANSLATOR_MSL_DISCOVERDEPENDENTFUNCTIONS_H_

#include <functional>
#include <unordered_set>

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

// Finds and returns all functions that contain the provided variables.
[[nodiscard]] std::unordered_set<const TFunction *> DiscoverDependentFunctions(
    TIntermBlock &root,
    const std::function<bool(const TVariable &)> &vars);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_DISCOVERDEPENDENTFUNCTIONS_H_
