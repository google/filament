//===--- FrontendAction.cpp -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/LayoutOverrideSource.h"
#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Serialization/ASTDeserializationListener.h"
#include "clang/Serialization/ASTReader.h"
#include "clang/Serialization/GlobalModuleIndex.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>
using namespace clang;

template class llvm::Registry<clang::PluginASTAction>;

#if 0 // HLSL Change Starts - no support for AST serialization

namespace {

class DelegatingDeserializationListener : public ASTDeserializationListener {
  ASTDeserializationListener *Previous;
  bool DeletePrevious;

public:
  explicit DelegatingDeserializationListener(
      ASTDeserializationListener *Previous, bool DeletePrevious)
      : Previous(Previous), DeletePrevious(DeletePrevious) {}
  ~DelegatingDeserializationListener() override {
    if (DeletePrevious)
      delete Previous;
  }

  void ReaderInitialized(ASTReader *Reader) override {
    if (Previous)
      Previous->ReaderInitialized(Reader);
  }
  void IdentifierRead(serialization::IdentID ID,
                      IdentifierInfo *II) override {
    if (Previous)
      Previous->IdentifierRead(ID, II);
  }
  void TypeRead(serialization::TypeIdx Idx, QualType T) override {
    if (Previous)
      Previous->TypeRead(Idx, T);
  }
  void DeclRead(serialization::DeclID ID, const Decl *D) override {
    if (Previous)
      Previous->DeclRead(ID, D);
  }
  void SelectorRead(serialization::SelectorID ID, Selector Sel) override {
    if (Previous)
      Previous->SelectorRead(ID, Sel);
  }
  void MacroDefinitionRead(serialization::PreprocessedEntityID PPID,
                           MacroDefinitionRecord *MD) override {
    if (Previous)
      Previous->MacroDefinitionRead(PPID, MD);
  }
};

/// \brief Dumps deserialized declarations.
class DeserializedDeclsDumper : public DelegatingDeserializationListener {
public:
  explicit DeserializedDeclsDumper(ASTDeserializationListener *Previous,
                                   bool DeletePrevious)
      : DelegatingDeserializationListener(Previous, DeletePrevious) {}

  void DeclRead(serialization::DeclID ID, const Decl *D) override {
    llvm::outs() << "PCH DECL: " << D->getDeclKindName();
    if (const NamedDecl *ND = dyn_cast<NamedDecl>(D))
      llvm::outs() << " - " << *ND;
    llvm::outs() << "\n";

    DelegatingDeserializationListener::DeclRead(ID, D);
  }
};

/// \brief Checks deserialized declarations and emits error if a name
/// matches one given in command-line using -error-on-deserialized-decl.
class DeserializedDeclsChecker : public DelegatingDeserializationListener {
  ASTContext &Ctx;
  std::set<std::string> NamesToCheck;

public:
  DeserializedDeclsChecker(ASTContext &Ctx,
                           const std::set<std::string> &NamesToCheck,
                           ASTDeserializationListener *Previous,
                           bool DeletePrevious)
      : DelegatingDeserializationListener(Previous, DeletePrevious), Ctx(Ctx),
        NamesToCheck(NamesToCheck) {}

  void DeclRead(serialization::DeclID ID, const Decl *D) override {
    if (const NamedDecl *ND = dyn_cast<NamedDecl>(D))
      if (NamesToCheck.find(ND->getNameAsString()) != NamesToCheck.end()) {
        unsigned DiagID
          = Ctx.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
                                                 "%0 was deserialized");
        Ctx.getDiagnostics().Report(Ctx.getFullLoc(D->getLocation()), DiagID)
            << ND->getNameAsString();
      }

    DelegatingDeserializationListener::DeclRead(ID, D);
  }
};

} // end anonymous namespace

#endif // HLSL Change Starts - no support for AST serialization

FrontendAction::FrontendAction() : Instance(nullptr) {}

FrontendAction::~FrontendAction() {}

void FrontendAction::setCurrentInput(const FrontendInputFile &CurrentInput,
                                     std::unique_ptr<ASTUnit> AST) {
  this->CurrentInput = CurrentInput;
  CurrentASTUnit = std::move(AST);
}

std::unique_ptr<ASTConsumer>
FrontendAction::CreateWrappedASTConsumer(CompilerInstance &CI,
                                         StringRef InFile) {
  std::unique_ptr<ASTConsumer> Consumer = CreateASTConsumer(CI, InFile);
  if (!Consumer)
    return nullptr;

  if (CI.getFrontendOpts().AddPluginActions.size() == 0)
    return Consumer;

  // Make sure the non-plugin consumer is first, so that plugins can't
  // modifiy the AST.
  std::vector<std::unique_ptr<ASTConsumer>> Consumers;
  Consumers.push_back(std::move(Consumer));

  for (size_t i = 0, e = CI.getFrontendOpts().AddPluginActions.size();
       i != e; ++i) { 
    // This is O(|plugins| * |add_plugins|), but since both numbers are
    // way below 50 in practice, that's ok.
    for (FrontendPluginRegistry::iterator
        it = FrontendPluginRegistry::begin(),
        ie = FrontendPluginRegistry::end();
        it != ie; ++it) {
      if (it->getName() != CI.getFrontendOpts().AddPluginActions[i])
        continue;
      std::unique_ptr<PluginASTAction> P = it->instantiate();
      if (P->ParseArgs(CI, CI.getFrontendOpts().AddPluginArgs[i]))
        Consumers.push_back(P->CreateASTConsumer(CI, InFile));
    }
  }

  return llvm::make_unique<MultiplexConsumer>(std::move(Consumers));
}

bool FrontendAction::BeginSourceFile(CompilerInstance &CI,
                                     const FrontendInputFile &Input) {
  assert(!Instance && "Already processing a source file!");
  assert(!Input.isEmpty() && "Unexpected empty filename!");
  setCurrentInput(Input);
  setCompilerInstance(&CI);

  StringRef InputFile = Input.getFile();
  bool HasBegunSourceFile = false;
  if (!BeginInvocation(CI))
    goto failure;

  // AST files follow a very different path, since they share objects via the
  // AST unit.
  if (Input.getKind() == IK_AST) {
#if 1 // HLSL Change Starts - no support for AST serialization
    goto failure;
#else
    assert(!usesPreprocessorOnly() &&
           "Attempt to pass AST file to preprocessor only action!");
    assert(hasASTFileSupport() &&
           "This action does not have AST file support!");

    IntrusiveRefCntPtr<DiagnosticsEngine> Diags(&CI.getDiagnostics());

    std::unique_ptr<ASTUnit> AST =
        ASTUnit::LoadFromASTFile(InputFile, CI.getPCHContainerReader(),
                                 Diags, CI.getFileSystemOpts());

    if (!AST)
      goto failure;

    // Inform the diagnostic client we are processing a source file.
    CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), nullptr);
    HasBegunSourceFile = true;

    // Set the shared objects, these are reset when we finish processing the
    // file, otherwise the CompilerInstance will happily destroy them.
    CI.setFileManager(&AST->getFileManager());
    CI.setSourceManager(&AST->getSourceManager());
    CI.setPreprocessor(&AST->getPreprocessor());
    CI.setASTContext(&AST->getASTContext());

    setCurrentInput(Input, std::move(AST));

    // Initialize the action.
    if (!BeginSourceFileAction(CI, InputFile))
      goto failure;

    // Create the AST consumer.
    CI.setASTConsumer(CreateWrappedASTConsumer(CI, InputFile));
    if (!CI.hasASTConsumer())
      goto failure;

    return true;
#endif // HLSL Change Ends - no support for AST serialization
  }

  if (!CI.hasVirtualFileSystem()) {
    if (IntrusiveRefCntPtr<vfs::FileSystem> VFS =
          createVFSFromCompilerInvocation(CI.getInvocation(),
                                          CI.getDiagnostics()))
      CI.setVirtualFileSystem(VFS);
    else
      goto failure;
  }

  // Set up the file and source managers, if needed.
  if (!CI.hasFileManager())
    CI.createFileManager();
  if (!CI.hasSourceManager())
    CI.createSourceManager(CI.getFileManager());

  // IR files bypass the rest of initialization.
  if (Input.getKind() == IK_LLVM_IR) {
    assert(hasIRSupport() &&
           "This action does not have IR file support!");

    // Inform the diagnostic client we are processing a source file.
    CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), nullptr);
    HasBegunSourceFile = true;

    // Initialize the action.
    if (!BeginSourceFileAction(CI, InputFile))
      goto failure;

    // Initialize the main file entry.
    if (!CI.InitializeSourceManager(CurrentInput))
      goto failure;

    return true;
  }

#if 0 // HLSL Change Starts - no support for AST serialization
  // If the implicit PCH include is actually a directory, rather than
  // a single file, search for a suitable PCH file in that directory.
  if (!CI.getPreprocessorOpts().ImplicitPCHInclude.empty()) {
    FileManager &FileMgr = CI.getFileManager();
    PreprocessorOptions &PPOpts = CI.getPreprocessorOpts();
    StringRef PCHInclude = PPOpts.ImplicitPCHInclude;
    std::string SpecificModuleCachePath = CI.getSpecificModuleCachePath();
    if (const DirectoryEntry *PCHDir = FileMgr.getDirectory(PCHInclude)) {
      std::error_code EC;
      SmallString<128> DirNative;
      llvm::sys::path::native(PCHDir->getName(), DirNative);
      bool Found = false;
      for (llvm::sys::fs::directory_iterator Dir(DirNative, EC), DirEnd;
           Dir != DirEnd && !EC; Dir.increment(EC)) {
        // Check whether this is an acceptable AST file.
        if (ASTReader::isAcceptableASTFile(
                Dir->path(), FileMgr, CI.getPCHContainerReader(),
                CI.getLangOpts(), CI.getTargetOpts(), CI.getPreprocessorOpts(),
                SpecificModuleCachePath)) {
          PPOpts.ImplicitPCHInclude = Dir->path();
          Found = true;
          break;
        }
      }

      if (!Found) {
        CI.getDiagnostics().Report(diag::err_fe_no_pch_in_dir) << PCHInclude;
        return true;
      }
    }
  }
#endif // HLSL Change Ends - no support for AST serialization

  // Set up the preprocessor if needed. When parsing model files the
  // preprocessor of the original source is reused.
  if (!isModelParsingAction())
    CI.createPreprocessor(getTranslationUnitKind());

  // Inform the diagnostic client we are processing a source file.
  CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(),
                                           &CI.getPreprocessor());
  HasBegunSourceFile = true;

  // Initialize the action.
  if (!BeginSourceFileAction(CI, InputFile))
    goto failure;

  // Initialize the main file entry. It is important that this occurs after
  // BeginSourceFileAction, which may change CurrentInput during module builds.
  if (!CI.InitializeSourceManager(CurrentInput))
    goto failure;

  // Create the AST context and consumer unless this is a preprocessor only
  // action.
  if (!usesPreprocessorOnly()) {
    // Parsing a model file should reuse the existing ASTContext.
    if (!isModelParsingAction())
      CI.createASTContext();

    std::unique_ptr<ASTConsumer> Consumer =
        CreateWrappedASTConsumer(CI, InputFile);
    if (!Consumer)
      goto failure;

    // FIXME: should not overwrite ASTMutationListener when parsing model files?
    if (!isModelParsingAction())
      CI.getASTContext().setASTMutationListener(Consumer->GetASTMutationListener());

#if 0 // HLSL Change Starts - no support for AST serialization
    if (!CI.getPreprocessorOpts().ChainedIncludes.empty()) {
      // Convert headers to PCH and chain them.
      IntrusiveRefCntPtr<ExternalSemaSource> source, FinalReader;
      source = createChainedIncludesSource(CI, FinalReader);
      if (!source)
        goto failure;
      CI.setModuleManager(static_cast<ASTReader *>(FinalReader.get()));
      CI.getASTContext().setExternalSource(source);
    } else if (!CI.getPreprocessorOpts().ImplicitPCHInclude.empty()) {
      // Use PCH.
      assert(hasPCHSupport() && "This action does not have PCH support!");
      ASTDeserializationListener *DeserialListener =
          Consumer->GetASTDeserializationListener();
      bool DeleteDeserialListener = false;
      if (CI.getPreprocessorOpts().DumpDeserializedPCHDecls) {
        DeserialListener = new DeserializedDeclsDumper(DeserialListener,
                                                       DeleteDeserialListener);
        DeleteDeserialListener = true;
      }
      if (!CI.getPreprocessorOpts().DeserializedPCHDeclsToErrorOn.empty()) {
        DeserialListener = new DeserializedDeclsChecker(
            CI.getASTContext(),
            CI.getPreprocessorOpts().DeserializedPCHDeclsToErrorOn,
            DeserialListener, DeleteDeserialListener);
        DeleteDeserialListener = true;
      }
      CI.createPCHExternalASTSource(
          CI.getPreprocessorOpts().ImplicitPCHInclude,
          CI.getPreprocessorOpts().DisablePCHValidation,
          CI.getPreprocessorOpts().AllowPCHWithCompilerErrors, DeserialListener,
          DeleteDeserialListener);
      if (!CI.getASTContext().getExternalSource())
        goto failure;
    }
#endif // HLSL Change Ends - no support for AST serialization

    CI.setASTConsumer(std::move(Consumer));
    if (!CI.hasASTConsumer())
      goto failure;
  }

  // Initialize built-in info as long as we aren't using an external AST
  // source.
  if (!CI.hasASTContext() || !CI.getASTContext().getExternalSource()) {
    Preprocessor &PP = CI.getPreprocessor();

    // If modules are enabled, create the module manager before creating
    // any builtins, so that all declarations know that they might be
    // extended by an external source.
    if (CI.getLangOpts().Modules)
      CI.createModuleManager();

    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                           PP.getLangOpts());
  } else {
    // FIXME: If this is a problem, recover from it by creating a multiplex
    // source.
#if 0 // HLSL Change Starts - no support for AST serialization
    assert((!CI.getLangOpts().Modules || CI.getModuleManager()) &&
           "modules enabled but created an external source that "
           "doesn't support modules");
#endif // HLSL Change End - no support for AST serialization
  }

  // If we were asked to load any module map files, do so now.
  for (const auto &Filename : CI.getFrontendOpts().ModuleMapFiles) {
    if (auto *File = CI.getFileManager().getFile(Filename))
      CI.getPreprocessor().getHeaderSearchInfo().loadModuleMapFile(
          File, /*IsSystem*/false);
    else
      CI.getDiagnostics().Report(diag::err_module_map_not_found) << Filename;
  }

  // If we were asked to load any module files, do so now.
  for (const auto &ModuleFile : CI.getFrontendOpts().ModuleFiles)
    if (!CI.loadModuleFile(ModuleFile))
      goto failure;

#if 0 // HLSL Change Starts - removes support for overriding layout source from external file specification
  // If there is a layout overrides file, attach an external AST source that
  // provides the layouts from that file.
  if (!CI.getFrontendOpts().OverrideRecordLayoutsFile.empty() && 
      CI.hasASTContext() && !CI.getASTContext().getExternalSource()) {
    IntrusiveRefCntPtr<ExternalASTSource> 
      Override(new LayoutOverrideSource(
                     CI.getFrontendOpts().OverrideRecordLayoutsFile));
    CI.getASTContext().setExternalSource(Override);
  }
#endif

  return true;

  // If we failed, reset state since the client will not end up calling the
  // matching EndSourceFile().
  failure:
  if (isCurrentFileAST()) {
    CI.setASTContext(nullptr);
    CI.setPreprocessor(nullptr);
    CI.setSourceManager(nullptr);
    CI.setFileManager(nullptr);
  }

  if (HasBegunSourceFile)
    CI.getDiagnosticClient().EndSourceFile();
  CI.clearOutputFiles(/*EraseFiles=*/true);
  setCurrentInput(FrontendInputFile());
  setCompilerInstance(nullptr);
  return false;
}

bool FrontendAction::Execute() {
  CompilerInstance &CI = getCompilerInstance();

  if (CI.hasFrontendTimer()) {
    llvm::TimeRegion Timer(CI.getFrontendTimer());
    ExecuteAction();
  }
  else ExecuteAction();

#if 0 // HLSL Change Starts - no support for AST serialization
  // If we are supposed to rebuild the global module index, do so now unless
  // there were any module-build failures.
  if (CI.shouldBuildGlobalModuleIndex() && CI.hasFileManager() &&
      CI.hasPreprocessor()) {
    GlobalModuleIndex::writeIndex(
        CI.getFileManager(), CI.getPCHContainerReader(),
        CI.getPreprocessor().getHeaderSearchInfo().getModuleCachePath());
  }
#endif // HLSL Change Ends - no support for AST serialization

  return true;
}

void FrontendAction::EndSourceFile() {
  CompilerInstance &CI = getCompilerInstance();

  // Inform the diagnostic client we are done with this source file.
  CI.getDiagnosticClient().EndSourceFile();

  // Inform the preprocessor we are done.
  if (CI.hasPreprocessor())
    CI.getPreprocessor().EndSourceFile();

  // Finalize the action.
  EndSourceFileAction();

  // Sema references the ast consumer, so reset sema first.
  //
  // FIXME: There is more per-file stuff we could just drop here?
  bool DisableFree = CI.getFrontendOpts().DisableFree;
  if (DisableFree) {
    CI.resetAndLeakSema();
    CI.resetAndLeakASTContext();
    BuryPointer(CI.takeASTConsumer().get());
  } else {
    CI.setSema(nullptr);
    CI.setASTContext(nullptr);
    CI.setASTConsumer(nullptr);
  }

  if (CI.getFrontendOpts().ShowStats) {
    llvm::errs() << "\nSTATISTICS FOR '" << getCurrentFile() << "':\n";
    CI.getPreprocessor().PrintStats();
    CI.getPreprocessor().getIdentifierTable().PrintStats();
    CI.getPreprocessor().getHeaderSearchInfo().PrintStats();
    CI.getSourceManager().PrintStats();
    llvm::errs() << "\n";
  }

  // Cleanup the output streams, and erase the output files if instructed by the
  // FrontendAction.
  CI.clearOutputFiles(/*EraseFiles=*/shouldEraseOutputFiles());

  if (isCurrentFileAST()) {
    if (DisableFree) {
      CI.resetAndLeakPreprocessor();
      CI.resetAndLeakSourceManager();
      CI.resetAndLeakFileManager();
    } else {
      CI.setPreprocessor(nullptr);
      CI.setSourceManager(nullptr);
      CI.setFileManager(nullptr);
    }
  }

  setCompilerInstance(nullptr);
  setCurrentInput(FrontendInputFile());
}

bool FrontendAction::shouldEraseOutputFiles() {
  return getCompilerInstance().getDiagnostics().hasErrorOccurred();
}

//===----------------------------------------------------------------------===//
// Utility Actions
//===----------------------------------------------------------------------===//

void ASTFrontendAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  if (!CI.hasPreprocessor())
    return;

  // FIXME: Move the truncation aspect of this into Sema, we delayed this till
  // here so the source manager would be initialized.
  if (hasCodeCompletionSupport() &&
      !CI.getFrontendOpts().CodeCompletionAt.FileName.empty())
    CI.createCodeCompletionConsumer();

  // Use a code completion consumer?
  CodeCompleteConsumer *CompletionConsumer = nullptr;
  if (CI.hasCodeCompletionConsumer())
    CompletionConsumer = &CI.getCodeCompletionConsumer();

  if (!CI.hasSema())
    CI.createSema(getTranslationUnitKind(), CompletionConsumer);

  ParseAST(CI.getSema(), CI.getFrontendOpts().ShowStats,
           CI.getFrontendOpts().SkipFunctionBodies);
}

void PluginASTAction::anchor() { }

std::unique_ptr<ASTConsumer>
PreprocessorFrontendAction::CreateASTConsumer(CompilerInstance &CI,
                                              StringRef InFile) {
  llvm_unreachable("Invalid CreateASTConsumer on preprocessor action!");
}

std::unique_ptr<ASTConsumer>
WrapperFrontendAction::CreateASTConsumer(CompilerInstance &CI,
                                         StringRef InFile) {
  return WrappedAction->CreateASTConsumer(CI, InFile);
}
bool WrapperFrontendAction::BeginInvocation(CompilerInstance &CI) {
  return WrappedAction->BeginInvocation(CI);
}
bool WrapperFrontendAction::BeginSourceFileAction(CompilerInstance &CI,
                                                  StringRef Filename) {
  WrappedAction->setCurrentInput(getCurrentInput());
  WrappedAction->setCompilerInstance(&CI);
  return WrappedAction->BeginSourceFileAction(CI, Filename);
}
void WrapperFrontendAction::ExecuteAction() {
  WrappedAction->ExecuteAction();
}
void WrapperFrontendAction::EndSourceFileAction() {
  WrappedAction->EndSourceFileAction();
}

bool WrapperFrontendAction::usesPreprocessorOnly() const {
  return WrappedAction->usesPreprocessorOnly();
}
TranslationUnitKind WrapperFrontendAction::getTranslationUnitKind() {
  return WrappedAction->getTranslationUnitKind();
}
bool WrapperFrontendAction::hasPCHSupport() const {
  return WrappedAction->hasPCHSupport();
}
bool WrapperFrontendAction::hasASTFileSupport() const {
  return WrappedAction->hasASTFileSupport();
}
bool WrapperFrontendAction::hasIRSupport() const {
  return WrappedAction->hasIRSupport();
}
bool WrapperFrontendAction::hasCodeCompletionSupport() const {
  return WrappedAction->hasCodeCompletionSupport();
}

WrapperFrontendAction::WrapperFrontendAction(FrontendAction *WrappedAction)
  : WrappedAction(WrappedAction) {}

