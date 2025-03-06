//===-- CheckerRegistration.h - Checker Registration Function ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_FRONTEND_CHECKERREGISTRATION_H
#define LLVM_CLANG_STATICANALYZER_FRONTEND_CHECKERREGISTRATION_H

#include "clang/Basic/LLVM.h"
#include <memory>
#include <string>

namespace clang {
  class AnalyzerOptions;
  class LangOptions;
  class DiagnosticsEngine;

namespace ento {
  class CheckerManager;

  std::unique_ptr<CheckerManager>
  createCheckerManager(AnalyzerOptions &opts, const LangOptions &langOpts,
                       ArrayRef<std::string> plugins, DiagnosticsEngine &diags);

} // end ento namespace

} // end namespace clang

#endif
