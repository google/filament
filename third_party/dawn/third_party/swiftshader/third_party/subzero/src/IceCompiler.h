//===- subzero/src/IceCompiler.h - Compiler driver --------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the driver for translating bitcode to native code.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECOMPILER_H
#define SUBZERO_SRC_ICECOMPILER_H

#include "IceDefs.h"

namespace llvm {
class DataStreamer;
}

namespace Ice {

class ClFlags;

/// A compiler driver. It may be called to handle a single compile request.
class Compiler {
  Compiler(const Compiler &) = delete;
  Compiler &operator=(const Compiler &) = delete;

public:
  Compiler() = default;

  /// Run the compiler with the given GlobalContext for compilation state. Upon
  /// error, the Context's error status will be set.
  void run(const ClFlags &ExtraFlags, GlobalContext &Ctx,
           std::unique_ptr<llvm::DataStreamer> &&InputStream);
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECOMPILER_H
