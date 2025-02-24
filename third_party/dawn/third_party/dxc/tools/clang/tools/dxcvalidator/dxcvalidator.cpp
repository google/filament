///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcvalidator.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Validator object.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/LLVMContext.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilHash/DxilHash.h"
#include "dxc/DxilValidation/DxilValidation.h"
#include "dxc/dxcapi.h"
#include "dxcvalidator.h"

#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/dxcapi.impl.h"

#ifdef _WIN32
#include "dxcetw.h"
#endif

using namespace llvm;
using namespace hlsl;

static void HashAndUpdate(DxilContainerHeader *Container, bool isPreRelease) {
  if (isPreRelease) {
    // If preview bypass is enabled, use the preview hash.
    memcpy(Container->Hash.Digest, PreviewByPassHash.Digest,
           sizeof(PreviewByPassHash.Digest));
    return;
  }
  // Compute hash and update stored hash.
  // Hash the container from this offset to the end.
  static const uint32_t DXBCHashStartOffset =
      offsetof(struct DxilContainerHeader, Version);
  const unsigned char *DataToHash =
      (const unsigned char *)Container + DXBCHashStartOffset;
  unsigned AmountToHash = Container->ContainerSizeInBytes - DXBCHashStartOffset;
  ComputeHashRetail(DataToHash, AmountToHash, Container->Hash.Digest);
}

static void HashAndUpdateOrCopy(uint32_t Flags, IDxcBlob *Shader,
                                IDxcBlob **Hashed) {
  bool isPreRelease = false;
  const DxilContainerHeader *DxilContainer =
      IsDxilContainerLike(Shader->GetBufferPointer(), Shader->GetBufferSize());
  if (!DxilContainer)
    return;

  const DxilProgramHeader *ProgramHeader =
      GetDxilProgramHeader(DxilContainer, DFCC_DXIL);

  // ProgramHeader may be null here, when hashing a root signature container
  if (ProgramHeader) {
    int PV = ProgramHeader->ProgramVersion;
    int major = (PV >> 4) & 0xF; // Extract the major version (next 4 bits)
    int minor = PV & 0xF;        // Extract the minor version (lowest 4 bits)
    isPreRelease = ShaderModel::IsPreReleaseShaderModel(major, minor);
  }

  if (Flags & DxcValidatorFlags_InPlaceEdit) {
    HashAndUpdate((DxilContainerHeader *)Shader->GetBufferPointer(),
                  isPreRelease);
    *Hashed = Shader;
    Shader->AddRef();
  } else {
    CComPtr<AbstractMemoryStream> HashedBlobStream;
    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &HashedBlobStream));
    unsigned long CB;
    IFT(HashedBlobStream->Write(Shader->GetBufferPointer(),
                                Shader->GetBufferSize(), &CB));
    HashAndUpdate((DxilContainerHeader *)HashedBlobStream->GetPtr(),
                  isPreRelease);
    IFT(HashedBlobStream.QueryInterface(Hashed));
  }
}

static uint32_t runValidation(
    IDxcBlob *Shader,
    uint32_t Flags,            // Validation flags.
    llvm::Module *DebugModule, // Debug module to validate, if available
    AbstractMemoryStream *DiagMemStream) {
  // Run validation may throw, but that indicates an inability to validate,
  // not that the validation failed (eg out of memory). That is indicated
  // by a failing HRESULT, and possibly error messages in the diagnostics
  // stream.

  raw_stream_ostream DiagStream(DiagMemStream);

  return ValidateDxilContainer(Shader->GetBufferPointer(),
                               Shader->GetBufferSize(), DebugModule,
                               DiagStream);
}

static uint32_t
runRootSignatureValidation(IDxcBlob *Shader,
                           AbstractMemoryStream *DiagMemStream) {

  const DxilContainerHeader *DxilContainer =
      IsDxilContainerLike(Shader->GetBufferPointer(), Shader->GetBufferSize());
  if (!DxilContainer)
    return DXC_E_IR_VERIFICATION_FAILED;

  const DxilProgramHeader *ProgramHeader =
      GetDxilProgramHeader(DxilContainer, DFCC_DXIL);
  const DxilPartHeader *PSVPart =
      GetDxilPartByType(DxilContainer, DFCC_PipelineStateValidation);
  const DxilPartHeader *RSPart =
      GetDxilPartByType(DxilContainer, DFCC_RootSignature);
  if (!RSPart)
    return DXC_E_MISSING_PART;
  if (ProgramHeader) {
    // Container has shader part, make sure we have PSV.
    if (!PSVPart)
      return DXC_E_MISSING_PART;
  }
  try {
    RootSignatureHandle RSH;
    RSH.LoadSerialized((const uint8_t *)GetDxilPartData(RSPart),
                       RSPart->PartSize);
    RSH.Deserialize();
    raw_stream_ostream DiagStream(DiagMemStream);
    if (ProgramHeader) {
      if (!VerifyRootSignatureWithShaderPSV(
              RSH.GetDesc(),
              GetVersionShaderType(ProgramHeader->ProgramVersion),
              GetDxilPartData(PSVPart), PSVPart->PartSize, DiagStream))
        return DXC_E_INCORRECT_ROOT_SIGNATURE;
    } else {
      if (!VerifyRootSignature(RSH.GetDesc(), DiagStream, false))
        return DXC_E_INCORRECT_ROOT_SIGNATURE;
    }
  } catch (...) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  return S_OK;
}

static uint32_t runDxilModuleValidation(IDxcBlob *Shader, // Shader to validate.
                                        AbstractMemoryStream *DiagMemStream) {
  if (IsDxilContainerLike(Shader->GetBufferPointer(), Shader->GetBufferSize()))
    return E_INVALIDARG;

  raw_stream_ostream DiagStream(DiagMemStream);
  return ValidateDxilBitcode((const char *)Shader->GetBufferPointer(),
                             (uint32_t)Shader->GetBufferSize(), DiagStream);
}

uint32_t hlsl::validateWithDebug(
    IDxcBlob *Shader,            // Shader to validate.
    uint32_t Flags,              // Validation flags.
    DxcBuffer *OptDebugBitcode,  // Optional debug module bitcode to provide
                                 // line numbers
    IDxcOperationResult **Result // Validation output status, buffer, and errors
) {
  if (Result == nullptr)
    return E_INVALIDARG;
  *Result = nullptr;
  if (Shader == nullptr || Flags & ~DxcValidatorFlags_ValidMask)
    return E_INVALIDARG;
  if ((Flags & DxcValidatorFlags_ModuleOnly) &&
      (Flags &
       (DxcValidatorFlags_InPlaceEdit | DxcValidatorFlags_RootSignatureOnly)))
    return E_INVALIDARG;
  if (OptDebugBitcode &&
      (OptDebugBitcode->Ptr == nullptr || OptDebugBitcode->Size == 0 ||
       OptDebugBitcode->Size >= UINT32_MAX))
    return E_INVALIDARG;

  HRESULT hr = S_OK;
  DxcThreadMalloc TM(DxcGetThreadMallocNoRef());
  try {
    LLVMContext Ctx;
    CComPtr<AbstractMemoryStream> DiagMemStream;
    hr = CreateMemoryStream(TM.GetInstalledAllocator(), &DiagMemStream);
    if (FAILED(hr))
      throw hlsl::Exception(hr);
    raw_stream_ostream DiagStream(DiagMemStream);
    llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
    PrintDiagnosticContext DiagContext(DiagPrinter);
    Ctx.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                             &DiagContext, true);
    std::unique_ptr<llvm::Module> DebugModule;
    if (OptDebugBitcode) {
      hr = ValidateLoadModule((const char *)OptDebugBitcode->Ptr,
                              (uint32_t)OptDebugBitcode->Size, DebugModule, Ctx,
                              DiagStream, /*bLazyLoad*/ false);
      if (FAILED(hr))
        throw hlsl::Exception(hr);
    }
    return validateWithOptDebugModule(Shader, Flags, DebugModule.get(), Result);
  }
  CATCH_CPP_ASSIGN_HRESULT();
  return hr;
}

uint32_t hlsl::validateWithOptDebugModule(
    IDxcBlob *Shader,            // Shader to validate.
    uint32_t Flags,              // Validation flags.
    llvm::Module *DebugModule,   // Debug module to validate, if available
    IDxcOperationResult **Result // Validation output status, buffer, and errors
) {
  *Result = nullptr;
  HRESULT hr = S_OK;
  HRESULT validationStatus = S_OK;
  DxcEtw_DxcValidation_Start();
  DxcThreadMalloc TM(DxcGetThreadMallocNoRef());
  try {
    CComPtr<AbstractMemoryStream> DiagStream;
    hr = CreateMemoryStream(TM.GetInstalledAllocator(), &DiagStream);
    if (FAILED(hr))
      throw hlsl::Exception(hr);
    // Run validation may throw, but that indicates an inability to validate,
    // not that the validation failed (eg out of memory).
    if (Flags & DxcValidatorFlags_RootSignatureOnly)
      validationStatus = runRootSignatureValidation(Shader, DiagStream);
    else if (Flags & DxcValidatorFlags_ModuleOnly)
      validationStatus = runDxilModuleValidation(Shader, DiagStream);
    else
      validationStatus = runValidation(Shader, Flags, DebugModule, DiagStream);
    if (FAILED(validationStatus)) {
      std::string msg("Validation failed.\n");
      ULONG cbWritten;
      DiagStream->Write(msg.c_str(), msg.size(), &cbWritten);
    }
    if (Flags & (DxcValidatorFlags_ModuleOnly)) {
      // Validating a module only, return DXC_OUT_NONE instead of
      // DXC_OUT_OBJECT.
      CComPtr<IDxcBlob> pDiagBlob;
      hr = DiagStream.QueryInterface(&pDiagBlob);
      DXASSERT_NOMSG(SUCCEEDED(hr));
      hr = DxcResult::Create(validationStatus, DXC_OUT_NONE,
                             {DxcOutputObject::ErrorOutput(
                                 CP_UTF8, // TODO Support DefaultTextCodePage
                                 (LPCSTR)pDiagBlob->GetBufferPointer(),
                                 pDiagBlob->GetBufferSize())},
                             Result);
      if (FAILED(hr))
        throw hlsl::Exception(hr);
    } else {
      CComPtr<IDxcBlob> HashedBlob;
      // Assemble the result object.
      CComPtr<IDxcBlob> DiagBlob;
      CComPtr<IDxcBlobEncoding> DiagBlobEnconding;
      hr = DiagStream.QueryInterface(&DiagBlob);
      DXASSERT_NOMSG(SUCCEEDED(hr));
      hr = DxcCreateBlobWithEncodingSet(DiagBlob, CP_UTF8, &DiagBlobEnconding);
      if (FAILED(hr))
        throw hlsl::Exception(hr);
      HashAndUpdateOrCopy(Flags, Shader, &HashedBlob);
      hr = DxcResult::Create(
          validationStatus, DXC_OUT_OBJECT,
          {DxcOutputObject::DataOutput(DXC_OUT_OBJECT, HashedBlob),
           DxcOutputObject::DataOutput(DXC_OUT_ERRORS, DiagBlobEnconding)},
          Result);
      if (FAILED(hr))
        throw hlsl::Exception(hr);
    }
  }
  CATCH_CPP_ASSIGN_HRESULT();

  DxcEtw_DxcValidation_Stop(SUCCEEDED(hr) ? validationStatus : hr);
  return hr;
}

uint32_t hlsl::getValidationVersion(unsigned *Major, unsigned *Minor) {
  if (Major == nullptr || Minor == nullptr)
    return E_INVALIDARG;
  hlsl::GetValidationVersion(Major, Minor);
  return S_OK;
}
