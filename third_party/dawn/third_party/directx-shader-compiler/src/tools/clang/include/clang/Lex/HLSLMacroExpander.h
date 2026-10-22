//===--- HLSLMacroExpander.h - Standalone Macro expansion ------*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLSLMacroExpander.h                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file defines utilites for expanding macros after lexing has         //
//  completed. Normally, macros are expanded as part of the lexing           //
//  phase and returned in an expanded form directly from the lexer.          //
//  For hlsl we need to be able to expand macros after the fact to           //
//  correctly capture semantic defines and root signature defines.           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#ifndef LLVM_CLANG_LEX_HLSLMACROEXPANDER_H
#define LLVM_CLANG_LEX_HLSLMACROEXPANDER_H

#include "clang/Basic/SourceLocation.h"

#include <string>
#include <utility>

namespace clang {
class Preprocessor;
class Token;
class MacroInfo;
} // namespace clang

namespace llvm {
class StringRef;
}

namespace hlsl {
class MacroExpander {
public:
  // Options used during macro expansion.
  enum Option : unsigned {
    // Strip quotes from string literals. Enables concatenating adjacent
    // string literals into a single value.
    STRIP_QUOTES = 1 << 1,
  };

  // Constructor
  MacroExpander(clang::Preprocessor &PP, unsigned options = 0);

  // Expand the given macro into the output string.
  // Returns true if macro was expanded successfully.
  bool ExpandMacro(clang::MacroInfo *macro, std::string *out);

  // Look in the preprocessor for a macro with the provided name.
  // Return nullptr if the macro could not be found.
  static clang::MacroInfo *FindMacroInfo(clang::Preprocessor &PP,
                                         llvm::StringRef macroName);

private:
  clang::Preprocessor &PP;
  clang::FileID m_expansionFileId;
  bool m_stripQuotes;
};
} // namespace hlsl

#endif // header include guard