///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxclibrary.cpp                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Compiler Library helper.                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/microcom.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"

#include "dxc/DXIL/DxilPDB.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxctools.h"

#include <unordered_set>
#include <vector>

using namespace llvm;
using namespace hlsl;

// Temporary: Define these here until a better header location is found.
namespace hlsl {
HRESULT CreateDxilShaderOrLibraryReflectionFromProgramHeader(
    const DxilProgramHeader *pProgramHeader, const DxilPartHeader *pRDATPart,
    REFIID iid, void **ppvObject);
HRESULT CreateDxilShaderOrLibraryReflectionFromModulePart(
    const DxilPartHeader *pModulePart, const DxilPartHeader *pRDATPart,
    REFIID iid, void **ppvObject);
} // namespace hlsl

class DxcIncludeHandlerForFS : public IDxcIncludeHandler {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcIncludeHandlerForFS)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  HRESULT STDMETHODCALLTYPE LoadSource(
      LPCWSTR pFilename,         // Candidate filename.
      IDxcBlob **ppIncludeSource // Resultant source object for included file,
                                 // nullptr if not found.
      ) override {
    try {
      CComPtr<IDxcBlobEncoding> pEncoding;
      HRESULT hr = ::hlsl::DxcCreateBlobFromFile(m_pMalloc, pFilename, nullptr,
                                                 &pEncoding);
      if (SUCCEEDED(hr)) {
        *ppIncludeSource = pEncoding.Detach();
      }
      return hr;
    }
    CATCH_CPP_RETURN_HRESULT();
  }
};

class DxcCompilerArgs : public IDxcCompilerArgs {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  std::unordered_set<std::wstring> m_Strings;
  std::vector<LPCWSTR> m_Arguments;

  LPCWSTR AddArgument(LPCWSTR pArg) {
    auto it = m_Strings.insert(pArg);
    LPCWSTR pInternalVersion = (it.first)->c_str();
    m_Arguments.push_back(pInternalVersion);
    return pInternalVersion;
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcCompilerArgs)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcCompilerArgs>(this, iid, ppvObject);
  }

  // Pass GetArguments() and GetCount() to Compile
  LPCWSTR *STDMETHODCALLTYPE GetArguments() override {
    return m_Arguments.data();
  }
  UINT32 STDMETHODCALLTYPE GetCount() override {
    return static_cast<UINT32>(m_Arguments.size());
  }

  // Add additional arguments or defines here, if desired.
  HRESULT STDMETHODCALLTYPE AddArguments(
      LPCWSTR *pArguments, // Array of pointers to arguments to add
      UINT32 argCount      // Number of arguments to add
      ) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      for (UINT32 i = 0; i < argCount; ++i) {
        AddArgument(pArguments[i]);
      }
      return S_OK;
    }
    CATCH_CPP_RETURN_HRESULT();
  }
  HRESULT STDMETHODCALLTYPE AddArgumentsUTF8(
      LPCSTR *pArguments, // Array of pointers to UTF-8 arguments to add
      UINT32 argCount     // Number of arguments to add
      ) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      for (UINT32 i = 0; i < argCount; ++i) {
        AddArgument(CA2W(pArguments[i]));
      }
      return S_OK;
    }
    CATCH_CPP_RETURN_HRESULT();
  }
  HRESULT STDMETHODCALLTYPE AddDefines(
      const DxcDefine *pDefines, // Array of defines
      UINT32 defineCount         // Number of defines
      ) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      for (UINT32 i = 0; i < defineCount; ++i) {
        LPCWSTR Name = pDefines[i].Name;
        LPCWSTR Value = pDefines[i].Value;
        AddArgument(L"-D");
        if (!Value) {
          // L"-D", L"<name>"
          AddArgument(Name);
          continue;
        }
        // L"-D", L"<name>=<value>"
        std::wstring defineArg;
        size_t size = 2 + wcslen(Name) + wcslen(Value);
        defineArg.reserve(size);
        defineArg = Name;
        defineArg += L"=";
        defineArg += pDefines[i].Value;
        AddArgument(defineArg.c_str());
      }
      return S_OK;
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  // This is used by BuildArguments to skip extra entry/profile arguments in the
  // arg list when already specified separatly.  This would lead to duplicate or
  // even contradictory arguments in the arg list, visible in debug information.
  HRESULT AddArgumentsOptionallySkippingEntryAndTarget(
      LPCWSTR *pArguments, // Array of pointers to arguments to add
      UINT32 argCount,     // Number of arguments to add
      bool skipEntry, bool skipTarget) {
    DxcThreadMalloc TM(m_pMalloc);
    bool skipNext = false;
    for (UINT32 i = 0; i < argCount; ++i) {
      if (skipNext) {
        skipNext = false;
        continue;
      }
      if (skipEntry || skipTarget) {
        LPCWSTR arg = pArguments[i];
        UINT size = wcslen(arg);
        if (size >= 2) {
          if (arg[0] == L'-' || arg[0] == L'/') {
            if ((skipEntry && arg[1] == L'E') ||
                (skipTarget && arg[1] == L'T')) {
              skipNext = size == 2; // skip next if value not joined
              continue;
            }
          }
        }
      }
      AddArgument(pArguments[i]);
    }
    return S_OK;
  }
};

class DxcUtils;

// DxcLibrary just wraps DxcUtils now.
class DxcLibrary : public IDxcLibrary {
private:
  DxcUtils &self;

public:
  DxcLibrary(DxcUtils &impl) : self(impl) {}
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override;
  HRESULT STDMETHODCALLTYPE SetMalloc(IMalloc *pMalloc) override;
  HRESULT STDMETHODCALLTYPE CreateBlobFromBlob(IDxcBlob *pBlob, UINT32 offset,
                                               UINT32 length,
                                               IDxcBlob **ppResult) override;
  HRESULT STDMETHODCALLTYPE
  CreateBlobFromFile(LPCWSTR pFileName, UINT32 *pCodePage,
                     IDxcBlobEncoding **pBlobEncoding) override;
  HRESULT STDMETHODCALLTYPE
  CreateBlobWithEncodingFromPinned(LPCVOID pText, UINT32 size, UINT32 codePage,
                                   IDxcBlobEncoding **pBlobEncoding) override;
  HRESULT STDMETHODCALLTYPE
  CreateBlobWithEncodingOnHeapCopy(LPCVOID pText, UINT32 size, UINT32 codePage,
                                   IDxcBlobEncoding **pBlobEncoding) override;
  HRESULT STDMETHODCALLTYPE CreateBlobWithEncodingOnMalloc(
      LPCVOID pText, IMalloc *pIMalloc, UINT32 size, UINT32 codePage,
      IDxcBlobEncoding **pBlobEncoding) override;
  HRESULT STDMETHODCALLTYPE
  CreateIncludeHandler(IDxcIncludeHandler **ppResult) override;
  HRESULT STDMETHODCALLTYPE
  CreateStreamFromBlobReadOnly(IDxcBlob *pBlob, IStream **ppStream) override;
  HRESULT STDMETHODCALLTYPE
  GetBlobAsUtf8(IDxcBlob *pBlob, IDxcBlobEncoding **pBlobEncoding) override;
  HRESULT STDMETHODCALLTYPE
  GetBlobAsWide(IDxcBlob *pBlob, IDxcBlobEncoding **pBlobEncoding) override;
};

class DxcUtils : public IDxcUtils {
  friend class DxcLibrary;

private:
  DXC_MICROCOM_TM_REF_FIELDS()
  DxcLibrary m_Library;

public:
  DxcUtils(IMalloc *pMalloc)
      : m_dwRef(0), m_pMalloc(pMalloc), m_Library(*this) {}
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcUtils)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    HRESULT hr = DoBasicQueryInterface<IDxcUtils>(this, iid, ppvObject);
    if (FAILED(hr)) {
      return DoBasicQueryInterface<IDxcLibrary>(&m_Library, iid, ppvObject);
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE CreateBlobFromBlob(IDxcBlob *pBlob, UINT32 offset,
                                               UINT32 length,
                                               IDxcBlob **ppResult) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      return ::hlsl::DxcCreateBlobFromBlob(pBlob, offset, length, ppResult);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  HRESULT STDMETHODCALLTYPE
  CreateBlobFromPinned(LPCVOID pData, UINT32 size, UINT32 codePage,
                       IDxcBlobEncoding **pBlobEncoding) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      return ::hlsl::DxcCreateBlobWithEncodingFromPinned(pData, size, codePage,
                                                         pBlobEncoding);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE
  MoveToBlob(LPCVOID pData, IMalloc *pIMalloc, UINT32 size, UINT32 codePage,
             IDxcBlobEncoding **pBlobEncoding) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      return ::hlsl::DxcCreateBlobWithEncodingOnMalloc(pData, pIMalloc, size,
                                                       codePage, pBlobEncoding);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE
  CreateBlob(LPCVOID pData, UINT32 size, UINT32 codePage,
             IDxcBlobEncoding **pBlobEncoding) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      return ::hlsl::DxcCreateBlobWithEncodingOnHeapCopy(pData, size, codePage,
                                                         pBlobEncoding);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE
  LoadFile(LPCWSTR pFileName, UINT32 *pCodePage,
           IDxcBlobEncoding **pBlobEncoding) override {
    DxcThreadMalloc TM(m_pMalloc);
    return ::hlsl::DxcCreateBlobFromFile(pFileName, pCodePage, pBlobEncoding);
  }

  HRESULT STDMETHODCALLTYPE
  CreateReadOnlyStreamFromBlob(IDxcBlob *pBlob, IStream **ppStream) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      return ::hlsl::CreateReadOnlyBlobStream(pBlob, ppStream);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE
  CreateDefaultIncludeHandler(IDxcIncludeHandler **ppResult) override {
    DxcThreadMalloc TM(m_pMalloc);
    CComPtr<DxcIncludeHandlerForFS> result;
    result = DxcIncludeHandlerForFS::Alloc(m_pMalloc);
    if (result.p == nullptr) {
      return E_OUTOFMEMORY;
    }
    *ppResult = result.Detach();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetBlobAsUtf8(IDxcBlob *pBlob, IDxcBlobUtf8 **pBlobEncoding) override {
    DxcThreadMalloc TM(m_pMalloc);
    return ::hlsl::DxcGetBlobAsUtf8(pBlob, m_pMalloc, pBlobEncoding);
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetBlobAsWide(IDxcBlob *pBlob, IDxcBlobWide **pBlobEncoding) override {
    DxcThreadMalloc TM(m_pMalloc);
    try {
      return ::hlsl::DxcGetBlobAsWide(pBlob, m_pMalloc, pBlobEncoding);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetDxilContainerPart(const DxcBuffer *pShader, UINT32 DxcPart,
                       void **ppPartData, UINT32 *pPartSizeInBytes) override {
    if (!ppPartData || !pPartSizeInBytes)
      return E_INVALIDARG;

    const DxilContainerHeader *pHeader =
        IsDxilContainerLike(pShader->Ptr, pShader->Size);
    if (!pHeader)
      return DXC_E_CONTAINER_INVALID;
    if (!IsValidDxilContainer(pHeader, pShader->Size))
      return DXC_E_CONTAINER_INVALID;
    DxilPartIterator it =
        std::find_if(begin(pHeader), end(pHeader), DxilPartIsType(DxcPart));
    if (it == end(pHeader))
      return DXC_E_MISSING_PART;

    *ppPartData = (void *)GetDxilPartData(*it);
    *pPartSizeInBytes = (*it)->PartSize;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE CreateReflection(
      const DxcBuffer *pData, REFIID iid, void **ppvReflection) override {
    if (!pData || !pData->Ptr || pData->Size < 8 ||
        pData->Encoding != DXC_CP_ACP || !ppvReflection)
      return E_INVALIDARG;

    DxcThreadMalloc TM(m_pMalloc);
    try {
      CComPtr<IDxcBlob> pPdbContainerBlob;
      const DxilPartHeader *pModulePart = nullptr;
      const DxilPartHeader *pRDATPart = nullptr;

      const DxilContainerHeader *pHeader =
          IsDxilContainerLike(pData->Ptr, pData->Size);
      if (!pHeader) {
        CComPtr<IDxcBlobEncoding> pBlob;
        IFR(hlsl::DxcCreateBlobWithEncodingFromPinned(pData->Ptr, pData->Size,
                                                      pData->Size, &pBlob));
        CComPtr<IStream> pStream;
        IFR(hlsl::CreateReadOnlyBlobStream(pBlob, &pStream));
        if (SUCCEEDED(hlsl::pdb::LoadDataFromStream(m_pMalloc, pStream,
                                                    &pPdbContainerBlob))) {
          pHeader = IsDxilContainerLike(pPdbContainerBlob->GetBufferPointer(),
                                        pPdbContainerBlob->GetBufferSize());
        }
      }

      // Is this a valid DxilContainer?
      if (pHeader) {
        if (!IsValidDxilContainer(pHeader, pData->Size))
          return E_INVALIDARG;

        const DxilPartHeader *pDXILPart = nullptr;
        const DxilPartHeader *pDebugDXILPart = nullptr;
        const DxilPartHeader *pStatsPart = nullptr;
        for (DxilPartIterator it = begin(pHeader), E = end(pHeader); it != E;
             ++it) {
          const DxilPartHeader *pPart = *it;
          switch (pPart->PartFourCC) {
          case DFCC_DXIL:
            IFRBOOL(!pDXILPart, DXC_E_DUPLICATE_PART); // Should only be one
            pDXILPart = pPart;
            break;
          case DFCC_ShaderDebugInfoDXIL:
            IFRBOOL(!pDebugDXILPart,
                    DXC_E_DUPLICATE_PART); // Should only be one
            pDebugDXILPart = pPart;
            break;
          case DFCC_ShaderStatistics:
            IFRBOOL(!pStatsPart, DXC_E_DUPLICATE_PART); // Should only be one
            pStatsPart = pPart;
            break;
          case DFCC_RuntimeData:
            IFRBOOL(!pRDATPart, DXC_E_DUPLICATE_PART); // Should only be one
            pRDATPart = pPart;
            break;
          }
        }

        // For now, pStatsPart contains module without function bodies for
        // reflection. If not found, fall back to DXIL part.
        pModulePart = pStatsPart       ? pStatsPart
                      : pDebugDXILPart ? pDebugDXILPart
                                       : pDXILPart;
        if (nullptr == pModulePart)
          return DXC_E_MISSING_PART;
      } else if (hlsl::IsValidDxilProgramHeader(
                     (const hlsl::DxilProgramHeader *)pData->Ptr,
                     pData->Size)) {

        return hlsl::CreateDxilShaderOrLibraryReflectionFromProgramHeader(
            (const hlsl::DxilProgramHeader *)pData->Ptr, pRDATPart, iid,
            ppvReflection);

      } else {
        // Not a container, try a statistics part that holds a valid program
        // part. In the future, this will just be the RDAT part.
        const DxilPartHeader *pPart =
            reinterpret_cast<const DxilPartHeader *>(pData->Ptr);
        if (pPart->PartSize < sizeof(DxilProgramHeader) ||
            pPart->PartSize + sizeof(DxilPartHeader) > pData->Size)
          return E_INVALIDARG;
        if (pPart->PartFourCC != DFCC_ShaderStatistics)
          return E_INVALIDARG;
        pModulePart = pPart;
        UINT32 SizeRemaining =
            pData->Size - (sizeof(DxilPartHeader) + pPart->PartSize);
        if (SizeRemaining > sizeof(DxilPartHeader)) {
          // Looks like we also have an RDAT part
          pPart = (const DxilPartHeader *)(GetDxilPartData(pPart) +
                                           pPart->PartSize);
          if (pPart->PartSize < /*sizeof(RuntimeDataHeader)*/ 8 ||
              pPart->PartSize + sizeof(DxilPartHeader) > SizeRemaining)
            return E_INVALIDARG;
          if (pPart->PartFourCC != DFCC_RuntimeData)
            return E_INVALIDARG;
          pRDATPart = pPart;
        }
      }

      return hlsl::CreateDxilShaderOrLibraryReflectionFromModulePart(
          pModulePart, pRDATPart, iid, ppvReflection);
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE BuildArguments(
      LPCWSTR pSourceName, // Optional file name for pSource. Used in errors and
                           // include handlers.
      LPCWSTR pEntryPoint, // Entry point name. (-E)
      LPCWSTR pTargetProfile,    // Shader profile to compile. (-T)
      LPCWSTR *pArguments,       // Array of pointers to arguments
      UINT32 NumArguments,       // Number of arguments
      const DxcDefine *pDefines, // Array of defines
      UINT32 NumDefines,         // Number of defines
      IDxcCompilerArgs **ppArgs  // Arguments you can use with Compile() method
      ) override {
    DxcThreadMalloc TM(m_pMalloc);

    try {
      CComPtr<DxcCompilerArgs> pArgs = DxcCompilerArgs::Alloc(m_pMalloc);
      if (!pArgs)
        return E_OUTOFMEMORY;

      if (pSourceName) {
        IFR(pArgs->AddArguments(&pSourceName, 1));
      }
      if (pEntryPoint) {
        if (wcslen(pEntryPoint)) {
          LPCWSTR args[] = {L"-E", pEntryPoint};
          IFR(pArgs->AddArguments(args, _countof(args)));
        } else {
          pEntryPoint = nullptr;
        }
      }
      if (pTargetProfile) {
        if (wcslen(pTargetProfile)) {
          LPCWSTR args[] = {L"-T", pTargetProfile};
          IFR(pArgs->AddArguments(args, _countof(args)));
        } else {
          pTargetProfile = nullptr;
        }
      }
      if (pArguments && NumArguments) {
        IFR(pArgs->AddArgumentsOptionallySkippingEntryAndTarget(
            pArguments, NumArguments, !!pEntryPoint, !!pTargetProfile));
      }
      if (pDefines && NumDefines) {
        IFR(pArgs->AddDefines(pDefines, NumDefines));
      }

      *ppArgs = pArgs.Detach();
      return S_OK;
    }
    CATCH_CPP_RETURN_HRESULT();
  }

  virtual HRESULT STDMETHODCALLTYPE GetPDBContents(
      IDxcBlob *pPDBBlob, IDxcBlob **ppHash, IDxcBlob **ppContainer) override {
    DxcThreadMalloc TM(m_pMalloc);

    try {
      CComPtr<IStream> pStream;
      IFR(hlsl::CreateReadOnlyBlobStream(pPDBBlob, &pStream));
      IFR(hlsl::pdb::LoadDataFromStream(m_pMalloc, pStream, ppHash,
                                        ppContainer));
      return S_OK;
    }
    CATCH_CPP_RETURN_HRESULT();
  }
};

//////////////////////////////////////////////////////////////
// legacy DxcLibrary implementation that maps to DxcCompiler
ULONG STDMETHODCALLTYPE DxcLibrary::AddRef() { return self.AddRef(); }
ULONG STDMETHODCALLTYPE DxcLibrary::Release() { return self.Release(); }
HRESULT STDMETHODCALLTYPE DxcLibrary::QueryInterface(REFIID iid,
                                                     void **ppvObject) {
  return self.QueryInterface(iid, ppvObject);
}

HRESULT STDMETHODCALLTYPE DxcLibrary::SetMalloc(IMalloc *pMalloc) {
  return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DxcLibrary::CreateBlobFromBlob(IDxcBlob *pBlob,
                                                         UINT32 offset,
                                                         UINT32 length,
                                                         IDxcBlob **ppResult) {
  return self.CreateBlobFromBlob(pBlob, offset, length, ppResult);
}

HRESULT STDMETHODCALLTYPE DxcLibrary::CreateBlobFromFile(
    LPCWSTR pFileName, UINT32 *pCodePage, IDxcBlobEncoding **pBlobEncoding) {
  return self.LoadFile(pFileName, pCodePage, pBlobEncoding);
}

HRESULT STDMETHODCALLTYPE DxcLibrary::CreateBlobWithEncodingFromPinned(
    LPCVOID pText, UINT32 size, UINT32 codePage,
    IDxcBlobEncoding **pBlobEncoding) {
  return self.CreateBlobFromPinned(pText, size, codePage, pBlobEncoding);
}

HRESULT STDMETHODCALLTYPE DxcLibrary::CreateBlobWithEncodingOnHeapCopy(
    LPCVOID pText, UINT32 size, UINT32 codePage,
    IDxcBlobEncoding **pBlobEncoding) {
  return self.CreateBlob(pText, size, codePage, pBlobEncoding);
}

HRESULT STDMETHODCALLTYPE DxcLibrary::CreateBlobWithEncodingOnMalloc(
    LPCVOID pText, IMalloc *pIMalloc, UINT32 size, UINT32 codePage,
    IDxcBlobEncoding **pBlobEncoding) {
  return self.MoveToBlob(pText, pIMalloc, size, codePage, pBlobEncoding);
}

HRESULT STDMETHODCALLTYPE
DxcLibrary::CreateIncludeHandler(IDxcIncludeHandler **ppResult) {
  return self.CreateDefaultIncludeHandler(ppResult);
}

HRESULT STDMETHODCALLTYPE
DxcLibrary::CreateStreamFromBlobReadOnly(IDxcBlob *pBlob, IStream **ppStream) {
  return self.CreateReadOnlyStreamFromBlob(pBlob, ppStream);
}

HRESULT STDMETHODCALLTYPE
DxcLibrary::GetBlobAsUtf8(IDxcBlob *pBlob, IDxcBlobEncoding **pBlobEncoding) {
  CComPtr<IDxcBlobUtf8> pBlobUtf8;
  IFR(self.GetBlobAsUtf8(pBlob, &pBlobUtf8));
  IFR(pBlobUtf8->QueryInterface(pBlobEncoding));
  return S_OK;
}

HRESULT STDMETHODCALLTYPE
DxcLibrary::GetBlobAsWide(IDxcBlob *pBlob, IDxcBlobEncoding **pBlobEncoding) {
  CComPtr<IDxcBlobWide> pBlobUtf16;
  IFR(self.GetBlobAsWide(pBlob, &pBlobUtf16));
  IFR(pBlobUtf16->QueryInterface(pBlobEncoding));
  return S_OK;
}

HRESULT CreateDxcCompilerArgs(REFIID riid, LPVOID *ppv) {
  CComPtr<DxcCompilerArgs> result =
      DxcCompilerArgs::Alloc(DxcGetThreadMallocNoRef());
  if (result == nullptr) {
    *ppv = nullptr;
    return E_OUTOFMEMORY;
  }

  return result.p->QueryInterface(riid, ppv);
}

HRESULT CreateDxcUtils(REFIID riid, LPVOID *ppv) {
  CComPtr<DxcUtils> result = DxcUtils::Alloc(DxcGetThreadMallocNoRef());
  if (result == nullptr) {
    *ppv = nullptr;
    return E_OUTOFMEMORY;
  }

  return result.p->QueryInterface(riid, ppv);
}
