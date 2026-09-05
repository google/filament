///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxilpdbutils.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements IDxcPdbUtils interface, which allows getting basic stuff from  //
// shader PDBS.                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.use.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "dxc/DXIL/DxilMetadataHelper.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilPDB.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Path.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"

#include "dxc/Support/dxcfilesystem.h"
#include "dxcshadersourceinfo.h"

#include "dxc/DxilCompression/DxilCompression.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <vector>

#ifdef _WIN32
#include "dxc/dxcpix.h"
#include <dia2.h>
#endif

using namespace dxc;
using namespace llvm;

struct DxcPdbVersionInfo : public IDxcVersionInfo2, public IDxcVersionInfo3 {
private:
  DXC_MICROCOM_TM_REF_FIELDS()

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPdbVersionInfo)

  DxcPdbVersionInfo(IMalloc *pMalloc) : m_dwRef(0), m_pMalloc(pMalloc) {}

  hlsl::DxilCompilerVersion m_Version = {};
  std::string m_VersionCommitSha = {};
  std::string m_VersionString = {};

  static HRESULT CopyStringToOutStringPtr(const std::string &Str,
                                          char **ppOutString) {
    *ppOutString = nullptr;
    char *const pString = (char *)CoTaskMemAlloc(Str.size() + 1);
    if (pString == nullptr)
      return E_OUTOFMEMORY;
    std::memcpy(pString, Str.data(), Str.size());
    pString[Str.size()] = 0;

    *ppOutString = pString;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcVersionInfo, IDxcVersionInfo2,
                                 IDxcVersionInfo3>(this, iid, ppvObject);
  }

  virtual HRESULT STDMETHODCALLTYPE GetVersion(UINT32 *pMajor,
                                               UINT32 *pMinor) override {
    if (!pMajor || !pMinor)
      return E_POINTER;
    *pMajor = m_Version.Major;
    *pMinor = m_Version.Minor;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetFlags(UINT32 *pFlags) override {
    if (!pFlags)
      return E_POINTER;
    *pFlags = m_Version.VersionFlags;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetCommitInfo(UINT32 *pCommitCount,
                                                  char **pCommitHash) override {
    if (!pCommitHash)
      return E_POINTER;

    IFR(CopyStringToOutStringPtr(m_VersionCommitSha, pCommitHash));
    *pCommitCount = m_Version.CommitCount;

    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetCustomVersionString(char **pVersionString) override {
    if (!pVersionString)
      return E_POINTER;
    IFR(CopyStringToOutStringPtr(m_VersionString, pVersionString));
    return S_OK;
  }
};

// Implement the legacy IDxcPdbUtils interface with an instance of
// the new impelmentation.
struct DxcPdbUtilsAdapter : public IDxcPdbUtils {
private:
  IDxcPdbUtils2 *m_pImpl;

  HRESULT CopyBlobWideToBSTR(IDxcBlobWide *pBlob, BSTR *pResult) {
    if (!pResult)
      return E_POINTER;
    *pResult = nullptr;
    if (pBlob) {
      CComBSTR pBstr(pBlob->GetStringLength(), pBlob->GetStringPointer());
      *pResult = pBstr.Detach();
    }
    return S_OK;
  }

public:
  DxcPdbUtilsAdapter(IDxcPdbUtils2 *pImpl) : m_pImpl(pImpl) {}
  ULONG STDMETHODCALLTYPE AddRef() override { return m_pImpl->AddRef(); }
  ULONG STDMETHODCALLTYPE Release() override { return m_pImpl->Release(); }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return m_pImpl->QueryInterface(iid, ppvObject);
  }

  HRESULT STDMETHODCALLTYPE Load(IDxcBlob *pPdbOrDxil) override {
    return m_pImpl->Load(pPdbOrDxil);
  }

  virtual HRESULT STDMETHODCALLTYPE GetSourceCount(UINT32 *pCount) override {
    return m_pImpl->GetSourceCount(pCount);
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetSource(UINT32 uIndex, IDxcBlobEncoding **ppResult) override {
    return m_pImpl->GetSource(uIndex, ppResult);
  }

  virtual HRESULT STDMETHODCALLTYPE GetSourceName(UINT32 uIndex,
                                                  BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetSourceName(uIndex, &pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }

  virtual HRESULT STDMETHODCALLTYPE GetFlagCount(UINT32 *pCount) override {
    return m_pImpl->GetFlagCount(pCount);
  }
  virtual HRESULT STDMETHODCALLTYPE GetFlag(UINT32 uIndex,
                                            BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetFlag(uIndex, &pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }
  virtual HRESULT STDMETHODCALLTYPE GetArgCount(UINT32 *pCount) override {
    return m_pImpl->GetArgCount(pCount);
  }
  virtual HRESULT STDMETHODCALLTYPE GetArg(UINT32 uIndex,
                                           BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetArg(uIndex, &pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }
  virtual HRESULT STDMETHODCALLTYPE GetDefineCount(UINT32 *pCount) override {
    return m_pImpl->GetDefineCount(pCount);
  }
  virtual HRESULT STDMETHODCALLTYPE GetDefine(UINT32 uIndex,
                                              BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetDefine(uIndex, &pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }

  virtual HRESULT STDMETHODCALLTYPE GetArgPairCount(UINT32 *pCount) override {
    return m_pImpl->GetArgPairCount(pCount);
  }

  virtual HRESULT STDMETHODCALLTYPE GetArgPair(UINT32 uIndex, BSTR *pName,
                                               BSTR *pValue) override {
    CComPtr<IDxcBlobWide> pNameBlob;
    CComPtr<IDxcBlobWide> pValueBlob;
    IFR(m_pImpl->GetArgPair(uIndex, &pNameBlob, &pValueBlob));
    IFR(CopyBlobWideToBSTR(pNameBlob, pName));
    IFR(CopyBlobWideToBSTR(pValueBlob, pValue));
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetTargetProfile(BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetTargetProfile(&pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }
  virtual HRESULT STDMETHODCALLTYPE GetEntryPoint(BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetEntryPoint(&pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }
  virtual HRESULT STDMETHODCALLTYPE GetMainFileName(BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetMainFileName(&pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }

  virtual BOOL STDMETHODCALLTYPE IsFullPDB() override {
    return m_pImpl->IsFullPDB();
  }

  virtual HRESULT STDMETHODCALLTYPE OverrideArgs(DxcArgPair *pArgPairs,
                                                 UINT32 uNumArgPairs) override {
    return E_NOTIMPL;
  }

  virtual HRESULT STDMETHODCALLTYPE
  OverrideRootSignature(const WCHAR *pRootSignature) override {
    return E_NOTIMPL;
  }

  virtual HRESULT STDMETHODCALLTYPE
  CompileForFullPDB(IDxcResult **ppResult) override {
    return E_NOTIMPL;
  }

  virtual HRESULT STDMETHODCALLTYPE GetFullPDB(IDxcBlob **ppFullPDB) override {
    return E_NOTIMPL;
  }

  virtual HRESULT STDMETHODCALLTYPE GetHash(IDxcBlob **ppResult) override {
    return m_pImpl->GetHash(ppResult);
  }

  virtual HRESULT STDMETHODCALLTYPE GetName(BSTR *pResult) override {
    CComPtr<IDxcBlobWide> pBlob;
    IFR(m_pImpl->GetName(&pBlob));
    return CopyBlobWideToBSTR(pBlob, pResult);
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetVersionInfo(IDxcVersionInfo **ppVersionInfo) override {
    return m_pImpl->GetVersionInfo(ppVersionInfo);
  }

  virtual HRESULT STDMETHODCALLTYPE
  SetCompiler(IDxcCompiler3 *pCompiler) override {
    return E_NOTIMPL;
  }
};

struct DxcPdbUtils : public IDxcPdbUtils2
#ifdef _WIN32
    // Skip Pix debug info on linux for dia dependence.
    ,
                     public IDxcPixDxilDebugInfoFactory
#endif
{
private:
  // Making the adapter and this interface the same object and share reference
  // counting.
  DxcPdbUtilsAdapter m_Adapter;

  DXC_MICROCOM_TM_REF_FIELDS()

  struct Source_File {
    CComPtr<IDxcBlobWide> Name;
    CComPtr<IDxcBlobEncoding> Content;
  };

  CComPtr<IDxcBlob> m_InputBlob;
  CComPtr<IDxcBlob> m_pDebugProgramBlob;
  CComPtr<IDxcBlob> m_ContainerBlob;
  std::vector<Source_File> m_SourceFiles;

  CComPtr<IDxcBlobWide> m_EntryPoint;
  CComPtr<IDxcBlobWide> m_TargetProfile;
  CComPtr<IDxcBlobWide> m_Name;
  CComPtr<IDxcBlobWide> m_MainFileName;
  CComPtr<IDxcBlob> m_HashBlob;
  CComPtr<IDxcBlob> m_WholeDxil;
  bool m_HasVersionInfo = false;
  hlsl::DxilCompilerVersion m_VersionInfo;
  std::string m_VersionCommitSha;
  std::string m_VersionString;
  CComPtr<IDxcResult> m_pCachedRecompileResult;

  // NOTE: This is not set to null by Reset() since it doesn't
  // necessarily change across different PDBs.
  CComPtr<IDxcCompiler3> m_pCompiler;

  struct ArgPair {
    CComPtr<IDxcBlobWide> Name;
    CComPtr<IDxcBlobWide> Value;
  };
  std::vector<ArgPair> m_ArgPairs;
  std::vector<CComPtr<IDxcBlobWide>> m_Defines;
  std::vector<CComPtr<IDxcBlobWide>> m_Args;
  std::vector<CComPtr<IDxcBlobWide>> m_Flags;

  struct LibraryEntry {
    std::vector<char> PdbInfo;
    CComPtr<IDxcBlobWide> pName;
  };

  std::vector<LibraryEntry> m_LibraryPdbs;
  // std::vector<CComPtr<IDxcBlobWide> > m_LibraryNames;
  UINT32 m_uCustomToolchainID = 0;
  CComPtr<IDxcBlob> m_customToolchainData;

  void ResetAllArgs() {
    m_ArgPairs.clear();
    m_Defines.clear();
    m_Args.clear();
    m_Flags.clear();
    m_EntryPoint = nullptr;
    m_TargetProfile = nullptr;
  }

  void Reset() {
    m_WholeDxil = 0;
    m_uCustomToolchainID = 0;
    m_pDebugProgramBlob = nullptr;
    m_InputBlob = nullptr;
    m_ContainerBlob = nullptr;
    m_SourceFiles.clear();
    m_Name = nullptr;
    m_MainFileName = nullptr;
    m_HashBlob = nullptr;
    m_HasVersionInfo = false;
    m_VersionInfo = {};
    m_VersionCommitSha.clear();
    m_VersionString.clear();
    m_pCachedRecompileResult = nullptr;
    m_LibraryPdbs.clear();
    m_customToolchainData = nullptr;
    ResetAllArgs();
  }

  HRESULT Utf8ToBlobWide(StringRef str, IDxcBlobWide **ppResult) {
    CComPtr<IDxcBlobEncoding> pUtf8Blob;
    IFR(hlsl::DxcCreateBlob(str.data(), str.size(),
                            /*bPinned*/ true, /*bCopy*/ false,
                            /*encodingKnown*/ true, CP_UTF8, m_pMalloc,
                            &pUtf8Blob));
    return hlsl::DxcGetBlobAsWide(pUtf8Blob, m_pMalloc, ppResult);
  }

  static HRESULT CopyBlobWide(IDxcBlobWide *pBlob, IDxcBlobWide **ppResult) {
    if (!ppResult)
      return E_POINTER;
    *ppResult = nullptr;

    if (!pBlob)
      return S_OK;

    return pBlob->QueryInterface(ppResult);
  }

  static bool IsBitcode(const void *ptr, size_t size) {
    const uint8_t pattern[] = {
        'B',
        'C',
    };
    if (size < _countof(pattern))
      return false;
    return !memcmp(ptr, pattern, _countof(pattern));
  }

  static std::vector<std::pair<std::string, std::string>>
  ComputeArgPairs(ArrayRef<const char *> args) {
    std::vector<std::pair<std::string, std::string>> ret;

    const llvm::opt::OptTable *optionTable = hlsl::options::getHlslOptTable();
    assert(optionTable);
    if (optionTable) {
      unsigned missingIndex = 0;
      unsigned missingCount = 0;
      llvm::opt::InputArgList argList =
          optionTable->ParseArgs(args, missingIndex, missingCount);
      for (llvm::opt::Arg *arg : argList) {
        std::pair<std::string, std::string> newPair;
        newPair.first = arg->getOption().getName();
        if (arg->getNumValues() > 0) {
          newPair.second = arg->getValue();
        }
        ret.push_back(std::move(newPair));
      }
    }
    return ret;
  }

  HRESULT AddSource(StringRef name, StringRef content) {
    Source_File source;
    IFR(hlsl::DxcCreateBlob(content.data(), content.size(),
                            /*bPinned*/ false, /*bCopy*/ true,
                            /*encodingKnown*/ true, CP_UTF8, m_pMalloc,
                            &source.Content));

    std::string normalizedPath = hlsl::NormalizePath(name);
    IFR(Utf8ToBlobWide(normalizedPath, &source.Name));
    // First file is the main file
    if (m_SourceFiles.empty()) {
      m_MainFileName = source.Name;
    }

    m_SourceFiles.push_back(std::move(source));
    return S_OK;
  }

  HRESULT LoadFromPDBInfoPart(const hlsl::DxilShaderPDBInfo *header,
                              uint32_t partSize) {
    if (header->Version > hlsl::DxilShaderPDBInfoVersion::Latest) {
      return E_FAIL;
    }

    hlsl::RDAT::DxilRuntimeData reader;

    SmallVector<char, 1024> UncompressedBuffer;
    const void *ptr = nullptr;
    size_t size = 0;
    if (header->CompressionType ==
        hlsl::DxilShaderPDBInfoCompressionType::Zlib) {
      UncompressedBuffer.resize(header->UncompressedSizeInBytes);
      if (hlsl::ZlibResult::Success !=
          hlsl::ZlibDecompress(DxcGetThreadMallocNoRef(), header + 1,
                               header->SizeInBytes, UncompressedBuffer.data(),
                               UncompressedBuffer.size())) {
        return E_FAIL;
      }
      ptr = UncompressedBuffer.data();
      size = UncompressedBuffer.size();
    } else if (header->CompressionType ==
               hlsl::DxilShaderPDBInfoCompressionType::Uncompressed) {
      assert(header->UncompressedSizeInBytes == header->SizeInBytes);
      ptr = header + 1;
      size = header->UncompressedSizeInBytes;
    } else {
      return E_FAIL;
    }

    if (!reader.InitFromRDAT(ptr, size)) {
      return E_FAIL;
    }

    auto pdbInfo = reader.GetDxilPdbInfoTable()[0];
    return LoadFromPdbInfoReader(pdbInfo);
  }

  HRESULT LoadFromPdbInfoReader(hlsl::RDAT::DxilPdbInfo_Reader &reader) {
    auto sources = reader.getSources();
    for (size_t i = 0; i < sources.Count(); i++) {
      const char *name = sources[i].getName();
      const char *content = sources[i].getContent();
      IFR(AddSource(name, content));
    }

    if (reader.sizeWholeDxil()) {
      assert(!m_WholeDxil);
      IFR(hlsl::DxcCreateBlobOnHeapCopy(reader.getWholeDxil(),
                                        reader.sizeWholeDxil(), &m_WholeDxil));
    }

    m_uCustomToolchainID = reader.getCustomToolchainId();
    if (size_t size = reader.sizeCustomToolchainData()) {
      IFR(hlsl::DxcCreateBlobOnHeapCopy(reader.getCustomToolchainData(), size,
                                        &m_customToolchainData));
    }

    auto libraries = reader.getLibraries();
    unsigned libCount = libraries.Count();
    for (size_t i = 0; i < libCount; i++) {
      auto libReader = reader.getLibraries()[i];
      if (libReader.sizeData() == 0)
        return E_FAIL;

      CComPtr<IDxcBlob> pLibraryPdb;
      CComPtr<IDxcBlobWide> pLibraryName;
      IFR(hlsl::DxcCreateBlobOnHeapCopy(libReader.getData(),
                                        libReader.sizeData(), &pLibraryPdb));
      IFR(Utf8ToBlobWide(libReader.getName(), &pLibraryName));

      LibraryEntry Entry;
      Entry.PdbInfo.assign((const char *)libReader.getData(),
                           (const char *)libReader.getData() +
                               libReader.sizeData());
      Entry.pName = pLibraryName;
      m_LibraryPdbs.push_back(std::move(Entry));
    }

    auto argPairs = reader.getArgPairs();
    for (size_t i = 0; i + 1 < argPairs.Count(); i += 2) {
      const char *name = argPairs[i + 0];
      const char *value = argPairs[i + 1];
      IFR(AddArgPair(name, value));
    }

    assert(!reader.sizeHash() ||
           reader.sizeHash() == sizeof(hlsl::DxilShaderHash));
    if (reader.sizeHash() == sizeof(hlsl::DxilShaderHash)) {
      hlsl::DxilShaderHash hash = {};
      memcpy(&hash, reader.getHash(), reader.sizeHash());
      if (!m_HashBlob) {
        IFR(hlsl::DxcCreateBlobOnHeapCopy(&hash, sizeof(hash), &m_HashBlob));
      } else {
        DXASSERT_NOMSG(
            m_HashBlob->GetBufferSize() == sizeof(hash) &&
            0 == memcmp(m_HashBlob->GetBufferPointer(), &hash, sizeof(hash)));
      }
    }

    if (!m_Name) {
      IFR(Utf8ToBlobWide(reader.getPdbName(), &m_Name));
    }

    // Entry point might have been omitted. Set it to main by default.
    if (!m_EntryPoint) {
      IFR(Utf8ToBlobWide("main", &m_EntryPoint));
    }

    return S_OK;
  }

  HRESULT PopulateSourcesFromProgramHeaderOrBitcode(IDxcBlob *pProgramBlob) {
    UINT32 bitcode_size = 0;
    const char *bitcode = nullptr;

    if (hlsl::IsValidDxilProgramHeader(
            (hlsl::DxilProgramHeader *)pProgramBlob->GetBufferPointer(),
            pProgramBlob->GetBufferSize())) {
      hlsl::GetDxilProgramBitcode(
          (hlsl::DxilProgramHeader *)pProgramBlob->GetBufferPointer(), &bitcode,
          &bitcode_size);
    } else if (IsBitcode(pProgramBlob->GetBufferPointer(),
                         pProgramBlob->GetBufferSize())) {
      bitcode = (char *)pProgramBlob->GetBufferPointer();
      bitcode_size = pProgramBlob->GetBufferSize();
    } else {
      return E_INVALIDARG;
    }

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> pModule;

    // NOTE: this doesn't copy the memory, just references it.
    std::unique_ptr<llvm::MemoryBuffer> mb =
        llvm::MemoryBuffer::getMemBuffer(StringRef(bitcode, bitcode_size), "-",
                                         /*RequiresNullTerminator*/ false);

    // Lazily parse the module
    std::string DiagStr;
    pModule = hlsl::dxilutil::LoadModuleFromBitcodeLazy(std::move(mb), context,
                                                        DiagStr);
    if (!pModule)
      return E_FAIL;

    // Materialize only the stuff we need, so it's fast
    {
      llvm::StringRef DebugMetadataList[] = {
          hlsl::DxilMDHelper::kDxilSourceContentsMDName,
          hlsl::DxilMDHelper::kDxilSourceDefinesMDName,
          hlsl::DxilMDHelper::kDxilSourceArgsMDName,
          hlsl::DxilMDHelper::kDxilVersionMDName,
          hlsl::DxilMDHelper::kDxilShaderModelMDName,
          hlsl::DxilMDHelper::kDxilEntryPointsMDName,
          hlsl::DxilMDHelper::kDxilSourceMainFileNameMDName,
      };
      pModule->materializeSelectNamedMetadata(DebugMetadataList);
    }

    hlsl::DxilModule &DM = pModule->GetOrCreateDxilModule();
    IFR(Utf8ToBlobWide(DM.GetEntryFunctionName(), &m_EntryPoint));
    IFR(Utf8ToBlobWide(DM.GetShaderModel()->GetName(), &m_TargetProfile));

    // For each all the named metadata node in the module
    for (llvm::NamedMDNode &node : pModule->named_metadata()) {
      llvm::StringRef node_name = node.getName();

      // dx.source.content
      if (node_name == hlsl::DxilMDHelper::kDxilSourceContentsMDName ||
          node_name == hlsl::DxilMDHelper::kDxilSourceContentsOldMDName) {
        for (unsigned i = 0; i < node.getNumOperands(); i++) {
          llvm::MDTuple *tup = cast<llvm::MDTuple>(node.getOperand(i));
          MDString *md_name = cast<MDString>(tup->getOperand(0));
          MDString *md_content = cast<MDString>(tup->getOperand(1));
          AddSource(md_name->getString(), md_content->getString());
        }
      }
      // dx.source.mainFileName
      else if (node_name == hlsl::DxilMDHelper::kDxilSourceMainFileNameMDName ||
               node_name ==
                   hlsl::DxilMDHelper::kDxilSourceMainFileNameOldMDName) {
        MDTuple *tup = cast<MDTuple>(node.getOperand(0));
        MDString *str = cast<MDString>(tup->getOperand(0));
        std::string normalized = hlsl::NormalizePath(str->getString());
        m_MainFileName =
            nullptr; // This may already be set from reading dx.source content.
                     // If we have a dx.source.mainFileName, we want to use that
                     // here as the source of truth. Set it to nullptr to avoid
                     // leak (and assert).
        IFR(Utf8ToBlobWide(normalized, &m_MainFileName));
      }
      // dx.source.args
      else if (node_name == hlsl::DxilMDHelper::kDxilSourceArgsMDName ||
               node_name == hlsl::DxilMDHelper::kDxilSourceArgsOldMDName) {
        MDTuple *tup = cast<MDTuple>(node.getOperand(0));
        std::vector<const char *> args;
        // Args
        for (unsigned i = 0; i < tup->getNumOperands(); i++) {
          StringRef arg = cast<MDString>(tup->getOperand(i))->getString();
          args.push_back(arg.data());
        }

        std::vector<std::pair<std::string, std::string>> Pairs =
            ComputeArgPairs(args);
        for (std::pair<std::string, std::string> &p : Pairs) {
          IFR(AddArgPair(p.first, p.second));
        }
      }
    }

    return S_OK;
  }

  HRESULT HandleDxilContainer(IDxcBlob *pContainer,
                              IDxcBlob **ppDebugProgramBlob) {
    const hlsl::DxilContainerHeader *header =
        (const hlsl::DxilContainerHeader *)m_ContainerBlob->GetBufferPointer();
    for (auto it = hlsl::begin(header); it != hlsl::end(header); it++) {
      const hlsl::DxilPartHeader *part = *it;
      hlsl::DxilFourCC four_cc = (hlsl::DxilFourCC)part->PartFourCC;

      switch (four_cc) {

      case hlsl::DFCC_CompilerVersion: {
        const hlsl::DxilCompilerVersion *header =
            (const hlsl::DxilCompilerVersion *)(part + 1);
        m_VersionInfo = *header;
        m_HasVersionInfo = true;

        const char *ptr = (const char *)(header + 1);
        unsigned i = 0;

        {
          unsigned commitShaLength = 0;
          const char *commitSha = (const char *)(header + 1) + i;
          for (; i < header->VersionStringListSizeInBytes; i++) {
            if (ptr[i] == 0) {
              i++;
              break;
            }
            commitShaLength++;
          }
          m_VersionCommitSha.assign(commitSha, commitShaLength);
        }

        {
          const char *versionString = (const char *)(header + 1) + i;
          unsigned versionStringLength = 0;
          for (; i < header->VersionStringListSizeInBytes; i++) {
            if (ptr[i] == 0) {
              i++;
              break;
            }
            versionStringLength++;
          }
          m_VersionString.assign(versionString, versionStringLength);
        }

      } break;

      case hlsl::DFCC_ShaderPDBInfo: {
        const hlsl::DxilShaderPDBInfo *header =
            (const hlsl::DxilShaderPDBInfo *)(part + 1);
        IFR(LoadFromPDBInfoPart(header, part->PartSize));
      } break;

      // This is now legacy.
      case hlsl::DFCC_ShaderSourceInfo: {
        const hlsl::DxilSourceInfo *header =
            (const hlsl::DxilSourceInfo *)(part + 1);
        hlsl::SourceInfoReader reader;
        if (!reader.Init(header, part->PartSize)) {
          Reset();
          return E_FAIL;
        }

        // Args
        for (unsigned i = 0; i < reader.GetArgPairCount(); i++) {
          const hlsl::SourceInfoReader::ArgPair &pair = reader.GetArgPair(i);
          IFR(AddArgPair(pair.Name, pair.Value));
        }

        // Sources
        for (unsigned i = 0; i < reader.GetSourcesCount(); i++) {
          hlsl::SourceInfoReader::Source source_data = reader.GetSource(i);
          IFR(AddSource(source_data.Name, source_data.Content));
        }

      } break;

      case hlsl::DFCC_ShaderHash: {
        const hlsl::DxilShaderHash *hash_header =
            (const hlsl::DxilShaderHash *)(part + 1);
        IFR(hlsl::DxcCreateBlobOnHeapCopy(hash_header, sizeof(*hash_header),
                                          &m_HashBlob));
      } break;

      case hlsl::DFCC_ShaderDebugName: {
        const hlsl::DxilShaderDebugName *name_header =
            (const hlsl::DxilShaderDebugName *)(part + 1);
        const char *ptr = (const char *)(name_header + 1);
        IFR(Utf8ToBlobWide(ptr, &m_Name));
      } break;

      case hlsl::DFCC_ShaderDebugInfoDXIL: {
        const hlsl::DxilProgramHeader *program_header =
            (const hlsl::DxilProgramHeader *)(part + 1);

        CComPtr<IDxcBlob> pProgramHeaderBlob;
        IFR(hlsl::DxcCreateBlobFromPinned(
            program_header, program_header->SizeInUint32 * sizeof(UINT32),
            &pProgramHeaderBlob));
        IFR(pProgramHeaderBlob.QueryInterface(ppDebugProgramBlob));

      } break; // hlsl::DFCC_ShaderDebugInfoDXIL
      }        // switch (four_cc)
    }          // For each part

    return S_OK;
  }

  HRESULT AddArgPair(StringRef name, StringRef value) {
    const llvm::opt::OptTable *optTable = hlsl::options::getHlslOptTable();

    // If the value for define is somehow empty, do not add it.
    if (name == "D" && value.empty())
      return S_OK;

    SmallVector<char, 32> fusedArgStorage;
    if (name.size() && value.size()) {
      // Handling case where old positional arguments used to have
      // <input> written as the option name.
      if (name == "<input>") {
        name = "";
      }
      // Check if the option and its value must be merged. Newer compiler
      // pre-merge them before writing them to the PDB, but older PDBs might
      // have them separated.
      else {
        llvm::opt::Option opt = optTable->findOption(name.data());
        if (opt.isValid()) {
          if (opt.getKind() == llvm::opt::Option::JoinedClass) {
            StringRef newName =
                (Twine(name) + Twine(value)).toStringRef(fusedArgStorage);
            name = newName;
            value = "";
          }
        }
      }
    }

    CComPtr<IDxcBlobWide> pValueBlob;
    CComPtr<IDxcBlobWide> pNameBlob;
    if (name.size())
      IFR(Utf8ToBlobWide(name, &pNameBlob));
    if (value.size())
      IFR(Utf8ToBlobWide(value, &pValueBlob));

    bool excludeFromFlags = false;
    if (name == "E") {
      m_EntryPoint = pValueBlob;
      excludeFromFlags = true;
    } else if (name == "T") {
      m_TargetProfile = pValueBlob;
      excludeFromFlags = true;
    } else if (name == "D") {
      m_Defines.push_back(pValueBlob);
      excludeFromFlags = true;
    }

    CComPtr<IDxcBlobWide> pNameWithDashBlob;
    if (name.size()) {
      SmallVector<char, 32> nameWithDashStorage;
      StringRef nameWithDash =
          (Twine("-") + Twine(name)).toStringRef(nameWithDashStorage);
      IFR(Utf8ToBlobWide(nameWithDash, &pNameWithDashBlob));
    }

    if (!excludeFromFlags) {
      if (pNameWithDashBlob)
        m_Flags.push_back(pNameWithDashBlob);
      if (pValueBlob)
        m_Flags.push_back(pValueBlob);
    }

    if (pNameWithDashBlob)
      m_Args.push_back(pNameWithDashBlob);
    if (pValueBlob)
      m_Args.push_back(pValueBlob);

    ArgPair newPair;
    newPair.Name = pNameBlob;
    newPair.Value = pValueBlob;
    m_ArgPairs.push_back(std::move(newPair));

    return S_OK;
  }

  bool NeedToLookInILDB() {
    return !m_SourceFiles.size() && m_LibraryPdbs.empty();
  }

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_ALLOC(DxcPdbUtils)

  DxcPdbUtils(IMalloc *pMalloc)
      : m_Adapter(this), m_dwRef(0), m_pMalloc(pMalloc) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
#ifdef _WIN32
    HRESULT hr =
        DoBasicQueryInterface<IDxcPdbUtils2, IDxcPixDxilDebugInfoFactory>(
            this, iid, ppvObject);
#else
    HRESULT hr = DoBasicQueryInterface<IDxcPdbUtils2>(this, iid, ppvObject);
#endif
    if (FAILED(hr)) {
      return DoBasicQueryInterface<IDxcPdbUtils>(&m_Adapter, iid, ppvObject);
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE Load(IDxcBlob *pPdbOrDxil) override {

    if (!pPdbOrDxil)
      return E_POINTER;

    try {
      DxcThreadMalloc TM(m_pMalloc);

      ::llvm::sys::fs::MSFileSystem *msfPtr = nullptr;
      IFT(CreateMSFileSystemForDisk(&msfPtr));
      std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);

      ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
      IFTLLVM(pts.error_code());

      // Remove all the data
      Reset();

      m_InputBlob = pPdbOrDxil;

      CComPtr<IStream> pStream;
      IFR(hlsl::CreateReadOnlyBlobStream(pPdbOrDxil, &pStream));

      // PDB
      if (SUCCEEDED(hlsl::pdb::LoadDataFromStream(m_pMalloc, pStream,
                                                  &m_ContainerBlob))) {
        IFR(HandleDxilContainer(m_ContainerBlob, &m_pDebugProgramBlob));
        if (NeedToLookInILDB()) {
          if (m_pDebugProgramBlob) {
            IFR(PopulateSourcesFromProgramHeaderOrBitcode(m_pDebugProgramBlob));
          } else {
            return E_FAIL;
          }
        }
      }
      // DXIL Container
      else if (hlsl::IsValidDxilContainer((const hlsl::DxilContainerHeader *)
                                              pPdbOrDxil->GetBufferPointer(),
                                          pPdbOrDxil->GetBufferSize())) {
        m_ContainerBlob = pPdbOrDxil;
        IFR(HandleDxilContainer(m_ContainerBlob, &m_pDebugProgramBlob));
        // If we have a Debug DXIL, populate the debug info.
        if (m_pDebugProgramBlob && NeedToLookInILDB()) {
          IFR(PopulateSourcesFromProgramHeaderOrBitcode(m_pDebugProgramBlob));
        }
      }
      // DXIL program header or bitcode
      else {
        CComPtr<IDxcBlob> pProgramHeaderBlob;
        IFR(hlsl::DxcCreateBlobFromPinned(
            (hlsl::DxilProgramHeader *)pPdbOrDxil->GetBufferPointer(),
            pPdbOrDxil->GetBufferSize(), &pProgramHeaderBlob));

        IFR(pProgramHeaderBlob.QueryInterface(&m_pDebugProgramBlob));
        IFR(PopulateSourcesFromProgramHeaderOrBitcode(m_pDebugProgramBlob));
      }

      IFR(SetEntryPointToDefaultIfEmpty());
    }
    CATCH_CPP_RETURN_HRESULT();

    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetSourceCount(UINT32 *pCount) override {
    if (!pCount)
      return E_POINTER;
    *pCount = (UINT32)m_SourceFiles.size();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetSource(UINT32 uIndex, IDxcBlobEncoding **ppResult) override {
    if (uIndex >= m_SourceFiles.size())
      return E_INVALIDARG;
    if (!ppResult)
      return E_POINTER;
    *ppResult = nullptr;
    return m_SourceFiles[uIndex].Content.QueryInterface(ppResult);
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetSourceName(UINT32 uIndex, IDxcBlobWide **ppResult) override {
    if (uIndex >= m_SourceFiles.size())
      return E_INVALIDARG;
    return m_SourceFiles[uIndex].Name.QueryInterface(ppResult);
  }

  static inline HRESULT
  GetStringCount(const std::vector<CComPtr<IDxcBlobWide>> &list,
                 UINT32 *pCount) {
    if (!pCount)
      return E_POINTER;
    *pCount = (UINT32)list.size();
    return S_OK;
  }

  static inline HRESULT
  GetStringOption(const std::vector<CComPtr<IDxcBlobWide>> &list, UINT32 uIndex,
                  IDxcBlobWide **ppResult) {
    if (uIndex >= list.size())
      return E_INVALIDARG;
    return list[uIndex].QueryInterface(ppResult);
  }

  virtual HRESULT STDMETHODCALLTYPE GetFlagCount(UINT32 *pCount) override {
    return GetStringCount(m_Flags, pCount);
  }
  virtual HRESULT STDMETHODCALLTYPE GetFlag(UINT32 uIndex,
                                            IDxcBlobWide **ppResult) override {
    return GetStringOption(m_Flags, uIndex, ppResult);
  }
  virtual HRESULT STDMETHODCALLTYPE GetArgCount(UINT32 *pCount) override {
    return GetStringCount(m_Args, pCount);
  }
  virtual HRESULT STDMETHODCALLTYPE GetArg(UINT32 uIndex,
                                           IDxcBlobWide **ppResult) override {
    return GetStringOption(m_Args, uIndex, ppResult);
  }
  virtual HRESULT STDMETHODCALLTYPE GetDefineCount(UINT32 *pCount) override {
    return GetStringCount(m_Defines, pCount);
  }
  virtual HRESULT STDMETHODCALLTYPE
  GetDefine(UINT32 uIndex, IDxcBlobWide **ppResult) override {
    return GetStringOption(m_Defines, uIndex, ppResult);
  }

  virtual HRESULT STDMETHODCALLTYPE GetArgPairCount(UINT32 *pCount) override {
    if (!pCount)
      return E_POINTER;
    *pCount = (UINT32)m_ArgPairs.size();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetArgPair(
      UINT32 uIndex, IDxcBlobWide **ppName, IDxcBlobWide **ppValue) override {
    if (!ppName || !ppValue)
      return E_POINTER;
    if (uIndex >= m_ArgPairs.size())
      return E_INVALIDARG;
    const ArgPair &pair = m_ArgPairs[uIndex];

    *ppName = nullptr;
    *ppValue = nullptr;

    if (pair.Name) {
      IFR(pair.Name.QueryInterface(ppName));
    }

    if (pair.Value) {
      IFR(pair.Value.QueryInterface(ppValue));
    }

    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetTargetProfile(IDxcBlobWide **ppResult) override {
    return CopyBlobWide(m_TargetProfile, ppResult);
  }
  virtual HRESULT STDMETHODCALLTYPE
  GetEntryPoint(IDxcBlobWide **ppResult) override {
    return CopyBlobWide(m_EntryPoint, ppResult);
  }
  virtual HRESULT STDMETHODCALLTYPE
  GetMainFileName(IDxcBlobWide **ppResult) override {
    return CopyBlobWide(m_MainFileName, ppResult);
  }

  virtual BOOL STDMETHODCALLTYPE IsFullPDB() override {
    return m_pDebugProgramBlob != nullptr;
  }
  virtual BOOL STDMETHODCALLTYPE IsPDBRef() override {
    return m_LibraryPdbs.size() == 0 && m_SourceFiles.size() == 0 &&
           !m_WholeDxil;
  }

  HRESULT SetEntryPointToDefaultIfEmpty() {
    // Entry point might have been omitted. Set it to main by default.
    // Don't set entry point if this instance is non-debug DXIL and has no
    // arguments at all.
    if ((!m_EntryPoint || m_EntryPoint->GetStringLength() == 0) &&
        !m_ArgPairs.empty()) {
      // Don't set the name if the target is a lib
      if (!m_TargetProfile || m_TargetProfile->GetStringLength() < 3 ||
          0 != wcsncmp(m_TargetProfile->GetStringPointer(), L"lib", 3)) {
        m_EntryPoint = nullptr;
        IFR(Utf8ToBlobWide("main", &m_EntryPoint));
      }
    }
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetHash(IDxcBlob **ppResult) override {
    if (!ppResult)
      return E_POINTER;
    *ppResult = nullptr;
    if (m_HashBlob)
      return m_HashBlob.QueryInterface(ppResult);
    return E_FAIL;
  }

  virtual HRESULT STDMETHODCALLTYPE GetWholeDxil(IDxcBlob **ppResult) override {
    if (!ppResult)
      return E_POINTER;
    *ppResult = nullptr;
    if (m_WholeDxil)
      return m_WholeDxil.QueryInterface(ppResult);
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetName(IDxcBlobWide **ppResult) override {
    return CopyBlobWide(m_Name, ppResult);
  }

#ifdef _WIN32
  virtual STDMETHODIMP
  NewDxcPixDxilDebugInfo(IDxcPixDxilDebugInfo **ppDxilDebugInfo) override {
    if (!m_pDebugProgramBlob)
      return E_FAIL;

    DxcThreadMalloc TM(m_pMalloc);

    CComPtr<IDiaDataSource> pDataSource;
    IFR(DxcCreateInstance2(m_pMalloc, CLSID_DxcDiaDataSource,
                           IID_PPV_ARGS(&pDataSource)));

    CComPtr<IStream> pStream;
    IFR(hlsl::CreateReadOnlyBlobStream(m_pDebugProgramBlob, &pStream));

    IFR(pDataSource->loadDataFromIStream(pStream));

    CComPtr<IDiaSession> pSession;
    IFR(pDataSource->openSession(&pSession));

    CComPtr<IDxcPixDxilDebugInfoFactory> pFactory;
    IFR(pSession.QueryInterface(&pFactory));

    return pFactory->NewDxcPixDxilDebugInfo(ppDxilDebugInfo);
  }

  virtual STDMETHODIMP NewDxcPixCompilationInfo(
      IDxcPixCompilationInfo **ppCompilationInfo) override {
    return E_NOTIMPL;
  }

#endif

  virtual HRESULT STDMETHODCALLTYPE
  GetVersionInfo(IDxcVersionInfo **ppVersionInfo) override {
    if (!ppVersionInfo)
      return E_POINTER;

    *ppVersionInfo = nullptr;
    if (!m_HasVersionInfo)
      return E_FAIL;

    DxcThreadMalloc TM(m_pMalloc);

    CComPtr<DxcPdbVersionInfo> result =
        CreateOnMalloc<DxcPdbVersionInfo>(m_pMalloc);
    if (result == nullptr) {
      return E_OUTOFMEMORY;
    }
    result->m_Version = m_VersionInfo;
    result->m_VersionCommitSha = m_VersionCommitSha;
    result->m_VersionString = m_VersionString;
    *ppVersionInfo = result.Detach();
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetLibraryPDBCount(UINT32 *pCount) override {
    if (!pCount)
      return E_POINTER;
    *pCount = (UINT32)m_LibraryPdbs.size();
    return S_OK;
  }
  virtual HRESULT STDMETHODCALLTYPE
  GetLibraryPDB(UINT32 uIndex, IDxcPdbUtils2 **ppOutPdbUtils,
                IDxcBlobWide **ppLibraryName) override {
    if (!ppOutPdbUtils)
      return E_POINTER;
    if (uIndex >= m_LibraryPdbs.size())
      return E_INVALIDARG;

    LibraryEntry &Entry = m_LibraryPdbs[uIndex];
    hlsl::RDAT::DxilRuntimeData rdat;
    if (!rdat.InitFromRDAT(Entry.PdbInfo.data(), Entry.PdbInfo.size()))
      return E_FAIL;
    if (rdat.GetDxilPdbInfoTable().Count() != 1)
      return E_FAIL;

    auto reader = rdat.GetDxilPdbInfoTable()[0];

    CComPtr<DxcPdbUtils> pNewPdbUtils = DxcPdbUtils::Alloc(m_pMalloc);
    IFR(pNewPdbUtils->LoadFromPdbInfoReader(reader));
    pNewPdbUtils.QueryInterface(ppOutPdbUtils);

    if (ppLibraryName) {
      *ppLibraryName = nullptr;
      if (Entry.pName) {
        IFR(Entry.pName.QueryInterface(ppLibraryName));
      }
    }

    return S_OK;
  }
  virtual HRESULT STDMETHODCALLTYPE GetCustomToolchainID(UINT32 *pID) override {
    if (!pID)
      return E_POINTER;
    *pID = m_uCustomToolchainID;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  GetCustomToolchainData(IDxcBlob **ppBlob) override {
    if (!ppBlob)
      return E_POINTER;
    *ppBlob = nullptr;
    if (m_customToolchainData)
      return m_customToolchainData.QueryInterface(ppBlob);
    return S_OK;
  }
};

HRESULT CreateDxcPdbUtils(REFIID riid, LPVOID *ppv) {
  if (!ppv)
    return E_POINTER;
  *ppv = nullptr;
  if (riid == __uuidof(IDxcPdbUtils)) {
    CComPtr<DxcPdbUtils> pdbUtils2 =
        CreateOnMalloc<DxcPdbUtils>(DxcGetThreadMallocNoRef());
    if (!pdbUtils2)
      return E_OUTOFMEMORY;
    return pdbUtils2.p->QueryInterface(riid, ppv);
  } else {
    CComPtr<DxcPdbUtils> result =
        CreateOnMalloc<DxcPdbUtils>(DxcGetThreadMallocNoRef());
    if (result == nullptr)
      return E_OUTOFMEMORY;
    return result.p->QueryInterface(riid, ppv);
  }
  return E_NOINTERFACE;
}
