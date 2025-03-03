//===-- FrontendAction.h - Generic Frontend Action Interface ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the clang::FrontendAction interface and various convenience
/// abstract classes (clang::ASTFrontendAction, clang::PluginASTAction,
/// clang::PreprocessorFrontendAction, and clang::WrapperFrontendAction)
/// derived from it.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_FRONTENDACTION_H
#define LLVM_CLANG_FRONTEND_FRONTENDACTION_H

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/FrontendOptions.h"
#include "llvm/ADT/StringRef.h"
#include <memory>
#include <string>
#include <vector>

namespace clang {
class ASTMergeAction;
class CompilerInstance;

/// Abstract base class for actions which can be performed by the frontend.
class FrontendAction {
  FrontendInputFile CurrentInput;
  std::unique_ptr<ASTUnit> CurrentASTUnit;
  CompilerInstance *Instance;
  friend class ASTMergeAction;
  friend class WrapperFrontendAction;

private:
  std::unique_ptr<ASTConsumer> CreateWrappedASTConsumer(CompilerInstance &CI,
                                                        StringRef InFile);

protected:
  /// @name Implementation Action Interface
  /// @{

  /// \brief Create the AST consumer object for this action, if supported.
  ///
  /// This routine is called as part of BeginSourceFile(), which will
  /// fail if the AST consumer cannot be created. This will not be called if the
  /// action has indicated that it only uses the preprocessor.
  ///
  /// \param CI - The current compiler instance, provided as a convenience, see
  /// getCompilerInstance().
  ///
  /// \param InFile - The current input file, provided as a convenience, see
  /// getCurrentFile().
  ///
  /// \return The new AST consumer, or null on failure.
  virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                         StringRef InFile) = 0;

  /// \brief Callback before starting processing a single input, giving the
  /// opportunity to modify the CompilerInvocation or do some other action
  /// before BeginSourceFileAction is called.
  ///
  /// \return True on success; on failure BeginSourceFileAction(),
  /// ExecuteAction() and EndSourceFileAction() will not be called.
  virtual bool BeginInvocation(CompilerInstance &CI) { return true; }

  /// \brief Callback at the start of processing a single input.
  ///
  /// \return True on success; on failure ExecutionAction() and
  /// EndSourceFileAction() will not be called.
  virtual bool BeginSourceFileAction(CompilerInstance &CI,
                                     StringRef Filename) {
    return true;
  }

  /// \brief Callback to run the program action, using the initialized
  /// compiler instance.
  ///
  /// This is guaranteed to only be called between BeginSourceFileAction()
  /// and EndSourceFileAction().
  virtual void ExecuteAction() = 0;

  /// \brief Callback at the end of processing a single input.
  ///
  /// This is guaranteed to only be called following a successful call to
  /// BeginSourceFileAction (and BeginSourceFile).
  virtual void EndSourceFileAction() {}

  /// \brief Callback at the end of processing a single input, to determine
  /// if the output files should be erased or not.
  ///
  /// By default it returns true if a compiler error occurred.
  /// This is guaranteed to only be called following a successful call to
  /// BeginSourceFileAction (and BeginSourceFile).
  virtual bool shouldEraseOutputFiles();

  /// @}

public:
  FrontendAction();
  virtual ~FrontendAction();

  /// @name Compiler Instance Access
  /// @{

  CompilerInstance &getCompilerInstance() const {
    assert(Instance && "Compiler instance not registered!");
    return *Instance;
  }

  void setCompilerInstance(CompilerInstance *Value) { Instance = Value; }

  /// @}
  /// @name Current File Information
  /// @{

  bool isCurrentFileAST() const {
    assert(!CurrentInput.isEmpty() && "No current file!");
    return (bool)CurrentASTUnit;
  }

  const FrontendInputFile &getCurrentInput() const {
    return CurrentInput;
  }
  
  const StringRef getCurrentFile() const {
    assert(!CurrentInput.isEmpty() && "No current file!");
    return CurrentInput.getFile();
  }

  InputKind getCurrentFileKind() const {
    assert(!CurrentInput.isEmpty() && "No current file!");
    return CurrentInput.getKind();
  }

  ASTUnit &getCurrentASTUnit() const {
    assert(CurrentASTUnit && "No current AST unit!");
    return *CurrentASTUnit;
  }

  std::unique_ptr<ASTUnit> takeCurrentASTUnit() {
    return std::move(CurrentASTUnit);
  }

  void setCurrentInput(const FrontendInputFile &CurrentInput,
                       std::unique_ptr<ASTUnit> AST = nullptr);

  /// @}
  /// @name Supported Modes
  /// @{

  /// \brief Is this action invoked on a model file? 
  ///
  /// Model files are incomplete translation units that relies on type
  /// information from another translation unit. Check ParseModelFileAction for
  /// details.
  virtual bool isModelParsingAction() const { return false; }

  /// \brief Does this action only use the preprocessor?
  ///
  /// If so no AST context will be created and this action will be invalid
  /// with AST file inputs.
  virtual bool usesPreprocessorOnly() const = 0;

  /// \brief For AST-based actions, the kind of translation unit we're handling.
  virtual TranslationUnitKind getTranslationUnitKind() { return TU_Complete; }

  /// \brief Does this action support use with PCH?
  virtual bool hasPCHSupport() const { return !usesPreprocessorOnly(); }

  /// \brief Does this action support use with AST files?
  virtual bool hasASTFileSupport() const { return !usesPreprocessorOnly(); }

  /// \brief Does this action support use with IR files?
  virtual bool hasIRSupport() const { return false; }

  /// \brief Does this action support use with code completion?
  virtual bool hasCodeCompletionSupport() const { return false; }

  /// @}
  /// @name Public Action Interface
  /// @{

  /// \brief Prepare the action for processing the input file \p Input.
  ///
  /// This is run after the options and frontend have been initialized,
  /// but prior to executing any per-file processing.
  ///
  /// \param CI - The compiler instance this action is being run from. The
  /// action may store and use this object up until the matching EndSourceFile
  /// action.
  ///
  /// \param Input - The input filename and kind. Some input kinds are handled
  /// specially, for example AST inputs, since the AST file itself contains
  /// several objects which would normally be owned by the
  /// CompilerInstance. When processing AST input files, these objects should
  /// generally not be initialized in the CompilerInstance -- they will
  /// automatically be shared with the AST file in between
  /// BeginSourceFile() and EndSourceFile().
  ///
  /// \return True on success; on failure the compilation of this file should
  /// be aborted and neither Execute() nor EndSourceFile() should be called.
  bool BeginSourceFile(CompilerInstance &CI, const FrontendInputFile &Input);

  /// \brief Set the source manager's main input file, and run the action.
  bool Execute();

  /// \brief Perform any per-file post processing, deallocate per-file
  /// objects, and run statistics and output file cleanup code.
  void EndSourceFile();

  /// @}
};

/// \brief Abstract base class to use for AST consumer-based frontend actions.
class ASTFrontendAction : public FrontendAction {
protected:
  /// \brief Implement the ExecuteAction interface by running Sema on
  /// the already-initialized AST consumer.
  ///
  /// This will also take care of instantiating a code completion consumer if
  /// the user requested it and the action supports it.
  void ExecuteAction() override;

public:
  ASTFrontendAction() {}
  bool usesPreprocessorOnly() const override { return false; }
};

class PluginASTAction : public ASTFrontendAction {
  virtual void anchor();
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override = 0;

  /// \brief Parse the given plugin command line arguments.
  ///
  /// \param CI - The compiler instance, for use in reporting diagnostics.
  /// \return True if the parsing succeeded; otherwise the plugin will be
  /// destroyed and no action run. The plugin is responsible for using the
  /// CompilerInstance's Diagnostic object to report errors.
  virtual bool ParseArgs(const CompilerInstance &CI,
                         const std::vector<std::string> &arg) = 0;
};

/// \brief Abstract base class to use for preprocessor-based frontend actions.
class PreprocessorFrontendAction : public FrontendAction {
protected:
  /// \brief Provide a default implementation which returns aborts;
  /// this method should never be called by FrontendAction clients.
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

public:
  bool usesPreprocessorOnly() const override { return true; }
};

/// \brief A frontend action which simply wraps some other runtime-specified
/// frontend action.
///
/// Deriving from this class allows an action to inject custom logic around
/// some existing action's behavior. It implements every virtual method in
/// the FrontendAction interface by forwarding to the wrapped action.
class WrapperFrontendAction : public FrontendAction {
  std::unique_ptr<FrontendAction> WrappedAction;

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
  bool BeginInvocation(CompilerInstance &CI) override;
  bool BeginSourceFileAction(CompilerInstance &CI, StringRef Filename) override;
  void ExecuteAction() override;
  void EndSourceFileAction() override;

public:
  /// Construct a WrapperFrontendAction from an existing action, taking
  /// ownership of it.
  WrapperFrontendAction(FrontendAction *WrappedAction);

  bool usesPreprocessorOnly() const override;
  TranslationUnitKind getTranslationUnitKind() override;
  bool hasPCHSupport() const override;
  bool hasASTFileSupport() const override;
  bool hasIRSupport() const override;
  bool hasCodeCompletionSupport() const override;
};

}  // end namespace clang

#endif
