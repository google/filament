///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilValidation.h                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This file provides support for validating DXIL shaders.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/Support/Global.h"
#include "dxc/WinAdapter.h"
#include <memory>

namespace llvm {
class Module;
class LLVMContext;
class raw_ostream;
class DiagnosticPrinter;
class DiagnosticInfo;
} // namespace llvm

namespace hlsl {

void GetValidationVersion(unsigned *pMajor, unsigned *pMinor);

// Validate the container parts, assuming supplied module is valid, loaded from
// the container provided
struct DxilContainerHeader;
HRESULT ValidateDxilContainerParts(llvm::Module *pModule,
                                   llvm::Module *pDebugModule,
                                   const DxilContainerHeader *pContainer,
                                   uint32_t ContainerSize);

// Loads module, validating load, but not module.
HRESULT ValidateLoadModule(const char *pIL, uint32_t ILLength,
                           std::unique_ptr<llvm::Module> &pModule,
                           llvm::LLVMContext &Ctx,
                           llvm::raw_ostream &DiagStream, unsigned bLazyLoad);

// Loads module from container, validating load, but not module.
HRESULT ValidateLoadModuleFromContainer(
    const void *pContainer, uint32_t ContainerSize,
    std::unique_ptr<llvm::Module> &pModule,
    std::unique_ptr<llvm::Module> &pDebugModule, llvm::LLVMContext &Ctx,
    llvm::LLVMContext &DbgCtx, llvm::raw_ostream &DiagStream);
// Lazy loads module from container, validating load, but not module.
HRESULT ValidateLoadModuleFromContainerLazy(
    const void *pContainer, uint32_t ContainerSize,
    std::unique_ptr<llvm::Module> &pModule,
    std::unique_ptr<llvm::Module> &pDebugModule, llvm::LLVMContext &Ctx,
    llvm::LLVMContext &DbgCtx, llvm::raw_ostream &DiagStream);

// Load and validate Dxil module from bitcode.
HRESULT ValidateDxilBitcode(const char *pIL, uint32_t ILLength,
                            llvm::raw_ostream &DiagStream);

// Full container validation, including ValidateDxilModule
HRESULT ValidateDxilContainer(const void *pContainer, uint32_t ContainerSize,
                              llvm::raw_ostream &DiagStream);

// Full container validation, including ValidateDxilModule, with debug module
HRESULT ValidateDxilContainer(const void *pContainer, uint32_t ContainerSize,
                              llvm::Module *pDebugModule,
                              llvm::raw_ostream &DiagStream);

class PrintDiagnosticContext {
private:
  llvm::DiagnosticPrinter &m_Printer;
  bool m_errorsFound;
  bool m_warningsFound;

public:
  PrintDiagnosticContext(llvm::DiagnosticPrinter &printer);

  bool HasErrors() const;
  bool HasWarnings() const;
  void Handle(const llvm::DiagnosticInfo &DI);

  static void PrintDiagnosticHandler(const llvm::DiagnosticInfo &DI,
                                     void *Context);
};

} // namespace hlsl
