///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxbcConverter.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX DXBC to DXIL converter.                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "llvm/Support/Debug.h" // Must be included first.

#include "DxbcConverterImpl.h"
#include "DxilConvPasses/DxilCleanup.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilContainerAssembler.h"
#include "dxc/DxilContainer/DxilContainerReader.h"

#define DXBCCONV_DBG 0

namespace hlsl {

__override HRESULT STDMETHODCALLTYPE
DxbcConverter::Convert(LPCVOID pDxbc, UINT32 DxbcSize, LPCWSTR pExtraOptions,
                       LPVOID *ppDxil, UINT32 *pDxilSize, LPWSTR *ppDiag) {
  DxcThreadMalloc TM(m_pMalloc);
  LARGE_INTEGER start, end;
  QueryPerformanceCounter(&start);
  DxcRuntimeEtw_DxcTranslate_Start();
  HRESULT hr = S_OK;
  try {
    sys::fs::MSFileSystem *pFSPtr;
    IFT(CreateMSFileSystemForDisk(&pFSPtr));
    unique_ptr<sys::fs::MSFileSystem> pFS(pFSPtr);
    sys::fs::AutoPerThreadSystem pTS(pFS.get());
    IFTLLVM(pTS.error_code());

    struct StdErrFlusher {
      ~StdErrFlusher() { dbgs().flush(); }
    } S;

    ConvertImpl(pDxbc, DxbcSize, pExtraOptions, ppDxil, pDxilSize, ppDiag);

    DxcRuntimeEtw_DxcTranslate_TranslateStats(DxbcSize, DxbcSize,
                                              (const BYTE *)pDxbc, *pDxilSize);
    hr = S_OK;
  }
  CATCH_CPP_ASSIGN_HRESULT();
  DxcRuntimeEtw_DxcTranslate_Stop(hr);
  QueryPerformanceCounter(&end);
  LogConvertResult(false, &start, &end, pDxbc, DxbcSize, pExtraOptions, *ppDxil,
                   *pDxilSize, hr);
  return hr;
}

__override HRESULT STDMETHODCALLTYPE DxbcConverter::ConvertInDriver(
    const UINT32 *pBytecode, LPCVOID pInputSignature,
    UINT32 NumInputSignatureElements, LPCVOID pOutputSignature,
    UINT32 NumOutputSignatureElements, LPCVOID pPatchConstantSignature,
    UINT32 NumPatchConstantSignatureElements, LPCWSTR pExtraOptions,
    IDxcBlob **ppDxilModule, LPWSTR *ppDiag) {
  DxcThreadMalloc TM(m_pMalloc);
  LARGE_INTEGER start, end;
  QueryPerformanceCounter(&start);
  DxcRuntimeEtw_DxcTranslate_Start();
  HRESULT hr = S_OK;
  UINT32 bcSize = pBytecode[1] * sizeof(UINT32);
  const BYTE *pDxilBytes = nullptr;
  UINT32 DxilByteCount = 0;
  try {
    sys::fs::MSFileSystem *pFSPtr;
    IFT(CreateMSFileSystemForDisk(&pFSPtr));
    unique_ptr<sys::fs::MSFileSystem> pFS(pFSPtr);
    sys::fs::AutoPerThreadSystem pTS(pFS.get());
    IFTLLVM(pTS.error_code());

    struct StdErrFlusher {
      ~StdErrFlusher() { dbgs().flush(); }
    } S;

    ConvertInDriverImpl(
        pBytecode, (const D3D12DDIARG_SIGNATURE_ENTRY_0012 *)pInputSignature,
        NumInputSignatureElements,
        (const D3D12DDIARG_SIGNATURE_ENTRY_0012 *)pOutputSignature,
        NumOutputSignatureElements,
        (const D3D12DDIARG_SIGNATURE_ENTRY_0012 *)pPatchConstantSignature,
        NumPatchConstantSignatureElements, pExtraOptions, ppDxilModule, ppDiag);

    pDxilBytes = (const BYTE *)(*ppDxilModule)->GetBufferPointer();
    DxilByteCount = (*ppDxilModule)->GetBufferSize();
    DxcRuntimeEtw_DxcTranslate_TranslateStats(
        bcSize, bcSize, (const BYTE *)pBytecode, DxilByteCount);

    hr = S_OK;
  }
  CATCH_CPP_ASSIGN_HRESULT();
  DxcRuntimeEtw_DxcTranslate_Stop(hr);
  QueryPerformanceCounter(&end);
  LogConvertResult(true, &start, &end, pBytecode, bcSize, pExtraOptions,
                   pDxilBytes, DxilByteCount, hr);
  return hr;
}

DxbcConverter::DxbcConverter()
    : m_dwRef(0), m_pPR(nullptr), m_pOP(nullptr), m_pSM(nullptr),
      m_DxbcMajor(0), m_DxbcMinor(0), m_bDisableHashCheck(false),
      m_bRunDxilCleanup(true), m_bLegacyCBufferLoad(true),
      m_DepthRegType(D3D10_SB_OPERAND_TYPE_NULL), m_bHasStencilRef(false),
      m_bHasCoverageOut(false), m_pUnusedF32(nullptr), m_pUnusedI32(nullptr),
      m_NumTempRegs(0), m_pIcbGV(nullptr), m_TGSMCount(0),
      m_bControlPointPhase(false), m_bPatchConstantPhase(false),
      m_pInterfaceDataBuffer(nullptr), m_pClassInstanceCBuffers(nullptr),
      m_pClassInstanceSamplers(nullptr),
      m_pClassInstanceComparisonSamplers(nullptr), m_NumIfaces(0),
      m_FcallCount(0) {
  DXASSERT(OP::CheckOpCodeTable(), "incorrect entry in OpCode property table");
}

DxbcConverter::~DxbcConverter() {}

static void
AddDxilPipelineStateValidationToDXBC(DxilModule *pModule,
                                     DxilPipelineStateValidation &PSV);
static void EmitIdentMetadata(llvm::Module *pModule, LPCSTR pValue) {
  llvm::NamedMDNode *IdentMetadata =
      pModule->getOrInsertNamedMetadata("llvm.ident");
  llvm::LLVMContext &Ctx = pModule->getContext();

  llvm::Metadata *IdentNode[] = {llvm::MDString::get(Ctx, pValue)};
  IdentMetadata->addOperand(llvm::MDNode::get(Ctx, IdentNode));
}

void WritePart(AbstractMemoryStream *pStream, const void *pData, size_t size) {
  ULONG cbWritten = 0;
  pStream->Write(pData, size, &cbWritten);
}

void WritePart(AbstractMemoryStream *pStream,
               const SmallVectorImpl<char> &Data) {
  WritePart(pStream, Data.data(), Data.size());
}

void DxbcConverter::ConvertImpl(LPCVOID pDxbc, UINT32 DxbcSize,
                                LPCWSTR pExtraOptions, LPVOID *ppDxil,
                                UINT32 *pDxilSize, LPWSTR *ppDiag) {
  IFTARG(pDxbc);
  IFTARG(ppDxil);
  IFTARG(pDxilSize);
  *ppDxil = nullptr;
  *pDxilSize = 0;
  if (ppDiag)
    *ppDiag = nullptr;

  // Parse pExtraOptions.
  ParseExtraOptions(pExtraOptions);

  // Create the module.
  m_pModule = std::make_unique<llvm::Module>("main", m_Ctx);

  // Setup DxilModule.
  m_pPR = &(m_pModule->GetOrCreateDxilModule(/*skipInit*/ true));
  m_pOP = m_pPR->GetOP();

  // Open DXBC container.
  DxilContainerReader dxbcReader;
  IFT(dxbcReader.Load(pDxbc, DxbcSize));
  const void *pMaxPtr = (const char *)pDxbc + DxbcSize;
  IFTBOOL(pDxbc < pMaxPtr, DXC_E_INCORRECT_DXBC);

  // Obtain the code blob.
  UINT uCodeBlob;
  IFT(dxbcReader.FindFirstPartKind(DXBC_GenericShaderEx, &uCodeBlob));
  if (uCodeBlob == DXIL_CONTAINER_BLOB_NOT_FOUND) {
    IFT(dxbcReader.FindFirstPartKind(DXBC_GenericShader, &uCodeBlob));
  }
  IFTBOOL(uCodeBlob != DXIL_CONTAINER_BLOB_NOT_FOUND, DXC_E_INCORRECT_DXBC);

  const CShaderToken *pByteCode;
  IFTBOOL(dxbcReader.GetPartContent(uCodeBlob, (const void **)&pByteCode) ==
              S_OK,
          DXC_E_INCORRECT_DXBC);

  // Parse DXBC container.
  D3D10ShaderBinary::CShaderCodeParser Parser;

  // 1. Collect information about the shader.
  Parser.SetShader(pByteCode);
  AnalyzeShader(Parser);

  // 2. Parse input signature(s).
  ExtractInputSignatureFromDXBC(dxbcReader, pMaxPtr);
  ConvertSignature(*m_pInputSignature, m_pPR->GetInputSignature());
  if (m_pSM->IsDS()) {
    ExtractPatchConstantSignatureFromDXBC(dxbcReader, pMaxPtr);
    ConvertSignature(*m_pPatchConstantSignature,
                     m_pPR->GetPatchConstOrPrimSignature());
  }

  // 3. Parse output signature(s).
  ExtractOutputSignatureFromDXBC(dxbcReader, pMaxPtr);
  ConvertSignature(*m_pOutputSignature, m_pPR->GetOutputSignature());
  if (m_pSM->IsHS()) {
    ExtractPatchConstantSignatureFromDXBC(dxbcReader, pMaxPtr);
    ConvertSignature(*m_pPatchConstantSignature,
                     m_pPR->GetPatchConstOrPrimSignature());
  }

  // 3.5. Callback before conversion
  PreConvertHook(pByteCode);

  // 4. Transform DXBC to DXIL.
  Parser.SetShader(pByteCode);
  ConvertInstructions(Parser);

  // 5. Emit medatada.
  m_pPR->EmitDxilMetadata();
  EmitIdentMetadata(m_pModule.get(), "dxbc2dxil 1.2");

  // 6. Cleanup/Optimize DXIL.
  Optimize();

  // 7. Callback after conversion
  PostConvertHook(pByteCode);

  // Serialize DXIL.
  SmallVector<char, 4 * 1024> DxilBuffer;
  SerializeDxil(DxilBuffer);

  // Wrap LLVM module in a DXBC container.
  size_t DXILSize = DxilBuffer.size_in_bytes();
  std::unique_ptr<DxilContainerWriter> pContainerWriter(
      hlsl::NewDxilContainerWriter());
  pContainerWriter->AddPart(
      DXBC_DXIL, DXILSize,
      [=](AbstractMemoryStream *pStream) { WritePart(pStream, DxilBuffer); });

  SmallVector<char, 512>
      PSVBuffer; // 512 bytes is enough for 30 resources + header
  {
    UINT uCBuffers = m_pPR->GetCBuffers().size();
    UINT uSamplers = m_pPR->GetSamplers().size();
    UINT uSRVs = m_pPR->GetSRVs().size();
    UINT uUAVs = m_pPR->GetUAVs().size();
    UINT uTotalResources = uCBuffers + uSamplers + uSRVs + uUAVs;
    uint32_t PSVBufferSize = 0;
    DxilPipelineStateValidation PSV;
    PSV.InitNew(uTotalResources, nullptr, &PSVBufferSize);
    PSVBuffer.resize(PSVBufferSize);
    PSV.InitNew(uTotalResources, PSVBuffer.data(), &PSVBufferSize);
    AddDxilPipelineStateValidationToDXBC(m_pPR, PSV);
    pContainerWriter->AddPart(
        DXBC_PipelineStateValidation, PSVBufferSize,
        [=](AbstractMemoryStream *pStream) { WritePart(pStream, PSVBuffer); });
  }

  UINT64 featureBody = 0;
  { // Append original IO signatures to DXIL blob
    DXBCFourCC IOSigFourCCArray[] = {
        DXBC_InputSignature11_1,    DXBC_InputSignature,
        DXBC_OutputSignature11_1,   DXBC_OutputSignature5,
        DXBC_OutputSignature,       DXBC_PatchConstantSignature11_1,
        DXBC_PatchConstantSignature};
    UINT NumSigs = sizeof(IOSigFourCCArray) / sizeof(IOSigFourCCArray[0]);
    UINT uBlob = DXIL_CONTAINER_BLOB_NOT_FOUND;
    UINT uElemSize = 0;
    const void *pBlobData = nullptr;
    for (UINT i = 0; i < NumSigs; i++) {
      IFT(dxbcReader.FindFirstPartKind(IOSigFourCCArray[i], &uBlob));
      if (uBlob != DXIL_CONTAINER_BLOB_NOT_FOUND) {
        IFT(dxbcReader.GetPartContent(uBlob, &pBlobData, &uElemSize));
        pContainerWriter->AddPart(IOSigFourCCArray[i], PSVALIGN4(uElemSize),
                                  [=](AbstractMemoryStream *pStream) {
                                    WritePart(pStream, pBlobData, uElemSize);
                                    unsigned padding =
                                        PSVALIGN4(uElemSize) - uElemSize;
                                    if (padding) {
                                      const char padZeros[4] = {0, 0, 0, 0};
                                      WritePart(pStream, padZeros, padding);
                                    }
                                  });
      }
    }
    // Add DXBC_RootSignature and DXBC_ShaderFeatureInfo if present
    IFT(dxbcReader.FindFirstPartKind(DXBC_RootSignature, &uBlob));
    if (uBlob != DXIL_CONTAINER_BLOB_NOT_FOUND) {
      IFT(dxbcReader.GetPartContent(uBlob, &pBlobData, &uElemSize));
      pContainerWriter->AddPart(DXBC_RootSignature, uElemSize,
                                [=](AbstractMemoryStream *pStream) {
                                  WritePart(pStream, pBlobData, uElemSize);
                                });
    }
    IFT(dxbcReader.FindFirstPartKind(DXBC_ShaderFeatureInfo, &uBlob));
    if (uBlob != DXIL_CONTAINER_BLOB_NOT_FOUND) {
      IFT(dxbcReader.GetPartContent(uBlob, &pBlobData, &uElemSize));
      pContainerWriter->AddPart(DXBC_ShaderFeatureInfo, uElemSize,
                                [=](AbstractMemoryStream *pStream) {
                                  WritePart(pStream, pBlobData, uElemSize);
                                });
    } else {
      // Add one anyway
      uElemSize = sizeof(UINT64);
      pContainerWriter->AddPart(DXBC_ShaderFeatureInfo, uElemSize,
                                [=](AbstractMemoryStream *pStream) {
                                  WritePart(pStream, (void *)&featureBody,
                                            sizeof(featureBody));
                                });
    }
  }

  // Serialize the container
  UINT32 OutputSize = pContainerWriter->size();
  CComHeapPtr<void> pOutput;
  IFTBOOL(pOutput.AllocateBytes(OutputSize), E_OUTOFMEMORY);

  CComPtr<AbstractMemoryStream> pOutputStream;
  IFT(CreateFixedSizeMemoryStream((LPBYTE)pOutput.m_pData, OutputSize,
                                  &pOutputStream));
  pContainerWriter->write(pOutputStream);
  // pOutputStream does not own the buffer; allow CComPtr to clean up the stream
  // object.

  *ppDxil = pOutput.Detach();
  *pDxilSize = OutputSize;

  m_pBuilder.reset();
  m_pModule.reset();

  // Diagnostics.
  if (ppDiag)
    *ppDiag = nullptr;
}

void DxbcConverter::ConvertInDriverImpl(
    const UINT32 *pByteCode,
    const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pInputSignature,
    UINT32 NumInputSignatureElements,
    const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pOutputSignature,
    UINT32 NumOutputSignatureElements,
    const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pPatchConstantSignature,
    UINT32 NumPatchConstantSignatureElements, LPCWSTR pExtraOptions,
    IDxcBlob **ppDxcBlob, LPWSTR *ppDiag) {
  IFTARG(pByteCode);
  IFTARG(ppDxcBlob);
  UINT SizeInUINTs = pByteCode[1];
  IFTBOOL(SizeInUINTs >= 2, DXC_E_ERROR_PARSING_DXBC_BYTECODE);
  *ppDxcBlob = nullptr;
  if (ppDiag)
    *ppDiag = nullptr;

  // Parse pExtraOptions.
  ParseExtraOptions(pExtraOptions);

  // Create the module.
  m_pModule = std::make_unique<llvm::Module>("main", m_Ctx);

  // Setup DxilModule.
  m_pPR = &(m_pModule->GetOrCreateDxilModule(/*skipInit*/ true));
  m_pOP = m_pPR->GetOP();

  // Parse DXBC bytecode.
  D3D10ShaderBinary::CShaderCodeParser Parser;

  // 1. Collect information about the shader.
  Parser.SetShader(pByteCode);
  AnalyzeShader(Parser);

  // 2. Parse input signature(s).
  ExtractSignatureFromDDI(pInputSignature, NumInputSignatureElements,
                          *m_pInputSignature);
  ConvertSignature(*m_pInputSignature, m_pPR->GetInputSignature());
  if (m_pSM->IsDS()) {
    ExtractSignatureFromDDI(pPatchConstantSignature,
                            NumPatchConstantSignatureElements,
                            *m_pPatchConstantSignature);
    ConvertSignature(*m_pPatchConstantSignature,
                     m_pPR->GetPatchConstOrPrimSignature());
  }

  // 3. Parse output signature(s).
  ExtractSignatureFromDDI(pOutputSignature, NumOutputSignatureElements,
                          *m_pOutputSignature);
  ConvertSignature(*m_pOutputSignature, m_pPR->GetOutputSignature());
  if (m_pSM->IsHS()) {
    ExtractSignatureFromDDI(pPatchConstantSignature,
                            NumPatchConstantSignatureElements,
                            *m_pPatchConstantSignature);
    ConvertSignature(*m_pPatchConstantSignature,
                     m_pPR->GetPatchConstOrPrimSignature());
  }

  // 3.5. Callback before conversion
  PreConvertHook(pByteCode);

  // 4. Transform DXBC to DXIL.
  Parser.SetShader(pByteCode);
  ConvertInstructions(Parser);

  // 5. Emit medatada.
  m_pPR->EmitDxilMetadata();

  // 6. Cleanup/Optimize DXIL.
  Optimize();

  // 7. Callback after conversion
  PostConvertHook(pByteCode);

  // Serialize DXIL.
  SmallVector<char, 8 * 1024> DxilBuffer;
  raw_svector_ostream DxilStream(DxilBuffer);
  WriteBitcodeToFile(m_pModule.get(), DxilStream);
  DxilStream.flush();

  IFT(DxcCreateBlobOnHeapCopy(DxilBuffer.data(), DxilBuffer.size_in_bytes(),
                              ppDxcBlob));

  m_pBuilder.reset();
  m_pModule.reset();

  // Diagnostics.
  if (ppDiag)
    *ppDiag = nullptr;
}

void DxbcConverter::ParseExtraOptions(const wchar_t *pExtraOptions) {
  if (pExtraOptions == nullptr)
    return;

  // This is temporary implementation for now.
  wstring Str(pExtraOptions);
  if (Str.find(L"-disableHashCheck") != wstring::npos)
    m_bDisableHashCheck = true;

  // Opt out from DXIL cleanup pass.
  if (Str.find(L"-no-dxil-cleanup") != wstring::npos)
    m_bRunDxilCleanup = false;
}

void DxbcConverter::SetShaderGlobalFlags(unsigned GlobalFlags) {
  // GlobalFlags takes the set of flags defined for
  // D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
  m_pPR->m_ShaderFlags.SetDisableOptimizations(DXBC::IsFlagDisableOptimizations(
      GlobalFlags)); // ~D3D11_1_SB_GLOBAL_FLAG_SKIP_OPTIMIZATION
  m_pPR->m_ShaderFlags.SetDisableMathRefactoring(
      DXBC::IsFlagDisableMathRefactoring(
          GlobalFlags)); // ~D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED
  m_pPR->m_ShaderFlags.SetEnableDoublePrecision(
      DXBC::IsFlagEnableDoublePrecision(
          GlobalFlags)); // D3D11_SB_GLOBAL_FLAG_ENABLE_DOUBLE_PRECISION_FLOAT_OPS
  m_pPR->m_ShaderFlags.SetForceEarlyDepthStencil(
      DXBC::IsFlagForceEarlyDepthStencil(
          GlobalFlags)); // D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL
  m_pPR->m_ShaderFlags.SetLowPrecisionPresent(DXBC::IsFlagEnableMinPrecision(
      GlobalFlags)); // D3D11_1_SB_GLOBAL_FLAG_ENABLE_MINIMUM_PRECISION
  m_pPR->m_ShaderFlags.SetEnableDoubleExtensions(
      DXBC::IsFlagEnableDoubleExtensions(
          GlobalFlags)); // D3D11_1_SB_GLOBAL_FLAG_ENABLE_DOUBLE_EXTENSIONS
  m_pPR->m_ShaderFlags.SetEnableMSAD(DXBC::IsFlagEnableMSAD(
      GlobalFlags)); // D3D11_1_SB_GLOBAL_FLAG_ENABLE_SHADER_EXTENSIONS
  if (IsSM51Plus()) {
    m_pPR->m_ShaderFlags.SetAllResourcesBound(DXBC::IsFlagAllResourcesBound(
        GlobalFlags)); // D3D12_SB_GLOBAL_FLAG_ALL_RESOURCES_BOUND
  }
  m_pPR->m_ShaderFlags.SetEnableRawAndStructuredBuffers(
      DXBC::IsFlagEnableRawAndStructuredBuffers(
          GlobalFlags)); // D3D12_SB_GLOBAL_FLAG_ALL_RESOURCES_BOUND
}

void DxbcConverter::ExtractInputSignatureFromDXBC(
    DxilContainerReader &dxbcReader, const void *pMaxPtr) {
  // Obtain the input signature blob.
  UINT uBlob;
  IFT(dxbcReader.FindFirstPartKind(DXBC_InputSignature11_1, &uBlob));
  UINT uElemSize = sizeof(D3D11_INTERNALSHADER_PARAMETER_11_1);

  if (uBlob == DXIL_CONTAINER_BLOB_NOT_FOUND) {
    IFT(dxbcReader.FindFirstPartKind(DXBC_InputSignature, &uBlob));
    uElemSize = sizeof(D3D10_INTERNALSHADER_PARAMETER);
  }
  IFTBOOL(uBlob != DXIL_CONTAINER_BLOB_NOT_FOUND, DXC_E_INCORRECT_DXBC);

  // Parse signature elements.
  const D3D10_INTERNALSHADER_SIGNATURE *pSig;
  IFT(dxbcReader.GetPartContent(uBlob, (const void **)&pSig))
  ExtractSignatureFromDXBC(pSig, uElemSize, pMaxPtr, *m_pInputSignature);
}

void DxbcConverter::ExtractOutputSignatureFromDXBC(
    DxilContainerReader &dxbcReader, const void *pMaxPtr) {
  // Obtain the output signature blob.
  UINT uBlob;
  IFT(dxbcReader.FindFirstPartKind(DXBC_OutputSignature11_1, &uBlob));
  UINT uElemSize = sizeof(D3D11_INTERNALSHADER_PARAMETER_11_1);

  if (uBlob == DXIL_CONTAINER_BLOB_NOT_FOUND) {
    IFT(dxbcReader.FindFirstPartKind(DXBC_OutputSignature5, &uBlob));
    uElemSize = sizeof(D3D11_INTERNALSHADER_PARAMETER_FOR_GS);
  }
  if (uBlob == DXIL_CONTAINER_BLOB_NOT_FOUND) {
    IFT(dxbcReader.FindFirstPartKind(DXBC_OutputSignature, &uBlob));
    uElemSize = sizeof(D3D10_INTERNALSHADER_PARAMETER);
  }
  IFTBOOL(uBlob != DXIL_CONTAINER_BLOB_NOT_FOUND, DXC_E_INCORRECT_DXBC);

  // Parse signature elements.
  const D3D10_INTERNALSHADER_SIGNATURE *pSig;
  IFT(dxbcReader.GetPartContent(uBlob, (const void **)&pSig));
  ExtractSignatureFromDXBC(pSig, uElemSize, pMaxPtr, *m_pOutputSignature);
}

void DxbcConverter::ExtractPatchConstantSignatureFromDXBC(
    DxilContainerReader &dxbcReader, const void *pMaxPtr) {
  // Obtain the patch-constant signature blob.
  UINT uBlob;
  IFT(dxbcReader.FindFirstPartKind(DXBC_PatchConstantSignature11_1, &uBlob));
  UINT uElemSize = sizeof(D3D11_INTERNALSHADER_PARAMETER_11_1);

  if (uBlob == DXIL_CONTAINER_BLOB_NOT_FOUND) {
    IFT(dxbcReader.FindFirstPartKind(DXBC_PatchConstantSignature, &uBlob));
    uElemSize = sizeof(D3D10_INTERNALSHADER_PARAMETER);
  }
  IFTBOOL(uBlob != DXIL_CONTAINER_BLOB_NOT_FOUND, DXC_E_INCORRECT_DXBC);

  // Parse signature elements.
  const D3D10_INTERNALSHADER_SIGNATURE *pSig;
  IFT(dxbcReader.GetPartContent(uBlob, (const void **)&pSig));
  ExtractSignatureFromDXBC(pSig, uElemSize, pMaxPtr,
                           *m_pPatchConstantSignature);
}

void DxbcConverter::ExtractSignatureFromDXBC(
    const D3D10_INTERNALSHADER_SIGNATURE *pSig, UINT uElemSize,
    const void *pMaxPtr, SignatureHelper &SigHelper) {
  // Verify signature offsets are within the blob.
  const char *pCheck = (const char *)pSig;
  const char *pCheck2 = pCheck + sizeof(D3D10_INTERNALSHADER_SIGNATURE);
  IFTBOOL(pCheck != nullptr && pCheck < pMaxPtr && pCheck2 <= pMaxPtr,
          DXC_E_INCORRECT_DXBC);
  pCheck = (const char *)pSig + pSig->ParameterInfo;
  pCheck2 = pCheck + pSig->Parameters * uElemSize;
  IFTBOOL(pCheck <= pMaxPtr && pCheck2 <= pMaxPtr && pCheck <= pCheck2,
          DXC_E_INCORRECT_DXBC);

  unsigned uParamCount = pSig->Parameters;
  const char *pSigBase = (const char *)pSig;
  const char *pParamBase = pSigBase + pSig->ParameterInfo;

  // This is to test in-driver conversion.
#define TestDDISignature 0
#if TestDDISignature
  vector<D3D12DDIARG_SIGNATURE_ENTRY_0012> TestDDI;
  TestDDI.resize(uParamCount);
  memset(TestDDI.data(), 0,
         TestDDI.size() * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012));

  unsigned EdgeTess = 0, InsideEdgeTess = 0;
#endif

  for (unsigned iElement = 0; iElement < uParamCount; iElement++) {
    D3D11_INTERNALSHADER_PARAMETER_11_1 P = {};
    // Properly copy parameters for the serialized form into P.
    switch (uElemSize) {
    case sizeof(D3D11_INTERNALSHADER_PARAMETER_11_1):
      memcpy(&P, pParamBase + iElement * uElemSize, uElemSize);
      break;
    case sizeof(D3D11_INTERNALSHADER_PARAMETER_FOR_GS):
      memcpy(&P, pParamBase + iElement * uElemSize, uElemSize);
      break;
    case sizeof(D3D10_INTERNALSHADER_PARAMETER):
      static_assert(
          sizeof(D3D11_INTERNALSHADER_PARAMETER_FOR_GS) ==
              sizeof(D3D10_INTERNALSHADER_PARAMETER) +
                  offsetof(D3D11_INTERNALSHADER_PARAMETER_FOR_GS, SemanticName),
          "Incorrect assumptions about field offset");
      memcpy(&P.SemanticName, pParamBase + iElement * uElemSize, uElemSize);
      break;
    default:
      IFT(DXC_E_INCORRECT_DXBC);
    }

    // Extract data from the blob.
    SignatureHelper::ElementRecord E;
    // Existing tests use testasm to create shaders with incorrect semantic
    // names. The converter is compensating for this.
    if (P.SystemValue == D3D_NAME_UNDEFINED) {
      // Retrive name from the signature blob.
      CheckDxbcString(pSigBase + P.SemanticName, pMaxPtr);
      E.SemanticName = string(pSigBase + P.SemanticName);
    } else {
      // Recover canonical SV_ name.
      E.SemanticName = string(DXBC::GetSemanticNameFromD3DName(P.SystemValue));
    }
    unsigned SemanticIndex = DXBC::GetSemanticIndexFromD3DName(P.SystemValue);
    E.SemanticIndex =
        (SemanticIndex == UINT_MAX) ? P.SemanticIndex : SemanticIndex;
    E.StartRow = P.Register;
    E.StartCol = CMask(P.Mask).GetFirstActiveComp();
    E.Rows = 1;
    E.Cols = CMask(P.Mask).GetNumActiveRangeComps();
    E.Stream = P.Stream;
    E.ComponentType = DXBC::GetCompTypeWithMinPrec(
        P.ComponentType, (D3D11_SB_OPERAND_MIN_PRECISION)P.MinPrecision);

#if TestDDISignature
    D3D12DDIARG_SIGNATURE_ENTRY_0012 &D = TestDDI[iElement];
    D.Register = P.Register;
    D.Mask = P.Mask;
    D.Stream = P.Stream;
    D.RegisterComponentType = (D3D10_SB_REGISTER_COMPONENT_TYPE)P.ComponentType;
    D.MinPrecision = (D3D11_SB_OPERAND_MIN_PRECISION)P.MinPrecision;

    switch (P.SystemValue) {
    case D3D_NAME_UNDEFINED:
      D.SystemValue = D3D10_SB_NAME_UNDEFINED;
      break;
    case D3D_NAME_POSITION:
      D.SystemValue = D3D10_SB_NAME_POSITION;
      break;
    case D3D_NAME_CLIP_DISTANCE:
      D.SystemValue = D3D10_SB_NAME_CLIP_DISTANCE;
      break;
    case D3D_NAME_CULL_DISTANCE:
      D.SystemValue = D3D10_SB_NAME_CULL_DISTANCE;
      break;
    case D3D_NAME_RENDER_TARGET_ARRAY_INDEX:
      D.SystemValue = D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX;
      break;
    case D3D_NAME_VIEWPORT_ARRAY_INDEX:
      D.SystemValue = D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX;
      break;
    case D3D_NAME_VERTEX_ID:
      D.SystemValue = D3D10_SB_NAME_VERTEX_ID;
      break;
    case D3D_NAME_PRIMITIVE_ID:
      D.SystemValue = D3D10_SB_NAME_PRIMITIVE_ID;
      break;
    case D3D_NAME_INSTANCE_ID:
      D.SystemValue = D3D10_SB_NAME_INSTANCE_ID;
      break;
    case D3D_NAME_IS_FRONT_FACE:
      D.SystemValue = D3D10_SB_NAME_IS_FRONT_FACE;
      break;
    case D3D_NAME_SAMPLE_INDEX:
      D.SystemValue = D3D10_SB_NAME_SAMPLE_INDEX;
      break;
    case D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR:
      switch (EdgeTess) {
      case 0:
        D.SystemValue = D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR;
        break;
      case 1:
        D.SystemValue = D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR;
        break;
      case 2:
        D.SystemValue = D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR;
        break;
      case 3:
        D.SystemValue = D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR;
        break;
      default:
        DXASSERT_NOMSG(false);
      }
      EdgeTess++;
      break;
    case D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR:
      switch (InsideEdgeTess) {
      case 0:
        D.SystemValue = D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR;
        break;
      case 1:
        D.SystemValue = D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR;
        break;
      default:
        DXASSERT_NOMSG(false);
      }
      InsideEdgeTess++;
      break;
    case D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR:
      switch (EdgeTess) {
      case 0:
        D.SystemValue = D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR;
        break;
      case 1:
        D.SystemValue = D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR;
        break;
      case 2:
        D.SystemValue = D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR;
        break;
      default:
        DXASSERT_NOMSG(false);
      }
      EdgeTess++;
      break;
    case D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR:
      D.SystemValue = D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR;
      break;
    case D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR:
      D.SystemValue = D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR;
      break;
    case D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR:
      D.SystemValue = D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR;
      break;
    case D3D_NAME_TARGET:
    case D3D_NAME_DEPTH:
    case D3D_NAME_COVERAGE:
    case D3D_NAME_DEPTH_GREATER_EQUAL:
    case D3D_NAME_DEPTH_LESS_EQUAL:
    case D3D_NAME_STENCIL_REF:
    case D3D_NAME_INNER_COVERAGE:
      D.SystemValue = D3D10_SB_NAME_UNDEFINED;
      break;
    default:
      DXASSERT_NOMSG(false);
    }
#else
    SigHelper.m_ElementRecords.emplace_back(E);
#endif
  }

#if TestDDISignature
  ExtractSignatureFromDDI(TestDDI.data(), (unsigned)TestDDI.size(), SigHelper);
#endif
}

void DxbcConverter::ExtractSignatureFromDDI(
    const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pElements, unsigned NumElements,
    SignatureHelper &SigHelper) {
  string NamePrefix;
  if (SigHelper.IsInput())
    NamePrefix = "_in";
  else if (SigHelper.IsOutput())
    NamePrefix = "_out";
  else
    NamePrefix = "_pc";

  unsigned iArbitrarySemantic = 0;
  for (unsigned iElement = 0; iElement < NumElements; iElement++) {
    const D3D12DDIARG_SIGNATURE_ENTRY_0012 &P = pElements[iElement];

    // Extract data from DDI signature element record.
    SignatureHelper::ElementRecord E;

    E.StartRow = P.Register;
    E.StartCol = CMask(P.Mask).GetFirstActiveComp();
    E.Rows = 1;
    E.Cols = CMask(P.Mask).GetNumActiveRangeComps();
    E.Stream = P.Stream;

    if (P.SystemValue == D3D10_SB_NAME_UNDEFINED) {
      E.ComponentType = DXBC::GetCompTypeWithMinPrec(
          (D3D_REGISTER_COMPONENT_TYPE)P.RegisterComponentType,
          (D3D11_SB_OPERAND_MIN_PRECISION)P.MinPrecision);

      // For PS output, try to disambiguate semantic based on register index.
      if (m_pSM->IsPS() && SigHelper.IsOutput()) {
        if (P.Register != UINT_MAX) {
          // This must be SV_Target.
          E.SemanticName = "SV_Target";
          E.SemanticIndex = P.Register;
        } else {
          E.SemanticIndex = P.Register;
          switch (P.RegisterComponentType) {
          case D3D10_SB_REGISTER_COMPONENT_UINT32:
          case D3D10_SB_REGISTER_COMPONENT_SINT32: {
            // This must be SV_StencilRef.
            if (m_bHasStencilRef) {
              E.SemanticName = "SV_StencilRef";
            } else if (m_bHasCoverageOut) {
              E.SemanticName = "SV_Coverage";
            } else {
              IFTBOOL(false, DXC_E_INCORRECT_DDI_SIGNATURE);
            }
            break;
          }
          case D3D10_SB_REGISTER_COMPONENT_FLOAT32: {
            // This must be SV_Depth*.
            switch (m_DepthRegType) {
            case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:
              E.SemanticName = "SV_Depth";
              break;
            case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL:
              E.SemanticName = "SV_DepthGreaterEqual";
              break;
            case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:
              E.SemanticName = "SV_DepthLessEqual";
              break;
            case D3D10_SB_OPERAND_TYPE_NULL:
            default:
              IFT(DXC_E_INCORRECT_DDI_SIGNATURE);
            }
            break;
          }
          default:
            IFT(DXC_E_INCORRECT_DDI_SIGNATURE);
          }
        }
      } else {
        // Arbitrary semantic.
        E.SemanticName = NamePrefix + std::to_string(iArbitrarySemantic++);
        E.SemanticIndex = iElement;
      }
    } else {
      E.SemanticName = string(DXBC::GetD3D10SBName(P.SystemValue));
      E.SemanticIndex = DXBC::GetD3D10SBSemanticIndex(P.SystemValue);
      if (P.RegisterComponentType != D3D10_SB_REGISTER_COMPONENT_UNKNOWN) {
        E.ComponentType = DXBC::GetCompTypeWithMinPrec(
            (D3D_REGISTER_COMPONENT_TYPE)P.RegisterComponentType,
            (D3D11_SB_OPERAND_MIN_PRECISION)P.MinPrecision);
      } else {
        E.ComponentType = DXBC::GetD3DRegCompType(P.SystemValue);
      }
    }

    // This would happen is component type is not supplied by the runtime.
    IFTBOOL(!E.ComponentType.IsInvalid(), DXC_E_INCORRECT_DDI_SIGNATURE);

    SigHelper.m_ElementRecords.emplace_back(E);
  }
}

void DxbcConverter::ConvertSignature(SignatureHelper &SigHelper,
                                     DxilSignature &DxilSig) {
  // Sort SigHelper.m_UsedElements for upcoming binary search.
  std::sort(SigHelper.m_UsedElements.begin(), SigHelper.m_UsedElements.end(),
            SignatureHelper::UsedElement::LTByStreamAndStartRowAndStartCol());

  if (!SigHelper.m_Ranges.empty()) {
    // Adjust range columns to tightly include components of signature elements.
    for (size_t iRange = 0; iRange < SigHelper.m_Ranges.size(); iRange++) {
      SignatureHelper::Range &R = SigHelper.m_Ranges[iRange];
      unsigned RangeStartCol = UINT32_MAX;
      unsigned RangeEndCol = UINT32_MAX;

      for (size_t iElement = 0; iElement < SigHelper.m_ElementRecords.size();
           iElement++) {
        const SignatureHelper::ElementRecord &SigElem =
            SigHelper.m_ElementRecords[iElement];
        unsigned StartRow = SigElem.StartRow;
        unsigned StartCol = SigElem.StartCol;
        unsigned Rows = SigElem.Rows;
        DXASSERT_LOCALVAR_NOMSG(Rows, Rows == 1);
        unsigned Cols = SigElem.Cols;
        unsigned Stream = SigElem.Stream;

        if (R.OutputStream != Stream)
          continue;

        if (R.StartRow <= StartRow && StartRow < R.StartRow + R.Rows) {
          if (!(StartCol + Cols - 1 < R.GetStartCol() ||
                R.GetEndCol() < StartCol)) {
            // Signature element overlaps with the declared range.
            if (RangeStartCol != UINT32_MAX) {
              RangeStartCol = std::min(RangeStartCol, StartCol);
              RangeEndCol = std::max(RangeEndCol, StartCol + Cols - 1);
            } else {
              RangeStartCol = StartCol;
              RangeEndCol = StartCol + Cols - 1;
            }
          }
        }
      }
      R.StartCol = RangeStartCol;
      R.Cols = RangeEndCol - RangeStartCol + 1;
    }

    // Coalesce declaration ranges if they overlap.
    std::sort(SigHelper.m_Ranges.begin(), SigHelper.m_Ranges.end(),
              SignatureHelper::Range::LTRangeByStreamAndStartRowAndStartCol());
    unsigned iLastEntryIndex = 0;
    for (size_t i = 1; i < SigHelper.m_Ranges.size(); i++) {
      // Current range into which we try to coalesce.
      SignatureHelper::Range &R1 = SigHelper.m_Ranges[iLastEntryIndex];
      // A range that is a candidate for coalescing.
      const SignatureHelper::Range &R2 = SigHelper.m_Ranges[i];
      // Do R1 and R2 overlap?
      DXASSERT_NOMSG(R1.GetStartRow() <= R2.GetStartRow());
      bool bOverlaps = (R1.GetStartRow() <= R2.GetStartRow() &&
                        R2.GetStartRow() <= R1.GetEndRow()) &&
                       !(R1.GetEndCol() < R2.GetStartCol() ||
                         R2.GetEndCol() < R1.GetStartCol());
      if (bOverlaps) {
        // Coalesce ranges.
        R1.Rows = std::max(R1.Rows, R2.GetEndRow() - R1.GetStartRow() + 1);
        unsigned StartCol = std::min(R1.GetStartCol(), R2.GetStartCol());
        unsigned EndCol = std::max(R1.GetEndCol(), R2.GetEndCol());
        R1.StartCol = StartCol;
        R1.Cols = EndCol - R1.StartCol + 1;
      } else {
        iLastEntryIndex++;
        SigHelper.m_Ranges[iLastEntryIndex] = SigHelper.m_Ranges[i];
      }
    }
    SigHelper.m_Ranges.resize(iLastEntryIndex + 1);
  }

  // map range elements from SigHelper.m_ElementRecords to dxil signature
  // element index
  std::map<unsigned, unsigned> RangeElementToDxilElement;

  for (size_t iElement = 0; iElement < SigHelper.m_ElementRecords.size();
       iElement++) {
    const SignatureHelper::ElementRecord &SigElem =
        SigHelper.m_ElementRecords[iElement];
    const string &SemanticName = SigElem.SemanticName;
    unsigned SemanticIndex = SigElem.SemanticIndex;
    unsigned StartRow = SigElem.StartRow;
    unsigned StartCol = SigElem.StartCol;
    unsigned Rows = SigElem.Rows;
    DXASSERT_NOMSG(Rows == 1);
    unsigned Cols = SigElem.Cols;
    unsigned Stream = SigElem.Stream;
    CompType ComponentType = SigElem.ComponentType;

    // Determine interpolation mode by matching the corresponding decl record.
    D3D_INTERPOLATION_MODE D3DInterpMode = D3D_INTERPOLATION_UNDEFINED;
    if (m_pSM->IsPS() && SigHelper.IsInput()) {
      bool bFirstUse = false;
      if (!SigHelper.m_UsedElements.empty()) {
        for (unsigned i = 0; i < Cols; i++) {
          unsigned c = StartCol + i;
          // Find used-element lower bound.
          SignatureHelper::UsedElement E1;
          E1.Row = StartRow;
          E1.StartCol = StartCol;
          E1.OutputStream = Stream;
          auto it = std::lower_bound(
              SigHelper.m_UsedElements.begin(), SigHelper.m_UsedElements.end(),
              E1,
              SignatureHelper::UsedElement::LTByStreamAndStartRowAndStartCol());

          if (it != SigHelper.m_UsedElements.end()) {
            SignatureHelper::UsedElement &E2 = *it;
            if (E2.Row == E1.Row &&
                (E2.StartCol <= c && c < E2.StartCol + E2.Cols)) {
              if (!bFirstUse) {
                bFirstUse = true;
                D3DInterpMode = E2.InterpolationMode;
              } else {
                DXASSERT_DXBC(D3DInterpMode == E2.InterpolationMode);
              }
            }
          }
        }
      }
    }

    // Create a new signature element.
    InterpolationMode::Kind IMK = DXBC::GetInterpolationModeKind(D3DInterpMode);
    unique_ptr<DxilSignatureElement> pE(SigHelper.m_Signature.CreateElement());
    pE->Initialize(SemanticName, ComponentType, InterpolationMode(IMK), Rows,
                   Cols, StartRow, StartCol);
    pE->SetOutputStream(Stream);
    DxilSignatureElement &E = *pE;

    // Check range containment.
    bool bInRange = false;
    if (!SigHelper.m_Ranges.empty()) {
      // Search which range contains the element.
      for (size_t iRange = 0; iRange < SigHelper.m_Ranges.size(); iRange++) {
        SignatureHelper::Range &R = SigHelper.m_Ranges[iRange];

        if (R.OutputStream != Stream)
          continue;

        if (R.StartRow <= StartRow && StartRow < R.StartRow + R.Rows) {
          if (!(StartCol + Cols - 1 < R.GetStartCol() ||
                R.GetEndCol() < StartCol)) {
            // Found containment.
            bInRange = true;
            auto itKeyDxilEl = RangeElementToDxilElement.find(iElement);
            if (itKeyDxilEl == RangeElementToDxilElement.end()) {
              // First element in range
              unsigned iDxilElementIndex =
                  (unsigned)SigHelper.m_Signature.GetElements().size();
              E.AppendSemanticIndex(SemanticIndex);

              // Search for all matching elements by semantic in range to expand
              // the range of this element:
              for (size_t iOtherEl = iElement + 1;
                   iOtherEl < SigHelper.m_ElementRecords.size() &&
                   StartRow + Rows < R.StartRow + R.Rows;
                   iOtherEl++) {
                // Skip elements that are part of another captured range already
                if (RangeElementToDxilElement.find(iOtherEl) !=
                    RangeElementToDxilElement.end())
                  continue;
                const SignatureHelper::ElementRecord &OtherEl =
                    SigHelper.m_ElementRecords[iOtherEl];
                // There should be no gaps for indexed element, so we're done if
                // we find one.
                if (OtherEl.StartRow > StartRow + Rows)
                  break;
                if (SemanticName.compare(OtherEl.SemanticName) == 0) {
                  // OtherEl should always have one row
                  DXASSERT_DXBC(OtherEl.Rows == 1);
                  // should always be adding one row at a time in order, and
                  // single indexed element should not have different start
                  // column.
                  if (OtherEl.StartRow == StartRow + Rows &&
                      StartCol == OtherEl.StartCol) {
                    RangeElementToDxilElement[iOtherEl] = iDxilElementIndex;
                    Cols = std::max(Cols, OtherEl.Cols);
                    Rows++;
                    E.AppendSemanticIndex(OtherEl.SemanticIndex);
                  }
                }
              }
              // Adjust element dimensions to encompas matching elements.
              E.SetStartCol(StartCol);
              E.SetCols(Cols);
              E.SetRows(Rows);
              SigHelper.m_Signature.AppendElement(std::move(pE));
            } else {
#ifndef NDEBUG
              // Verify match with range representative element.
              DxilSignatureElement &RE =
                  SigHelper.m_Signature.GetElement(itKeyDxilEl->second);
              DXASSERT_DXBC(RE.GetCompType() == E.GetCompType());
              DXASSERT_DXBC(*RE.GetInterpolationMode() ==
                            *E.GetInterpolationMode());
#endif
            }

            break;
          } else {
            // Check that there is no overlap.
            DXASSERT_DXBC(StartCol + Cols <= R.StartCol ||
                          StartCol >= R.StartCol + R.Cols);
          }
        }
      }
    }

    if (!bInRange) {
      DXASSERT(E.GetSemanticIndexVec().empty(), "otherwise a bug");
      E.AppendSemanticIndex(SemanticIndex);

      SigHelper.m_Signature.AppendElement(std::move(pE));
    }
  }

  // Add SGVs that are not present in the signature blob.
  if (SigHelper.m_bHasInputCoverage || SigHelper.m_bHasInnerInputCoverage) {
    DXASSERT_DXBC(m_pSM->IsPS() && SigHelper.IsInput());
    string SemName;
    if (SigHelper.m_bHasInputCoverage) {
      DXASSERT_DXBC(!SigHelper.m_bHasInnerInputCoverage);
      SemName = string("SV_Coverage");
    } else {
      DXASSERT_DXBC(!SigHelper.m_bHasInputCoverage &&
                    SigHelper.m_bHasInnerInputCoverage);
      SemName = string("SV_InnerCoverage");
    }

    unique_ptr<DxilSignatureElement> E(SigHelper.m_Signature.CreateElement());
    E->Initialize(SemName, CompType::Kind::U32, InterpolationMode(), 1, 1,
                  Semantic::kUndefinedRow, 0);
    E->AppendSemanticIndex(0);

    SigHelper.m_Signature.AppendElement(std::move(E));
  }

  // Set up DXBC <reg,comp> to Element mapping or DXBC OperandRegType to Element
  // mapping, depending on the semantic type.
  for (size_t iElem = 0; iElem < SigHelper.m_Signature.GetElements().size();
       iElem++) {
    DxilSignatureElement &E = SigHelper.m_Signature.GetElement(iElem);

    bool bUpdateRegMap = E.IsAllocated();

    switch (E.GetKind()) {
    case Semantic::Kind::Coverage:
    case Semantic::Kind::InnerCoverage:
    case Semantic::Kind::Depth:
    case Semantic::Kind::DepthGreaterEqual:
    case Semantic::Kind::DepthLessEqual:
    case Semantic::Kind::StencilRef: {
      bUpdateRegMap = false;
      D3D10_SB_OPERAND_TYPE OperandRegType = DXBC::GetOperandRegType(
          E.GetKind(), /*IsOutput*/ SigHelper.IsOutput());
      DXASSERT_DXBC(
          SigHelper.m_DxbcSgvToSignatureElement.find(OperandRegType) ==
          SigHelper.m_DxbcSgvToSignatureElement.end());
      SigHelper.m_DxbcSgvToSignatureElement[OperandRegType] = (unsigned)iElem;
      break;
    }
    }

    if (bUpdateRegMap) {
      DXASSERT_NOMSG(E.IsAllocated());
      unsigned Stream = E.GetOutputStream();
      for (unsigned iRow = 0; iRow < E.GetRows(); iRow++) {
        unsigned r = E.GetStartRow() + iRow;
        for (unsigned iCol = 0; iCol < E.GetCols(); iCol++) {
          unsigned c = E.GetStartCol() + iCol;
          SignatureHelper::RegAndCompAndStream Key(r, c, Stream);
          DXASSERT(SigHelper.m_DxbcRegisterToSignatureElement.find(Key) ==
                       SigHelper.m_DxbcRegisterToSignatureElement.end(),
                   "otherwise elements are wrong");
          SigHelper.m_DxbcRegisterToSignatureElement[Key] = (unsigned)iElem;
        }
      }
    }
  }

  // Clone signature elements into DxilModule.
  for (size_t i = 0; i < SigHelper.m_Signature.GetElements().size(); i++) {
    DxilSignatureElement &E = SigHelper.m_Signature.GetElement(i);
    DXIL::SemanticInterpretationKind I = E.GetInterpretation();
    switch (I) {
    case DXIL::SemanticInterpretationKind::NA:
    case DXIL::SemanticInterpretationKind::NotInSig:
    case DXIL::SemanticInterpretationKind::Invalid:
      continue;
    }
    unique_ptr<DxilSignatureElement> pClone(new DxilSignatureElement(E));
    switch (I) {
    case DXIL::SemanticInterpretationKind::NotPacked:
    case DXIL::SemanticInterpretationKind::Shadow:
      // Make sure element is unallocated in this case (DXBC allocates some of
      // these)
      pClone->SetStartRow(Semantic::kUndefinedRow);
      pClone->SetStartCol(Semantic::kUndefinedCol);
      break;
    }
    DxilSig.AppendElement(std::move(pClone));
  }
}

static void
AddDxilPipelineStateValidationToDXBC(DxilModule *pModule,
                                     DxilPipelineStateValidation &PSV) {
  UINT uCBuffers = pModule->GetCBuffers().size();
  UINT uSamplers = pModule->GetSamplers().size();
  UINT uSRVs = pModule->GetSRVs().size();
  UINT uUAVs = pModule->GetUAVs().size();
  UINT uTotalResources = uCBuffers + uSamplers + uSRVs + uUAVs;

  // Set DxilRuntimInfo
  PSVRuntimeInfo0 *pInfo = PSV.GetPSVRuntimeInfo0();
  const ShaderModel *pSM = pModule->GetShaderModel();
  pInfo->MinimumExpectedWaveLaneCount = 0;
  pInfo->MaximumExpectedWaveLaneCount = -1;

  switch (pSM->GetKind()) {
  case ShaderModel::Kind::Vertex: {
    pInfo->VS.OutputPositionPresent = 0;
    DxilSignature &S = pModule->GetOutputSignature();
    for (auto &&E : S.GetElements()) {
      if (E->GetKind() == Semantic::Kind::Position) {
        // Ideally, we might check never writes mask here,
        // but this is not yet part of the signature element in Dxil
        pInfo->VS.OutputPositionPresent = 1;
        break;
      }
    }
    break;
  }
  case ShaderModel::Kind::Hull: {
    pInfo->HS.InputControlPointCount =
        (UINT)pModule->GetInputControlPointCount();
    pInfo->HS.OutputControlPointCount =
        (UINT)pModule->GetOutputControlPointCount();
    pInfo->HS.TessellatorDomain = (UINT)pModule->GetTessellatorDomain();
    pInfo->HS.TessellatorOutputPrimitive =
        (UINT)pModule->GetTessellatorOutputPrimitive();
    break;
  }
  case ShaderModel::Kind::Domain: {
    pInfo->DS.InputControlPointCount =
        (UINT)pModule->GetInputControlPointCount();
    pInfo->DS.OutputPositionPresent = 0;
    DxilSignature &S = pModule->GetOutputSignature();
    for (auto &&E : S.GetElements()) {
      if (E->GetKind() == Semantic::Kind::Position) {
        // Ideally, we might check never writes mask here,
        // but this is not yet part of the signature element in Dxil
        pInfo->DS.OutputPositionPresent = 1;
        break;
      }
    }
    pInfo->DS.TessellatorDomain = (UINT)pModule->GetTessellatorDomain();
    break;
  }
  case ShaderModel::Kind::Geometry: {
    pInfo->GS.InputPrimitive = (UINT)pModule->GetInputPrimitive();
    // NOTE: For OutputTopology, pick one from a used stream, or if none
    // are used, use stream 0, and set OutputStreamMask to 1.
    pInfo->GS.OutputTopology = (UINT)pModule->GetStreamPrimitiveTopology();
    pInfo->GS.OutputStreamMask = pModule->GetActiveStreamMask();
    pInfo->GS.OutputPositionPresent = 0;
    DxilSignature &S = pModule->GetOutputSignature();
    for (auto &&E : S.GetElements()) {
      if (E->GetKind() == Semantic::Kind::Position) {
        // Ideally, we might check never writes mask here,
        // but this is not yet part of the signature element in Dxil
        pInfo->GS.OutputPositionPresent = 1;
        break;
      }
    }
    break;
  }
  case ShaderModel::Kind::Pixel: {
    pInfo->PS.DepthOutput = 0;
    pInfo->PS.SampleFrequency = 0;
    {
      DxilSignature &S = pModule->GetInputSignature();
      for (auto &&E : S.GetElements()) {
        if (E->GetInterpolationMode()->IsAnySample() ||
            E->GetKind() == Semantic::Kind::SampleIndex) {
          pInfo->PS.SampleFrequency = 1;
          break;
        }
      }
    }
    {
      DxilSignature &S = pModule->GetOutputSignature();
      for (auto &&E : S.GetElements()) {
        if (E->IsAnyDepth()) {
          pInfo->PS.DepthOutput = 1;
          break;
        }
      }
    }
    break;
  }
  }

  // Set resource binding information
  UINT uResIndex = 0;
  for (auto &&R : pModule->GetCBuffers()) {
    DXASSERT_LOCALVAR_NOMSG(uTotalResources, uResIndex < uTotalResources);
    PSVResourceBindInfo0 *pBindInfo = PSV.GetPSVResourceBindInfo0(uResIndex);
    DXASSERT_NOMSG(pBindInfo);
    InitPSVResourceBinding(pBindInfo, nullptr, R.get());
    uResIndex++;
  }
  for (auto &&R : pModule->GetSamplers()) {
    DXASSERT_NOMSG(uResIndex < uTotalResources);
    PSVResourceBindInfo0 *pBindInfo = PSV.GetPSVResourceBindInfo0(uResIndex);
    DXASSERT_NOMSG(pBindInfo);
    InitPSVResourceBinding(pBindInfo, nullptr, R.get());
    uResIndex++;
  }
  for (auto &&R : pModule->GetSRVs()) {
    DXASSERT_NOMSG(uResIndex < uTotalResources);
    PSVResourceBindInfo0 *pBindInfo = PSV.GetPSVResourceBindInfo0(uResIndex);
    DXASSERT_NOMSG(pBindInfo);
    InitPSVResourceBinding(pBindInfo, nullptr, R.get());
    uResIndex++;
  }
  for (auto &&R : pModule->GetUAVs()) {
    DXASSERT_NOMSG(uResIndex < uTotalResources);
    PSVResourceBindInfo0 *pBindInfo = PSV.GetPSVResourceBindInfo0(uResIndex);
    DXASSERT_NOMSG(pBindInfo);
    InitPSVResourceBinding(pBindInfo, nullptr, R.get());
    uResIndex++;
  }
  DXASSERT_NOMSG(uResIndex == uTotalResources);
}

void DxbcConverter::AnalyzeShader(
    D3D10ShaderBinary::CShaderCodeParser &Parser) {
  // Parse shader model.
  D3D10_SB_TOKENIZED_PROGRAM_TYPE ShaderType = Parser.ShaderType();
  m_DxbcMajor = Parser.ShaderMajorVersion();
  m_DxbcMinor = Parser.ShaderMinorVersion();
  ShaderModel::Kind ShaderKind = DXBC::GetShaderModelKind(ShaderType);
  // The converter always promotes the shader version to 6.0.
  m_pSM = ShaderModel::Get(ShaderKind, 6, 0);
  m_pPR->SetShaderModel(m_pSM);

  // By default refactoring is disallowed, unless we encounter
  // dcl_globalflags allowRefactoring
  m_pPR->m_ShaderFlags.SetDisableMathRefactoring(true);
  // By default, all resources are assumed bound for SM5.0 shaders,
  // unless we encounter interface declarations
  m_pPR->m_ShaderFlags.SetAllResourcesBound(true);

  // Setup signature helpers.
  m_pInputSignature.reset(
      new SignatureHelper(m_pSM->GetKind(), DXIL::SignatureKind::Input));
  m_pOutputSignature.reset(
      new SignatureHelper(m_pSM->GetKind(), DXIL::SignatureKind::Output));
  m_pPatchConstantSignature.reset(new SignatureHelper(
      m_pSM->GetKind(), DXIL::SignatureKind::PatchConstOrPrim));

  // Collect:
  //   1. Declarations
  //   2. Labels
  // Declare:
  //   1. Global symbols for resources/samplers.
  //   2. Their types.
  BYTE CurrentOutputStream = 0;
  unsigned MaxOutputRegister = 0;
  m_bControlPointPhase = false;
  bool bPatchConstantPhase = false;
  D3D10ShaderBinary::CInstruction Inst;
  while (!Parser.EndOfShader()) {
    Parser.ParseInstruction(&Inst);

    switch (Inst.OpCode()) {
    case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER: {
      // Record this cbuffer declaration in DxilModule.
      unsigned ID = m_pPR->AddCBuffer(unique_ptr<DxilCBuffer>(new DxilCBuffer));
      DxilCBuffer &R = m_pPR->GetCBuffer(ID); // R == record
      R.SetID(ID);
      // Root signature bindings.
      unsigned RangeID = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      unsigned CBufferSize = Inst.m_ConstantBufferDecl.Size * DXBC::kWidth * 4;
      unsigned LB, RangeSize;
      switch (Inst.m_Operands[0].m_IndexDimension) {
      case D3D10_SB_OPERAND_INDEX_2D: // SM 5.0-
        LB = RangeID;
        RangeSize = 1;
        break;
      case D3D10_SB_OPERAND_INDEX_3D: // SM 5.1
        LB = Inst.m_Operands[0].m_Index[1].m_RegIndex;
        RangeSize = Inst.m_Operands[0].m_Index[2].m_RegIndex != UINT_MAX
                        ? Inst.m_Operands[0].m_Index[2].m_RegIndex - LB + 1
                        : UINT_MAX;
        break;
      default:
        DXASSERT_DXBC(false);
        IFTARG(NULL);
      }
      R.SetLowerBound(LB);
      R.SetRangeSize(RangeSize);
      R.SetSpaceID(Inst.m_ConstantBufferDecl.Space);
      // Declare global variable.
      R.SetGlobalName(SynthesizeResGVName("CB", R.GetID()));
      StructType *pResType = GetStructResElemType(CBufferSize);
      R.SetGlobalSymbol(DeclareUndefPtr(pResType, DXIL::kCBufferAddrSpace));

      // CBuffer-specific state.
      R.SetSize(CBufferSize);
      // R.SetImmediateIndexed(Inst.m_ConstantBufferDecl.AccessPattern ==
      // D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED);

      // Record shader register/rangeID mapping for upcoming instruction
      // conversion.
      DXASSERT(m_CBufferRangeMap.find(RangeID) == m_CBufferRangeMap.end(),
               "otherwise overlapping declarations");
      m_CBufferRangeMap[RangeID] = R.GetID();
      break;
    }

    case D3D10_SB_OPCODE_DCL_SAMPLER: {
      // Record this sampler declaration in DxilModule.
      unsigned ID = m_pPR->AddSampler(unique_ptr<DxilSampler>(new DxilSampler));
      DxilSampler &R = m_pPR->GetSampler(ID); // R == record
      R.SetID(ID);
      // Root signature bindings.
      unsigned RangeID = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      unsigned LB, RangeSize;
      switch (Inst.m_Operands[0].m_IndexDimension) {
      case D3D10_SB_OPERAND_INDEX_1D: // SM 5.0-
        LB = RangeID;
        RangeSize = 1;
        break;
      case D3D10_SB_OPERAND_INDEX_3D: // SM 5.1
        LB = Inst.m_Operands[0].m_Index[1].m_RegIndex;
        RangeSize = Inst.m_Operands[0].m_Index[2].m_RegIndex != UINT_MAX
                        ? Inst.m_Operands[0].m_Index[2].m_RegIndex - LB + 1
                        : UINT_MAX;
        break;
      default:
        DXASSERT_DXBC(false);
        IFTARG(NULL);
      }
      R.SetLowerBound(LB);
      R.SetRangeSize(RangeSize);
      R.SetSpaceID(Inst.m_SamplerDecl.Space);
      // Declare global variable.
      R.SetGlobalName(SynthesizeResGVName("S", R.GetID()));
      string ResTypeName("dx.types.Sampler");
      StructType *pResType = m_pModule->getTypeByName(ResTypeName);
      if (pResType == nullptr) {
        pResType = StructType::create(m_Ctx, ResTypeName);
      }
      R.SetGlobalSymbol(
          DeclareUndefPtr(pResType, DXIL::kDeviceMemoryAddrSpace));

      // Sampler-specific state.
      R.SetSamplerKind(DXBC::GetSamplerKind(Inst.m_SamplerDecl.SamplerMode));

      // Record shader register/rangeID mapping for upcoming instruction
      // conversion.
      DXASSERT(m_SamplerRangeMap.find(RangeID) == m_SamplerRangeMap.end(),
               "otherwise overlapping declarations");
      m_SamplerRangeMap[RangeID] = R.GetID();
      break;
    }

    case D3D10_SB_OPCODE_DCL_RESOURCE:
    case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
    case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED: {
      // Record this SRV declaration in DxilModule.
      unsigned ID = m_pPR->AddSRV(unique_ptr<DxilResource>(new DxilResource));
      DxilResource &R = m_pPR->GetSRV(ID); // R == record
      R.SetID(ID);
      R.SetRW(false);
      // Root signature bindings.
      unsigned RangeID = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      unsigned LB, RangeSize;
      if (IsSM51Plus()) {
        LB = Inst.m_Operands[0].m_Index[1].m_RegIndex;
        RangeSize = Inst.m_Operands[0].m_Index[2].m_RegIndex != UINT_MAX
                        ? Inst.m_Operands[0].m_Index[2].m_RegIndex - LB + 1
                        : UINT_MAX;
      } else {
        LB = RangeID;
        RangeSize = 1;
      }
      R.SetLowerBound(LB);
      R.SetRangeSize(RangeSize);

      // Resource-specific state.
      StructType *pResType = nullptr;
      switch (Inst.OpCode()) {
      case D3D10_SB_OPCODE_DCL_RESOURCE: {
        R.SetSpaceID(Inst.m_ResourceDecl.Space);
        R.SetKind(DXBC::GetResourceKind(Inst.m_ResourceDecl.Dimension));
        const unsigned kTypedBufferElementSizeInBytes = 4;
        R.SetElementStride(kTypedBufferElementSizeInBytes);
        R.SetSampleCount(Inst.m_ResourceDecl.SampleCount);
        CompType DeclCT =
            DXBC::GetDeclResCompType(Inst.m_ResourceDecl.ReturnType[0]);
        if (DeclCT.IsInvalid())
          DeclCT = CompType::getU32();
        R.SetCompType(DeclCT);
        pResType = GetTypedResElemType(DeclCT);
        break;
      }
      case D3D11_SB_OPCODE_DCL_RESOURCE_RAW: {
        R.SetSpaceID(Inst.m_RawSRVDecl.Space);
        R.SetKind(DxilResource::Kind::RawBuffer);
        const unsigned kRawBufferElementSizeInBytes = 1;
        R.SetElementStride(kRawBufferElementSizeInBytes);
        pResType = GetTypedResElemType(CompType::getU32());
        break;
      }
      case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED: {
        R.SetSpaceID(Inst.m_StructuredSRVDecl.Space);
        R.SetKind(DxilResource::Kind::StructuredBuffer);
        unsigned Stride = Inst.m_StructuredSRVDecl.ByteStride;
        R.SetElementStride(Stride);
        pResType = GetStructResElemType(Stride);
        break;
      }
      default:;
      }

      // Declare global variable.
      R.SetGlobalName(SynthesizeResGVName("T", R.GetID()));
      R.SetGlobalSymbol(
          DeclareUndefPtr(pResType, DXIL::kDeviceMemoryAddrSpace));

      // Record shader register/rangeID mapping for upcoming instruction
      // conversion.
      DXASSERT(m_SRVRangeMap.find(RangeID) == m_SRVRangeMap.end(),
               "otherwise overlapping declarations");
      m_SRVRangeMap[RangeID] = R.GetID();
      break;
    }

    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED: {
      // Record this UAV declaration in DxilModule.
      unsigned ID = m_pPR->AddUAV(unique_ptr<DxilResource>(new DxilResource));
      DxilResource &R = m_pPR->GetUAV(ID); // R == record
      R.SetID(ID);
      R.SetRW(true);
      // Root signature bindings.
      unsigned RangeID = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      unsigned LB, RangeSize;
      if (IsSM51Plus()) {
        LB = Inst.m_Operands[0].m_Index[1].m_RegIndex;
        RangeSize = Inst.m_Operands[0].m_Index[2].m_RegIndex != UINT_MAX
                        ? Inst.m_Operands[0].m_Index[2].m_RegIndex - LB + 1
                        : UINT_MAX;
      } else {
        LB = RangeID;
        RangeSize = 1;
      }
      R.SetLowerBound(LB);
      R.SetRangeSize(RangeSize);

      // Resource-specific state.
      string GVTypeName;
      raw_string_ostream GVTypeNameStream(GVTypeName);
      StructType *pResType = nullptr;
      unsigned Flags = 0;
      switch (Inst.OpCode()) {
      case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED: {
        R.SetSpaceID(Inst.m_TypedUAVDecl.Space);
        Flags = Inst.m_TypedUAVDecl.Flags;
        R.SetKind(DXBC::GetResourceKind(Inst.m_TypedUAVDecl.Dimension));
        const unsigned kTypedBufferElementSizeInBytes = 4;
        R.SetElementStride(kTypedBufferElementSizeInBytes);
        CompType DeclCT =
            DXBC::GetDeclResCompType(Inst.m_TypedUAVDecl.ReturnType[0]);
        if (DeclCT.IsInvalid())
          DeclCT = CompType::getU32();
        R.SetCompType(DeclCT);
        pResType = GetTypedResElemType(DeclCT);
        break;
      }
      case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW: {
        R.SetSpaceID(Inst.m_RawUAVDecl.Space);
        R.SetKind(DxilResource::Kind::RawBuffer);
        Flags = Inst.m_RawUAVDecl.Flags;
        const unsigned kRawBufferElementSizeInBytes = 1;
        R.SetElementStride(kRawBufferElementSizeInBytes);
        pResType = GetTypedResElemType(CompType::getU32());
        break;
      }
      case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED: {
        R.SetSpaceID(Inst.m_StructuredUAVDecl.Space);
        R.SetKind(DxilResource::Kind::StructuredBuffer);
        Flags = Inst.m_StructuredUAVDecl.Flags;
        unsigned Stride = Inst.m_StructuredUAVDecl.ByteStride;
        R.SetElementStride(Stride);
        pResType = GetStructResElemType(Stride);
        break;
      }
      default:;
      }

      R.SetGloballyCoherent((Flags & D3D11_SB_GLOBALLY_COHERENT_ACCESS) != 0);
      R.SetHasCounter((Flags & D3D11_SB_UAV_HAS_ORDER_PRESERVING_COUNTER) != 0);
      R.SetROV((Flags & D3D11_SB_RASTERIZER_ORDERED_ACCESS) != 0);

      // Declare global variable.
      R.SetGlobalName(SynthesizeResGVName("U", R.GetID()));
      R.SetGlobalSymbol(
          DeclareUndefPtr(pResType, DXIL::kDeviceMemoryAddrSpace));

      // Record shader register/rangeID mapping for upcoming instruction
      // conversion.
      DXASSERT(m_UAVRangeMap.find(RangeID) == m_UAVRangeMap.end(),
               "otherwise overlapping declarations");
      m_UAVRangeMap[RangeID] = R.GetID();
      break;
    }

    case D3D10_SB_OPCODE_DCL_INDEX_RANGE: {
      unsigned RowRegIdx =
          (Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D)
              ? 0
              : 1;
      SignatureHelper::Range R;
      R.StartRow = Inst.m_Operands[0].m_Index[RowRegIdx].m_RegIndex;
      R.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      R.Rows = Inst.m_IndexRangeDecl.RegCount;
      R.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      R.OutputStream = CurrentOutputStream;

      switch (Inst.m_Operands[0].m_Type) {
      case D3D10_SB_OPERAND_TYPE_INPUT:
        m_pInputSignature->m_Ranges.emplace_back(R);
        break;
      case D3D10_SB_OPERAND_TYPE_OUTPUT:
        if (!m_pSM->IsHS() || m_bControlPointPhase) {
          m_pOutputSignature->m_Ranges.emplace_back(R);
        } else {
          DXASSERT_NOMSG(m_pSM->IsHS() && bPatchConstantPhase);
          m_pPatchConstantSignature->m_Ranges.emplace_back(R);
        }
        break;
      case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:
        DXASSERT_DXBC(m_pSM->IsHS() || m_pSM->IsDS());
        m_pPatchConstantSignature->m_Ranges.emplace_back(R);
        break;
      case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT:
        DXASSERT_DXBC(m_pSM->IsHS() || m_pSM->IsDS());
        m_pInputSignature->m_Ranges.emplace_back(R);
        break;
      default:
        DXASSERT_DXBC(false);
      }
      break;
    }

    case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
      m_pPR->SetInputPrimitive(
          DXBC::GetInputPrimitive(Inst.m_InputPrimitiveDecl.Primitive));
      break;

    case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
      m_pPR->SetStreamPrimitiveTopology(
          DXBC::GetPrimitiveTopology(Inst.m_OutputTopologyDecl.Topology));
      break;

    case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
      m_pPR->SetMaxVertexCount(
          Inst.m_GSMaxOutputVertexCountDecl.MaxOutputVertexCount);
      break;

    case D3D10_SB_OPCODE_DCL_INPUT: {
      D3D10_SB_OPERAND_TYPE RegType = Inst.m_Operands[0].m_Type;
      switch (RegType) {
      case D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK:
        m_pInputSignature->m_bHasInputCoverage = true;
        break;

      case D3D11_SB_OPERAND_TYPE_INNER_COVERAGE:
        m_pInputSignature->m_bHasInnerInputCoverage = true;
        break;

      case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID:
      case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:
      case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP:
      case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED:
      case D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT:
      case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:
      case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:
      case D3D11_SB_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID:
      case D3D11_SB_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID:
      case D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER:
      case D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:
        break;

      default: {
        unsigned NumUnits = 0, Row = 0;
        switch (Inst.m_Operands[0].m_IndexDimension) {
        case D3D10_SB_OPERAND_INDEX_1D:
          NumUnits = 0;
          Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
          break;

        case D3D10_SB_OPERAND_INDEX_2D:
          NumUnits = Inst.m_Operands[0].m_Index[0].m_RegIndex;
          Row = Inst.m_Operands[0].m_Index[1].m_RegIndex;
          break;

        default:
          DXASSERT(false, "there should no other index dimensions");
        }

        SignatureHelper::UsedElement E;
        E.NumUnits = NumUnits;
        E.Row = Row;
        E.StartCol = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                         .GetFirstActiveComp();
        E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                     .GetNumActiveRangeComps();
        E.InterpolationMode = D3D_INTERPOLATION_UNDEFINED;
        E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

        if (RegType == D3D10_SB_OPERAND_TYPE_INPUT) {
          m_pInputSignature->m_UsedElements.emplace_back(E);
        } else {
          if (m_pSM->IsDS()) {
            switch (RegType) {
            case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT:
              m_pInputSignature->m_UsedElements.emplace_back(E);
              break;
            case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:
              m_pPatchConstantSignature->m_UsedElements.emplace_back(E);
              break;
            default:
              DXASSERT(false, "check unsupported case");
              break;
            }
          }

          if (m_pSM->IsHS()) {
            switch (RegType) {
            case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT:
              m_pInputSignature->m_UsedElements.emplace_back(E);
              break;
            case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:
              break;
            case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT:
              break;
            default:
              DXASSERT(false, "check unsupported case");
              break;
            }
          }
        }

        break;
      }
      }

      break;
    }

    case D3D10_SB_OPCODE_DCL_INPUT_SGV: {
      SignatureHelper::UsedElement E;
      E.Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      E.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      E.InterpolationMode = D3D_INTERPOLATION_UNDEFINED;
      E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

      m_pInputSignature->m_UsedElements.emplace_back(E);
      break;
    }

    case D3D10_SB_OPCODE_DCL_INPUT_SIV: {
      unsigned NumUnits = 0;
      unsigned Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      if (m_pSM->IsGS()) {
        NumUnits = Inst.m_Operands[0].m_Index[0].m_RegIndex;
        Row = Inst.m_Operands[0].m_Index[1].m_RegIndex;
      }

      SignatureHelper::UsedElement E;
      E.NumUnits = NumUnits;
      E.Row = Row;
      E.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      E.InterpolationMode = D3D_INTERPOLATION_UNDEFINED;
      E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

      switch (Inst.m_Operands[0].m_Type) {
      case D3D10_SB_OPERAND_TYPE_INPUT:
        m_pInputSignature->m_UsedElements.emplace_back(E);
        break;

      case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:
        m_pPatchConstantSignature->m_UsedElements.emplace_back(E);
        break;

      default:
        DXASSERT(false, "missing case");
        break;
      }

      break;
    }

    case D3D10_SB_OPCODE_DCL_INPUT_PS: {
      SignatureHelper::UsedElement E;
      E.Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      E.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      E.InterpolationMode =
          (D3D_INTERPOLATION_MODE)Inst.m_InputPSDecl.InterpolationMode;
      E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

      m_pInputSignature->m_UsedElements.emplace_back(E);
      break;
    }

    case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV: {
      SignatureHelper::UsedElement E;
      E.Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      E.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      E.InterpolationMode =
          (D3D_INTERPOLATION_MODE)Inst.m_InputPSDeclSGV.InterpolationMode;
      E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

      m_pInputSignature->m_UsedElements.emplace_back(E);
      break;
    }

    case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV: {
      SignatureHelper::UsedElement E;
      E.Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      E.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      E.InterpolationMode =
          (D3D_INTERPOLATION_MODE)Inst.m_InputPSDeclSIV.InterpolationMode;
      E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

      m_pInputSignature->m_UsedElements.emplace_back(E);
      break;
    }

    case D3D10_SB_OPCODE_DCL_OUTPUT: {
      D3D10_SB_OPERAND_TYPE RegType = Inst.m_Operands[0].m_Type;
      switch (RegType) {
      case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:
      case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL:
      case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:
        m_DepthRegType = RegType;
        LLVM_FALLTHROUGH;
      case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:
      case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK: {
        m_bHasStencilRef = RegType == D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF;
        m_bHasCoverageOut =
            RegType == D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK;

        SignatureHelper::UsedElement E;
        E.Row = Semantic::kUndefinedRow;
        E.StartCol = 0;
        E.Cols = 1;
        E.InterpolationMode = D3D_INTERPOLATION_UNDEFINED;
        E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;

        m_pOutputSignature->m_UsedElements.emplace_back(E);
        break;
      }

      default: {
        SignatureHelper::UsedElement E;
        E.Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
        E.StartCol = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                         .GetFirstActiveComp();
        E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                     .GetNumActiveRangeComps();
        E.InterpolationMode = D3D_INTERPOLATION_UNDEFINED;
        E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;
        E.OutputStream = CurrentOutputStream;

        if (!m_pSM->IsHS() || m_bControlPointPhase) {
          m_pOutputSignature->m_UsedElements.emplace_back(E);
        } else {
          DXASSERT_NOMSG(m_pSM->IsHS() && bPatchConstantPhase);
          m_pPatchConstantSignature->m_UsedElements.emplace_back(E);
        }

        MaxOutputRegister = std::max(MaxOutputRegister, E.Row);
        break;
      }
      }

      break;
    }

    case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
    case D3D10_SB_OPCODE_DCL_OUTPUT_SIV: {
      SignatureHelper::UsedElement E;
      E.Row = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      E.StartCol =
          CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask).GetFirstActiveComp();
      E.Cols = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask)
                   .GetNumActiveRangeComps();
      E.InterpolationMode = D3D_INTERPOLATION_UNDEFINED;
      E.MinPrecision = Inst.m_Operands[0].m_MinPrecision;
      E.OutputStream = CurrentOutputStream;

      if (!m_pSM->IsHS() || m_bControlPointPhase) {
        m_pOutputSignature->m_UsedElements.emplace_back(E);
      } else {
        DXASSERT_NOMSG(m_pSM->IsHS() && bPatchConstantPhase);
        m_pPatchConstantSignature->m_UsedElements.emplace_back(E);
      }

      MaxOutputRegister = std::max(MaxOutputRegister, E.Row);
      break;
    }

    case D3D10_SB_OPCODE_DCL_TEMPS:
      m_NumTempRegs = std::max(m_NumTempRegs, Inst.m_TempsDecl.NumTemps);
      break;

    case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP: {
      // Record x-register.
      unsigned Reg = Inst.m_IndexableTempDecl.IndexableTempNumber;
      unsigned NumRegs = Inst.m_IndexableTempDecl.NumRegisters;
      CMask Mask = CMask::FromDXBC(Inst.m_IndexableTempDecl.Mask);
      IndexableReg IR = {nullptr, nullptr, NumRegs,
                         Mask.GetNumActiveRangeComps(), true};

      if (!bPatchConstantPhase) {
        // This is the main shader.
        DXASSERT_DXBC(m_IndexableRegs.find(Reg) == m_IndexableRegs.end());
        m_IndexableRegs[Reg] = IR;
      } else {
        // This is patch constant function.
        // Can have dcl per phase
        auto itIR = m_PatchConstantIndexableRegs.find(Reg);
        if (itIR != m_PatchConstantIndexableRegs.end()) {
          auto &theIR = itIR->second;
          theIR.NumComps = std::max(theIR.NumComps, IR.NumComps);
          theIR.NumComps = std::max(theIR.NumRegs, IR.NumRegs);
        } else {
          m_PatchConstantIndexableRegs[Reg] = IR;
        }
      }
      break;
    }

    case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
      SetShaderGlobalFlags(Inst.m_GlobalFlagsDecl.Flags);
      break;

    case D3D11_SB_OPCODE_DCL_STREAM: {
      BYTE Stream = (BYTE)Inst.m_Operands[0].m_Index[0].m_RegIndex;
      IFTBOOL(Stream < DXIL::kNumOutputStreams, DXC_E_INCORRECT_DXBC);
      CurrentOutputStream = Stream;
      m_pPR->SetStreamActive(Stream, true);
      break;
    }

    case D3D11_SB_OPCODE_HS_DECLS:
      break;

    case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
      m_pPR->SetInputControlPointCount(
          Inst.m_InputControlPointCountDecl.InputControlPointCount);
      break;

    case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
      m_pPR->SetOutputControlPointCount(
          Inst.m_OutputControlPointCountDecl.OutputControlPointCount);
      break;

    case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
      m_pPR->SetTessellatorDomain(DXBC::GetTessellatorDomain(
          Inst.m_TessellatorDomainDecl.TessellatorDomain));
      break;

    case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
      m_pPR->SetTessellatorPartitioning(DXBC::GetTessellatorPartitioning(
          Inst.m_TessellatorPartitioningDecl.TessellatorPartitioning));
      break;

    case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
      m_pPR->SetTessellatorOutputPrimitive(DXBC::GetTessellatorOutputPrimitive(
          Inst.m_TessellatorOutputPrimitiveDecl.TessellatorOutputPrimitive));
      break;

    case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:
      m_pPR->SetMaxTessellationFactor(Inst.m_HSMaxTessFactorDecl.MaxTessFactor);
      break;

    case D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE:
      DXASSERT_NOMSG(!m_bControlPointPhase && !bPatchConstantPhase);
      m_bControlPointPhase = true;
      break;

    case D3D11_SB_OPCODE_HS_FORK_PHASE:
    case D3D11_SB_OPCODE_HS_JOIN_PHASE:
      m_bControlPointPhase = false;
      bPatchConstantPhase = true;
      m_PatchConstantPhaseInstanceCounts.push_back(1);
      break;

    case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
      m_PatchConstantPhaseInstanceCounts.back() =
          Inst.m_HSForkPhaseInstanceCountDecl.InstanceCount;
      break;

    case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
      m_PatchConstantPhaseInstanceCounts.back() =
          Inst.m_HSJoinPhaseInstanceCountDecl.InstanceCount;
      break;

    case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
      m_pPR->SetNumThreads(Inst.m_ThreadGroupDecl.x, Inst.m_ThreadGroupDecl.y,
                           Inst.m_ThreadGroupDecl.z);
      break;

    case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
    case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED: {
      TGSMEntry E;
      E.Id = m_TGSMCount++;

      if (Inst.OpCode() == D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW) {
        E.Stride = 1;
        E.Count = Inst.m_RawTGSMDecl.ByteCount;
      } else {
        E.Stride = Inst.m_StructuredTGSMDecl.StructByteStride;
        E.Count = Inst.m_StructuredTGSMDecl.StructCount;
      }

      // Declare global variable.
      unsigned SizeInBytes = E.Stride * E.Count;
      Type *pArrayType = ArrayType::get(Type::getInt8Ty(m_Ctx), SizeInBytes);
      E.pVar = new GlobalVariable(
          *m_pModule, pArrayType, false, GlobalValue::InternalLinkage,
          UndefValue::get(pArrayType), Twine("TGSM") + Twine(E.Id), nullptr,
          GlobalVariable::NotThreadLocal, DXIL::kTGSMAddrSpace);
      E.pVar->setAlignment(kRegCompAlignment);

      // Mark GV as being used for LLVM.
      m_pPR->GetLLVMUsed().push_back(E.pVar);

      m_TGSMMap[Inst.m_Operands[0].m_Index[0].m_RegIndex] = E;
      break;
    }

    case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
      m_pPR->SetGSInstanceCount(Inst.m_GSInstanceCountDecl.InstanceCount);
      break;

    case D3D10_SB_OPCODE_CUSTOMDATA:
      break;

    case D3D11_SB_OPCODE_DCL_FUNCTION_BODY: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      unsigned FBIdx = Inst.m_FunctionBodyDecl.FunctionBodyNumber;
      m_InterfaceFunctionBodies[FBIdx].pFunc = nullptr;
      break;
    }

    case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      auto &FnTable =
          m_FunctionTables[Inst.m_FunctionTableDecl.FunctionTableNumber];
      FnTable.assign(Inst.m_FunctionTableDecl.pFunctionIdentifiers,
                     Inst.m_FunctionTableDecl.pFunctionIdentifiers +
                         Inst.m_FunctionTableDecl.TableLength);
      break;
    }

    case D3D11_SB_OPCODE_DCL_INTERFACE: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      auto &Iface = m_Interfaces[Inst.m_InterfaceDecl.InterfaceNumber];
      Iface.Tables.assign(Inst.m_InterfaceDecl.pFunctionTableIdentifiers,
                          Inst.m_InterfaceDecl.pFunctionTableIdentifiers +
                              Inst.m_InterfaceDecl.TableLength);
#ifndef NDEBUG
      for (unsigned TableIdx : Iface.Tables) {
        DXASSERT_DXBC(m_FunctionTables[TableIdx].size() ==
                      Inst.m_InterfaceDecl.ExpectedTableSize);
      }
#endif
      Iface.bDynamicallyIndexed = Inst.m_InterfaceDecl.bDynamicallyIndexed;
      Iface.NumArrayEntries = Inst.m_InterfaceDecl.ArrayLength;
      m_NumIfaces = std::max(m_NumIfaces, Inst.m_InterfaceDecl.InterfaceNumber +
                                              Iface.NumArrayEntries);
      InsertInterfacesResourceDecls();
      break;
    }

    case D3D10_SB_OPCODE_LABEL: {
      m_bControlPointPhase = false;
      bPatchConstantPhase = false;
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      DXASSERT_DXBC(Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_LABEL ||
                    Inst.m_Operands[0].m_Type ==
                        D3D11_SB_OPERAND_TYPE_FUNCTION_BODY);
      FunctionType *pFuncType =
          FunctionType::get(Type::getVoidTy(m_Ctx), false);
      unsigned LabelIdx = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      LabelEntry Label;
      const bool IsFb =
          Inst.m_Operands[0].m_Type == D3D11_SB_OPERAND_TYPE_FUNCTION_BODY;
      auto &LabelMap = IsFb ? m_InterfaceFunctionBodies : m_Labels;
      DXASSERT_DXBC(
          (LabelMap.find(LabelIdx) == LabelMap.end()) ==
          !IsFb); // Function bodies should be pre-declared, labels aren't
      Label.pFunc = Function::Create(
          pFuncType, GlobalValue::LinkageTypes::InternalLinkage,
          StringRef(IsFb ? "dx.fb." : "dx.label.") + Twine(LabelIdx),
          m_pModule.get());
      Label.pFunc->setCallingConv(CallingConv::C);
      LabelMap[LabelIdx] = Label;
      break;
    }

    default:
      break;
    }
  }
}

void DxbcConverter::ConvertInstructions(
    D3D10ShaderBinary::CShaderCodeParser &Parser) {
  if (m_pPR->GetShaderModel()->IsGS()) {
    // Set GS active stream mask.
    if (m_pPR->GetActiveStreamMask() == 0 &&
        !m_pPR->GetOutputSignature().GetElements().empty())
      m_pPR->SetStreamActive(0, true);

    // Make sure GS instance count is at least 1
    if (m_pPR->GetGSInstanceCount() == 0)
      m_pPR->SetGSInstanceCount(1);
  }

  // Add entry function declaration.
  m_pPR->SetEntryFunctionName("main");
  FunctionType *pEntryFuncType =
      FunctionType::get(Type::getVoidTy(m_Ctx), false);
  Function *pFunction = Function::Create(
      pEntryFuncType, GlobalValue::LinkageTypes::ExternalLinkage,
      m_pPR->GetEntryFunctionName(), m_pModule.get());
  pFunction->setCallingConv(CallingConv::C);
  m_pPR->SetEntryFunction(pFunction);

  // Create main entry function.
  BasicBlock *pBB = BasicBlock::Create(m_Ctx, "entry", pFunction);
  m_pBuilder = std::make_unique<IRBuilder<>>(pBB);

  FastMathFlags FMF;
  if (!m_pPR->m_ShaderFlags.GetDisableMathRefactoring()) {
    FMF.setUnsafeAlgebra();
  }
  m_pBuilder->SetFastMathFlags(FMF);

  // Empty instruction stream.
  if (Parser.EndOfShader()) {
    m_pBuilder->CreateRetVoid();
    return;
  }

  m_pUnusedF32 = UndefValue::get(Type::getFloatTy(m_Ctx));
  m_pUnusedI32 = UndefValue::get(Type::getInt32Ty(m_Ctx));

  // Create entry function scope.
  DXASSERT_NOMSG(m_ScopeStack.IsEmpty());
  (void)m_ScopeStack.Push(Scope::Function, nullptr);
  m_ScopeStack.Top().SetEntry(true);

  DeclareIndexableRegisters();

  // Parse DXBC instructions and emit DXIL equivalents.
  Value *pHullLoopInductionVar = nullptr;
  m_bControlPointPhase = false;
  bool bMustCloseHullLoop = false;
  m_bPatchConstantPhase = false;
  bool bInsertResourceHandles = true;
  unsigned ForkJoinPhaseIndex = 0;
  D3D10ShaderBinary::CInstruction Inst;
  bool bPasshThroughCP = false;
  bool bDoneParsing = false;
  for (;;) {
    AdvanceDxbcInstructionStream(Parser, Inst, bDoneParsing);

    // Terminate HS phase (HullLoop), if necessary.
    if (m_bPatchConstantPhase) {
      bool bTerminateHullLoop = false;

      if (bDoneParsing || bMustCloseHullLoop) {
        bTerminateHullLoop = true;
      } else {
        switch (Inst.OpCode()) {
        case D3D11_SB_OPCODE_HS_FORK_PHASE:
        case D3D11_SB_OPCODE_HS_JOIN_PHASE:
        case D3D10_SB_OPCODE_LABEL:
          bTerminateHullLoop = true;
          break;
        }
      }

      if (bTerminateHullLoop) {
        IFTBOOL(m_ScopeStack.Top().Kind == Scope::HullLoop, E_FAIL);
        // Hull shader control point phase fork/join.
        Scope &HullScope = m_ScopeStack.Top();

        // Increment HullLoop instance ID.
        Value *pOldInstID = m_pBuilder->CreateLoad(HullScope.pInductionVar);
        Value *pNewInstID =
            m_pBuilder->CreateAdd(pOldInstID, m_pOP->GetU32Const(1));
        (void)m_pBuilder->CreateStore(pNewInstID, HullScope.pInductionVar);

        // Insert backedge cbranch to HullLoop and AfterHullLoop BBs.
        Value *pCond = m_pBuilder->CreateICmpULT(
            pNewInstID, m_pOP->GetU32Const(HullScope.HullLoopTripCount));
        m_pBuilder->CreateCondBr(pCond, HullScope.pHullLoopBB,
                                 HullScope.pPostScopeBB);
        m_pPR->GetPatchConstantFunction()->getBasicBlockList().push_back(
            HullScope.pPostScopeBB);
        m_pBuilder->SetInsertPoint(HullScope.pPostScopeBB);
        m_ScopeStack.Pop();

        // Skip dead instructions to the next phase, label or EOS.
        for (; !bDoneParsing;) {
          if (Inst.OpCode() == D3D11_SB_OPCODE_HS_FORK_PHASE ||
              Inst.OpCode() == D3D11_SB_OPCODE_HS_JOIN_PHASE ||
              Inst.OpCode() == D3D10_SB_OPCODE_LABEL)
            break;

          AdvanceDxbcInstructionStream(Parser, Inst, bDoneParsing);
        }
      }

      bMustCloseHullLoop = false;
    }

    // Terminate function, if necessary.
    {
      bool bTerminateFunc = false;
      if (bDoneParsing) {
        bTerminateFunc = true;
      } else {
        switch (Inst.OpCode()) {
        case D3D11_SB_OPCODE_HS_FORK_PHASE:
        case D3D11_SB_OPCODE_HS_JOIN_PHASE:
          if (!m_bPatchConstantPhase)
            bTerminateFunc = true;
          break;
        case D3D10_SB_OPCODE_LABEL:
          bTerminateFunc = true;
          break;
        }
      }

      if (bTerminateFunc) {
        Scope &Scope = m_ScopeStack.FindParentFunction();
        IFTBOOL(Scope.Kind == Scope::Function, DXC_E_INCORRECT_DXBC);
        m_pBuilder->CreateRetVoid();
        m_ScopeStack.Pop();
        IFT(m_ScopeStack.IsEmpty());
        m_bPatchConstantPhase = false;
      }
    }

    if (bDoneParsing)
      break;

    m_PreciseMask = CMask(Inst.GetPreciseMask());

    // Fix up output register masks.
    // DXBC instruction conversion relies on the output mask(s) determining
    // what components need to be written.
    // Some output operand types have write mask that is 0 -- fix this.
    for (unsigned i = 0; i < std::min(Inst.m_NumOperands, (UINT)2); i++) {
      D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[i];
      switch (O.m_Type) {
      case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:
      case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL:
      case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:
      case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:
      case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK:
        DXASSERT_DXBC(O.m_WriteMask == 0);
        O.SetMask(D3D10_SB_OPERAND_4_COMPONENT_MASK_X);
        break;
      }
    }

    if (bInsertResourceHandles) {
      InsertSM50ResourceHandles();
      bInsertResourceHandles = false;
    }

    switch (Inst.OpCode()) {
      //
      // Declarations.
      //
    case D3D10_SB_OPCODE_DCL_RESOURCE:
    case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
    case D3D10_SB_OPCODE_DCL_SAMPLER:
    case D3D10_SB_OPCODE_DCL_INDEX_RANGE:
    case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
    case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
    case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
    case D3D10_SB_OPCODE_DCL_INPUT:
    case D3D10_SB_OPCODE_DCL_INPUT_SGV:
    case D3D10_SB_OPCODE_DCL_INPUT_SIV:
    case D3D10_SB_OPCODE_DCL_INPUT_PS:
    case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
    case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
    case D3D10_SB_OPCODE_DCL_OUTPUT:
    case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
    case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
    case D3D10_SB_OPCODE_DCL_TEMPS:
    case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
      break;

    case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
      break;

    case D3D11_SB_OPCODE_DCL_STREAM:
    case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
    case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
    case D3D11_SB_OPCODE_DCL_INTERFACE:

    case D3D11_SB_OPCODE_HS_DECLS:

    case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
    case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
    case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
    case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
    case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
    case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:

    case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
    case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
    case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
    case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
    case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:

    case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
      break;

      //
      // Immediate constant buffer.
      //
    case D3D10_SB_OPCODE_CUSTOMDATA:
      if (Inst.m_CustomData.Type ==
          D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER) {
        unsigned Size = Inst.m_CustomData.DataSizeInBytes >> 2;
        DXASSERT_DXBC(m_pIcbGV == nullptr &&
                      Inst.m_CustomData.DataSizeInBytes == Size * 4);

        llvm::Constant *pIcbData = ConstantDataArray::get(
            m_Ctx, ArrayRef<float>((float *)Inst.m_CustomData.pData, Size));
        m_pIcbGV = new GlobalVariable(
            *m_pModule, pIcbData->getType(), true, GlobalValue::InternalLinkage,
            pIcbData, "dx.icb", nullptr, GlobalVariable::NotThreadLocal,
            DXIL::kImmediateCBufferAddrSpace);
      }
      break;

      //
      // Mov, movc, swapc, dmov, dmovc.
      //
    case D3D10_SB_OPCODE_MOV: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType DstType = InferOperandType(Inst, 0, WriteMask);
      CompType SrcType = InferOperandType(Inst, 1, WriteMask);

      // For mov, movc, and swapc, use integer operation type unless
      // operand modifiers imply floating point.
      CompType OperationType = CompType::getI32();

      if (!DstType.IsInvalid())
        OperationType = DstType.GetBaseCompType();
      else if (!SrcType.IsInvalid())
        OperationType = SrcType;

      if (Inst.m_Operands[1].Modifier() != D3D10_SB_OPERAND_MODIFIER_NONE ||
          Inst.m_bSaturate) {
        OperationType = CompType::getF32();
      }

      OperandValue In;
      LoadOperand(In, Inst, 1, WriteMask, OperationType);
      StoreOperand(In, Inst, 0, WriteMask, OperationType);
      break;
    }

    case D3D10_SB_OPCODE_MOVC: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType DstType = InferOperandType(Inst, 0, WriteMask);
      CompType Src2Type = InferOperandType(Inst, 2, WriteMask);
      CompType Src3Type = InferOperandType(Inst, 3, WriteMask);

      CompType OperationType = CompType::getI32();

      if (Src2Type == Src3Type && !Src2Type.IsInvalid())
        OperationType = Src2Type;
      else if (!DstType.IsInvalid())
        OperationType = DstType.GetBaseCompType();
      else if (!Src2Type.IsInvalid())
        OperationType = Src2Type;
      else if (!Src3Type.IsInvalid())
        OperationType = Src3Type;

      if (Inst.m_Operands[2].Modifier() != D3D10_SB_OPERAND_MODIFIER_NONE ||
          Inst.m_Operands[3].Modifier() != D3D10_SB_OPERAND_MODIFIER_NONE ||
          Inst.m_bSaturate) {
        OperationType = CompType::getF32();
      }

      OperandValue In1, In2, In3, Out;
      LoadOperand(In1, Inst, 1, WriteMask, CompType::getI1());
      LoadOperand(In2, Inst, 2, WriteMask, OperationType);
      LoadOperand(In3, Inst, 3, WriteMask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!WriteMask.IsSet(c))
          continue;
        Out[c] = m_pBuilder->CreateSelect(In1[c], In2[c], In3[c]);
      }

      StoreOperand(Out, Inst, 0, WriteMask, OperationType);
      break;
    }

    case D3D11_SB_OPCODE_SWAPC: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask |
                                        Inst.m_Operands[1].m_WriteMask);
      CMask Dst1Mask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CMask Dst2Mask = CMask::FromDXBC(Inst.m_Operands[1].m_WriteMask);
      CompType Dst1Type = InferOperandType(Inst, 0, WriteMask);
      CompType Dst2Type = InferOperandType(Inst, 1, WriteMask);
      CompType Src2Type = InferOperandType(Inst, 3, WriteMask);
      CompType Src3Type = InferOperandType(Inst, 4, WriteMask);

      CompType OperationType = CompType::getI32();

      if (Src2Type == Src3Type && !Src2Type.IsInvalid())
        OperationType = Src2Type;
      else if (!Dst1Type.IsInvalid())
        OperationType = Dst1Type.GetBaseCompType();
      else if (!Dst2Type.IsInvalid())
        OperationType = Dst2Type.GetBaseCompType();
      else if (!Src2Type.IsInvalid())
        OperationType = Src2Type;
      else if (!Src3Type.IsInvalid())
        OperationType = Src3Type;

      if (Inst.m_Operands[3].Modifier() != D3D10_SB_OPERAND_MODIFIER_NONE ||
          Inst.m_Operands[4].Modifier() != D3D10_SB_OPERAND_MODIFIER_NONE ||
          Inst.m_bSaturate) {
        OperationType = CompType::getF32();
      }

      OperandValue In1, In2, In3, Out1, Out2;
      LoadOperand(In1, Inst, 2, WriteMask, CompType::getI1());
      LoadOperand(In2, Inst, 3, WriteMask, OperationType);
      LoadOperand(In3, Inst, 4, WriteMask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!Dst1Mask.IsSet(c))
          continue;
        Out1[c] = m_pBuilder->CreateSelect(In1[c], In3[c], In2[c]);
      }
      StoreOperand(Out1, Inst, 0, Dst1Mask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!Dst2Mask.IsSet(c))
          continue;
        Out2[c] = m_pBuilder->CreateSelect(In1[c], In2[c], In3[c]);
      }
      StoreOperand(Out2, Inst, 1, Dst2Mask, OperationType);

      break;
    }

      //
      // Floating point unary.
      //
    case D3D10_SB_OPCODE_EXP:
      ConvertUnary(OP::OpCode::Exp, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_FRC:
      ConvertUnary(OP::OpCode::Frc, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_LOG:
      ConvertUnary(OP::OpCode::Log, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_SQRT:
      ConvertUnary(OP::OpCode::Sqrt, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_RSQ:
      ConvertUnary(OP::OpCode::Rsqrt, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_ROUND_NE:
      ConvertUnary(OP::OpCode::Round_ne, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_ROUND_NI:
      ConvertUnary(OP::OpCode::Round_ni, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_ROUND_PI:
      ConvertUnary(OP::OpCode::Round_pi, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_ROUND_Z:
      ConvertUnary(OP::OpCode::Round_z, CompType::getF32(), Inst);
      break;

    case D3D11_SB_OPCODE_RCP: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType OperationType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[0].m_MinPrecision);

      OperandValue In, Out;
      LoadOperand(In, Inst, 1, WriteMask, OperationType);
      Value *One = m_pOP->GetFloatConst(1.0f);
      if (OperationType.Is16Bit())
        One = ConstantFP::get(m_pBuilder->getHalfTy(), 1.0);
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!WriteMask.IsSet(c))
          continue;

        Out[c] =
            m_pBuilder->CreateBinOp(Instruction::BinaryOps::FDiv, One, In[c]);
      }

      StoreOperand(Out, Inst, 0, WriteMask, OperationType);
      break;
    }

    case D3D10_SB_OPCODE_SINCOS: {
      CMask WriteMaskSin;
      CMask WriteMaskCos;
      CompType OperationType;
      if (Inst.m_Operands[0].m_Type != D3D10_SB_OPERAND_TYPE_NULL) {
        WriteMaskSin = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
        OperationType = DXBC::GetCompTypeWithMinPrec(
            CompType::getF32(), Inst.m_Operands[0].m_MinPrecision);
      }
      if (Inst.m_Operands[1].m_Type != D3D10_SB_OPERAND_TYPE_NULL) {
        WriteMaskCos = CMask::FromDXBC(Inst.m_Operands[1].m_WriteMask);
        CompType OperationTypeCos = DXBC::GetCompTypeWithMinPrec(
            CompType::getF32(), Inst.m_Operands[1].m_MinPrecision);
        DXASSERT_DXBC(OperationType.GetKind() == CompType::Kind::Invalid ||
                      OperationType == OperationTypeCos);
        OperationType = OperationTypeCos;
      }
      CMask WriteMaskAll = WriteMaskSin | WriteMaskCos;
      Type *pOperationType = OperationType.GetLLVMType(m_Ctx);

      OperandValue In;
      LoadOperand(In, Inst, 2, WriteMaskAll, OperationType);

      if (Inst.m_Operands[0].m_Type != D3D10_SB_OPERAND_TYPE_NULL) {
        OperandValue Out;
        Function *pFunc = m_pOP->GetOpFunc(OP::OpCode::Sin, pOperationType);
        for (BYTE c = 0; c < DXBC::kWidth; c++) {
          if (!WriteMaskSin.IsSet(c))
            continue;

          Out[c] = m_pBuilder->CreateCall(
              pFunc, {m_pOP->GetU32Const((unsigned)OP::OpCode::Sin), In[c]});
        }

        StoreOperand(Out, Inst, 0, WriteMaskSin, OperationType);
      }
      if (Inst.m_Operands[1].m_Type != D3D10_SB_OPERAND_TYPE_NULL) {
        OperandValue Out;
        Function *pFunc = m_pOP->GetOpFunc(OP::OpCode::Cos, pOperationType);
        for (BYTE c = 0; c < DXBC::kWidth; c++) {
          if (!WriteMaskCos.IsSet(c))
            continue;

          Out[c] = m_pBuilder->CreateCall(
              pFunc, {m_pOP->GetU32Const((unsigned)OP::OpCode::Cos), In[c]});
        }

        StoreOperand(Out, Inst, 1, WriteMaskCos, OperationType);
      }
      break;
    }

      //
      // Integer unary.
      //
    case D3D11_SB_OPCODE_BFREV:
      ConvertUnary(OP::OpCode::Bfrev, CompType::getU32(), Inst);
      break;
    case D3D11_SB_OPCODE_COUNTBITS:
      ConvertUnary(OP::OpCode::Countbits, CompType::getU32(), Inst);
      break;
    case D3D11_SB_OPCODE_FIRSTBIT_HI:
      ConvertUnary(OP::OpCode::FirstbitHi, CompType::getU32(), Inst);
      break;
    case D3D11_SB_OPCODE_FIRSTBIT_LO:
      ConvertUnary(OP::OpCode::FirstbitLo, CompType::getU32(), Inst);
      break;
    case D3D11_SB_OPCODE_FIRSTBIT_SHI:
      ConvertUnary(OP::OpCode::FirstbitSHi, CompType::getI32(), Inst);
      break;

    case D3D10_SB_OPCODE_INEG: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType OperationType = DXBC::GetCompTypeWithMinPrec(
          CompType::getI32(), Inst.m_Operands[0].m_MinPrecision);

      OperandValue In, Out;
      LoadOperand(In, Inst, 1, WriteMask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!WriteMask.IsSet(c))
          continue;

        Out[c] = m_pBuilder->CreateNeg(In[c]);
      }

      StoreOperand(Out, Inst, 0, WriteMask, OperationType);
      break;
    }

    case D3D10_SB_OPCODE_NOT: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType OperationType = DXBC::GetCompTypeWithMinPrec(
          CompType::getI32(), Inst.m_Operands[0].m_MinPrecision);

      OperandValue In, Out;
      LoadOperand(In, Inst, 1, WriteMask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!WriteMask.IsSet(c))
          continue;

        Out[c] = m_pBuilder->CreateNot(In[c]);
      }

      StoreOperand(Out, Inst, 0, WriteMask, OperationType);
      break;
    }

      //
      // Floating point binary.
      //
    case D3D10_SB_OPCODE_ADD:
      ConvertBinary(Instruction::FAdd, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_MUL:
      ConvertBinary(Instruction::FMul, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_DIV:
      ConvertBinary(Instruction::FDiv, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_MAX:
      ConvertBinary(OP::OpCode::FMax, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_MIN:
      ConvertBinary(OP::OpCode::FMin, CompType::getF32(), Inst);
      break;

      //
      // Integer binary.
      //
    case D3D10_SB_OPCODE_IADD:
      ConvertBinary(Instruction::Add, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_IMAX:
      ConvertBinary(OP::OpCode::IMax, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_IMIN:
      ConvertBinary(OP::OpCode::IMin, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_UMAX:
      ConvertBinary(OP::OpCode::UMax, CompType::getU32(), Inst);
      break;
    case D3D10_SB_OPCODE_UMIN:
      ConvertBinary(OP::OpCode::UMin, CompType::getU32(), Inst);
      break;

    case D3D10_SB_OPCODE_AND:
      ConvertBinary(Instruction::And, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_OR:
      ConvertBinary(Instruction::Or, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_XOR:
      ConvertBinary(Instruction::Xor, CompType::getI32(), Inst);
      break;

    case D3D10_SB_OPCODE_ISHL:
      ConvertBinary(Instruction::Shl, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_ISHR:
      ConvertBinary(Instruction::AShr, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_USHR:
      ConvertBinary(Instruction::LShr, CompType::getI32(), Inst);
      break;

      //
      // Integer binary with two outputs.
      //
    case D3D10_SB_OPCODE_IMUL:
      ConvertBinaryWithTwoOuts(OP::OpCode::IMul, Inst);
      break;
    case D3D10_SB_OPCODE_UMUL:
      ConvertBinaryWithTwoOuts(OP::OpCode::UMul, Inst);
      break;
    case D3D10_SB_OPCODE_UDIV:
      ConvertBinaryWithTwoOuts(OP::OpCode::UDiv, Inst);
      break;

      //
      // Integer binary with carry.
      //
    case D3D11_SB_OPCODE_UADDC:
      ConvertBinaryWithCarry(OP::OpCode::UAddc, Inst);
      break;
    case D3D11_SB_OPCODE_USUBB:
      ConvertBinaryWithCarry(OP::OpCode::USubb, Inst);
      break;

      //
      // Floating point tertiary.
      //
    case D3D10_SB_OPCODE_MAD:
      ConvertTertiary(OP::OpCode::FMad, CompType::getF32(), Inst);
      break;

      //
      // Integer tertiary.
      //
    case D3D10_SB_OPCODE_IMAD:
      ConvertTertiary(OP::OpCode::IMad, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_UMAD:
      ConvertTertiary(OP::OpCode::UMad, CompType::getI32(), Inst);
      break;
    case D3D11_1_SB_OPCODE_MSAD:
      ConvertTertiary(OP::OpCode::Msad, CompType::getI32(), Inst);
      break;
    case D3D11_SB_OPCODE_IBFE:
      ConvertTertiary(OP::OpCode::Ibfe, CompType::getI32(), Inst);
      break;
    case D3D11_SB_OPCODE_UBFE:
      ConvertTertiary(OP::OpCode::Ubfe, CompType::getI32(), Inst);
      break;

      //
      // Quaternary int.
      //
    case D3D11_SB_OPCODE_BFI:
      ConvertQuaternary(OP::OpCode::Bfi, CompType::getI32(), Inst);
      break;

      //
      // Logical comparison.
      //
    case D3D10_SB_OPCODE_EQ:
      ConvertComparison(CmpInst::FCMP_OEQ, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_NE:
      ConvertComparison(CmpInst::FCMP_UNE, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_LT:
      ConvertComparison(CmpInst::FCMP_OLT, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_GE:
      ConvertComparison(CmpInst::FCMP_OGE, CompType::getF32(), Inst);
      break;

    case D3D10_SB_OPCODE_IEQ:
      ConvertComparison(CmpInst::ICMP_EQ, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_INE:
      ConvertComparison(CmpInst::ICMP_NE, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_ILT:
      ConvertComparison(CmpInst::ICMP_SLT, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_IGE:
      ConvertComparison(CmpInst::ICMP_SGE, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_ULT:
      ConvertComparison(CmpInst::ICMP_ULT, CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_UGE:
      ConvertComparison(CmpInst::ICMP_UGE, CompType::getI32(), Inst);
      break;

      //
      // Dot product.
      //
    case D3D10_SB_OPCODE_DP2:
      ConvertDotProduct(OP::OpCode::Dot2, 2, CMask::MakeMask(1, 1, 0, 0), Inst);
      break;
    case D3D10_SB_OPCODE_DP3:
      ConvertDotProduct(OP::OpCode::Dot3, 3, CMask::MakeMask(1, 1, 1, 0), Inst);
      break;
    case D3D10_SB_OPCODE_DP4:
      ConvertDotProduct(OP::OpCode::Dot4, 4, CMask::MakeMask(1, 1, 1, 1), Inst);
      break;

      //
      // Type conversions.
      //
    case D3D10_SB_OPCODE_ITOF:
      ConvertCast(CompType::getI32(), CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_UTOF:
      ConvertCast(CompType::getU32(), CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_FTOI:
      ConvertCast(CompType::getF32(), CompType::getI32(), Inst);
      break;
    case D3D10_SB_OPCODE_FTOU:
      ConvertCast(CompType::getF32(), CompType::getU32(), Inst);
      break;

    case D3D11_SB_OPCODE_F32TOF16: {
      const unsigned DstIdx = 0;
      const unsigned SrcIdx = 1;
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);

      if (!WriteMask.IsZero()) {
        OperandValue In, Out;
        LoadOperand(In, Inst, SrcIdx, WriteMask, CompType::getF32());

        OP::OpCode OpCode = OP::OpCode::LegacyF32ToF16;
        CompType DstType = CompType::getU32();
        Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));

        for (BYTE c = 0; c < DXBC::kWidth; c++) {
          if (!WriteMask.IsSet(c))
            continue;

          Value *Args[2];
          Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
          Args[1] = In[c];
          Out[c] = m_pBuilder->CreateCall(F, Args);
        }

        StoreOperand(Out, Inst, DstIdx, WriteMask, DstType);
      }
      break;
    }

    case D3D11_SB_OPCODE_F16TOF32: {
      const unsigned DstIdx = 0;
      const unsigned SrcIdx = 1;
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);

      if (!WriteMask.IsZero()) {
        OperandValue In, Out;
        D3D10_SB_OPERAND_MODIFIER SrcModifier =
            Inst.m_Operands[SrcIdx].m_Modifier;
        Inst.m_Operands[SrcIdx].m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        LoadOperand(In, Inst, SrcIdx, WriteMask, CompType::getU32());

        OP::OpCode OpCode = OP::OpCode::LegacyF16ToF32;
        Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));

        for (BYTE c = 0; c < DXBC::kWidth; c++) {
          if (!WriteMask.IsSet(c))
            continue;

          Value *Args[2];
          Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
          Args[1] = In[c];
          Value *pResult = m_pBuilder->CreateCall(F, Args);

          // Special-case: propagate source operand modifiers to result.
          if (SrcModifier & D3D10_SB_OPERAND_MODIFIER_ABS) {
            Function *Fabs =
                m_pOP->GetOpFunc(OP::OpCode::FAbs, pResult->getType());
            Value *Args[2];
            Args[0] = m_pOP->GetU32Const((unsigned)OP::OpCode::FAbs);
            Args[1] = pResult;
            pResult = m_pBuilder->CreateCall(Fabs, Args);
          }
          if (SrcModifier & D3D10_SB_OPERAND_MODIFIER_NEG) {
            pResult =
                MarkPrecise(m_pBuilder->CreateFNeg(MarkPrecise(pResult, c)), c);
          }

          Out[c] = pResult;
        }

        StoreOperand(Out, Inst, DstIdx, WriteMask, CompType::getF32());
      }
      break;
    }

      //
      // Double-precision operations.
      //
    case D3D11_SB_OPCODE_DADD:
      ConvertBinary(Instruction::FAdd, CompType::getF64(), Inst);
      break;
    case D3D11_SB_OPCODE_DMAX:
      ConvertBinary(OP::OpCode::FMax, CompType::getF64(), Inst);
      break;
    case D3D11_SB_OPCODE_DMIN:
      ConvertBinary(OP::OpCode::FMin, CompType::getF64(), Inst);
      break;
    case D3D11_SB_OPCODE_DMUL:
      ConvertBinary(Instruction::FMul, CompType::getF64(), Inst);
      break;
    case D3D11_1_SB_OPCODE_DDIV:
      ConvertBinary(Instruction::FDiv, CompType::getF64(), Inst);
      break;

    case D3D11_1_SB_OPCODE_DFMA:
      ConvertTertiary(OP::OpCode::Fma, CompType::getF64(), Inst);
      break;

    case D3D11_SB_OPCODE_DEQ:
      ConvertComparison(CmpInst::FCMP_OEQ, CompType::getF64(), Inst);
      break;
    case D3D11_SB_OPCODE_DGE:
      ConvertComparison(CmpInst::FCMP_OGE, CompType::getF64(), Inst);
      break;
    case D3D11_SB_OPCODE_DLT:
      ConvertComparison(CmpInst::FCMP_OLT, CompType::getF64(), Inst);
      break;
    case D3D11_SB_OPCODE_DNE:
      ConvertComparison(CmpInst::FCMP_UNE, CompType::getF64(), Inst);
      break;

    case D3D11_SB_OPCODE_DMOV: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      OperandValue In;
      LoadOperand(In, Inst, 1, WriteMask, CompType::getF64());
      StoreOperand(In, Inst, 0, WriteMask, CompType::getF64());
      break;
    }

    case D3D11_SB_OPCODE_DMOVC: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType OperationType = CompType::getF64();
      OperandValue In1, In2, In3, Out;
      LoadOperand(In1, Inst, 1, CMask(1, 1, 0, 0), CompType::getI1());
      LoadOperand(In2, Inst, 2, WriteMask, OperationType);
      LoadOperand(In3, Inst, 3, WriteMask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c += 2) {
        if (!WriteMask.IsSet(c))
          continue;
        Out[c] = m_pBuilder->CreateSelect(In1[c >> 1], In2[c], In3[c]);
      }

      StoreOperand(Out, Inst, 0, WriteMask, OperationType);
      break;
    }

    case D3D11_1_SB_OPCODE_DRCP: {
      CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
      CompType OperationType = CompType::getF64();

      OperandValue In, Out;
      LoadOperand(In, Inst, 1, WriteMask, OperationType);

      for (BYTE c = 0; c < DXBC::kWidth; c += 2) {
        if (!WriteMask.IsSet(c))
          continue;

        Out[c] = m_pBuilder->CreateBinOp(Instruction::BinaryOps::FDiv,
                                         m_pOP->GetDoubleConst(1.0), In[c]);
      }

      StoreOperand(Out, Inst, 0, WriteMask, OperationType);
      break;
    }

    case D3D11_SB_OPCODE_DTOF:
      ConvertFromDouble(CompType::getF32(), Inst);
      break;
    case D3D11_1_SB_OPCODE_DTOI:
      ConvertFromDouble(CompType::getI32(), Inst);
      break;
    case D3D11_1_SB_OPCODE_DTOU:
      ConvertFromDouble(CompType::getU32(), Inst);
      break;
    case D3D11_SB_OPCODE_FTOD:
      ConvertToDouble(CompType::getF32(), Inst);
      break;
    case D3D11_1_SB_OPCODE_ITOD:
      ConvertToDouble(CompType::getI32(), Inst);
      break;
    case D3D11_1_SB_OPCODE_UTOD:
      ConvertToDouble(CompType::getU32(), Inst);
      break;

      //
      // Resource operations.
      //
    case D3D10_SB_OPCODE_SAMPLE:
    case D3DWDDM1_3_SB_OPCODE_SAMPLE_CLAMP_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::Sample;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpClamp = 4 + (bHasFeedback ? 1 : 0);
      Value *Args[11];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      // Clamp.
      Args[10] = m_pOP->GetFloatConst(0.f);
      if (bHasFeedback) {
        if (Inst.m_Operands[uOpClamp].m_Type !=
                D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
            Inst.m_Operands[uOpClamp].m_Valuef[0] != 0.f) {
          OperandValue InClamp;
          LoadOperand(InClamp, Inst, uOpClamp, CMask::MakeXMask(),
                      CompType::getF32());
          Args[10] = InClamp[0];
        }
      }

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_SB_OPCODE_SAMPLE_B:
    case D3DWDDM1_3_SB_OPCODE_SAMPLE_B_CLAMP_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::SampleBias;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpBias = 4 + (bHasFeedback ? 1 : 0);
      const unsigned uOpClamp = uOpBias + 1;
      Value *Args[12];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      OperandValue InBias;
      LoadOperand(InBias, Inst, uOpBias, CMask::MakeXMask(),
                  CompType::getF32());
      Args[10] = InBias[0];
      // Clamp.
      Args[11] = m_pOP->GetFloatConst(0.f);
      if (bHasFeedback) {
        if (Inst.m_Operands[uOpClamp].m_Type !=
                D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
            Inst.m_Operands[uOpClamp].m_Valuef[0] != 0.f) {
          OperandValue InClamp;
          LoadOperand(InClamp, Inst, uOpClamp, CMask::MakeXMask(),
                      CompType::getF32());
          Args[11] = InClamp[0];
        }
      }

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_SB_OPCODE_SAMPLE_L:
    case D3DWDDM1_3_SB_OPCODE_SAMPLE_L_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::SampleLevel;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpLevel = 4 + (bHasFeedback ? 1 : 0);
      Value *Args[11];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      OperandValue InLevel;
      LoadOperand(InLevel, Inst, uOpLevel, CMask::MakeXMask(),
                  CompType::getF32());
      Args[10] = InLevel[0];

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_SB_OPCODE_SAMPLE_D:
    case D3DWDDM1_3_SB_OPCODE_SAMPLE_D_CLAMP_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::SampleGrad;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpSRV = DXBC::GetResourceSlot(Inst.OpCode());
      const unsigned uOpDx = 4 + (bHasFeedback ? 1 : 0);
      const unsigned uOpDy = uOpDx + 1;
      const unsigned uOpClamp = uOpDy + 1;
      const DxilResource &R = GetSRVFromOperand(Inst, uOpSRV);
      Value *Args[17];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      OperandValue InDx, InDy;
      CMask DxDyMask =
          CMask::MakeFirstNCompMask(DXBC::GetNumResCoords(R.GetKind()));
      LoadOperand(InDx, Inst, uOpDx, DxDyMask, CompType::getF32());
      Args[10] = DxDyMask.IsSet(0) ? InDx[0] : m_pUnusedF32;
      Args[11] = DxDyMask.IsSet(1) ? InDx[1] : m_pUnusedF32;
      Args[12] = DxDyMask.IsSet(2) ? InDx[2] : m_pUnusedF32;
      LoadOperand(InDy, Inst, uOpDy, DxDyMask, CompType::getF32());
      Args[13] = DxDyMask.IsSet(0) ? InDy[0] : m_pUnusedF32;
      Args[14] = DxDyMask.IsSet(1) ? InDy[1] : m_pUnusedF32;
      Args[15] = DxDyMask.IsSet(2) ? InDy[2] : m_pUnusedF32;
      // Clamp.
      Args[16] = m_pOP->GetFloatConst(0.f);
      if (bHasFeedback) {
        if (Inst.m_Operands[uOpClamp].m_Type !=
                D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
            Inst.m_Operands[uOpClamp].m_Valuef[0] != 0.f) {
          OperandValue InClamp;
          LoadOperand(InClamp, Inst, uOpClamp, CMask::MakeXMask(),
                      CompType::getF32());
          Args[16] = InClamp[0];
        }
      }

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_SB_OPCODE_SAMPLE_C:
    case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::SampleCmp;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpCmp = 4 + (bHasFeedback ? 1 : 0);
      const unsigned uOpClamp = uOpCmp + 1;
      Value *Args[12];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      OperandValue InCmp;
      LoadOperand(InCmp, Inst, uOpCmp, CMask::MakeXMask(), CompType::getF32());
      Args[10] = InCmp[0];
      // Clamp.
      Args[11] = m_pOP->GetFloatConst(0.f);
      if (bHasFeedback) {
        if (Inst.m_Operands[uOpClamp].m_Type !=
                D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
            Inst.m_Operands[uOpClamp].m_Valuef[0] != 0.f) {
          OperandValue InClamp;
          LoadOperand(InClamp, Inst, uOpClamp, CMask::MakeXMask(),
                      CompType::getF32());
          Args[11] = InClamp[0];
        }
      }

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_SB_OPCODE_SAMPLE_C_LZ:
    case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::SampleCmpLevelZero;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpCmp = 4 + (bHasFeedback ? 1 : 0);
      Value *Args[11];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      OperandValue InCmp;
      LoadOperand(InCmp, Inst, uOpCmp, CMask::MakeXMask(), CompType::getF32());
      Args[10] = InCmp[0];

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_SB_OPCODE_LD:
    case D3D10_SB_OPCODE_LD_MS:
    case D3DWDDM1_3_SB_OPCODE_LD_FEEDBACK:
    case D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK: {
      bool bIsTexture2DMS =
          Inst.OpCode() == D3D10_SB_OPCODE_LD_MS ||
          Inst.OpCode() == D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpStatus = 1;
      const unsigned uOpCoord = uOpStatus + (bHasFeedback ? 1 : 0);
      const unsigned uOpRes = uOpCoord + 1;
      const unsigned uOpSampleCount = uOpRes + 1;
      DXASSERT_DXBC(Inst.m_Operands[uOpRes].m_Type ==
                    D3D10_SB_OPERAND_TYPE_RESOURCE);

      // Resource.
      OperandValue InSRV;
      const DxilResource &R = LoadSRVOperand(
          InSRV, Inst, uOpRes, CMask::MakeXMask(), CompType::getInvalid());

      // Return type.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          R.GetCompType().GetBaseCompType(),
          Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);

      // Create Load call.
      Value *pOpRet;
      if (R.GetKind() != DxilResource::Kind::TypedBuffer) {
        OP::OpCode OpCode = OP::OpCode::TextureLoad;

        // Coordinates.
        OperandValue InCoord;
        CMask CoordMask =
            CMask::MakeFirstNCompMask(DXBC::GetNumResCoords(R.GetKind()));
        // MIP level.
        if (!bIsTexture2DMS) {
          CoordMask.Set(3);
        }
        LoadOperand(InCoord, Inst, uOpCoord, CoordMask, CompType::getI32());

        Value *Args[9];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InSRV[0];                             // Texture SRV handle
        if (!bIsTexture2DMS) {
          Args[2] = InCoord[3]; // MIP level
        } else {
          BYTE Comp = Inst.m_Operands[uOpSampleCount].m_ComponentName;
          OperandValue InSampleCount;
          LoadOperand(InSampleCount, Inst, uOpSampleCount,
                      CMask::MakeCompMask(Comp), CompType::getI32());
          Args[2] = InSampleCount[Comp]; // Sample count
        }
        // Coordinates.
        Args[3] =
            CoordMask.IsSet(0) ? InCoord[0] : m_pUnusedI32; // Coordinate 0
        Args[4] =
            CoordMask.IsSet(1) ? InCoord[1] : m_pUnusedI32; // Coordinate 1
        Args[5] =
            CoordMask.IsSet(2) ? InCoord[2] : m_pUnusedI32; // Coordinate 2
        // Offsets.
        CMask OffsetMask =
            CMask::MakeFirstNCompMask(DXBC::GetNumResOffsets(R.GetKind()));
        Args[6] = OffsetMask.IsSet(0)
                      ? m_pOP->GetU32Const(Inst.m_TexelOffset[0])
                      : m_pUnusedI32; // Offset 0
        Args[7] = OffsetMask.IsSet(1)
                      ? m_pOP->GetU32Const(Inst.m_TexelOffset[1])
                      : m_pUnusedI32; // Offset 1
        Args[8] = OffsetMask.IsSet(2)
                      ? m_pOP->GetU32Const(Inst.m_TexelOffset[2])
                      : m_pUnusedI32; // Offset 2

        Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
        pOpRet = m_pBuilder->CreateCall(F, Args);
      } else {
        // R.GetKind() == DxilResource::TypedBuffer
        OP::OpCode OpCode = OP::OpCode::BufferLoad;

        Value *Args[4];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InSRV[0];                             // Buffer SRV handle
        Args[2] = GetCoordValue(Inst, uOpCoord);        // Coord 0: in elements
        Args[3] = m_pUnusedI32;                         // Coord 1: unused

        Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
        pOpRet = m_pBuilder->CreateCall(F, Args);
      }

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D11_SB_OPCODE_LD_UAV_TYPED:
    case D3DWDDM1_3_SB_OPCODE_LD_UAV_TYPED_FEEDBACK: {
      bool bHasStatus = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpStatus = 1;
      const unsigned uOpCoord = uOpStatus + (bHasStatus ? 1 : 0);
      const unsigned uOpUAV = uOpCoord + 1;
      DXASSERT_DXBC(Inst.m_Operands[uOpUAV].m_Type ==
                    D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW);
      const DxilResource &R = m_pPR->GetUAV(
          m_UAVRangeMap[Inst.m_Operands[uOpUAV].m_Index[0].m_RegIndex]);

      // Resource.
      OperandValue InUAV;
      LoadOperand(InUAV, Inst, uOpUAV, CMask::MakeXMask(),
                  CompType::getInvalid());

      // Return type.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          R.GetCompType().GetBaseCompType(),
          Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);

      // Create Load call.
      Value *pOpRet;
      if (R.GetKind() != DxilResource::Kind::TypedBuffer) {
        OP::OpCode OpCode = OP::OpCode::TextureLoad;

        // Coordinates.
        OperandValue InCoord;
        CMask CoordMask =
            CMask::MakeFirstNCompMask(DXBC::GetNumResCoords(R.GetKind()));
        LoadOperand(InCoord, Inst, uOpCoord, CoordMask, CompType::getI32());

        Value *Args[9];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InUAV[0];                             // RWTexture UAV handle
        Args[2] = m_pUnusedI32;                         // MIP level.
        // Coordinates.
        Args[3] =
            CoordMask.IsSet(0) ? InCoord[0] : m_pUnusedI32; // Coordinate 0
        Args[4] =
            CoordMask.IsSet(1) ? InCoord[1] : m_pUnusedI32; // Coordinate 1
        Args[5] =
            CoordMask.IsSet(2) ? InCoord[2] : m_pUnusedI32; // Coordinate 2
        // Offsets.
        Args[6] = m_pUnusedI32; // Offset 0
        Args[7] = m_pUnusedI32; // Offset 1
        Args[8] = m_pUnusedI32; // Offset 2

        Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
        pOpRet = m_pBuilder->CreateCall(F, Args);
      } else {
        // R.GetKind() == DxilResource::TypedBuffer
        OP::OpCode OpCode = OP::OpCode::BufferLoad;

        Value *Args[4];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InUAV[0];                             // RWBuffer UAV handle
        Args[2] = GetCoordValue(Inst, uOpCoord);        // Coord 0: in elements
        Args[3] = m_pUnusedI32;                         // Coord 1: undef

        Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
        pOpRet = m_pBuilder->CreateCall(F, Args);
      }

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D11_SB_OPCODE_STORE_UAV_TYPED: {
      const unsigned uOpUAV = 0;
      const unsigned uOpCoord = uOpUAV + 1;
      const unsigned uOpValue = uOpCoord + 1;
      DXASSERT_DXBC(Inst.m_Operands[uOpUAV].m_Type ==
                    D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW);
      const DxilResource &R = m_pPR->GetUAV(
          m_UAVRangeMap[Inst.m_Operands[uOpUAV].m_Index[0].m_RegIndex]);
      OperandValue InUAV, InCoord, InValue;

      // Resource.
      LoadOperand(InUAV, Inst, uOpUAV, CMask::MakeXMask(),
                  CompType::getInvalid());

      // Coordinates.
      CMask CoordMask =
          CMask::MakeFirstNCompMask(DXBC::GetNumResCoords(R.GetKind()));
      LoadOperand(InCoord, Inst, uOpCoord, CoordMask, CompType::getI32());

      // Value type.
      CompType ValueType =
          DXBC::GetCompTypeWithMinPrec(R.GetCompType().GetBaseCompType(),
                                       Inst.m_Operands[uOpUAV].m_MinPrecision);
      Type *pValueType = ValueType.GetLLVMType(m_Ctx);

      // Value.
      CMask ValueMask = CMask::FromDXBC(Inst.m_Operands[uOpUAV].m_WriteMask);
      LoadOperand(InValue, Inst, uOpValue, ValueMask, ValueType);

      // Create Store call.
      if (R.GetKind() != DxilResource::Kind::TypedBuffer) {
        OP::OpCode OpCode = OP::OpCode::TextureStore;

        Value *Args[10];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InUAV[0];                             // RWTexture UAV handle
        // Coordinates.
        Args[2] =
            CoordMask.IsSet(0) ? InCoord[0] : m_pUnusedI32; // Coordinate 0
        Args[3] =
            CoordMask.IsSet(1) ? InCoord[1] : m_pUnusedI32; // Coordinate 1
        Args[4] =
            CoordMask.IsSet(2) ? InCoord[2] : m_pUnusedI32; // Coordinate 2
        // Value.
        Args[5] = ValueMask.IsSet(0) ? InValue[0] : m_pUnusedI32; // Value 0
        Args[6] = ValueMask.IsSet(1) ? InValue[1] : m_pUnusedI32; // Value 1
        Args[7] = ValueMask.IsSet(2) ? InValue[2] : m_pUnusedI32; // Value 2
        Args[8] = ValueMask.IsSet(3) ? InValue[3] : m_pUnusedI32; // Value 3
        Args[9] = m_pOP->GetU8Const(ValueMask.ToByte());          // Value mask

        Function *F = m_pOP->GetOpFunc(OpCode, pValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      } else {
        // R.GetKind() == DxilResource::TypedBuffer
        OP::OpCode OpCode = OP::OpCode::BufferStore;

        Value *Args[9];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InUAV[0];                             // RWBuffer UAV handle
        Args[2] = InCoord[0];                           // Coord 0: in elements
        Args[3] = m_pUnusedI32;                         // Coord 1: unused
        Args[4] = ValueMask.IsSet(0) ? InValue[0] : m_pUnusedI32; // Value 0
        Args[5] = ValueMask.IsSet(1) ? InValue[1] : m_pUnusedI32; // Value 1
        Args[6] = ValueMask.IsSet(2) ? InValue[2] : m_pUnusedI32; // Value 2
        Args[7] = ValueMask.IsSet(3) ? InValue[3] : m_pUnusedI32; // Value 3
        Args[8] = m_pOP->GetU8Const(ValueMask.ToByte());          // Value mask

        Function *F = m_pOP->GetOpFunc(OpCode, pValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      }
      break;
    }

    case D3D11_SB_OPCODE_LD_RAW:
    case D3DWDDM1_3_SB_OPCODE_LD_RAW_FEEDBACK: {
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpStatus = 1;
      const unsigned uOpByteOffset = uOpStatus + (bHasFeedback ? 1 : 0);
      const unsigned uOpRes = uOpByteOffset + 1;
      // Byte offset.
      Value *pByteOffset = GetCoordValue(Inst, uOpByteOffset);

      if (Inst.m_Operands[uOpRes].m_Type !=
          D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY) {
        OP::OpCode OpCode = OP::OpCode::BufferLoad;
        OperandValue InRes, InByteOffset;

        // Resource.
        LoadOperand(InRes, Inst, uOpRes, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Create Load call.
        Value *Args[4];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InRes[0];     // [RW]ByteAddressBuffer UAV/SRV handle
        Args[2] = pByteOffset;  // Coord 0: in bytes
        Args[3] = m_pUnusedI32; // Coord 1: unused

        CompType DstType = CompType::getI32();
        Type *pDstType = DstType.GetLLVMType(m_Ctx);
        Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
        Value *pOpRet = m_pBuilder->CreateCall(F, Args);

        StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      } else {
        const unsigned uOpTGSM = uOpRes;
        CompType SrcType = CompType::getI32();
        ConvertLoadTGSM(Inst, uOpTGSM, uOpOutput, SrcType, pByteOffset);
      }

      break;
    }

    case D3D11_SB_OPCODE_STORE_RAW: {
      const unsigned uOpRes = 0;
      const unsigned uOpByteOffset = uOpRes + 1;
      const unsigned uOpValue = uOpByteOffset + 1;
      // Byte offset.
      Value *pByteOffset = GetCoordValue(Inst, uOpByteOffset);

      if (Inst.m_Operands[uOpRes].m_Type ==
          D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW) {
        const unsigned uOpUAV = uOpRes;
        OP::OpCode OpCode = OP::OpCode::BufferStore;
        DXASSERT_DXBC(Inst.m_Operands[uOpUAV].m_Type ==
                      D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW);
        OperandValue InUAV, InByteOffset, InValue;

        // Resource.
        LoadOperand(InUAV, Inst, uOpUAV, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Value type.
        CompType ValueType = CompType::getI32();
        Type *pValueType = ValueType.GetLLVMType(m_Ctx);

        // Value.
        CMask ValueMask = CMask::FromDXBC(Inst.m_Operands[uOpUAV].m_WriteMask);
        LoadOperand(InValue, Inst, uOpValue, ValueMask, ValueType);

        // Create Store call.
        Value *Args[9];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InUAV[0];     // RWByteAddressBuffer UAV handle
        Args[2] = pByteOffset;  // Coord 0: in bytes
        Args[3] = m_pUnusedI32; // Coord 1: undef
        Args[4] = ValueMask.IsSet(0) ? InValue[0] : m_pUnusedI32; // Value 0
        Args[5] = ValueMask.IsSet(1) ? InValue[1] : m_pUnusedI32; // Value 1
        Args[6] = ValueMask.IsSet(2) ? InValue[2] : m_pUnusedI32; // Value 2
        Args[7] = ValueMask.IsSet(3) ? InValue[3] : m_pUnusedI32; // Value 3
        Args[8] = m_pOP->GetU8Const(ValueMask.ToByte());          // Value mask

        Function *F = m_pOP->GetOpFunc(OpCode, pValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      } else {
        const unsigned uOpTGSM = uOpRes;
        CompType ValueType = CompType::getI32();
        ConvertStoreTGSM(Inst, uOpTGSM, uOpValue, ValueType, pByteOffset);
      }

      break;
    }

    case D3D11_SB_OPCODE_LD_STRUCTURED:
    case D3DWDDM1_3_SB_OPCODE_LD_STRUCTURED_FEEDBACK: {
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpStatus = 1;
      const unsigned uOpElementOffset = uOpStatus + (bHasFeedback ? 1 : 0);
      const unsigned uOpStructByteOffset = uOpElementOffset + 1;
      const unsigned uOpRes = uOpStructByteOffset + 1;

      if (Inst.m_Operands[uOpRes].m_Type !=
          D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY) {
        OP::OpCode OpCode = OP::OpCode::BufferLoad;
        OperandValue InRes, InElementOffset, InStructByteOffset;

        // Resource.
        LoadOperand(InRes, Inst, uOpRes, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Create Load call.
        Value *Args[4];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InRes[0]; // [RW]ByteAddressBuffer UAV/SRV handle
        Args[2] =
            GetCoordValue(Inst, uOpElementOffset); // Coord 1: element index
        Args[3] = GetCoordValue(
            Inst,
            uOpStructByteOffset); // Coord 2: byte offset within the element

        CompType DstType = CompType::getI32();
        Type *pDstType = DstType.GetLLVMType(m_Ctx);
        Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
        Value *pOpRet = m_pBuilder->CreateCall(F, Args);

        StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      } else {
        const unsigned uOpTGSM = uOpRes;
        DXASSERT_DXBC(Inst.m_Operands[uOpTGSM].m_Type ==
                      D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY);
        const TGSMEntry &R =
            m_TGSMMap[Inst.m_Operands[uOpTGSM].m_Index[0].m_RegIndex];
        CompType SrcType = CompType::getF32();

        // Byte offset.
        Value *pByteOffset = GetByteOffset(Inst, uOpElementOffset,
                                           uOpStructByteOffset, R.Stride);

        ConvertLoadTGSM(Inst, uOpTGSM, uOpOutput, SrcType, pByteOffset);
      }

      break;
    }

    case D3D11_SB_OPCODE_STORE_STRUCTURED: {
      const unsigned uOpRes = 0;
      const unsigned uOpElementOffset = uOpRes + 1;
      const unsigned uOpStructByteOffset = uOpElementOffset + 1;
      const unsigned uOpValue = uOpStructByteOffset + 1;

      if (Inst.m_Operands[0].m_Type ==
          D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW) {
        OP::OpCode OpCode = OP::OpCode::BufferStore;
        const unsigned uOpUAV = uOpRes;
        DXASSERT_DXBC(Inst.m_Operands[uOpUAV].m_Type ==
                      D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW);
        OperandValue InUAV, InElementOffset, InStructByteOffset, InValue;

        // Resource.
        LoadOperand(InUAV, Inst, uOpUAV, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Value type.
        CompType ValueType = CompType::getI32();
        Type *pValueType = ValueType.GetLLVMType(m_Ctx);

        // Value.
        CMask ValueMask = CMask::FromDXBC(Inst.m_Operands[uOpUAV].m_WriteMask);
        LoadOperand(InValue, Inst, uOpValue, ValueMask, ValueType);

        // Create Store call.
        Value *Args[9];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InUAV[0]; // RWByteAddressBuffer UAV handle
        Args[2] =
            GetCoordValue(Inst, uOpElementOffset); // Coord 1: element index
        Args[3] = GetCoordValue(
            Inst,
            uOpStructByteOffset); // Coord 2: byte offset within the element
        Args[4] = ValueMask.IsSet(0) ? InValue[0] : m_pUnusedI32; // Value 0
        Args[5] = ValueMask.IsSet(1) ? InValue[1] : m_pUnusedI32; // Value 1
        Args[6] = ValueMask.IsSet(2) ? InValue[2] : m_pUnusedI32; // Value 2
        Args[7] = ValueMask.IsSet(3) ? InValue[3] : m_pUnusedI32; // Value 3
        Args[8] = m_pOP->GetU8Const(ValueMask.ToByte());          // Value mask

        Function *F = m_pOP->GetOpFunc(OpCode, pValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      } else {
        const unsigned uOpTGSM = uOpRes;
        const TGSMEntry &R =
            m_TGSMMap[Inst.m_Operands[uOpTGSM].m_Index[0].m_RegIndex];
        CompType ValueType = CompType::getF32();

        // Byte offset.
        Value *pByteOffset = GetByteOffset(Inst, uOpElementOffset,
                                           uOpStructByteOffset, R.Stride);

        ConvertStoreTGSM(Inst, uOpTGSM, uOpValue, ValueType, pByteOffset);
      }

      break;
    }

    //
    // Atomic operations.
    //
    case D3D11_SB_OPCODE_ATOMIC_AND:
    case D3D11_SB_OPCODE_ATOMIC_OR:
    case D3D11_SB_OPCODE_ATOMIC_XOR:
    case D3D11_SB_OPCODE_ATOMIC_IADD:
    case D3D11_SB_OPCODE_ATOMIC_IMAX:
    case D3D11_SB_OPCODE_ATOMIC_IMIN:
    case D3D11_SB_OPCODE_ATOMIC_UMAX:
    case D3D11_SB_OPCODE_ATOMIC_UMIN:
    case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
    case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
    case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
    case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
    case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
    case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
    case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
    case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
    case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
    case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
    case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH: {
      bool bHasReturn = DXBC::AtomicBinOpHasReturn(Inst.OpCode());
      bool bHasCompare = DXBC::IsCompareExchAtomicBinOp(Inst.OpCode());
      const unsigned uOpRes = bHasReturn ? 1 : 0;
      const unsigned uOpCoord = uOpRes + 1;
      const unsigned uOpCompareValue = uOpCoord + (bHasCompare ? 1 : 0);
      const unsigned uOpValue = uOpCompareValue + 1;

      if (Inst.m_Operands[uOpRes].m_Type ==
          D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW) {
        const unsigned uOpUAV = uOpRes;
        const DxilResource &R = m_pPR->GetUAV(
            m_UAVRangeMap[Inst.m_Operands[uOpUAV].m_Index[0].m_RegIndex]);
        OperandValue InUAV, InCoord, InCompareValue, InValue;

        // Resource.
        LoadOperand(InUAV, Inst, uOpUAV, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Coordinates.
        CMask CoordMask =
            CMask::MakeFirstNCompMask(DxilResource::GetNumCoords(R.GetKind()));
        LoadOperand(InCoord, Inst, uOpCoord, CoordMask, CompType::getI32());
        Value *pOffset[3];
        pOffset[0] = InCoord[0];
        pOffset[1] = CoordMask.IsSet(1) ? InCoord[1] : m_pUnusedI32;
        pOffset[2] = CoordMask.IsSet(2) ? InCoord[2] : m_pUnusedI32;

        // Value type.
        CompType ValueType = CompType::getI32();
        Type *pValueType = ValueType.GetLLVMType(m_Ctx);

        // Compare value.
        if (bHasCompare) {
          LoadOperand(InCompareValue, Inst, uOpCompareValue, CMask::MakeXMask(),
                      ValueType);
        }

        // Value.
        LoadOperand(InValue, Inst, uOpValue, CMask::MakeXMask(), ValueType);

        // Create atomic call.
        Value *pOpRet;
        if (!bHasCompare) {
          OP::OpCode OpCode = OP::OpCode::AtomicBinOp;
          Value *Args[7];
          Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
          Args[1] = InUAV[0]; // Typed (uint/int) UAV handle
          Args[2] = m_pOP->GetU32Const((unsigned)DXBC::GetAtomicBinOp(
              Inst.OpCode()));  // Atomic operation kind.
          Args[3] = pOffset[0]; // Offset 0, in elements
          Args[4] = pOffset[1]; // Offset 1
          Args[5] = pOffset[2]; // Offset 2
          Args[6] = InValue[0]; // New value

          Function *F = m_pOP->GetOpFunc(OpCode, pValueType);
          pOpRet = m_pBuilder->CreateCall(F, Args);
        } else {
          OP::OpCode OpCode = OP::OpCode::AtomicCompareExchange;
          Value *Args[7];
          Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
          Args[1] = InUAV[0];          // Typed (uint/int) UAV handle
          Args[2] = pOffset[0];        // Offset 0, in elements
          Args[3] = pOffset[1];        // Offset 1
          Args[4] = pOffset[2];        // Offset 2
          Args[5] = InCompareValue[0]; // Compare value
          Args[6] = InValue[0];        // New value

          Function *F = m_pOP->GetOpFunc(OpCode, pValueType);
          pOpRet = m_pBuilder->CreateCall(F, Args);
        }

        StoreBroadcastOutput(Inst, pOpRet, ValueType);
      } else {
        const unsigned uOpTGSM = uOpRes;
        DXASSERT_DXBC(Inst.m_Operands[uOpTGSM].m_Type ==
                      D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY);
        const TGSMEntry &R =
            m_TGSMMap[Inst.m_Operands[uOpTGSM].m_Index[0].m_RegIndex];
        OperandValue InElementOffset, InCompareValue, InValue;

        // Byte offset.
        CMask ElementOffsetMask =
            CMask::MakeFirstNCompMask(R.Stride == 1 ? 1 : 2);
        LoadOperand(InElementOffset, Inst, uOpCoord, ElementOffsetMask,
                    CompType::getI32());
        Value *pByteOffset = InElementOffset[0];
        if (R.Stride > 1) { // Structured TGSM.
          Value *pOffset2 = InElementOffset[1];
          Value *pStride = m_pOP->GetU32Const(R.Stride);
          pByteOffset = m_pBuilder->CreateAdd(
              m_pBuilder->CreateMul(pByteOffset, pStride), pOffset2);
        }

        // Value type.
        CompType ValueType = CompType::getI32();

        // Compare value.
        if (bHasCompare) {
          LoadOperand(InCompareValue, Inst, uOpCompareValue, CMask::MakeXMask(),
                      ValueType);
        }

        Type *pDstType = Type::getInt32PtrTy(m_Ctx, DXIL::kTGSMAddrSpace);

        // Value.
        LoadOperand(InValue, Inst, uOpValue, CMask::MakeXMask(), ValueType);

        // Create GEP.
        Value *pGEPIndices[2] = {m_pOP->GetU32Const(0), pByteOffset};
        Value *pPtrI8 = m_pBuilder->CreateGEP(R.pVar, pGEPIndices);
        Value *pPtr = m_pBuilder->CreatePointerCast(pPtrI8, pDstType);

        // Generate atomic instruction.
        Value *pRetVal;
        if (!bHasCompare) {
          pRetVal = m_pBuilder->CreateAtomicRMW(
              DXBC::GetLlvmAtomicBinOp(Inst.OpCode()), pPtr, InValue[0],
              AtomicOrdering::Monotonic);
        } else {
          pRetVal = m_pBuilder->CreateAtomicCmpXchg(
              pPtr, InCompareValue[0], InValue[0], AtomicOrdering::Monotonic,
              AtomicOrdering::Monotonic);
          pRetVal = m_pBuilder->CreateExtractValue(pRetVal, 0);
        }

        StoreBroadcastOutput(Inst, pRetVal, ValueType);
      }

      break;
    }

    case D3D10_1_SB_OPCODE_GATHER4:
    case D3DWDDM1_3_SB_OPCODE_GATHER4_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::TextureGather;
      const unsigned uOpOutput = 0;
      const unsigned uOpSRV = DXBC::GetResourceSlot(Inst.OpCode());
      const unsigned uOpSampler = uOpSRV + 1;
      const DxilResource &R = GetSRVFromOperand(Inst, uOpSRV);
      Value *Args[10];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      // Offset.
      bool bUseOffset = (R.GetKind() == DxilResource::Kind::Texture2D) ||
                        (R.GetKind() == DxilResource::Kind::Texture2DArray);
      if (!bUseOffset) {
        Args[7] = m_pUnusedI32;
        Args[8] = m_pUnusedI32;
      }
      // Channel.
      unsigned uChannel = Inst.m_Operands[uOpSampler].m_ComponentName;
      Args[9] = m_pOP->GetU32Const(uChannel);

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          R.GetCompType().GetBaseCompType(),
          Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D11_SB_OPCODE_GATHER4_C:
    case D3DWDDM1_3_SB_OPCODE_GATHER4_C_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::TextureGatherCmp;
      const unsigned uOpOutput = 0;
      const unsigned uOpSRV = DXBC::GetResourceSlot(Inst.OpCode());
      const unsigned uOpSampler = uOpSRV + 1;
      const unsigned uOpCmp = uOpSampler + 1;
      const DxilResource &R = GetSRVFromOperand(Inst, uOpSRV);
      Value *Args[11];

      LoadCommonSampleInputs(Inst, &Args[0]);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      // Offset.
      bool bUseOffset = (R.GetKind() == DxilResource::Kind::Texture2D) ||
                        (R.GetKind() == DxilResource::Kind::Texture2DArray);
      if (!bUseOffset) {
        Args[7] = m_pUnusedI32;
        Args[8] = m_pUnusedI32;
      }
      // Channel.
      unsigned uChannel = Inst.m_Operands[uOpSampler].m_ComponentName;
      Args[9] = m_pOP->GetU32Const(uChannel);
      // Comparison value.
      OperandValue InCmp;
      LoadOperand(InCmp, Inst, uOpCmp, CMask::MakeXMask(), CompType::getF32());
      Args[10] = InCmp[0];

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          R.GetCompType().GetBaseCompType(),
          Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D11_SB_OPCODE_GATHER4_PO:
    case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::TextureGather;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpCoord = uOpOutput + 1 + (bHasFeedback ? 1 : 0);
      const unsigned uOpOffset = uOpCoord + 1;
      const unsigned uOpSRV = DXBC::GetResourceSlot(Inst.OpCode());
      const unsigned uOpSampler = uOpSRV + 1;
      const DxilResource &R = GetSRVFromOperand(Inst, uOpSRV);
      Value *Args[10];

      LoadCommonSampleInputs(Inst, &Args[0], false);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      // Programmable offset.
      OperandValue InOffset;
      LoadOperand(InOffset, Inst, uOpOffset, CMask::MakeFirstNCompMask(2),
                  CompType::getI32());
      Args[7] = InOffset[0];
      Args[8] = InOffset[1];
      // Channel.
      unsigned uChannel = Inst.m_Operands[uOpSampler].m_ComponentName;
      Args[9] = m_pOP->GetU32Const(uChannel);

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          R.GetCompType().GetBaseCompType(),
          Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D11_SB_OPCODE_GATHER4_PO_C:
    case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK: {
      OP::OpCode OpCode = OP::OpCode::TextureGatherCmp;
      bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
      const unsigned uOpOutput = 0;
      const unsigned uOpCoord = uOpOutput + 1 + (bHasFeedback ? 1 : 0);
      const unsigned uOpOffset = uOpCoord + 1;
      const unsigned uOpSRV = DXBC::GetResourceSlot(Inst.OpCode());
      const unsigned uOpSampler = uOpSRV + 1;
      const unsigned uOpCmp = uOpSampler + 1;
      const DxilResource &R = GetSRVFromOperand(Inst, uOpSRV);
      Value *Args[11];

      LoadCommonSampleInputs(Inst, &Args[0], false);

      // Other arguments.
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
      // Programmable offset.
      OperandValue InOffset;
      LoadOperand(InOffset, Inst, uOpOffset, CMask::MakeFirstNCompMask(2),
                  CompType::getI32());
      Args[7] = InOffset[0];
      Args[8] = InOffset[1];
      // Channel.
      unsigned uChannel = Inst.m_Operands[uOpSampler].m_ComponentName;
      Args[9] = m_pOP->GetU32Const(uChannel);
      // Comparison value.
      OperandValue InCmp;
      LoadOperand(InCmp, Inst, uOpCmp, CMask::MakeXMask(), CompType::getF32());
      Args[10] = InCmp[0];

      // Function call.
      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          R.GetCompType().GetBaseCompType(),
          Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreResRetOutputAndStatus(Inst, pOpRet, DstType);
      break;
    }

    case D3D10_1_SB_OPCODE_SAMPLE_POS: {
      const unsigned uOpOutput = 0;
      const unsigned uOpResOrRast = uOpOutput + 1;
      const unsigned uOpSample = uOpResOrRast + 1;

      // Sample.
      OperandValue InSample;
      LoadOperand(InSample, Inst, uOpSample, CMask::MakeXMask(),
                  CompType::getI32());
      Value *pOpRet;

      if (Inst.m_Operands[uOpResOrRast].m_Type ==
          D3D10_SB_OPERAND_TYPE_RESOURCE) {
        // Resource.
        OP::OpCode OpCode = OP::OpCode::Texture2DMSGetSamplePosition;
        OperandValue InRes;
        LoadOperand(InRes, Inst, uOpResOrRast, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Create SamplePosition call.
        Value *Args[3];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InRes[0];                             // Resource handle
        Args[2] = InSample[0];                          // Sample index

        Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
        pOpRet = m_pBuilder->CreateCall(F, Args);
      } else {
        // Render target.
        OP::OpCode OpCode = OP::OpCode::RenderTargetGetSamplePosition;

        // Create SamplePosition call.
        Value *Args[2];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InSample[0];                          // Sample index

        Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
        pOpRet = m_pBuilder->CreateCall(F, Args);
      }

      StoreSamplePosOutput(Inst, pOpRet);
      break;
    }

    case D3DWDDM1_3_SB_OPCODE_CHECK_ACCESS_FULLY_MAPPED: {
      OP::OpCode OpCode = OP::OpCode::CheckAccessFullyMapped;
      OperandValue InStatus;
      LoadOperand(InStatus, Inst, 1, CMask::MakeXMask(), CompType::getI32());

      // Create CheckAccessFullyMapped call.
      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode);         // OpCode
      Args[1] = InStatus[Inst.m_Operands[0].m_ComponentName]; // Status

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getInt32Ty(m_Ctx));
      Value *pRetValue = m_pBuilder->CreateCall(F, Args);
      pRetValue =
          CastDxbcValue(pRetValue, CompType::getI1(), CompType::getI32());

      StoreBroadcastOutput(Inst, pRetValue, CompType::getI32());
      break;
    }

    case D3D10_SB_OPCODE_RESINFO: {
      OP::OpCode OpCode = OP::OpCode::GetDimensions;
      const unsigned uOpOutput = 0;
      const unsigned uOpMipLevel = uOpOutput + 1;
      const unsigned uOpRes = uOpMipLevel + 1;

      // MipLevel.
      OperandValue InMipLevel;
      LoadOperand(InMipLevel, Inst, uOpMipLevel, CMask::MakeXMask(),
                  CompType::getI32());

      // Resource.
      OperandValue InRes;
      LoadOperand(InRes, Inst, uOpRes, CMask::MakeXMask(),
                  CompType::getInvalid());

      // Create GetDimensions call.
      Value *Args[3];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = InRes[0];                             // Resource handle
      Args[2] = InMipLevel[0];                        // MipLevel

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreGetDimensionsOutput(Inst, pOpRet);
      break;
    }

    case D3D11_SB_OPCODE_BUFINFO: {
      OP::OpCode OpCode = OP::OpCode::GetDimensions;
      const unsigned uOpOutput = 0;
      const unsigned uOpRes = uOpOutput + 1;

      // Resource.
      OperandValue InRes;
      LoadOperand(InRes, Inst, uOpRes, CMask::MakeXMask(),
                  CompType::getInvalid());

      // Create GetDimensions call.
      Value *Args[3];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = InRes[0];                             // Resource handle
      Args[2] = m_pUnusedI32;                         // MipLevel (undefined)

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);
      Value *pOpWidth = m_pBuilder->CreateExtractValue(pOpRet, 0);

      // Store output.
      StoreBroadcastOutput(Inst, pOpWidth, CompType::getI32());
      break;
    }

    case D3D10_1_SB_OPCODE_SAMPLE_INFO: {
      const unsigned uOpOutput = 0;
      const unsigned uOpResOrRast = uOpOutput + 1;

      bool bDxbcRetFloat = true;
      if (Inst.m_InstructionReturnType == D3D10_SB_INSTRUCTION_RETURN_UINT) {
        bDxbcRetFloat = false;
      }

      // Return type.
      CompType DstType;
      if (bDxbcRetFloat) {
        DstType = DXBC::GetCompTypeWithMinPrec(
            CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      } else {
        DstType = DXBC::GetCompTypeWithMinPrec(
            CompType::getI32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      }

      Value *pRetValue;

      if (Inst.m_Operands[uOpResOrRast].m_Type ==
          D3D10_SB_OPERAND_TYPE_RESOURCE) {
        // Resource.
        OP::OpCode OpCode = OP::OpCode::GetDimensions;

        OperandValue InRes;
        LoadOperand(InRes, Inst, uOpResOrRast, CMask::MakeXMask(),
                    CompType::getInvalid());

        // Create GetDimensions call.
        Value *Args[3];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = InRes[0];                             // Resource handle
        Args[2] = m_pOP->GetU32Const(0);                // MipLevel

        Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
        Value *pOpRet = m_pBuilder->CreateCall(F, Args);
        pRetValue = m_pBuilder->CreateExtractValue(pOpRet, 3);
      } else {
        OP::OpCode OpCode = OP::OpCode::RenderTargetGetSampleCount;

        // Create SampleCount call.
        Value *Args[1];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode

        Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
        pRetValue = m_pBuilder->CreateCall(F, Args);
      }

      Value *pZeroValue;
      if (bDxbcRetFloat) {
        pRetValue = m_pBuilder->CreateCast(Instruction::CastOps::UIToFP,
                                           pRetValue, Type::getFloatTy(m_Ctx));
        pZeroValue = m_pOP->GetFloatConst(0.f);
      } else {
        pZeroValue = m_pOP->GetU32Const(0);
      }

      // Store output.
      CMask OutputMask =
          CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
      if (!OutputMask.IsZero()) {
        OperandValue Out;
        for (BYTE c = 0; c < DXBC::kWidth; c++) {
          if (!OutputMask.IsSet(c))
            continue;

          BYTE Comp = Inst.m_Operands[uOpResOrRast].m_Swizzle[c];
          if (Comp == 0) {
            Out[c] = pRetValue;
          } else {
            Out[c] = pZeroValue;
          }
        }
        StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);
      }
      break;
    }

    case D3D11_SB_OPCODE_IMM_ATOMIC_ALLOC:
    case D3D11_SB_OPCODE_IMM_ATOMIC_CONSUME: {
      OP::OpCode OpCode = OP::OpCode::BufferUpdateCounter;
      const unsigned uOpOutput = 0;
      const unsigned uOpUAV = uOpOutput + 1;
      bool bInc = Inst.OpCode() == D3D11_SB_OPCODE_IMM_ATOMIC_ALLOC;

      // Resource.
      OperandValue InRes;
      LoadOperand(InRes, Inst, uOpUAV, CMask::MakeXMask(),
                  CompType::getInvalid());
      // SetHasCounter.
      SetHasCounter(Inst, uOpUAV);

      // Create BufferUpdateCounter call.
      Value *Args[3];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = InRes[0];                             // Resource handle
      Args[2] = m_pOP->GetI8Const(bInc ? 1 : -1);     // Inc or Dec

      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getI32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      Value *pOpRet = m_pBuilder->CreateCall(F, Args);

      StoreBroadcastOutput(Inst, pOpRet, DstType);
      break;
    }

    case D3D11_SB_OPCODE_SYNC: {
      OP::OpCode OpCode = OP::OpCode::Barrier;
      DXIL::BarrierMode BMode = DXBC::GetBarrierMode(
          Inst.m_SyncFlags.bThreadsInGroup,
          Inst.m_SyncFlags.bUnorderedAccessViewMemoryGlobal,
          Inst.m_SyncFlags.bUnorderedAccessViewMemoryGroup,
          Inst.m_SyncFlags.bThreadGroupSharedMemory);

      // Create BufferUpdateCounter call.
      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU32Const((unsigned)BMode);  // Barrier mode

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      MarkPrecise(m_pBuilder->CreateCall(F, Args));
      break;
    }

      //
      // Control-flow operations.
      //
    case D3D10_SB_OPCODE_IF: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      // Create If-scope.
      Scope &Scope = m_ScopeStack.Push(Scope::If, m_pBuilder->GetInsertBlock());

      // Prepare condition.
      Scope.pCond = LoadZNZCondition(Inst, 0);

      // Create then-branch BB and set it as active.
      Scope.pThenBB = BasicBlock::Create(
          m_Ctx, Twine("if") + Twine(Scope.NameIndex) + Twine(".then"),
          pFunction);
      m_pBuilder->SetInsertPoint(Scope.pThenBB);

      // Create endif BB.
      Scope.pPostScopeBB = BasicBlock::Create(
          m_Ctx, Twine("if") + Twine(Scope.NameIndex) + Twine(".end"));
      break;
    }

    case D3D10_SB_OPCODE_ELSE: {
      // Get If-scope.
      Scope &Scope = m_ScopeStack.Top();
      IFTBOOL(Scope.Kind == Scope::If, E_FAIL);

      // Terminate then-branch.
      CreateBranchIfNeeded(m_pBuilder->GetInsertBlock(), Scope.pPostScopeBB);

      // Create else-branch BB and set it as active.
      Scope.pElseBB = BasicBlock::Create(
          m_Ctx, Twine("if") + Twine(Scope.NameIndex) + Twine(".else"),
          pFunction);
      m_pBuilder->SetInsertPoint(Scope.pElseBB);
      break;
    }

    case D3D10_SB_OPCODE_ENDIF: {
      // Get If-scope.
      Scope &Scope = m_ScopeStack.Top();
      IFTBOOL(Scope.Kind == Scope::If, E_FAIL);

      // Terminate else-branch.
      CreateBranchIfNeeded(m_pBuilder->GetInsertBlock(), Scope.pPostScopeBB);

      // Insert IF cbranch.
      m_pBuilder->SetInsertPoint(Scope.pPreScopeBB);
      if (Scope.pElseBB != nullptr) {
        m_pBuilder->CreateCondBr(Scope.pCond, Scope.pThenBB, Scope.pElseBB);
      } else {
        m_pBuilder->CreateCondBr(Scope.pCond, Scope.pThenBB,
                                 Scope.pPostScopeBB);
      }

      // Set endif BB as active.
      pFunction->getBasicBlockList().push_back(Scope.pPostScopeBB);
      m_pBuilder->SetInsertPoint(Scope.pPostScopeBB);

      // Finish If-scope.
      m_ScopeStack.Pop();
      break;
    }

    case D3D10_SB_OPCODE_LOOP: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      // Create Loop-scope.
      Scope &Scope =
          m_ScopeStack.Push(Scope::Loop, m_pBuilder->GetInsertBlock());

      // Create Loop and EndLoop BBs.
      Scope.pLoopBB = BasicBlock::Create(
          m_Ctx, Twine("loop") + Twine(Scope.NameIndex), pFunction);
      Scope.pPostScopeBB = BasicBlock::Create(
          m_Ctx, Twine("loop") + Twine(Scope.NameIndex) + Twine(".end"));

      // Insert branch to Loop BB.
      m_pBuilder->CreateBr(Scope.pLoopBB);

      // Set Loop BB as active.
      m_pBuilder->SetInsertPoint(Scope.pLoopBB);
      break;
    }

    case D3D10_SB_OPCODE_ENDLOOP: {
      // Get Loop-scope.
      Scope &Scope = m_ScopeStack.Top();
      IFTBOOL(Scope.Kind == Scope::Loop, E_FAIL);

      // Insert back-edge.
      CreateBranchIfNeeded(m_pBuilder->GetInsertBlock(), Scope.pLoopBB);

      // Set EndLoop BB as active.
      pFunction->getBasicBlockList().push_back(Scope.pPostScopeBB);
      m_pBuilder->SetInsertPoint(Scope.pPostScopeBB);

      // Finish Loop-scope.
      m_ScopeStack.Pop();
      break;
    }

    case D3D10_SB_OPCODE_SWITCH: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      // Create Switch-scope.
      Scope &Scope =
          m_ScopeStack.Push(Scope::Switch, m_pBuilder->GetInsertBlock());

      // Prepare selector.
      BYTE Comp = (BYTE)Inst.m_Operands[0].m_ComponentName;
      CMask ReadMask = CMask::MakeCompMask(Comp);
      OperandValue In1;
      LoadOperand(In1, Inst, 0, ReadMask, CompType::getI32());
      Scope.pSelector = In1[Comp];

      // Create 1st casegroup BB and set it as active.
      BasicBlock *pBB = BasicBlock::Create(
          m_Ctx,
          Twine("switch") + Twine(Scope.NameIndex) + Twine(".casegroup") +
              Twine(Scope.CaseGroupIndex++),
          pFunction);
      m_pBuilder->SetInsertPoint(pBB);

      // Create endswitch BB.
      Scope.pPostScopeBB = BasicBlock::Create(
          m_Ctx, Twine("switch") + Twine(Scope.NameIndex) + Twine(".end"));
      break;
    }

    case D3D10_SB_OPCODE_CASE: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      // Get Switch-scope.
      Scope &Scope = m_ScopeStack.Top();
      IFTBOOL(Scope.Kind == Scope::Switch, E_FAIL);

      // Retrieve selector value.
      const D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[0];
      DXASSERT_DXBC(O.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 &&
                    O.m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT);
      int CaseValue = O.m_Value[0];

      // Remember case clause.
      pair<unsigned, BasicBlock *> Case(CaseValue,
                                        m_pBuilder->GetInsertBlock());
      Scope.SwitchCases.emplace_back(Case);
      break;
    }

    case D3D10_SB_OPCODE_DEFAULT: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      // Get Switch-scope.
      Scope &Scope = m_ScopeStack.Top();
      IFTBOOL(Scope.Kind == Scope::Switch, E_FAIL);

      // Remember default clause.
      Scope.pDefaultBB = m_pBuilder->GetInsertBlock();
      break;
    }

    case D3D10_SB_OPCODE_ENDSWITCH: {
      // Get Switch-scope.
      Scope &Scope = m_ScopeStack.Top();
      IFTBOOL(Scope.Kind == Scope::Switch, E_FAIL);

      // Terminate case/default BB.
      CreateBranchIfNeeded(m_pBuilder->GetInsertBlock(), Scope.pPostScopeBB);

      // Insert switch branch.
      m_pBuilder->SetInsertPoint(Scope.pPreScopeBB);
      BasicBlock *pDefaultBB =
          Scope.pDefaultBB != nullptr ? Scope.pDefaultBB : Scope.pPostScopeBB;
      SwitchInst *pSwitch =
          m_pBuilder->CreateSwitch(Scope.pSelector, pDefaultBB);
      for (size_t i = 0; i < Scope.SwitchCases.size(); i++) {
        auto &Case = Scope.SwitchCases[i];
        if (Case.second == Scope.pDefaultBB)
          continue;

        pSwitch->addCase(m_pBuilder->getInt32(Case.first), Case.second);
      }

      // Rename casegroups BBs.
      SwitchInst *pSwI =
          dyn_cast<SwitchInst>(Scope.pPreScopeBB->getTerminator());
      DXASSERT_NOMSG(pSwI != nullptr);
      BasicBlock *pPrevCaseBB = nullptr;
      unsigned CaseGroupIdx = 0;
      for (auto itCase = pSwI->case_begin(), endCase = pSwI->case_end();
           itCase != endCase; ++itCase) {
        BasicBlock *pCaseBB = itCase.getCaseSuccessor();
        if (pCaseBB != pPrevCaseBB) {
          pCaseBB->setName(Twine("switch") + Twine(Scope.NameIndex) +
                           Twine(".casegroup") + Twine(CaseGroupIdx++));
          pPrevCaseBB = pCaseBB;
        }
      }

      // Rename default BB.
      if (Scope.pDefaultBB != nullptr) {
        Scope.pDefaultBB->setName(Twine("switch") + Twine(Scope.NameIndex) +
                                  Twine(".default"));
      }

      // Set endswitch BB as active.
      pFunction->getBasicBlockList().push_back(Scope.pPostScopeBB);
      m_pBuilder->SetInsertPoint(Scope.pPostScopeBB);

      // Finish Switch-scope.
      m_ScopeStack.Pop();
      break;
    }

    case D3D10_SB_OPCODE_CONTINUE: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      // Find parent scope.
      Scope &Scope = m_ScopeStack.FindParentLoop();

      // Create a new basic block.
      BasicBlock *pNextBB = BasicBlock::Create(
          m_Ctx,
          Twine("loop") + Twine(Scope.NameIndex) + Twine(".continue") +
              Twine(Scope.ContinueIndex++),
          pFunction);

      // Insert branch to Loop BB.
      m_pBuilder->CreateBr(Scope.pLoopBB);

      // Set Next BB as active.
      m_pBuilder->SetInsertPoint(pNextBB);
      break;
    }

    case D3D10_SB_OPCODE_CONTINUEC: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);

      // Prepare condition.
      Value *pCond = LoadZNZCondition(Inst, 0);

      // Find parent scope.
      Scope &Scope = m_ScopeStack.FindParentLoop();

      // Create a new basic block.
      BasicBlock *pNextBB = BasicBlock::Create(
          m_Ctx,
          Twine("loop") + Twine(Scope.NameIndex) + Twine(".continuec") +
              Twine(Scope.ContinueIndex++),
          pFunction);

      // Insert cbranch to Loop and Next BBs.
      m_pBuilder->CreateCondBr(pCond, Scope.pLoopBB, pNextBB);

      // Set Next BB as active.
      m_pBuilder->SetInsertPoint(pNextBB);
      break;
    }

    case D3D10_SB_OPCODE_BREAK: {
      DXASSERT_DXBC(Inst.m_NumOperands == 0);
      // Find parent scope.
      Scope &Scope = m_ScopeStack.FindParentLoopOrSwitch();

      // Create a new basic block.
      BasicBlock *pNextBB;
      if (Scope.Kind == Scope::Loop) {
        pNextBB = BasicBlock::Create(m_Ctx,
                                     Twine("loop") + Twine(Scope.NameIndex) +
                                         Twine(".break") +
                                         Twine(Scope.LoopBreakIndex++),
                                     pFunction);
      } else {
        if (m_ScopeStack.Top().Kind == Scope::Switch) {
          pNextBB = BasicBlock::Create(
              m_Ctx,
              Twine("switch") + Twine(Scope.NameIndex) +
                  Twine(".tmpcasegroup") + Twine(Scope.CaseGroupIndex++),
              pFunction);
        } else {
          pNextBB = BasicBlock::Create(
              m_Ctx,
              Twine("switch") + Twine(Scope.NameIndex) + Twine(".break") +
                  Twine(Scope.SwitchBreakIndex++),
              pFunction);
        }
      }

      // Insert branch to PostScope BB.
      m_pBuilder->CreateBr(Scope.pPostScopeBB);

      // Set Next BB as active.
      m_pBuilder->SetInsertPoint(pNextBB);
      break;
    }

    case D3D10_SB_OPCODE_BREAKC: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);

      // Prepare condition.
      Value *pCond = LoadZNZCondition(Inst, 0);

      // Find parent scope.
      Scope &Scope = m_ScopeStack.FindParentLoopOrSwitch();

      // Create a new basic block.
      BasicBlock *pNextBB;
      if (Scope.Kind == Scope::Loop) {
        pNextBB = BasicBlock::Create(m_Ctx,
                                     Twine("loop") + Twine(Scope.NameIndex) +
                                         Twine(".breakc") +
                                         Twine(Scope.LoopBreakIndex++),
                                     pFunction);
      } else {
        pNextBB = BasicBlock::Create(m_Ctx,
                                     Twine("switch") + Twine(Scope.NameIndex) +
                                         Twine(".break") +
                                         Twine(Scope.SwitchBreakIndex++),
                                     pFunction);
      }

      // Insert cbranch to PostScope and Next BB.
      m_pBuilder->CreateCondBr(pCond, Scope.pPostScopeBB, pNextBB);

      // Set Next BB as active.
      m_pBuilder->SetInsertPoint(pNextBB);
      break;
    }

    case D3D10_SB_OPCODE_LABEL: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      DXASSERT_DXBC(Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_LABEL ||
                    Inst.m_Operands[0].m_Type ==
                        D3D11_SB_OPERAND_TYPE_FUNCTION_BODY);
      unsigned LabelIdx = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      const bool IsFb =
          Inst.m_Operands[0].m_Type == D3D11_SB_OPERAND_TYPE_FUNCTION_BODY;
      auto &Label =
          IsFb ? m_InterfaceFunctionBodies[LabelIdx] : m_Labels[LabelIdx];
      // Create entry basic block.
      pFunction = Label.pFunc;
      BasicBlock *pBB = BasicBlock::Create(m_Ctx, "entry", pFunction);
      m_pBuilder->SetInsertPoint(pBB);
      IFT(m_ScopeStack.IsEmpty());
      (void)m_ScopeStack.Push(Scope::Function, nullptr);
      InsertSM50ResourceHandles();
      break;
    }

    case D3D10_SB_OPCODE_CALL: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      DXASSERT_DXBC(Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_LABEL);
      unsigned LabelIdx = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      auto &Label = m_Labels[LabelIdx];
      // Create call instruction.
      m_pBuilder->CreateCall(Label.pFunc);
      break;
    }

    case D3D11_SB_OPCODE_INTERFACE_CALL: {
      DXASSERT_DXBC(Inst.m_Operands[0].m_Type ==
                    D3D11_SB_OPERAND_TYPE_INTERFACE);
      DXASSERT_DXBC(Inst.m_Operands[0].m_IndexDimension ==
                    D3D10_SB_OPERAND_INDEX_2D);
      DXASSERT_DXBC(Inst.m_Operands[0].m_IndexType[0] ==
                    D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      unsigned BaseIfaceIdx = Inst.m_Operands[0].m_Index[0].m_RegIndex;
      unsigned CallSiteIdx = Inst.m_InterfaceCall.FunctionIndex;
      Interface &Iface = m_Interfaces[BaseIfaceIdx];
      DXASSERT_DXBC(Inst.m_Operands[0].m_IndexType[0] ==
                        D3D10_SB_OPERAND_INDEX_IMMEDIATE32 ||
                    Iface.bDynamicallyIndexed);

      Value *pIfaceArrayIdx = LoadOperandIndex(
          Inst.m_Operands[0].m_Index[1], Inst.m_Operands[0].m_IndexType[1]);
      Value *pIfaceIdx = m_pBuilder->CreateAdd(m_pOP->GetU32Const(BaseIfaceIdx),
                                               pIfaceArrayIdx);

      // Load function table index
      Value *pCBufferRetValue;
      {
        Value *Args[3];
        Args[0] = m_pOP->GetU32Const(
            (unsigned)OP::OpCode::CBufferLoadLegacy); // OpCode
        Args[1] = CreateHandle(
            m_pInterfaceDataBuffer->GetClass(), m_pInterfaceDataBuffer->GetID(),
            m_pOP->GetU32Const(m_pInterfaceDataBuffer->GetLowerBound()),
            false /*Nonuniform*/); // CBuffer handle
        Args[2] = pIfaceIdx;       // 0-based index into cbuffer instance
        Function *pCBufferLoadFunc = m_pOP->GetOpFunc(
            OP::OpCode::CBufferLoadLegacy, Type::getInt32Ty(m_Ctx));

        pCBufferRetValue = m_pBuilder->CreateCall(pCBufferLoadFunc, Args);
        pCBufferRetValue = m_pBuilder->CreateExtractValue(pCBufferRetValue, 0);
      }

      // Switch on function table index
      // Create endswitch BB.
      BasicBlock *pPostSwitchBB = BasicBlock::Create(
          m_Ctx, Twine("fcall") + Twine(m_FcallCount) + Twine(".end"));
      SwitchInst *pSwitch =
          m_pBuilder->CreateSwitch(pCBufferRetValue, pPostSwitchBB);
      for (unsigned caseIdx = 0; caseIdx < Iface.Tables.size(); ++caseIdx) {
        BasicBlock *pCaseBB =
            BasicBlock::Create(m_Ctx,
                               Twine("fcall") + Twine(m_FcallCount) +
                                   Twine(".case") + Twine(caseIdx),
                               pFunction);
        m_pBuilder->SetInsertPoint(pCaseBB);

        unsigned fbIdx = m_FunctionTables[Iface.Tables[caseIdx]][CallSiteIdx];
        m_pBuilder->CreateCall(m_InterfaceFunctionBodies[fbIdx].pFunc);
        m_pBuilder->CreateBr(pPostSwitchBB);

        pSwitch->addCase(m_pBuilder->getInt32(Iface.Tables[caseIdx]), pCaseBB);
      }

      pFunction->getBasicBlockList().push_back(pPostSwitchBB);
      m_pBuilder->SetInsertPoint(pPostSwitchBB);
      ++m_FcallCount;
      break;
    }

    case D3D10_SB_OPCODE_CALLC: {
      DXASSERT_DXBC(Inst.m_NumOperands == 2);
      DXASSERT_DXBC(Inst.m_Operands[1].m_Type == D3D10_SB_OPERAND_TYPE_LABEL);
      unsigned LabelIdx = Inst.m_Operands[1].m_Index[0].m_RegIndex;
      auto &Label = m_Labels[LabelIdx];

      // Prepare condition.
      Value *pCond = LoadZNZCondition(Inst, 0);

      // Create call and after-call BBs.
      Function *pCurFunc = m_pBuilder->GetInsertBlock()->getParent();
      BasicBlock *pCallBB = BasicBlock::Create(
          m_Ctx, Twine("label") + Twine(LabelIdx) + Twine(".callc"), pCurFunc);
      BasicBlock *pPostCallBB = BasicBlock::Create(
          m_Ctx, Twine("label") + Twine(LabelIdx) + Twine(".callc"), pCurFunc);

      // Create cbranch for callc.
      m_pBuilder->CreateCondBr(pCond, pCallBB, pPostCallBB);
      m_pBuilder->SetInsertPoint(pCallBB);

      // Create call.
      m_pBuilder->CreateCall(Label.pFunc);
      m_pBuilder->CreateBr(pPostCallBB);
      m_pBuilder->SetInsertPoint(pPostCallBB);
      break;
    }

    case D3D10_SB_OPCODE_RET: {
      // Find parent scope.
      Scope &FuncScope = m_ScopeStack.FindParentFunction();

      if ((FuncScope.IsEntry() && !m_bPatchConstantPhase) ||
          !FuncScope.IsEntry()) {
        m_pBuilder->CreateRetVoid();
        BasicBlock *pAfterRet =
            BasicBlock::Create(m_Ctx, Twine("afterret"), pFunction);
        m_pBuilder->SetInsertPoint(pAfterRet);
      } else {
        // Hull shader control point phase fork/join.
        Scope &HullScope = m_ScopeStack.FindParentHullLoop();
        BasicBlock *pAfterRet =
            BasicBlock::Create(m_Ctx, Twine("afterret"), pFunction);

        if (m_ScopeStack.Top().Kind == Scope::HullLoop) {
          bMustCloseHullLoop = true;
          m_pBuilder->CreateBr(pAfterRet);
        } else {
          // A non-terminating return.
          m_pBuilder->CreateBr(HullScope.pPostScopeBB);
        }

        m_pBuilder->SetInsertPoint(pAfterRet);
      }
      break;
    }

    case D3D10_SB_OPCODE_RETC: {
      DXASSERT_DXBC(Inst.m_NumOperands == 1);
      // Find parent scope.
      Scope &FuncScope = m_ScopeStack.FindParentFunction();

      // Prepare condition.
      Value *pCond = LoadZNZCondition(Inst, 0);

      if ((FuncScope.IsEntry() && !m_bPatchConstantPhase) ||
          !FuncScope.IsEntry()) {
        // Create retc and after-retc BB.
        BasicBlock *pRetc = BasicBlock::Create(
            m_Ctx,
            Twine("label") + Twine(FuncScope.LabelIdx) + Twine(".callc") +
                Twine(FuncScope.CallIdx) + Twine(".retc") +
                Twine(FuncScope.ReturnIndex),
            pFunction);
        BasicBlock *pAfterRetc = BasicBlock::Create(
            m_Ctx,
            Twine("label") + Twine(FuncScope.LabelIdx) + Twine(".callc") +
                Twine(FuncScope.CallIdx) + Twine(".afterretc") +
                Twine(FuncScope.ReturnIndex++),
            pFunction);

        // Create cbranch for retc.
        m_pBuilder->CreateCondBr(pCond, pRetc, pAfterRetc);

        // Emit return.
        m_pBuilder->SetInsertPoint(pRetc);
        m_pBuilder->CreateRetVoid();
        m_pBuilder->SetInsertPoint(pAfterRetc);
      } else {
        // Hull shader control point phase fork/join.
        Scope &HullScope = m_ScopeStack.FindParentHullLoop();

        // Create HullLoopBreak and AfterHullLoopBreak BB.
        BasicBlock *pAfterHullBreakc = BasicBlock::Create(
            m_Ctx,
            Twine("hullloop") + Twine(FuncScope.NameIndex) + Twine(".retc") +
                Twine(FuncScope.HullLoopBreakIndex) + Twine(".afterretc"),
            pFunction);

        // Create cbranch for retc (HullLoopBreak).
        m_pBuilder->CreateCondBr(pCond, HullScope.pPostScopeBB,
                                 pAfterHullBreakc);
        m_pBuilder->SetInsertPoint(pAfterHullBreakc);
      }

      break;
    }

    case D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE:
      IFTBOOL(m_ScopeStack.FindParentFunction().IsEntry(), E_FAIL);
      m_bControlPointPhase = true;
      break;

    case D3D11_SB_OPCODE_HS_FORK_PHASE:
    case D3D11_SB_OPCODE_HS_JOIN_PHASE: {
      if (!m_bPatchConstantPhase) {
        if (!m_bControlPointPhase) {
          // This is a pass-through CP HS.
          bPasshThroughCP = true;
        }
        m_bControlPointPhase = false;
        m_bPatchConstantPhase = true;

        // Start patch constant function.
        (void)m_ScopeStack.Push(Scope::Function, nullptr);
        m_ScopeStack.Top().SetEntry(true);
        pFunction = Function::Create(pEntryFuncType,
                                     GlobalValue::LinkageTypes::ExternalLinkage,
                                     "pc_main", m_pModule.get());
        pFunction->setCallingConv(CallingConv::C);
        m_pPR->SetPatchConstantFunction(pFunction);
        BasicBlock *pBB = BasicBlock::Create(m_Ctx, "entry", pFunction);
        m_pBuilder->SetInsertPoint(pBB);

        // Swap active x-registers.
        m_IndexableRegs.swap(m_PatchConstantIndexableRegs);

        DeclareIndexableRegisters();

        // Create HullLoop induction variable.
        pHullLoopInductionVar = m_pBuilder->CreateAlloca(
            Type::getInt32Ty(m_Ctx), nullptr, "InstanceID");

        InsertSM50ResourceHandles();
      }

      // Create HullLoop-scope.
      Scope &Scope =
          m_ScopeStack.Push(Scope::HullLoop, m_pBuilder->GetInsertBlock());

      // Initialize HullLoop induction variable.
      Scope.pInductionVar = pHullLoopInductionVar;
      m_pBuilder->CreateStore(m_pOP->GetI32Const(0), Scope.pInductionVar);

      Scope.HullLoopTripCount =
          m_PatchConstantPhaseInstanceCounts[ForkJoinPhaseIndex];
      ForkJoinPhaseIndex++;

      // Create HullLoop and EndHullLoop BBs.
      Scope.pHullLoopBB = BasicBlock::Create(
          m_Ctx, Twine("hullloop") + Twine(Scope.NameIndex), pFunction);
      Scope.pPostScopeBB = BasicBlock::Create(
          m_Ctx, Twine("hullloop") + Twine(Scope.NameIndex) + Twine(".end"));

      // Insert branch to Loop BB.
      m_pBuilder->CreateBr(Scope.pLoopBB);

      // Set Loop BB as active.
      m_pBuilder->SetInsertPoint(Scope.pLoopBB);
      break;
    }

    case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
    case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
      break;

    //
    // Pixel shader.
    //
    case D3D10_1_SB_OPCODE_LOD: {
      OP::OpCode OpCode = OP::OpCode::CalculateLOD;
      const unsigned uOpOutput = 0;
      const unsigned uOpCoord = uOpOutput + 1;
      const unsigned uOpSRV = uOpCoord + 1;
      const unsigned uOpSampler = uOpSRV + 1;
      DXASSERT_DXBC(Inst.m_Operands[uOpSRV].m_Type ==
                    D3D10_SB_OPERAND_TYPE_RESOURCE);

      OperandValue InCoord, InSRV, InSampler;
      // Resource.
      const DxilResource &R = LoadSRVOperand(
          InSRV, Inst, uOpSRV, CMask::MakeXMask(), CompType::getInvalid());
      // Coordinates.
      CMask CoordMask =
          CMask::MakeFirstNCompMask(DXBC::GetNumResOffsets(R.GetKind()));
      LoadOperand(InCoord, Inst, uOpCoord, CoordMask, CompType::getF32());
      // Sampler.
      LoadOperand(InSampler, Inst, uOpSampler, CMask::MakeXMask(),
                  CompType::getInvalid());

      // Create CalculateLOD call.
      Value *Args[7];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = InSRV[0];                             // Resource handle
      Args[2] = InSampler[0];                         // Sampler handle
      Args[3] = CoordMask.IsSet(0) ? InCoord[0] : m_pUnusedF32;
      Args[4] = CoordMask.IsSet(1) ? InCoord[1] : m_pUnusedF32;
      Args[5] = CoordMask.IsSet(2) ? InCoord[2] : m_pUnusedF32;

      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);

      // Create unclamped CalculateLOD.
      Args[6] = m_pOP->GetI1Const(false); // Unclamped
      Value *pOpRetUnclamped = m_pBuilder->CreateCall(F, Args);
      // Create clamped CalculateLOD.
      Args[6] = m_pOP->GetI1Const(true); // Clamped
      Value *pOpRetClamped = m_pBuilder->CreateCall(F, Args);

      CMask OutputMask =
          CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
      OperandValue Out;
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!OutputMask.IsSet(c))
          continue;

        // Respect swizzle: resource swizzle == return value swizzle.
        BYTE Comp = Inst.m_Operands[uOpSRV].m_Swizzle[c];

        switch (Comp) {
        case 0:
          Out[c] = pOpRetClamped;
          break;
        case 1:
          Out[c] = pOpRetUnclamped;
          break;
        case 2:
          LLVM_FALLTHROUGH;
        case 3:
          Out[c] = m_pOP->GetFloatConst(0.f);
          break;
        default:
          DXASSERT_DXBC(false);
        }
      }
      StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);

      break;
    }

    case D3D10_SB_OPCODE_DISCARD: {
      OP::OpCode OpCode = OP::OpCode::Discard;

      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = LoadZNZCondition(Inst, 0);            // Condition

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      MarkPrecise(m_pBuilder->CreateCall(F, Args));
      break;
    }

    case D3D10_SB_OPCODE_DERIV_RTX:
      LLVM_FALLTHROUGH;
    case D3D11_SB_OPCODE_DERIV_RTX_COARSE:
      ConvertUnary(OP::OpCode::DerivCoarseX, CompType::getF32(), Inst);
      break;
    case D3D10_SB_OPCODE_DERIV_RTY:
      LLVM_FALLTHROUGH;
    case D3D11_SB_OPCODE_DERIV_RTY_COARSE:
      ConvertUnary(OP::OpCode::DerivCoarseY, CompType::getF32(), Inst);
      break;
    case D3D11_SB_OPCODE_DERIV_RTX_FINE:
      ConvertUnary(OP::OpCode::DerivFineX, CompType::getF32(), Inst);
      break;
    case D3D11_SB_OPCODE_DERIV_RTY_FINE:
      ConvertUnary(OP::OpCode::DerivFineY, CompType::getF32(), Inst);
      break;

    case D3D11_SB_OPCODE_EVAL_SNAPPED: {
      OP::OpCode OpCode = OP::OpCode::EvalSnapped;
      const unsigned uOpOutput = 0;
      const unsigned uOpInput = uOpOutput + 1;
      const unsigned uOpOffset = uOpInput + 1;

      OperandValue InOffset;
      CMask OutputMask =
          CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
      LoadOperand(InOffset, Inst, uOpOffset, CMask::MakeFirstNCompMask(2),
                  CompType::getI32());
      const D3D10ShaderBinary::COperandBase &OpInput =
          Inst.m_Operands[uOpInput];
      DXASSERT_NOMSG(Inst.m_Operands[uOpInput].m_IndexDimension ==
                     D3D10_SB_OPERAND_INDEX_1D);
      unsigned Register = OpInput.m_Index[0].m_RegIndex;
      Value *pRowIndexValue =
          LoadOperandIndex(OpInput.m_Index[0], OpInput.m_IndexType[0]);

      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);

      Value *Args[6];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[4] = InOffset[0];                          // Offset X
      Args[5] = InOffset[1];                          // Offset Y

      OperandValue Out;
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!OutputMask.IsSet(c))
          continue;

        BYTE Comp = OpInput.m_Swizzle[c];
        // Retrieve signature element.
        const DxilSignatureElement *E =
            m_pInputSignature->GetElement(Register, Comp);

        // Make row/col index relative within element.
        Value *pRowIndexValueRel = m_pBuilder->CreateSub(
            pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));

        Args[1] = m_pOP->GetU32Const(E->GetID()); // Input signature element ID
        Args[2] = pRowIndexValueRel; // Row, relative to the element
        Args[3] = m_pOP->GetU8Const(
            Comp - E->GetStartCol()); // Col, relative to the element

        Out[c] = m_pBuilder->CreateCall(F, Args);
      }
      StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);

      break;
    }

    case D3D11_SB_OPCODE_EVAL_SAMPLE_INDEX: {
      OP::OpCode OpCode = OP::OpCode::EvalSampleIndex;
      const unsigned uOpOutput = 0;
      const unsigned uOpInput = uOpOutput + 1;
      const unsigned uOpSampleIndex = uOpInput + 1;

      CMask OutputMask =
          CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
      OperandValue InSampleIndex;
      LoadOperand(InSampleIndex, Inst, uOpSampleIndex, CMask::MakeXMask(),
                  CompType::getI32());
      const D3D10ShaderBinary::COperandBase &OpInput =
          Inst.m_Operands[uOpInput];
      DXASSERT_NOMSG(Inst.m_Operands[uOpInput].m_IndexDimension ==
                     D3D10_SB_OPERAND_INDEX_1D);
      unsigned Register = OpInput.m_Index[0].m_RegIndex;
      Value *pRowIndexValue =
          LoadOperandIndex(OpInput.m_Index[0], OpInput.m_IndexType[0]);

      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);

      Value *Args[5];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[4] = InSampleIndex[0];                     // Sample index

      OperandValue Out;
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!OutputMask.IsSet(c))
          continue;

        BYTE Comp = OpInput.m_Swizzle[c];
        // Retrieve signature element.
        const DxilSignatureElement *E =
            m_pInputSignature->GetElement(Register, Comp);

        // Make row/col index relative within element.
        Value *pRowIndexValueRel = m_pBuilder->CreateSub(
            pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));

        Args[1] = m_pOP->GetU32Const(E->GetID()); // Input signature element ID
        Args[2] = pRowIndexValueRel; // Row, relative to the element
        Args[3] = m_pOP->GetU8Const(
            Comp - E->GetStartCol()); // Col, relative to the element

        Out[c] = m_pBuilder->CreateCall(F, Args);
      }
      StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);

      break;
    }

    case D3D11_SB_OPCODE_EVAL_CENTROID: {
      OP::OpCode OpCode = OP::OpCode::EvalCentroid;
      const unsigned uOpOutput = 0;
      const unsigned uOpInput = uOpOutput + 1;

      CMask OutputMask =
          CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
      const D3D10ShaderBinary::COperandBase &OpInput =
          Inst.m_Operands[uOpInput];
      DXASSERT_NOMSG(Inst.m_Operands[uOpInput].m_IndexDimension ==
                     D3D10_SB_OPERAND_INDEX_1D);
      unsigned Register = OpInput.m_Index[0].m_RegIndex;
      Value *pRowIndexValue =
          LoadOperandIndex(OpInput.m_Index[0], OpInput.m_IndexType[0]);

      CompType DstType = DXBC::GetCompTypeWithMinPrec(
          CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);
      Type *pDstType = DstType.GetLLVMType(m_Ctx);
      Function *F = m_pOP->GetOpFunc(OpCode, pDstType);

      Value *Args[4];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode

      OperandValue Out;
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!OutputMask.IsSet(c))
          continue;

        BYTE Comp = OpInput.m_Swizzle[c];
        // Retrieve signature element.
        const DxilSignatureElement *E =
            m_pInputSignature->GetElement(Register, Comp);

        // Make row/col index relative within element.
        Value *pRowIndexValueRel = m_pBuilder->CreateSub(
            pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));

        Args[1] = m_pOP->GetU32Const(E->GetID()); // Input signature element ID
        Args[2] = pRowIndexValueRel; // Row, relative to the element
        Args[3] = m_pOP->GetU8Const(
            Comp - E->GetStartCol()); // Col, relative to the element

        Out[c] = m_pBuilder->CreateCall(F, Args);
      }
      StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);

      break;
    }

    case D3D10_SB_OPCODE_EMIT:
    case D3D11_SB_OPCODE_EMIT_STREAM: {
      OP::OpCode OpCode = OP::OpCode::EmitStream;
      BYTE StreamId = 0;

      if (Inst.OpCode() == D3D11_SB_OPCODE_EMIT_STREAM) {
        StreamId = (BYTE)Inst.m_Operands[0].m_Index[0].m_RegIndex;
      }

      // For GS with multiple streams, capture the values of output registers at
      // the emit point.
      if (m_pPR->HasMultipleOutputStreams()) {
        EmitGSOutputRegisterStore(StreamId);
      }

      // Create EmitStream call.
      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU8Const(StreamId);          // Stream ID

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      MarkPrecise(m_pBuilder->CreateCall(F, Args));

      break;
    }

    case D3D10_SB_OPCODE_CUT:
    case D3D11_SB_OPCODE_CUT_STREAM: {
      OP::OpCode OpCode = OP::OpCode::CutStream;
      BYTE StreamId = 0;

      if (Inst.OpCode() == D3D11_SB_OPCODE_CUT_STREAM) {
        StreamId = (BYTE)Inst.m_Operands[0].m_Index[0].m_RegIndex;
      }

      // Create CutStream call.
      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU8Const(StreamId);          // Stream ID

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      MarkPrecise(m_pBuilder->CreateCall(F, Args));

      break;
    }

    case D3D10_SB_OPCODE_EMITTHENCUT:
    case D3D11_SB_OPCODE_EMITTHENCUT_STREAM: {
      OP::OpCode OpCode = OP::OpCode::EmitThenCutStream;
      BYTE StreamId = 0;

      if (Inst.OpCode() == D3D11_SB_OPCODE_EMITTHENCUT_STREAM) {
        StreamId = (BYTE)Inst.m_Operands[0].m_Index[0].m_RegIndex;
      }

      // For GS with multiple streams, capture the values of output registers at
      // the emit point.
      if (m_pPR->HasMultipleOutputStreams()) {
        EmitGSOutputRegisterStore(StreamId);
      }

      // Create EmitThenCutStream call.
      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU8Const(StreamId);          // Stream ID

      Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
      MarkPrecise(m_pBuilder->CreateCall(F, Args));

      break;
    }

    case D3D10_SB_OPCODE_NOP:
      break;

    default:
      HandleUnknownInstruction(Inst);
      break;
    }
  }

  DXASSERT_NOMSG(m_ScopeStack.IsEmpty());

  if (bPasshThroughCP) {
    Function *Entry = m_pPR->GetEntryFunction();
    m_pPR->SetEntryFunction(nullptr);
    Entry->eraseFromParent();
    m_pPR->SetEntryFunctionName("");
  }

  CleanupIndexableRegisterDecls(m_IndexableRegs);
  CleanupIndexableRegisterDecls(m_PatchConstantIndexableRegs);

  RemoveUnreachableBasicBlocks();
  CleanupGEP();
}

void DxbcConverter::LogConvertResult(bool InDriver,
                                     const LARGE_INTEGER *pQPCConvertStart,
                                     const LARGE_INTEGER *pQPCConvertEnd,
                                     LPCVOID pDxbc, UINT32 DxbcSize,
                                     LPCWSTR pExtraOptions, LPCVOID pConverted,
                                     UINT32 ConvertedSize, HRESULT hr) {
  // intentionaly empty - override to report conversion results
}

HRESULT DxbcConverter::PreConvertHook(const CShaderToken *pByteCode) {
  return S_OK;
}

HRESULT DxbcConverter::PostConvertHook(const CShaderToken *pByteCode) {
  return S_OK;
}

void DxbcConverter::HandleUnknownInstruction(
    D3D10ShaderBinary::CInstruction &Inst) {
  DXASSERT_ARGS(false, "OpCode %u is not yet implemented", Inst.OpCode());
}

unsigned DxbcConverter::GetResourceSlot(D3D10ShaderBinary::CInstruction &Inst) {
  return DXBC::GetResourceSlot(Inst.OpCode());
}

void DxbcConverter::AdvanceDxbcInstructionStream(
    D3D10ShaderBinary::CShaderCodeParser &Parser,
    D3D10ShaderBinary::CInstruction &Inst, bool &bDoneParsing) {
  if (bDoneParsing)
    return;

  if (!Parser.EndOfShader()) {
    DXASSERT_NOMSG(!bDoneParsing);
    Parser.ParseInstruction(&Inst);
  } else {
    IFTBOOL(!bDoneParsing, E_FAIL);
    bDoneParsing = true;
  }
}

bool DxbcConverter::GetNextDxbcInstruction(
    D3D10ShaderBinary::CShaderCodeParser &Parser,
    D3D10ShaderBinary::CInstruction &NextInst) {
  if (Parser.EndOfShader()) {
    return false;
  }

  UINT CurPos = Parser.CurrentTokenOffset();
  Parser.ParseInstruction(&NextInst);
  Parser.SetCurrentTokenOffset(CurPos);
  return true;
}

void DxbcConverter::InsertSM50ResourceHandles() {
  // Create resource handles for SM5.0- to reduce the number of call
  // instructions (to reduce IR size). Later: it may be worthwhile to implement
  // a pass to hoist handle creation for SM5.1 here when the index into range is
  // constant and used more than once within the shader.
  if (!IsSM51Plus()) {
    for (size_t i = 0; i < m_pPR->GetSRVs().size(); ++i) {
      DxilResource &R = m_pPR->GetSRV(i);
      SetCachedHandle(R);
    }
    for (size_t i = 0; i < m_pPR->GetUAVs().size(); ++i) {
      DxilResource &R = m_pPR->GetUAV(i);
      SetCachedHandle(R);
    }
    for (size_t i = 0; i < m_pPR->GetCBuffers().size(); ++i) {
      DxilCBuffer &R = m_pPR->GetCBuffer(i);
      SetCachedHandle(R);
    }
    for (size_t i = 0; i < m_pPR->GetSamplers().size(); ++i) {
      DxilSampler &R = m_pPR->GetSampler(i);
      SetCachedHandle(R);
    }
  }
}

void DxbcConverter::InsertInterfacesResourceDecls() {
  // Insert decls for:
  // 1. CB14 containing interface table selections, along with "this pointer"
  // information,
  // 2. 14 CBs in space 1,
  // 3. 32 samplers in space 1 and 32 comparison samplers in space 2
  // SRVs will be inserted dynamically as needed
  if (m_pInterfaceDataBuffer) {
    return;
  }

  m_pPR->m_ShaderFlags.SetAllResourcesBound(false);

  // Create interface data buffer
  {
    unsigned ID = m_pPR->AddCBuffer(unique_ptr<DxilCBuffer>(new DxilCBuffer));
    DxilCBuffer &R = m_pPR->GetCBuffer(ID); // R == record
    m_pInterfaceDataBuffer = &R;
    R.SetID(ID);
    // Root signature bindings.
    unsigned CBufferSize =
        D3D11_SHADER_MAX_INTERFACES * 8 /*UINTs per interface*/ * sizeof(UINT);
    R.SetLowerBound(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT); // 14
    R.SetRangeSize(1);
    R.SetSpaceID(0);
    // Declare global variable.
    R.SetGlobalName(SynthesizeResGVName("CB", R.GetID()));
    StructType *pResType = GetStructResElemType(CBufferSize);
    R.SetGlobalSymbol(DeclareUndefPtr(pResType, DXIL::kCBufferAddrSpace));

    // CBuffer-specific state.
    R.SetSize(CBufferSize);
  }

  // Create CB array for class instances
  {
    unsigned ID = m_pPR->AddCBuffer(unique_ptr<DxilCBuffer>(new DxilCBuffer));
    DxilCBuffer &R = m_pPR->GetCBuffer(ID); // R == record
    m_pClassInstanceCBuffers = &R;
    R.SetID(ID);
    // Root signature bindings.
    unsigned CBufferSize = DXIL::kMaxCBufferSize * DXBC::kWidth * 4;
    R.SetLowerBound(0);
    R.SetRangeSize(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT); // 14
    R.SetSpaceID(1);
    // Declare global variable.
    R.SetGlobalName(SynthesizeResGVName("CB", R.GetID()));
    StructType *pResType = GetStructResElemType(CBufferSize);
    R.SetGlobalSymbol(DeclareUndefPtr(pResType, DXIL::kCBufferAddrSpace));

    // CBuffer-specific state.
    R.SetSize(CBufferSize);
  }

  // Create sampler arrays for class instances
  for (unsigned i = 0; i < 2; ++i) {
    unsigned ID = m_pPR->AddSampler(unique_ptr<DxilSampler>(new DxilSampler));
    DxilSampler &R = m_pPR->GetSampler(ID); // R == record
    R.SetID(ID);
    // Root signature bindings.
    R.SetLowerBound(0);
    R.SetRangeSize(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
    R.SetSpaceID(i + 1);
    // Declare global variable.
    R.SetGlobalName(SynthesizeResGVName("S", R.GetID()));
    string ResTypeName("dx.types.Sampler");
    StructType *pResType = m_pModule->getTypeByName(ResTypeName);
    if (pResType == nullptr) {
      pResType = StructType::create(m_Ctx, ResTypeName);
    }
    R.SetGlobalSymbol(DeclareUndefPtr(pResType, DXIL::kDeviceMemoryAddrSpace));

    // Sampler-specific state.
    R.SetSamplerKind(i == 0 ? DXIL::SamplerKind::Default
                            : DXIL::SamplerKind::Comparison);
    DxilSampler *&pSampler = (i == 0 ? m_pClassInstanceSamplers
                                     : m_pClassInstanceComparisonSamplers);
    pSampler = &R;
  }
}

const DxilResource &
DxbcConverter::GetInterfacesSRVDecl(D3D10ShaderBinary::CInstruction &Inst) {
  InterfaceShaderResourceKey Key = {};
  DXASSERT_DXBC(Inst.m_ExtendedOpCodeCount ==
                2); // Extended resource dimension and return type
  Key.Kind = DXBC::GetResourceKind(Inst.m_ResourceDimEx);
  if (Inst.m_ResourceDimEx == D3D11_SB_RESOURCE_DIMENSION_STRUCTURED_BUFFER) {
    Key.StructureByteStride = Inst.m_ResourceDimStructureStrideEx;
  } else if (Inst.m_ResourceDimEx != D3D11_SB_RESOURCE_DIMENSION_RAW_BUFFER) {
    Key.TypedSRVRet =
        DXBC::GetDeclResCompType(Inst.m_ResourceReturnTypeEx[0]).GetKind();
  }
  auto iter = m_ClassInstanceSRVs.find(Key);
  if (iter != m_ClassInstanceSRVs.end()) {
    return m_pPR->GetSRV(iter->second);
  }

  unsigned ID = m_pPR->AddSRV(unique_ptr<DxilResource>(new DxilResource));
  DxilResource &R = m_pPR->GetSRV(ID); // R == record
  R.SetID(ID);
  R.SetRW(false);
  // Root signature bindings.
  R.SetLowerBound(0);
  R.SetRangeSize(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
  R.SetSpaceID(m_ClassInstanceSRVs.size() + 1);

  unsigned SampleCount = (Key.Kind == DXIL::ResourceKind::Texture2DMS ||
                          Key.Kind == DXIL::ResourceKind::Texture2DMSArray)
                             ? 4
                             : 0;
  DXASSERT_DXBC(
      SampleCount ==
      0); // Don't expect to actually see this used within interfaces...

  // Resource-specific state.
  StructType *pResType = nullptr;
  switch (Inst.m_ResourceDimEx) {
  default: {
    R.SetKind(DXBC::GetResourceKind(Inst.m_ResourceDimEx));
    const unsigned kTypedBufferElementSizeInBytes = 4;
    R.SetElementStride(kTypedBufferElementSizeInBytes);
    R.SetSampleCount(SampleCount);
    CompType DeclCT = DXBC::GetDeclResCompType(Inst.m_ResourceReturnTypeEx[0]);
    if (DeclCT.IsInvalid())
      DeclCT = CompType::getU32();
    R.SetCompType(DeclCT);
    pResType = GetTypedResElemType(DeclCT);
    break;
  }
  case D3D11_SB_RESOURCE_DIMENSION_RAW_BUFFER: {
    R.SetKind(DxilResource::Kind::RawBuffer);
    const unsigned kRawBufferElementSizeInBytes = 1;
    R.SetElementStride(kRawBufferElementSizeInBytes);
    pResType = GetTypedResElemType(CompType::getU32());
    break;
  }
  case D3D11_SB_RESOURCE_DIMENSION_STRUCTURED_BUFFER: {
    R.SetKind(DxilResource::Kind::StructuredBuffer);
    unsigned Stride = Inst.m_ResourceDimStructureStrideEx;
    R.SetElementStride(Stride);
    pResType = GetStructResElemType(Stride);
    break;
  }
  }

  // Declare global variable.
  R.SetGlobalName(SynthesizeResGVName("T", R.GetID()));
  R.SetGlobalSymbol(DeclareUndefPtr(pResType, DXIL::kDeviceMemoryAddrSpace));
  m_ClassInstanceSRVs[Key] = ID;
  return R;
}

void DxbcConverter::DeclareIndexableRegisters() {
  // Reserve storage for x-registers.
  if (!HasLabels()) {
    // Only main subroutine: use alloca, as optimization.
    for (auto &IR : m_IndexableRegs) {
      DXASSERT_NOMSG(IR.second.pValue32 == nullptr &&
                     IR.second.pValue16 == nullptr);
      Type *pType32 = ArrayType::get(Type::getFloatTy(m_Ctx),
                                     IR.second.NumRegs * IR.second.NumComps);
      AllocaInst *pAlloca32 = m_pBuilder->CreateAlloca(
          pType32, nullptr, Twine("dx.v32.x") + Twine(IR.first));
      pAlloca32->setAlignment(kRegCompAlignment);
      IR.second.pValue32 = pAlloca32;
      Type *pType16 = ArrayType::get(Type::getHalfTy(m_Ctx),
                                     IR.second.NumRegs * IR.second.NumComps);
      AllocaInst *pAlloca16 = m_pBuilder->CreateAlloca(
          pType16, nullptr, Twine("dx.v16.x") + Twine(IR.first));
      pAlloca16->setAlignment(kRegCompAlignment);
      IR.second.pValue16 = pAlloca16;
      IR.second.bIsAlloca = true;
    }
  } else {
    // Several subroutines: use global storage.
    for (auto &IR : m_IndexableRegs) {
      Type *pType32 = ArrayType::get(Type::getFloatTy(m_Ctx),
                                     IR.second.NumRegs * IR.second.NumComps);
      GlobalVariable *pGV32 = new GlobalVariable(
          *m_pModule, pType32, false, GlobalValue::InternalLinkage,
          UndefValue::get(pType32), Twine("dx.v32.x") + Twine(IR.first),
          nullptr, GlobalVariable::NotThreadLocal, DXIL::kDefaultAddrSpace);
      IR.second.pValue32 = pGV32;
      Type *pType16 = ArrayType::get(Type::getHalfTy(m_Ctx),
                                     IR.second.NumRegs * IR.second.NumComps);
      GlobalVariable *pGV16 = new GlobalVariable(
          *m_pModule, pType16, false, GlobalValue::InternalLinkage,
          UndefValue::get(pType16), Twine("dx.v16.x") + Twine(IR.first),
          nullptr, GlobalVariable::NotThreadLocal, DXIL::kDefaultAddrSpace);
      IR.second.pValue16 = pGV16;
      IR.second.bIsAlloca = false;
    }
  }
}

void DxbcConverter::CleanupIndexableRegisterDecls(
    map<unsigned, IndexableReg> &IdxRegMap) {
  for (auto &IR : IdxRegMap) {
    if (IR.second.pValue32 && !IR.second.pValue32->hasNUsesOrMore(1)) {
      if (IR.second.bIsAlloca)
        cast<Instruction>(IR.second.pValue32)->eraseFromParent();
      else
        cast<GlobalVariable>(IR.second.pValue32)->eraseFromParent();
    }
    if (IR.second.pValue16 && !IR.second.pValue16->hasNUsesOrMore(1)) {
      if (IR.second.bIsAlloca)
        cast<Instruction>(IR.second.pValue16)->eraseFromParent();
      else
        cast<GlobalVariable>(IR.second.pValue16)->eraseFromParent();
    }
  }
}

void DxbcConverter::RemoveUnreachableBasicBlocks() {
  for (auto itFn = m_pModule->begin(), endFn = m_pModule->end(); itFn != endFn;
       ++itFn) {
    Function *F = itFn;

    vector<BasicBlock *> NoPredSet;
    // 1. Detect basic blocks without predecessors.
    for (auto itBB = ++(F->begin()), endBB = F->end(); itBB != endBB; ++itBB) {
      BasicBlock *B = itBB;
      if (pred_begin(B) == pred_end(B)) {
        NoPredSet.emplace_back(B);
      }
    }

    // 2. Remove BBs with no predecessors.
    while (!NoPredSet.empty()) {
      BasicBlock *B = NoPredSet.back();
      NoPredSet.pop_back();

      TerminatorInst *pTI = B->getTerminator();
      vector<BasicBlock *> Successors(pTI->getNumSuccessors());
      for (unsigned i = 0; i < pTI->getNumSuccessors(); i++) {
        Successors[i] = pTI->getSuccessor(i);
      }

      B->eraseFromParent();

      for (auto S : Successors) {
        if (pred_begin(S) == pred_end(S)) {
          NoPredSet.emplace_back(S);
        }
      }
    }
  }
}

class GEPVisitor : public InstVisitor<GEPVisitor> {
public:
  void visitInstruction(Instruction &I) {
    for (Instruction::op_iterator itOp = I.op_begin(), endOp = I.op_end();
         itOp != endOp; ++itOp) {
      Value *V1 = itOp->get()->stripPointerCasts();

      if (GEPOperator *pGEP = dyn_cast<GEPOperator>(V1)) {
        bool bReplace = false;
        SmallVector<Value *, 4> GEPIndices;

        for (GEPOperator::op_iterator itOp = pGEP->idx_begin(),
                                      endOp = pGEP->idx_end();
             itOp != endOp; ++itOp) {
          Value *V = itOp->get();
          GEPIndices.push_back(V);

          if (ConstantInt *C = dyn_cast<ConstantInt>(V)) {
            LLVMContext &Ctx = C->getContext();

            if (C->getType() != Type::getInt32Ty(Ctx)) {
              uint64_t n = C->getZExtValue();

              if (n <= (uint64_t)(UINT32_MAX)) {
                GEPIndices.back() = Constant::getIntegerValue(
                    IntegerType::get(Ctx, 32), APInt(32, (unsigned)n));
                bReplace = true;
              }
            }
          }
        }

        if (bReplace) {
          Constant *pGEP2 = ConstantExpr::getGetElementPtr(
              pGEP->getPointerOperandType()->getPointerElementType(),
              dyn_cast<Constant>(pGEP->getPointerOperand()), GEPIndices);
          pGEP->replaceAllUsesWith(pGEP2);
        }
      }
    }
  }
};

// GEPOperators may get i64 constant index values.
// We replace them here with i32 values, if possible, to avoid 64-bit values in
// DXIL.
void DxbcConverter::CleanupGEP() {
  GEPVisitor a;
  a.visit(*m_pModule);
}

void DxbcConverter::ConvertUnary(OP::OpCode OpCode, const CompType &ElementType,
                                 D3D10ShaderBinary::CInstruction &Inst,
                                 const unsigned DstIdx, const unsigned SrcIdx) {
  DXASSERT_NOMSG(OP::GetOpCodeClass(OpCode) == OP::OpCodeClass::Unary ||
                 OP::GetOpCodeClass(OpCode) == OP::OpCodeClass::UnaryBits);
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      ElementType, Inst.m_Operands[DstIdx].m_MinPrecision);
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In, Out;
  LoadOperand(In, Inst, SrcIdx, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Out[c] = m_pBuilder->CreateCall(
        pFunc, {m_pOP->GetU32Const((unsigned)OpCode), In[c]});
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, OperationType);
}

void DxbcConverter::ConvertBinary(OP::OpCode OpCode,
                                  const CompType &ElementType,
                                  D3D10ShaderBinary::CInstruction &Inst,
                                  const unsigned DstIdx, const unsigned SrcIdx1,
                                  const unsigned SrcIdx2) {
  DXASSERT_NOMSG(OP::GetOpCodeClass(OpCode) == OP::OpCodeClass::Binary);
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      ElementType, Inst.m_Operands[DstIdx].m_MinPrecision);
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In1, In2, Out;
  LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
  LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Out[c] = m_pBuilder->CreateCall(
        pFunc, {m_pOP->GetU32Const((unsigned)OpCode), In1[c], In2[c]});

    if (ElementType.GetKind() == CompType::Kind::F64) {
      c++;
    }
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, OperationType);
}

void DxbcConverter::ConvertBinary(Instruction::BinaryOps OpCode,
                                  const CompType &ElementType,
                                  D3D10ShaderBinary::CInstruction &Inst,
                                  const unsigned DstIdx, const unsigned SrcIdx1,
                                  const unsigned SrcIdx2) {
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      ElementType, Inst.m_Operands[DstIdx].m_MinPrecision);

  OperandValue In1, In2, Out;
  LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
  LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Value *pVal2 = In2[c];
    // Limit shift amount to 5 bits.
    switch (OpCode) {
    case Instruction::Shl:
    case Instruction::AShr:
    case Instruction::LShr:
      pVal2 = m_pBuilder->CreateAnd(pVal2, 0x0000001F);
    }

    Out[c] = m_pBuilder->CreateBinOp(OpCode, In1[c], pVal2);

    if (ElementType.GetKind() == CompType::Kind::F64) {
      c++;
    }
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, OperationType);
}

void DxbcConverter::ConvertBinaryWithTwoOuts(
    OP::OpCode OpCode, D3D10ShaderBinary::CInstruction &Inst,
    const unsigned DstIdx1, const unsigned DstIdx2, const unsigned SrcIdx1,
    const unsigned SrcIdx2) {
  DXASSERT_NOMSG(OP::GetOpCodeClass(OpCode) ==
                 OP::OpCodeClass::BinaryWithTwoOuts);
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx1].m_WriteMask |
                                    Inst.m_Operands[DstIdx2].m_WriteMask);

  if (WriteMask.ToByte() == 0) {
    // No-op if both destinations are null
    DXASSERT_NOMSG(
        Inst.m_Operands[DstIdx1].m_Type == D3D10_SB_OPERAND_TYPE_NULL &&
        Inst.m_Operands[DstIdx2].m_Type == D3D10_SB_OPERAND_TYPE_NULL);
    return;
  }

  CMask Dst1Mask = CMask::FromDXBC(Inst.m_Operands[DstIdx1].m_WriteMask);
  CMask Dst2Mask = CMask::FromDXBC(Inst.m_Operands[DstIdx2].m_WriteMask);
  CompType OperationType = CompType::getI32();
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In1, In2, Out1, Out2;
  LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
  LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Value *pRes = m_pBuilder->CreateCall(
        pFunc, {m_pOP->GetU32Const((unsigned)OpCode), In1[c], In2[c]});
    pRes = MarkPrecise(pRes, c);
    Out1[c] = m_pBuilder->CreateExtractValue(pRes, 0);
    Out2[c] = m_pBuilder->CreateExtractValue(pRes, 1);
  }

  StoreOperand(Out1, Inst, DstIdx1, Dst1Mask, OperationType);
  StoreOperand(Out2, Inst, DstIdx2, Dst2Mask, OperationType);
}

void DxbcConverter::ConvertBinaryWithCarry(
    OP::OpCode OpCode, D3D10ShaderBinary::CInstruction &Inst,
    const unsigned DstIdx1, const unsigned DstIdx2, const unsigned SrcIdx1,
    const unsigned SrcIdx2) {
  DXASSERT_NOMSG(OP::GetOpCodeClass(OpCode) ==
                 OP::OpCodeClass::BinaryWithCarryOrBorrow);
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx1].m_WriteMask |
                                    Inst.m_Operands[DstIdx2].m_WriteMask);
  CompType OperationType = CompType::getI32();
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In1, In2, Out1, Out2;
  LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
  LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Value *pRes = m_pBuilder->CreateCall(
        pFunc, {m_pOP->GetU32Const((unsigned)OpCode), In1[c], In2[c]});
    pRes = MarkPrecise(pRes, c);
    Out1[c] = m_pBuilder->CreateExtractValue(pRes, 0);
    Out2[c] = m_pBuilder->CreateExtractValue(pRes, 1);
    Out2[c] = m_pBuilder->CreateZExt(Out2[c], Type::getInt32Ty(m_Ctx));
  }

  StoreOperand(Out1, Inst, DstIdx1, WriteMask, OperationType);
  StoreOperand(Out2, Inst, DstIdx2, WriteMask, CompType::getI32());
}

void DxbcConverter::ConvertTertiary(
    OP::OpCode OpCode, const CompType &ElementType,
    D3D10ShaderBinary::CInstruction &Inst, const unsigned DstIdx,
    const unsigned SrcIdx1, const unsigned SrcIdx2, const unsigned SrcIdx3) {
  DXASSERT_NOMSG(OP::GetOpCodeClass(OpCode) == OP::OpCodeClass::Tertiary);
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      ElementType, Inst.m_Operands[DstIdx].m_MinPrecision);
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  if (!m_pOP->IsOverloadLegal(OpCode, pOperationType)) {
    if (pOperationType == Type::getInt16Ty(m_Ctx)) {
      pOperationType = Type::getInt32Ty(m_Ctx);
      OperationType = ElementType;
    }
  }
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In1, In2, In3, Out;
  LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
  LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);
  LoadOperand(In3, Inst, SrcIdx3, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Out[c] = m_pBuilder->CreateCall(
        pFunc, {m_pOP->GetU32Const((unsigned)OpCode), In1[c], In2[c], In3[c]});

    if (ElementType.GetKind() == CompType::Kind::F64) {
      c++;
    }
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, OperationType);
}

void DxbcConverter::ConvertQuaternary(
    OP::OpCode OpCode, const CompType &ElementType,
    D3D10ShaderBinary::CInstruction &Inst, const unsigned DstIdx,
    const unsigned SrcIdx1, const unsigned SrcIdx2, const unsigned SrcIdx3,
    const unsigned SrcIdx4) {
  DXASSERT_NOMSG(OP::GetOpCodeClass(OpCode) == OP::OpCodeClass::Quaternary);
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      ElementType, Inst.m_Operands[DstIdx].m_MinPrecision);
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In1, In2, In3, In4, Out;
  LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
  LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);
  LoadOperand(In3, Inst, SrcIdx3, WriteMask, OperationType);
  LoadOperand(In4, Inst, SrcIdx4, WriteMask, OperationType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    Out[c] =
        m_pBuilder->CreateCall(pFunc, {m_pOP->GetU32Const((unsigned)OpCode),
                                       In1[c], In2[c], In3[c], In4[c]});
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, OperationType);
}

void DxbcConverter::ConvertComparison(CmpInst::Predicate Predicate,
                                      const CompType &ElementType,
                                      D3D10ShaderBinary::CInstruction &Inst,
                                      const unsigned DstIdx,
                                      const unsigned SrcIdx1,
                                      const unsigned SrcIdx2) {
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      ElementType, GetHigherPrecision(Inst.m_Operands[SrcIdx1].m_MinPrecision,
                                      Inst.m_Operands[SrcIdx2].m_MinPrecision));

  if (ElementType.GetKind() != CompType::Kind::F64) {
    OperandValue In1, In2, Out;
    LoadOperand(In1, Inst, SrcIdx1, WriteMask, OperationType);
    LoadOperand(In2, Inst, SrcIdx2, WriteMask, OperationType);

    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (!WriteMask.IsSet(c))
        continue;

      switch (Predicate) {
      case CmpInst::FCMP_OEQ:
      case CmpInst::FCMP_UNE:
      case CmpInst::FCMP_OLT:
      case CmpInst::FCMP_OGE:
        Out[c] = m_pBuilder->CreateFCmp(Predicate, In1[c], In2[c]);
        break;

      case CmpInst::ICMP_EQ:
      case CmpInst::ICMP_NE:
      case CmpInst::ICMP_SLT:
      case CmpInst::ICMP_SGE:
      case CmpInst::ICMP_ULT:
      case CmpInst::ICMP_UGE:
        Out[c] = m_pBuilder->CreateICmp(Predicate, In1[c], In2[c]);
        break;

      default:
        DXASSERT_NOMSG(false);
      }
    }

    StoreOperand(Out, Inst, DstIdx, WriteMask, CompType::getI1());
  } else {
    // Double-precision comparison.
    CMask Mask = CMask::GetMaskForDoubleOperation(WriteMask);

    OperandValue In1, In2, Out;
    LoadOperand(In1, Inst, SrcIdx1, Mask, OperationType);
    LoadOperand(In2, Inst, SrcIdx2, Mask, OperationType);

    BYTE OperationComp = 0;
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (!WriteMask.IsSet(c))
        continue;

      switch (Predicate) {
      case CmpInst::FCMP_OEQ:
      case CmpInst::FCMP_UNE:
      case CmpInst::FCMP_OLT:
      case CmpInst::FCMP_OGE:
        Out[c] = m_pBuilder->CreateFCmp(Predicate, In1[OperationComp],
                                        In2[OperationComp]);
        break;

      default:
        DXASSERT_NOMSG(false);
      }

      OperationComp += 2;
    }

    StoreOperand(Out, Inst, DstIdx, WriteMask, CompType::getI1());
  }
}

void DxbcConverter::ConvertDotProduct(OP::OpCode OpCode, const BYTE NumComps,
                                      const CMask &LoadMask,
                                      D3D10ShaderBinary::CInstruction &Inst) {
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[0].m_WriteMask);
  CompType OperationType = DXBC::GetCompTypeWithMinPrec(
      CompType::getF32(), Inst.m_Operands[0].m_MinPrecision);
  Type *pOperationType = OperationType.GetLLVMType(m_Ctx);
  Function *pFunc = m_pOP->GetOpFunc(OpCode, pOperationType);

  OperandValue In1, In2, Out;
  LoadOperand(In1, Inst, 1, LoadMask, OperationType);
  LoadOperand(In2, Inst, 2, LoadMask, OperationType);

  vector<Value *> Args;
  Args.resize(1 + NumComps * 2);
  Args[0] = m_pOP->GetU32Const((unsigned)OpCode);
  for (BYTE c = 0; c < NumComps; c++) {
    Args[1 + c] = In1[c];
    Args[1 + NumComps + c] = In2[c];
  }
  Value *pValue = m_pBuilder->CreateCall(pFunc, Args);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;
    Out[c] = pValue;
  }

  StoreOperand(Out, Inst, 0, WriteMask, OperationType);
}

static Value *SafeConvertCast(IRBuilder<> &Builder, Value *pSrc, Type *pDstType,
                              CompType::Kind SrcKind, CompType::Kind DstKind) {
  // Prevent undef or nullptr values from getting through
  Value *pResult = nullptr;

  switch (SrcKind) {
  case CompType::Kind::F32:
    switch (DstKind) {
    case CompType::Kind::I32:
      pResult = Builder.CreateFPToSI(pSrc, pDstType);
      break;
    case CompType::Kind::U32:
      pResult = Builder.CreateFPToUI(pSrc, pDstType);
      break;
    case CompType::Kind::F16:
      pResult = Builder.CreateFPTrunc(pSrc, pDstType);
      break;
    case CompType::Kind::F64:
      pResult = Builder.CreateFPExt(pSrc, pDstType);
      break;
    }
    break;

  case CompType::Kind::I32:
    switch (DstKind) {
    case CompType::Kind::F32:
    case CompType::Kind::F64:
      pResult = Builder.CreateSIToFP(pSrc, pDstType);
      break;
    }
    break;

  case CompType::Kind::U32:
    switch (DstKind) {
    case CompType::Kind::F32:
    case CompType::Kind::F64:
      pResult = Builder.CreateUIToFP(pSrc, pDstType);
      break;
    }
    break;

  case CompType::Kind::F16:
    switch (DstKind) {
    case CompType::Kind::F32:
    case CompType::Kind::F64:
      pResult = Builder.CreateFPExt(pSrc, pDstType);
      break;
    }
    break;
  }

  // Note: Conversion from F64 uses ConvertFromDouble instead.

  DXASSERT(pResult != nullptr,
           "otherwise the caller passed incorrect type combination");

  // nullptr result indicates an error, but undef result may also occur with
  // out-of-range constants Rescue null or undef result by converting to
  // max/min(u)int/+-infinity, or 0xfefefefe/+-nan, to prevent invalid IR.
  if (!pResult || isa<UndefValue>(pResult)) {
    bool bSrcNegative = false;
    bool bInvalid = !pResult;
    // Get src sign:
    if (ConstantFP *pConstFP = dyn_cast<ConstantFP>(pSrc)) {
      bSrcNegative = pConstFP->getValueAPF().isNegative();
    } else if (ConstantInt *pConstInt = dyn_cast<ConstantInt>(pSrc)) {
      bSrcNegative = pConstInt->getValue().isNegative();
    } else {
      DXASSERT(false, "unhandled case for SafeConvertCast failure.");
      bInvalid = true;
    }

    if (pDstType->isIntegerTy()) {
      DXASSERT(pDstType->getScalarSizeInBits() == 32,
               "otherwise, int dest type is not expected size");
      APInt API(32, 0xFEFEFEFE);
      if (!bInvalid) {
        switch (DstKind) {
        case CompType::Kind::I32:
          API = bSrcNegative ? APInt::getSignedMinValue(32)
                             : APInt::getSignedMaxValue(32);
          break;
        case CompType::Kind::U32:
          API = bSrcNegative ? APInt::getNullValue(32) : APInt::getMaxValue(32);
          break;
        }
      }
      pResult = ConstantInt::get(pDstType->getContext(), API);
    } else {
      if (bInvalid) {
        pResult = ConstantFP::getNaN(pDstType, bSrcNegative);
      } else {
        pResult = ConstantFP::getInfinity(pDstType, bSrcNegative);
      }
    }
  }

  return pResult;
}

void DxbcConverter::ConvertCast(const CompType &SrcElementType,
                                const CompType &DstElementType,
                                D3D10ShaderBinary::CInstruction &Inst,
                                const unsigned DstIdx, const unsigned SrcIdx) {
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  Type *pDstType = DstElementType.GetLLVMType(m_Ctx);

  OperandValue In, Out;
  LoadOperand(In, Inst, SrcIdx, WriteMask, SrcElementType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;
    Out[c] =
        SafeConvertCast(*m_pBuilder, In[c], pDstType, SrcElementType.GetKind(),
                        DstElementType.GetKind());
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, DstElementType);
}

void DxbcConverter::ConvertToDouble(const CompType &SrcElementType,
                                    D3D10ShaderBinary::CInstruction &Inst) {
  const unsigned DstIdx = 0;
  const unsigned SrcIdx = 1;
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType DstElementType = CompType::getF64();
  Type *pDstType = DstElementType.GetLLVMType(m_Ctx);
  CMask Mask;
  BYTE OutputComp;
  switch (WriteMask.ToByte()) {
  case 0x0:
    return;
  case 0x3:
    Mask = CMask(1, 0, 0, 0);
    OutputComp = 0;
    break;
  case 0xC:
    Mask = CMask(1, 0, 0, 0);
    OutputComp = 2;
    break;
  case 0xF:
    Mask = CMask(1, 1, 0, 0);
    OutputComp = 0;
    break;
  default:
    DXASSERT_DXBC(false);
  }

  OperandValue In, Out;
  LoadOperand(In, Inst, SrcIdx, Mask, SrcElementType);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!Mask.IsSet(c))
      continue;
    Out[OutputComp] =
        SafeConvertCast(*m_pBuilder, In[c], pDstType, SrcElementType.GetKind(),
                        DstElementType.GetKind());
    OutputComp += 2;
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, DstElementType);
}

void DxbcConverter::ConvertFromDouble(const CompType &DstElementType,
                                      D3D10ShaderBinary::CInstruction &Inst) {
  const unsigned DstIdx = 0;
  const unsigned SrcIdx = 1;
  CMask WriteMask = CMask::FromDXBC(Inst.m_Operands[DstIdx].m_WriteMask);
  CompType SrcElementType = CompType::getF64();
  CMask Mask = CMask::GetMaskForDoubleOperation(WriteMask);

  OperandValue In, Out;
  LoadOperand(In, Inst, SrcIdx, Mask, SrcElementType);

  BYTE OperationComp = 0;
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!WriteMask.IsSet(c))
      continue;

    OP::OpCode OpCode = OP::OpCode(0);
    switch (DstElementType.GetKind()) {
    case CompType::Kind::I32:
      OpCode = OP::OpCode::LegacyDoubleToSInt32;
      break;
    case CompType::Kind::U32:
      OpCode = OP::OpCode::LegacyDoubleToUInt32;
      break;
    case CompType::Kind::F32:
      OpCode = OP::OpCode::LegacyDoubleToFloat;
      break;
    default:
      DXASSERT_NOMSG(false);
    }

    // Create call.
    Function *F = F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
    Value *Args[2];
    Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
    Args[1] = In[OperationComp];                    // Double value

    Out[c] = MarkPrecise(m_pBuilder->CreateCall(F, Args));

    OperationComp += 2;
  }

  StoreOperand(Out, Inst, DstIdx, WriteMask, DstElementType);
}

void DxbcConverter::LoadCommonSampleInputs(
    D3D10ShaderBinary::CInstruction &Inst, Value *pArgs[], bool bSetOffsets) {
  bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
  const unsigned uOpStatus = 1;
  const unsigned uOpCoord = uOpStatus + (bHasFeedback ? 1 : 0);
  const unsigned uOpSRV = DXBC::GetResourceSlot(Inst.OpCode());
  const unsigned uOpSampler = uOpSRV + 1;
  DXASSERT_DXBC(Inst.m_Operands[uOpSRV].m_Type ==
                D3D10_SB_OPERAND_TYPE_RESOURCE);
  DXASSERT_DXBC(Inst.m_Operands[uOpSampler].m_Type ==
                D3D10_SB_OPERAND_TYPE_SAMPLER);

  OperandValue InSRV, InSampler, InCoord;
  // Resource.
  const DxilResource &R = LoadSRVOperand(
      InSRV, Inst, uOpSRV, CMask::MakeXMask(), CompType::getInvalid());
  // Coordinates.
  CMask CoordMask =
      CMask::MakeFirstNCompMask(DXBC::GetNumResCoords(R.GetKind()));
  LoadOperand(InCoord, Inst, uOpCoord, CoordMask, CompType::getF32());
  // Sampler.
  LoadOperand(InSampler, Inst, uOpSampler, CMask::MakeXMask(),
              CompType::getInvalid());

  // Create Sample call's common arguments.
  pArgs[1] = InSRV[0];                                       // SRV handle
  pArgs[2] = InSampler[0];                                   // Sampler handle
  pArgs[3] = CoordMask.IsSet(0) ? InCoord[0] : m_pUnusedF32; // Coordinate 0
  pArgs[4] = CoordMask.IsSet(1) ? InCoord[1] : m_pUnusedF32; // Coordinate 1
  pArgs[5] = CoordMask.IsSet(2) ? InCoord[2] : m_pUnusedF32; // Coordinate 2
  pArgs[6] = CoordMask.IsSet(3) ? InCoord[3] : m_pUnusedF32; // Coordinate 3

  // Offsets.
  if (bSetOffsets) {
    CMask ResOffsetMask =
        CMask::MakeFirstNCompMask(DXBC::GetNumResOffsets(R.GetKind()));
    pArgs[7] = ResOffsetMask.IsSet(0)
                   ? m_pOP->GetU32Const(Inst.m_TexelOffset[0])
                   : m_pUnusedI32; // Offset 0
    pArgs[8] = ResOffsetMask.IsSet(1)
                   ? m_pOP->GetU32Const(Inst.m_TexelOffset[1])
                   : m_pUnusedI32; // Offset 1
    pArgs[9] = ResOffsetMask.IsSet(2)
                   ? m_pOP->GetU32Const(Inst.m_TexelOffset[2])
                   : m_pUnusedI32; // Offset 2
  }
}

void DxbcConverter::StoreResRetOutputAndStatus(
    D3D10ShaderBinary::CInstruction &Inst, Value *pResRet, CompType DstType) {
  bool bHasFeedback = DXBC::HasFeedback(Inst.OpCode());
  const unsigned uOpOutput = 0;
  const unsigned uOpStatus = 1;
  const unsigned uOpRes = GetResourceSlot(Inst);

  MarkPrecise(pResRet);

  // Store output.
  CMask OutputMask = CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
  if (!OutputMask.IsZero()) {
    OperandValue Out;
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (!OutputMask.IsSet(c))
        continue;

      // Respect swizzle: resource swizzle == return value swizzle.
      BYTE Comp = Inst.m_Operands[uOpRes].m_Swizzle[c];

      Out[c] = m_pBuilder->CreateExtractValue(pResRet, Comp);
    }
    StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);
  }

  // Store status.
  if (bHasFeedback) {
    CMask StatusMask = CMask::FromDXBC(Inst.m_Operands[uOpStatus].m_WriteMask);
    if (!StatusMask.IsZero()) {
      OperandValue Status;
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!StatusMask.IsSet(c))
          continue;

        const unsigned uStatusField = 4;
        Status[c] = m_pBuilder->CreateExtractValue(pResRet, uStatusField);
      }
      StoreOperand(Status, Inst, uOpStatus, StatusMask, CompType::getU32());
    }
  }
}

void DxbcConverter::StoreGetDimensionsOutput(
    D3D10ShaderBinary::CInstruction &Inst, Value *pGetDimRet) {
  const unsigned uOpOutput = 0;
  const unsigned uOpRes = DXBC::GetResourceSlot(Inst.OpCode());

  CMask OutputMask = CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
  if (OutputMask.IsZero())
    return;

  // Resource.
  const DxilResource *R;
  if (Inst.m_Operands[uOpRes].m_Type == D3D10_SB_OPERAND_TYPE_RESOURCE) {
    R = &GetSRVFromOperand(Inst, uOpRes);
  } else {
    unsigned RangeID = Inst.m_Operands[uOpRes].m_Index[0].m_RegIndex;
    R = &m_pPR->GetUAV(m_UAVRangeMap[RangeID]);
  }

  // Value type.
  CompType ValueType = CompType::getI32();
  bool bRcp = false;
  switch (Inst.m_ResInfoReturnType) {
  case D3D10_SB_RESINFO_INSTRUCTION_RETURN_FLOAT:
    ValueType = CompType::getF32();
    break;
  case D3D10_SB_RESINFO_INSTRUCTION_RETURN_RCPFLOAT:
    ValueType = CompType::getF32();
    bRcp = true;
    break;
  case D3D10_SB_RESINFO_INSTRUCTION_RETURN_UINT:
    ValueType = CompType::getI32();
    break;
  default:
    DXASSERT_DXBC(false);
  }

  OperandValue Out;
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!OutputMask.IsSet(c))
      continue;

    // Respect swizzle: resource swizzle == return value swizzle.
    BYTE Comp = Inst.m_Operands[uOpRes].m_Swizzle[c];

    Value *pCompVal = m_pBuilder->CreateExtractValue(pGetDimRet, Comp);
    if (ValueType.IsFloatTy()) {
      pCompVal = m_pBuilder->CreateCast(Instruction::CastOps::UIToFP, pCompVal,
                                        Type::getFloatTy(m_Ctx));
    }
    if (bRcp) {
      if (Comp < DxilResource::GetNumDimensions(R->GetKind())) {
        pCompVal = m_pBuilder->CreateBinOp(
            Instruction::BinaryOps::FDiv, m_pOP->GetFloatConst(1.0f), pCompVal);
      }
    }

    Out[c] = pCompVal;
  }

  StoreOperand(Out, Inst, uOpOutput, OutputMask, ValueType);
}

void DxbcConverter::StoreSamplePosOutput(D3D10ShaderBinary::CInstruction &Inst,
                                         Value *pSamplePosVal) {
  const unsigned uOpOutput = 0;
  const unsigned uOpRes = DXBC::GetResourceSlot(Inst.OpCode());
  CompType DstType = DXBC::GetCompTypeWithMinPrec(
      CompType::getF32(), Inst.m_Operands[uOpOutput].m_MinPrecision);

  // Store output.
  CMask OutputMask = CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
  if (!OutputMask.IsZero()) {
    OperandValue Out;
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (!OutputMask.IsSet(c))
        continue;

      BYTE Comp = Inst.m_Operands[uOpRes].m_Swizzle[c];
      if (Comp < 2) {
        Out[c] = m_pBuilder->CreateExtractValue(pSamplePosVal, Comp);
      } else {
        Out[c] = m_pOP->GetFloatConst(0);
      }
    }
    StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);
  }
}

void DxbcConverter::StoreBroadcastOutput(D3D10ShaderBinary::CInstruction &Inst,
                                         Value *pValue, CompType DstType) {
  const unsigned uOpOutput = 0;
  CMask OutputMask = CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
  if (!OutputMask.IsZero()) {
    OperandValue Out;
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (!OutputMask.IsSet(c))
        continue;

      Out[c] = pValue;
    }
    StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);
  }
}

Value *DxbcConverter::GetCoordValue(D3D10ShaderBinary::CInstruction &Inst,
                                    const unsigned uCoordIdx) {
  BYTE CoordComp = Inst.m_Operands[uCoordIdx].m_ComponentName;
  OperandValue InCoord;
  CMask CoordMask = CMask::MakeCompMask(CoordComp);
  LoadOperand(InCoord, Inst, uCoordIdx, CoordMask, CompType::getI32());
  return InCoord[CoordComp];
}

Value *DxbcConverter::GetByteOffset(D3D10ShaderBinary::CInstruction &Inst,
                                    const unsigned Idx1, const unsigned Idx2,
                                    const unsigned Stride) {
  const unsigned uOpElementOffset = Idx1;
  const unsigned uOpStructByteOffset = Idx2;
  OperandValue InElementOffset, InStructByteOffset;

  // Element offset.
  BYTE ElementOffsetComp = Inst.m_Operands[uOpElementOffset].m_ComponentName;
  CMask CoordMask = CMask::MakeCompMask(ElementOffsetComp);
  LoadOperand(InElementOffset, Inst, uOpElementOffset, CoordMask,
              CompType::getI32());

  // Byte offset into the structure.
  BYTE StructByteOffsetComp =
      Inst.m_Operands[uOpStructByteOffset].m_ComponentName;
  CMask StructByteOffsetMask = CMask::MakeCompMask(StructByteOffsetComp);
  LoadOperand(InStructByteOffset, Inst, uOpStructByteOffset,
              StructByteOffsetMask, CompType::getI32());

  // Calculate byte offset.
  Value *pOffset1 = InElementOffset[ElementOffsetComp];
  Value *pOffset2 = InStructByteOffset[StructByteOffsetComp];
  Value *pMul = pOffset1;
  if (Stride > 1) {
    Value *pStride = m_pOP->GetU32Const(Stride);
    pMul = m_pBuilder->CreateMul(pOffset1, pStride);
  }
  Value *pByteOffset = m_pBuilder->CreateAdd(pMul, pOffset2);

  return pByteOffset;
}

void DxbcConverter::ConvertLoadTGSM(D3D10ShaderBinary::CInstruction &Inst,
                                    const unsigned uOpTGSM,
                                    const unsigned uOpOutput, CompType SrcType,
                                    Value *pByteOffset) {
  DXASSERT_DXBC(Inst.m_Operands[uOpTGSM].m_Type ==
                D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY);
  const TGSMEntry &R =
      m_TGSMMap[Inst.m_Operands[uOpTGSM].m_Index[0].m_RegIndex];

  CMask OutputMask = CMask::FromDXBC(Inst.m_Operands[uOpOutput].m_WriteMask);
  if (OutputMask.IsZero())
    return;

  OperandValue Out;
  CompType DstType = DXBC::GetCompTypeFromMinPrec(
      Inst.m_Operands[uOpOutput].m_MinPrecision, CompType::getF32());
  Type *pSrcType = SrcType.GetLLVMPtrType(m_Ctx, DXIL::kTGSMAddrSpace);

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!OutputMask.IsSet(c))
      continue;

    // Swizzle.
    BYTE Comp = Inst.m_Operands[uOpTGSM].m_Swizzle[c];

    // Adjust index for component.
    Value *pValueIndex = pByteOffset;
    if (Comp > 0) {
      pValueIndex = m_pBuilder->CreateAdd(
          pByteOffset, m_pOP->GetU32Const(Comp * kRegCompAlignment));
    }

    // Create GEP.
    Value *pGEPIndices[2] = {m_pOP->GetU32Const(0), pValueIndex};
    Value *pPtrI8 = m_pBuilder->CreateGEP(R.pVar, pGEPIndices);

    // Create load.
    Value *pPtr = m_pBuilder->CreatePointerCast(pPtrI8, pSrcType);
    LoadInst *pLoad = m_pBuilder->CreateLoad(pPtr);
    pLoad->setAlignment(kRegCompAlignment);

    Out[c] = CastDxbcValue(pLoad, SrcType, DstType);
  }

  StoreOperand(Out, Inst, uOpOutput, OutputMask, DstType);
}

void DxbcConverter::ConvertStoreTGSM(D3D10ShaderBinary::CInstruction &Inst,
                                     const unsigned uOpTGSM,
                                     const unsigned uOpValue,
                                     CompType BaseValueType,
                                     Value *pByteOffset) {
  DXASSERT_DXBC(Inst.m_Operands[uOpTGSM].m_Type ==
                D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY);
  const TGSMEntry &R =
      m_TGSMMap[Inst.m_Operands[uOpTGSM].m_Index[0].m_RegIndex];

  // Value type.
  CompType ValueType = DXBC::GetCompTypeFromMinPrec(
      Inst.m_Operands[uOpValue].m_MinPrecision, BaseValueType);

  // Store TGSM value.
  CMask OutputMask = CMask::FromDXBC(Inst.m_Operands[uOpTGSM].m_WriteMask);
  if (OutputMask.IsZero())
    return;

  // Value.
  OperandValue InValue;
  LoadOperand(InValue, Inst, uOpValue, OutputMask, ValueType);

  CompType DstType = BaseValueType;
  Type *pDstType = DstType.GetLLVMPtrType(m_Ctx, DXIL::kTGSMAddrSpace);
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!OutputMask.IsSet(c))
      continue;

    // Adjust index for component.
    Value *pValueIndex = pByteOffset;
    if (c > 0) {
      pValueIndex = m_pBuilder->CreateAdd(
          pByteOffset, m_pOP->GetU32Const(c * kRegCompAlignment));
    }

    // Cast value to the right type.
    Value *pValue = CastDxbcValue(InValue[c], ValueType, DstType);

    // Create GEP.
    Value *pGEPIndices[2] = {m_pOP->GetU32Const(0), pValueIndex};
    Value *pPtrI8 = m_pBuilder->CreateGEP(R.pVar, pGEPIndices);

    // Create store.
    Value *pPtr = m_pBuilder->CreatePointerCast(pPtrI8, pDstType);
    StoreInst *pStore = m_pBuilder->CreateStore(pValue, pPtr);
    pStore->setAlignment(kRegCompAlignment);
    (void)MarkPrecise(pStore, c);
  }
}

void DxbcConverter::EmitGSOutputRegisterStore(unsigned StreamId) {
  const auto &Sig = m_pOutputSignature->m_Signature.GetElements();

  // For each output decl for stream StreamID.
  for (size_t i = 0; i < Sig.size(); i++) {
    DxilSignatureElement &SE = m_pOutputSignature->m_Signature.GetElement(i);

    if (SE.GetOutputStream() != StreamId)
      continue;

    DXASSERT(SE.GetRows() == 1,
             "to support indexable output in GS with multiple output streams");
    unsigned TempReg = GetGSTempRegForOutputReg(SE.GetStartRow());

    CompType DxbcValueType = SE.GetCompType();
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

    for (BYTE c = 0; c < SE.GetCols(); c++) {
      BYTE Comp = SE.GetStartCol() + c;

      Value *pValue;
      // 1. Load value from the corresponding temp reg.
      {
        Value *Args[2];
        Args[0] =
            m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegLoad); // OpCode
        Args[1] = m_pOP->GetU32Const(
            DXBC::GetRegIndex(TempReg, Comp)); // Linearized register index
        Function *F = m_pOP->GetOpFunc(OP::OpCode::TempRegLoad, pDxbcValueType);
        pValue = m_pBuilder->CreateCall(F, Args);
      }
      // 2. Store the value to the output reg.
      {
        Value *Args[5];
        Args[0] =
            m_pOP->GetU32Const((unsigned)OP::OpCode::StoreOutput); // OpCode
        Args[1] = m_pOP->GetU32Const(SE.GetID()); // Output signature element ID
        Args[2] = m_pOP->GetU32Const(0); // Row, relative to the element
        Args[3] = m_pOP->GetU8Const(c);  // Col, relative to the element
        Args[4] = pValue;                // Value
        Function *F = m_pOP->GetOpFunc(OP::OpCode::StoreOutput, pDxbcValueType);
        m_pBuilder->CreateCall(F, Args);
      }
    }
  }
}

Value *DxbcConverter::CreateHandle(DxilResourceBase::Class Class,
                                   unsigned RangeID, Value *pIndex,
                                   bool bNonUniformIndex) {
  DXASSERT(pIndex->getType() == Type::getInt32Ty(m_Ctx),
           "index should be i32 type");
  OP::OpCode OpCode = OP::OpCode::CreateHandle;
  Value *Args[5];
  Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
  Args[1] = m_pOP->GetU8Const(
      (BYTE)Class); // Resource class (SRV, UAV, CBuffer, Sampler)
  Args[2] = m_pOP->GetU32Const(RangeID);         // Range ID
  Args[3] = pIndex;                              // 0-based index into the range
  Args[4] = m_pOP->GetI1Const(bNonUniformIndex); // Non-uniform resource index
  Function *pCreateHandleFunc =
      m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
  return m_pBuilder->CreateCall(pCreateHandleFunc, Args);
}
void DxbcConverter::SetCachedHandle(const DxilResourceBase &R) {
  DXASSERT(!IsSM51Plus(), "must not cache handles on SM 5.1");
  if (R.GetSpaceID() == 0) {
    // Note: Even though space should normally be 0 for SM 5.0 and below,
    // the interfaces implementation uses non-zero space when converting from
    // SM 5.0.
    m_HandleMap[std::make_pair((unsigned)R.GetClass(),
                               (unsigned)R.GetLowerBound())] =
        CreateHandle(R.GetClass(), R.GetID(),
                     m_pOP->GetU32Const(R.GetLowerBound()), false);
  }
}
Value *DxbcConverter::GetCachedHandle(const DxilResourceBase &R) {
  if (IsSM51Plus() || R.GetSpaceID() != 0)
    return nullptr;
  auto it = m_HandleMap.find(
      std::make_pair((unsigned)R.GetClass(), (unsigned)R.GetLowerBound()));
  if (it != m_HandleMap.end())
    return it->second;
  return nullptr;
}

Value *DxbcConverter::LoadConstFloat(float &fVal) {
  unsigned uVal = *(unsigned *)&fVal;
  APFloat V(fVal);
  float fVal2 = V.convertToFloat();

  if ((*(unsigned *)&fVal2) == uVal) {
    return m_pOP->GetFloatConst(fVal);
  } else {
    OP::OpCode OpCode = OP::OpCode::BitcastI32toF32;
    Value *Args[2];
    Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
    Args[1] = m_pOP->GetU32Const(uVal);             // Input
    Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));
    return m_pBuilder->CreateCall(F, Args);
  }
}

void DxbcConverter::SetHasCounter(D3D10ShaderBinary::CInstruction &Inst,
                                  const unsigned uOpUAV) {
  D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[uOpUAV];
  DXASSERT_DXBC(O.m_Type == D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW);

  // Retrieve UAV range ID and record.
  DXASSERT_DXBC(O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
  unsigned RangeID = O.m_Index[0].m_RegIndex;
  unsigned RecIdx = m_UAVRangeMap[RangeID];
  DxilResource &R = m_pPR->GetUAV(RecIdx);
  R.SetHasCounter(true);
}

void DxbcConverter::LoadOperand(OperandValue &SrcVal,
                                D3D10ShaderBinary::CInstruction &Inst,
                                const unsigned OpIdx, const CMask &Mask,
                                const CompType &ValueType) {
  D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[OpIdx];

  switch (O.m_Type) {
  case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
    DXASSERT_DXBC(O.m_Modifier == D3D10_SB_OPERAND_MODIFIER_NONE);
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (!Mask.IsSet(c))
        continue;

      bool bVec4 = O.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT;
      BYTE Comp = bVec4 ? c : 0;

      switch (ValueType.GetKind()) {
      case CompType::Kind::F32:
        SrcVal[c] = LoadConstFloat(O.m_Valuef[Comp]);
        break;

      case CompType::Kind::F16:
        SrcVal[c] = CastDxbcValue(LoadConstFloat(O.m_Valuef[Comp]),
                                  CompType::Kind::F32, CompType::Kind::F16);
        break;

      case CompType::Kind::I32:
        LLVM_FALLTHROUGH;
      case CompType::Kind::U32:
        SrcVal[c] = m_pOP->GetU32Const(O.m_Value[Comp]);
        break;

      case CompType::Kind::I16:
        LLVM_FALLTHROUGH;
      case CompType::Kind::U16:
        SrcVal[c] = CastDxbcValue(m_pOP->GetU32Const(O.m_Value[Comp]),
                                  CompType::Kind::U32, CompType::Kind::I16);
        break;

      case CompType::Kind::I1:
        SrcVal[c] = CastDxbcValue(m_pOP->GetU32Const(O.m_Value[Comp]),
                                  CompType::Kind::U32, CompType::Kind::I1);
        break;

      default:
        DXASSERT_DXBC(false);
      }
    }
    break;

  case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:
    DXASSERT_NOMSG(ValueType.GetKind() == CompType::Kind::F64);
    for (BYTE c = 0; c < DXBC::kWidth; c += 2) {
      if (!Mask.IsSet(c))
        continue;

      SrcVal[c] = m_pOP->GetDoubleConst(O.m_Valued[c]);
    }
    break;

  case D3D10_SB_OPERAND_TYPE_TEMP: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    unsigned Reg = O.m_Index[0].m_RegIndex;
    CompType DxbcValueType =
        DXBC::GetCompTypeFromMinPrec(O.m_MinPrecision, ValueType);
    if (DxbcValueType.IsBoolTy()) {
      DxbcValueType = CompType::getI32();
    }
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

    if (DxbcValueType.GetKind() != CompType::Kind::F64) {
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();

        Value *Args[2];
        Args[0] =
            m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegLoad); // OpCode
        Args[1] = m_pOP->GetU32Const(
            DXBC::GetRegIndex(Reg, Comp)); // Linearized register index
        Function *F = m_pOP->GetOpFunc(OP::OpCode::TempRegLoad, pDxbcValueType);
        Value *pValue = m_pBuilder->CreateCall(F, Args);

        pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
      }
    } else {
      DXASSERT_DXBC(CMask::IsValidDoubleMask(Mask));
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();

        Value *pValue1, *pValue2;
        {
          Value *Args[2];
          Args[0] =
              m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegLoad); // OpCode
          Args[1] = m_pOP->GetU32Const(
              DXBC::GetRegIndex(Reg, Comp)); // Linearized register index1
          Function *F = m_pOP->GetOpFunc(OP::OpCode::TempRegLoad,
                                         CompType::getU32().GetLLVMType(m_Ctx));
          pValue1 = m_pBuilder->CreateCall(F, Args);
          Args[1] = m_pOP->GetU32Const(
              DXBC::GetRegIndex(Reg, Comp + 1)); // Linearized register index2
          pValue2 = m_pBuilder->CreateCall(F, Args);
        }

        Value *pValue;
        {
          Value *Args[3];
          Function *F =
              m_pOP->GetOpFunc(OP::OpCode::MakeDouble, pDxbcValueType);
          Args[0] =
              m_pOP->GetU32Const((unsigned)OP::OpCode::MakeDouble); // OpCode
          Args[1] = pValue1;                                        // Lo part
          Args[2] = pValue2;                                        // Hi part
          pValue = m_pBuilder->CreateCall(F, Args);
          pValue = ApplyOperandModifiers(pValue, O);
        }

        OVH.SetValue(pValue);
        OVH.Advance();
      }
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D);
    DXASSERT_DXBC(O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    unsigned Reg = O.m_Index[0].m_RegIndex;
    IndexableReg &IRRec = m_IndexableRegs[Reg];
    Value *pXRegIndex = LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]);
    Value *pRegIndex =
        m_pBuilder->CreateMul(pXRegIndex, m_pOP->GetI32Const(IRRec.NumComps));
    CompType DxbcValueType =
        DXBC::GetCompTypeFromMinPrec(O.m_MinPrecision, ValueType);
    if (DxbcValueType.IsBoolTy()) {
      DxbcValueType = CompType::getI32();
    }

    if (DxbcValueType.GetKind() != CompType::Kind::F64) {
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();
        Value *pValue = nullptr;

        // Create GEP.
        Value *pIndex =
            m_pBuilder->CreateAdd(pRegIndex, m_pOP->GetU32Const(Comp));
        Value *pGEPIndices[2] = {m_pOP->GetU32Const(0), pIndex};

        if (!DxbcValueType.HasMinPrec()) {
          Value *pBasePtr = m_IndexableRegs[Reg].pValue32;
          Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
          pValue = m_pBuilder->CreateAlignedLoad(pPtr, kRegCompAlignment);
          pValue = CastDxbcValue(pValue, CompType::getF32(), ValueType);
        } else {
          // Create GEP.
          Value *pBasePtr = m_IndexableRegs[Reg].pValue16;
          Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
          pValue = m_pBuilder->CreateAlignedLoad(pPtr, kRegCompAlignment / 2);
          pValue = CastDxbcValue(pValue, CompType::getF16(), ValueType);
        }

        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
      }
    } else {
      // Double precision.
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();
        Value *pValue = nullptr;

        // Create GEP.
        Value *pIndex =
            m_pBuilder->CreateAdd(pRegIndex, m_pOP->GetU32Const(Comp));
        Value *pGEPIndices[1] = {pIndex};
        Value *pBasePtr = m_pBuilder->CreateBitCast(
            m_IndexableRegs[Reg].pValue32, Type::getDoublePtrTy(m_Ctx));
        Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
        pValue = m_pBuilder->CreateAlignedLoad(pPtr, kRegCompAlignment * 2);

        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
        OVH.Advance();
        OVH.SetValue(pValue);
      }
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_INPUT:
  case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT: {
    OP::OpCode OpCode = OP::OpCode::LoadInput;
    unsigned Register;      // Starting index of the register range.
    Value *pUnitIndexValue; // Vertex/point index expression.
    Value *pRowIndexValue;  // Row index expression.

    switch (O.m_IndexDimension) {
    case D3D10_SB_OPERAND_INDEX_1D:
      Register = O.m_Index[0].m_RegIndex;
      pUnitIndexValue = m_pUnusedI32;
      pRowIndexValue = LoadOperandIndex(O.m_Index[0], O.m_IndexType[0]);
      break;

    case D3D10_SB_OPERAND_INDEX_2D:
      // 2D input register index: <index1, input register index>.
      // index1: GS -- vertex index, DS -- input control point index.
      Register = O.m_Index[1].m_RegIndex;
      pUnitIndexValue = LoadOperandIndex(O.m_Index[0], O.m_IndexType[0]);
      pRowIndexValue = LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]);
      break;

    default:
      DXASSERT(false, "there should no other index dimensions");
    }

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE Comp = OVH.GetComp();
      // Retrieve signature element.
      const DxilSignatureElement *E =
          m_pInputSignature->GetElement(Register, Comp);
      CompType DxbcValueType = E->GetCompType();
      if (DxbcValueType.IsBoolTy()) {
        DxbcValueType = CompType::getI32();
      }
      Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

      MutableArrayRef<Value *> Args;
      Value *Args1[1];
      Value *Args5[5];

      if (E->GetKind() == DXIL::SemanticKind::SampleIndex) {
        // Use SampleIndex intrinsic instead of LoadInput
        Args = Args1;
        OpCode = OP::OpCode::SampleIndex;
      } else {
        Args = Args5;
        // Make row/col index relative within element.
        Value *pRowIndexValueRel = m_pBuilder->CreateSub(
            pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));
        Args[1] = m_pOP->GetU32Const(E->GetID()); // Input signature element ID
        Args[2] = pRowIndexValueRel; // Row, relative to the element
        Args[3] = m_pOP->GetU8Const(
            Comp - E->GetStartCol()); // Col, relative to the element
        Args[4] = pUnitIndexValue;    // Vertex/point index
      }

      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode

      Function *F = m_pOP->GetOpFunc(OpCode, pDxbcValueType);
      Value *pValue = m_pBuilder->CreateCall(F, Args);

      pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER: {
    // Upconvert operand to SM5.1.
    if (O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D) {
      O.m_IndexDimension = D3D10_SB_OPERAND_INDEX_3D;
      O.m_IndexType[2] = O.m_IndexType[1];
      O.m_Index[2] = O.m_Index[1];
      O.m_IndexType[1] = O.m_IndexType[0];
      O.m_Index[1] = O.m_Index[0];
    }

    // Retrieve cbuffer range ID and record.
    const DxilCBuffer *pR = m_pClassInstanceCBuffers;
    if (O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32) {
      unsigned RangeID = O.m_Index[0].m_RegIndex;
      unsigned RecIdx = m_CBufferRangeMap[RangeID];
      pR = &m_pPR->GetCBuffer(RecIdx);
    }

    const DxilCBuffer &R = *pR;

    // Setup cbuffer handle.
    Value *pHandle = GetCachedHandle(R);
    if (pHandle == nullptr) {
      // Create dynamic-index handle.
      pHandle = CreateHandle(R.GetClass(), R.GetID(),
                             LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]),
                             O.m_Nonuniform);
    }

    // Load values for unique components.
    Value *pRegIndexValue = LoadOperandIndex(O.m_Index[2], O.m_IndexType[2]);
    CompType DxbcValueType = ValueType.GetBaseCompType();
    if (DxbcValueType.IsBoolTy()) {
      DxbcValueType = CompType::getI32();
    }
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

    DXASSERT_NOMSG(m_bLegacyCBufferLoad);
    Value *Args[3];
    Args[0] =
        m_pOP->GetU32Const((unsigned)OP::OpCode::CBufferLoadLegacy); // OpCode
    Args[1] = pHandle;        // CBuffer handle
    Args[2] = pRegIndexValue; // 0-based index into cbuffer instance
    Function *pCBufferLoadFunc =
        m_pOP->GetOpFunc(OP::OpCode::CBufferLoadLegacy, pDxbcValueType);

    Value *pCBufferRetValue = m_pBuilder->CreateCall(pCBufferLoadFunc, Args);

    if (ValueType.GetKind() != CompType::Kind::F64) {
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();

        Value *pValue = m_pBuilder->CreateExtractValue(pCBufferRetValue, Comp);
        pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
      }
    } else {
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp() / 2;

        Value *pValue = m_pBuilder->CreateExtractValue(pCBufferRetValue, Comp);
        pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
        OVH.Advance();
        OVH.SetValue(pValue);
      }
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    Value *pRegIndex = LoadOperandIndex(O.m_Index[0], O.m_IndexType[0]);

    if (ValueType.GetKind() != CompType::Kind::F64) {
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();

        Value *pValueIndex =
            m_pBuilder->CreateMul(pRegIndex, m_pOP->GetI32Const(DXBC::kWidth));
        pValueIndex =
            m_pBuilder->CreateAdd(pValueIndex, m_pOP->GetI32Const(Comp));
        // Create GEP.
        Value *pGEPIndices[2] = {m_pOP->GetU32Const(0), pValueIndex};
        Value *pPtr = m_pBuilder->CreateGEP(m_pIcbGV, pGEPIndices);
        LoadInst *pLoad = m_pBuilder->CreateLoad(pPtr);
        pLoad->setAlignment(kRegCompAlignment);
        Value *pValue = CastDxbcValue(pLoad, CompType::getF32(), ValueType);
        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
      }
    } else {
      // Double precision ICB.
      for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
           OVH.Advance()) {
        BYTE Comp = OVH.GetComp();

        Value *pValueIndex =
            m_pBuilder->CreateMul(pRegIndex, m_pOP->GetI32Const(DXBC::kWidth));
        pValueIndex =
            m_pBuilder->CreateAdd(pValueIndex, m_pOP->GetI32Const(Comp));
        // Bitcast pointer.
        Value *pPtrBase =
            m_pBuilder->CreateBitCast(m_pIcbGV, Type::getDoublePtrTy(m_Ctx));
        // Create GEP.
        Value *pGEPIndices[1] = {pValueIndex};
        Value *pPtr = m_pBuilder->CreateGEP(pPtrBase, pGEPIndices);
        LoadInst *pLoad = m_pBuilder->CreateLoad(pPtr);
        pLoad->setAlignment(kRegCompAlignment * 2);
        Value *pValue = pLoad;
        pValue = ApplyOperandModifiers(pValue, O);

        OVH.SetValue(pValue);
        OVH.Advance();
        OVH.SetValue(pValue);
      }
    }
    break;
  }

  case D3D10_SB_OPERAND_TYPE_SAMPLER: {
    // Upconvert operand to SM5.1.
    if (O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D) {
      O.m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
      O.m_IndexType[1] = O.m_IndexType[0];
      O.m_Index[1] = O.m_Index[0];
    }

    // Retrieve sampler range ID and record.
    const DxilSampler *pR = nullptr;
    if (O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32) {
      unsigned RangeID = O.m_Index[0].m_RegIndex;
      unsigned RecIdx = m_SamplerRangeMap[RangeID];
      pR = &m_pPR->GetSampler(RecIdx);
    } else {
      switch (Inst.OpCode()) {
      case D3D10_SB_OPCODE_SAMPLE_C:
      case D3D10_SB_OPCODE_SAMPLE_C_LZ:
      case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK:
      case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK:
      case D3D11_SB_OPCODE_GATHER4_PO_C:
      case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK:
        pR = m_pClassInstanceComparisonSamplers;
        break;
      default:
        pR = m_pClassInstanceSamplers;
        break;
      }
    }
    const DxilSampler &R = *pR;

    // Setup sampler handle.
    Value *pHandle = GetCachedHandle(R);
    if (pHandle == nullptr) {
      // Create dynamic-index handle.
      pHandle = CreateHandle(R.GetClass(), R.GetID(),
                             LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]),
                             O.m_Nonuniform);
    }

    // Replicate handle values.
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (Mask.IsSet(c))
        SrcVal[c] = pHandle;
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_RESOURCE: {
    (void)LoadSRVOperand(SrcVal, Inst, OpIdx, Mask, ValueType);
    break;
  }

  case D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW: {
    // Upconvert operand to SM5.1.
    if (O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D) {
      DXASSERT_DXBC(O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      O.m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
      O.m_IndexType[1] = O.m_IndexType[0];
      O.m_Index[1] = O.m_Index[0];
    }

    // Retrieve UAV range ID and record.
    DXASSERT_DXBC(O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    unsigned RangeID = O.m_Index[0].m_RegIndex;
    unsigned RecIdx = m_UAVRangeMap[RangeID];
    const DxilResource &R = m_pPR->GetUAV(RecIdx);

    // Setup UAV handle.
    Value *pHandle = GetCachedHandle(R);
    if (pHandle == nullptr) {
      DXASSERT(IsSM51Plus(),
               "otherwise did not initialize handles on entry to main");
      // Create dynamic-index handle.
      pHandle = CreateHandle(R.GetClass(), R.GetID(),
                             LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]),
                             O.m_Nonuniform);
    }

    // Replicate handle values.
    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      if (Mask.IsSet(c))
        SrcVal[c] = pHandle;
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_RASTERIZER: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_0D);
    DXASSERT_DXBC(false); // "rasterizer" register is not used in DXIL.
    break;
  }

  case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID:
  case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:
  case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP: {
    OP::OpCode OpCode;
    switch (O.m_Type) {
    case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID:
      OpCode = OP::OpCode::ThreadId;
      break;
    case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:
      OpCode = OP::OpCode::GroupId;
      break;
    case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP:
      OpCode = OP::OpCode::ThreadIdInGroup;
      break;
    }
    CompType DxbcValueType = CompType::Kind::I32;
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);
    Function *F = m_pOP->GetOpFunc(OpCode, pDxbcValueType);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE Comp = OVH.GetComp();

      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU32Const(Comp);             // Component: x,y,z
      Value *pValue = m_pBuilder->CreateCall(F, Args);

      pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }
    break;
  }

  case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED: {
    OP::OpCode OpCode = OP::OpCode::FlattenedThreadIdInGroup;
    CompType DxbcValueType = CompType::Kind::I32;
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);
    Function *F = m_pOP->GetOpFunc(OpCode, pDxbcValueType);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      Value *Args[1];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Value *pValue = m_pBuilder->CreateCall(F, Args);

      pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }
    break;
  }

  case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    unsigned Register = O.m_Index[0].m_RegIndex;
    Value *pRowIndexValue = LoadOperandIndex(O.m_Index[0], O.m_IndexType[0]);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE Comp = OVH.GetComp();
      // Retrieve signature element.
      const DxilSignatureElement *E =
          m_pPatchConstantSignature->GetElement(Register, Comp);
      CompType DxbcValueType = E->GetCompType();
      if (DxbcValueType.IsBoolTy()) {
        DxbcValueType = CompType::getI32();
      }
      Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

      // Make row/col index relative within element.
      Value *pRowIndexValueRel = m_pBuilder->CreateSub(
          pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));

      Value *Args[4];
      Args[0] =
          m_pOP->GetU32Const((unsigned)OP::OpCode::LoadPatchConstant); // OpCode
      Args[1] =
          m_pOP->GetU32Const(E->GetID()); // Patch constant signature element ID
      Args[2] = pRowIndexValueRel;        // Row, relative to the element
      Args[3] = m_pOP->GetU8Const(
          Comp - E->GetStartCol()); // Col, relative to the element
      Function *F =
          m_pOP->GetOpFunc(OP::OpCode::LoadPatchConstant, pDxbcValueType);
      Value *pValue = m_pBuilder->CreateCall(F, Args);

      pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }

    break;
  }

  case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D);
    OP::OpCode OpCode = OP::OpCode::LoadOutputControlPoint;
    unsigned Register =
        O.m_Index[1].m_RegIndex; // Starting index of the register range.
    Value *pUnitIndexValue = LoadOperandIndex(
        O.m_Index[0], O.m_IndexType[0]); // Vertex/point index expression.
    Value *pRowIndexValue = LoadOperandIndex(
        O.m_Index[1], O.m_IndexType[1]); // Row index expression.

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE Comp = OVH.GetComp();
      // Retrieve signature element.
      const DxilSignatureElement *E =
          m_pOutputSignature->GetElement(Register, Comp);
      CompType DxbcValueType = E->GetCompType();
      if (DxbcValueType.IsBoolTy()) {
        DxbcValueType = CompType::getI32();
      }
      Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

      // Make row/col index relative within element.
      Value *pRowIndexValueRel = m_pBuilder->CreateSub(
          pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));

      Value *Args[5];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU32Const(E->GetID()); // Output signature element ID
      Args[2] = pRowIndexValueRel;              // Row, relative to the element
      Args[3] = m_pOP->GetU8Const(
          Comp - E->GetStartCol()); // Col, relative to the element
      Args[4] = pUnitIndexValue;    // Vertex/point index
      Function *F = m_pOP->GetOpFunc(OpCode, pDxbcValueType);
      Value *pValue = m_pBuilder->CreateCall(F, Args);

      pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }

    break;
  }

  case D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT: {
    OP::OpCode OpCode = OP::OpCode::DomainLocation;
    CompType DxbcValueType = CompType::Kind::F32;
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);
    Function *F = m_pOP->GetOpFunc(OpCode, pDxbcValueType);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE Comp = OVH.GetComp();
      Value *Args[2];
      Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
      Args[1] = m_pOP->GetU8Const(Comp);              // Component
      Value *pValue = m_pBuilder->CreateCall(F, Args);

      pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }

    break;
  }

  case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:
  case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:
  case D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:
  case D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK:
  case D3D11_SB_OPERAND_TYPE_INNER_COVERAGE: {
    OP::OpCode OpCode;
    switch (O.m_Type) {
    case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:
      OpCode = OP::OpCode::OutputControlPointID;
      break;
    case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:
      OpCode = OP::OpCode::PrimitiveID;
      break;
    case D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:
      OpCode = OP::OpCode::GSInstanceID;
      break;
    case D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK:
      OpCode = OP::OpCode::Coverage;
      break;
    case D3D11_SB_OPERAND_TYPE_INNER_COVERAGE:
      OpCode = OP::OpCode::InnerCoverage;
      break;
    }
    CompType DxbcValueType = CompType::Kind::I32;
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);
    Function *F = m_pOP->GetOpFunc(OpCode, pDxbcValueType);

    Value *Args[1];
    Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
    Value *pValue = m_pBuilder->CreateCall(F, Args);

    pValue = CastDxbcValue(pValue, DxbcValueType, ValueType);
    pValue = ApplyOperandModifiers(pValue, O);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      OVH.SetValue(pValue);
    }
    break;
  }

  case D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER: {
    OP::OpCode OpCode = OP::OpCode::CycleCounterLegacy;
    Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));

    Value *Args[1];
    Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
    Value *pValue = m_pBuilder->CreateCall(F, Args);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE c = OVH.GetComp();
      switch (c) {
      case 0: {
        Value *pLo32 = m_pBuilder->CreateExtractValue(pValue, 0);
        pLo32 = CastDxbcValue(pLo32, CompType::Kind::I32, ValueType);
        OVH.SetValue(pLo32);
        break;
      }
      case 1: {
        Value *pHi32 = m_pBuilder->CreateExtractValue(pValue, 1);
        pHi32 = CastDxbcValue(pHi32, CompType::Kind::I32, ValueType);
        OVH.SetValue(pHi32);
        break;
      }
      default:
        OVH.SetValue(m_pOP->GetU32Const(0));
      }
    }
    break;
  }

  case D3D11_SB_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID:
  case D3D11_SB_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID: {
    Scope &HullScope = m_ScopeStack.FindParentHullLoop();
    Value *pValue = m_pBuilder->CreateLoad(HullScope.pInductionVar);
    pValue = ApplyOperandModifiers(pValue, O);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      OVH.SetValue(pValue);
    }

    break;
  }

  case D3D11_SB_OPERAND_TYPE_THIS_POINTER: {
    Value *pIfaceIdx = LoadOperandIndex(O.m_Index[0], O.m_IndexType[0]);
    // The CBuffer layout here is a UINT for the interface class type selection,
    // then 3 UINTs padding, per interface. After that, there's another 4 UINTs
    // per interface which defines the "this" pointer data. Note, legacy CBuffer
    // loads address their data in number of 4-float constants, not bytes or
    // single elements. Since the "this" data comes after 4 UINTs per interface,
    // adjust the CB offset just by the number of interfaces.
    Value *pCBOffset =
        m_pBuilder->CreateAdd(m_pOP->GetU32Const(m_NumIfaces), pIfaceIdx);

    Value *Args[3];
    Args[0] =
        m_pOP->GetU32Const((unsigned)OP::OpCode::CBufferLoadLegacy); // OpCode
    Args[1] = CreateHandle(
        m_pInterfaceDataBuffer->GetClass(), m_pInterfaceDataBuffer->GetID(),
        m_pOP->GetU32Const(m_pInterfaceDataBuffer->GetLowerBound()),
        false /*Nonuniform*/); // CBuffer handle
    Args[2] = pCBOffset;       // 0-based index into cbuffer instance
    Function *pCBufferLoadFunc = m_pOP->GetOpFunc(OP::OpCode::CBufferLoadLegacy,
                                                  Type::getInt32Ty(m_Ctx));
    Value *pCBufferRetValue = m_pBuilder->CreateCall(pCBufferLoadFunc, Args);

    for (OperandValueHelper OVH(SrcVal, Mask, O); !OVH.IsDone();
         OVH.Advance()) {
      BYTE Comp = OVH.GetComp();

      Value *pValue = m_pBuilder->CreateExtractValue(pCBufferRetValue, Comp);
      pValue = CastDxbcValue(pValue, CompType::Kind::I32, ValueType);
      pValue = ApplyOperandModifiers(pValue, O);

      OVH.SetValue(pValue);
    }
    break;
  }

  default:
    DXASSERT_ARGS(false, "Operand type %u is not yet implemented", O.m_Type);
  }
}

const DxilResource &DxbcConverter::LoadSRVOperand(
    OperandValue &SrcVal, D3D10ShaderBinary::CInstruction &Inst,
    const unsigned OpIdx, const CMask &Mask, const CompType &ValueType) {
  D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[OpIdx];
  DXASSERT(O.m_Type == D3D10_SB_OPERAND_TYPE_RESOURCE,
           "LoadSRVOperand should only be called for SRV operands.");
  const DxilResource &R = GetSRVFromOperand(Inst, OpIdx);

  // Setup SRV handle.
  Value *pHandle = GetCachedHandle(R);
  if (pHandle == nullptr) {
    // Create dynamic-index handle.
    pHandle = CreateHandle(R.GetClass(), R.GetID(),
                           LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]),
                           O.m_Nonuniform);
  }

  // Replicate handle values.
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (Mask.IsSet(c))
      SrcVal[c] = pHandle;
  }

  return R;
}

const DxilResource &
DxbcConverter::GetSRVFromOperand(D3D10ShaderBinary::CInstruction &Inst,
                                 const unsigned OpIdx) {
  D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[OpIdx];
  DXASSERT(O.m_Type == D3D10_SB_OPERAND_TYPE_RESOURCE,
           "GetSRVFromOperand should only be called for SRV operands.");
  // Upconvert operand to SM5.1.
  if (O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D) {
    O.m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    O.m_IndexType[1] = O.m_IndexType[0];
    O.m_Index[1] = O.m_Index[0];
  }

  // Retrieve SRV range ID and record.
  if (O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32) {
    unsigned RangeID = O.m_Index[0].m_RegIndex;
    unsigned RecIdx = m_SRVRangeMap[RangeID];
    return m_pPR->GetSRV(RecIdx);
  } else {
    return GetInterfacesSRVDecl(Inst);
  }
}

void DxbcConverter::StoreOperand(OperandValue &DstVal,
                                 const D3D10ShaderBinary::CInstruction &Inst,
                                 const unsigned OpIdx, const CMask &Mask,
                                 const CompType &ValueType) {
  const D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[OpIdx];

  // Mark value as precise, if needed.
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    Value *pValue = DstVal[c];
    if (pValue != nullptr)
      DstVal[c] = MarkPrecise(DstVal[c], c);
  }

  ApplyInstructionModifiers(DstVal, Inst);

  switch (O.m_Type) {
  case D3D10_SB_OPERAND_TYPE_TEMP: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    unsigned Reg = O.m_Index[0].m_RegIndex;
    CompType DxbcValueType =
        DXBC::GetCompTypeFromMinPrec(O.m_MinPrecision, ValueType);
    if (DxbcValueType.IsBoolTy()) {
      DxbcValueType = CompType::getI32();
    }
    Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

    if (DxbcValueType.GetKind() != CompType::Kind::F64) {
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!Mask.IsSet(c))
          continue;

        Value *Args[3];
        Args[0] =
            m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegStore); // OpCode
        Args[1] = m_pOP->GetU32Const(
            DXBC::GetRegIndex(Reg, c)); // Linearized register index
        Args[2] = MarkPrecise(
            CastDxbcValue(DstVal[c], ValueType, DxbcValueType), c); // Value
        Function *F =
            m_pOP->GetOpFunc(OP::OpCode::TempRegStore, pDxbcValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      }
    } else {
      for (BYTE c = 0; c < DXBC::kWidth; c += 2) {
        if (!Mask.IsSet(c))
          continue;

        Value *pSDT; // Split double type.
        {
          Value *Args[2];
          Args[0] =
              m_pOP->GetU32Const((unsigned)OP::OpCode::SplitDouble); // OpCode
          Args[1] = DstVal[c]; // Double value
          Function *F =
              m_pOP->GetOpFunc(OP::OpCode::SplitDouble, pDxbcValueType);
          pSDT = MarkPrecise(m_pBuilder->CreateCall(F, Args), c);
        }

        Value *Args[3];
        Args[0] =
            m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegStore); // OpCode
        Args[1] = m_pOP->GetU32Const(
            DXBC::GetRegIndex(Reg, c)); // Linearized register index 1
        Args[2] = MarkPrecise(m_pBuilder->CreateExtractValue(pSDT, 0),
                              c); // Value to store
        Function *F =
            m_pOP->GetOpFunc(OP::OpCode::TempRegStore, Type::getInt32Ty(m_Ctx));
        Value *pVal = m_pBuilder->CreateCall(F, Args);
        MarkPrecise(pVal, c);
        Args[1] = m_pOP->GetU32Const(
            DXBC::GetRegIndex(Reg, c + 1)); // Linearized register index 2
        Args[2] = MarkPrecise(m_pBuilder->CreateExtractValue(pSDT, 1),
                              c + 1); // Value to store
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      }
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D);
    DXASSERT_DXBC(O.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    unsigned Reg = O.m_Index[0].m_RegIndex;
    IndexableReg &IRRec = m_IndexableRegs[Reg];
    Value *pXRegIndex = LoadOperandIndex(O.m_Index[1], O.m_IndexType[1]);
    Value *pRegIndex =
        m_pBuilder->CreateMul(pXRegIndex, m_pOP->GetI32Const(IRRec.NumComps));
    CompType DxbcValueType =
        DXBC::GetCompTypeFromMinPrec(O.m_MinPrecision, ValueType);
    if (DxbcValueType.IsBoolTy()) {
      DxbcValueType = CompType::getI32();
    }

    if (DxbcValueType.GetKind() != CompType::Kind::F64) {
      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!Mask.IsSet(c))
          continue;

        // Create GEP.
        Value *pIndex = m_pBuilder->CreateAdd(pRegIndex, m_pOP->GetU32Const(c));
        Value *pGEPIndices[2] = {m_pOP->GetU32Const(0), pIndex};
        if (!DxbcValueType.HasMinPrec()) {
          Value *pBasePtr = m_IndexableRegs[Reg].pValue32;
          Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
          Value *pValue = MarkPrecise(
              CastDxbcValue(DstVal[c], ValueType, CompType::getF32()), c);
          MarkPrecise(
              m_pBuilder->CreateAlignedStore(pValue, pPtr, kRegCompAlignment),
              c);
        } else {
          Value *pBasePtr = m_IndexableRegs[Reg].pValue16;
          Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
          Value *pValue = MarkPrecise(
              CastDxbcValue(DstVal[c], ValueType, CompType::getF16()), c);
          MarkPrecise(m_pBuilder->CreateAlignedStore(pValue, pPtr,
                                                     kRegCompAlignment / 2),
                      c);
        }
      }
    } else {
      // Double precision.
      for (BYTE c = 0; c < DXBC::kWidth; c += 2) {
        if (!Mask.IsSet(c))
          continue;

        // Create GEP.
        Value *pIndex = m_pBuilder->CreateAdd(pRegIndex, m_pOP->GetU32Const(c));
        Value *pGEPIndices[] = {pIndex};
        Value *pBasePtr = m_pBuilder->CreateBitCast(
            m_IndexableRegs[Reg].pValue32, Type::getDoublePtrTy(m_Ctx));
        Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
        MarkPrecise(m_pBuilder->CreateAlignedStore(DstVal[c], pPtr,
                                                   kRegCompAlignment * 2));
      }
    }
    break;
  }

  case D3D10_SB_OPERAND_TYPE_OUTPUT: {
    unsigned Reg = O.m_Index[0].m_RegIndex;
    // Row index expression.
    Value *pRowIndexValue = LoadOperandIndex(O.m_Index[0], O.m_IndexType[0]);

    bool bStoreOutputReg =
        !(m_pSM->IsGS() && m_pPR->HasMultipleOutputStreams());

    if (bStoreOutputReg) {
      for (unsigned c = 0; c < DXBC::kWidth; c++) {
        if (!Mask.IsSet(c))
          continue;

        // Retrieve signature element.
        OP::OpCode OpCode;
        const DxilSignatureElement *E;
        if (!m_bPatchConstantPhase) {
          E = m_pOutputSignature->GetElementWithStream(
              Reg, c, m_pPR->GetOutputStream());
          OpCode = OP::OpCode::StoreOutput;
        } else {
          E = m_pPatchConstantSignature->GetElementWithStream(
              Reg, c, m_pPR->GetOutputStream());
          OpCode = OP::OpCode::StorePatchConstant;
        }
        CompType DxbcValueType = E->GetCompType();
        if (DxbcValueType.IsBoolTy()) {
          DxbcValueType = CompType::getI32();
        }
        Type *pLlvmDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

        // Make row index relative within element.
        Value *pRowIndexValueRel = m_pBuilder->CreateSub(
            pRowIndexValue, m_pOP->GetU32Const(E->GetStartRow()));

        Value *Args[5];
        Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
        Args[1] = m_pOP->GetU32Const(E->GetID()); // Output signature element ID
        Args[2] = pRowIndexValueRel; // Row, relative to the element
        Args[3] = m_pOP->GetU8Const(
            c - E->GetStartCol()); // Col, relative to the element
        Args[4] = MarkPrecise(
            CastDxbcValue(DstVal[c], ValueType, DxbcValueType), c); // Value
        Function *F = m_pOP->GetOpFunc(OpCode, pLlvmDxbcValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      }
    } else {
      // In GS with multiple streams, output register file is shared among the
      // streams. Store the values into additional temp registers, and later,
      // store these at the emit points.
      CompType DxbcValueType =
          DXBC::GetCompTypeFromMinPrec(O.m_MinPrecision, ValueType);
      if (DxbcValueType.IsBoolTy()) {
        DxbcValueType = CompType::getI32();
      }
      Type *pDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

      for (BYTE c = 0; c < DXBC::kWidth; c++) {
        if (!Mask.IsSet(c))
          continue;

        Value *Args[3];
        Args[0] =
            m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegStore); // OpCode
        unsigned TempReg = GetGSTempRegForOutputReg(Reg);
        Args[1] = m_pOP->GetU32Const(
            DXBC::GetRegIndex(TempReg, c)); // Linearized register index
        Args[2] =
            MarkPrecise(CastDxbcValue(DstVal[c], ValueType, DxbcValueType),
                        c); // Value to store
        Function *F =
            m_pOP->GetOpFunc(OP::OpCode::TempRegStore, pDxbcValueType);
        MarkPrecise(m_pBuilder->CreateCall(F, Args));
      }
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:
  case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL:
  case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:
  case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:
  case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK: {
    DXASSERT_DXBC(O.m_IndexDimension == D3D10_SB_OPERAND_INDEX_0D);
    for (unsigned c = 0; c < DXBC::kWidth; c++) {
      if (!Mask.IsSet(c))
        continue;

      // Retrieve signature element.
      DXASSERT(m_pSM->IsPS(), "PS has only one output stream.");
      const DxilSignatureElement *E = m_pOutputSignature->GetElement(O.m_Type);
      CompType DxbcValueType = E->GetCompType();
      Type *pLlvmDxbcValueType = DxbcValueType.GetLLVMType(m_Ctx);

      Value *Args[5];
      Args[0] = m_pOP->GetU32Const((unsigned)OP::OpCode::StoreOutput); // OpCode
      Args[1] = m_pOP->GetU32Const(E->GetID()); // Output signature element ID
      Args[2] = m_pOP->GetU32Const(0);          // Row, relative to the element
      Args[3] = m_pOP->GetU8Const(
          c - E->GetStartCol()); // Col, relative to the element
      Args[4] = MarkPrecise(CastDxbcValue(DstVal[c], ValueType, DxbcValueType),
                            c); // Value
      Function *F =
          m_pOP->GetOpFunc(OP::OpCode::StoreOutput, pLlvmDxbcValueType);
      MarkPrecise(m_pBuilder->CreateCall(F, Args));
    }

    break;
  }

  case D3D10_SB_OPERAND_TYPE_NULL:
    break;

  default:
    DXASSERT_ARGS(false, "Operand type %u is not yet implemented", O.m_Type);
  }
}

Value *DxbcConverter::LoadOperandIndex(
    const D3D10ShaderBinary::COperandIndex &OpIndex,
    const D3D10_SB_OPERAND_INDEX_REPRESENTATION IndexType) {
  Value *pValue = nullptr;

  switch (IndexType) {
  case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
    DXASSERT_DXBC(OpIndex.m_RelRegType == D3D10_SB_OPERAND_TYPE_IMMEDIATE32);
    pValue = m_pOP->GetU32Const(OpIndex.m_RegIndex);
    break;

  case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:
    DXASSERT_DXBC(false);
    break;

  case D3D10_SB_OPERAND_INDEX_RELATIVE:
    pValue = LoadOperandIndexRelative(OpIndex);
    break;

  case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE: {
    unsigned Offset = OpIndex.m_RegIndex;
    pValue = LoadOperandIndexRelative(OpIndex);
    if (Offset != 0) {
      pValue = m_pBuilder->CreateAdd(pValue, m_pOP->GetU32Const(Offset));
    }
    break;
  }

  case D3D10_SB_OPERAND_INDEX_IMMEDIATE64_PLUS_RELATIVE:
    DXASSERT_DXBC(false);
    break;

  default:
    DXASSERT_DXBC(false);
    break;
  }

  return pValue;
}

Value *DxbcConverter::LoadOperandIndexRelative(
    const D3D10ShaderBinary::COperandIndex &OpIndex) {
  Value *pValue = nullptr;

  switch (OpIndex.m_RelRegType) {
  case D3D10_SB_OPERAND_TYPE_TEMP: {
    unsigned Reg = OpIndex.m_RelIndex;
    unsigned Comp = OpIndex.m_ComponentName;

    Value *Args[2];
    Args[0] = m_pOP->GetU32Const((unsigned)OP::OpCode::TempRegLoad); // OpCode
    Args[1] = m_pOP->GetU32Const(
        DXBC::GetRegIndex(Reg, Comp)); // Linearized register index
    Function *F =
        m_pOP->GetOpFunc(OP::OpCode::TempRegLoad, Type::getInt32Ty(m_Ctx));
    pValue = m_pBuilder->CreateCall(F, Args);

    break;
  }

  case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP: {
    unsigned Reg = OpIndex.m_RelIndex;
    unsigned RegIdx = OpIndex.m_RelIndex1;
    unsigned Comp = OpIndex.m_ComponentName;
    IndexableReg &IRRec = m_IndexableRegs[Reg];

    Value *pGEPIndices[2] = {
        m_pOP->GetU32Const(0),
        m_pOP->GetU32Const(RegIdx * IRRec.NumComps + Comp)};
    Value *pBasePtr = m_IndexableRegs[Reg].pValue32;
    Value *pPtr = m_pBuilder->CreateGEP(pBasePtr, pGEPIndices);
    pValue = m_pBuilder->CreateAlignedLoad(pPtr, kRegCompAlignment);
    DXASSERT(pValue->getType()->isFloatTy(),
             "otherwise broke the assumption that alloca locations are floats");
    pValue = CastDxbcValue(pValue, CompType::getF32(), CompType::getI32());

    break;
  }

  default:
    DXASSERT_DXBC(false);
  }

  return pValue;
}

Value *DxbcConverter::CastDxbcValue(Value *pValue, const CompType &SrcType,
                                    const CompType &DstType) {
  if (SrcType == DstType)
    return pValue;

  DXASSERT(SrcType.GetLLVMType(m_Ctx) == pValue->getType(),
           "otherwise caller passed incorrect args");

  switch (SrcType.GetKind()) {
  case CompType::Kind::I1:
    switch (DstType.GetKind()) {
    case CompType::Kind::I1:
      return pValue;
    case CompType::Kind::I16:
    case CompType::Kind::U16:
      return m_pBuilder->CreateSExt(pValue, Type::getInt16Ty(m_Ctx));
    case CompType::Kind::I32:
    case CompType::Kind::U32:
      return m_pBuilder->CreateSExt(pValue, Type::getInt32Ty(m_Ctx));
    case CompType::Kind::F16:
      return m_pBuilder->CreateBitCast(
          m_pBuilder->CreateSExt(pValue, Type::getInt16Ty(m_Ctx)),
          Type::getHalfTy(m_Ctx));
    case CompType::Kind::F32:
      return m_pBuilder->CreateBitCast(
          m_pBuilder->CreateSExt(pValue, Type::getInt32Ty(m_Ctx)),
          Type::getFloatTy(m_Ctx));
    default:
      break;
    }
    break;

  case CompType::Kind::I16:
    switch (DstType.GetKind()) {
    case CompType::Kind::I1:
      return m_pBuilder->CreateICmpNE(pValue, m_pOP->GetI16Const(0));
    case CompType::Kind::U16:
      DXASSERT_DXBC(false);
      return pValue;
    case CompType::Kind::I32:
    case CompType::Kind::U32:
      return m_pBuilder->CreateSExt(pValue, Type::getInt32Ty(m_Ctx));
    case CompType::Kind::F16: {
      DXASSERT_DXBC(false);
      pValue = m_pBuilder->CreateSExt(pValue, Type::getInt32Ty(m_Ctx));
      pValue = CreateBitCast(pValue, CompType::getI32(), CompType::getF32());
      return m_pBuilder->CreateFPTrunc(pValue, Type::getHalfTy(m_Ctx));
    }
    case CompType::Kind::F32: { // mov
      pValue = m_pBuilder->CreateSExt(pValue, Type::getInt32Ty(m_Ctx));
      return CreateBitCast(pValue, CompType::getI32(), CompType::getF32());
    }
    default:
      break;
    }
    break;

  case CompType::Kind::U16:
    switch (DstType.GetKind()) {
    case CompType::Kind::I1:
      return m_pBuilder->CreateICmpNE(pValue, m_pOP->GetU16Const(0));
    case CompType::Kind::I16:
      DXASSERT_DXBC(false);
      return pValue;
    case CompType::Kind::I32:
    case CompType::Kind::U32:
      return m_pBuilder->CreateZExt(pValue, Type::getInt32Ty(m_Ctx));
    case CompType::Kind::F16: {
      DXASSERT_DXBC(false);
      pValue = m_pBuilder->CreateZExt(pValue, Type::getInt32Ty(m_Ctx));
      pValue = CreateBitCast(pValue, CompType::getI32(), CompType::getF32());
      return m_pBuilder->CreateFPTrunc(pValue, Type::getHalfTy(m_Ctx));
    }
    case CompType::Kind::F32: { // mov
      pValue = m_pBuilder->CreateZExt(pValue, Type::getInt32Ty(m_Ctx));
      return CreateBitCast(pValue, CompType::getI32(), CompType::getF32());
    }
    default:
      break;
    }
    break;

  case CompType::Kind::I32:
  case CompType::Kind::U32:
    switch (DstType.GetKind()) {
    case CompType::Kind::I1:
      return m_pBuilder->CreateICmpNE(pValue, m_pOP->GetI32Const(0));
    case CompType::Kind::I16:
    case CompType::Kind::U16:
      return m_pBuilder->CreateTrunc(pValue, Type::getInt16Ty(m_Ctx));
    case CompType::Kind::I32:
    case CompType::Kind::U32:
      return pValue;
    case CompType::Kind::F16: {
      DXASSERT_DXBC(false);
      pValue = CreateBitCast(pValue, CompType::getI32(), CompType::getF32());
      return m_pBuilder->CreateFPTrunc(pValue, Type::getHalfTy(m_Ctx));
    }
    case CompType::Kind::F32:
      return CreateBitCast(pValue, CompType::getI32(), CompType::getF32());
    default:
      break;
    }
    break;

  case CompType::Kind::F16:
    switch (DstType.GetKind()) {
    case CompType::Kind::I16:
    case CompType::Kind::U16: {
      DXASSERT_DXBC(false);
      pValue = m_pBuilder->CreateFPExt(pValue, Type::getFloatTy(m_Ctx));
      pValue = CreateBitCast(pValue, CompType::getF32(), CompType::getI32());
      return m_pBuilder->CreateTrunc(pValue, Type::getInt16Ty(m_Ctx));
    }
    case CompType::Kind::I32:
    case CompType::Kind::U32: { // mov
      pValue = m_pBuilder->CreateFPExt(pValue, Type::getFloatTy(m_Ctx));
      return CreateBitCast(pValue, CompType::getF32(), CompType::getI32());
    }
    case CompType::Kind::F32:
      return m_pBuilder->CreateFPExt(pValue, Type::getFloatTy(m_Ctx));
    default:
      break;
    }
    break;

  case CompType::Kind::F32:
    switch (DstType.GetKind()) {
    case CompType::Kind::I1: {
      pValue = CreateBitCast(pValue, CompType::getF32(), CompType::getI32());
      return m_pBuilder->CreateICmpNE(pValue, m_pOP->GetI32Const(0));
    }
    case CompType::Kind::I16:
    case CompType::Kind::U16: { // min-prec for TGSM load.
      pValue = CreateBitCast(pValue, CompType::getF32(), CompType::getI32());
      return m_pBuilder->CreateTrunc(pValue, Type::getInt16Ty(m_Ctx));
    }
    case CompType::Kind::I32:
    case CompType::Kind::U32:
      return CreateBitCast(pValue, CompType::getF32(), CompType::getI32());
    case CompType::Kind::F16:
      return m_pBuilder->CreateFPTrunc(pValue, Type::getHalfTy(m_Ctx));
    default:
      break;
    }
    break;

  default:
    break;
  }

  DXASSERT(false, "unsupported cast combination");
  return nullptr;
}

Value *DxbcConverter::CreateBitCast(Value *pValue, const CompType &SrcType,
                                    const CompType &DstType) {
  DXASSERT(SrcType.GetLLVMType(m_Ctx) == pValue->getType(),
           "otherwise caller passed incorrect args");

  OP::OpCode OpCode = (OP::OpCode)(-1);

  switch (SrcType.GetKind()) {
  case CompType::Kind::I16:
    switch (DstType.GetKind()) {
    case CompType::Kind::F16:
      OpCode = OP::OpCode::BitcastI16toF16;
      break;
    }
    break;

  case CompType::Kind::I32:
    switch (DstType.GetKind()) {
    case CompType::Kind::F32:
      OpCode = OP::OpCode::BitcastI32toF32;
      break;
    }
    break;

  case CompType::Kind::I64:
    switch (DstType.GetKind()) {
    case CompType::Kind::F64:
      OpCode = OP::OpCode::BitcastI64toF64;
      break;
    }
    break;

  case CompType::Kind::F16:
    switch (DstType.GetKind()) {
    case CompType::Kind::I16:
      OpCode = OP::OpCode::BitcastF16toI16;
      break;
    }
    break;

  case CompType::Kind::F32:
    switch (DstType.GetKind()) {
    case CompType::Kind::I32:
      OpCode = OP::OpCode::BitcastF32toI32;
      break;
    }
    break;

  case CompType::Kind::F64:
    switch (DstType.GetKind()) {
    case CompType::Kind::I64:
      OpCode = OP::OpCode::BitcastF64toI64;
      break;
    }
    break;
  }

  Value *Args[2];
  Args[0] = m_pOP->GetU32Const((unsigned)OpCode); // OpCode
  Args[1] = pValue;                               // Input

  Function *F = m_pOP->GetOpFunc(OpCode, Type::getVoidTy(m_Ctx));

  return m_pBuilder->CreateCall(F, Args);
}

Value *
DxbcConverter::ApplyOperandModifiers(Value *pValue,
                                     const D3D10ShaderBinary::COperandBase &O) {
  bool bAbsModifier = (O.m_Modifier & D3D10_SB_OPERAND_MODIFIER_ABS) != 0;
  bool bNegModifier = (O.m_Modifier & D3D10_SB_OPERAND_MODIFIER_NEG) != 0;

  if (bAbsModifier) {
    DXASSERT_DXBC(pValue->getType()->isFloatingPointTy());
    Function *F = m_pOP->GetOpFunc(OP::OpCode::FAbs, pValue->getType());
    Value *Args[2];
    Args[0] = m_pOP->GetU32Const((unsigned)OP::OpCode::FAbs);
    Args[1] = pValue;
    pValue = m_pBuilder->CreateCall(F, Args);
  }

  if (bNegModifier) {
    if (pValue->getType()->isFloatingPointTy()) {
      pValue = m_pBuilder->CreateFNeg(pValue);
    } else {
      DXASSERT_DXBC(pValue->getType()->isIntegerTy());
      pValue = m_pBuilder->CreateNeg(pValue);
    }
  }

  return pValue;
}

void DxbcConverter::ApplyInstructionModifiers(
    OperandValue &DstVal, const D3D10ShaderBinary::CInstruction &Inst) {
  if (Inst.m_bSaturate) {
    map<Value *, Value *> M;

    for (BYTE c = 0; c < DXBC::kWidth; c++) {
      Value *pValue = DstVal[c];
      if (pValue == nullptr)
        continue;

      auto const &it = M.find(pValue);
      if (it != M.end()) {
        DstVal[c] = it->second;
      } else {
        Value *Args[2];
        Args[0] = m_pOP->GetU32Const((unsigned)OP::OpCode::Saturate); // OpCode
        Args[1] = pValue;                                             // Value
        Function *F = m_pOP->GetOpFunc(OP::OpCode::Saturate, pValue->getType());
        Value *pSaturatedValue =
            MarkPrecise(m_pBuilder->CreateCall(F, Args), c);
        DstVal[c] = pSaturatedValue;
        M[pValue] = pSaturatedValue;
      }

      if (pValue->getType() == Type::getDoubleTy(m_Ctx)) {
        c++;
      }
    }
  }
}

CompType
DxbcConverter::InferOperandType(const D3D10ShaderBinary::CInstruction &Inst,
                                const unsigned OpIdx, const CMask &Mask) {
  const D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[OpIdx];

  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (!Mask.IsSet(c))
      continue;

    switch (O.m_Type) {
    case D3D10_SB_OPERAND_TYPE_INPUT: {
      unsigned Reg =
          O.m_Index[(m_pSM->IsGS() || m_pSM->IsHS()) ? 1 : 0].m_RegIndex;
      unsigned Comp = O.m_ComponentName;
      if (O.m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE)
        Comp = O.m_Swizzle[c];
      const DxilSignatureElement *E = m_pInputSignature->GetElement(Reg, Comp);
      return E->GetCompType();
    }

    case D3D10_SB_OPERAND_TYPE_OUTPUT: {
      unsigned Reg = O.m_Index[0].m_RegIndex;

      if (!m_pSM->IsGS()) {
        if (!m_bPatchConstantPhase) {
          const DxilSignatureElement *E =
              m_pOutputSignature->GetElement(Reg, c);
          return E->GetCompType();
        } else {
          const DxilSignatureElement *E =
              m_pPatchConstantSignature->GetElement(Reg, c);
          return E->GetCompType();
        }
      } else {
        CompType CT;
        bool bCTInitialized = false;
        for (unsigned Stream = 0; Stream < DXIL::kNumOutputStreams; Stream++) {
          const DxilSignatureElement *E =
              m_pOutputSignature->GetElement(Reg, c);
          if (E == nullptr)
            continue;

          if (!bCTInitialized) {
            bCTInitialized = true;
            CT = E->GetCompType();
          } else {
            if (CT.GetKind() != E->GetCompType().GetKind())
              return CompType::getInvalid();
          }
        }

        return CT;
      }
    }

    default:
      break;
    }
  }

  if (O.m_MinPrecision != D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    return DXBC::GetCompTypeFromMinPrec(O.m_MinPrecision,
                                        CompType::getInvalid());
  }

  return CompType::getInvalid();
}

void DxbcConverter::CheckDxbcString(const char *pStr,
                                    const void *pMaxPtrInclusive) {
  for (;; pStr++) {
    if (pStr > pMaxPtrInclusive)
      IFT(DXC_E_INCORRECT_DXBC);
    if (*pStr == '\0')
      break;
  }
}

void DxbcConverter::Optimize() {
  class PassManager PassManager;

#if DXBCCONV_DBG
  IFTBOOL(
      !verifyModule(*m_pModule),
      DXC_E_IR_VERIFICATION_FAILED); // verifyModule returns true for failure
#endif

  // Verify that CFG is reducible.
  IFTBOOL(IsReducible(*m_pModule, IrreducibilityAction::ThrowException),
          DXC_E_IRREDUCIBLE_CFG);

  if (m_bRunDxilCleanup) {
    PassManager.add(createDxilCleanupPass());
    PassManager.run(*m_pModule);
  }

#if DXBCCONV_DBG
  IFTBOOL(!verifyModule(*m_pModule), DXC_E_IR_VERIFICATION_FAILED);
#endif
}

void DxbcConverter::CreateBranchIfNeeded(BasicBlock *pBB,
                                         BasicBlock *pTargetBB) {
  bool bNeedBranch = true;
  if (!pBB->empty()) {
    Instruction *pLastInst = &pBB->getInstList().back();
    if (pLastInst->getOpcode() == Instruction::Br ||
        pLastInst->getOpcode() == Instruction::Ret)
      bNeedBranch = false;
    else
      DXASSERT(!pLastInst->isTerminator(),
               "otherwise broke possible assumptions of control flow");
  }

  if (bNeedBranch)
    m_pBuilder->CreateBr(pTargetBB);
}

Value *DxbcConverter::LoadZNZCondition(D3D10ShaderBinary::CInstruction &Inst,
                                       const unsigned OpIdx) {
  D3D10ShaderBinary::COperandBase &O = Inst.m_Operands[OpIdx];
  D3D10_SB_INSTRUCTION_TEST_BOOLEAN TestType = Inst.m_Test;
  BYTE Comp = (BYTE)O.m_ComponentName;
  CMask ReadMask = CMask::MakeCompMask(Comp);
  OperandValue In1;
  LoadOperand(In1, Inst, 0, ReadMask, CompType::getI32());

  Value *pCond = In1[Comp];
  if (TestType == D3D10_SB_INSTRUCTION_TEST_NONZERO) {
    pCond = m_pBuilder->CreateICmpNE(pCond, m_pOP->GetI32Const(0));
  } else {
    pCond = m_pBuilder->CreateICmpEQ(pCond, m_pOP->GetI32Const(0));
  }

  return pCond;
}

D3D11_SB_OPERAND_MIN_PRECISION
DxbcConverter::GetHigherPrecision(D3D11_SB_OPERAND_MIN_PRECISION p1,
                                  D3D11_SB_OPERAND_MIN_PRECISION p2) {
  if (p1 == D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8)
    p1 = D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16;
  if (p2 == D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8)
    p2 = D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16;

  if (p1 == p2)
    return p1;

  return D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT;
}

unsigned DxbcConverter::GetGSTempRegForOutputReg(unsigned OutputReg) const {
  return m_NumTempRegs + OutputReg;
}

//------------------------------------------------------------------------------
//
//  DxbcConverter::ScopeStack methods.
//
DxbcConverter::ScopeStack::ScopeStack()
    : m_FuncCount(0), m_IfCount(0), m_LoopCount(0), m_SwitchCount(0),
      m_HullLoopCount(0) {}

DxbcConverter::Scope &DxbcConverter::ScopeStack::Top() {
  IFTBOOL(!m_Scopes.empty(), E_FAIL);
  return m_Scopes.back();
}

DxbcConverter::Scope &DxbcConverter::ScopeStack::Push(enum Scope::Kind Kind,
                                                      BasicBlock *pPreScopeBB) {
  Scope S;
  DXASSERT(Kind < Scope::LastKind,
           "otherwise the caller passed incorrect scope kind value");
  S.Kind = Kind;
  S.pPreScopeBB = pPreScopeBB;
  switch (Kind) {
  case Scope::Function:
    S.NameIndex = m_FuncCount++;
    break;
  case Scope::If:
    S.NameIndex = m_IfCount++;
    break;
  case Scope::Loop:
    S.NameIndex = m_LoopCount++;
    break;
  case Scope::Switch:
    S.NameIndex = m_SwitchCount++;
    break;
  case Scope::HullLoop:
    S.NameIndex = m_HullLoopCount++;
    break;
  }
  m_Scopes.emplace_back(S);
  return Top();
}

void DxbcConverter::ScopeStack::Pop() { m_Scopes.pop_back(); }

bool DxbcConverter::ScopeStack::IsEmpty() const { return m_Scopes.empty(); }

DxbcConverter::Scope &DxbcConverter::ScopeStack::FindParentLoop() {
  for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
    Scope &Scope = *it;

    if (Scope.Kind == Scope::Loop)
      return Scope;
  }

  DXASSERT(false, "otherwise was not able to find the parent enclosing scope");
  IFTBOOL(false, E_FAIL);
  return Top();
}

DxbcConverter::Scope &DxbcConverter::ScopeStack::FindParentLoopOrSwitch() {
  for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
    Scope &Scope = *it;

    if (Scope.Kind == Scope::Loop || Scope.Kind == Scope::Switch)
      return Scope;
  }

  DXASSERT(false, "otherwise was not able to find the parent enclosing scope");
  IFTBOOL(false, E_FAIL);
  return Top();
}

DxbcConverter::Scope &DxbcConverter::ScopeStack::FindParentFunction() {
  for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
    Scope &Scope = *it;

    if (Scope.Kind == Scope::Function)
      return Scope;
  }

  DXASSERT(false, "otherwise was not able to find the parent enclosing scope");
  IFTBOOL(false, E_FAIL);
  return Top();
}

DxbcConverter::Scope &DxbcConverter::ScopeStack::FindParentHullLoop() {
  for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
    Scope &Scope = *it;

    if (Scope.Kind == Scope::HullLoop)
      return Scope;
  }

  DXASSERT(false, "otherwise was not able to find the parent enclosing scope");
  IFTBOOL(false, E_FAIL);
  return Top();
}

string DxbcConverter::SynthesizeResGVName(const char *pNamePrefix,
                                          unsigned ID) {
  string GVName;
  raw_string_ostream GVNameStream(GVName);
  (GVNameStream << pNamePrefix << ID).flush();
  return GVName;
}

StructType *DxbcConverter::GetStructResElemType(unsigned StructSizeInBytes) {
  string GVTypeName;
  raw_string_ostream GVTypeNameStream(GVTypeName);
  (GVTypeNameStream << "dx.types.i8x" << StructSizeInBytes).flush();
  StructType *pGVType = m_pModule->getTypeByName(GVTypeName);
  if (pGVType == nullptr) {
    pGVType = StructType::create(
        m_Ctx, ArrayType::get(Type::getInt8Ty(m_Ctx), StructSizeInBytes),
        GVTypeName);
  }
  return pGVType;
}

StructType *DxbcConverter::GetTypedResElemType(CompType CT) {
  string GVTypeName;
  raw_string_ostream GVTypeNameStream(GVTypeName);
  (GVTypeNameStream << "dx.types." << CT.GetName()).flush();
  StructType *pGVType = m_pModule->getTypeByName(GVTypeName);
  if (pGVType == nullptr) {
    Type *pElemType = nullptr;
    if (CT.GetKind() == CompType::Kind::SNormF32) {
      pElemType = m_pPR->GetTypeSystem().GetSNormF32Type(1);
    } else if (CT.GetKind() == CompType::Kind::UNormF32) {
      pElemType = m_pPR->GetTypeSystem().GetUNormF32Type(1);
    } else {
      pElemType = CT.GetLLVMType(m_Ctx);
    }
    if (!pElemType->isStructTy()) {
      pGVType = StructType::create(m_Ctx, pElemType, GVTypeName);
    } else {
      pGVType = dyn_cast<StructType>(pElemType);
    }
  }
  return pGVType;
}

UndefValue *DxbcConverter::DeclareUndefPtr(Type *pType, unsigned AddrSpace) {
  Type *pPtrType = PointerType::get(pType, AddrSpace);
  UndefValue *pUV = UndefValue::get(pPtrType);
  return pUV;
}

Value *DxbcConverter::MarkPrecise(Value *pVal, BYTE Comp) {
  if ((Comp == BYTE(-1) && !m_PreciseMask.IsZero()) ||
      (Comp != BYTE(-1) && m_PreciseMask.IsSet(Comp))) {
    if (Instruction *pInst = dyn_cast<Instruction>(pVal)) {
      bool bAttachPreciseMD = true;
      if (dyn_cast<FPMathOperator>(pInst) != nullptr &&
          dyn_cast<CallInst>(pInst) == nullptr) {
        FastMathFlags FMF;
        pInst->copyFastMathFlags(FMF);
        bAttachPreciseMD = false;
      }

      if (bAttachPreciseMD) {
        MDNode *pMD =
            MDNode::get(m_Ctx, ConstantAsMetadata::get(m_pOP->GetI32Const(1)));
        pInst->setMetadata(DxilMDHelper::kDxilPreciseAttributeMDName, pMD);
      }
    }
  }

  return pVal;
}

void DxbcConverter::SerializeDxil(SmallVectorImpl<char> &DxilBitcode) {
  raw_svector_ostream DxilStream(DxilBitcode);
  // a. Reserve header.
  DxilProgramHeader Header = {};
  DxilStream.write((char *)&Header, sizeof(Header));
  // b. Bitcode.
  WriteBitcodeToFile(m_pModule.get(), DxilStream);
  DxilStream.flush();
  // c. Fix header.
  uint32_t bitcodeSize =
      (uint32_t)DxilBitcode.size_in_bytes() - sizeof(DxilProgramHeader);
  DxilProgramHeader *pHeader = (DxilProgramHeader *)DxilBitcode.data();
  InitProgramHeader(
      *pHeader,
      EncodeVersion(m_pSM->GetKind(), m_pSM->GetMajor(), m_pSM->GetMinor()),
      DXIL::MakeDxilVersion(1, 0), bitcodeSize);
  // d. Trailer. Pad to 16 bytes.
  while (DxilBitcode.size() & 0xF) {
    DxilBitcode.push_back(0);
  }

  IFTBOOL(DxilBitcode.size_in_bytes() < UINT_MAX &&
              (DxilBitcode.size_in_bytes() & 0xF) == 0,
          DXC_E_DATA_TOO_LARGE);
}

} // namespace hlsl

HRESULT CreateDxbcConverter(REFIID riid, LPVOID *ppv) {
  try {
    CComPtr<hlsl::DxbcConverter> result(
        hlsl::DxbcConverter::Alloc(DxcGetThreadMallocNoRef()));
    IFROOM(result.p);
    return result.p->QueryInterface(riid, ppv);
  }
  CATCH_CPP_RETURN_HRESULT();
}
