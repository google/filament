//===--- SimpleFormatContext.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
///
/// \brief Defines a utility class for use of clang-format in libclang
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_INDEX_SIMPLEFORMATCONTEXT_H
#define LLVM_CLANG_LIB_INDEX_SIMPLEFORMATCONTEXT_H

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

namespace clang {
namespace index {

/// \brief A small class to be used by libclang clients to format
/// a declaration string in memory. This object is instantiated once
/// and used each time a formatting is needed.
class SimpleFormatContext {
public:
  SimpleFormatContext(LangOptions Options)
      : DiagOpts(new DiagnosticOptions()),
        Diagnostics(new DiagnosticsEngine(new DiagnosticIDs,
                                          DiagOpts.get())),
        Files((FileSystemOptions())),
        Sources(*Diagnostics, Files),
        Rewrite(Sources, Options) {
    Diagnostics->setClient(new IgnoringDiagConsumer, true);
  }

  FileID createInMemoryFile(StringRef Name, StringRef Content) {
    std::unique_ptr<llvm::MemoryBuffer> Source =
        llvm::MemoryBuffer::getMemBuffer(Content);
    const FileEntry *Entry =
        Files.getVirtualFile(Name, Source->getBufferSize(), 0);
    Sources.overrideFileContents(Entry, std::move(Source));
    assert(Entry != nullptr);
    return Sources.createFileID(Entry, SourceLocation(), SrcMgr::C_User);
  }

  std::string getRewrittenText(FileID ID) {
    std::string Result;
    llvm::raw_string_ostream OS(Result);
    Rewrite.getEditBuffer(ID).write(OS);
    OS.flush();
    return Result;
  }

  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;
  IntrusiveRefCntPtr<DiagnosticsEngine> Diagnostics;
  FileManager Files;
  SourceManager Sources;
  Rewriter Rewrite;
};

} // end namespace index
} // end namespace clang

#endif
