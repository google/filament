///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcontainerbuilder.cpp                                                    //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the Dxil Container Builder                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxcContainerBuilder.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/ErrorCodes.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"

#include "llvm/ADT/SmallVector.h"
#include <algorithm>

// This declaration is used for the locally-linked validator.
HRESULT CreateDxcValidator(REFIID riid, LPVOID *ppv);
template <class TInterface>
HRESULT DxilLibCreateInstance(REFCLSID rclsid, TInterface **ppInterface);

using namespace hlsl;

HRESULT STDMETHODCALLTYPE DxcContainerBuilder::Load(IDxcBlob *pSource) {
  DxcThreadMalloc TM(m_pMalloc);
  try {
    IFTBOOL(m_pContainer == nullptr && pSource != nullptr &&
                IsDxilContainerLike(pSource->GetBufferPointer(),
                                    pSource->GetBufferSize()),
            E_INVALIDARG);
    m_pContainer = pSource;
    const DxilContainerHeader *pHeader =
        (DxilContainerHeader *)pSource->GetBufferPointer();
    for (DxilPartIterator it = begin(pHeader), itEnd = end(pHeader);
         it != itEnd; ++it) {
      const DxilPartHeader *pPartHeader = *it;
      CComPtr<IDxcBlob> pBlob;
      IFT(DxcCreateBlobFromPinned((const void *)(pPartHeader + 1),
                                  pPartHeader->PartSize, &pBlob));
      AddPart(DxilPart(pPartHeader->PartFourCC, pBlob));
    }
    // Collect hash function.
    const DxilContainerHeader *Header =
        (DxilContainerHeader *)pSource->GetBufferPointer();
    DetermineHashFunctionFromContainerContents(Header);
    return S_OK;
  }
  CATCH_CPP_RETURN_HRESULT();
}

HRESULT STDMETHODCALLTYPE DxcContainerBuilder::AddPart(UINT32 fourCC,
                                                       IDxcBlob *pSource) {
  DxcThreadMalloc TM(m_pMalloc);
  try {
    IFTBOOL(pSource != nullptr &&
                !IsDxilContainerLike(pSource->GetBufferPointer(),
                                     pSource->GetBufferSize()),
            E_INVALIDARG);
    // You can only add debug info, debug info name, rootsignature, or private
    // data blob
    IFTBOOL(fourCC == DxilFourCC::DFCC_ShaderDebugInfoDXIL ||
                fourCC == DxilFourCC::DFCC_ShaderDebugName ||
                fourCC == DxilFourCC::DFCC_RootSignature ||
                fourCC == DxilFourCC::DFCC_ShaderStatistics ||
                fourCC == DxilFourCC::DFCC_PrivateData,
            E_INVALIDARG);
    AddPart(DxilPart(fourCC, pSource));
    if (fourCC == DxilFourCC::DFCC_RootSignature) {
      m_RequireValidation = true;
    }
    return S_OK;
  }
  CATCH_CPP_RETURN_HRESULT();
}

HRESULT STDMETHODCALLTYPE DxcContainerBuilder::RemovePart(UINT32 fourCC) {
  DxcThreadMalloc TM(m_pMalloc);
  try {
    IFTBOOL(fourCC == DxilFourCC::DFCC_ShaderDebugInfoDXIL ||
                fourCC == DxilFourCC::DFCC_ShaderDebugName ||
                fourCC == DxilFourCC::DFCC_RootSignature ||
                fourCC == DxilFourCC::DFCC_PrivateData ||
                fourCC == DxilFourCC::DFCC_ShaderStatistics,
            E_INVALIDARG); // You can only remove debug info, debug info name,
                           // rootsignature, or private data blob
    PartList::iterator it =
        std::find_if(m_parts.begin(), m_parts.end(),
                     [&](DxilPart part) { return part.m_fourCC == fourCC; });
    IFTBOOL(it != m_parts.end(), DXC_E_MISSING_PART);
    m_parts.erase(it);
    if (fourCC == DxilFourCC::DFCC_PrivateData) {
      m_HasPrivateData = false;
    }
    return S_OK;
  }
  CATCH_CPP_RETURN_HRESULT();
}

HRESULT STDMETHODCALLTYPE
DxcContainerBuilder::SerializeContainer(IDxcOperationResult **ppResult) {
  if (ppResult == nullptr)
    return E_INVALIDARG;

  DxcThreadMalloc TM(m_pMalloc);

  try {
    // Allocate memory for new dxil container.
    uint32_t ContainerSize = ComputeContainerSize();
    CComPtr<AbstractMemoryStream> pMemoryStream;
    CComPtr<IDxcBlob> pResult;
    IFT(CreateMemoryStream(m_pMalloc, &pMemoryStream));
    IFT(pMemoryStream->QueryInterface(&pResult));
    IFT(pMemoryStream->Reserve(ContainerSize))

    // Update Dxil Container
    IFT(UpdateContainerHeader(pMemoryStream, ContainerSize));

    // Update offset Table
    IFT(UpdateOffsetTable(pMemoryStream));

    // Update Parts
    IFT(UpdateParts(pMemoryStream));

    CComPtr<IDxcBlobUtf8> pValErrorUtf8;
    HRESULT valHR = S_OK;
    if (m_RequireValidation) {
      CComPtr<IDxcValidator> pValidator;
      IFT(CreateDxcValidator(IID_PPV_ARGS(&pValidator)));
      CComPtr<IDxcOperationResult> pValidationResult;
      IFT(pValidator->Validate(pResult, DxcValidatorFlags_RootSignatureOnly,
                               &pValidationResult));
      IFT(pValidationResult->GetStatus(&valHR));
      if (FAILED(valHR)) {
        CComPtr<IDxcBlobEncoding> pValError;
        IFT(pValidationResult->GetErrorBuffer(&pValError));
        if (pValError->GetBufferPointer() && pValError->GetBufferSize())
          IFT(hlsl::DxcGetBlobAsUtf8(pValError, m_pMalloc, &pValErrorUtf8));
      }
    }
    // Combine existing warnings and errors from validation
    CComPtr<IDxcBlobEncoding> pErrorBlob;
    CDxcMallocHeapPtr<char> errorHeap(m_pMalloc);
    SIZE_T totalErrorLength =
        pValErrorUtf8 ? pValErrorUtf8->GetStringLength() : 0;
    if (totalErrorLength) {
      SIZE_T errorSizeInBytes = totalErrorLength + 1;
      errorHeap.AllocateBytes(errorSizeInBytes);

      memcpy(errorHeap.m_pData, pValErrorUtf8->GetStringPointer(),
             totalErrorLength);
      errorHeap.m_pData[totalErrorLength] = L'\0';
      IFT(hlsl::DxcCreateBlobWithEncodingOnMalloc(errorHeap.m_pData, m_pMalloc,
                                                  errorSizeInBytes, DXC_CP_UTF8,
                                                  &pErrorBlob));
      errorHeap.Detach();
    }

    // Add Hash.
    if (SUCCEEDED(valHR))
      HashAndUpdate(IsDxilContainerLike(pResult->GetBufferPointer(),
                                        pResult->GetBufferSize()));

    IFT(DxcResult::Create(
        valHR, DXC_OUT_OBJECT,
        {DxcOutputObject::DataOutput(DXC_OUT_OBJECT, pResult, DxcOutNoName),
         DxcOutputObject::DataOutput(DXC_OUT_ERRORS, pErrorBlob, DxcOutNoName)},
        ppResult));
  }
  CATCH_CPP_RETURN_HRESULT();

  return S_OK;
}

// Try hashing the source contained in ContainerHeader using retail and debug
// hashing functions. If either of them match the stored result, set the
// HashFunction to the matching variant. If neither match, set it to null.
void DxcContainerBuilder::DetermineHashFunctionFromContainerContents(
    const DxilContainerHeader *ContainerHeader) {
  DXASSERT(ContainerHeader != nullptr &&
               IsDxilContainerLike(ContainerHeader,
                                   ContainerHeader->ContainerSizeInBytes),
           "otherwise load function should have returned an error.");
  constexpr uint32_t HashStartOffset =
      offsetof(struct DxilContainerHeader, Version);
  auto *DataToHash = (const BYTE *)ContainerHeader + HashStartOffset;
  UINT AmountToHash = ContainerHeader->ContainerSizeInBytes - HashStartOffset;
  BYTE Result[DxilContainerHashSize];
  ComputeHashRetail(DataToHash, AmountToHash, Result);
  if (0 == memcmp(Result, ContainerHeader->Hash.Digest, sizeof(Result))) {
    m_HashFunction = ComputeHashRetail;
  } else {
    ComputeHashDebug(DataToHash, AmountToHash, Result);
    if (0 == memcmp(Result, ContainerHeader->Hash.Digest, sizeof(Result)))
      m_HashFunction = ComputeHashDebug;
    else
      m_HashFunction = nullptr;
  }
}

// For Internal hash function.
void DxcContainerBuilder::HashAndUpdate(DxilContainerHeader *ContainerHeader) {
  if (m_HashFunction != nullptr) {
    DXASSERT(ContainerHeader != nullptr,
             "Otherwise serialization should have failed.");
    static const UINT32 HashStartOffset =
        offsetof(struct DxilContainerHeader, Version);
    const BYTE *DataToHash = (const BYTE *)ContainerHeader + HashStartOffset;
    UINT AmountToHash = ContainerHeader->ContainerSizeInBytes - HashStartOffset;
    m_HashFunction(DataToHash, AmountToHash, ContainerHeader->Hash.Digest);
  }
}

UINT32 DxcContainerBuilder::ComputeContainerSize() {
  UINT32 partsSize = 0;
  for (DxilPart part : m_parts) {
    partsSize += part.m_Blob->GetBufferSize();
  }
  return GetDxilContainerSizeFromParts(m_parts.size(), partsSize);
}

HRESULT
DxcContainerBuilder::UpdateContainerHeader(AbstractMemoryStream *pStream,
                                           uint32_t containerSize) {
  DxilContainerHeader header;
  InitDxilContainer(&header, m_parts.size(), containerSize);
  ULONG cbWritten;
  IFR(pStream->Write(&header, sizeof(DxilContainerHeader), &cbWritten));
  if (cbWritten != sizeof(DxilContainerHeader)) {
    return E_FAIL;
  }
  return S_OK;
}

HRESULT DxcContainerBuilder::UpdateOffsetTable(AbstractMemoryStream *pStream) {
  UINT32 offset =
      sizeof(DxilContainerHeader) + GetOffsetTableSize(m_parts.size());
  for (size_t i = 0; i < m_parts.size(); ++i) {
    ULONG cbWritten;
    IFR(pStream->Write(&offset, sizeof(UINT32), &cbWritten));
    if (cbWritten != sizeof(UINT32)) {
      return E_FAIL;
    }
    offset += sizeof(DxilPartHeader) + m_parts[i].m_Blob->GetBufferSize();
  }
  return S_OK;
}

HRESULT DxcContainerBuilder::UpdateParts(AbstractMemoryStream *pStream) {
  for (size_t i = 0; i < m_parts.size(); ++i) {
    ULONG cbWritten;
    CComPtr<IDxcBlob> pBlob = m_parts[i].m_Blob;
    // Write part header
    DxilPartHeader partHeader = {m_parts[i].m_fourCC,
                                 (uint32_t)pBlob->GetBufferSize()};
    IFR(pStream->Write(&partHeader, sizeof(DxilPartHeader), &cbWritten));
    if (cbWritten != sizeof(DxilPartHeader)) {
      return E_FAIL;
    }
    // Write part content
    IFR(pStream->Write(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
                       &cbWritten));
    if (cbWritten != pBlob->GetBufferSize()) {
      return E_FAIL;
    }
  }
  return S_OK;
}

void DxcContainerBuilder::AddPart(DxilPart &&part) {
  PartList::iterator it =
      std::find_if(m_parts.begin(), m_parts.end(), [&](DxilPart checkPart) {
        return checkPart.m_fourCC == part.m_fourCC;
      });
  IFTBOOL(it == m_parts.end(), DXC_E_DUPLICATE_PART);
  if (m_HasPrivateData) {
    // Keep PrivateData at end, since it may have unaligned size.
    m_parts.insert(m_parts.end() - 1, std::move(part));
  } else {
    m_parts.emplace_back(std::move(part));
  }
  if (part.m_fourCC == DxilFourCC::DFCC_PrivateData) {
    m_HasPrivateData = true;
  }
}
