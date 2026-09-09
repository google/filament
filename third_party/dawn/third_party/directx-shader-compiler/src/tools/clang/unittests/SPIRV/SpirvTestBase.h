//===- unittests/SPIRV/SpirvTestBase.h ---- Base class for SPIR-V Tests ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvContext.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace clang::spirv;

class SpirvTestBase : public ::testing::Test {
public:
  SpirvTestBase() : spvContext(), compilerInstance(), initialized(false) {}

  SpirvContext &getSpirvContext() { return spvContext; }

  clang::ASTContext &getAstContext() {
    if (!initialized)
      initialize();
    return compilerInstance.getASTContext();
  }

  FeatureManager &getFeatureManager() {
    if (!initialized)
      initialize();
    compilerInstance.getCodeGenOpts().SpirvOptions.targetEnv = "vulkan1.0";
    static FeatureManager featureManager(
        compilerInstance.getDiagnostics(),
        compilerInstance.getCodeGenOpts().SpirvOptions);
    return featureManager;
  }

private:
  // We don't initialize the compiler instance unless it is asked for in order
  // to make the tests run faster.
  void initialize() {
    std::string warnings;
    llvm::raw_string_ostream w(warnings);
    std::unique_ptr<clang::TextDiagnosticPrinter> diagPrinter =
        llvm::make_unique<clang::TextDiagnosticPrinter>(
            w, &compilerInstance.getDiagnosticOpts());

    std::shared_ptr<clang::TargetOptions> targetOptions(
        new clang::TargetOptions);
    targetOptions->Triple = "dxil-ms-dx";
    compilerInstance.createDiagnostics(diagPrinter.get(), false);
    compilerInstance.createFileManager();
    compilerInstance.createSourceManager(compilerInstance.getFileManager());
    compilerInstance.setTarget(clang::TargetInfo::CreateTargetInfo(
        compilerInstance.getDiagnostics(), targetOptions));

    clang::HeaderSearchOptions &HSOpts = compilerInstance.getHeaderSearchOpts();
    HSOpts.UseBuiltinIncludes = 0;

    compilerInstance.createPreprocessor(
        clang::TranslationUnitKind::TU_Complete);
    compilerInstance.createASTContext();
    initialized = true;
  }

private:
  SpirvContext spvContext;
  clang::CompilerInstance compilerInstance;
  bool initialized;
};
