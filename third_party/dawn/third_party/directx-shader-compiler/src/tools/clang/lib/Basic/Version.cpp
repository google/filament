//===- Version.cpp - Clang Version Number -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines several version-related utility functions for Clang.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/Version.h"
#include "clang/Basic/LLVM.h"
#include "clang/Config/config.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <cstring>

#ifdef HAVE_SVN_VERSION_INC
#  include "SVNVersion.inc"
#endif

// HLSL Change Starts
#include "dxcversion.inc"

// Here are some defaults, but these should be defined in dxcversion.inc
#ifndef HLSL_TOOL_NAME
  #define HLSL_TOOL_NAME "dxc(private)"
#endif
#ifndef HLSL_LLVM_IDENT
  #ifdef RC_FILE_VERSION
    #define HLSL_LLVM_IDENT HLSL_TOOL_NAME " " RC_FILE_VERSION
  #else
    #define HLSL_LLVM_IDENT HLSL_TOOL_NAME " version unknown"
  #endif
#endif
#ifndef HLSL_VERSION_MACRO
  #ifdef RC_PRODUCT_VERSION
    #define HLSL_VERSION_MACRO HLSL_TOOL_NAME " " RC_PRODUCT_VERSION
  #else
    #define HLSL_VERSION_MACRO HLSL_TOOL_NAME " version unknown"
  #endif
#endif
// HLSL Change Ends

namespace clang {

std::string getClangRepositoryPath() {
#ifdef HLSL_FIXED_VER // HLSL Change Starts
  return std::string();
#else
#if defined(CLANG_REPOSITORY_STRING)
  return CLANG_REPOSITORY_STRING;
#else
#ifdef SVN_REPOSITORY
  StringRef URL(SVN_REPOSITORY);
#else
  StringRef URL("");
#endif

  // If the SVN_REPOSITORY is empty, try to use the SVN keyword. This helps us
  // pick up a tag in an SVN export, for example.
  StringRef SVNRepository("$URL: https://llvm.org/svn/llvm-project/cfe/tags/RELEASE_370/final/lib/Basic/Version.cpp $");
  if (URL.empty()) {
    URL = SVNRepository.slice(SVNRepository.find(':'),
                              SVNRepository.find("/lib/Basic"));
  }

  // Strip off version from a build from an integration branch.
  URL = URL.slice(0, URL.find("/src/tools/clang"));

  // Trim path prefix off, assuming path came from standard cfe path.
  size_t Start = URL.find("cfe/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start + 4);

  return URL;
#endif
#endif // HLSL Change Ends
}

std::string getLLVMRepositoryPath() {
#ifdef HLSL_FIXED_VER // HLSL Change Starts
  return std::string();
#else
#ifdef LLVM_REPOSITORY
  StringRef URL(LLVM_REPOSITORY);
#else
  StringRef URL("");
#endif

  // Trim path prefix off, assuming path came from standard llvm path.
  // Leave "llvm/" prefix to distinguish the following llvm revision from the
  // clang revision.
  size_t Start = URL.find("llvm/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start);

  return URL;
#endif // HLSL Change Ends
}

std::string getClangRevision() {
#ifdef HLSL_FIXED_VER // HLSL Change Starts
  return std::string();
#else
#ifdef SVN_REVISION
  return SVN_REVISION;
#else
  return "";
#endif
#endif // HLSL Change Ends
}

std::string getLLVMRevision() {
#ifdef HLSL_FIXED_VER // HLSL Change Starts
  return std::string();
#else
#ifdef LLVM_REVISION
  return LLVM_REVISION;
#else
  return "";
#endif
#endif // HLSL Change Ends
}

std::string getClangFullRepositoryVersion() {
#ifdef HLSL_FIXED_VER // HLSL Change Starts
  return std::string();
#else
  std::string buf;
  llvm::raw_string_ostream OS(buf);
  std::string Path = getClangRepositoryPath();
  std::string Revision = getClangRevision();
  if (!Path.empty() || !Revision.empty()) {
    OS << '(';
    if (!Path.empty())
      OS << Path;
    if (!Revision.empty()) {
      if (!Path.empty())
        OS << ' ';
      OS << Revision;
    }
    OS << ')';
  }
  // Support LLVM in a separate repository.
  std::string LLVMRev = getLLVMRevision();
  if (!LLVMRev.empty() && LLVMRev != Revision) {
    OS << " (";
    std::string LLVMRepo = getLLVMRepositoryPath();
    if (!LLVMRepo.empty())
      OS << LLVMRepo << ' ';
    OS << LLVMRev << ')';
  }
  return OS.str();
#endif
}

std::string getClangFullVersion() {
  return getClangToolFullVersion(HLSL_TOOL_NAME); // HLSL Change
}

std::string getClangToolFullVersion(StringRef ToolName) {
#ifdef HLSL_LLVM_IDENT // HLSL Change Starts
  return std::string(HLSL_LLVM_IDENT);
#else // HLSL Change Ends
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << ToolName << " version " CLANG_VERSION_STRING " "
     << getClangFullRepositoryVersion();

  // If vendor supplied, include the base LLVM version as well.
#ifdef CLANG_VENDOR
  OS << " (based on " << BACKEND_PACKAGE_STRING << ")";
#endif

  return OS.str();
#endif // HLSL Change
}

std::string getClangFullCPPVersion() {
#ifdef HLSL_VERSION_MACRO // HLSL Change Starts
  return std::string(HLSL_VERSION_MACRO);
#else // HLSL Change Ends
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << "unofficial";

  return OS.str();
#endif // HLSL Change
}

// HLSL Change Starts
#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
#include "GitCommitInfo.inc"
uint32_t getGitCommitCount() { return kGitCommitCount; }
const char *getGitCommitHash() { return kGitCommitHash; }
#endif // SUPPORT_QUERY_GIT_COMMIT_INFO
// HLSL Change Ends

} // end namespace clang
