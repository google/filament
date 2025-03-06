//===--- FrontendActions.cpp ----------------------------------------------===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// FrontendActions_rewrite.cpp                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/Parser.h"
#include "clang/Rewrite/Frontend/ASTConsumers.h"
#include "clang/Rewrite/Frontend/FixItRewriter.h"
#include "clang/Rewrite/Frontend/FrontendActions.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace clang;

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//===----------------------------------------------------------------------===//

std::unique_ptr<ASTConsumer>
HTMLPrintAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  if (raw_ostream *OS = CI.createDefaultOutputFile(false, InFile))
    return CreateHTMLPrinter(OS, CI.getPreprocessor());
  return nullptr;
}

FixItAction::FixItAction() {}
FixItAction::~FixItAction() {}

std::unique_ptr<ASTConsumer>
FixItAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  return llvm::make_unique<ASTConsumer>();
}

namespace {
class FixItRewriteInPlace : public FixItOptions {
public:
  FixItRewriteInPlace() { InPlace = true; }

  std::string RewriteFilename(const std::string &Filename, int &fd) override {
    llvm_unreachable("don't call RewriteFilename for inplace rewrites");
  }
};

class FixItActionSuffixInserter : public FixItOptions {
  std::string NewSuffix;

public:
  FixItActionSuffixInserter(std::string NewSuffix, bool FixWhatYouCan)
      : NewSuffix(NewSuffix) {
    this->FixWhatYouCan = FixWhatYouCan;
  }

  std::string RewriteFilename(const std::string &Filename, int &fd) override {
    fd = -1;
    SmallString<128> Path(Filename);
    llvm::sys::path::replace_extension(
        Path, NewSuffix + llvm::sys::path::extension(Path));
    return Path.str();
  }
};

class FixItRewriteToTemp : public FixItOptions {
public:
  std::string RewriteFilename(const std::string &Filename, int &fd) override {
    SmallString<128> Path;
    llvm::sys::fs::createTemporaryFile(llvm::sys::path::filename(Filename),
                                       llvm::sys::path::extension(Filename), fd,
                                       Path);
    return Path.str();
  }
};
} // end anonymous namespace

bool FixItAction::BeginSourceFileAction(CompilerInstance &CI,
                                        StringRef Filename) {
  const FrontendOptions &FEOpts = getCompilerInstance().getFrontendOpts();
  if (!FEOpts.FixItSuffix.empty()) {
    FixItOpts.reset(new FixItActionSuffixInserter(FEOpts.FixItSuffix,
                                                  FEOpts.FixWhatYouCan));
  } else {
    FixItOpts.reset(new FixItRewriteInPlace);
    FixItOpts->FixWhatYouCan = FEOpts.FixWhatYouCan;
  }
  Rewriter.reset(new FixItRewriter(CI.getDiagnostics(), CI.getSourceManager(),
                                   CI.getLangOpts(), FixItOpts.get()));
  return true;
}

void FixItAction::EndSourceFileAction() {
  // Otherwise rewrite all files.
  Rewriter->WriteFixedFiles();
}

bool FixItRecompile::BeginInvocation(CompilerInstance &CI) {

  std::vector<std::pair<std::string, std::string>> RewrittenFiles;
  bool err = false;
  {
    const FrontendOptions &FEOpts = CI.getFrontendOpts();
    std::unique_ptr<FrontendAction> FixAction(new SyntaxOnlyAction());
    if (FixAction->BeginSourceFile(CI, FEOpts.Inputs[0])) {
      std::unique_ptr<FixItOptions> FixItOpts;
      if (FEOpts.FixToTemporaries)
        FixItOpts.reset(new FixItRewriteToTemp());
      else
        FixItOpts.reset(new FixItRewriteInPlace());
      FixItOpts->Silent = true;
      FixItOpts->FixWhatYouCan = FEOpts.FixWhatYouCan;
      FixItOpts->FixOnlyWarnings = FEOpts.FixOnlyWarnings;
      FixItRewriter Rewriter(CI.getDiagnostics(), CI.getSourceManager(),
                             CI.getLangOpts(), FixItOpts.get());
      FixAction->Execute();

      err = Rewriter.WriteFixedFiles(&RewrittenFiles);

      FixAction->EndSourceFile();
      CI.setSourceManager(nullptr);
      CI.setFileManager(nullptr);
    } else {
      err = true;
    }
  }
  if (err)
    return false;
  CI.getDiagnosticClient().clear();
  CI.getDiagnostics().Reset();

  PreprocessorOptions &PPOpts = CI.getPreprocessorOpts();
  PPOpts.RemappedFiles.insert(PPOpts.RemappedFiles.end(),
                              RewrittenFiles.begin(), RewrittenFiles.end());
  PPOpts.RemappedFilesKeepOriginalName = false;

  return true;
}

#ifdef CLANG_ENABLE_OBJC_REWRITER

std::unique_ptr<ASTConsumer>
RewriteObjCAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  if (raw_ostream *OS = CI.createDefaultOutputFile(false, InFile, "cpp")) {
    if (CI.getLangOpts().ObjCRuntime.isNonFragile())
      return CreateModernObjCRewriter(
          InFile, OS, CI.getDiagnostics(), CI.getLangOpts(),
          CI.getDiagnosticOpts().NoRewriteMacros,
          (CI.getCodeGenOpts().getDebugInfo() != CodeGenOptions::NoDebugInfo));
    return CreateObjCRewriter(InFile, OS, CI.getDiagnostics(), CI.getLangOpts(),
                              CI.getDiagnosticOpts().NoRewriteMacros);
  }
  return nullptr;
}

#endif

//===----------------------------------------------------------------------===//
// Preprocessor Actions
//===----------------------------------------------------------------------===//

void RewriteMacrosAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  raw_ostream *OS = CI.createDefaultOutputFile(true, getCurrentFile());
  if (!OS)
    return;

  RewriteMacrosInInput(CI.getPreprocessor(), OS);
}

void RewriteTestAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  raw_ostream *OS = CI.createDefaultOutputFile(false, getCurrentFile());
  if (!OS)
    return;

  DoRewriteTest(CI.getPreprocessor(), OS);
}

void RewriteIncludesAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  raw_ostream *OS = CI.createDefaultOutputFile(true, getCurrentFile());
  if (!OS)
    return;

  RewriteIncludesInInput(CI.getPreprocessor(), OS,
                         CI.getPreprocessorOutputOpts());
}
