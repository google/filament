//===--- Utils.h - Misc utilities for the front-end -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This header contains miscellaneous utilities for various front-end actions
//  which were split from Frontend to minimise Frontend's dependencies.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTENDTOOL_UTILS_H
#define LLVM_CLANG_FRONTENDTOOL_UTILS_H

namespace clang {

class CompilerInstance;

/// ExecuteCompilerInvocation - Execute the given actions described by the
/// compiler invocation object in the given compiler instance.
///
/// \return - True on success.
bool ExecuteCompilerInvocation(CompilerInstance *Clang);

}  // end namespace clang

#endif
