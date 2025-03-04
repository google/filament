//===-- FrontendActions.h - Useful Frontend Actions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_STATICANALYZER_FRONTEND_FRONTENDACTIONS_H
#define LLVM_CLANG_STATICANALYZER_FRONTEND_FRONTENDACTIONS_H

#include "clang/Frontend/FrontendAction.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

namespace clang {

class Stmt;

namespace ento {

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

class AnalysisAction : public ASTFrontendAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
};

/// \brief Frontend action to parse model files.
///
/// This frontend action is responsible for parsing model files. Model files can
/// not be parsed on their own, they rely on type information that is available
/// in another translation unit. The parsing of model files is done by a
/// separate compiler instance that reuses the ASTContext and othen information
/// from the main translation unit that is being compiled. After a model file is
/// parsed, the function definitions will be collected into a StringMap.
class ParseModelFileAction : public ASTFrontendAction {
public:
  ParseModelFileAction(llvm::StringMap<Stmt *> &Bodies);
  bool isModelParsingAction() const override { return true; }

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

private:
  llvm::StringMap<Stmt *> &Bodies;
};

void printCheckerHelp(raw_ostream &OS, ArrayRef<std::string> plugins);

} // end GR namespace

} // end namespace clang

#endif
