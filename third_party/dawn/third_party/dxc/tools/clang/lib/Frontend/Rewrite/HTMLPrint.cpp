//===--- HTMLPrint.cpp - Source code -> HTML pretty-printing --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Pretty-printing of source code to HTML.
//
//===----------------------------------------------------------------------===//

#include "clang/Rewrite/Frontend/ASTConsumers.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Rewrite/Core/HTMLRewrite.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

//===----------------------------------------------------------------------===//
// Functional HTML pretty-printing.
//===----------------------------------------------------------------------===//

namespace {
  class HTMLPrinter : public ASTConsumer {
    Rewriter R;
    raw_ostream *Out;
    Preprocessor &PP;
    bool SyntaxHighlight, HighlightMacros;

  public:
    HTMLPrinter(raw_ostream *OS, Preprocessor &pp,
                bool _SyntaxHighlight, bool _HighlightMacros)
      : Out(OS), PP(pp), SyntaxHighlight(_SyntaxHighlight),
        HighlightMacros(_HighlightMacros) {}

    void Initialize(ASTContext &context) override;
    void HandleTranslationUnit(ASTContext &Ctx) override;
  };
}

std::unique_ptr<ASTConsumer> clang::CreateHTMLPrinter(raw_ostream *OS,
                                                      Preprocessor &PP,
                                                      bool SyntaxHighlight,
                                                      bool HighlightMacros) {
  return llvm::make_unique<HTMLPrinter>(OS, PP, SyntaxHighlight,
                                        HighlightMacros);
}

void HTMLPrinter::Initialize(ASTContext &context) {
  R.setSourceMgr(context.getSourceManager(), context.getLangOpts());
}

void HTMLPrinter::HandleTranslationUnit(ASTContext &Ctx) {
  if (PP.getDiagnostics().hasErrorOccurred())
    return;

  // Format the file.
  FileID FID = R.getSourceMgr().getMainFileID();
  const FileEntry* Entry = R.getSourceMgr().getFileEntryForID(FID);
  const char* Name;
  // In some cases, in particular the case where the input is from stdin,
  // there is no entry.  Fall back to the memory buffer for a name in those
  // cases.
  if (Entry)
    Name = Entry->getName();
  else
    Name = R.getSourceMgr().getBuffer(FID)->getBufferIdentifier();

  html::AddLineNumbers(R, FID);
  html::AddHeaderFooterInternalBuiltinCSS(R, FID, Name);

  // If we have a preprocessor, relex the file and syntax highlight.
  // We might not have a preprocessor if we come from a deserialized AST file,
  // for example.

  if (SyntaxHighlight) html::SyntaxHighlight(R, FID, PP);
  if (HighlightMacros) html::HighlightMacros(R, FID, PP);
  html::EscapeText(R, FID, false, true);

  // Emit the HTML.
  const RewriteBuffer &RewriteBuf = R.getEditBuffer(FID);
  char *Buffer = new char[RewriteBuf.size()]; // HLSL Change: Use overridable operator new
  std::copy(RewriteBuf.begin(), RewriteBuf.end(), Buffer);
  Out->write(Buffer, RewriteBuf.size());
  delete[] Buffer; // HLSL Change: Use overridable operator delete
}
