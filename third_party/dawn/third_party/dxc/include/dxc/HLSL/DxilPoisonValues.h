///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilPoisonValues.h                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Allows insertion of poisoned values with error messages that get          //
// cleaned up late in the compiler.                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "llvm/IR/DebugLoc.h"

namespace llvm {
class Instruction;
class Type;
class Value;
class Module;
} // namespace llvm

namespace hlsl {
// Create a special dx.poison.* instruction with debug location and an error
// message. The reason for this is certain invalid code is allowed in the code
// as long as it is removed by optimization at the end of compilation. We only
// want to emit the error for real if we are sure the code with the problem is
// in the final DXIL.
//
// This "emits" an error message with the specified type. If by the end the
// compilation it is still used, then FinalizePoisonValues will emit the correct
// error for real.
llvm::Value *CreatePoisonValue(llvm::Type *ty, const llvm::Twine &errMsg,
                               llvm::DebugLoc DL, llvm::Instruction *InsertPt);

// If there's any dx.poison.* values present in the module, emit them. Returns
// true M was modified in any way.
bool FinalizePoisonValues(llvm::Module &M);
} // namespace hlsl
