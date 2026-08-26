//===- HLSLEmbeddedHeaders.cpp - Embedded HLSL header data ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements clang::hlsl::getEmbeddedHeaders() by composing two small
// build-time generated ``.inc`` files (one with the per-header
// ``Data`` StringRef declarations, one with an X-macro list of the
// (rel_path, namespace) pairs) with hand-written boilerplate that
// remains in source control.
//
//===----------------------------------------------------------------------===//

#include "clang/Lex/HLSLEmbeddedHeaders.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

#include <utility>

// Pulls in one ``namespace ns { #include "<path>.inc" }`` block per
// embedded header.  The included ``.inc`` snippets define a
// ``llvm::StringRef Data`` at namespace scope.
#include "HLSLEmbeddedHeadersDecls.inc"

namespace hlsl {

const llvm::StringMap<llvm::StringRef> &getEmbeddedHeaders() {
  static const llvm::StringMap<llvm::StringRef> Map = []() {
    llvm::StringMap<llvm::StringRef> M;
#define HLSL_EMBEDDED_HEADER(REL, NS)                                          \
  M.insert(std::make_pair(llvm::StringRef(REL), NS::Data));
#include "HLSLEmbeddedHeadersEntries.inc"
#undef HLSL_EMBEDDED_HEADER
    return M;
  }();
  return Map;
}

} // namespace hlsl
