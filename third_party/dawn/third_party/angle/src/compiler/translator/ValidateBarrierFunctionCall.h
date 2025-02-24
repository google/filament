//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ValidateBarrierFunctionCalls:
//   Runs compilation checks related to the "barrier built-in function.

#ifndef COMPILER_TRANSLATOR_VALIDATEBARRIERFUNCTIONCALL_H_
#define COMPILER_TRANSLATOR_VALIDATEBARRIERFUNCTIONCALL_H_

#include "common/angleutils.h"

namespace sh
{
class TDiagnostics;
class TIntermBlock;

[[nodiscard]] bool ValidateBarrierFunctionCall(TIntermBlock *root, TDiagnostics *diagnostics);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATEBARRIERFUNCTIONCALL_H_
