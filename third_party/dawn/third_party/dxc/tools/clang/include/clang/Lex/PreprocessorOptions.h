//===--- PreprocessorOptions.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_PREPROCESSOROPTIONS_H_
#define LLVM_CLANG_LEX_PREPROCESSOROPTIONS_H_

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include <cassert>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace llvm {
  class MemoryBuffer;
}

namespace clang {

class Preprocessor;
class LangOptions;

/// \brief Enumerate the kinds of standard library that 
enum ObjCXXARCStandardLibraryKind {
  ARCXX_nolib,
  /// \brief libc++
  ARCXX_libcxx,
  /// \brief libstdc++
  ARCXX_libstdcxx
};
  
/// PreprocessorOptions - This class is used for passing the various options
/// used in preprocessor initialization to InitializePreprocessor().
class PreprocessorOptions : public RefCountedBase<PreprocessorOptions> {
public:
  std::vector<std::pair<std::string, bool/*isUndef*/> > Macros;
  std::vector<std::string> Includes;
  std::vector<std::string> MacroIncludes;

  /// \brief Initialize the preprocessor with the compiler and target specific
  /// predefines.
  unsigned UsePredefines : 1;

  /// \brief Whether we should maintain a detailed record of all macro
  /// definitions and expansions.
  unsigned DetailedRecord : 1;

  // HLSL Change Begin - ignore line directives.
  /// \brief Whether we should ignore #line directives.
  unsigned IgnoreLineDirectives : 1;
  /// \brief Expand the operands before performing token-pasting (fxc behavior)
  unsigned ExpandTokPastingArg : 1;
  // HLSL Change End

  /// The implicit PCH included at the start of the translation unit, or empty.
  std::string ImplicitPCHInclude;

  /// \brief Headers that will be converted to chained PCHs in memory.
  std::vector<std::string> ChainedIncludes;

  /// \brief When true, disables most of the normal validation performed on
  /// precompiled headers.
  bool DisablePCHValidation;

  /// \brief When true, a PCH with compiler errors will not be rejected.
  bool AllowPCHWithCompilerErrors;

  /// \brief Dump declarations that are deserialized from PCH, for testing.
  bool DumpDeserializedPCHDecls;

  /// \brief This is a set of names for decls that we do not want to be
  /// deserialized, and we emit an error if they are; for testing purposes.
  std::set<std::string> DeserializedPCHDeclsToErrorOn;

  /// \brief If non-zero, the implicit PCH include is actually a precompiled
  /// preamble that covers this number of bytes in the main source file.
  ///
  /// The boolean indicates whether the preamble ends at the start of a new
  /// line.
  std::pair<unsigned, bool> PrecompiledPreambleBytes;
  
  /// The implicit PTH input included at the start of the translation unit, or
  /// empty.
  std::string ImplicitPTHInclude;

  /// If given, a PTH cache file to use for speeding up header parsing.
  std::string TokenCache;

  /// \brief True if the SourceManager should report the original file name for
  /// contents of files that were remapped to other files. Defaults to true.
  bool RemappedFilesKeepOriginalName;

  /// \brief The set of file remappings, which take existing files on
  /// the system (the first part of each pair) and gives them the
  /// contents of other files on the system (the second part of each
  /// pair).
  std::vector<std::pair<std::string, std::string>> RemappedFiles;

  /// \brief The set of file-to-buffer remappings, which take existing files
  /// on the system (the first part of each pair) and gives them the contents
  /// of the specified memory buffer (the second part of each pair).
  std::vector<std::pair<std::string, llvm::MemoryBuffer *>> RemappedFileBuffers;

  /// \brief Whether the compiler instance should retain (i.e., not free)
  /// the buffers associated with remapped files.
  ///
  /// This flag defaults to false; it can be set true only through direct
  /// manipulation of the compiler invocation object, in cases where the 
  /// compiler invocation and its buffers will be reused.
  bool RetainRemappedFileBuffers;
  
  /// \brief The Objective-C++ ARC standard library that we should support,
  /// by providing appropriate definitions to retrofit the standard library
  /// with support for lifetime-qualified pointers.
  ObjCXXARCStandardLibraryKind ObjCXXARCStandardLibrary;
    
  /// \brief Records the set of modules
  class FailedModulesSet : public RefCountedBase<FailedModulesSet> {
    llvm::StringSet<> Failed;

  public:
    bool hasAlreadyFailed(StringRef mod) {
      return Failed.count(mod) > 0;
    }

    void addFailed(StringRef mod) {
      Failed.insert(mod);
    }
  };
  
  /// \brief The set of modules that failed to build.
  ///
  /// This pointer will be shared among all of the compiler instances created
  /// to (re)build modules, so that once a module fails to build anywhere,
  /// other instances will see that the module has failed and won't try to
  /// build it again.
  IntrusiveRefCntPtr<FailedModulesSet> FailedModules;

public:
  PreprocessorOptions() : UsePredefines(true), DetailedRecord(false),
                          IgnoreLineDirectives(false), // HLSL Change - ignore line directives.
                          DisablePCHValidation(false),
                          AllowPCHWithCompilerErrors(false),
                          DumpDeserializedPCHDecls(false),
                          PrecompiledPreambleBytes(0, true),
                          RemappedFilesKeepOriginalName(true),
                          RetainRemappedFileBuffers(false),
                          ObjCXXARCStandardLibrary(ARCXX_nolib) { }

  void addMacroDef(StringRef Name) { Macros.emplace_back(Name, false); }
  void addMacroUndef(StringRef Name) { Macros.emplace_back(Name, true); }
  void addRemappedFile(StringRef From, StringRef To) {
    RemappedFiles.emplace_back(From, To);
  }

  void addRemappedFile(StringRef From, llvm::MemoryBuffer *To) {
    RemappedFileBuffers.emplace_back(From, To);
  }

  void clearRemappedFiles() {
    RemappedFiles.clear();
    RemappedFileBuffers.clear();
  }
  
  /// \brief Reset any options that are not considered when building a
  /// module.
  void resetNonModularOptions() {
    Includes.clear();
    MacroIncludes.clear();
    ChainedIncludes.clear();
    DumpDeserializedPCHDecls = false;
    ImplicitPCHInclude.clear();
    ImplicitPTHInclude.clear();
    TokenCache.clear();
    RetainRemappedFileBuffers = true;
    PrecompiledPreambleBytes.first = 0;
    PrecompiledPreambleBytes.second = 0;
  }
};

} // end namespace clang

#endif
