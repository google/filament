//===- HLSLEmbeddedHeaders.h - Embedded HLSL header data ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Provides access to the contents of the HLSL header files that ship with
// DXC (located under tools/clang/lib/Headers/hlsl) as compiled-in data.
//
// The data is generated at build time from those header files by
// utils/embed_header.py and utils/generate_hlsl_embedded_headers.py.  The
// resulting StringMap maps each header's path relative to the hlsl/
// directory (with no leading separator, e.g. "dx/linalg.h") to the raw
// bytes of that file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_HLSLEMBEDDEDHEADERS_H
#define LLVM_CLANG_LEX_HLSLEMBEDDEDHEADERS_H

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

namespace hlsl {

/// Returns a map from each HLSL header's relative path under
/// tools/clang/lib/Headers/hlsl (e.g. "dx/linalg.h") to a StringRef
/// holding the file's raw contents.
const llvm::StringMap<llvm::StringRef> &getEmbeddedHeaders();

} // namespace hlsl

#endif // LLVM_CLANG_LEX_HLSLEMBEDDEDHEADERS_H
