//===--- CheckerRegistration.cpp - Registration for the Analyzer Checkers -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Defines the registration function for the analyzer checkers.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Frontend/CheckerRegistration.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/StaticAnalyzer/Checkers/ClangCheckers.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/CheckerOptInfo.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Frontend/FrontendActions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace clang;
using namespace ento;
using llvm::sys::DynamicLibrary;

namespace {
class ClangCheckerRegistry : public CheckerRegistry {
  typedef void (*RegisterCheckersFn)(CheckerRegistry &);

  static bool isCompatibleAPIVersion(const char *versionString);
  static void warnIncompatible(DiagnosticsEngine *diags, StringRef pluginPath,
                               const char *pluginAPIVersion);

public:
  ClangCheckerRegistry(ArrayRef<std::string> plugins,
                       DiagnosticsEngine *diags = nullptr);
};
  
} // end anonymous namespace

ClangCheckerRegistry::ClangCheckerRegistry(ArrayRef<std::string> plugins,
                                           DiagnosticsEngine *diags) {
  registerBuiltinCheckers(*this);

  for (ArrayRef<std::string>::iterator i = plugins.begin(), e = plugins.end();
       i != e; ++i) {
    // Get access to the plugin.
    DynamicLibrary lib = DynamicLibrary::getPermanentLibrary(i->c_str());

    // See if it's compatible with this build of clang.
    const char *pluginAPIVersion =
      (const char *) lib.getAddressOfSymbol("clang_analyzerAPIVersionString");
    if (!isCompatibleAPIVersion(pluginAPIVersion)) {
      warnIncompatible(diags, *i, pluginAPIVersion);
      continue;
    }

    // Register its checkers.
    RegisterCheckersFn registerPluginCheckers =
      (RegisterCheckersFn) (intptr_t) lib.getAddressOfSymbol(
                                                      "clang_registerCheckers");
    if (registerPluginCheckers)
      registerPluginCheckers(*this);
  }
}

bool ClangCheckerRegistry::isCompatibleAPIVersion(const char *versionString) {
  // If the version string is null, it's not an analyzer plugin.
  if (!versionString)
    return false;

  // For now, none of the static analyzer API is considered stable.
  // Versions must match exactly.
  if (strcmp(versionString, CLANG_ANALYZER_API_VERSION_STRING) == 0)
    return true;

  return false;
}

void ClangCheckerRegistry::warnIncompatible(DiagnosticsEngine *diags,
                                            StringRef pluginPath,
                                            const char *pluginAPIVersion) {
  if (!diags)
    return;
  if (!pluginAPIVersion)
    return;

  diags->Report(diag::warn_incompatible_analyzer_plugin_api)
      << llvm::sys::path::filename(pluginPath);
  diags->Report(diag::note_incompatible_analyzer_plugin_api)
      << CLANG_ANALYZER_API_VERSION_STRING
      << pluginAPIVersion;
}

std::unique_ptr<CheckerManager>
ento::createCheckerManager(AnalyzerOptions &opts, const LangOptions &langOpts,
                           ArrayRef<std::string> plugins,
                           DiagnosticsEngine &diags) {
  std::unique_ptr<CheckerManager> checkerMgr(
      new CheckerManager(langOpts, &opts));

  SmallVector<CheckerOptInfo, 8> checkerOpts;
  for (unsigned i = 0, e = opts.CheckersControlList.size(); i != e; ++i) {
    const std::pair<std::string, bool> &opt = opts.CheckersControlList[i];
    checkerOpts.push_back(CheckerOptInfo(opt.first.c_str(), opt.second));
  }

  ClangCheckerRegistry allCheckers(plugins, &diags);
  allCheckers.initializeManager(*checkerMgr, checkerOpts);
  allCheckers.validateCheckerOptions(opts, diags);
  checkerMgr->finishedCheckerRegistration();

  for (unsigned i = 0, e = checkerOpts.size(); i != e; ++i) {
    if (checkerOpts[i].isUnclaimed()) {
      diags.Report(diag::err_unknown_analyzer_checker)
          << checkerOpts[i].getName();
      diags.Report(diag::note_suggest_disabling_all_checkers);
    }

  }

  return checkerMgr;
}

void ento::printCheckerHelp(raw_ostream &out, ArrayRef<std::string> plugins) {
  out << "OVERVIEW: Clang Static Analyzer Checkers List\n\n";
  out << "USAGE: -analyzer-checker <CHECKER or PACKAGE,...>\n\n";

  ClangCheckerRegistry(plugins).printHelp(out);
}
