//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_MAPVARIABLESTOMEMBERACCESS_H_
#define COMPILER_TRANSLATOR_MSL_MAPVARIABLESTOMEMBERACCESS_H_

#include <functional>

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

// Maps TIntermSymbol nodes to TIntermNode nodes.
// The parent function of a symbol is provided to the mapping when applicable.
[[nodiscard]] bool MapSymbols(TCompiler &compiler,
                              TIntermBlock &root,
                              std::function<TIntermNode &(const TFunction *, TIntermSymbol &)> map);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_MAPVARIABLESTOMEMBERACCESS_H_
