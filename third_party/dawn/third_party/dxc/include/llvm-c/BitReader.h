/*===-- llvm-c/BitReader.h - BitReader Library C Interface ------*- C++ -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header declares the C interface to libLLVMBitReader.a, which          *|
|* implements input of the LLVM bitcode format.                               *|
|*                                                                            *|
|* Many exotic languages can interoperate with C code but have a harder time  *|
|* with C++ due to name mangling. So in addition to C, this interface enables *|
|* tools written in such languages.                                           *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_BITREADER_H
#define LLVM_C_BITREADER_H

#include "llvm-c/Core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup LLVMCBitReader Bit Reader
 * @ingroup LLVMC
 *
 * @{
 */

/* Builds a module from the bitcode in the specified memory buffer, returning a
   reference to the module via the OutModule parameter. Returns 0 on success.
   Optionally returns a human-readable error message via OutMessage. */
LLVMBool LLVMParseBitcode(LLVMMemoryBufferRef MemBuf,
                          LLVMModuleRef *OutModule, char **OutMessage);

LLVMBool LLVMParseBitcodeInContext(LLVMContextRef ContextRef,
                                   LLVMMemoryBufferRef MemBuf,
                                   LLVMModuleRef *OutModule, char **OutMessage);

/** Reads a module from the specified path, returning via the OutMP parameter
    a module provider which performs lazy deserialization. Returns 0 on success.
    Optionally returns a human-readable error message via OutMessage. */
LLVMBool LLVMGetBitcodeModuleInContext(LLVMContextRef ContextRef,
                                       LLVMMemoryBufferRef MemBuf,
                                       LLVMModuleRef *OutM,
                                       char **OutMessage);

LLVMBool LLVMGetBitcodeModule(LLVMMemoryBufferRef MemBuf, LLVMModuleRef *OutM,
                              char **OutMessage);


/** Deprecated: Use LLVMGetBitcodeModuleInContext instead. */
LLVMBool LLVMGetBitcodeModuleProviderInContext(LLVMContextRef ContextRef,
                                               LLVMMemoryBufferRef MemBuf,
                                               LLVMModuleProviderRef *OutMP,
                                               char **OutMessage);

/** Deprecated: Use LLVMGetBitcodeModule instead. */
LLVMBool LLVMGetBitcodeModuleProvider(LLVMMemoryBufferRef MemBuf,
                                      LLVMModuleProviderRef *OutMP,
                                      char **OutMessage);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
