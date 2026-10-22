///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// assert.cpp                                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "assert.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

#if defined(LLVM_ASSERTIONS_TRAP) || !defined(WIN32)
namespace {
void llvm_assert_trap(const char *_Message, const char *_File, unsigned _Line,
                      const char *_Function) {
  llvm::errs() << "Error: assert(" << _Message << ")\nFile:\n"
               << _File << "(" << _Line << ")\nFunc:\t" << _Function << "\n";
  LLVM_BUILTIN_TRAP;
}
} // namespace
#endif

#ifdef _WIN32
#include "dxc/Support/Global.h"
#include "windows.h"

void llvm_assert(const char *Message, const char *File, unsigned Line,
                 const char *Function) {
#ifdef LLVM_ASSERTIONS_TRAP
  llvm_assert_trap(Message, File, Line, Function);
#else
  OutputDebugFormatA("Error: assert(%s)\nFile:\n%s(%d)\nFunc:\t%s\n", Message,
                     File, Line, Function);
  RaiseException(STATUS_LLVM_ASSERT, 0, 0, 0);
#endif
}

#else /* _WIN32 */

void llvm_assert(const char *Message, const char *File, unsigned Line,
                 const char *Function) {
  llvm_assert_trap(Message, File, Line, Function);
}

#endif /* _WIN32 */
