//===--- Rewriters.h - Rewriter implementations     -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This header contains miscellaneous utilities for various front-end actions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_REWRITE_FRONTEND_REWRITERS_H
#define LLVM_CLANG_REWRITE_FRONTEND_REWRITERS_H

#include "clang/Basic/LLVM.h"
// HLSL Change Begin - RewriteIncludesToSnippet
#include <string>
#include <vector>
// HLSL Change End
namespace clang {
class Preprocessor;
class PreprocessorOutputOptions;

/// RewriteMacrosInInput - Implement -rewrite-macros mode.
void RewriteMacrosInInput(Preprocessor &PP, raw_ostream *OS);

/// DoRewriteTest - A simple test for the TokenRewriter class.
void DoRewriteTest(Preprocessor &PP, raw_ostream *OS);

/// RewriteIncludesInInput - Implement -frewrite-includes mode.
void RewriteIncludesInInput(Preprocessor &PP, raw_ostream *OS,
                            const PreprocessorOutputOptions &Opts);

// HLSL Change Begin - RewriteIncludesToSnippet
/// RewriteIncludesToSnippet - Write include files into snippets.
void RewriteIncludesToSnippet(Preprocessor &PP,
                              const PreprocessorOutputOptions &Opts,
                              std::vector<std::string> &Snippets);
// HLSL Change End
}  // end namespace clang

#endif
