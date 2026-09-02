//===--- ModuleBuilder.cpp - Emit LLVM Code from ASTs ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This builds an AST and converts it to LLVM Code.
//
//===----------------------------------------------------------------------===//

#include "clang/CodeGen/ModuleBuilder.h"
#include "CGDebugInfo.h"
#include "CodeGenModule.h"
#include "dxc/DXIL/DxilMetadataHelper.h"         // HLSL Change - dx source info
#include "dxc/DxcBindingTable/DxcBindingTable.h" // HLSL Change
#include "dxc/Support/Path.h"                    // HLSL Change
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Path.h"
#include <memory>
using namespace clang;

namespace {
  class CodeGeneratorImpl : public CodeGenerator {
    DiagnosticsEngine &Diags;
    std::unique_ptr<const llvm::DataLayout> TD;
    ASTContext *Ctx;
    const HeaderSearchOptions &HeaderSearchOpts; // Only used for debug info.
    const PreprocessorOptions &PreprocessorOpts; // Only used for debug info.
    const CodeGenOptions CodeGenOpts;  // Intentionally copied in.

    unsigned HandlingTopLevelDecls;
    struct HandlingTopLevelDeclRAII {
      CodeGeneratorImpl &Self;
      HandlingTopLevelDeclRAII(CodeGeneratorImpl &Self) : Self(Self) {
        ++Self.HandlingTopLevelDecls;
      }
      ~HandlingTopLevelDeclRAII() {
        if (--Self.HandlingTopLevelDecls == 0)
          Self.EmitDeferredDecls();
      }
    };

    CoverageSourceInfo *CoverageInfo;

  protected:
    std::unique_ptr<llvm::Module> M;
    std::unique_ptr<CodeGen::CodeGenModule> Builder;

  private:
    SmallVector<CXXMethodDecl *, 8> DeferredInlineMethodDefinitions;

  public:
    CodeGeneratorImpl(DiagnosticsEngine &diags, const std::string &ModuleName,
                      const HeaderSearchOptions &HSO,
                      const PreprocessorOptions &PPO, const CodeGenOptions &CGO,
                      llvm::LLVMContext &C,
                      CoverageSourceInfo *CoverageInfo = nullptr)
        : Diags(diags), Ctx(nullptr), HeaderSearchOpts(HSO),
          PreprocessorOpts(PPO), CodeGenOpts(CGO), HandlingTopLevelDecls(0),
          CoverageInfo(CoverageInfo),
          M(new llvm::Module(ModuleName, C)) {}

    ~CodeGeneratorImpl() override {
      // There should normally not be any leftover inline method definitions.
      assert(DeferredInlineMethodDefinitions.empty() ||
             Diags.hasErrorOccurred());
    }

    llvm::Module* GetModule() override {
      return M.get();
    }

    const Decl *GetDeclForMangledName(StringRef MangledName) override {
      GlobalDecl Result;
      if (!Builder->lookupRepresentativeDecl(MangledName, Result))
        return nullptr;
      const Decl *D = Result.getCanonicalDecl().getDecl();
      if (auto FD = dyn_cast<FunctionDecl>(D)) {
        if (FD->hasBody(FD))
          return FD;
      } else if (auto TD = dyn_cast<TagDecl>(D)) {
        if (auto Def = TD->getDefinition())
          return Def;
      }
      return D;
    }

    llvm::Module *ReleaseModule() override { return M.release(); }

    void Initialize(ASTContext &Context) override {
      Ctx = &Context;

      M->setTargetTriple(Ctx->getTargetInfo().getTriple().getTriple());
      M->setDataLayout(Ctx->getTargetInfo().getTargetDescription());
      TD.reset(
          new llvm::DataLayout(Ctx->getTargetInfo().getTargetDescription()));
      Builder.reset(new CodeGen::CodeGenModule(Context,
                                               HeaderSearchOpts,
                                               PreprocessorOpts,
                                               CodeGenOpts, *M, *TD,
                                               Diags, CoverageInfo));

      for (size_t i = 0, e = CodeGenOpts.DependentLibraries.size(); i < e; ++i)
        HandleDependentLibrary(CodeGenOpts.DependentLibraries[i]);
    }

    void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override {
      if (Diags.hasErrorOccurred())
        return;

      Builder->HandleCXXStaticMemberVarInstantiation(VD);
    }

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
      if (Diags.hasErrorOccurred())
        return true;

      HandlingTopLevelDeclRAII HandlingDecl(*this);

      // Make sure to emit all elements of a Decl.
      for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I)
        Builder->EmitTopLevelDecl(*I);

      return true;
    }

    void EmitDeferredDecls() {
      if (DeferredInlineMethodDefinitions.empty())
        return;

      // Emit any deferred inline method definitions. Note that more deferred
      // methods may be added during this loop, since ASTConsumer callbacks
      // can be invoked if AST inspection results in declarations being added.
      HandlingTopLevelDeclRAII HandlingDecl(*this);
      for (unsigned I = 0; I != DeferredInlineMethodDefinitions.size(); ++I)
        Builder->EmitTopLevelDecl(DeferredInlineMethodDefinitions[I]);
      DeferredInlineMethodDefinitions.clear();
    }

    void HandleInlineMethodDefinition(CXXMethodDecl *D) override {
      if (Diags.hasErrorOccurred())
        return;

      assert(D->doesThisDeclarationHaveABody());

      // We may want to emit this definition. However, that decision might be
      // based on computing the linkage, and we have to defer that in case we
      // are inside of something that will change the method's final linkage,
      // e.g.
      //   typedef struct {
      //     void bar();
      //     void foo() { bar(); }
      //   } A;
      DeferredInlineMethodDefinitions.push_back(D);

      // Provide some coverage mapping even for methods that aren't emitted.
      // Don't do this for templated classes though, as they may not be
      // instantiable.
      if (!D->getParent()->getDescribedClassTemplate())
        Builder->AddDeferredUnusedCoverageMapping(D);
    }

    /// HandleTagDeclDefinition - This callback is invoked each time a TagDecl
    /// to (e.g. struct, union, enum, class) is completed. This allows the
    /// client hack on the type, which can occur at any point in the file
    /// (because these can be defined in declspecs).
    void HandleTagDeclDefinition(TagDecl *D) override {
      if (Diags.hasErrorOccurred())
        return;

      Builder->UpdateCompletedType(D);

      // For MSVC compatibility, treat declarations of static data members with
      // inline initializers as definitions.
      if (Ctx->getLangOpts().MSVCCompat) {
        for (Decl *Member : D->decls()) {
          if (VarDecl *VD = dyn_cast<VarDecl>(Member)) {
            if (Ctx->isMSStaticDataMemberInlineDefinition(VD) &&
                Ctx->DeclMustBeEmitted(VD)) {
              Builder->EmitGlobal(VD);
            }
          }
        }
      }
    }

    void HandleTagDeclRequiredDefinition(const TagDecl *D) override {
      if (Diags.hasErrorOccurred())
        return;

      if (CodeGen::CGDebugInfo *DI = Builder->getModuleDebugInfo())
        if (const RecordDecl *RD = dyn_cast<RecordDecl>(D))
          DI->completeRequiredType(RD);
    }

    void HandleTranslationUnit(ASTContext &Ctx) override {
      if (Diags.hasErrorOccurred()) {
        if (Builder)
          Builder->clear();
        M.reset();
        return;
      }
      // HLSL Change Begins.
      if (&Ctx == this->Ctx) {
        if (Builder) {
          // Add semantic defines for extensions if any are available.
          auto &CodeGenOpts =
              const_cast<CodeGenOptions &>(Builder->getCodeGenOpts());
          if (CodeGenOpts.HLSLExtensionsCodegen) {
            CodeGenOpts.HLSLExtensionsCodegen->WriteSemanticDefines(
                    M.get());
            // Builder->CodeGenOpts is a copy. So update it for every Builder.
            CodeGenOpts.HLSLExtensionsCodegen->UpdateCodeGenOptions(
                CodeGenOpts);
          }
        }
      }
      // HLSL Change Ends.
      if (Builder)
        Builder->Release();

      // HLSL Change Begins

      if (CodeGenOpts.BindingTableParser) {
        hlsl::DxcBindingTable bindingTable;
        std::string errors;
        llvm::raw_string_ostream os(errors);

        if (!CodeGenOpts.BindingTableParser->Parse(os, &bindingTable)) {
          os.flush();
          unsigned DiagID = Diags.getCustomDiagID(
                DiagnosticsEngine::Error, "%0");
          Diags.Report(DiagID) << errors;
        }
        else {
          hlsl::WriteBindingTableToMetadata(*M, bindingTable);
        }
      }
      else {
        // Add resource binding overrides to the metadata.
        hlsl::WriteBindingTableToMetadata(*M, CodeGenOpts.HLSLBindingTable);
      }

      // Error may happen in Builder->Release for HLSL
      if (CodeGenOpts.HLSLEmbedSourcesInModule) {
        llvm::LLVMContext &LLVMCtx = M->getContext();
        // Add all file contents in a list of filename/content pairs.
        llvm::NamedMDNode *pContents = nullptr;
        auto AddFile = [&](StringRef name, StringRef content) {
          if (pContents == nullptr) {
            pContents = M->getOrInsertNamedMetadata(
              hlsl::DxilMDHelper::kDxilSourceContentsMDName);
          }
          llvm::MDTuple *pFileInfo = llvm::MDNode::get(
            LLVMCtx,
            { llvm::MDString::get(LLVMCtx, name),
              llvm::MDString::get(LLVMCtx, content) });
          pContents->addOperand(pFileInfo);
        };
        std::map<std::string, StringRef> filesMap;
        bool bFoundMainFile = false;
        for (SourceManager::fileinfo_iterator
                 it = Ctx.getSourceManager().fileinfo_begin(),
                 end = Ctx.getSourceManager().fileinfo_end();
             it != end; ++it) {
          if (it->first->isValid() && !it->second->IsSystemFile) {
            llvm::SmallString<256> path = StringRef(it->first->getName());
            llvm::sys::path::native(path);

            StringRef contentBuffer = it->second->getRawBuffer()->getBuffer();
            // If main file, write that to metadata first.
            // Add the rest to filesMap to sort by name.
            if (CodeGenOpts.MainFileName.compare(it->first->getName()) == 0) {
              assert(!bFoundMainFile && "otherwise, more than one file matches main filename");
              AddFile(path, contentBuffer);
              bFoundMainFile = true;
            } else {
              // We want the include file paths to match the values passed into
              // the include handlers exactly. The SourceManager entries should
              // match it except the call to MakeAbsoluteOrCurDirRelative.
              filesMap[path.str()] = contentBuffer;
            }
          }
        }
        assert(bFoundMainFile && "otherwise, no file found matches main filename");
        // Emit the rest of the files in sorted order.
        for (auto it : filesMap) {
          AddFile(it.first, it.second);
        }

        // Add Defines to Debug Info
        llvm::NamedMDNode *pDefines = M->getOrInsertNamedMetadata(
            hlsl::DxilMDHelper::kDxilSourceDefinesMDName);
        std::vector<llvm::Metadata *> vecDefines;
        vecDefines.resize(CodeGenOpts.HLSLDefines.size());
        std::transform(CodeGenOpts.HLSLDefines.begin(), CodeGenOpts.HLSLDefines.end(),
          vecDefines.begin(), [&LLVMCtx](const std::string &str) { return llvm::MDString::get(LLVMCtx, str); });
        llvm::MDTuple *pDefinesInfo = llvm::MDNode::get(LLVMCtx, vecDefines);
        pDefines->addOperand(pDefinesInfo);

        // Add main file name to debug info
        llvm::NamedMDNode *pSourceFilename = M->getOrInsertNamedMetadata(
            hlsl::DxilMDHelper::kDxilSourceMainFileNameMDName);
        llvm::SmallString<128> NormalizedPath;
        llvm::sys::path::native(CodeGenOpts.MainFileName, NormalizedPath);
        llvm::MDTuple *pFileName = llvm::MDNode::get(
            LLVMCtx, llvm::MDString::get(LLVMCtx, NormalizedPath));
        pSourceFilename->addOperand(pFileName);

        // Pass in any other arguments to debug info
        llvm::NamedMDNode *pArgs = M->getOrInsertNamedMetadata(
            hlsl::DxilMDHelper::kDxilSourceArgsMDName);
        std::vector<llvm::Metadata *> vecArguments;
        vecArguments.resize(CodeGenOpts.HLSLArguments.size());
        std::transform(CodeGenOpts.HLSLArguments.begin(), CodeGenOpts.HLSLArguments.end(),
          vecArguments.begin(), [&LLVMCtx](const std::string &str) { return llvm::MDString::get(LLVMCtx, str); });
        llvm::MDTuple *pArgumentsInfo = llvm::MDNode::get(LLVMCtx, vecArguments);
        pArgs->addOperand(pArgumentsInfo);

      }
      if (Diags.hasErrorOccurred())
        M.reset();
      // HLSL Change Ends
    }

    void CompleteTentativeDefinition(VarDecl *D) override {
      if (Diags.hasErrorOccurred())
        return;

      Builder->EmitTentativeDefinition(D);
    }

    void HandleVTable(CXXRecordDecl *RD) override {
      if (Diags.hasErrorOccurred())
        return;

      Builder->EmitVTable(RD);
    }

    void HandleLinkerOptionPragma(llvm::StringRef Opts) override {
      Builder->AppendLinkerOptions(Opts);
    }

    void HandleDetectMismatch(llvm::StringRef Name,
                              llvm::StringRef Value) override {
      Builder->AddDetectMismatch(Name, Value);
    }

    void HandleDependentLibrary(llvm::StringRef Lib) override {
      Builder->AddDependentLib(Lib);
    }
  };
}

void CodeGenerator::anchor() { }

CodeGenerator *clang::CreateLLVMCodeGen(
    DiagnosticsEngine &Diags, const std::string &ModuleName,
    const HeaderSearchOptions &HeaderSearchOpts,
    const PreprocessorOptions &PreprocessorOpts, const CodeGenOptions &CGO,
    llvm::LLVMContext &C, CoverageSourceInfo *CoverageInfo) {
  return new CodeGeneratorImpl(Diags, ModuleName, HeaderSearchOpts,
                               PreprocessorOpts, CGO, C, CoverageInfo);
}
