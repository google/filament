///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcvalidator.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the Dxil Validation                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

struct IDxcOperationResult;
struct IDxcBlob;
struct DxcBuffer;

namespace llvm {
class Module;
class raw_ostream;
class LLVMContext;
} // namespace llvm

namespace hlsl {

// For internal use only.
uint32_t validateWithOptDebugModule(
    IDxcBlob *Shader,            // Shader to validate.
    uint32_t Flags,              // Validation flags.
    llvm::Module *DebugModule,   // Debug module to validate, if available
    IDxcOperationResult **Result // Validation output status, buffer, and errors
);

// IDxcValidator2
uint32_t validateWithDebug(
    IDxcBlob *Shader,            // Shader to validate.
    uint32_t Flags,              // Validation flags.
    DxcBuffer *OptDebugBitcode,  // Optional debug module bitcode to provide
                                 // line numbers
    IDxcOperationResult **Result // Validation output status, buffer, and errors
);

uint32_t getValidationVersion(unsigned *pMajor, unsigned *pMinor);

} // namespace hlsl
